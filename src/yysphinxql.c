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
#define YYLAST   983

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  118
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  99
/* YYNRULES -- Number of rules. */
#define YYNRULES  277
/* YYNRULES -- Number of states. */
#define YYNSTATES  531

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
     141,   146,   150,   156,   158,   162,   163,   165,   168,   170,
     174,   178,   183,   187,   191,   197,   204,   208,   213,   219,
     223,   227,   231,   235,   239,   241,   247,   251,   255,   259,
     263,   267,   271,   275,   277,   279,   284,   288,   292,   294,
     296,   299,   301,   304,   306,   310,   311,   315,   317,   321,
     322,   324,   330,   331,   333,   337,   343,   345,   349,   351,
     354,   357,   358,   360,   363,   368,   369,   371,   374,   376,
     380,   384,   388,   394,   401,   405,   407,   411,   415,   417,
     419,   421,   423,   425,   427,   430,   433,   437,   441,   445,
     449,   453,   457,   461,   465,   469,   473,   477,   481,   485,
     489,   493,   497,   501,   505,   509,   511,   516,   521,   525,
     532,   539,   543,   545,   549,   551,   553,   557,   563,   566,
     567,   570,   572,   575,   578,   582,   584,   589,   594,   598,
     600,   602,   604,   606,   608,   610,   614,   619,   624,   629,
     633,   638,   646,   652,   654,   656,   658,   660,   662,   664,
     666,   668,   670,   673,   680,   682,   684,   685,   689,   691,
     695,   697,   701,   705,   707,   711,   713,   715,   717,   721,
     724,   732,   742,   749,   751,   755,   757,   761,   763,   767,
     768,   771,   773,   777,   781,   782,   784,   786,   788,   792,
     794,   796,   800,   807,   809,   813,   817,   821,   827,   832,
     837,   838,   840,   843,   845,   849,   853,   856,   860,   867,
     868,   870,   872,   875,   878,   881,   883,   891,   893,   895,
     897,   901,   908,   912,   916,   918,   922,   926
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
      -1,    26,   112,   106,   113,    -1,    41,   112,   113,    -1,
      26,   112,    31,     3,   113,    -1,     3,    -1,   132,   114,
       3,    -1,    -1,   134,    -1,    92,   135,    -1,   136,    -1,
     135,    95,   135,    -1,   112,   135,   113,    -1,    53,   112,
       8,   113,    -1,   138,    98,   139,    -1,   138,    99,   139,
      -1,   138,    44,   112,   141,   113,    -1,   138,   109,    44,
     112,   141,   113,    -1,   138,    44,     9,    -1,   138,   109,
      44,     9,    -1,   138,    19,   139,    95,   139,    -1,   138,
     101,   139,    -1,   138,   100,   139,    -1,   138,   102,   139,
      -1,   138,   103,   139,    -1,   138,    98,   140,    -1,   137,
      -1,   138,    19,   140,    95,   140,    -1,   138,   101,   140,
      -1,   138,   100,   140,    -1,   138,   102,   140,    -1,   138,
     103,   140,    -1,   138,    98,     8,    -1,   138,    99,     8,
      -1,   138,    99,   140,    -1,     3,    -1,     4,    -1,    26,
     112,   106,   113,    -1,    41,   112,   113,    -1,    91,   112,
     113,    -1,    43,    -1,     5,    -1,   105,     5,    -1,     6,
      -1,   105,     6,    -1,   139,    -1,   141,   114,   139,    -1,
      -1,    40,    20,   143,    -1,   138,    -1,   143,   114,   138,
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
     113,    -1,    54,   112,   158,   114,   158,   113,    -1,    91,
     112,   113,    -1,   161,    -1,   160,   114,   161,    -1,   158,
      -1,     8,    -1,     3,    98,   139,    -1,   162,   114,     3,
      98,   139,    -1,    75,   165,    -1,    -1,    51,     8,    -1,
      90,    -1,    78,   164,    -1,    55,   164,    -1,    13,    78,
     164,    -1,    63,    -1,    13,     8,    78,   164,    -1,    13,
       3,    78,   164,    -1,    45,     3,    78,    -1,     3,    -1,
      59,    -1,     8,    -1,     5,    -1,     6,    -1,   166,    -1,
     167,   105,   166,    -1,    73,     3,    98,   171,    -1,    73,
       3,    98,   170,    -1,    73,     3,    98,    59,    -1,    73,
      58,   167,    -1,    73,    10,    98,   167,    -1,    73,    39,
       9,    98,   112,   141,   113,    -1,    73,    39,     3,    98,
     170,    -1,     3,    -1,     8,    -1,    84,    -1,    34,    -1,
     139,    -1,    24,    -1,    69,    -1,   173,    -1,    18,    -1,
      77,    83,    -1,   175,    48,     3,   176,    88,   178,    -1,
      46,    -1,    67,    -1,    -1,   112,   177,   113,    -1,   138,
      -1,   177,   114,   138,    -1,   179,    -1,   178,   114,   179,
      -1,   112,   180,   113,    -1,   181,    -1,   180,   114,   181,
      -1,   139,    -1,   140,    -1,     8,    -1,   112,   141,   113,
      -1,   112,   113,    -1,    28,    37,   132,    92,    43,    98,
     139,    -1,    28,    37,   132,    92,    43,    44,   112,   141,
     113,    -1,    21,     3,   112,   184,   187,   113,    -1,   185,
      -1,   184,   114,   185,    -1,   181,    -1,   112,   186,   113,
      -1,     8,    -1,   186,   114,     8,    -1,    -1,   114,   188,
      -1,   189,    -1,   188,   114,   189,    -1,   181,   190,   191,
      -1,    -1,    14,    -1,     3,    -1,    52,    -1,   193,     3,
     164,    -1,    30,    -1,    29,    -1,    75,    81,   164,    -1,
      87,   132,    73,   196,   134,   152,    -1,   197,    -1,   196,
     114,   197,    -1,     3,    98,   139,    -1,     3,    98,   140,
      -1,     3,    98,   112,   141,   113,    -1,     3,    98,   112,
     113,    -1,    75,   206,    89,   199,    -1,    -1,   200,    -1,
      92,   201,    -1,   202,    -1,   201,    94,   202,    -1,     3,
      98,     8,    -1,    75,    23,    -1,    75,    22,    73,    -1,
      73,   206,    83,    49,    50,   207,    -1,    -1,    39,    -1,
      74,    -1,    65,    86,    -1,    65,    25,    -1,    66,    65,
      -1,    72,    -1,    27,    38,     3,    68,   209,    76,     8,
      -1,    47,    -1,    35,    -1,    79,    -1,    33,    38,     3,
      -1,    16,    45,     3,    82,    70,     3,    -1,    36,    70,
       3,    -1,    71,   214,   150,    -1,    10,    -1,    10,   117,
       3,    -1,    85,    70,     3,    -1,    62,    45,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   129,   129,   130,   131,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   160,   161,   165,   166,   170,
     171,   179,   192,   200,   202,   207,   216,   232,   233,   237,
     238,   241,   243,   244,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   260,   261,   264,   266,   270,   274,   275,
     276,   280,   285,   292,   300,   308,   317,   322,   327,   332,
     337,   342,   347,   352,   357,   362,   367,   372,   377,   382,
     387,   392,   400,   404,   405,   410,   416,   422,   428,   437,
     438,   449,   450,   454,   460,   466,   468,   472,   476,   482,
     484,   488,   499,   501,   505,   509,   516,   517,   521,   522,
     523,   526,   528,   532,   537,   544,   546,   550,   554,   555,
     559,   564,   569,   575,   580,   588,   593,   600,   610,   611,
     612,   613,   614,   615,   616,   617,   618,   619,   620,   621,
     622,   623,   624,   625,   626,   627,   628,   629,   630,   631,
     632,   633,   634,   635,   636,   637,   641,   642,   643,   644,
     645,   646,   650,   651,   655,   656,   660,   661,   667,   670,
     672,   676,   677,   678,   679,   680,   681,   686,   691,   701,
     702,   703,   704,   705,   709,   710,   714,   719,   724,   729,
     730,   734,   739,   747,   748,   752,   753,   754,   768,   769,
     770,   774,   775,   781,   789,   790,   793,   795,   799,   800,
     804,   805,   809,   813,   814,   818,   819,   820,   821,   822,
     828,   836,   850,   858,   862,   869,   870,   877,   887,   893,
     895,   899,   904,   908,   915,   917,   921,   922,   928,   936,
     937,   943,   949,   957,   958,   962,   966,   970,   974,   984,
     990,   991,   995,   999,  1000,  1004,  1008,  1015,  1022,  1028,
    1029,  1030,  1034,  1035,  1036,  1037,  1043,  1054,  1055,  1056,
    1060,  1071,  1083,  1094,  1102,  1103,  1112,  1123
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
  "const_float", "const_list", "opt_group_clause", "group_items_list", 
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
     131,   131,   131,   132,   132,   133,   133,   134,   135,   135,
     135,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   137,   138,   138,   138,   138,   138,   138,   139,
     139,   140,   140,   141,   141,   142,   142,   143,   143,   144,
     144,   145,   146,   146,   147,   147,   148,   148,   149,   149,
     149,   150,   150,   151,   151,   152,   152,   153,   154,   154,
     155,   155,   155,   155,   155,   156,   156,   157,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   159,   159,   159,   159,
     159,   159,   160,   160,   161,   161,   162,   162,   163,   164,
     164,   165,   165,   165,   165,   165,   165,   165,   165,   166,
     166,   166,   166,   166,   167,   167,   168,   168,   168,   168,
     168,   169,   169,   170,   170,   171,   171,   171,   172,   172,
     172,   173,   173,   174,   175,   175,   176,   176,   177,   177,
     178,   178,   179,   180,   180,   181,   181,   181,   181,   181,
     182,   182,   183,   184,   184,   185,   185,   186,   186,   187,
     187,   188,   188,   189,   190,   190,   191,   191,   192,   193,
     193,   194,   195,   196,   196,   197,   197,   197,   197,   198,
     199,   199,   200,   201,   201,   202,   203,   204,   205,   206,
     206,   206,   207,   207,   207,   207,   208,   209,   209,   209,
     210,   211,   212,   213,   214,   214,   215,   216
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       9,     0,     3,     0,     2,     4,    10,     1,     3,     1,
       2,     0,     1,     2,     1,     4,     4,     4,     4,     4,
       4,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       3,     4,     3,     3,     5,     6,     3,     4,     5,     3,
       3,     3,     3,     3,     1,     5,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     4,     3,     3,     1,     1,
       2,     1,     2,     1,     3,     0,     3,     1,     3,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     6,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     4,     4,     3,     6,
       6,     3,     1,     3,     1,     1,     3,     5,     2,     0,
       2,     1,     2,     2,     3,     1,     4,     4,     3,     1,
       1,     1,     1,     1,     1,     3,     4,     4,     4,     3,
       4,     7,     5,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     6,     1,     1,     0,     3,     1,     3,
       1,     3,     3,     1,     3,     1,     1,     1,     3,     2,
       7,     9,     6,     1,     3,     1,     3,     1,     3,     0,
       2,     1,     3,     3,     0,     1,     1,     1,     3,     1,
       1,     3,     6,     1,     3,     3,     3,     5,     4,     4,
       0,     1,     2,     1,     3,     3,     2,     3,     6,     0,
       1,     1,     2,     2,     2,     1,     7,     1,     1,     1,
       3,     6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   201,     0,   198,     0,     0,   240,   239,     0,
       0,   204,     0,   205,   199,     0,   259,   259,     0,     0,
       0,     0,     2,     3,    25,    27,    29,    28,     7,     8,
       9,   200,     5,     0,     6,    10,    11,     0,    12,    13,
      14,    15,    16,    21,    17,    18,    19,    20,    22,    23,
      24,     0,     0,     0,     0,     0,     0,     0,   128,   129,
     131,   132,   133,   274,     0,     0,     0,     0,   130,     0,
       0,     0,     0,     0,     0,    39,     0,     0,     0,     0,
      37,    41,    44,   155,   111,     0,     0,   260,     0,   261,
       0,     0,     0,   256,   260,     0,   169,   175,   169,   169,
     171,   168,     0,   202,     0,    53,     0,     1,     4,     0,
     169,     0,     0,     0,     0,   270,   272,   277,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   134,   135,     0,     0,     0,     0,     0,    42,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   273,
     112,     0,     0,     0,     0,   179,   182,   183,   181,   180,
     184,   189,     0,     0,     0,   169,   257,     0,     0,   173,
     172,   241,   250,   276,     0,     0,     0,     0,    26,   206,
     238,     0,    89,    91,   217,     0,     0,   215,   216,   225,
     229,   223,     0,     0,   165,   158,   164,     0,   162,   275,
       0,     0,     0,    51,     0,     0,     0,     0,     0,   161,
       0,     0,   153,     0,     0,   154,    31,    55,    38,    43,
     145,   146,   152,   151,   143,   142,   149,   150,   140,   141,
     148,   147,   136,   137,   138,   139,   144,   113,   193,   194,
     196,   188,   195,     0,   197,   187,   186,   190,     0,     0,
       0,     0,   169,   169,   174,   178,   170,     0,   249,   251,
       0,     0,   243,    54,     0,     0,     0,    90,    92,   227,
     219,    93,     0,     0,     0,     0,   268,   267,   269,     0,
       0,   156,     0,    45,     0,    50,    49,   157,    46,     0,
      47,     0,    48,     0,     0,   166,     0,     0,     0,    95,
      56,     0,   192,     0,   185,     0,   177,   176,     0,   252,
     253,     0,     0,   115,    83,    84,     0,     0,    88,     0,
     208,     0,     0,   271,   218,     0,   226,     0,   225,   224,
     230,   231,   222,     0,     0,     0,   163,    52,     0,     0,
       0,     0,     0,     0,     0,    57,    58,    74,     0,     0,
      99,   114,     0,     0,     0,   265,   258,     0,     0,     0,
     245,   246,   244,     0,   242,   116,     0,     0,     0,   207,
       0,     0,   203,   210,    94,   228,   235,     0,     0,   266,
       0,   220,   160,   159,   167,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   102,   100,   191,   263,   262,   264,   255,   254,   248,
       0,     0,   117,   118,     0,    86,    87,   209,     0,     0,
     213,     0,   236,   237,   233,   234,   232,     0,     0,     0,
      33,     0,    60,    59,     0,     0,    66,     0,    80,    62,
      73,    81,    63,    82,    70,    77,    69,    76,    71,    78,
      72,    79,     0,    97,    96,     0,     0,   111,   103,   247,
       0,     0,    85,   212,     0,   211,   221,     0,     0,    30,
      61,     0,     0,     0,    67,     0,     0,     0,     0,   115,
     120,   121,   124,     0,   119,   214,   108,    32,   106,    34,
      68,     0,    75,    64,     0,    98,     0,     0,   104,    36,
       0,     0,     0,   125,   109,   110,     0,     0,    65,   101,
       0,     0,     0,   122,     0,   107,    35,   105,   123,   127,
     126
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   307,   440,   479,    26,
      79,    80,   140,    81,   227,   309,   310,   355,   356,   357,
     496,   281,   198,   282,   360,   464,   411,   412,   467,   468,
     497,   498,   159,   160,   374,   375,   422,   423,   512,   513,
      82,    83,   207,   208,   135,    27,   179,   101,   170,   171,
      28,    29,   255,   256,    30,    31,    32,    33,   275,   331,
     382,   383,   429,   199,    34,    35,   200,   201,   283,   285,
     340,   341,   387,   434,    36,    37,    38,    39,   271,   272,
      40,   268,   269,   319,   320,    41,    42,    43,    90,   366,
      44,   289,    45,    46,    47,    48,    84,    49,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -385
