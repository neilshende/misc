/*
 * Copyright (C) 2012- Maxta, Inc
 */

#include "mxfs_i.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/uio.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/pagemap.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include "maxtaVersion.h"

MODULE_ALIAS_MISCDEV(MXFS_MINOR);

static const char *versionString = "MAXTA_VERSION "MAXTA_VERSION;

static struct kmem_cache *mxfs_req_cachep = NULL;

static void
mxfs_request_init(struct mxfs_req *req)
{
   memset(req, 0, sizeof(*req));
   INIT_LIST_HEAD(&req->list);
   spin_lock_init(&req->lock);
   init_waitqueue_head(&req->waitq);
}

struct mxfs_req *
mxfs_request_alloc(void)
{
   struct mxfs_req *req = kmem_cache_alloc(mxfs_req_cachep, GFP_KERNEL);
   if (req)
      mxfs_request_init(req);
   return req;
}

void
mxfs_request_free(struct mxfs_req *req)
{
   /* We should never free a request that is still processing. */
   BUG_ON(req->state == MXFS_REQ_PROCESSING);
   kmem_cache_free(mxfs_req_cachep, req);
}

struct mxfs_req *
mxfs_get_req(struct mxfs_conn *mconn)
{
   struct mxfs_req *req;
   int err;

   err = -EAGAIN;
   spin_lock(&mconn->lock);
   if (!mconn->connected || !mconn->mounted) {
      spin_unlock(&mconn->lock);
      goto out;
   }
   spin_unlock(&mconn->lock);

   req = mxfs_request_alloc();
   err = -ENOMEM;
   if (!req)
      goto out;

   return req;

out:
   return ERR_PTR(err);
}

void
mxfs_put_request(struct mxfs_conn *mconn, struct mxfs_req *req)
{
   mxfs_request_free(req);
}

static unsigned
len_args(unsigned numargs, struct mxfs_arg *args)
{
   unsigned nbytes = 0;
   unsigned i;

   for (i = 0; i < numargs; i++)
      nbytes += args[i].size;

   return nbytes;
}

static u64
mxfs_get_unique(struct mxfs_conn *mconn)
{
   mconn->reqctr++;
   /* zero is special */
   if (mconn->reqctr == 0)
      mconn->reqctr = 1;

   return mconn->reqctr;
}

static void
queue_request(struct mxfs_conn *mconn, struct mxfs_req *req)
{
   req->in.h.unique = mxfs_get_unique(mconn);
   req->mlen = sizeof (struct mxfs_header);
   req->dlen = len_args(req->in.numargs, (struct mxfs_arg *) req->in.args);
   spin_lock(&req->lock);
   list_add_tail(&req->list, &mconn->pending);
   req->state = MXFS_REQ_PROCESSING;
   spin_unlock(&req->lock);
   wake_up(&mconn->waitq);
}


/*
 * Function to end a request while it is pending with mfsd in userland.
 * We acquire the connection and the request's lock, ensuring that the
 * mxfs device read/write code is not fiddling with it. If the request
 * is still not marked as finished, remove it from whatever list it
 * is on (waiting to be sent to userland, or waiting for reply from
 * userland).*
 *
 * This will ensure that we don't free any resources from underneath
 * the device read/write code, if the request completes after it was
 * interrupted.
 */
static void
mxfs_interrupted_request(struct mxfs_conn *mconn,
                         struct mxfs_req *req,
                         int error)
{
   spin_lock(&mconn->lock);
   spin_lock(&req->lock);
   /* When we get here we know the mxfs device read/write code is
    * not fiddling with this request.
    */
   if (req->state != MXFS_REQ_FINISHED) {
      /* The request is processing. Remove it from the list of pending
       * requests, so that it is safe to terminate this request.
       */
      list_del(&req->list);
      req->state = MXFS_REQ_FINISHED;
   }
   req->out.h.error = error;
   spin_unlock(&req->lock);
   spin_unlock(&mconn->lock);
}


