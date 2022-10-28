#include <stdlib.h>
#include <stdio.h>
#include <math.h>
int isprime(long long n) {
   for (long long i=2; i<=sqrt(n); i++) {
       if (n%i==0) return 0;
   }
   return 1;
}
int main()
{
long long num=600851475143;
long long i, max=0;

for (i=2; i<=sqrt(num); i++) {
    if (num%i==0 && (isprime(i))) max=i;
}
printf("%lld\n", max);
}
