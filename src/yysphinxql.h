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
     TOK_AGENT = 268,
     TOK_AS = 269,
     TOK_ASC = 270,
     TOK_ATTACH = 271,
     TOK_AVG = 272,
     TOK_BEGIN = 273,
     TOK_BETWEEN = 274,
     TOK_BY = 275,
     TOK_CALL = 276,
     TOK_CHARACTER = 277,
     TOK_COLLATION = 278,
     TOK_COMMIT = 279,
     TOK_COMMITTED = 280,
     TOK_COUNT = 281,
     TOK_CREATE = 282,
     TOK_DELETE = 283,
     TOK_DESC = 284,
     TOK_DESCRIBE = 285,
     TOK_DISTINCT = 286,
     TOK_DIV = 287,
     TOK_DROP = 288,
     TOK_FALSE = 289,
     TOK_FLOAT = 290,
     TOK_FLUSH = 291,
     TOK_FROM = 292,
     TOK_FUNCTION = 293,
     TOK_GLOBAL = 294,
     TOK_GROUP = 295,
     TOK_GROUPBY = 296,
     TOK_GROUP_CONCAT = 297,
     TOK_ID = 298,
     TOK_IN = 299,
     TOK_INDEX = 300,
     TOK_INSERT = 301,
     TOK_INT = 302,
     TOK_INTO = 303,
     TOK_ISOLATION = 304,
     TOK_LEVEL = 305,
     TOK_LIKE = 306,
     TOK_LIMIT = 307,
     TOK_MATCH = 308,
     TOK_MAX = 309,
     TOK_META = 310,
     TOK_MIN = 311,
     TOK_MOD = 312,
     TOK_NAMES = 313,
     TOK_NULL = 314,
     TOK_OPTION = 315,
     TOK_ORDER = 316,
     TOK_OPTIMIZE = 317,
     TOK_PROFILE = 318,
     TOK_RAND = 319,
     TOK_READ = 320,
     TOK_REPEATABLE = 321,
     TOK_REPLACE = 322,
     TOK_RETURNS = 323,
     TOK_ROLLBACK = 324,
     TOK_RTINDEX = 325,
     TOK_SELECT = 326,
     TOK_SERIALIZABLE = 327,
     TOK_SET = 328,
     TOK_SESSION = 329,
     TOK_SHOW = 330,
     TOK_SONAME = 331,
     TOK_START = 332,
     TOK_STATUS = 333,
     TOK_STRING = 334,
     TOK_SUM = 335,
     TOK_TABLES = 336,
     TOK_TO = 337,
     TOK_TRANSACTION = 338,
     TOK_TRUE = 339,
     TOK_TRUNCATE = 340,
     TOK_UNCOMMITTED = 341,
     TOK_UPDATE = 342,
     TOK_VALUES = 343,
     TOK_VARIABLES = 344,
     TOK_WARNINGS = 345,
     TOK_WEIGHT = 346,
     TOK_WHERE = 347,
     TOK_WITHIN = 348,
     TOK_OR = 349,
     TOK_AND = 350,
     TOK_NE = 351,
     TOK_GTE = 352,
     TOK_LTE = 353,
     TOK_NOT = 354,
     TOK_NEG = 355
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
#define TOK_AGENT 268
#define TOK_AS 269
#define TOK_ASC 270
#define TOK_ATTACH 271
#define TOK_AVG 272
#define TOK_BEGIN 273
#define TOK_BETWEEN 274
#define TOK_BY 275
#define TOK_CALL 276
#define TOK_CHARACTER 277
#define TOK_COLLATION 278
#define TOK_COMMIT 279
#define TOK_COMMITTED 280
#define TOK_COUNT 281
#define TOK_CREATE 282
#define TOK_DELETE 283
#define TOK_DESC 284
#define TOK_DESCRIBE 285
#define TOK_DISTINCT 286
#define TOK_DIV 287
#define TOK_DROP 288
#define TOK_FALSE 289
#define TOK_FLOAT 290
#define TOK_FLUSH 291
#define TOK_FROM 292
#define TOK_FUNCTION 293
#define TOK_GLOBAL 294
#define TOK_GROUP 295
#define TOK_GROUPBY 296
#define TOK_GROUP_CONCAT 297
#define TOK_ID 298
#define TOK_IN 299
#define TOK_INDEX 300
#define TOK_INSERT 301
#define TOK_INT 302
#define TOK_INTO 303
#define TOK_ISOLATION 304
#define TOK_LEVEL 305
#define TOK_LIKE 306
#define TOK_LIMIT 307
#define TOK_MATCH 308
#define TOK_MAX 309
#define TOK_META 310
#define TOK_MIN 311
#define TOK_MOD 312
#define TOK_NAMES 313
#define TOK_NULL 314
#define TOK_OPTION 315
#define TOK_ORDER 316
#define TOK_OPTIMIZE 317
#define TOK_PROFILE 318
#define TOK_RAND 319
#define TOK_READ 320
#define TOK_REPEATABLE 321
#define TOK_REPLACE 322
#define TOK_RETURNS 323
#define TOK_ROLLBACK 324
#define TOK_RTINDEX 325
#define TOK_SELECT 326
#define TOK_SERIALIZABLE 327
#define TOK_SET 328
#define TOK_SESSION 329
#define TOK_SHOW 330
#define TOK_SONAME 331
#define TOK_START 332
#define TOK_STATUS 333
#define TOK_STRING 334
#define TOK_SUM 335
#define TOK_TABLES 336
#define TOK_TO 337
#define TOK_TRANSACTION 338
#define TOK_TRUE 339
#define TOK_TRUNCATE 340
#define TOK_UNCOMMITTED 341
#define TOK_UPDATE 342
#define TOK_VALUES 343
#define TOK_VARIABLES 344
#define TOK_WARNINGS 345
#define TOK_WEIGHT 346
#define TOK_WHERE 347
#define TOK_WITHIN 348
#define TOK_OR 349
#define TOK_AND 350
#define TOK_NE 351
#define TOK_GTE 352
#define TOK_LTE 353
#define TOK_NOT 354
#define TOK_NEG 355




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





