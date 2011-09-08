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
     TOK_FLUSH = 286,
     TOK_FROM = 287,
     TOK_FUNCTION = 288,
     TOK_GLOBAL = 289,
     TOK_GROUP = 290,
     TOK_ID = 291,
     TOK_IN = 292,
     TOK_INDEX = 293,
     TOK_INSERT = 294,
     TOK_INT = 295,
     TOK_INTO = 296,
     TOK_LIMIT = 297,
     TOK_MATCH = 298,
     TOK_MAX = 299,
     TOK_META = 300,
     TOK_MIN = 301,
     TOK_MOD = 302,
     TOK_NAMES = 303,
     TOK_NULL = 304,
     TOK_OPTION = 305,
     TOK_ORDER = 306,
     TOK_RAND = 307,
     TOK_REPLACE = 308,
     TOK_RETURNS = 309,
     TOK_ROLLBACK = 310,
     TOK_RTINDEX = 311,
     TOK_SELECT = 312,
     TOK_SET = 313,
     TOK_SESSION = 314,
     TOK_SHOW = 315,
     TOK_SONAME = 316,
     TOK_START = 317,
     TOK_STATUS = 318,
     TOK_SUM = 319,
     TOK_TABLES = 320,
     TOK_TO = 321,
     TOK_TRANSACTION = 322,
     TOK_TRUE = 323,
     TOK_UPDATE = 324,
     TOK_VALUES = 325,
     TOK_VARIABLES = 326,
     TOK_WARNINGS = 327,
     TOK_WEIGHT = 328,
     TOK_WHERE = 329,
     TOK_WITHIN = 330,
     TOK_OR = 331,
     TOK_AND = 332,
     TOK_NE = 333,
     TOK_GTE = 334,
     TOK_LTE = 335,
     TOK_NOT = 336,
     TOK_NEG = 337
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
#define TOK_FLUSH 286
#define TOK_FROM 287
#define TOK_FUNCTION 288
#define TOK_GLOBAL 289
#define TOK_GROUP 290
#define TOK_ID 291
#define TOK_IN 292
#define TOK_INDEX 293
#define TOK_INSERT 294
#define TOK_INT 295
#define TOK_INTO 296
#define TOK_LIMIT 297
#define TOK_MATCH 298
#define TOK_MAX 299
#define TOK_META 300
#define TOK_MIN 301
#define TOK_MOD 302
#define TOK_NAMES 303
#define TOK_NULL 304
#define TOK_OPTION 305
#define TOK_ORDER 306
#define TOK_RAND 307
#define TOK_REPLACE 308
#define TOK_RETURNS 309
#define TOK_ROLLBACK 310
#define TOK_RTINDEX 311
#define TOK_SELECT 312
#define TOK_SET 313
#define TOK_SESSION 314
#define TOK_SHOW 315
#define TOK_SONAME 316
#define TOK_START 317
#define TOK_STATUS 318
#define TOK_SUM 319
#define TOK_TABLES 320
#define TOK_TO 321
#define TOK_TRANSACTION 322
#define TOK_TRUE 323
#define TOK_UPDATE 324
#define TOK_VALUES 325
#define TOK_VARIABLES 326
#define TOK_WARNINGS 327
#define TOK_WEIGHT 328
#define TOK_WHERE 329
#define TOK_WITHIN 330
#define TOK_OR 331
#define TOK_AND 332
#define TOK_NE 333
#define TOK_GTE 334
#define TOK_LTE 335
#define TOK_NOT 336
#define TOK_NEG 337




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