static void
mxfs_handle_interrupted_wait(struct mxfs_conn *mconn,
                             struct mxfs_req *req,
                             long result)
{
   /* We must have timed out or an error condition has occurred. */
   BUG_ON(result > 0);

   /* We woke up either because of a signal or a timeout. */
   switch (result) {
   case -ERESTARTSYS:
      /* Interrupted system call. */
      break;
   case 0:
      /* Timer went off. -- We should log a message. */
      LOG_ERROR("Timeout waiting for operation %llu (%u). %d seconds elapsed.",
                req->in.h.unique, (unsigned int)req->in.h.unique, requestTimeout);
      result = -EAGAIN;
      break;
   default:
      /* Hmm... -- Other error than interrupted system call or timeout.
       * Maybe we should log this...
       */
      LOG_ERROR("Unexpected error %ld while waiting for completion of "
                "operation %llu (%u).", result, req->in.h.unique,
                (unsigned int)req->in.h.unique);
      break;
   }
   mxfs_interrupted_request(mconn, req, result);
}

void
mxfs_request_send(struct mxfs_conn *mconn, struct mxfs_req *req)
{
   long result;
   bool done = false;
   long timeout = requestTimeout * HZ;

   spin_lock(&mconn->lock);
   if (!mconn->connected) {
      req->out.h.error = -EAGAIN;
      spin_unlock(&mconn->lock);
   } else {
      //LOG_ERROR("XXX RS QUEUEING REQUEST %llu", req->in.h.unique);
      queue_request(mconn, req);
      spin_unlock(&mconn->lock);

      do {
         //LOG_ERROR("XXX RS WAIT FOR REQUEST %llu", req->in.h.unique);
         result = wait_event_interruptible_timeout(req->waitq,
                                                   req->state == MXFS_REQ_FINISHED,
                                                   timeout);
         //LOG_ERROR("XXX RS REPLY TO REQUEST %llu: %ld", req->in.h.unique, result);
         if (result <= 0) {
            mxfs_handle_interrupted_wait(mconn, req, result);
            done = true;
         } else {
            /* Confirm we are REALLY in finished state */
            spin_lock(&req->lock);
            done = (req->state == MXFS_REQ_FINISHED);
            spin_unlock(&req->lock);

            if (!done) {
               /* wait_event returned the number of jiffies remaining... */
               timeout = result;
               LOG_ERROR("ODDBALL CASE FOR REQUEST %llu", req->in.h.unique);
            }
         }
      } while (!done);
   }
}


static int
copy_in_write_args(char *buf, struct iov_iter *ioviter)
{
   while (iov_iter_count(ioviter) > 0) {
      ssize_t len = iov_iter_single_seg_count(ioviter);

      if (copy_to_user(buf, ioviter->iov->iov_base + ioviter->iov_offset, len)) {
         return - EFAULT;
      }

      buf += len;
      iov_iter_advance(ioviter, len);
   }

   return 0;
}


static int
copy_in_args(char *buf, struct mxfs_in_arg *args, unsigned long numargs)
{
   int err = 0;
   unsigned long i;

   for (i = 0; i < numargs; i++)  {
      if (args[i].value) {
         if (copy_to_user(buf, args[i].value, args[i].size)) {
            err = -EFAULT;
            break;
         }
      }

      buf += args[i].size;
   }

   return err;
}

static int
request_pending(struct mxfs_conn *mconn)
{
   return !list_empty(&mconn->pending);
}


/*
 * Find a pending request based on its cookie. Must be
 * called with mconn->lock held. Also, caller should
 * grab req->lock before releasing mconn->lock.
 */
static struct mxfs_req *
mxfs_find_pending_request(struct mxfs_conn *mconn, __u64 cookie)
{
   struct mxfs_req *req = NULL;
   struct list_head *ptr;
   int slot = cookie % MXFS_PROCESSING_TABLE_SIZE;

   list_for_each(ptr, &(mconn->processing[slot])) {
      req = list_entry(ptr, struct mxfs_req, list);
      if (req->in.h.unique == cookie) {
         break;
      }
   }

   return req;
}


