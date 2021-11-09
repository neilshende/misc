#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <execinfo.h>

static sigjmp_buf mark;
void handler(int signum) {
  void *array[10];
  char **strings;
  int size, i;

  size = backtrace (array, 10);
#if 0
  strings = backtrace_symbols (array, size);
  if (strings != NULL)
  {
    printf("got signal %d, printing %d stack frames.\n", signum, size);
    for (i = 0; i < size; i++) {
      printf("[frame%d]\t%s\n", i, strings[i]);
    }
  }
  free (strings);
#endif
  backtrace_symbols_fd(array, size, 1);
  siglongjmp(mark, -1);
  return;
}

int main(int argc, char *argv[]) {

  if (sigsetjmp(mark, 1) != 0) {
    printf("Houston, we have a problem!\n");
    return 1;
  }
  signal(SIGSEGV, handler);
  signal(SIGILL, handler);
  signal(SIGBUS, handler);
  signal(SIGABRT, handler);

  //do stuff
  *(int *)0 = 0;
  return 0;
}
