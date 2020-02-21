#include "mxfs_i.h"

#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/aio.h>
#include <linux/version.h>

void
mxfs_update_stats(bool read_flag, unsigned long count,
        unsigned long latency)
{
    int index;
    unsigned long bw;
    if (latency) {
        bw = (count*1000000)/latency;
    } else {
        bw = (unsigned long)-1;
    }
    if (count<=512) {
        index = 0;
    } else if(count <= 4*1024) {
        index = 1;
    } else if (count <= 8*1024) {
        index = 2;
    } else if (count <= 16*1024) {
        index = 3;
    } else if (count <= 32*1024) {
        index = 4;
    } else if (count <= 64*1024) {
        index = 5;
    } else {
        index = 6;
    }
    mutex_lock(&mxfs_stats_mutex);
    if (resetStats) {
        resetStats = 0;
        memset(ReadIO, 0, sizeof(ReadIO));
        memset(ReadLatency, 0, sizeof(ReadLatency));
        memset(ReadBandwidth, 0, sizeof(ReadBandwidth));
        memset(WriteIO, 0, sizeof(WriteIO));
        memset(WriteLatency, 0, sizeof(WriteLatency));
        memset(WriteBandwidth, 0, sizeof(WriteBandwidth));
    }
    if (read_flag) {
        ReadIO[index]++;
        ReadLatency[index] = latency;
        ReadLatency[7] = max(latency, ReadLatency[7]);
        ReadBandwidth[index] = bw;
    } else {
        WriteIO[index]++;
        WriteLatency[index] = latency;
        WriteLatency[7] = max(latency, WriteLatency[7]);
        WriteBandwidth[index] = bw;
    }
    mutex_unlock(&mxfs_stats_mutex);
}

struct mxfs_file *
mxfs_file_alloc(struct mxfs_conn *mconn)
{
   struct mxfs_file *ff;

   ff = kmalloc(sizeof(struct mxfs_file), GFP_KERNEL);
   if (unlikely(!ff))
      return NULL;

   ff->mconn = mconn;
   atomic_set(&ff->count, 0);
   return ff;
}

void
mxfs_file_free(struct mxfs_file *ff)
{
   kfree(ff);
}

struct mxfs_file *
mxfs_file_get(struct mxfs_file *ff)
{
   atomic_inc(&ff->count);
   return ff;
}

static void
mxfs_file_put(struct mxfs_file *ff)
{
   if (atomic_dec_and_test(&ff->count)) {
      mxfs_file_free(ff);
   }
}


int
mxfs_finish_open(struct inode *inode, struct file *file)
{

   int result = 0;

   if (file->f_flags & O_TRUNC) {
      struct mxfs_setattr attribs;

      memset(&attribs, 0, sizeof(attribs));
      // Set size to zero
      attribs.valid |= MXFS_FATTR_SIZE;
      attribs.size = 0;

      // Set atime
      attribs.valid |= (MXFS_FATTR_ATIME|MXFS_FATTR_ATIME_NOW);

      // Set mtime
      attribs.valid |= (MXFS_FATTR_MTIME|MXFS_FATTR_MTIME_NOW);

      // XXX: POSIX says the CTIME should be set as well, but mfs
      // XXX: currently does not support this...

      result = mxfs_perform_setattr(inode, &attribs);
   }
   return result;
}


int
mxfs_open_common(struct inode *inode, struct file *file)
{
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   struct mxfs_file *ff;
   int err;

   err = generic_file_open(inode, file);
   if (err)
      return err;

   ff = mxfs_file_alloc(mconn);
   if (!ff)
      return -ENOMEM;

   ff->nodeid = mxfs_get_nodeid(inode);
   file->private_data = mxfs_file_get(ff);
   return mxfs_finish_open(inode, file);
}

void
mxfs_release_common(struct file *file)
{
   struct mxfs_file *ff;

   ff = file->private_data;
   if (unlikely(!ff))
      return;

   mxfs_file_put(ff);
}

static int
mxfs_open(struct inode *inode, struct file *file)
{
   return mxfs_open_common(inode, file);
}

static int
mxfs_release(struct inode *inode, struct file *file)
{
   mxfs_release_common(file);
   return 0;
}

