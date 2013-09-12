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
     TOK_SUBKEY = 268,
     TOK_DOT_NUMBER = 269,
     TOK_ADD = 270,
     TOK_AGENT = 271,
     TOK_ALTER = 272,
     TOK_AS = 273,
     TOK_ASC = 274,
     TOK_ATTACH = 275,
     TOK_AVG = 276,
     TOK_BEGIN = 277,
     TOK_BETWEEN = 278,
     TOK_BIGINT = 279,
     TOK_BOOL = 280,
     TOK_BY = 281,
     TOK_CALL = 282,
     TOK_CHARACTER = 283,
     TOK_COLLATION = 284,
     TOK_COLUMN = 285,
     TOK_COMMIT = 286,
     TOK_COMMITTED = 287,
     TOK_COUNT = 288,
     TOK_CREATE = 289,
     TOK_DELETE = 290,
     TOK_DESC = 291,
     TOK_DESCRIBE = 292,
     TOK_DISTINCT = 293,
     TOK_DIV = 294,
     TOK_DROP = 295,
     TOK_FALSE = 296,
     TOK_FLOAT = 297,
     TOK_FLUSH = 298,
     TOK_FROM = 299,
     TOK_FUNCTION = 300,
     TOK_GLOBAL = 301,
     TOK_GROUP = 302,
     TOK_GROUPBY = 303,
     TOK_GROUP_CONCAT = 304,
     TOK_ID = 305,
     TOK_IN = 306,
     TOK_INDEX = 307,
     TOK_INSERT = 308,
     TOK_INT = 309,
     TOK_INTEGER = 310,
     TOK_INTO = 311,
     TOK_IS = 312,
     TOK_ISOLATION = 313,
     TOK_LEVEL = 314,
     TOK_LIKE = 315,
     TOK_LIMIT = 316,
     TOK_MATCH = 317,
     TOK_MAX = 318,
     TOK_META = 319,
     TOK_MIN = 320,
     TOK_MOD = 321,
     TOK_NAMES = 322,
     TOK_NULL = 323,
     TOK_OPTION = 324,
     TOK_ORDER = 325,
     TOK_OPTIMIZE = 326,
     TOK_PLAN = 327,
     TOK_PROFILE = 328,
     TOK_RAND = 329,
     TOK_RAMCHUNK = 330,
     TOK_READ = 331,
     TOK_REPEATABLE = 332,
     TOK_REPLACE = 333,
     TOK_RETURNS = 334,
     TOK_ROLLBACK = 335,
     TOK_RTINDEX = 336,
     TOK_SELECT = 337,
     TOK_SERIALIZABLE = 338,
     TOK_SET = 339,
     TOK_SESSION = 340,
     TOK_SHOW = 341,
     TOK_SONAME = 342,
     TOK_START = 343,
     TOK_STATUS = 344,
     TOK_STRING = 345,
     TOK_SUM = 346,
     TOK_TABLE = 347,
     TOK_TABLES = 348,
     TOK_TO = 349,
     TOK_TRANSACTION = 350,
     TOK_TRUE = 351,
     TOK_TRUNCATE = 352,
     TOK_UNCOMMITTED = 353,
     TOK_UPDATE = 354,
     TOK_VALUES = 355,
     TOK_VARIABLES = 356,
     TOK_WARNINGS = 357,
     TOK_WEIGHT = 358,
     TOK_WHERE = 359,
     TOK_WITHIN = 360,
     TOK_OR = 361,
     TOK_AND = 362,
     TOK_NE = 363,
     TOK_GTE = 364,
     TOK_LTE = 365,
     TOK_NOT = 366,
     TOK_NEG = 367
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
#define TOK_SUBKEY 268
#define TOK_DOT_NUMBER 269
#define TOK_ADD 270
#define TOK_AGENT 271
#define TOK_ALTER 272
#define TOK_AS 273
#define TOK_ASC 274
#define TOK_ATTACH 275
#define TOK_AVG 276
#define TOK_BEGIN 277
#define TOK_BETWEEN 278
#define TOK_BIGINT 279
#define TOK_BOOL 280
#define TOK_BY 281
#define TOK_CALL 282
#define TOK_CHARACTER 283
#define TOK_COLLATION 284
#define TOK_COLUMN 285
#define TOK_COMMIT 286
#define TOK_COMMITTED 287
#define TOK_COUNT 288
#define TOK_CREATE 289
#define TOK_DELETE 290
#define TOK_DESC 291
#define TOK_DESCRIBE 292
#define TOK_DISTINCT 293
#define TOK_DIV 294
#define TOK_DROP 295
#define TOK_FALSE 296
#define TOK_FLOAT 297
#define TOK_FLUSH 298
#define TOK_FROM 299
#define TOK_FUNCTION 300
#define TOK_GLOBAL 301
#define TOK_GROUP 302
#define TOK_GROUPBY 303
#define TOK_GROUP_CONCAT 304
#define TOK_ID 305
#define TOK_IN 306
#define TOK_INDEX 307
#define TOK_INSERT 308
#define TOK_INT 309
#define TOK_INTEGER 310
#define TOK_INTO 311
#define TOK_IS 312
#define TOK_ISOLATION 313
#define TOK_LEVEL 314
#define TOK_LIKE 315
#define TOK_LIMIT 316
#define TOK_MATCH 317
#define TOK_MAX 318
#define TOK_META 319
#define TOK_MIN 320
#define TOK_MOD 321
#define TOK_NAMES 322
#define TOK_NULL 323
#define TOK_OPTION 324
#define TOK_ORDER 325
#define TOK_OPTIMIZE 326
#define TOK_PLAN 327
#define TOK_PROFILE 328
#define TOK_RAND 329
#define TOK_RAMCHUNK 330
#define TOK_READ 331
#define TOK_REPEATABLE 332
#define TOK_REPLACE 333
#define TOK_RETURNS 334
#define TOK_ROLLBACK 335
#define TOK_RTINDEX 336
#define TOK_SELECT 337
#define TOK_SERIALIZABLE 338
#define TOK_SET 339
#define TOK_SESSION 340
#define TOK_SHOW 341
#define TOK_SONAME 342
#define TOK_START 343
#define TOK_STATUS 344
#define TOK_STRING 345
#define TOK_SUM 346
#define TOK_TABLE 347
#define TOK_TABLES 348
#define TOK_TO 349
#define TOK_TRANSACTION 350
#define TOK_TRUE 351
#define TOK_TRUNCATE 352
#define TOK_UNCOMMITTED 353
#define TOK_UPDATE 354
#define TOK_VALUES 355
#define TOK_VARIABLES 356
#define TOK_WARNINGS 357
#define TOK_WEIGHT 358
#define TOK_WHERE 359
#define TOK_WITHIN 360
#define TOK_OR 361
#define TOK_AND 362
#define TOK_NE 363
#define TOK_GTE 364
#define TOK_LTE 365
#define TOK_NOT 366
#define TOK_NEG 367




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif



// some helpers
#include <float.h> // for FLT_MAX

