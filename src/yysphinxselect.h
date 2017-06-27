/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_YYSPHINXSELECT_H_INCLUDED
# define YY_YY_YYSPHINXSELECT_H_INCLUDED
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
    SEL_TOKEN = 258,
    SEL_ID = 259,
    SEL_AS = 260,
    SEL_AVG = 261,
    SEL_MAX = 262,
    SEL_MIN = 263,
    SEL_SUM = 264,
    SEL_GROUP_CONCAT = 265,
    SEL_GROUPBY = 266,
    SEL_COUNT = 267,
    SEL_WEIGHT = 268,
    SEL_DISTINCT = 269,
    SEL_OPTION = 270,
    SEL_COMMENT_OPEN = 271,
    SEL_COMMENT_CLOSE = 272,
    TOK_DIV = 273,
    TOK_MOD = 274,
    TOK_NEG = 275,
    TOK_LTE = 276,
    TOK_GTE = 277,
    TOK_EQ = 278,
    TOK_NE = 279,
    TOK_CONST_STRING = 280,
    TOK_OR = 281,
    TOK_AND = 282,
    TOK_NOT = 283,
    TOK_IS = 284,
    TOK_NULL = 285,
    TOK_FOR = 286,
    TOK_FUNC_IN = 287,
    TOK_FUNC_RAND = 288
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (SelectParser_t * pParser);

#endif /* !YY_YY_YYSPHINXSELECT_H_INCLUDED  */
