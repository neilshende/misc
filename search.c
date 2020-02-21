#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
int main(int argc, char *argv[])
{
int wc = 0;
int s = 0;
char x[] = "this is a test of quick brown fox jumping over the lazy goat";
char y[] = "my fix";

int hash[256];

int xl=strlen(x);
int yl=strlen(y);

int i,j;
i =0;

memset(hash, 0, sizeof(hash));

for (j=0; j<yl; j++) {
  if (hash[(int)y[j]] > 0 ) {
     hash[(int)y[j]]--;
     continue;
  }
  for(; i<xl; i++) {
     ++hash[(int)x[i]];
     if (x[i] == y[j]) { 
       hash[(int)x[i]]--;
       break;
     }
  }
  if (i==xl) {
     printf("Can not build.\n");
     return 0;
  }
}
if (j==yl) {
  printf("possible.\n");
}
return 0;
}   