static int
mxfs_flush(struct file *file, fl_owner_t id)
{
   return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

static int
mxfs_fsync(struct file *file, struct dentry *de, int datasync)
{
   return 0;
}

#else

static int
mxfs_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
   return 0;
}

#endif

/**
 * Check whether an iovec refers to buffers in kernel space.
 * We just check the first buffer, assuming that hybrid iovecs that
 * hold both kernel and user space buffers do not exist.
 */
static bool
mxfs_is_kernel_iov(const struct iovec *iov,
                   unsigned long nr_segs)
{
   return (iov == NULL || nr_segs == 0 ||
           iov[0].iov_base >= (void*)PAGE_OFFSET);
}


/**
 * Free an iovec and all the buffers pointed to it. -- Assumes
 * that we are handling a kernel iov.
 */

void
mxfs_free_iov(const struct iovec *iov, unsigned long nr_segs)
{
   unsigned long i;

   if (iov == NULL) {
      return;
   }

   for (i = 0; i < nr_segs; i++) {
      kfree(iov[i].iov_base);
   }

   kfree(iov);
}


/**
 * Make a clone of data represented by an iov_iter. Typically used to
 * handle user-space request buffers, which must be copied into a
 * temp. kernel space.
 *
 * Note that the iovec returned will always have exactly ONE entry.
 */

static struct iovec *
mxfs_clone_iov_iter(struct iov_iter *in_iter, bool copyIn,
                    int *error)
{
   struct iovec *res = NULL;
   char *buf;

   res = kzalloc(sizeof(*res), GFP_KERNEL);
   if (res == NULL) {
      *error = -ENOMEM;
      goto failed;
   }

   res->iov_base = kmalloc(in_iter->count, GFP_KERNEL);
   if (res->iov_base == NULL) {
      *error = -ENOMEM;
      goto failed;
   }
   res->iov_len = in_iter->count;

   if (copyIn) {
      buf = res->iov_base;
      while (in_iter->count > 0) {
         unsigned long len = iov_iter_single_seg_count(in_iter);

         if (copy_from_user(buf, in_iter->iov->iov_base + in_iter->iov_offset,
                            len)) {
            *error = -EFAULT;
            goto failed;
         }

         buf += len;
         iov_iter_advance(in_iter, len);
      }
   }

   return res;

 failed:
   mxfs_free_iov(res, 1);
   return NULL;
}


/**
 * Copy data that was obtained by doing a read from an in-kernel buffer
 * to the destination user-space buffer, both identified by iov_iters.
 */

static int
mxfs_copyout_read_data(struct iov_iter *dst,
                       struct iov_iter *src)
{
   if (src->count > dst->count) {
      LOG_ERROR("Read result expecting at most %lu bytes, but got "
                "%lu", dst->count, src->count);
      return -EINVAL;
   }

   while (src->count > 0) {
      unsigned long src_len;
      unsigned long dst_len;
      unsigned long len;

      src_len = iov_iter_single_seg_count(src);
      dst_len = iov_iter_single_seg_count(dst);
      len = src_len < dst_len ? src_len : dst_len;

      if (copy_to_user(dst->iov->iov_base + dst->iov_offset,
                       src->iov->iov_base + src->iov_offset, len)) {
         return -EFAULT;
      }

      iov_iter_advance(src, len);
      iov_iter_advance(dst, len);
   }

   return 0;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)

