/*
 * Copyright (C) 2012- Maxta, Inc
 */

#include "mxfs_i.h"

#include <linux/pagemap.h>
#include <linux/file.h>
#include <linux/gfp.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
#define hlist_empty list_empty
#endif

static inline void
mxfs_dentry_settime(struct dentry *entry, u64 time)
{
   entry->d_time = time;
}

static inline u64
mxfs_dentry_time(struct dentry *entry)
{
   return entry->d_time;
}

/*
 * Set dentry and possibly attribute timeouts from the lookup/mk*
 * replies
 */
static void
mxfs_change_entry_timeout(struct dentry *entry, struct mxfs_entry *o)
{
   //If  dentry cache timeout is greater than 0 then set timeout appropriately
   if (dentryTimeout == 0) {
      mxfs_dentry_settime(entry, 0);
   } else {
      mxfs_dentry_settime(entry, get_jiffies_64() + (dentryTimeout*HZ));
   }
}

/*
 * Mark the attributes as stale, so that at the next call to
 * ->getattr() they will be fetched from userspace
 */
void
mxfs_invalidate_attr(struct inode *inode)
{
   LOG_DEBUG4("%s inode=%lx",__func__,inode->i_ino);
   mxfs_get_inode(inode)->i_time = 0;
}

/*
 * Just mark the entry as stale, so that a next attempt to look it up
 * will result in a new lookup call to userspace
 *
 * This is called when a dentry is about to become negative and the
 * timeout is unknown (unlink, rmdir, rename and in some cases
 * lookup)
 */
void
mxfs_invalidate_entry_cache(struct dentry *entry)
{
   mxfs_dentry_settime(entry, 0);
}

/*
 * Same as mxfs_invalidate_entry_cache(), but also try to remove the
 * dentry from the hash
 */
static void
mxfs_invalidate_entry(struct dentry *entry)
{
   d_invalidate(entry);
   mxfs_invalidate_entry_cache(entry);
}

static void
mxfs_lookup_init(struct mxfs_conn *mconn, struct mxfs_req *req,
            u64 nodeid, struct qstr *name, struct mxfs_entry *outarg)
{
   memset(outarg, 0, sizeof(struct mxfs_entry));
   req->in.h.opcode = MXFS_LOOKUP;
   req->in.h.nodeid = nodeid;
   req->in.numargs = 2;
   req->in.args[0].size = sizeof(struct mxfs_entry);
   req->in.args[0].value = outarg;
   req->in.args[1].size = name->len + 1;
   req->in.args[1].value = name->name;
   req->out.numargs = 1;
   req->out.args[0].size = sizeof(struct mxfs_entry);
   req->out.args[0].value = outarg;
}

u64
mxfs_get_attr_version(struct mxfs_conn *mconn)
{
   u64 curr_version;

   /*
    * The spin lock isn't actually needed on 64bit archs, but we
    * don't yet care too much about such optimizations.
    */
   spin_lock(&mconn->lock);
   curr_version = mconn->attr_version;
   spin_unlock(&mconn->lock);
   return curr_version;
}

/*
 * Check whether the dentry is still valid
 *
 * If the entry validity timeout has expired and the dentry is
 * positive, try to redo the lookup.  If the lookup results in a
 * different inode, then let the VFS invalidate the dentry and redo
 * the lookup once more.  If the lookup results in the same inode,
 * then refresh the attributes, timeouts and mark the dentry valid.
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
static int
mxfs_dentry_revalidate(struct dentry *entry, struct nameidata *nd)
#else
static int
mxfs_dentry_revalidate(struct dentry *entry, unsigned int flags)
#endif
{
   struct inode *inode = ACCESS_ONCE(entry->d_inode);
   int err;
   struct mxfs_entry outarg;
   struct mxfs_conn *mconn;
   struct mxfs_req *req;
   struct dentry *parent;
   u64 attr_version;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
   if ((flags & LOOKUP_RCU) != 0) {
     return -ECHILD;
   }
#endif

   if (inode && is_bad_inode(inode)) {
      return 0;
   }

   /* For negative dentries, always do a fresh lookup */
   if (!inode)
      return 0;

   if (entry->d_time < get_jiffies_64()) {
      //Dentry is expired so validate it by fetching it from mfs
      mconn = get_mxfs_conn(inode);
      req = mxfs_get_req(mconn);
      if (IS_ERR(req))
         return 0;

      attr_version = mxfs_get_attr_version(mconn);
      parent = dget_parent(entry);
      mxfs_lookup_init(mconn, req, mxfs_get_nodeid(parent->d_inode),
            &entry->d_name, &outarg);
      mxfs_request_send(mconn, req);
      dput(parent);
      err = req->out.h.error;
      mxfs_put_request(mconn, req);
      /* Zero nodeid is same as -ENOENT */
      if (!err && !outarg.nodeid)
         err = -ENOENT;
      LOG_DEBUG4("%s: revalidated entry with mfs %s err=%d \n",__func__, entry->d_name.name, err);
      if (!err) {
         if (outarg.nodeid != mxfs_get_nodeid(inode)) {
            if (!have_submounts(entry)) {
               shrink_dcache_parent(entry);
               d_drop(entry);
            }
            return 0;
         }
      }
      if (err || (outarg.attr.mode ^ inode->i_mode) & S_IFMT){
         return 0;
      }

      mxfs_change_attributes(inode, &outarg.attr, attr_version);
      mxfs_change_entry_timeout(entry, &outarg);
   } else {
     //Entry is not expired so assume it's valid
     LOG_DEBUG4("%s: using cached dentry %s\n",__func__, entry->d_name.name);
   }

   return 1;
}