/*
 * We were unable to complete the transfer of a request to userland,
 * so we have to simulate an error being returned from userland and
 * transtion the request to REQ_FINISHED.
 */
static void
mxfs_terminate_request(struct mxfs_conn *mconn, __u64 cookie, int error)
{
   struct mxfs_req *req;

   spin_lock(&mconn->lock);
   req = mxfs_find_pending_request(mconn, cookie);
   if (req != NULL) {
      /* Request is still pending (caller has not hit a timeout or was
       * interrupted). Turn request around with specified error.
       */
      spin_lock(&req->lock);
      spin_unlock(&mconn->lock);
      req->out.h.error = error;
      req->state = MXFS_REQ_FINISHED;
      wake_up_interruptible(&req->waitq);
      spin_unlock(&req->lock);
      return;
   }
   spin_unlock(&mconn->lock);
}


/*
 * Read a single request into the userspace filesystem's buffer.  This
 * function waits until a request is available, then removes it from
 * the pending list and copies request data to userspace buffer.  If
 * no reply is needed (FORGET) or request has been aborted or there
 * was an error during the copying then it's finished by calling
 * request_end().  Otherwise add it to the processing list, and set
 * the 'sent' flag.
 *
 * Assumes: one and only one reader in userspace
 */
static ssize_t
mxfs_dev_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
   struct mxfs_conn *mconn = mxfs_get_conn();
   size_t nlen = 0;
   struct mxfs_req *req;
   struct mxfs_in *in;
   size_t nsize;
   int err = 0;
   int slot;
   __u64 cookie;
   unsigned int requestsRemaining = maxRequestsPerBuffer;

   if (!mconn) {
      LOG_ERROR("read on mxfs device failed, mxfs is not initialized");
      return -EINVAL;
   }

   spin_lock(&mconn->lock);
   if (!mconn->connected) {
      spin_unlock(&mconn->lock);
      LOG_ERROR("read from mxfs device failed, mxfs device is not opened");
      return -EAGAIN;
   }

   if (! request_pending(mconn)) {
      spin_unlock(&mconn->lock);
      LOG_ERROR("read from mxfs device failed, request not pending.");
      return -EAGAIN;
   }

   spin_unlock(&mconn->lock);
   while (nlen < len && requestsRemaining > 0) {
      spin_lock(&mconn->lock);
      if (list_empty(&mconn->pending)) {
         spin_unlock(&mconn->lock);
         break;
      }

      //LOG_ERROR("XXX DR Looking.");

      req = list_entry(mconn->pending.next, struct mxfs_req, list);

      /* Prevent the request from being freed from underneath us. */
      spin_lock(&req->lock);

      //LOG_ERROR("XXX DR PREPARING REQUEST %llu", req->in.h.unique);

      /*
       * the proto here is - starting on 8-byte aligned boundary
       * msg len     - 4 bytes
       * data len    - 4 bytes
       * msg buffer  - 8-byte aligned
       * data buffer - 8-byte aligned
       */
      nsize = sizeof (req->mlen) + sizeof (req->dlen) +
            ROUNDUP_8BYTE(req->mlen) + req->dlen;

      /*
       * check to validate if we have buffer large enough to accomodate
       * io request
       */
      if (nsize > len) {
         LOG_ERROR("io buffer not big enough to process request opcode %d - "
                   "buffer size %ld, request size %ld",
                   req->in.h.opcode, len, nsize);
         list_del(&req->list);
         req->out.h.error = -EIO;
         req->state = MXFS_REQ_FINISHED;
         wake_up_interruptible(&req->waitq);
         spin_unlock(&req->lock);
         spin_unlock(&mconn->lock);
         continue;
      }

      /* Request does not exceed max. size. -- But check if there is
       * enough space available in the user-supplied buffer, or
       * whether we have to wait for the next read request.
       */
      nsize += ROUNDUP_8BYTE(nlen);
      if (nsize > len) {
         /* Wait for next read... */
         spin_unlock(&req->lock);
         spin_unlock(&mconn->lock);
         break;
      }

      /* Request can fit into buffer. -- Move it to "processing" */
      slot = req->in.h.unique % MXFS_PROCESSING_TABLE_SIZE;
      list_move(&req->list, &mconn->processing[slot]);
      req->state = MXFS_REQ_PROCESSING;
      spin_unlock(&mconn->lock);


      nsize = ROUNDUP_8BYTE(nlen);
      in = &req->in;
      if (copy_to_user(((char __user *) buf + nsize),
                                       &req->mlen, sizeof (req->mlen))) {
         err = -EFAULT;
         break;
      }

      nsize += sizeof (req->mlen);
      if (copy_to_user(((char __user *) buf + nsize),
                                       &req->dlen, sizeof (req->dlen))) {
         err = -EFAULT;
         break;
      }

      nsize += sizeof (req->dlen);
      if (copy_to_user(((char __user *) buf + nsize),
                                        &in->h, sizeof (struct mxfs_in))) {
         err = -EFAULT;
         break;
      }

      nsize += ROUNDUP_8BYTE(req->mlen);
      switch(in->h.opcode) {
      case MXFS_WRITE:
         if (copy_to_user(((char __user *) buf + nsize),
                                        in->args[0].value, in->args[0].size)) {
            err = -EFAULT;
            LOG_ERROR("read on mxfs device failed @1 with error %d, "
                  "cannot copy write request args", err);
            break;
         }
         err = copy_in_write_args(
                              ((char __user *) buf + nsize + in->args[0].size),
                              req->ioviter);
         if (err) {
            LOG_ERROR("read on mxfs device failed @2 with error %d, "
                  "cannot copy write request data", err);
         }

         break;

      default:
         err = copy_in_args(((char __user *) buf + nsize),
                                 (struct mxfs_in_arg *) in->args, in->numargs);
         if (err) {
            LOG_ERROR("read on mxfs device failed @3 with error %d, "
                  "cannot copy request opcode %d args", err, req->in.h.opcode);
         }

         break;
      }

      if (err) {
         break;
      }

      /* request was enqueued successfully. */
      nsize += req->dlen;
      spin_unlock(&req->lock);
      nlen = nsize;

      requestsRemaining -= 1;
   }

   if (err != 0) {
      /* The latest request failed to transfer to user space.
       * We need to turn it around right now, since we will
       * never get a callback from user space that terminates it.
       * However, return success to user space below if we have
       * previous requests that were successfully transfered.
       */
      LOG_ERROR("Request %llu failed to transfer to userland: %d",
                req->in.h.unique, err);
      cookie = req->in.h.unique;
      spin_unlock(&req->lock);
      mxfs_terminate_request(mconn, cookie, err);
   }

   if (nlen) {
      return nlen;
   }

   BUG_ON(err > 0);
   return err;
}

