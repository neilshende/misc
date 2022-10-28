#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
void swap(char *l, char* r)
{
char t= *l;
*l=*r;
*r=t;
}
void rev(char *x, int len)
{
int i;
for (i=0; i<len/2; i++) swap(&x[i], &x[len-i-1]);
}
int main(int argc, char *argv[])
{
int wc = 0;
int s = 0;
char x[] = "this is a test of quick brown fox jumping over the lazy goat";
char *p=x;
rev(x, strlen(x));
while ((p=strstr(p, " "))) {wc++; p++;}
while (wc--) {
   rev(x+s, strstr(x+s, " ")-(x+s));
   s+=strstr(x+s, " ")-x-s+1;
}
rev(x+s, strlen(x+s));
printf("%s\n", x);
return 0;
}

