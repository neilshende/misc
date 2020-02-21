#include "mxfs_i.h"

#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/parser.h>
#include <linux/statfs.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/exportfs.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sysctl.h>

MODULE_AUTHOR("Maxta, Inc. <info@maxta.com>");
MODULE_DESCRIPTION("Pseudo Filter File System");
MODULE_LICENSE("GPL");

static struct kmem_cache *mxfs_inode_cachep = NULL;
static int debuglevel = MXFS_NO_DEBUG;
int dl_min = MXFS_DEBUG1;
int dl_max = MXFS_NO_DEBUG;
module_param(debuglevel, int, 0);
MODULE_PARM_DESC(debuglevel, "message/trace debug level <1 - 4>");

/* The kernel argument mechanism only works with static variables.
 * However, gcc in kernel mode barfs at "extern static", so we
 * kludge around this by having two variables...
 */

static int argRequestTimeout = DEFAULT_TIMEOUT;
static long argTestInode = 0;
static long argProbeInode = 0;
unsigned int requestTimeout;
long testInode;
long probeInode;
unsigned long ti_min = 0;
unsigned long ti_max = 0xffffFFFFffffFFFF;

module_param(argRequestTimeout, int, 0);
MODULE_PARM_DESC(argRequestTimeout,
                 "If an operation takes longer than this many seconds to "
                 "process, we log a warning message.");

static unsigned int argDentryTimeout = DEFAULT_DENTRY_TIMEOUT;
unsigned int dentryTimeout;

module_param(argDentryTimeout, int, 0);
MODULE_PARM_DESC(argDentryTimeout,
                 "Timeout for entry in dentry lookup cache before it expires.");

static unsigned int argMaxRequestSize = DEFAULT_MAX_REQUEST_SIZE;
unsigned long maxRequestSize;
module_param(argMaxRequestSize, int, 0);
MODULE_PARM_DESC(argMaxRequestSize,
                 "Max. I/O request size for user space.");

module_param(argTestInode, long, 0);
MODULE_PARM_DESC(argTestInode,
		 "Enable I/O short-circuit test mode.");

module_param(argProbeInode, long, 0);
MODULE_PARM_DESC(argProbeInode,
		 "Enable I/O stats gather mode.");

static unsigned int argMaxRequestsPerBuffer;
unsigned int maxRequestsPerBuffer = DEFAULT_MAX_REQUESTS_PER_BUFFER;
module_param(argMaxRequestsPerBuffer, int, 0);
MODULE_PARM_DESC(argMaxRequestsPerBuffer,
                 "Max. requests per buffer returned to user space.");

struct mxfs_conn *mxfs_connp = NULL;
u64 gRootInum = 0;
/* stats are maintained for 512, 4k, 8k, 16k, 32k, 64k, and bigger.
 * The ReadIO and WriteIO have an extra long that tracks the number of
 * errors.
 * The ReadLatency and WriteLatency have extra long that tracks the
 * maximum latency observed.
 */
