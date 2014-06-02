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
     TOK_CHUNK = 284,
     TOK_COLLATION = 285,
     TOK_COLUMN = 286,
     TOK_COMMIT = 287,
     TOK_COMMITTED = 288,
     TOK_COUNT = 289,
     TOK_CREATE = 290,
     TOK_DATABASES = 291,
     TOK_DELETE = 292,
     TOK_DESC = 293,
     TOK_DESCRIBE = 294,
     TOK_DISTINCT = 295,
     TOK_DIV = 296,
     TOK_DOUBLE = 297,
     TOK_DROP = 298,
     TOK_FACET = 299,
     TOK_FALSE = 300,
     TOK_FLOAT = 301,
     TOK_FLUSH = 302,
     TOK_FOR = 303,
     TOK_FROM = 304,
     TOK_FUNCTION = 305,
     TOK_GLOBAL = 306,
     TOK_GROUP = 307,
     TOK_GROUPBY = 308,
     TOK_GROUP_CONCAT = 309,
     TOK_HAVING = 310,
     TOK_ID = 311,
     TOK_IN = 312,
     TOK_INDEX = 313,
     TOK_INSERT = 314,
     TOK_INT = 315,
     TOK_INTEGER = 316,
     TOK_INTO = 317,
     TOK_IS = 318,
     TOK_ISOLATION = 319,
     TOK_JSON = 320,
     TOK_LEVEL = 321,
     TOK_LIKE = 322,
     TOK_LIMIT = 323,
     TOK_MATCH = 324,
     TOK_MAX = 325,
     TOK_META = 326,
     TOK_MIN = 327,
     TOK_MOD = 328,
     TOK_MULTI = 329,
     TOK_MULTI64 = 330,
     TOK_NAMES = 331,
     TOK_NULL = 332,
     TOK_OPTION = 333,
     TOK_ORDER = 334,
     TOK_OPTIMIZE = 335,
     TOK_PLAN = 336,
     TOK_PLUGIN = 337,
     TOK_PLUGINS = 338,
     TOK_PROFILE = 339,
     TOK_RAND = 340,
     TOK_RAMCHUNK = 341,
     TOK_READ = 342,
     TOK_RECONFIGURE = 343,
     TOK_REPEATABLE = 344,
     TOK_REPLACE = 345,
     TOK_REMAP = 346,
     TOK_RETURNS = 347,
     TOK_ROLLBACK = 348,
     TOK_RTINDEX = 349,
     TOK_SELECT = 350,
     TOK_SERIALIZABLE = 351,
     TOK_SET = 352,
     TOK_SETTINGS = 353,
     TOK_SESSION = 354,
     TOK_SHOW = 355,
     TOK_SONAME = 356,
     TOK_START = 357,
     TOK_STATUS = 358,
     TOK_STRING = 359,
     TOK_SUM = 360,
     TOK_TABLE = 361,
     TOK_TABLES = 362,
     TOK_THREADS = 363,
     TOK_TO = 364,
     TOK_TRANSACTION = 365,
     TOK_TRUE = 366,
     TOK_TRUNCATE = 367,
     TOK_TYPE = 368,
     TOK_UNCOMMITTED = 369,
     TOK_UPDATE = 370,
     TOK_VALUES = 371,
     TOK_VARIABLES = 372,
     TOK_WARNINGS = 373,
     TOK_WEIGHT = 374,
     TOK_WHERE = 375,
     TOK_WITHIN = 376,
     TOK_OR = 377,
     TOK_AND = 378,
     TOK_NE = 379,
     TOK_GTE = 380,
     TOK_LTE = 381,
     TOK_NOT = 382,
     TOK_NEG = 383
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
#define TOK_CHUNK 284
#define TOK_COLLATION 285
#define TOK_COLUMN 286
#define TOK_COMMIT 287
#define TOK_COMMITTED 288
#define TOK_COUNT 289
#define TOK_CREATE 290
#define TOK_DATABASES 291
#define TOK_DELETE 292
#define TOK_DESC 293
#define TOK_DESCRIBE 294
#define TOK_DISTINCT 295
#define TOK_DIV 296
#define TOK_DOUBLE 297
#define TOK_DROP 298
#define TOK_FACET 299
#define TOK_FALSE 300
#define TOK_FLOAT 301
#define TOK_FLUSH 302
#define TOK_FOR 303
#define TOK_FROM 304
#define TOK_FUNCTION 305
#define TOK_GLOBAL 306
#define TOK_GROUP 307
#define TOK_GROUPBY 308
#define TOK_GROUP_CONCAT 309
#define TOK_HAVING 310
#define TOK_ID 311
#define TOK_IN 312
#define TOK_INDEX 313
#define TOK_INSERT 314
#define TOK_INT 315
#define TOK_INTEGER 316
#define TOK_INTO 317
#define TOK_IS 318
#define TOK_ISOLATION 319
#define TOK_JSON 320
#define TOK_LEVEL 321
#define TOK_LIKE 322
#define TOK_LIMIT 323
#define TOK_MATCH 324
#define TOK_MAX 325
#define TOK_META 326
#define TOK_MIN 327
#define TOK_MOD 328
#define TOK_MULTI 329
#define TOK_MULTI64 330
#define TOK_NAMES 331
#define TOK_NULL 332
#define TOK_OPTION 333
#define TOK_ORDER 334
#define TOK_OPTIMIZE 335
#define TOK_PLAN 336
#define TOK_PLUGIN 337
#define TOK_PLUGINS 338
#define TOK_PROFILE 339
#define TOK_RAND 340
#define TOK_RAMCHUNK 341
#define TOK_READ 342
#define TOK_RECONFIGURE 343
#define TOK_REPEATABLE 344
#define TOK_REPLACE 345
#define TOK_REMAP 346
#define TOK_RETURNS 347
#define TOK_ROLLBACK 348
#define TOK_RTINDEX 349
#define TOK_SELECT 350
#define TOK_SERIALIZABLE 351
#define TOK_SET 352
#define TOK_SETTINGS 353
#define TOK_SESSION 354
#define TOK_SHOW 355
#define TOK_SONAME 356
#define TOK_START 357
#define TOK_STATUS 358
#define TOK_STRING 359
#define TOK_SUM 360
#define TOK_TABLE 361
#define TOK_TABLES 362
#define TOK_THREADS 363
#define TOK_TO 364
#define TOK_TRANSACTION 365
#define TOK_TRUE 366
#define TOK_TRUNCATE 367
#define TOK_TYPE 368
#define TOK_UNCOMMITTED 369
#define TOK_UPDATE 370
#define TOK_VALUES 371
#define TOK_VARIABLES 372
#define TOK_WARNINGS 373
#define TOK_WEIGHT 374
#define TOK_WHERE 375
#define TOK_WITHIN 376
#define TOK_OR 377
#define TOK_AND 378
#define TOK_NE 379
#define TOK_GTE 380
#define TOK_LTE 381
#define TOK_NOT 382
#define TOK_NEG 383




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





