#include "internals.h"
#include "fs.h"

static int
Lookup(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_entry *pEntry;
   char *pName;
   unsigned nLen;
   int err;

   pEntry = (struct mxfs_entry *) pData;
   pName = (char *) (pData + sizeof (struct mxfs_entry));

   err = FS_Lookup(pMsg->nodeid, pName, pEntry);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = (unsigned) sizeof (struct mxfs_entry);
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   nLen += sizeof (struct mxfs_entry);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
GetAttr(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_attr_out *pAttrOut;
   unsigned nLen;
   int err;

   pAttrOut = (struct mxfs_attr_out *) pData;

   err = FS_GetAttr(pMsg->nodeid, pAttrOut);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = (unsigned) sizeof (struct mxfs_attr_out);
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   nLen += sizeof (struct mxfs_attr_out);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
SetAttr(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_attr_out *pAttrOut;
   struct mxfs_setattr *pSetAttr;
   unsigned nLen;
   int err;

   pAttrOut = (struct mxfs_attr_out *) pData;
   pSetAttr = (struct mxfs_setattr *) (pData + sizeof (struct mxfs_attr_out));

   err = FS_SetAttr(pMsg->nodeid, pSetAttr, pAttrOut);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = (unsigned) sizeof (struct mxfs_attr_out);
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   nLen += sizeof (struct mxfs_attr_out);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
MkDir(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_mkdir *pMkDir;
   struct mxfs_entry *pEntry;
   char *pName;
   unsigned nLen;
   int err;

   pMkDir = (struct mxfs_mkdir *) pData;
   pEntry = (struct mxfs_entry *) pData;
   pName = (char *) (pData + sizeof (struct mxfs_mkdir));

   err = FS_MkDir(pMsg->nodeid, pName, pMkDir->mode, pEntry);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = (unsigned) sizeof (struct mxfs_entry);
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   nLen += sizeof (struct mxfs_entry);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
Unlink(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   char *pName;
   unsigned nLen;
   int err;

   pName = (char *) pData;

   err = FS_Unlink(pMsg->nodeid, pName);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = 0;
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
RmDir(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   char *pName;
   unsigned nLen;
   int err;

   pName = (char *) pData;

   err = FS_RmDir(pMsg->nodeid, pName);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = 0;
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
Rename(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_rename *pRename;
   char *pOldName;
   char *pNewName;
   unsigned nLen;
   int err;

   pRename = (struct mxfs_rename *) pData;
   pOldName = (char *) (pData + sizeof (struct mxfs_rename));
   nLen = strlen(pOldName) + 1;
   pNewName = (char *) (pOldName + nLen);

   err = FS_Rename(pMsg->nodeid, pOldName, pRename->newdir, pNewName);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = 0;
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
Read(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_read *pRead;
   __u32 nSize;
   void *pBuffer;
   unsigned nLen;
   int err;

   pRead = (struct mxfs_read *) pData;
   nSize = pRead->size;
   pBuffer = (void *) pData;

   err = FS_Read(pMsg->nodeid, pRead, pBuffer);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = nSize;
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   nLen += nSize;
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
Write(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_write *pWrite;
   void *pBuffer;
   unsigned nLen;
   int err;

   pWrite = (struct mxfs_write *) pData;
   pBuffer = (void *) (pData + sizeof (struct mxfs_write));

   err = FS_Write(pMsg->nodeid, pWrite, pBuffer);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = 0;
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
StatFS(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_statfs *pStatFS;
   unsigned nLen;
   int err;

   pStatFS = (struct mxfs_statfs *) pData;

   err = FS_StatFS(pMsg->nodeid, pStatFS);
   pMsg->error = err;
   nLen = sizeof (unsigned) + sizeof (unsigned) +
                     sizeof (struct mxfs_header) + sizeof (struct mxfs_statfs);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
ReadDir(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_read *pRead;
   char *pBuffer;
   unsigned MaxBufferLen;
   unsigned nLen;
   int err;

   pRead = (struct mxfs_read *) pData;
   pBuffer = (char *) (pData + sizeof (struct mxfs_read));
   MaxBufferLen = nDataLen - sizeof (struct mxfs_read);

   err = FS_ReadDir(pMsg->nodeid, pRead, pBuffer, MaxBufferLen);
   pMsg->error = err;
   nLen = sizeof (unsigned) + sizeof (unsigned) +
                                       sizeof (struct mxfs_header) + nDataLen;
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
Create(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   struct mxfs_create *pCreate;
   char *pName;
   unsigned nLen;
   int err;

   pCreate = (struct mxfs_create *) pData;
   pName = (char *) (pData + sizeof (struct mxfs_create));

   err = FS_Create(pMsg->nodeid, pName, pCreate);
   pMsg->error = err;
   nLen = sizeof (unsigned);
   *(unsigned *) (pOutBuffer + nLen) = (unsigned) sizeof (struct mxfs_create);
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   nLen += sizeof (struct mxfs_create);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}

static int
GetRootInum(char *pOutBuffer, struct mxfs_header *pMsg,
            char *pData, unsigned nDataLen)
{
   struct mxfs_getrootino *pGetIno;
   int err;
   unsigned nLen;

   pGetIno = (struct mxfs_getrootino*)pData;
   pGetIno->ino = gMXFSRootInum;
   pMsg->error = 0;
   nLen = sizeof(unsigned);
   *(unsigned *)(pOutBuffer + nLen) = (unsigned) sizeof(struct mxfs_getrootino);
   nLen += sizeof (unsigned);
   nLen += sizeof (struct mxfs_header);
   nLen += sizeof (struct mxfs_getrootino);
   err = WriteDev(pOutBuffer, nLen);
   return err;
}


int
ProcessRequest(char *pOutBuffer, struct mxfs_header *pMsg,
                                                char *pData, unsigned nDataLen)
{
   int err;

   switch (pMsg->opcode) {
   case MXFS_LOOKUP:
      err = Lookup(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_GETATTR:
      err = GetAttr(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_SETATTR:
      err = SetAttr(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_MKDIR:
      err = MkDir(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_UNLINK:
      err = Unlink(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_RMDIR:
      err = RmDir(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_RENAME:
      err = Rename(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_READ:
      err = Read(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_WRITE:
      err = Write(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_STATFS:
      err = StatFS(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_READDIR:
      err = ReadDir(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_CREATE:
      err = Create(pOutBuffer, pMsg, pData, nDataLen);
      break;

   case MXFS_GETROOTINUM:
      err = GetRootInum(pOutBuffer, pMsg, pData, nDataLen);
      break;

   default:
      printf("error: invalid request %d received\n", pMsg->opcode);
      err = EINVAL;
      break;
   }

   return err;
}

