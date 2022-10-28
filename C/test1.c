#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int main() {
  int t,i,j;
  int num1,num2;
  int sum; 

  scanf("%d",&t);
  for ( i = 0;i < t; i++ ) {
    sum=0;
    scanf("%d",&num1);
    for ( j = 0; j < num1, j++) {
       scanf("%d ",&num2);
       sum ^= num2;  
    }
    printf("%d\n",sum);
  }
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */    
    return 0;
}
