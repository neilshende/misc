#include "internals.h"
#include "fs.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>

static const char *pBaseDir = "/root/tmp/mxfsdir/";
static int gNULLIO = 0;

static int
getPathFromINUM(__u64 ino, char **pPath)
{
   char pCommand[FS_PATH_MAX];
   FILE *pFile;
   int err;
   size_t nLen = 0;

   if (ino == gMXFSRootInum) {
      *pPath = strdup(pBaseDir);
      return 0;
   }

   snprintf(pCommand, FS_PATH_MAX, "find %s -inum %llu -print", pBaseDir, ino);
   pFile = popen(pCommand, "re");
   if (! pFile) {
      printf("error: popen(%s) failed\n", pCommand);
      return EINVAL;
   }

   err = getline(pPath, &nLen, pFile);
   if (err == -1) {
      printf("error: getline() failed for popen(%s)\n", pCommand);
      pclose(pFile);
      return EINVAL;
   }

   pclose(pFile);
   if (err > 0) {
      (*pPath)[(err - 1)] = '\0';
   }

   return 0;
}

int
FS_Lookup(__u64 Parentino, const char *pName, struct mxfs_entry *pEntry)
{
   char *pPath = NULL;
   char FilePath[FS_PATH_MAX];
   struct stat sbuf;
   int err;

   err = getPathFromINUM(Parentino, &pPath);
   if (err) {
      return -EINVAL;
   }

   snprintf(FilePath, FS_PATH_MAX, "%s/%s", pPath, pName);
   free(pPath);
   err = lstat(FilePath, &sbuf);
   if (err == -1) {
      return -errno;
   }

   memset(pEntry, 0, sizeof (struct mxfs_entry));
   pEntry->nodeid = sbuf.st_ino;
   pEntry->attr.ino = sbuf.st_ino;
   pEntry->attr.size = sbuf.st_size;
   pEntry->attr.blocks = sbuf.st_blocks;
   pEntry->attr.atime = sbuf.st_atime;
   pEntry->attr.mtime = sbuf.st_mtime;
   pEntry->attr.ctime = sbuf.st_ctime;
   pEntry->attr.atimensec = 0;
   pEntry->attr.mtimensec = 0;
   pEntry->attr.ctimensec = 0;
   pEntry->attr.mode = sbuf.st_mode;
   pEntry->attr.nlink = sbuf.st_nlink;
   pEntry->attr.uid = sbuf.st_uid;
   pEntry->attr.gid = sbuf.st_gid;
   pEntry->attr.rdev = sbuf.st_rdev;
   pEntry->attr.blksize = sbuf.st_blksize;
   return 0;
}

int
FS_GetAttr(__u64 ino, struct mxfs_attr_out *pAttrOut)
{
   char *pPath = NULL;
   struct stat sbuf;
   int err;

   err = getPathFromINUM(ino, &pPath);
   if (err) {
      return -EINVAL;
   }

   err = lstat(pPath, &sbuf);
   if (err == -1) {
      err = -errno;
      free(pPath);
      return err;
   }

   free(pPath);
   memset(pAttrOut, 0, sizeof (struct mxfs_attr_out));
   pAttrOut->attr.ino = sbuf.st_ino;
   pAttrOut->attr.size = sbuf.st_size;
   pAttrOut->attr.blocks = sbuf.st_blocks;
   pAttrOut->attr.atime = sbuf.st_atime;
   pAttrOut->attr.mtime = sbuf.st_mtime;
   pAttrOut->attr.ctime = sbuf.st_ctime;
   pAttrOut->attr.atimensec = 0;
   pAttrOut->attr.mtimensec = 0;
   pAttrOut->attr.ctimensec = 0;
   pAttrOut->attr.mode = sbuf.st_mode;
   pAttrOut->attr.nlink = sbuf.st_nlink;
   pAttrOut->attr.uid = sbuf.st_uid;
   pAttrOut->attr.gid = sbuf.st_gid;
   pAttrOut->attr.rdev = sbuf.st_rdev;
   pAttrOut->attr.blksize = sbuf.st_blksize;
   return 0;
}

