#ifndef __MXFS_I_H__
#define __MXFS_I_H__

#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/backing-dev.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/rbtree.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#include "mxfs.h"
#include "log.h"

/** Size of (hash) table to hold requests pending in userland. */
#define MXFS_PROCESSING_TABLE_SIZE 1024

#define MXFS_SUPER_MAGIC      0x65735546

/** It could be as large as PATH_MAX, but would that have any uses? */
#define MXFS_NAME_MAX         1024

/** Default timeout (in seconds) before we log a warning. */
#define DEFAULT_TIMEOUT 60

/** Default dentry cache timeout in seconds. */
#define DEFAULT_DENTRY_TIMEOUT 1

/** Default max. i/o request size for user space. */
#define DEFAULT_MAX_REQUEST_SIZE (128 * 1024)

/** Default setting for max. requests per buffer */
#define DEFAULT_MAX_REQUESTS_PER_BUFFER 1

/** MXFS inode */
struct mxfs_inode {
   /** Inode data */
   struct inode inode;

   /** Unique ID, which identifies the inode between userspace
    * and kernel */
   u64 nodeid;

   /** Time in jiffies until the file attributes are valid */
   u64 i_time;

   /** The sticky bit in inode->i_mode may have been removed, so
     preserve the original mode */
   mode_t orig_i_mode;

   /** Version of last attribute change */
   u64 attr_version;
};

struct mxfs_conn;

/** MXFS specific file data */
struct mxfs_file {
   /** Fuse connection for this file */
   struct mxfs_conn *mconn;

   /** Node id of this file */
   u64 nodeid;

   /** Refcount */
   atomic_t count;
};

/** One input argument of a request */
struct mxfs_in_arg {
   unsigned long size;
   const void *value;
};

/** The request input */
struct mxfs_in {
   /** The request header */
   struct mxfs_header h;

   /** True if the data for the last argument is in req->pages */
   unsigned argpages:1;

   /** Number of arguments */
   unsigned numargs;

   /** Array of arguments */
   struct mxfs_in_arg args[3];
};

/** One output argument of a request */
struct mxfs_arg {
   unsigned long size;
   void *value;
};

/** The request output */
struct mxfs_out {
   /** Header returned from userspace */
   struct mxfs_header h;

   /** Number or arguments */
   unsigned numargs;

   /** Array of arguments */
   struct mxfs_arg args[3];
};

/** The request state */
enum mxfs_req_state {
   MXFS_REQ_INIT = 0,
   MXFS_REQ_PROCESSING,
   MXFS_REQ_FINISHED
};

/**
 * A request to the client
 */
struct mxfs_req {
   /** This can be on either pending processing or io lists */
   struct list_head list;

   spinlock_t lock;

   /** State of the request */
   enum mxfs_req_state state;

   /** The request input */
   struct mxfs_in in;

   /** The request output */
   struct mxfs_out out;

   /** Used to wake up the task waiting for completion of request*/
   wait_queue_head_t waitq;

   /** page */
   struct page *page;

   /** File used in the request (or NULL) */
   struct mxfs_file *ff;

   /** Inode used in the request or NULL */
   struct inode *inode;
   unsigned mlen;
   unsigned dlen;

   struct iov_iter *ioviter;
};

/**
 *
 * This structure is created, when the filesystem is mounted, and is
 * destroyed, when the client device is closed and the filesystem is
 * unmounted.
 */
struct mxfs_conn {
   /** Lock protecting accessess to  members of this structure */
   spinlock_t lock;

   /** Mutex protecting against directory alias creation */
   struct mutex mutex;

   /** Refcount */
   atomic_t count;

   /** Readers of the connection are waiting on this */
   wait_queue_head_t waitq;

   /** The list of pending requests */
   struct list_head pending;

   /** The table of requests being processed */
   struct list_head processing[MXFS_PROCESSING_TABLE_SIZE];

   /** The next unique request id */
   u64 reqctr;

   /** Connection established, cleared on umount, connection
     abort and device release */
   unsigned connected;

   /** File system mounted. */
   unsigned mounted;

   /** Version counter for attribute changes */
   u64 attr_version;

   /** Called on final put */
   void (*release)(struct mxfs_conn *);

   /** Super block for this connection. */
   struct super_block *sb;

   /** Read/write semaphore to hold when accessing sb. */
   struct rw_semaphore killsb;
};


/** Default timeout (in seconds) for NFS operations before we
 *  log a warning...
 */
extern unsigned int requestTimeout;

/**
 * If true, we are running in test mode, and no reads/writes will
 * be forwarded to mfsd for inode # testInode.
 */
extern long testInode;

/*
 * probeInode when set, will enable stats gathering for that inode.
 * Setting value to -1 will enable stats gathering for all inodes.
 */
extern long probeInode;

/*
 * following stats are gathered: number of IOs, Latency and Bandwidth.
 * The seperate measurements are made for read and write.
 * The IO sizes are 512, 4k, 8k, 16k, 32k, 64k, and bigger than 64k.
 * The last integer in IO array contains number of errors:
 *    ReadIO[7] and WriteIO[7].
 * The last interger in latency array stores max latency observed:
 *    ReadLatency[7] and WriteLatency[7]
 * All these values are exported via procfs.
 */

extern unsigned long ReadIO[8];
extern unsigned long ReadLatency[8];
extern unsigned long ReadBandwidth[7];
extern unsigned long WriteIO[8];
extern unsigned long WriteLatency[8];
extern unsigned long WriteBandwidth[7];

