#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <iostream>
std::map<int, long long> mem;
long long ways(int n) {
#if 0
if (n==0) return 0;
if (n==1) return 0;
if (n==2) return 1;
#endif
auto search = mem.find(n);
if (search != mem.end()) {
    std::cout << "found " << n << "\t" << search->second << std::endl;
    return search->second;
}
int i;
long long l, m;
long long sum =0;
for (i=1; i<n/2; i++) {
  sum += 1+ways(i);
}
if (n%2==0) {
l = ways(n/2);
sum += (l+1)*(l+1);
} else {
l=ways(n/2);
m=ways(n/2+1);
sum += (l+1)*(m+1);
}
mem[n]=sum;
    std::cout << "adding " << n << "\t" << sum << std::endl;
return sum;
}
int main() {
mem[0]=0;
mem[1]=0;
mem[2]=1;
std::cout << "reached here" << std::endl;

int i=100;
//  for (i=3;i<101;i++) {
  printf("%d %lld\n", i, ways(i));
//  }
  return 0;
}
#if 0
int maini2()
{
mem[0]=0;
mem[1]=0;
mem[2]=1;
std::cout << "reached here" << std::endl;
int i=0;
for(i=3; i<1000; i+=3) {sum +=i;}
for(i=5; i<1000; i+=5) {sum +=i;}
printf("%lld\n", sum);
return 0;
}
#endif

