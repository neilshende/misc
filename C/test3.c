#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
int *x, *y, *used;
int xlen;
int len=0;
double maxmin = 0;
unsigned int min = -1;
void calc() {
    unsigned int xor;
    min = -1;
    for (int i=0; i<xlen; i++) {
        xor = x[i]^x[(i+1)%(xlen)];
        min = xor<min? xor : min;
    }
    maxmin = min>maxmin? min : maxmin;
}
void permute()
{
int i;
   if (len == xlen) {
      //printf("%d %d %d \n",y[0], y[1], y[2]);
       calc();
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

int powof2(int x) {
    return ((x&x-1)==0);
}

int main() {
    int i, ans;

    /* Enter your code here. Read input from STDIN. Print output to STDOUT */ 

    scanf("%d",&xlen);
    x = malloc(sizeof(int)*xlen);
    y = malloc(sizeof(int)*xlen);
    used = malloc(sizeof(int)*xlen);
    memset(used, 0, sizeof(int)*xlen);

    for (i = 0;i < xlen; i++ ) {
       scanf("%d",&x[i]);
    }
    permute();
    
    ans = powof2((int)maxmin)? ((int)log2(maxmin))+1 : ((int)log2(maxmin));
    
    printf("%d\n", ans);
    free(x);
    free(y);
    free(used);

    return 0;
}
