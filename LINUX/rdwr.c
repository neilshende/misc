#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <execinfo.h>
static sigjmp_buf mark;

#define MAX_PATH 1024
       void
       bye(void)
       {
           printf("That was all, folks\n");
       }

void handler(int signum) {
  void *array[10];
  char **strings;
  int size, i;

  size = backtrace (array, 10);
  backtrace_symbols_fd(array, size, 1);
  siglongjmp(mark, -1);
  return;
}


int main(int argc, char *argv[]) {
   
        //char *volID = strtok(argv[1], ":");
        //char *mountPt = strtok( NULL, ":");
        //printf("[%s] is volId [%s] is mountPt", volID, mountPt);
#if 1
   int i = atexit(bye);
  if (sigsetjmp(mark, 1) != 0) {
    printf("Houston, we have a problem!\n");
    return 1;
  }
  signal(SIGSEGV, handler);
  signal(SIGILL, handler);
  signal(SIGBUS, handler);
  signal(SIGABRT, handler);
  signal(SIGINT, handler);
  signal(SIGSTOP, SIG_IGN);
#endif
 int x=0;
   int n = 1000000;
   int cr, cw;
   if (argc > 1) n = atoi(argv[1]);
   int fdw = open("/dev/null", O_WRONLY);
   int fdr = open("/dev/random", O_RDONLY);
   char buf[1024*1024] = {0};
   while (1) {
   printf("test %d\n", x++);
   //sleep(10000);
   for (int i=0; i<n; i++) {
     cr = read(fdr, buf, sizeof(buf));
     if (cr != sizeof(buf)) {
       static int once=1;
       if (once) {
          printf("Unable to read %d %d\n", sizeof(buf), cr);
          once = 0;
       }
     }
     cw = write(fdw, buf, cr);
     if (cw != cr) {
       printf("Unable to write %d %d\n", cr, cw);
       return 1;
     }
   }
   }
   return 0;
}

