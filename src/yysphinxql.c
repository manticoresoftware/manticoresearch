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
     TOK_PROFILE = 317,
     TOK_RAND = 318,
     TOK_READ = 319,
     TOK_REPEATABLE = 320,
     TOK_REPLACE = 321,
     TOK_RETURNS = 322,
     TOK_ROLLBACK = 323,
     TOK_RTINDEX = 324,
     TOK_SELECT = 325,
     TOK_SERIALIZABLE = 326,
     TOK_SET = 327,
     TOK_SESSION = 328,
     TOK_SHOW = 329,
     TOK_SONAME = 330,
     TOK_START = 331,
     TOK_STATUS = 332,
     TOK_STRING = 333,
     TOK_SUM = 334,
     TOK_TABLES = 335,
     TOK_TO = 336,
     TOK_TRANSACTION = 337,
     TOK_TRUE = 338,
     TOK_TRUNCATE = 339,
     TOK_UNCOMMITTED = 340,
     TOK_UPDATE = 341,
     TOK_VALUES = 342,
     TOK_VARIABLES = 343,
     TOK_WARNINGS = 344,
     TOK_WEIGHT = 345,
     TOK_WHERE = 346,
     TOK_WITHIN = 347,
     TOK_OR = 348,
     TOK_AND = 349,
     TOK_NE = 350,
     TOK_GTE = 351,
     TOK_LTE = 352,
     TOK_NOT = 353,
     TOK_NEG = 354
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
#define TOK_PROFILE 317
#define TOK_RAND 318
#define TOK_READ 319
#define TOK_REPEATABLE 320
#define TOK_REPLACE 321
#define TOK_RETURNS 322
#define TOK_ROLLBACK 323
#define TOK_RTINDEX 324
#define TOK_SELECT 325
#define TOK_SERIALIZABLE 326
#define TOK_SET 327
#define TOK_SESSION 328
#define TOK_SHOW 329
#define TOK_SONAME 330
#define TOK_START 331
#define TOK_STATUS 332
#define TOK_STRING 333
#define TOK_SUM 334
#define TOK_TABLES 335
#define TOK_TO 336
#define TOK_TRANSACTION 337
#define TOK_TRUE 338
#define TOK_TRUNCATE 339
#define TOK_UNCOMMITTED 340
#define TOK_UPDATE 341
#define TOK_VALUES 342
#define TOK_VARIABLES 343
#define TOK_WARNINGS 344
#define TOK_WEIGHT 345
#define TOK_WHERE 346
#define TOK_WITHIN 347
#define TOK_OR 348
#define TOK_AND 349
#define TOK_NE 350
#define TOK_GTE 351
#define TOK_LTE 352
#define TOK_NOT 353
#define TOK_NEG 354




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
#define YYLAST   936

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  117
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  99
/* YYNRULES -- Number of rules. */
#define YYNRULES  277
/* YYNRULES -- Number of states. */
#define YYNSTATES  529

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   354

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   107,    96,     2,
     111,   112,   105,   103,   113,   104,   116,   106,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   110,
      99,    97,   100,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   114,    95,   115,     2,     2,     2,     2,
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
      98,   101,   102,   108,   109
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
     564,   567,   569,   572,   575,   579,   581,   586,   591,   595,
     597,   599,   601,   603,   605,   607,   611,   616,   621,   626,
     630,   635,   643,   649,   651,   653,   655,   657,   659,   661,
     663,   665,   667,   670,   677,   679,   681,   682,   686,   688,
     692,   694,   698,   702,   704,   708,   710,   712,   714,   718,
     721,   729,   739,   746,   748,   752,   754,   758,   760,   764,
     765,   768,   770,   774,   778,   779,   781,   783,   785,   789,
     791,   793,   797,   804,   806,   810,   814,   818,   824,   829,
     834,   835,   837,   840,   842,   846,   850,   853,   857,   864,
     865,   867,   869,   872,   875,   878,   880,   888,   890,   892,
     894,   898,   905,   909,   913,   915,   919,   923
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     118,     0,    -1,   119,    -1,   120,    -1,   120,   110,    -1,
     173,    -1,   181,    -1,   167,    -1,   168,    -1,   171,    -1,
     182,    -1,   191,    -1,   193,    -1,   194,    -1,   197,    -1,
     202,    -1,   203,    -1,   207,    -1,   209,    -1,   210,    -1,
     211,    -1,   204,    -1,   212,    -1,   214,    -1,   215,    -1,
     121,    -1,   120,   110,   121,    -1,   122,    -1,   162,    -1,
     126,    -1,    70,   127,    36,   111,   123,   126,   112,   124,
     125,    -1,    -1,    -1,    60,    19,   147,    -1,    -1,    51,
       5,    -1,    51,     5,   113,     5,    -1,    70,   127,    36,
     131,   132,   141,   143,   145,   149,   151,    -1,   128,    -1,
     127,   113,   128,    -1,   105,    -1,   130,   129,    -1,    -1,
       3,    -1,    13,     3,    -1,   157,    -1,    16,   111,   157,
     112,    -1,    53,   111,   157,   112,    -1,    55,   111,   157,
     112,    -1,    79,   111,   157,   112,    -1,    41,   111,   157,
     112,    -1,    25,   111,   105,   112,    -1,    90,   111,   112,
      -1,    40,   111,   112,    -1,    25,   111,    30,     3,   112,
      -1,     3,    -1,   131,   113,     3,    -1,    -1,   133,    -1,
      91,   134,    -1,   135,    -1,   134,    94,   134,    -1,   111,
     134,   112,    -1,    52,   111,     8,   112,    -1,   137,    97,
     138,    -1,   137,    98,   138,    -1,   137,    43,   111,   140,
     112,    -1,   137,   108,    43,   111,   140,   112,    -1,   137,
      43,     9,    -1,   137,   108,    43,     9,    -1,   137,    18,
     138,    94,   138,    -1,   137,   100,   138,    -1,   137,    99,
     138,    -1,   137,   101,   138,    -1,   137,   102,   138,    -1,
     136,    -1,   137,    18,   139,    94,   139,    -1,   137,   100,
     139,    -1,   137,    99,   139,    -1,   137,   101,   139,    -1,
     137,   102,   139,    -1,   137,    97,     8,    -1,   137,    98,
       8,    -1,   137,    97,   139,    -1,   137,    98,   139,    -1,
       3,    -1,     4,    -1,    25,   111,   105,   112,    -1,    40,
     111,   112,    -1,    90,   111,   112,    -1,    42,    -1,     5,
      -1,   104,     5,    -1,     6,    -1,   104,     6,    -1,   138,
      -1,   140,   113,   138,    -1,    -1,   142,    -1,    39,    19,
     137,    -1,    -1,   144,    -1,    92,    39,    60,    19,   147,
      -1,    -1,   146,    -1,    60,    19,   147,    -1,    60,    19,
      63,   111,   112,    -1,   148,    -1,   147,   113,   148,    -1,
     137,    -1,   137,    14,    -1,   137,    28,    -1,    -1,   150,
      -1,    51,     5,    -1,    51,     5,   113,     5,    -1,    -1,
     152,    -1,    59,   153,    -1,   154,    -1,   153,   113,   154,
      -1,     3,    97,     3,    -1,     3,    97,     5,    -1,     3,
      97,   111,   155,   112,    -1,     3,    97,     3,   111,     8,
     112,    -1,     3,    97,     8,    -1,   156,    -1,   155,   113,
     156,    -1,     3,    97,   138,    -1,     3,    -1,     4,    -1,
      42,    -1,     5,    -1,     6,    -1,     9,    -1,   104,   157,
      -1,   108,   157,    -1,   157,   103,   157,    -1,   157,   104,
     157,    -1,   157,   105,   157,    -1,   157,   106,   157,    -1,
     157,    99,   157,    -1,   157,   100,   157,    -1,   157,    96,
     157,    -1,   157,    95,   157,    -1,   157,   107,   157,    -1,
     157,    31,   157,    -1,   157,    56,   157,    -1,   157,   102,
     157,    -1,   157,   101,   157,    -1,   157,    97,   157,    -1,
     157,    98,   157,    -1,   157,    94,   157,    -1,   157,    93,
     157,    -1,   111,   157,   112,    -1,   114,   161,   115,    -1,
     158,    -1,     3,   111,   159,   112,    -1,    43,   111,   159,
     112,    -1,     3,   111,   112,    -1,    55,   111,   157,   113,
     157,   112,    -1,    53,   111,   157,   113,   157,   112,    -1,
     160,    -1,   159,   113,   160,    -1,   157,    -1,     8,    -1,
       3,    97,   138,    -1,   161,   113,     3,    97,   138,    -1,
      74,   164,    -1,    -1,    50,     8,    -1,    89,    -1,    77,
     163,    -1,    54,   163,    -1,    12,    77,   163,    -1,    62,
      -1,    12,     8,    77,   163,    -1,    12,     3,    77,   163,
      -1,    44,     3,    77,    -1,     3,    -1,    58,    -1,     8,
      -1,     5,    -1,     6,    -1,   165,    -1,   166,   104,   165,
      -1,    72,     3,    97,   170,    -1,    72,     3,    97,   169,
      -1,    72,     3,    97,    58,    -1,    72,    57,   166,    -1,
      72,    10,    97,   166,    -1,    72,    38,     9,    97,   111,
     140,   112,    -1,    72,    38,     3,    97,   169,    -1,     3,
      -1,     8,    -1,    83,    -1,    33,    -1,   138,    -1,    23,
      -1,    68,    -1,   172,    -1,    17,    -1,    76,    82,    -1,
     174,    47,     3,   175,    87,   177,    -1,    45,    -1,    66,
      -1,    -1,   111,   176,   112,    -1,   137,    -1,   176,   113,
     137,    -1,   178,    -1,   177,   113,   178,    -1,   111,   179,
     112,    -1,   180,    -1,   179,   113,   180,    -1,   138,    -1,
     139,    -1,     8,    -1,   111,   140,   112,    -1,   111,   112,
      -1,    27,    36,   131,    91,    42,    97,   138,    -1,    27,
      36,   131,    91,    42,    43,   111,   140,   112,    -1,    20,
       3,   111,   183,   186,   112,    -1,   184,    -1,   183,   113,
     184,    -1,   180,    -1,   111,   185,   112,    -1,     8,    -1,
     185,   113,     8,    -1,    -1,   113,   187,    -1,   188,    -1,
     187,   113,   188,    -1,   180,   189,   190,    -1,    -1,    13,
      -1,     3,    -1,    51,    -1,   192,     3,   163,    -1,    29,
      -1,    28,    -1,    74,    80,   163,    -1,    86,   131,    72,
     195,   133,   151,    -1,   196,    -1,   195,   113,   196,    -1,
       3,    97,   138,    -1,     3,    97,   139,    -1,     3,    97,
     111,   140,   112,    -1,     3,    97,   111,   112,    -1,    74,
     205,    88,   198,    -1,    -1,   199,    -1,    91,   200,    -1,
     201,    -1,   200,    93,   201,    -1,     3,    97,     8,    -1,
      74,    22,    -1,    74,    21,    72,    -1,    72,   205,    82,
      48,    49,   206,    -1,    -1,    38,    -1,    73,    -1,    64,
      85,    -1,    64,    24,    -1,    65,    64,    -1,    71,    -1,
      26,    37,     3,    67,   208,    75,     8,    -1,    46,    -1,
      34,    -1,    78,    -1,    32,    37,     3,    -1,    15,    44,
       3,    81,    69,     3,    -1,    35,    69,     3,    -1,    70,
     213,   149,    -1,    10,    -1,    10,   116,     3,    -1,    84,
      69,     3,    -1,    61,    44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   128,   128,   129,   130,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   159,   160,   164,   165,   169,
     170,   178,   192,   195,   202,   204,   208,   216,   232,   233,
     237,   238,   241,   243,   244,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   261,   262,   265,   267,   271,
     275,   276,   277,   281,   286,   293,   301,   309,   318,   323,
     328,   333,   338,   343,   348,   353,   358,   363,   368,   373,
     378,   383,   388,   396,   397,   401,   402,   407,   413,   419,
     425,   434,   435,   446,   447,   451,   457,   463,   465,   469,
     475,   477,   481,   492,   494,   498,   502,   509,   510,   514,
     515,   516,   519,   521,   525,   530,   537,   539,   543,   547,
     548,   552,   557,   562,   568,   573,   581,   586,   593,   603,
     604,   605,   606,   607,   608,   609,   610,   611,   612,   613,
     614,   615,   616,   617,   618,   619,   620,   621,   622,   623,
     624,   625,   626,   627,   628,   629,   630,   634,   635,   636,
     637,   638,   642,   643,   647,   648,   652,   653,   659,   662,
     664,   668,   669,   670,   671,   672,   673,   678,   683,   693,
     694,   695,   696,   697,   701,   702,   706,   711,   716,   721,
     722,   726,   731,   739,   740,   744,   745,   746,   760,   761,
     762,   766,   767,   773,   781,   782,   785,   787,   791,   792,
     796,   797,   801,   805,   806,   810,   811,   812,   813,   814,
     820,   828,   842,   850,   854,   861,   862,   869,   879,   885,
     887,   891,   896,   900,   907,   909,   913,   914,   920,   928,
     929,   935,   941,   949,   950,   954,   958,   962,   966,   976,
     982,   983,   987,   991,   992,   996,  1000,  1007,  1014,  1020,
    1021,  1022,  1026,  1027,  1028,  1029,  1035,  1046,  1047,  1048,
    1052,  1063,  1075,  1086,  1094,  1095,  1104,  1115
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
  "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_PROFILE", "TOK_RAND", 
  "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLES", "TOK_TO", 
  "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", 
  "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", 
  "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", 
  "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", 
  "','", "'{'", "'}'", "'.'", "$accept", "request", "statement", 
  "multi_stmt_list", "multi_stmt", "select", "subselect_start", 
  "opt_outer_order", "opt_outer_limit", "select_from", 
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
     345,   346,   347,   348,   349,   124,    38,    61,   350,    60,
      62,   351,   352,    43,    45,    42,    47,    37,   353,   354,
      59,    40,    41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   117,   118,   118,   118,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   120,   120,   121,   121,   122,
     122,   123,   124,   124,   125,   125,   125,   126,   127,   127,
     128,   128,   129,   129,   129,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   131,   131,   132,   132,   133,
     134,   134,   134,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   136,   136,   137,   137,   137,   137,   137,
     137,   138,   138,   139,   139,   140,   140,   141,   141,   142,
     143,   143,   144,   145,   145,   146,   146,   147,   147,   148,
     148,   148,   149,   149,   150,   150,   151,   151,   152,   153,
     153,   154,   154,   154,   154,   154,   155,   155,   156,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   158,   158,   158,
     158,   158,   159,   159,   160,   160,   161,   161,   162,   163,
     163,   164,   164,   164,   164,   164,   164,   164,   164,   165,
     165,   165,   165,   165,   166,   166,   167,   167,   167,   167,
     167,   168,   168,   169,   169,   170,   170,   170,   171,   171,
     171,   172,   172,   173,   174,   174,   175,   175,   176,   176,
     177,   177,   178,   179,   179,   180,   180,   180,   180,   180,
     181,   181,   182,   183,   183,   184,   184,   185,   185,   186,
     186,   187,   187,   188,   189,   189,   190,   190,   191,   192,
     192,   193,   194,   195,   195,   196,   196,   196,   196,   197,
     198,   198,   199,   200,   200,   201,   202,   203,   204,   205,
     205,   205,   206,   206,   206,   206,   207,   208,   208,   208,
     209,   210,   211,   212,   213,   213,   214,   215
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
      24,     0,     0,     0,     0,     0,     0,     0,   129,   130,
     132,   133,   134,   274,     0,     0,     0,     0,   131,     0,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
      38,    42,    45,   156,   112,     0,     0,   260,     0,   261,
       0,     0,     0,   256,   260,     0,   169,   175,   169,   169,
     171,   168,     0,   202,     0,    55,     0,     1,     4,     0,
     169,     0,     0,     0,     0,   270,   272,   277,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   135,   136,     0,     0,     0,     0,     0,    43,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   273,
     113,     0,     0,     0,     0,   179,   182,   183,   181,   180,
     184,   189,     0,     0,     0,   169,   257,     0,     0,   173,
     172,   241,   250,   276,     0,     0,     0,     0,    26,   206,
     238,     0,    91,    93,   217,     0,     0,   215,   216,   225,
     229,   223,     0,     0,   165,   159,   164,     0,   162,   275,
       0,     0,     0,    53,     0,     0,     0,     0,     0,    52,
       0,     0,   154,     0,     0,   155,    31,    57,    39,    44,
     146,   147,   153,   152,   144,   143,   150,   151,   141,   142,
     149,   148,   137,   138,   139,   140,   145,   114,   193,   194,
     196,   188,   195,     0,   197,   187,   186,   190,     0,     0,
       0,     0,   169,   169,   174,   178,   170,     0,   249,   251,
       0,     0,   243,    56,     0,     0,     0,    92,    94,   227,
     219,    95,     0,     0,     0,     0,   268,   267,   269,     0,
       0,   157,     0,    46,     0,    51,    50,   158,    47,     0,
      48,     0,    49,     0,     0,   166,     0,     0,     0,    97,
      58,     0,   192,     0,   185,     0,   177,   176,     0,   252,
     253,     0,     0,   116,    85,    86,     0,     0,    90,     0,
     208,     0,     0,   271,   218,     0,   226,     0,   225,   224,
     230,   231,   222,     0,     0,     0,   163,    54,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    75,     0,     0,
     100,    98,   115,     0,     0,     0,   265,   258,     0,     0,
       0,   245,   246,   244,     0,   242,   117,     0,     0,     0,
     207,     0,     0,   203,   210,    96,   228,   235,     0,     0,
     266,     0,   220,   161,   160,   167,     0,    32,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   103,   101,   191,   263,   262,   264,   255,   254,
     248,     0,     0,   118,   119,     0,    88,    89,   209,     0,
       0,   213,     0,   236,   237,   233,   234,   232,     0,     0,
       0,    34,     0,    62,    61,     0,     0,    68,     0,    81,
      64,    83,    82,    65,    84,    72,    78,    71,    77,    73,
      79,    74,    80,     0,    99,     0,     0,   112,   104,   247,
       0,     0,    87,   212,     0,   211,   221,     0,     0,    30,
      63,     0,     0,     0,    69,     0,     0,     0,   116,   121,
     122,   125,     0,   120,   214,   109,    33,   107,    35,    70,
       0,    76,    66,     0,     0,     0,   105,    37,     0,     0,
       0,   126,   110,   111,     0,     0,    67,   102,     0,     0,
       0,   123,     0,   108,    36,   106,   124,   128,   127
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
#define YYPACT_NINF -335
static const short yypact[] =
{
     850,   -28,  -335,    29,  -335,    30,    62,  -335,  -335,    69,
     -21,  -335,    65,  -335,  -335,   110,   635,   625,    47,    76,
     154,   168,  -335,    93,  -335,  -335,  -335,  -335,  -335,  -335,
    -335,  -335,  -335,   139,  -335,  -335,  -335,   213,  -335,  -335,
    -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335,
    -335,   217,   111,   224,   154,   231,   238,   247,   152,  -335,
    -335,  -335,  -335,   158,   165,   171,   173,   177,  -335,   179,
     186,   195,   227,   232,   323,  -335,   323,   323,   299,   -24,
    -335,   184,   633,  -335,   294,   249,   251,   182,   190,  -335,
     273,    94,   280,  -335,  -335,   354,   309,  -335,   309,   309,
    -335,  -335,   283,  -335,   358,  -335,   -26,  -335,   -35,   360,
     309,   287,    23,   306,   -50,  -335,  -335,  -335,   135,   372,
     323,    12,   274,   323,   256,   323,   323,   323,   276,   284,
     285,  -335,  -335,   423,   290,    86,    11,   226,  -335,   394,
    -335,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   393,  -335,
    -335,   275,   190,   308,   310,  -335,  -335,  -335,  -335,  -335,
    -335,   302,   355,   348,   349,   309,  -335,   356,   424,  -335,
    -335,  -335,   344,  -335,   433,   435,   226,   163,  -335,   345,
    -335,   388,  -335,  -335,  -335,   239,    19,  -335,  -335,  -335,
     347,  -335,   130,   434,  -335,  -335,   633,   159,  -335,  -335,
     451,   474,   366,  -335,   477,   174,   316,   346,   503,  -335,
     323,   323,  -335,    21,   480,  -335,  -335,    89,  -335,  -335,
    -335,  -335,   647,   660,   737,   748,   757,   757,   150,   150,
     150,   150,   106,   106,  -335,  -335,  -335,   367,  -335,  -335,
    -335,  -335,  -335,   479,  -335,  -335,  -335,   302,   202,   374,
     190,   437,   309,   309,  -335,  -335,  -335,   484,  -335,  -335,
     391,    91,  -335,  -335,   314,   402,   487,  -335,  -335,  -335,
    -335,  -335,   191,   200,    23,   379,  -335,  -335,  -335,   436,
      58,  -335,   256,  -335,   397,  -335,  -335,  -335,  -335,   323,
    -335,   323,  -335,   368,   399,  -335,   413,   443,   233,   475,
    -335,   510,  -335,    21,  -335,   236,  -335,  -335,   439,   438,
    -335,    56,   433,   473,  -335,  -335,   426,   427,  -335,   428,
    -335,   207,   429,  -335,  -335,    21,  -335,   533,   143,  -335,
     430,  -335,  -335,   534,   450,    21,  -335,  -335,   529,   555,
      21,   226,   452,   454,   233,   468,  -335,  -335,   292,   547,
     476,  -335,  -335,   209,    20,   505,  -335,  -335,   559,   484,
       0,  -335,  -335,  -335,   584,  -335,  -335,   483,   478,   481,
    -335,   314,    50,   482,  -335,  -335,  -335,  -335,    14,    50,
    -335,    21,  -335,  -335,  -335,  -335,   -17,   531,   586,   -69,
     233,    54,    -3,    28,    32,    54,    54,    54,    54,   549,
     314,   573,   553,  -335,  -335,  -335,  -335,  -335,  -335,  -335,
    -335,   212,   517,   504,  -335,   506,  -335,  -335,  -335,     6,
     229,  -335,   429,  -335,  -335,  -335,   603,  -335,   237,   154,
     600,   569,   509,  -335,  -335,   545,   546,  -335,    21,  -335,
    -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335,
    -335,  -335,  -335,    13,  -335,   582,   624,   294,  -335,  -335,
      10,   584,  -335,  -335,    50,  -335,  -335,   314,   639,  -335,
    -335,    21,    24,   268,  -335,    21,   646,   311,   473,   557,
    -335,  -335,   663,  -335,  -335,   156,   558,  -335,   561,  -335,
     664,  -335,  -335,   270,   314,   564,   558,  -335,   668,   575,
     272,  -335,  -335,  -335,   314,   672,  -335,   558,   568,   570,
      21,  -335,   663,  -335,  -335,  -335,  -335,  -335,  -335
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -335,  -335,  -335,  -335,   576,  -335,  -335,  -335,  -335,   376,
     330,   548,  -335,  -335,    79,  -335,   415,  -334,  -335,  -335,
    -251,  -112,  -313,  -306,  -335,  -335,  -335,  -335,  -335,  -335,
    -321,   176,   221,  -335,   205,  -335,  -335,   223,  -335,   175,
     -73,  -335,   571,   404,  -335,  -335,   -89,  -335,   440,   537,
    -335,  -335,   446,  -335,  -335,  -335,  -335,  -335,  -335,  -335,
    -335,   269,  -335,  -282,  -335,  -335,  -335,   422,  -335,  -335,
    -335,   318,  -335,  -335,  -335,  -335,  -335,  -335,  -335,   387,
    -335,  -335,  -335,  -335,   341,  -335,  -335,  -335,   694,  -335,
    -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335,  -335
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -235
static const short yytable[] =
{
     197,   131,   338,   132,   133,   192,   447,   363,   372,   180,
     181,   192,   136,   489,   105,   490,    51,   433,   491,   439,
     399,   190,   484,   330,   192,   400,   192,   279,   192,   193,
     193,   194,    52,   192,   193,   186,   449,   192,   193,   187,
     452,   203,   211,   443,   415,   206,   184,   210,    56,   254,
     214,   206,   216,   217,   218,   192,   193,   358,   194,   192,
     193,   192,   193,   185,   421,   434,   444,    53,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   438,   264,   185,   446,   137,
     451,   454,   456,   458,   460,   462,   137,   173,    54,   106,
     431,   344,   174,   358,   253,   416,    55,   436,   448,    57,
     253,   305,   420,    58,    59,    60,    61,   212,   280,    62,
      63,   492,   226,   253,   485,   253,    64,   195,   500,   103,
     428,   280,   195,   114,   196,    65,   195,   141,    58,    59,
      60,    61,   483,   204,    62,   104,  -234,   303,   304,   358,
      66,    67,    68,    69,   195,   345,   387,   105,   195,   464,
     195,   429,   142,    70,   286,    71,   506,   370,   107,   501,
     512,   175,   197,   316,   317,    91,   287,    68,    69,   503,
     308,   141,   308,   517,   513,   163,   109,   138,   129,    72,
     130,   164,   494,   165,  -234,   166,   167,   139,   168,   224,
      73,   225,   185,   108,   322,   248,   142,    95,   288,   371,
     249,   155,   156,   157,    74,    75,   110,    96,    76,   206,
     111,    77,   112,   385,    78,    97,   348,   113,   349,    58,
      59,    60,    61,   392,   115,    62,   324,   325,   395,    74,
      98,   116,    64,    76,   277,   278,    77,   205,   169,    78,
     117,    65,   100,   153,   154,   155,   156,   157,   326,    58,
      59,    60,    61,   118,   204,    62,    66,    67,    68,    69,
     197,   291,   292,   327,   119,   328,   120,   197,   248,    70,
     192,    71,   121,   249,   122,   353,   297,   292,   123,   445,
     124,   450,   453,   455,   457,   459,   461,   125,    68,    69,
     364,   365,   134,   334,   335,    72,   126,   366,   250,   129,
     401,   130,   336,   337,   324,   325,    73,   324,   325,   380,
     381,   414,   335,   329,   469,   335,    58,    59,    60,    61,
      74,    75,    62,   251,    76,   402,   326,    77,   127,   326,
      78,   473,   474,   128,   354,   158,   161,   141,   162,   476,
     335,   327,   176,   328,   327,   172,   328,   177,   252,   178,
      74,   183,   197,   189,    76,    68,    69,    77,   191,   499,
      78,   182,   142,   202,   505,   209,   129,   141,   130,   253,
     502,   335,   516,   335,   521,   522,   213,   223,   219,   403,
     404,   405,   406,   407,   408,   220,   221,   229,   247,   141,
     409,   329,   142,   261,   329,   258,   260,   259,   527,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   142,   262,   263,    74,   298,   299,
     141,    76,   266,   265,    77,   267,   270,    78,   273,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   141,   142,   274,   276,   300,   301,
     284,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   290,   294,   295,   142,
     311,   299,   141,   306,   277,   313,   315,   318,   321,   332,
     333,   342,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   142,   141,   347,
     350,   343,   301,   351,   359,   362,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   369,   374,   142,   141,   222,   368,   377,   378,   379,
     382,   386,   390,   389,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   142,
     141,   391,   400,   293,   397,   398,   410,   418,   411,   417,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   142,   141,   422,   425,   296,
     426,   440,   463,   427,   442,   432,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   142,   465,   466,   470,   302,   387,   471,   472,   477,
     478,   480,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,    91,    85,   481,
     482,   393,   486,   487,   498,    86,    92,    93,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    94,   141,   504,   509,   394,   508,    95,
     278,   514,   520,    87,   515,   518,   519,   524,   141,    96,
     525,   396,   526,   352,   188,   228,   323,    97,   488,   142,
     523,   141,    88,   507,   493,   215,   346,   528,    89,   257,
     314,   475,    98,   142,   312,    99,   339,   437,    89,   373,
     419,   102,     0,     0,   100,     0,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   141,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,     0,     0,     0,     0,     0,     0,   141,     0,
       0,     0,     0,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,     0,     0,     0,     0,     0,
       0,     0,     0,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   149,   150,   151,   152,
     153,   154,   155,   156,   157,     1,     0,     2,     0,     0,
       3,     0,     0,     4,     0,     0,     5,     6,     7,     8,
       0,     0,     9,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,    13,     0,    14,     0,
      15,     0,    16,     0,    17,     0,    18,     0,     0,     0,
       0,     0,     0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     112,    74,   284,    76,    77,     5,     9,   313,   321,    98,
      99,     5,    36,     3,     3,     5,    44,     3,     8,    36,
     354,   110,     9,   274,     5,    94,     5,     8,     5,     6,
       6,     8,     3,     5,     6,    70,     8,     5,     6,    74,
       8,    91,    30,   112,    24,   118,    72,   120,    69,   161,
     123,   124,   125,   126,   127,     5,     6,   308,     8,     5,
       6,     5,     6,   113,   370,    51,   400,    37,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   391,   175,   113,   401,   113,
     403,   404,   405,   406,   407,   408,   113,     3,    36,    20,
     382,    43,     8,   354,   104,    85,    37,   389,   111,    44,
     104,   223,   112,     3,     4,     5,     6,   105,   112,     9,
      10,   111,   111,   104,   111,   104,    16,   104,   104,    82,
     381,   112,   104,    54,   111,    25,   104,    31,     3,     4,
       5,     6,   448,     8,     9,    69,     3,   220,   221,   400,
      40,    41,    42,    43,   104,    97,    13,     3,   104,   410,
     104,   111,    56,    53,    34,    55,   487,   111,     0,   482,
      14,    77,   284,   262,   263,    12,    46,    42,    43,   485,
      91,    31,    91,   504,    28,     3,    47,     3,    53,    79,
      55,     9,   474,     3,    51,     5,     6,    13,     8,   113,
      90,   115,   113,   110,   113,     3,    56,    44,    78,   321,
       8,   105,   106,   107,   104,   105,     3,    54,   108,   292,
       3,   111,   111,   335,   114,    62,   299,     3,   301,     3,
       4,     5,     6,   345,     3,     9,     3,     4,   350,   104,
      77,     3,    16,   108,     5,     6,   111,   112,    58,   114,
       3,    25,    89,   103,   104,   105,   106,   107,    25,     3,
       4,     5,     6,   111,     8,     9,    40,    41,    42,    43,
     382,   112,   113,    40,   116,    42,   111,   389,     3,    53,
       5,    55,   111,     8,   111,    52,   112,   113,   111,   401,
     111,   403,   404,   405,   406,   407,   408,   111,    42,    43,
      64,    65,     3,   112,   113,    79,   111,    71,    33,    53,
      18,    55,   112,   113,     3,     4,    90,     3,     4,   112,
     113,   112,   113,    90,   112,   113,     3,     4,     5,     6,
     104,   105,     9,    58,   108,    43,    25,   111,   111,    25,
     114,   112,   113,   111,   111,    51,    97,    31,    97,   112,
     113,    40,    72,    42,    40,    82,    42,     3,    83,    50,
     104,     3,   474,     3,   108,    42,    43,   111,    81,   481,
     114,    88,    56,    67,    63,     3,    53,    31,    55,   104,
     112,   113,   112,   113,   112,   113,   112,    97,   112,    97,
      98,    99,   100,   101,   102,   111,   111,     3,     5,    31,
     108,    90,    56,    48,    90,    97,   104,    97,   520,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    56,    77,    77,   104,   112,   113,
      31,   108,     8,    77,   111,    91,     3,   114,     3,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    31,    56,   111,    69,   112,   113,
     113,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    42,     3,   112,    56,
     113,   113,    31,     3,     5,   111,    49,     3,    97,    87,
       3,   112,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    56,    31,   112,
      97,    75,   113,    70,    39,     5,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    93,    59,    56,    31,   112,    97,   111,   111,   111,
     111,     8,     8,   113,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,    56,
      31,   111,    94,   112,   112,   111,    19,     8,    92,    64,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    56,    31,     3,   105,   112,
     112,    60,    43,   112,     8,   113,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    56,    39,    60,    97,   112,    13,   113,   112,    19,
      51,   112,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    12,     3,    94,
      94,   112,    60,    19,     5,    10,    21,    22,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    38,    31,    19,     3,   112,   111,    44,
       6,   113,    97,    38,   113,   111,     8,     5,    31,    54,
     112,   351,   112,   307,   108,   137,   271,    62,   467,    56,
     514,    31,    57,   488,   471,   124,   292,   522,    73,   162,
     260,   432,    77,    56,   258,    80,   284,   389,    73,   322,
     369,    17,    -1,    -1,    89,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    15,    -1,    17,    -1,    -1,
      20,    -1,    -1,    23,    -1,    -1,    26,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    -1,    -1,    -1,    -1,    66,    -1,    68,    -1,
      70,    -1,    72,    -1,    74,    -1,    76,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    84,    -1,    86
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    61,    66,    68,    70,    72,    74,    76,    84,
      86,   118,   119,   120,   121,   122,   126,   162,   167,   168,
     171,   172,   173,   174,   181,   182,   191,   192,   193,   194,
     197,   202,   203,   204,   207,   209,   210,   211,   212,   214,
     215,    44,     3,    37,    36,    37,    69,    44,     3,     4,
       5,     6,     9,    10,    16,    25,    40,    41,    42,    43,
      53,    55,    79,    90,   104,   105,   108,   111,   114,   127,
     128,   130,   157,   158,   213,     3,    10,    38,    57,    73,
     205,    12,    21,    22,    38,    44,    54,    62,    77,    80,
      89,   164,   205,    82,    69,     3,   131,     0,   110,    47,
       3,     3,   111,     3,   131,     3,     3,     3,   111,   116,
     111,   111,   111,   111,   111,   111,   111,   111,   111,    53,
      55,   157,   157,   157,     3,   161,    36,   113,     3,    13,
     129,    31,    56,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    51,   149,
     150,    97,    97,     3,     9,     3,     5,     6,     8,    58,
     165,   166,    82,     3,     8,    77,    72,     3,    50,   163,
     163,   163,    88,     3,    72,   113,    70,    74,   121,     3,
     163,    81,     5,     6,     8,   104,   111,   138,   139,   180,
     183,   184,    67,    91,     8,   112,   157,   159,   160,     3,
     157,    30,   105,   112,   157,   159,   157,   157,   157,   112,
     111,   111,   112,    97,   113,   115,   111,   131,   128,     3,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,     5,     3,     8,
      33,    58,    83,   104,   138,   169,   170,   166,    97,    97,
     104,    48,    77,    77,   163,    77,     8,    91,   198,   199,
       3,   195,   196,     3,   111,   175,    69,     5,     6,     8,
     112,   138,   140,   185,   113,   186,    34,    46,    78,   208,
      42,   112,   113,   112,     3,   112,   112,   112,   112,   113,
     112,   113,   112,   157,   157,   138,     3,   123,    91,   132,
     133,   113,   169,   111,   165,    49,   163,   163,     3,   200,
     201,    97,   113,   133,     3,     4,    25,    40,    42,    90,
     137,   176,    87,     3,   112,   113,   112,   113,   180,   184,
     187,   188,   112,    75,    43,    97,   160,   112,   157,   157,
      97,    70,   126,    52,   111,   134,   135,   136,   137,    39,
     141,   142,     5,   140,    64,    65,    71,   206,    97,    93,
     111,   138,   139,   196,    59,   151,   152,   111,   111,   111,
     112,   113,   111,   177,   178,   138,     8,    13,   189,   113,
       8,   111,   138,   112,   112,   138,   127,   112,   111,   134,
      94,    18,    43,    97,    98,    99,   100,   101,   102,   108,
      19,    92,   143,   144,   112,    24,    85,    64,     8,   201,
     112,   140,     3,   153,   154,   105,   112,   112,   137,   111,
     179,   180,   113,     3,    51,   190,   180,   188,   140,    36,
      60,   124,     8,   112,   134,   138,   139,     9,   111,     8,
     138,   139,     8,   138,   139,   138,   139,   138,   139,   138,
     139,   138,   139,    43,   137,    39,    60,   145,   146,   112,
      97,   113,   112,   112,   113,   178,   112,    19,    51,   125,
     112,    94,    94,   140,     9,   111,    60,    19,   149,     3,
       5,     8,   111,   154,   180,   137,   147,   148,     5,   138,
     104,   139,   112,   140,    19,    63,   147,   151,   111,     3,
     155,   156,    14,    28,   113,   113,   112,   147,   111,     8,
      97,   112,   113,   148,     5,   112,   112,   138,   156
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
			pParser->m_pStmt->m_eStmt = STMT_DESC;
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
			tStmt.m_eStmt = STMT_CREATE_FUNC;
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
			tStmt.m_eStmt = STMT_DROP_FUNC;
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

