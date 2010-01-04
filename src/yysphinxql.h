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
     TOK_CONST_INT = 259,
     TOK_CONST_FLOAT = 260,
     TOK_QUOTED_STRING = 261,
     TOK_AS = 262,
     TOK_ASC = 263,
     TOK_AVG = 264,
     TOK_BETWEEN = 265,
     TOK_BY = 266,
     TOK_COUNT = 267,
     TOK_DESC = 268,
     TOK_DELETE = 269,
     TOK_DISTINCT = 270,
     TOK_FROM = 271,
     TOK_GROUP = 272,
     TOK_LIMIT = 273,
     TOK_IN = 274,
     TOK_INSERT = 275,
     TOK_INTO = 276,
     TOK_ID = 277,
     TOK_MATCH = 278,
     TOK_MAX = 279,
     TOK_META = 280,
     TOK_MIN = 281,
     TOK_OPTION = 282,
     TOK_ORDER = 283,
     TOK_REPLACE = 284,
     TOK_SELECT = 285,
     TOK_SHOW = 286,
     TOK_STATUS = 287,
     TOK_SUM = 288,
     TOK_VALUES = 289,
     TOK_WARNINGS = 290,
     TOK_WEIGHT = 291,
     TOK_WITHIN = 292,
     TOK_WHERE = 293,
     TOK_SET = 294,
     TOK_COMMIT = 295,
     TOK_ROLLBACK = 296,
     TOK_START = 297,
     TOK_TRANSACTION = 298,
     TOK_BEGIN = 299,
     TOK_TRUE = 300,
     TOK_FALSE = 301,
     TOK_OR = 302,
     TOK_AND = 303,
     TOK_NOT = 304,
     TOK_NE = 305,
     TOK_GTE = 306,
     TOK_LTE = 307,
     TOK_NEG = 308
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST_INT 259
#define TOK_CONST_FLOAT 260
#define TOK_QUOTED_STRING 261
#define TOK_AS 262
#define TOK_ASC 263
#define TOK_AVG 264
#define TOK_BETWEEN 265
#define TOK_BY 266
#define TOK_COUNT 267
#define TOK_DESC 268
#define TOK_DELETE 269
#define TOK_DISTINCT 270
#define TOK_FROM 271
#define TOK_GROUP 272
#define TOK_LIMIT 273
#define TOK_IN 274
#define TOK_INSERT 275
#define TOK_INTO 276
#define TOK_ID 277
#define TOK_MATCH 278
#define TOK_MAX 279
#define TOK_META 280
#define TOK_MIN 281
#define TOK_OPTION 282
#define TOK_ORDER 283
#define TOK_REPLACE 284
#define TOK_SELECT 285
#define TOK_SHOW 286
#define TOK_STATUS 287
#define TOK_SUM 288
#define TOK_VALUES 289
#define TOK_WARNINGS 290
#define TOK_WEIGHT 291
#define TOK_WITHIN 292
#define TOK_WHERE 293
#define TOK_SET 294
#define TOK_COMMIT 295
#define TOK_ROLLBACK 296
#define TOK_START 297
#define TOK_TRANSACTION 298
#define TOK_BEGIN 299
#define TOK_TRUE 300
#define TOK_FALSE 301
#define TOK_OR 302
#define TOK_AND 303
#define TOK_NOT 304
#define TOK_NE 305
#define TOK_GTE 306
#define TOK_LTE 307
#define TOK_NEG 308




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





