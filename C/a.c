#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>
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
char s[] ="/this/is/a/test/foo";
char *q = std::strrchr(s, '/');
char cwd[] = "../../bar";
char *p = q; 
*q = '\0';
char *y = strtok(cwd, "/");
while (y != NULL ) {printf("%s  ", y); y= strtok(NULL, "/");}
printf("%s\n",s);
return 0;
}
