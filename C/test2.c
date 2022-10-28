#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int fact(int n) {
    int i;
    int prod = 1;
    for (i = 2; i <=n ; i++) prod *= i;
    return prod;
}

void ss(int *a, int no_of_element,  long long int *sum) {
*sum = 0;
for(int no_of_subset=1;no_of_subset<=no_of_element;no_of_subset++)
{
   for(int start=0;start<=no_of_element-no_of_subset;start++)
   {
      int xor = 0;
      int index;
      if(no_of_subset==1)
         //printf(“%d\n”,a[start]);
          *sum += a[start];
      else {
         index=start+no_of_subset-1;
         for(int j=index;j<no_of_element;j++)
         {
             xor = 0;
             for(int i=start;i<index;i++)
                  //printf(“%d”,a[i]);
                  xor ^= a[i];
             //printf(“%d\n”,a[j]);
             xor ^= a[j];
             *sum += xor;
             //xor = 0;
         }
     }
   }
}
}



int main() {
  int t,i,j;
  int num1, *num2;
  long long int sum;
  int x[3] = {1, 2, 3};
  ss(x, 3, &sum);
  printf ("%lld\n", sum); 
#if 0
  scanf("%d",&t);
  for ( i = 0;i < t; i++ ) {
    sum=0;
    scanf("%d",&num1);
    num2= malloc(sizeof(int)*num1);
    for ( j = 0; j < num1; j++) {
       scanf("%d ",&num2[j]);
    }
    ss(num2, num1, &sum) ;   
    printf("%lld\n",sum);
    free(num2);
  }
#endif

  return 0;
}

