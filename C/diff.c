#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
int a[] ={ 1, 2 , 3, 4, 5 ,6, 7, 8, 9};
int b[] ={10, 11, 12, 1, 2, 4, 5, 13, 14, 7, 8, 15, 16};
int main (int argc, char *argv[])
{
   int i=0;
   int j=0;
   int k=0;
   int m = sizeof(a)/sizeof(a[0]);
   int n = sizeof(b)/sizeof(a[0]);

   while (i<m) {
      k = j;
      while(k<n) {
         if (a[i] == b[k]) {
            break;
         }
         k++;
      }
      if (k==n) {
         printf("- %d\n", a[i]);
         i++;
      } else {
         while (j<k) {
            printf("+ %d\n", b[j]);
            j++;
         }
         printf("= %d\n", a[i]);
         i++;
         j++;
      }
   }
   while(j<n) {
      printf("+ %d\n", b[j]);
      j++;
   }
   return 0;
}