static const short yypact[] =
{
     896,    -8,  -385,   136,  -385,   157,    72,  -385,  -385,   168,
     141,  -385,   152,  -385,  -385,   140,    95,   671,   144,   162,
     226,   234,  -385,   155,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,   203,  -385,  -385,  -385,   250,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,   251,   148,   264,   226,   267,   269,   278,   172,  -385,
    -385,  -385,  -385,   183,   177,   204,   209,   210,  -385,   212,
     213,   214,   215,   217,   302,  -385,   302,   302,   329,   -13,
    -385,    43,   679,  -385,   279,   235,   236,    58,   153,  -385,
     252,    40,   270,  -385,  -385,   337,   291,  -385,   291,   291,
    -385,  -385,   259,  -385,   347,  -385,   -14,  -385,   116,   348,
     291,   272,    20,   287,    59,  -385,  -385,  -385,   274,   354,
     302,    31,   246,   302,   293,   302,   302,   302,   248,   255,
     256,  -385,  -385,   471,   265,    38,     2,   159,  -385,   368,
    -385,   302,   302,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   369,  -385,
    -385,   307,   153,   275,   280,  -385,  -385,  -385,  -385,  -385,
    -385,   271,   323,   299,   303,   291,  -385,   304,   386,  -385,
    -385,  -385,   305,  -385,   392,   397,   159,   325,  -385,   284,
    -385,   331,  -385,  -385,  -385,    60,    10,  -385,  -385,  -385,
     290,  -385,   151,   363,  -385,  -385,   679,   -17,  -385,  -385,
     497,   406,   313,  -385,   523,   103,   367,   393,   549,  -385,
     302,   302,  -385,     8,   413,  -385,  -385,    75,  -385,  -385,
    -385,  -385,   693,   706,   783,   794,   803,   803,   328,   328,
     328,   328,    71,    71,  -385,  -385,  -385,   314,  -385,  -385,
    -385,  -385,  -385,   422,  -385,  -385,  -385,   271,   202,   318,
     153,   381,   291,   291,  -385,  -385,  -385,   434,  -385,  -385,
     340,    79,  -385,  -385,   218,   351,   438,  -385,  -385,  -385,
    -385,  -385,   111,   127,    20,   330,  -385,  -385,  -385,   366,
      19,  -385,   293,  -385,   332,  -385,  -385,  -385,  -385,   302,
    -385,   302,  -385,   419,   445,  -385,   346,   375,   232,   407,
    -385,   443,  -385,     8,  -385,   142,  -385,  -385,   355,   358,
    -385,    28,   392,   389,  -385,  -385,   342,   343,  -385,   344,
    -385,   129,   345,  -385,  -385,     8,  -385,   450,   185,  -385,
     364,  -385,  -385,   451,   370,     8,  -385,  -385,   575,   601,
       8,   159,   371,   373,   232,   365,  -385,  -385,   320,   459,
     390,  -385,   134,    16,   421,  -385,  -385,   496,   434,    11,
    -385,  -385,  -385,   502,  -385,  -385,   402,   396,   398,  -385,
     218,    24,   416,  -385,  -385,  -385,  -385,    55,    24,  -385,
       8,  -385,  -385,  -385,  -385,    -6,   449,   504,   -53,   232,
      50,    -2,    15,    30,    50,    50,    50,    50,   487,   218,
     492,   473,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
     143,   437,   423,  -385,   425,  -385,  -385,  -385,    14,   149,
    -385,   345,  -385,  -385,  -385,   522,  -385,   173,   226,   536,
     505,   447,  -385,  -385,   463,   466,  -385,     8,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,    18,  -385,   448,   503,   543,   279,  -385,  -385,
       9,   502,  -385,  -385,    24,  -385,  -385,   218,   577,  -385,
    -385,     8,    33,   190,  -385,     8,   218,   563,   349,   389,
     474,  -385,  -385,   582,  -385,  -385,   175,   475,  -385,   476,
    -385,   581,  -385,  -385,   200,  -385,   218,   499,   475,  -385,
     580,   510,   206,  -385,  -385,  -385,   218,   604,  -385,   475,
     500,   501,     8,  -385,   582,  -385,  -385,  -385,  -385,  -385,
    -385
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -385,  -385,  -385,  -385,   507,  -385,  -385,  -385,  -385,   309,
     261,   498,  -385,  -385,   106,  -385,   388,  -310,  -385,  -385,
    -268,  -112,  -312,  -305,  -385,  -385,  -385,  -385,  -385,  -385,
    -384,   118,   170,  -385,   150,  -385,  -385,   167,  -385,   117,
     -73,  -385,   516,   350,  -385,  -385,   -88,  -385,   400,   506,
    -385,  -385,   403,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,   233,  -385,  -282,  -385,  -385,  -385,   379,  -385,  -385,
    -385,   277,  -385,  -385,  -385,  -385,  -385,  -385,  -385,   391,
    -385,  -385,  -385,  -385,   298,  -385,  -385,  -385,   650,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -235
static const short yytable[] =
{
     197,   131,   338,   132,   133,   105,   330,   446,   362,   371,
     180,   181,   490,   192,   491,   192,   192,   492,   279,   192,
     192,   193,   190,   448,   136,   192,   193,   484,   194,   192,
     193,   438,   194,   192,   193,   192,   193,    51,   451,   193,
     358,   414,   399,   173,   398,   206,   138,   210,   174,   254,
     214,   206,   216,   217,   218,   192,   193,   139,   432,   184,
     442,   163,   211,   344,   420,   277,   278,   164,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   437,   358,   264,   445,   443,
     450,   453,   455,   457,   459,   461,   291,   292,    85,   430,
     185,   137,   415,   141,   508,    86,   435,   433,   137,    54,
     447,   305,   427,   253,   226,   253,   253,   345,   175,   253,
     195,   493,   519,   280,   419,   195,   106,   280,   142,   195,
     485,   358,   196,   195,    87,   195,   428,   212,   501,    52,
     369,   463,   483,    58,    59,    60,    61,   303,   304,    62,
      63,   203,   224,    88,   225,   195,   165,    64,   166,   167,
     114,   168,    58,    59,    60,    61,    65,   308,    62,    89,
     502,   308,   197,   185,   316,   317,    64,   155,   156,   157,
     504,    66,    67,    68,    69,    65,   286,   186,  -234,   185,
     514,   187,   495,   322,    70,    53,    71,    57,   287,   386,
      66,    67,    68,    69,   515,   248,    55,   363,   364,   370,
     249,    56,   169,    70,   365,    71,   297,   292,   505,   206,
      72,   324,   325,   384,   334,   335,   348,   103,   349,   105,
     288,    73,   104,   391,   107,   324,   325,  -234,   394,    72,
     336,   337,   379,   380,   326,    74,    75,   413,   335,    76,
      73,   109,    77,   110,   111,    78,   469,   335,   326,   327,
     112,   328,   473,   474,    74,    75,   108,   113,    76,   197,
     115,    77,   116,   327,    78,   328,   197,    58,    59,    60,
      61,   117,   204,    62,   118,   353,   476,   335,   444,   120,
     449,   452,   454,   456,   458,   460,    58,    59,    60,    61,
     119,   204,    62,   503,   335,    58,    59,    60,    61,   329,
     248,    62,   192,   518,   335,   249,   121,    68,    69,   523,
     524,   122,   123,   329,   124,   125,   126,   127,   129,   128,
     130,   158,   134,   161,   162,   172,    68,    69,    91,   400,
     177,   250,   178,   176,   354,    68,    69,   129,   182,   130,
     183,   189,   324,   325,   191,   202,   129,   209,   130,   213,
     141,   219,   197,   223,   401,    73,   251,   220,   221,   500,
      95,   229,   261,   258,   247,   326,   260,   262,   259,    74,
      96,   263,   265,    76,    73,   142,    77,   205,    97,    78,
     327,   252,   328,    73,   266,   270,   274,   267,    74,   141,
     273,   276,    76,    98,   284,    77,   290,    74,    78,   294,
     529,    76,   253,   507,    77,   100,   306,    78,   402,   403,
     404,   405,   406,   407,   142,   141,   295,   277,   311,   408,
     313,   315,   153,   154,   155,   156,   157,   318,   321,   332,
     329,   333,   343,   342,   350,   347,   351,   359,   361,   373,
     142,   141,   368,   367,   376,   377,   378,   381,   385,   389,
     399,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   142,   141,   388,   409,
     298,   299,   390,   410,   396,   397,   416,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   142,   141,   417,   421,   300,   301,   424,   425,
     439,   426,   441,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   142,   141,
     431,   462,   465,   299,   466,   470,   386,   471,   472,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   142,   141,   477,   478,   481,   301,
     480,   482,   486,   488,   487,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     142,   141,   499,   506,   222,   511,   510,   278,   521,   516,
     517,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   142,   141,   522,   526,
     293,   520,   395,   527,   528,   188,   352,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   142,   141,   525,   228,   296,   489,   494,   509,
     215,   530,   346,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   142,   323,
     314,   312,   302,   339,   475,   436,   418,   102,   257,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,    91,     0,     0,     0,   392,     0,
       0,     0,     0,    92,    93,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
      94,   141,     0,   372,   393,     0,    95,     0,     0,     0,
       0,     0,     0,     0,     0,   141,    96,     0,     0,     0,
       0,     0,     0,     0,    97,     0,   142,     0,   141,     0,
       0,     0,     0,     0,     0,    89,     0,     0,     0,    98,
     142,     0,    99,     0,     0,     0,     0,     0,     0,     0,
       0,   100,     0,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   141,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   141,     0,     0,     0,
       0,     0,     0,     0,     0,   141,     0,     0,     0,     0,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   142,     0,     0,     0,     0,     0,     0,     0,     0,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   149,   150,   151,   152,   153,   154,   155,
     156,   157,     1,     0,     2,     0,     0,     3,     0,     0,
       4,     0,     0,     5,     6,     7,     8,     0,     0,     9,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,    13,     0,    14,     0,    15,     0,    16,
       0,    17,     0,    18,     0,     0,     0,     0,     0,     0,
       0,    19,     0,    20
};

static const short yycheck[] =
{
     112,    74,   284,    76,    77,     3,   274,     9,   313,   321,
      98,    99,     3,     5,     5,     5,     5,     8,     8,     5,
       5,     6,   110,     8,    37,     5,     6,     9,     8,     5,
       6,    37,     8,     5,     6,     5,     6,    45,     8,     6,
     308,    25,    95,     3,   354,   118,     3,   120,     8,   161,
     123,   124,   125,   126,   127,     5,     6,    14,     3,    73,
     113,     3,    31,    44,   369,     5,     6,     9,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   390,   354,   175,   400,   399,
     402,   403,   404,   405,   406,   407,   113,   114,     3,   381,
     114,   114,    86,    32,   488,    10,   388,    52,   114,    37,
     112,   223,   380,   105,   112,   105,   105,    98,    78,   105,
     105,   112,   506,   113,   113,   105,    20,   113,    57,   105,
     112,   399,   112,   105,    39,   105,   112,   106,   105,     3,
     112,   409,   447,     3,     4,     5,     6,   220,   221,     9,
      10,    92,   114,    58,   116,   105,     3,    17,     5,     6,
      54,     8,     3,     4,     5,     6,    26,    92,     9,    74,
     482,    92,   284,   114,   262,   263,    17,   106,   107,   108,
     485,    41,    42,    43,    44,    26,    35,    71,     3,   114,
      15,    75,   474,   114,    54,    38,    56,    45,    47,    14,
      41,    42,    43,    44,    29,     3,    38,    65,    66,   321,
       8,    70,    59,    54,    72,    56,   113,   114,   486,   292,
      80,     3,     4,   335,   113,   114,   299,    83,   301,     3,
      79,    91,    70,   345,     0,     3,     4,    52,   350,    80,
     113,   114,   113,   114,    26,   105,   106,   113,   114,   109,
      91,    48,   112,     3,     3,   115,   113,   114,    26,    41,
     112,    43,   113,   114,   105,   106,   111,     3,   109,   381,
       3,   112,     3,    41,   115,    43,   388,     3,     4,     5,
       6,     3,     8,     9,   112,    53,   113,   114,   400,   112,
     402,   403,   404,   405,   406,   407,     3,     4,     5,     6,
     117,     8,     9,   113,   114,     3,     4,     5,     6,    91,
       3,     9,     5,   113,   114,     8,   112,    43,    44,   113,
     114,   112,   112,    91,   112,   112,   112,   112,    54,   112,
      56,    52,     3,    98,    98,    83,    43,    44,    13,    19,
       3,    34,    51,    73,   112,    43,    44,    54,    89,    56,
       3,     3,     3,     4,    82,    68,    54,     3,    56,   113,
      32,   113,   474,    98,    44,    91,    59,   112,   112,   481,
      45,     3,    49,    98,     5,    26,   105,    78,    98,   105,
      55,    78,    78,   109,    91,    57,   112,   113,    63,   115,
      41,    84,    43,    91,     8,     3,   112,    92,   105,    32,
       3,    70,   109,    78,   114,   112,    43,   105,   115,     3,
     522,   109,   105,    64,   112,    90,     3,   115,    98,    99,
     100,   101,   102,   103,    57,    32,   113,     5,   114,   109,
     112,    50,   104,   105,   106,   107,   108,     3,    98,    88,
      91,     3,    76,   113,    98,   113,    71,    40,     5,    60,
      57,    32,    94,    98,   112,   112,   112,   112,     8,     8,
      95,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,    57,    32,   114,    20,
     113,   114,   112,    93,   113,   112,    65,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    57,    32,     8,     3,   113,   114,   106,   113,
      61,   113,     8,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    57,    32,
     114,    44,    40,   114,    61,    98,    14,   114,   113,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    57,    32,    20,    52,    95,   114,
     113,    95,   114,    20,    61,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      57,    32,     5,    20,   113,     3,   112,     6,     8,   114,
     114,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,    57,    32,    98,     5,
     113,   112,   351,   113,   113,   108,   307,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    57,    32,   516,   137,   113,   467,   471,   489,
     124,   524,   292,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    57,   271,
     260,   258,   113,   284,   431,   388,   368,    17,   162,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    13,    -1,    -1,    -1,   113,    -1,
      -1,    -1,    -1,    22,    23,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      39,    32,    -1,   322,   113,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    57,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    78,
      57,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    16,    -1,    18,    -1,    -1,    21,    -1,    -1,
      24,    -1,    -1,    27,    28,    29,    30,    -1,    -1,    33,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    -1,    -1,    67,    -1,    69,    -1,    71,    -1,    73,
      -1,    75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    87
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
     142,     5,   141,    65,    66,    72,   207,    98,    94,   112,
     139,   140,   197,    60,   152,   153,   112,   112,   112,   113,
     114,   112,   178,   179,   139,     8,    14,   190,   114,     8,
     112,   139,   113,   113,   139,   128,   113,   112,   135,    95,
      19,    44,    98,    99,   100,   101,   102,   103,   109,    20,
      93,   144,   145,   113,    25,    86,    65,     8,   202,   113,
     141,     3,   154,   155,   106,   113,   113,   138,   112,   180,
     181,   114,     3,    52,   191,   181,   189,   141,    37,    61,
     125,     8,   113,   135,   139,   140,     9,   112,     8,   139,
     140,     8,   139,   140,   139,   140,   139,   140,   139,   140,
     139,   140,    44,   138,   143,    40,    61,   146,   147,   113,
      98,   114,   113,   113,   114,   179,   113,    20,    52,   126,
     113,    95,    95,   141,     9,   112,   114,    61,    20,   150,
       3,     5,     8,   112,   155,   181,   138,   148,   149,     5,
     139,   105,   140,   113,   141,   138,    20,    64,   148,   152,
     112,     3,   156,   157,    15,    29,   114,   114,   113,   148,
     112,     8,    98,   113,   114,   149,     5,   113,   113,   139,
     157
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

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 52:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 54:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 61:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 62:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 63:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 64:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 65:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 66:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 67:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 73:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
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

  case 84:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 85:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 86:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 87:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 88:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 89:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 90:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 91:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 92:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 93:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 94:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 97:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 98:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
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

  case 161:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 166:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 167:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 175:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 176:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 177:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 178:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 186:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 187:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 188:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 190:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 191:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 192:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 195:

    { yyval.m_iValue = 1; ;}
    break;

  case 196:

    { yyval.m_iValue = 0; ;}
    break;

  case 197:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 198:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 199:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 200:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 203:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 205:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 208:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 209:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 212:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 213:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 214:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 215:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 216:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 217:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 218:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 219:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 220:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 221:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 222:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 223:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 224:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 226:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 227:

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

  case 228:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 231:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 233:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 237:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 238:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 241:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 242:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 245:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 246:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 247:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 248:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 249:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 256:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 258:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 266:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 267:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 268:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 269:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 270:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 271:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 272:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 273:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 275:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 276:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 277:

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

