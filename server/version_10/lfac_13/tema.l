%{
#include <stdio.h>
#include "y.tab.h"
#include <stdbool.h>
%}

%option noyywrap

%%
"int"|"float"|"char"|"string"                       {yylval.strVal = strdup(yytext); return TIP;}
"bool"                                              {yylval.strVal = strdup(yytext); return TIP_BOOL;}
"class"                                             {return CLASS;}
":="                                                {return ASSIGN;}
"main"                                              {return MAIN;}
[1-9][0-9]*|0                                       {yylval.intVal=atoi(yytext); return INT;}
[0-9]*"."[0-9]*                                     {yylval.floatVal=atof(yytext); return FLOAT;}
\".*\"                                              {yylval.strVal=strdup(yytext); return STRING;}
\'.\'                                               {yylval.charVal = yytext[1]; return CHAR;}
"true"                                              {yylval.boolVal = 1; return BOOL;}
"false"                                             {yylval.boolVal = 0; return BOOL;}
"const"                                             {return CONST;}
"public"                                            {return PUBLIC;}
"private"                                           {return PRIVATE;}
"function"                                          {return FUNCTION;}
"if"                                                {return IF;}
"else"                                              {return ELSE;}
"while"                                             {return WHILE;}
"do"                                                {return DO;}
"for"                                               {return FOR;}
"print"                                             {return PRINT;}
"print_simboluri"                                   {return PRINT_SIMBOLURI;}
"print_functii"                                     {return PRINT_FUNCTII;}
"Eval"                                              {return EVAL;}
"Typeof"                                            {return TYPEOF;}
[a-zA-Z][a-zA-Z0-9]*                                {yylval.strVal = strdup(yytext); return ID;}

"+"                                                 {return ADD;}
"-"                                                 {return SUB;}
"*"                                                 {return MULT;}
"/"                                                 {return DIV;}
"&&"                                                {return AND;}
"||"                                                {return OR;}

">"                                                 {return GT;}
">="                                                {return GEQ;}
"<"                                                 {return LT;}
"<="                                                {return LEQ;}
"!="                                                {return NEQ;}
"=="                                                {return EQQ;}



[ \t] ;
"/*"(.|\n)+"*/";
\n                                                  {yylineno++;}
.                                                   {return yytext[0];}
%%

 