static ssize_t
mxfs_file_aio_read(struct kiocb *iocb, const struct iovec *iov,
                   unsigned long nr_segs, loff_t pos)
{
   struct  mxfs_file *ff = iocb->ki_filp->private_data;
   struct inode *inode = iocb->ki_filp->f_mapping->host;
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   int err = 0;
   struct mxfs_req *req;
   struct mxfs_read inarg;
   int copyoutErr;
   struct iov_iter iter_src;
   struct iov_iter iter_dst;
   const struct iovec *my_iov = NULL;
   unsigned long remain;
   unsigned long processed = 0;
   unsigned long this_request;
   bool is_kernel_iov;
   bool gathering_stats = false;
   unsigned long start, end;

   if ((probeInode == -1) || (probeInode == ff->nodeid)) {
       start = get_jiffies_64()*(1000000/HZ);
       gathering_stats = true;
   }

   remain = iov_length(iov, nr_segs);

   if (pos + remain > i_size_read(inode)) {
      int err;
      /*
       * If trying to read past EOF, make sure the i_size
       * attribute is up-to-date.
       */
      err = mxfs_update_attributes(inode, NULL, iocb->ki_filp, NULL);
      if (err) {
         return err;
      }
   }

   if (is_bad_inode(inode)) {
      return -EIO;
   }

   is_kernel_iov = mxfs_is_kernel_iov(iov, nr_segs);
   if (is_kernel_iov) {
      iov_iter_init(&iter_src, iov, nr_segs, remain, 0);
   } else {
      iov_iter_init(&iter_dst, iov, nr_segs, remain, 0);
   }

   while (remain > 0 && err == 0) {
      if (remain > maxRequestSize) {
         // truncate the iovec
         this_request = maxRequestSize;
      } else {
         this_request = remain;
      }

      if (is_kernel_iov) {
         iter_src.count = this_request;
      } else {
         iter_dst.count = this_request;
         my_iov = mxfs_clone_iov_iter(&iter_dst, false, &err);
         iter_dst.count = remain;
         if (my_iov == NULL) {
            goto exit_func;
         }

         iov_iter_init(&iter_src, my_iov, 1, this_request, 0);
      }

      req = mxfs_get_req(mconn);
      if (IS_ERR(req)) {
         err = PTR_ERR(req);
         goto exit_func;
      }

      memset(&inarg, 0, sizeof(inarg));
      inarg.offset = pos;
      inarg.size = this_request;
      req->in.h.opcode = MXFS_READ;
      req->in.h.nodeid = ff->nodeid;
      req->in.numargs = 2;
      req->in.args[0].size = sizeof(inarg);
      req->in.args[0].value = &inarg;
      req->in.args[1].size = this_request;
      req->in.args[1].value = NULL;
      req->out.numargs = 1;
      req->out.args[0].size = this_request;
      req->out.args[0].value = NULL;
      req->ioviter = &iter_src;

      if ((testInode != -1) && (testInode != ff->nodeid)) {
	mxfs_request_send(mconn, req);
	err = req->out.h.error;
      } else {
	err = this_request;
      }
      if (err >= 0) {

         if (err < this_request) {
            // short read...
            remain = 0;
            this_request = err;
         } else {
            remain -= this_request;
         }
         processed += this_request;
         pos += this_request;
         iocb->ki_pos = pos;
         err = 0;

         if (is_kernel_iov) {
            iov_iter_advance(&iter_src, this_request);
         } else {
            // needed for short reads...
            //iov_iter_reexpand(&iter_src, processed + this_request);
            iter_src.count = this_request;

            copyoutErr = mxfs_copyout_read_data(&iter_dst, &iter_src);
            if (copyoutErr != 0) {
               err = copyoutErr;
            }

            mxfs_free_iov(my_iov, 1);
            my_iov = NULL;
         }
      }

      mxfs_put_request(mconn, req);
   }

   if (gathering_stats && (err==0)) {
       end = get_jiffies_64()*(1000000/HZ);
       mxfs_update_stats(true, processed, end-start);
   }
 exit_func:
   mxfs_free_iov(my_iov, 1);
   if (err && gathering_stats) {
       mutex_lock(&mxfs_stats_mutex);
       ReadIO[7]++;
       mutex_unlock(&mxfs_stats_mutex);
   }
   return err == 0 ? processed : err;
}

#else