int
mxfs_notify_inval_inode(struct mxfs_conn *mconn,
                        struct mxfs_notify_inval_inode_out *outarg)
{
   int err;

   down_read(&mconn->killsb);
   if (!mconn->sb) {
      up_read(&mconn->killsb);
      return -ENOENT;
   }

   err = mxfs_reverse_inval_inode(mconn->sb, outarg->ino, outarg->off, outarg->len);
   up_read(&mconn->killsb);
   return err;
}

int
mxfs_notify_inval_entry(struct mxfs_conn *mconn,
                        struct mxfs_notify_inval_entry_out *outarg,
                        struct qstr *name)

{
   int err;

   down_read(&mconn->killsb);
   if (!mconn->sb) {
      up_read(&mconn->killsb);
      return -ENOENT;
   }

   err = mxfs_reverse_inval_entry(mconn->sb, outarg->parent, name);
   up_read(&mconn->killsb);
   return err;
}


/**
 * Copy data read from user space (buf/len) into the target buffer
 * identified by ioviter. -- The iterator is NOT advanced.
 */

static int
copy_out_read_args(struct iov_iter *ioviter,
                   const char __user *buf, unsigned long len)
{
   unsigned long nlen = 0;
   int err = 0;
   struct iov_iter tmp_iter = *ioviter;

   if (tmp_iter.count < len) {
      err = -EINVAL;
      LOG_ERROR("copying read args failed with err %d, "
                "max. expected: %lu, got: %lu", err, tmp_iter.count, len);
   } else {

      while (len > 0) {
         nlen = iov_iter_single_seg_count(&tmp_iter);
         if (nlen > len) {
            nlen = len;
         }
         if (copy_from_user(tmp_iter.iov->iov_base + tmp_iter.iov_offset, buf, nlen)) {
            err = -EFAULT;
            break;
         }

         buf += nlen;
         len -= nlen;
         iov_iter_advance(&tmp_iter, nlen);
      }
   }

   return err;
}


