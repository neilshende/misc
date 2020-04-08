#pragma once
#include <stdint.h>
int NvmeCheckHelper( const char *dev, uint64_t &key );
int NvmeAcquireHelper( const char *dev, uint64_t key );
int NvmeReleaseHelper( const char *dev, uint64_t key );
int NvmeChangeHelper( const char *dev, uint64_t key );
int NvmeResetHelper( const char *dev, uint64_t key );