static int
invalid_nodeid(u64 nodeid)
{
   return !nodeid || nodeid == MXFS_LEGACY_ROOT_ID;
}

const struct dentry_operations mxfs_dentry_operations = {
   .d_revalidate        = mxfs_dentry_revalidate,
};

int
mxfs_valid_type(int m)
{
   return S_ISREG(m) || S_ISDIR(m);
}

/*
 * Add a directory inode to a dentry, ensuring that no other dentry
 * refers to this inode.  Called with mconn->inst_mutex.
 */
static struct dentry *
mxfs_d_add_directory(struct dentry *entry, struct inode *inode)
{
   struct dentry *alias = d_find_alias(inode);
   if (alias && !(alias->d_flags & DCACHE_DISCONNECTED)) {
      /* This tries to shrink the subtree below alias */
      mxfs_invalidate_entry(alias);
      dput(alias);
      if (!hlist_empty(&inode->i_dentry))
         return ERR_PTR(-EBUSY);
   } else {
      dput(alias);
   }
   return d_splice_alias(inode, entry);
}

int
mxfs_lookup_name(struct super_block *sb, u64 nodeid, struct qstr *name,
                        struct mxfs_entry *outarg, struct inode **inode)
{
   struct mxfs_conn *mconn = get_mxfs_conn_super(sb);
   struct mxfs_req *req;
   u64 attr_version;
   int err;

   *inode = NULL;
   err = -ENAMETOOLONG;
   if (name->len > MXFS_NAME_MAX)
      goto out;

   req = mxfs_get_req(mconn);
   err = PTR_ERR(req);
   if (IS_ERR(req))
      goto out;

   attr_version = mxfs_get_attr_version(mconn);
   mxfs_lookup_init(mconn, req, nodeid, name, outarg);
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   /* Zero nodeid is same as -ENOENT, but with valid timeout */
   if (err || !outarg->nodeid)
      goto out;

   err = -EIO;
   if (!outarg->nodeid)
      goto out;
   if (!mxfs_valid_type(outarg->attr.mode))
      goto out;

   *inode = mxfs_iget(sb, outarg->nodeid, &outarg->attr, attr_version);
   err = -ENOMEM;
   if (!*inode) {
      goto out;
   }

