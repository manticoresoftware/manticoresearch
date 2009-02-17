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
     TOK_IDENT = 258,
     TOK_CONST = 259,
     TOK_QUOTED_STRING = 260,
     TOK_AS = 261,
     TOK_ASC = 262,
     TOK_AVG = 263,
     TOK_BETWEEN = 264,
     TOK_BY = 265,
     TOK_COUNT = 266,
     TOK_DESC = 267,
     TOK_DISTINCT = 268,
     TOK_FROM = 269,
     TOK_GROUP = 270,
     TOK_LIMIT = 271,
     TOK_IN = 272,
     TOK_ID = 273,
     TOK_MATCH = 274,
     TOK_MAX = 275,
     TOK_META = 276,
     TOK_MIN = 277,
     TOK_OPTION = 278,
     TOK_ORDER = 279,
     TOK_SELECT = 280,
     TOK_SHOW = 281,
     TOK_STATUS = 282,
     TOK_SUM = 283,
     TOK_WARNINGS = 284,
     TOK_WEIGHT = 285,
     TOK_WITHIN = 286,
     TOK_WHERE = 287,
     TOK_OR = 288,
     TOK_AND = 289,
     TOK_NOT = 290,
     TOK_NE = 291,
     TOK_GTE = 292,
     TOK_LTE = 293,
     TOK_NEG = 294
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST 259
#define TOK_QUOTED_STRING 260
#define TOK_AS 261
#define TOK_ASC 262
#define TOK_AVG 263
#define TOK_BETWEEN 264
#define TOK_BY 265
#define TOK_COUNT 266
#define TOK_DESC 267
#define TOK_DISTINCT 268
#define TOK_FROM 269
#define TOK_GROUP 270
#define TOK_LIMIT 271
#define TOK_IN 272
#define TOK_ID 273
#define TOK_MATCH 274
#define TOK_MAX 275
#define TOK_META 276
#define TOK_MIN 277
#define TOK_OPTION 278
#define TOK_ORDER 279
#define TOK_SELECT 280
#define TOK_SHOW 281
#define TOK_STATUS 282
#define TOK_SUM 283
#define TOK_WARNINGS 284
#define TOK_WEIGHT 285
#define TOK_WITHIN 286
#define TOK_WHERE 287
#define TOK_OR 288
#define TOK_AND 289
#define TOK_NOT 290
#define TOK_NE 291
#define TOK_GTE 292
#define TOK_LTE 293
#define TOK_NEG 294




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





