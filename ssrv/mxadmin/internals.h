#ifndef __INTERNALS_H__
#define __INTERNALS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "mxfs.h"

static const int MaxBufferSize = 1024 * 128 + 512;
static const unsigned long gMXFSRootInum = 1UL;

int InitDev(void);
void DeinitDev(void);
int CleanupDev(void);
int PollDev(void);
ssize_t ReadDev(void *pBuffer, size_t nlen);
ssize_t WriteDev(const void *pBuffer, size_t nlen);

int ProcessRequest(char *pOutBuffer, struct mxfs_header *pMsg,
                                             char *pData, unsigned nDataLen);

#endif /* __INTERNALS_H__ */