/* Mutex for stats calculations.
 */
extern struct mutex mxfs_stats_mutex;

/*
 * procfs int to force reset of stats
 */
extern int resetStats;

/** How long are we willing to cache a dEntry. */
extern unsigned int dentryTimeout;

/** Max. request size to send to user space. */
extern unsigned long maxRequestSize;

/** Max. requests per buffer returned to user space. */
extern unsigned int maxRequestsPerBuffer;

/** Inode number of root directory. */
extern u64 gRootInum;


static inline struct mxfs_conn *get_mxfs_conn_super(struct super_block *sb)
{
   return sb->s_fs_info;
}

static inline struct mxfs_conn *get_mxfs_conn(struct inode *inode)
{
   return get_mxfs_conn_super(inode->i_sb);
}

static inline struct mxfs_inode *mxfs_get_inode(struct inode *inode)
{
   return container_of(inode, struct mxfs_inode, inode);
}

static inline u64 mxfs_get_nodeid(struct inode *inode)
{
   return mxfs_get_inode(inode)->nodeid;
}

/** Device operations */
extern const struct file_operations mxfs_dev_operations;

extern const struct dentry_operations mxfs_dentry_operations;

/**
 * Inode to nodeid comparison.
 */
int mxfs_inode_eq(struct inode *inode, void *_nodeidp);

/**
 * Get a filled in inode
 */
struct inode *mxfs_iget(struct super_block *sb, u64 nodeid,
      struct mxfs_attr *attr, u64 attr_version);

int mxfs_lookup_name(struct super_block *sb, u64 nodeid, struct qstr *name,
      struct mxfs_entry *outarg, struct inode **inode);

/**
 * Send OPEN or OPENDIR request
 */
int mxfs_open_common(struct inode *inode, struct file *file);

struct mxfs_file *mxfs_file_alloc(struct mxfs_conn *mconn);
struct mxfs_file *mxfs_file_get(struct mxfs_file *ff);
void mxfs_file_free(struct mxfs_file *ff);
int mxfs_finish_open(struct inode *inode, struct file *file);

/**
 * Send RELEASE or RELEASEDIR request
 */
void mxfs_release_common(struct file *file);

/**
 * Initialize file operations on a regular file
 */
void mxfs_init_file_inode(struct inode *inode);

/**
 * Initialize inode operations on regular files and special files
 */
void mxfs_init_common(struct inode *inode);

/**
 * Initialize inode and file operations on a directory
 */
void mxfs_init_dir(struct inode *inode);

/**
 * Change attributes of an inode
 */
void mxfs_change_attributes(struct inode *inode, struct mxfs_attr *attr,
      u64 attr_version);

void mxfs_change_attributes_common(struct inode *inode,
      struct mxfs_attr *attr, bool read_only);

struct mxfs_conn *mxfs_get_conn(void);

/**
 * Initialize the client device
 */
int mxfs_dev_init(void);

/**
 * Cleanup the client device
 */
void mxfs_dev_cleanup(void);

/**
 * Allocate a request
 */
struct mxfs_req *mxfs_request_alloc(void);

/**
 * Free a request
 */
void mxfs_request_free(struct mxfs_req *req);

/**
 * Get a request, may fail with -ENOMEM
 */
struct mxfs_req *mxfs_get_req(struct mxfs_conn *mconn);

/**
 * Decrement reference count of a request.  If count goes to zero free
 * the request.
 */
void mxfs_put_request(struct mxfs_conn *mconn, struct mxfs_req *req);

/**
 * Send a request (synchronous)
 */
void mxfs_request_send(struct mxfs_conn *mconn, struct mxfs_req *req);



/* Abort all requests */
void mxfs_end_queued_requests(struct mxfs_conn *mconn);


/**
 * Invalidate inode attributes
 */
void mxfs_invalidate_attr(struct inode *inode);

void mxfs_invalidate_entry_cache(struct dentry *entry);

void mxfs_conn_kill(struct mxfs_conn *mconn);

/**
 * Initialize mxfs_conn
 */
struct mxfs_conn *mxfs_conn_init(void);

/**
 * Release reference to mxfs_conn
 */
void mxfs_conn_put(void);

/**
 * Is file type valid?
 */
int mxfs_valid_type(int m);

int mxfs_lookup_inode(struct super_block *sb, u64 nodeid,
      struct inode **inode);

int mxfs_update_attributes(struct inode *inode, struct kstat *stat,
      struct file *file, bool *refreshed);

/**
 * Do a SETATTR upcall to mfsd and change the internal inode state to pick
 * up changes specified in "inarg"
 */
int mxfs_perform_setattr(struct inode *inode, const struct mxfs_setattr *inarg);

u64 mxfs_get_attr_version(struct mxfs_conn *mconn);

/**
 * File-system tells the kernel to invalidate cache for the given node id.
 */
int mxfs_reverse_inval_inode(struct super_block *sb, u64 nodeid,
      loff_t offset, loff_t len);

/**
 * File-system tells the kernel to invalidate parent attributes and
 * the dentry matching parent/name.
 */
int mxfs_reverse_inval_entry(struct super_block *sb, u64 parent_nodeid,
      struct qstr *name);

int mxfs_dev_release(struct inode *inode, struct file *file);

#endif /* __MXFS_I_H__ */

