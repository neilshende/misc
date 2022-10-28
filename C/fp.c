#include <stdio.h>
#include <stdlib.h>
int main()
{
long long sum, i=2, j, k, d;
while (1) {
   j = i;
   sum = 0;
   while (j) {
     k = (j/10)*10;
     d = j -k;
     sum += d*d*d*d*d;
     j = j/10;
   }
   if (sum == i) printf("%lld %lld\n", i, sum);
   i++;
}

printf("%lld %lld\n", i, sum);


return 0;
}
