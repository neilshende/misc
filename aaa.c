#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
int main(int argc, char *argv[])
{
switch (argc) {
default: printf("Default %d\n", argc); break;
case 1 : printf("1\n");break;
case 2 : printf("2\n"); break;
case 3 : printf("3\n"); break;
}
return 0;
}
