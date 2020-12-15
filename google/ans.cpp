#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <list>
using namespace std;

char * uncompress(char *comp, int length) {
   if (comp == NULL || *comp == 0 || length == 0) return NULL;
   char * brc = strstr(comp, '[');
   if (brc == NULL) {
      return comp;
   }
   int len;
   scanf("%d", comp, &len);
   if (isalpha(brc+1)) {
      char * alphaend = brc+1;
      while (islapha(alphaend)) alphaend++;
      if (*alphaend == ']') {
         char *out = (char *)malloc(len*(alphaend-1-brc-1));
         //TODO init it
         rest = uncompress(alphaend+1, (comp+lenght)-(alphaend+1));
         //TODO return out + rest 
      } else {

      }
   } else {
   


}
int main (int argc, char *argv[]) {


}