static ssize_t
mxfs_file_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
   struct  mxfs_file *ff = iocb->ki_filp->private_data;
   struct inode *inode = iocb->ki_filp->f_mapping->host;
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   int err = 0;
   struct mxfs_req *req;
   struct mxfs_read inarg;
   int copyoutErr;
   struct iov_iter iter_src;
   struct iov_iter iter_dst;
   const struct iovec *my_iov = NULL;
   unsigned long processed = 0;
   unsigned long this_request;
   bool is_kernel_iov;
   unsigned long remain;
   loff_t pos = iocb->ki_nbytes;
   bool gathering_stats = false;
   unsigned long start, end;

   if ((probeInode == -1) || (probeInode == ff->nodeid)) {
       start = get_jiffies_64()*(1000000/HZ);
       gathering_stats = true;
   }

   remain = iov_length(to->iov, to->nr_segs);
   if (pos + remain > i_size_read(inode)) {
      int err;
      /*
       * If trying to read past EOF, make sure the i_size
       * attribute is up-to-date.
       */
      err = mxfs_update_attributes(inode, NULL, iocb->ki_filp, NULL);
      if (err) {
         return err;
      }
   }

   if (is_bad_inode(inode)) {
      return -EIO;
   }

   is_kernel_iov = mxfs_is_kernel_iov(to->iov, to->nr_segs);
   if (is_kernel_iov) {
      iov_iter_init(&iter_src, READ, to->iov, to->nr_segs, remain);
   } else {
      iov_iter_init(&iter_dst, READ, to->iov, to->nr_segs, remain);
   }

   while (remain > 0 && err == 0) {

      if (remain > maxRequestSize) {
         // truncate the iovec
         this_request = maxRequestSize;
      } else {
         this_request = remain;
      }
      if (is_kernel_iov) {
         iter_src.count = this_request;
      } else {
         iter_dst.count = this_request;
         my_iov = mxfs_clone_iov_iter(&iter_dst, false, &err);
         iter_dst.count = remain;
         if (my_iov == NULL) {
            goto exit_func;
         }

         iov_iter_init(&iter_src, READ, my_iov, 1, this_request);
      }

      req = mxfs_get_req(mconn);
      if (IS_ERR(req)) {
         err = PTR_ERR(req);
         goto exit_func;
      }

      memset(&inarg, 0, sizeof(inarg));

      inarg.offset = pos;
      inarg.size = this_request;
      req->in.h.opcode = MXFS_READ;
      req->in.h.nodeid = ff->nodeid;
      req->in.numargs = 2;
      req->in.args[0].size = sizeof(inarg);
      req->in.args[0].value = &inarg;
      req->in.args[1].size = this_request;
      req->in.args[1].value = NULL;
      req->out.numargs = 1;
      req->out.args[0].size = this_request;
      req->out.args[0].value = NULL;
      req->ioviter = &iter_src;

      if ((testInode != -1) && (testInode != ff->nodeid)) {
         mxfs_request_send(mconn, req);
         err = req->out.h.error;
      } else {
         err = this_request;
      }

      if (err >= 0) {

         if (err < this_request) {
            // short read...
            remain = 0;
            this_request = err;
         } else {
            remain -= this_request;
         }
         processed += this_request;
         pos += this_request;
         iocb->ki_pos = pos;
         err = 0;

         if (is_kernel_iov) {
            iov_iter_advance(&iter_src, this_request);
         } else {
            // needed for short reads...
            //iov_iter_reexpand(&iter_src, processed + this_request);
            iter_src.count = this_request;

            copyoutErr = mxfs_copyout_read_data(&iter_dst, &iter_src);
            if (copyoutErr != 0) {
               err = copyoutErr;
            }

            mxfs_free_iov(my_iov, 1);
            my_iov = NULL;
         }
      }
      mxfs_put_request(mconn, req);
   }

   if (gathering_stats && (err == 0)) {
       end = get_jiffies_64()*(1000000/HZ);
       mxfs_update_stats(true, processed, end-start);
   }
 exit_func:
   mxfs_free_iov(my_iov, 1);
   if (err && gathering_stats) {
       mutex_lock(&mxfs_stats_mutex);
       ReadIO[7]++;
       mutex_unlock(&mxfs_stats_mutex);
   }
   return err == 0 ? processed : err;
}
#endif

static void
mxfs_write_update_size(struct inode *inode, loff_t pos)
{
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   struct mxfs_inode *fi = mxfs_get_inode(inode);

   spin_lock(&mconn->lock);
   fi->attr_version = ++mconn->attr_version;
   if (pos > inode->i_size)
      i_size_write(inode, pos);
   spin_unlock(&mconn->lock);
}