static int
copy_out_args(struct mxfs_arg *args, unsigned long numargs,
               const char __user *buf, unsigned long len)
{
   unsigned long i;
   unsigned long nlen = 0;
   int err = 0;

   for (i = 0; i < numargs; i++) {
      if (nlen + args[i].size > len) {
         err = -EINVAL;
         LOG_ERROR("copying out args failed with err %d, "
               "index %ld numargs %ld nlen %ld size %ld len %ld",
               err, i, numargs, nlen, args[i].size, len);
         break;
      }

      if (copy_from_user(args[i].value, buf, args[i].size)) {
         err = -EFAULT;
         break;
      }

      buf += args[i].size;
      nlen += args[i].size;
   }

   return err;
}

/*
 * Write a single reply to a request.  First the header is copied from
 * the write buffer.  The request is then searched on the processing
 * list by the unique ID found in the header.  If found, then remove
 * it from the list and copy the rest of the buffer to the request.
 * The request is finished by calling request_end()
 */
static ssize_t
mxfs_dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
   struct mxfs_conn *mconn = mxfs_get_conn();
   size_t nlen = 0;
   unsigned mlen;
   unsigned dlen;
   struct mxfs_header oh;
   struct mxfs_req *req;
   int err = 0;
   if (!mconn) {
      LOG_ERROR("write to mxfs device failed, mxfs is not initialized");
      return -EINVAL;
   }

   spin_lock(&mconn->lock);
   if (!mconn->connected) {
      spin_unlock(&mconn->lock);
      LOG_ERROR("write to mxfs device failed, mxfs device is not opened");
      return -EAGAIN;
   }
   spin_unlock(&mconn->lock);

   if (len < sizeof(struct mxfs_header)) {
      LOG_ERROR("write to mxfs device failed, invalid mxfs header");
      return -EINVAL;
   }

   if (copy_from_user(&mlen, ((char __user *) buf + nlen), sizeof (mlen))) {
      LOG_ERROR("write to mxfs device failed, "
            "cannot copy message length args");
      return -EFAULT;
   }

   nlen += sizeof (mlen);
   if (copy_from_user(&dlen, ((char __user *) buf + nlen), sizeof (dlen))) {
      LOG_ERROR("write to mxfs device failed, cannot copy data length args");
      return -EFAULT;
   }

   nlen += sizeof (dlen);
   if (copy_from_user(&oh, ((char __user *) buf + nlen), sizeof (oh))) {
      LOG_ERROR("write to mxfs device failed, cannot copy mxfs header args");
      return -EFAULT;
   }

   nlen += sizeof (oh);
   nlen = ROUNDUP_8BYTE(nlen);
   if (nlen + dlen != len) {
      LOG_ERROR("write to mxfs device failed, data message not aligned");
      return -EINVAL;
   }

   spin_lock(&mconn->lock);
   if (!mconn->connected) {
      spin_unlock(&mconn->lock);
      LOG_ERROR("write to mxfs device failed, mxfs device is not opened");
      return -EAGAIN;
   }

   req = mxfs_find_pending_request(mconn, oh.unique);

   if (req == NULL) {
      spin_unlock(&mconn->lock);
      LOG_WARN("Ignoring reply to timed out or canceled request %llu.", oh.unique);
      return len;
   }

   spin_lock(&req->lock);

   /* If a request is found in the table of processing requests, its state
    * must be PROCESSING as well or something is seriously wrong...
    */
   BUG_ON(req->state != MXFS_REQ_PROCESSING);

   // Remove request from processing list
   list_del(&req->list);

   spin_unlock(&mconn->lock);
   req->out.h = oh;
   if (dlen) {
      switch(req->in.h.opcode) {
      case MXFS_READ:
         err = copy_out_read_args(req->ioviter,
                                  ((char __user *) buf + nlen), dlen);
         if (err) {
            LOG_ERROR("write to mxfs device failed with error %d on request %llu, "
                      "cannot copy read args", err, oh.unique);
         }

         break;

      default:
         err = copy_out_args((struct mxfs_arg *) &req->out.args,
                        req->out.numargs, ((char __user *) buf + nlen), dlen);
         if (err) {
            LOG_ERROR("write to mxfs device failed with error %d, "
                      "cannot copy request opcode %d args for request %llu",
                      err, req->in.h.opcode, oh.unique);
         }

         break;
      }
   }
   if (err) {
      req->out.h.error = err;
   }

   req->state = MXFS_REQ_FINISHED;
   // Attempt wakeup with req->lock held, since when not holding req->lock,
   // req might get freed from underneath us in case of -ERESTARTSYS.
   wake_up_interruptible(&req->waitq);
   spin_unlock(&req->lock);

   if (err != 0) {
      BUG_ON(err > 0);
      return err;
   }
   return len;
}