int
FS_ReadDir(__u64 ino, struct mxfs_read *pRead, char *pBuffer,
                                                         __u32 MaxBufferLen)
{
   char *pPath = NULL;
   DIR *pDir;
   struct dirent *pDirent;
   struct stat sbuf;
   off_t NextOffset;
   struct mxfs_dirent *pMXDirent;
   __u32 namelen;
   __u32 entlen;
   __u32 reclen;
   int err;

   err = getPathFromINUM(ino, &pPath);
   if (err) {
      return -EINVAL;
   }

   pDir = opendir(pPath);
   if (! pDir) {
      err = -errno;
      free(pPath);
      return err;
   }

   free(pPath);
   seekdir(pDir, pRead->offset);
   pRead->size = 0;
   while (1) {
      pDirent = readdir(pDir);
      if (! pDirent) {
         break;
      }

      memset(&sbuf, 0, sizeof (sbuf));
      sbuf.st_ino = pDirent->d_ino;
      sbuf.st_mode = pDirent->d_type << 12;
      NextOffset = telldir(pDir);

      namelen = strlen(pDirent->d_name);
      entlen = MXFS_NAME_OFFSET + namelen;
      reclen = MXFS_DIRENT_ALIGN(entlen);
      if (pRead->size + reclen > MaxBufferLen) {
         break;
      }

      pMXDirent = (struct mxfs_dirent *) (pBuffer + pRead->size);
      pMXDirent->ino = sbuf.st_ino;
      pMXDirent->off = NextOffset;
      pMXDirent->type = (sbuf.st_mode & 0170000) >> 12;
      pMXDirent->namelen = namelen;
      strncpy(pMXDirent->name, pDirent->d_name, pMXDirent->namelen);
      if (reclen - entlen) {
         memset((pBuffer + pRead->size + entlen), 0, (reclen - entlen));
      }

      pRead->size += reclen;
   }

   closedir(pDir);
   return 0;
}

int
FS_StatFS(__u64 ino, struct mxfs_statfs *pStatFS)
{
   char *pPath = NULL;
   struct statvfs sbuf;
   int err;

   err = getPathFromINUM(ino, &pPath);
   if (err) {
      return -EINVAL;
   }

   err = statvfs(pPath, &sbuf);
   if (err == -1) {
      err = -errno;
      free(pPath);
      return err;
   }

   free(pPath);
   memset(pStatFS, 0, sizeof (struct mxfs_statfs));
   pStatFS->st.blocks = sbuf.f_blocks;
   pStatFS->st.bfree = sbuf.f_bfree;
   pStatFS->st.bavail = sbuf.f_bavail;
   pStatFS->st.files = sbuf.f_files;
   pStatFS->st.ffree = sbuf.f_ffree;
   pStatFS->st.bsize = sbuf.f_bsize;
   pStatFS->st.namelen = sbuf.f_namemax;
   pStatFS->st.frsize = sbuf.f_frsize;
   return 0;
}

int
FS_MkDir(__u64 Parentino, char *pName, __u32 mode, struct mxfs_entry *pEntry)
{
   char *pPath = NULL;
   char Path[FS_PATH_MAX];
   struct stat sbuf;
   int err;

   err = getPathFromINUM(Parentino, &pPath);
   if (err) {
      return -EINVAL;
   }

   snprintf(Path, FS_PATH_MAX, "%s/%s", pPath, pName);
   free(pPath);
   err = mkdir(Path, mode);
   if (err == -1) {
      err = -errno;
      return err;
   }

   err = lstat(Path, &sbuf);
   if (err == -1) {
      err = -errno;
      return err;
   }

   memset(pEntry, 0, sizeof (struct mxfs_entry));
   pEntry->nodeid = sbuf.st_ino;
   pEntry->attr.ino = sbuf.st_ino;
   pEntry->attr.size = sbuf.st_size;
   pEntry->attr.blocks = sbuf.st_blocks;
   pEntry->attr.atime = sbuf.st_atime;
   pEntry->attr.mtime = sbuf.st_mtime;
   pEntry->attr.ctime = sbuf.st_ctime;
   pEntry->attr.atimensec = 0;
   pEntry->attr.mtimensec = 0;
   pEntry->attr.ctimensec = 0;
   pEntry->attr.mode = sbuf.st_mode;
   pEntry->attr.nlink = sbuf.st_nlink;
   pEntry->attr.uid = sbuf.st_uid;
   pEntry->attr.gid = sbuf.st_gid;
   pEntry->attr.rdev = sbuf.st_rdev;
   pEntry->attr.blksize = sbuf.st_blksize;
   return 0;
}

