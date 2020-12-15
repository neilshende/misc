%{
#include <stdio.h>
#include <string.h>
char * yytext;
char * mytext;
int number;
extern int yylex();
int yyerror(char *s)
{
  fprintf(stderr, "%s\n",s);
  return 1;
}
void yywrap()
{
//  return(1);
}
%}
%start exp
%token DIGITS LETTERS BEGINN ENDD
%%
                   /* beginning of rules section */
exp: DIGITS BEGINN LETTERS ENDD
     {
         for (int i=0; i < number; i++) printf("%s", mytext);
         char *sa = mytext;
         //printf("DEBUGexp1--%d--%lu--\n",number, strlen(mytext));
         mytext=(char*)malloc(number*strlen(mytext)+1);
         for (int i=0; i < number; i++) memcpy(mytext+i*strlen(mytext),mytext, strlen(mytext));
         mytext[number*strlen(mytext)]=0;
         //printf("DEBUGexp%s\n", mytext);
         free(sa);
         
     }
     |
     LETTERS 
     {
         printf("%s", mytext);
         free(mytext);
     }
     |
     exp LETTERS
     {
         printf("DEBUGexpLetters--%s\n", mytext);
     }
/*
     DIGITS BEGINN exp ENDD
     {
        printf("Debug --%s--%d--\n", mytext,number);
        for (int i=0; i < number; i++) printf("%s", mytext);
     }
*/
     ;
%%
int main(int argc, char *argv[])

{

 return(yyparse());

}
#if 0
extern int yylex();
int yyerror(char *s)
{
  fprintf(stderr, "%s\n",s);
  return 1;
}
void yywrap()
{
//  return(1);
}
#endif