   err = 0;

out:
   return err;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
static struct dentry *
mxfs_lookup(struct inode *dir, struct dentry *entry, struct nameidata *nd)
#else
static struct dentry *
mxfs_lookup(struct inode *dir, struct dentry *entry, unsigned int flags)
#endif
{
   int err;
   struct mxfs_entry outarg;
   struct inode *inode;
   struct dentry *newent;
   struct mxfs_conn *mconn = get_mxfs_conn(dir);
   bool outarg_valid = true;

   LOG_DEBUG4("%s: inode=%lx lookup.name=%s\n",__func__,  dir->i_ino, entry->d_name.name);
   err = mxfs_lookup_name(dir->i_sb, mxfs_get_nodeid(dir), &entry->d_name,
                                                            &outarg, &inode);
   if (err == -ENOENT) {
      outarg_valid = false;
      err = 0;
   }

   if (err)
      goto out_err;

   err = -EIO;
   if (inode && mxfs_get_nodeid(inode) == gRootInum)
      goto out_iput;

   if (inode && S_ISDIR(inode->i_mode)) {
      mutex_lock(&mconn->mutex);
      newent = mxfs_d_add_directory(entry, inode);
      mutex_unlock(&mconn->mutex);
      err = PTR_ERR(newent);
      if (IS_ERR(newent))
         goto out_iput;
   } else {
      newent = d_splice_alias(inode, entry);
   }

   entry = newent ? newent : entry;
   entry->d_op = &mxfs_dentry_operations;
   if (outarg_valid) {
      mxfs_change_entry_timeout(entry, &outarg);
   } else {
      mxfs_invalidate_entry_cache(entry);
   }

   return newent;

out_iput:
   iput(inode);
out_err:
   return ERR_PTR(err);
}

static int
create_new_entry(struct mxfs_conn *mconn, struct mxfs_req *req,
		 struct inode *dir, struct dentry *entry, int mode);

/*
 * Atomic create+open operation for 2.x kernels. For 3.x kernels, this
 * is just a create...
 *
 * If the filesystem doesn't support this, then fall back to separate
 * 'mknod' + 'open' requests.
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
static int
mxfs_create_open(struct inode *dir, struct dentry *entry,
                 int mode, struct nameidata *nd)
#else
static int
mxfs_create_open(struct inode *dir, struct dentry *entry,
                 umode_t mode)
#endif
{
   int err;
   struct mxfs_conn *mconn = get_mxfs_conn(dir);
   struct mxfs_req *req;
   struct mxfs_create inarg;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   struct mxfs_entry outentry;
   struct inode *inode;
   struct mxfs_file *ff;
#endif

   req = mxfs_get_req(mconn);
   err = PTR_ERR(req);
   if (IS_ERR(req))
      return err;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   err = -ENOMEM;
   ff = mxfs_file_alloc(mconn);
   if (!ff)
      goto out_put_request;
#endif

   memset(&inarg, 0, sizeof(inarg));
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   memset(&outentry, 0, sizeof(outentry));
   inarg.uid = current_fsuid();
   inarg.gid = dir->i_gid;
#else
   inarg.uid = current_fsuid().val;
   inarg.gid = dir->i_gid.val;
#endif
   inarg.mode = mode;
   inarg.umask = current_umask();
   req->in.h.opcode = MXFS_CREATE;
   req->in.h.nodeid = mxfs_get_nodeid(dir);
   req->in.numargs = 2;
   req->in.args[0].size = sizeof(inarg);
   req->in.args[0].value = &inarg;
   req->in.args[1].size = entry->d_name.len + 1;
   req->in.args[1].value = entry->d_name.name;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   req->out.numargs = 1;
   req->out.args[0].size = sizeof(outentry);
   req->out.args[0].value = &outentry;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   if (err) {
      goto out_free_ff;
   }

   err = -EIO;
   if (!S_ISREG(outentry.attr.mode) || invalid_nodeid(outentry.nodeid))
      goto out_free_ff;

   mxfs_put_request(mconn, req);
   ff->nodeid = outentry.nodeid;
   inode = mxfs_iget(dir->i_sb, outentry.nodeid, &outentry.attr, 0);
   if (!inode) {
      mxfs_file_free(ff);
      return -ENOMEM;
   }
   d_instantiate(entry, inode);
   mxfs_change_entry_timeout(entry, &outentry);

   if (nd) {
      struct file *file;

      file = lookup_instantiate_filp(nd, entry, generic_file_open);
      if (IS_ERR(file)) {
         mxfs_file_free(ff);
         return PTR_ERR(file);
      }
      file->private_data = mxfs_file_get(ff);
      err = mxfs_finish_open(inode, file);
      if (err != 0) {
         goto out_free_ff;
      }
   }

   return 0;

out_free_ff:
   mxfs_file_free(ff);
out_put_request:
   mxfs_put_request(mconn, req);
   mxfs_invalidate_attr(dir);
   return err;
#else
   return create_new_entry(mconn, req, dir, entry, S_IFREG);
#endif
}

/*
 * Code shared between mknod, mkdir, symlink and link
 */
static int
create_new_entry(struct mxfs_conn *mconn, struct mxfs_req *req,
                        struct inode *dir, struct dentry *entry, int mode)
{
   struct mxfs_entry outarg;
   struct inode *inode;
   int err;

