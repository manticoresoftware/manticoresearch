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
     TOK_BETWEEN = 263,
     TOK_BY = 264,
     TOK_DESC = 265,
     TOK_FROM = 266,
     TOK_GROUP = 267,
     TOK_LIMIT = 268,
     TOK_IN = 269,
     TOK_ID = 270,
     TOK_MATCH = 271,
     TOK_ORDER = 272,
     TOK_SELECT = 273,
     TOK_SHOW = 274,
     TOK_STATUS = 275,
     TOK_WARNINGS = 276,
     TOK_WEIGHT = 277,
     TOK_WITHIN = 278,
     TOK_WHERE = 279,
     TOK_OR = 280,
     TOK_AND = 281,
     TOK_NOT = 282,
     TOK_NE = 283,
     TOK_GTE = 284,
     TOK_LTE = 285,
     TOK_NEG = 286
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST 259
#define TOK_QUOTED_STRING 260
#define TOK_AS 261
#define TOK_ASC 262
#define TOK_BETWEEN 263
#define TOK_BY 264
#define TOK_DESC 265
#define TOK_FROM 266
#define TOK_GROUP 267
#define TOK_LIMIT 268
#define TOK_IN 269
#define TOK_ID 270
#define TOK_MATCH 271
#define TOK_ORDER 272
#define TOK_SELECT 273
#define TOK_SHOW 274
#define TOK_STATUS 275
#define TOK_WARNINGS 276
#define TOK_WEIGHT 277
#define TOK_WITHIN 278
#define TOK_WHERE 279
#define TOK_OR 280
#define TOK_AND 281
#define TOK_NOT 282
#define TOK_NE 283
#define TOK_GTE 284
#define TOK_LTE 285
#define TOK_NEG 286




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





