#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <getopt.h>
#include <linux/limits.h>
#include <unistd.h>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <queue>
#include <chrono>
#include <thread>
#include <sys/sendfile.h>
#include <random>
#include <list>
#include <grp.h>
#include <functional>       // std::function
#include <utility>          // std::move

    using std::function;
    using std::move;

    class Non_copyable
    {
    private:
        auto operator=( Non_copyable const& ) -> Non_copyable& = delete;
        Non_copyable( Non_copyable const& ) = delete;
    public:
        auto operator=( Non_copyable&& ) -> Non_copyable& = default;
        Non_copyable() = default;
        Non_copyable( Non_copyable&& ) = default;
    };

    class Scope_guard
        : public Non_copyable
    {
    private:
        function<void()>    cleanup_;

    public:
        friend
        void dismiss( Scope_guard& g ) { g.cleanup_ = []{}; }

        ~Scope_guard() { cleanup_(); }

        template< class Func >
        Scope_guard( Func const& cleanup )
            : cleanup_( cleanup )
        {}

        Scope_guard( Scope_guard&& other )
            : cleanup_( move( other.cleanup_ ) )
        { dismiss( other ); }
    };

std::string GetCommandByPid(int pid) {
   std::string cmdline;
   std::string pcl = "/proc/" + std::to_string(pid) + "/cmdline";
   std::ifstream pcls(pcl);
   if ( pcls >> cmdline ) return cmdline;
   return "";
}
void pop_helper(int pid, std::string &exp_cmdline) {
    std::string pid_cmd = GetCommandByPid(pid);
    //if (pid_cmd.compare(exp_cmdline) != 0) {
    if (strcmp(pid_cmd.c_str(), exp_cmdline.c_str())) {
      std::cout << "mismatch [" << pid_cmd << "] [" << exp_cmdline << "]\n";
      //return;
    }
    char cmd[512] = {0}; 
    snprintf(cmd, sizeof(cmd), "awk '/State:/ {print $3}' /proc/%d/status 2>/dev/null",
             (int)pid);
    FILE *cmd_out = popen(cmd, "r");
    if (cmd_out == NULL) {
      std::cout << "popen [" << cmd << "] failed\n";
      return;
    }
    Scope_guard guard_cmd_out ( [cmd_out] () { pclose(cmd_out); });
    char result[80] = {0}; 
    fgets(result, sizeof(result), cmd_out);
    if (strncmp(result, "(stopped)", 9)) {
      std::cout << "not stopped [" << result << "]\n";
      return;
    }
    pid_t pgid = getpgid(pid);
    if (pgid < 0) { 
      std::cout << "failed to get pgid " << pgid << "\n";
      return;
    }
    int rc = kill(-pgid, SIGKILL);
    if (rc) {
      std::cout << "Failed to kill " << rc << "\n";
    }
  }

int main( int argc, char *argv[] )
{
    if (argc < 2) {
      std::cout << argv[0] << " pid cmd\n";
      return 0;
    }
    int pid = atoi(argv[1]);
    std::string exp_cmd = argv[2];

    pop_helper(pid, exp_cmd);
    return 0;
}

