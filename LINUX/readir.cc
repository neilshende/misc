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

using namespace std; 
int main(int argc, char *argv[]) {
   unordered_map<string, string> rev_map;
   if (argc != 3) {
       cout << argv[0] << " data_dir lookup_path" << endl;
       return 0;
   }
   string data_dir_ = argv[1];
   string lookup_path = argv[2];
   char buf[PATH_MAX];
   char *cur = getcwd(buf, sizeof(buf));
   cout << "CUR " << buf << endl;
   struct dirent *de;
   DIR *dir = opendir(data_dir_.c_str());
   chdir(data_dir_.c_str());
   while ((de = readdir(dir))) {
      if (de->d_type != DT_LNK) {
         cout << "skipping " << de->d_name << endl;
         continue;
      }
      if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
         continue;
      }
      char snapshot_dir[PATH_MAX+1];
      #if 1
      char *r = realpath(de->d_name, snapshot_dir);
      if (r == nullptr) {
        continue;
      }
      #else
      size_t l = readlink(de->d_name, snapshot_dir, PATH_MAX);
      if (l < 0) continue;
      snapshot_dir[l] = '\0';
      #endif
      cout << "ADD " << snapshot_dir << " --> " << de->d_name << endl;
      rev_map[snapshot_dir] = de->d_name;
   }
   closedir(dir);
   chdir(buf);
   cout << "GET " << lookup_path << " --> " << rev_map[lookup_path] << endl;
   return 0;
}

