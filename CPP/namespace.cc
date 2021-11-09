#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wordexp.h>
#include <fcntl.h>
#include <dirent.h>
#include <functional>

#include <vector>

enum {
  NS_START = 0,
  NS_NET = NS_START,
  NS_PID,
  NS_MNT,
  NS_END
};
static constexpr struct NsDesc {
  unsigned int cflag;
  const char *str;
  bool need_fork;
} ns_descs_[NS_END] = {
  {
    .cflag = CLONE_NEWNET,
    .str = "net",
    .need_fork = false
  },
  {
    .cflag = CLONE_NEWPID,
    .str = "pid",
    .need_fork = true
  },
  /* Have to put mnt ns the last one, because after changed mnt ns,
   * it cannot see /proc/self/ns/pid anymore */
  {
    .cflag = CLONE_NEWNS,
    .str = "mnt",
    .need_fork = false
  }
};

/* TODO: refactor this class when merging to master */
class MvNamespace {
 public:
  MvNamespace(const char *cmd)
      : MvNamespace({"/bin/sh", "-c", cmd, nullptr}) {}

  MvNamespace(const std::vector<const char *> &argv, int target_ns = 0, bool clone = true)
      : argv_(argv), target_ns_(target_ns), ns_name_(nullptr), clone_(clone) {}

  void SetNSName(const char *name) {
     ns_name_ = name;
  }

  int Run(const char *pidns_path = nullptr) {
    int rc;
    /* If set target, always enter the target namespace first */
    if (target_ns_ > 0) {
      rc = EnterNamespaces(target_ns_);
      if (rc) return rc;
    }

    /* Enter net ns if needed */
    if (ns_name_ != nullptr) {
      rc = EnterNetNs();
      if (rc != 0) {
        printf("Run network namespace %s failed.\n", ns_name_);
        return rc;
      }
    }

    /* Create new PID&MNT ns for clone */
    if (clone_) {
      return ExecuteInNewPidNs(pidns_path);
    }
    return Execute(pidns_path);
  }

  static uint32_t GetNamespacePID(pid_t pid = 0)
  {
    char buf[64] = {0};
    std::string ns_link = "/proc/" + (pid ? std::to_string(pid) : "self") + "/ns/pid";
    int nread = readlink(ns_link.c_str(), buf, sizeof(buf));
    if (nread < 0 || nread >= sizeof(buf)) return 0;

    uint32_t id = 0;
    if (sscanf(buf, "pid:[%u]", &id) == 0) return 0;

    return id;
  }

  static int CheckNamespaces(pid_t target_pid, uint32_t &cflags, 
                             std::function<int(const std::string&, const int)> work_fn = {}) {
    for (int i = NS_START; i < NS_END; i++) {
      const std::string self_ns = std::string("/proc/self/ns/") + ns_descs_[i].str;
      struct stat self_stat;
      if (stat(self_ns.c_str(), &self_stat)) {
        printf("Can't get the status of %s: %m\n", self_ns.c_str());
        return 1;
      }
      const std::string target_ns = 
        "/proc/" + std::to_string(target_pid) + "/ns/" + ns_descs_[i].str;
      struct stat target_stat;
      if (stat(target_ns.c_str(), &target_stat)) {
        printf("Can't get the status of %s: %m\n", target_ns.c_str());
        return 1;
      }
      if (target_stat.st_ino != self_stat.st_ino) {
        if (work_fn) {
          int ret = work_fn(target_ns, i);
          if (ret != 0) return ret;
        }
        cflags |= ns_descs_[i].cflag;
      }
    }
    return 0;
  }

  /* associate current thread with target process's net, mount and pid namespace. */
  static int EnterNamespaces(pid_t target_pid, uint32_t *ns_cflags = nullptr) {
    bool need_fork = false;
    uint32_t cflags = 0;
    int rc = CheckNamespaces(target_pid, cflags, 
      [&need_fork](const std::string &target_ns, const int descs_index) -> int {
        printf("Joining %s ns %s\n", ns_descs_[descs_index].str, target_ns.c_str());
        int ret = OpenLinkAndSetns(target_ns);
        if (ret != 0) return ret;
        if (ns_descs_[descs_index].need_fork) need_fork = true;
        return 0;
    });

    if (rc) return rc;

    if (ns_cflags) *ns_cflags = cflags;

    if (need_fork) {
      /* spawn process in target namespace */
      pid_t pid = fork();
      if (pid < 0) {
        printf("Fork failed when joining namespace: %m\n");
        return 1;
      }

      if (pid > 0) {
        int status = 0;
        rc = waitpid(pid, &status, 0);
        if (rc != pid) {
          printf("Failed to wait child process to exit in target namespace: %m");
          exit(3);
        }
        exit(CheckChildStatus(status));
      }
    }
    return 0;
  }

