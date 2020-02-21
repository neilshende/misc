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

void subset(int *num, int count, int scount, long long int *sum) {
    int i, j;
    int sum1 = 0;
    int sum2 = 0;
    
    for (i=0; i<count; i++) {
        for (j=0; j<scount; j++) {
            sum1 ^= num[i];
        }
        sum2 += sum1;
    }
    *sum += sum2;
}

int main() {
  int t,i,j;
  int num1, *num2;
  long long int sum; 

  scanf("%d",&t);
  for ( i = 0;i < t; i++ ) {
    sum=0;
    scanf("%d",&num1);
    num2= malloc(sizeof(int)*num1);
    for ( j = 0; j < num1; j++) {
       scanf("%d ",&num2[i]);
       sum += num2[i];
    }
      for (i=2; i < num1; i++) {
          subset(num2, num1, i, &sum);
      }
          
    printf("%d\n",sum%(1000000007));
  }
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */    
    return 0;
}

