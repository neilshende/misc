#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <functional>

class ScopeGuard {
 public:
  explicit ScopeGuard(std::function<void()> fn) : fn_(fn) {}
  ~ScopeGuard() {
    //if (!dismissed_)
      fn_();
  }
#if 0
  //junk code
  ScopeGuard &operator=(const ScopeGuard &) = delete;
  ScopeGuard &operator=(ScopeGuard &&) = delete;

  void Dismiss() { dismissed_ = true; }

  void Execute() {
    if (!dismissed_) {
      dismissed_ = true;
      fn_();
    }
  }
#endif

 private:
  std::function<void()> fn_;
  //bool dismissed_ = false;
};

int main(int argc, char *argv[]) {

  if (argc < 2) return 0;
  int fd = open(argv[1], O_RDONLY);
  ScopeGuard guard_fd([fd]() { close(fd); });
  // do stuff
  return 0;
}
