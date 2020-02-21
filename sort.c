#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
void insertionSort1(int ar_size, int *  ar) {
    int i,j,k,elem;
    for (i=ar_size-1; i>0; i--) {
       for(j=i-1, elem=ar[i]; j>=0; j--) {
           if (elem<ar[j]) {ar[j+1]=ar[j]; ar[j]=elem;} else {ar[j+1] = elem; break;}
           for (k=0; k<ar_size; k++) {printf("%d ", ar[k]);}
           printf("\n");
           
       }
    }
}

void printarr(int ar_size, int *ar) {
   for (int k=0; k<ar_size; k++) {printf("%d ", ar[k]);}
   printf("\n");
}
void insertionSort(int ar_size, int *  ar) {
    int i,j,k,elem;
    for (i=1; i<ar_size; i++) {
        for (j=0; j<i; j++) {
            if (ar[i]<ar[j]) {
                for (k=i, elem=ar[i]; k>j; k--) {ar[k]=ar[k-1];}
                ar[j]=elem;
            }
        }
                        printarr(ar_size, ar);
    }
}
int a[] ={10, 11, 12, 1, 2, 4, 5, 13, 14, 7, 8, 15, 16};
int
compare_func(const void *l, const void *r)
{
    int *ll = (int *)l;
    int *rr = (int *)r;
    return (*ll-*rr);
}
void swap(int *a, int *b)
{
   int temp = *a;
   *a = *b;
   *b = temp;
}
void mysort(int data[], int left, int right) 
{
   int pivot = data[(left+right)/2];
   int i = left;
   int j = right;
   while (i <= j) {
      // find leftmost value >= pivot
      while (data[i] < pivot) i++;
      // find rightmost value <= pivot
      while (data[j] > pivot) j--;
      // swap if wrong order
      if (i <= j) {
         swap(&data[i], &data[j]);
         i++;
         j--;
     }
   }
   if (left < j) mysort(data, left, j);
   if (i < right) mysort(data, i , right);
}
int main (int argc, char *argv[])
{
//    qsort(a, sizeof(a)/sizeof(a[0]), sizeof(a[0]), compare_func);
    mysort(a, 0, sizeof(a)/sizeof(a[0]) -1);
    for (int i=0; i< sizeof(a)/sizeof(a[0]); i++)
        printf("%d ", a[i]);
    printf("\n");
    return 0;
}


