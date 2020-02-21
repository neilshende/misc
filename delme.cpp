#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <set>

int isprime(long long int n) {
   if (n<0) return 0;
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

int isAbundant(long long n) {

int long long i;
long long sum=0;
for (i=1; i<=n/2; i++) {
   if (n%i==0) sum += i;
}
return (sum > n);
}

int isPerfect(long long n) {

int long long i;
long long sum=0;
for (i=1; i<=n/2; i++) {
   if (n%i==0) sum += i;
}
return (sum == n);
}

std::set<long long> A;

int main() {
long long i, j, k, x, y, z, sum=0;

for (i=1; i<=28123; i++) {
if (isAbundant(i)) A.insert(i);
}

std::cout << "size of A is " << A.size() << std::endl;
for (i=1; i<=28123; i++) {
int ignore = 0;
   for (auto ja=A.begin(); ja != A.end(); ja++) {
      if ( *ja >= i) break;
      if (A.find(i-*ja) != A.end()) { ignore = 1; break; }   
   }	
   if (!ignore) sum+=i;
}
printf("%lld\n", sum);
return 0;
}


int Q(int a, int b) {
int i;
   for (i=0; i<10000; i++) {
      if (isprime(i*i+a*i+b)) {
         continue;
      } else {
         return (i>0? i-1 : 0);
      }
   }
   return 10000;
}
int maini99()
{
int d=0;
int a,b,c;
int p;
int maxp;
int maxd=0;
for(p=1000; p>500; p--){
for(a=1; a<500; a++)
  for(b=1; b<500; b++)
     if ((p-a-b)>0 && (a*a+b*b==(p-a-b)*(p-a-b))) d++;
if (maxd<d) {maxp=p; maxd=d;}
d=0;
}
printf("%d %d\n",maxp, maxd);
return 0;
}
int main27()
{
int a,b;
int sa=0, sb=0, sab=0;
int mr=0, r=0;
for (a=-999; a<=999; a++) {
   for (b=-1000; b<=1000; b++) {
      r = Q(a,b);
      printf("%d %d %d %d\n",r, a, b,a*b);
      if (mr<r) {
          mr=r;
          sa=a;
          sb=b;
          sab=a*b;
      }
   }
}
   printf("%d %d %d %d\n", mr, sa, sb, sab);
   return 0;

}
int main1()
{
long long num=600851475143;
long long p, i, j, max=0;
long long s1=0, s2=0;
long long sum=0;

for (i=6, j=3, sum=10; i<2000000; i++) {
   if (isprime(i)){
      j++;
      sum+=i;
//      if (j== 10001) {
//      printf("10k1 prime is %lld\n", i);
//      break;
//     }
   }
}
printf("%lld\n",sum);
return 0;
}
