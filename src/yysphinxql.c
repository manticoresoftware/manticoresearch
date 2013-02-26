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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif


// some helpers
#include <float.h> // for FLT_MAX



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */


#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  107
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   934

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  118
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  99
/* YYNRULES -- Number of rules. */
#define YYNRULES  276
/* YYNRULES -- Number of states. */
#define YYNSTATES  529

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   355

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   108,    97,     2,
     112,   113,   106,   104,   114,   105,   117,   107,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   111,
     100,    98,   101,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   115,    96,   116,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    99,   102,   103,   109,   110
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    56,    58,    60,
      62,    72,    73,    77,    78,    81,    86,    97,    99,   103,
     105,   108,   109,   111,   114,   116,   121,   126,   131,   136,
     141,   146,   150,   154,   160,   162,   166,   167,   169,   172,
     174,   178,   182,   187,   191,   195,   201,   208,   212,   217,
     223,   227,   231,   235,   239,   241,   247,   251,   255,   259,
     263,   267,   271,   275,   279,   281,   283,   288,   292,   296,
     298,   300,   303,   305,   308,   310,   314,   315,   317,   321,
     322,   324,   330,   331,   333,   337,   343,   345,   349,   351,
     354,   357,   358,   360,   363,   368,   369,   371,   374,   376,
     380,   384,   388,   394,   401,   405,   407,   411,   415,   417,
     419,   421,   423,   425,   427,   430,   433,   437,   441,   445,
     449,   453,   457,   461,   465,   469,   473,   477,   481,   485,
     489,   493,   497,   501,   505,   509,   511,   516,   521,   525,
     532,   539,   541,   545,   547,   549,   553,   559,   562,   563,
     566,   568,   571,   574,   578,   580,   585,   590,   594,   596,
     598,   600,   602,   604,   606,   610,   615,   620,   625,   629,
     634,   642,   648,   650,   652,   654,   656,   658,   660,   662,
     664,   666,   669,   676,   678,   680,   681,   685,   687,   691,
     693,   697,   701,   703,   707,   709,   711,   713,   717,   720,
     728,   738,   745,   747,   751,   753,   757,   759,   763,   764,
     767,   769,   773,   777,   778,   780,   782,   784,   788,   790,
     792,   796,   803,   805,   809,   813,   817,   823,   828,   833,
     834,   836,   839,   841,   845,   849,   852,   856,   863,   864,
     866,   868,   871,   874,   877,   879,   887,   889,   891,   893,
     897,   904,   908,   912,   914,   918,   922
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     119,     0,    -1,   120,    -1,   121,    -1,   121,   111,    -1,
     174,    -1,   182,    -1,   168,    -1,   169,    -1,   172,    -1,
     183,    -1,   192,    -1,   194,    -1,   195,    -1,   198,    -1,
     203,    -1,   204,    -1,   208,    -1,   210,    -1,   211,    -1,
     212,    -1,   205,    -1,   213,    -1,   215,    -1,   216,    -1,
     122,    -1,   121,   111,   122,    -1,   123,    -1,   163,    -1,
     127,    -1,    71,   128,    37,   112,   124,   127,   113,   125,
     126,    -1,    -1,    61,    20,   148,    -1,    -1,    52,     5,
      -1,    52,     5,   114,     5,    -1,    71,   128,    37,   132,
     133,   142,   144,   146,   150,   152,    -1,   129,    -1,   128,
     114,   129,    -1,   106,    -1,   131,   130,    -1,    -1,     3,
      -1,    14,     3,    -1,   158,    -1,    17,   112,   158,   113,
      -1,    54,   112,   158,   113,    -1,    56,   112,   158,   113,
      -1,    80,   112,   158,   113,    -1,    42,   112,   158,   113,
      -1,    26,   112,   106,   113,    -1,    91,   112,   113,    -1,
      41,   112,   113,    -1,    26,   112,    31,     3,   113,    -1,
       3,    -1,   132,   114,     3,    -1,    -1,   134,    -1,    92,
     135,    -1,   136,    -1,   135,    95,   135,    -1,   112,   135,
     113,    -1,    53,   112,     8,   113,    -1,   138,    98,   139,
      -1,   138,    99,   139,    -1,   138,    44,   112,   141,   113,
      -1,   138,   109,    44,   112,   141,   113,    -1,   138,    44,
       9,    -1,   138,   109,    44,     9,    -1,   138,    19,   139,
      95,   139,    -1,   138,   101,   139,    -1,   138,   100,   139,
      -1,   138,   102,   139,    -1,   138,   103,   139,    -1,   137,
      -1,   138,    19,   140,    95,   140,    -1,   138,   101,   140,
      -1,   138,   100,   140,    -1,   138,   102,   140,    -1,   138,
     103,   140,    -1,   138,    98,     8,    -1,   138,    99,     8,
      -1,   138,    98,   140,    -1,   138,    99,   140,    -1,     3,
      -1,     4,    -1,    26,   112,   106,   113,    -1,    41,   112,
     113,    -1,    91,   112,   113,    -1,    43,    -1,     5,    -1,
     105,     5,    -1,     6,    -1,   105,     6,    -1,   139,    -1,
     141,   114,   139,    -1,    -1,   143,    -1,    40,    20,   138,
      -1,    -1,   145,    -1,    93,    40,    61,    20,   148,    -1,
      -1,   147,    -1,    61,    20,   148,    -1,    61,    20,    64,
     112,   113,    -1,   149,    -1,   148,   114,   149,    -1,   138,
      -1,   138,    15,    -1,   138,    29,    -1,    -1,   151,    -1,
      52,     5,    -1,    52,     5,   114,     5,    -1,    -1,   153,
      -1,    60,   154,    -1,   155,    -1,   154,   114,   155,    -1,
       3,    98,     3,    -1,     3,    98,     5,    -1,     3,    98,
     112,   156,   113,    -1,     3,    98,     3,   112,     8,   113,
      -1,     3,    98,     8,    -1,   157,    -1,   156,   114,   157,
      -1,     3,    98,   139,    -1,     3,    -1,     4,    -1,    43,
      -1,     5,    -1,     6,    -1,     9,    -1,   105,   158,    -1,
     109,   158,    -1,   158,   104,   158,    -1,   158,   105,   158,
      -1,   158,   106,   158,    -1,   158,   107,   158,    -1,   158,
     100,   158,    -1,   158,   101,   158,    -1,   158,    97,   158,
      -1,   158,    96,   158,    -1,   158,   108,   158,    -1,   158,
      32,   158,    -1,   158,    57,   158,    -1,   158,   103,   158,
      -1,   158,   102,   158,    -1,   158,    98,   158,    -1,   158,
      99,   158,    -1,   158,    95,   158,    -1,   158,    94,   158,
      -1,   112,   158,   113,    -1,   115,   162,   116,    -1,   159,
      -1,     3,   112,   160,   113,    -1,    44,   112,   160,   113,
      -1,     3,   112,   113,    -1,    56,   112,   158,   114,   158,
     113,    -1,    54,   112,   158,   114,   158,   113,    -1,   161,
      -1,   160,   114,   161,    -1,   158,    -1,     8,    -1,     3,
      98,   139,    -1,   162,   114,     3,    98,   139,    -1,    75,
     165,    -1,    -1,    51,     8,    -1,    90,    -1,    78,   164,
      -1,    55,   164,    -1,    13,    78,   164,    -1,    63,    -1,
      13,     8,    78,   164,    -1,    13,     3,    78,   164,    -1,
      45,     3,    78,    -1,     3,    -1,    59,    -1,     8,    -1,
       5,    -1,     6,    -1,   166,    -1,   167,   105,   166,    -1,
      73,     3,    98,   171,    -1,    73,     3,    98,   170,    -1,
      73,     3,    98,    59,    -1,    73,    58,   167,    -1,    73,
      10,    98,   167,    -1,    73,    39,     9,    98,   112,   141,
     113,    -1,    73,    39,     3,    98,   170,    -1,     3,    -1,
       8,    -1,    84,    -1,    34,    -1,   139,    -1,    24,    -1,
      69,    -1,   173,    -1,    18,    -1,    77,    83,    -1,   175,
      48,     3,   176,    88,   178,    -1,    46,    -1,    67,    -1,
      -1,   112,   177,   113,    -1,   138,    -1,   177,   114,   138,
      -1,   179,    -1,   178,   114,   179,    -1,   112,   180,   113,
      -1,   181,    -1,   180,   114,   181,    -1,   139,    -1,   140,
      -1,     8,    -1,   112,   141,   113,    -1,   112,   113,    -1,
      28,    37,   132,    92,    43,    98,   139,    -1,    28,    37,
     132,    92,    43,    44,   112,   141,   113,    -1,    21,     3,
     112,   184,   187,   113,    -1,   185,    -1,   184,   114,   185,
      -1,   181,    -1,   112,   186,   113,    -1,     8,    -1,   186,
     114,     8,    -1,    -1,   114,   188,    -1,   189,    -1,   188,
     114,   189,    -1,   181,   190,   191,    -1,    -1,    14,    -1,
       3,    -1,    52,    -1,   193,     3,   164,    -1,    30,    -1,
      29,    -1,    75,    81,   164,    -1,    87,   132,    73,   196,
     134,   152,    -1,   197,    -1,   196,   114,   197,    -1,     3,
      98,   139,    -1,     3,    98,   140,    -1,     3,    98,   112,
     141,   113,    -1,     3,    98,   112,   113,    -1,    75,   206,
      89,   199,    -1,    -1,   200,    -1,    92,   201,    -1,   202,
      -1,   201,    94,   202,    -1,     3,    98,     8,    -1,    75,
      23,    -1,    75,    22,    73,    -1,    73,   206,    83,    49,
      50,   207,    -1,    -1,    39,    -1,    74,    -1,    65,    86,
      -1,    65,    25,    -1,    66,    65,    -1,    72,    -1,    27,
      38,     3,    68,   209,    76,     8,    -1,    47,    -1,    35,
      -1,    79,    -1,    33,    38,     3,    -1,    16,    45,     3,
      82,    70,     3,    -1,    36,    70,     3,    -1,    71,   214,
     150,    -1,    10,    -1,    10,   117,     3,    -1,    85,    70,
       3,    -1,    62,    45,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   129,   129,   130,   131,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   160,   161,   165,   166,   170,
     171,   179,   192,   200,   202,   207,   216,   232,   233,   237,
     238,   241,   243,   244,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   261,   262,   265,   267,   271,   275,
     276,   277,   281,   286,   293,   301,   309,   318,   323,   328,
     333,   338,   343,   348,   353,   358,   363,   368,   373,   378,
     383,   388,   396,   397,   401,   402,   407,   413,   419,   425,
     434,   435,   446,   447,   451,   457,   463,   465,   469,   475,
     477,   481,   492,   494,   498,   502,   509,   510,   514,   515,
     516,   519,   521,   525,   530,   537,   539,   543,   547,   548,
     552,   557,   562,   568,   573,   581,   586,   593,   603,   604,
     605,   606,   607,   608,   609,   610,   611,   612,   613,   614,
     615,   616,   617,   618,   619,   620,   621,   622,   623,   624,
     625,   626,   627,   628,   629,   630,   634,   635,   636,   637,
     638,   642,   643,   647,   648,   652,   653,   659,   662,   664,
     668,   669,   670,   671,   672,   673,   678,   683,   693,   694,
     695,   696,   697,   701,   702,   706,   711,   716,   721,   722,
     726,   731,   739,   740,   744,   745,   746,   760,   761,   762,
     766,   767,   773,   781,   782,   785,   787,   791,   792,   796,
     797,   801,   805,   806,   810,   811,   812,   813,   814,   820,
     828,   842,   850,   854,   861,   862,   869,   879,   885,   887,
     891,   896,   900,   907,   909,   913,   914,   920,   928,   929,
     935,   941,   949,   950,   954,   958,   962,   966,   976,   982,
     983,   987,   991,   992,   996,  1000,  1007,  1014,  1020,  1021,
    1022,  1026,  1027,  1028,  1029,  1035,  1046,  1047,  1048,  1052,
    1063,  1075,  1086,  1094,  1095,  1104,  1115
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_ATIDENT", 
  "TOK_CONST_INT", "TOK_CONST_FLOAT", "TOK_CONST_MVA", 
  "TOK_QUOTED_STRING", "TOK_USERVAR", "TOK_SYSVAR", "TOK_CONST_STRINGS", 
  "TOK_BAD_NUMERIC", "TOK_AGENT", "TOK_AS", "TOK_ASC", "TOK_ATTACH", 
  "TOK_AVG", "TOK_BEGIN", "TOK_BETWEEN", "TOK_BY", "TOK_CALL", 
  "TOK_CHARACTER", "TOK_COLLATION", "TOK_COMMIT", "TOK_COMMITTED", 
  "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", 
  "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", 
  "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_ISOLATION", "TOK_LEVEL", 
  "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", 
  "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_OPTIMIZE", "TOK_PROFILE", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", 
  "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", 
  "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", 
  "';'", "'('", "')'", "','", "'{'", "'}'", "'.'", "$accept", "request", 
  "statement", "multi_stmt_list", "multi_stmt", "select", 
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "expr_float_unhandled", "expr_ident", "const_int", 
  "const_float", "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "consthash", "show_stmt", "like_filter", "show_what", 
  "simple_set_value", "set_value", "set_stmt", "set_global_stmt", 
  "set_string_value", "boolean_value", "transact_op", "start_transaction", 
  "insert_into", "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", "show_variables", 
  "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "select_sysvar", "sysvar_name", 
  "truncate", "optimize_index", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   124,    38,    61,   351,
      60,    62,   352,   353,    43,    45,    42,    47,    37,   354,
     355,    59,    40,    41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   118,   119,   119,   119,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   121,   121,   122,   122,   123,
     123,   124,   125,   126,   126,   126,   127,   128,   128,   129,
     129,   130,   130,   130,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   132,   132,   133,   133,   134,   135,
     135,   135,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   137,   137,   138,   138,   138,   138,   138,   138,
     139,   139,   140,   140,   141,   141,   142,   142,   143,   144,
     144,   145,   146,   146,   147,   147,   148,   148,   149,   149,
     149,   150,   150,   151,   151,   152,   152,   153,   154,   154,
     155,   155,   155,   155,   155,   156,   156,   157,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   159,   159,   159,   159,
     159,   160,   160,   161,   161,   162,   162,   163,   164,   164,
     165,   165,   165,   165,   165,   165,   165,   165,   166,   166,
     166,   166,   166,   167,   167,   168,   168,   168,   168,   168,
     169,   169,   170,   170,   171,   171,   171,   172,   172,   172,
     173,   173,   174,   175,   175,   176,   176,   177,   177,   178,
     178,   179,   180,   180,   181,   181,   181,   181,   181,   182,
     182,   183,   184,   184,   185,   185,   186,   186,   187,   187,
     188,   188,   189,   190,   190,   191,   191,   192,   193,   193,
     194,   195,   196,   196,   197,   197,   197,   197,   198,   199,
     199,   200,   201,   201,   202,   203,   204,   205,   206,   206,
     206,   207,   207,   207,   207,   208,   209,   209,   209,   210,
     211,   212,   213,   214,   214,   215,   216
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       9,     0,     3,     0,     2,     4,    10,     1,     3,     1,
       2,     0,     1,     2,     1,     4,     4,     4,     4,     4,
       4,     3,     3,     5,     1,     3,     0,     1,     2,     1,
       3,     3,     4,     3,     3,     5,     6,     3,     4,     5,
       3,     3,     3,     3,     1,     5,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     4,     3,     3,     1,
       1,     2,     1,     2,     1,     3,     0,     1,     3,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     6,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     4,     4,     3,     6,
       6,     1,     3,     1,     1,     3,     5,     2,     0,     2,
       1,     2,     2,     3,     1,     4,     4,     3,     1,     1,
       1,     1,     1,     1,     3,     4,     4,     4,     3,     4,
       7,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     6,     1,     1,     0,     3,     1,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     3,     2,     7,
       9,     6,     1,     3,     1,     3,     1,     3,     0,     2,
       1,     3,     3,     0,     1,     1,     1,     3,     1,     1,
       3,     6,     1,     3,     3,     3,     5,     4,     4,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     3,
       6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   200,     0,   197,     0,     0,   239,   238,     0,
       0,   203,     0,   204,   198,     0,   258,   258,     0,     0,
       0,     0,     2,     3,    25,    27,    29,    28,     7,     8,
       9,   199,     5,     0,     6,    10,    11,     0,    12,    13,
      14,    15,    16,    21,    17,    18,    19,    20,    22,    23,
      24,     0,     0,     0,     0,     0,     0,     0,   128,   129,
     131,   132,   133,   273,     0,     0,     0,     0,   130,     0,
       0,     0,     0,     0,     0,    39,     0,     0,     0,     0,
      37,    41,    44,   155,   111,     0,     0,   259,     0,   260,
       0,     0,     0,   255,   259,     0,   168,   174,   168,   168,
     170,   167,     0,   201,     0,    54,     0,     1,     4,     0,
     168,     0,     0,     0,     0,   269,   271,   276,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   134,   135,     0,     0,     0,     0,     0,    42,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   272,
     112,     0,     0,     0,     0,   178,   181,   182,   180,   179,
     183,   188,     0,     0,     0,   168,   256,     0,     0,   172,
     171,   240,   249,   275,     0,     0,     0,     0,    26,   205,
     237,     0,    90,    92,   216,     0,     0,   214,   215,   224,
     228,   222,     0,     0,   164,   158,   163,     0,   161,   274,
       0,     0,     0,    52,     0,     0,     0,     0,     0,    51,
       0,     0,   153,     0,     0,   154,    31,    56,    38,    43,
     145,   146,   152,   151,   143,   142,   149,   150,   140,   141,
     148,   147,   136,   137,   138,   139,   144,   113,   192,   193,
     195,   187,   194,     0,   196,   186,   185,   189,     0,     0,
       0,     0,   168,   168,   173,   177,   169,     0,   248,   250,
       0,     0,   242,    55,     0,     0,     0,    91,    93,   226,
     218,    94,     0,     0,     0,     0,   267,   266,   268,     0,
       0,   156,     0,    45,     0,    50,    49,   157,    46,     0,
      47,     0,    48,     0,     0,   165,     0,     0,     0,    96,
      57,     0,   191,     0,   184,     0,   176,   175,     0,   251,
     252,     0,     0,   115,    84,    85,     0,     0,    89,     0,
     207,     0,     0,   270,   217,     0,   225,     0,   224,   223,
     229,   230,   221,     0,     0,     0,   162,    53,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    74,     0,     0,
      99,    97,   114,     0,     0,     0,   264,   257,     0,     0,
       0,   244,   245,   243,     0,   241,   116,     0,     0,     0,
     206,     0,     0,   202,   209,    95,   227,   234,     0,     0,
     265,     0,   219,   160,   159,   166,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   102,   100,   190,   262,   261,   263,   254,   253,
     247,     0,     0,   117,   118,     0,    87,    88,   208,     0,
       0,   212,     0,   235,   236,   232,   233,   231,     0,     0,
       0,    33,     0,    61,    60,     0,     0,    67,     0,    80,
      63,    82,    81,    64,    83,    71,    77,    70,    76,    72,
      78,    73,    79,     0,    98,     0,     0,   111,   103,   246,
       0,     0,    86,   211,     0,   210,   220,     0,     0,    30,
      62,     0,     0,     0,    68,     0,     0,     0,   115,   120,
     121,   124,     0,   119,   213,   108,    32,   106,    34,    69,
       0,    75,    65,     0,     0,     0,   104,    36,     0,     0,
       0,   125,   109,   110,     0,     0,    66,   101,     0,     0,
       0,   122,     0,   107,    35,   105,   123,   127,   126
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   307,   441,   479,    26,
      79,    80,   140,    81,   227,   309,   310,   355,   356,   357,
     495,   281,   198,   282,   360,   361,   412,   413,   467,   468,
     496,   497,   159,   160,   375,   376,   423,   424,   510,   511,
      82,    83,   207,   208,   135,    27,   179,   101,   170,   171,
      28,    29,   255,   256,    30,    31,    32,    33,   275,   331,
     383,   384,   430,   199,    34,    35,   200,   201,   283,   285,
     340,   341,   388,   435,    36,    37,    38,    39,   271,   272,
      40,   268,   269,   319,   320,    41,    42,    43,    90,   367,
      44,   289,    45,    46,    47,    48,    84,    49,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -365
