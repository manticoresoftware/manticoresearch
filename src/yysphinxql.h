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
     TOK_USERVAR = 263,
     TOK_SYSVAR = 264,
     TOK_AS = 265,
     TOK_ASC = 266,
     TOK_AVG = 267,
     TOK_BEGIN = 268,
     TOK_BETWEEN = 269,
     TOK_BY = 270,
     TOK_CALL = 271,
     TOK_COLLATION = 272,
     TOK_COMMIT = 273,
     TOK_COUNT = 274,
     TOK_CREATE = 275,
     TOK_DELETE = 276,
     TOK_DESC = 277,
     TOK_DESCRIBE = 278,
     TOK_DISTINCT = 279,
     TOK_DIV = 280,
     TOK_DROP = 281,
     TOK_FALSE = 282,
     TOK_FLOAT = 283,
     TOK_FROM = 284,
     TOK_FUNCTION = 285,
     TOK_GLOBAL = 286,
     TOK_GROUP = 287,
     TOK_ID = 288,
     TOK_IN = 289,
     TOK_INSERT = 290,
     TOK_INT = 291,
     TOK_INTO = 292,
     TOK_LIMIT = 293,
     TOK_MATCH = 294,
     TOK_MAX = 295,
     TOK_META = 296,
     TOK_MIN = 297,
     TOK_MOD = 298,
     TOK_NAMES = 299,
     TOK_NULL = 300,
     TOK_OPTION = 301,
     TOK_ORDER = 302,
     TOK_REPLACE = 303,
     TOK_RETURNS = 304,
     TOK_ROLLBACK = 305,
     TOK_SELECT = 306,
     TOK_SET = 307,
     TOK_SHOW = 308,
     TOK_SONAME = 309,
     TOK_START = 310,
     TOK_STATUS = 311,
     TOK_SUM = 312,
     TOK_TABLES = 313,
     TOK_TRANSACTION = 314,
     TOK_TRUE = 315,
     TOK_UPDATE = 316,
     TOK_VALUES = 317,
     TOK_VARIABLES = 318,
     TOK_WARNINGS = 319,
     TOK_WEIGHT = 320,
     TOK_WHERE = 321,
     TOK_WITHIN = 322,
     TOK_OR = 323,
     TOK_AND = 324,
     TOK_NE = 325,
     TOK_GTE = 326,
     TOK_LTE = 327,
     TOK_NOT = 328,
     TOK_NEG = 329
   };
#endif
#define TOK_IDENT 258
#define TOK_ATIDENT 259
#define TOK_CONST_INT 260
#define TOK_CONST_FLOAT 261
#define TOK_QUOTED_STRING 262
#define TOK_USERVAR 263
#define TOK_SYSVAR 264
#define TOK_AS 265
#define TOK_ASC 266
#define TOK_AVG 267
#define TOK_BEGIN 268
#define TOK_BETWEEN 269
#define TOK_BY 270
#define TOK_CALL 271
#define TOK_COLLATION 272
#define TOK_COMMIT 273
#define TOK_COUNT 274
#define TOK_CREATE 275
#define TOK_DELETE 276
#define TOK_DESC 277
#define TOK_DESCRIBE 278
#define TOK_DISTINCT 279
#define TOK_DIV 280
#define TOK_DROP 281
#define TOK_FALSE 282
#define TOK_FLOAT 283
#define TOK_FROM 284
#define TOK_FUNCTION 285
#define TOK_GLOBAL 286
#define TOK_GROUP 287
#define TOK_ID 288
#define TOK_IN 289
#define TOK_INSERT 290
#define TOK_INT 291
#define TOK_INTO 292
#define TOK_LIMIT 293
#define TOK_MATCH 294
#define TOK_MAX 295
#define TOK_META 296
#define TOK_MIN 297
#define TOK_MOD 298
#define TOK_NAMES 299
#define TOK_NULL 300
#define TOK_OPTION 301
#define TOK_ORDER 302
#define TOK_REPLACE 303
#define TOK_RETURNS 304
#define TOK_ROLLBACK 305
#define TOK_SELECT 306
#define TOK_SET 307
#define TOK_SHOW 308
#define TOK_SONAME 309
#define TOK_START 310
#define TOK_STATUS 311
#define TOK_SUM 312
#define TOK_TABLES 313
#define TOK_TRANSACTION 314
#define TOK_TRUE 315
#define TOK_UPDATE 316
#define TOK_VALUES 317
#define TOK_VARIABLES 318
#define TOK_WARNINGS 319
#define TOK_WEIGHT 320
#define TOK_WHERE 321
#define TOK_WITHIN 322
#define TOK_OR 323
#define TOK_AND 324
#define TOK_NE 325
#define TOK_GTE 326
#define TOK_LTE 327
#define TOK_NOT 328
#define TOK_NEG 329




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





