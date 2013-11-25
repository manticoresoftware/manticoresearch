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
     TOK_BAD_NUMERIC = 267,
     TOK_SUBKEY = 268,
     TOK_DOT_NUMBER = 269,
     TOK_ADD = 270,
     TOK_AGENT = 271,
     TOK_ALTER = 272,
     TOK_AS = 273,
     TOK_ASC = 274,
     TOK_ATTACH = 275,
     TOK_AVG = 276,
     TOK_BEGIN = 277,
     TOK_BETWEEN = 278,
     TOK_BIGINT = 279,
     TOK_BOOL = 280,
     TOK_BY = 281,
     TOK_CALL = 282,
     TOK_CHARACTER = 283,
     TOK_COLLATION = 284,
     TOK_COLUMN = 285,
     TOK_COMMIT = 286,
     TOK_COMMITTED = 287,
     TOK_COUNT = 288,
     TOK_CREATE = 289,
     TOK_DATABASES = 290,
     TOK_DELETE = 291,
     TOK_DESC = 292,
     TOK_DESCRIBE = 293,
     TOK_DISTINCT = 294,
     TOK_DIV = 295,
     TOK_DOUBLE = 296,
     TOK_DROP = 297,
     TOK_FALSE = 298,
     TOK_FLOAT = 299,
     TOK_FLUSH = 300,
     TOK_FOR = 301,
     TOK_FROM = 302,
     TOK_FUNCTION = 303,
     TOK_GLOBAL = 304,
     TOK_GROUP = 305,
     TOK_GROUPBY = 306,
     TOK_GROUP_CONCAT = 307,
     TOK_HAVING = 308,
     TOK_ID = 309,
     TOK_IN = 310,
     TOK_INDEX = 311,
     TOK_INSERT = 312,
     TOK_INT = 313,
     TOK_INTEGER = 314,
     TOK_INTO = 315,
     TOK_IS = 316,
     TOK_ISOLATION = 317,
     TOK_JSON = 318,
     TOK_LEVEL = 319,
     TOK_LIKE = 320,
     TOK_LIMIT = 321,
     TOK_MATCH = 322,
     TOK_MAX = 323,
     TOK_META = 324,
     TOK_MIN = 325,
     TOK_MOD = 326,
     TOK_MULTI = 327,
     TOK_MULTI64 = 328,
     TOK_NAMES = 329,
     TOK_NULL = 330,
     TOK_OPTION = 331,
     TOK_ORDER = 332,
     TOK_OPTIMIZE = 333,
     TOK_PLAN = 334,
     TOK_PROFILE = 335,
     TOK_RAND = 336,
     TOK_RAMCHUNK = 337,
     TOK_RANKER = 338,
     TOK_READ = 339,
     TOK_REPEATABLE = 340,
     TOK_REPLACE = 341,
     TOK_RETURNS = 342,
     TOK_ROLLBACK = 343,
     TOK_RTINDEX = 344,
     TOK_SELECT = 345,
     TOK_SERIALIZABLE = 346,
     TOK_SET = 347,
     TOK_SESSION = 348,
     TOK_SHOW = 349,
     TOK_SONAME = 350,
     TOK_START = 351,
     TOK_STATUS = 352,
     TOK_STRING = 353,
     TOK_SUM = 354,
     TOK_TABLE = 355,
     TOK_TABLES = 356,
     TOK_TO = 357,
     TOK_TRANSACTION = 358,
     TOK_TRUE = 359,
     TOK_TRUNCATE = 360,
     TOK_UNCOMMITTED = 361,
     TOK_UPDATE = 362,
     TOK_VALUES = 363,
     TOK_VARIABLES = 364,
     TOK_WARNINGS = 365,
     TOK_WEIGHT = 366,
     TOK_WHERE = 367,
     TOK_WITHIN = 368,
     TOK_OR = 369,
     TOK_AND = 370,
     TOK_NE = 371,
     TOK_GTE = 372,
     TOK_LTE = 373,
     TOK_NOT = 374,
     TOK_NEG = 375
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
#define TOK_BAD_NUMERIC 267
#define TOK_SUBKEY 268
#define TOK_DOT_NUMBER 269
#define TOK_ADD 270
#define TOK_AGENT 271
#define TOK_ALTER 272
#define TOK_AS 273
#define TOK_ASC 274
#define TOK_ATTACH 275
#define TOK_AVG 276
#define TOK_BEGIN 277
#define TOK_BETWEEN 278
#define TOK_BIGINT 279
#define TOK_BOOL 280
#define TOK_BY 281
#define TOK_CALL 282
#define TOK_CHARACTER 283
#define TOK_COLLATION 284
#define TOK_COLUMN 285
#define TOK_COMMIT 286
#define TOK_COMMITTED 287
#define TOK_COUNT 288
#define TOK_CREATE 289
#define TOK_DATABASES 290
#define TOK_DELETE 291
#define TOK_DESC 292
#define TOK_DESCRIBE 293
#define TOK_DISTINCT 294
#define TOK_DIV 295
#define TOK_DOUBLE 296
#define TOK_DROP 297
#define TOK_FALSE 298
#define TOK_FLOAT 299
#define TOK_FLUSH 300
#define TOK_FOR 301
#define TOK_FROM 302
#define TOK_FUNCTION 303
#define TOK_GLOBAL 304
#define TOK_GROUP 305
#define TOK_GROUPBY 306
#define TOK_GROUP_CONCAT 307
#define TOK_HAVING 308
#define TOK_ID 309
#define TOK_IN 310
#define TOK_INDEX 311
#define TOK_INSERT 312
#define TOK_INT 313
#define TOK_INTEGER 314
#define TOK_INTO 315
#define TOK_IS 316
#define TOK_ISOLATION 317
#define TOK_JSON 318
#define TOK_LEVEL 319
#define TOK_LIKE 320
#define TOK_LIMIT 321
#define TOK_MATCH 322
#define TOK_MAX 323
#define TOK_META 324
#define TOK_MIN 325
#define TOK_MOD 326
#define TOK_MULTI 327
#define TOK_MULTI64 328
#define TOK_NAMES 329
#define TOK_NULL 330
#define TOK_OPTION 331
#define TOK_ORDER 332
#define TOK_OPTIMIZE 333
#define TOK_PLAN 334
#define TOK_PROFILE 335
#define TOK_RAND 336
#define TOK_RAMCHUNK 337
#define TOK_RANKER 338
#define TOK_READ 339
#define TOK_REPEATABLE 340
#define TOK_REPLACE 341
#define TOK_RETURNS 342
#define TOK_ROLLBACK 343
#define TOK_RTINDEX 344
#define TOK_SELECT 345
#define TOK_SERIALIZABLE 346
#define TOK_SET 347
#define TOK_SESSION 348
#define TOK_SHOW 349
#define TOK_SONAME 350
#define TOK_START 351
#define TOK_STATUS 352
#define TOK_STRING 353
#define TOK_SUM 354
#define TOK_TABLE 355
#define TOK_TABLES 356
#define TOK_TO 357
#define TOK_TRANSACTION 358
#define TOK_TRUE 359
#define TOK_TRUNCATE 360
#define TOK_UNCOMMITTED 361
#define TOK_UPDATE 362
#define TOK_VALUES 363
#define TOK_VARIABLES 364
#define TOK_WARNINGS 365
#define TOK_WEIGHT 366
#define TOK_WHERE 367
#define TOK_WITHIN 368
#define TOK_OR 369
#define TOK_AND 370
#define TOK_NE 371
#define TOK_GTE 372
#define TOK_LTE 373
#define TOK_NOT 374
#define TOK_NEG 375




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





