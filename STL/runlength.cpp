#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <list>
using namespace std;
int encode(int currLength, const char *p, string &out)
{
   if (p == NULL) {
      out += "null";
      return 1;
   }
   if (*p == 0) {
      return 0;
   }
   if (currLength >= 4) {
      out += "err4";
      return 1; 
  }
#if 1
  if (currLength == 1) {
     if (*p != '1' && *p != '2' && *p != '3') {
        out += "terminating";
        return 1;
     }
  }
#endif
  if (*p == *(p+1)) {
      return encode(currLength+1, p+1, out);
  }
  out += to_string(currLength) + *p;
  return encode(1, p+1, out);
}
list<string> processed;
int process(string n, list<string> &processing) 
{
    bool found;
    cout << "processing " << n << endl; 
    found = (std::find(processed.begin(), processed.end(), n) != processed.end());
    if (found) {
          cout << n << " already processed" << endl;
          return 0;
    }
    found = (std::find(processing.begin(), processing.end(), n) != processing.end());
    if (found) {
        // dump processing list
        list<string>::const_iterator it;
        for ( it = processed.begin(); it != processed.end(); it++) {
            cout << *it << " ";
        }
        cout << endl;
        exit(0);
    }
    processed.push_front(n);
    processing.push_front(n);
    string out;
    int err, err2;
    const char *buff = n.c_str();
    err = encode(1, buff, out);
    if (err == 0) {
       if (out.length() > 10000) {
            cout << "Too big, terminating" << endl;
            return 0;
       }
       string next = out.c_str();
       err2 = process(next, processing);
    }
    return 0;
}
    

    
int main(int argc, char *argv[])
{
   int i=1;
   for (i=1; i<10000000; i++) {
   string ii = to_string(i);
   if (ii.find("0") != std::string::npos) continue;
   if (ii.find("4") != std::string::npos) continue;
   if (ii.find("5") != std::string::npos) continue;
   if (ii.find("6") != std::string::npos) continue;
   if (ii.find("7") != std::string::npos) continue;
   if (ii.find("8") != std::string::npos) continue;
   if (ii.find("9") != std::string::npos) continue;
   list<string> LIST;
   LIST.clear();
   process(ii, LIST);
   }
   return 0;
}
