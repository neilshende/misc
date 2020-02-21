#ifndef __LOG_H__
#define __LOG_H__

enum mx_debuglevel {
   MXFS_DEBUG1 = 1,
   MXFS_DEBUG2 = 2,
   MXFS_DEBUG3 = 3,
   MXFS_DEBUG4 = 4,
   MXFS_NO_DEBUG = 5
};

int mxfs_get_debuglevel(void);

#define LOG_ERROR(_fmt_, _args_...)                             \
      printk(KERN_ERR "mxfs: " _fmt_, ##_args_)

#define LOG_WARN(_fmt_, _args_...)                              \
      printk(KERN_WARNING "mxfs: " _fmt_, ##_args_)

#define LOG_INFO(_fmt_, _args_...)                              \
   printk(KERN_INFO "mxfs: " _fmt_, ##_args_)

#define LOG_DEBUG1(_fmt_, _args_...)                            \
   if (mxfs_get_debuglevel() <= MXFS_DEBUG1)                    \
      printk(KERN_INFO "mxfs: " _fmt_, ##_args_)

#define LOG_DEBUG2(_fmt_, _args_...)                            \
   if (mxfs_get_debuglevel() <= MXFS_DEBUG2)                    \
      printk(KERN_INFO "mxfs: " _fmt_, ##_args_)

#define LOG_DEBUG3(_fmt_, _args_...)                            \
   if (mxfs_get_debuglevel() <= MXFS_DEBUG3)                    \
      printk(KERN_INFO "mxfs: " _fmt_, ##_args_)

#define LOG_DEBUG4(_fmt_, _args_...)                            \
   if (mxfs_get_debuglevel() <= MXFS_DEBUG4)                    \
      printk(KERN_INFO "mxfs: " _fmt_, ##_args_)

#endif /* __LOG_H__ */


