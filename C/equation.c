#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#define bool _Bool
#define true 1
#define false 0

int generate_equation(int a, int b, char *buf, int buflen) {

   int offset = 0;
   int sum = 0;
   int rc;
   //RANGECHECK(buf, buflen);
   if (buf == NULL) return -2;
   if (buflen <= 0) return -3;
   if (a>b) return -4;

   bool first = true;
   for (int i = a; i <= b; i++) {
      if (i%3 == 0 || i%5 == 0) {
         if (buflen <= offset) {
             return -1;
         }
         if (first) {
            first = false;
            //print it.
            offset += rc = snprintf(buf+offset, buflen-offset, "%d", i);
            if (rc < 0) return -1;
         } else {
            // print + and number
            offset += rc = snprintf(buf+offset, buflen-offset, "+%d", i);
            if (rc < 0) return -1;
         }
         sum += i;
      }
   }
   if (buflen <= offset) {
      return -1;
   }
   if (sum > 0) {
      offset += rc = snprintf(buf+offset, buflen-offset, "=%d", sum);
      if (rc < 0) return -1;
   }
   printf("debug [%d,%d] %s\n", offset, buflen, buf);
   if (buflen <= offset) {
      return -1;
   }
   return 0;
}

int main(int argc, char *argv[]) {
    char buf[1024];
    int err =0;
    memset(buf, 0, 1024);
    err = generate_equation(1, 99, buf, 1024);
    assert(err==0);
    printf("%s\n", buf);

    memset(buf, 0, 1024);
    int a = 1;
    int b = 999;
    err = generate_equation(a, b, buf, 1024);
    assert(err==-1);
    printf("expected Error in generating equation %d %d %d\n", err, a , b);

    memset(buf, 0, 1024);
    a = 10;
    b = 13;
    err = generate_equation(a, b, buf, 9);
    assert(err==0);
    assert(strcmp(buf, "10+12=22") == 0);
    printf("%s\n", buf);

    memset(buf, 0, 1024);
    a = 10;
    b = 13;
    err = generate_equation(a, b, buf, 8);
    assert(err==-1);
    printf("expected Error in generating equation %d %d %d\n", err, a, b);

    return 0;
}