unsigned long ReadIO[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long ReadLatency[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long ReadBandwidth[7]= {0, 0, 0, 0, 0, 0, 0};
unsigned long WriteIO[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long WriteLatency[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long WriteBandwidth[7] = {0, 0, 0, 0, 0, 0, 0};
/* Mutex for stats calculations.
 */
struct mutex mxfs_stats_mutex;
int resetStats = 0;

static ctl_table test_table[] = {
    {
        .procname    = "testInode",
        .data        = &testInode,
        .maxlen        = sizeof(long),
        .mode        = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {
        .procname    = "probeInode",
        .data        = &probeInode,
        .maxlen        = sizeof(long),
        .mode        = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {
        .procname = "debuglevel",
        .data     = &debuglevel,
        .maxlen   = sizeof(int),
        .mode     = 0644,
        .proc_handler = &proc_dointvec_minmax,
        .extra1   = &dl_min,
        .extra2   = &dl_max
    },
    {
        .procname = "dentryTimeout",
        .data     = &dentryTimeout,
        .maxlen   = sizeof(int),
        .mode     = 0644,
        .proc_handler = &proc_dointvec
    },
    {
        .procname = "maxRequestsPerBuffer",
        .data     = &maxRequestsPerBuffer,
        .maxlen   = sizeof(int),
        .mode     = 0644,
        .proc_handler = &proc_dointvec
    },
    {
        .procname = "resetStats",
        .data     = &resetStats,
        .maxlen   = sizeof(int),
        .mode     = 0644,
        .proc_handler = &proc_dointvec
    },
    {
        .procname = "ReadIO",
        .data     = &ReadIO[0],
        .maxlen   = sizeof(ReadIO),
        .mode     = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {
        .procname = "ReadLatency",
        .data     = &ReadLatency[0],
        .maxlen   = sizeof(ReadLatency),
        .mode     = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {
        .procname = "ReadBandwidth",
        .data     = &ReadBandwidth[0],
        .maxlen   = sizeof(ReadBandwidth),
        .mode     = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {
        .procname = "WriteIO",
        .data     = &WriteIO[0],
        .maxlen   = sizeof(WriteIO),
        .mode     = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {
        .procname = "WriteLatency",
        .data     = &WriteLatency[0],
        .maxlen   = sizeof(WriteLatency),
        .mode     = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {
        .procname = "WriteBandwidth",
        .data     = &WriteBandwidth[0],
        .maxlen   = sizeof(WriteBandwidth),
        .mode     = 0644,
        .proc_handler    = &proc_doulongvec_minmax,
        .extra1      = &ti_min,
        .extra2      = &ti_max
    },
    {}
};
static ctl_table test_mxfs_table[] = {
    {
        .procname    = "mxfs",
        .mode        = 0555,
        .child        = test_table
    },
    {}
};
static ctl_table test_root_table[] = {
    {
        .procname    = "fs",
        .mode        = 0555,
        .child        = test_mxfs_table
    },
    {}
};
static struct ctl_table_header * test_sysctl_header;

static struct inode *
mxfs_alloc_inode(struct super_block *sb)
{
   struct inode *inode;
   struct mxfs_inode *fi;

   inode = kmem_cache_alloc(mxfs_inode_cachep, GFP_KERNEL);
   if (!inode)
      return NULL;

   fi = mxfs_get_inode(inode);
   fi->i_time = 0;
   fi->nodeid = 0;
   fi->attr_version = 0;
   return inode;
}

static void
mxfs_destroy_inode(struct inode *inode)
{
   kmem_cache_free(mxfs_inode_cachep, inode);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
static void
mxfs_clear_inode(struct inode *inode)
{
   return;
}
#endif


static void
mxfs_free_conn(struct mxfs_conn *mconn)
{
   mutex_destroy(&mconn->mutex);
   kfree(mconn);
}

/**
 * Please note if read_only flag is false grabbing spinlock on mconn is mandatory.
 * If read_only is true, we dont modify mconn->attr_version hence we don't need
 * spinlock.
 */
void
mxfs_change_attributes_common(struct inode *inode, struct mxfs_attr *attr, bool read_only)
{
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   struct mxfs_inode *fi = mxfs_get_inode(inode);

   if (!read_only) {
      fi->attr_version = ++mconn->attr_version;
      fi->i_time = 0;
   }

   inode->i_ino     = attr->ino;
   inode->i_mode    = (inode->i_mode & S_IFMT) | (attr->mode & 07777);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   /* Newer kernel version maintain the link count themselves
    * and have it marked as a read-only property.
    */
   inode->i_nlink   = attr->nlink;
   inode->i_uid     = attr->uid;
   inode->i_gid     = attr->gid;
#else
   inode->i_uid.val = attr->uid;
   inode->i_gid.val = attr->gid;
#endif
   inode->i_blocks  = attr->blocks;
   inode->i_atime.tv_sec   = attr->atime;
   inode->i_atime.tv_nsec  = attr->atimensec;
   inode->i_mtime.tv_sec   = attr->mtime;
   inode->i_mtime.tv_nsec  = attr->mtimensec;
   inode->i_ctime.tv_sec   = attr->ctime;
   inode->i_ctime.tv_nsec  = attr->ctimensec;

   if (attr->blksize != 0)
      inode->i_blkbits = ilog2(attr->blksize);
   else
      inode->i_blkbits = inode->i_sb->s_blocksize_bits;

   /*
    * Don't set the sticky bit in i_mode, unless we want the VFS
    * to check permissions.  This prevents failures due to the
    * check in may_delete().
    */
   fi->orig_i_mode = inode->i_mode;
   inode->i_mode &= ~S_ISVTX;
}

void
mxfs_change_attributes(struct inode *inode, struct mxfs_attr *attr,
                                          u64 attr_version)
{
   struct mxfs_conn *mconn = get_mxfs_conn(inode);
   struct mxfs_inode *fi = mxfs_get_inode(inode);
   loff_t oldsize;

   spin_lock(&mconn->lock);
   if (attr_version != 0 && fi->attr_version > attr_version) {
      spin_unlock(&mconn->lock);
      return;
   }

   mxfs_change_attributes_common(inode, attr, false);

   oldsize = inode->i_size;
   i_size_write(inode, attr->size);
   spin_unlock(&mconn->lock);

   if (S_ISREG(inode->i_mode) && oldsize != attr->size) {
#     if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
      truncate_pagecache(inode, attr->size);
#     else
      truncate_pagecache(inode, oldsize, attr->size);
#     endif
      invalidate_inode_pages2(inode->i_mapping);
   }
}

static int
mxfs_init_inode(struct inode *inode, struct mxfs_attr *attr)
{
   inode->i_mode = attr->mode & S_IFMT;
   inode->i_size = attr->size;
   if (S_ISREG(inode->i_mode)) {
      mxfs_init_common(inode);
      mxfs_init_file_inode(inode);
   } else if (S_ISDIR(inode->i_mode))
      mxfs_init_dir(inode);
   else {
      LOG_ERROR("failed to init inode, file mode %ho not supported", inode->i_mode);
      return -EINVAL;
   }

   return 0;
}

int
mxfs_inode_eq(struct inode *inode, void *_nodeidp)
{
   u64 nodeid = *(u64 *) _nodeidp;
   if (mxfs_get_nodeid(inode) == nodeid)
      return 1;
   else
      return 0;
}

static int
mxfs_inode_set(struct inode *inode, void *_nodeidp)
{
   u64 nodeid = *(u64 *) _nodeidp;
   mxfs_get_inode(inode)->nodeid = nodeid;
   return 0;
}

struct inode *
mxfs_iget(struct super_block *sb, u64 nodeid,
                  struct mxfs_attr *attr, u64 attr_version)
{
   int err;
   struct inode *inode;
   struct mxfs_inode *fi;
   bool read_only=true;

retry:
   inode = iget5_locked(sb, nodeid, mxfs_inode_eq, mxfs_inode_set, &nodeid);
   if (!inode)
      return NULL;

   if ((inode->i_state & I_NEW)) {
      inode->i_flags |= S_NOATIME|S_NOCMTIME;
      inode->i_generation = 0;
      err = mxfs_init_inode(inode, attr);
      unlock_new_inode(inode);
      if (err) {
         make_bad_inode(inode);
         iput(inode);
         return NULL;
      }
   } else if ((inode->i_mode ^ attr->mode) & S_IFMT) {
      /* Inode has changed type, any I/O on the old should fail */
      make_bad_inode(inode);
      iput(inode);
      goto retry;
   }
   fi = mxfs_get_inode(inode);
   mxfs_change_attributes_common(inode, attr, read_only);
   inode->i_sb=sb;

   return inode;
}

int
mxfs_reverse_inval_inode(struct super_block *sb, u64 nodeid,
                                                loff_t offset, loff_t len)
{
   struct inode *inode;
   pgoff_t pg_start;
   pgoff_t pg_end;

   inode = ilookup5(sb, nodeid, mxfs_inode_eq, &nodeid);
   if (!inode)
      return -ENOENT;

   mxfs_invalidate_attr(inode);
   if (offset >= 0) {
      pg_start = offset >> PAGE_CACHE_SHIFT;
      if (len <= 0)
         pg_end = -1;
      else
         pg_end = (offset + len - 1) >> PAGE_CACHE_SHIFT;
      invalidate_inode_pages2_range(inode->i_mapping,
            pg_start, pg_end);
   }
   iput(inode);
   return 0;
}

static void
mxfs_umount_begin(struct super_block *sb)
{
   struct mxfs_conn *mconn;

   mconn = get_mxfs_conn_super(sb);
   spin_lock(&mconn->lock);
   mconn->mounted = 0;
   mxfs_end_queued_requests(mconn);
   wake_up_all(&mconn->waitq);
   spin_unlock(&mconn->lock);
}

void
mxfs_conn_kill(struct mxfs_conn *mconn)
{
   /* Flush all readers on this fs */
   wake_up_all(&mconn->waitq);
}

static void
mxfs_put_super(struct super_block *sb)
{
   struct mxfs_conn *mconn = get_mxfs_conn_super(sb);

   mxfs_conn_kill(mconn);
}

static void
convert_mxfs_statfs(struct kstatfs *stbuf, struct mxfs_kstatfs *attr)
{
   stbuf->f_type    = MXFS_SUPER_MAGIC;
   stbuf->f_bsize   = attr->bsize;
   stbuf->f_frsize  = attr->frsize;
   stbuf->f_blocks  = attr->blocks;
   stbuf->f_bfree   = attr->bfree;
   stbuf->f_bavail  = attr->bavail;
   stbuf->f_files   = attr->files;
   stbuf->f_ffree   = attr->ffree;
   stbuf->f_namelen = attr->namelen;
   /* fsid is left zero */
}

static int
mxfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
   struct super_block *sb = dentry->d_sb;
   struct mxfs_conn *mconn = get_mxfs_conn_super(sb);
   struct mxfs_req *req;
   struct mxfs_statfs outarg;
   int err;

   req = mxfs_get_req(mconn);
   if (IS_ERR(req))
      return PTR_ERR(req);

   memset(&outarg, 0, sizeof(outarg));
   req->in.numargs = 0;
   req->in.h.opcode = MXFS_STATFS;
   req->in.h.nodeid = mxfs_get_nodeid(dentry->d_inode);
   req->in.numargs = 1;
   req->in.args[0].size = sizeof(outarg);
   req->in.args[0].value = &outarg;
   req->out.numargs = 1;
   req->out.args[0].size = sizeof(outarg);
   req->out.args[0].value = &outarg;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   if (!err)
      convert_mxfs_statfs(buf, &outarg.st);
   mxfs_put_request(mconn, req);
   return err;
}

struct mxfs_conn *
mxfs_conn_init()
{
   struct mxfs_conn *mconn;
   int i;

   mconn = kzalloc(sizeof(*mconn), GFP_KERNEL);
   mxfs_connp = mconn;
   if (mconn == NULL) {
      return NULL;
   }

   spin_lock_init(&mconn->lock);
   mutex_init(&mconn->mutex);
   init_rwsem(&mconn->killsb);
   atomic_set(&mconn->count, 0);
   init_waitqueue_head(&mconn->waitq);
   INIT_LIST_HEAD(&mconn->pending);
   for (i = 0; i < MXFS_PROCESSING_TABLE_SIZE; i++) {
      INIT_LIST_HEAD(&(mconn->processing[i]));
   }
   mconn->reqctr = 0;
   mconn->attr_version = 1;
   atomic_set(&mconn->count, 0);
   mconn->release = mxfs_free_conn;
   return mconn;
}

void
mxfs_conn_put()
{
   struct mxfs_conn *mconn = mxfs_connp;

   if (mconn && atomic_read(&mconn->count) == 0) {
      mconn->release(mconn);
      mxfs_connp = NULL;
   }
}


static int
mxfs_get_root_inode(struct mxfs_conn *mconn,
                    struct super_block *sb,
                    unsigned mode,
                    struct inode **rootInode)
{
   struct mxfs_attr attr;
   struct mxfs_req *req;
   int err;
   struct mxfs_getrootino outarg;

   *rootInode = NULL;

   spin_lock(&mconn->lock);
   mconn->mounted = 1;
   spin_unlock(&mconn->lock);

   req = mxfs_get_req(mconn);
   if (IS_ERR(req)) {
      return PTR_ERR(req);
   }

   memset(&outarg, 0, sizeof(outarg));
   req->in.h.opcode = MXFS_GETROOTINUM;
   req->in.h.nodeid = 0;
   req->in.numargs = 1;
   req->in.args[0].size = sizeof(outarg);
   req->in.args[0].value = &outarg;
   req->out.numargs = 1;
   req->out.args[0].size = sizeof(outarg);
   req->out.args[0].value = &outarg;
   mxfs_request_send(mconn, req);
   err = req->out.h.error;
   mxfs_put_request(mconn, req);

   if (err != 0) {
      return err;
   }

   attr.mode = mode;
   attr.ino = outarg.ino;
   attr.nlink = 1;

   gRootInum = outarg.ino;

   LOG_INFO("Root inode is %llx", attr.ino);
   *rootInode = mxfs_iget(sb, attr.ino, &attr, 0);
   return 0;
}

struct mxfs_inode_handle {
   u64 nodeid;
   u32 generation;
};

static struct dentry *
mxfs_get_dentry(struct super_block *sb, struct mxfs_inode_handle *handle)
{
   struct inode *inode;
   struct dentry *entry;
   int err = -ESTALE;

   if (handle->nodeid == 0)
      goto out_err;

   inode = ilookup5(sb, handle->nodeid, mxfs_inode_eq, &handle->nodeid);
   if (!inode) {
      err = mxfs_lookup_inode(sb, handle->nodeid, &inode);
      if (err && err != -ENOENT)
         goto out_err;

      if (err || !inode) {
         err = -ESTALE;
         goto out_err;
      }

      err = -EIO;
      if (mxfs_get_nodeid(inode) != handle->nodeid)
         goto out_iput;
   }

   err = -ESTALE;
   if (inode->i_generation != handle->generation)
      goto out_iput;

   entry = d_obtain_alias(inode);
   if (!IS_ERR(entry) && mxfs_get_nodeid(inode) != gRootInum) {
      entry->d_op = &mxfs_dentry_operations;
      mxfs_invalidate_entry_cache(entry);
   }

   return entry;

out_iput:
   iput(inode);

out_err:
   return ERR_PTR(err);
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

static int
mxfs_encode_fh(struct dentry *dentry, u32 *fh, int *max_len, int connectable)
{
   struct inode *inode = dentry->d_inode;
   bool encode_parent = connectable && !S_ISDIR(inode->i_mode);
   int len = encode_parent ? 6 : 3;
   u64 nodeid;
   u32 generation;

   if (*max_len < len)
      return  255;

   nodeid = mxfs_get_inode(inode)->nodeid;
   generation = inode->i_generation;

   fh[0] = (u32)(nodeid >> 32);
   fh[1] = (u32)(nodeid & 0xffffffff);
   fh[2] = generation;

   if (encode_parent) {
      struct inode *parent;

      spin_lock(&dentry->d_lock);
      parent = dentry->d_parent->d_inode;
      nodeid = mxfs_get_inode(parent)->nodeid;
      generation = parent->i_generation;
      spin_unlock(&dentry->d_lock);

      fh[3] = (u32)(nodeid >> 32);
      fh[4] = (u32)(nodeid & 0xffffffff);
      fh[5] = generation;
   }

   *max_len = len;
   return encode_parent ? 0x82 : 0x81;
}

#else

static int
mxfs_encode_fh(struct inode *inode, u32 *fh, int *max_len,
	       struct inode *parent)
{
   int len = parent ? 6 : 3;
   u64 nodeid;
   u32 generation;

   if (*max_len < len)
      return  255;

   nodeid = mxfs_get_inode(inode)->nodeid;
   generation = inode->i_generation;

   fh[0] = (u32)(nodeid >> 32);
   fh[1] = (u32)(nodeid & 0xffffffff);
   fh[2] = generation;

   if (parent) {
      nodeid = mxfs_get_inode(parent)->nodeid;
      generation = parent->i_generation;

      fh[3] = (u32)(nodeid >> 32);
      fh[4] = (u32)(nodeid & 0xffffffff);
      fh[5] = generation;
   }

   *max_len = len;
   return parent ? 0x82 : 0x81;
}

#endif



static struct dentry *
mxfs_fh_to_dentry(struct super_block *sb, struct fid *fid,
                                                int fh_len, int fh_type)
{
   struct mxfs_inode_handle handle;

   if ((fh_type != 0x81 && fh_type != 0x82) || fh_len < 3)
      return NULL;

   handle.nodeid = (u64) fid->raw[0] << 32;
   handle.nodeid |= (u64) fid->raw[1];
   handle.generation = fid->raw[2];
   return mxfs_get_dentry(sb, &handle);
}

static struct dentry *
mxfs_fh_to_parent(struct super_block *sb, struct fid *fid,
                                                int fh_len, int fh_type)
{
   struct mxfs_inode_handle parent;

   if (fh_type != 0x82 || fh_len < 6)
      return NULL;

   parent.nodeid = (u64) fid->raw[3] << 32;
   parent.nodeid |= (u64) fid->raw[4];
   parent.generation = fid->raw[5];
   return mxfs_get_dentry(sb, &parent);
}

static struct dentry *
mxfs_get_parent(struct dentry *child)
{
   struct inode *child_inode = child->d_inode;
   struct inode *inode;
   struct dentry *parent;
   struct mxfs_entry outarg;
   struct qstr name;
   int err;

   name.len = 2;
   name.name = "..";
   err = mxfs_lookup_name(child_inode->i_sb, mxfs_get_nodeid(child_inode),
                                                      &name, &outarg, &inode);
   if (err) {
      if (err == -ENOENT)
         return ERR_PTR(-ESTALE);
      return ERR_PTR(err);
   }

   parent = d_obtain_alias(inode);
   if (!IS_ERR(parent) && mxfs_get_nodeid(inode) != gRootInum) {
      parent->d_op = &mxfs_dentry_operations;
      mxfs_invalidate_entry_cache(parent);
   }

   return parent;
}

static const struct export_operations mxfs_export_operations = {
   .fh_to_dentry        = mxfs_fh_to_dentry,
   .fh_to_parent        = mxfs_fh_to_parent,
   .encode_fh           = mxfs_encode_fh,
   .get_parent          = mxfs_get_parent,
};

static const struct super_operations mxfs_super_operations = {
   .alloc_inode         = mxfs_alloc_inode,
   .destroy_inode       = mxfs_destroy_inode,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   .clear_inode         = mxfs_clear_inode,
#endif
   .drop_inode          = generic_delete_inode,
   .put_super           = mxfs_put_super,
   .umount_begin        = mxfs_umount_begin,
   .statfs              = mxfs_statfs,
};



static int
mxfs_fill_super(struct super_block *sb, void *data, int silent)
{

   struct inode *root;
   struct dentry *root_dentry;
   int err;
   struct mxfs_conn *mconn = mxfs_connp;

   if (gRootInum != 0) {
       LOG_ERROR("mxfs is already mounted.");
       err = -EBUSY;
       goto err;
   }

   err = -EINVAL;
   if (sb->s_flags & MS_MANDLOCK) {
      LOG_ERROR("no support for mandatory lock");
      goto err;
   }

   sb->s_blocksize = PAGE_CACHE_SIZE;
   sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
   sb->s_magic = MXFS_SUPER_MAGIC;
   sb->s_op = &mxfs_super_operations;
   sb->s_maxbytes = MAX_LFS_FILESIZE;
   sb->s_export_op = &mxfs_export_operations;

   mconn->sb = sb;
   sb->s_flags |= MS_POSIXACL;

   /* Used by get_mxfs_conn_super() */
   sb->s_fs_info = mconn;

   err = mxfs_get_root_inode(mconn, sb, MXFS_ROOT_MODE, &root);
   if (err != 0) {
      LOG_ERROR("Cannot get root inode: %d", err);
      goto err;
   }
#if 0
   /* Disabling this fix as the one done above with gRootInum
    * is sufficient.
    */
   if (atomic_read(&root->i_count) > 1) {
      err = -EBUSY;
      iput(root);
      LOG_ERROR("Mounting mxfs more than once is not allowed.");
      goto err;
   }
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   root_dentry = d_alloc_root(root);
#else
   root_dentry = d_make_root(root);
#endif
   if (!root_dentry) {
      LOG_ERROR("Cannot allocate root inode.");
      iput(root);
      err = -ENOMEM;
      goto err_dput;
   }

   sb->s_root = root_dentry;
   return 0;

err_dput:
   dput(root_dentry);
err:
   return err;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

static int
mxfs_get_sb(struct file_system_type *fs_type, int flags,
	    const char *dev_name, void *raw_data, struct vfsmount *mnt)
{
   return get_sb_nodev(fs_type, flags, raw_data, mxfs_fill_super, mnt);
}

#else

static struct dentry*
mxfs_mount(struct file_system_type *fs_type, int flags,
	   const char *dev_name, void *raw_data)
{
   return mount_nodev(fs_type, flags, raw_data, mxfs_fill_super);
}

#endif


static void
mxfs_kill_sb_anon(struct super_block *sb)
{
   struct mxfs_conn *mconn = get_mxfs_conn_super(sb);

   if (mconn) {
      down_write(&mconn->killsb);
      mconn->sb = NULL;
      up_write(&mconn->killsb);
   }

   kill_anon_super(sb);
}

static struct file_system_type mxfs_fs_type = {
   .owner       = THIS_MODULE,
   .name        = "mxfs",
   .fs_flags    = FS_HAS_SUBTYPE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
   .get_sb      = mxfs_get_sb,
#else
   .mount       = mxfs_mount,
#endif
   .kill_sb     = mxfs_kill_sb_anon,
};

static void
mxfs_inode_init_once(void *foo)
{
   struct inode *inode = foo;

   inode_init_once(inode);
}

static int
mxfs_fs_init(void)
{
   int err;

   err = register_filesystem(&mxfs_fs_type);
   if (err)
      goto out;

   mxfs_inode_cachep = kmem_cache_create("mxfs_inode",
         sizeof(struct mxfs_inode),
         0, SLAB_HWCACHE_ALIGN,
         mxfs_inode_init_once);
   err = -ENOMEM;
   if (!mxfs_inode_cachep)
      goto out_unreg2;

   return 0;

out_unreg2:
   unregister_filesystem(&mxfs_fs_type);
out:
   return err;
}

static void
mxfs_fs_cleanup(void)
{
   unregister_filesystem(&mxfs_fs_type);
   kmem_cache_destroy(mxfs_inode_cachep);
}

static int __init
mxfs_init(void)
{
   int res;

   if (argRequestTimeout >= 0) {
      requestTimeout = argRequestTimeout;
   } else {
      requestTimeout = DEFAULT_TIMEOUT;
   }

   testInode = argTestInode;
   if (testInode != 0) {
     LOG_WARN("mxfs MODULE IN TEST MODE. NO I/O WILL BE FORWARDED TO MFSD FOR "
	      "INODE %lu.", testInode);
   }

   probeInode = argProbeInode;
   if (probeInode != 0) {
       LOG_INFO("mxfs module in stats gathering mode for Inode %lu.",
                probeInode);
   }

   /* Set dentry cache timeout */
   dentryTimeout = argDentryTimeout;
   LOG_INFO("mxfs module using denrty timeout = %u", dentryTimeout);

   if (argMaxRequestSize > 0) {
      maxRequestSize = argMaxRequestSize;
   } else {
      maxRequestSize = DEFAULT_MAX_REQUEST_SIZE;
   }
   LOG_INFO("mxfs module using max. i/o request size = %lu", maxRequestSize);

   if (argMaxRequestsPerBuffer < 0) {
      LOG_INFO("mxfs module: Invalid max. requests per buffer setting, using "
               "default.");
   } else if (argMaxRequestsPerBuffer > 0) {
      // User-specified limit
      maxRequestsPerBuffer = argMaxRequestsPerBuffer;
   }
   // argMaxRequestsPerBuffer == 0 --> argument not specified/use default
   LOG_INFO("mxfs module: Returning up to %u requests per buffer.",
            maxRequestsPerBuffer);

   res = mxfs_fs_init();
   if (res)
      goto err;

   res = mxfs_dev_init();
   if (res)
      goto err_fs_cleanup;

   test_sysctl_header = register_sysctl_table(test_root_table);

   mutex_init(&mxfs_stats_mutex);

   LOG_INFO("mxfs module loaded (API version %i)", MXFS_VERSION);
   return 0;

   mxfs_dev_cleanup();
err_fs_cleanup:
   mxfs_fs_cleanup();
err:
   return res;
}

static void __exit
mxfs_exit(void)
{
   mxfs_fs_cleanup();
   mxfs_dev_cleanup();
   unregister_sysctl_table(test_sysctl_header);
   mutex_destroy(&mxfs_stats_mutex);
   LOG_INFO("mxfs module unloaded");
}

struct mxfs_conn *
mxfs_get_conn(void)
{
   return mxfs_connp;
}

int
mxfs_get_debuglevel(void)
{
   return debuglevel;
}

module_init(mxfs_init);
module_exit(mxfs_exit);

