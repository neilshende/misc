#include <stdlib.h>
#include <stdio.h>
#include <cstdio>
#include <vector>
#include <deque>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <pthread.h>
#include <mutex>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

using namespace std;
static void scandir(const string &dir) {
   struct dirent *dp;
   DIR *dfd = opendir(dir.c_str());
   if (dfd == NULL) {
      printf("Unable to opendir.\n");
      return;
   }
   while((dp = readdir(dfd)) != NULL) {
      if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
         continue;
      }
      cout << dir << "/" << dp->d_name << endl;
      if (dp->d_type == DT_UNKNOWN) {
         //TODO handle this by calling stat();
      }
      else if (dp->d_type == DT_DIR) {
         string subdir = dir + "/" +dp->d_name;
         scandir(subdir);
      }
   }
   closedir(dfd);
}
int main() {

string dir = ".";
scandir(dir);
return 0;
}
