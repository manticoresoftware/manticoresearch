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
     TOK_FROM = 300,
     TOK_FUNCTION = 301,
     TOK_GLOBAL = 302,
     TOK_GROUP = 303,
     TOK_GROUPBY = 304,
     TOK_GROUP_CONCAT = 305,
     TOK_ID = 306,
     TOK_IN = 307,
     TOK_INDEX = 308,
     TOK_INSERT = 309,
     TOK_INT = 310,
     TOK_INTEGER = 311,
     TOK_INTO = 312,
     TOK_IS = 313,
     TOK_ISOLATION = 314,
     TOK_LEVEL = 315,
     TOK_LIKE = 316,
     TOK_LIMIT = 317,
     TOK_MATCH = 318,
     TOK_MAX = 319,
     TOK_META = 320,
     TOK_MIN = 321,
     TOK_MOD = 322,
     TOK_NAMES = 323,
     TOK_NULL = 324,
     TOK_OPTION = 325,
     TOK_ORDER = 326,
     TOK_OPTIMIZE = 327,
     TOK_PLAN = 328,
     TOK_PROFILE = 329,
     TOK_RAND = 330,
     TOK_RAMCHUNK = 331,
     TOK_READ = 332,
     TOK_REPEATABLE = 333,
     TOK_REPLACE = 334,
     TOK_RETURNS = 335,
     TOK_ROLLBACK = 336,
     TOK_RTINDEX = 337,
     TOK_SELECT = 338,
     TOK_SERIALIZABLE = 339,
     TOK_SET = 340,
     TOK_SESSION = 341,
     TOK_SHOW = 342,
     TOK_SONAME = 343,
     TOK_START = 344,
     TOK_STATUS = 345,
     TOK_STRING = 346,
     TOK_SUM = 347,
     TOK_TABLE = 348,
     TOK_TABLES = 349,
     TOK_TO = 350,
     TOK_TRANSACTION = 351,
     TOK_TRUE = 352,
     TOK_TRUNCATE = 353,
     TOK_UNCOMMITTED = 354,
     TOK_UPDATE = 355,
     TOK_VALUES = 356,
     TOK_VARIABLES = 357,
     TOK_WARNINGS = 358,
     TOK_WEIGHT = 359,
     TOK_WHERE = 360,
     TOK_WITHIN = 361,
     TOK_OR = 362,
     TOK_AND = 363,
     TOK_NE = 364,
     TOK_GTE = 365,
     TOK_LTE = 366,
     TOK_NOT = 367,
     TOK_NEG = 368
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
#define TOK_FROM 300
#define TOK_FUNCTION 301
#define TOK_GLOBAL 302
#define TOK_GROUP 303
#define TOK_GROUPBY 304
#define TOK_GROUP_CONCAT 305
#define TOK_ID 306
#define TOK_IN 307
#define TOK_INDEX 308
#define TOK_INSERT 309
#define TOK_INT 310
#define TOK_INTEGER 311
#define TOK_INTO 312
#define TOK_IS 313
#define TOK_ISOLATION 314
#define TOK_LEVEL 315
#define TOK_LIKE 316
#define TOK_LIMIT 317
#define TOK_MATCH 318
#define TOK_MAX 319
#define TOK_META 320
#define TOK_MIN 321
#define TOK_MOD 322
#define TOK_NAMES 323
#define TOK_NULL 324
#define TOK_OPTION 325
#define TOK_ORDER 326
#define TOK_OPTIMIZE 327
#define TOK_PLAN 328
#define TOK_PROFILE 329
#define TOK_RAND 330
#define TOK_RAMCHUNK 331
#define TOK_READ 332
#define TOK_REPEATABLE 333
#define TOK_REPLACE 334
#define TOK_RETURNS 335
#define TOK_ROLLBACK 336
#define TOK_RTINDEX 337
#define TOK_SELECT 338
#define TOK_SERIALIZABLE 339
#define TOK_SET 340
#define TOK_SESSION 341
#define TOK_SHOW 342
#define TOK_SONAME 343
#define TOK_START 344
#define TOK_STATUS 345
#define TOK_STRING 346
#define TOK_SUM 347
#define TOK_TABLE 348
#define TOK_TABLES 349
#define TOK_TO 350
#define TOK_TRANSACTION 351
#define TOK_TRUE 352
#define TOK_TRUNCATE 353
#define TOK_UNCOMMITTED 354
#define TOK_UPDATE 355
#define TOK_VALUES 356
#define TOK_VARIABLES 357
#define TOK_WARNINGS 358
#define TOK_WEIGHT 359
#define TOK_WHERE 360
#define TOK_WITHIN 361
#define TOK_OR 362
#define TOK_AND 363
#define TOK_NE 364
#define TOK_GTE 365
#define TOK_LTE 366
#define TOK_NOT 367
#define TOK_NEG 368




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





