%option noyywrap
%{
#include <cstdlib>
#include <string>
#include "y.tab.h"
%}
%%
[[:alpha:]]+     yylval.s = new std::string(yytext);  return VAR;
[[:digit:]]+     yylval.i = strtol(yytext, NULL, 10); return INT;
\[               return BEGINN;
\]               return ENDD;
.|\n                          ; /* ignore all the rest */
%%
