#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <limits>
using namespace std;
bool isLittleEndian() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 0x04;
}

bool isPowerOfTwo(unsigned int n) {
    return (n != 0) && ((n & (n - 1)) == 0);
}

int countSetBits(unsigned int n) {
   int b = 0;
   while(n) {
      n &= n-1;
      ++b;
   }
   return b;
}

void helper(string &encoded, char *buf, int &i, int &j, int &n) {
#if 0
        encoded += ((char *)(&n))[0];
        encoded += ((char *)(&n))[1];
        encoded += ((char *)(&n))[2];
        encoded += ((char *)(&n))[3];
#else
        encoded += to_string(n);
#endif
        encoded += buf[i];
        i=j;
        ++j;
        n=1;
}

string rl32(char *buf, int len) {
int i=0;
int j=1;
int n=1;
string encoded = "";
#if 0
int max = std::numeric_limits<int>::max();
#else
int max = 9;
#endif

while(j < len) {
  if (buf[i] == buf[j]) {
     if (n < max) {
        ++n;
        ++j;
     } else {
        helper(encoded, buf, i, j, n);
     }
  } else {
     helper(encoded, buf, i, j, n);
  }
}
helper(encoded, buf, i, j, n);
return encoded;
}

int main() {
char foo[] = "aaaaabbbbbbbbbbbbbbcccdefgggggggh";
cout << foo << endl;
cout << rl32(foo, strlen(foo)) << endl;
cout << std::numeric_limits<int>::max() <<endl;
cout << std::boolalpha << isLittleEndian() << endl;
return 0;
}
