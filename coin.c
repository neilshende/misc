#include <stdio.h>
#include <stdlib.h>
int main()
{
int i100, i50, i20, i10, i5, i2, i1;
long long n=0;

for (i100=0; i100<=2; i100++)
for (i50=0; i50<=4; i50++)
for (i20=0; i20<=10; i20++)
for (i10=0; i10<21; i10++)
for (i5=0; i5<41; i5++)
for (i2=0; i2<101; i2++)
for (i1=0; i1<201; i1++)
{
if (100*i100+50*i50+20*i20+10*i10+5*i5+2*i2+i1 > 200) break;
if (100*i100+50*i50+20*i20+10*i10+5*i5+2*i2+i1 == 200) n++;
}
printf("%lld\n", n+1);
return 0;
}
