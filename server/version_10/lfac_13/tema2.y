%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "y.tab.h"
#include "functii.h"
extern FILE* yyin;
extern char* yytext;
extern int yylineno;
int yylex();
int yyerror(char* s);
extern char tipData[50];
extern char parametri[50];
char tipData1[50];
char tipData2[50];
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
%token <strVal>  STRING


%token PRINT PRINT_SIMBOLURI PRINT_FUNCTII
%token ADD SUB MULT DIV AND OR GT GEQ LT LEQ EQQ NEQ
%token IF ELSE WHILE FOR DO
%token CONST CLASS FUNCTION
%token PRIVATE PUBLIC 
%token EVAL TYPEOF
%type <strVal> NUMAR expresie_matematica expresie NUMAR_INT  expresie_bool NUMAR_FLOAT
%left ADD SUB 
%left GT GEQ LT LEQ EQQ NEQ 
%right MULT DIV
%left AND OR


%start program
%%

program : global main 
        | main
        ;

global : bloc_global 
       ;

bloc_global : secventa_instructiuni_global 
            ;

secventa_instructiuni_global : secventa_instructiuni_global declaratie_globala
                             | declaratie_globala
                             | secventa_instructiuni_global atribuire
                             | atribuire
                             | seclasa
                             ;             

declaratie_globala : TIP ID ';' {if(verificareLocalizare($2,"global") == 0) declarareId($1,$2,"global",0);
                              else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);}
                             }
                   | TIP ID ASSIGN NUMAR ';' {
                                             if(verificareLocalizare($2,"global") == 0) 
                                                  {    if(checkAssign($1,tipData)==1)
                                                       {
                                                            declarareIdVal($1,$2,$4,"global",0);
                                                       }
                                                       else 
                                                       {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                       } 
                                                  }
                                           else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);}
                                          }
                   | TIP ID ASSIGN STRING ';' {if(verificareLocalizare($2,"global") == 0) 
                                             {    if(checkAssign($1,"string")==1)
                                                  {
                                                       declarareIdVal($1,$2,$4,"global",0);
                                                  }
                                                  else 
                                                       {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                       }
                                             }
                                           else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);}
                                           }
                   | CONST TIP ID ';' {printf("Unei variabile declarata const trebuie sa-i fie asociata o valoare, linia %d.\n",yylineno); exit(1);}
                   | CONST TIP ID ASSIGN NUMAR ';'{if(verificareLocalizare($3,"global") == 0) 
                                                  {if(checkAssign($2,tipData)==1)
                                                       {declarareIdVal($2,$3,$5,"global",1);}
                                                   else 
                                                       {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                       }    
                                                  }
                                              else {printf("Nu poti redeclara o variabila de tip const, linia %d.\n",yylineno); exit(1);}
                                           }
                   | CONST TIP ID ASSIGN STRING ';'{if(verificareLocalizare($3,"global") == 0)
                                                  {if(checkAssign($2,"string")==1)
                                                  {
                                                       declarareIdVal($2,$3,$5,"global",1);
                                                  }
                                                  else 
                                                  {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                  }

                                                  }             
                                           else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);}
                                           }
                   | TIP ID '[' INT ']' ';'     {if(verificareLocalizare($2,"global") == 0) declarareId($1,$2,"global",0);
                                              else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);} 
                                             }
                | TIP ID '[' INT ']' '[' INT ']' ';' {if(verificareLocalizare($2,"global") == 0) declarareId($1,$2,"global",0);
                                                    else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);} 
                                                      }
                | declaratie_functie 
                ;
declaratie_functie :  TIP ID '(' TIP ID ')' '{' bloc_functie '}'  {
                                                                      if(verificaDuplicat_functie($2) == 0)
                                                                      {
                                                                           declara_functie($1, $2);
                                                                           adauga_parametru($2,$4,$5);
                                                                      }
                                                                      else
                                                                      {
                                                                           printf("[%d] O functie cu acest nume a fost declarata anterior.\n", yylineno);
                                                                           exit(69);
                                                                      }
                                                                 }
                   ;

