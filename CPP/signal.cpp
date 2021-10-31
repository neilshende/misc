#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

static sigjmp_buf mark;

void handler(int signum) {
  void *array[10];
  char **strings;
  int size, i;

  size = backtrace (array, 10);
  //backtrace_symbols_fd(array, size, STDOUT_FILENO);
  //or do it with following code.
  strings = backtrace_symbols (array, size);
  if (strings != NULL)
  {
    printf("got signal %d, printing %d stack frames.\n", signum, size);
    for (i = 0; i < size; i++) {
      printf("[frame%d]\t%s\n", i, strings[i]);
    }
  }
  free (strings);

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
