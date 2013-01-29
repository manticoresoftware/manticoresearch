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
     TOK_AGENT = 267,
     TOK_AS = 268,
     TOK_ASC = 269,
     TOK_ATTACH = 270,
     TOK_AVG = 271,
     TOK_BEGIN = 272,
     TOK_BETWEEN = 273,
     TOK_BY = 274,
     TOK_CALL = 275,
     TOK_CHARACTER = 276,
     TOK_COLLATION = 277,
     TOK_COMMIT = 278,
     TOK_COMMITTED = 279,
     TOK_COUNT = 280,
     TOK_CREATE = 281,
     TOK_DELETE = 282,
     TOK_DESC = 283,
     TOK_DESCRIBE = 284,
     TOK_DISTINCT = 285,
     TOK_DIV = 286,
     TOK_DROP = 287,
     TOK_FALSE = 288,
     TOK_FLOAT = 289,
     TOK_FLUSH = 290,
     TOK_FROM = 291,
     TOK_FUNCTION = 292,
     TOK_GLOBAL = 293,
     TOK_GROUP = 294,
     TOK_GROUPBY = 295,
     TOK_GROUP_CONCAT = 296,
     TOK_ID = 297,
     TOK_IN = 298,
     TOK_INDEX = 299,
     TOK_INSERT = 300,
     TOK_INT = 301,
     TOK_INTO = 302,
     TOK_ISOLATION = 303,
     TOK_LEVEL = 304,
     TOK_LIKE = 305,
     TOK_LIMIT = 306,
     TOK_MATCH = 307,
     TOK_MAX = 308,
     TOK_META = 309,
     TOK_MIN = 310,
     TOK_MOD = 311,
     TOK_NAMES = 312,
     TOK_NULL = 313,
     TOK_OPTION = 314,
     TOK_ORDER = 315,
     TOK_OPTIMIZE = 316,
     TOK_RAND = 317,
     TOK_READ = 318,
     TOK_REPEATABLE = 319,
     TOK_REPLACE = 320,
     TOK_RETURNS = 321,
     TOK_ROLLBACK = 322,
     TOK_RTINDEX = 323,
     TOK_SELECT = 324,
     TOK_SERIALIZABLE = 325,
     TOK_SET = 326,
     TOK_SESSION = 327,
     TOK_SHOW = 328,
     TOK_SONAME = 329,
     TOK_START = 330,
     TOK_STATUS = 331,
     TOK_STRING = 332,
     TOK_SUM = 333,
     TOK_TABLES = 334,
     TOK_TO = 335,
     TOK_TRANSACTION = 336,
     TOK_TRUE = 337,
     TOK_TRUNCATE = 338,
     TOK_UNCOMMITTED = 339,
     TOK_UPDATE = 340,
     TOK_VALUES = 341,
     TOK_VARIABLES = 342,
     TOK_WARNINGS = 343,
     TOK_WEIGHT = 344,
     TOK_WHERE = 345,
     TOK_WITHIN = 346,
     TOK_OR = 347,
     TOK_AND = 348,
     TOK_NE = 349,
     TOK_GTE = 350,
     TOK_LTE = 351,
     TOK_NOT = 352,
     TOK_NEG = 353
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
#define TOK_AGENT 267
#define TOK_AS 268
#define TOK_ASC 269
#define TOK_ATTACH 270
#define TOK_AVG 271
#define TOK_BEGIN 272
#define TOK_BETWEEN 273
#define TOK_BY 274
#define TOK_CALL 275
#define TOK_CHARACTER 276
#define TOK_COLLATION 277
#define TOK_COMMIT 278
#define TOK_COMMITTED 279
#define TOK_COUNT 280
#define TOK_CREATE 281
#define TOK_DELETE 282
#define TOK_DESC 283
#define TOK_DESCRIBE 284
#define TOK_DISTINCT 285
#define TOK_DIV 286
#define TOK_DROP 287
#define TOK_FALSE 288
#define TOK_FLOAT 289
#define TOK_FLUSH 290
#define TOK_FROM 291
#define TOK_FUNCTION 292
#define TOK_GLOBAL 293
#define TOK_GROUP 294
#define TOK_GROUPBY 295
#define TOK_GROUP_CONCAT 296
#define TOK_ID 297
#define TOK_IN 298
#define TOK_INDEX 299
#define TOK_INSERT 300
#define TOK_INT 301
#define TOK_INTO 302
#define TOK_ISOLATION 303
#define TOK_LEVEL 304
#define TOK_LIKE 305
#define TOK_LIMIT 306
#define TOK_MATCH 307
#define TOK_MAX 308
#define TOK_META 309
#define TOK_MIN 310
#define TOK_MOD 311
#define TOK_NAMES 312
#define TOK_NULL 313
#define TOK_OPTION 314
#define TOK_ORDER 315
#define TOK_OPTIMIZE 316
#define TOK_RAND 317
#define TOK_READ 318
#define TOK_REPEATABLE 319
#define TOK_REPLACE 320
#define TOK_RETURNS 321
#define TOK_ROLLBACK 322
#define TOK_RTINDEX 323
#define TOK_SELECT 324
#define TOK_SERIALIZABLE 325
#define TOK_SET 326
#define TOK_SESSION 327
#define TOK_SHOW 328
#define TOK_SONAME 329
#define TOK_START 330
#define TOK_STATUS 331
#define TOK_STRING 332
#define TOK_SUM 333
#define TOK_TABLES 334
#define TOK_TO 335
#define TOK_TRANSACTION 336
#define TOK_TRUE 337
#define TOK_TRUNCATE 338
#define TOK_UNCOMMITTED 339
#define TOK_UPDATE 340
#define TOK_VALUES 341
#define TOK_VARIABLES 342
#define TOK_WARNINGS 343
#define TOK_WEIGHT 344
#define TOK_WHERE 345
#define TOK_WITHIN 346
#define TOK_OR 347
#define TOK_AND 348
#define TOK_NE 349
#define TOK_GTE 350
#define TOK_LTE 351
#define TOK_NOT 352
#define TOK_NEG 353




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
#define YYFINAL  106
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   891

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  99
/* YYNRULES -- Number of rules. */
#define YYNRULES  276
/* YYNRULES -- Number of states. */
#define YYNSTATES  528

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   353

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   106,    95,     2,
     110,   111,   104,   102,   112,   103,   115,   105,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   109,
      98,    96,    99,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   113,    94,   114,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    97,
     100,   101,   107,   108
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    56,    58,    60,
      62,    72,    73,    74,    78,    79,    82,    87,    98,   100,
     104,   106,   109,   110,   112,   115,   117,   122,   127,   132,
     137,   142,   147,   151,   155,   161,   163,   167,   168,   170,
     173,   175,   179,   183,   188,   192,   196,   202,   209,   213,
     218,   224,   228,   232,   236,   240,   242,   248,   252,   256,
     260,   264,   268,   272,   276,   280,   282,   284,   289,   293,
     297,   299,   301,   304,   306,   309,   311,   315,   316,   318,
     322,   323,   325,   331,   332,   334,   338,   344,   346,   350,
     352,   355,   358,   359,   361,   364,   369,   370,   372,   375,
     377,   381,   385,   389,   395,   402,   406,   408,   412,   416,
     418,   420,   422,   424,   426,   428,   431,   434,   438,   442,
     446,   450,   454,   458,   462,   466,   470,   474,   478,   482,
     486,   490,   494,   498,   502,   506,   510,   512,   517,   522,
     526,   533,   540,   542,   546,   548,   550,   554,   560,   563,
     564,   567,   569,   572,   575,   579,   584,   589,   593,   595,
     597,   599,   601,   603,   605,   609,   614,   619,   624,   628,
     633,   641,   647,   649,   651,   653,   655,   657,   659,   661,
     663,   665,   668,   675,   677,   679,   680,   684,   686,   690,
     692,   696,   700,   702,   706,   708,   710,   712,   716,   719,
     727,   737,   744,   746,   750,   752,   756,   758,   762,   763,
     766,   768,   772,   776,   777,   779,   781,   783,   787,   789,
     791,   795,   802,   804,   808,   812,   816,   822,   827,   832,
     833,   835,   838,   840,   844,   848,   851,   855,   862,   863,
     865,   867,   870,   873,   876,   878,   886,   888,   890,   892,
     896,   903,   907,   911,   913,   917,   921
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     117,     0,    -1,   118,    -1,   119,    -1,   119,   109,    -1,
     172,    -1,   180,    -1,   166,    -1,   167,    -1,   170,    -1,
     181,    -1,   190,    -1,   192,    -1,   193,    -1,   196,    -1,
     201,    -1,   202,    -1,   206,    -1,   208,    -1,   209,    -1,
     210,    -1,   203,    -1,   211,    -1,   213,    -1,   214,    -1,
     120,    -1,   119,   109,   120,    -1,   121,    -1,   161,    -1,
     125,    -1,    69,   126,    36,   110,   122,   125,   111,   123,
     124,    -1,    -1,    -1,    60,    19,   146,    -1,    -1,    51,
       5,    -1,    51,     5,   112,     5,    -1,    69,   126,    36,
     130,   131,   140,   142,   144,   148,   150,    -1,   127,    -1,
     126,   112,   127,    -1,   104,    -1,   129,   128,    -1,    -1,
       3,    -1,    13,     3,    -1,   156,    -1,    16,   110,   156,
     111,    -1,    53,   110,   156,   111,    -1,    55,   110,   156,
     111,    -1,    78,   110,   156,   111,    -1,    41,   110,   156,
     111,    -1,    25,   110,   104,   111,    -1,    89,   110,   111,
      -1,    40,   110,   111,    -1,    25,   110,    30,     3,   111,
      -1,     3,    -1,   130,   112,     3,    -1,    -1,   132,    -1,
      90,   133,    -1,   134,    -1,   133,    93,   133,    -1,   110,
     133,   111,    -1,    52,   110,     8,   111,    -1,   136,    96,
     137,    -1,   136,    97,   137,    -1,   136,    43,   110,   139,
     111,    -1,   136,   107,    43,   110,   139,   111,    -1,   136,
      43,     9,    -1,   136,   107,    43,     9,    -1,   136,    18,
     137,    93,   137,    -1,   136,    99,   137,    -1,   136,    98,
     137,    -1,   136,   100,   137,    -1,   136,   101,   137,    -1,
     135,    -1,   136,    18,   138,    93,   138,    -1,   136,    99,
     138,    -1,   136,    98,   138,    -1,   136,   100,   138,    -1,
     136,   101,   138,    -1,   136,    96,     8,    -1,   136,    97,
       8,    -1,   136,    96,   138,    -1,   136,    97,   138,    -1,
       3,    -1,     4,    -1,    25,   110,   104,   111,    -1,    40,
     110,   111,    -1,    89,   110,   111,    -1,    42,    -1,     5,
      -1,   103,     5,    -1,     6,    -1,   103,     6,    -1,   137,
      -1,   139,   112,   137,    -1,    -1,   141,    -1,    39,    19,
     136,    -1,    -1,   143,    -1,    91,    39,    60,    19,   146,
      -1,    -1,   145,    -1,    60,    19,   146,    -1,    60,    19,
      62,   110,   111,    -1,   147,    -1,   146,   112,   147,    -1,
     136,    -1,   136,    14,    -1,   136,    28,    -1,    -1,   149,
      -1,    51,     5,    -1,    51,     5,   112,     5,    -1,    -1,
     151,    -1,    59,   152,    -1,   153,    -1,   152,   112,   153,
      -1,     3,    96,     3,    -1,     3,    96,     5,    -1,     3,
      96,   110,   154,   111,    -1,     3,    96,     3,   110,     8,
     111,    -1,     3,    96,     8,    -1,   155,    -1,   154,   112,
     155,    -1,     3,    96,   137,    -1,     3,    -1,     4,    -1,
      42,    -1,     5,    -1,     6,    -1,     9,    -1,   103,   156,
      -1,   107,   156,    -1,   156,   102,   156,    -1,   156,   103,
     156,    -1,   156,   104,   156,    -1,   156,   105,   156,    -1,
     156,    98,   156,    -1,   156,    99,   156,    -1,   156,    95,
     156,    -1,   156,    94,   156,    -1,   156,   106,   156,    -1,
     156,    31,   156,    -1,   156,    56,   156,    -1,   156,   101,
     156,    -1,   156,   100,   156,    -1,   156,    96,   156,    -1,
     156,    97,   156,    -1,   156,    93,   156,    -1,   156,    92,
     156,    -1,   110,   156,   111,    -1,   113,   160,   114,    -1,
     157,    -1,     3,   110,   158,   111,    -1,    43,   110,   158,
     111,    -1,     3,   110,   111,    -1,    55,   110,   156,   112,
     156,   111,    -1,    53,   110,   156,   112,   156,   111,    -1,
     159,    -1,   158,   112,   159,    -1,   156,    -1,     8,    -1,
       3,    96,   137,    -1,   160,   112,     3,    96,   137,    -1,
      73,   163,    -1,    -1,    50,     8,    -1,    88,    -1,    76,
     162,    -1,    54,   162,    -1,    12,    76,   162,    -1,    12,
       8,    76,   162,    -1,    12,     3,    76,   162,    -1,    44,
       3,    76,    -1,     3,    -1,    58,    -1,     8,    -1,     5,
      -1,     6,    -1,   164,    -1,   165,   103,   164,    -1,    71,
       3,    96,   169,    -1,    71,     3,    96,   168,    -1,    71,
       3,    96,    58,    -1,    71,    57,   165,    -1,    71,    10,
      96,   165,    -1,    71,    38,     9,    96,   110,   139,   111,
      -1,    71,    38,     3,    96,   168,    -1,     3,    -1,     8,
      -1,    82,    -1,    33,    -1,   137,    -1,    23,    -1,    67,
      -1,   171,    -1,    17,    -1,    75,    81,    -1,   173,    47,
       3,   174,    86,   176,    -1,    45,    -1,    65,    -1,    -1,
     110,   175,   111,    -1,   136,    -1,   175,   112,   136,    -1,
     177,    -1,   176,   112,   177,    -1,   110,   178,   111,    -1,
     179,    -1,   178,   112,   179,    -1,   137,    -1,   138,    -1,
       8,    -1,   110,   139,   111,    -1,   110,   111,    -1,    27,
      36,   130,    90,    42,    96,   137,    -1,    27,    36,   130,
      90,    42,    43,   110,   139,   111,    -1,    20,     3,   110,
     182,   185,   111,    -1,   183,    -1,   182,   112,   183,    -1,
     179,    -1,   110,   184,   111,    -1,     8,    -1,   184,   112,
       8,    -1,    -1,   112,   186,    -1,   187,    -1,   186,   112,
     187,    -1,   179,   188,   189,    -1,    -1,    13,    -1,     3,
      -1,    51,    -1,   191,     3,   162,    -1,    29,    -1,    28,
      -1,    73,    79,   162,    -1,    85,   130,    71,   194,   132,
     150,    -1,   195,    -1,   194,   112,   195,    -1,     3,    96,
     137,    -1,     3,    96,   138,    -1,     3,    96,   110,   139,
     111,    -1,     3,    96,   110,   111,    -1,    73,   204,    87,
     197,    -1,    -1,   198,    -1,    90,   199,    -1,   200,    -1,
     199,    92,   200,    -1,     3,    96,     8,    -1,    73,    22,
      -1,    73,    21,    71,    -1,    71,   204,    81,    48,    49,
     205,    -1,    -1,    38,    -1,    72,    -1,    63,    84,    -1,
      63,    24,    -1,    64,    63,    -1,    70,    -1,    26,    37,
       3,    66,   207,    74,     8,    -1,    46,    -1,    34,    -1,
      77,    -1,    32,    37,     3,    -1,    15,    44,     3,    80,
      68,     3,    -1,    35,    68,     3,    -1,    69,   212,   148,
      -1,    10,    -1,    10,   115,     3,    -1,    83,    68,     3,
      -1,    61,    44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   127,   127,   128,   129,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   158,   159,   163,   164,   168,
     169,   177,   191,   194,   201,   203,   207,   215,   231,   232,
     236,   237,   240,   242,   243,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   260,   261,   264,   266,   270,
     274,   275,   276,   280,   285,   292,   300,   308,   317,   322,
     327,   332,   337,   342,   347,   352,   357,   362,   367,   372,
     377,   382,   387,   395,   396,   400,   401,   406,   412,   418,
     424,   433,   434,   445,   446,   450,   456,   462,   464,   468,
     474,   476,   480,   491,   493,   497,   501,   508,   509,   513,
     514,   515,   518,   520,   524,   529,   536,   538,   542,   546,
     547,   551,   556,   561,   567,   572,   580,   585,   592,   602,
     603,   604,   605,   606,   607,   608,   609,   610,   611,   612,
     613,   614,   615,   616,   617,   618,   619,   620,   621,   622,
     623,   624,   625,   626,   627,   628,   629,   633,   634,   635,
     636,   637,   641,   642,   646,   647,   651,   652,   658,   661,
     663,   667,   668,   669,   670,   671,   676,   681,   691,   692,
     693,   694,   695,   699,   700,   704,   709,   714,   719,   720,
     724,   729,   737,   738,   742,   743,   744,   758,   759,   760,
     764,   765,   771,   779,   780,   783,   785,   789,   790,   794,
     795,   799,   803,   804,   808,   809,   810,   811,   812,   818,
     826,   840,   848,   852,   859,   860,   867,   877,   883,   885,
     889,   894,   898,   905,   907,   911,   912,   918,   926,   927,
     933,   939,   947,   948,   952,   956,   960,   964,   974,   980,
     981,   985,   989,   990,   994,   998,  1005,  1012,  1018,  1019,
    1020,  1024,  1025,  1026,  1027,  1033,  1044,  1045,  1046,  1050,
    1061,  1073,  1084,  1092,  1093,  1102,  1113
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
  "TOK_AGENT", "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", "TOK_BEGIN", 
  "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", "TOK_COLLATION", 
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", 
  "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTO", 
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_RAND", "TOK_READ", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_STRING", "TOK_SUM", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", 
  "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", 
  "'}'", "'.'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select", "subselect_start", "opt_outer_order", 
  "opt_outer_limit", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "expr_float_unhandled", 
  "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "show_stmt", "like_filter", "show_what", 
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
     345,   346,   347,   348,   124,    38,    61,   349,    60,    62,
     350,   351,    43,    45,    42,    47,    37,   352,   353,    59,
      40,    41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   116,   117,   117,   117,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   119,   119,   120,   120,   121,
     121,   122,   123,   123,   124,   124,   124,   125,   126,   126,
     127,   127,   128,   128,   128,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   130,   130,   131,   131,   132,
     133,   133,   133,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   135,   135,   136,   136,   136,   136,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   141,
     142,   142,   143,   144,   144,   145,   145,   146,   146,   147,
     147,   147,   148,   148,   149,   149,   150,   150,   151,   152,
     152,   153,   153,   153,   153,   153,   154,   154,   155,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   157,   157,   157,
     157,   157,   158,   158,   159,   159,   160,   160,   161,   162,
     162,   163,   163,   163,   163,   163,   163,   163,   164,   164,
     164,   164,   164,   165,   165,   166,   166,   166,   166,   166,
     167,   167,   168,   168,   169,   169,   169,   170,   170,   170,
     171,   171,   172,   173,   173,   174,   174,   175,   175,   176,
     176,   177,   178,   178,   179,   179,   179,   179,   179,   180,
     180,   181,   182,   182,   183,   183,   184,   184,   185,   185,
     186,   186,   187,   188,   188,   189,   189,   190,   191,   191,
     192,   193,   194,   194,   195,   195,   195,   195,   196,   197,
     197,   198,   199,   199,   200,   201,   202,   203,   204,   204,
     204,   205,   205,   205,   205,   206,   207,   207,   207,   208,
     209,   210,   211,   212,   212,   213,   214
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       9,     0,     0,     3,     0,     2,     4,    10,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     4,     3,     3,     5,     1,     3,     0,     1,     2,
       1,     3,     3,     4,     3,     3,     5,     6,     3,     4,
       5,     3,     3,     3,     3,     1,     5,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     4,     3,     3,
       1,     1,     2,     1,     2,     1,     3,     0,     1,     3,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     4,     4,     3,
       6,     6,     1,     3,     1,     1,     3,     5,     2,     0,
       2,     1,     2,     2,     3,     4,     4,     3,     1,     1,
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
      24,     0,     0,     0,     0,     0,     0,     0,   129,   130,
     132,   133,   134,   273,     0,     0,     0,     0,   131,     0,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
      38,    42,    45,   156,   112,     0,     0,   259,     0,   260,
       0,     0,     0,   255,   259,     0,   169,   169,   169,   171,
     168,     0,   201,     0,    55,     0,     1,     4,     0,   169,
       0,     0,     0,     0,   269,   271,   276,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     135,   136,     0,     0,     0,     0,     0,    43,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   272,   113,
       0,     0,     0,     0,   178,   181,   182,   180,   179,   183,
     188,     0,     0,     0,   169,   256,     0,     0,   173,   172,
     240,   249,   275,     0,     0,     0,     0,    26,   205,   237,
       0,    91,    93,   216,     0,     0,   214,   215,   224,   228,
     222,     0,     0,   165,   159,   164,     0,   162,   274,     0,
       0,     0,    53,     0,     0,     0,     0,     0,    52,     0,
       0,   154,     0,     0,   155,    31,    57,    39,    44,   146,
     147,   153,   152,   144,   143,   150,   151,   141,   142,   149,
     148,   137,   138,   139,   140,   145,   114,   192,   193,   195,
     187,   194,     0,   196,   186,   185,   189,     0,     0,     0,
       0,   169,   169,   174,   177,   170,     0,   248,   250,     0,
       0,   242,    56,     0,     0,     0,    92,    94,   226,   218,
      95,     0,     0,     0,     0,   267,   266,   268,     0,     0,
     157,     0,    46,     0,    51,    50,   158,    47,     0,    48,
       0,    49,     0,     0,   166,     0,     0,     0,    97,    58,
       0,   191,     0,   184,     0,   176,   175,     0,   251,   252,
       0,     0,   116,    85,    86,     0,     0,    90,     0,   207,
       0,     0,   270,   217,     0,   225,     0,   224,   223,   229,
     230,   221,     0,     0,     0,   163,    54,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    75,     0,     0,   100,
      98,   115,     0,     0,     0,   264,   257,     0,     0,     0,
     244,   245,   243,     0,   241,   117,     0,     0,     0,   206,
       0,     0,   202,   209,    96,   227,   234,     0,     0,   265,
       0,   219,   161,   160,   167,     0,    32,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   103,   101,   190,   262,   261,   263,   254,   253,   247,
       0,     0,   118,   119,     0,    88,    89,   208,     0,     0,
     212,     0,   235,   236,   232,   233,   231,     0,     0,     0,
      34,     0,    62,    61,     0,     0,    68,     0,    81,    64,
      83,    82,    65,    84,    72,    78,    71,    77,    73,    79,
      74,    80,     0,    99,     0,     0,   112,   104,   246,     0,
       0,    87,   211,     0,   210,   220,     0,     0,    30,    63,
       0,     0,     0,    69,     0,     0,     0,   116,   121,   122,
     125,     0,   120,   213,   109,    33,   107,    35,    70,     0,
      76,    66,     0,     0,     0,   105,    37,     0,     0,     0,
     126,   110,   111,     0,     0,    67,   102,     0,     0,     0,
     123,     0,   108,    36,   106,   124,   128,   127
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   306,   440,   478,    26,
      79,    80,   139,    81,   226,   308,   309,   354,   355,   356,
     494,   280,   197,   281,   359,   360,   411,   412,   466,   467,
     495,   496,   158,   159,   374,   375,   422,   423,   509,   510,
      82,    83,   206,   207,   134,    27,   178,   100,   169,   170,
      28,    29,   254,   255,    30,    31,    32,    33,   274,   330,
     382,   383,   429,   198,    34,    35,   199,   200,   282,   284,
     339,   340,   387,   434,    36,    37,    38,    39,   270,   271,
      40,   267,   268,   318,   319,    41,    42,    43,    90,   366,
      44,   288,    45,    46,    47,    48,    84,    49,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -313
