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
     TOK_GTE = 385,
     TOK_LTE = 386,
     TOK_NOT = 387,
     TOK_NEG = 388
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
#define TOK_ALL 273
#define TOK_ANY 274
#define TOK_AS 275
#define TOK_ASC 276
#define TOK_ATTACH 277
#define TOK_ATTRIBUTES 278
#define TOK_AVG 279
#define TOK_BEGIN 280
#define TOK_BETWEEN 281
#define TOK_BIGINT 282
#define TOK_BOOL 283
#define TOK_BY 284
#define TOK_CALL 285
#define TOK_CHARACTER 286
#define TOK_CHUNK 287
#define TOK_COLLATION 288
#define TOK_COLUMN 289
#define TOK_COMMIT 290
#define TOK_COMMITTED 291
#define TOK_COUNT 292
#define TOK_CREATE 293
#define TOK_DATABASES 294
#define TOK_DELETE 295
#define TOK_DESC 296
#define TOK_DESCRIBE 297
#define TOK_DISTINCT 298
#define TOK_DIV 299
#define TOK_DOUBLE 300
#define TOK_DROP 301
#define TOK_FACET 302
#define TOK_FALSE 303
#define TOK_FLOAT 304
#define TOK_FLUSH 305
#define TOK_FOR 306
#define TOK_FROM 307
#define TOK_FUNCTION 308
#define TOK_GLOBAL 309
#define TOK_GROUP 310
#define TOK_GROUPBY 311
#define TOK_GROUP_CONCAT 312
#define TOK_HAVING 313
#define TOK_ID 314
#define TOK_IN 315
#define TOK_INDEX 316
#define TOK_INDEXOF 317
#define TOK_INSERT 318
#define TOK_INT 319
#define TOK_INTEGER 320
#define TOK_INTO 321
#define TOK_IS 322
#define TOK_ISOLATION 323
#define TOK_JSON 324
#define TOK_LEVEL 325
#define TOK_LIKE 326
#define TOK_LIMIT 327
#define TOK_MATCH 328
#define TOK_MAX 329
#define TOK_META 330
#define TOK_MIN 331
#define TOK_MOD 332
#define TOK_MULTI 333
#define TOK_MULTI64 334
#define TOK_NAMES 335
#define TOK_NULL 336
#define TOK_OPTION 337
#define TOK_ORDER 338
#define TOK_OPTIMIZE 339
#define TOK_PLAN 340
#define TOK_PLUGIN 341
#define TOK_PLUGINS 342
#define TOK_PROFILE 343
#define TOK_RAND 344
#define TOK_RAMCHUNK 345
#define TOK_READ 346
#define TOK_RECONFIGURE 347
#define TOK_RELOAD 348
#define TOK_REPEATABLE 349
#define TOK_REPLACE 350
#define TOK_REMAP 351
#define TOK_RETURNS 352
#define TOK_ROLLBACK 353
#define TOK_RTINDEX 354
#define TOK_SELECT 355
#define TOK_SERIALIZABLE 356
#define TOK_SET 357
#define TOK_SETTINGS 358
#define TOK_SESSION 359
#define TOK_SHOW 360
#define TOK_SONAME 361
#define TOK_START 362
#define TOK_STATUS 363
#define TOK_STRING 364
#define TOK_SUM 365
#define TOK_TABLE 366
#define TOK_TABLES 367
#define TOK_THREADS 368
#define TOK_TO 369
#define TOK_TRANSACTION 370
#define TOK_TRUE 371
#define TOK_TRUNCATE 372
#define TOK_TYPE 373
#define TOK_UNCOMMITTED 374
#define TOK_UPDATE 375
#define TOK_VALUES 376
#define TOK_VARIABLES 377
#define TOK_WARNINGS 378
#define TOK_WEIGHT 379
#define TOK_WHERE 380
#define TOK_WITHIN 381
#define TOK_OR 382
#define TOK_AND 383
#define TOK_NE 384
#define TOK_GTE 385
#define TOK_LTE 386
#define TOK_NOT 387
#define TOK_NEG 388




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