int
FS_RmDir(__u64 Parentino, char *pName)
{
   char *pPath = NULL;
   char Path[FS_PATH_MAX];
   int err;

   err = getPathFromINUM(Parentino, &pPath);
   if (err) {
      return -EINVAL;
   }

   snprintf(Path, FS_PATH_MAX, "%s/%s", pPath, pName);
   free(pPath);
   err = rmdir(Path);
   if (err == -1) {
      err = -errno;
      return err;
   }

   return 0;
}

int
FS_Unlink(__u64 Parentino, char *pName)
{
   char *pPath = NULL;
   char Path[FS_PATH_MAX];
   int err;

   err = getPathFromINUM(Parentino, &pPath);
   if (err) {
      return -EINVAL;
   }

   snprintf(Path, FS_PATH_MAX, "%s/%s", pPath, pName);
   free(pPath);
   err = unlink(Path);
   if (err == -1) {
      err = -errno;
      return err;
   }

   return 0;
}

int
FS_Rename(__u64 Oldino, char *pOldName, __u64 Newino, char *pNewName)
{
   char *pPath = NULL;
   char OldPath[FS_PATH_MAX];
   char NewPath[FS_PATH_MAX];
   int err;

   err = getPathFromINUM(Oldino, &pPath);
   if (err) {
      return -EINVAL;
   }

   snprintf(OldPath, FS_PATH_MAX, "%s/%s", pPath, pOldName);
   free(pPath);
   err = getPathFromINUM(Newino, &pPath);
   if (err) {
      return -EINVAL;
   }

   snprintf(NewPath, FS_PATH_MAX, "%s/%s", pPath, pNewName);
   free(pPath);
   err = rename(OldPath, NewPath);
   if (err == -1) {
      err = -errno;
      return err;
   }

   return 0;
}

int
FS_SetAttr(__u64 ino, struct mxfs_setattr *pSetAttr,
                                              struct mxfs_attr_out *pAttrOut)
{
   char *pPath = NULL;
   uid_t uid;
   gid_t gid;
   struct timeval tv[2];
   struct stat sbuf;
   int err;

   err = getPathFromINUM(ino, &pPath);
   if (err) {
      return -EINVAL;
   }

   if (!err && (pSetAttr->valid & MXFS_FATTR_MODE)) {
      err = chmod(pPath, pSetAttr->mode);
      if (err == -1) {
         err = -errno;
      }
   }

   if (!err && (pSetAttr->valid & (MXFS_FATTR_UID | MXFS_FATTR_GID))) {
      uid = -1;
      gid = -1;
      if (pSetAttr->valid & MXFS_FATTR_UID) {
         uid = pSetAttr->uid;
      }

      if (pSetAttr->valid & MXFS_FATTR_GID) {
         gid = pSetAttr->gid;
      }

      err = lchown(pPath, uid, gid);
      if (err == -1) {
         err = -errno;
      }
   }

   if (!err && (pSetAttr->valid & MXFS_FATTR_SIZE)) {
      err = truncate(pPath, pSetAttr->size);
      if (err == -1) {
         err = -errno;
      }
   }

   if (!err && ((pSetAttr->valid & (MXFS_FATTR_MTIME | MXFS_FATTR_ATIME)) ==
                                    (MXFS_FATTR_MTIME | MXFS_FATTR_ATIME))) {
         tv[0].tv_sec = pSetAttr->atime;
         tv[0].tv_usec = pSetAttr->atimensec / 1000;
         tv[1].tv_sec = pSetAttr->mtime;
         tv[1].tv_usec = pSetAttr->mtimensec / 1000;
         err = utimes(pPath, tv);
         if (err == -1) {
            err = -errno;
         }
   }

   if (err) {
      free(pPath);
      return err;
   }

   err = lstat(pPath, &sbuf);
   if (err == -1) {
      err = -errno;
      free(pPath);
      return err;
   }

