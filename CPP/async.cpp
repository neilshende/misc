#include <iostream>
#include <semaphore>
const long MAX_WRITE_SIZE = 4*12024*1024;
const long MAX_MEM = 100*1024*1024;
void my_write(void *buff, int size, int offset, std::function<void()> done_cb) {
}
int my_read(void *buf, int max) {
}
int main() {
  bool eof = false;
  long goffset;
  int max_pending_writes = MAX_MEM/MAX_WRITE_SIZE;
  std::counting_semaphore s(max_pending_writes);
  while (true) {
    long size = 0, rc=0, offset=0; 
    char *buff = (char *) malloc(MAX_WRITE_SIZE); 
      if (buff == NULL) {
        // FIXME LOG an error
        return 1;
      }  
    while (size <= MAX_WRITE_SIZE) {
       rc = my_read (buff+offset, MAX_WRITE_SIZE - size);
       if (rc == -1) {
         eof = true;
         break;
       }
       size += rc;
       offset += rc;
    }
    if (size) {
       s.acquire(1);
       my_write(buff, size, goffset, [buf, s] () {
          s.release(1);
          free(buf);
       });
       goffset += size;
    }
    if (eof) {
      // now wait here till all writes complete.
      // This we accomplish by trying to acquire full count.
      s.acquire(max_pending_writes);
      break;
    }
  }
  return 0;
}
