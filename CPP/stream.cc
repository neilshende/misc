#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

int main() {

std::vector<int> pids;
    pids.push_back(1);
    pids.push_back(2);
    std::ostringstream oss;
    std::copy(pids.begin(), pids.end(),
              std::ostream_iterator<int>(oss, ","));
    std::string pids_str = oss.str();
    pids_str.pop_back();

    char req[1024];
    int len = snprintf(req, sizeof(req), "req=[%s]",
                        pids_str.c_str()
              );  
    printf("%s\n",req);

return 0;
}
