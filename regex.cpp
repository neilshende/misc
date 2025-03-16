#include <string.h>
#include <iostream>
using namespace std;

    int match(char sc, char pc, char pn) {
        cout << "in match " << sc << " " << pc << " " << pn << " " << endl;
        if (pn == '*') {
            return (pc == '.' || pc == sc) ? 3 : 4; 
        } else {
            return (pc == '.' || pc == sc) ? 1 : 2;
        }
        return 100;
    }
    bool isMatch(string s, string p) {
        int sn = s.size();
        int pn = p.size();
        int si = 0;
        int pi = 0;
        const char *ss = s.c_str();
        const char *ps = p.c_str();
        while (pi<pn && si<sn){
            cout << "iter " << si << " " << pi << " " << sn << " " << pn << endl;
            switch (match(ss[si], ps[pi], pi+1<pn?ps[pi+1]:'x')) {
                case 1: ++si; ++pi;
                cout << "got 1\n";
                break;
                case 2: 
                cout << "got 2\n";
                return false;
                break;
                case 3: ++si; if (si == sn) pi += 2;
                cout << "got 3\n";
                break;
                case 4: pi += 2;
                cout << "got 4\n";
                break;
            }
        }
        return (si==sn && pi==pn);
    }

int main() {
   string s = "aab";
   string p = "c*a*b";
   cout << isMatch(s, p) << endl;
   return 0;
}
