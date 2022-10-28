#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int main() {

    /* Enter your code here. Read input from STDIN. Print output to STDOUT */  
      int t,i,j;
  int n, q;
  int *num;
    int xor, max;
    int a, pi, qi;

  scanf("%d",&t);
  for ( i = 0;i < t; i++ ) {
    scanf("%d %d",&n, &q);
    num= malloc(sizeof(int)*n);
    for ( j = 0; j < n; j++) {
       scanf("%d ",&num[j]);
    }
    for ( j = 0; j < q; j++) {
       scanf("%d %d %d",&a, &pi, &qi);
        for (i=pi; i<=qi; i++) {
            xor = a ^ num[i];
            if (i==pi) { 
               max = xor; 
            } else {
                max = max>xor? max : xor;
            }
        }
        printf("%d\n", max);

    }
    
    free(num);
  }


    return 0;
}
