%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
int c;
extern int yylval;
extern char *yytext;
extern char *mytext;
extern int number;
%}
%%

" "       ;
[a-z]*     {
            c = yytext[0];
            mytext = strdup(yytext);
            yylval = yyleng;
            return(LETTERS);
          }
[0-9]*     {
            c = yytext[0];
            sscanf(yytext, "%d", &number);
            yylval = yyleng;
            return(DIGITS);
           }
\[         {
            c = yytext[0];
            yylval = yyleng;
            return(BEGINN);
           }
\]         {
            c = yytext[0];
            yylval = yyleng;
            return(ENDD);
           }
