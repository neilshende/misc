#include <stdio.h>
#include <stdlib.h>
int main(void)
{
int price[] = { 5, 10, 11, 6 , 8 , 9 , 10, 5, 7 };
int lowest=100;
int best_deal=0;
int i;
for (i=0; i<sizeof(price)/sizeof(price[0]); i++) {
   if (price[i]-lowest > best_deal) best_deal = price[i]-lowest;
   if (lowest>price[i]) lowest=price[i];
}
printf("Lowest=%d, best_deal=%d.\n", lowest, best_deal);
return 0;
}
