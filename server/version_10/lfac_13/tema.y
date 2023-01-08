%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
#include "functii.h"
extern FILE* yyin;
extern char* yytext;
extern int yylineno;
int yylex();
int yyerror(char* s);

%}

%union {
  int intVal;
  char* strVal;
  float floatVal;
  char charVal;
  int boolVal;
}

%token ASSIGN MAIN
%token <strVal>   TIP
%token <strVal>   TIP_BOOL
%token <intVal>   INT
%token <floatVal> FLOAT
%token <charVal>  CHAR
%token <boolVal>  BOOL
%token <strVal>   ID
%token <strVal>   STRING


%token PRINT PRINT_SIMBOLURI PRINT_FUNCTII
%token ADD SUB MULT DIV AND OR GT GEQ LT LEQ EQQ NEQ
%token IF ELSE WHILE FOR DO
%token CONST CLASS FUNCTION
%token PRIVATE PUBLIC
%type <intVal> operatie
%left ADD SUB MULT DIV
%left AND OR


%start program
%%

program : declaratii_globale main 
        | main
        ;
declaratii_globale : declaratii_globale instructiune
                   | declaratii_globale clasa
                   | clasa
                   | instructiune
;

clasa: CLASS ID '{' bloc_clasa '}' ;

bloc_clasa: instructiune_clasa
          | bloc_clasa instructiune_clasa
          ;
instructiune_clasa: vizibilitate TIP ID ';'
                  | vizibilitate functie 
                  | vizibilitate vector ';'
                  ;
vizibilitate: PUBLIC
            | PRIVATE
            ;

main : MAIN '(' ')' bloc
     ;

bloc : '{' secventa_instructiuni '}'
     ;

secventa_instructiuni : instructiune
                      | secventa_instructiuni instructiune
                      ;

instructiune : TIP ID ';'                         {
                                                  if(eDuplicat($2) == 0)
                                                       adaugaID($2, $1);
                                                  else
                                                       printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                  }                     
             | TIP ID                             {printf("Lipseste ';' la finalul instructiunii la linia %d\n",yylineno);}
             | TIP ID ASSIGN expresie ';'         {
                                                  if(eDuplicat($2) == 0)
                                                       adaugaID($2, $1);
                                                  else
                                                       printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                  } 
             | ID ASSIGN expresie ';'             {
                                                  if(eDuplicat($1) == 0)
                                                       printf("Variabila de la linia %d nu fost inca declarata.\n",yylineno);
                                                  else if(eDuplicat($1) == 1 && getConst($1) == 1)
                                                       printf("Variabila de la linia %d  nu-si poate modifica valoarea deoarece a fost declarata anterior ca si CONST.\n", yylineno);
                                                  } 
             | ID ASSIGN expresie                 {printf("Lipseste ';' la finalul instructiunii la linia %d\n",yylineno);}
             | TIP ID ASSIGN expresie             {printf("Lipseste ';' la finalul instructiunii la linia %d\n",yylineno);}
             | TIP ID ASSIGN operatie ';'
             | TIP_BOOL ID ';'                    {
                                                  if(eDuplicat($2) == 0)
                                                       adaugaID($2, $1);
                                                  else
                                                       printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                  } 
             | TIP_BOOL ID ASSIGN BOOL ';'        {
                                                  if(eDuplicat($2) == 0)
                                                       adaugaID($2, $1);
                                                  else
                                                       printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                  } 
             | TIP_BOOL ID                        {printf("Lipseste ';' la finalul instructiunii la linia %d\n",yylineno);}
             | TIP_BOOL ID ASSIGN BOOL            {printf("Lipseste ';' la finalul instructiunii la linia %d\n",yylineno);}
             | CONST TIP ID ASSIGN INT ';'        {
                                                       {
                                                       if(eDuplicat($3) == 0)
                                                            adaugaConstID_int($3, $5);
                                                       else
                                                            printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                       }
                                                  }
             | CONST TIP ID ASSIGN FLOAT ';'        {
                                                       {
                                                       if(eDuplicat($3) == 0)
                                                            adaugaConstID_float($3, $5);
                                                       else
                                                            printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                       }
                                                  }
             | CONST TIP ID ASSIGN CHAR ';'        {
                                                       {
                                                       if(eDuplicat($3) == 0)
                                                            adaugaConstID_char($3, $5);
                                                       else
                                                            printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                       }
                                                  }
             | CONST TIP ID ASSIGN STRING ';'        {
                                                       
                                                       if(eDuplicat($3) == 0)
                                                            adaugaConstID_string($3, $5);
                                                       else
                                                            printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno);
                                                       
                                                  }
             | vector
             | functie
             | PRINT_SIMBOLURI '(' ')' ';'
             | PRINT_FUNCTII   '(' ')' ';'
             | control_if
             | control_while
             | control_for
             ;
control_if: IF '(' lista_conditii ')' bloc
          | IF '(' conditie ')' bloc
          | IF '(' lista_conditii ')' bloc ELSE bloc
          ;
control_while : WHILE '(' lista_conditii ')' DO bloc
              | WHILE '(' conditie ')' DO bloc
              ;

control_for : FOR '(' ID ASSIGN expresie ';' lista_conditii ';' ID ASSIGN expresie ')'  {
                                                       if(eDuplicat($3) == 0)
                                                            printf("Variabila din for, la linia %d, nu a fost declarata.\n",yylineno);
     
                                                       }bloc
            | FOR '(' ID ASSIGN expresie ';' conditie ';' ID ASSIGN expresie ')'  {
                                                       if(eDuplicat($3) == 0)
                                                            printf("Variabila din for, la linia %d, nu a fost declarata.\n",yylineno);
     
                                                       }bloc ;

conditie :  expresie EQQ expresie
         |  expresie NEQ expresie
         |  expresie GEQ expresie
         |  expresie LEQ expresie
         |  expresie GT expresie
         |  expresie LT expresie
         |  '(' conditie ')'
         ;
         
                                                  
lista_conditii : '(' lista_conditii ')'
               | lista_conditii OR conditie
               | conditie OR conditie
               | lista_conditii AND conditie
               | conditie AND conditie
               ;

expresie : INT
         | FLOAT
         | CHAR
         | STRING 
         ;

operatie: INT                           {$$ = $1;}
        | operatie ADD operatie         {$$ = $1 + $3;}
        | operatie SUB operatie         {$$ = $1 - $3;}
        | operatie MULT operatie        {$$ = $1 * $3;}
        | operatie DIV operatie         {$$ = $1 / $3;}
        | operatie AND operatie         {$$ = $1 && $3;}
        | operatie OR operatie          {$$ = $1 || $3;}
        ;

vector : ID dimensiune
       | TIP ID dimensiune ';' 
       ;

functie : FUNCTION TIP ID '(' ')' bloc       
        | FUNCTION TIP ID '(' lista_parametri ')' bloc
        ;

lista_parametri : TIP ID 
                | lista_parametri ',' TIP ID
                | ID
                | lista_parametri ',' ID
                ;

dimensiune: '[' size ']' '[' size ']'
    | '[' size ']'
    ;

size : INT
     ;
%%
int yyerror(char * s){
     printf("eroare: %s la linia:%d\n",s,yylineno);
}
int main(int argc, char** argv){
     yyin=fopen(argv[1],"r");
     yyparse();
} 
