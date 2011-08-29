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
     TOK_CONST_MVA = 262,
     TOK_QUOTED_STRING = 263,
     TOK_USERVAR = 264,
     TOK_SYSVAR = 265,
     TOK_AS = 266,
     TOK_ASC = 267,
     TOK_ATTACH = 268,
     TOK_AVG = 269,
     TOK_BEGIN = 270,
     TOK_BETWEEN = 271,
     TOK_BY = 272,
     TOK_CALL = 273,
     TOK_COLLATION = 274,
     TOK_COMMIT = 275,
     TOK_COUNT = 276,
     TOK_CREATE = 277,
     TOK_DELETE = 278,
     TOK_DESC = 279,
     TOK_DESCRIBE = 280,
     TOK_DISTINCT = 281,
     TOK_DIV = 282,
     TOK_DROP = 283,
     TOK_FALSE = 284,
     TOK_FLOAT = 285,
     TOK_FROM = 286,
     TOK_FUNCTION = 287,
     TOK_GLOBAL = 288,
     TOK_GROUP = 289,
     TOK_ID = 290,
     TOK_IN = 291,
     TOK_INDEX = 292,
     TOK_INSERT = 293,
     TOK_INT = 294,
     TOK_INTO = 295,
     TOK_LIMIT = 296,
     TOK_MATCH = 297,
     TOK_MAX = 298,
     TOK_META = 299,
     TOK_MIN = 300,
     TOK_MOD = 301,
     TOK_NAMES = 302,
     TOK_NULL = 303,
     TOK_OPTION = 304,
     TOK_ORDER = 305,
     TOK_RAND = 306,
     TOK_REPLACE = 307,
     TOK_RETURNS = 308,
     TOK_ROLLBACK = 309,
     TOK_RTINDEX = 310,
     TOK_SELECT = 311,
     TOK_SET = 312,
     TOK_SESSION = 313,
     TOK_SHOW = 314,
     TOK_SONAME = 315,
     TOK_START = 316,
     TOK_STATUS = 317,
     TOK_SUM = 318,
     TOK_TABLES = 319,
     TOK_TO = 320,
     TOK_TRANSACTION = 321,
     TOK_TRUE = 322,
     TOK_UPDATE = 323,
     TOK_VALUES = 324,
     TOK_VARIABLES = 325,
     TOK_WARNINGS = 326,
     TOK_WEIGHT = 327,
     TOK_WHERE = 328,
     TOK_WITHIN = 329,
     TOK_OR = 330,
     TOK_AND = 331,
     TOK_NE = 332,
     TOK_GTE = 333,
     TOK_LTE = 334,
     TOK_NOT = 335,
     TOK_NEG = 336
   };
#endif
#define TOK_IDENT 258
#define TOK_ATIDENT 259
#define TOK_CONST_INT 260
#define TOK_CONST_FLOAT 261
#define TOK_CONST_MVA 262
#define TOK_QUOTED_STRING 263
#define TOK_USERVAR 264
#define TOK_SYSVAR 265
#define TOK_AS 266
#define TOK_ASC 267
#define TOK_ATTACH 268
#define TOK_AVG 269
#define TOK_BEGIN 270
#define TOK_BETWEEN 271
#define TOK_BY 272
#define TOK_CALL 273
#define TOK_COLLATION 274
#define TOK_COMMIT 275
#define TOK_COUNT 276
#define TOK_CREATE 277
#define TOK_DELETE 278
#define TOK_DESC 279
#define TOK_DESCRIBE 280
#define TOK_DISTINCT 281
#define TOK_DIV 282
#define TOK_DROP 283
#define TOK_FALSE 284
#define TOK_FLOAT 285
#define TOK_FROM 286
#define TOK_FUNCTION 287
#define TOK_GLOBAL 288
#define TOK_GROUP 289
#define TOK_ID 290
#define TOK_IN 291
#define TOK_INDEX 292
#define TOK_INSERT 293
#define TOK_INT 294
#define TOK_INTO 295
#define TOK_LIMIT 296
#define TOK_MATCH 297
#define TOK_MAX 298
#define TOK_META 299
#define TOK_MIN 300
#define TOK_MOD 301
#define TOK_NAMES 302
#define TOK_NULL 303
#define TOK_OPTION 304
#define TOK_ORDER 305
#define TOK_RAND 306
#define TOK_REPLACE 307
#define TOK_RETURNS 308
#define TOK_ROLLBACK 309
#define TOK_RTINDEX 310
#define TOK_SELECT 311
#define TOK_SET 312
#define TOK_SESSION 313
#define TOK_SHOW 314
#define TOK_SONAME 315
#define TOK_START 316
#define TOK_STATUS 317
#define TOK_SUM 318
#define TOK_TABLES 319
#define TOK_TO 320
#define TOK_TRANSACTION 321
#define TOK_TRUE 322
#define TOK_UPDATE 323
#define TOK_VALUES 324
#define TOK_VARIABLES 325
#define TOK_WARNINGS 326
#define TOK_WEIGHT 327
#define TOK_WHERE 328
#define TOK_WITHIN 329
#define TOK_OR 330
#define TOK_AND 331
#define TOK_NE 332
#define TOK_GTE 333
#define TOK_LTE 334
#define TOK_NOT 335
#define TOK_NEG 336




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





