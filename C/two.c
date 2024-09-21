#include <stdio.h>
#include <math.h>
int main () {
int n=1;
double two = 2.0;
double b = pow(two, 1/two);
double i = b;
again:
double a = pow(b, i);
printf("In %d steps, answer is %75.75f\n",n, a);
if (a != two /*&& n<50*/) {
    n++;
    i = a;
    goto again;
}
return 0;
}