static ssize_t
mxfs_perform_write(struct file *file, const struct iovec *iov,
           unsigned long nr_segs, size_t count, loff_t pos)
{
   struct  mxfs_file *ff = file->private_data;
   struct inode *inode = file->f_mapping->host;
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   int err = 0;
   struct mxfs_req *req;
   struct mxfs_write inarg;
   struct iov_iter in_iter;
   struct iov_iter out_iter;
   const struct iovec *my_iov = NULL;
   bool is_kernel_iov;

   unsigned long processed = 0;
   unsigned long remain = count;
   unsigned long this_request;

   bool gathering_stats = false;
   unsigned long start, end;

   if ((probeInode == -1) || (probeInode == ff->nodeid)) {
       start = get_jiffies_64()*(1000000/HZ);
       gathering_stats = true;
   }

   if (is_bad_inode(inode)) {
      return -EIO;
   }

   is_kernel_iov = mxfs_is_kernel_iov(iov, nr_segs);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)

   if (is_kernel_iov) {
      iov_iter_init(&out_iter, iov, nr_segs, count, 0);
   } else {
      iov_iter_init(&in_iter, iov, nr_segs, count, 0);
   }
#else

   if (is_kernel_iov) {
      iov_iter_init(&out_iter, WRITE, iov, nr_segs, count);
   } else {
      iov_iter_init(&in_iter, WRITE, iov, nr_segs, count);
   }
#endif

   while (remain > 0 && err == 0) {
      if (remain > maxRequestSize) {
         // truncate the iovec
         this_request = maxRequestSize;
      } else {
         this_request = remain;
      }

      if (is_kernel_iov) {
         out_iter.count = this_request;
      } else {
         in_iter.count = this_request;
         my_iov = mxfs_clone_iov_iter(&in_iter, true, &err);
         if (my_iov == NULL) {
            goto exit_func;
         }
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
         iov_iter_init(&out_iter, my_iov, 1, this_request, 0);
#else
         iov_iter_init(&out_iter, WRITE, my_iov, 1, this_request);
#endif
      }

      req = mxfs_get_req(mconn);
      if (IS_ERR(req)) {
         err = PTR_ERR(req);
         goto exit_func;
      }

      memset(&inarg, 0, sizeof(inarg));
      inarg.offset = pos;
      inarg.size = this_request;
      req->in.h.opcode = MXFS_WRITE;
      req->in.h.nodeid = ff->nodeid;
      req->in.numargs = 2;
      req->in.args[0].size = sizeof(inarg);
      req->in.args[0].value = &inarg;
      req->in.args[1].size = this_request;
      req->in.args[1].value = NULL;
      req->ioviter = &out_iter;

      req->out.numargs = 0;
      if ((testInode != -1) && (testInode != ff->nodeid)) {
         mxfs_request_send(mconn, req);
         err = req->out.h.error;
      } else {
         err = 0;
      }

      mxfs_put_request(mconn, req);

      if (err == 0) {
         processed += this_request;
         remain -= this_request;
         pos += this_request;
      }

      if (!is_kernel_iov) {
         mxfs_free_iov(my_iov, 1);
         my_iov = NULL;
      }
   }

   if (gathering_stats && (err == 0)) {
       end = get_jiffies_64()*(1000000/HZ);
       mxfs_update_stats(false, count, end-start);
   }
 exit_func:
   mxfs_free_iov(my_iov, 1);

   if (err) {
      if (gathering_stats) {
          mutex_lock(&mxfs_stats_mutex);
          WriteIO[7]++;
          mutex_unlock(&mxfs_stats_mutex);
      }
      return err;
   }

   return count;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)

static ssize_t
mxfs_file_aio_write(struct kiocb *iocb, const struct iovec *iov,
                    unsigned long nr_segs, loff_t pos)
{
   struct file *file = iocb->ki_filp;
   struct address_space *mapping = file->f_mapping;
   struct inode *inode = mapping->host;
   size_t count = 0;
   ssize_t written = 0;
   ssize_t err;

   WARN_ON(iocb->ki_pos != pos);

   err = generic_segment_checks(iov, &nr_segs, &count, VERIFY_READ);
   if (err)
      return err;

   mutex_lock(&inode->i_mutex);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   vfs_check_frozen(inode->i_sb, SB_FREEZE_WRITE);
#endif
   err = generic_write_checks(file, &pos, &count, S_ISBLK(inode->i_mode));
   if (err) {
      mutex_unlock(&inode->i_mutex);
      goto out;
   }

   if (count == 0) {
      mutex_unlock(&inode->i_mutex);
      goto out;
   }

   err = file_remove_suid(file);
   if (err) {
      mutex_unlock(&inode->i_mutex);
      goto out;
   }

   file_update_time(file);
   mutex_unlock(&inode->i_mutex);
   written = mxfs_perform_write(file, iov, nr_segs, count, pos);
   if (written >= 0) {
      mutex_lock(&inode->i_mutex);
      mxfs_write_update_size(inode, pos + written);
      mutex_unlock(&inode->i_mutex);
      iocb->ki_pos = pos + written;
   }

out:
   return written ? written : err;
}