   memset(&outarg, 0, sizeof(outarg));
   req->in.h.nodeid = mxfs_get_nodeid(dir);
   req->out.numargs = 1;
   req->out.args[0].size = sizeof(outarg);
   req->out.args[0].value = &outarg;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   mxfs_invalidate_entry(entry);
   if (err)
      return err;

   err = -EIO;
   if (invalid_nodeid(outarg.nodeid))
      return err;

   if ((outarg.attr.mode ^ mode) & S_IFMT)
      return err;

   inode = mxfs_iget(dir->i_sb, outarg.nodeid, &outarg.attr, 0);
   if (!inode) {
      return -ENOMEM;
   }

#if 1
   if (S_ISDIR(inode->i_mode)) {
      struct dentry *alias;
      mutex_lock(&mconn->mutex);
      alias = d_find_alias(inode);
      if (alias) {
         /* New directory must have moved since mkdir */
         mutex_unlock(&mconn->mutex);
         dput(alias);
         iput(inode);
         return -EBUSY;
      }
      d_instantiate(entry, inode);
      mutex_unlock(&mconn->mutex);
   } else
      d_instantiate(entry, inode);
#else
   err = d_instantiate_no_diralias(entry, inode);
   if (err)
      return err;
#endif

   mxfs_change_entry_timeout(entry, &outarg);
   mxfs_invalidate_attr(dir);
   return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

static int
mxfs_create(struct inode *dir, struct dentry *entry,
            int mode, struct nameidata *nd)
{
   int err;

   err = mxfs_create_open(dir, entry, mode, nd);
   return err;
}

#else

static int
mxfs_create(struct inode *dir, struct dentry *entry,
            umode_t mode, bool excl)
{
   return mxfs_create_open(dir, entry, mode);
}

#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
static int
mxfs_mkdir(struct inode *dir, struct dentry *entry, int mode)
#else
static int
mxfs_mkdir(struct inode *dir, struct dentry *entry, umode_t mode)
#endif
{
   struct mxfs_mkdir inarg;
   struct mxfs_conn *mconn = get_mxfs_conn(dir);
   struct mxfs_req *req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   memset(&inarg, 0, sizeof(inarg));
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   inarg.uid = current_fsuid();
   inarg.gid = dir->i_gid;
#else
   inarg.uid = current_fsuid().val;
   inarg.gid = dir->i_gid.val;
#endif
   inarg.mode = mode;
   inarg.umask = current_umask();
   req->in.h.opcode = MXFS_MKDIR;
   req->in.h.nodeid = mxfs_get_nodeid(dir);
   req->in.numargs = 2;
   req->in.args[0].size = sizeof(inarg);
   req->in.args[0].value = &inarg;
   req->in.args[1].size = entry->d_name.len + 1;
   req->in.args[1].value = entry->d_name.name;
   return create_new_entry(mconn, req, dir, entry, S_IFDIR);
}

static int
mxfs_unlink(struct inode *dir, struct dentry *entry)
{
   int err;
   struct mxfs_conn *mconn = get_mxfs_conn(dir);
   struct mxfs_req *req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   req->in.h.opcode = MXFS_UNLINK;
   req->in.h.nodeid = mxfs_get_nodeid(dir);
   req->in.numargs = 1;
   req->in.args[0].size = entry->d_name.len + 1;
   req->in.args[0].value = entry->d_name.name;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   if (!err) {
      struct inode *inode = entry->d_inode;

      /*
       * Set nlink to zero so the inode can be cleared, if the inode
       * does have more links this will be discovered at the next
       * lookup/getattr.
       */
      clear_nlink(inode);
      mxfs_invalidate_attr(inode);
      mxfs_invalidate_attr(dir);
      mxfs_invalidate_entry_cache(entry);
   } else if (err == -EINTR)
      mxfs_invalidate_entry(entry);

   return err;
}

static int
mxfs_rmdir(struct inode *dir, struct dentry *entry)
{
   int err;
   struct mxfs_conn *mconn = get_mxfs_conn(dir);
   struct mxfs_req *req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   req->in.h.opcode = MXFS_RMDIR;
   req->in.h.nodeid = mxfs_get_nodeid(dir);
   req->in.numargs = 1;
   req->in.args[0].size = entry->d_name.len + 1;
   req->in.args[0].value = entry->d_name.name;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   if (!err) {
      clear_nlink(entry->d_inode);
      mxfs_invalidate_attr(dir);
      mxfs_invalidate_entry_cache(entry);
   } else if (err == -EINTR)
      mxfs_invalidate_entry(entry);
   return err;
}

static int
mxfs_rename(struct inode *olddir, struct dentry *oldent,
                           struct inode *newdir, struct dentry *newent)
{
   int err;
   struct mxfs_rename inarg;
   struct mxfs_conn *mconn = get_mxfs_conn(olddir);
   struct mxfs_req *req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   memset(&inarg, 0, sizeof(inarg));
   inarg.newdir = mxfs_get_nodeid(newdir);
   req->in.h.opcode = MXFS_RENAME;
   req->in.h.nodeid = mxfs_get_nodeid(olddir);
   req->in.numargs = 3;
   req->in.args[0].size = sizeof(inarg);
   req->in.args[0].value = &inarg;
   req->in.args[1].size = oldent->d_name.len + 1;
   req->in.args[1].value = oldent->d_name.name;
   req->in.args[2].size = newent->d_name.len + 1;
   req->in.args[2].value = newent->d_name.name;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   if (!err) {
      /* ctime changes */
      mxfs_invalidate_attr(oldent->d_inode);

      mxfs_invalidate_attr(olddir);
      if (olddir != newdir)
         mxfs_invalidate_attr(newdir);

      /* newent will end up negative */
      if (newent->d_inode) {
         mxfs_invalidate_attr(newent->d_inode);
         mxfs_invalidate_entry_cache(newent);
      }
   } else if (err == -EINTR) {
      /* If request was interrupted, DEITY only knows if the
         rename actually took place.  If the invalidation
         fails (e.g. some process has CWD under the renamed
         directory), then there can be inconsistency between
         the dcache and the real filesystem.  Tough luck. */
      mxfs_invalidate_entry(oldent);
      if (newent->d_inode)
         mxfs_invalidate_entry(newent);
   }

