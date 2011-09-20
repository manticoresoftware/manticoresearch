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
     TOK_COMMITTED = 276,
     TOK_COUNT = 277,
     TOK_CREATE = 278,
     TOK_DELETE = 279,
     TOK_DESC = 280,
     TOK_DESCRIBE = 281,
     TOK_DISTINCT = 282,
     TOK_DIV = 283,
     TOK_DROP = 284,
     TOK_FALSE = 285,
     TOK_FLOAT = 286,
     TOK_FLUSH = 287,
     TOK_FROM = 288,
     TOK_FUNCTION = 289,
     TOK_GLOBAL = 290,
     TOK_GROUP = 291,
     TOK_ID = 292,
     TOK_IN = 293,
     TOK_INDEX = 294,
     TOK_INSERT = 295,
     TOK_INT = 296,
     TOK_INTO = 297,
     TOK_ISOLATION = 298,
     TOK_LEVEL = 299,
     TOK_LIMIT = 300,
     TOK_MATCH = 301,
     TOK_MAX = 302,
     TOK_META = 303,
     TOK_MIN = 304,
     TOK_MOD = 305,
     TOK_NAMES = 306,
     TOK_NULL = 307,
     TOK_OPTION = 308,
     TOK_ORDER = 309,
     TOK_RAND = 310,
     TOK_READ = 311,
     TOK_REPEATABLE = 312,
     TOK_REPLACE = 313,
     TOK_RETURNS = 314,
     TOK_ROLLBACK = 315,
     TOK_RTINDEX = 316,
     TOK_SELECT = 317,
     TOK_SERIALIZABLE = 318,
     TOK_SET = 319,
     TOK_SESSION = 320,
     TOK_SHOW = 321,
     TOK_SONAME = 322,
     TOK_START = 323,
     TOK_STATUS = 324,
     TOK_SUM = 325,
     TOK_TABLES = 326,
     TOK_TO = 327,
     TOK_TRANSACTION = 328,
     TOK_TRUE = 329,
     TOK_UNCOMMITTED = 330,
     TOK_UPDATE = 331,
     TOK_VALUES = 332,
     TOK_VARIABLES = 333,
     TOK_WARNINGS = 334,
     TOK_WEIGHT = 335,
     TOK_WHERE = 336,
     TOK_WITHIN = 337,
     TOK_OR = 338,
     TOK_AND = 339,
     TOK_NE = 340,
     TOK_GTE = 341,
     TOK_LTE = 342,
     TOK_NOT = 343,
     TOK_NEG = 344
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
#define TOK_COMMITTED 276
#define TOK_COUNT 277
#define TOK_CREATE 278
#define TOK_DELETE 279
#define TOK_DESC 280
#define TOK_DESCRIBE 281
#define TOK_DISTINCT 282
#define TOK_DIV 283
#define TOK_DROP 284
#define TOK_FALSE 285
#define TOK_FLOAT 286
#define TOK_FLUSH 287
#define TOK_FROM 288
#define TOK_FUNCTION 289
#define TOK_GLOBAL 290
#define TOK_GROUP 291
#define TOK_ID 292
#define TOK_IN 293
#define TOK_INDEX 294
#define TOK_INSERT 295
#define TOK_INT 296
#define TOK_INTO 297
#define TOK_ISOLATION 298
#define TOK_LEVEL 299
#define TOK_LIMIT 300
#define TOK_MATCH 301
#define TOK_MAX 302
#define TOK_META 303
#define TOK_MIN 304
#define TOK_MOD 305
#define TOK_NAMES 306
#define TOK_NULL 307
#define TOK_OPTION 308
#define TOK_ORDER 309
#define TOK_RAND 310
#define TOK_READ 311
#define TOK_REPEATABLE 312
#define TOK_REPLACE 313
#define TOK_RETURNS 314
#define TOK_ROLLBACK 315
#define TOK_RTINDEX 316
#define TOK_SELECT 317
#define TOK_SERIALIZABLE 318
#define TOK_SET 319
#define TOK_SESSION 320
#define TOK_SHOW 321
#define TOK_SONAME 322
#define TOK_START 323
#define TOK_STATUS 324
#define TOK_SUM 325
#define TOK_TABLES 326
#define TOK_TO 327
#define TOK_TRANSACTION 328
#define TOK_TRUE 329
#define TOK_UNCOMMITTED 330
#define TOK_UPDATE 331
#define TOK_VALUES 332
#define TOK_VARIABLES 333
#define TOK_WARNINGS 334
#define TOK_WEIGHT 335
#define TOK_WHERE 336
#define TOK_WITHIN 337
#define TOK_OR 338
#define TOK_AND 339
#define TOK_NE 340
#define TOK_GTE 341
#define TOK_LTE 342
#define TOK_NOT 343
#define TOK_NEG 344




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