/*
 * Abort all requests on the given list (pending or processing)
 *
 * This function releases and reacquires mconn->lock
 */
static void
end_requests(struct mxfs_conn *mconn, struct list_head *head)
{
   while (!list_empty(head)) {
      struct mxfs_req *req;
      req = list_entry(head->next, struct mxfs_req, list);
      spin_lock(&req->lock);
      req->out.h.error = -EAGAIN;
      list_del(&req->list);
      req->state = MXFS_REQ_FINISHED;
      wake_up(&req->waitq);
      spin_unlock(&req->lock);
   }
}

void
mxfs_end_queued_requests(struct mxfs_conn *mconn)
{
   int i;

   end_requests(mconn, &mconn->pending);
   for (i = 0; i < MXFS_PROCESSING_TABLE_SIZE; i++) {
      end_requests(mconn, &(mconn->processing[i]));
   }
}

/*
 * Abort all requests.
 *
 * Emergency exit in case of a malicious or accidental deadlock, or
 * just a hung filesystem.
 *
 * The same effect is usually achievable through killing the
 * filesystem daemon and all users of the filesystem.  The exception
 * is the combination of an asynchronous request and the tricky
 * deadlock (see Documentation/filesystems/mxfs.txt).
 *
 * During the aborting, progression of requests from the pending and
 * processing lists onto the io list, and progression of new requests
 * onto the pending list is prevented by req->connected being false.
 *
 * Progression of requests under I/O to the processing list is
 * prevented by the req->aborted flag being true for these requests.
 * For this reason requests on the io list must be aborted first.
 */
void
mxfs_abort_conn(struct mxfs_conn *mconn)
{
   spin_lock(&mconn->lock);
   if (mconn->connected) {
      mconn->connected = 0;
      mxfs_end_queued_requests(mconn);
      wake_up_all(&mconn->waitq);
   }
   spin_unlock(&mconn->lock);
}