   return err;
}

static void
mxfs_fillattr(struct inode *inode, struct mxfs_attr *attr, struct kstat *stat)
{
   stat->dev = inode->i_sb->s_dev;
   stat->ino = attr->ino;
   stat->mode = (inode->i_mode & S_IFMT) | (attr->mode & 07777);
   stat->nlink = attr->nlink;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   stat->uid = attr->uid;
   stat->gid = attr->gid;
#else
   stat->uid.val = attr->uid;
   stat->gid.val = attr->gid;
#endif
   stat->rdev = inode->i_rdev;
   stat->atime.tv_sec = attr->atime;
   stat->atime.tv_nsec = attr->atimensec;
   stat->mtime.tv_sec = attr->mtime;
   stat->mtime.tv_nsec = attr->mtimensec;
   stat->ctime.tv_sec = attr->ctime;
   stat->ctime.tv_nsec = attr->ctimensec;
   stat->size = attr->size;
   stat->blocks = attr->blocks;
   stat->blksize = (1 << inode->i_blkbits);
}

static int
do_getattr(struct mxfs_conn *mconn, u64 nodeid,
         struct mxfs_attr_out *outarg)
{
   int err;
   struct mxfs_req *req;

   req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   memset(outarg, 0, sizeof(*outarg));
   req->in.h.opcode = MXFS_GETATTR;
   req->in.h.nodeid = nodeid;
   req->in.numargs = 1;
   req->in.args[0].size = sizeof(*outarg);
   req->in.args[0].value = outarg;
   req->out.numargs = 1;
   req->out.args[0].size = sizeof(*outarg);
   req->out.args[0].value = outarg;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   return err;
}

static int
mxfs_do_getattr(struct inode *inode, struct kstat *stat, struct file *file)
{
   int err;
   struct mxfs_attr_out outarg;
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   u64 attr_version;

   attr_version = mxfs_get_attr_version(mconn);
   err = do_getattr(mconn, mxfs_get_nodeid(inode), &outarg);
   if (!err) {
      if ((inode->i_mode ^ outarg.attr.mode) & S_IFMT) {
         make_bad_inode(inode);
         err = -EIO;
      } else {
         mxfs_change_attributes(inode, &outarg.attr, attr_version);
         if (stat)
            mxfs_fillattr(inode, &outarg.attr, stat);
      }
   }

   return err;
}

int
mxfs_lookup_inode(struct super_block *sb, u64 nodeid, struct inode **inode)
{
   int err;
   struct mxfs_attr_out outarg;
   struct mxfs_conn *mconn = get_mxfs_conn_super(sb);
   u64 attr_version;

   *inode = NULL;
   attr_version = mxfs_get_attr_version(mconn);
   err = do_getattr(mconn, nodeid, &outarg);
   if (!err) {
      if (!mxfs_valid_type(outarg.attr.mode)) {
         return -EINVAL;
      }

      *inode = mxfs_iget(sb, nodeid, &outarg.attr, attr_version);
      if (!*inode) {
         return -ENOMEM;
      }
   }

   return err;
}

int
mxfs_update_attributes(struct inode *inode, struct kstat *stat,
                                        struct file *file, bool *refreshed)
{
   struct mxfs_inode *fi = mxfs_get_inode(inode);
   int err;
   bool r;

