#include <dirent.h>
#include <mvmlog.h>
#include <sys/eventfd.h>
#include <sys/wait.h>
#include <unistd.h>

#include <functional>
#include <unordered_set>
#include <vector>

#include "scope_guard.h"

inline void CloseAllFds(const std::unordered_set<int> &except_fds) {
  DIR *dir = opendir("/proc/self/fd");
  assert(dir != nullptr);
  ScopeGuard guard_dir([dir]() { closedir(dir); });
  for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
    int fd = std::stoi(ent->d_name);
    if (except_fds.find(fd) != except_fds.end()) continue;
    close(fd);
  }
}

/**
 * Executes a binary with pipe and extra arguments. The executed binary will inherit two
 * pipeline fd's for input and output parameters. The binary will also inherit an event fd
 * which the binary should use to indicate the parent process that the execution is done
 * and termination of the child process is expected.
 *
 * The binary to be executed should receive the following arguments:
 * $1: the fd number of the pipeline to read from the parent
 * $2: the fd number of the pipeline to write to the parent
 * $3: the fd number of an eventfd which is used to notify the parent that execution is
 * terminated
 * $4...: any other custom arguments
 *
 * The parameter \p preserved_fds provides a list of fds that should not be closed in a
 * spawned subprocess.
 *
 * TODO(@muxi): use SIGCHLD instead of eventfd to identify termination of child
 */
inline int SubprocInvoke(const std::string &path, const std::string *in, std::string *out,
                         std::vector<const char *> args,
                         std::unordered_set<int> preserved_fds) {
  int pipe_in[2];
  int pipe_out[2];
  int efd = -1;
  int r = pipe2(pipe_in, 0);
  if (r < 0) {
    MVMLOG("Failed creating pipe for invoke in subprocess: %m\n");
    return -1;
  }
  ScopeGuard guard_pipe_in_0([pipe_in]() { close(pipe_in[0]); });
  ScopeGuard guard_pipe_in_1([pipe_in]() { close(pipe_in[1]); });
  r = pipe2(pipe_out, 0);
  if (r < 0) {
    MVMLOG("Failed creating pipe for invoke in subprocess: %m\n");
    return -1;
  }
  ScopeGuard guard_pipe_out_0([pipe_out]() { close(pipe_out[0]); });
  ScopeGuard guard_pipe_out_1([pipe_out]() { close(pipe_out[1]); });
  efd = eventfd(0, 0);
  if (efd < 0) {
    MVMLOG("Failed creating efd for invoke in subprocess: %m\n");
    return -1;
  }
  ScopeGuard guard_efd([efd]() { close(efd); });
  pid_t child_pid = fork();
  if (child_pid < 0) {
    MVMLOG("Failed forking for invoke in subprocess: %m\n");
    return -1;
  }
  if (child_pid > 0) {
    /* parent */
    guard_pipe_in_0.Execute();
    guard_pipe_out_1.Execute();
    if (in != nullptr) {
      size_t offset = 0;
      while (offset < in->size()) {
        int r = write(pipe_in[1], &(*in)[offset], in->size() - offset);
        if (r < 0 && r != EINTR) {
          MVMLOG("Failed writing input parameters to the child process\n");
          return -1;
        } else if (r > 0) {
          offset += r;
        }
      }
    }
    guard_pipe_in_1.Execute();
    /* need to poll an eventfd to determine whether the child process exited because the
     * pipe may remain open due to the child process forking a grand-child, which
     * remains running after the child exited and holds a reference to the pipe */
    if (out != nullptr) out->clear();
    struct pollfd pfds[] = {{efd, POLLIN, 0}, {pipe_out[0], POLLIN, 0}};
    int nfds = 2;
    while (true) {
      /* TODO(@muxi): use SIGCHLD instead of eventfd when EINTR is handled across the
       * project */
      int r = poll(pfds, nfds, -1);
      assert(r != 0);
      if (r < 0 && r != EINTR) {
        MVMLOG("Failed polling child process events.\n");
        return -1;
      }
      if (r > 0) {
        if (nfds == 2 && pfds[1].revents != 0) {
          char buf[4096];
          r = read(pipe_out[0], buf, sizeof(buf));
          if (r > 0) {
            if (out != nullptr) out->insert(out->size(), buf, r);
          } else if (r < 0) {
            MVMLOG("Failed reading child returned data: %m\n");
            return -1;
          } else {
            /* do not poll the pipe any more since it's closed */
            nfds = 1;
          }
        } else {
          uint64_t unused;
          read(efd, &unused, sizeof(unused));
          assert(pfds[0].revents != 0);
          break;
        }
      }
    }
    int wstatus;
    waitpid(child_pid, &wstatus, 0);
    return WEXITSTATUS(wstatus);
  } else {
    /* child */
    guard_pipe_in_1.Execute();
    guard_pipe_out_0.Execute();
    preserved_fds.insert(pipe_in[0]);
    preserved_fds.insert(pipe_out[1]);
    preserved_fds.insert(efd);
    CloseAllFds(preserved_fds);
    std::string pipe_in_0 = std::to_string(pipe_in[0]);
    std::string pipe_out_1 = std::to_string(pipe_out[1]);
    std::string efd_str = std::to_string(efd);
    std::vector<const char *> fixed_args = {path.c_str(), pipe_in_0.c_str(),
                                            pipe_out_1.c_str(), efd_str.c_str()};
    args.insert(args.begin(), fixed_args.begin(), fixed_args.end());
    args.push_back(nullptr);
    /* The const cast is ok since execv is guaranteed to not change the contents of the
     * array; the signature did not add const specifier only for historical reason.
     * See ref: https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html */
    int r = execv(path.c_str(), const_cast<char **>(&args[0]));
    if (r != 0) {
      MVMLOG("Failed exec: %m\n");
      exit(-1);
    }
    exit(0);
  }
}
