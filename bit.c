#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
void endian()
{
int x = 1;
  unsigned char *y = (unsigned char *)&x;
  if (*y==1) {
     printf("Little Endian.\n");
  } else {
     printf("Big Endian.\n");
  }
}
int countbits(unsigned int x)
{
   int count = 0;
   while (x) {
      x &= x-1;
      count++;
   }
   return count;
}
int isset(unsigned int x, unsigned int bit)
{
   if (bit > 31) return 0;
   x = x<<(31-bit);
   x = x>>31;
   return x;
}

void setbit(unsigned int *x, unsigned int bit, int val)
{
   unsigned int y = 1<<bit;
   if (bit > 31) return;
   if (val) {
      *x |= y;
   } else {
      *x &= ~y;
   }
}
int ispoweroftwo(unsigned int x)
{
   return ((x&(x-1)) == 0);
}
int isnetmask(unsigned int x)
{
   if (x==0 || x==0xffffFFFF) return 0;
   x = ~x;
   x++;
   return x && ispoweroftwo(x);
}

int main()
{
unsigned int x = 0xffffFFFF;
setbit(&x, 15, 0);
setbit(&x, 0, 0);
printf("number is %x, bit count is %d, pow of two %d. 14 %d, 15 %d.\n", x, countbits(x), ispoweroftwo(x), isset(x, 14), isset(x, 15));
printf("netmask %d, netmask 0xff000000 %d.\n", isnetmask(x), isnetmask(0xff000000));
endian();
return 0;
} 
