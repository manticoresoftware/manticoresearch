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
     TOK_ALL = 273,
     TOK_ANY = 274,
     TOK_AS = 275,
     TOK_ASC = 276,
     TOK_ATTACH = 277,
     TOK_ATTRIBUTES = 278,
     TOK_AVG = 279,
     TOK_BEGIN = 280,
     TOK_BETWEEN = 281,
     TOK_BIGINT = 282,
     TOK_BOOL = 283,
     TOK_BY = 284,
     TOK_CALL = 285,
     TOK_CHARACTER = 286,
     TOK_CHUNK = 287,
     TOK_COLLATION = 288,
     TOK_COLUMN = 289,
     TOK_COMMIT = 290,
     TOK_COMMITTED = 291,
     TOK_COUNT = 292,
     TOK_CREATE = 293,
     TOK_DATABASES = 294,
     TOK_DELETE = 295,
     TOK_DESC = 296,
     TOK_DESCRIBE = 297,
     TOK_DISTINCT = 298,
     TOK_DIV = 299,
     TOK_DOUBLE = 300,
     TOK_DROP = 301,
     TOK_FACET = 302,
     TOK_FALSE = 303,
     TOK_FLOAT = 304,
     TOK_FLUSH = 305,
     TOK_FOR = 306,
     TOK_FROM = 307,
     TOK_FUNCTION = 308,
     TOK_GLOBAL = 309,
     TOK_GROUP = 310,
     TOK_GROUPBY = 311,
     TOK_GROUP_CONCAT = 312,
     TOK_HAVING = 313,
     TOK_ID = 314,
     TOK_IN = 315,
     TOK_INDEX = 316,
     TOK_INDEXOF = 317,
     TOK_INSERT = 318,
     TOK_INT = 319,
     TOK_INTEGER = 320,
     TOK_INTO = 321,
     TOK_IS = 322,
     TOK_ISOLATION = 323,
     TOK_JSON = 324,
     TOK_LEVEL = 325,
     TOK_LIKE = 326,
     TOK_LIMIT = 327,
     TOK_MATCH = 328,
     TOK_MAX = 329,
     TOK_META = 330,
     TOK_MIN = 331,
     TOK_MOD = 332,
     TOK_MULTI = 333,
     TOK_MULTI64 = 334,
     TOK_NAMES = 335,
     TOK_NULL = 336,
     TOK_OPTION = 337,
     TOK_ORDER = 338,
     TOK_OPTIMIZE = 339,
     TOK_PLAN = 340,
     TOK_PLUGIN = 341,
     TOK_PLUGINS = 342,
     TOK_PROFILE = 343,
     TOK_RAND = 344,
     TOK_RAMCHUNK = 345,
     TOK_READ = 346,
     TOK_RECONFIGURE = 347,
     TOK_RELOAD = 348,
     TOK_REPEATABLE = 349,
     TOK_REPLACE = 350,
     TOK_REMAP = 351,
     TOK_RETURNS = 352,
     TOK_ROLLBACK = 353,
     TOK_RTINDEX = 354,
     TOK_SELECT = 355,
     TOK_SERIALIZABLE = 356,
     TOK_SET = 357,
     TOK_SETTINGS = 358,
     TOK_SESSION = 359,
     TOK_SHOW = 360,
     TOK_SONAME = 361,
     TOK_START = 362,
     TOK_STATUS = 363,
     TOK_STRING = 364,
     TOK_SUM = 365,
     TOK_TABLE = 366,
     TOK_TABLES = 367,
     TOK_THREADS = 368,
     TOK_TO = 369,
     TOK_TRANSACTION = 370,
     TOK_TRUE = 371,
     TOK_TRUNCATE = 372,
     TOK_TYPE = 373,
     TOK_UNCOMMITTED = 374,
     TOK_UPDATE = 375,
     TOK_VALUES = 376,
     TOK_VARIABLES = 377,
     TOK_WARNINGS = 378,
     TOK_WEIGHT = 379,
     TOK_WHERE = 380,
     TOK_WITHIN = 381,
     TOK_OR = 382,
     TOK_AND = 383,
     TOK_NE = 384,
     TOK_GTE = 385,
     TOK_LTE = 386,
     TOK_NOT = 387,
     TOK_NEG = 388
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
#define TOK_ALL 273
#define TOK_ANY 274
#define TOK_AS 275
#define TOK_ASC 276
#define TOK_ATTACH 277
#define TOK_ATTRIBUTES 278
#define TOK_AVG 279
#define TOK_BEGIN 280
#define TOK_BETWEEN 281
#define TOK_BIGINT 282
#define TOK_BOOL 283
#define TOK_BY 284
#define TOK_CALL 285
#define TOK_CHARACTER 286
#define TOK_CHUNK 287
#define TOK_COLLATION 288
#define TOK_COLUMN 289
#define TOK_COMMIT 290
#define TOK_COMMITTED 291
#define TOK_COUNT 292
#define TOK_CREATE 293
#define TOK_DATABASES 294
#define TOK_DELETE 295
#define TOK_DESC 296
#define TOK_DESCRIBE 297
#define TOK_DISTINCT 298
#define TOK_DIV 299
#define TOK_DOUBLE 300
#define TOK_DROP 301
#define TOK_FACET 302
#define TOK_FALSE 303
#define TOK_FLOAT 304
#define TOK_FLUSH 305
#define TOK_FOR 306
#define TOK_FROM 307
#define TOK_FUNCTION 308
#define TOK_GLOBAL 309
#define TOK_GROUP 310
#define TOK_GROUPBY 311
#define TOK_GROUP_CONCAT 312
#define TOK_HAVING 313
#define TOK_ID 314
#define TOK_IN 315
#define TOK_INDEX 316
#define TOK_INDEXOF 317
#define TOK_INSERT 318
#define TOK_INT 319
#define TOK_INTEGER 320
#define TOK_INTO 321
#define TOK_IS 322
#define TOK_ISOLATION 323
#define TOK_JSON 324
#define TOK_LEVEL 325
#define TOK_LIKE 326
#define TOK_LIMIT 327
#define TOK_MATCH 328
#define TOK_MAX 329
#define TOK_META 330
#define TOK_MIN 331
#define TOK_MOD 332
#define TOK_MULTI 333
#define TOK_MULTI64 334
#define TOK_NAMES 335
#define TOK_NULL 336
#define TOK_OPTION 337
#define TOK_ORDER 338
#define TOK_OPTIMIZE 339
#define TOK_PLAN 340
#define TOK_PLUGIN 341
#define TOK_PLUGINS 342
#define TOK_PROFILE 343
#define TOK_RAND 344
#define TOK_RAMCHUNK 345
#define TOK_READ 346
#define TOK_RECONFIGURE 347
#define TOK_RELOAD 348
#define TOK_REPEATABLE 349
#define TOK_REPLACE 350
#define TOK_REMAP 351
#define TOK_RETURNS 352
#define TOK_ROLLBACK 353
#define TOK_RTINDEX 354
#define TOK_SELECT 355
#define TOK_SERIALIZABLE 356
#define TOK_SET 357
#define TOK_SETTINGS 358
#define TOK_SESSION 359
#define TOK_SHOW 360
#define TOK_SONAME 361
#define TOK_START 362
#define TOK_STATUS 363
#define TOK_STRING 364
#define TOK_SUM 365
#define TOK_TABLE 366
#define TOK_TABLES 367
#define TOK_THREADS 368
#define TOK_TO 369
#define TOK_TRANSACTION 370
#define TOK_TRUE 371
#define TOK_TRUNCATE 372
#define TOK_TYPE 373
#define TOK_UNCOMMITTED 374
#define TOK_UPDATE 375
#define TOK_VALUES 376
#define TOK_VARIABLES 377
#define TOK_WARNINGS 378
#define TOK_WEIGHT 379
#define TOK_WHERE 380
#define TOK_WITHIN 381
#define TOK_OR 382
#define TOK_AND 383
#define TOK_NE 384
#define TOK_GTE 385
#define TOK_LTE 386
#define TOK_NOT 387
#define TOK_NEG 388




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