int
mxfs_dev_release(struct inode *inode, struct file *file)
{
   struct mxfs_conn *mconn = mxfs_get_conn();
   if (mconn) {
      mxfs_abort_conn(mconn);
   }

   return 0;
}

static long
mxfs_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
   int i;
   int version;
   struct mxfs_conn *mconn;
   long err;

   mconn = mxfs_get_conn();
   if (!mconn) {
      return -EACCES;
   }

   err = -ENOTTY;
   switch (cmd) {
      case MXFS_IOCTL_INIT:
         if (get_user(version, (int __user *) arg)) {
            return -EFAULT;
         }

         if (version != MXFS_VERSION) {
            LOG_ERROR("init failed, version %u mismatch", version);
            return -EINVAL;
         }

         spin_lock(&mconn->lock);
         if (!list_empty(&mconn->pending)) {
            spin_unlock(&mconn->lock);
            LOG_ERROR("ioctl init failed, pending list not empty");
            return -EBUSY;
         }


         for (i = 0; i < MXFS_PROCESSING_TABLE_SIZE; i++) {
            if (!list_empty(&(mconn->processing[i]))) {
               spin_unlock(&mconn->lock);
               LOG_ERROR("ioctl init failed, processing list not empty");
               return -EBUSY;
            }
         }

         if (mconn->connected) {
            spin_unlock(&mconn->lock);
            LOG_ERROR("ioctl init failed, already intitialized");
            return -EBUSY;
         }

         mconn->connected = 1;
         spin_unlock(&mconn->lock);
         err = 0;
         break;

      case MXFS_IOCTL_DEINIT:
         mxfs_abort_conn(mconn);
         err = 0;
         break;

      default:
         break;
   }

   return err;
}

static unsigned int mxfs_dev_poll(struct file *filp, poll_table *wait)
{
   struct mxfs_conn *mconn = mxfs_get_conn();
   unsigned int mask = 0;

   if (!mconn)
      return POLLERR;

   spin_lock(&mconn->lock);
   poll_wait(filp, &mconn->waitq,  wait);
   if (request_pending(mconn)) {
      mask |= POLLIN | POLLRDNORM;
   }

   if (!mconn->connected) {
      mask = POLLERR;
   }

   spin_unlock(&mconn->lock);
   return mask;
}

const struct file_operations mxfs_dev_operations = {
   .owner               = THIS_MODULE,
   .llseek              = no_llseek,
   .read                = mxfs_dev_read,
   .write               = mxfs_dev_write,
   .release             = mxfs_dev_release,
   .unlocked_ioctl      = mxfs_unlocked_ioctl,
   .poll                = mxfs_dev_poll,
};

static struct miscdevice mxfs_miscdevice = {
   .minor = MXFS_MINOR,
   .name  = "mxfs",
   .fops = &mxfs_dev_operations,
};


int __init mxfs_dev_init(void)
{
   int err = -ENOMEM;
   struct mxfs_conn *mconn;

   LOG_INFO("Loading mxfs module version %s", versionString);

   mxfs_req_cachep = kmem_cache_create("mxfs_request",
         sizeof(struct mxfs_req),
         0, 0, NULL);
   if (!mxfs_req_cachep)
      goto out;

   mconn = mxfs_conn_init();
   if (!mconn) {
      err = -ENOMEM;
      goto out_cache_clean;
   }

   err = misc_register(&mxfs_miscdevice);
   if (err)
      goto out_put_conn;

   return 0;

out_put_conn:
   mxfs_conn_put();
out_cache_clean:
   kmem_cache_destroy(mxfs_req_cachep);
out:
   return err;
}

void __exit mxfs_dev_cleanup(void)
{
   misc_deregister(&mxfs_miscdevice);
   mxfs_conn_put();
   kmem_cache_destroy(mxfs_req_cachep);
}

