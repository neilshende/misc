#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
char string[] = "This is a +test+1234+4567:wq
of my program-911The end.";
char * number(char *str) {
   char * begin = str;
   char * end;
   while (isalpha(*begin)) {
     if (*begin ==  '\0') {
        return NULL;
     }
     begin++;
     if (isdigit(*begin) || *begin == '+' || *begin == '-') {
        end = begin+1;
        if (isdigit(*begin) break;
     }
  
  }
  end = begin+1;
  while (isdigit(*end)) {
     end++;
  }
  
  if (*end != '0') {
     *end = '\0';
  }
  return begin;

}
int main() {

char *x = number(string);
printf("%s\n",x);

return 0;
}
