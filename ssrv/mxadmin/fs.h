#ifndef __FS_H__
#define __FS_H__

#define FS_PATH_MAX  150

int FS_Lookup(__u64 Parentino, const char *pName, struct mxfs_entry *pEntry);
int FS_GetAttr(__u64 ino, struct mxfs_attr_out *pAttrOut);
int FS_ReadDir(__u64 ino, struct mxfs_read *pRead, char *pBuffer,
                                                         __u32 MaxBufferLen);
int FS_StatFS(__u64 ino, struct mxfs_statfs *pStatFS);
int FS_MkDir(__u64 Parentino, char *pName,
                                       __u32 mode, struct mxfs_entry *pEntry);
int FS_RmDir(__u64 Parentino, char *pName);
int FS_Unlink(__u64 Parentino, char *pName);
int FS_Rename(__u64 Oldino, char *pOldName, __u64 Newino, char *pNewName);
int FS_SetAttr(__u64 ino, struct mxfs_setattr *pSetAttr,
                                             struct mxfs_attr_out *pAttrOut);
int FS_Write(__u64 ino, struct mxfs_write *pWrite, const void *pBuffer);
int FS_Read(__u64 ino, struct mxfs_read *pRead, void *pBuffer);
int FS_Create(__u64 Parentino, char *pName, struct mxfs_create *pCreate);

#endif /* __FS_H__ */

