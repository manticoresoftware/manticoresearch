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
    TOK_AS = 273,
    TOK_ASC = 274,
    TOK_ATTACH = 275,
    TOK_ATTRIBUTES = 276,
    TOK_AVG = 277,
    TOK_BEGIN = 278,
    TOK_BETWEEN = 279,
    TOK_BIGINT = 280,
    TOK_BOOL = 281,
    TOK_BY = 282,
    TOK_CALL = 283,
    TOK_CHARACTER = 284,
    TOK_CHUNK = 285,
    TOK_COLLATION = 286,
    TOK_COLUMN = 287,
    TOK_COMMIT = 288,
    TOK_COMMITTED = 289,
    TOK_COUNT = 290,
    TOK_CREATE = 291,
    TOK_DATABASES = 292,
    TOK_DELETE = 293,
    TOK_DESC = 294,
    TOK_DESCRIBE = 295,
    TOK_DISTINCT = 296,
    TOK_DIV = 297,
    TOK_DOUBLE = 298,
    TOK_DROP = 299,
    TOK_FACET = 300,
    TOK_FALSE = 301,
    TOK_FLOAT = 302,
    TOK_FLUSH = 303,
    TOK_FOR = 304,
    TOK_FROM = 305,
    TOK_FUNCTION = 306,
    TOK_GLOBAL = 307,
    TOK_GROUP = 308,
    TOK_GROUPBY = 309,
    TOK_GROUP_CONCAT = 310,
    TOK_HAVING = 311,
    TOK_ID = 312,
    TOK_IN = 313,
    TOK_INDEX = 314,
    TOK_INSERT = 315,
    TOK_INT = 316,
    TOK_INTEGER = 317,
    TOK_INTO = 318,
    TOK_IS = 319,
    TOK_ISOLATION = 320,
    TOK_JSON = 321,
    TOK_LEVEL = 322,
    TOK_LIKE = 323,
    TOK_LIMIT = 324,
    TOK_MATCH = 325,
    TOK_MAX = 326,
    TOK_META = 327,
    TOK_MIN = 328,
    TOK_MOD = 329,
    TOK_MULTI = 330,
    TOK_MULTI64 = 331,
    TOK_NAMES = 332,
    TOK_NULL = 333,
    TOK_OPTION = 334,
    TOK_ORDER = 335,
    TOK_OPTIMIZE = 336,
    TOK_PLAN = 337,
    TOK_PLUGIN = 338,
    TOK_PLUGINS = 339,
    TOK_PROFILE = 340,
    TOK_RAND = 341,
    TOK_RAMCHUNK = 342,
    TOK_READ = 343,
    TOK_RECONFIGURE = 344,
    TOK_REPEATABLE = 345,
    TOK_REPLACE = 346,
    TOK_REMAP = 347,
    TOK_RETURNS = 348,
    TOK_ROLLBACK = 349,
    TOK_RTINDEX = 350,
    TOK_SELECT = 351,
    TOK_SERIALIZABLE = 352,
    TOK_SET = 353,
    TOK_SETTINGS = 354,
    TOK_SESSION = 355,
    TOK_SHOW = 356,
    TOK_SONAME = 357,
    TOK_START = 358,
    TOK_STATUS = 359,
    TOK_STRING = 360,
    TOK_SUM = 361,
    TOK_TABLE = 362,
    TOK_TABLES = 363,
    TOK_THREADS = 364,
    TOK_TO = 365,
    TOK_TRANSACTION = 366,
    TOK_TRUE = 367,
    TOK_TRUNCATE = 368,
    TOK_TYPE = 369,
    TOK_UNCOMMITTED = 370,
    TOK_UPDATE = 371,
    TOK_VALUES = 372,
    TOK_VARIABLES = 373,
    TOK_WARNINGS = 374,
    TOK_WEIGHT = 375,
    TOK_WHERE = 376,
    TOK_WITHIN = 377,
    TOK_OR = 378,
    TOK_AND = 379,
    TOK_NE = 380,
    TOK_LTE = 381,
    TOK_GTE = 382,
    TOK_NOT = 383,
    TOK_NEG = 384
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