   free(pPath);
   memset(pAttrOut, 0, sizeof (struct mxfs_attr_out));
   pAttrOut->attr.ino = sbuf.st_ino;
   pAttrOut->attr.size = sbuf.st_size;
   pAttrOut->attr.blocks = sbuf.st_blocks;
   pAttrOut->attr.atime = sbuf.st_atime;
   pAttrOut->attr.mtime = sbuf.st_mtime;
   pAttrOut->attr.ctime = sbuf.st_ctime;
   pAttrOut->attr.atimensec = 0;
   pAttrOut->attr.mtimensec = 0;
   pAttrOut->attr.ctimensec = 0;
   pAttrOut->attr.mode = sbuf.st_mode;
   pAttrOut->attr.nlink = sbuf.st_nlink;
   pAttrOut->attr.uid = sbuf.st_uid;
   pAttrOut->attr.gid = sbuf.st_gid;
   pAttrOut->attr.rdev = sbuf.st_rdev;
   pAttrOut->attr.blksize = sbuf.st_blksize;
   return 0;
}

int
FS_Write(__u64 ino, struct mxfs_write *pWrite, const void *pBuffer)
{
   char *pPath = NULL;
   int fd;
   int err;

   if (gNULLIO) return pWrite->size;
   err = getPathFromINUM(ino, &pPath);
   if (err) {
      return -EINVAL;
   }

   fd = open(pPath, O_WRONLY);
   if (fd == -1) {
      free(pPath);
      err = -errno;
      return err;
   }

   free(pPath);
   err = pwrite(fd, pBuffer, pWrite->size, pWrite->offset);
   if (err == -1) {
      err = -errno;
      close(fd);
      return err;
   }

   close(fd);
   return 0;
}

int
FS_Read(__u64 ino, struct mxfs_read *pRead, void *pBuffer)
{
   char *pPath = NULL;
   int fd;
   int err;
   struct stat statBuf;

   if (gNULLIO) return pRead->size;

   err = getPathFromINUM(ino, &pPath);
   if (err) {
      return -EINVAL;
   }

   fd = open(pPath, O_RDONLY);
   if (fd == -1) {
      err = -errno;
      free(pPath);
      return err;
   }

   err = fstat(fd, &statBuf);
   if (err == -1) {
      err = -errno;
      close(fd);
      free(pPath);
      return err;
   }

   if (statBuf.st_size < pRead->offset + pRead->size) {
      fprintf(stderr, "WARNING: Confirmed that %d byte read from from %s, "
           "offset %lld would extend past end of file which is at %ld.\n",
           pRead->size, pPath, pRead->offset, statBuf.st_size);
   }


   free(pPath);
   err = pread(fd, pBuffer, pRead->size, pRead->offset);
   if (err == -1) {
      err = -errno;
      close(fd);
      return err;
   }

   close(fd);
   return err;
}

int
FS_Create(__u64 Parentino, char *pName, struct mxfs_create *pCreate)
{
   char *pPath = NULL;
   char Path[FS_PATH_MAX];
   int fd;
   struct stat sbuf;
   int err;

   err = getPathFromINUM(Parentino, &pPath);
   if (err) {
      return -EINVAL;
   }

   snprintf(Path, FS_PATH_MAX, "%s/%s", pPath, pName);
   free(pPath);
   fd = open(Path, (O_CREAT | O_EXCL | O_WRONLY), pCreate->mode);
   if (fd == -1) {
      err = -errno;
      return err;
   }

   err = fstat(fd, &sbuf);
   if (err == -1) {
      err = -errno;
      close(fd);
      return err;
   }

   close(fd);
   memset(&pCreate->entry, 0, sizeof (struct mxfs_entry));
   pCreate->entry.nodeid = sbuf.st_ino;
   pCreate->entry.attr.ino = sbuf.st_ino;
   pCreate->entry.attr.size = sbuf.st_size;
   pCreate->entry.attr.blocks = sbuf.st_blocks;
   pCreate->entry.attr.atime = sbuf.st_atime;
   pCreate->entry.attr.mtime = sbuf.st_mtime;
   pCreate->entry.attr.ctime = sbuf.st_ctime;
   pCreate->entry.attr.atimensec = 0;
   pCreate->entry.attr.mtimensec = 0;
   pCreate->entry.attr.ctimensec = 0;
   pCreate->entry.attr.mode = sbuf.st_mode;
   pCreate->entry.attr.nlink = sbuf.st_nlink;
   pCreate->entry.attr.uid = sbuf.st_uid;
   pCreate->entry.attr.gid = sbuf.st_gid;
   pCreate->entry.attr.rdev = sbuf.st_rdev;
   pCreate->entry.attr.blksize = sbuf.st_blksize;
   return 0;
}

