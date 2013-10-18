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
     TOK_DROP = 296,
     TOK_FALSE = 297,
     TOK_FLOAT = 298,
     TOK_FLUSH = 299,
     TOK_FOR = 300,
     TOK_FROM = 301,
     TOK_FUNCTION = 302,
     TOK_GLOBAL = 303,
     TOK_GROUP = 304,
     TOK_GROUPBY = 305,
     TOK_GROUP_CONCAT = 306,
     TOK_HAVING = 307,
     TOK_ID = 308,
     TOK_IN = 309,
     TOK_INDEX = 310,
     TOK_INSERT = 311,
     TOK_INT = 312,
     TOK_INTEGER = 313,
     TOK_INTO = 314,
     TOK_IS = 315,
     TOK_ISOLATION = 316,
     TOK_LEVEL = 317,
     TOK_LIKE = 318,
     TOK_LIMIT = 319,
     TOK_MATCH = 320,
     TOK_MAX = 321,
     TOK_META = 322,
     TOK_MIN = 323,
     TOK_MOD = 324,
     TOK_NAMES = 325,
     TOK_NULL = 326,
     TOK_OPTION = 327,
     TOK_ORDER = 328,
     TOK_OPTIMIZE = 329,
     TOK_PLAN = 330,
     TOK_PROFILE = 331,
     TOK_RAND = 332,
     TOK_RAMCHUNK = 333,
     TOK_READ = 334,
     TOK_REPEATABLE = 335,
     TOK_REPLACE = 336,
     TOK_RETURNS = 337,
     TOK_ROLLBACK = 338,
     TOK_RTINDEX = 339,
     TOK_SELECT = 340,
     TOK_SERIALIZABLE = 341,
     TOK_SET = 342,
     TOK_SESSION = 343,
     TOK_SHOW = 344,
     TOK_SONAME = 345,
     TOK_START = 346,
     TOK_STATUS = 347,
     TOK_STRING = 348,
     TOK_SUM = 349,
     TOK_TABLE = 350,
     TOK_TABLES = 351,
     TOK_TO = 352,
     TOK_TRANSACTION = 353,
     TOK_TRUE = 354,
     TOK_TRUNCATE = 355,
     TOK_UNCOMMITTED = 356,
     TOK_UPDATE = 357,
     TOK_VALUES = 358,
     TOK_VARIABLES = 359,
     TOK_WARNINGS = 360,
     TOK_WEIGHT = 361,
     TOK_WHERE = 362,
     TOK_WITHIN = 363,
     TOK_OR = 364,
     TOK_AND = 365,
     TOK_NE = 366,
     TOK_GTE = 367,
     TOK_LTE = 368,
     TOK_NOT = 369,
     TOK_NEG = 370
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
#define TOK_DROP 296
#define TOK_FALSE 297
#define TOK_FLOAT 298
#define TOK_FLUSH 299
#define TOK_FOR 300
#define TOK_FROM 301
#define TOK_FUNCTION 302
#define TOK_GLOBAL 303
#define TOK_GROUP 304
#define TOK_GROUPBY 305
#define TOK_GROUP_CONCAT 306
#define TOK_HAVING 307
#define TOK_ID 308
#define TOK_IN 309
#define TOK_INDEX 310
#define TOK_INSERT 311
#define TOK_INT 312
#define TOK_INTEGER 313
#define TOK_INTO 314
#define TOK_IS 315
#define TOK_ISOLATION 316
#define TOK_LEVEL 317
#define TOK_LIKE 318
#define TOK_LIMIT 319
#define TOK_MATCH 320
#define TOK_MAX 321
#define TOK_META 322
#define TOK_MIN 323
#define TOK_MOD 324
#define TOK_NAMES 325
#define TOK_NULL 326
#define TOK_OPTION 327
#define TOK_ORDER 328
#define TOK_OPTIMIZE 329
#define TOK_PLAN 330
#define TOK_PROFILE 331
#define TOK_RAND 332
#define TOK_RAMCHUNK 333
#define TOK_READ 334
#define TOK_REPEATABLE 335
#define TOK_REPLACE 336
#define TOK_RETURNS 337
#define TOK_ROLLBACK 338
#define TOK_RTINDEX 339
#define TOK_SELECT 340
#define TOK_SERIALIZABLE 341
#define TOK_SET 342
#define TOK_SESSION 343
#define TOK_SHOW 344
#define TOK_SONAME 345
#define TOK_START 346
#define TOK_STATUS 347
#define TOK_STRING 348
#define TOK_SUM 349
#define TOK_TABLE 350
#define TOK_TABLES 351
#define TOK_TO 352
#define TOK_TRANSACTION 353
#define TOK_TRUE 354
#define TOK_TRUNCATE 355
#define TOK_UNCOMMITTED 356
#define TOK_UPDATE 357
#define TOK_VALUES 358
#define TOK_VARIABLES 359
#define TOK_WARNINGS 360
#define TOK_WEIGHT 361
#define TOK_WHERE 362
#define TOK_WITHIN 363
#define TOK_OR 364
#define TOK_AND 365
#define TOK_NE 366
#define TOK_GTE 367
#define TOK_LTE 368
#define TOK_NOT 369
#define TOK_NEG 370




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