static void AddInsval ( SqlParser_c * pParser, CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iType;
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	if ( tIns.m_iType==TOK_QUOTED_STRING )
		pParser->ToStringUnescape ( tIns.m_sVal, tNode );
	tIns.m_pVals = tNode.m_pValues;
}

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	_res.m_iEnd = _right.m_iEnd; \
	_res.m_iType = 0;



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
#define YYFINAL  120
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1399

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  132
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  113
/* YYNRULES -- Number of rules. */
#define YYNRULES  324
/* YYNRULES -- Number of states. */
#define YYNSTATES  608

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   367

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   120,   109,     2,
     124,   125,   118,   116,   126,   117,   129,   119,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   123,
     112,   110,   113,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   130,     2,   131,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   127,   108,   128,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   111,   114,   115,   121,   122
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    56,    58,
      62,    64,    66,    68,    77,    79,    89,    90,    93,    95,
      99,   101,   103,   105,   106,   110,   111,   114,   119,   130,
     132,   136,   138,   141,   142,   144,   147,   149,   154,   159,
     164,   169,   174,   179,   183,   189,   191,   195,   196,   198,
     201,   203,   207,   211,   216,   220,   224,   230,   237,   241,
     246,   252,   256,   260,   264,   268,   272,   274,   280,   286,
     292,   296,   300,   304,   308,   312,   316,   320,   325,   329,
     331,   333,   338,   342,   346,   348,   350,   352,   355,   357,
     360,   362,   364,   368,   369,   374,   375,   377,   379,   383,
     384,   386,   392,   393,   395,   399,   405,   407,   411,   413,
     416,   419,   420,   422,   425,   430,   431,   433,   436,   438,
     442,   446,   450,   456,   463,   467,   469,   473,   477,   479,
     481,   483,   485,   487,   489,   491,   494,   497,   501,   505,
     509,   513,   517,   521,   525,   529,   533,   537,   541,   545,
     549,   553,   557,   561,   565,   569,   573,   575,   577,   582,
     587,   592,   597,   602,   606,   613,   620,   624,   626,   630,
     632,   634,   638,   644,   646,   648,   650,   652,   655,   656,
     659,   661,   664,   667,   671,   673,   675,   680,   685,   689,
     691,   693,   695,   697,   699,   701,   705,   710,   715,   720,
     724,   729,   737,   743,   745,   747,   749,   751,   753,   755,
     757,   759,   761,   764,   771,   773,   775,   776,   780,   782,
     786,   788,   792,   796,   798,   802,   804,   806,   808,   812,
     815,   823,   833,   840,   842,   846,   848,   852,   854,   858,
     859,   862,   864,   868,   872,   873,   875,   877,   879,   883,
     885,   887,   891,   898,   900,   904,   908,   912,   918,   923,
     927,   931,   933,   935,   937,   939,   947,   952,   953,   955,
     958,   960,   964,   968,   971,   975,   982,   983,   985,   987,
     990,   993,   996,   998,  1006,  1008,  1010,  1012,  1014,  1018,
    1025,  1029,  1033,  1037,  1039,  1043,  1046,  1050,  1054,  1057,
    1059,  1062,  1064,  1066,  1070
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     133,     0,    -1,   134,    -1,   135,    -1,   135,   123,    -1,
     195,    -1,   203,    -1,   189,    -1,   190,    -1,   193,    -1,
     204,    -1,   213,    -1,   215,    -1,   216,    -1,   221,    -1,
     226,    -1,   227,    -1,   231,    -1,   233,    -1,   234,    -1,
     235,    -1,   236,    -1,   228,    -1,   237,    -1,   239,    -1,
     240,    -1,   241,    -1,   220,    -1,   136,    -1,   135,   123,
     136,    -1,   137,    -1,   184,    -1,   138,    -1,    82,     3,
     124,   124,   138,   125,   139,   125,    -1,   145,    -1,    82,
     146,    44,   124,   142,   145,   125,   143,   144,    -1,    -1,
     126,   140,    -1,   141,    -1,   140,   126,   141,    -1,     3,
      -1,     5,    -1,    50,    -1,    -1,    70,    26,   167,    -1,
      -1,    61,     5,    -1,    61,     5,   126,     5,    -1,    82,
     146,    44,   150,   151,   160,   163,   165,   169,   171,    -1,
     147,    -1,   146,   126,   147,    -1,   118,    -1,   149,   148,
      -1,    -1,     3,    -1,    18,     3,    -1,   177,    -1,    21,
     124,   177,   125,    -1,    63,   124,   177,   125,    -1,    65,
     124,   177,   125,    -1,    91,   124,   177,   125,    -1,    49,
     124,   177,   125,    -1,    33,   124,   118,   125,    -1,    48,
     124,   125,    -1,    33,   124,    38,     3,   125,    -1,     3,
      -1,   150,   126,     3,    -1,    -1,   152,    -1,   104,   153,
      -1,   154,    -1,   153,   107,   153,    -1,   124,   153,   125,
      -1,    62,   124,     8,   125,    -1,   156,   110,   157,    -1,
     156,   111,   157,    -1,   156,    51,   124,   159,   125,    -1,
     156,   121,    51,   124,   159,   125,    -1,   156,    51,     9,
      -1,   156,   121,    51,     9,    -1,   156,    23,   157,   107,
     157,    -1,   156,   113,   157,    -1,   156,   112,   157,    -1,
     156,   114,   157,    -1,   156,   115,   157,    -1,   156,   110,
     158,    -1,   155,    -1,   156,    23,   158,   107,   158,    -1,
     156,    23,   157,   107,   158,    -1,   156,    23,   158,   107,
     157,    -1,   156,   113,   158,    -1,   156,   112,   158,    -1,
     156,   114,   158,    -1,   156,   115,   158,    -1,   156,   110,
       8,    -1,   156,   111,     8,    -1,   156,    57,    68,    -1,
     156,    57,   121,    68,    -1,   156,   111,   158,    -1,     3,
      -1,     4,    -1,    33,   124,   118,   125,    -1,    48,   124,
     125,    -1,   103,   124,   125,    -1,    50,    -1,   242,    -1,
       5,    -1,   117,     5,    -1,     6,    -1,   117,     6,    -1,
      14,    -1,   157,    -1,   159,   126,   157,    -1,    -1,    47,
     161,    26,   162,    -1,    -1,     5,    -1,   156,    -1,   162,
     126,   156,    -1,    -1,   164,    -1,   105,    47,    70,    26,
     167,    -1,    -1,   166,    -1,    70,    26,   167,    -1,    70,
      26,    74,   124,   125,    -1,   168,    -1,   167,   126,   168,
      -1,   156,    -1,   156,    19,    -1,   156,    36,    -1,    -1,
     170,    -1,    61,     5,    -1,    61,     5,   126,     5,    -1,
      -1,   172,    -1,    69,   173,    -1,   174,    -1,   173,   126,
     174,    -1,     3,   110,     3,    -1,     3,   110,     5,    -1,
       3,   110,   124,   175,   125,    -1,     3,   110,     3,   124,
       8,   125,    -1,     3,   110,     8,    -1,   176,    -1,   175,
     126,   176,    -1,     3,   110,   157,    -1,     3,    -1,     4,
      -1,    50,    -1,     5,    -1,     6,    -1,    14,    -1,     9,
      -1,   117,   177,    -1,   121,   177,    -1,   177,   116,   177,
      -1,   177,   117,   177,    -1,   177,   118,   177,    -1,   177,
     119,   177,    -1,   177,   112,   177,    -1,   177,   113,   177,
      -1,   177,   109,   177,    -1,   177,   108,   177,    -1,   177,
     120,   177,    -1,   177,    39,   177,    -1,   177,    66,   177,
      -1,   177,   115,   177,    -1,   177,   114,   177,    -1,   177,
     110,   177,    -1,   177,   111,   177,    -1,   177,   107,   177,
      -1,   177,   106,   177,    -1,   124,   177,   125,    -1,   127,
     181,   128,    -1,   178,    -1,   242,    -1,     3,   124,   179,
     125,    -1,    51,   124,   179,   125,    -1,    55,   124,   179,
     125,    -1,    24,   124,   179,   125,    -1,    42,   124,   179,
     125,    -1,     3,   124,   125,    -1,    65,   124,   177,   126,
     177,   125,    -1,    63,   124,   177,   126,   177,   125,    -1,
     103,   124,   125,    -1,   180,    -1,   179,   126,   180,    -1,
     177,    -1,     8,    -1,   182,   110,   183,    -1,   181,   126,
     182,   110,   183,    -1,     3,    -1,    51,    -1,   157,    -1,
       3,    -1,    86,   186,    -1,    -1,    60,     8,    -1,   102,
      -1,    89,   185,    -1,    64,   185,    -1,    16,    89,   185,
      -1,    73,    -1,    72,    -1,    16,     8,    89,   185,    -1,
      16,     3,    89,   185,    -1,    52,     3,    89,    -1,     3,
      -1,    68,    -1,     8,    -1,     5,    -1,     6,    -1,   187,
      -1,   188,   117,   187,    -1,    84,     3,   110,   192,    -1,
      84,     3,   110,   191,    -1,    84,     3,   110,    68,    -1,
      84,    67,   188,    -1,    84,    10,   110,   188,    -1,    84,
      46,     9,   110,   124,   159,   125,    -1,    84,    46,     3,
     110,   191,    -1,     3,    -1,     8,    -1,    96,    -1,    41,
      -1,   157,    -1,    31,    -1,    80,    -1,   194,    -1,    22,
      -1,    88,    95,    -1,   196,    56,     3,   197,   100,   199,
      -1,    53,    -1,    78,    -1,    -1,   124,   198,   125,    -1,
     156,    -1,   198,   126,   156,    -1,   200,    -1,   199,   126,
     200,    -1,   124,   201,   125,    -1,   202,    -1,   201,   126,
     202,    -1,   157,    -1,   158,    -1,     8,    -1,   124,   159,
     125,    -1,   124,   125,    -1,    35,    44,   150,   104,    50,
     110,   157,    -1,    35,    44,   150,   104,    50,    51,   124,
     159,   125,    -1,    27,     3,   124,   205,   208,   125,    -1,
     206,    -1,   205,   126,   206,    -1,   202,    -1,   124,   207,
     125,    -1,     8,    -1,   207,   126,     8,    -1,    -1,   126,
     209,    -1,   210,    -1,   209,   126,   210,    -1,   202,   211,
     212,    -1,    -1,    18,    -1,     3,    -1,    61,    -1,   214,
       3,   185,    -1,    37,    -1,    36,    -1,    86,    93,   185,
      -1,    99,   150,    84,   217,   152,   171,    -1,   218,    -1,
     217,   126,   218,    -1,     3,   110,   157,    -1,     3,   110,
     158,    -1,     3,   110,   124,   159,   125,    -1,     3,   110,
     124,   125,    -1,   242,   110,   157,    -1,   242,   110,   158,
      -1,    55,    -1,    24,    -1,    42,    -1,    25,    -1,    17,
      92,     3,    15,    30,     3,   219,    -1,    86,   229,   101,
     222,    -1,    -1,   223,    -1,   104,   224,    -1,   225,    -1,
     224,   106,   225,    -1,     3,   110,     8,    -1,    86,    29,
      -1,    86,    28,    84,    -1,    84,   229,    95,    58,    59,
     230,    -1,    -1,    46,    -1,    85,    -1,    76,    98,    -1,
      76,    32,    -1,    77,    76,    -1,    83,    -1,    34,    45,
       3,    79,   232,    87,     8,    -1,    54,    -1,    24,    -1,
      42,    -1,    90,    -1,    40,    45,     3,    -1,    20,    52,
       3,    94,    81,     3,    -1,    43,    81,     3,    -1,    43,
      75,     3,    -1,    82,   238,   169,    -1,    10,    -1,    10,
     129,     3,    -1,    82,   177,    -1,    97,    81,     3,    -1,
      71,    52,     3,    -1,     3,   243,    -1,   244,    -1,   243,
     244,    -1,    13,    -1,    14,    -1,   130,   177,   131,    -1,
     130,     8,   131,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   158,   158,   159,   160,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   192,   193,
     197,   198,   202,   203,   211,   212,   219,   221,   225,   229,
     236,   237,   238,   242,   255,   262,   264,   269,   278,   293,
     294,   298,   299,   302,   304,   305,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   321,   322,   325,   327,   331,
     335,   336,   337,   341,   346,   353,   361,   369,   378,   383,
     388,   393,   398,   403,   408,   413,   418,   423,   428,   433,
     438,   443,   448,   453,   458,   463,   468,   473,   481,   485,
     486,   491,   497,   503,   509,   515,   519,   520,   531,   532,
     533,   537,   543,   549,   551,   554,   556,   563,   567,   573,
     575,   579,   590,   592,   596,   600,   607,   608,   612,   613,
     614,   617,   619,   623,   628,   635,   637,   641,   645,   646,
     650,   655,   660,   666,   671,   679,   684,   691,   701,   702,
     703,   704,   705,   706,   707,   708,   709,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   728,   729,   730,   731,   735,   736,
     737,   738,   739,   740,   741,   742,   743,   747,   748,   752,
     753,   757,   758,   762,   763,   767,   768,   774,   777,   779,
     783,   784,   785,   786,   787,   788,   789,   794,   799,   809,
     810,   811,   812,   813,   817,   818,   822,   827,   832,   837,
     838,   842,   847,   855,   856,   860,   861,   862,   876,   877,
     878,   882,   883,   889,   897,   898,   901,   903,   907,   908,
     912,   913,   917,   921,   922,   926,   927,   928,   929,   930,
     936,   944,   958,   966,   970,   977,   978,   985,   995,  1001,
    1003,  1007,  1012,  1016,  1023,  1025,  1029,  1030,  1036,  1044,
    1045,  1051,  1057,  1065,  1066,  1070,  1084,  1090,  1094,  1099,
    1113,  1124,  1125,  1126,  1127,  1131,  1144,  1150,  1151,  1155,
    1159,  1160,  1164,  1168,  1175,  1182,  1188,  1189,  1190,  1194,
    1195,  1196,  1197,  1203,  1214,  1215,  1216,  1217,  1221,  1232,
    1244,  1253,  1264,  1272,  1273,  1277,  1287,  1298,  1309,  1312,
    1313,  1317,  1318,  1319,  1320
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
  "TOK_BAD_NUMERIC", "TOK_SUBKEY", "TOK_DOT_NUMBER", "TOK_ADD", 
  "TOK_AGENT", "TOK_ALTER", "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", 
  "TOK_BEGIN", "TOK_BETWEEN", "TOK_BIGINT", "TOK_BOOL", "TOK_BY", 
  "TOK_CALL", "TOK_CHARACTER", "TOK_COLLATION", "TOK_COLUMN", 
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", 
  "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTEGER", 
  "TOK_INTO", "TOK_IS", "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", 
  "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", 
  "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", 
  "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_STRING", "TOK_SUM", "TOK_TABLE", "TOK_TABLES", "TOK_TO", 
  "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", 
  "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", 
  "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", 
  "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", 
  "','", "'{'", "'}'", "'.'", "'['", "']'", "$accept", "request", 
  "statement", "multi_stmt_list", "multi_stmt", "select", "select1", 
  "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg", 
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "expr_float_unhandled", "expr_ident", "const_int", 
  "const_float", "const_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_group_order_clause", "group_order_clause", 
  "opt_order_clause", "order_clause", "order_items_list", "order_item", 
  "opt_limit_clause", "limit_clause", "opt_option_clause", 
  "option_clause", "option_list", "option_item", "named_const_list", 
  "named_const", "expr", "function", "arglist", "arg", "consthash", 
  "hash_key", "hash_val", "show_stmt", "like_filter", "show_what", 
  "simple_set_value", "set_value", "set_stmt", "set_global_stmt", 
  "set_string_value", "boolean_value", "transact_op", "start_transaction", 
  "insert_into", "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", "alter_col_type", "alter", 
  "show_variables", "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "flush_ramchunk", "select_sysvar", 
  "sysvar_name", "select_dual", "truncate", "optimize_index", 
  "json_field", "subscript", "subkey", 0
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
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   124,    38,
      61,   363,    60,    62,   364,   365,    43,    45,    42,    47,
      37,   366,   367,    59,    40,    41,    44,   123,   125,    46,
      91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   132,   133,   133,   133,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   135,   135,
     136,   136,   137,   137,   138,   138,   139,   139,   140,   140,
     141,   141,   141,   142,   143,   144,   144,   144,   145,   146,
     146,   147,   147,   148,   148,   148,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   150,   150,   151,   151,   152,
     153,   153,   153,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   155,   156,
     156,   156,   156,   156,   156,   156,   157,   157,   158,   158,
     158,   159,   159,   160,   160,   161,   161,   162,   162,   163,
     163,   164,   165,   165,   166,   166,   167,   167,   168,   168,
     168,   169,   169,   170,   170,   171,   171,   172,   173,   173,
     174,   174,   174,   174,   174,   175,   175,   176,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   179,   179,   180,
     180,   181,   181,   182,   182,   183,   183,   184,   185,   185,
     186,   186,   186,   186,   186,   186,   186,   186,   186,   187,
     187,   187,   187,   187,   188,   188,   189,   189,   189,   189,
     189,   190,   190,   191,   191,   192,   192,   192,   193,   193,
     193,   194,   194,   195,   196,   196,   197,   197,   198,   198,
     199,   199,   200,   201,   201,   202,   202,   202,   202,   202,
     203,   203,   204,   205,   205,   206,   206,   207,   207,   208,
     208,   209,   209,   210,   211,   211,   212,   212,   213,   214,
     214,   215,   216,   217,   217,   218,   218,   218,   218,   218,
     218,   219,   219,   219,   219,   220,   221,   222,   222,   223,
     224,   224,   225,   226,   227,   228,   229,   229,   229,   230,
     230,   230,   230,   231,   232,   232,   232,   232,   233,   234,
     235,   236,   237,   238,   238,   239,   240,   241,   242,   243,
     243,   244,   244,   244,   244
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     8,     1,     9,     0,     2,     1,     3,
       1,     1,     1,     0,     3,     0,     2,     4,    10,     1,
       3,     1,     2,     0,     1,     2,     1,     4,     4,     4,
       4,     4,     4,     3,     5,     1,     3,     0,     1,     2,
       1,     3,     3,     4,     3,     3,     5,     6,     3,     4,
       5,     3,     3,     3,     3,     3,     1,     5,     5,     5,
       3,     3,     3,     3,     3,     3,     3,     4,     3,     1,
       1,     4,     3,     3,     1,     1,     1,     2,     1,     2,
       1,     1,     3,     0,     4,     0,     1,     1,     3,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     6,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     4,     4,
       4,     4,     4,     3,     6,     6,     3,     1,     3,     1,
       1,     3,     5,     1,     1,     1,     1,     2,     0,     2,
       1,     2,     2,     3,     1,     1,     4,     4,     3,     1,
       1,     1,     1,     1,     1,     3,     4,     4,     4,     3,
       4,     7,     5,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     6,     1,     1,     0,     3,     1,     3,
       1,     3,     3,     1,     3,     1,     1,     1,     3,     2,
       7,     9,     6,     1,     3,     1,     3,     1,     3,     0,
       2,     1,     3,     3,     0,     1,     1,     1,     3,     1,
       1,     3,     6,     1,     3,     3,     3,     5,     4,     3,
       3,     1,     1,     1,     1,     7,     4,     0,     1,     2,
       1,     3,     3,     2,     3,     6,     0,     1,     1,     2,
       2,     2,     1,     7,     1,     1,     1,     1,     3,     6,
       3,     3,     3,     1,     3,     2,     3,     3,     2,     1,
       2,     1,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   231,     0,   228,     0,     0,   270,   269,
       0,     0,   234,     0,   235,   229,     0,   296,   296,     0,
       0,     0,     0,     2,     3,    28,    30,    32,    34,    31,
       7,     8,     9,   230,     5,     0,     6,    10,    11,     0,
      12,    13,    27,    14,    15,    16,    22,    17,    18,    19,
      20,    21,    23,    24,    25,    26,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   148,   149,   151,   152,   154,
     313,   153,     0,     0,     0,     0,     0,     0,   150,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,    49,    53,    56,   176,   131,   177,     0,     0,   297,
       0,   298,     0,     0,     0,   293,   297,     0,   198,   205,
     204,   198,   198,   200,   197,     0,   232,     0,    65,     0,
       1,     4,     0,   198,     0,     0,     0,     0,     0,   308,
     311,   310,   317,   321,   322,     0,     0,   318,   319,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   148,     0,     0,   155,   156,     0,   193,   194,
       0,     0,     0,     0,    54,     0,    52,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   312,   132,     0,     0,     0,
       0,   209,   212,   213,   211,   210,   214,   219,     0,     0,
       0,   198,   294,     0,     0,   202,   201,   271,   287,   316,
       0,     0,     0,     0,    29,   236,   268,     0,     0,   106,
     108,   247,   110,     0,     0,   245,   246,   255,   259,   253,
       0,     0,   190,     0,   183,   189,     0,   187,     0,     0,
     320,   314,     0,     0,     0,     0,     0,    63,     0,     0,
       0,     0,     0,     0,   186,     0,     0,     0,   174,     0,
     175,     0,    43,    67,    50,    56,    55,   166,   167,   173,
     172,   164,   163,   170,   171,   161,   162,   169,   168,   157,
     158,   159,   160,   165,   133,   223,   224,   226,   218,   225,
       0,   227,   217,   216,   220,     0,     0,     0,     0,   198,
     198,   203,   208,   199,     0,   286,   288,     0,     0,   273,
       0,    66,     0,     0,     0,     0,   107,   109,   257,   249,
     111,     0,     0,     0,     0,   305,   306,   304,   307,     0,
       0,     0,     0,   178,     0,   324,   323,    57,   181,     0,
      62,   182,    61,   179,   180,    58,     0,    59,     0,    60,
       0,     0,     0,   196,   195,   191,     0,     0,   113,    68,
       0,   222,     0,   215,     0,   207,   206,     0,   289,   290,
       0,     0,   135,     0,    99,   100,     0,     0,   104,     0,
     238,     0,   105,     0,     0,   309,   248,     0,   256,     0,
     255,   254,   260,   261,   252,     0,     0,     0,    36,   188,
      64,     0,     0,     0,     0,     0,     0,     0,    69,    70,
      86,     0,   115,   119,   134,     0,     0,     0,   302,   295,
       0,     0,     0,   275,   276,   274,     0,   272,   136,   279,
     280,     0,     0,     0,   237,     0,     0,   233,   240,   282,
     284,   283,   281,   285,   112,   258,   265,     0,     0,   303,
       0,   250,     0,     0,   185,   184,   192,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   116,     0,     0,   122,   120,   221,   300,   299,
     301,   292,   291,   278,     0,     0,   137,   138,     0,   102,
     103,   239,     0,     0,   243,     0,   266,   267,   263,   264,
     262,     0,    40,    41,    42,    37,    38,    33,     0,     0,
      45,     0,    72,    71,     0,     0,    78,     0,    96,     0,
      94,    74,    85,    95,    75,    98,    82,    91,    81,    90,
      83,    92,    84,    93,     0,     0,     0,     0,   131,   123,
     277,     0,     0,   101,   242,     0,   241,   251,     0,     0,
       0,    35,    73,     0,     0,     0,    97,    79,     0,   117,
     114,     0,     0,   135,   140,   141,   144,     0,   139,   244,
      39,   128,    44,   126,    46,    80,    88,    89,    87,    76,
       0,     0,     0,     0,   124,    48,     0,     0,     0,   145,
     129,   130,     0,     0,    77,   118,   121,     0,     0,     0,
     142,     0,   127,    47,   125,   143,   147,   146
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   453,   505,   506,
     356,   510,   551,    28,    90,    91,   166,    92,   263,   358,
     359,   408,   409,   410,   571,   320,   226,   321,   413,   473,
     560,   475,   476,   538,   539,   572,   573,   185,   186,   427,
     428,   486,   487,   588,   589,   235,    94,   236,   237,   160,
     161,   355,    29,   205,   114,   196,   197,    30,    31,   292,
     293,    32,    33,    34,    35,   313,   381,   437,   438,   493,
     227,    36,    37,   228,   229,   322,   324,   392,   393,   447,
     498,    38,    39,    40,    41,   308,   309,   443,    42,    43,
     305,   306,   368,   369,    44,    45,    46,   102,   419,    47,
     329,    48,    49,    50,    51,    52,    95,    53,    54,    55,
      96,   137,   138
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -370
static const short yypact[] =
{
    1184,   106,    65,  -370,   246,  -370,   217,   222,  -370,  -370,
     231,   113,  -370,   191,  -370,  -370,   254,   245,  1180,   193,
     209,   289,   301,  -370,   185,  -370,  -370,  -370,  -370,  -370,
    -370,  -370,  -370,  -370,  -370,   255,  -370,  -370,  -370,   310,
    -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,
    -370,  -370,  -370,  -370,  -370,  -370,   315,   320,   214,   325,
     289,   346,   353,   355,   365,    16,  -370,  -370,  -370,  -370,
     244,  -370,   250,   252,   253,   258,   259,   264,  -370,   265,
     267,   268,   270,   272,   277,   704,  -370,   704,   704,    42,
     -40,  -370,   130,   727,  -370,   308,  -370,   269,   288,   227,
      46,  -370,   309,    29,   319,  -370,  -370,   403,   347,  -370,
    -370,   347,   347,  -370,  -370,   307,  -370,   406,  -370,   -36,
    -370,   -26,   408,   347,   397,   321,    30,   334,   -51,  -370,
    -370,  -370,  -370,  -370,  -370,   511,   599,    -4,  -370,   413,
     704,   616,   -16,   616,   292,   704,   616,   616,   704,   704,
     704,   293,    55,   296,   297,  -370,  -370,   935,  -370,  -370,
      90,   312,     4,   381,  -370,   422,  -370,   704,   704,   704,
     704,   704,   704,   704,   704,   704,   704,   704,   704,   704,
     704,   704,   704,   704,   421,  -370,  -370,   197,    46,   323,
     324,  -370,  -370,  -370,  -370,  -370,  -370,   318,   379,   349,
     350,   347,  -370,   351,   433,  -370,  -370,  -370,   338,  -370,
     440,   442,   446,   167,  -370,   330,  -370,   417,   367,  -370,
    -370,  -370,  -370,    53,    19,  -370,  -370,  -370,   331,  -370,
      31,   409,  -370,   679,  -370,  1183,   -10,  -370,   327,   742,
    -370,  -370,   963,   107,   453,   336,   109,  -370,   991,   116,
     148,   824,   845,  1019,  -370,   536,   704,   704,  -370,    42,
    -370,    38,  -370,   -13,  -370,  1183,  -370,  -370,  -370,  1197,
    1211,  1279,   699,   414,   414,   358,   358,   358,   358,   228,
     228,  -370,  -370,  -370,   337,  -370,  -370,  -370,  -370,  -370,
     457,  -370,  -370,  -370,   318,   212,   340,    46,   407,   347,
     347,  -370,  -370,  -370,   462,  -370,  -370,    12,    83,  -370,
     359,  -370,   204,   368,   468,   478,  -370,  -370,  -370,  -370,
    -370,   156,   158,    30,   357,  -370,  -370,  -370,  -370,   396,
     -28,   381,   360,  -370,   616,  -370,  -370,  -370,  -370,   361,
    -370,  -370,  -370,  -370,  -370,  -370,   704,  -370,   704,  -370,
     879,   907,   377,  -370,  -370,  -370,   410,    17,   443,  -370,
     484,  -370,    59,  -370,   134,  -370,  -370,   383,   385,  -370,
      57,   440,   431,   198,    -4,  -370,   380,   382,  -370,   386,
    -370,   160,  -370,   388,   274,  -370,  -370,    59,  -370,   495,
      39,  -370,   387,  -370,  -370,   499,   394,    59,   395,  -370,
    -370,  1047,  1075,    38,   381,   398,   400,    17,   415,  -370,
    -370,   249,   531,   438,  -370,   199,     7,   470,  -370,  -370,
     530,   462,    26,  -370,  -370,  -370,   544,  -370,  -370,  -370,
    -370,   430,   426,   427,  -370,   204,    75,   428,  -370,  -370,
    -370,  -370,  -370,  -370,  -370,  -370,  -370,    37,    75,  -370,
      59,  -370,   179,   432,  -370,  -370,  -370,   -27,   485,   548,
     -41,    17,   198,    -1,   -49,    89,   172,   198,   198,   198,
     198,   507,  -370,   533,   518,   498,  -370,  -370,  -370,  -370,
    -370,  -370,  -370,  -370,   201,   459,   445,  -370,   447,  -370,
    -370,  -370,    28,   206,  -370,   388,  -370,  -370,  -370,   557,
    -370,   208,  -370,  -370,  -370,   451,  -370,  -370,   289,   553,
     519,   456,  -370,  -370,   475,   476,  -370,    59,  -370,   516,
    -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,
    -370,  -370,  -370,  -370,    25,   204,   515,   562,   308,  -370,
    -370,     8,   544,  -370,  -370,    75,  -370,  -370,   179,   204,
     584,  -370,  -370,   198,   198,   226,  -370,  -370,    59,  -370,
     464,   566,   247,   431,   469,  -370,  -370,   591,  -370,  -370,
    -370,    51,   471,  -370,   472,  -370,  -370,  -370,  -370,  -370,
     229,   204,   204,   482,   471,  -370,   587,   486,   240,  -370,
    -370,  -370,   204,   595,  -370,  -370,   471,   487,   490,    59,
    -370,   591,  -370,  -370,  -370,  -370,  -370,  -370
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -370,  -370,  -370,  -370,   488,  -370,   378,  -370,  -370,    62,
    -370,  -370,  -370,   260,   213,   455,  -370,  -370,    67,  -370,
     326,  -360,  -370,  -370,  -311,  -126,  -358,  -344,  -370,  -370,
    -370,  -370,  -370,  -370,  -370,  -369,    34,    91,  -370,    64,
    -370,  -370,    95,  -370,    32,   -11,  -370,    80,   311,  -370,
     372,   239,  -370,  -109,  -370,   354,   458,  -370,  -370,   348,
    -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,   149,  -370,
    -317,  -370,  -370,  -370,   329,  -370,  -370,  -370,   200,  -370,
    -370,  -370,  -370,  -370,  -370,  -370,   276,  -370,  -370,  -370,
    -370,  -370,  -370,   234,  -370,  -370,  -370,   638,  -370,  -370,
    -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,  -370,
    -182,  -370,   522
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -316
static const short yytable[] =
{
     225,   380,   206,   207,   162,    93,   390,   118,   516,   133,
     134,   564,   424,   565,   216,   430,   566,   508,   415,   518,
     374,   375,   244,   396,   219,   133,   134,   318,   310,   133,
     134,   219,   199,   219,   557,   219,   220,   200,   221,   478,
     496,   353,  -264,   219,   222,   158,   411,   460,   210,   191,
     376,   192,   193,   231,   194,   325,   212,   446,   316,   317,
     213,   291,   219,   220,   219,   377,   461,   378,   133,   134,
     590,   222,   519,   326,   155,   211,   156,   157,   484,   406,
     219,   220,   397,   221,   512,   327,   163,   591,   119,   222,
     211,   357,   301,   159,   219,   220,   411,   520,   497,   163,
    -264,   513,   245,   222,   515,   479,   501,   522,   525,   527,
     529,   531,   533,   211,   195,   333,   334,    57,   201,   494,
     379,   328,   370,   517,   491,   239,   136,   128,   262,   242,
     382,   499,   567,   164,   248,   354,   290,   251,   252,   253,
     135,   407,   136,   290,   319,   290,   136,   223,   165,   558,
     411,   483,   265,   319,   224,   290,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   555,   223,   382,   290,   219,   220,   255,
     523,   422,   502,   103,   503,   136,   222,   357,    62,   310,
     365,   366,   223,   584,    63,   576,   578,   225,    56,   492,
     285,   265,   219,   219,   220,   286,   223,   374,   375,   371,
     416,   417,   222,   596,   580,   285,   259,   418,   260,   107,
     286,   243,   157,   246,   559,   382,   249,   250,   569,   504,
     189,   108,   338,   334,   341,   334,   190,   376,   287,   109,
     110,   343,   334,    64,   423,   350,   351,   429,    97,    58,
     374,   375,   377,   382,   378,    98,   111,    65,    66,    67,
      68,   444,    59,    69,    70,   288,    60,   167,    71,   113,
     595,   451,   462,   344,   334,    72,    61,   354,    73,   382,
     376,   386,   387,   388,   389,   434,   435,    74,   116,   223,
     117,    99,   118,   289,   168,   377,    75,   378,   439,   440,
     463,   120,    76,    77,    78,    79,   464,   379,   121,    80,
     225,   122,   100,   123,   290,   223,   441,    81,   124,    82,
     265,   583,   225,   125,   477,   387,   540,   387,   127,   442,
     101,   544,   545,   547,   387,   401,   514,   402,   126,   521,
     524,   526,   528,   530,   532,    83,   181,   182,   183,   129,
     379,   579,   387,   382,   594,   387,   130,    84,   131,   465,
     466,   467,   468,   469,   470,   600,   601,   382,   132,   184,
     471,    85,    86,   139,   140,    87,   141,   142,    88,   187,
     382,    89,   143,   144,   152,    66,    67,    68,   145,   146,
      69,   147,   148,   265,   149,    71,   150,   167,   188,   382,
     382,   151,    72,   202,   198,    73,   203,   204,   208,   209,
     382,   215,   217,   230,    74,   218,   241,   247,   254,   225,
     256,   257,   261,    75,   168,   266,   284,   575,   577,    76,
      77,    78,    79,   295,   296,   297,    80,   298,   299,   300,
     302,   303,   304,   307,    81,   311,    82,   314,   315,    65,
      66,    67,    68,   167,   312,    69,   339,   323,   335,   330,
      71,   340,   316,   360,   362,   367,   364,    72,   383,   373,
      73,   384,    83,   606,   179,   180,   181,   182,   183,    74,
     168,   385,   394,   395,    84,   398,   400,   403,    75,   414,
     412,   421,   404,   420,    76,    77,    78,    79,    85,    86,
     426,    80,    87,   445,   431,    88,   432,   449,    89,    81,
     433,    82,   436,   448,   152,    66,    67,    68,   450,   232,
      69,   452,   461,   458,   459,    71,   175,   176,   177,   178,
     179,   180,   181,   182,   183,    73,   472,    83,   481,   152,
      66,    67,    68,   474,   232,    69,   480,   485,   488,    84,
      71,   489,   490,    75,   495,   509,   511,   507,   534,   535,
      73,    78,    79,    85,    86,   536,    80,    87,   537,   541,
      88,   542,   543,    89,   153,   446,   154,   548,    75,   549,
     550,   552,   553,   554,   556,   561,    78,    79,   562,   574,
     581,    80,   582,   586,   587,   598,   599,   592,   593,   153,
     603,   154,   152,    66,    67,    68,   597,   238,    69,   214,
     570,   332,   604,    71,    84,   605,   405,   457,   264,   152,
      66,    67,    68,    73,   232,    69,   602,   585,    85,   563,
      71,   352,    87,   607,   372,   233,   234,   568,    89,    84,
      73,    75,   456,   361,   546,   399,   294,   425,   500,    78,
      79,   363,   391,    85,    80,   482,   115,    87,    75,   240,
      88,   234,   153,    89,   154,     0,    78,    79,     0,     0,
       0,    80,     0,     0,     0,     0,     0,     0,     0,   153,
       0,   154,   152,    66,    67,    68,     0,     0,    69,     0,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
       0,     0,    84,    73,     0,     0,     0,   152,    66,    67,
      68,     0,     0,    69,     0,     0,    85,     0,    71,    84,
      87,    75,     0,    88,     0,     0,    89,  -315,    73,    78,
      79,     0,     0,    85,    80,     0,     0,    87,   167,     0,
      88,     0,   153,    89,   154,     0,    75,     0,     0,     0,
       0,     0,     0,     0,    78,    79,     0,     0,     0,    80,
       0,   331,     0,     0,     0,   168,   167,   153,     0,   154,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   167,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   168,     0,     0,    85,     0,     0,     0,
      87,     0,     0,    88,     0,     0,    89,    84,   168,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
       0,    85,     0,     0,     0,    87,     0,     0,    88,     0,
       0,    89,     0,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   167,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   336,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   167,     0,     0,     0,     0,     0,
     168,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   168,     0,     0,     0,     0,     0,     0,   167,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   168,   167,     0,     0,   345,
     346,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,     0,     0,     0,     0,
     347,   348,     0,   168,   167,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
       0,   168,   167,     0,     0,   346,     0,     0,     0,     0,
       0,     0,     0,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,     0,   168,
     167,     0,     0,   348,     0,     0,     0,     0,     0,     0,
       0,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,     0,   168,   167,     0,
     258,     0,     0,     0,     0,     0,     0,     0,     0,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,     0,   168,   167,     0,   337,     0,
       0,     0,     0,     0,     0,     0,     0,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,     0,   168,   167,     0,   342,     0,     0,     0,
       0,     0,     0,     0,     0,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
       0,   168,     0,     0,   349,     0,     0,     0,     0,     0,
       0,     0,     0,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,     0,     0,
       0,     0,   454,     0,     0,     0,     0,     0,     0,     0,
       0,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   103,     0,     0,     0,
     455,     1,     0,     0,     2,     0,     3,     0,   104,   105,
       0,     4,     0,     0,     0,     5,     0,     0,     6,     7,
       8,     9,   167,     0,    10,     0,   106,    11,     0,     0,
       0,     0,   107,     0,     0,     0,   167,    12,     0,     0,
       0,     0,     0,     0,   108,     0,     0,     0,     0,   168,
     167,     0,   109,   110,     0,    13,     0,     0,     0,     0,
       0,     0,    14,   168,    15,   101,    16,     0,    17,   111,
      18,     0,    19,   112,     0,     0,     0,   168,     0,     0,
       0,    20,   113,    21,     0,     0,     0,     0,     0,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   167,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   168,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183
};

static const short yycheck[] =
{
     126,   312,   111,   112,    44,    16,   323,     3,     9,    13,
      14,     3,   370,     5,   123,   373,     8,    44,   362,    68,
       3,     4,    38,    51,     5,    13,    14,     8,   210,    13,
      14,     5,     3,     5,     9,     5,     6,     8,     8,    32,
       3,     3,     3,     5,    14,     3,   357,   407,    84,     3,
      33,     5,     6,   104,     8,    24,    82,    18,     5,     6,
      86,   187,     5,     6,     5,    48,   107,    50,    13,    14,
      19,    14,   121,    42,    85,   126,    87,    88,   422,    62,
       5,     6,   110,     8,   125,    54,   126,    36,    21,    14,
     126,   104,   201,    51,     5,     6,   407,     8,    61,   126,
      61,   461,   118,    14,   462,    98,   450,   465,   466,   467,
     468,   469,   470,   126,    68,   125,   126,    52,    89,   436,
     103,    90,   110,   124,   435,   136,   130,    60,   124,   140,
     312,   448,   124,     3,   145,   261,   117,   148,   149,   150,
     124,   124,   130,   117,   125,   117,   130,   117,    18,   124,
     461,   125,   163,   125,   124,   117,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   517,   117,   357,   117,     5,     6,   124,
       8,   124,     3,    16,     5,   130,    14,   104,    75,   371,
     299,   300,   117,   562,    81,   553,   554,   323,    92,   124,
       3,   212,     5,     5,     6,     8,   117,     3,     4,   126,
      76,    77,    14,   582,   558,     3,   126,    83,   128,    52,
       8,   141,   233,   143,   535,   407,   146,   147,   545,    50,
       3,    64,   125,   126,   125,   126,     9,    33,    41,    72,
      73,   125,   126,    52,   370,   256,   257,   373,     3,     3,
       3,     4,    48,   435,    50,    10,    89,     3,     4,     5,
       6,   387,    45,     9,    10,    68,    44,    39,    14,   102,
     581,   397,    23,   125,   126,    21,    45,   403,    24,   461,
      33,   125,   126,   125,   126,   125,   126,    33,    95,   117,
      81,    46,     3,    96,    66,    48,    42,    50,    24,    25,
      51,     0,    48,    49,    50,    51,    57,   103,   123,    55,
     436,    56,    67,     3,   117,   117,    42,    63,     3,    65,
     331,    74,   448,     3,   125,   126,   125,   126,     3,    55,
      85,   125,   126,   125,   126,   346,   462,   348,   124,   465,
     466,   467,   468,   469,   470,    91,   118,   119,   120,     3,
     103,   125,   126,   535,   125,   126,     3,   103,     3,   110,
     111,   112,   113,   114,   115,   125,   126,   549,     3,    61,
     121,   117,   118,   129,   124,   121,   124,   124,   124,   110,
     562,   127,   124,   124,     3,     4,     5,     6,   124,   124,
       9,   124,   124,   404,   124,    14,   124,    39,   110,   581,
     582,   124,    21,    84,    95,    24,     3,    60,   101,     3,
     592,     3,    15,    79,    33,    94,     3,   125,   125,   545,
     124,   124,   110,    42,    66,     3,     5,   553,   554,    48,
      49,    50,    51,   110,   110,   117,    55,    58,    89,    89,
      89,     8,   104,     3,    63,     3,    65,    30,    81,     3,
       4,     5,     6,    39,   124,     9,     3,   126,   131,    50,
      14,   125,     5,   126,   124,     3,    59,    21,   100,   110,
      24,     3,    91,   599,   116,   117,   118,   119,   120,    33,
      66,     3,   125,    87,   103,   125,   125,   110,    42,     5,
      47,   106,    82,   110,    48,    49,    50,    51,   117,   118,
      69,    55,   121,     8,   124,   124,   124,     8,   127,    63,
     124,    65,   124,   126,     3,     4,     5,     6,   124,     8,
       9,   126,   107,   125,   124,    14,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    24,     5,    91,     8,     3,
       4,     5,     6,   105,     8,     9,    76,     3,   118,   103,
      14,   125,   125,    42,   126,    70,     8,   125,    51,    26,
      24,    50,    51,   117,   118,    47,    55,   121,    70,   110,
     124,   126,   125,   127,    63,    18,    65,   126,    42,    26,
      61,   125,   107,   107,    68,    70,    50,    51,    26,     5,
     126,    55,    26,   124,     3,     8,   110,   126,   126,    63,
       5,    65,     3,     4,     5,     6,   124,     8,     9,   121,
     548,   233,   125,    14,   103,   125,   356,   404,   163,     3,
       4,     5,     6,    24,     8,     9,   592,   563,   117,   538,
      14,   259,   121,   601,   308,   124,   125,   542,   127,   103,
      24,    42,   403,   295,   495,   334,   188,   371,   448,    50,
      51,   297,   323,   117,    55,   421,    18,   121,    42,   137,
     124,   125,    63,   127,    65,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      -1,    65,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    24,    -1,    -1,    -1,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    -1,   117,    -1,    14,   103,
     121,    42,    -1,   124,    -1,    -1,   127,     0,    24,    50,
      51,    -1,    -1,   117,    55,    -1,    -1,   121,    39,    -1,
     124,    -1,    63,   127,    65,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,
      -1,    82,    -1,    -1,    -1,    66,    39,    63,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    39,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,   117,    -1,    -1,    -1,
     121,    -1,    -1,   124,    -1,    -1,   127,   103,    66,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,   117,    -1,    -1,    -1,   121,    -1,    -1,   124,    -1,
      -1,   127,    -1,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    66,    39,    -1,    -1,   125,
     126,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
     125,   126,    -1,    66,    39,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    66,    39,    -1,    -1,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    66,
      39,    -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    66,    39,    -1,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    66,    39,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    -1,    66,    39,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    66,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    16,    -1,    -1,    -1,
     125,    17,    -1,    -1,    20,    -1,    22,    -1,    28,    29,
      -1,    27,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,
      36,    37,    39,    -1,    40,    -1,    46,    43,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    39,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    66,
      39,    -1,    72,    73,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    66,    80,    85,    82,    -1,    84,    89,
      86,    -1,    88,    93,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    97,   102,    99,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    39,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    35,    36,    37,
      40,    43,    53,    71,    78,    80,    82,    84,    86,    88,
      97,    99,   133,   134,   135,   136,   137,   138,   145,   184,
     189,   190,   193,   194,   195,   196,   203,   204,   213,   214,
     215,   216,   220,   221,   226,   227,   228,   231,   233,   234,
     235,   236,   237,   239,   240,   241,    92,    52,     3,    45,
      44,    45,    75,    81,    52,     3,     4,     5,     6,     9,
      10,    14,    21,    24,    33,    42,    48,    49,    50,    51,
      55,    63,    65,    91,   103,   117,   118,   121,   124,   127,
     146,   147,   149,   177,   178,   238,   242,     3,    10,    46,
      67,    85,   229,    16,    28,    29,    46,    52,    64,    72,
      73,    89,    93,   102,   186,   229,    95,    81,     3,   150,
       0,   123,    56,     3,     3,     3,   124,     3,   150,     3,
       3,     3,     3,    13,    14,   124,   130,   243,   244,   129,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,     3,    63,    65,   177,   177,   177,     3,    51,
     181,   182,    44,   126,     3,    18,   148,    39,    66,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    61,   169,   170,   110,   110,     3,
       9,     3,     5,     6,     8,    68,   187,   188,    95,     3,
       8,    89,    84,     3,    60,   185,   185,   185,   101,     3,
      84,   126,    82,    86,   136,     3,   185,    15,    94,     5,
       6,     8,    14,   117,   124,   157,   158,   202,   205,   206,
      79,   104,     8,   124,   125,   177,   179,   180,     8,   177,
     244,     3,   177,   179,    38,   118,   179,   125,   177,   179,
     179,   177,   177,   177,   125,   124,   124,   124,   125,   126,
     128,   110,   124,   150,   147,   177,     3,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,     5,     3,     8,    41,    68,    96,
     117,   157,   191,   192,   188,   110,   110,   117,    58,    89,
      89,   185,    89,     8,   104,   222,   223,     3,   217,   218,
     242,     3,   124,   197,    30,    81,     5,     6,     8,   125,
     157,   159,   207,   126,   208,    24,    42,    54,    90,   232,
      50,    82,   138,   125,   126,   131,   131,   125,   125,     3,
     125,   125,   125,   125,   125,   125,   126,   125,   126,   125,
     177,   177,   182,     3,   157,   183,   142,   104,   151,   152,
     126,   191,   124,   187,    59,   185,   185,     3,   224,   225,
     110,   126,   152,   110,     3,     4,    33,    48,    50,   103,
     156,   198,   242,   100,     3,     3,   125,   126,   125,   126,
     202,   206,   209,   210,   125,    87,    51,   110,   125,   180,
     125,   177,   177,   110,    82,   145,    62,   124,   153,   154,
     155,   156,    47,   160,     5,   159,    76,    77,    83,   230,
     110,   106,   124,   157,   158,   218,    69,   171,   172,   157,
     158,   124,   124,   124,   125,   126,   124,   199,   200,    24,
      25,    42,    55,   219,   157,     8,    18,   211,   126,     8,
     124,   157,   126,   139,   125,   125,   183,   146,   125,   124,
     153,   107,    23,    51,    57,   110,   111,   112,   113,   114,
     115,   121,     5,   161,   105,   163,   164,   125,    32,    98,
      76,     8,   225,   125,   159,     3,   173,   174,   118,   125,
     125,   156,   124,   201,   202,   126,     3,    61,   212,   202,
     210,   159,     3,     5,    50,   140,   141,   125,    44,    70,
     143,     8,   125,   153,   157,   158,     9,   124,    68,   121,
       8,   157,   158,     8,   157,   158,   157,   158,   157,   158,
     157,   158,   157,   158,    51,    26,    47,    70,   165,   166,
     125,   110,   126,   125,   125,   126,   200,   125,   126,    26,
      61,   144,   125,   107,   107,   159,    68,     9,   124,   156,
     162,    70,    26,   169,     3,     5,     8,   124,   174,   202,
     141,   156,   167,   168,     5,   157,   158,   157,   158,   125,
     159,   126,    26,    74,   167,   171,   124,     3,   175,   176,
      19,    36,   126,   126,   125,   156,   167,   124,     8,   110,
     125,   126,   168,     5,   125,   125,   157,   176
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

  case 28:

    { pParser->PushQuery(); ;}
    break;

  case 29:

    { pParser->PushQuery(); ;}
    break;

  case 33:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 35:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 38:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 39:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 43:

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

  case 44:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 46:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 47:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 48:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-6] );
		;}
    break;

  case 51:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 54:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 55:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 56:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 59:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 60:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 61:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 62:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 63:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 64:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 66:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 73:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 74:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 75:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 76:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 77:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 78:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 95:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 96:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 97:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 100:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 101:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 102:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 103:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 104:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 106:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 107:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 108:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 109:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 110:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 111:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 112:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 116:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 117:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 118:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 121:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 124:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 125:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 127:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 129:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 130:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 133:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 134:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 143:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 146:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 147:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 149:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 150:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 155:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 156:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 157:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 158:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 159:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 160:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 161:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 162:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 163:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 164:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 165:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 166:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 167:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 168:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 169:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 170:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 171:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 172:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 173:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 174:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 175:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 178:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 179:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 180:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 181:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 182:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 183:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 186:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 191:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 192:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 199:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 200:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 205:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 206:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 207:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 208:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 216:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 217:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 218:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 219:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 220:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 221:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 222:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 225:

    { yyval.m_iValue = 1; ;}
    break;

  case 226:

    { yyval.m_iValue = 0; ;}
    break;

  case 227:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 228:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 229:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 230:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 233:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 234:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 235:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 238:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 239:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 242:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 243:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 244:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 245:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 246:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 247:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 248:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 249:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 250:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 251:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 252:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 253:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 254:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 256:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 257:

    {
			// FIXME? for now, one such array per CALL statement, tops
			if ( pParser->m_pStmt->m_dCallStrings.GetLength() )
			{
				yyerror ( pParser, "unexpected constant string list" );
				YYERROR;
			}
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 258:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 261:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 263:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 268:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 271:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 272:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 275:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)yyvsp[0].m_iValue );
			DWORD uHi = (DWORD)( yyvsp[0].m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_INTEGER );
			}
		;}
    break;

  case 276:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 277:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 278:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 279:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)yyvsp[0].m_iValue );
			DWORD uHi = (DWORD)( yyvsp[0].m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_INTEGER );
			}
		;}
    break;

  case 280:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 281:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 282:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 283:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 284:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 285:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 286:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 293:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 294:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 295:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 303:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 304:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 305:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 306:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 307:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 308:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 309:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 310:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 311:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 312:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 315:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 316:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 317:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 318:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 320:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 321:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 322:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 323:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 324:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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
	    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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