  static int SetnsWrapper(pid_t target_host_pid) {
    const std::string ns_link = "/proc/" + std::to_string(target_host_pid) + "/ns/";
    const std::string net_ns_link = ns_link + "net";
    const std::string pid_ns_link = ns_link + "pid";
    const std::string mnt_ns_link = ns_link + "mnt";

    int ret = OpenLinkAndSetns(net_ns_link);
    if (ret) return ret;

    ret = OpenLinkAndSetns(pid_ns_link);
    if (ret) return ret;

    ret = OpenLinkAndSetns(mnt_ns_link);
    return ret;
  }

  /* get a random procees's pid in host namepsace that running inside the target pid namespace.
   * This pid will be used to enter the target namespace. */
  static pid_t GetRandomPidInHostByNsId(uint32_t ns_id) {
    DIR *dir = opendir("/proc");
    if (dir == nullptr) {
      printf("Failed to open dir %s, %m\n", "/proc");
      return 0;
    }

    struct dirent *de;
    pid_t tmppid;
    pid_t res_pid = 0;
    while ((de = readdir(dir))) {
      if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
        continue;

      if (sscanf(de->d_name, "%d", &tmppid) == 0)
        continue;

      char buf[64];
      int nread = readlink((std::string("/proc/") + de->d_name + "/ns/pid").c_str(),
          buf, sizeof(buf));
      if (nread < 0) continue;

      uint32_t id;
      sscanf(buf, "pid:[%d]", &id);

      if (id != ns_id) continue;
      res_pid = tmppid;
      break;
    }
    closedir(dir);
    return res_pid;
  }

  /* do the work inside giving pid namespace without affect current process */
  static int GetInfoInNamepsace(pid_t pid_for_ns, char *buf, size_t len,
                                std::function<int(char *, size_t)> work_fn) {
    constexpr const char *kFailed = "failed";
    assert(len > strlen(kFailed));
    /* make a pipe to return instance command */
    int fds[2];
    int rc = pipe2(fds, O_DIRECT | O_CLOEXEC);
    if (rc != 0) {
      printf("Failed to create pipe: %m\n");
      return 1;
    }

    /* need to fork twice. Otherwise, parent process will be detached from mount namespace and
     * no longer able to read /proc/<pid>/... */
    pid_t child_pid = fork();
    if (child_pid < 0) {
      printf("Failed to fork: %m\n");
      return 1;
    }

    if (child_pid > 0) {
      /* close write end of pipe */
      close(fds[1]);

      int ret = 0;
      /* read from pipe */
      rc = read(fds[0], buf, len);

      if (rc < 0 || rc > len) {
        printf("Failed to read command of pid %d inside PID namespace: rc %d\n", pid_for_ns, rc);
        ret = 1;
      }
      close(fds[0]);

      if (strncmp(kFailed, buf, strlen(kFailed)) == 0) {
        printf("Work function failed\n");
        ret = 2;
      }

      int status = 0;
      rc = waitpid(child_pid, &status, 0);
      if (rc != child_pid) {
        printf("Failed to wait child process to exit in target namespace: %m\n");
        return 3;
      }
      
      return ret ? ret : CheckChildStatus(status);
    }

    /* child close read end of pipe */
    close(fds[0]);

    rc = EnterNamespaces(pid_for_ns);
    if (rc) {
      close(fds[1]);
      exit(1);
    }

    char tmp_buf[len];
    if (work_fn(tmp_buf, sizeof(tmp_buf)) != 0) {
      write(fds[1], kFailed, strlen(kFailed));
    } else {
      write(fds[1], tmp_buf, sizeof(tmp_buf));
    }

    close(fds[1]);
    exit(0);
  }

  static bool NeedSaveTty(uint32_t ns_cflags) {
    return ns_cflags & CLONE_NEWNS;
  }

 private:
  static int CheckChildStatus(int status) {
    if (WIFEXITED(status)) {
      int rc = WEXITSTATUS(status);
      printf("Command in namespace exit code %d\n", rc);
      if (rc) return rc;
    } else if (WIFSIGNALED(status)) {
      printf("Signal %d\n", WTERMSIG(status));
      return 3;
    } else if (WIFSTOPPED(status)) {
      printf("Stopped %d\n", WSTOPSIG(status));
      return 3;
    } else {
      printf("Status 0x%x\n", status);
      return 3;
    }
    return 0;
  }

  static int OpenLinkAndSetns(const std::string &ns_link) {
    int ns_fd = open(ns_link.c_str(), O_RDONLY | O_CLOEXEC);
    if (ns_fd < 0 || setns(ns_fd, 0)) {
      printf("Failed to enter namespace %s: %m\n", ns_link.c_str());
      if (ns_fd >= 0) close(ns_fd);
      return 1;
    }
    close(ns_fd);
    return 0;
  }