bloc_functie : bloc_functie secventa_instructiuni_functie
             | secventa_instructiuni_functie
             ;
secventa_instructiuni_functie : TIP ID ';'
                              | TIP ID ASSIGN expresie ';'
                              ;
main : MAIN '(' ')' bloc_main 
     ; 

bloc_main : '{' secventa_instructiuni_main '}'
          ;

secventa_instructiuni_main : secventa_instructiuni_main declaratie_main 
                           | secventa_instructiuni_main atribuire 
                           | declaratie_main
                           | atribuire
                           | secventa_instructiuni_main instructiuni_control 
                           | instructiuni_control
                           | eval
                           | secventa_instructiuni_main eval
                           ;
eval : EVAL '(' ID ')' ';'
     ;


declaratie_main : TIP ID ';' {if(verificareLocalizare($2,"main") == 0) declarareId($1,$2,"main",0);
                              else if(verificareLocalizare($2,"main") == 2) redeclarareScope($1,$2,"main");
                              else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);} 
                             }
                | TIP ID ASSIGN NUMAR ';' {
                                             if(verificareLocalizare($2,"main") == 0) 
                                                  {    if(checkAssign($1,tipData)==1)
                                                       {
                                                            declarareIdVal($1,$2,$4,"main",0);
                                                       }
                                                       else 
                                                       {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                       } 
                                                  }
                                           else if(verificareLocalizare($2,"main") == 2) redeclarareScopeVal($1,tipData,$2,$4,"main");
                                           else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);}
                                          }
                | TIP ID ASSIGN STRING ';' {if(verificareLocalizare($2,"main") == 0) 
                                             {    if(checkAssign($1,"string")==1)
                                                  {
                                                       declarareIdVal($1,$2,$4,"main",0);
                                                  }
                                                  else 
                                                       {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                       }
                                             }
                                           else if(verificareLocalizare($2,"main") == 2) redeclarareScopeVal($1,tipData,$2,$4,"main");
                                           else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);}
                                           }
                | CONST TIP ID ASSIGN NUMAR ';'{if(verificareLocalizare($3,"main") == 0) 
                                                  {if(checkAssign($2,tipData)==1)
                                                       { declarareIdVal($2,$3,$5,"main",1);}
                                                   else 
                                                       {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                       }    
                                                  }
                                              else {printf("Nu poti redeclara o variabila de tip const, linia %d.\n",yylineno); exit(1);}
                                           }
                | CONST TIP ID ASSIGN STRING ';'{if(verificareLocalizare($3,"main") == 0)
                                                  {if(checkAssign($2,"string")==1)
                                                  {
                                                        declarareIdVal($2,$3,$5,"main",1);
                                                  }
                                                  else 
                                                  {
                                                            printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                                  }

                                                  }             
                                           else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);}
                                           }
                | TIP ID '[' INT ']' ';'     {if(verificareLocalizare($2,"main") == 0) declarareId($1,$2,"main",0);
                                              else if(verificareLocalizare($2,"main") == 2) redeclarareScope($1,$2,"main");
                                              else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);} 
                                             }
                | TIP ID '[' INT ']' '[' INT ']' ';' {if(verificareLocalizare($2,"main") == 0) declarareId($1,$2,"main",0);
                                                      else if(verificareLocalizare($2,"main") == 2) redeclarareScope($1,$2,"main");
                                                      else {printf("Variabila declarata la linia %d a fost declarata deja.\n",yylineno); exit(1);} 
                                        }
                | TIP ID ASSIGN typeof ';'
                | ID ASSIGN typeof ';'
                ;

typeof : TYPEOF '(' ID ')' 
       ;

