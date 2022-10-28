#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
int x[] = {10, 3, 13};
int y[sizeof(x)] = {0};
int used[sizeof(x)] = {0};
int xlen = 3;
int len =0;
/*
void combine(int start)
{
   int i;
   int len=strlen(y);
   int xlen = strlen(x);

   for (int i=start; i<xlen; i++) {
      y[len]= x[i];
      y[++len] = '\0';
      printf("%s \n",y);
      if (i<xlen) combine(i+1);
      y[--len] = '\0';
   }
}
*/
void permute()
{
int i;
   if (len == xlen) {
      printf("%d %d %d \n",y[0], y[1], y[2]);
      return;
   }
   for (i = 0; i <xlen; i++) {
      if (used[i]) continue;
      y[len]= x[i];
      ++len;
      used[i] = 1;
      permute();
      used[i] = 0;
      --len;
   }
}
int main(int argc, char *argv[])
{
permute();
return 0;
}

