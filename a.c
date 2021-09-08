#include <stdio.h>
#include <stdlib.h>
int main() {
int count, partno;
char *name;
char *devPart="nvme10n1p1";
short int x =32767;
x = x+1;
x = x+1;
float a = 3.1415;
long double b = 3.1415160000001;
printf("X is %d\n", x);
printf("%f %010.15Lf\n",a, b);
char *s="thisisa%stest%d\n";
printf("%%%s",s);
return 0;
}
