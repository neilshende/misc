#include <stdlib.h>
#include <stdio.h>
#include "scope_guard.h"

int main(int argc, char *argv[]) {

  if (argc < 2) return 0;
  int fd = open(argv[1], O_RDONLY);
  ScopeGuard guard_fd([fd]() { close(fd); });
  // do stuff
  return 0;
}
