/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_YYSPHINXQL_H_INCLUDED
# define YY_YY_YYSPHINXQL_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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
    TOK_LTE = 385,
    TOK_GTE = 386,
    TOK_NOT = 387,
    TOK_NEG = 388
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (SqlParser_c * pParser);

#endif /* !YY_YY_YYSPHINXQL_H_INCLUDED  */
