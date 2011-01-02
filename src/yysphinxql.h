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
     TOK_CONST_INT = 259,
     TOK_CONST_FLOAT = 260,
     TOK_QUOTED_STRING = 261,
     TOK_AS = 262,
     TOK_ASC = 263,
     TOK_AVG = 264,
     TOK_BEGIN = 265,
     TOK_BETWEEN = 266,
     TOK_BY = 267,
     TOK_CALL = 268,
     TOK_COLLATION = 269,
     TOK_COMMIT = 270,
     TOK_COUNT = 271,
     TOK_DELETE = 272,
     TOK_DESC = 273,
     TOK_DESCRIBE = 274,
     TOK_DISTINCT = 275,
     TOK_FALSE = 276,
     TOK_FROM = 277,
     TOK_GLOBAL = 278,
     TOK_GROUP = 279,
     TOK_ID = 280,
     TOK_IN = 281,
     TOK_INSERT = 282,
     TOK_INTO = 283,
     TOK_LIMIT = 284,
     TOK_MATCH = 285,
     TOK_MAX = 286,
     TOK_META = 287,
     TOK_MIN = 288,
     TOK_NULL = 289,
     TOK_OPTION = 290,
     TOK_ORDER = 291,
     TOK_REPLACE = 292,
     TOK_ROLLBACK = 293,
     TOK_SELECT = 294,
     TOK_SET = 295,
     TOK_SHOW = 296,
     TOK_START = 297,
     TOK_STATUS = 298,
     TOK_SUM = 299,
     TOK_TABLES = 300,
     TOK_TRANSACTION = 301,
     TOK_TRUE = 302,
     TOK_UPDATE = 303,
     TOK_USERVAR = 304,
     TOK_VALUES = 305,
     TOK_VARIABLES = 306,
     TOK_WARNINGS = 307,
     TOK_WEIGHT = 308,
     TOK_WHERE = 309,
     TOK_WITHIN = 310,
     TOK_OR = 311,
     TOK_AND = 312,
     TOK_NE = 313,
     TOK_GTE = 314,
     TOK_LTE = 315,
     TOK_NOT = 316,
     TOK_NEG = 317
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST_INT 259
#define TOK_CONST_FLOAT 260
#define TOK_QUOTED_STRING 261
#define TOK_AS 262
#define TOK_ASC 263
#define TOK_AVG 264
#define TOK_BEGIN 265
#define TOK_BETWEEN 266
#define TOK_BY 267
#define TOK_CALL 268
#define TOK_COLLATION 269
#define TOK_COMMIT 270
#define TOK_COUNT 271
#define TOK_DELETE 272
#define TOK_DESC 273
#define TOK_DESCRIBE 274
#define TOK_DISTINCT 275
#define TOK_FALSE 276
#define TOK_FROM 277
#define TOK_GLOBAL 278
#define TOK_GROUP 279
#define TOK_ID 280
#define TOK_IN 281
#define TOK_INSERT 282
#define TOK_INTO 283
#define TOK_LIMIT 284
#define TOK_MATCH 285
#define TOK_MAX 286
#define TOK_META 287
#define TOK_MIN 288
#define TOK_NULL 289
#define TOK_OPTION 290
#define TOK_ORDER 291
#define TOK_REPLACE 292
#define TOK_ROLLBACK 293
#define TOK_SELECT 294
#define TOK_SET 295
#define TOK_SHOW 296
#define TOK_START 297
#define TOK_STATUS 298
#define TOK_SUM 299
#define TOK_TABLES 300
#define TOK_TRANSACTION 301
#define TOK_TRUE 302
#define TOK_UPDATE 303
#define TOK_USERVAR 304
#define TOK_VALUES 305
#define TOK_VARIABLES 306
#define TOK_WARNINGS 307
#define TOK_WEIGHT 308
#define TOK_WHERE 309
#define TOK_WITHIN 310
#define TOK_OR 311
#define TOK_AND 312
#define TOK_NE 313
#define TOK_GTE 314
#define TOK_LTE 315
#define TOK_NOT 316
#define TOK_NEG 317




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





