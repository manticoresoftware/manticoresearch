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
     TOK_DATABASES = 290,
     TOK_DELETE = 291,
     TOK_DESC = 292,
     TOK_DESCRIBE = 293,
     TOK_DISTINCT = 294,
     TOK_DIV = 295,
     TOK_DROP = 296,
     TOK_FALSE = 297,
     TOK_FLOAT = 298,
     TOK_FLUSH = 299,
     TOK_FOR = 300,
     TOK_FROM = 301,
     TOK_FUNCTION = 302,
     TOK_GLOBAL = 303,
     TOK_GROUP = 304,
     TOK_GROUPBY = 305,
     TOK_GROUP_CONCAT = 306,
     TOK_HAVING = 307,
     TOK_ID = 308,
     TOK_IN = 309,
     TOK_INDEX = 310,
     TOK_INSERT = 311,
     TOK_INT = 312,
     TOK_INTEGER = 313,
     TOK_INTO = 314,
     TOK_IS = 315,
     TOK_ISOLATION = 316,
     TOK_LEVEL = 317,
     TOK_LIKE = 318,
     TOK_LIMIT = 319,
     TOK_MATCH = 320,
     TOK_MAX = 321,
     TOK_META = 322,
     TOK_MIN = 323,
     TOK_MOD = 324,
     TOK_NAMES = 325,
     TOK_NULL = 326,
     TOK_OPTION = 327,
     TOK_ORDER = 328,
     TOK_OPTIMIZE = 329,
     TOK_PLAN = 330,
     TOK_PROFILE = 331,
     TOK_RAND = 332,
     TOK_RAMCHUNK = 333,
     TOK_READ = 334,
     TOK_REPEATABLE = 335,
     TOK_REPLACE = 336,
     TOK_RETURNS = 337,
     TOK_ROLLBACK = 338,
     TOK_RTINDEX = 339,
     TOK_SELECT = 340,
     TOK_SERIALIZABLE = 341,
     TOK_SET = 342,
     TOK_SESSION = 343,
     TOK_SHOW = 344,
     TOK_SONAME = 345,
     TOK_START = 346,
     TOK_STATUS = 347,
     TOK_STRING = 348,
     TOK_SUM = 349,
     TOK_TABLE = 350,
     TOK_TABLES = 351,
     TOK_TO = 352,
     TOK_TRANSACTION = 353,
     TOK_TRUE = 354,
     TOK_TRUNCATE = 355,
     TOK_UNCOMMITTED = 356,
     TOK_UPDATE = 357,
     TOK_VALUES = 358,
     TOK_VARIABLES = 359,
     TOK_WARNINGS = 360,
     TOK_WEIGHT = 361,
     TOK_WHERE = 362,
     TOK_WITHIN = 363,
     TOK_OR = 364,
     TOK_AND = 365,
     TOK_NE = 366,
     TOK_GTE = 367,
     TOK_LTE = 368,
     TOK_NOT = 369,
     TOK_NEG = 370
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
#define TOK_DATABASES 290
#define TOK_DELETE 291
#define TOK_DESC 292
#define TOK_DESCRIBE 293
#define TOK_DISTINCT 294
#define TOK_DIV 295
#define TOK_DROP 296
#define TOK_FALSE 297
#define TOK_FLOAT 298
#define TOK_FLUSH 299
#define TOK_FOR 300
#define TOK_FROM 301
#define TOK_FUNCTION 302
#define TOK_GLOBAL 303
#define TOK_GROUP 304
#define TOK_GROUPBY 305
#define TOK_GROUP_CONCAT 306
#define TOK_HAVING 307
#define TOK_ID 308
#define TOK_IN 309
#define TOK_INDEX 310
#define TOK_INSERT 311
#define TOK_INT 312
#define TOK_INTEGER 313
#define TOK_INTO 314
#define TOK_IS 315
#define TOK_ISOLATION 316
#define TOK_LEVEL 317
#define TOK_LIKE 318
#define TOK_LIMIT 319
#define TOK_MATCH 320
#define TOK_MAX 321
#define TOK_META 322
#define TOK_MIN 323
#define TOK_MOD 324
#define TOK_NAMES 325
#define TOK_NULL 326
#define TOK_OPTION 327
#define TOK_ORDER 328
#define TOK_OPTIMIZE 329
#define TOK_PLAN 330
#define TOK_PROFILE 331
#define TOK_RAND 332
#define TOK_RAMCHUNK 333
#define TOK_READ 334
#define TOK_REPEATABLE 335
#define TOK_REPLACE 336
#define TOK_RETURNS 337
#define TOK_ROLLBACK 338
#define TOK_RTINDEX 339
#define TOK_SELECT 340
#define TOK_SERIALIZABLE 341
#define TOK_SET 342
#define TOK_SESSION 343
#define TOK_SHOW 344
#define TOK_SONAME 345
#define TOK_START 346
#define TOK_STATUS 347
#define TOK_STRING 348
#define TOK_SUM 349
#define TOK_TABLE 350
#define TOK_TABLES 351
#define TOK_TO 352
#define TOK_TRANSACTION 353
#define TOK_TRUE 354
#define TOK_TRUNCATE 355
#define TOK_UNCOMMITTED 356
#define TOK_UPDATE 357
#define TOK_VALUES 358
#define TOK_VARIABLES 359
#define TOK_WARNINGS 360
#define TOK_WEIGHT 361
#define TOK_WHERE 362
#define TOK_WITHIN 363
#define TOK_OR 364
#define TOK_AND 365
#define TOK_NE 366
#define TOK_GTE 367
#define TOK_LTE 368
#define TOK_NOT 369
#define TOK_NEG 370




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
#define YYFINAL  126
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1415

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  135
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  118
/* YYNRULES -- Number of rules. */
#define YYNRULES  336
/* YYNRULES -- Number of states. */
#define YYNSTATES  633

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   370

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   123,   112,     2,
     127,   128,   121,   119,   129,   120,   132,   122,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   126,
     115,   113,   116,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   133,     2,   134,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   130,   111,   131,     2,     2,     2,     2,
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
     105,   106,   107,   108,   109,   110,   114,   117,   118,   124,
     125
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    56,    58,
      60,    64,    66,    68,    70,    79,    81,    91,    92,    95,
      97,   101,   103,   105,   107,   108,   112,   113,   116,   121,
     133,   135,   139,   141,   144,   145,   147,   150,   152,   157,
     162,   167,   172,   177,   182,   186,   192,   194,   198,   199,
     201,   204,   206,   210,   214,   219,   221,   225,   229,   235,
     242,   246,   251,   257,   261,   265,   269,   273,   277,   279,
     285,   291,   297,   301,   305,   309,   313,   317,   321,   325,
     330,   334,   336,   338,   343,   347,   351,   353,   355,   357,
     360,   362,   365,   367,   369,   373,   374,   379,   380,   382,
     384,   388,   389,   392,   393,   395,   401,   402,   404,   408,
     414,   416,   420,   422,   425,   428,   429,   431,   434,   439,
     440,   442,   445,   447,   451,   455,   459,   465,   472,   476,
     478,   482,   486,   488,   490,   492,   494,   496,   498,   500,
     503,   506,   510,   514,   518,   522,   526,   530,   534,   538,
     542,   546,   550,   554,   558,   562,   566,   570,   574,   578,
     582,   584,   586,   588,   593,   598,   603,   608,   613,   617,
     624,   631,   635,   644,   646,   650,   652,   654,   658,   664,
     666,   668,   670,   672,   675,   676,   679,   681,   684,   687,
     691,   693,   695,   700,   705,   709,   711,   713,   715,   717,
     719,   721,   725,   730,   735,   740,   744,   749,   754,   762,
     768,   770,   772,   774,   776,   778,   780,   782,   784,   786,
     789,   796,   798,   800,   801,   805,   807,   811,   813,   817,
     821,   823,   827,   829,   831,   833,   837,   840,   848,   858,
     865,   867,   871,   873,   877,   879,   883,   884,   887,   889,
     893,   897,   898,   900,   902,   904,   908,   910,   912,   916,
     920,   927,   929,   933,   937,   941,   947,   952,   956,   960,
     962,   964,   966,   968,   976,   981,   987,   988,   990,   993,
     995,   999,  1003,  1006,  1010,  1017,  1018,  1020,  1022,  1025,
    1028,  1031,  1033,  1041,  1043,  1045,  1047,  1049,  1053,  1060,
    1064,  1068,  1072,  1074,  1078,  1081,  1085,  1089,  1092,  1094,
    1097,  1099,  1101,  1105,  1109,  1113,  1117
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     136,     0,    -1,   137,    -1,   138,    -1,   138,   126,    -1,
     200,    -1,   208,    -1,   194,    -1,   195,    -1,   198,    -1,
     209,    -1,   218,    -1,   220,    -1,   221,    -1,   222,    -1,
     227,    -1,   232,    -1,   233,    -1,   237,    -1,   239,    -1,
     240,    -1,   241,    -1,   242,    -1,   234,    -1,   243,    -1,
     245,    -1,   246,    -1,   247,    -1,   226,    -1,   139,    -1,
     138,   126,   139,    -1,   140,    -1,   189,    -1,   141,    -1,
      85,     3,   127,   127,   141,   128,   142,   128,    -1,   148,
      -1,    85,   149,    46,   127,   145,   148,   128,   146,   147,
      -1,    -1,   129,   143,    -1,   144,    -1,   143,   129,   144,
      -1,     3,    -1,     5,    -1,    53,    -1,    -1,    73,    26,
     172,    -1,    -1,    64,     5,    -1,    64,     5,   129,     5,
      -1,    85,   149,    46,   153,   154,   164,   168,   167,   170,
     174,   176,    -1,   150,    -1,   149,   129,   150,    -1,   121,
      -1,   152,   151,    -1,    -1,     3,    -1,    18,     3,    -1,
     182,    -1,    21,   127,   182,   128,    -1,    66,   127,   182,
     128,    -1,    68,   127,   182,   128,    -1,    94,   127,   182,
     128,    -1,    51,   127,   182,   128,    -1,    33,   127,   121,
     128,    -1,    50,   127,   128,    -1,    33,   127,    39,     3,
     128,    -1,     3,    -1,   153,   129,     3,    -1,    -1,   155,
      -1,   107,   156,    -1,   157,    -1,   156,   110,   156,    -1,
     127,   156,   128,    -1,    65,   127,     8,   128,    -1,   158,
      -1,   160,   113,   161,    -1,   160,   114,   161,    -1,   160,
      54,   127,   163,   128,    -1,   160,   124,    54,   127,   163,
     128,    -1,   160,    54,     9,    -1,   160,   124,    54,     9,
      -1,   160,    23,   161,   110,   161,    -1,   160,   116,   161,
      -1,   160,   115,   161,    -1,   160,   117,   161,    -1,   160,
     118,   161,    -1,   160,   113,   162,    -1,   159,    -1,   160,
      23,   162,   110,   162,    -1,   160,    23,   161,   110,   162,
      -1,   160,    23,   162,   110,   161,    -1,   160,   116,   162,
      -1,   160,   115,   162,    -1,   160,   117,   162,    -1,   160,
     118,   162,    -1,   160,   113,     8,    -1,   160,   114,     8,
      -1,   160,    60,    71,    -1,   160,    60,   124,    71,    -1,
     160,   114,   162,    -1,     3,    -1,     4,    -1,    33,   127,
     121,   128,    -1,    50,   127,   128,    -1,   106,   127,   128,
      -1,    53,    -1,   248,    -1,     5,    -1,   120,     5,    -1,
       6,    -1,   120,     6,    -1,    14,    -1,   161,    -1,   163,
     129,   161,    -1,    -1,    49,   165,    26,   166,    -1,    -1,
       5,    -1,   160,    -1,   166,   129,   160,    -1,    -1,    52,
     158,    -1,    -1,   169,    -1,   108,    49,    73,    26,   172,
      -1,    -1,   171,    -1,    73,    26,   172,    -1,    73,    26,
      77,   127,   128,    -1,   173,    -1,   172,   129,   173,    -1,
     160,    -1,   160,    19,    -1,   160,    37,    -1,    -1,   175,
      -1,    64,     5,    -1,    64,     5,   129,     5,    -1,    -1,
     177,    -1,    72,   178,    -1,   179,    -1,   178,   129,   179,
      -1,     3,   113,     3,    -1,     3,   113,     5,    -1,     3,
     113,   127,   180,   128,    -1,     3,   113,     3,   127,     8,
     128,    -1,     3,   113,     8,    -1,   181,    -1,   180,   129,
     181,    -1,     3,   113,   161,    -1,     3,    -1,     4,    -1,
      53,    -1,     5,    -1,     6,    -1,    14,    -1,     9,    -1,
     120,   182,    -1,   124,   182,    -1,   182,   119,   182,    -1,
     182,   120,   182,    -1,   182,   121,   182,    -1,   182,   122,
     182,    -1,   182,   115,   182,    -1,   182,   116,   182,    -1,
     182,   112,   182,    -1,   182,   111,   182,    -1,   182,   123,
     182,    -1,   182,    40,   182,    -1,   182,    69,   182,    -1,
     182,   118,   182,    -1,   182,   117,   182,    -1,   182,   113,
     182,    -1,   182,   114,   182,    -1,   182,   110,   182,    -1,
     182,   109,   182,    -1,   127,   182,   128,    -1,   130,   186,
     131,    -1,   183,    -1,   248,    -1,   251,    -1,     3,   127,
     184,   128,    -1,    54,   127,   184,   128,    -1,    58,   127,
     184,   128,    -1,    24,   127,   184,   128,    -1,    43,   127,
     184,   128,    -1,     3,   127,   128,    -1,    68,   127,   182,
     129,   182,   128,    -1,    66,   127,   182,   129,   182,   128,
      -1,   106,   127,   128,    -1,     3,   127,   182,    45,     3,
      54,   248,   128,    -1,   185,    -1,   184,   129,   185,    -1,
     182,    -1,     8,    -1,   187,   113,   188,    -1,   186,   129,
     187,   113,   188,    -1,     3,    -1,    54,    -1,   161,    -1,
       3,    -1,    89,   191,    -1,    -1,    63,     8,    -1,   105,
      -1,    92,   190,    -1,    67,   190,    -1,    16,    92,   190,
      -1,    76,    -1,    75,    -1,    16,     8,    92,   190,    -1,
      16,     3,    92,   190,    -1,    55,     3,    92,    -1,     3,
      -1,    71,    -1,     8,    -1,     5,    -1,     6,    -1,   192,
      -1,   193,   120,   192,    -1,    87,     3,   113,   197,    -1,
      87,     3,   113,   196,    -1,    87,     3,   113,    71,    -1,
      87,    70,   193,    -1,    87,    10,   113,   193,    -1,    87,
      28,    87,   193,    -1,    87,    48,     9,   113,   127,   163,
     128,    -1,    87,    48,     3,   113,   196,    -1,     3,    -1,
       8,    -1,    99,    -1,    42,    -1,   161,    -1,    31,    -1,
      83,    -1,   199,    -1,    22,    -1,    91,    98,    -1,   201,
      59,     3,   202,   103,   204,    -1,    56,    -1,    81,    -1,
      -1,   127,   203,   128,    -1,   160,    -1,   203,   129,   160,
      -1,   205,    -1,   204,   129,   205,    -1,   127,   206,   128,
      -1,   207,    -1,   206,   129,   207,    -1,   161,    -1,   162,
      -1,     8,    -1,   127,   163,   128,    -1,   127,   128,    -1,
      36,    46,   153,   107,    53,   113,   161,    -1,    36,    46,
     153,   107,    53,    54,   127,   163,   128,    -1,    27,     3,
     127,   210,   213,   128,    -1,   211,    -1,   210,   129,   211,
      -1,   207,    -1,   127,   212,   128,    -1,     8,    -1,   212,
     129,     8,    -1,    -1,   129,   214,    -1,   215,    -1,   214,
     129,   215,    -1,   207,   216,   217,    -1,    -1,    18,    -1,
       3,    -1,    64,    -1,   219,     3,   190,    -1,    38,    -1,
      37,    -1,    89,    96,   190,    -1,    89,    35,   190,    -1,
     102,   153,    87,   223,   155,   176,    -1,   224,    -1,   223,
     129,   224,    -1,     3,   113,   161,    -1,     3,   113,   162,
      -1,     3,   113,   127,   163,   128,    -1,     3,   113,   127,
     128,    -1,   248,   113,   161,    -1,   248,   113,   162,    -1,
      58,    -1,    24,    -1,    43,    -1,    25,    -1,    17,    95,
       3,    15,    30,     3,   225,    -1,    89,   235,   104,   228,
      -1,    89,   235,   104,    63,     8,    -1,    -1,   229,    -1,
     107,   230,    -1,   231,    -1,   230,   109,   231,    -1,     3,
     113,     8,    -1,    89,    29,    -1,    89,    28,    87,    -1,
      87,   235,    98,    61,    62,   236,    -1,    -1,    48,    -1,
      88,    -1,    79,   101,    -1,    79,    32,    -1,    80,    79,
      -1,    86,    -1,    34,    47,     3,    82,   238,    90,     8,
      -1,    57,    -1,    24,    -1,    43,    -1,    93,    -1,    41,
      47,     3,    -1,    20,    55,     3,    97,    84,     3,    -1,
      44,    84,     3,    -1,    44,    78,     3,    -1,    85,   244,
     174,    -1,    10,    -1,    10,   132,     3,    -1,    85,   182,
      -1,   100,    84,     3,    -1,    74,    55,     3,    -1,     3,
     249,    -1,   250,    -1,   249,   250,    -1,    13,    -1,    14,
      -1,   133,   182,   134,    -1,   133,     8,   134,    -1,   182,
     113,   252,    -1,   252,   113,   182,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   161,   161,   162,   163,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   196,
     197,   201,   202,   206,   207,   215,   216,   223,   225,   229,
     233,   240,   241,   242,   246,   259,   266,   268,   273,   282,
     298,   299,   303,   304,   307,   309,   310,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   326,   327,   330,   332,
     336,   340,   341,   342,   346,   351,   355,   362,   370,   378,
     387,   392,   397,   402,   407,   412,   417,   422,   427,   432,
     437,   442,   447,   452,   457,   462,   467,   472,   477,   482,
     490,   494,   495,   500,   506,   512,   518,   524,   528,   529,
     540,   541,   542,   546,   552,   558,   560,   563,   565,   572,
     576,   582,   584,   590,   592,   596,   607,   609,   613,   617,
     624,   625,   629,   630,   631,   634,   636,   640,   645,   652,
     654,   658,   662,   663,   667,   672,   677,   683,   688,   696,
     701,   708,   718,   719,   720,   721,   722,   723,   724,   725,
     726,   728,   729,   730,   731,   732,   733,   734,   735,   736,
     737,   738,   739,   740,   741,   742,   743,   744,   745,   746,
     747,   748,   749,   753,   754,   755,   756,   757,   758,   759,
     760,   761,   762,   766,   767,   771,   772,   776,   777,   781,
     782,   786,   787,   793,   796,   798,   802,   803,   804,   805,
     806,   807,   808,   813,   818,   828,   829,   830,   831,   832,
     836,   837,   841,   846,   851,   856,   857,   858,   862,   867,
     875,   876,   880,   881,   882,   896,   897,   898,   902,   903,
     909,   917,   918,   921,   923,   927,   928,   932,   933,   937,
     941,   942,   946,   947,   948,   949,   950,   956,   964,   978,
     986,   990,   997,   998,  1005,  1015,  1021,  1023,  1027,  1032,
    1036,  1043,  1045,  1049,  1050,  1056,  1064,  1065,  1071,  1075,
    1081,  1089,  1090,  1094,  1108,  1114,  1118,  1123,  1137,  1148,
    1149,  1150,  1151,  1155,  1168,  1172,  1179,  1180,  1184,  1188,
    1189,  1193,  1197,  1204,  1211,  1217,  1218,  1219,  1223,  1224,
    1225,  1226,  1232,  1243,  1244,  1245,  1246,  1250,  1261,  1273,
    1282,  1293,  1301,  1302,  1306,  1316,  1327,  1338,  1341,  1342,
    1346,  1347,  1348,  1349,  1353,  1354,  1358
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
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", 
  "TOK_DATABASES", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", 
  "TOK_FLUSH", "TOK_FOR", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", 
  "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_HAVING", "TOK_ID", 
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
  "where_item", "filter_item", "expr_float_unhandled", "expr_ident", 
  "const_int", "const_float", "const_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_having_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "hash_key", "hash_val", "show_stmt", "like_filter", 
  "show_what", "simple_set_value", "set_value", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "show_databases", "update", 
  "update_items_list", "update_item", "alter_col_type", "alter", 
  "show_variables", "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "flush_ramchunk", "select_sysvar", 
  "sysvar_name", "select_dual", "truncate", "optimize_index", 
  "json_field", "subscript", "subkey", "streq", "strval", 0
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
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   124,    38,    61,   366,    60,    62,   367,   368,    43,
      45,    42,    47,    37,   369,   370,    59,    40,    41,    44,
     123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   135,   136,   136,   136,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   138,
     138,   139,   139,   140,   140,   141,   141,   142,   142,   143,
     143,   144,   144,   144,   145,   146,   147,   147,   147,   148,
     149,   149,   150,   150,   151,   151,   151,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   153,   153,   154,   154,
     155,   156,   156,   156,   157,   157,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     159,   160,   160,   160,   160,   160,   160,   160,   161,   161,
     162,   162,   162,   163,   163,   164,   164,   165,   165,   166,
     166,   167,   167,   168,   168,   169,   170,   170,   171,   171,
     172,   172,   173,   173,   173,   174,   174,   175,   175,   176,
     176,   177,   178,   178,   179,   179,   179,   179,   179,   180,
     180,   181,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   184,   184,   185,   185,   186,   186,   187,
     187,   188,   188,   189,   190,   190,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   192,   192,   192,   192,   192,
     193,   193,   194,   194,   194,   194,   194,   194,   195,   195,
     196,   196,   197,   197,   197,   198,   198,   198,   199,   199,
     200,   201,   201,   202,   202,   203,   203,   204,   204,   205,
     206,   206,   207,   207,   207,   207,   207,   208,   208,   209,
     210,   210,   211,   211,   212,   212,   213,   213,   214,   214,
     215,   216,   216,   217,   217,   218,   219,   219,   220,   221,
     222,   223,   223,   224,   224,   224,   224,   224,   224,   225,
     225,   225,   225,   226,   227,   227,   228,   228,   229,   230,
     230,   231,   232,   233,   234,   235,   235,   235,   236,   236,
     236,   236,   237,   238,   238,   238,   238,   239,   240,   241,
     242,   243,   244,   244,   245,   246,   247,   248,   249,   249,
     250,   250,   250,   250,   251,   251,   252
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     8,     1,     9,     0,     2,     1,
       3,     1,     1,     1,     0,     3,     0,     2,     4,    11,
       1,     3,     1,     2,     0,     1,     2,     1,     4,     4,
       4,     4,     4,     4,     3,     5,     1,     3,     0,     1,
       2,     1,     3,     3,     4,     1,     3,     3,     5,     6,
       3,     4,     5,     3,     3,     3,     3,     3,     1,     5,
       5,     5,     3,     3,     3,     3,     3,     3,     3,     4,
       3,     1,     1,     4,     3,     3,     1,     1,     1,     2,
       1,     2,     1,     1,     3,     0,     4,     0,     1,     1,
       3,     0,     2,     0,     1,     5,     0,     1,     3,     5,
       1,     3,     1,     2,     2,     0,     1,     2,     4,     0,
       1,     2,     1,     3,     3,     3,     5,     6,     3,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     1,     4,     4,     4,     4,     4,     3,     6,
       6,     3,     8,     1,     3,     1,     1,     3,     5,     1,
       1,     1,     1,     2,     0,     2,     1,     2,     2,     3,
       1,     1,     4,     4,     3,     1,     1,     1,     1,     1,
       1,     3,     4,     4,     4,     3,     4,     4,     7,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       6,     1,     1,     0,     3,     1,     3,     1,     3,     3,
       1,     3,     1,     1,     1,     3,     2,     7,     9,     6,
       1,     3,     1,     3,     1,     3,     0,     2,     1,     3,
       3,     0,     1,     1,     1,     3,     1,     1,     3,     3,
       6,     1,     3,     3,     3,     5,     4,     3,     3,     1,
       1,     1,     1,     7,     4,     5,     0,     1,     2,     1,
       3,     3,     2,     3,     6,     0,     1,     1,     2,     2,
       2,     1,     7,     1,     1,     1,     1,     3,     6,     3,
       3,     3,     1,     3,     2,     3,     3,     2,     1,     2,
       1,     1,     3,     3,     3,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   238,     0,   235,     0,     0,   277,   276,
       0,     0,   241,     0,   242,   236,     0,   305,   305,     0,
       0,     0,     0,     2,     3,    29,    31,    33,    35,    32,
       7,     8,     9,   237,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   152,   153,   155,   156,
     336,   158,   322,   157,     0,     0,     0,     0,     0,     0,
     154,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,    50,    54,    57,   180,   135,   181,   182,
       0,     0,     0,     0,   306,     0,   307,     0,     0,     0,
     302,   204,   306,     0,   204,   211,   210,   204,   204,   206,
     203,     0,   239,     0,    66,     0,     1,     4,     0,   204,
       0,     0,     0,     0,     0,   317,   320,   319,   326,   330,
     331,     0,     0,   327,   328,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   152,     0,
       0,   159,   160,     0,   199,   200,     0,     0,     0,     0,
      55,     0,    53,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   321,   136,     0,     0,     0,     0,     0,     0,   215,
     218,   219,   217,   216,   220,   225,     0,     0,     0,   204,
     303,     0,   279,     0,   208,   207,   278,   296,   325,     0,
       0,     0,     0,    30,   243,   275,     0,     0,   108,   110,
     254,   112,     0,     0,   252,   253,   262,   266,   260,     0,
       0,   196,     0,   188,   195,     0,   193,   336,     0,   329,
     323,     0,   195,     0,     0,     0,     0,    64,     0,     0,
       0,     0,     0,     0,   191,     0,     0,     0,   178,     0,
     179,     0,    44,    68,    51,    57,    56,   170,   171,   177,
     176,   168,   167,   174,   334,   175,   165,   166,   173,   172,
     161,   162,   163,   164,   169,   137,   335,   230,   231,   233,
     224,   232,     0,   234,   223,   222,   226,   227,     0,     0,
       0,     0,   204,   204,   209,   205,   214,     0,     0,   294,
     297,     0,     0,   281,     0,    67,     0,     0,     0,     0,
     109,   111,   264,   256,   113,     0,     0,     0,     0,   314,
     315,   313,   316,     0,     0,     0,     0,     0,   183,     0,
     333,   332,    58,   186,     0,    63,   187,    62,   184,   185,
      59,     0,    60,     0,    61,     0,     0,     0,   202,   201,
     197,     0,     0,   115,    69,     0,   229,     0,   221,     0,
     213,   212,   295,     0,   298,   299,     0,     0,   139,     0,
     101,   102,     0,     0,   106,     0,   245,     0,   107,     0,
       0,   318,   255,     0,   263,     0,   262,   261,   267,   268,
     259,     0,     0,     0,    37,     0,   194,    65,     0,     0,
       0,     0,     0,     0,     0,    70,    71,    75,    88,     0,
     117,   123,   138,     0,     0,     0,   311,   304,     0,     0,
       0,   283,   284,   282,     0,   280,   140,   287,   288,     0,
       0,     0,   244,     0,     0,   240,   247,   290,   292,   291,
     289,   293,   114,   265,   272,     0,     0,   312,     0,   257,
       0,     0,     0,   190,   189,   198,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   118,     0,     0,   121,   124,   228,   309,   308,   310,
     301,   300,   286,     0,     0,   141,   142,     0,   104,   105,
     246,     0,     0,   250,     0,   273,   274,   270,   271,   269,
       0,    41,    42,    43,    38,    39,    34,     0,     0,     0,
       0,    46,     0,    73,    72,     0,     0,    80,     0,    98,
       0,    96,    76,    87,    97,    77,   100,    84,    93,    83,
      92,    85,    94,    86,    95,     0,     0,     0,     0,   126,
     285,     0,     0,   103,   249,     0,   248,   258,     0,   192,
       0,     0,    36,    74,     0,     0,     0,    99,    81,     0,
     119,   116,     0,   122,     0,   135,   127,   144,   145,   148,
       0,   143,   251,    40,   132,    45,   130,    47,    82,    90,
      91,    89,    78,     0,     0,     0,     0,   139,     0,     0,
       0,   149,   133,   134,     0,     0,    79,   120,   125,     0,
     128,    49,     0,     0,   146,     0,   131,    48,     0,   147,
     151,   150,   129
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   471,   524,   525,
     371,   531,   572,    28,    92,    93,   172,    94,   273,   373,
     374,   425,   426,   427,   428,   429,   334,   235,   335,   431,
     492,   581,   559,   494,   495,   585,   586,   595,   596,   191,
     192,   445,   446,   505,   506,   610,   611,   252,    96,   245,
     246,   166,   167,   370,    29,   212,   120,   204,   205,    30,
      31,   304,   305,    32,    33,    34,    35,   327,   397,   455,
     456,   512,   236,    36,    37,   237,   238,   336,   338,   408,
     409,   465,   517,    38,    39,    40,    41,    42,   322,   323,
     461,    43,    44,   319,   320,   384,   385,    45,    46,    47,
     107,   437,    48,   343,    49,    50,    51,    52,    53,    97,
      54,    55,    56,    98,   143,   144,    99,   100
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -400
static const short yypact[] =
{
    1201,   -38,    11,  -400,    75,  -400,    78,    81,  -400,  -400,
     124,    74,  -400,   140,  -400,  -400,   209,   315,   956,   130,
     154,   244,   270,  -400,   142,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,   217,  -400,  -400,  -400,   275,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,   277,   282,   160,
     288,   244,   299,   301,   308,   311,    34,  -400,  -400,  -400,
    -400,  -400,   185,  -400,   208,   211,   219,   234,   240,   251,
    -400,   263,   271,   273,   283,   285,   290,   614,  -400,   614,
     614,   111,   -10,  -400,    46,   679,  -400,   287,  -400,  -400,
     317,   324,   325,   352,   228,    55,  -400,   342,    13,   354,
    -400,   382,  -400,   447,   382,  -400,  -400,   382,   382,  -400,
    -400,   347,  -400,   449,  -400,   -61,  -400,   118,   451,   382,
     440,   363,    24,   383,    -8,  -400,  -400,  -400,  -400,  -400,
    -400,   410,   650,    61,  -400,   463,   614,   718,    16,   718,
     341,   614,   718,   718,   614,   614,   614,   343,    37,   346,
     348,  -400,  -400,   953,  -400,  -400,    80,   357,     2,   378,
    -400,   476,  -400,   614,   614,   614,   614,   614,   614,   614,
     614,   614,   614,   614,   614,   614,   614,   614,   614,   614,
     475,  -400,  -400,   614,   248,    55,    55,   368,   369,  -400,
    -400,  -400,  -400,  -400,  -400,   365,   422,   395,   396,   382,
    -400,   481,  -400,   398,  -400,  -400,  -400,   -17,  -400,   489,
     490,   453,   245,  -400,   367,  -400,   465,   413,  -400,  -400,
    -400,  -400,   230,    14,  -400,  -400,  -400,   371,  -400,    30,
     448,   397,   546,  -400,  1150,   144,  -400,   375,   695,  -400,
    -400,   973,  1207,   166,   509,   385,   168,  -400,  1009,   195,
     198,   763,   785,  1037,  -400,   582,   614,   614,  -400,   111,
    -400,    28,  -400,    92,  -400,  1207,  -400,  -400,  -400,  1237,
    1262,  1274,  1292,   879,  -400,   879,   186,   186,   186,   186,
     326,   326,  -400,  -400,  -400,   386,   879,  -400,  -400,  -400,
    -400,  -400,   512,  -400,  -400,  -400,   365,   365,   224,   387,
      55,   456,   382,   382,  -400,  -400,  -400,   514,   517,  -400,
    -400,    56,    93,  -400,   411,  -400,   295,   420,   522,   523,
    -400,  -400,  -400,  -400,  -400,   203,   212,    24,   399,  -400,
    -400,  -400,  -400,   438,   -30,   378,   401,   528,  -400,   718,
    -400,  -400,  -400,  -400,   404,  -400,  -400,  -400,  -400,  -400,
    -400,   614,  -400,   614,  -400,   818,   848,   423,  -400,  -400,
    -400,   450,     6,   484,  -400,   534,  -400,     8,  -400,    36,
    -400,  -400,  -400,   428,   433,  -400,    39,   489,   471,    90,
      61,  -400,   417,   418,  -400,   419,  -400,   247,  -400,   421,
     200,  -400,  -400,     8,  -400,   545,    49,  -400,   427,  -400,
    -400,   549,   431,     8,   432,   508,  -400,  -400,  1064,  1094,
      28,   378,   435,   437,     6,   455,  -400,  -400,  -400,   256,
     561,   459,  -400,   260,    -4,   492,  -400,  -400,   560,   517,
      15,  -400,  -400,  -400,   566,  -400,  -400,  -400,  -400,   454,
     444,   464,  -400,   295,    29,   452,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,    98,    29,  -400,     8,  -400,
      79,   466,   573,  -400,  -400,  -400,    -5,   505,   571,    88,
       6,    90,    33,   -44,    71,    86,    90,    90,    90,    90,
     530,  -400,   556,   544,   543,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,   265,   485,   468,  -400,   473,  -400,  -400,
    -400,    18,   268,  -400,   421,  -400,  -400,  -400,   584,  -400,
     276,  -400,  -400,  -400,   474,  -400,  -400,    61,   477,   244,
     581,   547,   480,  -400,  -400,   499,   500,  -400,     8,  -400,
     542,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,    63,   295,   548,   295,   551,
    -400,     9,   566,  -400,  -400,    29,  -400,  -400,    79,  -400,
     295,   610,  -400,  -400,    90,    90,   278,  -400,  -400,     8,
    -400,   487,   600,  -400,   601,   287,  -400,   502,  -400,  -400,
     627,  -400,  -400,  -400,    83,   503,  -400,   504,  -400,  -400,
    -400,  -400,  -400,   280,   295,   295,   236,   471,   626,   524,
     294,  -400,  -400,  -400,   295,   634,  -400,  -400,   503,   515,
     503,  -400,   513,     8,  -400,   627,  -400,  -400,   516,  -400,
    -400,  -400,  -400
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -400,  -400,  -400,  -400,   518,  -400,   405,  -400,  -400,    94,
    -400,  -400,  -400,   272,   225,   482,  -400,  -400,    96,  -400,
     327,  -399,  -400,   102,  -400,  -322,  -132,  -378,  -375,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -247,    47,    84,
    -400,    58,  -400,  -400,   101,  -400,    50,    -1,  -400,    97,
     322,  -400,   408,   258,  -400,  -111,  -400,   373,   169,  -400,
    -400,   376,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
     167,  -400,  -336,  -400,  -400,  -400,   349,  -400,  -400,  -400,
     221,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,   298,
    -400,  -400,  -400,  -400,  -400,  -400,   250,  -400,  -400,  -400,
     672,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -179,  -400,   552,  -400,   519
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -337
static const short yytable[] =
{
     234,   406,   433,   214,   396,   124,   215,   216,   442,   390,
     391,   448,   587,   228,   588,    95,   207,   589,   225,   228,
     228,   208,   332,   228,   412,   479,   219,   539,   497,   228,
     229,   368,   230,   228,   228,   229,   168,   230,   231,   392,
     324,   529,   537,   231,   228,   229,   317,   139,   140,   170,
     139,   140,  -271,   231,   339,   254,   393,    57,   199,   394,
     200,   201,   303,   202,   171,   503,    58,   464,   220,   139,
     140,   423,   578,   340,   139,   140,   228,   229,    59,   541,
     540,   534,   521,   413,   522,   231,   161,   341,   162,   163,
     318,   228,   229,   520,   544,   228,   229,   498,   314,   240,
     231,   515,   612,   536,   231,   209,   543,   546,   548,   550,
     552,   554,   395,  -271,   164,   434,   435,   125,   513,   169,
     613,   220,   436,   342,   169,    60,   203,    61,   302,   272,
     518,   510,   523,   424,   302,   302,   590,   255,   302,   369,
     244,   248,   333,   502,   232,   251,   333,   398,   302,   232,
     258,   233,    63,   261,   262,   263,   511,   134,    64,   232,
     538,   141,   516,   576,   265,   165,   440,   142,   275,   386,
     142,    62,   277,   278,   279,   280,   281,   282,   283,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   142,
     579,   232,   296,   398,   142,    65,   599,   601,   480,   372,
     372,   380,   381,   221,   603,   234,   232,   222,   324,   269,
     232,   270,    66,    67,    68,    69,   533,    70,    71,    72,
     275,   220,   387,    73,   457,   458,   173,   297,   122,   592,
      74,   197,   298,    75,   580,   330,   331,   198,   123,   390,
     391,   163,    76,   459,   253,   398,   256,   124,   594,   259,
     260,   297,    77,   228,   441,   174,   298,   447,   460,    78,
      79,   108,    80,    81,   244,   365,   366,    82,   127,   392,
     126,   462,   348,   349,   398,    83,   128,    84,   129,   481,
     130,   469,   617,   594,   594,   131,   393,   132,   369,   394,
     299,   133,   594,   528,   353,   349,   356,   349,   390,   391,
     113,   398,   135,    85,   136,   185,   186,   187,   188,   189,
     482,   137,   114,   619,   138,    86,   483,   145,   101,   300,
     115,   116,   234,   358,   349,   102,   359,   349,   392,    87,
      88,   402,   403,    89,   234,   146,    90,   117,   147,    91,
     404,   405,   395,   103,   275,   393,   148,   301,   394,   535,
     119,   190,   542,   545,   547,   549,   551,   553,   618,   620,
     418,   149,   419,   104,   306,   307,   173,   150,   302,   484,
     485,   486,   487,   488,   489,   452,   453,   398,   151,   398,
     490,   158,    67,    68,    69,   105,    70,    71,   496,   403,
     152,   398,    73,   560,   403,   174,   564,   565,   153,    74,
     154,   395,    75,   106,   567,   403,   602,   403,   616,   403,
     155,    76,   156,   158,    67,    68,    69,   157,   241,    71,
     275,    77,   624,   625,    73,   398,   398,   398,    78,    79,
     193,    80,    81,   234,    75,   398,    82,   194,   195,   196,
     206,   210,   598,   600,    83,   211,    84,   187,   188,   189,
     213,   217,   218,    77,   224,   226,    66,    67,    68,    69,
     227,    70,    71,    80,    81,   239,   250,    73,    82,   257,
     271,   264,    85,   266,    74,   267,   159,    75,   160,   276,
     295,   308,   309,   311,    86,   310,    76,   312,   313,   315,
     316,   630,   321,   325,   326,   328,    77,   329,    87,    88,
     337,   344,    89,    78,    79,    90,    80,    81,    91,   350,
    -336,    82,   354,   355,   377,   375,    86,   330,   379,    83,
     383,    84,   382,   399,   389,   400,   401,   410,   411,   414,
      87,   415,   417,   430,    89,   421,   420,   242,   243,   432,
      91,   438,   439,   444,   449,   450,   451,    85,   454,   158,
      67,    68,    69,   463,    70,    71,   466,   467,   468,    86,
      73,   470,   472,   477,   478,   480,   491,   493,   500,   504,
      75,   499,   508,    87,    88,   507,   527,    89,   530,   532,
      90,   514,   556,    91,   555,   158,    67,    68,    69,    77,
     241,    71,   509,   557,   526,   558,    73,   562,   561,    80,
      81,   563,   464,   568,    82,   569,    75,   570,   573,   574,
     575,   571,   159,   577,   160,   597,   604,   158,    67,    68,
      69,   582,    70,    71,   584,    77,   605,   606,    73,   608,
     609,   345,   614,   615,   622,    80,    81,   623,    75,   627,
      82,   629,   628,   422,   632,   223,   476,   346,   159,   388,
     160,   274,    86,   158,    67,    68,    69,    77,   247,    71,
     583,   626,   593,   591,    73,   621,    87,    80,    81,   607,
      89,   416,    82,    90,    75,   631,    91,   367,   475,  -324,
     159,   566,   160,   378,   376,   443,   407,   519,    86,   501,
     121,     0,     0,    77,     0,   249,     0,     0,   284,     0,
       0,     0,    87,    80,    81,     0,    89,     0,    82,    90,
     243,     0,    91,     0,     0,     0,   159,     0,   160,   173,
      86,   158,    67,    68,    69,     0,   241,    71,     0,     0,
       0,     0,    73,     0,    87,   173,     0,     0,    89,     0,
       0,    90,    75,     0,    91,     0,     0,     0,   174,     0,
       0,     0,     0,     0,     0,     0,    86,     0,     0,     0,
       0,    77,     0,     0,   174,     0,     0,     0,     0,     0,
      87,    80,    81,     0,    89,     0,    82,    90,     0,     0,
      91,     0,     0,     0,   159,     0,   160,     0,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   173,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,     0,
       0,     0,     0,     0,    86,   173,     0,     0,     0,   351,
       0,     0,   174,     0,     0,     0,     0,     0,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,   174,     0,     0,     0,   173,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   174,   173,     0,
       0,   360,   361,     0,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,     0,
       0,     0,     0,   362,   363,     0,     0,   174,     0,   173,
       0,     0,     0,     0,     0,     0,     0,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,     0,     0,     0,     0,     0,   361,   174,     0,
       0,     0,     0,     0,     0,     0,     0,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   108,     0,     0,     0,     0,   363,     0,     0,
       0,     0,     0,     0,   109,   110,     0,     0,     0,     0,
       0,   111,     0,   173,   181,   182,   183,   184,   185,   186,
     187,   188,   189,     0,   112,     0,     0,     0,     0,     0,
       0,   113,     0,   173,     0,     0,     0,     0,     0,     0,
       0,     0,   174,   114,     0,     0,     0,     0,     0,     0,
       0,   115,   116,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   174,     0,   106,     0,     0,     0,   117,   173,
       0,     0,   118,     0,     0,     0,     0,     0,     0,     0,
       0,   119,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   173,   174,     0,
       0,   268,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,     0,     0,     0,
       0,   352,     0,     0,   173,     0,   174,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   174,   173,     0,     0,   357,     0,     0,
       0,     0,     0,     0,     0,     0,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,     0,     0,   174,     0,   364,     0,     0,     0,     0,
       0,     0,     0,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,     0,     0,
     173,     0,   473,     0,     0,   347,     0,     0,     0,     0,
       0,     0,     0,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,     1,   174,
       0,     2,   474,     3,     0,     0,     0,     0,     4,     0,
       0,     0,     5,     0,     0,     6,     0,     7,     8,     9,
       0,     0,    10,     0,     0,    11,     0,   173,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,     0,    13,   174,   173,     0,     0,
       0,     0,    14,     0,    15,     0,    16,     0,    17,     0,
      18,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,    20,   173,    21,     0,     0,   174,     0,     0,     0,
       0,     0,     0,     0,   173,     0,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   174,   173,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   174,     0,     0,     0,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   174,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,     0,     0,
       0,     0,     0,     0,     0,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189
};

static const short yycheck[] =
{
     132,   337,   377,   114,   326,     3,   117,   118,   386,     3,
       4,   389,     3,     5,     5,    16,     3,     8,   129,     5,
       5,     8,     8,     5,    54,   424,    87,    71,    32,     5,
       6,     3,     8,     5,     5,     6,    46,     8,    14,    33,
     219,    46,     9,    14,     5,     6,    63,    13,    14,     3,
      13,    14,     3,    14,    24,    39,    50,    95,     3,    53,
       5,     6,   194,     8,    18,   440,    55,    18,   129,    13,
      14,    65,     9,    43,    13,    14,     5,     6,     3,     8,
     124,   480,     3,   113,     5,    14,    87,    57,    89,    90,
     107,     5,     6,   468,     8,     5,     6,   101,   209,   107,
      14,     3,    19,   481,    14,    92,   484,   485,   486,   487,
     488,   489,   106,    64,     3,    79,    80,    21,   454,   129,
      37,   129,    86,    93,   129,    47,    71,    46,   120,   127,
     466,   453,    53,   127,   120,   120,   127,   121,   120,   271,
     141,   142,   128,   128,   120,   146,   128,   326,   120,   120,
     151,   127,    78,   154,   155,   156,   127,    61,    84,   120,
     127,   127,    64,   538,   127,    54,   127,   133,   169,   113,
     133,    47,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   133,
     127,   120,   193,   372,   133,    55,   574,   575,   110,   107,
     107,   312,   313,    85,   579,   337,   120,    89,   387,   129,
     120,   131,     3,     4,     5,     6,   128,     8,     9,    10,
     221,   129,   129,    14,    24,    25,    40,     3,    98,   565,
      21,     3,     8,    24,   556,     5,     6,     9,    84,     3,
       4,   242,    33,    43,   147,   424,   149,     3,   570,   152,
     153,     3,    43,     5,   386,    69,     8,   389,    58,    50,
      51,    16,    53,    54,   265,   266,   267,    58,   126,    33,
       0,   403,   128,   129,   453,    66,    59,    68,     3,    23,
       3,   413,   604,   605,   606,     3,    50,   127,   420,    53,
      42,     3,   614,   472,   128,   129,   128,   129,     3,     4,
      55,   480,     3,    94,     3,   119,   120,   121,   122,   123,
      54,     3,    67,    77,     3,   106,    60,   132,     3,    71,
      75,    76,   454,   128,   129,    10,   128,   129,    33,   120,
     121,   128,   129,   124,   466,   127,   127,    92,   127,   130,
     128,   129,   106,    28,   345,    50,   127,    99,    53,   481,
     105,    64,   484,   485,   486,   487,   488,   489,   605,   606,
     361,   127,   363,    48,   195,   196,    40,   127,   120,   113,
     114,   115,   116,   117,   118,   128,   129,   556,   127,   558,
     124,     3,     4,     5,     6,    70,     8,     9,   128,   129,
     127,   570,    14,   128,   129,    69,   128,   129,   127,    21,
     127,   106,    24,    88,   128,   129,   128,   129,   128,   129,
     127,    33,   127,     3,     4,     5,     6,   127,     8,     9,
     421,    43,   128,   129,    14,   604,   605,   606,    50,    51,
     113,    53,    54,   565,    24,   614,    58,   113,   113,    87,
      98,    87,   574,   575,    66,    63,    68,   121,   122,   123,
       3,   104,     3,    43,     3,    15,     3,     4,     5,     6,
      97,     8,     9,    53,    54,    82,     3,    14,    58,   128,
     113,   128,    94,   127,    21,   127,    66,    24,    68,     3,
       5,   113,   113,    61,   106,   120,    33,    92,    92,     8,
      92,   623,     3,     3,   127,    30,    43,    84,   120,   121,
     129,    53,   124,    50,    51,   127,    53,    54,   130,   134,
     113,    58,     3,   128,   127,   129,   106,     5,    62,    66,
       3,    68,     8,   103,   113,     3,     3,   128,    90,   128,
     120,     3,   128,    49,   124,    85,   113,   127,   128,     5,
     130,   113,   109,    72,   127,   127,   127,    94,   127,     3,
       4,     5,     6,     8,     8,     9,   129,     8,   127,   106,
      14,   129,    54,   128,   127,   110,     5,   108,     8,     3,
      24,    79,   128,   120,   121,   121,     3,   124,    73,     8,
     127,   129,    26,   130,    54,     3,     4,     5,     6,    43,
       8,     9,   128,    49,   128,    52,    14,   129,   113,    53,
      54,   128,    18,   129,    58,   128,    24,    26,   128,   110,
     110,    64,    66,    71,    68,     5,   129,     3,     4,     5,
       6,    73,     8,     9,    73,    43,    26,    26,    14,   127,
       3,    85,   129,   129,     8,    53,    54,   113,    24,     5,
      58,   128,   127,   371,   128,   127,   421,   242,    66,   322,
      68,   169,   106,     3,     4,     5,     6,    43,     8,     9,
     558,   614,   568,   562,    14,   607,   120,    53,    54,   585,
     124,   349,    58,   127,    24,   625,   130,   269,   420,     0,
      66,   514,    68,   310,   308,   387,   337,   466,   106,   439,
      18,    -1,    -1,    43,    -1,   143,    -1,    -1,   179,    -1,
      -1,    -1,   120,    53,    54,    -1,   124,    -1,    58,   127,
     128,    -1,   130,    -1,    -1,    -1,    66,    -1,    68,    40,
     106,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,   120,    40,    -1,    -1,   124,    -1,
      -1,   127,    24,    -1,   130,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
     120,    53,    54,    -1,   124,    -1,    58,   127,    -1,    -1,
     130,    -1,    -1,    -1,    66,    -1,    68,    -1,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    40,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,    -1,    -1,   106,    40,    -1,    -1,    -1,   134,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,   124,    -1,    -1,   127,    -1,    -1,   130,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    69,    40,    -1,
      -1,   128,   129,    -1,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,    -1,   128,   129,    -1,    -1,    69,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,    -1,    -1,    -1,   129,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    16,    -1,    -1,    -1,    -1,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    35,    -1,    40,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    88,    -1,    -1,    -1,    92,    40,
      -1,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    40,    69,    -1,
      -1,   128,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
      -1,   128,    -1,    -1,    40,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    69,    40,    -1,    -1,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    69,    -1,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
      40,    -1,   128,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    17,    69,
      -1,    20,   128,    22,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    31,    -1,    -1,    34,    -1,    36,    37,    38,
      -1,    -1,    41,    -1,    -1,    44,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    -1,    74,    69,    40,    -1,    -1,
      -1,    -1,    81,    -1,    83,    -1,    85,    -1,    87,    -1,
      89,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    40,   102,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    69,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      41,    44,    56,    74,    81,    83,    85,    87,    89,    91,
     100,   102,   136,   137,   138,   139,   140,   141,   148,   189,
     194,   195,   198,   199,   200,   201,   208,   209,   218,   219,
     220,   221,   222,   226,   227,   232,   233,   234,   237,   239,
     240,   241,   242,   243,   245,   246,   247,    95,    55,     3,
      47,    46,    47,    78,    84,    55,     3,     4,     5,     6,
       8,     9,    10,    14,    21,    24,    33,    43,    50,    51,
      53,    54,    58,    66,    68,    94,   106,   120,   121,   124,
     127,   130,   149,   150,   152,   182,   183,   244,   248,   251,
     252,     3,    10,    28,    48,    70,    88,   235,    16,    28,
      29,    35,    48,    55,    67,    75,    76,    92,    96,   105,
     191,   235,    98,    84,     3,   153,     0,   126,    59,     3,
       3,     3,   127,     3,   153,     3,     3,     3,     3,    13,
      14,   127,   133,   249,   250,   132,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,     3,    66,
      68,   182,   182,   182,     3,    54,   186,   187,    46,   129,
       3,    18,   151,    40,    69,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      64,   174,   175,   113,   113,   113,    87,     3,     9,     3,
       5,     6,     8,    71,   192,   193,    98,     3,     8,    92,
      87,    63,   190,     3,   190,   190,   190,   104,     3,    87,
     129,    85,    89,   139,     3,   190,    15,    97,     5,     6,
       8,    14,   120,   127,   161,   162,   207,   210,   211,    82,
     107,     8,   127,   128,   182,   184,   185,     8,   182,   250,
       3,   182,   182,   184,    39,   121,   184,   128,   182,   184,
     184,   182,   182,   182,   128,   127,   127,   127,   128,   129,
     131,   113,   127,   153,   150,   182,     3,   182,   182,   182,
     182,   182,   182,   182,   252,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,     5,   182,     3,     8,    42,
      71,    99,   120,   161,   196,   197,   193,   193,   113,   113,
     120,    61,    92,    92,   190,     8,    92,    63,   107,   228,
     229,     3,   223,   224,   248,     3,   127,   202,    30,    84,
       5,     6,     8,   128,   161,   163,   212,   129,   213,    24,
      43,    57,    93,   238,    53,    85,   141,    45,   128,   129,
     134,   134,   128,   128,     3,   128,   128,   128,   128,   128,
     128,   129,   128,   129,   128,   182,   182,   187,     3,   161,
     188,   145,   107,   154,   155,   129,   196,   127,   192,    62,
     190,   190,     8,     3,   230,   231,   113,   129,   155,   113,
       3,     4,    33,    50,    53,   106,   160,   203,   248,   103,
       3,     3,   128,   129,   128,   129,   207,   211,   214,   215,
     128,    90,    54,   113,   128,     3,   185,   128,   182,   182,
     113,    85,   148,    65,   127,   156,   157,   158,   159,   160,
      49,   164,     5,   163,    79,    80,    86,   236,   113,   109,
     127,   161,   162,   224,    72,   176,   177,   161,   162,   127,
     127,   127,   128,   129,   127,   204,   205,    24,    25,    43,
      58,   225,   161,     8,    18,   216,   129,     8,   127,   161,
     129,   142,    54,   128,   128,   188,   149,   128,   127,   156,
     110,    23,    54,    60,   113,   114,   115,   116,   117,   118,
     124,     5,   165,   108,   168,   169,   128,    32,   101,    79,
       8,   231,   128,   163,     3,   178,   179,   121,   128,   128,
     160,   127,   206,   207,   129,     3,    64,   217,   207,   215,
     163,     3,     5,    53,   143,   144,   128,     3,   248,    46,
      73,   146,     8,   128,   156,   161,   162,     9,   127,    71,
     124,     8,   161,   162,     8,   161,   162,   161,   162,   161,
     162,   161,   162,   161,   162,    54,    26,    49,    52,   167,
     128,   113,   129,   128,   128,   129,   205,   128,   129,   128,
      26,    64,   147,   128,   110,   110,   163,    71,     9,   127,
     160,   166,    73,   158,    73,   170,   171,     3,     5,     8,
     127,   179,   207,   144,   160,   172,   173,     5,   161,   162,
     161,   162,   128,   163,   129,    26,    26,   174,   127,     3,
     180,   181,    19,    37,   129,   129,   128,   160,   172,    77,
     172,   176,     8,   113,   128,   129,   173,     5,   127,   128,
     161,   181,   128
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

  case 29:

    { pParser->PushQuery(); ;}
    break;

  case 30:

    { pParser->PushQuery(); ;}
    break;

  case 34:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 36:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 39:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 40:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 44:

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

  case 45:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 47:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 48:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 49:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 52:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 55:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 56:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 59:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 60:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 61:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 62:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 63:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 64:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 65:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 67:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 74:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 76:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 77:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 78:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 79:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 80:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 95:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 96:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 97:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 98:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 99:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 102:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 103:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 104:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 105:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 106:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 108:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 109:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 110:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 111:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 112:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 113:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 114:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 118:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 119:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 120:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 122:

    {
			pParser->AddHaving();
		;}
    break;

  case 125:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 128:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 129:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 131:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 133:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 134:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 137:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 138:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 147:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 150:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 151:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 153:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 154:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 159:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 160:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 176:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 177:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 178:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 179:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 183:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 186:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 187:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 188:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 189:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 190:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 191:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 192:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 197:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 198:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 205:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 206:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 207:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 208:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 209:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 210:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 211:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 212:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 213:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 214:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 222:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 223:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 224:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 225:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 226:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 227:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 228:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 229:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 232:

    { yyval.m_iValue = 1; ;}
    break;

  case 233:

    { yyval.m_iValue = 0; ;}
    break;

  case 234:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 235:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 236:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 237:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 240:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 241:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 242:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 245:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 246:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 249:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 250:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 251:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 252:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 253:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 254:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 255:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 256:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 258:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 259:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 260:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 261:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 263:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 264:

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

  case 265:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 268:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 270:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 275:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 278:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 279:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 280:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 283:

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

  case 284:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 285:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 286:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 287:

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

  case 288:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 289:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 290:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 291:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 292:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 293:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 294:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 295:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 302:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 303:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 304:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 312:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 313:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 314:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 315:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 316:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 317:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 318:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 319:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 320:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 321:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 324:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 325:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 326:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 327:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 329:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 330:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 331:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 332:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 333:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 334:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 335:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
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

