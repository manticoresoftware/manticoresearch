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
     TOK_CONST_STRINGS = 266,
     TOK_AS = 267,
     TOK_ASC = 268,
     TOK_ATTACH = 269,
     TOK_AVG = 270,
     TOK_BEGIN = 271,
     TOK_BETWEEN = 272,
     TOK_BY = 273,
     TOK_CALL = 274,
     TOK_COLLATION = 275,
     TOK_COMMIT = 276,
     TOK_COMMITTED = 277,
     TOK_COUNT = 278,
     TOK_CREATE = 279,
     TOK_DELETE = 280,
     TOK_DESC = 281,
     TOK_DESCRIBE = 282,
     TOK_DISTINCT = 283,
     TOK_DIV = 284,
     TOK_DROP = 285,
     TOK_FALSE = 286,
     TOK_FLOAT = 287,
     TOK_FLUSH = 288,
     TOK_FROM = 289,
     TOK_FUNCTION = 290,
     TOK_GLOBAL = 291,
     TOK_GROUP = 292,
     TOK_GROUPBY = 293,
     TOK_GROUP_CONCAT = 294,
     TOK_ID = 295,
     TOK_IN = 296,
     TOK_INDEX = 297,
     TOK_INSERT = 298,
     TOK_INT = 299,
     TOK_INTO = 300,
     TOK_ISOLATION = 301,
     TOK_LEVEL = 302,
     TOK_LIMIT = 303,
     TOK_MATCH = 304,
     TOK_MAX = 305,
     TOK_META = 306,
     TOK_MIN = 307,
     TOK_MOD = 308,
     TOK_NAMES = 309,
     TOK_NULL = 310,
     TOK_OPTION = 311,
     TOK_ORDER = 312,
     TOK_RAND = 313,
     TOK_READ = 314,
     TOK_REPEATABLE = 315,
     TOK_REPLACE = 316,
     TOK_RETURNS = 317,
     TOK_ROLLBACK = 318,
     TOK_RTINDEX = 319,
     TOK_SELECT = 320,
     TOK_SERIALIZABLE = 321,
     TOK_SET = 322,
     TOK_SESSION = 323,
     TOK_SHOW = 324,
     TOK_SONAME = 325,
     TOK_START = 326,
     TOK_STATUS = 327,
     TOK_SUM = 328,
     TOK_TABLES = 329,
     TOK_TO = 330,
     TOK_TRANSACTION = 331,
     TOK_TRUE = 332,
     TOK_TRUNCATE = 333,
     TOK_UNCOMMITTED = 334,
     TOK_UPDATE = 335,
     TOK_VALUES = 336,
     TOK_VARIABLES = 337,
     TOK_WARNINGS = 338,
     TOK_WEIGHT = 339,
     TOK_WHERE = 340,
     TOK_WITHIN = 341,
     TOK_OR = 342,
     TOK_AND = 343,
     TOK_NE = 344,
     TOK_GTE = 345,
     TOK_LTE = 346,
     TOK_NOT = 347,
     TOK_NEG = 348
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
#define TOK_CONST_STRINGS 266
#define TOK_AS 267
#define TOK_ASC 268
#define TOK_ATTACH 269
#define TOK_AVG 270
#define TOK_BEGIN 271
#define TOK_BETWEEN 272
#define TOK_BY 273
#define TOK_CALL 274
#define TOK_COLLATION 275
#define TOK_COMMIT 276
#define TOK_COMMITTED 277
#define TOK_COUNT 278
#define TOK_CREATE 279
#define TOK_DELETE 280
#define TOK_DESC 281
#define TOK_DESCRIBE 282
#define TOK_DISTINCT 283
#define TOK_DIV 284
#define TOK_DROP 285
#define TOK_FALSE 286
#define TOK_FLOAT 287
#define TOK_FLUSH 288
#define TOK_FROM 289
#define TOK_FUNCTION 290
#define TOK_GLOBAL 291
#define TOK_GROUP 292
#define TOK_GROUPBY 293
#define TOK_GROUP_CONCAT 294
#define TOK_ID 295
#define TOK_IN 296
#define TOK_INDEX 297
#define TOK_INSERT 298
#define TOK_INT 299
#define TOK_INTO 300
#define TOK_ISOLATION 301
#define TOK_LEVEL 302
#define TOK_LIMIT 303
#define TOK_MATCH 304
#define TOK_MAX 305
#define TOK_META 306
#define TOK_MIN 307
#define TOK_MOD 308
#define TOK_NAMES 309
#define TOK_NULL 310
#define TOK_OPTION 311
#define TOK_ORDER 312
#define TOK_RAND 313
#define TOK_READ 314
#define TOK_REPEATABLE 315
#define TOK_REPLACE 316
#define TOK_RETURNS 317
#define TOK_ROLLBACK 318
#define TOK_RTINDEX 319
#define TOK_SELECT 320
#define TOK_SERIALIZABLE 321
#define TOK_SET 322
#define TOK_SESSION 323
#define TOK_SHOW 324
#define TOK_SONAME 325
#define TOK_START 326
#define TOK_STATUS 327
#define TOK_SUM 328
#define TOK_TABLES 329
#define TOK_TO 330
#define TOK_TRANSACTION 331
#define TOK_TRUE 332
#define TOK_TRUNCATE 333
#define TOK_UNCOMMITTED 334
#define TOK_UPDATE 335
#define TOK_VALUES 336
#define TOK_VARIABLES 337
#define TOK_WARNINGS 338
#define TOK_WEIGHT 339
#define TOK_WHERE 340
#define TOK_WITHIN 341
#define TOK_OR 342
#define TOK_AND 343
#define TOK_NE 344
#define TOK_GTE 345
#define TOK_LTE 346
#define TOK_NOT 347
#define TOK_NEG 348




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





