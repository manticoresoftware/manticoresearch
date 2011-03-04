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
     TOK_ATIDENT = 259,
     TOK_CONST_INT = 260,
     TOK_CONST_FLOAT = 261,
     TOK_QUOTED_STRING = 262,
     TOK_AS = 263,
     TOK_ASC = 264,
     TOK_AVG = 265,
     TOK_BEGIN = 266,
     TOK_BETWEEN = 267,
     TOK_BY = 268,
     TOK_CALL = 269,
     TOK_COLLATION = 270,
     TOK_COMMIT = 271,
     TOK_COUNT = 272,
     TOK_CREATE = 273,
     TOK_DELETE = 274,
     TOK_DESC = 275,
     TOK_DESCRIBE = 276,
     TOK_DISTINCT = 277,
     TOK_DIV = 278,
     TOK_DROP = 279,
     TOK_FALSE = 280,
     TOK_FLOAT = 281,
     TOK_FROM = 282,
     TOK_FUNCTION = 283,
     TOK_GLOBAL = 284,
     TOK_GROUP = 285,
     TOK_ID = 286,
     TOK_IN = 287,
     TOK_INSERT = 288,
     TOK_INT = 289,
     TOK_INTO = 290,
     TOK_LIMIT = 291,
     TOK_MATCH = 292,
     TOK_MATCH_WEIGHT = 293,
     TOK_MAX = 294,
     TOK_META = 295,
     TOK_MIN = 296,
     TOK_MOD = 297,
     TOK_NULL = 298,
     TOK_OPTION = 299,
     TOK_ORDER = 300,
     TOK_REPLACE = 301,
     TOK_RETURNS = 302,
     TOK_ROLLBACK = 303,
     TOK_SELECT = 304,
     TOK_SET = 305,
     TOK_SHOW = 306,
     TOK_SONAME = 307,
     TOK_START = 308,
     TOK_STATUS = 309,
     TOK_SUM = 310,
     TOK_TABLES = 311,
     TOK_TRANSACTION = 312,
     TOK_TRUE = 313,
     TOK_UPDATE = 314,
     TOK_USERVAR = 315,
     TOK_VALUES = 316,
     TOK_VARIABLES = 317,
     TOK_WARNINGS = 318,
     TOK_WEIGHT = 319,
     TOK_WHERE = 320,
     TOK_WITHIN = 321,
     TOK_OR = 322,
     TOK_AND = 323,
     TOK_NE = 324,
     TOK_GTE = 325,
     TOK_LTE = 326,
     TOK_NOT = 327,
     TOK_NEG = 328
   };
#endif
#define TOK_IDENT 258
#define TOK_ATIDENT 259
#define TOK_CONST_INT 260
#define TOK_CONST_FLOAT 261
#define TOK_QUOTED_STRING 262
#define TOK_AS 263
#define TOK_ASC 264
#define TOK_AVG 265
#define TOK_BEGIN 266
#define TOK_BETWEEN 267
#define TOK_BY 268
#define TOK_CALL 269
#define TOK_COLLATION 270
#define TOK_COMMIT 271
#define TOK_COUNT 272
#define TOK_CREATE 273
#define TOK_DELETE 274
#define TOK_DESC 275
#define TOK_DESCRIBE 276
#define TOK_DISTINCT 277
#define TOK_DIV 278
#define TOK_DROP 279
#define TOK_FALSE 280
#define TOK_FLOAT 281
#define TOK_FROM 282
#define TOK_FUNCTION 283
#define TOK_GLOBAL 284
#define TOK_GROUP 285
#define TOK_ID 286
#define TOK_IN 287
#define TOK_INSERT 288
#define TOK_INT 289
#define TOK_INTO 290
#define TOK_LIMIT 291
#define TOK_MATCH 292
#define TOK_MATCH_WEIGHT 293
#define TOK_MAX 294
#define TOK_META 295
#define TOK_MIN 296
#define TOK_MOD 297
#define TOK_NULL 298
#define TOK_OPTION 299
#define TOK_ORDER 300
#define TOK_REPLACE 301
#define TOK_RETURNS 302
#define TOK_ROLLBACK 303
#define TOK_SELECT 304
#define TOK_SET 305
#define TOK_SHOW 306
#define TOK_SONAME 307
#define TOK_START 308
#define TOK_STATUS 309
#define TOK_SUM 310
#define TOK_TABLES 311
#define TOK_TRANSACTION 312
#define TOK_TRUE 313
#define TOK_UPDATE 314
#define TOK_USERVAR 315
#define TOK_VALUES 316
#define TOK_VARIABLES 317
#define TOK_WARNINGS 318
#define TOK_WEIGHT 319
#define TOK_WHERE 320
#define TOK_WITHIN 321
#define TOK_OR 322
#define TOK_AND 323
#define TOK_NE 324
#define TOK_GTE 325
#define TOK_LTE 326
#define TOK_NOT 327
#define TOK_NEG 328




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





