#include <list> 
#include <unordered_map>
#include <iostream>
#include <fstream>
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
        std::string ebs_metadata_path =  "./ebs_snap.info";
        bool vol_snap_present = access(ebs_metadata_path.c_str(), F_OK) == 0;
        if (vol_snap_present) {
          std::ifstream ebs_vol(ebs_metadata_path.c_str());
          std::string line;
          if (ebs_vol.is_open()) {
            while (getline(ebs_vol, line)) {
               char tk1[64] = {0};
               char tk2[64] = {0};
               char tk3[64] = {0};
               sscanf(line.c_str(), "%s %s %s", tk1, tk2, tk3);
               //cout << tk1 << endl;
               if (strcmp(tk1, "MountPt") == 0) cout << "mntpt= " << tk3 << endl;
               if (strcmp(tk1, "OrigVolumeId") == 0) cout << "vol= " << tk3 << endl;
            }
          }
        }
return 0;
}