atribuire : ID ASSIGN expresie ';' {    printf("[%d] Valoarea expresiei este %d.\n",yylineno,atoi($3));
                                        if(verificareLocalizare($1,"main") == 1) 
                                    {if(checkAssign(verificareTip($1,"main"),tipData)==1) //daca imi gaseste variabila declarata in main
                                        asignare($1,$3,"main");
                                        else
                                        {
                                             printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                        }
                                    }
                                    else if(verificareLocalizare($1,"main") == 2) 
                                   {
                                        if(checkAssign(verificareTip($1,"global"),tipData)==1) //daca imi gaseste variabila declarata global
                                             asignare($1,$3,"global");
                                        else
                                        {
                                             printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                        }
                                   } 
                                   else
                                   {
                                        printf("Variabila careia incercati sa-i atribuiti o valoare nu a fost declarata, linia %d",yylineno);exit(1);
                                   }
                                   }
          | ID ASSIGN STRING ';'{if(verificareLocalizare($1,"main") == 1) 
                                    {if(checkAssign(verificareTip($1,"main"),"string")==1) //daca imi gaseste variabila declarata in main
                                        asignare($1,$3,"main");
                                        else
                                        {
                                             printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                        }
                                    }
                                    else if(verificareLocalizare($1,"main") == 2) 
                                   {
                                        if(checkAssign(verificareTip($1,"global"),"string")==1) //daca imi gaseste variabila declarata global
                                             asignare($1,$3,"global");
                                        else
                                        {
                                             printf("Nu pot exista asignari intre tipuri de date diferite la linia %d.\n",yylineno); exit(1);
                                        }
                                   } 
                                   else
                                   {
                                        printf("Variabila careia incercati sa-i atribuiti o valoare nu a fost declarata, linia %d",yylineno);exit(1);
                                   }
                                   }
          ;

expresie : expresie_matematica {$$=$1;strcpy(tipData,"int");}
         | expresie_bool {$$=$1; }
         ;

expresie_matematica : expresie_matematica ADD expresie_matematica  {
char temp[32]; // temporary buffer to store the result
snprintf(temp, 32, "%d", atoi($1)+atoi($3)); // write the result to the buffer
strcpy($1, temp); // copy the result from the buffer to $1
$$=$1;
}
         | expresie_matematica SUB expresie_matematica {sprintf($1, "%d", atoi($1)-atoi($3)); $$=$1;}
         | expresie_matematica MULT expresie_matematica {sprintf($1, "%d", atoi($1)*atoi($3)); $$=$1;}
         | expresie_matematica DIV expresie_matematica {sprintf($1, "%d", atoi($1)/atoi($3)); $$=$1;}
         | '(' expresie_matematica ')' {$$ = $2;}
         | NUMAR_INT {$$ = $1;}
         | NUMAR_FLOAT {$$ = $1;}
         | ID {$$ = $1;}
        ;
//sub_expresie_mat : NUMAR ADD NUMAR {strcpy(tipData1,tipData);} {if(verificareTipExpresie(tipData2,tipData1) ==0){printf("Eroare");exit(1);}}
  //               ;


NUMAR_INT : INT {$$=itoa($1);} 
          ;
NUMAR_FLOAT : FLOAT {$$=ftoa($1);} 
          ;

NUMAR : INT {$$=itoa($1); strcpy(tipData,"int");}
      | FLOAT {$$=ftoa($1); strcpy(tipData,"float");}
      | CHAR {$$=itoa($1); strcpy(tipData,"char");}
      ;
expresie_bool : expresie AND expresie
              | expresie OR expresie
              | expresie GT expresie
              | expresie LT expresie 
              | expresie EQQ expresie 
              | expresie NEQ expresie
              | expresie GEQ expresie
              | expresie LEQ expresie 
              ;
instructiuni_control : control_if
                     | control_while
                     | control_for
                     ;
control_if: IF '(' expresie_bool ')' bloc_main
          | IF '(' expresie_bool ')' bloc_main ELSE bloc_main
          ;
control_while : WHILE '(' expresie_bool ')' DO bloc_main
              ;
control_for : FOR '(' ID ASSIGN expresie ';' expresie_bool ';' ID ASSIGN expresie ')'  {
                                                       if(eDuplicat($3) == 0)
                                                            printf("Variabila din for, la linia %d, nu a fost declarata.\n",yylineno);
     
                                                       }bloc_main
            ;

%%
int yyerror(char * s){
     printf("eroare: %s la linia:%d\n",s,yylineno);
}
int main(int argc, char** argv){
     yyin=fopen(argv[1],"r");
     yyparse();
     afisare();
     afisare_functii();
}