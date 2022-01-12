#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
pid_t pid;
pid = fork();
if (pid == 0) {
   setuid(1077);
   if ( system("touch ~viveks/touchmenot") ) {
      printf("failed system call \n");
      exit (1);
   }
   exit(0);
}
else if (pid < 0) {
  printf("fork failure\n");
  return 1;
}
pid_t got_pid;
int state;
got_pid = waitpid(pid, &state, 0);
if (got_pid != pid) printf("(%ld) got_pid=%d pid=%d\n", time(0), got_pid, pid);
        printf("(%ld) got_pid=%d\n", time(0), got_pid);   // 2
        printf("(%ld) WIFEXITED: %d\n", time(0), WIFEXITED(state));  // 3
        printf("(%ld) WEXITSTATUS: %d\n", time(0), WEXITSTATUS(state)); // 4
        printf("(%ld) Done from parent\n", time(0));
sleep(600);
return 0;
}

