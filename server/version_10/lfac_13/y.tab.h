/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ASSIGN = 258,
    MAIN = 259,
    TIP = 260,
    TIP_BOOL = 261,
    INT = 262,
    FLOAT = 263,
    CHAR = 264,
    BOOL = 265,
    ID = 266,
    STRING = 267,
    PRINT = 268,
    PRINT_SIMBOLURI = 269,
    PRINT_FUNCTII = 270,
    ADD = 271,
    SUB = 272,
    MULT = 273,
    DIV = 274,
    AND = 275,
    OR = 276,
    GT = 277,
    GEQ = 278,
    LT = 279,
    LEQ = 280,
    EQQ = 281,
    NEQ = 282,
    IF = 283,
    ELSE = 284,
    WHILE = 285,
    FOR = 286,
    DO = 287,
    CONST = 288,
    CLASS = 289,
    FUNCTION = 290,
    PRIVATE = 291,
    PUBLIC = 292,
    EVAL = 293,
    TYPEOF = 294
  };
#endif
/* Tokens.  */
#define ASSIGN 258
#define MAIN 259
#define TIP 260
#define TIP_BOOL 261
#define INT 262
#define FLOAT 263
#define CHAR 264
#define BOOL 265
#define ID 266
#define STRING 267
#define PRINT 268
#define PRINT_SIMBOLURI 269
#define PRINT_FUNCTII 270
#define ADD 271
#define SUB 272
#define MULT 273
#define DIV 274
#define AND 275
#define OR 276
#define GT 277
#define GEQ 278
#define LT 279
#define LEQ 280
#define EQQ 281
#define NEQ 282
#define IF 283
#define ELSE 284
#define WHILE 285
#define FOR 286
#define DO 287
#define CONST 288
#define CLASS 289
#define FUNCTION 290
#define PRIVATE 291
#define PUBLIC 292
#define EVAL 293
#define TYPEOF 294

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "tema2.y"

  int intVal;
  char* strVal;
  float floatVal;
  char charVal;
  int boolVal;

#line 143 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