static const short yypact[] =
{
     847,   -27,  -365,    43,  -365,     1,    96,  -365,  -365,   105,
     100,  -365,   149,  -365,  -365,   109,   271,   630,   115,   147,
     224,   220,  -365,   125,  -365,  -365,  -365,  -365,  -365,  -365,
    -365,  -365,  -365,   202,  -365,  -365,  -365,   251,  -365,  -365,
    -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,
    -365,   253,   145,   258,   224,   263,   266,   275,   168,  -365,
    -365,  -365,  -365,   181,   197,   200,   209,   214,  -365,   222,
     227,   228,   231,   232,   279,  -365,   279,   279,   329,   -18,
    -365,   163,   638,  -365,   294,   249,   255,   175,   180,  -365,
     268,   102,   276,  -365,  -365,   357,   310,  -365,   310,   310,
    -365,  -365,   277,  -365,   367,  -365,    25,  -365,   -13,   369,
     310,   291,    26,   307,   -49,  -365,  -365,  -365,   243,   374,
     279,    -5,   265,   279,   259,   279,   279,   279,   269,   273,
     274,  -365,  -365,   429,   283,    85,     9,   199,  -365,   380,
    -365,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   382,  -365,
    -365,   153,   180,   292,   295,  -365,  -365,  -365,  -365,  -365,
    -365,   284,   343,   325,   326,   310,  -365,   327,   394,  -365,
    -365,  -365,   340,  -365,   430,   434,   199,   631,  -365,   341,
    -365,   384,  -365,  -365,  -365,   205,    12,  -365,  -365,  -365,
     345,  -365,   -20,   412,  -365,  -365,   638,   116,  -365,  -365,
     455,   457,   349,  -365,   482,   118,   322,   344,   508,  -365,
     279,   279,  -365,    17,   460,  -365,  -365,    76,  -365,  -365,
    -365,  -365,   655,   668,   690,   745,   754,   754,   306,   306,
     306,   306,   165,   165,  -365,  -365,  -365,   350,  -365,  -365,
    -365,  -365,  -365,   461,  -365,  -365,  -365,   284,   188,   353,
     180,   417,   310,   310,  -365,  -365,  -365,   465,  -365,  -365,
     371,    79,  -365,  -365,   324,   400,   467,  -365,  -365,  -365,
    -365,  -365,   121,   131,    26,   376,  -365,  -365,  -365,   414,
     -11,  -365,   259,  -365,   379,  -365,  -365,  -365,  -365,   279,
    -365,   279,  -365,   377,   399,  -365,   410,   438,    63,   470,
    -365,   506,  -365,    17,  -365,   141,  -365,  -365,   418,   421,
    -365,    50,   430,   458,  -365,  -365,   405,   407,  -365,   408,
    -365,   146,   409,  -365,  -365,    17,  -365,   514,   143,  -365,
     424,  -365,  -365,   533,   431,    17,  -365,  -365,   534,   560,
      17,   199,   432,   435,    63,   449,  -365,  -365,   297,   526,
     471,  -365,  -365,   162,    11,   483,  -365,  -365,   559,   465,
      19,  -365,  -365,  -365,   566,  -365,  -365,   464,   459,   462,
    -365,   324,    32,   479,  -365,  -365,  -365,  -365,   124,    32,
    -365,    17,  -365,  -365,  -365,  -365,   -12,   510,   565,    69,
      63,    24,    -4,    36,    55,    24,    24,    24,    24,   530,
     324,   554,   535,  -365,  -365,  -365,  -365,  -365,  -365,  -365,
    -365,   187,   499,   484,  -365,   486,  -365,  -365,  -365,    23,
     193,  -365,   409,  -365,  -365,  -365,   586,  -365,   204,   224,
     581,   567,   505,  -365,  -365,   525,   527,  -365,    17,  -365,
    -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,
    -365,  -365,  -365,    -3,  -365,   562,   604,   294,  -365,  -365,
       8,   566,  -365,  -365,    32,  -365,  -365,   324,   620,  -365,
    -365,    17,    29,   211,  -365,    17,   606,   316,   458,   515,
    -365,  -365,   642,  -365,  -365,   152,   532,  -365,   536,  -365,
     643,  -365,  -365,   217,   324,   539,   532,  -365,   640,   573,
     223,  -365,  -365,  -365,   324,   667,  -365,   532,   561,   564,
      17,  -365,   642,  -365,  -365,  -365,  -365,  -365,  -365
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -365,  -365,  -365,  -365,   570,  -365,  -365,  -365,  -365,   372,
     330,   543,  -365,  -365,    -6,  -365,   411,  -255,  -365,  -365,
    -251,  -112,  -313,  -306,  -365,  -365,  -365,  -365,  -365,  -365,
    -364,   169,   221,  -365,   196,  -365,  -365,   218,  -365,   170,
     -73,  -365,   572,   398,  -365,  -365,   -89,  -365,   437,   529,
    -365,  -365,   440,  -365,  -365,  -365,  -365,  -365,  -365,  -365,
    -365,   267,  -365,  -282,  -365,  -365,  -365,   419,  -365,  -365,
    -365,   312,  -365,  -365,  -365,  -365,  -365,  -365,  -365,   383,
    -365,  -365,  -365,  -365,   333,  -365,  -365,  -365,   689,  -365,
    -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365,  -365
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -234
static const short yytable[] =
{
     197,   131,   338,   132,   133,   447,   484,   363,   372,   180,
     181,   489,   105,   490,   106,   286,   491,   192,    51,   136,
     279,   190,   192,   330,   192,   439,   211,   287,   192,   192,
     193,   192,   193,   344,   194,   193,   415,   192,   193,    53,
     194,   192,   193,   203,   449,   206,    52,   210,   114,   254,
     214,   206,   216,   217,   218,   192,   193,   358,   186,   288,
     192,   193,   187,   452,   421,   185,   324,   325,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   438,   264,   345,   446,   326,
     451,   454,   456,   458,   460,   462,   137,   416,   184,   399,
     431,   212,   137,   358,   327,   173,   328,   436,   448,   485,
     174,   305,    58,    59,    60,    61,   353,   253,    62,    63,
     492,   226,   253,   506,   253,   280,    64,   433,   253,   195,
     428,   195,   420,    54,   500,    65,   280,   195,   196,   185,
     517,   195,   483,    55,   429,   444,  -233,   303,   304,   358,
      66,    67,    68,    69,   329,   195,   248,   387,   192,   464,
     195,   249,   370,    70,   400,    71,   138,   512,   308,   501,
      56,   308,   197,   316,   317,   354,   434,   139,   163,   503,
     175,   513,   443,   165,   164,   166,   167,   250,   168,    72,
     185,   248,   494,   322,    57,  -233,   249,   141,   103,   224,
      73,   225,    58,    59,    60,    61,   364,   365,    62,   371,
     277,   278,   251,   366,    74,    75,    64,   104,    76,   206,
     107,    77,   142,   385,    78,    65,   348,   105,   349,   291,
     292,   297,   292,   392,   334,   335,   108,   252,   395,   169,
      66,    67,    68,    69,   336,   337,    58,    59,    60,    61,
     109,   204,    62,    70,   110,    71,   111,   112,   253,   380,
     381,   113,    58,    59,    60,    61,   115,   204,    62,   116,
     197,   155,   156,   157,    85,   414,   335,   197,   117,    72,
     118,    86,    58,    59,    60,    61,    68,    69,    62,   445,
      73,   450,   453,   455,   457,   459,   461,   129,   119,   130,
     469,   335,    68,    69,    74,    75,   473,   474,    76,   120,
      87,    77,   121,   129,    78,   130,   401,   476,   335,   324,
     325,   122,    68,    69,   502,   335,   123,   324,   325,    88,
     516,   335,   134,   129,   124,   130,   521,   522,   141,   125,
     126,   402,   326,   127,   128,    89,   158,   161,    74,   176,
     326,   172,    76,   162,   141,    77,   205,   327,    78,   328,
     177,   178,   197,   142,    74,   327,   182,   328,    76,   499,
     183,    77,   189,   191,    78,   202,   141,   209,   213,   142,
     505,   223,   219,   229,    74,   220,   221,   247,    76,   260,
     258,    77,   261,   259,    78,   403,   404,   405,   406,   407,
     408,   142,   266,   262,   263,   265,   409,   329,   527,   141,
     153,   154,   155,   156,   157,   329,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   141,   267,   270,   142,   298,   299,   273,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   274,   276,   290,   142,   300,   301,   284,
     294,   141,   295,   306,   311,   313,   277,   315,   318,   321,
     333,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   142,   141,   332,   342,
     343,   299,   347,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   350,   351,
     359,   362,   142,   301,   141,   369,   368,   377,   374,   378,
     379,   382,   386,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   389,   142,
     141,   390,   222,   391,   400,   397,   410,   398,   417,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   411,   142,   141,   418,   293,   422,
     425,   440,   426,   442,   463,   427,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   142,   141,   432,   465,   296,   466,   470,   471,   472,
     387,   477,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   142,   480,   478,
     481,   302,   482,   486,   487,   498,   504,   508,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    91,    91,   509,   514,   393,   519,   278,
     515,   518,    92,    93,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,    94,
     141,   520,   524,   394,   525,    95,    95,   526,   188,   352,
     228,   396,   323,   523,   507,    96,    96,   141,   488,   493,
     346,   257,   528,    97,    97,   142,   215,   314,   312,   475,
     141,   437,   419,   339,    89,   373,   102,     0,    98,    98,
       0,    99,   142,     0,     0,     0,     0,     0,     0,     0,
     100,   100,   141,     0,     0,   142,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   142,     0,     0,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   141,     0,     0,
       0,     0,     0,     0,     0,     0,   141,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,     0,
       0,     0,   142,     0,     0,     0,     0,     0,     0,     0,
       0,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   149,   150,   151,   152,   153,   154,
     155,   156,   157,     1,     0,     2,     0,     0,     3,     0,
       0,     4,     0,     0,     5,     6,     7,     8,     0,     0,
       9,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,    13,     0,    14,     0,    15,     0,
      16,     0,    17,     0,    18,     0,     0,     0,     0,     0,
       0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     112,    74,   284,    76,    77,     9,     9,   313,   321,    98,
      99,     3,     3,     5,    20,    35,     8,     5,    45,    37,
       8,   110,     5,   274,     5,    37,    31,    47,     5,     5,
       6,     5,     6,    44,     8,     6,    25,     5,     6,    38,
       8,     5,     6,    92,     8,   118,     3,   120,    54,   161,
     123,   124,   125,   126,   127,     5,     6,   308,    71,    79,
       5,     6,    75,     8,   370,   114,     3,     4,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   391,   175,    98,   401,    26,
     403,   404,   405,   406,   407,   408,   114,    86,    73,   354,
     382,   106,   114,   354,    41,     3,    43,   389,   112,   112,
       8,   223,     3,     4,     5,     6,    53,   105,     9,    10,
     112,   112,   105,   487,   105,   113,    17,     3,   105,   105,
     381,   105,   113,    37,   105,    26,   113,   105,   112,   114,
     504,   105,   448,    38,   112,   400,     3,   220,   221,   400,
      41,    42,    43,    44,    91,   105,     3,    14,     5,   410,
     105,     8,   112,    54,    95,    56,     3,    15,    92,   482,
      70,    92,   284,   262,   263,   112,    52,    14,     3,   485,
      78,    29,   113,     3,     9,     5,     6,    34,     8,    80,
     114,     3,   474,   114,    45,    52,     8,    32,    83,   114,
      91,   116,     3,     4,     5,     6,    65,    66,     9,   321,
       5,     6,    59,    72,   105,   106,    17,    70,   109,   292,
       0,   112,    57,   335,   115,    26,   299,     3,   301,   113,
     114,   113,   114,   345,   113,   114,   111,    84,   350,    59,
      41,    42,    43,    44,   113,   114,     3,     4,     5,     6,
      48,     8,     9,    54,     3,    56,     3,   112,   105,   113,
     114,     3,     3,     4,     5,     6,     3,     8,     9,     3,
     382,   106,   107,   108,     3,   113,   114,   389,     3,    80,
     112,    10,     3,     4,     5,     6,    43,    44,     9,   401,
      91,   403,   404,   405,   406,   407,   408,    54,   117,    56,
     113,   114,    43,    44,   105,   106,   113,   114,   109,   112,
      39,   112,   112,    54,   115,    56,    19,   113,   114,     3,
       4,   112,    43,    44,   113,   114,   112,     3,     4,    58,
     113,   114,     3,    54,   112,    56,   113,   114,    32,   112,
     112,    44,    26,   112,   112,    74,    52,    98,   105,    73,
      26,    83,   109,    98,    32,   112,   113,    41,   115,    43,
       3,    51,   474,    57,   105,    41,    89,    43,   109,   481,
       3,   112,     3,    82,   115,    68,    32,     3,   113,    57,
      64,    98,   113,     3,   105,   112,   112,     5,   109,   105,
      98,   112,    49,    98,   115,    98,    99,   100,   101,   102,
     103,    57,     8,    78,    78,    78,   109,    91,   520,    32,
     104,   105,   106,   107,   108,    91,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,    32,    92,     3,    57,   113,   114,     3,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   112,    70,    43,    57,   113,   114,   114,
       3,    32,   113,     3,   114,   112,     5,    50,     3,    98,
       3,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,    57,    32,    88,   113,
      76,   114,   113,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    98,    71,
      40,     5,    57,   114,    32,    94,    98,   112,    60,   112,
     112,   112,     8,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   114,    57,
      32,     8,   113,   112,    95,   113,    20,   112,    65,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    93,    57,    32,     8,   113,     3,
     106,    61,   113,     8,    44,   113,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,    57,    32,   114,    40,   113,    61,    98,   114,   113,
      14,    20,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    57,   113,    52,
      95,   113,    95,    61,    20,     5,    20,   112,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,    13,    13,     3,   114,   113,     8,     6,
     114,   112,    22,    23,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,    39,
      32,    98,     5,   113,   113,    45,    45,   113,   108,   307,
     137,   351,   271,   514,   488,    55,    55,    32,   467,   471,
     292,   162,   522,    63,    63,    57,   124,   260,   258,   432,
      32,   389,   369,   284,    74,   322,    17,    -1,    78,    78,
      -1,    81,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    90,    32,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    57,    -1,    -1,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   100,   101,   102,   103,   104,   105,
     106,   107,   108,    16,    -1,    18,    -1,    -1,    21,    -1,
      -1,    24,    -1,    -1,    27,    28,    29,    30,    -1,    -1,
      33,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    -1,    -1,    67,    -1,    69,    -1,    71,    -1,
      73,    -1,    75,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    87
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    16,    18,    21,    24,    27,    28,    29,    30,    33,
      36,    46,    62,    67,    69,    71,    73,    75,    77,    85,
      87,   119,   120,   121,   122,   123,   127,   163,   168,   169,
     172,   173,   174,   175,   182,   183,   192,   193,   194,   195,
     198,   203,   204,   205,   208,   210,   211,   212,   213,   215,
     216,    45,     3,    38,    37,    38,    70,    45,     3,     4,
       5,     6,     9,    10,    17,    26,    41,    42,    43,    44,
      54,    56,    80,    91,   105,   106,   109,   112,   115,   128,
     129,   131,   158,   159,   214,     3,    10,    39,    58,    74,
     206,    13,    22,    23,    39,    45,    55,    63,    78,    81,
      90,   165,   206,    83,    70,     3,   132,     0,   111,    48,
       3,     3,   112,     3,   132,     3,     3,     3,   112,   117,
     112,   112,   112,   112,   112,   112,   112,   112,   112,    54,
      56,   158,   158,   158,     3,   162,    37,   114,     3,    14,
     130,    32,    57,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    52,   150,
     151,    98,    98,     3,     9,     3,     5,     6,     8,    59,
     166,   167,    83,     3,     8,    78,    73,     3,    51,   164,
     164,   164,    89,     3,    73,   114,    71,    75,   122,     3,
     164,    82,     5,     6,     8,   105,   112,   139,   140,   181,
     184,   185,    68,    92,     8,   113,   158,   160,   161,     3,
     158,    31,   106,   113,   158,   160,   158,   158,   158,   113,
     112,   112,   113,    98,   114,   116,   112,   132,   129,     3,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,     5,     3,     8,
      34,    59,    84,   105,   139,   170,   171,   167,    98,    98,
     105,    49,    78,    78,   164,    78,     8,    92,   199,   200,
       3,   196,   197,     3,   112,   176,    70,     5,     6,     8,
     113,   139,   141,   186,   114,   187,    35,    47,    79,   209,
      43,   113,   114,   113,     3,   113,   113,   113,   113,   114,
     113,   114,   113,   158,   158,   139,     3,   124,    92,   133,
     134,   114,   170,   112,   166,    50,   164,   164,     3,   201,
     202,    98,   114,   134,     3,     4,    26,    41,    43,    91,
     138,   177,    88,     3,   113,   114,   113,   114,   181,   185,
     188,   189,   113,    76,    44,    98,   161,   113,   158,   158,
      98,    71,   127,    53,   112,   135,   136,   137,   138,    40,
     142,   143,     5,   141,    65,    66,    72,   207,    98,    94,
     112,   139,   140,   197,    60,   152,   153,   112,   112,   112,
     113,   114,   112,   178,   179,   139,     8,    14,   190,   114,
       8,   112,   139,   113,   113,   139,   128,   113,   112,   135,
      95,    19,    44,    98,    99,   100,   101,   102,   103,   109,
      20,    93,   144,   145,   113,    25,    86,    65,     8,   202,
     113,   141,     3,   154,   155,   106,   113,   113,   138,   112,
     180,   181,   114,     3,    52,   191,   181,   189,   141,    37,
      61,   125,     8,   113,   135,   139,   140,     9,   112,     8,
     139,   140,     8,   139,   140,   139,   140,   139,   140,   139,
     140,   139,   140,    44,   138,    40,    61,   146,   147,   113,
      98,   114,   113,   113,   114,   179,   113,    20,    52,   126,
     113,    95,    95,   141,     9,   112,    61,    20,   150,     3,
       5,     8,   112,   155,   181,   138,   148,   149,     5,   139,
     105,   140,   113,   141,    20,    64,   148,   152,   112,     3,
     156,   157,    15,    29,   114,   114,   113,   148,   112,     8,
      98,   113,   114,   149,     5,   113,   113,   139,   157
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror (pParser, "syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, pParser)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse ( SqlParser_c * pParser );
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse ( SqlParser_c * pParser )
#else
int
yyparse (pParser)
     SqlParser_c * pParser ;
#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

    { pParser->PushQuery(); ;}
    break;

  case 25:

    { pParser->PushQuery(); ;}
    break;

  case 26:

    { pParser->PushQuery(); ;}
    break;

  case 30:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 31:

    {
		CSphVector<CSphQueryItem> & dItems = pParser->m_pQuery->m_dItems;
		if ( dItems.GetLength()!=1 || dItems[0].m_sExpr!="*" )
		{
			yyerror ( pParser, "outer select list must be a single star" );
			YYERROR;
		}
		dItems.Reset();
		pParser->ResetSelect();
	;}
    break;

  case 32:

    {
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 34:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 35:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 36:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 39:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 42:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 43:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 44:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 45:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 46:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 47:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 48:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 49:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 50:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 51:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 52:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 53:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 55:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 62:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 63:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 64:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 65:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 66:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 67:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 73:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			yyerror ( pParser, "EQ and NEQ filters on floats are not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 75:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 76:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 86:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 87:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 88:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 89:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 90:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 91:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 92:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 93:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 94:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 95:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 98:

    {
			pParser->SetGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 101:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 104:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 105:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 107:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 109:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 110:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 113:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 114:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 120:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 121:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 122:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 123:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 124:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 125:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 126:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 127:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 129:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 130:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 134:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 135:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 137:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 138:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 139:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 140:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 141:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 142:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 143:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 144:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 145:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 146:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 147:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 148:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 149:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 150:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 151:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 152:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 153:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 154:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 156:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 157:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 158:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 159:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 160:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 165:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 166:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 169:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 175:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 176:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 177:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 185:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 186:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 187:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 188:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 190:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 191:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 194:

    { yyval.m_iValue = 1; ;}
    break;

  case 195:

    { yyval.m_iValue = 0; ;}
    break;

  case 196:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 197:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 198:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 199:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 202:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 207:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 208:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 211:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 212:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 213:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 214:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 215:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 216:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 217:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 218:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 219:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 220:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 221:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 222:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 223:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 225:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 226:

    {
			// FIXME? for now, one such array per CALL statement, tops
			if ( pParser->m_pStmt->m_dCallStrings.GetLength() )
			{
				yyerror ( pParser, "unexpected constant string list" );
				YYERROR;
			}
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 227:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 230:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 232:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 236:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 237:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 240:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 241:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 244:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 245:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 246:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 247:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 248:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 255:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 256:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 265:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 266:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 267:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 268:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 269:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 270:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 271:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 272:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 274:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 275:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 276:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;


    }

/* Line 991 of yacc.c.  */


  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 4)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      else
		{
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			snprintf (yyp, (int)(yysize - (yyp - yymsg)), ", expecting %s (or %d other tokens)", yytname[yyx], yycount - 1);
			while (*yyp++);
			break;
		      }
		}

	      yyerror (pParser, yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror (pParser, "syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (pParser, "syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
//  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror (pParser, "parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}





#if USE_WINDOWS
#pragma warning(pop)
#endif

