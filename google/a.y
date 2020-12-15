%{
#include <iostream>
#include <string>
#include <map>
static std::map<std::string, int> vars;
%}
 
%union { int i; std::string *s; }
 
%token<i> INT
%token<s> VAR
%token<s> BEGINN
%token<s> ENDD
%type<s> expr
/*
%type<s> list */
 
 
%%
/*
list: expr   {$$ = $1; }
    | list expr {$$ = $1 + $2; }
    ; 
*/ 
/*expr: VAR               { $$ = $1; }*/
expr : INT BEGINN VAR ENDD              { $$ = new std::string($3); for (int ii=0; ii < $1-2; ii++) $$ +=$3; delete $3;}
     | INT BEGINN expr ENDD      { $$ = new std::string($3); for (int ii=0; ii < $1-2; ii++) $$ +=$3; delete $3 ;}
/*    | INT BEGINN VAR  expr ENDD { $$ = new std::string($3); $$ += $4; for (int ii=0; ii < $1-2; ii++) $$ +=$3+$4; delete $3; delete $4 ;}
    | INT BEGINN expr VAR ENDD { $$ = new std::string($3); $$ += $4; for (int ii=0; ii < $1-2; ii++) $$ +=$3+$4; delete $3; delete $4 ;}
*/
    ;
 
%%
extern int yylex();
extern int yyparse();
void yyerror(char *s) { std::cout << s << std::endl; }
int main() { yyparse(); }

