/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
#define SEL_TOKEN 258
#define SEL_ID 259
#define SEL_AS 260
#define SEL_AVG 261
#define SEL_MAX 262
#define SEL_MIN 263
#define SEL_SUM 264
#define SEL_GROUP_CONCAT 265
#define SEL_GROUPBY 266
#define SEL_COUNT 267
#define SEL_WEIGHT 268
#define SEL_DISTINCT 269
#define SEL_OPTION 270
#define SEL_COMMENT_OPEN 271
#define SEL_COMMENT_CLOSE 272
#define TOK_DIV 273
#define TOK_MOD 274
#define TOK_NEG 275
#define TOK_LTE 276
#define TOK_GTE 277
#define TOK_EQ 278
#define TOK_NE 279
#define TOK_CONST_STRING 280
#define TOK_OR 281
#define TOK_AND 282
#define TOK_NOT 283
#define TOK_IS 284
#define TOK_NULL 285
#define TOK_FOR 286
#define TOK_FUNC_IN 287
#define TOK_FUNC_RAND 288




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