  /*
   * Enter the network namespace specified in ns_name_. This can be used with RunPID
   * to have the  PID namespace and network namespace combo.
   *   - Enter the existing network namespace.
   *   - Mount the root "/" directory recursively.
   *   - Mount /sys for accesing network stack info.
   *   - Mount network namespace specific configure to /etc
   */
  int EnterNetNs() {
    std::string path = "/var/run/netns/" + std::string(ns_name_);

    int nsfd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (nsfd < 0) {
      printf("Open file %s failed: %s\n", path.c_str(), strerror(errno));
      return 1;
    }

    int rc = setns(nsfd, CLONE_NEWNET);
    if (rc < 0) {
      printf("Associate with network namespace %s failed: %s\n", ns_name_, strerror(errno));
      close(nsfd);
      return 1;
    }

    close(nsfd);

    rc = unshare(CLONE_NEWNS);
    if (rc < 0) {
      printf("Unshare failed: %s\n", strerror(errno));
      return 1;
    }

    /* Any new mounts will not show in the parent space */
    rc = mount("", "/", "none", MS_SLAVE | MS_REC, nullptr);
    if (rc < 0) {
       printf("Mount root directory failed: %s\n", strerror(errno));
       return 2;
    }

    rc = umount2("/sys", MNT_DETACH);
    if (rc < 0) {
      /* Log the error and continue. The error could be the dir busy or not mounted. */
      printf("Lazy umount /sys failed: %s\n", strerror(errno));
    }

    /* Sys info for net namespace net stack */
    rc = mount(ns_name_, "/sys", "sysfs", 0, nullptr);
    if (rc < 0) {
      printf("Mount /sys failed: %s\n", strerror(errno));
      return 2;
    }

    /*
     * Try to mount configuration in /etc/netns/<ns-name>/ to /etc for the applications that
     * are aware of network namespace. If such netns directory does not exist, the application
     * would use the files under /etc.
     */
    path = "/etc/netns/" + std::string(ns_name_);
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
      /* It is ok to return 0 if opendir fails. */
      return 0;
    }

    struct dirent *de;
    std::string etcPath;
    while ((de = readdir(dir)) != nullptr) {
      if (strcmp(de->d_name, ".") == 0 ||
          strcmp(de->d_name, "..") == 0)
        continue;

      path = "/etc/netns/" + std::string(de->d_name);
      etcPath = "/etc/" + std::string(de->d_name);
      rc = mount(path.c_str(), etcPath.c_str(), "none", MS_BIND, nullptr);
      if (rc < 0) {
        printf("Bind mount %s to %s failed: %s\n",
                     path.c_str(), etcPath.c_str(), strerror(errno));
        closedir(dir);
        return 2;
      }
    }

    closedir(dir);

    return 0;
  }

  /*
   * Run a command in a namespace. The workflow is  
   *  1. Create new PID&MNT namespace
   *  2. Spawn "init" process
   *  3. Mount new root "/" and mount /proc in namespace
   *  4. Execute the given command. Get status for the child process.
   *  5. Only parent returns, child process always exec&exit
   */
  int ExecuteInNewPidNs(const char *pidns_path) {
    int rc = unshare(CLONE_NEWNS | CLONE_NEWPID);
    if (rc != 0) {
      printf("Unshare failed: %s\n", strerror(errno));
      return 1;
    }

    /* spawn init process */
    pid_t pid = fork();
    if (pid < 0) {
      printf("Fork failed: %s\n", strerror(errno));
      return 1;
    }

    if (pid > 0) {
      int status = 0;
      if (waitpid(pid, &status, 0) != pid) return 2;
      /* Parent waits child and returns */
      return CheckChildStatus(status);
    }

    rc = mount("", "/", "", MS_SLAVE | MS_REC, nullptr);
    if (rc != 0) {
      printf("Change root filesystem propagation failed: %m\n");
      _exit(2);
    }

    rc = mount("proc", "/proc", "proc", 0, nullptr);
    if (rc != 0) {
      printf("Mount proc directory failed: %s\n", strerror(errno));
      _exit(2);
    }

    /* Child executes the command and always exits */
    _exit(Execute(pidns_path));
  }

  int Execute(const char *pidns_path) {
    if (pidns_path != nullptr) {
      /* get PID namespace ID in child and write it into `pidns_path` file. */
      uint32_t pidns_id = GetNamespacePID();
      if (pidns_id == 0) {
        printf("Failed to get PID namespace ID: %m\n");
        return 1;
      }

      int pidns_fd = creat(pidns_path, S_IRUSR | S_IWUSR);
      if (-1 == pidns_fd) {
        printf("Unable to open pidns file %s: %m\n", pidns_path);
        return 1;
      }
      ssize_t wc = write(pidns_fd, &pidns_id, sizeof(pidns_id));
      close(pidns_fd);
      if (wc != sizeof(pidns_id)) {
        printf("Failed to write PID namespace ID: %m\n");
        return 1;
      }
    }

    execv(argv_[0], const_cast<char **>(argv_.data()));
    printf("execv failed: %m\n");
    return 1;
  }

  std::vector<const char *> argv_;
  pid_t target_ns_;
  const char *ns_name_;
  bool clone_;
};
