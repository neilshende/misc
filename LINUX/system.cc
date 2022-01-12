#include <list> 
#include <unordered_map>
#include <iostream>
#include <linux/limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <cstring>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <ctype.h>
#include <sys/sendfile.h>
#include <sched.h>
#include <sys/capability.h>
#include <sys/mount.h>
#include <elf.h>
#include <regex.h>
#include <sstream> 
using namespace std; 
int main(int argc, char *argv[]) {
  //for(int i=0; i<argc; i++) std::cout << argv[i] << std::endl;
  if (argc == 2) {
    char tmp_file[PATH_MAX] = "/tmp/tarlistXXXXXX";
    int fd = mkstemp(tmp_file);
    if (fd < 0) {
     // MVMLOG("Warning: Can't open file %s during tar info gathering.\n", tmp_file);
     return 1;
    }
    close(fd);
    string tf = tmp_file;
    string find_cmd = string("find /tmp -regex '") + argv[1] + string("' -type d -print > ") + tf;
    std::cout << find_cmd << std::endl;
    string cmd = string("tar --absolute-names -cvf backup.tar -T ") + tf;
    if (system(find_cmd.c_str())) {
      std::cout << "FAILED 1" << std::endl;
      return 2;
   }
    stringstream af;
    af << "#!/bin/bash\n"
       << "if [ ${CRTOOLS_SCRIPT_ACTION} == \"post-dump\" ]; then\n"
       << "   echo executing post-dump hook\n"
       << "   tar --absolute-names -cvf backup.tar -T " << tf << "\n"
       << "fi\n";
    char action_script[PATH_MAX] = "/tmp/actionscriptXXXXXX";
    fd = mkstemp(action_script);
    if (fd < 0) {
      return 3;
    }
    int rc = write(fd, af.str().c_str(), af.str().size());
    if (rc != af.str().size()) return 4;
    close(fd);
    chmod(action_script, 0755);
     // after success of snapshot.
    remove(tmp_file);
    remove(action_script);
     //std::ofstream out("output.txt");
     //std::cout << af << std::endl;
    // if (system(cmd.c_str())) std::cout << "FAILED 2" << std::endl;
  }

  return 0;
}

