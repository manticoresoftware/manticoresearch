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
     TOK_CHARACTER = 275,
     TOK_COLLATION = 276,
     TOK_COMMIT = 277,
     TOK_COMMITTED = 278,
     TOK_COUNT = 279,
     TOK_CREATE = 280,
     TOK_DELETE = 281,
     TOK_DESC = 282,
     TOK_DESCRIBE = 283,
     TOK_DISTINCT = 284,
     TOK_DIV = 285,
     TOK_DROP = 286,
     TOK_FALSE = 287,
     TOK_FLOAT = 288,
     TOK_FLUSH = 289,
     TOK_FROM = 290,
     TOK_FUNCTION = 291,
     TOK_GLOBAL = 292,
     TOK_GROUP = 293,
     TOK_GROUPBY = 294,
     TOK_GROUP_CONCAT = 295,
     TOK_ID = 296,
     TOK_IN = 297,
     TOK_INDEX = 298,
     TOK_INSERT = 299,
     TOK_INT = 300,
     TOK_INTO = 301,
     TOK_ISOLATION = 302,
     TOK_LEVEL = 303,
     TOK_LIMIT = 304,
     TOK_MATCH = 305,
     TOK_MAX = 306,
     TOK_META = 307,
     TOK_MIN = 308,
     TOK_MOD = 309,
     TOK_NAMES = 310,
     TOK_NULL = 311,
     TOK_OPTION = 312,
     TOK_ORDER = 313,
     TOK_OPTIMIZE = 314,
     TOK_RAND = 315,
     TOK_READ = 316,
     TOK_REPEATABLE = 317,
     TOK_REPLACE = 318,
     TOK_RETURNS = 319,
     TOK_ROLLBACK = 320,
     TOK_RTINDEX = 321,
     TOK_SELECT = 322,
     TOK_SERIALIZABLE = 323,
     TOK_SET = 324,
     TOK_SESSION = 325,
     TOK_SHOW = 326,
     TOK_SONAME = 327,
     TOK_START = 328,
     TOK_STATUS = 329,
     TOK_SUM = 330,
     TOK_TABLES = 331,
     TOK_TO = 332,
     TOK_TRANSACTION = 333,
     TOK_TRUE = 334,
     TOK_TRUNCATE = 335,
     TOK_UNCOMMITTED = 336,
     TOK_UPDATE = 337,
     TOK_VALUES = 338,
     TOK_VARIABLES = 339,
     TOK_WARNINGS = 340,
     TOK_WEIGHT = 341,
     TOK_WHERE = 342,
     TOK_WITHIN = 343,
     TOK_OR = 344,
     TOK_AND = 345,
     TOK_NE = 346,
     TOK_GTE = 347,
     TOK_LTE = 348,
     TOK_NOT = 349,
     TOK_NEG = 350
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
#define TOK_CHARACTER 275
#define TOK_COLLATION 276
#define TOK_COMMIT 277
#define TOK_COMMITTED 278
#define TOK_COUNT 279
#define TOK_CREATE 280
#define TOK_DELETE 281
#define TOK_DESC 282
#define TOK_DESCRIBE 283
#define TOK_DISTINCT 284
#define TOK_DIV 285
#define TOK_DROP 286
#define TOK_FALSE 287
#define TOK_FLOAT 288
#define TOK_FLUSH 289
#define TOK_FROM 290
#define TOK_FUNCTION 291
#define TOK_GLOBAL 292
#define TOK_GROUP 293
#define TOK_GROUPBY 294
#define TOK_GROUP_CONCAT 295
#define TOK_ID 296
#define TOK_IN 297
#define TOK_INDEX 298
#define TOK_INSERT 299
#define TOK_INT 300
#define TOK_INTO 301
#define TOK_ISOLATION 302
#define TOK_LEVEL 303
#define TOK_LIMIT 304
#define TOK_MATCH 305
#define TOK_MAX 306
#define TOK_META 307
#define TOK_MIN 308
#define TOK_MOD 309
#define TOK_NAMES 310
#define TOK_NULL 311
#define TOK_OPTION 312
#define TOK_ORDER 313
#define TOK_OPTIMIZE 314
#define TOK_RAND 315
#define TOK_READ 316
#define TOK_REPEATABLE 317
#define TOK_REPLACE 318
#define TOK_RETURNS 319
#define TOK_ROLLBACK 320
#define TOK_RTINDEX 321
#define TOK_SELECT 322
#define TOK_SERIALIZABLE 323
#define TOK_SET 324
#define TOK_SESSION 325
#define TOK_SHOW 326
#define TOK_SONAME 327
#define TOK_START 328
#define TOK_STATUS 329
#define TOK_SUM 330
#define TOK_TABLES 331
#define TOK_TO 332
#define TOK_TRANSACTION 333
#define TOK_TRUE 334
#define TOK_TRUNCATE 335
#define TOK_UNCOMMITTED 336
#define TOK_UPDATE 337
#define TOK_VALUES 338
#define TOK_VARIABLES 339
#define TOK_WARNINGS 340
#define TOK_WEIGHT 341
#define TOK_WHERE 342
#define TOK_WITHIN 343
#define TOK_OR 344
#define TOK_AND 345
#define TOK_NE 346
#define TOK_GTE 347
#define TOK_LTE 348
#define TOK_NOT 349
#define TOK_NEG 350




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





