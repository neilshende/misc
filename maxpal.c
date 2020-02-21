#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <string.h>

int isprime(long long n) {
   for (long long i=2; i<=sqrt(n); i++) {
       if (n%i==0) return 0;
   }
   return 1;
}
int ispalindrome(long long n) {
char s[100];
memset(s, 0, 100);
sprintf(s, "%lld", n);
for (int i=0; i<strlen(s)/2; i++) {
     if (s[i]!=s[strlen(s)-i-1]) return 0;
}
return 1;
}
int main()
{
long long num=600851475143;
long long p, i, j, max=0;
int sum=0;

for (i=100; i<=999; i++) {
    for (j=100; j<=999; j++) {
       p=i*j;
       if ((ispalindrome(p)) && p > max) max=p;
    }
}
printf("%lld\n", max);
}
