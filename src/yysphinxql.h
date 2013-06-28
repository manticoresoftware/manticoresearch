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
     TOK_ADD = 268,
     TOK_AGENT = 269,
     TOK_ALTER = 270,
     TOK_AS = 271,
     TOK_ASC = 272,
     TOK_ATTACH = 273,
     TOK_AVG = 274,
     TOK_BEGIN = 275,
     TOK_BETWEEN = 276,
     TOK_BIGINT = 277,
     TOK_BOOL = 278,
     TOK_BY = 279,
     TOK_CALL = 280,
     TOK_CHARACTER = 281,
     TOK_COLLATION = 282,
     TOK_COLUMN = 283,
     TOK_COMMIT = 284,
     TOK_COMMITTED = 285,
     TOK_COUNT = 286,
     TOK_CREATE = 287,
     TOK_DELETE = 288,
     TOK_DESC = 289,
     TOK_DESCRIBE = 290,
     TOK_DISTINCT = 291,
     TOK_DIV = 292,
     TOK_DROP = 293,
     TOK_FALSE = 294,
     TOK_FLOAT = 295,
     TOK_FLUSH = 296,
     TOK_FROM = 297,
     TOK_FUNCTION = 298,
     TOK_GLOBAL = 299,
     TOK_GROUP = 300,
     TOK_GROUPBY = 301,
     TOK_GROUP_CONCAT = 302,
     TOK_ID = 303,
     TOK_IN = 304,
     TOK_INDEX = 305,
     TOK_INSERT = 306,
     TOK_INT = 307,
     TOK_INTEGER = 308,
     TOK_INTO = 309,
     TOK_ISOLATION = 310,
     TOK_LEVEL = 311,
     TOK_LIKE = 312,
     TOK_LIMIT = 313,
     TOK_MATCH = 314,
     TOK_MAX = 315,
     TOK_META = 316,
     TOK_MIN = 317,
     TOK_MOD = 318,
     TOK_NAMES = 319,
     TOK_NULL = 320,
     TOK_OPTION = 321,
     TOK_ORDER = 322,
     TOK_OPTIMIZE = 323,
     TOK_PLAN = 324,
     TOK_PROFILE = 325,
     TOK_RAND = 326,
     TOK_RAMCHUNK = 327,
     TOK_READ = 328,
     TOK_REPEATABLE = 329,
     TOK_REPLACE = 330,
     TOK_RETURNS = 331,
     TOK_ROLLBACK = 332,
     TOK_RTINDEX = 333,
     TOK_SELECT = 334,
     TOK_SERIALIZABLE = 335,
     TOK_SET = 336,
     TOK_SESSION = 337,
     TOK_SHOW = 338,
     TOK_SONAME = 339,
     TOK_START = 340,
     TOK_STATUS = 341,
     TOK_STRING = 342,
     TOK_SUM = 343,
     TOK_TABLE = 344,
     TOK_TABLES = 345,
     TOK_TO = 346,
     TOK_TRANSACTION = 347,
     TOK_TRUE = 348,
     TOK_TRUNCATE = 349,
     TOK_UNCOMMITTED = 350,
     TOK_UPDATE = 351,
     TOK_VALUES = 352,
     TOK_VARIABLES = 353,
     TOK_WARNINGS = 354,
     TOK_WEIGHT = 355,
     TOK_WHERE = 356,
     TOK_WITHIN = 357,
     TOK_OR = 358,
     TOK_AND = 359,
     TOK_NE = 360,
     TOK_GTE = 361,
     TOK_LTE = 362,
     TOK_NOT = 363,
     TOK_NEG = 364
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
#define TOK_ADD 268
#define TOK_AGENT 269
#define TOK_ALTER 270
#define TOK_AS 271
#define TOK_ASC 272
#define TOK_ATTACH 273
#define TOK_AVG 274
#define TOK_BEGIN 275
#define TOK_BETWEEN 276
#define TOK_BIGINT 277
#define TOK_BOOL 278
#define TOK_BY 279
#define TOK_CALL 280
#define TOK_CHARACTER 281
#define TOK_COLLATION 282
#define TOK_COLUMN 283
#define TOK_COMMIT 284
#define TOK_COMMITTED 285
#define TOK_COUNT 286
#define TOK_CREATE 287
#define TOK_DELETE 288
#define TOK_DESC 289
#define TOK_DESCRIBE 290
#define TOK_DISTINCT 291
#define TOK_DIV 292
#define TOK_DROP 293
#define TOK_FALSE 294
#define TOK_FLOAT 295
#define TOK_FLUSH 296
#define TOK_FROM 297
#define TOK_FUNCTION 298
#define TOK_GLOBAL 299
#define TOK_GROUP 300
#define TOK_GROUPBY 301
#define TOK_GROUP_CONCAT 302
#define TOK_ID 303
#define TOK_IN 304
#define TOK_INDEX 305
#define TOK_INSERT 306
#define TOK_INT 307
#define TOK_INTEGER 308
#define TOK_INTO 309
#define TOK_ISOLATION 310
#define TOK_LEVEL 311
#define TOK_LIKE 312
#define TOK_LIMIT 313
#define TOK_MATCH 314
#define TOK_MAX 315
#define TOK_META 316
#define TOK_MIN 317
#define TOK_MOD 318
#define TOK_NAMES 319
#define TOK_NULL 320
#define TOK_OPTION 321
#define TOK_ORDER 322
#define TOK_OPTIMIZE 323
#define TOK_PLAN 324
#define TOK_PROFILE 325
#define TOK_RAND 326
#define TOK_RAMCHUNK 327
#define TOK_READ 328
#define TOK_REPEATABLE 329
#define TOK_REPLACE 330
#define TOK_RETURNS 331
#define TOK_ROLLBACK 332
#define TOK_RTINDEX 333
#define TOK_SELECT 334
#define TOK_SERIALIZABLE 335
#define TOK_SET 336
#define TOK_SESSION 337
#define TOK_SHOW 338
#define TOK_SONAME 339
#define TOK_START 340
#define TOK_STATUS 341
#define TOK_STRING 342
#define TOK_SUM 343
#define TOK_TABLE 344
#define TOK_TABLES 345
#define TOK_TO 346
#define TOK_TRANSACTION 347
#define TOK_TRUE 348
#define TOK_TRUNCATE 349
#define TOK_UNCOMMITTED 350
#define TOK_UPDATE 351
#define TOK_VALUES 352
#define TOK_VARIABLES 353
#define TOK_WARNINGS 354
#define TOK_WEIGHT 355
#define TOK_WHERE 356
#define TOK_WITHIN 357
#define TOK_OR 358
#define TOK_AND 359
#define TOK_NE 360
#define TOK_GTE 361
#define TOK_LTE 362
#define TOK_NOT 363
#define TOK_NEG 364




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





