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
     TOK_BY = 278,
     TOK_CALL = 279,
     TOK_CHARACTER = 280,
     TOK_COLLATION = 281,
     TOK_COLUMN = 282,
     TOK_COMMIT = 283,
     TOK_COMMITTED = 284,
     TOK_COUNT = 285,
     TOK_CREATE = 286,
     TOK_DELETE = 287,
     TOK_DESC = 288,
     TOK_DESCRIBE = 289,
     TOK_DISTINCT = 290,
     TOK_DIV = 291,
     TOK_DROP = 292,
     TOK_FALSE = 293,
     TOK_FLOAT = 294,
     TOK_FLUSH = 295,
     TOK_FROM = 296,
     TOK_FUNCTION = 297,
     TOK_GLOBAL = 298,
     TOK_GROUP = 299,
     TOK_GROUPBY = 300,
     TOK_GROUP_CONCAT = 301,
     TOK_ID = 302,
     TOK_IN = 303,
     TOK_INDEX = 304,
     TOK_INSERT = 305,
     TOK_INT = 306,
     TOK_INTEGER = 307,
     TOK_INTO = 308,
     TOK_ISOLATION = 309,
     TOK_LEVEL = 310,
     TOK_LIKE = 311,
     TOK_LIMIT = 312,
     TOK_MATCH = 313,
     TOK_MAX = 314,
     TOK_META = 315,
     TOK_MIN = 316,
     TOK_MOD = 317,
     TOK_NAMES = 318,
     TOK_NULL = 319,
     TOK_OPTION = 320,
     TOK_ORDER = 321,
     TOK_OPTIMIZE = 322,
     TOK_PLAN = 323,
     TOK_PROFILE = 324,
     TOK_RAND = 325,
     TOK_RAMCHUNK = 326,
     TOK_READ = 327,
     TOK_REPEATABLE = 328,
     TOK_REPLACE = 329,
     TOK_RETURNS = 330,
     TOK_ROLLBACK = 331,
     TOK_RTINDEX = 332,
     TOK_SELECT = 333,
     TOK_SERIALIZABLE = 334,
     TOK_SET = 335,
     TOK_SESSION = 336,
     TOK_SHOW = 337,
     TOK_SONAME = 338,
     TOK_START = 339,
     TOK_STATUS = 340,
     TOK_STRING = 341,
     TOK_SUM = 342,
     TOK_TABLE = 343,
     TOK_TABLES = 344,
     TOK_TO = 345,
     TOK_TRANSACTION = 346,
     TOK_TRUE = 347,
     TOK_TRUNCATE = 348,
     TOK_UNCOMMITTED = 349,
     TOK_UPDATE = 350,
     TOK_VALUES = 351,
     TOK_VARIABLES = 352,
     TOK_WARNINGS = 353,
     TOK_WEIGHT = 354,
     TOK_WHERE = 355,
     TOK_WITHIN = 356,
     TOK_OR = 357,
     TOK_AND = 358,
     TOK_NE = 359,
     TOK_GTE = 360,
     TOK_LTE = 361,
     TOK_NOT = 362,
     TOK_NEG = 363
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
#define TOK_BY 278
#define TOK_CALL 279
#define TOK_CHARACTER 280
#define TOK_COLLATION 281
#define TOK_COLUMN 282
#define TOK_COMMIT 283
#define TOK_COMMITTED 284
#define TOK_COUNT 285
#define TOK_CREATE 286
#define TOK_DELETE 287
#define TOK_DESC 288
#define TOK_DESCRIBE 289
#define TOK_DISTINCT 290
#define TOK_DIV 291
#define TOK_DROP 292
#define TOK_FALSE 293
#define TOK_FLOAT 294
#define TOK_FLUSH 295
#define TOK_FROM 296
#define TOK_FUNCTION 297
#define TOK_GLOBAL 298
#define TOK_GROUP 299
#define TOK_GROUPBY 300
#define TOK_GROUP_CONCAT 301
#define TOK_ID 302
#define TOK_IN 303
#define TOK_INDEX 304
#define TOK_INSERT 305
#define TOK_INT 306
#define TOK_INTEGER 307
#define TOK_INTO 308
#define TOK_ISOLATION 309
#define TOK_LEVEL 310
#define TOK_LIKE 311
#define TOK_LIMIT 312
#define TOK_MATCH 313
#define TOK_MAX 314
#define TOK_META 315
#define TOK_MIN 316
#define TOK_MOD 317
#define TOK_NAMES 318
#define TOK_NULL 319
#define TOK_OPTION 320
#define TOK_ORDER 321
#define TOK_OPTIMIZE 322
#define TOK_PLAN 323
#define TOK_PROFILE 324
#define TOK_RAND 325
#define TOK_RAMCHUNK 326
#define TOK_READ 327
#define TOK_REPEATABLE 328
#define TOK_REPLACE 329
#define TOK_RETURNS 330
#define TOK_ROLLBACK 331
#define TOK_RTINDEX 332
#define TOK_SELECT 333
#define TOK_SERIALIZABLE 334
#define TOK_SET 335
#define TOK_SESSION 336
#define TOK_SHOW 337
#define TOK_SONAME 338
#define TOK_START 339
#define TOK_STATUS 340
#define TOK_STRING 341
#define TOK_SUM 342
#define TOK_TABLE 343
#define TOK_TABLES 344
#define TOK_TO 345
#define TOK_TRANSACTION 346
#define TOK_TRUE 347
#define TOK_TRUNCATE 348
#define TOK_UNCOMMITTED 349
#define TOK_UPDATE 350
#define TOK_VALUES 351
#define TOK_VARIABLES 352
#define TOK_WARNINGS 353
#define TOK_WEIGHT 354
#define TOK_WHERE 355
#define TOK_WITHIN 356
#define TOK_OR 357
#define TOK_AND 358
#define TOK_NE 359
#define TOK_GTE 360
#define TOK_LTE 361
#define TOK_NOT 362
#define TOK_NEG 363




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





