#define Swap4Bytes(val) \
 ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | \
   (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )
 
#define Swap8Bytes(val) \
 ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
   (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
   (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
   (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )




//typedef unsigned int uint32_t;
//typedef unsigned long uint64_t;
//typedef unsigned short uint16_t;
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <strings.h>

typedef struct QCowSnapshotHeader {
      /* header is 8 byte aligned */
      uint64_t l1_table_offset;

      uint32_t l1_size;
      uint16_t id_str_size;
      uint16_t name_size;

      uint32_t date_sec;
      uint32_t date_nsec;

      uint64_t vm_clock_nsec;

      uint32_t vm_state_size;
      uint32_t extra_data_size; /* for extension */
      /* extra data follows */
      /* id_str follows */
      /* name follows  */
  } QCowSnapshotHeader;

  typedef struct QCowHeader {
      uint32_t magic;
      uint32_t version;

      uint64_t backing_file_offset;
      uint32_t backing_file_size;

      uint32_t cluster_bits;
      uint64_t size; /* in bytes */
      uint32_t crypt_method;

      uint32_t l1_size;
      uint64_t l1_table_offset;

      uint64_t refcount_table_offset;
      uint32_t refcount_table_clusters;

      uint32_t nb_snapshots;
      uint64_t snapshots_offset;
  } QCowHeader;
#if 0
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <strings.h>
#endif
int main(int argc, char *argv[])
{
int ret;
QCowHeader head;
if (argc != 2) {
   printf("%s qcow-file\n",argv[0]);
   return 0;
}

int fd=open(argv[1],O_RDONLY);
if (fd < 0) {
   printf("unable to open %s.\n", argv[1]);
   return 1;
}
int count=read(fd, (void *)&head, sizeof(head));
if (count != sizeof(head)) {
   printf("Unable to read %s.\n", argv[1]);
   ret = 1;
}
else if (head.magic != 0xfb494651 ) {
   printf("%s is not a qcow file.\n", argv[1]);
   ret = 0;
} 
else if (head.backing_file_offset != 0) {

   head.backing_file_offset = Swap8Bytes(head.backing_file_offset);
   head.backing_file_size = Swap4Bytes(head.backing_file_size);
   char filename[head.backing_file_size+1];
   memset(filename, 0, head.backing_file_size+1);
   pread(fd,(void *)filename,head.backing_file_size, head.backing_file_offset);
   
   printf("This qcow file has backing file %s.\n", filename);
   
   ret= 1;
} else {
  printf("This qcow file does not have a backing file.\n");
  ret= 0;
}
close(fd);
return ret;
}

