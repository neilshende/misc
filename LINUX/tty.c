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
  //close(0);
  //close(1);
  //close(2);
  //setsid();

int fd = open("/dev/tty", O_RDWR);
ioctl(fd, TIOCNOTTY, NULL);
close(fd);
   sleep(1000000);
   return 0;
}