static CSphFilterSettings * AddMvaRange ( SqlParser_c * pParser, const SqlNode_t & tNode, int64_t iMin, int64_t iMax )
{
	CSphFilterSettings * f = pParser->AddFilter ( tNode, SPH_FILTER_RANGE );
	f->m_eMvaFunc = ( tNode.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
	f->m_iMinValue = iMin;
	f->m_iMaxValue = iMax;
	return f;
}

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	if ( _res.m_iStart>0 && pParser->m_pBuf[_res.m_iStart-1]=='`' ) \
		_res.m_iStart--; \
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
#define YYFINAL  251
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5041

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  153
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  141
/* YYNRULES -- Number of rules. */
#define YYNRULES  511
/* YYNRULES -- Number of states. */
#define YYNSTATES  908

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   388

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   141,   130,     2,
     145,   146,   139,   137,   147,   138,   150,   140,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   144,
     133,   131,   134,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   151,     2,   152,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   148,   129,   149,     2,     2,     2,     2,
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
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   132,   135,   136,   142,   143
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
     140,   142,   144,   146,   148,   150,   152,   154,   156,   158,
     160,   162,   164,   166,   168,   170,   172,   174,   176,   178,
     180,   182,   184,   186,   188,   190,   192,   194,   196,   198,
     200,   202,   204,   206,   208,   210,   212,   214,   216,   218,
     220,   222,   224,   226,   228,   230,   232,   234,   236,   238,
     240,   242,   244,   246,   248,   250,   252,   254,   256,   258,
     260,   262,   264,   266,   270,   273,   275,   277,   279,   288,
     290,   300,   301,   304,   306,   310,   312,   314,   316,   317,
     321,   322,   325,   330,   342,   344,   348,   350,   353,   354,
     356,   359,   361,   366,   371,   376,   381,   386,   391,   395,
     401,   403,   407,   408,   410,   413,   415,   419,   423,   428,
     430,   434,   438,   444,   451,   457,   464,   468,   473,   479,
     486,   490,   494,   498,   502,   506,   510,   516,   522,   528,
     532,   536,   540,   544,   548,   552,   556,   561,   565,   569,
     575,   582,   588,   595,   599,   603,   607,   611,   613,   615,
     620,   624,   628,   630,   632,   637,   642,   647,   651,   656,
     661,   663,   666,   668,   671,   673,   675,   679,   681,   685,
     686,   691,   692,   694,   696,   700,   701,   704,   705,   707,
     713,   714,   716,   720,   726,   728,   732,   734,   737,   740,
     741,   743,   746,   751,   752,   754,   757,   759,   763,   765,
     769,   773,   779,   786,   790,   792,   796,   800,   802,   804,
     806,   808,   810,   812,   814,   817,   820,   824,   828,   832,
     836,   840,   844,   848,   852,   856,   860,   864,   868,   872,
     876,   880,   884,   888,   892,   896,   898,   900,   902,   906,
     911,   916,   921,   926,   931,   936,   941,   945,   952,   959,
     963,   972,   987,   991,   996,   998,  1002,  1004,  1006,  1008,
    1010,  1012,  1016,  1022,  1024,  1026,  1028,  1030,  1033,  1034,
    1037,  1039,  1042,  1045,  1049,  1051,  1053,  1055,  1058,  1063,
    1068,  1072,  1077,  1082,  1084,  1086,  1087,  1090,  1095,  1100,
    1105,  1109,  1114,  1119,  1127,  1133,  1139,  1149,  1151,  1153,
    1155,  1157,  1159,  1161,  1165,  1167,  1169,  1171,  1173,  1175,
    1177,  1179,  1181,  1183,  1186,  1194,  1196,  1198,  1199,  1203,
    1205,  1207,  1209,  1213,  1215,  1219,  1223,  1225,  1229,  1231,
    1233,  1235,  1239,  1242,  1243,  1246,  1248,  1252,  1256,  1261,
    1268,  1270,  1274,  1276,  1280,  1282,  1286,  1287,  1290,  1292,
    1296,  1300,  1301,  1303,  1305,  1307,  1311,  1313,  1315,  1319,
    1323,  1330,  1332,  1336,  1340,  1344,  1350,  1355,  1359,  1363,
    1365,  1367,  1369,  1371,  1373,  1375,  1377,  1379,  1381,  1389,
    1396,  1401,  1406,  1412,  1413,  1415,  1418,  1420,  1424,  1428,
    1431,  1435,  1442,  1443,  1445,  1447,  1450,  1453,  1456,  1458,
    1466,  1468,  1470,  1472,  1474,  1476,  1480,  1487,  1491,  1495,
    1498,  1502,  1504,  1508,  1511,  1515,  1519,  1527,  1533,  1539,
    1541,  1543,  1546,  1548,  1551,  1553,  1555,  1559,  1563,  1567,
    1571,  1573,  1574,  1577,  1579,  1582,  1584,  1586,  1590,  1596,
    1597,  1600
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     154,     0,    -1,   155,    -1,   158,    -1,   158,   144,    -1,
     224,    -1,   236,    -1,   216,    -1,   217,    -1,   222,    -1,
     237,    -1,   246,    -1,   248,    -1,   249,    -1,   250,    -1,
     255,    -1,   260,    -1,   261,    -1,   265,    -1,   267,    -1,
     268,    -1,   269,    -1,   270,    -1,   271,    -1,   262,    -1,
     272,    -1,   274,    -1,   275,    -1,   276,    -1,   254,    -1,
     277,    -1,   278,    -1,   279,    -1,   293,    -1,     3,    -1,
      15,    -1,    16,    -1,    18,    -1,    17,    -1,    19,    -1,
      21,    -1,    22,    -1,    23,    -1,    24,    -1,    25,    -1,
      26,    -1,    27,    -1,    28,    -1,    30,    -1,    31,    -1,
      32,    -1,    33,    -1,    34,    -1,    35,    -1,    36,    -1,
      37,    -1,    38,    -1,    39,    -1,    40,    -1,    41,    -1,
      42,    -1,    43,    -1,    45,    -1,    46,    -1,    49,    -1,
      50,    -1,    51,    -1,    53,    -1,    54,    -1,    55,    -1,
      57,    -1,    56,    -1,    58,    -1,    61,    -1,    62,    -1,
      63,    -1,    64,    -1,    65,    -1,    66,    -1,    68,    -1,
      69,    -1,    70,    -1,    71,    -1,    73,    -1,    74,    -1,
      75,    -1,    76,    -1,    78,    -1,    79,    -1,    84,    -1,
      82,    -1,    85,    -1,    86,    -1,    87,    -1,    88,    -1,
      90,    -1,    89,    -1,    91,    -1,    92,    -1,    96,    -1,
      94,    -1,    95,    -1,    97,    -1,    98,    -1,    99,    -1,
     101,    -1,   104,    -1,   102,    -1,   103,    -1,   105,    -1,
     106,    -1,   107,    -1,   108,    -1,   109,    -1,   110,    -1,
     111,    -1,   112,    -1,   113,    -1,   114,    -1,   117,    -1,
     118,    -1,   119,    -1,   120,    -1,   121,    -1,   122,    -1,
     123,    -1,   124,    -1,   125,    -1,   126,    -1,   156,    -1,
      80,    -1,   115,    -1,   159,    -1,   158,   144,   159,    -1,
     158,   291,    -1,   160,    -1,   211,    -1,   161,    -1,   100,
       3,   145,   145,   161,   146,   162,   146,    -1,   168,    -1,
     100,   169,    52,   145,   165,   168,   146,   166,   167,    -1,
      -1,   147,   163,    -1,   164,    -1,   163,   147,   164,    -1,
     157,    -1,     5,    -1,    59,    -1,    -1,    83,    29,   193,
      -1,    -1,    72,     5,    -1,    72,     5,   147,     5,    -1,
     100,   169,    52,   173,   174,   185,   189,   188,   191,   195,
     197,    -1,   170,    -1,   169,   147,   170,    -1,   139,    -1,
     172,   171,    -1,    -1,   157,    -1,    20,   157,    -1,   203,
      -1,    24,   145,   203,   146,    -1,    74,   145,   203,   146,
      -1,    76,   145,   203,   146,    -1,   110,   145,   203,   146,
      -1,    57,   145,   203,   146,    -1,    37,   145,   139,   146,
      -1,    56,   145,   146,    -1,    37,   145,    43,   157,   146,
      -1,   157,    -1,   173,   147,   157,    -1,    -1,   175,    -1,
     125,   176,    -1,   177,    -1,   176,   128,   176,    -1,   145,
     176,   146,    -1,    73,   145,     8,   146,    -1,   178,    -1,
     179,   131,   181,    -1,   179,   132,   181,    -1,   179,    60,
     145,   183,   146,    -1,   179,   142,    60,   145,   183,   146,
      -1,   179,    60,   145,   184,   146,    -1,   179,   142,    60,
     145,   184,   146,    -1,   179,    60,     9,    -1,   179,   142,
      60,     9,    -1,   179,    26,   181,   128,   181,    -1,   179,
     142,    26,   181,   128,   181,    -1,   179,   134,   181,    -1,
     179,   133,   181,    -1,   179,   135,   181,    -1,   179,   136,
     181,    -1,   179,   131,   182,    -1,   179,   132,   182,    -1,
     179,    26,   182,   128,   182,    -1,   179,    26,   181,   128,
     182,    -1,   179,    26,   182,   128,   181,    -1,   179,   134,
     182,    -1,   179,   133,   182,    -1,   179,   135,   182,    -1,
     179,   136,   182,    -1,   179,   131,     8,    -1,   179,   132,
       8,    -1,   179,    67,    81,    -1,   179,    67,   142,    81,
      -1,   180,   131,   181,    -1,   180,   132,   181,    -1,   180,
      60,   145,   183,   146,    -1,   180,   142,    60,   145,   183,
     146,    -1,   180,    26,   181,   128,   181,    -1,   180,   142,
      26,   181,   128,   181,    -1,   180,   133,   181,    -1,   180,
     134,   181,    -1,   180,   136,   181,    -1,   180,   135,   181,
      -1,   157,    -1,     4,    -1,    37,   145,   139,   146,    -1,
      56,   145,   146,    -1,   124,   145,   146,    -1,    59,    -1,
     281,    -1,    65,   145,   281,   146,    -1,    45,   145,   281,
     146,    -1,    27,   145,   281,   146,    -1,    47,   145,   146,
      -1,    19,   145,   157,   146,    -1,    18,   145,   157,   146,
      -1,     5,    -1,   138,     5,    -1,     6,    -1,   138,     6,
      -1,    14,    -1,   181,    -1,   183,   147,   181,    -1,     8,
      -1,   184,   147,     8,    -1,    -1,    55,   186,    29,   187,
      -1,    -1,     5,    -1,   179,    -1,   187,   147,   179,    -1,
      -1,    58,   178,    -1,    -1,   190,    -1,   126,    55,    83,
      29,   193,    -1,    -1,   192,    -1,    83,    29,   193,    -1,
      83,    29,    89,   145,   146,    -1,   194,    -1,   193,   147,
     194,    -1,   179,    -1,   179,    21,    -1,   179,    41,    -1,
      -1,   196,    -1,    72,     5,    -1,    72,     5,   147,     5,
      -1,    -1,   198,    -1,    82,   199,    -1,   200,    -1,   199,
     147,   200,    -1,   157,    -1,   157,   131,   157,    -1,   157,
     131,     5,    -1,   157,   131,   145,   201,   146,    -1,   157,
     131,   157,   145,     8,   146,    -1,   157,   131,     8,    -1,
     202,    -1,   201,   147,   202,    -1,   157,   131,   181,    -1,
     157,    -1,     4,    -1,    59,    -1,     5,    -1,     6,    -1,
      14,    -1,     9,    -1,   138,   203,    -1,   142,   203,    -1,
     203,   137,   203,    -1,   203,   138,   203,    -1,   203,   139,
     203,    -1,   203,   140,   203,    -1,   203,   133,   203,    -1,
     203,   134,   203,    -1,   203,   130,   203,    -1,   203,   129,
     203,    -1,   203,   141,   203,    -1,   203,    44,   203,    -1,
     203,    77,   203,    -1,   203,   136,   203,    -1,   203,   135,
     203,    -1,   203,   131,   203,    -1,   203,   132,   203,    -1,
     203,   128,   203,    -1,   203,   127,   203,    -1,   145,   203,
     146,    -1,   148,   208,   149,    -1,   204,    -1,   281,    -1,
     284,    -1,   280,    67,    81,    -1,   280,    67,   142,    81,
      -1,     3,   145,   205,   146,    -1,    60,   145,   205,   146,
      -1,    65,   145,   205,   146,    -1,    27,   145,   205,   146,
      -1,    49,   145,   205,   146,    -1,    45,   145,   205,   146,
      -1,     3,   145,   146,    -1,    76,   145,   203,   147,   203,
     146,    -1,    74,   145,   203,   147,   203,   146,    -1,   124,
     145,   146,    -1,   207,   145,   203,    51,   157,    60,   280,
     146,    -1,    96,   145,   203,   147,   203,   147,   145,   205,
     146,   147,   145,   205,   146,   146,    -1,    89,   145,   146,
      -1,    89,   145,   205,   146,    -1,   206,    -1,   205,   147,
     206,    -1,   203,    -1,     8,    -1,    19,    -1,    18,    -1,
      62,    -1,   209,   131,   210,    -1,   208,   147,   209,   131,
     210,    -1,   157,    -1,    60,    -1,   181,    -1,   157,    -1,
     105,   213,    -1,    -1,    71,     8,    -1,   123,    -1,   108,
     212,    -1,    75,   212,    -1,    16,   108,   212,    -1,    88,
      -1,    85,    -1,    87,    -1,   113,   197,    -1,    16,     8,
     108,   212,    -1,    16,   157,   108,   212,    -1,   214,   157,
     108,    -1,   214,   157,   215,   103,    -1,   214,   157,    14,
     103,    -1,    61,    -1,   111,    -1,    -1,    32,     5,    -1,
     102,   156,   131,   219,    -1,   102,   156,   131,   218,    -1,
     102,   156,   131,    81,    -1,   102,    80,   220,    -1,   102,
      10,   131,   220,    -1,   102,    31,   102,   220,    -1,   102,
      54,     9,   131,   145,   183,   146,    -1,   102,    54,   156,
     131,   218,    -1,   102,    54,   156,   131,     5,    -1,   102,
      61,   157,    54,     9,   131,   145,   183,   146,    -1,   157,
      -1,     8,    -1,   116,    -1,    48,    -1,   181,    -1,   221,
      -1,   220,   138,   221,    -1,   157,    -1,    81,    -1,     8,
      -1,     5,    -1,     6,    -1,    35,    -1,    98,    -1,   223,
      -1,    25,    -1,   107,   115,    -1,   225,    66,   157,   226,
     121,   229,   233,    -1,    63,    -1,    95,    -1,    -1,   145,
     228,   146,    -1,   157,    -1,    59,    -1,   227,    -1,   228,
     147,   227,    -1,   230,    -1,   229,   147,   230,    -1,   145,
     231,   146,    -1,   232,    -1,   231,   147,   232,    -1,   181,
      -1,   182,    -1,     8,    -1,   145,   183,   146,    -1,   145,
     146,    -1,    -1,    82,   234,    -1,   235,    -1,   234,   147,
     235,    -1,     3,   131,     8,    -1,    40,    52,   173,   175,
      -1,    30,   157,   145,   238,   241,   146,    -1,   239,    -1,
     238,   147,   239,    -1,   232,    -1,   145,   240,   146,    -1,
       8,    -1,   240,   147,     8,    -1,    -1,   147,   242,    -1,
     243,    -1,   242,   147,   243,    -1,   232,   244,   245,    -1,
      -1,    20,    -1,   157,    -1,    72,    -1,   247,   157,   212,
      -1,    42,    -1,    41,    -1,   105,   112,   212,    -1,   105,
      39,   212,    -1,   120,   173,   102,   251,   175,   197,    -1,
     252,    -1,   251,   147,   252,    -1,   157,   131,   181,    -1,
     157,   131,   182,    -1,   157,   131,   145,   183,   146,    -1,
     157,   131,   145,   146,    -1,   281,   131,   181,    -1,   281,
     131,   182,    -1,    65,    -1,    27,    -1,    49,    -1,    28,
      -1,    78,    -1,    79,    -1,    69,    -1,   109,    -1,    64,
      -1,    17,   111,   157,    15,    34,   157,   253,    -1,    17,
     111,   157,    46,    34,   157,    -1,    17,    99,   157,    92,
      -1,   105,   263,   122,   256,    -1,   105,   263,   122,    71,
       8,    -1,    -1,   257,    -1,   125,   258,    -1,   259,    -1,
     258,   127,   259,    -1,   157,   131,     8,    -1,   105,    33,
      -1,   105,    31,   102,    -1,   102,   263,   115,    68,    70,
     264,    -1,    -1,    54,    -1,   104,    -1,    91,   119,    -1,
      91,    36,    -1,    94,    91,    -1,   101,    -1,    38,    53,
     157,    97,   266,   106,     8,    -1,    64,    -1,    27,    -1,
      49,    -1,   109,    -1,    65,    -1,    46,    53,   157,    -1,
      22,    61,   157,   114,    99,   157,    -1,    50,    99,   157,
      -1,    50,    90,   157,    -1,    50,    23,    -1,   100,   273,
     195,    -1,    10,    -1,    10,   150,   157,    -1,   100,   203,
      -1,   117,    99,   157,    -1,    84,    61,   157,    -1,    38,
      86,   157,   118,     8,   106,     8,    -1,    46,    86,   157,
     118,     8,    -1,    93,    87,    52,   106,     8,    -1,   281,
      -1,   157,    -1,   157,   282,    -1,   283,    -1,   282,   283,
      -1,    13,    -1,    14,    -1,   151,   203,   152,    -1,   151,
       8,   152,    -1,   203,   131,   285,    -1,   285,   131,   203,
      -1,     8,    -1,    -1,   287,   290,    -1,    29,    -1,   289,
     171,    -1,   203,    -1,   288,    -1,   290,   147,   288,    -1,
      47,   290,   286,   191,   195,    -1,    -1,    52,     8,    -1,
      93,    61,   157,   292,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   187,   187,   188,   189,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   236,   237,   237,   237,   237,   237,
     237,   237,   237,   238,   238,   238,   238,   238,   238,   239,
     239,   239,   239,   239,   240,   240,   240,   240,   240,   241,
     241,   241,   241,   241,   242,   242,   242,   242,   242,   242,
     243,   243,   243,   243,   243,   243,   244,   244,   244,   244,
     244,   244,   245,   245,   245,   245,   245,   245,   246,   246,
     246,   246,   246,   247,   247,   247,   247,   247,   248,   248,
     248,   248,   248,   249,   249,   249,   249,   249,   250,   250,
     250,   250,   250,   250,   251,   251,   251,   251,   251,   251,
     252,   252,   252,   252,   252,   253,   253,   253,   253,   257,
     257,   257,   263,   264,   265,   269,   270,   274,   275,   283,
     284,   291,   293,   297,   301,   308,   309,   310,   314,   327,
     334,   336,   341,   350,   366,   367,   371,   372,   375,   377,
     378,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     394,   395,   398,   400,   404,   408,   409,   410,   414,   419,
     423,   430,   438,   446,   455,   460,   465,   470,   475,   480,
     485,   490,   495,   500,   505,   510,   515,   520,   525,   530,
     535,   540,   545,   550,   555,   560,   565,   572,   578,   589,
     596,   605,   609,   618,   622,   626,   630,   637,   638,   643,
     649,   655,   661,   667,   668,   669,   670,   671,   675,   676,
     680,   681,   692,   693,   694,   698,   704,   711,   717,   723,
     725,   728,   730,   737,   741,   747,   749,   755,   757,   761,
     772,   774,   778,   782,   789,   790,   794,   795,   796,   799,
     801,   805,   810,   817,   819,   823,   827,   828,   832,   837,
     842,   847,   853,   858,   866,   871,   878,   888,   889,   890,
     891,   892,   893,   894,   895,   896,   898,   899,   900,   901,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     925,   926,   927,   928,   929,   930,   931,   932,   933,   934,
     935,   936,   937,   938,   942,   943,   947,   948,   952,   953,
     954,   958,   959,   963,   964,   968,   969,   975,   978,   980,
     984,   985,   986,   987,   988,   989,   990,   991,   992,   997,
    1002,  1007,  1012,  1021,  1022,  1025,  1027,  1035,  1040,  1045,
    1050,  1051,  1052,  1056,  1061,  1066,  1071,  1080,  1081,  1085,
    1086,  1087,  1099,  1100,  1104,  1105,  1106,  1107,  1108,  1115,
    1116,  1117,  1121,  1122,  1128,  1136,  1137,  1140,  1142,  1146,
    1147,  1151,  1152,  1156,  1157,  1161,  1165,  1166,  1170,  1171,
    1172,  1173,  1174,  1177,  1178,  1182,  1183,  1187,  1193,  1203,
    1211,  1215,  1222,  1223,  1230,  1240,  1246,  1248,  1252,  1257,
    1261,  1268,  1270,  1274,  1275,  1281,  1289,  1290,  1296,  1300,
    1306,  1314,  1315,  1319,  1333,  1339,  1343,  1348,  1362,  1373,
    1374,  1375,  1376,  1377,  1378,  1379,  1380,  1381,  1385,  1393,
    1400,  1411,  1415,  1422,  1423,  1427,  1431,  1432,  1436,  1440,
    1447,  1454,  1460,  1461,  1462,  1466,  1467,  1468,  1469,  1475,
    1486,  1487,  1488,  1489,  1490,  1495,  1506,  1518,  1527,  1536,
    1546,  1554,  1555,  1559,  1569,  1580,  1591,  1602,  1612,  1623,
    1624,  1628,  1631,  1632,  1636,  1637,  1638,  1639,  1643,  1644,
    1648,  1653,  1655,  1659,  1668,  1672,  1680,  1681,  1685,  1696,
    1698,  1705
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
  "TOK_AGENT", "TOK_ALTER", "TOK_ALL", "TOK_ANY", "TOK_AS", "TOK_ASC", 
  "TOK_ATTACH", "TOK_ATTRIBUTES", "TOK_AVG", "TOK_BEGIN", "TOK_BETWEEN", 
  "TOK_BIGINT", "TOK_BOOL", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", 
  "TOK_CHUNK", "TOK_COLLATION", "TOK_COLUMN", "TOK_COMMIT", 
  "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DATABASES", 
  "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", 
  "TOK_DOUBLE", "TOK_DROP", "TOK_FACET", "TOK_FALSE", "TOK_FLOAT", 
  "TOK_FLUSH", "TOK_FOR", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", 
  "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_HAVING", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INDEXOF", "TOK_INSERT", "TOK_INT", 
  "TOK_INTEGER", "TOK_INTO", "TOK_IS", "TOK_ISOLATION", "TOK_JSON", 
  "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_MULTI", "TOK_MULTI64", 
  "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", 
  "TOK_PLAN", "TOK_PLUGIN", "TOK_PLUGINS", "TOK_PROFILE", "TOK_RAND", 
  "TOK_RAMCHUNK", "TOK_READ", "TOK_RECONFIGURE", "TOK_RELOAD", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_REMAP", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SETTINGS", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
  "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", 
  "TOK_TABLES", "TOK_THREADS", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", 
  "TOK_TRUNCATE", "TOK_TYPE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", 
  "'}'", "'.'", "'['", "']'", "$accept", "request", "statement", 
  "ident_set", "ident", "multi_stmt_list", "multi_stmt", "select", 
  "select1", "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg", 
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "filter_item", "expr_ident", "mva_aggr", "const_int", 
  "const_float", "const_list", "string_list", "opt_group_clause", 
  "opt_int", "group_items_list", "opt_having_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "json_aggr", "consthash", "hash_key", "hash_val", 
  "show_stmt", "like_filter", "show_what", "index_or_table", "opt_chunk", 
  "set_stmt", "set_global_stmt", "set_string_value", "boolean_value", 
  "set_value", "simple_set_value", "transact_op", "start_transaction", 
  "insert_into", "insert_or_replace", "opt_column_list", "column_ident", 
  "column_list", "insert_rows_list", "insert_row", "insert_vals_list", 
  "insert_val", "opt_insert_options", "insert_options_list", 
  "insert_option", "delete_from", "call_proc", "call_args_list", 
  "call_arg", "const_string_list", "opt_call_opts_list", "call_opts_list", 
  "call_opt", "opt_as", "call_opt_name", "describe", "describe_tok", 
  "show_tables", "show_databases", "update", "update_items_list", 
  "update_item", "alter_col_type", "alter", "show_variables", 
  "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "flush_ramchunk", "flush_index", 
  "select_sysvar", "sysvar_name", "select_dual", "truncate", 
  "optimize_index", "create_plugin", "drop_plugin", "reload_plugins", 
  "json_field", "json_expr", "subscript", "subkey", "streq", "strval", 
  "opt_facet_by_items_list", "facet_by", "facet_item", "facet_expr", 
  "facet_items_list", "facet_stmt", "opt_reload_index_from", 
  "reload_index", 0
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
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   124,
      38,    61,   384,    60,    62,   385,   386,    43,    45,    42,
      47,    37,   387,   388,    59,    40,    41,    44,   123,   125,
      46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   153,   154,   154,   154,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   157,
     157,   157,   158,   158,   158,   159,   159,   160,   160,   161,
     161,   162,   162,   163,   163,   164,   164,   164,   165,   166,
     167,   167,   167,   168,   169,   169,   170,   170,   171,   171,
     171,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     173,   173,   174,   174,   175,   176,   176,   176,   177,   177,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   180,   180,
     181,   181,   182,   182,   182,   183,   183,   184,   184,   185,
     185,   186,   186,   187,   187,   188,   188,   189,   189,   190,
     191,   191,   192,   192,   193,   193,   194,   194,   194,   195,
     195,   196,   196,   197,   197,   198,   199,   199,   200,   200,
     200,   200,   200,   200,   201,   201,   202,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   205,   205,   206,   206,   207,   207,
     207,   208,   208,   209,   209,   210,   210,   211,   212,   212,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     213,   213,   213,   214,   214,   215,   215,   216,   216,   216,
     216,   216,   216,   217,   217,   217,   217,   218,   218,   219,
     219,   219,   220,   220,   221,   221,   221,   221,   221,   222,
     222,   222,   223,   223,   224,   225,   225,   226,   226,   227,
     227,   228,   228,   229,   229,   230,   231,   231,   232,   232,
     232,   232,   232,   233,   233,   234,   234,   235,   236,   237,
     238,   238,   239,   239,   240,   240,   241,   241,   242,   242,
     243,   244,   244,   245,   245,   246,   247,   247,   248,   249,
     250,   251,   251,   252,   252,   252,   252,   252,   252,   253,
     253,   253,   253,   253,   253,   253,   253,   253,   254,   254,
     254,   255,   255,   256,   256,   257,   258,   258,   259,   260,
     261,   262,   263,   263,   263,   264,   264,   264,   264,   265,
     266,   266,   266,   266,   266,   267,   268,   269,   270,   271,
     272,   273,   273,   274,   275,   276,   277,   278,   279,   280,
     280,   281,   282,   282,   283,   283,   283,   283,   284,   284,
     285,   286,   286,   287,   288,   289,   290,   290,   291,   292,
     292,   293
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     1,     1,     1,     8,     1,
       9,     0,     2,     1,     3,     1,     1,     1,     0,     3,
       0,     2,     4,    11,     1,     3,     1,     2,     0,     1,
       2,     1,     4,     4,     4,     4,     4,     4,     3,     5,
       1,     3,     0,     1,     2,     1,     3,     3,     4,     1,
       3,     3,     5,     6,     5,     6,     3,     4,     5,     6,
       3,     3,     3,     3,     3,     3,     5,     5,     5,     3,
       3,     3,     3,     3,     3,     3,     4,     3,     3,     5,
       6,     5,     6,     3,     3,     3,     3,     1,     1,     4,
       3,     3,     1,     1,     4,     4,     4,     3,     4,     4,
       1,     2,     1,     2,     1,     1,     3,     1,     3,     0,
       4,     0,     1,     1,     3,     0,     2,     0,     1,     5,
       0,     1,     3,     5,     1,     3,     1,     2,     2,     0,
       1,     2,     4,     0,     1,     2,     1,     3,     1,     3,
       3,     5,     6,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     3,     4,
       4,     4,     4,     4,     4,     4,     3,     6,     6,     3,
       8,    14,     3,     4,     1,     3,     1,     1,     1,     1,
       1,     3,     5,     1,     1,     1,     1,     2,     0,     2,
       1,     2,     2,     3,     1,     1,     1,     2,     4,     4,
       3,     4,     4,     1,     1,     0,     2,     4,     4,     4,
       3,     4,     4,     7,     5,     5,     9,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     7,     1,     1,     0,     3,     1,
       1,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     3,     2,     0,     2,     1,     3,     3,     4,     6,
       1,     3,     1,     3,     1,     3,     0,     2,     1,     3,
       3,     0,     1,     1,     1,     3,     1,     1,     3,     3,
       6,     1,     3,     3,     3,     5,     4,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     7,     6,
       4,     4,     5,     0,     1,     2,     1,     3,     3,     2,
       3,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     1,     1,     1,     3,     6,     3,     3,     2,
       3,     1,     3,     2,     3,     3,     7,     5,     5,     1,
       1,     2,     1,     2,     1,     1,     3,     3,     3,     3,
       1,     0,     2,     1,     2,     1,     1,     3,     5,     0,
       2,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   382,     0,   379,     0,     0,   427,   426,
       0,     0,   385,     0,     0,   386,   380,     0,   462,   462,
       0,     0,     0,     0,     2,     3,   132,   135,   137,   139,
     136,     7,     8,     9,   381,     5,     0,     6,    10,    11,
       0,    12,    13,    14,    29,    15,    16,    17,    24,    18,
      19,    20,    21,    22,    23,    25,    26,    27,    28,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    38,
      37,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    71,    70,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,   130,    90,    89,    91,    92,    93,    94,    96,    95,
      97,    98,   100,   101,    99,   102,   103,   104,   105,   107,
     108,   106,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   131,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,     0,     0,     0,     0,     0,     0,
     479,     0,     0,     0,     0,     0,    34,   278,   280,   281,
     500,   283,   481,   282,    37,    39,    43,    46,    55,    62,
      64,    71,    70,   279,     0,    74,    77,    84,    86,    96,
      99,   114,   126,     0,   156,     0,     0,     0,   277,     0,
     154,   158,   161,   305,     0,   259,     0,   306,   307,     0,
       0,    49,    68,    73,     0,   106,     0,     0,     0,     0,
     459,   338,   463,   353,   338,   345,   346,   344,   464,   338,
     354,   338,   263,   340,   337,     0,     0,   383,     0,   170,
       0,     1,     0,     4,   134,     0,   338,     0,     0,     0,
       0,     0,     0,     0,   475,     0,   478,   477,   485,   509,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    34,    84,
      86,   284,   285,     0,   334,   333,     0,     0,   494,   495,
       0,   491,   492,     0,     0,     0,   159,   157,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   480,   260,     0,
       0,     0,     0,     0,     0,     0,   377,   378,   376,   375,
     374,   360,   372,     0,     0,     0,   338,     0,   460,     0,
     429,   342,   341,   428,     0,   347,   264,   355,   453,   484,
       0,     0,   505,   506,   158,   501,     0,     0,   133,   387,
     425,   450,     0,     0,     0,   230,   232,   400,   234,     0,
       0,   398,   399,   412,   416,   410,     0,     0,     0,   408,
       0,     0,   511,     0,   327,     0,   316,   326,     0,   324,
     482,     0,     0,     0,     0,     0,     0,   168,     0,     0,
       0,     0,     0,   322,     0,     0,     0,   319,     0,     0,
       0,   303,     0,   304,     0,   500,     0,   493,   148,   172,
     155,   161,   160,   295,   296,   302,   301,   293,   292,   299,
     498,   300,   290,   291,   298,   297,   286,   287,   288,   289,
     294,     0,   261,   308,     0,   499,   361,   362,     0,     0,
       0,     0,   368,   370,   359,   369,     0,   367,   371,   358,
     357,     0,   338,   343,   338,   339,   268,   265,   266,     0,
       0,   350,     0,     0,     0,   451,   454,     0,     0,   431,
       0,   171,   504,   503,     0,   250,     0,     0,     0,     0,
       0,     0,   231,   233,   414,   402,   235,     0,     0,     0,
       0,   471,   472,   470,   474,   473,     0,     0,   218,    37,
      39,    46,    55,    62,     0,    71,   222,    77,    83,   126,
       0,   217,   174,   175,   179,     0,     0,   223,   487,   510,
     488,     0,     0,   310,     0,   162,   313,     0,   167,   315,
     314,   166,   311,   312,   163,     0,   164,     0,   323,     0,
     165,     0,     0,     0,   336,   335,   331,   497,   496,     0,
     239,   173,     0,     0,   309,     0,   365,   364,     0,   373,
       0,   348,   349,     0,     0,   352,   356,   351,   452,     0,
     455,   456,     0,     0,   263,     0,   507,     0,   259,   251,
     502,   390,   389,   391,     0,     0,     0,   449,   476,   401,
       0,   413,     0,   421,   411,   417,   418,   409,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   141,   325,   169,     0,     0,     0,     0,     0,     0,
     241,   247,     0,   262,     0,     0,     0,     0,   468,   461,
     270,   273,     0,   269,   267,     0,     0,     0,   433,   434,
     432,   430,   437,   438,     0,   508,   388,     0,     0,   403,
     393,   440,   442,   441,   447,   439,   445,   443,   444,   446,
     448,   236,   415,   422,     0,     0,   469,   486,     0,     0,
       0,     0,     0,     0,   227,   220,     0,     0,   221,   177,
     176,     0,     0,   186,     0,   205,     0,   203,   180,   194,
     204,   181,   195,   191,   200,   190,   199,   192,   201,   193,
     202,     0,     0,     0,     0,   207,   208,   213,   214,   216,
     215,     0,     0,     0,     0,   318,   317,     0,   332,     0,
       0,   242,     0,     0,   245,   248,     0,   363,     0,   466,
     465,   467,     0,     0,   274,     0,   458,   457,   436,     0,
      96,   256,   252,   254,   392,     0,     0,   396,     0,     0,
     384,   424,   423,   420,   421,   419,   229,   228,   226,   219,
     225,   224,   178,     0,     0,   237,     0,     0,   206,     0,
     187,     0,     0,     0,     0,     0,   146,   147,   145,   142,
     143,   138,     0,     0,     0,   150,     0,     0,     0,   250,
     490,     0,   489,     0,     0,   271,     0,     0,   435,     0,
     257,   258,     0,   395,     0,     0,   404,   405,   394,   188,
     197,   198,   196,   182,   184,     0,     0,     0,     0,   211,
     209,     0,     0,     0,     0,     0,     0,   140,   243,   240,
       0,   246,   259,   320,   366,   276,   275,   272,   253,   255,
     397,     0,     0,   238,   189,   183,   185,   212,   210,   144,
       0,   149,   151,     0,     0,   263,   407,   406,     0,     0,
     244,   249,   153,     0,   152,     0,     0,   321
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    23,    24,   163,   208,    25,    26,    27,    28,   754,
     819,   820,   569,   825,   867,    29,   209,   210,   307,   211,
     429,   570,   389,   532,   533,   534,   535,   536,   506,   382,
     507,   807,   661,   762,   869,   829,   764,   765,   598,   599,
     782,   783,   327,   328,   355,   356,   477,   478,   773,   774,
     397,   213,   398,   399,   214,   296,   297,   566,    30,   350,
     244,   245,   482,    31,    32,   469,   470,   341,   342,    33,
      34,    35,    36,   498,   603,   604,   689,   690,   786,   383,
     790,   846,   847,    37,    38,   384,   385,   508,   510,   615,
     616,   704,   793,    39,    40,    41,    42,    43,   488,   489,
     700,    44,    45,   485,   486,   590,   591,    46,    47,    48,
     227,   669,    49,   516,    50,    51,    52,    53,    54,    55,
     215,    56,    57,    58,    59,    60,    61,   216,   217,   301,
     302,   218,   219,   495,   496,   363,   364,   365,   254,   392,
      62
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -779
static const short yypact[] =
{
     635,    25,   -27,  -779,  4259,  -779,    49,    82,  -779,  -779,
      53,     8,  -779,    17,    23,  -779,  -779,   877,  4383,   551,
     -14,    32,  4259,   138,  -779,   -26,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,    93,  -779,  -779,  -779,
    4259,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  4259,  4259,  4259,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,    35,  4259,  4259,  4259,  4259,  4259,
    -779,  4259,  4259,  4259,  4259,   143,    63,  -779,  -779,  -779,
    -779,  -779,    73,  -779,    65,    87,   107,   124,   126,   133,
     142,   149,   151,  -779,   155,   159,   174,   181,   193,   195,
     204,   208,   219,  1899,  -779,  1899,  1899,  3683,    26,   -25,
    -779,  3796,    13,  -779,   225,   251,   305,   306,  -779,   245,
     248,   278,  4507,  4259,  2950,   266,   252,   267,  3920,   283,
    -779,   316,  -779,  -779,   316,  -779,  -779,  -779,  -779,   316,
    -779,   316,   307,  -779,  -779,  4259,   268,  -779,  4259,  -779,
     113,  -779,  1899,    20,  -779,  4259,   316,   299,   190,   279,
      44,   297,   277,    69,  -779,   280,  -779,  -779,  -779,   345,
     313,  1023,  4259,  1899,  2045,   -18,  2045,  2045,   275,  1899,
    2045,  2045,  1899,  1899,  1169,  1899,  1899,   285,   288,   289,
     290,  -779,  -779,  4655,  -779,  -779,    84,   293,  -779,  -779,
    2191,    34,  -779,  2577,  1315,  4259,  -779,  -779,  1899,  1899,
    1899,  1899,  1899,  1899,  1899,  1899,  1899,  1899,  1899,  1899,
    1899,  1899,  1899,  1899,  1899,  1899,   432,  -779,  -779,   -37,
    1899,  2950,  2950,   322,   323,   402,  -779,  -779,  -779,  -779,
    -779,   321,  -779,  2702,   392,   353,    11,   354,  -779,   456,
    -779,  -779,  -779,  -779,  4259,  -779,  -779,    83,   -39,  -779,
    4259,  4259,  4868,  -779,  3796,   -15,  1461,   416,  -779,   320,
    -779,  -779,   433,   434,   367,  -779,  -779,  -779,  -779,   212,
      33,  -779,  -779,  -779,   325,  -779,    64,   462,  2327,  -779,
     465,   466,  -779,   467,   347,  1607,  -779,  4868,    91,  -779,
    -779,  4689,    96,  4259,   330,   100,   110,  -779,  4720,   112,
     127,   648,   716,  -779,   129,  4510,  4754,  -779,  1753,  1899,
    1899,  -779,  3683,  -779,  2826,   327,   311,  -779,  -779,    69,
    -779,  4868,  -779,  -779,  -779,  4887,  4900,   177,   676,   425,
    -779,   425,   152,   152,   152,   152,    60,    60,  -779,  -779,
    -779,  4853,   334,  -779,   401,   425,   321,   321,   339,  3446,
     477,  2950,  -779,  -779,  -779,  -779,   483,  -779,  -779,  -779,
    -779,   419,   316,  -779,   316,  -779,   359,   351,  -779,   389,
     489,  -779,   397,   497,  4259,  -779,  -779,    61,    81,  -779,
     375,  -779,  -779,  -779,  1899,   424,  1899,  4033,   387,  4259,
    4259,  4259,  -779,  -779,  -779,  -779,  -779,   136,   139,    44,
     363,  -779,  -779,  -779,  -779,  -779,   404,   407,  -779,   369,
     370,   371,   372,   373,   374,   376,  -779,   378,   380,   383,
    2327,    34,   403,  -779,  -779,   294,   130,  -779,  -779,  -779,
    -779,  1315,   384,  -779,  2045,  -779,  -779,   386,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  1899,  -779,  1899,  -779,  1899,
    -779,  4546,  4590,   405,  -779,  -779,  -779,  -779,  -779,   420,
     478,  -779,  4259,   529,  -779,    46,  -779,  -779,   406,  -779,
     176,  -779,  -779,  2452,  4259,  -779,  -779,  -779,  -779,   409,
     408,  -779,    59,  4259,   307,    89,  -779,   509,   251,  -779,
     394,  -779,  -779,  -779,   156,   398,   694,  -779,  -779,  -779,
      46,  -779,   534,    79,  -779,   399,  -779,  -779,   536,   537,
    4259,  4259,  4259,   413,  4259,   410,   423,  4259,   539,   427,
     102,  2327,    89,    10,    -9,    71,    75,    89,    89,    89,
      89,    66,    46,   426,    46,    46,    46,    46,    46,    46,
      70,   428,  -779,  -779,  4788,  4819,  4611,  2826,  1315,   430,
     543,   429,   493,  -779,   175,   436,    -7,   479,  -779,  -779,
    -779,  -779,  4259,   438,  -779,   566,  4259,    28,  -779,  -779,
    -779,  -779,  -779,  -779,  3074,  -779,  -779,  4033,    48,   -47,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  4146,    48,  -779,  -779,   431,   439,
      34,   441,   445,   446,  -779,  -779,   447,   449,  -779,  -779,
    -779,   450,   468,  -779,    38,  -779,   522,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,    46,    15,   485,    46,  -779,  -779,  -779,  -779,  -779,
    -779,    46,   469,  3570,   475,  -779,  -779,   470,  -779,   -24,
     541,  -779,   593,   570,   571,  -779,  4259,  -779,    46,  -779,
    -779,  -779,   500,   178,  -779,   620,  -779,  -779,  -779,   183,
     487,    67,   488,  -779,  -779,    37,   185,  -779,   631,   398,
    -779,  -779,  -779,  -779,   617,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,    89,    89,  -779,   187,   189,  -779,   512,
    -779,    38,    46,   196,   513,    46,  -779,  -779,  -779,   495,
    -779,  -779,  2045,  4259,   614,   572,  3198,   562,  3322,   424,
      34,   502,  -779,   199,    46,  -779,  4259,   508,  -779,   510,
    -779,  -779,  3198,  -779,    48,   515,   511,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,   653,    46,   201,   205,  -779,
    -779,    46,   216,  3570,   220,  3198,   661,  -779,  -779,   524,
     640,  -779,   251,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,   670,   631,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
     532,   488,   535,  3198,  3198,   307,  -779,  -779,   542,   679,
    -779,   488,  -779,  2045,  -779,   222,   540,  -779
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -779,  -779,  -779,    -8,    -4,  -779,   435,  -779,   295,  -779,
    -779,  -174,  -779,  -779,  -779,   122,    36,   391,   329,  -779,
     -10,  -779,  -374,  -514,  -779,  -132,  -654,  -779,   -38,  -569,
    -566,  -114,  -779,  -779,  -779,  -779,  -779,  -779,  -130,  -779,
    -778,  -141,  -592,  -779,  -589,  -779,  -779,   118,  -779,  -131,
      92,  -779,  -273,   160,  -779,  -779,   284,    50,  -779,  -219,
    -779,  -779,  -779,  -779,  -779,   249,  -779,    27,   250,  -779,
    -779,  -779,  -779,  -779,    22,  -779,  -779,   -79,  -779,  -507,
    -779,  -779,  -170,  -779,  -779,  -779,   206,  -779,  -779,  -779,
       9,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,   123,
    -779,  -779,  -779,  -779,  -779,  -779,    41,  -779,  -779,  -779,
     699,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,  -779,
    -779,  -779,  -779,  -779,  -779,  -779,  -779,   -43,  -343,  -779,
     437,  -779,   412,  -779,  -779,   230,  -779,   231,  -779,  -779,
    -779
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -501
static const short yytable[] =
{
     164,   402,   613,   405,   406,   681,   685,   409,   410,   664,
     226,   414,   250,  -483,   493,   351,   630,   490,   249,   723,
     352,   252,   353,   679,   810,   403,   683,   303,   823,   769,
     781,   170,   483,   375,    65,   788,   256,   370,   375,   298,
     299,   504,   375,   375,   453,   537,   805,   298,   299,   375,
     376,   375,   377,   375,   376,   571,   377,   308,   378,   257,
     258,   259,   378,   722,   375,   376,   729,   732,   734,   736,
     738,   740,   725,   378,   298,   299,   375,   376,   173,   727,
     375,   376,   349,   730,   174,   378,   484,   891,   840,   378,
     309,   511,   741,  -490,   375,   376,   751,   479,   171,   703,
     789,   247,   165,   378,   308,   454,   168,   172,   841,   212,
     175,   779,   770,   512,   594,   480,   901,   720,   253,  -112,
     366,   404,   304,   304,    63,   367,   742,   473,   513,   514,
     752,   248,   494,   726,   167,   166,    64,   309,   251,   169,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   724,   642,   263,   806,   255,
     811,   261,   262,   249,   264,   265,   466,   266,   267,   268,
     269,   466,   868,   515,   778,   466,   466,   300,   813,   505,
     260,   787,   379,   505,   466,   300,   379,   537,   781,   380,
     643,   481,   592,   785,   388,   270,   308,   379,   794,   322,
     323,   324,   833,   295,   677,   372,   388,   306,   271,   379,
    -329,   781,   300,   379,   334,   360,   361,   502,   503,   335,
     340,   308,   381,   272,   347,  -412,  -412,   379,   593,   309,
     631,   422,  -328,   423,   850,   852,   373,   543,   544,   900,
     781,   357,   546,   544,   359,   857,   549,   544,   719,   862,
     490,   369,   273,   581,   309,   582,   550,   544,   552,   544,
     361,   644,   645,   646,   647,   648,   649,   666,   400,   274,
     667,   275,   650,   553,   544,   558,   544,   668,   276,   711,
     895,   713,   609,   610,   716,   611,   612,   277,   537,   320,
     321,   322,   323,   324,   278,   291,   279,   292,   293,   249,
     280,   432,   686,   687,  -330,   468,   902,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   281,
     632,   767,   610,   326,   835,   836,   282,   340,   340,   838,
     610,   843,   844,   853,   610,   854,   855,   880,   283,   467,
     284,   537,   860,   610,   362,   874,   610,   885,   610,   285,
     476,   886,   855,   286,   633,   308,   487,   491,   456,   457,
     306,   634,   888,   610,   287,   401,   890,   544,   906,   544,
     325,   408,   329,  -489,   411,   412,   330,   415,   416,   331,
     332,  -464,   344,   343,   531,   348,   565,   349,   309,   354,
     358,   371,   426,   374,   386,   387,   431,   391,   390,   547,
     433,   434,   435,   436,   437,   438,   439,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   295,   393,
     564,   407,   455,   832,   424,   635,   636,   637,   638,   639,
     640,   417,   228,   418,   419,   420,   641,   452,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   458,   459,   467,   460,   340,   431,   461,
     471,   472,   474,   568,   475,   497,   501,   499,   500,   308,
     517,   381,   509,   538,   539,   540,   548,   233,  -500,   567,
     589,   573,   574,   537,   575,   537,   578,   293,   502,   580,
     583,   234,   585,   602,   586,   606,   607,   608,   584,   537,
     587,   235,   309,   236,   237,   588,   595,   597,   605,   617,
     618,   561,   562,   619,   620,   621,   622,   623,   624,   625,
     658,   626,   537,   627,   239,   628,   531,   240,   629,   242,
     651,   631,   653,   660,   663,   676,   657,   665,   684,   243,
     675,   494,   702,   688,   706,   707,   705,   717,   761,   864,
     537,   537,   712,   766,   678,   763,   714,   682,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   228,   662,   715,
     771,   744,   701,   718,   776,   753,   760,   796,   803,   673,
     476,   768,   229,   775,   230,   797,   362,   798,   362,   487,
     231,   799,   800,   801,   721,   802,   804,   728,   731,   733,
     735,   737,   739,   808,   743,   232,   745,   746,   747,   748,
     749,   750,   233,   812,   815,   822,   708,   709,   710,   565,
     710,   821,   826,   710,   824,   827,   234,   531,   837,   828,
     905,   834,   839,   431,   845,   842,   235,   703,   236,   237,
     856,   861,   863,   865,   866,   870,   881,   654,   873,   655,
     381,   656,     1,   564,   877,   238,   878,     2,   882,   239,
       3,   883,   240,   241,   242,     4,   892,   381,   772,   894,
       5,   893,   589,     6,   243,     7,     8,     9,   896,   898,
     531,    10,   899,   602,   904,    11,   907,   903,   368,   889,
     542,   659,   308,   492,   759,   430,   871,   858,    12,   872,
     792,   879,   674,   809,   652,   876,   563,   758,   577,   784,
     848,   579,   897,   814,   795,   614,   680,   777,   246,    13,
     308,   691,   692,   831,   596,   309,   440,   600,    14,     0,
      15,     0,     0,    16,     0,    17,     0,    18,   427,     0,
      19,     0,    20,   693,     0,     0,     0,     0,     0,   818,
     431,     0,    21,   309,     0,    22,     0,     0,   694,   695,
     308,     0,   830,   696,     0,   849,   851,     0,     0,     0,
       0,     0,   697,   698,   859,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
       0,     0,     0,   309,   554,   555,   875,     0,     0,     0,
       0,     0,     0,   699,     0,     0,   381,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   884,   249,
       0,     0,   531,   887,   531,     0,     0,     0,     0,     0,
       0,     0,   772,     0,     0,     0,     0,     0,   531,     0,
       0,     0,     0,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,     0,   818,
       0,   531,   556,   557,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     176,   177,   178,   179,     0,   180,   181,   182,     0,   531,
     531,   183,    67,    68,    69,   184,   185,     0,    72,    73,
      74,   186,    76,    77,   187,    79,     0,    80,    81,    82,
      83,    84,    85,    86,   188,    88,    89,    90,    91,    92,
      93,     0,   189,    95,     0,     0,   190,    97,    98,     0,
      99,   100,   101,   191,   192,   104,   193,   194,   105,   195,
     107,   108,   196,   110,     0,   111,   112,   113,   114,     0,
     115,   197,   117,   198,     0,   119,   120,   121,     0,   122,
       0,   123,   124,   125,   126,   127,   199,   129,   130,   131,
       0,   132,   133,   200,   135,   136,   137,     0,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   201,   148,   149,
     150,   151,   152,     0,   153,   154,   155,   156,   157,   158,
     159,   202,   161,   162,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   203,   204,     0,     0,   205,
       0,     0,   206,     0,     0,   207,   288,   177,   178,   179,
       0,   394,   181,     0,     0,     0,     0,   183,    67,    68,
      69,   184,   185,     0,    72,    73,    74,    75,    76,    77,
     187,    79,     0,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,     0,   189,    95,
       0,     0,   190,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   193,   194,   105,   195,   107,   108,   196,   110,
       0,   111,   112,   113,   114,     0,   115,   289,   117,   290,
       0,   119,   120,   121,     0,   122,     0,   123,   124,   125,
     126,   127,   199,   129,   130,   131,     0,   132,   133,   200,
     135,   136,   137,     0,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,     0,
     153,   154,   155,   156,   157,   158,   159,   202,   161,   162,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,     0,     0,     0,   205,     0,     0,   395,   396,
       0,   207,   288,   177,   178,   179,     0,   394,   181,     0,
       0,     0,     0,   183,    67,    68,    69,   184,   185,     0,
      72,    73,    74,    75,    76,    77,   187,    79,     0,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,     0,   189,    95,     0,     0,   190,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   193,   194,
     105,   195,   107,   108,   196,   110,     0,   111,   112,   113,
     114,     0,   115,   289,   117,   290,     0,   119,   120,   121,
       0,   122,     0,   123,   124,   125,   126,   127,   199,   129,
     130,   131,     0,   132,   133,   200,   135,   136,   137,     0,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,     0,   153,   154,   155,   156,
     157,   158,   159,   202,   161,   162,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   203,     0,     0,
       0,   205,     0,     0,   206,   413,     0,   207,   288,   177,
     178,   179,     0,   180,   181,     0,     0,     0,     0,   183,
      67,    68,    69,   184,   185,     0,    72,    73,    74,   186,
      76,    77,   187,    79,     0,    80,    81,    82,    83,    84,
      85,    86,   188,    88,    89,    90,    91,    92,    93,     0,
     189,    95,     0,     0,   190,    97,    98,     0,    99,   100,
     101,   191,   192,   104,   193,   194,   105,   195,   107,   108,
     196,   110,     0,   111,   112,   113,   114,     0,   115,   197,
     117,   198,     0,   119,   120,   121,     0,   122,     0,   123,
     124,   125,   126,   127,   199,   129,   130,   131,     0,   132,
     133,   200,   135,   136,   137,     0,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   201,   148,   149,   150,   151,
     152,     0,   153,   154,   155,   156,   157,   158,   159,   202,
     161,   162,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   203,   204,     0,     0,   205,     0,     0,
     206,     0,     0,   207,   176,   177,   178,   179,     0,   180,
     181,     0,     0,     0,     0,   183,    67,    68,    69,   184,
     185,     0,    72,    73,    74,   186,    76,    77,   187,    79,
       0,    80,    81,    82,    83,    84,    85,    86,   188,    88,
      89,    90,    91,    92,    93,     0,   189,    95,     0,     0,
     190,    97,    98,     0,    99,   100,   101,   191,   192,   104,
     193,   194,   105,   195,   107,   108,   196,   110,     0,   111,
     112,   113,   114,     0,   115,   197,   117,   198,     0,   119,
     120,   121,     0,   122,     0,   123,   124,   125,   126,   127,
     199,   129,   130,   131,     0,   132,   133,   200,   135,   136,
     137,     0,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   201,   148,   149,   150,   151,   152,     0,   153,   154,
     155,   156,   157,   158,   159,   202,   161,   162,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   203,
     204,     0,     0,   205,     0,     0,   206,     0,     0,   207,
     288,   177,   178,   179,     0,   180,   181,     0,     0,     0,
       0,   183,    67,    68,    69,   184,   185,     0,    72,    73,
      74,    75,    76,    77,   187,    79,     0,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,   189,    95,     0,     0,   190,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   193,   194,   105,   195,
     107,   108,   196,   110,     0,   111,   112,   113,   114,     0,
     115,   289,   117,   290,     0,   119,   120,   121,     0,   122,
       0,   123,   124,   125,   126,   127,   199,   129,   130,   131,
       0,   132,   133,   200,   135,   136,   137,   541,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,     0,   153,   154,   155,   156,   157,   158,
     159,   202,   161,   162,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   203,     0,     0,     0,   205,
       0,     0,   206,     0,     0,   207,   288,   177,   178,   179,
       0,   394,   181,     0,     0,     0,     0,   183,    67,    68,
      69,   184,   185,     0,    72,    73,    74,    75,    76,    77,
     187,    79,     0,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,     0,   189,    95,
       0,     0,   190,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   193,   194,   105,   195,   107,   108,   196,   110,
       0,   111,   112,   113,   114,     0,   115,   289,   117,   290,
       0,   119,   120,   121,     0,   122,     0,   123,   124,   125,
     126,   127,   199,   129,   130,   131,     0,   132,   133,   200,
     135,   136,   137,     0,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,     0,
     153,   154,   155,   156,   157,   158,   159,   202,   161,   162,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,     0,     0,     0,   205,     0,     0,   206,   396,
       0,   207,   288,   177,   178,   179,     0,   180,   181,     0,
       0,     0,     0,   183,    67,    68,    69,   184,   185,     0,
      72,    73,    74,    75,    76,    77,   187,    79,     0,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,     0,   189,    95,     0,     0,   190,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   193,   194,
     105,   195,   107,   108,   196,   110,     0,   111,   112,   113,
     114,     0,   115,   289,   117,   290,     0,   119,   120,   121,
       0,   122,     0,   123,   124,   125,   126,   127,   199,   129,
     130,   131,     0,   132,   133,   200,   135,   136,   137,     0,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,     0,   153,   154,   155,   156,
     157,   158,   159,   202,   161,   162,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   203,     0,     0,
       0,   205,     0,     0,   206,     0,     0,   207,   288,   177,
     178,   179,     0,   394,   181,     0,     0,     0,     0,   183,
      67,    68,    69,   184,   185,     0,    72,    73,    74,    75,
      76,    77,   187,    79,     0,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,     0,
     189,    95,     0,     0,   190,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   193,   194,   105,   195,   107,   108,
     196,   110,     0,   111,   112,   113,   114,     0,   115,   289,
     117,   290,     0,   119,   120,   121,     0,   122,     0,   123,
     124,   125,   126,   127,   199,   129,   130,   131,     0,   132,
     133,   200,   135,   136,   137,     0,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,     0,   153,   154,   155,   156,   157,   158,   159,   202,
     161,   162,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   203,     0,     0,     0,   205,     0,     0,
     206,     0,     0,   207,   288,   177,   178,   179,     0,   425,
     181,     0,     0,     0,     0,   183,    67,    68,    69,   184,
     185,     0,    72,    73,    74,    75,    76,    77,   187,    79,
       0,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,     0,   189,    95,     0,     0,
     190,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     193,   194,   105,   195,   107,   108,   196,   110,     0,   111,
     112,   113,   114,     0,   115,   289,   117,   290,     0,   119,
     120,   121,     0,   122,     0,   123,   124,   125,   126,   127,
     199,   129,   130,   131,     0,   132,   133,   200,   135,   136,
     137,     0,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,     0,   153,   154,
     155,   156,   157,   158,   159,   202,   161,   162,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   203,
      66,   518,     0,   205,     0,     0,   206,     0,     0,   207,
       0,     0,    67,    68,    69,   519,   520,     0,    72,    73,
      74,    75,    76,    77,   521,    79,     0,    80,    81,    82,
      83,    84,    85,    86,   522,    88,    89,    90,    91,    92,
      93,     0,   523,    95,   524,     0,    96,    97,    98,     0,
      99,   100,   101,   525,   103,   104,   526,     0,   105,   106,
     107,   108,   527,   110,     0,   111,   112,   113,   114,     0,
     528,   116,   117,   118,     0,   119,   120,   121,     0,   122,
       0,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   132,   133,   134,   135,   136,   137,     0,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,     0,   153,   154,   155,   156,   157,   158,
     159,   529,   161,   162,     0,    66,     0,   670,     0,     0,
     671,     0,     0,     0,     0,     0,     0,    67,    68,    69,
      70,    71,   530,    72,    73,    74,    75,    76,    77,    78,
      79,     0,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,     0,    94,    95,     0,
       0,    96,    97,    98,     0,    99,   100,   101,   102,   103,
     104,     0,     0,   105,   106,   107,   108,   109,   110,     0,
     111,   112,   113,   114,     0,   115,   116,   117,   118,     0,
     119,   120,   121,     0,   122,     0,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   132,   133,   134,   135,
     136,   137,     0,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,     0,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,     0,
      66,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    67,    68,    69,    70,    71,   672,    72,    73,
      74,    75,    76,    77,    78,    79,     0,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,    95,     0,     0,    96,    97,    98,     0,
      99,   100,   101,   102,   103,   104,     0,     0,   105,   106,
     107,   108,   109,   110,     0,   111,   112,   113,   114,     0,
     115,   116,   117,   118,     0,   119,   120,   121,     0,   122,
       0,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   132,   133,   134,   135,   136,   137,     0,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,     0,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,     0,    66,     0,   375,     0,     0,
     462,     0,     0,     0,     0,     0,     0,    67,    68,    69,
      70,    71,   428,    72,    73,    74,    75,    76,    77,    78,
      79,     0,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,     0,    94,    95,     0,
     463,    96,    97,    98,     0,    99,   100,   101,   102,   103,
     104,     0,     0,   105,   106,   107,   108,   109,   110,     0,
     111,   112,   113,   114,     0,   115,   116,   117,   118,     0,
     119,   120,   121,   464,   122,     0,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   132,   133,   134,   135,
     136,   137,     0,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   465,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,    66,
       0,   375,     0,     0,     0,     0,     0,     0,     0,     0,
     466,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,    77,    78,    79,     0,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
       0,    94,    95,     0,     0,    96,    97,    98,     0,    99,
     100,   101,   102,   103,   104,     0,     0,   105,   106,   107,
     108,   109,   110,     0,   111,   112,   113,   114,     0,   115,
     116,   117,   118,     0,   119,   120,   121,     0,   122,     0,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
     132,   133,   134,   135,   136,   137,     0,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,    66,     0,   336,   337,     0,   338,     0,
       0,     0,     0,     0,   466,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,    77,    78,    79,     0,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,     0,    94,    95,     0,     0,    96,
      97,    98,     0,    99,   100,   101,   102,   103,   104,     0,
       0,   105,   106,   107,   108,   109,   110,     0,   111,   112,
     113,   114,     0,   115,   116,   117,   118,     0,   119,   120,
     121,   339,   122,     0,   123,   124,   125,   126,   127,   128,
     129,   130,   131,     0,   132,   133,   134,   135,   136,   137,
       0,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,     0,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,    66,   518,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
      77,   521,    79,     0,    80,    81,    82,    83,    84,    85,
      86,   522,    88,    89,    90,    91,    92,    93,     0,   523,
      95,   524,     0,    96,    97,    98,     0,    99,   100,   101,
     525,   103,   104,   526,     0,   105,   106,   107,   108,   527,
     110,     0,   111,   112,   113,   114,     0,   115,   116,   117,
     118,     0,   119,   120,   121,     0,   122,     0,   123,   124,
     125,   126,   127,   780,   129,   130,   131,     0,   132,   133,
     134,   135,   136,   137,     0,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
       0,   153,   154,   155,   156,   157,   158,   159,   529,   161,
     162,    66,   518,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    67,    68,    69,    70,    71,     0,    72,
      73,    74,    75,    76,    77,   521,    79,     0,    80,    81,
      82,    83,    84,    85,    86,   522,    88,    89,    90,    91,
      92,    93,     0,   523,    95,   524,     0,    96,    97,    98,
       0,    99,   100,   101,   525,   103,   104,   526,     0,   105,
     106,   107,   108,   527,   110,     0,   111,   112,   113,   114,
       0,   115,   116,   117,   118,     0,   119,   120,   121,     0,
     122,     0,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,     0,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,   153,   154,   155,   156,   157,
     158,   159,   529,   161,   162,    66,   518,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    67,    68,    69,
     519,   520,     0,    72,    73,    74,    75,    76,    77,   521,
      79,     0,    80,    81,    82,    83,    84,    85,    86,   522,
      88,    89,    90,    91,    92,    93,     0,   523,    95,   524,
       0,    96,    97,    98,     0,    99,   100,   101,   525,   103,
     104,   526,     0,   105,   106,   107,   108,   527,   110,     0,
     111,   112,   113,   114,     0,   115,   116,   117,   118,     0,
     119,   120,   121,     0,   122,     0,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   132,   133,   134,   135,
     136,   137,     0,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,     0,   153,
     154,   155,   156,   157,   158,   159,   529,   161,   162,    66,
       0,   576,     0,     0,   462,     0,     0,     0,     0,     0,
       0,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,    77,    78,    79,     0,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
       0,    94,    95,     0,     0,    96,    97,    98,     0,    99,
     100,   101,   102,   103,   104,     0,     0,   105,   106,   107,
     108,   109,   110,     0,   111,   112,   113,   114,     0,   115,
     116,   117,   118,     0,   119,   120,   121,     0,   122,     0,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
     132,   133,   134,   135,   136,   137,     0,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,    66,     0,   816,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,    77,    78,    79,     0,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,     0,    94,    95,     0,     0,    96,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   817,
       0,   105,   106,   107,   108,   109,   110,     0,   111,   112,
     113,   114,     0,   115,   116,   117,   118,     0,   119,   120,
     121,     0,   122,     0,   123,   124,   125,   126,   127,   128,
     129,   130,   131,     0,   132,   133,   134,   135,   136,   137,
       0,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,    66,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,     0,    67,    68,
      69,    70,    71,     0,    72,    73,    74,    75,    76,    77,
      78,    79,     0,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,     0,    94,    95,
       0,     0,    96,    97,    98,     0,    99,   100,   101,   102,
     103,   104,     0,   294,   105,   106,   107,   108,   109,   110,
       0,   111,   112,   113,   114,     0,   115,   116,   117,   118,
       0,   119,   120,   121,     0,   122,     0,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,   132,   133,   134,
     135,   136,   137,     0,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    66,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
       0,    67,    68,    69,    70,    71,   305,    72,    73,    74,
      75,    76,    77,    78,    79,     0,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
       0,    94,    95,     0,     0,    96,    97,    98,     0,    99,
     100,   101,   102,   103,   104,     0,     0,   105,   106,   107,
     108,   109,   110,     0,   111,   112,   113,   114,     0,   115,
     116,   117,   118,     0,   119,   120,   121,     0,   122,     0,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
     132,   133,   134,   135,   136,   137,     0,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,    66,     0,     0,     0,     0,   345,     0,
       0,     0,     0,     0,     0,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,    77,    78,    79,     0,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,     0,    94,    95,     0,     0,    96,
      97,    98,     0,    99,   100,   101,   102,   103,   104,     0,
       0,   105,   106,   107,   108,   109,   110,     0,   111,   112,
     113,   114,     0,   115,   116,   117,   118,     0,   119,   120,
     121,     0,   122,     0,   123,   124,   125,   126,   127,   128,
     129,   130,   131,     0,   132,   133,   134,   135,   136,   137,
       0,   138,   139,   140,   141,   142,   143,   144,   346,   146,
     147,   148,   149,   150,   151,   152,    66,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,     0,    67,    68,
      69,    70,    71,     0,    72,    73,    74,    75,    76,    77,
      78,    79,     0,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,     0,    94,    95,
       0,     0,    96,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   601,     0,   105,   106,   107,   108,   109,   110,
       0,   111,   112,   113,   114,     0,   115,   116,   117,   118,
       0,   119,   120,   121,     0,   122,     0,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,   132,   133,   134,
     135,   136,   137,     0,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    66,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
       0,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,    77,    78,    79,     0,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
       0,    94,    95,     0,     0,    96,    97,    98,     0,    99,
     100,   101,   102,   103,   104,     0,     0,   105,   106,   107,
     108,   109,   110,     0,   111,   112,   113,   114,   791,   115,
     116,   117,   118,     0,   119,   120,   121,     0,   122,     0,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
     132,   133,   134,   135,   136,   137,     0,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,    66,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,     0,    67,    68,    69,    70,    71,     0,
      72,    73,    74,    75,    76,    77,    78,    79,     0,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,     0,    94,    95,     0,     0,    96,    97,
      98,     0,    99,   100,   101,   102,   103,   104,     0,     0,
     105,   106,   107,   108,   109,   110,     0,   111,   112,   113,
     114,     0,   115,   116,   117,   118,     0,   119,   120,   121,
       0,   122,     0,   123,   124,   125,   126,   127,   128,   129,
     130,   131,     0,   132,   133,   134,   135,   136,   137,     0,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,     0,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,    66,     0,     0,     0,
       0,     0,     0,   220,     0,     0,     0,     0,    67,    68,
      69,    70,    71,     0,    72,    73,    74,    75,    76,    77,
      78,    79,     0,    80,   221,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,     0,    94,    95,
       0,     0,    96,    97,    98,     0,    99,   222,   101,   102,
     103,   104,     0,     0,   223,   106,   107,   108,   109,   110,
       0,   111,   112,   113,   114,     0,   115,   116,   117,   118,
       0,   119,   120,   224,     0,   122,     0,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,   132,   133,   134,
     135,   136,   137,     0,   138,   139,   140,   225,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,     0,     0,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
      66,     0,     0,     0,     0,     0,   333,     0,     0,     0,
       0,     0,    67,    68,    69,    70,    71,     0,    72,    73,
      74,    75,    76,    77,    78,    79,     0,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,    95,   308,     0,    96,    97,    98,     0,
      99,   100,   101,   102,   103,   104,     0,     0,   105,   106,
     107,   108,   109,   110,     0,   111,   112,   113,   114,     0,
     115,   116,   117,   118,     0,   119,   120,   309,     0,   122,
     308,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   132,   133,   134,   135,   136,   137,     0,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,  -463,   309,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   308,     0,     0,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,     0,     0,     0,   308,     0,   559,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   309,     0,     0,
       0,     0,     0,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   309,     0,
       0,     0,     0,   555,     0,     0,     0,     0,     0,   308,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   309,   308,     0,     0,     0,   557,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,     0,     0,     0,     0,     0,   757,     0,
       0,     0,     0,     0,   308,     0,   309,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   309,   308,     0,
       0,   421,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   309,   308,     0,     0,   545,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,     0,   308,     0,   309,   551,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   309,   308,     0,     0,
     560,     0,     0,     0,   572,     0,     0,     0,     0,     0,
       0,     0,   308,     0,     0,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     309,   308,     0,     0,   755,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,     0,     0,     0,   309,   756,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   309,     0,     0,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
       0,     0,     0,     0,     0,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324
};

static const short yycheck[] =
{
       4,   274,   509,   276,   277,   594,   598,   280,   281,   575,
      18,   284,    22,     0,    29,   234,   530,   360,    22,     9,
     239,    47,   241,   592,     9,    43,   595,    52,    52,    36,
     684,    23,    71,     5,    61,    82,    40,   256,     5,    13,
      14,     8,     5,     5,    81,   388,     8,    13,    14,     5,
       6,     5,     8,     5,     6,   429,     8,    44,    14,    63,
      64,    65,    14,   632,     5,     6,   635,   636,   637,   638,
     639,   640,    81,    14,    13,    14,     5,     6,    61,     8,
       5,     6,    71,     8,    61,    14,   125,   865,    21,    14,
      77,    27,    26,    67,     5,     6,    26,    14,    90,    20,
     147,   115,    53,    14,    44,   142,    53,    99,    41,    17,
      87,   677,   119,    49,   488,    32,   894,   631,   144,   108,
     100,   139,   147,   147,    99,   105,    60,   346,    64,    65,
      60,    99,   147,   142,    52,    86,   111,    77,     0,    86,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   145,    26,   167,   724,    66,
     145,   165,   166,   167,   168,   169,   138,   171,   172,   173,
     174,   138,   826,   109,   146,   138,   138,   151,   744,   146,
     145,   688,   138,   146,   138,   151,   138,   530,   842,   145,
      60,   108,   131,   145,   125,    52,    44,   138,   705,   139,
     140,   141,   768,   207,   145,    15,   125,   211,   145,   138,
     145,   865,   151,   138,   222,   102,   147,     5,     6,   223,
     224,    44,   260,   150,   228,   146,   147,   138,   147,    77,
     128,   147,   145,   149,   803,   804,    46,   146,   147,   893,
     894,   245,   146,   147,   248,   811,   146,   147,   146,   815,
     593,   255,   145,   472,    77,   474,   146,   147,   146,   147,
     147,   131,   132,   133,   134,   135,   136,    91,   272,   145,
      94,   145,   142,   146,   147,   146,   147,   101,   145,   622,
     872,   624,   146,   147,   627,   146,   147,   145,   631,   137,
     138,   139,   140,   141,   145,   203,   145,   205,   206,   303,
     145,   305,   146,   147,   145,   343,   895,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   145,
      26,   146,   147,    72,   146,   147,   145,   331,   332,   146,
     147,   146,   147,   146,   147,   146,   147,   844,   145,   343,
     145,   684,   146,   147,   252,   146,   147,   146,   147,   145,
     354,   146,   147,   145,    60,    44,   360,   361,   331,   332,
     364,    67,   146,   147,   145,   273,   146,   147,   146,   147,
     145,   279,    67,    67,   282,   283,   131,   285,   286,   131,
     102,   115,   115,   131,   388,   102,   424,    71,    77,    82,
     122,    92,   300,   114,    97,   118,   304,    52,   118,   403,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   422,   106,
     424,   146,   330,   766,   131,   131,   132,   133,   134,   135,
     136,   146,    16,   145,   145,   145,   142,     5,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   131,   131,   459,    54,   461,   366,   138,
      68,   108,   108,   152,     8,   145,    99,    34,    34,    44,
       8,   509,   147,     8,     8,     8,   146,    61,   131,   152,
     484,   147,    81,   826,   145,   828,     9,   395,     5,    70,
     131,    75,   103,   497,     5,   499,   500,   501,   147,   842,
     103,    85,    77,    87,    88,     8,   131,    83,   121,   146,
     106,   419,   420,   106,   145,   145,   145,   145,   145,   145,
     100,   145,   865,   145,   108,   145,   530,   111,   145,   113,
     146,   128,   146,    55,     5,   127,   131,   131,    29,   123,
     131,   147,     8,   145,     8,     8,   147,     8,     5,   822,
     893,   894,   139,    60,   592,   126,   146,   595,   133,   134,
     135,   136,   137,   138,   139,   140,   141,    16,   572,   146,
      91,   145,   610,   146,     8,   147,   146,   146,   128,   583,
     584,   145,    31,   145,    33,   146,   494,   146,   496,   593,
      39,   146,   146,   146,   632,   146,   128,   635,   636,   637,
     638,   639,   640,    81,   642,    54,   644,   645,   646,   647,
     648,   649,    61,   128,   145,   145,   620,   621,   622,   657,
     624,   146,    29,   627,    83,    55,    75,   631,     8,    58,
     903,   131,   145,   541,     3,   147,    85,    20,    87,    88,
     128,   128,   147,    29,    72,    83,   131,   555,   146,   557,
     688,   559,    17,   657,   146,   104,   146,    22,   147,   108,
      25,     8,   111,   112,   113,    30,     5,   705,   672,    29,
      35,   147,   676,    38,   123,    40,    41,    42,     8,   147,
     684,    46,   147,   687,     5,    50,   146,   145,   253,   863,
     395,   569,    44,   364,   658,   304,   828,   811,    63,   829,
     704,   842,   584,   741,   544,   836,   422,   657,   459,   687,
     789,   461,   882,   751,   705,   509,   593,   676,    19,    84,
      44,    27,    28,   766,   494,    77,   314,   496,    93,    -1,
      95,    -1,    -1,    98,    -1,   100,    -1,   102,   301,    -1,
     105,    -1,   107,    49,    -1,    -1,    -1,    -1,    -1,   753,
     658,    -1,   117,    77,    -1,   120,    -1,    -1,    64,    65,
      44,    -1,   766,    69,    -1,   803,   804,    -1,    -1,    -1,
      -1,    -1,    78,    79,   812,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    77,   146,   147,   834,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,   844,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   856,   823,
      -1,    -1,   826,   861,   828,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   836,    -1,    -1,    -1,    -1,    -1,   842,    -1,
      -1,    -1,    -1,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,    -1,   863,
      -1,   865,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    10,    -1,   893,
     894,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      73,    74,    75,    76,    -1,    78,    79,    80,    -1,    82,
      -1,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,    -1,    -1,   142,
      -1,    -1,   145,    -1,    -1,   148,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    76,
      -1,    78,    79,    80,    -1,    82,    -1,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,    96,
      97,    98,    99,    -1,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,   142,    -1,    -1,   145,   146,
      -1,   148,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    73,    74,    75,    76,    -1,    78,    79,    80,
      -1,    82,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    96,    97,    98,    99,    -1,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,    -1,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,   142,    -1,    -1,   145,   146,    -1,   148,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    73,    74,
      75,    76,    -1,    78,    79,    80,    -1,    82,    -1,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    -1,    94,
      95,    96,    97,    98,    99,    -1,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,    -1,    -1,   142,    -1,    -1,
     145,    -1,    -1,   148,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    73,    74,    75,    76,    -1,    78,
      79,    80,    -1,    82,    -1,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    -1,    94,    95,    96,    97,    98,
      99,    -1,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    -1,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,    -1,    -1,   142,    -1,    -1,   145,    -1,    -1,   148,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      73,    74,    75,    76,    -1,    78,    79,    80,    -1,    82,
      -1,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,   142,
      -1,    -1,   145,    -1,    -1,   148,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    76,
      -1,    78,    79,    80,    -1,    82,    -1,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,    96,
      97,    98,    99,    -1,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,   142,    -1,    -1,   145,   146,
      -1,   148,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    73,    74,    75,    76,    -1,    78,    79,    80,
      -1,    82,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    96,    97,    98,    99,    -1,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,    -1,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,   142,    -1,    -1,   145,    -1,    -1,   148,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    73,    74,
      75,    76,    -1,    78,    79,    80,    -1,    82,    -1,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    -1,    94,
      95,    96,    97,    98,    99,    -1,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,   142,    -1,    -1,
     145,    -1,    -1,   148,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    73,    74,    75,    76,    -1,    78,
      79,    80,    -1,    82,    -1,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    -1,    94,    95,    96,    97,    98,
      99,    -1,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    -1,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
       3,     4,    -1,   142,    -1,    -1,   145,    -1,    -1,   148,
      -1,    -1,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    47,    -1,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    -1,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      73,    74,    75,    76,    -1,    78,    79,    80,    -1,    82,
      -1,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,     3,    -1,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,   145,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    -1,    -1,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    73,    74,    75,    76,    -1,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    -1,    94,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    -1,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
       3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    16,    17,    18,    19,   145,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    -1,    -1,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      73,    74,    75,    76,    -1,    78,    79,    80,    -1,    82,
      -1,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,     3,    -1,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,   145,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    -1,    -1,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    -1,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    -1,    94,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,     3,
      -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    -1,    -1,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    73,
      74,    75,    76,    -1,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    -1,
      94,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    -1,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,     3,    -1,     5,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,   138,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    -1,
      -1,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    -1,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    -1,    94,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    47,    -1,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    -1,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    73,    74,    75,
      76,    -1,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    -1,    94,    95,
      96,    97,    98,    99,    -1,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
      -1,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    47,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    73,    74,    75,    76,    -1,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    -1,    94,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    -1,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,     3,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    47,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    73,    74,    75,    76,    -1,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    -1,    94,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    -1,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,     3,
      -1,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    -1,    -1,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    73,
      74,    75,    76,    -1,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    -1,
      94,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    -1,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,     3,    -1,     5,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      -1,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    -1,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    -1,    94,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,     3,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    -1,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    76,
      -1,    78,    79,    80,    -1,    82,    -1,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,    96,
      97,    98,    99,    -1,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,     3,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    -1,    -1,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    73,
      74,    75,    76,    -1,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    -1,
      94,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    -1,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,     3,    -1,    -1,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    -1,
      -1,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    -1,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    -1,    94,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,     3,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    -1,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    76,
      -1,    78,    79,    80,    -1,    82,    -1,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,    96,
      97,    98,    99,    -1,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,     3,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    -1,    -1,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    -1,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    -1,
      94,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,     3,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    -1,    -1,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    73,    74,    75,    76,    -1,    78,    79,    80,
      -1,    82,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    96,    97,    98,    99,    -1,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,    -1,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,     3,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    -1,    -1,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    76,
      -1,    78,    79,    80,    -1,    82,    -1,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,    96,
      97,    98,    99,    -1,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    -1,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
       3,    -1,    -1,    -1,    -1,    -1,     9,    -1,    -1,    -1,
      -1,    -1,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    44,    -1,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    -1,    -1,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      73,    74,    75,    76,    -1,    78,    79,    77,    -1,    82,
      44,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    77,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    44,    -1,    -1,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    44,    -1,   147,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,    77,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,    77,    44,    -1,    -1,    -1,   147,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,    -1,    44,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,    77,    44,    -1,
      -1,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,    77,    44,    -1,    -1,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,    -1,    44,    -1,    77,   146,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,    77,    44,    -1,    -1,
     146,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    -1,    -1,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      77,    44,    -1,    -1,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    77,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    77,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    22,    25,    30,    35,    38,    40,    41,    42,
      46,    50,    63,    84,    93,    95,    98,   100,   102,   105,
     107,   117,   120,   154,   155,   158,   159,   160,   161,   168,
     211,   216,   217,   222,   223,   224,   225,   236,   237,   246,
     247,   248,   249,   250,   254,   255,   260,   261,   262,   265,
     267,   268,   269,   270,   271,   272,   274,   275,   276,   277,
     278,   279,   293,    99,   111,    61,     3,    15,    16,    17,
      18,    19,    21,    22,    23,    24,    25,    26,    27,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    45,    46,    49,    50,    51,    53,
      54,    55,    56,    57,    58,    61,    62,    63,    64,    65,
      66,    68,    69,    70,    71,    73,    74,    75,    76,    78,
      79,    80,    82,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    94,    95,    96,    97,    98,    99,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   156,   157,    53,    86,    52,    53,    86,
      23,    90,    99,    61,    61,    87,     3,     4,     5,     6,
       8,     9,    10,    14,    18,    19,    24,    27,    37,    45,
      49,    56,    57,    59,    60,    62,    65,    74,    76,    89,
      96,   110,   124,   138,   139,   142,   145,   148,   157,   169,
     170,   172,   203,   204,   207,   273,   280,   281,   284,   285,
      10,    31,    54,    61,    80,   104,   156,   263,    16,    31,
      33,    39,    54,    61,    75,    85,    87,    88,   104,   108,
     111,   112,   113,   123,   213,   214,   263,   115,    99,   157,
     173,     0,    47,   144,   291,    66,   157,   157,   157,   157,
     145,   157,   157,   173,   157,   157,   157,   157,   157,   157,
      52,   145,   150,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,     3,    74,
      76,   203,   203,   203,    60,   157,   208,   209,    13,    14,
     151,   282,   283,    52,   147,    20,   157,   171,    44,    77,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   145,    72,   195,   196,    67,
     131,   131,   102,     9,   156,   157,     5,     6,     8,    81,
     157,   220,   221,   131,   115,     8,   108,   157,   102,    71,
     212,   212,   212,   212,    82,   197,   198,   157,   122,   157,
     102,   147,   203,   288,   289,   290,   100,   105,   159,   157,
     212,    92,    15,    46,   114,     5,     6,     8,    14,   138,
     145,   181,   182,   232,   238,   239,    97,   118,   125,   175,
     118,    52,   292,   106,     8,   145,   146,   203,   205,   206,
     157,   203,   205,    43,   139,   205,   205,   146,   203,   205,
     205,   203,   203,   146,   205,   203,   203,   146,   145,   145,
     145,   146,   147,   149,   131,     8,   203,   283,   145,   173,
     170,   203,   157,   203,   203,   203,   203,   203,   203,   203,
     285,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,     5,    81,   142,   203,   220,   220,   131,   131,
      54,   138,     8,    48,    81,   116,   138,   157,   181,   218,
     219,    68,   108,   212,   108,     8,   157,   199,   200,    14,
      32,   108,   215,    71,   125,   256,   257,   157,   251,   252,
     281,   157,   171,    29,   147,   286,   287,   145,   226,    34,
      34,    99,     5,     6,     8,   146,   181,   183,   240,   147,
     241,    27,    49,    64,    65,   109,   266,     8,     4,    18,
      19,    27,    37,    45,    47,    56,    59,    65,    73,   124,
     145,   157,   176,   177,   178,   179,   180,   281,     8,     8,
       8,   100,   161,   146,   147,   146,   146,   157,   146,   146,
     146,   146,   146,   146,   146,   147,   146,   147,   146,   147,
     146,   203,   203,   209,   157,   181,   210,   152,   152,   165,
     174,   175,    51,   147,    81,   145,     5,   218,     9,   221,
      70,   212,   212,   131,   147,   103,     5,   103,     8,   157,
     258,   259,   131,   147,   175,   131,   288,    83,   191,   192,
     290,    59,   157,   227,   228,   121,   157,   157,   157,   146,
     147,   146,   147,   232,   239,   242,   243,   146,   106,   106,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     176,   128,    26,    60,    67,   131,   132,   133,   134,   135,
     136,   142,    26,    60,   131,   132,   133,   134,   135,   136,
     142,   146,   206,   146,   203,   203,   203,   131,   100,   168,
      55,   185,   157,     5,   183,   131,    91,    94,   101,   264,
       5,     8,   145,   157,   200,   131,   127,   145,   181,   182,
     252,   197,   181,   182,    29,   195,   146,   147,   145,   229,
     230,    27,    28,    49,    64,    65,    69,    78,    79,   109,
     253,   181,     8,    20,   244,   147,     8,     8,   157,   157,
     157,   281,   139,   281,   146,   146,   281,     8,   146,   146,
     176,   181,   182,     9,   145,    81,   142,     8,   181,   182,
       8,   181,   182,   181,   182,   181,   182,   181,   182,   181,
     182,    26,    60,   181,   145,   181,   181,   181,   181,   181,
     181,    26,    60,   147,   162,   146,   146,   147,   210,   169,
     146,     5,   186,   126,   189,   190,    60,   146,   145,    36,
     119,    91,   157,   201,   202,   145,     8,   259,   146,   183,
      89,   179,   193,   194,   227,   145,   231,   232,    82,   147,
     233,    72,   157,   245,   232,   243,   146,   146,   146,   146,
     146,   146,   146,   128,   128,     8,   183,   184,    81,   181,
       9,   145,   128,   183,   181,   145,     5,    59,   157,   163,
     164,   146,   145,    52,    83,   166,    29,    55,    58,   188,
     157,   280,   281,   183,   131,   146,   147,     8,   146,   145,
      21,    41,   147,   146,   147,     3,   234,   235,   230,   181,
     182,   181,   182,   146,   146,   147,   128,   183,   184,   181,
     146,   128,   183,   147,   205,    29,    72,   167,   179,   187,
      83,   178,   191,   146,   146,   181,   202,   146,   146,   194,
     232,   131,   147,     8,   181,   146,   146,   181,   146,   164,
     146,   193,     5,   147,    29,   195,     8,   235,   147,   147,
     179,   193,   197,   145,     5,   205,   146,   146
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

  case 132:

    { pParser->PushQuery(); ;}
    break;

  case 133:

    { pParser->PushQuery(); ;}
    break;

  case 134:

    { pParser->PushQuery(); ;}
    break;

  case 138:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 140:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 143:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 144:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 148:

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

  case 149:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 151:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 152:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 153:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 156:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 159:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 160:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 161:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 162:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 163:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 164:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 165:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 166:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 167:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 168:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 169:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 171:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 178:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 180:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 181:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 182:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 183:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_bExclude = true;
		;}
    break;

  case 184:

    {
			if ( !pParser->AddStringListFilter ( yyvsp[-4], yyvsp[-1], false ) )
				YYERROR;
		;}
    break;

  case 185:

    {
			if ( !pParser->AddStringListFilter ( yyvsp[-5], yyvsp[-1], true ) )
				YYERROR;
		;}
    break;

  case 186:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 187:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 188:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 189:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-5], yyvsp[-3].m_iValue, yyvsp[-1].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 190:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 191:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 192:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 193:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 194:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 195:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true, true ) )
				YYERROR;
		;}
    break;

  case 196:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 197:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 198:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 199:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 200:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 201:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 202:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 203:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 204:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 205:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 206:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 207:

    {
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-2], SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( yyvsp[-2].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 208:

    {
			// tricky bit
			// any(tags!=val) is not equivalent to not(any(tags==val))
			// any(tags!=val) is instead equivalent to not(all(tags)==val)
			// thus, along with setting the exclude flag on, we also need to invert the function
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-2], SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( yyvsp[-2].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 209:

    {
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-4], SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( yyvsp[-4].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			f->m_dValues.Uniq();
		;}
    break;

  case 210:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-5], SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( yyvsp[-5].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			f->m_dValues.Uniq();
		;}
    break;

  case 211:

    {
			AddMvaRange ( pParser, yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 212:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-5], SPH_FILTER_RANGE );
			f->m_eMvaFunc = ( yyvsp[-5].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_iMinValue = yyvsp[-2].m_iValue;
			f->m_iMaxValue = yyvsp[0].m_iValue;
		;}
    break;

  case 213:

    {
			AddMvaRange ( pParser, yyvsp[-2], INT64_MIN, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 214:

    {
			AddMvaRange ( pParser, yyvsp[-2], yyvsp[0].m_iValue+1, INT64_MAX );
		;}
    break;

  case 215:

    {
			AddMvaRange ( pParser, yyvsp[-2], INT64_MIN, yyvsp[0].m_iValue );
		;}
    break;

  case 216:

    {
			AddMvaRange ( pParser, yyvsp[-2], yyvsp[0].m_iValue, INT64_MAX );
		;}
    break;

  case 218:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 219:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 220:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 221:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 222:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 228:

    { yyval = yyvsp[-1]; yyval.m_iType = TOK_ANY; ;}
    break;

  case 229:

    { yyval = yyvsp[-1]; yyval.m_iType = TOK_ALL; ;}
    break;

  case 230:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 231:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 232:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 233:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 234:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 235:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 236:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 237:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 238:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 242:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 243:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 244:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 246:

    {
			pParser->AddHaving();
		;}
    break;

  case 249:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 252:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 253:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 255:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 257:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 258:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 261:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 262:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 268:

    {
			if ( !pParser->AddOption ( yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 269:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 270:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 271:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 272:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 273:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 274:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 275:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 276:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 278:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 279:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 284:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 285:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 286:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 287:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 288:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 289:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 290:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 291:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 292:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 293:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 294:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 295:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 296:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 297:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 298:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 299:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 300:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 301:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 302:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 303:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 304:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 308:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 309:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 310:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 311:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 312:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 313:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 314:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 315:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 316:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 317:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 318:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 319:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 320:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 321:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 322:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 323:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 331:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 332:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 339:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 340:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 341:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 342:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 343:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 344:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 345:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 346:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 347:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 348:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 349:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 350:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 351:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 352:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 356:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 357:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 358:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 359:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 360:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 361:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 362:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 363:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 364:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 365:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 366:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 369:

    { yyval.m_iValue = 1; ;}
    break;

  case 370:

    { yyval.m_iValue = 0; ;}
    break;

  case 371:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 379:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 380:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 381:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 384:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 385:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 386:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 391:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 392:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 395:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 396:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 397:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 398:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 399:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 400:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 401:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 402:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 407:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 408:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 409:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 410:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 411:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 413:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 414:

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

  case 415:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 418:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 420:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 425:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 428:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 429:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 430:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 433:

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

  case 434:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 435:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 436:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 437:

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

  case 438:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 439:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 440:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 441:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 442:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 443:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 444:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 445:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 446:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 447:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 448:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 449:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 450:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 451:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 452:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 459:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 460:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 461:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 469:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 470:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 471:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 472:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 473:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 474:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 475:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 476:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 477:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 478:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 479:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 480:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 483:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 484:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 485:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 486:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 487:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 488:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 491:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 493:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 494:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 495:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 496:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 497:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 498:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 499:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 503:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 505:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 508:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
		;}
    break;

  case 510:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 511:

    {
			pParser->m_pStmt->m_eStmt = STMT_RELOAD_INDEX;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1]);
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

