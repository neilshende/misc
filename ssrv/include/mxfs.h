#ifndef __MXFS_H__
#define __MXFS_H__

#include <linux/types.h>

#define MXFS_VERSION             1
#define MXFS_MINOR               240
#define MXFS_LEGACY_ROOT_ID      1UL
#define MXFS_ROOT_MODE           (040777)

#define MXFS_IOCTL_INIT          _IOW(MXFS_MINOR, 1, int)
#define MXFS_IOCTL_DEINIT        _IO(MXFS_MINOR, 2)

#define MXFS_HEADER_FMT "gInode: %lu, unique: %lx, opcode: %d, error: %d"
#define MXFS_HEADER_ARGS(A) (unsigned long)(A).nodeid, (unsigned long)(A).unique, \
                            (A).opcode, (A).error

/* Make sure all structures are padded to 64bit boundary, so 32bit
   userspace works under 64bit kernels */

struct mxfs_attr {
   __u64        ino;
   __u64        size;
   __u64        blocks;
   __u64        atime;
   __u64        mtime;
   __u64        ctime;
   __u32        atimensec;
   __u32        mtimensec;
   __u32        ctimensec;
   __u32        mode;
   __u32        nlink;
   __u32        uid;
   __u32        gid;
   __u32        rdev;
   __u32        blksize;
   __u32        pagesize;
};

struct mxfs_kstatfs {
   __u64        blocks;
   __u64        bfree;
   __u64        bavail;
   __u64        files;
   __u64        ffree;
   __u32        bsize;
   __u32        namelen;
   __u32        frsize;
   __u32        padding;
   __u32        spare[6];
};

/**
 * Bitmasks for mxfs_setattr_in.valid
 */
#define MXFS_FATTR_MODE      (1 << 0)
#define MXFS_FATTR_UID       (1 << 1)
#define MXFS_FATTR_GID       (1 << 2)
#define MXFS_FATTR_SIZE      (1 << 3)
#define MXFS_FATTR_ATIME     (1 << 4)
#define MXFS_FATTR_MTIME     (1 << 5)
#define MXFS_FATTR_ATIME_NOW (1 << 6)
#define MXFS_FATTR_MTIME_NOW (1 << 7)

enum mxfs_opcode {
   MXFS_FORGET          = 1,
   MXFS_LOOKUP          = 2,
   MXFS_GETATTR         = 3,
   MXFS_SETATTR         = 4,
   MXFS_MKDIR           = 5,
   MXFS_UNLINK          = 6,
   MXFS_RMDIR           = 7,
   MXFS_RENAME          = 8,
   MXFS_READ            = 9,
   MXFS_WRITE           = 10,
   MXFS_STATFS          = 11,
   MXFS_READDIR         = 12,
   MXFS_CREATE          = 13,
   MXFS_GETROOTINUM     = 14,
};

enum mxfs_notify_code {
   MXFS_NOTIFY_INVAL_INODE = 2,
   MXFS_NOTIFY_INVAL_ENTRY = 3,
   MXFS_NOTIFY_CODE_MAX,
};

struct mxfs_entry {
   __u64        nodeid;
   struct mxfs_attr attr;
};

struct mxfs_attr_out {
   struct mxfs_attr attr;
};

struct mxfs_mkdir {
   struct mxfs_entry entry;
   __u32        mode;
   __u32        uid;
   __u32        gid;
   __u32        umask;
};

struct mxfs_rename {
   __u64        newdir;
};

struct mxfs_setattr {
   __u32        valid;
   __u32        padding;
   __u64        size;
   __u64        atime;
   __u64        mtime;
   __u64        unused2;
   __u32        atimensec;
   __u32        mtimensec;
   __u32        unused3;
   __u32        mode;
   __u32        unused4;
   __u32        uid;
   __u32        gid;
   __u32        unused5;
};

struct mxfs_create {
   struct mxfs_entry entry;
   __u32        mode;
   __u32        uid;
   __u32        gid;
   __u32        umask;
};

struct mxfs_read {
   __u64        offset;
   __u32        size;
   __u32        flags;
};

struct mxfs_write {
   __u64        offset;
   __u32        size;
   __u32        padding;
};

struct mxfs_statfs {
   struct mxfs_kstatfs st;
};

struct mxfs_header {
   __u32        opcode;
   __s32        error;
   __u64        unique;
   __u64        unused;
   __u64        nodeid;
};

struct mxfs_dirent {
   __u64        ino;
   __u64        off;
   __u32        type;
   __u32        namelen;
   char         name[0];
};

struct mxfs_getrootino {
   __u64 ino;
};


#define MXFS_NAME_OFFSET     offsetof(struct mxfs_dirent, name)
#define MXFS_DIRENT_ALIGN(x) (((x) + sizeof(__u64) - 1) & ~(sizeof(__u64) - 1))
#define MXFS_DIRENT_SIZE(d)  MXFS_DIRENT_ALIGN(MXFS_NAME_OFFSET + (d)->namelen)

struct mxfs_notify_inval_inode_out {
   __u64        ino;
   __s64        off;
   __s64        len;
};

struct mxfs_notify_inval_entry_out {
   __u64        parent;
   __u32        namelen;
   __u32        padding;
};

#define ROUNDUP_8BYTE(_x) (((_x) + 7) & ~(7))

#endif /* __MXFS_H__ */

