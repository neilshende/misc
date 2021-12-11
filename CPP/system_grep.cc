#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>
int main(int argc, char *argv[]) {
std::string log_file_;
if (argc > 1 ) {
   log_file_ = argv[1];
} else {
   log_file_ = "test";
}
std::string cmd = std::string("tail -n 10 ") + log_file_ +
   " | grep -e 'Pid.*do not match expected' -e 'Unable to create a thread' >/dev/null 2>&1";

if (std::system(cmd.c_str())) {
   std::cout << "Not found\n";
} else {
   std::cout << "Found\n";
}
return 0;
}
