#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
char x[] = "0123456789";
char y[sizeof(x)] = {0};
int used[sizeof(x)] = {0};
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
void permute()
{
int i;
int len=strlen(y);
int xlen = strlen(x);
   if (len == xlen) {
      printf("%s \n",y);
      return;
   }
   for (i = 0; i <strlen(x); i++) {
      if (used[i]) continue;
      y[len]= x[i];
      y[++len] = '\0';
      used[i] = 1;
      permute();
      used[i] = 0;
      y[--len] = '\0';
   }
}
int main(int argc, char *argv[])
{
permute();
//printf("Now combine\n");
//combine(0);
return 0;
}
