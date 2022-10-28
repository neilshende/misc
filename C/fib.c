#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
int a,b;
int tn(int n) {
    if (n=1) return a;
    if (n=2) return b;
    int x=tn(n-1);
    int y=tn(n-2);
    return x*x+y;
}
int main() {

    /* Enter your code here. Read input from STDIN. Print output to STDOUT */ 
    
  int n;

  scanf("%d %d %d",&a, &b, &n);
  printf("%d\n", tn(n));

    return 0;
}