   if (fi->i_time < get_jiffies_64()) {
      r = true;
      err = mxfs_do_getattr(inode, stat, file);
      fi->i_time = get_jiffies_64() + HZ;
   } else {
      r = false;
      err = 0;
      if (stat) {
         generic_fillattr(inode, stat);
         stat->mode = fi->orig_i_mode;
      }
   }

   if (refreshed != NULL)
      *refreshed = r;

   return err;
}


/*
 * Set attributes, and at the same time refresh them.
 *
 * Truncation is slightly complicated, because the 'truncate' request
 * may fail, in which case we don't want to touch the mapping.
 * vmtruncate() doesn't allow for this case, so do the rlimit checking
 * and the actual truncation by hand.
 */
int
mxfs_perform_setattr(struct inode *inode, const struct mxfs_setattr *inarg)
{
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   struct mxfs_req *req;
   struct mxfs_attr_out outarg;
   loff_t oldsize;
   int err;

   req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   LOG_DEBUG4("%s: inode=%lx ",__func__,  inode->i_ino);
   memset(&outarg, 0, sizeof(outarg));
   req->in.h.opcode = MXFS_SETATTR;
   req->in.h.nodeid = mxfs_get_nodeid(inode);
   req->in.numargs = 2;
   req->in.args[0].size = sizeof(outarg);
   req->in.args[0].value = &outarg;
   req->in.args[1].size = sizeof(*inarg);
   req->in.args[1].value = inarg;
   req->out.numargs = 1;
   req->out.args[0].size = sizeof(outarg);
   req->out.args[0].value = &outarg;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   if (err) {
      if (err == -EINTR)
         mxfs_invalidate_attr(inode);
      goto error;
   }

   if ((inode->i_mode ^ outarg.attr.mode) & S_IFMT) {
      make_bad_inode(inode);
      err = -EIO;
      goto error;
   }

   spin_lock(&mconn->lock);
   mxfs_change_attributes_common(inode, &outarg.attr, false);
   oldsize = inode->i_size;
   i_size_write(inode, outarg.attr.size);
   spin_unlock(&mconn->lock);

   /*
    * Only call invalidate_inode_pages2() after removing
    * MXFS_NOWRITE, otherwise mxfs_launder_page() would deadlock.
    */
   if (S_ISREG(inode->i_mode) && oldsize != outarg.attr.size) {
//     if RHEL_MAJOR >= 7 && RHEL_MINOR >= 1
#     if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
      truncate_pagecache(inode, outarg.attr.size);
#     else
      truncate_pagecache(inode, oldsize, outarg.attr.size);
#     endif
      invalidate_inode_pages2(inode->i_mapping);
   }

   return 0;

error:
   return err;
}


int
mxfs_reverse_inval_entry(struct super_block *sb,
                                        u64 parent_nodeid, struct qstr *name)
{
   int err = -ENOTDIR;
   struct inode *parent;
   struct dentry *dir;
   struct dentry *entry;

   parent = ilookup5(sb, parent_nodeid, mxfs_inode_eq, &parent_nodeid);
   if (!parent)
      return -ENOENT;

   mutex_lock(&parent->i_mutex);
   if (!S_ISDIR(parent->i_mode))
      goto unlock;

   err = -ENOENT;
   dir = d_find_alias(parent);
   if (!dir)
      goto unlock;

   entry = d_lookup(dir, name);
   dput(dir);
   if (!entry)
      goto unlock;

   mxfs_invalidate_attr(parent);
   mxfs_invalidate_entry(entry);
   dput(entry);
   err = 0;

unlock:
   mutex_unlock(&parent->i_mutex);
   iput(parent);
   return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
static int
parse_dirfile(struct inode *dir, struct dentry *entry, char *buf, size_t nbytes,
        struct file *file, void *dstbuf, filldir_t filldir,
        struct dir_context *ctx)
#else
static int
parse_dirfile(struct inode *dir, struct dentry *entry, char *buf, size_t nbytes,
        struct file *file, void *dstbuf, filldir_t filldir)
#endif
{
   while (nbytes >= MXFS_NAME_OFFSET) {
      struct mxfs_dirent *dirent = (struct mxfs_dirent *) buf;
      size_t reclen = MXFS_DIRENT_SIZE(dirent);
      int over;

      if (!dirent->namelen || dirent->namelen > MXFS_NAME_MAX)
         return -EIO;
      if (reclen > nbytes)
         break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION (3,11,0)
      over = dir_emit (ctx, dirent->name, dirent->namelen,
                       dirent->ino, dirent->type);
#else
      over = filldir(dstbuf, dirent->name, dirent->namelen,
                     file->f_pos, dirent->ino, dirent->type);
#endif

      if (over)
         break;

      buf += reclen;
      nbytes -= reclen;
      file->f_pos = dirent->off;
   }

   return 0;
}



#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
static int
mxfs_iterator(struct file *file, struct dir_context *ctx)
{
   void *dstbuf = &ctx;
   filldir_t filldir = ctx->actor;
#else
static int
mxfs_readdir(struct file *file, void *dstbuf, filldir_t filldir)
{
#endif
   int err;
   size_t nbytes;
   struct page *page;
   struct inode *inode = file->f_path.dentry->d_inode;
   struct mxfs_file *ff = file->private_data;
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   struct mxfs_req *req;
   struct mxfs_read inarg;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
   ctx->pos = ++file->f_pos;
#endif

   if (is_bad_inode(inode))
      return -EIO;

   req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   page = alloc_page(GFP_KERNEL);
   if (!page) {
      mxfs_put_request(mconn, req);
      return -ENOMEM;
   }

   req->page = page;
   inarg.offset = file->f_pos;
   inarg.size = PAGE_SIZE;
   inarg.flags = file->f_flags;
   req->in.h.opcode = MXFS_READDIR;
   req->in.h.nodeid = ff->nodeid;
   req->in.numargs = 2;
   req->in.args[0].size = sizeof (struct mxfs_read);
   req->in.args[0].value = &inarg;
   req->in.args[1].size = PAGE_SIZE;
   req->in.args[1].value = NULL;
   req->out.numargs = 2;
   req->out.args[0].size = sizeof (struct mxfs_read);;
   req->out.args[0].value = &inarg;
   req->out.args[1].size = PAGE_SIZE;
   req->out.args[1].value = page_address(req->page);
   mxfs_request_send(mconn, req);
   nbytes = inarg.size;
   err = req->out.h.error;
   mxfs_put_request(mconn, req);
   if (!err) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
      err = parse_dirfile(inode, file->f_path.dentry,
              page_address(page), nbytes, file, dstbuf, filldir,
                          ctx);
#else
      err = parse_dirfile(inode, file->f_path.dentry,
              page_address(page), nbytes, file, dstbuf, filldir);
#endif
      mxfs_invalidate_attr(inode); /* atime changed */

   } else {
      LOG_ERROR("%s: error from mfs %d", __func__, err);
   }

   __free_page(page);
   return err;
}


static int
mxfs_dir_open(struct inode *inode, struct file *file)
{
   return mxfs_open_common(inode, file);
}

static int
mxfs_dir_release(struct inode *inode, struct file *file)
{
   mxfs_release_common(file);
   return 0;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

static int
mxfs_dir_fsync(struct file *file, struct dentry *de, int datasync)
{
   return 0;
}

#else

static int
mxfs_dir_fsync(struct file *file, loff_t start, loff_t end,
           int datasync)
{
   return 0;
}

#endif


static bool
update_mtime(unsigned ivalid)
{
   /* Always update if mtime is explicitly set  */
   if (ivalid & ATTR_MTIME_SET)
      return true;

   /* If it's an open(O_TRUNC) or an ftruncate(), don't update */
   if ((ivalid & ATTR_SIZE) && (ivalid & (ATTR_OPEN | ATTR_FILE)))
      return false;

   /* In all other cases update */
   return true;
}

static void
iattr_to_fattr(struct iattr *iattr, struct mxfs_setattr *arg)
{
   unsigned ivalid = iattr->ia_valid;

   if (ivalid & ATTR_MODE)
      arg->valid |= MXFS_FATTR_MODE,   arg->mode = iattr->ia_mode;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   if (ivalid & ATTR_UID)
      arg->valid |= MXFS_FATTR_UID,    arg->uid = iattr->ia_uid;
   if (ivalid & ATTR_GID)
      arg->valid |= MXFS_FATTR_GID,    arg->gid = iattr->ia_gid;
#else
   if (ivalid & ATTR_UID)
      arg->valid |= MXFS_FATTR_UID,    arg->uid = iattr->ia_uid.val;
   if (ivalid & ATTR_GID)
      arg->valid |= MXFS_FATTR_GID,    arg->gid = iattr->ia_gid.val;
#endif

   if (ivalid & ATTR_SIZE)
      arg->valid |= MXFS_FATTR_SIZE,   arg->size = iattr->ia_size;
   if (ivalid & ATTR_ATIME) {
      arg->valid |= MXFS_FATTR_ATIME;
      arg->atime = iattr->ia_atime.tv_sec;
      arg->atimensec = iattr->ia_atime.tv_nsec;
      if (!(ivalid & ATTR_ATIME_SET))
         arg->valid |= MXFS_FATTR_ATIME_NOW;
   }
   if ((ivalid & ATTR_MTIME) && update_mtime(ivalid)) {
      arg->valid |= MXFS_FATTR_MTIME;
      arg->mtime = iattr->ia_mtime.tv_sec;
      arg->mtimensec = iattr->ia_mtime.tv_nsec;
      if (!(ivalid & ATTR_MTIME_SET))
         arg->valid |= MXFS_FATTR_MTIME_NOW;
   }
}

static int
mxfs_setattr(struct dentry *entry, struct iattr *attr)
{
   struct inode *inode = entry->d_inode;
   struct mxfs_setattr inarg;
   int err;
   int attr_result;

   if (attr->ia_valid & ATTR_OPEN)
      return 0;

   if (attr->ia_valid & ATTR_SIZE) {
      err = inode_newsize_ok(inode, attr->ia_size);
      if (err)
         return err;
   }

   LOG_DEBUG4("%s: inode=%lx ",__func__,  inode->i_ino);
   memset(&inarg, 0, sizeof(inarg));
   iattr_to_fattr(attr, &inarg);

   attr_result = mxfs_perform_setattr(inode, &inarg);
   mxfs_invalidate_attr(inode);
   mxfs_invalidate_entry_cache(entry);
   return attr_result;
}


static int
mxfs_getattr(struct vfsmount *mnt, struct dentry *entry, struct kstat *stat)
{
   struct inode *inode = entry->d_inode;
   int attr_result;

   LOG_DEBUG4("%s: inode=%lx",__func__,  inode->i_ino);

   attr_result = mxfs_update_attributes(inode, stat, NULL, NULL);
   mxfs_invalidate_attr(inode);
   mxfs_invalidate_entry_cache(entry);
   return attr_result;
}

static const struct inode_operations mxfs_dir_inode_operations = {
   .lookup              = mxfs_lookup,
   .mkdir               = mxfs_mkdir,
   .unlink              = mxfs_unlink,
   .rmdir               = mxfs_rmdir,
   .rename              = mxfs_rename,
   .setattr             = mxfs_setattr,
   .create              = mxfs_create,
   .getattr             = mxfs_getattr,
};

static const struct file_operations mxfs_dir_operations = {
   .llseek              = generic_file_llseek,
   .read                = generic_read_dir,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
   .iterate             = mxfs_iterator,
#else
   .readdir             = mxfs_readdir,
#endif
   .open                = mxfs_dir_open,
   .release             = mxfs_dir_release,
   .fsync               = mxfs_dir_fsync,
};

static const struct inode_operations mxfs_common_inode_operations = {
   .setattr             = mxfs_setattr,
   .getattr             = mxfs_getattr,
};

void
mxfs_init_common(struct inode *inode)
{
   inode->i_op = &mxfs_common_inode_operations;
}

void
mxfs_init_dir(struct inode *inode)
{
   inode->i_op = &mxfs_dir_inode_operations;
   inode->i_fop = &mxfs_dir_operations;
}