static const short yypact[] =
{
     806,    -1,  -313,    27,  -313,    50,    19,  -313,  -313,    63,
     -22,  -313,    78,  -313,  -313,   111,   326,   622,    62,    70,
     195,   222,  -313,   122,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,   198,  -313,  -313,  -313,   268,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,   272,   168,   287,   195,   296,   300,   314,   217,  -313,
    -313,  -313,  -313,   216,   223,   227,   233,   236,  -313,   240,
     241,   246,   247,   251,   263,  -313,   263,   263,   360,   -23,
    -313,   177,   585,  -313,   316,   269,   279,   172,   277,  -313,
     291,    32,   297,  -313,  -313,   374,   338,   338,   338,  -313,
    -313,   306,  -313,   392,  -313,   -51,  -313,   114,   394,   338,
     319,    31,   337,   -52,  -313,  -313,  -313,   231,   401,   263,
       3,   294,   263,   245,   263,   263,   263,   295,   299,   317,
    -313,  -313,   423,   311,   190,     0,   152,  -313,   428,  -313,
     263,   263,   263,   263,   263,   263,   263,   263,   263,   263,
     263,   263,   263,   263,   263,   263,   263,   421,  -313,  -313,
     194,   277,   351,   352,  -313,  -313,  -313,  -313,  -313,  -313,
     346,   402,   379,   380,   338,  -313,   381,   450,  -313,  -313,
    -313,   370,  -313,   456,   473,   152,   132,  -313,   367,  -313,
     410,  -313,  -313,  -313,    53,    20,  -313,  -313,  -313,   371,
    -313,   167,   440,  -313,  -313,   585,   105,  -313,  -313,   449,
     481,   375,  -313,   475,   149,   318,   340,   501,  -313,   263,
     263,  -313,     6,   482,  -313,  -313,    94,  -313,  -313,  -313,
    -313,   636,   662,   688,   714,   697,   697,   276,   276,   276,
     276,   106,   106,  -313,  -313,  -313,   376,  -313,  -313,  -313,
    -313,  -313,   499,  -313,  -313,  -313,   346,   255,   377,   277,
     458,   338,   338,  -313,  -313,  -313,   505,  -313,  -313,   414,
     135,  -313,  -313,   305,   425,   509,  -313,  -313,  -313,  -313,
    -313,   211,   213,    31,   403,  -313,  -313,  -313,   439,     5,
    -313,   245,  -313,   419,  -313,  -313,  -313,  -313,   263,  -313,
     263,  -313,   369,   397,  -313,   437,   466,    93,   497,  -313,
     532,  -313,     6,  -313,   256,  -313,  -313,   442,   447,  -313,
      57,   456,   500,  -313,  -313,   430,   446,  -313,   451,  -313,
     228,   452,  -313,  -313,     6,  -313,   555,   137,  -313,   453,
    -313,  -313,   556,   472,     6,  -313,  -313,   527,   553,     6,
     152,   455,   477,    93,   492,  -313,  -313,   214,   569,   498,
    -313,  -313,   242,    81,   528,  -313,  -313,   582,   505,     1,
    -313,  -313,  -313,   589,  -313,  -313,   504,   502,   503,  -313,
     305,    36,   506,  -313,  -313,  -313,  -313,    16,    36,  -313,
       6,  -313,  -313,  -313,  -313,   -14,   550,   603,    85,    93,
      60,    15,    10,    26,    60,    60,    60,    60,   572,   305,
     578,   575,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
     248,   540,   525,  -313,   529,  -313,  -313,  -313,    21,   273,
    -313,   452,  -313,  -313,  -313,   626,  -313,   275,   195,   623,
     610,   551,  -313,  -313,   570,   576,  -313,     6,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,    18,  -313,   605,   649,   316,  -313,  -313,     9,
     589,  -313,  -313,    36,  -313,  -313,   305,   665,  -313,  -313,
       6,    23,   278,  -313,     6,   652,   239,   500,   562,  -313,
    -313,   670,  -313,  -313,   157,   563,  -313,   583,  -313,   668,
    -313,  -313,   280,   305,   586,   563,  -313,   689,   604,   290,
    -313,  -313,  -313,   305,   694,  -313,   563,   591,   592,     6,
    -313,   670,  -313,  -313,  -313,  -313,  -313,  -313
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -313,  -313,  -313,  -313,   597,  -313,  -313,  -313,  -313,   399,
     356,   571,  -313,  -313,   150,  -313,   438,  -297,  -313,  -313,
    -250,  -111,  -312,  -305,  -313,  -313,  -313,  -313,  -313,  -313,
    -257,   196,   249,  -313,   224,  -313,  -313,   243,  -313,   191,
     -72,  -313,   593,   426,  -313,  -313,   -88,  -313,   461,   560,
    -313,  -313,   457,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,   292,  -313,  -282,  -313,  -313,  -313,   441,  -313,  -313,
    -313,   334,  -313,  -313,  -313,  -313,  -313,  -313,  -313,   404,
    -313,  -313,  -313,  -313,   358,  -313,  -313,  -313,   710,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -234
static const short yytable[] =
{
     196,   337,   130,   104,   131,   132,   191,   362,   371,   179,
     180,   191,   488,   135,   489,   191,   192,   490,   448,   432,
     183,   189,   438,   329,   446,   191,   191,   483,   278,   192,
      52,   191,   192,   210,   451,   172,   191,   192,   202,   193,
     173,   191,   192,    51,   193,   205,    56,   209,   343,   253,
     213,   205,   215,   216,   217,    54,   398,   357,   276,   277,
     184,   184,   191,   192,   420,   191,   192,   433,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   437,   263,    53,   445,   136,
     450,   453,   455,   457,   459,   461,   323,   324,   136,   430,
      55,   344,   443,   357,   252,   414,   435,   211,   174,   252,
     225,   304,   419,   194,    58,    59,    60,    61,   325,   491,
      62,    63,    57,   252,   252,   447,   499,    64,   484,   194,
     427,   279,   279,   326,   194,   327,    65,   140,   103,   194,
    -233,   195,   482,   102,    91,   352,   428,   302,   303,   357,
     386,    66,    67,    68,    69,    58,    59,    60,    61,   463,
     194,    62,   141,   194,    70,   415,    71,   369,    64,   500,
     105,   511,   196,   315,   316,   162,    95,    65,   399,   502,
     137,   163,   328,   185,   307,   512,    96,   186,  -233,    72,
     138,   493,    66,    67,    68,    69,   442,   247,   104,   191,
      73,   285,   248,   353,   113,    70,   184,    71,    97,   370,
     154,   155,   156,   286,    74,    75,   290,   291,    76,   205,
      99,    77,   106,   384,    78,   307,   347,   249,   348,   505,
      72,   107,   400,   391,    58,    59,    60,    61,   394,   203,
      62,    73,   323,   324,   287,   108,   516,   321,    58,    59,
      60,    61,   250,   203,    62,    74,    75,   401,   247,    76,
     296,   291,    77,   248,   325,    78,    58,    59,    60,    61,
     196,   109,    62,    68,    69,   110,   251,   196,   111,   326,
     164,   327,   165,   166,   128,   167,   129,    68,    69,   444,
     112,   449,   452,   454,   456,   458,   460,   252,   128,   114,
     129,   504,   223,   115,   224,    68,    69,   140,   323,   324,
     402,   403,   404,   405,   406,   407,   128,   116,   129,   363,
     364,   408,   333,   334,   335,   336,   365,   117,   328,    85,
     325,   118,   141,   119,    74,   168,    86,   120,    76,   379,
     380,    77,   204,   121,    78,   326,   122,   327,    74,   140,
     123,   124,    76,   413,   334,    77,   125,   126,    78,   468,
     334,   127,   196,   133,    87,   160,    74,   157,   175,   498,
      76,   140,   171,    77,   141,   161,    78,   176,   152,   153,
     154,   155,   156,    88,   472,   473,   475,   334,   177,   501,
     334,   515,   334,   181,   328,   182,   141,   188,    89,   190,
     140,   520,   521,   201,   208,   212,   218,   222,   526,   219,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   141,   246,   220,   140,   297,
     298,   228,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   257,   258,   259,
     260,   299,   300,   141,   140,   261,   262,   264,   265,   269,
     266,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   272,   273,   275,   141,
     140,   298,   289,   283,   293,   305,   294,   312,   310,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   276,   141,   140,   314,   317,   300,
     320,   331,   332,   342,   341,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     346,   141,   140,   349,   221,   350,   358,   361,   367,   368,
     376,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   377,   141,   140,   373,
     292,   378,   381,   385,   389,   388,   396,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   390,   141,   140,   399,   295,   397,   409,   410,
     417,   416,   421,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   424,   141,
     439,   441,   301,   425,   426,   462,   140,   464,   431,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,    91,   465,   469,   470,   392,   386,
     471,   141,   476,    92,    93,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
      94,   477,   479,   480,   393,   485,    95,   140,   486,   481,
     497,   503,   507,   508,   277,   513,    96,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   141,   140,    89,   514,   517,   518,    97,   523,
     519,    98,   524,   525,   187,   351,   395,   227,   322,   522,
      99,   506,   527,   492,   311,   487,   214,   345,   141,   140,
     313,   256,   436,   474,   338,   372,   418,   101,   140,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,     0,   141,   140,     0,     0,     0,     0,
       0,     0,     0,   141,     0,     0,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,     0,
     141,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   148,   149,   150,   151,   152,
     153,   154,   155,   156,     0,     0,     0,     0,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,     1,     0,     2,     0,     0,     3,     0,     0,     4,
       0,     0,     5,     6,     7,     8,     0,     0,     9,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,    13,     0,    14,     0,    15,     0,    16,     0,    17,
       0,    18,     0,     0,     0,     0,     0,     0,     0,    19,
       0,    20
};

static const short yycheck[] =
{
     111,   283,    74,     3,    76,    77,     5,   312,   320,    97,
      98,     5,     3,    36,     5,     5,     6,     8,     8,     3,
      71,   109,    36,   273,     9,     5,     5,     9,     8,     6,
       3,     5,     6,    30,     8,     3,     5,     6,    90,     8,
       8,     5,     6,    44,     8,   117,    68,   119,    43,   160,
     122,   123,   124,   125,   126,    36,   353,   307,     5,     6,
     112,   112,     5,     6,   369,     5,     6,    51,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   390,   174,    37,   400,   112,
     402,   403,   404,   405,   406,   407,     3,     4,   112,   381,
      37,    96,   399,   353,   103,    24,   388,   104,    76,   103,
     110,   222,   111,   103,     3,     4,     5,     6,    25,   110,
       9,    10,    44,   103,   103,   110,   103,    16,   110,   103,
     380,   111,   111,    40,   103,    42,    25,    31,    68,   103,
       3,   110,   447,    81,    12,    52,   110,   219,   220,   399,
      13,    40,    41,    42,    43,     3,     4,     5,     6,   409,
     103,     9,    56,   103,    53,    84,    55,   110,    16,   481,
      20,    14,   283,   261,   262,     3,    44,    25,    93,   484,
       3,     9,    89,    69,    90,    28,    54,    73,    51,    78,
      13,   473,    40,    41,    42,    43,   111,     3,     3,     5,
      89,    34,     8,   110,    54,    53,   112,    55,    76,   320,
     104,   105,   106,    46,   103,   104,   111,   112,   107,   291,
      88,   110,     0,   334,   113,    90,   298,    33,   300,   486,
      78,   109,    18,   344,     3,     4,     5,     6,   349,     8,
       9,    89,     3,     4,    77,    47,   503,   112,     3,     4,
       5,     6,    58,     8,     9,   103,   104,    43,     3,   107,
     111,   112,   110,     8,    25,   113,     3,     4,     5,     6,
     381,     3,     9,    42,    43,     3,    82,   388,   110,    40,
       3,    42,     5,     6,    53,     8,    55,    42,    43,   400,
       3,   402,   403,   404,   405,   406,   407,   103,    53,     3,
      55,    62,   112,     3,   114,    42,    43,    31,     3,     4,
      96,    97,    98,    99,   100,   101,    53,     3,    55,    63,
      64,   107,   111,   112,   111,   112,    70,   110,    89,     3,
      25,   115,    56,   110,   103,    58,    10,   110,   107,   111,
     112,   110,   111,   110,   113,    40,   110,    42,   103,    31,
     110,   110,   107,   111,   112,   110,   110,   110,   113,   111,
     112,   110,   473,     3,    38,    96,   103,    51,    71,   480,
     107,    31,    81,   110,    56,    96,   113,     3,   102,   103,
     104,   105,   106,    57,   111,   112,   111,   112,    50,   111,
     112,   111,   112,    87,    89,     3,    56,     3,    72,    80,
      31,   111,   112,    66,     3,   111,   111,    96,   519,   110,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    56,     5,   110,    31,   111,
     112,     3,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    96,    96,   103,
      48,   111,   112,    56,    31,    76,    76,    76,     8,     3,
      90,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,     3,   110,    68,    56,
      31,   112,    42,   112,     3,     3,   111,   110,   112,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,     5,    56,    31,    49,     3,   112,
      96,    86,     3,    74,   111,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     111,    56,    31,    96,   111,    69,    39,     5,    96,    92,
     110,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   110,    56,    31,    59,
     111,   110,   110,     8,     8,   112,   111,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   110,    56,    31,    93,   111,   110,    19,    91,
       8,    63,     3,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   104,    56,
      60,     8,   111,   111,   111,    43,    31,    39,   112,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    12,    60,    96,   112,   111,    13,
     111,    56,    19,    21,    22,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      38,    51,   111,    93,   111,    60,    44,    31,    19,    93,
       5,    19,   110,     3,     6,   112,    54,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,    56,    31,    72,   112,   110,     8,    76,     5,
      96,    79,   111,   111,   107,   306,   350,   136,   270,   513,
      88,   487,   521,   470,   257,   466,   123,   291,    56,    31,
     259,   161,   388,   431,   283,   321,   368,    17,    31,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,    -1,    56,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    -1,    -1,    -1,    -1,    -1,    -1,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    15,    -1,    17,    -1,    -1,    20,    -1,    -1,    23,
      -1,    -1,    26,    27,    28,    29,    -1,    -1,    32,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,    -1,
      -1,    65,    -1,    67,    -1,    69,    -1,    71,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      -1,    85
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    61,    65,    67,    69,    71,    73,    75,    83,
      85,   117,   118,   119,   120,   121,   125,   161,   166,   167,
     170,   171,   172,   173,   180,   181,   190,   191,   192,   193,
     196,   201,   202,   203,   206,   208,   209,   210,   211,   213,
     214,    44,     3,    37,    36,    37,    68,    44,     3,     4,
       5,     6,     9,    10,    16,    25,    40,    41,    42,    43,
      53,    55,    78,    89,   103,   104,   107,   110,   113,   126,
     127,   129,   156,   157,   212,     3,    10,    38,    57,    72,
     204,    12,    21,    22,    38,    44,    54,    76,    79,    88,
     163,   204,    81,    68,     3,   130,     0,   109,    47,     3,
       3,   110,     3,   130,     3,     3,     3,   110,   115,   110,
     110,   110,   110,   110,   110,   110,   110,   110,    53,    55,
     156,   156,   156,     3,   160,    36,   112,     3,    13,   128,
      31,    56,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    51,   148,   149,
      96,    96,     3,     9,     3,     5,     6,     8,    58,   164,
     165,    81,     3,     8,    76,    71,     3,    50,   162,   162,
     162,    87,     3,    71,   112,    69,    73,   120,     3,   162,
      80,     5,     6,     8,   103,   110,   137,   138,   179,   182,
     183,    66,    90,     8,   111,   156,   158,   159,     3,   156,
      30,   104,   111,   156,   158,   156,   156,   156,   111,   110,
     110,   111,    96,   112,   114,   110,   130,   127,     3,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,     5,     3,     8,    33,
      58,    82,   103,   137,   168,   169,   165,    96,    96,   103,
      48,    76,    76,   162,    76,     8,    90,   197,   198,     3,
     194,   195,     3,   110,   174,    68,     5,     6,     8,   111,
     137,   139,   184,   112,   185,    34,    46,    77,   207,    42,
     111,   112,   111,     3,   111,   111,   111,   111,   112,   111,
     112,   111,   156,   156,   137,     3,   122,    90,   131,   132,
     112,   168,   110,   164,    49,   162,   162,     3,   199,   200,
      96,   112,   132,     3,     4,    25,    40,    42,    89,   136,
     175,    86,     3,   111,   112,   111,   112,   179,   183,   186,
     187,   111,    74,    43,    96,   159,   111,   156,   156,    96,
      69,   125,    52,   110,   133,   134,   135,   136,    39,   140,
     141,     5,   139,    63,    64,    70,   205,    96,    92,   110,
     137,   138,   195,    59,   150,   151,   110,   110,   110,   111,
     112,   110,   176,   177,   137,     8,    13,   188,   112,     8,
     110,   137,   111,   111,   137,   126,   111,   110,   133,    93,
      18,    43,    96,    97,    98,    99,   100,   101,   107,    19,
      91,   142,   143,   111,    24,    84,    63,     8,   200,   111,
     139,     3,   152,   153,   104,   111,   111,   136,   110,   178,
     179,   112,     3,    51,   189,   179,   187,   139,    36,    60,
     123,     8,   111,   133,   137,   138,     9,   110,     8,   137,
     138,     8,   137,   138,   137,   138,   137,   138,   137,   138,
     137,   138,    43,   136,    39,    60,   144,   145,   111,    96,
     112,   111,   111,   112,   177,   111,    19,    51,   124,   111,
      93,    93,   139,     9,   110,    60,    19,   148,     3,     5,
       8,   110,   153,   179,   136,   146,   147,     5,   137,   103,
     138,   111,   139,    19,    62,   146,   150,   110,     3,   154,
     155,    14,    28,   112,   112,   111,   146,   110,     8,    96,
     111,   112,   147,     5,   111,   111,   137,   155
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
			pParser->m_pQuery->m_sOuterOrderBy = pParser->m_pQuery->m_sOrderBy;
		;}
    break;

  case 33:

    {
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 35:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 36:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 37:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 40:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 43:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 44:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 45:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 46:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 47:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 48:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 49:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 50:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 51:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 52:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 53:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 54:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 56:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 63:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 64:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 65:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 66:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 67:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 68:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 73:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyerror ( pParser, "EQ and NEQ filters on floats are not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 76:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 87:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 88:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 89:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 90:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 91:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 92:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 93:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 94:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 95:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 96:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 99:

    {
			pParser->SetGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 102:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 105:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 106:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 108:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 110:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 111:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 114:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 115:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 123:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 124:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 125:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 126:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 127:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 128:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 130:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 131:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 135:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 155:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 157:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 158:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 159:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 160:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 161:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
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
			pParser->m_pStmt->m_eStmt = STMT_DESC;
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
			tStmt.m_eStmt = STMT_CREATE_FUNC;
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
			tStmt.m_eStmt = STMT_DROP_FUNC;
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

