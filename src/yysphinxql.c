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
     TOK_DOUBLE = 296,
     TOK_DROP = 297,
     TOK_FALSE = 298,
     TOK_FLOAT = 299,
     TOK_FLUSH = 300,
     TOK_FOR = 301,
     TOK_FROM = 302,
     TOK_FUNCTION = 303,
     TOK_GLOBAL = 304,
     TOK_GROUP = 305,
     TOK_GROUPBY = 306,
     TOK_GROUP_CONCAT = 307,
     TOK_HAVING = 308,
     TOK_ID = 309,
     TOK_IN = 310,
     TOK_INDEX = 311,
     TOK_INSERT = 312,
     TOK_INT = 313,
     TOK_INTEGER = 314,
     TOK_INTO = 315,
     TOK_IS = 316,
     TOK_ISOLATION = 317,
     TOK_LEVEL = 318,
     TOK_LIKE = 319,
     TOK_LIMIT = 320,
     TOK_MATCH = 321,
     TOK_MAX = 322,
     TOK_META = 323,
     TOK_MIN = 324,
     TOK_MOD = 325,
     TOK_NAMES = 326,
     TOK_NULL = 327,
     TOK_OPTION = 328,
     TOK_ORDER = 329,
     TOK_OPTIMIZE = 330,
     TOK_PLAN = 331,
     TOK_PROFILE = 332,
     TOK_RAND = 333,
     TOK_RAMCHUNK = 334,
     TOK_READ = 335,
     TOK_REPEATABLE = 336,
     TOK_REPLACE = 337,
     TOK_RETURNS = 338,
     TOK_ROLLBACK = 339,
     TOK_RTINDEX = 340,
     TOK_SELECT = 341,
     TOK_SERIALIZABLE = 342,
     TOK_SET = 343,
     TOK_SESSION = 344,
     TOK_SHOW = 345,
     TOK_SONAME = 346,
     TOK_START = 347,
     TOK_STATUS = 348,
     TOK_STRING = 349,
     TOK_SUM = 350,
     TOK_TABLE = 351,
     TOK_TABLES = 352,
     TOK_TO = 353,
     TOK_TRANSACTION = 354,
     TOK_TRUE = 355,
     TOK_TRUNCATE = 356,
     TOK_UNCOMMITTED = 357,
     TOK_UPDATE = 358,
     TOK_VALUES = 359,
     TOK_VARIABLES = 360,
     TOK_WARNINGS = 361,
     TOK_WEIGHT = 362,
     TOK_WHERE = 363,
     TOK_WITHIN = 364,
     TOK_OR = 365,
     TOK_AND = 366,
     TOK_NE = 367,
     TOK_GTE = 368,
     TOK_LTE = 369,
     TOK_NOT = 370,
     TOK_NEG = 371
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
#define TOK_DOUBLE 296
#define TOK_DROP 297
#define TOK_FALSE 298
#define TOK_FLOAT 299
#define TOK_FLUSH 300
#define TOK_FOR 301
#define TOK_FROM 302
#define TOK_FUNCTION 303
#define TOK_GLOBAL 304
#define TOK_GROUP 305
#define TOK_GROUPBY 306
#define TOK_GROUP_CONCAT 307
#define TOK_HAVING 308
#define TOK_ID 309
#define TOK_IN 310
#define TOK_INDEX 311
#define TOK_INSERT 312
#define TOK_INT 313
#define TOK_INTEGER 314
#define TOK_INTO 315
#define TOK_IS 316
#define TOK_ISOLATION 317
#define TOK_LEVEL 318
#define TOK_LIKE 319
#define TOK_LIMIT 320
#define TOK_MATCH 321
#define TOK_MAX 322
#define TOK_META 323
#define TOK_MIN 324
#define TOK_MOD 325
#define TOK_NAMES 326
#define TOK_NULL 327
#define TOK_OPTION 328
#define TOK_ORDER 329
#define TOK_OPTIMIZE 330
#define TOK_PLAN 331
#define TOK_PROFILE 332
#define TOK_RAND 333
#define TOK_RAMCHUNK 334
#define TOK_READ 335
#define TOK_REPEATABLE 336
#define TOK_REPLACE 337
#define TOK_RETURNS 338
#define TOK_ROLLBACK 339
#define TOK_RTINDEX 340
#define TOK_SELECT 341
#define TOK_SERIALIZABLE 342
#define TOK_SET 343
#define TOK_SESSION 344
#define TOK_SHOW 345
#define TOK_SONAME 346
#define TOK_START 347
#define TOK_STATUS 348
#define TOK_STRING 349
#define TOK_SUM 350
#define TOK_TABLE 351
#define TOK_TABLES 352
#define TOK_TO 353
#define TOK_TRANSACTION 354
#define TOK_TRUE 355
#define TOK_TRUNCATE 356
#define TOK_UNCOMMITTED 357
#define TOK_UPDATE 358
#define TOK_VALUES 359
#define TOK_VARIABLES 360
#define TOK_WARNINGS 361
#define TOK_WEIGHT 362
#define TOK_WHERE 363
#define TOK_WITHIN 364
#define TOK_OR 365
#define TOK_AND 366
#define TOK_NE 367
#define TOK_GTE 368
#define TOK_LTE 369
#define TOK_NOT 370
#define TOK_NEG 371




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
#define YYFINAL  127
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1415

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  136
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  118
/* YYNRULES -- Number of rules. */
#define YYNRULES  340
/* YYNRULES -- Number of states. */
#define YYNSTATES  649

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   371

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   124,   113,     2,
     128,   129,   122,   120,   130,   121,   133,   123,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   127,
     116,   114,   117,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   134,     2,   135,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   131,   112,   132,     2,     2,     2,     2,
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
     105,   106,   107,   108,   109,   110,   111,   115,   118,   119,
     125,   126
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
     330,   334,   336,   338,   343,   347,   351,   353,   355,   360,
     365,   370,   372,   375,   377,   380,   382,   384,   388,   389,
     394,   395,   397,   399,   403,   404,   407,   408,   410,   416,
     417,   419,   423,   429,   431,   435,   437,   440,   443,   444,
     446,   449,   454,   455,   457,   460,   462,   466,   470,   474,
     480,   487,   491,   493,   497,   501,   503,   505,   507,   509,
     511,   513,   515,   518,   521,   525,   529,   533,   537,   541,
     545,   549,   553,   557,   561,   565,   569,   573,   577,   581,
     585,   589,   593,   597,   599,   601,   603,   608,   613,   618,
     623,   628,   633,   637,   644,   651,   655,   664,   666,   670,
     672,   674,   678,   684,   686,   688,   690,   692,   695,   696,
     699,   701,   704,   707,   711,   713,   715,   720,   725,   729,
     731,   733,   735,   737,   739,   741,   745,   750,   755,   760,
     764,   769,   774,   782,   788,   790,   792,   794,   796,   798,
     800,   802,   804,   806,   809,   816,   818,   820,   821,   825,
     827,   831,   833,   837,   841,   843,   847,   849,   851,   853,
     857,   860,   868,   878,   885,   887,   891,   893,   897,   899,
     903,   904,   907,   909,   913,   917,   918,   920,   922,   924,
     928,   930,   932,   936,   940,   947,   949,   953,   957,   961,
     967,   972,   976,   980,   982,   984,   986,   988,   996,  1001,
    1007,  1008,  1010,  1013,  1015,  1019,  1023,  1026,  1030,  1037,
    1038,  1040,  1042,  1045,  1048,  1051,  1053,  1061,  1063,  1065,
    1067,  1069,  1073,  1080,  1084,  1088,  1092,  1094,  1098,  1101,
    1105,  1109,  1112,  1114,  1117,  1119,  1121,  1125,  1129,  1133,
    1137
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     137,     0,    -1,   138,    -1,   139,    -1,   139,   127,    -1,
     201,    -1,   209,    -1,   195,    -1,   196,    -1,   199,    -1,
     210,    -1,   219,    -1,   221,    -1,   222,    -1,   223,    -1,
     228,    -1,   233,    -1,   234,    -1,   238,    -1,   240,    -1,
     241,    -1,   242,    -1,   243,    -1,   235,    -1,   244,    -1,
     246,    -1,   247,    -1,   248,    -1,   227,    -1,   140,    -1,
     139,   127,   140,    -1,   141,    -1,   190,    -1,   142,    -1,
      86,     3,   128,   128,   142,   129,   143,   129,    -1,   149,
      -1,    86,   150,    47,   128,   146,   149,   129,   147,   148,
      -1,    -1,   130,   144,    -1,   145,    -1,   144,   130,   145,
      -1,     3,    -1,     5,    -1,    54,    -1,    -1,    74,    26,
     173,    -1,    -1,    65,     5,    -1,    65,     5,   130,     5,
      -1,    86,   150,    47,   154,   155,   165,   169,   168,   171,
     175,   177,    -1,   151,    -1,   150,   130,   151,    -1,   122,
      -1,   153,   152,    -1,    -1,     3,    -1,    18,     3,    -1,
     183,    -1,    21,   128,   183,   129,    -1,    67,   128,   183,
     129,    -1,    69,   128,   183,   129,    -1,    95,   128,   183,
     129,    -1,    52,   128,   183,   129,    -1,    33,   128,   122,
     129,    -1,    51,   128,   129,    -1,    33,   128,    39,     3,
     129,    -1,     3,    -1,   154,   130,     3,    -1,    -1,   156,
      -1,   108,   157,    -1,   158,    -1,   157,   111,   157,    -1,
     128,   157,   129,    -1,    66,   128,     8,   129,    -1,   159,
      -1,   161,   114,   162,    -1,   161,   115,   162,    -1,   161,
      55,   128,   164,   129,    -1,   161,   125,    55,   128,   164,
     129,    -1,   161,    55,     9,    -1,   161,   125,    55,     9,
      -1,   161,    23,   162,   111,   162,    -1,   161,   117,   162,
      -1,   161,   116,   162,    -1,   161,   118,   162,    -1,   161,
     119,   162,    -1,   161,   114,   163,    -1,   160,    -1,   161,
      23,   163,   111,   163,    -1,   161,    23,   162,   111,   163,
      -1,   161,    23,   163,   111,   162,    -1,   161,   117,   163,
      -1,   161,   116,   163,    -1,   161,   118,   163,    -1,   161,
     119,   163,    -1,   161,   114,     8,    -1,   161,   115,     8,
      -1,   161,    61,    72,    -1,   161,    61,   125,    72,    -1,
     161,   115,   163,    -1,     3,    -1,     4,    -1,    33,   128,
     122,   129,    -1,    51,   128,   129,    -1,   107,   128,   129,
      -1,    54,    -1,   249,    -1,    59,   128,   249,   129,    -1,
      41,   128,   249,   129,    -1,    24,   128,   249,   129,    -1,
       5,    -1,   121,     5,    -1,     6,    -1,   121,     6,    -1,
      14,    -1,   162,    -1,   164,   130,   162,    -1,    -1,    50,
     166,    26,   167,    -1,    -1,     5,    -1,   161,    -1,   167,
     130,   161,    -1,    -1,    53,   159,    -1,    -1,   170,    -1,
     109,    50,    74,    26,   173,    -1,    -1,   172,    -1,    74,
      26,   173,    -1,    74,    26,    78,   128,   129,    -1,   174,
      -1,   173,   130,   174,    -1,   161,    -1,   161,    19,    -1,
     161,    37,    -1,    -1,   176,    -1,    65,     5,    -1,    65,
       5,   130,     5,    -1,    -1,   178,    -1,    73,   179,    -1,
     180,    -1,   179,   130,   180,    -1,     3,   114,     3,    -1,
       3,   114,     5,    -1,     3,   114,   128,   181,   129,    -1,
       3,   114,     3,   128,     8,   129,    -1,     3,   114,     8,
      -1,   182,    -1,   181,   130,   182,    -1,     3,   114,   162,
      -1,     3,    -1,     4,    -1,    54,    -1,     5,    -1,     6,
      -1,    14,    -1,     9,    -1,   121,   183,    -1,   125,   183,
      -1,   183,   120,   183,    -1,   183,   121,   183,    -1,   183,
     122,   183,    -1,   183,   123,   183,    -1,   183,   116,   183,
      -1,   183,   117,   183,    -1,   183,   113,   183,    -1,   183,
     112,   183,    -1,   183,   124,   183,    -1,   183,    40,   183,
      -1,   183,    70,   183,    -1,   183,   119,   183,    -1,   183,
     118,   183,    -1,   183,   114,   183,    -1,   183,   115,   183,
      -1,   183,   111,   183,    -1,   183,   110,   183,    -1,   128,
     183,   129,    -1,   131,   187,   132,    -1,   184,    -1,   249,
      -1,   252,    -1,     3,   128,   185,   129,    -1,    55,   128,
     185,   129,    -1,    59,   128,   185,   129,    -1,    24,   128,
     185,   129,    -1,    44,   128,   185,   129,    -1,    41,   128,
     185,   129,    -1,     3,   128,   129,    -1,    69,   128,   183,
     130,   183,   129,    -1,    67,   128,   183,   130,   183,   129,
      -1,   107,   128,   129,    -1,     3,   128,   183,    46,     3,
      55,   249,   129,    -1,   186,    -1,   185,   130,   186,    -1,
     183,    -1,     8,    -1,   188,   114,   189,    -1,   187,   130,
     188,   114,   189,    -1,     3,    -1,    55,    -1,   162,    -1,
       3,    -1,    90,   192,    -1,    -1,    64,     8,    -1,   106,
      -1,    93,   191,    -1,    68,   191,    -1,    16,    93,   191,
      -1,    77,    -1,    76,    -1,    16,     8,    93,   191,    -1,
      16,     3,    93,   191,    -1,    56,     3,    93,    -1,     3,
      -1,    72,    -1,     8,    -1,     5,    -1,     6,    -1,   193,
      -1,   194,   121,   193,    -1,    88,     3,   114,   198,    -1,
      88,     3,   114,   197,    -1,    88,     3,   114,    72,    -1,
      88,    71,   194,    -1,    88,    10,   114,   194,    -1,    88,
      28,    88,   194,    -1,    88,    49,     9,   114,   128,   164,
     129,    -1,    88,    49,     3,   114,   197,    -1,     3,    -1,
       8,    -1,   100,    -1,    43,    -1,   162,    -1,    31,    -1,
      84,    -1,   200,    -1,    22,    -1,    92,    99,    -1,   202,
      60,     3,   203,   104,   205,    -1,    57,    -1,    82,    -1,
      -1,   128,   204,   129,    -1,   161,    -1,   204,   130,   161,
      -1,   206,    -1,   205,   130,   206,    -1,   128,   207,   129,
      -1,   208,    -1,   207,   130,   208,    -1,   162,    -1,   163,
      -1,     8,    -1,   128,   164,   129,    -1,   128,   129,    -1,
      36,    47,   154,   108,    54,   114,   162,    -1,    36,    47,
     154,   108,    54,    55,   128,   164,   129,    -1,    27,     3,
     128,   211,   214,   129,    -1,   212,    -1,   211,   130,   212,
      -1,   208,    -1,   128,   213,   129,    -1,     8,    -1,   213,
     130,     8,    -1,    -1,   130,   215,    -1,   216,    -1,   215,
     130,   216,    -1,   208,   217,   218,    -1,    -1,    18,    -1,
       3,    -1,    65,    -1,   220,     3,   191,    -1,    38,    -1,
      37,    -1,    90,    97,   191,    -1,    90,    35,   191,    -1,
     103,   154,    88,   224,   156,   177,    -1,   225,    -1,   224,
     130,   225,    -1,     3,   114,   162,    -1,     3,   114,   163,
      -1,     3,   114,   128,   164,   129,    -1,     3,   114,   128,
     129,    -1,   249,   114,   162,    -1,   249,   114,   163,    -1,
      59,    -1,    24,    -1,    44,    -1,    25,    -1,    17,    96,
       3,    15,    30,     3,   226,    -1,    90,   236,   105,   229,
      -1,    90,   236,   105,    64,     8,    -1,    -1,   230,    -1,
     108,   231,    -1,   232,    -1,   231,   110,   232,    -1,     3,
     114,     8,    -1,    90,    29,    -1,    90,    28,    88,    -1,
      88,   236,    99,    62,    63,   237,    -1,    -1,    49,    -1,
      89,    -1,    80,   102,    -1,    80,    32,    -1,    81,    80,
      -1,    87,    -1,    34,    48,     3,    83,   239,    91,     8,
      -1,    58,    -1,    24,    -1,    44,    -1,    94,    -1,    42,
      48,     3,    -1,    20,    56,     3,    98,    85,     3,    -1,
      45,    85,     3,    -1,    45,    79,     3,    -1,    86,   245,
     175,    -1,    10,    -1,    10,   133,     3,    -1,    86,   183,
      -1,   101,    85,     3,    -1,    75,    56,     3,    -1,     3,
     250,    -1,   251,    -1,   250,   251,    -1,    13,    -1,    14,
      -1,   134,   183,   135,    -1,   134,     8,   135,    -1,   183,
     114,   253,    -1,   253,   114,   183,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   162,   162,   163,   164,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   197,
     198,   202,   203,   207,   208,   216,   217,   224,   226,   230,
     234,   241,   242,   243,   247,   260,   267,   269,   274,   283,
     299,   300,   304,   305,   308,   310,   311,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   327,   328,   331,   333,
     337,   341,   342,   343,   347,   352,   356,   363,   371,   379,
     388,   393,   398,   403,   408,   413,   418,   423,   428,   433,
     438,   443,   448,   453,   458,   463,   468,   473,   478,   483,
     491,   495,   496,   501,   507,   513,   519,   525,   526,   527,
     528,   532,   533,   544,   545,   546,   550,   556,   562,   564,
     567,   569,   576,   580,   586,   588,   594,   596,   600,   611,
     613,   617,   621,   628,   629,   633,   634,   635,   638,   640,
     644,   649,   656,   658,   662,   666,   667,   671,   676,   681,
     687,   692,   700,   705,   712,   722,   723,   724,   725,   726,
     727,   728,   729,   730,   732,   733,   734,   735,   736,   737,
     738,   739,   740,   741,   742,   743,   744,   745,   746,   747,
     748,   749,   750,   751,   752,   753,   757,   758,   759,   760,
     761,   762,   763,   764,   765,   766,   767,   771,   772,   776,
     777,   781,   782,   786,   787,   791,   792,   798,   801,   803,
     807,   808,   809,   810,   811,   812,   813,   818,   823,   833,
     834,   835,   836,   837,   841,   842,   846,   851,   856,   861,
     862,   863,   867,   872,   880,   881,   885,   886,   887,   901,
     902,   903,   907,   908,   914,   922,   923,   926,   928,   932,
     933,   937,   938,   942,   946,   947,   951,   952,   953,   954,
     955,   961,   969,   983,   991,   995,  1002,  1003,  1010,  1020,
    1026,  1028,  1032,  1037,  1041,  1048,  1050,  1054,  1055,  1061,
    1069,  1070,  1076,  1080,  1086,  1094,  1095,  1099,  1113,  1119,
    1123,  1128,  1142,  1153,  1154,  1155,  1156,  1160,  1173,  1177,
    1184,  1185,  1189,  1193,  1194,  1198,  1202,  1209,  1216,  1222,
    1223,  1224,  1228,  1229,  1230,  1231,  1237,  1248,  1249,  1250,
    1251,  1255,  1266,  1278,  1287,  1298,  1306,  1307,  1311,  1321,
    1332,  1343,  1346,  1347,  1351,  1352,  1353,  1354,  1358,  1359,
    1363
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
  "TOK_DISTINCT", "TOK_DIV", "TOK_DOUBLE", "TOK_DROP", "TOK_FALSE", 
  "TOK_FLOAT", "TOK_FLUSH", "TOK_FOR", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", 
  "TOK_HAVING", "TOK_ID", "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", 
  "TOK_INTEGER", "TOK_INTO", "TOK_IS", "TOK_ISOLATION", "TOK_LEVEL", 
  "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", 
  "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", 
  "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", "TOK_TABLES", 
  "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", 
  "';'", "'('", "')'", "','", "'{'", "'}'", "'.'", "'['", "']'", 
  "$accept", "request", "statement", "multi_stmt_list", "multi_stmt", 
  "select", "select1", "opt_tablefunc_args", "tablefunc_args_list", 
  "tablefunc_arg", "subselect_start", "opt_outer_order", 
  "opt_outer_limit", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "filter_item", 
  "expr_float_unhandled", "expr_ident", "const_int", "const_float", 
  "const_list", "opt_group_clause", "opt_int", "group_items_list", 
  "opt_having_clause", "opt_group_order_clause", "group_order_clause", 
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
  "show_databases", "update", "update_items_list", "update_item", 
  "alter_col_type", "alter", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "show_character_set", 
  "set_transaction", "opt_scope", "isolation_level", "create_function", 
  "udf_type", "drop_function", "attach_index", "flush_rtindex", 
  "flush_ramchunk", "select_sysvar", "sysvar_name", "select_dual", 
  "truncate", "optimize_index", "json_field", "subscript", "subkey", 
  "streq", "strval", 0
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
     365,   366,   124,    38,    61,   367,    60,    62,   368,   369,
      43,    45,    42,    47,    37,   370,   371,    59,    40,    41,
      44,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   136,   137,   137,   137,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   139,
     139,   140,   140,   141,   141,   142,   142,   143,   143,   144,
     144,   145,   145,   145,   146,   147,   148,   148,   148,   149,
     150,   150,   151,   151,   152,   152,   152,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   154,   154,   155,   155,
     156,   157,   157,   157,   158,   158,   159,   159,   159,   159,
     159,   159,   159,   159,   159,   159,   159,   159,   159,   159,
     159,   159,   159,   159,   159,   159,   159,   159,   159,   159,
     160,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   162,   162,   163,   163,   163,   164,   164,   165,   165,
     166,   166,   167,   167,   168,   168,   169,   169,   170,   171,
     171,   172,   172,   173,   173,   174,   174,   174,   175,   175,
     176,   176,   177,   177,   178,   179,   179,   180,   180,   180,
     180,   180,   181,   181,   182,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   185,   185,   186,
     186,   187,   187,   188,   188,   189,   189,   190,   191,   191,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   193,
     193,   193,   193,   193,   194,   194,   195,   195,   195,   195,
     195,   195,   196,   196,   197,   197,   198,   198,   198,   199,
     199,   199,   200,   200,   201,   202,   202,   203,   203,   204,
     204,   205,   205,   206,   207,   207,   208,   208,   208,   208,
     208,   209,   209,   210,   211,   211,   212,   212,   213,   213,
     214,   214,   215,   215,   216,   217,   217,   218,   218,   219,
     220,   220,   221,   222,   223,   224,   224,   225,   225,   225,
     225,   225,   225,   226,   226,   226,   226,   227,   228,   228,
     229,   229,   230,   231,   231,   232,   233,   234,   235,   236,
     236,   236,   237,   237,   237,   237,   238,   239,   239,   239,
     239,   240,   241,   242,   243,   244,   245,   245,   246,   247,
     248,   249,   250,   250,   251,   251,   251,   251,   252,   252,
     253
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
       3,     1,     1,     4,     3,     3,     1,     1,     4,     4,
       4,     1,     2,     1,     2,     1,     1,     3,     0,     4,
       0,     1,     1,     3,     0,     2,     0,     1,     5,     0,
       1,     3,     5,     1,     3,     1,     2,     2,     0,     1,
       2,     4,     0,     1,     2,     1,     3,     3,     3,     5,
       6,     3,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     4,     4,     4,     4,
       4,     4,     3,     6,     6,     3,     8,     1,     3,     1,
       1,     3,     5,     1,     1,     1,     1,     2,     0,     2,
       1,     2,     2,     3,     1,     1,     4,     4,     3,     1,
       1,     1,     1,     1,     1,     3,     4,     4,     4,     3,
       4,     4,     7,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     6,     1,     1,     0,     3,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     1,     3,
       2,     7,     9,     6,     1,     3,     1,     3,     1,     3,
       0,     2,     1,     3,     3,     0,     1,     1,     1,     3,
       1,     1,     3,     3,     6,     1,     3,     3,     3,     5,
       4,     3,     3,     1,     1,     1,     1,     7,     4,     5,
       0,     1,     2,     1,     3,     3,     2,     3,     6,     0,
       1,     1,     2,     2,     2,     1,     7,     1,     1,     1,
       1,     3,     6,     3,     3,     3,     1,     3,     2,     3,
       3,     2,     1,     2,     1,     1,     3,     3,     3,     3,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   242,     0,   239,     0,     0,   281,   280,
       0,     0,   245,     0,   246,   240,     0,   309,   309,     0,
       0,     0,     0,     2,     3,    29,    31,    33,    35,    32,
       7,     8,     9,   241,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   155,   156,   158,   159,
     340,   161,   326,   160,     0,     0,     0,     0,     0,     0,
       0,   157,     0,     0,     0,     0,     0,     0,     0,    52,
       0,     0,     0,     0,    50,    54,    57,   183,   138,   184,
     185,     0,     0,     0,     0,   310,     0,   311,     0,     0,
       0,   306,   208,   310,     0,   208,   215,   214,   208,   208,
     210,   207,     0,   243,     0,    66,     0,     1,     4,     0,
     208,     0,     0,     0,     0,     0,   321,   324,   323,   330,
     334,   335,     0,     0,   331,   332,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     155,     0,     0,   162,   163,     0,   203,   204,     0,     0,
       0,     0,    55,     0,    53,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   325,   139,     0,     0,     0,     0,     0,
       0,   219,   222,   223,   221,   220,   224,   229,     0,     0,
       0,   208,   307,     0,   283,     0,   212,   211,   282,   300,
     329,     0,     0,     0,     0,    30,   247,   279,     0,     0,
     111,   113,   258,   115,     0,     0,   256,   257,   266,   270,
     264,     0,     0,   200,     0,   192,   199,     0,   197,   340,
       0,   333,   327,     0,   199,     0,     0,     0,     0,     0,
      64,     0,     0,     0,     0,     0,     0,   195,     0,     0,
       0,   181,     0,   182,     0,    44,    68,    51,    57,    56,
     173,   174,   180,   179,   171,   170,   177,   338,   178,   168,
     169,   176,   175,   164,   165,   166,   167,   172,   140,   339,
     234,   235,   237,   228,   236,     0,   238,   227,   226,   230,
     231,     0,     0,     0,     0,   208,   208,   213,   209,   218,
       0,     0,   298,   301,     0,     0,   285,     0,    67,     0,
       0,     0,     0,   112,   114,   268,   260,   116,     0,     0,
       0,     0,   318,   319,   317,   320,     0,     0,     0,     0,
       0,   186,     0,   337,   336,    58,   189,     0,    63,   191,
     190,    62,   187,   188,    59,     0,    60,     0,    61,     0,
       0,     0,   206,   205,   201,     0,     0,   118,    69,     0,
     233,     0,   225,     0,   217,   216,   299,     0,   302,   303,
       0,     0,   142,     0,   101,   102,     0,     0,     0,     0,
     106,     0,     0,   249,     0,   107,     0,     0,   322,   259,
       0,   267,     0,   266,   265,   271,   272,   263,     0,     0,
       0,    37,     0,   198,    65,     0,     0,     0,     0,     0,
       0,     0,    70,    71,    75,    88,     0,   120,   126,   141,
       0,     0,     0,   315,   308,     0,     0,     0,   287,   288,
     286,     0,   284,   143,   291,   292,     0,     0,     0,     0,
       0,     0,   248,     0,     0,   244,   251,   294,   296,   295,
     293,   297,   117,   269,   276,     0,     0,   316,     0,   261,
       0,     0,     0,   194,   193,   202,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   121,     0,     0,   124,   127,   232,   313,   312,   314,
     305,   304,   290,     0,     0,   144,   145,     0,     0,     0,
       0,   104,     0,   105,   250,     0,     0,   254,     0,   277,
     278,   274,   275,   273,     0,    41,    42,    43,    38,    39,
      34,     0,     0,     0,    46,     0,    73,    72,     0,     0,
      80,     0,    98,     0,    96,    76,    87,    97,    77,   100,
      84,    93,    83,    92,    85,    94,    86,    95,     0,     0,
       0,     0,   129,   289,     0,     0,   110,   103,   109,   108,
     253,     0,   252,   262,     0,   196,     0,     0,    36,    74,
       0,     0,     0,    99,    81,     0,   122,   119,     0,   125,
       0,   138,   130,   147,   148,   151,     0,   146,   255,    40,
     135,    45,   133,    47,    82,    90,    91,    89,    78,     0,
       0,     0,     0,   142,     0,     0,     0,   152,   136,   137,
       0,     0,    79,   123,   128,     0,   131,    49,     0,     0,
     149,     0,   134,    48,     0,   150,   154,   153,   132
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   481,   538,   539,
     375,   544,   588,    28,    93,    94,   174,    95,   276,   377,
     378,   432,   433,   434,   435,   436,   337,   237,   338,   438,
     502,   597,   572,   504,   505,   601,   602,   611,   612,   193,
     194,   452,   453,   515,   516,   626,   627,   254,    97,   247,
     248,   168,   169,   374,    29,   214,   121,   206,   207,    30,
      31,   307,   308,    32,    33,    34,    35,   330,   404,   465,
     466,   526,   238,    36,    37,   239,   240,   339,   341,   415,
     416,   475,   531,    38,    39,    40,    41,    42,   325,   326,
     471,    43,    44,   322,   323,   388,   389,    45,    46,    47,
     108,   444,    48,   346,    49,    50,    51,    52,    53,    98,
      54,    55,    56,    99,   144,   145,   100,   101
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -416
static const short yypact[] =
{
    1221,   -37,   -33,  -416,    67,  -416,    41,    51,  -416,  -416,
     191,   -25,  -416,    49,  -416,  -416,    28,   286,   975,   145,
     163,   249,   264,  -416,   151,  -416,  -416,  -416,  -416,  -416,
    -416,  -416,  -416,  -416,  -416,   241,  -416,  -416,  -416,   302,
    -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,
    -416,  -416,  -416,  -416,  -416,  -416,  -416,   307,   310,   144,
     312,   249,   330,   347,   349,   351,    93,  -416,  -416,  -416,
    -416,  -416,   250,  -416,   243,   245,   290,   291,   293,   294,
     298,  -416,   301,   304,   306,   308,   309,   313,   672,  -416,
     672,   672,    45,   -39,  -416,   118,   206,  -416,   373,  -416,
    -416,   328,   332,   333,   356,   240,    50,  -416,   350,    38,
     362,  -416,   387,  -416,   449,   387,  -416,  -416,   387,   387,
    -416,  -416,   348,  -416,   451,  -416,    95,  -416,   110,   452,
     387,   441,   361,    80,   382,   -73,  -416,  -416,  -416,  -416,
    -416,  -416,   515,   701,    53,  -416,   465,   672,   758,   -30,
     758,   758,   341,   672,   758,   758,   672,   672,   672,   344,
     141,   346,   352,  -416,  -416,   972,  -416,  -416,    -6,   364,
       2,   376,  -416,   472,  -416,   672,   672,   672,   672,   672,
     672,   672,   672,   672,   672,   672,   672,   672,   672,   672,
     672,   672,   471,  -416,  -416,   672,   190,    50,    50,   367,
     370,  -416,  -416,  -416,  -416,  -416,  -416,   365,   423,   395,
     396,   387,  -416,   485,  -416,   399,  -416,  -416,  -416,   -43,
    -416,   491,   493,   458,   484,  -416,   375,  -416,   475,   426,
    -416,  -416,  -416,  -416,   207,    17,  -416,  -416,  -416,   378,
    -416,   192,   461,   402,   593,  -416,  1169,   -79,  -416,   391,
     725,  -416,  -416,   992,  1224,   152,   519,   401,   154,   162,
    -416,  1027,   178,   210,   780,   811,  1058,  -416,   615,   672,
     672,  -416,    45,  -416,    68,  -416,     4,  -416,  1224,  -416,
    -416,  -416,  1255,  1279,  1291,   899,   492,  -416,   492,   225,
     225,   225,   225,   175,   175,  -416,  -416,  -416,   398,   492,
    -416,  -416,  -416,  -416,  -416,   526,  -416,  -416,  -416,   365,
     365,   189,   405,    50,   473,   387,   387,  -416,  -416,  -416,
     527,   531,  -416,  -416,    13,   150,  -416,   427,  -416,   534,
     438,   541,   542,  -416,  -416,  -416,  -416,  -416,   212,   226,
      80,   417,  -416,  -416,  -416,  -416,   456,   -11,   376,   419,
     546,  -416,   758,  -416,  -416,  -416,  -416,   421,  -416,  -416,
    -416,  -416,  -416,  -416,  -416,   672,  -416,   672,  -416,   835,
     866,   437,  -416,  -416,  -416,   468,   181,   505,  -416,   552,
    -416,    -1,  -416,   289,  -416,  -416,  -416,   450,   453,  -416,
     146,   491,   495,   185,    53,  -416,   443,   444,   445,   448,
    -416,   459,   463,  -416,   230,  -416,   464,   194,  -416,  -416,
      -1,  -416,   558,    25,  -416,   470,  -416,  -416,   570,   466,
      -1,   474,   540,  -416,  -416,  1082,  1113,    68,   376,   476,
     478,   181,   514,  -416,  -416,  -416,   277,   576,   494,  -416,
     238,   -18,   547,  -416,  -416,   618,   531,    57,  -416,  -416,
    -416,   625,  -416,  -416,  -416,  -416,   627,   509,   627,   503,
     627,   504,  -416,   534,    96,   508,  -416,  -416,  -416,  -416,
    -416,  -416,  -416,  -416,  -416,    16,    96,  -416,    -1,  -416,
      42,   506,   627,  -416,  -416,  -416,   -17,   568,   637,    91,
     181,   185,    20,   -32,   223,   265,   185,   185,   185,   185,
     594,  -416,   624,   601,   600,  -416,  -416,  -416,  -416,  -416,
    -416,  -416,  -416,   248,   543,   524,  -416,    53,   529,   532,
     535,  -416,   536,  -416,  -416,    59,   258,  -416,   464,  -416,
    -416,  -416,   645,  -416,   269,  -416,  -416,  -416,   525,  -416,
    -416,   537,   249,   641,   603,   544,  -416,  -416,   560,   561,
    -416,    -1,  -416,   611,  -416,  -416,  -416,  -416,  -416,  -416,
    -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,    30,   534,
     613,   534,   614,  -416,    12,   625,  -416,  -416,  -416,  -416,
    -416,    96,  -416,  -416,    42,  -416,   534,   680,  -416,  -416,
     185,   185,   276,  -416,  -416,    -1,  -416,   559,   664,  -416,
     665,   373,  -416,   564,  -416,  -416,   690,  -416,  -416,  -416,
     204,   565,  -416,   567,  -416,  -416,  -416,  -416,  -416,   281,
     534,   534,   436,   495,   686,   584,   283,  -416,  -416,  -416,
     534,   694,  -416,  -416,   565,   573,   565,  -416,   574,    -1,
    -416,   690,  -416,  -416,   579,  -416,  -416,  -416,  -416
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -416,  -416,  -416,  -416,   583,  -416,   479,  -416,  -416,   128,
    -416,  -416,  -416,   327,   292,   548,  -416,  -416,    47,  -416,
     392,  -415,  -416,   157,  -416,  -318,  -133,  -380,  -369,  -416,
    -416,  -416,  -416,  -416,  -416,  -416,  -416,  -285,    99,   129,
    -416,   109,  -416,  -416,   158,  -416,    94,   -14,  -416,   253,
     385,  -416,   462,   311,  -416,  -112,  -416,   434,   218,  -416,
    -416,   439,  -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,
     220,  -416,  -339,  -416,  -416,  -416,   409,  -416,  -416,  -416,
     275,  -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,   363,
    -416,  -416,  -416,  -416,  -416,  -416,   323,  -416,  -416,  -416,
     734,  -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,  -416,
    -416,  -416,  -416,  -197,  -416,   609,  -416,   577
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -341
static const short yytable[] =
{
     236,   413,    96,   216,   230,   125,   217,   218,   170,   256,
     449,   403,   440,   455,   507,   603,   489,   604,   227,   529,
     605,   320,   230,    58,   327,   335,   140,   141,  -275,   550,
     542,    66,    67,    68,    69,   242,    70,    71,    72,   594,
     552,   209,    73,   474,   419,   535,   210,   536,   166,    74,
     351,   352,    75,   201,    63,   202,   203,   222,   204,    57,
      64,    76,   230,   306,   230,   321,   140,   141,   126,    77,
      59,   372,    78,   230,   163,   547,   164,   165,   513,    79,
      80,   530,    81,    82,   508,   230,   231,    83,   232,    60,
    -275,   171,   257,   553,   233,    84,   537,    85,    61,   317,
     167,   230,   231,   420,   232,    65,   140,   141,   135,   534,
     233,   549,   376,   171,   556,   559,   561,   563,   565,   567,
     305,   172,   205,    86,   272,   527,   273,   390,   246,   250,
     275,   211,   405,   253,   222,    87,   173,   532,   305,   261,
     606,   373,   264,   265,   266,   524,   336,   143,   551,    88,
      89,   230,   231,    90,   140,   141,    91,   278,   595,    92,
     233,   280,   281,   282,   283,   284,   285,   286,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   305,   405,
     305,   299,   592,   221,   394,   395,   512,   143,   336,   305,
     230,   231,   300,   300,   327,   230,   223,   301,   301,   233,
     224,   234,   490,   384,   385,   396,  -328,   236,   235,   278,
     615,   617,   333,   334,   397,   175,   342,   234,   467,   468,
     546,   142,   398,   628,   525,   222,   619,   143,   230,   231,
     165,   554,   399,   302,   405,   400,   343,   233,   469,    62,
     401,   629,   608,   199,   123,   176,   175,   430,   124,   200,
     344,   596,   125,   470,   246,   369,   370,   448,   376,   518,
     454,   520,   303,   522,   127,   175,   405,   234,   610,   268,
     230,   231,   133,   557,   447,   143,   176,   472,   128,   233,
     391,   356,   352,   359,   352,   541,   345,   479,   402,   102,
     304,   360,   352,   405,   373,   176,   103,   189,   190,   191,
     491,   129,   633,   610,   610,   130,   234,   362,   352,   431,
     131,   305,   610,   132,   104,   134,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   236,   492,   136,   278,   105,   634,   636,   493,   363,
     352,   409,   410,   236,   234,   187,   188,   189,   190,   191,
     137,   425,   138,   426,   139,   411,   412,   106,   548,   462,
     463,   555,   558,   560,   562,   564,   566,   506,   410,   441,
     442,   147,   405,   148,   405,   107,   443,   573,   410,   160,
      67,    68,    69,   146,    70,    71,   234,   580,   581,   405,
      73,   494,   495,   496,   497,   498,   499,    74,   583,   410,
      75,   255,   500,   258,   259,   618,   410,   262,   263,    76,
     632,   410,   640,   641,   278,   309,   310,    77,   149,   150,
      78,   151,   152,   405,   405,   405,   153,    79,    80,   154,
      81,    82,   155,   405,   156,    83,   157,   158,   192,   394,
     395,   159,   195,    84,   198,    85,   196,   197,   236,   208,
     212,   213,   215,   219,   220,   226,   228,   614,   616,   229,
     396,    66,    67,    68,    69,   241,    70,    71,   252,   397,
     260,    86,    73,   267,   269,   279,   298,   398,   274,    74,
     270,   311,    75,    87,   312,   314,   313,   399,   315,   316,
     400,    76,   319,   318,   324,   401,   328,    88,    89,    77,
     109,    90,    78,   329,    91,   331,   646,    92,   340,    79,
      80,   332,    81,    82,   635,   347,  -340,    83,   160,    67,
      68,    69,   357,   243,    71,    84,   353,    85,   379,    73,
     358,   333,   175,   381,   387,   386,   383,   394,   395,    75,
     114,   393,   406,   402,   407,   408,   417,   418,   421,   422,
     424,   427,   115,    86,   428,   437,    77,   439,   396,    78,
     116,   117,   176,   446,   445,    87,   473,   397,   451,    81,
      82,   456,   457,   458,    83,   398,   459,   118,   477,    88,
      89,   501,   161,    90,   162,   399,    91,   460,   400,    92,
     120,   461,   464,   401,   478,   482,   160,    67,    68,    69,
     476,    70,    71,   503,   480,   487,   488,    73,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    75,   160,    67,
      68,    69,    87,   243,    71,   490,   510,   509,   514,    73,
     517,   519,   521,   523,    77,   540,    88,    78,   528,    75,
      90,   402,   543,   244,   245,   545,    92,    81,    82,   568,
     569,   570,    83,   571,   575,   584,    77,   574,   576,    78,
     161,   577,   162,   474,   578,   579,   585,   586,   587,    81,
      82,   590,   591,   589,    83,   160,    67,    68,    69,   348,
      70,    71,   161,   593,   162,   613,    73,   598,   600,   620,
     621,   622,   624,   625,   638,   630,    75,   631,   639,   643,
      87,   644,   429,   645,   160,    67,    68,    69,   648,   249,
      71,   225,   609,    77,    88,    73,    78,   392,    90,   277,
     486,    91,    87,   349,    92,    75,    81,    82,   599,   642,
     623,    83,   637,   607,   371,   647,    88,   423,   485,   161,
      90,   162,    77,    91,   245,    78,    92,   382,   582,   414,
     380,   533,   122,   251,   450,    81,    82,     0,   287,     0,
      83,   160,    67,    68,    69,   175,   243,    71,   161,   511,
     162,     0,    73,     0,     0,     0,     0,     0,     0,    87,
       0,     0,    75,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    88,     0,   176,     0,    90,     0,    77,
      91,     0,    78,    92,     0,     0,     0,     0,    87,     0,
       0,     0,    81,    82,     0,     0,     0,    83,     0,     0,
     175,     0,    88,     0,     0,   161,    90,   162,     0,    91,
       0,     0,    92,     0,     0,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     176,   175,     0,     0,     0,     0,     0,     0,     0,     0,
     354,     0,     0,     0,     0,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   175,     0,     0,     0,    88,
       0,   176,     0,    90,     0,     0,    91,     0,     0,    92,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   176,   175,     0,     0,   364,
     365,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   176,     0,     0,   175,
     366,   367,     0,     0,     0,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
       0,     0,     0,     0,     0,   365,     0,     0,     0,   176,
       0,     0,     0,     0,     0,     0,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   109,     0,     0,     0,     0,   367,     0,     0,     0,
       0,     0,     0,   110,   111,     0,     0,     0,     0,     0,
     112,     0,   175,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   113,     0,     0,     0,     0,     0,
       0,   114,   175,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   176,   115,     0,     0,     0,     0,     0,     0,
       0,   116,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   176,     0,   107,     0,     0,   175,   118,     0,
       0,     0,   119,     0,     0,     0,     0,     0,     0,     0,
       0,   120,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   176,   175,     0,
       0,   271,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,     0,     0,     0,
       0,   355,   175,     0,     0,     0,     0,     0,   176,     0,
       0,     0,     0,     0,     0,     0,     0,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   176,   175,     0,     0,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   176,     0,     0,     0,   368,     0,     0,
       0,     0,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,     0,     0,   175,
       0,   483,     0,     0,     0,   350,     0,     0,     0,     0,
       0,     0,     0,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,     1,   176,
       0,     2,   484,     3,     0,     0,     0,     0,     4,     0,
       0,     0,     5,     0,     0,     6,     0,     7,     8,     9,
       0,     0,     0,    10,   175,     0,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   176,   175,    13,     0,     0,     0,
       0,     0,     0,    14,     0,    15,     0,    16,     0,    17,
       0,    18,     0,    19,     0,     0,     0,     0,     0,   175,
       0,     0,    20,     0,    21,   176,     0,     0,     0,     0,
       0,   175,     0,     0,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   176,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   176,     0,     0,     0,     0,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191
};

static const short yycheck[] =
{
     133,   340,    16,   115,     5,     3,   118,   119,    47,    39,
     390,   329,   381,   393,    32,     3,   431,     5,   130,     3,
       8,    64,     5,    56,   221,     8,    13,    14,     3,     9,
      47,     3,     4,     5,     6,   108,     8,     9,    10,     9,
      72,     3,    14,    18,    55,     3,     8,     5,     3,    21,
     129,   130,    24,     3,    79,     5,     6,   130,     8,    96,
      85,    33,     5,   196,     5,   108,    13,    14,    21,    41,
       3,     3,    44,     5,    88,   490,    90,    91,   447,    51,
      52,    65,    54,    55,   102,     5,     6,    59,     8,    48,
      65,   130,   122,   125,    14,    67,    54,    69,    47,   211,
      55,     5,     6,   114,     8,    56,    13,    14,    61,   478,
      14,   491,   108,   130,   494,   495,   496,   497,   498,   499,
     121,     3,    72,    95,   130,   464,   132,   114,   142,   143,
     128,    93,   329,   147,   130,   107,    18,   476,   121,   153,
     128,   274,   156,   157,   158,   463,   129,   134,   128,   121,
     122,     5,     6,   125,    13,    14,   128,   171,   128,   131,
      14,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   121,   376,
     121,   195,   551,    88,     3,     4,   129,   134,   129,   121,
       5,     6,     3,     3,   391,     5,    86,     8,     8,    14,
      90,   121,   111,   315,   316,    24,     0,   340,   128,   223,
     590,   591,     5,     6,    33,    40,    24,   121,    24,    25,
     129,   128,    41,    19,   128,   130,   595,   134,     5,     6,
     244,     8,    51,    43,   431,    54,    44,    14,    44,    48,
      59,    37,   581,     3,    99,    70,    40,    66,    85,     9,
      58,   569,     3,    59,   268,   269,   270,   390,   108,   456,
     393,   458,    72,   460,     0,    40,   463,   121,   586,   128,
       5,     6,   128,     8,   128,   134,    70,   410,   127,    14,
     130,   129,   130,   129,   130,   482,    94,   420,   107,     3,
     100,   129,   130,   490,   427,    70,    10,   122,   123,   124,
      23,    60,   620,   621,   622,     3,   121,   129,   130,   128,
       3,   121,   630,     3,    28,     3,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   464,    55,     3,   348,    49,   621,   622,    61,   129,
     130,   129,   130,   476,   121,   120,   121,   122,   123,   124,
       3,   365,     3,   367,     3,   129,   130,    71,   491,   129,
     130,   494,   495,   496,   497,   498,   499,   129,   130,    80,
      81,   128,   569,   128,   571,    89,    87,   129,   130,     3,
       4,     5,     6,   133,     8,     9,   121,   129,   130,   586,
      14,   114,   115,   116,   117,   118,   119,    21,   129,   130,
      24,   148,   125,   150,   151,   129,   130,   154,   155,    33,
     129,   130,   129,   130,   428,   197,   198,    41,   128,   128,
      44,   128,   128,   620,   621,   622,   128,    51,    52,   128,
      54,    55,   128,   630,   128,    59,   128,   128,    65,     3,
       4,   128,   114,    67,    88,    69,   114,   114,   581,    99,
      88,    64,     3,   105,     3,     3,    15,   590,   591,    98,
      24,     3,     4,     5,     6,    83,     8,     9,     3,    33,
     129,    95,    14,   129,   128,     3,     5,    41,   114,    21,
     128,   114,    24,   107,   114,    62,   121,    51,    93,    93,
      54,    33,    93,     8,     3,    59,     3,   121,   122,    41,
      16,   125,    44,   128,   128,    30,   639,   131,   130,    51,
      52,    85,    54,    55,    78,    54,   114,    59,     3,     4,
       5,     6,     3,     8,     9,    67,   135,    69,   130,    14,
     129,     5,    40,   128,     3,     8,    63,     3,     4,    24,
      56,   114,   104,   107,     3,     3,   129,    91,   129,     3,
     129,   114,    68,    95,    86,    50,    41,     5,    24,    44,
      76,    77,    70,   110,   114,   107,     8,    33,    73,    54,
      55,   128,   128,   128,    59,    41,   128,    93,     8,   121,
     122,     5,    67,   125,    69,    51,   128,   128,    54,   131,
     106,   128,   128,    59,   128,    55,     3,     4,     5,     6,
     130,     8,     9,   109,   130,   129,   128,    14,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    24,     3,     4,
       5,     6,   107,     8,     9,   111,     8,    80,     3,    14,
       3,   122,   129,   129,    41,   129,   121,    44,   130,    24,
     125,   107,    74,   128,   129,     8,   131,    54,    55,    55,
      26,    50,    59,    53,   130,   130,    41,   114,   129,    44,
      67,   129,    69,    18,   129,   129,   129,    26,    65,    54,
      55,   111,   111,   129,    59,     3,     4,     5,     6,    86,
       8,     9,    67,    72,    69,     5,    14,    74,    74,   130,
      26,    26,   128,     3,     8,   130,    24,   130,   114,     5,
     107,   128,   375,   129,     3,     4,     5,     6,   129,     8,
       9,   128,   584,    41,   121,    14,    44,   325,   125,   171,
     428,   128,   107,   244,   131,    24,    54,    55,   571,   630,
     601,    59,   623,   575,   272,   641,   121,   352,   427,    67,
     125,    69,    41,   128,   129,    44,   131,   313,   528,   340,
     311,   476,    18,   144,   391,    54,    55,    -1,   181,    -1,
      59,     3,     4,     5,     6,    40,     8,     9,    67,   446,
      69,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,   107,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    70,    -1,   125,    -1,    41,
     128,    -1,    44,   131,    -1,    -1,    -1,    -1,   107,    -1,
      -1,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,
      40,    -1,   121,    -1,    -1,    67,   125,    69,    -1,   128,
      -1,    -1,   131,    -1,    -1,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      70,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     135,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,   121,
      -1,    70,    -1,   125,    -1,    -1,   128,    -1,    -1,   131,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    70,    40,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,    70,    -1,    -1,    40,
     129,   130,    -1,    -1,    -1,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,    16,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
      35,    -1,    40,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    -1,    89,    -1,    -1,    40,    93,    -1,
      -1,    -1,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    70,    40,    -1,
      -1,   129,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
      -1,   129,    40,    -1,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    70,    40,    -1,    -1,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    70,    -1,    -1,    -1,   129,    -1,    -1,
      -1,    -1,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,    -1,    40,
      -1,   129,    -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,    17,    70,
      -1,    20,   129,    22,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    31,    -1,    -1,    34,    -1,    36,    37,    38,
      -1,    -1,    -1,    42,    40,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    70,    40,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    92,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,   101,    -1,   103,    70,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    -1,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    75,    82,    84,    86,    88,    90,    92,
     101,   103,   137,   138,   139,   140,   141,   142,   149,   190,
     195,   196,   199,   200,   201,   202,   209,   210,   219,   220,
     221,   222,   223,   227,   228,   233,   234,   235,   238,   240,
     241,   242,   243,   244,   246,   247,   248,    96,    56,     3,
      48,    47,    48,    79,    85,    56,     3,     4,     5,     6,
       8,     9,    10,    14,    21,    24,    33,    41,    44,    51,
      52,    54,    55,    59,    67,    69,    95,   107,   121,   122,
     125,   128,   131,   150,   151,   153,   183,   184,   245,   249,
     252,   253,     3,    10,    28,    49,    71,    89,   236,    16,
      28,    29,    35,    49,    56,    68,    76,    77,    93,    97,
     106,   192,   236,    99,    85,     3,   154,     0,   127,    60,
       3,     3,     3,   128,     3,   154,     3,     3,     3,     3,
      13,    14,   128,   134,   250,   251,   133,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
       3,    67,    69,   183,   183,   183,     3,    55,   187,   188,
      47,   130,     3,    18,   152,    40,    70,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    65,   175,   176,   114,   114,   114,    88,     3,
       9,     3,     5,     6,     8,    72,   193,   194,    99,     3,
       8,    93,    88,    64,   191,     3,   191,   191,   191,   105,
       3,    88,   130,    86,    90,   140,     3,   191,    15,    98,
       5,     6,     8,    14,   121,   128,   162,   163,   208,   211,
     212,    83,   108,     8,   128,   129,   183,   185,   186,     8,
     183,   251,     3,   183,   183,   185,    39,   122,   185,   185,
     129,   183,   185,   185,   183,   183,   183,   129,   128,   128,
     128,   129,   130,   132,   114,   128,   154,   151,   183,     3,
     183,   183,   183,   183,   183,   183,   183,   253,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,     5,   183,
       3,     8,    43,    72,   100,   121,   162,   197,   198,   194,
     194,   114,   114,   121,    62,    93,    93,   191,     8,    93,
      64,   108,   229,   230,     3,   224,   225,   249,     3,   128,
     203,    30,    85,     5,     6,     8,   129,   162,   164,   213,
     130,   214,    24,    44,    58,    94,   239,    54,    86,   142,
      46,   129,   130,   135,   135,   129,   129,     3,   129,   129,
     129,   129,   129,   129,   129,   130,   129,   130,   129,   183,
     183,   188,     3,   162,   189,   146,   108,   155,   156,   130,
     197,   128,   193,    63,   191,   191,     8,     3,   231,   232,
     114,   130,   156,   114,     3,     4,    24,    33,    41,    51,
      54,    59,   107,   161,   204,   249,   104,     3,     3,   129,
     130,   129,   130,   208,   212,   215,   216,   129,    91,    55,
     114,   129,     3,   186,   129,   183,   183,   114,    86,   149,
      66,   128,   157,   158,   159,   160,   161,    50,   165,     5,
     164,    80,    81,    87,   237,   114,   110,   128,   162,   163,
     225,    73,   177,   178,   162,   163,   128,   128,   128,   128,
     128,   128,   129,   130,   128,   205,   206,    24,    25,    44,
      59,   226,   162,     8,    18,   217,   130,     8,   128,   162,
     130,   143,    55,   129,   129,   189,   150,   129,   128,   157,
     111,    23,    55,    61,   114,   115,   116,   117,   118,   119,
     125,     5,   166,   109,   169,   170,   129,    32,   102,    80,
       8,   232,   129,   164,     3,   179,   180,     3,   249,   122,
     249,   129,   249,   129,   161,   128,   207,   208,   130,     3,
      65,   218,   208,   216,   164,     3,     5,    54,   144,   145,
     129,   249,    47,    74,   147,     8,   129,   157,   162,   163,
       9,   128,    72,   125,     8,   162,   163,     8,   162,   163,
     162,   163,   162,   163,   162,   163,   162,   163,    55,    26,
      50,    53,   168,   129,   114,   130,   129,   129,   129,   129,
     129,   130,   206,   129,   130,   129,    26,    65,   148,   129,
     111,   111,   164,    72,     9,   128,   161,   167,    74,   159,
      74,   171,   172,     3,     5,     8,   128,   180,   208,   145,
     161,   173,   174,     5,   162,   163,   162,   163,   129,   164,
     130,    26,    26,   175,   128,     3,   181,   182,    19,    37,
     130,   130,   129,   161,   173,    78,   173,   177,     8,   114,
     129,   130,   174,     5,   128,   129,   162,   182,   129
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

  case 111:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 112:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 113:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 114:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 115:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 116:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 117:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 121:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 122:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 123:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 125:

    {
			pParser->AddHaving();
		;}
    break;

  case 128:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 131:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 132:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 134:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 136:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 137:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 140:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 141:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
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
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 150:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 153:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 154:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 156:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 157:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 162:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 163:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 180:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 181:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 182:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 186:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 187:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 188:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 189:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 190:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 191:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 192:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 193:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 194:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 195:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 196:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 201:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 202:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 209:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 210:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 211:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 212:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 213:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 214:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 215:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 216:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 217:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 218:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 226:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 227:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 228:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 229:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 230:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 231:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 232:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 233:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 236:

    { yyval.m_iValue = 1; ;}
    break;

  case 237:

    { yyval.m_iValue = 0; ;}
    break;

  case 238:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 239:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 240:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 241:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 244:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 245:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 246:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 249:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 250:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 253:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 254:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 255:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 256:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 257:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 258:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 259:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 260:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 261:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 262:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 263:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 264:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 265:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 267:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 268:

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

  case 269:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 272:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 274:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 279:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 282:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 283:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 284:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
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

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 290:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 291:

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

  case 292:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 293:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 294:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 295:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 296:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 297:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 298:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 299:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 306:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 307:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 308:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 316:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 317:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 318:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 319:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 320:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 321:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 322:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 323:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 324:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 325:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 328:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 329:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 330:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 331:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 333:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 334:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 335:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 336:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 337:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 338:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 339:

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