#else

static ssize_t
mxfs_file_write_iter(struct kiocb *iocb, struct iov_iter *from)
{
   struct file *file = iocb->ki_filp;
   struct address_space *mapping = file->f_mapping;
   struct inode *inode = mapping->host;
   size_t count = from->count;
   ssize_t written = 0;
   ssize_t err;
   loff_t pos = iocb->ki_pos;

   /*
   err = generic_write_checks(file, &pos, &count, S_ISBLK(inode->i_mode));
   if (err) {
      mutex_unlock(&inode->i_mutex);
      goto out;
   }
   LOG_ERROR("---------------- mxfs_file_write_iter : generic write check\n");

   if (count == 0) {
      mutex_unlock(&inode->i_mutex);
      goto out;
   }
   */

   err = file_remove_suid(file);
   if (err) {
      mutex_unlock(&inode->i_mutex);
      goto out;
   }

   file_update_time(file);
   mutex_unlock(&inode->i_mutex);
   //LOG_ERROR("YYY Write pos  : %lu\n", iov_offset);
   written = mxfs_perform_write(file, from->iov, from->nr_segs, count, pos);
   //written = mxfs_perform_write(file, from, count, pos);
   if (written >= 0) {
      mutex_lock(&inode->i_mutex);
      mxfs_write_update_size(inode, pos + written);
      mutex_unlock(&inode->i_mutex);
      iov_iter_advance(from, written);
      //iocb->ki_pos = pos + written;
   }

out:
   return written ? written : err;
}
#endif

static loff_t
mxfs_file_llseek(struct file *file, loff_t offset, int origin)
{
   loff_t retval;
   struct inode *inode = file->f_path.dentry->d_inode;

   mutex_lock(&inode->i_mutex);
   switch (origin) {
      case SEEK_END:
         retval = mxfs_update_attributes(inode, NULL, file, NULL);
         if (retval)
            goto exit;

         offset += i_size_read(inode);
         break;

      case SEEK_CUR:
         offset += file->f_pos;
         break;
   }

   retval = -EINVAL;
   if (offset >= 0 && offset <= inode->i_sb->s_maxbytes) {
      if (offset != file->f_pos) {
         file->f_pos = offset;
         file->f_version = 0;
      }
      retval = offset;
   }

exit:
   mutex_unlock(&inode->i_mutex);
   return retval;
}


static ssize_t
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
mxfs_direct_io(int rw, struct kiocb *iocb, const struct iovec *iov, loff_t offset,
               unsigned long nr_segs)
#else
mxfs_direct_io(int rw, struct kiocb *iocb, struct iov_iter *iter, loff_t offset)
#endif
{
   int result;

   if (rw & WRITE) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
      result = mxfs_file_aio_write(iocb, iov, nr_segs, offset);
#else
      result = mxfs_file_write_iter(iocb, iter);
#endif
   } else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
      result = mxfs_file_aio_read(iocb, iov, nr_segs, offset);
#else
      result = mxfs_file_read_iter(iocb, iter);
#endif
   }

   return result;
}

static const struct file_operations mxfs_file_operations = {
   .llseek              = mxfs_file_llseek,

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
   .read                = do_sync_read,
   .aio_read            = mxfs_file_aio_read,
#else
   .read                = new_sync_read,
   .read_iter           = mxfs_file_read_iter,
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
   .write               = do_sync_write,
   .aio_write           = mxfs_file_aio_write,
#else
   .write               = new_sync_write,
   .write_iter          = mxfs_file_write_iter,
#endif

   .open                = mxfs_open,
   .flush               = mxfs_flush,
   .release             = mxfs_release,
   .fsync               = mxfs_fsync,
};

static const struct address_space_operations mxfs_addr_operations = {
   .direct_IO = mxfs_direct_io,
};


void
mxfs_init_file_inode(struct inode *inode)
{
   inode->i_fop = &mxfs_file_operations;
   inode->i_mapping->a_ops = &mxfs_addr_operations;
}

