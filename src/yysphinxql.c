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
     TOK_JSON = 318,
     TOK_LEVEL = 319,
     TOK_LIKE = 320,
     TOK_LIMIT = 321,
     TOK_MATCH = 322,
     TOK_MAX = 323,
     TOK_META = 324,
     TOK_MIN = 325,
     TOK_MOD = 326,
     TOK_MULTI = 327,
     TOK_MULTI64 = 328,
     TOK_NAMES = 329,
     TOK_NULL = 330,
     TOK_OPTION = 331,
     TOK_ORDER = 332,
     TOK_OPTIMIZE = 333,
     TOK_PLAN = 334,
     TOK_PROFILE = 335,
     TOK_RAND = 336,
     TOK_RAMCHUNK = 337,
     TOK_RANKER = 338,
     TOK_READ = 339,
     TOK_REPEATABLE = 340,
     TOK_REPLACE = 341,
     TOK_RETURNS = 342,
     TOK_ROLLBACK = 343,
     TOK_RTINDEX = 344,
     TOK_SELECT = 345,
     TOK_SERIALIZABLE = 346,
     TOK_SET = 347,
     TOK_SESSION = 348,
     TOK_SHOW = 349,
     TOK_SONAME = 350,
     TOK_START = 351,
     TOK_STATUS = 352,
     TOK_STRING = 353,
     TOK_SUM = 354,
     TOK_TABLE = 355,
     TOK_TABLES = 356,
     TOK_TO = 357,
     TOK_TRANSACTION = 358,
     TOK_TRUE = 359,
     TOK_TRUNCATE = 360,
     TOK_UNCOMMITTED = 361,
     TOK_UPDATE = 362,
     TOK_VALUES = 363,
     TOK_VARIABLES = 364,
     TOK_WARNINGS = 365,
     TOK_WEIGHT = 366,
     TOK_WHERE = 367,
     TOK_WITHIN = 368,
     TOK_OR = 369,
     TOK_AND = 370,
     TOK_NE = 371,
     TOK_GTE = 372,
     TOK_LTE = 373,
     TOK_NOT = 374,
     TOK_NEG = 375
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
#define TOK_JSON 318
#define TOK_LEVEL 319
#define TOK_LIKE 320
#define TOK_LIMIT 321
#define TOK_MATCH 322
#define TOK_MAX 323
#define TOK_META 324
#define TOK_MIN 325
#define TOK_MOD 326
#define TOK_MULTI 327
#define TOK_MULTI64 328
#define TOK_NAMES 329
#define TOK_NULL 330
#define TOK_OPTION 331
#define TOK_ORDER 332
#define TOK_OPTIMIZE 333
#define TOK_PLAN 334
#define TOK_PROFILE 335
#define TOK_RAND 336
#define TOK_RAMCHUNK 337
#define TOK_RANKER 338
#define TOK_READ 339
#define TOK_REPEATABLE 340
#define TOK_REPLACE 341
#define TOK_RETURNS 342
#define TOK_ROLLBACK 343
#define TOK_RTINDEX 344
#define TOK_SELECT 345
#define TOK_SERIALIZABLE 346
#define TOK_SET 347
#define TOK_SESSION 348
#define TOK_SHOW 349
#define TOK_SONAME 350
#define TOK_START 351
#define TOK_STATUS 352
#define TOK_STRING 353
#define TOK_SUM 354
#define TOK_TABLE 355
#define TOK_TABLES 356
#define TOK_TO 357
#define TOK_TRANSACTION 358
#define TOK_TRUE 359
#define TOK_TRUNCATE 360
#define TOK_UNCOMMITTED 361
#define TOK_UPDATE 362
#define TOK_VALUES 363
#define TOK_VARIABLES 364
#define TOK_WARNINGS 365
#define TOK_WEIGHT 366
#define TOK_WHERE 367
#define TOK_WITHIN 368
#define TOK_OR 369
#define TOK_AND 370
#define TOK_NE 371
#define TOK_GTE 372
#define TOK_LTE 373
#define TOK_NOT 374
#define TOK_NEG 375




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
#define YYFINAL  179
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4283

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  140
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  123
/* YYNRULES -- Number of rules. */
#define YYNRULES  401
/* YYNRULES -- Number of states. */
#define YYNSTATES  718

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   375

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   128,   117,     2,
     132,   133,   126,   124,   134,   125,   137,   127,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   131,
     120,   118,   121,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   138,     2,   139,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   135,   116,   136,     2,     2,     2,     2,
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
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   119,   122,   123,   129,   130
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    56,    58,
      60,    62,    64,    66,    68,    70,    72,    74,    76,    78,
      80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
     100,   102,   104,   106,   108,   110,   112,   114,   116,   118,
     120,   122,   124,   126,   128,   130,   132,   134,   136,   138,
     140,   142,   144,   146,   148,   150,   152,   154,   156,   160,
     162,   164,   166,   175,   177,   187,   188,   191,   193,   197,
     199,   201,   203,   204,   208,   209,   212,   217,   229,   231,
     235,   237,   240,   241,   243,   246,   248,   253,   258,   263,
     268,   273,   278,   282,   288,   290,   294,   295,   297,   300,
     302,   306,   310,   315,   317,   321,   325,   331,   338,   342,
     347,   353,   357,   361,   365,   369,   373,   375,   381,   387,
     393,   397,   401,   405,   409,   413,   417,   421,   426,   430,
     432,   434,   439,   443,   447,   449,   451,   456,   461,   466,
     468,   471,   473,   476,   478,   480,   484,   485,   490,   491,
     493,   495,   499,   500,   503,   504,   506,   512,   513,   515,
     519,   525,   527,   531,   533,   536,   539,   540,   542,   545,
     550,   551,   553,   556,   558,   562,   566,   570,   574,   580,
     587,   594,   598,   602,   604,   608,   612,   614,   616,   618,
     620,   622,   624,   626,   629,   632,   636,   640,   644,   648,
     652,   656,   660,   664,   668,   672,   676,   680,   684,   688,
     692,   696,   700,   704,   708,   710,   712,   714,   718,   723,
     728,   733,   738,   743,   748,   753,   757,   764,   771,   775,
     784,   786,   790,   792,   794,   798,   804,   806,   808,   810,
     812,   815,   816,   819,   821,   824,   827,   831,   833,   835,
     840,   845,   849,   854,   859,   864,   868,   873,   878,   886,
     892,   898,   900,   902,   904,   906,   908,   910,   914,   916,
     918,   920,   922,   924,   926,   928,   930,   932,   935,   942,
     944,   946,   947,   951,   953,   955,   957,   961,   963,   967,
     971,   973,   977,   979,   981,   983,   987,   990,   995,  1002,
    1004,  1008,  1010,  1014,  1016,  1020,  1021,  1024,  1026,  1030,
    1034,  1035,  1037,  1039,  1041,  1045,  1047,  1049,  1053,  1057,
    1064,  1066,  1070,  1074,  1078,  1084,  1089,  1093,  1097,  1099,
    1101,  1103,  1105,  1107,  1109,  1111,  1113,  1121,  1126,  1132,
    1133,  1135,  1138,  1140,  1144,  1148,  1151,  1155,  1162,  1163,
    1165,  1167,  1170,  1173,  1176,  1178,  1186,  1188,  1190,  1192,
    1194,  1200,  1204,  1208,  1215,  1219,  1223,  1227,  1229,  1233,
    1236,  1240,  1244,  1247,  1249,  1252,  1254,  1256,  1260,  1264,
    1268,  1272
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     141,     0,    -1,   142,    -1,   145,    -1,   145,   131,    -1,
     207,    -1,   216,    -1,   199,    -1,   200,    -1,   205,    -1,
     217,    -1,   226,    -1,   228,    -1,   229,    -1,   230,    -1,
     235,    -1,   240,    -1,   241,    -1,   245,    -1,   249,    -1,
     247,    -1,   248,    -1,   250,    -1,   251,    -1,   252,    -1,
     242,    -1,   253,    -1,   255,    -1,   256,    -1,   257,    -1,
     234,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    29,    -1,    33,    -1,    45,    -1,
      48,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      62,    -1,    64,    -1,    65,    -1,    67,    -1,    68,    -1,
      69,    -1,    79,    -1,    80,    -1,    82,    -1,    81,    -1,
      84,    -1,    85,    -1,    87,    -1,    88,    -1,    89,    -1,
      91,    -1,    93,    -1,    96,    -1,    97,    -1,    98,    -1,
      99,    -1,   101,    -1,   105,    -1,   106,    -1,   109,    -1,
     110,    -1,   111,    -1,   113,    -1,    63,    -1,   143,    -1,
      74,    -1,   103,    -1,   146,    -1,   145,   131,   146,    -1,
     147,    -1,   196,    -1,   148,    -1,    90,     3,   132,   132,
     148,   133,   149,   133,    -1,   155,    -1,    90,   156,    47,
     132,   152,   155,   133,   153,   154,    -1,    -1,   134,   150,
      -1,   151,    -1,   150,   134,   151,    -1,   144,    -1,     5,
      -1,    54,    -1,    -1,    77,    26,   179,    -1,    -1,    66,
       5,    -1,    66,     5,   134,     5,    -1,    90,   156,    47,
     160,   161,   171,   175,   174,   177,   181,   183,    -1,   157,
      -1,   156,   134,   157,    -1,   126,    -1,   159,   158,    -1,
      -1,   144,    -1,    18,   144,    -1,   189,    -1,    21,   132,
     189,   133,    -1,    68,   132,   189,   133,    -1,    70,   132,
     189,   133,    -1,    99,   132,   189,   133,    -1,    52,   132,
     189,   133,    -1,    33,   132,   126,   133,    -1,    51,   132,
     133,    -1,    33,   132,    39,   144,   133,    -1,   144,    -1,
     160,   134,   144,    -1,    -1,   162,    -1,   112,   163,    -1,
     164,    -1,   163,   115,   163,    -1,   132,   163,   133,    -1,
      67,   132,     8,   133,    -1,   165,    -1,   167,   118,   168,
      -1,   167,   119,   168,    -1,   167,    55,   132,   170,   133,
      -1,   167,   129,    55,   132,   170,   133,    -1,   167,    55,
       9,    -1,   167,   129,    55,     9,    -1,   167,    23,   168,
     115,   168,    -1,   167,   121,   168,    -1,   167,   120,   168,
      -1,   167,   122,   168,    -1,   167,   123,   168,    -1,   167,
     118,   169,    -1,   166,    -1,   167,    23,   169,   115,   169,
      -1,   167,    23,   168,   115,   169,    -1,   167,    23,   169,
     115,   168,    -1,   167,   121,   169,    -1,   167,   120,   169,
      -1,   167,   122,   169,    -1,   167,   123,   169,    -1,   167,
     118,     8,    -1,   167,   119,     8,    -1,   167,    61,    75,
      -1,   167,    61,   129,    75,    -1,   167,   119,   169,    -1,
     144,    -1,     4,    -1,    33,   132,   126,   133,    -1,    51,
     132,   133,    -1,   111,   132,   133,    -1,    54,    -1,   258,
      -1,    59,   132,   258,   133,    -1,    41,   132,   258,   133,
      -1,    24,   132,   258,   133,    -1,     5,    -1,   125,     5,
      -1,     6,    -1,   125,     6,    -1,    14,    -1,   168,    -1,
     170,   134,   168,    -1,    -1,    50,   172,    26,   173,    -1,
      -1,     5,    -1,   167,    -1,   173,   134,   167,    -1,    -1,
      53,   165,    -1,    -1,   176,    -1,   113,    50,    77,    26,
     179,    -1,    -1,   178,    -1,    77,    26,   179,    -1,    77,
      26,    81,   132,   133,    -1,   180,    -1,   179,   134,   180,
      -1,   167,    -1,   167,    19,    -1,   167,    37,    -1,    -1,
     182,    -1,    66,     5,    -1,    66,     5,   134,     5,    -1,
      -1,   184,    -1,    76,   185,    -1,   186,    -1,   185,   134,
     186,    -1,   144,   118,   144,    -1,    83,   118,   144,    -1,
     144,   118,     5,    -1,   144,   118,   132,   187,   133,    -1,
     144,   118,   144,   132,     8,   133,    -1,    83,   118,   144,
     132,     8,   133,    -1,   144,   118,     8,    -1,    83,   118,
       8,    -1,   188,    -1,   187,   134,   188,    -1,   144,   118,
     168,    -1,   144,    -1,     4,    -1,    54,    -1,     5,    -1,
       6,    -1,    14,    -1,     9,    -1,   125,   189,    -1,   129,
     189,    -1,   189,   124,   189,    -1,   189,   125,   189,    -1,
     189,   126,   189,    -1,   189,   127,   189,    -1,   189,   120,
     189,    -1,   189,   121,   189,    -1,   189,   117,   189,    -1,
     189,   116,   189,    -1,   189,   128,   189,    -1,   189,    40,
     189,    -1,   189,    71,   189,    -1,   189,   123,   189,    -1,
     189,   122,   189,    -1,   189,   118,   189,    -1,   189,   119,
     189,    -1,   189,   115,   189,    -1,   189,   114,   189,    -1,
     132,   189,   133,    -1,   135,   193,   136,    -1,   190,    -1,
     258,    -1,   261,    -1,   258,    61,    75,    -1,   258,    61,
     129,    75,    -1,     3,   132,   191,   133,    -1,    55,   132,
     191,   133,    -1,    59,   132,   191,   133,    -1,    24,   132,
     191,   133,    -1,    44,   132,   191,   133,    -1,    41,   132,
     191,   133,    -1,     3,   132,   133,    -1,    70,   132,   189,
     134,   189,   133,    -1,    68,   132,   189,   134,   189,   133,
      -1,   111,   132,   133,    -1,     3,   132,   189,    46,   144,
      55,   258,   133,    -1,   192,    -1,   191,   134,   192,    -1,
     189,    -1,     8,    -1,   194,   118,   195,    -1,   193,   134,
     194,   118,   195,    -1,   144,    -1,    55,    -1,   168,    -1,
     144,    -1,    94,   198,    -1,    -1,    65,     8,    -1,   110,
      -1,    97,   197,    -1,    69,   197,    -1,    16,    97,   197,
      -1,    80,    -1,    79,    -1,    16,     8,    97,   197,    -1,
      16,   144,    97,   197,    -1,    56,   144,    97,    -1,    92,
     143,   118,   202,    -1,    92,   143,   118,   201,    -1,    92,
     143,   118,    75,    -1,    92,    74,   203,    -1,    92,    10,
     118,   203,    -1,    92,    28,    92,   203,    -1,    92,    49,
       9,   118,   132,   170,   133,    -1,    92,    49,   143,   118,
     201,    -1,    92,    49,   143,   118,   203,    -1,   144,    -1,
       8,    -1,   104,    -1,    43,    -1,   168,    -1,   204,    -1,
     203,   125,   204,    -1,   144,    -1,    75,    -1,     8,    -1,
       5,    -1,     6,    -1,    31,    -1,    88,    -1,   206,    -1,
      22,    -1,    96,   103,    -1,   208,    60,   144,   209,   108,
     212,    -1,    57,    -1,    86,    -1,    -1,   132,   211,   133,
      -1,   144,    -1,    54,    -1,   210,    -1,   211,   134,   210,
      -1,   213,    -1,   212,   134,   213,    -1,   132,   214,   133,
      -1,   215,    -1,   214,   134,   215,    -1,   168,    -1,   169,
      -1,     8,    -1,   132,   170,   133,    -1,   132,   133,    -1,
      36,    47,   160,   162,    -1,    27,   144,   132,   218,   221,
     133,    -1,   219,    -1,   218,   134,   219,    -1,   215,    -1,
     132,   220,   133,    -1,     8,    -1,   220,   134,     8,    -1,
      -1,   134,   222,    -1,   223,    -1,   222,   134,   223,    -1,
     215,   224,   225,    -1,    -1,    18,    -1,   144,    -1,    66,
      -1,   227,   144,   197,    -1,    38,    -1,    37,    -1,    94,
     101,   197,    -1,    94,    35,   197,    -1,   107,   160,    92,
     231,   162,   183,    -1,   232,    -1,   231,   134,   232,    -1,
     144,   118,   168,    -1,   144,   118,   169,    -1,   144,   118,
     132,   170,   133,    -1,   144,   118,   132,   133,    -1,   258,
     118,   168,    -1,   258,   118,   169,    -1,    59,    -1,    24,
      -1,    44,    -1,    25,    -1,    72,    -1,    73,    -1,    63,
      -1,    98,    -1,    17,   100,   144,    15,    30,   144,   233,
      -1,    94,   243,   109,   236,    -1,    94,   243,   109,    65,
       8,    -1,    -1,   237,    -1,   112,   238,    -1,   239,    -1,
     238,   114,   239,    -1,   144,   118,     8,    -1,    94,    29,
      -1,    94,    28,    92,    -1,    92,   243,   103,    62,    64,
     244,    -1,    -1,    49,    -1,    93,    -1,    84,   106,    -1,
      84,    32,    -1,    85,    84,    -1,    91,    -1,    34,    48,
     144,    87,   246,    95,     8,    -1,    58,    -1,    24,    -1,
      44,    -1,    98,    -1,    34,    83,   144,    95,     8,    -1,
      42,    83,   144,    -1,    42,    48,   144,    -1,    20,    56,
     144,   102,    89,   144,    -1,    45,    89,   144,    -1,    45,
      82,   144,    -1,    90,   254,   181,    -1,    10,    -1,    10,
     137,   144,    -1,    90,   189,    -1,   105,    89,   144,    -1,
      78,    56,   144,    -1,   144,   259,    -1,   260,    -1,   259,
     260,    -1,    13,    -1,    14,    -1,   138,   189,   139,    -1,
     138,     8,   139,    -1,   189,   118,   262,    -1,   262,   118,
     189,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   166,   166,   167,   168,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   208,   209,   209,   209,   209,   209,   210,   210,   210,
     210,   211,   211,   211,   211,   211,   212,   212,   212,   212,
     212,   213,   213,   213,   213,   213,   214,   214,   214,   214,
     215,   215,   215,   215,   215,   216,   216,   216,   216,   217,
     217,   217,   217,   218,   222,   222,   222,   228,   229,   233,
     234,   238,   239,   247,   248,   255,   257,   261,   265,   272,
     273,   274,   278,   291,   298,   300,   305,   314,   330,   331,
     335,   336,   339,   341,   342,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   358,   359,   362,   364,   368,   372,
     373,   374,   378,   383,   387,   394,   402,   410,   419,   424,
     429,   434,   439,   444,   449,   454,   459,   464,   469,   474,
     479,   484,   489,   494,   499,   504,   509,   514,   522,   526,
     527,   532,   538,   544,   550,   556,   557,   558,   559,   563,
     564,   575,   576,   577,   581,   587,   593,   595,   598,   600,
     607,   611,   617,   619,   625,   627,   631,   642,   644,   648,
     652,   659,   660,   664,   665,   666,   669,   671,   675,   680,
     687,   689,   693,   697,   698,   702,   707,   713,   718,   724,
     729,   734,   739,   747,   752,   759,   769,   770,   771,   772,
     773,   774,   775,   776,   777,   779,   780,   781,   782,   783,
     784,   785,   786,   787,   788,   789,   790,   791,   792,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   806,
     807,   808,   809,   810,   811,   812,   813,   814,   815,   816,
     820,   821,   825,   826,   830,   831,   835,   836,   840,   841,
     847,   850,   852,   856,   857,   858,   859,   860,   861,   862,
     867,   872,   882,   887,   892,   897,   898,   899,   903,   908,
     913,   921,   922,   926,   927,   928,   940,   941,   945,   946,
     947,   948,   949,   956,   957,   958,   962,   963,   969,   977,
     978,   981,   983,   987,   988,   992,   993,   997,   998,  1002,
    1006,  1007,  1011,  1012,  1013,  1014,  1015,  1021,  1031,  1039,
    1043,  1050,  1051,  1058,  1068,  1074,  1076,  1080,  1085,  1089,
    1096,  1098,  1102,  1103,  1109,  1117,  1118,  1124,  1128,  1134,
    1142,  1143,  1147,  1161,  1167,  1171,  1176,  1190,  1201,  1202,
    1203,  1204,  1205,  1206,  1207,  1208,  1212,  1225,  1229,  1236,
    1237,  1241,  1245,  1246,  1250,  1254,  1261,  1268,  1274,  1275,
    1276,  1280,  1281,  1282,  1283,  1289,  1300,  1301,  1302,  1303,
    1307,  1317,  1326,  1337,  1349,  1358,  1369,  1377,  1378,  1382,
    1392,  1403,  1414,  1417,  1418,  1422,  1423,  1424,  1425,  1429,
    1430,  1434
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
  "TOK_INTEGER", "TOK_INTO", "TOK_IS", "TOK_ISOLATION", "TOK_JSON", 
  "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_MULTI", "TOK_MULTI64", 
  "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", 
  "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", "TOK_RANKER", 
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
  "$accept", "request", "statement", "ident_set", "ident", 
  "multi_stmt_list", "multi_stmt", "select", "select1", 
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
  "show_what", "set_stmt", "set_global_stmt", "set_string_value", 
  "boolean_value", "set_value", "simple_set_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_ident", "column_list", "insert_rows_list", 
  "insert_row", "insert_vals_list", "insert_val", "delete_from", 
  "call_proc", "call_args_list", "call_arg", "const_string_list", 
  "opt_call_opts_list", "call_opts_list", "call_opt", "opt_as", 
  "call_opt_name", "describe", "describe_tok", "show_tables", 
  "show_databases", "update", "update_items_list", "update_item", 
  "alter_col_type", "alter", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "show_character_set", 
  "set_transaction", "opt_scope", "isolation_level", "create_function", 
  "udf_type", "create_ranker", "drop_ranker", "drop_function", 
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
     365,   366,   367,   368,   369,   370,   124,    38,    61,   371,
      60,    62,   372,   373,    43,    45,    42,    47,    37,   374,
     375,    59,    40,    41,    44,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   140,   141,   141,   141,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   144,   144,   144,   145,   145,   146,
     146,   147,   147,   148,   148,   149,   149,   150,   150,   151,
     151,   151,   152,   153,   154,   154,   154,   155,   156,   156,
     157,   157,   158,   158,   158,   159,   159,   159,   159,   159,
     159,   159,   159,   159,   160,   160,   161,   161,   162,   163,
     163,   163,   164,   164,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   166,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   168,
     168,   169,   169,   169,   170,   170,   171,   171,   172,   172,
     173,   173,   174,   174,   175,   175,   176,   177,   177,   178,
     178,   179,   179,   180,   180,   180,   181,   181,   182,   182,
     183,   183,   184,   185,   185,   186,   186,   186,   186,   186,
     186,   186,   186,   187,   187,   188,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   190,
     190,   190,   190,   190,   190,   190,   190,   190,   190,   190,
     191,   191,   192,   192,   193,   193,   194,   194,   195,   195,
     196,   197,   197,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   199,   199,   199,   199,   199,   199,   200,   200,
     200,   201,   201,   202,   202,   202,   203,   203,   204,   204,
     204,   204,   204,   205,   205,   205,   206,   206,   207,   208,
     208,   209,   209,   210,   210,   211,   211,   212,   212,   213,
     214,   214,   215,   215,   215,   215,   215,   216,   217,   218,
     218,   219,   219,   220,   220,   221,   221,   222,   222,   223,
     224,   224,   225,   225,   226,   227,   227,   228,   229,   230,
     231,   231,   232,   232,   232,   232,   232,   232,   233,   233,
     233,   233,   233,   233,   233,   233,   234,   235,   235,   236,
     236,   237,   238,   238,   239,   240,   241,   242,   243,   243,
     243,   244,   244,   244,   244,   245,   246,   246,   246,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   254,   255,
     256,   257,   258,   259,   259,   260,   260,   260,   260,   261,
     261,   262
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     8,     1,     9,     0,     2,     1,     3,     1,
       1,     1,     0,     3,     0,     2,     4,    11,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     4,     3,     5,     1,     3,     0,     1,     2,     1,
       3,     3,     4,     1,     3,     3,     5,     6,     3,     4,
       5,     3,     3,     3,     3,     3,     1,     5,     5,     5,
       3,     3,     3,     3,     3,     3,     3,     4,     3,     1,
       1,     4,     3,     3,     1,     1,     4,     4,     4,     1,
       2,     1,     2,     1,     1,     3,     0,     4,     0,     1,
       1,     3,     0,     2,     0,     1,     5,     0,     1,     3,
       5,     1,     3,     1,     2,     2,     0,     1,     2,     4,
       0,     1,     2,     1,     3,     3,     3,     3,     5,     6,
       6,     3,     3,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     1,     3,     4,     4,
       4,     4,     4,     4,     4,     3,     6,     6,     3,     8,
       1,     3,     1,     1,     3,     5,     1,     1,     1,     1,
       2,     0,     2,     1,     2,     2,     3,     1,     1,     4,
       4,     3,     4,     4,     4,     3,     4,     4,     7,     5,
       5,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     6,     1,
       1,     0,     3,     1,     1,     1,     3,     1,     3,     3,
       1,     3,     1,     1,     1,     3,     2,     4,     6,     1,
       3,     1,     3,     1,     3,     0,     2,     1,     3,     3,
       0,     1,     1,     1,     3,     1,     1,     3,     3,     6,
       1,     3,     3,     3,     5,     4,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     7,     4,     5,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     1,
       5,     3,     3,     6,     3,     3,     3,     1,     3,     2,
       3,     3,     2,     1,     2,     1,     1,     3,     3,     3,
       3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   296,     0,   293,     0,     0,   336,   335,
       0,     0,   299,     0,   300,   294,     0,   368,   368,     0,
       0,     0,     0,     2,     3,    77,    79,    81,    83,    80,
       7,     8,     9,   295,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    30,    15,    16,    17,    25,    18,    20,
      21,    19,    22,    23,    24,    26,    27,    28,    29,     0,
       0,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    73,    46,    47,    48,
      49,    50,    75,    51,    52,    54,    53,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    76,
      67,    68,    69,    70,    71,    72,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    31,   207,   209,   210,
     401,   212,   387,   211,    34,     0,    38,     0,     0,    43,
      44,   208,     0,     0,    49,     0,    65,    71,     0,   100,
       0,     0,     0,   206,     0,    98,   102,   105,   234,   186,
     235,   236,     0,     0,     0,    41,     0,    61,     0,     0,
       0,     0,   365,   261,   369,     0,   261,   268,   267,   370,
     261,   261,   263,   260,     0,   297,     0,   114,     0,     1,
       4,     0,   261,     0,     0,     0,     0,     0,     0,   382,
     381,   385,   384,   391,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
      49,     0,   213,   214,     0,   257,   256,     0,     0,   395,
     396,     0,   392,   393,     0,     0,     0,   103,   101,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   386,   187,     0,
       0,     0,     0,     0,     0,   291,   292,   290,   289,   288,
     275,   286,     0,     0,     0,   261,     0,   366,     0,   338,
       0,   265,   264,   337,   359,   390,     0,     0,     0,     0,
      78,   301,   334,     0,     0,   159,   161,   314,   163,     0,
       0,   312,   313,   321,   325,   319,     0,     0,     0,   317,
     253,     0,   245,   252,     0,   250,   388,     0,   252,     0,
       0,     0,     0,     0,   112,     0,     0,     0,     0,     0,
       0,   248,     0,     0,     0,   232,     0,   233,     0,   401,
       0,   394,    92,   116,    99,   105,   104,   224,   225,   231,
     230,   222,   221,   228,   399,   229,   219,   220,   227,   226,
     215,   216,   217,   218,   223,   188,   237,     0,   400,   276,
     277,     0,     0,     0,   282,   284,   274,   283,     0,   281,
     285,   273,   272,     0,   261,   266,   261,   262,   271,     0,
       0,   357,   360,     0,     0,   340,     0,   115,     0,     0,
       0,     0,   160,   162,   323,   316,   164,     0,     0,     0,
       0,   377,   378,   376,   379,     0,   380,   150,     0,    38,
       0,    43,   154,     0,    48,    71,     0,   149,   118,   119,
     123,   136,     0,   155,     0,     0,     0,   239,     0,   106,
     242,     0,   111,   244,   243,   110,   240,   241,   107,     0,
     108,     0,   109,     0,     0,     0,   259,   258,   254,   398,
     397,     0,   166,   117,     0,   238,     0,   282,   281,   279,
     280,   287,     0,   269,   270,   358,     0,   361,   362,     0,
       0,   190,     0,   304,   303,   305,     0,     0,     0,   383,
     315,     0,   322,     0,   330,   320,   326,   327,   318,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
       0,   251,   113,     0,     0,     0,     0,     0,   168,   174,
     189,     0,     0,     0,   374,   367,     0,     0,     0,   342,
     343,   341,     0,   339,   191,   346,   347,   302,     0,     0,
     298,   307,   349,   351,   350,   348,   354,   352,   353,   355,
     356,   165,   324,   331,     0,     0,   375,     0,     0,     0,
       0,   152,     0,     0,   153,   121,   120,     0,     0,   128,
       0,   146,     0,   144,   124,   135,   145,   125,   148,   132,
     141,   131,   140,   133,   142,   134,   143,     0,     0,     0,
       0,   247,   246,   255,     0,     0,   169,     0,     0,   172,
     175,   278,   372,   371,   373,   364,   363,   345,     0,     0,
       0,   192,   193,   306,     0,     0,   310,     0,   333,   332,
     329,   330,   328,   158,   151,   157,   156,   122,     0,     0,
       0,   147,   129,     0,    90,    91,    89,    86,    87,    82,
       0,     0,     0,    94,     0,     0,     0,   177,   344,     0,
       0,     0,   309,     0,   308,   130,   138,   139,   137,   126,
       0,     0,   249,     0,     0,    84,   170,   167,     0,   173,
       0,   186,   178,   202,   196,   197,   201,     0,   195,   194,
     311,   127,    88,   183,    93,   181,    95,     0,     0,     0,
     190,     0,     0,     0,   203,     0,   184,   185,     0,     0,
     171,   176,    54,   179,    97,     0,     0,   198,     0,     0,
     182,    96,     0,   200,   205,   204,   199,   180
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   106,   143,    24,    25,    26,    27,   589,
     637,   638,   451,   643,   665,    28,   144,   145,   228,   146,
     333,   452,   299,   418,   419,   420,   421,   422,   396,   292,
     397,   519,   597,   667,   647,   599,   600,   671,   672,   684,
     685,   247,   248,   533,   534,   611,   612,   693,   694,   308,
     148,   304,   305,   217,   218,   448,    29,   269,   173,    30,
      31,   371,   372,   260,   261,    32,    33,    34,    35,   389,
     475,   476,   540,   541,   615,   293,    36,    37,   294,   295,
     398,   400,   486,   487,   554,   620,    38,    39,    40,    41,
      42,   384,   385,   550,    43,    44,   381,   382,   467,   468,
      45,    46,    47,   159,   525,    48,   405,    49,    50,    51,
      52,    53,    54,    55,   149,    56,    57,    58,   150,   222,
     223,   151,   152
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -530
static const short yypact[] =
{
    4116,   -72,     9,  -530,  3726,  -530,    11,   103,  -530,  -530,
      30,    32,  -530,    35,  -530,  -530,   687,  2934,   620,    17,
      23,  3726,   173,  -530,    45,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,   119,  -530,  -530,  -530,  3726,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  3726,
    3726,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,    60,  3726,  3726,
    3726,  3726,  3726,  3726,  3726,  3726,    69,  -530,  -530,  -530,
    -530,  -530,    53,  -530,    76,    79,    85,    87,    89,    91,
      98,  -530,   102,   104,   106,   124,   139,   157,  1485,  -530,
    1485,  1485,  3033,    36,   -34,  -530,  3132,     8,  -530,   138,
     244,  -530,   192,   194,   222,  3825,  2622,   215,   205,   233,
    3231,   245,  -530,   273,  -530,  3726,   273,  -530,  -530,  -530,
     273,   273,  -530,  -530,   230,  -530,  3726,  -530,   -39,  -530,
     -53,  3726,   273,   345,   259,    26,   275,   268,   -44,  -530,
    -530,  -530,  -530,  -530,   820,  3726,  1485,  1618,   -24,  1618,
    1618,   231,  1485,  1618,  1618,  1485,  1485,  1485,   234,   236,
     237,   238,  -530,  -530,  3856,  -530,  -530,    31,   248,  -530,
    -530,  1751,    36,  -530,  2078,   953,  3726,  -530,  -530,  1485,
    1485,  1485,  1485,  1485,  1485,  1485,  1485,  1485,  1485,  1485,
    1485,  1485,  1485,  1485,  1485,  1485,   366,  -530,  -530,   -52,
    1485,  2622,  2622,   254,   255,  -530,  -530,  -530,  -530,  -530,
     249,  -530,  2187,   313,   280,     2,   281,  -530,   371,  -530,
     283,  -530,  -530,  -530,   -32,  -530,  3726,  3726,  1086,   147,
    -530,   250,  -530,   353,   296,  -530,  -530,  -530,  -530,    80,
      14,  -530,  -530,  -530,   257,  -530,   151,   380,  1868,  -530,
     271,  1219,  -530,  4063,   -18,  -530,  -530,  3885,  4122,    65,
    3726,   260,    73,    99,  -530,  3915,   112,   117,   421,   442,
    3945,  -530,  1352,  1485,  1485,  -530,  3033,  -530,  2298,   253,
     160,  -530,  -530,   -44,  -530,  4122,  -530,  -530,  -530,   229,
    4136,  4155,   350,   540,  -530,   540,   115,   115,   115,   115,
     126,   126,  -530,  -530,  -530,   261,  -530,   319,   540,   249,
     249,   265,  2731,  2622,  -530,  -530,  -530,  -530,   394,  -530,
    -530,  -530,  -530,   336,   273,  -530,   273,  -530,  -530,   393,
    3726,  -530,  -530,    44,   -16,  -530,   284,  -530,  3330,   295,
    3726,  3726,  -530,  -530,  -530,  -530,  -530,   132,   134,    26,
     272,  -530,  -530,  -530,  -530,   311,  -530,  -530,   278,   282,
     285,   286,  -530,   287,   288,   292,  1868,    36,   298,  -530,
    -530,  -530,   141,  -530,   953,   293,  3726,  -530,  1618,  -530,
    -530,   297,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  1485,
    -530,  1485,  -530,   480,  3826,   307,  -530,  -530,  -530,  -530,
    -530,   325,   378,  -530,   411,  -530,    21,   304,   323,  -530,
     249,  -530,   144,  -530,  -530,  -530,   331,   337,  -530,    56,
    3726,   374,    24,  -530,  -530,  -530,   168,   321,  2965,  -530,
    -530,    21,  -530,   448,    51,  -530,   324,  -530,  -530,   449,
    3726,   333,  3726,   327,  3726,   454,   330,    72,  1868,    24,
       5,   -48,    58,    68,    24,    24,    24,    24,   409,   347,
     428,  -530,  -530,  3974,  4004,  2298,   953,   352,   482,   376,
    -530,   170,    -8,   407,  -530,  -530,   485,  3726,    15,  -530,
    -530,  -530,  3429,  -530,  -530,  -530,  -530,  -530,  3330,    46,
     361,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  3528,    46,  -530,    36,   363,   364,
     365,  -530,   367,   368,  -530,  -530,  -530,   384,   387,  -530,
      21,  -530,   429,  -530,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,    12,  2830,   372,
    3726,  -530,  -530,  -530,   -31,   430,  -530,   486,   456,   461,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,   175,   397,
     398,   383,  -530,  -530,    20,   183,  -530,   321,  -530,  -530,
    -530,   500,  -530,  -530,  -530,  -530,  -530,  -530,    24,    24,
     187,  -530,  -530,    21,  -530,  -530,  -530,   388,  -530,  -530,
     396,  3726,   495,   464,  2409,   450,  2409,   475,  -530,  3627,
    1977,  3429,  -530,    46,  -530,  -530,  -530,  -530,  -530,  -530,
     193,  2830,  -530,  2409,   526,  -530,  -530,   399,   527,  -530,
     545,   138,  -530,  -530,   440,  -530,  -530,  3726,   441,  -530,
    -530,  -530,  -530,    52,   443,  -530,   444,  2409,  2409,  2513,
     374,   566,   463,   201,  -530,   571,  -530,  -530,  2409,   577,
    -530,   443,   451,   443,  -530,   452,    21,  -530,  3726,   455,
    -530,  -530,   457,  -530,  -530,  -530,  -530,  -530
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -530,  -530,  -530,   -12,    -4,  -530,   412,  -530,   290,  -530,
    -530,   -74,  -530,  -530,  -530,   142,    94,   390,  -530,  -530,
     -17,  -530,  -297,  -406,  -530,   -57,  -530,  -474,  -174,  -460,
    -453,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -529,
     -89,   -59,  -530,   -77,  -530,  -530,   -35,  -530,   -91,   202,
    -530,    93,   190,  -530,   294,   107,  -530,  -164,  -530,  -530,
    -530,   262,  -530,  -168,   256,  -530,  -530,  -530,  -530,  -530,
      83,  -530,  -530,     6,  -530,  -398,  -530,  -530,  -530,   226,
    -530,  -530,  -530,    74,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,   158,  -530,  -530,  -530,  -530,  -530,  -530,   100,
    -530,  -530,  -530,   612,  -530,  -530,  -530,  -530,  -530,  -530,
    -530,  -530,  -530,  -530,  -530,  -530,  -530,  -530,  -179,  -530,
     410,  -530,   400
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -402
static const short yytable[] =
{
     107,   484,   271,   521,   178,   158,   272,   273,  -389,   530,
     497,   291,   536,   224,   569,   310,   641,   177,   282,   285,
     285,   632,   394,   356,   602,   285,   285,   571,    59,   285,
     286,   285,   286,   379,   287,   182,   453,   278,   288,   568,
     288,   279,   575,   578,   580,   582,   584,   586,   229,   219,
     220,   285,   286,   276,   287,   183,   184,   219,   220,   108,
     288,   285,   286,   285,   286,    60,   573,   268,   298,   553,
     288,   696,   288,   285,   286,   608,   576,   357,   111,   230,
     380,   572,   288,   359,   360,   392,   393,   471,   370,   697,
     277,   115,   566,   188,   109,   277,   298,   386,   603,   -63,
     225,   375,   311,   225,   186,   187,   177,   189,   190,   191,
     192,   193,   176,   112,   113,   427,   428,   630,   470,   423,
     175,   114,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   570,   216,   368,
     368,   616,   227,   254,   633,   368,   368,   395,   607,   289,
     110,   289,   259,   395,   447,   229,   266,   621,   290,   701,
     703,   270,   469,   160,   499,   326,   229,   327,   656,   658,
     666,   289,   275,   179,   221,   401,   180,   281,   614,   181,
     660,   289,   221,   289,  -321,  -321,   230,   498,   528,   683,
     195,   306,   185,   289,   460,   402,   500,   230,   430,   428,
     229,   194,   501,   165,   246,   565,   433,   428,   196,   403,
     463,   197,   464,   700,   683,   683,   166,   198,   147,   199,
     177,   200,   336,   201,   683,   291,   167,   168,   522,   523,
     202,   230,   434,   428,   203,   524,   204,   423,   205,   241,
     242,   243,   244,   245,   170,   436,   428,   259,   259,   404,
     437,   428,   243,   244,   245,   680,   206,   172,   369,   502,
     503,   504,   505,   506,   507,   480,   481,   482,   483,   229,
     508,   207,   383,   387,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   208,
     309,   386,   312,   313,   417,   529,   316,   317,   535,   450,
     230,   537,   538,   601,   481,   249,   431,   551,   648,   481,
     250,   558,   251,   560,   252,   562,   652,   653,  -370,   423,
     659,   481,   216,   262,   446,   567,   681,   481,   574,   577,
     579,   581,   583,   585,   707,   708,   263,   267,   268,   274,
     212,   447,   213,   214,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   458,   259,
     283,   284,   296,   297,   314,   291,   328,   321,   322,   323,
     324,   355,   361,   362,   363,   373,   466,   374,   376,   377,
     378,   291,   388,   390,   474,   391,   478,   479,   406,  -401,
     229,   399,   449,   432,   455,   454,   303,   456,   307,   392,
     462,   465,   472,   477,   315,   488,   489,   318,   319,   320,
     490,   640,   417,   498,   491,   516,   520,   492,   493,   494,
     495,   230,   510,   330,   496,   515,   509,   335,   518,  -290,
     512,   337,   338,   339,   340,   341,   342,   343,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,  -288,   526,
     532,   527,   358,   539,   655,   657,   552,   556,   555,   559,
     561,   229,   563,   564,   587,   423,   383,   423,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   291,
     335,   588,   229,   590,   423,   595,   557,   596,   557,   598,
     557,   604,   230,   605,   417,   617,   623,   624,   625,   628,
     626,   627,   629,   214,   631,   639,   645,   642,   423,   423,
     423,   446,   644,   230,   646,   649,   650,   651,   553,   423,
     229,   663,   661,   466,   303,   443,   444,   668,   610,   662,
     664,   686,   714,   687,   474,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     619,   230,   670,   688,   438,   439,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   689,   691,   695,   705,   440,   441,   698,   699,   709,
     229,   706,   711,   712,   636,   713,   557,   682,   716,   669,
     717,   425,   280,   517,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   710,
     594,   230,   690,   704,   439,   334,   679,   715,   511,   461,
     445,   613,   593,   654,   459,   485,   335,   606,   531,   622,
     174,     0,   331,     0,     0,   344,   160,   177,     0,     0,
     417,   513,   417,   514,     0,   674,   678,   610,   161,   162,
       0,     0,     0,     0,     0,   163,     0,   636,     0,   417,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   164,
       0,     0,     0,   692,     0,     0,   165,     0,     0,     0,
       0,     0,     0,   417,   417,   417,     0,     0,     0,   166,
     116,   117,   118,   119,   417,   120,   121,   122,     0,   167,
     168,   123,     0,    62,   692,     0,     0,    63,   124,    65,
       0,   125,    66,   169,     0,     0,    67,   170,   335,     0,
     126,   171,     0,     0,     0,     0,     0,     0,   127,     0,
     172,   128,    69,     0,     0,    70,    71,    72,   129,   130,
       0,   131,   132,     0,     0,     0,   133,     0,     0,    75,
      76,    77,    78,     0,    79,   134,    81,   135,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
       0,    87,    88,     0,    89,    90,    91,     0,    92,     0,
      93,     0,     0,    94,    95,    96,   136,     0,    98,     0,
      99,     0,   100,   101,     0,     0,   102,   103,   137,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   138,   139,     0,     0,   140,     0,     0,   141,
       0,     0,   142,   209,   117,   118,   119,     0,   300,   121,
       0,     0,     0,     0,   123,     0,    62,     0,     0,     0,
      63,    64,    65,     0,   125,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,   127,     0,     0,   128,    69,     0,     0,    70,    71,
      72,    73,    74,     0,   131,   132,     0,     0,     0,   133,
       0,     0,    75,    76,    77,    78,     0,    79,   210,    81,
     211,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,     0,    87,    88,     0,    89,    90,    91,
       0,    92,     0,    93,     0,     0,    94,    95,    96,    97,
       0,    98,     0,    99,     0,   100,   101,     0,     0,   102,
     103,   137,     0,   105,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   138,     0,     0,     0,   140,
       0,     0,   301,   302,     0,   142,   209,   117,   118,   119,
       0,   120,   121,     0,     0,     0,     0,   123,     0,    62,
       0,     0,     0,    63,   124,    65,     0,   125,    66,     0,
       0,     0,    67,     0,     0,     0,   126,     0,     0,     0,
       0,     0,     0,     0,   127,     0,     0,   128,    69,     0,
       0,    70,    71,    72,   129,   130,     0,   131,   132,     0,
       0,     0,   133,     0,     0,    75,    76,    77,    78,     0,
      79,   134,    81,   135,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,     0,    87,    88,     0,
      89,    90,    91,     0,    92,     0,    93,     0,     0,    94,
      95,    96,   136,     0,    98,     0,    99,     0,   100,   101,
       0,     0,   102,   103,   137,     0,   105,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   138,   139,
       0,     0,   140,     0,     0,   141,     0,     0,   142,   116,
     117,   118,   119,     0,   120,   121,     0,     0,     0,     0,
     123,     0,    62,     0,     0,     0,    63,   124,    65,     0,
     125,    66,     0,     0,     0,    67,     0,     0,     0,   126,
       0,     0,     0,     0,     0,     0,     0,   127,     0,     0,
     128,    69,     0,     0,    70,    71,    72,   129,   130,     0,
     131,   132,     0,     0,     0,   133,     0,     0,    75,    76,
      77,    78,     0,    79,   134,    81,   135,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,     0,
      87,    88,     0,    89,    90,    91,     0,    92,     0,    93,
       0,     0,    94,    95,    96,   136,     0,    98,     0,    99,
       0,   100,   101,     0,     0,   102,   103,   137,     0,   105,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   138,   139,     0,     0,   140,     0,     0,   141,     0,
       0,   142,   209,   117,   118,   119,     0,   120,   121,     0,
       0,     0,     0,   123,     0,    62,     0,     0,     0,    63,
      64,    65,     0,   125,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
     127,     0,     0,   128,    69,     0,     0,    70,    71,    72,
      73,    74,     0,   131,   132,     0,     0,     0,   133,     0,
       0,    75,    76,    77,    78,     0,    79,   210,    81,   211,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,     0,    87,    88,     0,    89,    90,    91,   424,
      92,     0,    93,     0,     0,    94,    95,    96,    97,     0,
      98,     0,    99,     0,   100,   101,     0,     0,   102,   103,
     137,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   138,     0,     0,     0,   140,     0,
       0,   141,     0,     0,   142,   209,   117,   118,   119,     0,
     300,   121,     0,     0,     0,     0,   123,     0,    62,     0,
       0,     0,    63,    64,    65,     0,   125,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,   127,     0,     0,   128,    69,     0,     0,
      70,    71,    72,    73,    74,     0,   131,   132,     0,     0,
       0,   133,     0,     0,    75,    76,    77,    78,     0,    79,
     210,    81,   211,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,     0,    87,    88,     0,    89,
      90,    91,     0,    92,     0,    93,     0,     0,    94,    95,
      96,    97,     0,    98,     0,    99,     0,   100,   101,     0,
       0,   102,   103,   137,     0,   105,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   138,     0,     0,
       0,   140,     0,     0,   141,   302,     0,   142,   209,   117,
     118,   119,     0,   120,   121,     0,     0,     0,     0,   123,
       0,    62,     0,     0,     0,    63,    64,    65,     0,   125,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,   127,     0,     0,   128,
      69,     0,     0,    70,    71,    72,    73,    74,     0,   131,
     132,     0,     0,     0,   133,     0,     0,    75,    76,    77,
      78,     0,    79,   210,    81,   211,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,     0,    87,
      88,     0,    89,    90,    91,     0,    92,     0,    93,     0,
       0,    94,    95,    96,    97,     0,    98,     0,    99,     0,
     100,   101,     0,     0,   102,   103,   137,     0,   105,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     138,     0,     0,     0,   140,     0,     0,   141,     0,     0,
     142,   209,   117,   118,   119,     0,   300,   121,     0,     0,
       0,     0,   123,     0,    62,     0,     0,     0,    63,    64,
      65,     0,   125,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,   127,
       0,     0,   128,    69,     0,     0,    70,    71,    72,    73,
      74,     0,   131,   132,     0,     0,     0,   133,     0,     0,
      75,    76,    77,    78,     0,    79,   210,    81,   211,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,     0,    87,    88,     0,    89,    90,    91,     0,    92,
       0,    93,     0,     0,    94,    95,    96,    97,     0,    98,
       0,    99,     0,   100,   101,     0,     0,   102,   103,   137,
       0,   105,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   138,     0,     0,     0,   140,     0,     0,
     141,     0,     0,   142,   209,   117,   118,   119,     0,   329,
     121,     0,     0,     0,     0,   123,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   125,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,   127,     0,     0,   128,    69,     0,     0,    70,
      71,    72,    73,    74,     0,   131,   132,     0,     0,     0,
     133,     0,     0,    75,    76,    77,    78,     0,    79,   210,
      81,   211,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,     0,    87,    88,     0,    89,    90,
      91,     0,    92,     0,    93,     0,     0,    94,    95,    96,
      97,     0,    98,     0,    99,     0,   100,   101,     0,     0,
     102,   103,   137,     0,   105,     0,     0,     0,     0,     0,
       0,    61,   407,     0,     0,     0,   138,     0,     0,     0,
     140,     0,     0,   141,    62,     0,   142,     0,    63,    64,
      65,     0,   408,    66,     0,     0,     0,    67,     0,     0,
       0,   409,     0,     0,     0,     0,     0,     0,     0,   410,
       0,     0,     0,    69,     0,     0,    70,    71,    72,   411,
      74,     0,   412,     0,     0,     0,     0,   413,     0,     0,
      75,    76,    77,    78,     0,   414,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,     0,    87,    88,     0,    89,    90,    91,     0,    92,
       0,    93,     0,     0,    94,    95,    96,    97,     0,    98,
       0,    99,     0,   100,   101,     0,     0,   102,   103,   415,
      61,   105,   675,     0,     0,   676,     0,     0,     0,     0,
       0,     0,     0,    62,     0,     0,     0,    63,    64,    65,
     416,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
       0,    87,    88,     0,    89,    90,    91,     0,    92,     0,
      93,     0,     0,    94,    95,    96,    97,     0,    98,     0,
      99,    61,   100,   101,     0,     0,   102,   103,   104,     0,
     105,     0,     0,     0,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,   677,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,     0,    87,    88,     0,    89,    90,    91,     0,    92,
       0,    93,     0,     0,    94,    95,    96,    97,     0,    98,
       0,    99,     0,   100,   101,     0,     0,   102,   103,   104,
      61,   105,   285,     0,     0,   364,     0,     0,     0,     0,
       0,     0,     0,    62,     0,     0,     0,    63,    64,    65,
     332,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     365,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,   366,     0,     0,     0,    83,    84,    85,    86,
       0,    87,    88,     0,    89,    90,    91,     0,    92,     0,
      93,     0,     0,    94,    95,    96,    97,     0,    98,     0,
      99,   367,   100,   101,     0,     0,   102,   103,   104,     0,
     105,    61,     0,   285,     0,     0,     0,     0,     0,     0,
       0,     0,   368,     0,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,     0,    87,    88,     0,    89,    90,    91,     0,    92,
       0,    93,     0,     0,    94,    95,    96,    97,     0,    98,
       0,    99,     0,   100,   101,     0,     0,   102,   103,   104,
       0,   105,    61,   407,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   368,     0,    62,     0,     0,     0,    63,
      64,    65,     0,   408,    66,     0,     0,     0,    67,     0,
       0,     0,   409,     0,     0,     0,     0,     0,     0,     0,
     410,     0,     0,     0,    69,     0,     0,    70,    71,    72,
     411,    74,     0,   412,     0,     0,     0,     0,   413,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,     0,    87,    88,     0,    89,    90,    91,     0,
      92,     0,    93,     0,     0,    94,    95,    96,    97,     0,
      98,     0,    99,     0,   100,   101,    61,   407,   102,   103,
     415,     0,   105,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   408,    66,     0,
       0,     0,    67,     0,     0,     0,   409,     0,     0,     0,
       0,     0,     0,     0,   410,     0,     0,     0,    69,     0,
       0,    70,    71,    72,   411,    74,     0,   412,     0,     0,
       0,     0,   413,     0,     0,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,   702,    86,     0,    87,    88,     0,
      89,    90,    91,     0,    92,     0,    93,     0,     0,    94,
      95,    96,    97,     0,    98,     0,    99,     0,   100,   101,
       0,     0,   102,   103,   415,    61,   105,   255,   256,     0,
     257,     0,     0,     0,     0,     0,     0,     0,    62,     0,
       0,     0,    63,    64,    65,     0,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,   258,     0,     0,
       0,    83,    84,    85,    86,     0,    87,    88,     0,    89,
      90,    91,     0,    92,     0,    93,     0,     0,    94,    95,
      96,    97,     0,    98,     0,    99,     0,   100,   101,     0,
       0,   102,   103,   104,    61,   105,   255,   256,     0,   457,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,    82,   258,     0,     0,     0,
      83,    84,    85,    86,     0,    87,    88,     0,    89,    90,
      91,     0,    92,     0,    93,     0,     0,    94,    95,    96,
      97,     0,    98,    61,    99,   634,   100,   101,     0,     0,
     102,   103,   104,     0,   105,     0,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,   635,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,     0,    87,    88,     0,    89,    90,    91,
       0,    92,     0,    93,     0,     0,    94,    95,    96,    97,
       0,    98,     0,    99,     0,   100,   101,    61,     0,   102,
     103,   104,     0,   105,   153,     0,     0,     0,     0,     0,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,   154,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,   155,    72,    73,    74,     0,     0,   542,
     543,     0,     0,     0,     0,     0,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,   156,   544,
       0,     0,     0,    83,    84,    85,    86,     0,    87,    88,
       0,    89,    90,    91,   545,    92,     0,   157,   546,     0,
      94,    95,    96,    97,     0,    98,    61,   547,   548,   100,
     101,     0,     0,   102,   103,   104,     0,   105,     0,    62,
       0,     0,     0,    63,    64,    65,     0,     0,    66,     0,
       0,     0,    67,   549,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,     0,   215,     0,
       0,     0,     0,     0,     0,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,     0,    87,    88,     0,
      89,    90,    91,     0,    92,     0,    93,     0,     0,    94,
      95,    96,    97,     0,    98,    61,    99,     0,   100,   101,
       0,     0,   102,   103,   104,     0,   105,     0,    62,     0,
     226,     0,    63,    64,    65,     0,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,     0,    87,    88,     0,    89,
      90,    91,     0,    92,     0,    93,     0,     0,    94,    95,
      96,    97,     0,    98,    61,    99,     0,   100,   101,   264,
       0,   102,   103,   104,     0,   105,     0,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,     0,    87,    88,     0,    89,    90,
      91,     0,    92,     0,    93,     0,     0,    94,   265,    96,
      97,     0,    98,    61,    99,     0,   100,   101,     0,     0,
     102,   103,   104,     0,   105,     0,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,   473,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,     0,    87,    88,     0,    89,    90,    91,
       0,    92,     0,    93,     0,     0,    94,    95,    96,    97,
       0,    98,    61,    99,     0,   100,   101,     0,     0,   102,
     103,   104,     0,   105,     0,    62,     0,     0,     0,    63,
      64,    65,     0,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,   609,    87,    88,     0,    89,    90,    91,     0,
      92,     0,    93,     0,     0,    94,    95,    96,    97,     0,
      98,    61,    99,     0,   100,   101,     0,     0,   102,   103,
     104,     0,   105,     0,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,   618,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,     0,    87,    88,     0,    89,    90,    91,     0,    92,
       0,    93,     0,     0,    94,    95,    96,    97,     0,    98,
      61,    99,     0,   100,   101,   673,     0,   102,   103,   104,
       0,   105,     0,    62,     0,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
       0,    87,    88,     0,    89,    90,    91,     0,    92,     0,
      93,     0,     0,    94,    95,    96,    97,     0,    98,    61,
      99,     0,   100,   101,     0,     0,   102,   103,   104,     0,
     105,     0,    62,     0,     0,     0,    63,    64,    65,     0,
       0,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,     0,
      87,    88,     0,    89,    90,    91,     0,    92,     0,    93,
       0,     0,    94,    95,    96,    97,     0,    98,    61,    99,
       0,   100,   101,     0,   253,   102,   103,   104,     0,   105,
       0,    62,     0,     0,     0,    63,    64,    65,     0,     0,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,   229,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,     0,    79,    80,    81,     0,   229,   230,     0,     0,
       0,     0,     0,     0,    83,    84,    85,    86,     0,    87,
      88,     0,    89,    90,    91,     0,    92,     0,    93,     0,
       0,    94,    95,    96,    97,   229,    98,   230,  -369,     0,
     100,   101,     0,     0,   102,   103,   104,     0,   105,     0,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   229,   230,     0,     0,     0,
     441,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   229,   230,     0,     0,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   229,     0,   230,     0,   429,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   229,   230,     0,     0,   435,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,     0,   230,     0,     0,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   229,     0,     0,     0,   591,     0,   426,
       0,     0,     0,     0,     0,     0,     0,     0,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,     1,   230,     0,     2,   592,     3,     0,
       0,     0,     0,     4,     0,     0,     0,     5,     0,     0,
       6,     0,     7,     8,     9,     0,     0,     0,    10,     0,
       0,    11,   229,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,     0,     0,   229,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,     0,   230,    13,   229,     0,     0,     0,     0,
       0,     0,    14,     0,    15,     0,    16,   230,    17,     0,
      18,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,    20,     0,    21,     0,     0,   230,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,     0,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,     0,     0,     0,     0,     0,
       0,     0,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245
};

static const short yycheck[] =
{
       4,   399,   166,   456,    21,    17,   170,   171,     0,   469,
     416,   185,   472,    47,     9,    39,    47,    21,   182,     5,
       5,     9,     8,    75,    32,     5,     5,    75,   100,     5,
       6,     5,     6,    65,     8,    39,   333,    90,    14,   499,
      14,    94,   502,   503,   504,   505,   506,   507,    40,    13,
      14,     5,     6,    92,     8,    59,    60,    13,    14,    48,
      14,     5,     6,     5,     6,    56,     8,    65,   112,    18,
      14,    19,    14,     5,     6,   528,     8,   129,    48,    71,
     112,   129,    14,   251,   252,     5,     6,   384,   262,    37,
     134,    56,   498,   110,    83,   134,   112,   276,   106,    97,
     134,   265,   126,   134,   108,   109,   110,   111,   112,   113,
     114,   115,    89,    83,    82,   133,   134,   570,   134,   298,
     103,    89,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   132,   142,   125,
     125,   539,   146,   155,   132,   125,   125,   133,   133,   125,
      47,   125,   156,   133,   328,    40,   160,   555,   132,   688,
     689,   165,   118,    16,    23,   134,    40,   136,   628,   629,
     644,   125,   176,     0,   138,    24,   131,   181,   132,    60,
     633,   125,   138,   125,   133,   134,    71,   115,   132,   663,
     137,   195,   132,   125,   362,    44,    55,    71,   133,   134,
      40,   132,    61,    56,    66,   133,   133,   134,   132,    58,
     374,   132,   376,   687,   688,   689,    69,   132,    16,   132,
     224,   132,   226,   132,   698,   399,    79,    80,    84,    85,
     132,    71,   133,   134,   132,    91,   132,   416,   132,   124,
     125,   126,   127,   128,    97,   133,   134,   251,   252,    98,
     133,   134,   126,   127,   128,   653,   132,   110,   262,   118,
     119,   120,   121,   122,   123,   133,   134,   133,   134,    40,
     129,   132,   276,   277,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   132,
     197,   470,   199,   200,   298,   469,   203,   204,   472,   139,
      71,   133,   134,   133,   134,    61,   310,   481,   133,   134,
     118,   490,   118,   492,    92,   494,   133,   134,   103,   498,
     133,   134,   326,   118,   328,   499,   133,   134,   502,   503,
     504,   505,   506,   507,   133,   134,   103,    92,    65,   109,
     138,   515,   140,   141,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   362,   363,
      15,   102,    87,    95,   133,   539,   118,   133,   132,   132,
     132,     5,   118,   118,   125,    62,   380,    97,    97,     8,
      97,   555,   132,    30,   388,    89,   390,   391,     8,   118,
      40,   134,   139,   133,    75,   134,   194,   132,   196,     5,
      64,     8,   118,   108,   202,   133,    95,   205,   206,   207,
     132,   590,   416,   115,   132,    90,     5,   132,   132,   132,
     132,    71,   426,   221,   132,   118,   133,   225,    50,   125,
     133,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   125,   118,
      76,   114,   250,   132,   628,   629,     8,     8,   134,   126,
     133,    40,     8,   133,    55,   644,   470,   646,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   653,
     278,   134,    40,    55,   663,   133,   490,     5,   492,   113,
     494,    84,    71,     8,   498,   134,   133,   133,   133,   115,
     133,   133,   115,   301,    75,   133,    50,    77,   687,   688,
     689,   515,    26,    71,    53,   118,   118,   134,    18,   698,
      40,    26,   134,   527,   322,   323,   324,    77,   532,   133,
      66,     5,   706,   134,   538,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     554,    71,    77,    26,   133,   134,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    26,   132,   132,     8,   133,   134,   134,   134,     8,
      40,   118,     5,   132,   588,   133,   590,   661,   133,   646,
     133,   301,   180,   451,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   698,
     516,    71,   671,   690,   134,   225,   651,   708,   428,   363,
     326,   538,   515,   617,   362,   399,   424,   527,   470,   555,
      18,    -1,   222,    -1,    -1,   235,    16,   641,    -1,    -1,
     644,   439,   646,   441,    -1,   649,   650,   651,    28,    29,
      -1,    -1,    -1,    -1,    -1,    35,    -1,   661,    -1,   663,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    49,
      -1,    -1,    -1,   677,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,   687,   688,   689,    -1,    -1,    -1,    69,
       3,     4,     5,     6,   698,     8,     9,    10,    -1,    79,
      80,    14,    -1,    16,   708,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    93,    -1,    -1,    29,    97,   516,    -1,
      33,   101,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
     110,    44,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    85,    -1,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    -1,    96,    97,    98,    99,    -1,   101,    -1,
     103,    -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,
     113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,    -1,   129,    -1,    -1,   132,
      -1,    -1,   135,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,    59,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    -1,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,    99,
      -1,   101,    -1,   103,    -1,   105,   106,    -1,    -1,   109,
     110,   111,    -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,   129,
      -1,    -1,   132,   133,    -1,   135,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    54,    55,    -1,
      -1,    -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    85,    -1,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,
      97,    98,    99,    -1,   101,    -1,   103,    -1,   105,   106,
      -1,    -1,   109,   110,   111,    -1,   113,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,    -1,   129,    -1,    -1,   132,    -1,    -1,   135,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    85,    -1,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    -1,    96,    97,    98,    99,    -1,   101,    -1,   103,
      -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,   113,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,    -1,   129,    -1,    -1,   132,    -1,
      -1,   135,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    44,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    85,    -1,    87,    88,    89,    90,
      91,    -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,
     101,    -1,   103,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,   129,    -1,
      -1,   132,    -1,    -1,   135,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    54,    55,    -1,    -1,
      -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    85,    -1,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,
      98,    99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,
      -1,   109,   110,   111,    -1,   113,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,
      -1,   129,    -1,    -1,   132,   133,    -1,   135,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    44,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      85,    -1,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      -1,    96,    97,    98,    99,    -1,   101,    -1,   103,    -1,
     105,   106,    -1,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,    -1,    -1,    -1,   129,    -1,    -1,   132,    -1,    -1,
     135,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    44,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    85,    -1,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,   101,
      -1,   103,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,    -1,    -1,    -1,   129,    -1,    -1,
     132,    -1,    -1,   135,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    -1,   113,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,    -1,    -1,   125,    -1,    -1,    -1,
     129,    -1,    -1,   132,    16,    -1,   135,    -1,    20,    21,
      22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    85,    -1,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,   101,
      -1,   103,    -1,   105,   106,    -1,    -1,   109,   110,   111,
       3,   113,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
     132,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    85,    -1,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    -1,    96,    97,    98,    99,    -1,   101,    -1,
     103,     3,   105,   106,    -1,    -1,   109,   110,   111,    -1,
     113,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,   132,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    85,    -1,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,   101,
      -1,   103,    -1,   105,   106,    -1,    -1,   109,   110,   111,
       3,   113,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
     132,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    85,    -1,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    -1,    96,    97,    98,    99,    -1,   101,    -1,
     103,   104,   105,   106,    -1,    -1,   109,   110,   111,    -1,
     113,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    85,    -1,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,   101,
      -1,   103,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      -1,   113,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    59,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    85,    -1,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,
     101,    -1,   103,    -1,   105,   106,     3,     4,   109,   110,
     111,    -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    85,    -1,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,
      97,    98,    99,    -1,   101,    -1,   103,    -1,   105,   106,
      -1,    -1,   109,   110,   111,     3,   113,     5,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    85,    -1,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,
      98,    99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,
      -1,   109,   110,   111,     3,   113,     5,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,     3,   103,     5,   105,   106,    -1,    -1,
     109,   110,   111,    -1,   113,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    -1,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,    99,
      -1,   101,    -1,   103,    -1,   105,   106,     3,    -1,   109,
     110,   111,    -1,   113,    10,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    28,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,    24,
      25,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    44,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    85,
      -1,    87,    88,    89,    59,    91,    -1,    93,    63,    -1,
      96,    97,    98,    99,    -1,   101,     3,    72,    73,   105,
     106,    -1,    -1,   109,   110,   111,    -1,   113,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    98,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    85,    -1,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,
      97,    98,    99,    -1,   101,     3,   103,    -1,   105,   106,
      -1,    -1,   109,   110,   111,    -1,   113,    -1,    16,    -1,
      18,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    85,    -1,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,
      98,    99,    -1,   101,     3,   103,    -1,   105,   106,     8,
      -1,   109,   110,   111,    -1,   113,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,     3,   103,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    -1,   113,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    -1,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,    99,
      -1,   101,     3,   103,    -1,   105,   106,    -1,    -1,   109,
     110,   111,    -1,   113,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    -1,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,
     101,     3,   103,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,   113,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    85,    -1,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,   101,
       3,   103,    -1,   105,   106,     8,    -1,   109,   110,   111,
      -1,   113,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    85,    -1,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    -1,    96,    97,    98,    99,    -1,   101,     3,
     103,    -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,
     113,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    85,    -1,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    -1,    96,    97,    98,    99,    -1,   101,     3,   103,
      -1,   105,   106,    -1,     9,   109,   110,   111,    -1,   113,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    40,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      85,    -1,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      -1,    96,    97,    98,    99,    40,   101,    71,   103,    -1,
     105,   106,    -1,    -1,   109,   110,   111,    -1,   113,    -1,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    40,    71,    -1,    -1,    -1,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    40,    71,    -1,    -1,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    40,    -1,    71,    -1,   133,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    40,    71,    -1,    -1,   133,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,    71,    -1,    -1,   133,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    40,    -1,    -1,    -1,   133,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    17,    71,    -1,    20,   133,    22,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    31,    -1,    -1,
      34,    -1,    36,    37,    38,    -1,    -1,    -1,    42,    -1,
      -1,    45,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    40,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,    71,    78,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    88,    -1,    90,    71,    92,    -1,
      94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,   107,    -1,    -1,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    78,    86,    88,    90,    92,    94,    96,
     105,   107,   141,   142,   145,   146,   147,   148,   155,   196,
     199,   200,   205,   206,   207,   208,   216,   217,   226,   227,
     228,   229,   230,   234,   235,   240,   241,   242,   245,   247,
     248,   249,   250,   251,   252,   253,   255,   256,   257,   100,
      56,     3,    16,    20,    21,    22,    25,    29,    33,    45,
      48,    49,    50,    51,    52,    62,    63,    64,    65,    67,
      68,    69,    74,    79,    80,    81,    82,    84,    85,    87,
      88,    89,    91,    93,    96,    97,    98,    99,   101,   103,
     105,   106,   109,   110,   111,   113,   143,   144,    48,    83,
      47,    48,    83,    82,    89,    56,     3,     4,     5,     6,
       8,     9,    10,    14,    21,    24,    33,    41,    44,    51,
      52,    54,    55,    59,    68,    70,    99,   111,   125,   126,
     129,   132,   135,   144,   156,   157,   159,   189,   190,   254,
     258,   261,   262,    10,    28,    49,    74,    93,   143,   243,
      16,    28,    29,    35,    49,    56,    69,    79,    80,    93,
      97,   101,   110,   198,   243,   103,    89,   144,   160,     0,
     131,    60,   144,   144,   144,   132,   144,   144,   160,   144,
     144,   144,   144,   144,   132,   137,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,     3,
      68,    70,   189,   189,   189,    55,   144,   193,   194,    13,
      14,   138,   259,   260,    47,   134,    18,   144,   158,    40,
      71,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    66,   181,   182,    61,
     118,   118,    92,     9,   143,     5,     6,     8,    75,   144,
     203,   204,   118,   103,     8,    97,   144,    92,    65,   197,
     144,   197,   197,   197,   109,   144,    92,   134,    90,    94,
     146,   144,   197,    15,   102,     5,     6,     8,    14,   125,
     132,   168,   169,   215,   218,   219,    87,    95,   112,   162,
       8,   132,   133,   189,   191,   192,   144,   189,   189,   191,
      39,   126,   191,   191,   133,   189,   191,   191,   189,   189,
     189,   133,   132,   132,   132,   133,   134,   136,   118,     8,
     189,   260,   132,   160,   157,   189,   144,   189,   189,   189,
     189,   189,   189,   189,   262,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,     5,    75,   129,   189,   203,
     203,   118,   118,   125,     8,    43,    75,   104,   125,   144,
     168,   201,   202,    62,    97,   197,    97,     8,    97,    65,
     112,   236,   237,   144,   231,   232,   258,   144,   132,   209,
      30,    89,     5,     6,     8,   133,   168,   170,   220,   134,
     221,    24,    44,    58,    98,   246,     8,     4,    24,    33,
      41,    51,    54,    59,    67,   111,   132,   144,   163,   164,
     165,   166,   167,   258,    90,   148,    46,   133,   134,   133,
     133,   144,   133,   133,   133,   133,   133,   133,   133,   134,
     133,   134,   133,   189,   189,   194,   144,   168,   195,   139,
     139,   152,   161,   162,   134,    75,   132,     8,   144,   201,
     203,   204,    64,   197,   197,     8,   144,   238,   239,   118,
     134,   162,   118,    54,   144,   210,   211,   108,   144,   144,
     133,   134,   133,   134,   215,   219,   222,   223,   133,    95,
     132,   132,   132,   132,   132,   132,   132,   163,   115,    23,
      55,    61,   118,   119,   120,   121,   122,   123,   129,   133,
     144,   192,   133,   189,   189,   118,    90,   155,    50,   171,
       5,   170,    84,    85,    91,   244,   118,   114,   132,   168,
     169,   232,    76,   183,   184,   168,   169,   133,   134,   132,
     212,   213,    24,    25,    44,    59,    63,    72,    73,    98,
     233,   168,     8,    18,   224,   134,     8,   144,   258,   126,
     258,   133,   258,     8,   133,   133,   163,   168,   169,     9,
     132,    75,   129,     8,   168,   169,     8,   168,   169,   168,
     169,   168,   169,   168,   169,   168,   169,    55,   134,   149,
      55,   133,   133,   195,   156,   133,     5,   172,   113,   175,
     176,   133,    32,   106,    84,     8,   239,   133,   170,    83,
     144,   185,   186,   210,   132,   214,   215,   134,    66,   144,
     225,   215,   223,   133,   133,   133,   133,   133,   115,   115,
     170,    75,     9,   132,     5,    54,   144,   150,   151,   133,
     258,    47,    77,   153,    26,    50,    53,   174,   133,   118,
     118,   134,   133,   134,   213,   168,   169,   168,   169,   133,
     170,   134,   133,    26,    66,   154,   167,   173,    77,   165,
      77,   177,   178,     8,   144,     5,     8,   132,   144,   186,
     215,   133,   151,   167,   179,   180,     5,   134,    26,    26,
     181,   132,   144,   187,   188,   132,    19,    37,   134,   134,
     167,   179,    81,   179,   183,     8,   118,   133,   134,     8,
     180,     5,   132,   133,   168,   188,   133,   133
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

  case 77:

    { pParser->PushQuery(); ;}
    break;

  case 78:

    { pParser->PushQuery(); ;}
    break;

  case 82:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 84:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 87:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 88:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 92:

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

  case 93:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 95:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 96:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 97:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 100:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 103:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 104:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 105:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 106:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 107:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 108:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 109:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 110:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 111:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 112:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 113:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 115:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 122:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 124:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 125:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 126:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 127:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 128:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 129:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 130:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 131:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 132:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 133:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 135:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 136:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 151:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 152:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 153:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 154:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 159:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 160:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 161:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 162:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 163:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 164:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 165:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 169:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 170:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 171:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 173:

    {
			pParser->AddHaving();
		;}
    break;

  case 176:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 179:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 180:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 182:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 188:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 189:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 195:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 196:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 197:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 198:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 199:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 200:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 201:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 202:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 203:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 204:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 205:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 207:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 208:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 213:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 214:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 215:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 216:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 217:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 218:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 219:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 220:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 221:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 222:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 223:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 224:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 225:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 226:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 227:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 228:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 229:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 230:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 231:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 232:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 233:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 237:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 238:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 239:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 240:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 241:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 242:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 243:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 244:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 245:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 247:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 248:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 249:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 254:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 255:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 262:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 263:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 264:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 265:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 266:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 267:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 268:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 269:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 270:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 271:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 272:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 273:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 274:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 275:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 276:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 277:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 278:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 279:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 280:

    {
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
		;}
    break;

  case 283:

    { yyval.m_iValue = 1; ;}
    break;

  case 284:

    { yyval.m_iValue = 0; ;}
    break;

  case 285:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 293:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 294:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 295:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 298:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 299:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 300:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 305:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 306:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 309:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 310:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 311:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 312:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 313:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 314:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 315:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 316:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 317:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 318:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 319:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 320:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 322:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 323:

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

  case 324:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 327:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 329:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 334:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 337:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 338:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 339:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 342:

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

  case 343:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 344:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 345:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 346:

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

  case 347:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 348:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 349:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 350:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 351:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 352:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 353:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 354:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 355:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 356:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 357:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 358:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 365:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 366:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 367:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 375:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 376:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 377:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 378:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 379:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 380:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[-2] );
			pParser->ToStringUnescape ( tStmt.m_sUdrLib, yyvsp[0] );
		;}
    break;

  case 381:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[0] );
		;}
    break;

  case 382:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 383:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 384:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 385:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 386:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 389:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 390:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 391:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 392:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 394:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 395:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 396:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 397:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 398:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 399:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 400:

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

