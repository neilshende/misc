#include <unistd.h>
static int ExecuteCommandSilently(const std::string command) {
    if (command.empty()) return 1;

    pid_t child_pid = fork();
    if (child_pid < 0) {
      return 2;
    }

    if (0 == child_pid) {
      int fd = open("/dev/null", O_WRONLY);
      if (fd < 0) {
        return 3;
      }
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      close(fd);
      execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
      _exit(EXIT_FAILURE);
    }

    int status = 0;
    waitpid(child_pid, &status, 0);
    return status;
  }
