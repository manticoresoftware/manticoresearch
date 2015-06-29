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
#define YYFINAL  208
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4936

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  153
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  143
/* YYNRULES -- Number of rules. */
#define YYNRULES  467
/* YYNRULES -- Number of states. */
#define YYNSTATES  862

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
     180,   184,   187,   189,   191,   193,   202,   204,   214,   215,
     218,   220,   224,   226,   228,   230,   231,   235,   236,   239,
     244,   256,   258,   262,   264,   267,   268,   270,   273,   275,
     280,   285,   290,   295,   300,   305,   309,   315,   317,   321,
     322,   324,   327,   329,   333,   337,   342,   344,   348,   352,
     358,   365,   369,   374,   380,   387,   391,   395,   399,   403,
     407,   409,   415,   421,   427,   431,   435,   439,   443,   447,
     451,   455,   460,   464,   468,   474,   481,   487,   494,   498,
     502,   506,   510,   514,   516,   518,   523,   527,   531,   533,
     535,   540,   545,   550,   554,   559,   564,   566,   569,   571,
     574,   576,   578,   582,   584,   588,   590,   592,   593,   598,
     599,   601,   603,   607,   608,   611,   612,   614,   620,   621,
     623,   627,   633,   635,   639,   641,   644,   647,   648,   650,
     653,   658,   659,   661,   664,   666,   670,   672,   676,   680,
     686,   693,   697,   699,   703,   707,   709,   711,   713,   715,
     717,   719,   721,   724,   727,   731,   735,   739,   743,   747,
     751,   755,   759,   763,   767,   771,   775,   779,   783,   787,
     791,   795,   799,   803,   805,   807,   809,   813,   818,   823,
     828,   833,   838,   843,   848,   852,   859,   866,   870,   879,
     894,   898,   903,   905,   909,   911,   913,   915,   917,   919,
     923,   929,   931,   933,   935,   937,   940,   941,   944,   946,
     949,   952,   956,   958,   960,   962,   965,   970,   975,   979,
     984,   989,   991,   993,   994,   997,  1002,  1007,  1012,  1016,
    1021,  1026,  1034,  1040,  1046,  1056,  1058,  1060,  1062,  1064,
    1066,  1068,  1072,  1074,  1076,  1078,  1080,  1082,  1084,  1086,
    1088,  1090,  1093,  1101,  1103,  1105,  1106,  1110,  1112,  1114,
    1116,  1120,  1122,  1126,  1130,  1132,  1136,  1138,  1140,  1142,
    1146,  1149,  1150,  1153,  1155,  1159,  1163,  1168,  1175,  1177,
    1181,  1183,  1187,  1189,  1193,  1194,  1197,  1199,  1203,  1207,
    1208,  1210,  1212,  1214,  1218,  1220,  1222,  1226,  1230,  1237,
    1239,  1243,  1247,  1251,  1257,  1262,  1266,  1270,  1272,  1274,
    1276,  1278,  1280,  1282,  1284,  1286,  1294,  1301,  1306,  1311,
    1317,  1318,  1320,  1323,  1325,  1329,  1333,  1336,  1340,  1347,
    1348,  1350,  1352,  1355,  1358,  1361,  1363,  1371,  1373,  1375,
    1377,  1379,  1383,  1390,  1394,  1398,  1401,  1405,  1407,  1411,
    1414,  1418,  1422,  1430,  1436,  1442,  1444,  1446,  1449,  1451,
    1454,  1456,  1458,  1462,  1466,  1470,  1474,  1476,  1477,  1480,
    1482,  1485,  1487,  1489,  1493,  1499,  1500,  1503
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     154,     0,    -1,   155,    -1,   158,    -1,   158,   144,    -1,
     226,    -1,   238,    -1,   218,    -1,   219,    -1,   224,    -1,
     239,    -1,   248,    -1,   250,    -1,   251,    -1,   252,    -1,
     257,    -1,   262,    -1,   263,    -1,   267,    -1,   269,    -1,
     270,    -1,   271,    -1,   272,    -1,   273,    -1,   264,    -1,
     274,    -1,   276,    -1,   277,    -1,   278,    -1,   256,    -1,
     279,    -1,   280,    -1,   281,    -1,   295,    -1,     3,    -1,
      16,    -1,    19,    -1,    18,    -1,    22,    -1,    23,    -1,
      24,    -1,    25,    -1,    28,    -1,    33,    -1,    37,    -1,
      50,    -1,    53,    -1,    54,    -1,    55,    -1,    56,    -1,
      57,    -1,    62,    -1,    68,    -1,    70,    -1,    71,    -1,
      73,    -1,    74,    -1,    75,    -1,    85,    -1,    86,    -1,
      87,    -1,    88,    -1,    90,    -1,    89,    -1,    91,    -1,
      93,    -1,    94,    -1,    97,    -1,    98,    -1,    99,    -1,
     101,    -1,   104,    -1,   107,    -1,   108,    -1,   109,    -1,
     110,    -1,   112,    -1,   113,    -1,   117,    -1,   118,    -1,
     119,    -1,   122,    -1,   123,    -1,   124,    -1,   126,    -1,
      69,    -1,   156,    -1,    80,    -1,   115,    -1,   159,    -1,
     158,   144,   159,    -1,   158,   293,    -1,   160,    -1,   213,
      -1,   161,    -1,   100,     3,   145,   145,   161,   146,   162,
     146,    -1,   168,    -1,   100,   169,    52,   145,   165,   168,
     146,   166,   167,    -1,    -1,   147,   163,    -1,   164,    -1,
     163,   147,   164,    -1,   157,    -1,     5,    -1,    59,    -1,
      -1,    83,    29,   195,    -1,    -1,    72,     5,    -1,    72,
       5,   147,     5,    -1,   100,   169,    52,   173,   174,   187,
     191,   190,   193,   197,   199,    -1,   170,    -1,   169,   147,
     170,    -1,   139,    -1,   172,   171,    -1,    -1,   157,    -1,
      20,   157,    -1,   205,    -1,    24,   145,   205,   146,    -1,
      74,   145,   205,   146,    -1,    76,   145,   205,   146,    -1,
     110,   145,   205,   146,    -1,    57,   145,   205,   146,    -1,
      37,   145,   139,   146,    -1,    56,   145,   146,    -1,    37,
     145,    43,   157,   146,    -1,   157,    -1,   173,   147,   157,
      -1,    -1,   175,    -1,   125,   176,    -1,   177,    -1,   176,
     128,   176,    -1,   145,   176,   146,    -1,    73,   145,     8,
     146,    -1,   178,    -1,   180,   131,   182,    -1,   180,   132,
     182,    -1,   180,    60,   145,   186,   146,    -1,   180,   142,
      60,   145,   186,   146,    -1,   180,    60,     9,    -1,   180,
     142,    60,     9,    -1,   180,    26,   182,   128,   182,    -1,
     180,   142,    26,   182,   128,   182,    -1,   180,   134,   182,
      -1,   180,   133,   182,    -1,   180,   135,   182,    -1,   180,
     136,   182,    -1,   180,   131,   183,    -1,   179,    -1,   180,
      26,   183,   128,   183,    -1,   180,    26,   182,   128,   183,
      -1,   180,    26,   183,   128,   182,    -1,   180,   134,   183,
      -1,   180,   133,   183,    -1,   180,   135,   183,    -1,   180,
     136,   183,    -1,   180,   131,     8,    -1,   180,   132,     8,
      -1,   180,    67,    81,    -1,   180,    67,   142,    81,    -1,
     181,   131,   182,    -1,   181,   132,   182,    -1,   181,    60,
     145,   184,   146,    -1,   181,   142,    60,   145,   184,   146,
      -1,   181,    26,   182,   128,   182,    -1,   181,   142,    26,
     182,   128,   182,    -1,   181,   133,   182,    -1,   181,   134,
     182,    -1,   181,   136,   182,    -1,   181,   135,   182,    -1,
     180,   132,   183,    -1,   157,    -1,     4,    -1,    37,   145,
     139,   146,    -1,    56,   145,   146,    -1,   124,   145,   146,
      -1,    59,    -1,   283,    -1,    65,   145,   283,   146,    -1,
      45,   145,   283,   146,    -1,    27,   145,   283,   146,    -1,
      47,   145,   146,    -1,    19,   145,   157,   146,    -1,    18,
     145,   157,   146,    -1,     5,    -1,   138,     5,    -1,     6,
      -1,   138,     6,    -1,    14,    -1,   182,    -1,   184,   147,
     182,    -1,     8,    -1,   185,   147,     8,    -1,   184,    -1,
     185,    -1,    -1,    55,   188,    29,   189,    -1,    -1,     5,
      -1,   180,    -1,   189,   147,   180,    -1,    -1,    58,   178,
      -1,    -1,   192,    -1,   126,    55,    83,    29,   195,    -1,
      -1,   194,    -1,    83,    29,   195,    -1,    83,    29,    89,
     145,   146,    -1,   196,    -1,   195,   147,   196,    -1,   180,
      -1,   180,    21,    -1,   180,    41,    -1,    -1,   198,    -1,
      72,     5,    -1,    72,     5,   147,     5,    -1,    -1,   200,
      -1,    82,   201,    -1,   202,    -1,   201,   147,   202,    -1,
     157,    -1,   157,   131,   157,    -1,   157,   131,     5,    -1,
     157,   131,   145,   203,   146,    -1,   157,   131,   157,   145,
       8,   146,    -1,   157,   131,     8,    -1,   204,    -1,   203,
     147,   204,    -1,   157,   131,   182,    -1,   157,    -1,     4,
      -1,    59,    -1,     5,    -1,     6,    -1,    14,    -1,     9,
      -1,   138,   205,    -1,   142,   205,    -1,   205,   137,   205,
      -1,   205,   138,   205,    -1,   205,   139,   205,    -1,   205,
     140,   205,    -1,   205,   133,   205,    -1,   205,   134,   205,
      -1,   205,   130,   205,    -1,   205,   129,   205,    -1,   205,
     141,   205,    -1,   205,    44,   205,    -1,   205,    77,   205,
      -1,   205,   136,   205,    -1,   205,   135,   205,    -1,   205,
     131,   205,    -1,   205,   132,   205,    -1,   205,   128,   205,
      -1,   205,   127,   205,    -1,   145,   205,   146,    -1,   148,
     210,   149,    -1,   206,    -1,   283,    -1,   286,    -1,   282,
      67,    81,    -1,   282,    67,   142,    81,    -1,     3,   145,
     207,   146,    -1,    60,   145,   207,   146,    -1,    65,   145,
     207,   146,    -1,    27,   145,   207,   146,    -1,    49,   145,
     207,   146,    -1,    45,   145,   207,   146,    -1,     3,   145,
     146,    -1,    76,   145,   205,   147,   205,   146,    -1,    74,
     145,   205,   147,   205,   146,    -1,   124,   145,   146,    -1,
     209,   145,   205,    51,   157,    60,   282,   146,    -1,    96,
     145,   205,   147,   205,   147,   145,   207,   146,   147,   145,
     207,   146,   146,    -1,    89,   145,   146,    -1,    89,   145,
     207,   146,    -1,   208,    -1,   207,   147,   208,    -1,   205,
      -1,     8,    -1,    19,    -1,    18,    -1,    62,    -1,   211,
     131,   212,    -1,   210,   147,   211,   131,   212,    -1,   157,
      -1,    60,    -1,   182,    -1,   157,    -1,   105,   215,    -1,
      -1,    71,     8,    -1,   123,    -1,   108,   214,    -1,    75,
     214,    -1,    16,   108,   214,    -1,    88,    -1,    85,    -1,
      87,    -1,   113,   199,    -1,    16,     8,   108,   214,    -1,
      16,   157,   108,   214,    -1,   216,   157,   108,    -1,   216,
     157,   217,   103,    -1,   216,   157,    14,   103,    -1,    61,
      -1,   111,    -1,    -1,    32,     5,    -1,   102,   156,   131,
     221,    -1,   102,   156,   131,   220,    -1,   102,   156,   131,
      81,    -1,   102,    80,   222,    -1,   102,    10,   131,   222,
      -1,   102,    31,   102,   222,    -1,   102,    54,     9,   131,
     145,   184,   146,    -1,   102,    54,   156,   131,   220,    -1,
     102,    54,   156,   131,     5,    -1,   102,    61,   157,    54,
       9,   131,   145,   184,   146,    -1,   157,    -1,     8,    -1,
     116,    -1,    48,    -1,   182,    -1,   223,    -1,   222,   138,
     223,    -1,   157,    -1,    81,    -1,     8,    -1,     5,    -1,
       6,    -1,    35,    -1,    98,    -1,   225,    -1,    25,    -1,
     107,   115,    -1,   227,    66,   157,   228,   121,   231,   235,
      -1,    63,    -1,    95,    -1,    -1,   145,   230,   146,    -1,
     157,    -1,    59,    -1,   229,    -1,   230,   147,   229,    -1,
     232,    -1,   231,   147,   232,    -1,   145,   233,   146,    -1,
     234,    -1,   233,   147,   234,    -1,   182,    -1,   183,    -1,
       8,    -1,   145,   184,   146,    -1,   145,   146,    -1,    -1,
      82,   236,    -1,   237,    -1,   236,   147,   237,    -1,     3,
     131,     8,    -1,    40,    52,   173,   175,    -1,    30,   157,
     145,   240,   243,   146,    -1,   241,    -1,   240,   147,   241,
      -1,   234,    -1,   145,   242,   146,    -1,     8,    -1,   242,
     147,     8,    -1,    -1,   147,   244,    -1,   245,    -1,   244,
     147,   245,    -1,   234,   246,   247,    -1,    -1,    20,    -1,
     157,    -1,    72,    -1,   249,   157,   214,    -1,    42,    -1,
      41,    -1,   105,   112,   214,    -1,   105,    39,   214,    -1,
     120,   173,   102,   253,   175,   199,    -1,   254,    -1,   253,
     147,   254,    -1,   157,   131,   182,    -1,   157,   131,   183,
      -1,   157,   131,   145,   184,   146,    -1,   157,   131,   145,
     146,    -1,   283,   131,   182,    -1,   283,   131,   183,    -1,
      65,    -1,    27,    -1,    49,    -1,    28,    -1,    78,    -1,
      79,    -1,    69,    -1,   109,    -1,    17,   111,   157,    15,
      34,   157,   255,    -1,    17,   111,   157,    46,    34,   157,
      -1,    17,    99,   157,    92,    -1,   105,   265,   122,   258,
      -1,   105,   265,   122,    71,     8,    -1,    -1,   259,    -1,
     125,   260,    -1,   261,    -1,   260,   127,   261,    -1,   157,
     131,     8,    -1,   105,    33,    -1,   105,    31,   102,    -1,
     102,   265,   115,    68,    70,   266,    -1,    -1,    54,    -1,
     104,    -1,    91,   119,    -1,    91,    36,    -1,    94,    91,
      -1,   101,    -1,    38,    53,   157,    97,   268,   106,     8,
      -1,    64,    -1,    27,    -1,    49,    -1,   109,    -1,    46,
      53,   157,    -1,    22,    61,   157,   114,    99,   157,    -1,
      50,    99,   157,    -1,    50,    90,   157,    -1,    50,    23,
      -1,   100,   275,   197,    -1,    10,    -1,    10,   150,   157,
      -1,   100,   205,    -1,   117,    99,   157,    -1,    84,    61,
     157,    -1,    38,    86,   157,   118,     8,   106,     8,    -1,
      46,    86,   157,   118,     8,    -1,    93,    87,    52,   106,
       8,    -1,   283,    -1,   157,    -1,   157,   284,    -1,   285,
      -1,   284,   285,    -1,    13,    -1,    14,    -1,   151,   205,
     152,    -1,   151,     8,   152,    -1,   205,   131,   287,    -1,
     287,   131,   205,    -1,     8,    -1,    -1,   289,   292,    -1,
      29,    -1,   291,   171,    -1,   205,    -1,   290,    -1,   292,
     147,   290,    -1,    47,   292,   288,   193,   197,    -1,    -1,
      52,     8,    -1,    93,    61,   157,   294,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   187,   187,   188,   189,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   232,   233,   233,   233,   233,   233,
     233,   233,   233,   234,   234,   234,   234,   235,   235,   235,
     235,   235,   235,   236,   236,   236,   236,   236,   237,   237,
     237,   237,   237,   237,   237,   238,   238,   238,   238,   238,
     239,   239,   239,   239,   239,   240,   240,   240,   240,   240,
     240,   241,   241,   241,   241,   242,   246,   246,   246,   252,
     253,   254,   258,   259,   263,   264,   272,   273,   280,   282,
     286,   290,   297,   298,   299,   303,   316,   323,   325,   330,
     339,   355,   356,   360,   361,   364,   366,   367,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   383,   384,   387,
     389,   393,   397,   398,   399,   403,   408,   412,   419,   427,
     436,   446,   451,   456,   461,   466,   471,   476,   481,   486,
     491,   496,   501,   506,   511,   516,   521,   526,   531,   536,
     541,   546,   554,   560,   571,   578,   587,   591,   600,   604,
     608,   612,   619,   623,   624,   629,   635,   641,   647,   653,
     654,   655,   656,   657,   661,   662,   666,   667,   678,   679,
     680,   684,   690,   697,   703,   710,   711,   714,   716,   719,
     721,   728,   732,   738,   740,   746,   748,   752,   763,   765,
     769,   773,   780,   781,   785,   786,   787,   790,   792,   796,
     801,   808,   810,   814,   818,   819,   823,   828,   833,   838,
     844,   849,   857,   862,   869,   879,   880,   881,   882,   883,
     884,   885,   886,   887,   889,   890,   891,   892,   893,   894,
     895,   896,   897,   898,   899,   900,   901,   902,   903,   904,
     905,   906,   907,   908,   909,   910,   911,   912,   916,   917,
     918,   919,   920,   921,   922,   923,   924,   925,   926,   927,
     928,   929,   933,   934,   938,   939,   943,   944,   945,   949,
     950,   954,   955,   959,   960,   966,   969,   971,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   988,   993,   998,
    1003,  1012,  1013,  1016,  1018,  1026,  1031,  1036,  1041,  1042,
    1043,  1047,  1052,  1057,  1062,  1071,  1072,  1076,  1077,  1078,
    1090,  1091,  1095,  1096,  1097,  1098,  1099,  1106,  1107,  1108,
    1112,  1113,  1119,  1127,  1128,  1131,  1133,  1137,  1138,  1142,
    1143,  1147,  1148,  1152,  1156,  1157,  1161,  1162,  1163,  1164,
    1165,  1168,  1169,  1173,  1174,  1178,  1184,  1194,  1202,  1206,
    1213,  1214,  1221,  1231,  1237,  1239,  1243,  1248,  1252,  1259,
    1261,  1265,  1266,  1272,  1280,  1281,  1287,  1291,  1297,  1305,
    1306,  1310,  1324,  1330,  1334,  1339,  1353,  1364,  1365,  1366,
    1367,  1368,  1369,  1370,  1371,  1375,  1383,  1390,  1401,  1405,
    1412,  1413,  1417,  1421,  1422,  1426,  1430,  1437,  1444,  1450,
    1451,  1452,  1456,  1457,  1458,  1459,  1465,  1476,  1477,  1478,
    1479,  1484,  1495,  1507,  1516,  1525,  1535,  1543,  1544,  1548,
    1558,  1569,  1580,  1591,  1601,  1612,  1613,  1617,  1620,  1621,
    1625,  1626,  1627,  1628,  1632,  1633,  1637,  1642,  1644,  1648,
    1657,  1661,  1669,  1670,  1674,  1685,  1687,  1694
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
  "where_item", "filter_item", "expr_float_unhandled", "expr_ident", 
  "mva_aggr", "const_int", "const_float", "const_list", "string_list", 
  "const_list_or_string_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_having_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "json_aggr", "consthash", "hash_key", "hash_val", "show_stmt", 
  "like_filter", "show_what", "index_or_table", "opt_chunk", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "set_value", 
  "simple_set_value", "transact_op", "start_transaction", "insert_into", 
  "insert_or_replace", "opt_column_list", "column_ident", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "opt_insert_options", "insert_options_list", "insert_option", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "show_databases", "update", "update_items_list", "update_item", 
  "alter_col_type", "alter", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "show_character_set", 
  "set_transaction", "opt_scope", "isolation_level", "create_function", 
  "udf_type", "drop_function", "attach_index", "flush_rtindex", 
  "flush_ramchunk", "flush_index", "select_sysvar", "sysvar_name", 
  "select_dual", "truncate", "optimize_index", "create_plugin", 
  "drop_plugin", "reload_plugins", "json_field", "json_expr", "subscript", 
  "subkey", "streq", "strval", "opt_facet_by_items_list", "facet_by", 
  "facet_item", "facet_expr", "facet_items_list", "facet_stmt", 
  "opt_reload_index_from", "reload_index", 0
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
     156,   156,   156,   156,   156,   156,   157,   157,   157,   158,
     158,   158,   159,   159,   160,   160,   161,   161,   162,   162,
     163,   163,   164,   164,   164,   165,   166,   167,   167,   167,
     168,   169,   169,   170,   170,   171,   171,   171,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   173,   173,   174,
     174,   175,   176,   176,   176,   177,   177,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   179,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   181,   181,   182,   182,   183,   183,
     183,   184,   184,   185,   185,   186,   186,   187,   187,   188,
     188,   189,   189,   190,   190,   191,   191,   192,   193,   193,
     194,   194,   195,   195,   196,   196,   196,   197,   197,   198,
     198,   199,   199,   200,   201,   201,   202,   202,   202,   202,
     202,   202,   203,   203,   204,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   207,   207,   208,   208,   209,   209,   209,   210,
     210,   211,   211,   212,   212,   213,   214,   214,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   216,   216,   217,   217,   218,   218,   218,   218,   218,
     218,   219,   219,   219,   219,   220,   220,   221,   221,   221,
     222,   222,   223,   223,   223,   223,   223,   224,   224,   224,
     225,   225,   226,   227,   227,   228,   228,   229,   229,   230,
     230,   231,   231,   232,   233,   233,   234,   234,   234,   234,
     234,   235,   235,   236,   236,   237,   238,   239,   240,   240,
     241,   241,   242,   242,   243,   243,   244,   244,   245,   246,
     246,   247,   247,   248,   249,   249,   250,   251,   252,   253,
     253,   254,   254,   254,   254,   254,   254,   255,   255,   255,
     255,   255,   255,   255,   255,   256,   256,   256,   257,   257,
     258,   258,   259,   260,   260,   261,   262,   263,   264,   265,
     265,   265,   266,   266,   266,   266,   267,   268,   268,   268,
     268,   269,   270,   271,   272,   273,   274,   275,   275,   276,
     277,   278,   279,   280,   281,   282,   282,   283,   284,   284,
     285,   285,   285,   285,   286,   286,   287,   288,   288,   289,
     290,   291,   292,   292,   293,   294,   294,   295
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
       3,     2,     1,     1,     1,     8,     1,     9,     0,     2,
       1,     3,     1,     1,     1,     0,     3,     0,     2,     4,
      11,     1,     3,     1,     2,     0,     1,     2,     1,     4,
       4,     4,     4,     4,     4,     3,     5,     1,     3,     0,
       1,     2,     1,     3,     3,     4,     1,     3,     3,     5,
       6,     3,     4,     5,     6,     3,     3,     3,     3,     3,
       1,     5,     5,     5,     3,     3,     3,     3,     3,     3,
       3,     4,     3,     3,     5,     6,     5,     6,     3,     3,
       3,     3,     3,     1,     1,     4,     3,     3,     1,     1,
       4,     4,     4,     3,     4,     4,     1,     2,     1,     2,
       1,     1,     3,     1,     3,     1,     1,     0,     4,     0,
       1,     1,     3,     0,     2,     0,     1,     5,     0,     1,
       3,     5,     1,     3,     1,     2,     2,     0,     1,     2,
       4,     0,     1,     2,     1,     3,     1,     3,     3,     5,
       6,     3,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     3,     4,     4,     4,
       4,     4,     4,     4,     3,     6,     6,     3,     8,    14,
       3,     4,     1,     3,     1,     1,     1,     1,     1,     3,
       5,     1,     1,     1,     1,     2,     0,     2,     1,     2,
       2,     3,     1,     1,     1,     2,     4,     4,     3,     4,
       4,     1,     1,     0,     2,     4,     4,     4,     3,     4,
       4,     7,     5,     5,     9,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     7,     1,     1,     0,     3,     1,     1,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     1,     3,
       2,     0,     2,     1,     3,     3,     4,     6,     1,     3,
       1,     3,     1,     3,     0,     2,     1,     3,     3,     0,
       1,     1,     1,     3,     1,     1,     3,     3,     6,     1,
       3,     3,     3,     5,     4,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     7,     6,     4,     4,     5,
       0,     1,     2,     1,     3,     3,     2,     3,     6,     0,
       1,     1,     2,     2,     2,     1,     7,     1,     1,     1,
       1,     3,     6,     3,     3,     2,     3,     1,     3,     2,
       3,     3,     7,     5,     5,     1,     1,     2,     1,     2,
       1,     1,     3,     3,     3,     3,     1,     0,     2,     1,
       2,     1,     1,     3,     5,     0,     2,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   340,     0,   337,     0,     0,   385,   384,
       0,     0,   343,     0,     0,   344,   338,     0,   419,   419,
       0,     0,     0,     0,     2,     3,    89,    92,    94,    96,
      93,     7,     8,     9,   339,     5,     0,     6,    10,    11,
       0,    12,    13,    14,    29,    15,    16,    17,    24,    18,
      19,    20,    21,    22,    23,    25,    26,    27,    28,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    37,    36,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    85,    53,    54,    55,    56,
      57,    87,    58,    59,    60,    61,    63,    62,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    88,    78,    79,    80,    81,    82,    83,    84,
      86,     0,     0,     0,     0,     0,     0,   435,     0,     0,
       0,     0,     0,    34,   236,   238,   239,   456,   241,   437,
     240,    37,    36,    40,     0,    44,     0,     0,    49,    50,
     237,     0,    51,     0,    56,     0,    63,     0,    75,    83,
       0,   113,     0,     0,     0,   235,     0,   111,   115,   118,
     263,     0,   217,     0,   264,   265,     0,     0,     0,    47,
       0,     0,    71,     0,     0,     0,     0,   416,   296,   420,
     311,   296,   303,   304,   302,   421,   296,   312,   296,   221,
     298,   295,     0,     0,   341,     0,   127,     0,     1,     0,
       4,    91,     0,   296,     0,     0,     0,     0,     0,     0,
       0,   431,     0,   434,   433,   441,   465,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    34,    56,     0,   242,   243,
       0,   292,   291,     0,     0,   450,   451,     0,   447,   448,
       0,     0,     0,   116,   114,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   436,   218,     0,     0,     0,     0,
       0,     0,     0,   335,   336,   334,   333,   332,   318,   330,
       0,     0,     0,   296,     0,   417,     0,   387,   300,   299,
     386,     0,   305,   222,   313,   410,   440,     0,     0,   461,
     462,   115,   457,     0,     0,    90,   345,   383,   407,     0,
       0,     0,   186,   188,   358,   190,     0,     0,   356,   357,
     370,   374,   368,     0,     0,     0,   366,     0,     0,   467,
       0,   285,     0,   274,   284,     0,   282,   438,     0,     0,
       0,     0,     0,     0,   125,     0,     0,     0,     0,     0,
     280,     0,     0,     0,   277,     0,     0,     0,   261,     0,
     262,     0,   456,     0,   449,   105,   129,   112,   118,   117,
     253,   254,   260,   259,   251,   250,   257,   454,   258,   248,
     249,   256,   255,   244,   245,   246,   247,   252,     0,   219,
     266,     0,   455,   319,   320,     0,     0,     0,     0,   326,
     328,   317,   327,     0,   325,   329,   316,   315,     0,   296,
     301,   296,   297,   226,   223,   224,     0,     0,   308,     0,
       0,     0,   408,   411,     0,     0,   389,     0,   128,   460,
     459,     0,   208,     0,     0,     0,     0,     0,     0,   187,
     189,   372,   360,   191,     0,     0,     0,     0,   428,   429,
     427,   430,     0,     0,   174,    37,    36,     0,    44,     0,
       0,    49,   178,     0,    55,    83,     0,   173,   131,   132,
     136,   150,     0,     0,   179,   443,   466,   444,     0,     0,
     268,     0,   119,   271,     0,   124,   273,   272,   123,   269,
     270,   120,     0,   121,     0,   281,     0,   122,     0,     0,
       0,   294,   293,   289,   453,   452,     0,   197,   130,     0,
       0,   267,     0,   323,   322,     0,   331,     0,   306,   307,
       0,     0,   310,   314,   309,   409,     0,   412,   413,     0,
       0,   221,     0,   463,     0,   217,   209,   458,   348,   347,
     349,     0,     0,     0,   406,   432,   359,     0,   371,     0,
     379,   369,   375,   376,   367,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    98,   283,
     126,     0,     0,     0,     0,     0,     0,   199,   205,     0,
     220,     0,     0,     0,     0,   425,   418,   228,   231,     0,
     227,   225,     0,     0,     0,   391,   392,   390,   388,   395,
     396,     0,   464,   346,     0,     0,   361,   351,   398,   400,
     399,   397,   403,   401,   402,   404,   405,   192,   373,   380,
       0,     0,   426,   442,     0,     0,     0,     0,     0,     0,
     183,   176,     0,     0,   177,   134,   133,     0,     0,   141,
       0,   160,     0,   158,   137,   149,   159,   138,   172,   146,
     155,   145,   154,   147,   156,   148,   157,     0,     0,     0,
       0,   162,   163,   168,   169,   171,   170,     0,     0,     0,
       0,   276,   275,     0,   290,     0,     0,   200,     0,     0,
     203,   206,     0,   321,     0,   423,   422,   424,     0,     0,
     232,     0,   415,   414,   394,     0,    63,   214,   210,   212,
     350,     0,     0,   354,     0,     0,   342,   382,   381,   378,
     379,   377,   185,   184,   182,   175,   181,   180,   135,     0,
       0,   193,   195,   196,     0,   161,     0,   142,     0,     0,
       0,     0,     0,   103,   104,   102,    99,   100,    95,     0,
       0,     0,   107,     0,     0,     0,   208,   446,     0,   445,
       0,     0,   229,     0,     0,   393,     0,   215,   216,     0,
     353,     0,     0,   362,   363,   352,   143,   152,   153,   151,
       0,   139,     0,     0,   166,   164,     0,     0,     0,     0,
       0,     0,    97,   201,   198,     0,   204,   217,   278,   324,
     234,   233,   230,   211,   213,   355,     0,     0,   194,   144,
     140,   167,   165,   101,     0,   106,   108,     0,     0,   221,
     365,   364,     0,     0,   202,   207,   110,     0,   109,     0,
       0,   279
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    23,    24,   120,   165,    25,    26,    27,    28,   710,
     776,   777,   526,   782,   822,    29,   166,   167,   264,   168,
     386,   527,   346,   488,   489,   490,   491,   492,   493,   463,
     339,   464,   763,   764,   618,   718,   824,   786,   720,   721,
     555,   556,   738,   739,   284,   285,   312,   313,   434,   435,
     729,   730,   354,   170,   355,   356,   171,   253,   254,   523,
      30,   307,   201,   202,   439,    31,    32,   426,   427,   298,
     299,    33,    34,    35,    36,   455,   560,   561,   646,   647,
     742,   340,   746,   803,   804,    37,    38,   341,   342,   465,
     467,   572,   573,   660,   749,    39,    40,    41,    42,    43,
     445,   446,   656,    44,    45,   442,   443,   547,   548,    46,
      47,    48,   184,   626,    49,   472,    50,    51,    52,    53,
      54,    55,   172,    56,    57,    58,    59,    60,    61,   173,
     174,   258,   259,   175,   176,   452,   453,   320,   321,   322,
     211,   349,    62
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -630
static const short yypact[] =
{
     698,   113,    26,  -630,  4286,  -630,    62,    49,  -630,  -630,
     120,    39,  -630,    43,   129,  -630,  -630,   876,  3385,   508,
      -7,    60,  4286,   211,  -630,    -1,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,   147,  -630,  -630,  -630,
    4286,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  4286,  4286,  4286,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,    70,  4286,  4286,  4286,  4286,  4286,  -630,  4286,  4286,
    4286,  4286,   170,   127,  -630,  -630,  -630,  -630,  -630,   133,
    -630,   135,   148,   158,   167,   171,   174,   180,   182,   206,
    -630,   208,   209,   210,   218,   220,   222,   224,   228,   229,
    1898,  -630,  1898,  1898,  3729,    38,    -5,  -630,  3840,   109,
    -630,   231,   193,   278,   285,  -630,   160,   240,   276,  4397,
    4286,  3274,   266,   252,   269,  3953,   283,  -630,   316,  -630,
    -630,   316,  -630,  -630,  -630,  -630,   316,  -630,   316,   306,
    -630,  -630,  4286,   273,  -630,  4286,  -630,   -19,  -630,  1898,
     104,  -630,  4286,   316,   305,    87,   290,    17,   308,   288,
     -41,  -630,   291,  -630,  -630,  -630,   356,   307,  1022,  4286,
    1898,  2044,    48,  2044,  2044,   270,  1898,  2044,  2044,  1898,
    1898,  1168,  1898,  1898,   271,   274,   275,   277,  -630,  -630,
    4518,  -630,  -630,   -54,   287,  -630,  -630,  2190,    51,  -630,
    2553,  1314,  4286,  -630,  -630,  1898,  1898,  1898,  1898,  1898,
    1898,  1898,  1898,  1898,  1898,  1898,  1898,  1898,  1898,  1898,
    1898,  1898,  1898,   416,  -630,  -630,   -12,  1898,  3274,  3274,
     293,   309,   384,  -630,  -630,  -630,  -630,  -630,   303,  -630,
    2677,   371,   335,     5,   336,  -630,   437,  -630,  -630,  -630,
    -630,  4286,  -630,  -630,    80,    14,  -630,  4286,  4286,  4714,
    -630,  3840,    -2,  1460,   207,  -630,   302,  -630,  -630,   412,
     414,   350,  -630,  -630,  -630,  -630,   146,    12,  -630,  -630,
    -630,   304,  -630,    47,   447,  2318,  -630,   449,   451,  -630,
     452,   330,  1606,  -630,  4714,   107,  -630,  -630,  4543,   115,
    4286,   317,   124,   128,  -630,  4567,   130,   132,   693,  4398,
    -630,   141,   736,  4588,  -630,  1752,  1898,  1898,  -630,  3729,
    -630,  2801,   310,   379,  -630,  -630,   -41,  -630,  4714,  -630,
    -630,  -630,  4730,  4795,   715,   295,   338,  -630,   338,    91,
      91,    91,    91,    55,    55,  -630,  -630,  -630,  4686,   318,
    -630,   383,   338,   303,   303,   321,  3507,   458,  3274,  -630,
    -630,  -630,  -630,   464,  -630,  -630,  -630,  -630,   410,   316,
    -630,   316,  -630,   352,   334,  -630,   382,   481,  -630,   385,
     479,  4286,  -630,  -630,    54,   -33,  -630,   358,  -630,  -630,
    -630,  1898,   407,  1898,  4064,   370,  4286,  4286,  4286,  -630,
    -630,  -630,  -630,  -630,   143,   151,    17,   346,  -630,  -630,
    -630,  -630,   387,   389,  -630,   351,   353,   354,   355,   357,
     376,   377,  -630,   378,   381,   390,  2318,    51,   369,  -630,
    -630,  -630,   200,   226,  -630,  -630,  -630,  -630,  1314,   388,
    -630,  2044,  -630,  -630,   392,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  1898,  -630,  1898,  -630,  1898,  -630,  4420,  4445,
     372,  -630,  -630,  -630,  -630,  -630,   401,   472,  -630,  4286,
     523,  -630,    28,  -630,  -630,   398,  -630,     9,  -630,  -630,
    2442,  4286,  -630,  -630,  -630,  -630,   399,   413,  -630,    34,
    4286,   306,    76,  -630,   513,   193,  -630,   396,  -630,  -630,
    -630,   153,   400,   301,  -630,  -630,  -630,    28,  -630,   536,
      46,  -630,   403,  -630,  -630,   540,   543,  4286,  4286,  4286,
     415,  4286,   406,   409,  4286,   545,   417,   -30,  2318,    76,
      20,    -6,    65,    72,    76,    76,    76,    76,    63,    28,
     419,    28,    28,    28,    28,    28,    28,    81,   411,  -630,
    -630,  4616,  4665,  4469,  2801,  1314,   421,   554,   434,   501,
    -630,   155,   420,    27,   485,  -630,  -630,  -630,  -630,  4286,
     427,  -630,   570,  4286,    11,  -630,  -630,  -630,  -630,  -630,
    -630,  2918,  -630,  -630,  4064,    29,   -10,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    4175,    29,  -630,  -630,   433,   435,    51,   436,   439,   440,
    -630,  -630,   441,   443,  -630,  -630,  -630,   462,   463,  -630,
      37,  -630,   511,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,    28,    35,   466,
      28,  -630,  -630,  -630,  -630,  -630,  -630,    28,   454,  3618,
     455,  -630,  -630,   457,  -630,    -3,   514,  -630,   571,   548,
     546,  -630,  4286,  -630,    28,  -630,  -630,  -630,   474,   159,
    -630,   598,  -630,  -630,  -630,   162,   469,   176,   460,  -630,
    -630,    23,   164,  -630,   605,   400,  -630,  -630,  -630,  -630,
     591,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,    76,
      76,  -630,   468,   471,   476,  -630,   495,  -630,    37,    28,
     175,   496,    28,  -630,  -630,  -630,   483,  -630,  -630,  2044,
    4286,   597,   556,  3035,   549,  3152,   407,    51,   487,  -630,
     177,    28,  -630,  4286,   488,  -630,   489,  -630,  -630,  3035,
    -630,    29,   505,   492,  -630,  -630,  -630,  -630,  -630,  -630,
     633,  -630,    28,   497,  -630,  -630,    28,   191,  3618,   197,
    3035,   637,  -630,  -630,   498,   617,  -630,   193,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,   641,   605,  -630,  -630,
    -630,  -630,  -630,  -630,   506,   460,   507,  3035,  3035,   306,
    -630,  -630,   510,   647,  -630,   460,  -630,  2044,  -630,   201,
     512,  -630
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -630,  -630,  -630,     3,    -4,  -630,   450,  -630,   311,  -630,
    -630,  -161,  -630,  -630,  -630,   136,    44,   405,   340,  -630,
      10,  -630,  -298,  -448,  -630,  -121,  -630,  -629,  -630,  -203,
    -539,  -517,  -630,  -101,  -630,  -630,  -630,  -630,  -630,  -630,
    -118,  -630,  -621,  -129,  -546,  -630,  -545,  -630,  -630,   131,
    -630,  -122,   408,  -630,  -230,   190,  -630,  -630,   313,    79,
    -630,  -172,  -630,  -630,  -630,  -630,  -630,   280,  -630,   -55,
     279,  -630,  -630,  -630,  -630,  -630,    50,  -630,  -630,   -47,
    -630,  -461,  -630,  -630,  -138,  -630,  -630,  -630,   234,  -630,
    -630,  -630,    40,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,   152,  -630,  -630,  -630,  -630,  -630,  -630,    71,  -630,
    -630,  -630,   684,  -630,  -630,  -630,  -630,  -630,  -630,  -630,
    -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,  -630,   -16,
    -315,  -630,   453,  -630,   438,  -630,  -630,   256,  -630,   255,
    -630,  -630,  -630
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -457
static const short yytable[] =
{
     121,   359,   447,   362,   363,   570,   638,   366,   367,   642,
     636,   371,   737,   640,   338,   621,   332,   332,   206,   308,
     461,   183,   332,   333,   309,   334,   310,   450,   332,   679,
     494,   335,   207,   332,   332,   333,   213,   334,   587,   332,
     333,   327,   332,   335,   767,   761,   209,   260,   335,   780,
     678,   255,   256,   685,   688,   690,   692,   694,   696,   214,
     215,   216,   127,   725,   255,   256,   659,   255,   256,   410,
     332,   333,   744,   683,   468,   681,   306,   332,   333,   335,
     686,   332,   333,   317,   345,   440,   335,    65,   528,   697,
     335,   360,   345,   379,   436,   380,   469,   425,   588,   265,
     623,   124,   329,   624,   130,  -446,   318,   707,   204,  -439,
     625,   470,   437,   -73,   550,   122,   675,   735,   218,   219,
     206,   221,   222,   698,   223,   224,   225,   226,   318,   128,
     411,   430,   266,   330,   220,   265,   682,   745,   129,   441,
     676,   708,   261,   210,   261,   451,   726,   551,   123,   423,
     423,   459,   460,   265,   823,   336,   471,   734,   462,   205,
     252,   423,   337,   762,   263,   680,   423,   336,   266,   462,
     737,   494,   336,   125,   741,   423,   292,   297,   522,   634,
     768,   304,   291,   770,   743,   549,   266,   361,   438,   257,
     131,   737,  -370,  -370,   279,   280,   281,   797,   314,   845,
     750,   316,   257,   336,   323,   257,   126,   790,   326,   324,
     336,   208,    63,   212,   336,   217,   132,   798,   854,   737,
     807,   809,   227,   185,    64,   357,   589,   855,   277,   278,
     279,   280,   281,   413,   414,   447,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   762,   599,   500,   501,   817,   206,   538,   389,   539,
     590,   503,   501,   338,   667,   283,   669,   591,   190,   672,
     506,   501,   228,   494,   507,   501,   509,   501,   510,   501,
    -287,   849,   191,   229,   297,   297,   600,   515,   501,   566,
     567,   287,   192,  -286,   193,   194,   424,   568,   569,   643,
     644,   723,   567,   230,   856,   792,   793,   433,   795,   567,
     800,   801,   231,   444,   448,   196,   232,   263,   197,   233,
     199,   815,   567,   829,   567,   234,   494,   235,   648,   649,
     200,   592,   593,   594,   595,   596,   597,   842,   567,   265,
     835,   487,   598,   844,   501,   286,   635,   860,   501,   639,
     650,   236,  -445,   237,  -288,   238,   504,   601,   602,   603,
     604,   605,   606,   239,   657,   240,   651,   241,   607,   242,
     652,   288,   266,   243,   244,   252,   282,   521,   289,   653,
     654,  -421,   265,   300,   301,   305,   677,   306,   311,   684,
     687,   689,   691,   693,   695,   315,   699,   328,   701,   702,
     703,   704,   705,   706,   331,   343,   344,   789,   348,   347,
     655,   522,   424,   350,   297,   266,   364,   374,   381,   375,
     376,   409,   377,   265,   415,   169,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   546,   417,   428,
     416,   418,   338,   429,   431,   432,   456,   454,   457,   458,
     559,   466,   563,   564,   565,   473,   266,   495,   338,   496,
     497,  -456,   524,   505,   531,   530,   532,   535,   494,   459,
     494,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     537,   541,   487,   540,   494,   542,   543,   545,   544,   552,
     554,   562,   574,   575,   766,   576,   577,   588,   578,   579,
     580,   615,   581,   614,   771,   494,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   582,   583,   584,   185,   619,   585,   617,   620,   622,
     632,   525,   494,   494,   608,   586,   630,   433,   610,   186,
     633,   187,   641,   451,   658,   645,   444,   188,   662,   819,
     661,   663,   670,   673,   668,   671,   806,   808,   709,   717,
     719,   722,   189,   674,   700,   724,   814,   716,   248,   190,
     249,   250,   731,   664,   665,   666,   727,   666,   732,   752,
     666,   753,   754,   191,   487,   755,   756,   757,   830,   758,
     759,   760,   765,   192,   769,   193,   194,   781,   338,   772,
     783,   778,   779,   784,   785,   791,   794,   799,   802,   839,
     521,   659,   195,   841,   796,   567,   196,   319,   810,   197,
     198,   199,   811,   812,   816,   728,   820,   859,   821,   546,
     818,   200,   825,   828,   832,   833,   836,   487,   358,   837,
     559,   838,   846,   840,   365,   847,   848,   368,   369,   850,
     372,   373,   858,   852,   853,   857,   748,   843,   861,   715,
     325,   449,   616,   499,   826,   383,   387,   813,   827,   388,
     834,   831,   631,   390,   391,   392,   393,   394,   395,   396,
     398,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     408,   609,   520,   714,   740,   412,   534,   536,   805,   851,
     571,   751,   637,   203,   733,   775,   788,   553,   557,   397,
       0,   384,     0,     0,     0,     1,     0,     0,   787,     0,
       2,     0,     0,     3,     0,     0,     0,     0,     4,     0,
       0,   388,     0,     5,     0,     0,     6,   265,     7,     8,
       9,     0,     0,     0,    10,     0,     0,     0,    11,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   265,
     250,    12,     0,     0,     0,     0,     0,     0,     0,     0,
     266,     0,     0,     0,     0,     0,   206,     0,     0,   487,
     265,   487,    13,     0,   518,   519,     0,     0,     0,   728,
       0,    14,   266,    15,     0,   487,    16,     0,    17,     0,
      18,     0,     0,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,   266,   775,    21,   487,     0,    22,     0,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,     0,     0,     0,     0,   511,
     512,     0,     0,   487,   487,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,     0,     0,   319,
       0,   319,     0,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,     0,   133,
     134,   135,   136,   516,   137,   138,   139,     0,     0,     0,
     140,     0,    67,     0,   141,   142,     0,     0,    70,    71,
     143,    73,     0,   144,    74,     0,   388,     0,     0,    75,
       0,     0,     0,   145,     0,     0,     0,     0,     0,     0,
     611,   146,   612,     0,   613,   147,    77,     0,     0,    78,
      79,    80,   148,   149,     0,   150,   151,     0,   152,     0,
       0,   153,     0,     0,    84,    85,    86,    87,     0,    88,
     154,    90,   155,     0,     0,     0,    91,     0,     0,     0,
       0,    92,    93,    94,    95,   156,    97,    98,     0,    99,
     100,     0,   157,   101,   102,   103,     0,   104,     0,     0,
     105,     0,     0,   106,   107,   108,   158,     0,   110,   111,
       0,   112,     0,   113,   114,   115,     0,     0,   116,   117,
     159,     0,   119,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   160,   161,     0,     0,   162,     0,
       0,   163,     0,   388,   164,   245,   134,   135,   136,     0,
     351,   138,     0,     0,     0,     0,   140,     0,    67,     0,
     141,   142,     0,     0,    70,    71,    72,    73,     0,   144,
      74,     0,     0,     0,     0,    75,     0,     0,     0,    76,
       0,     0,     0,     0,     0,     0,     0,   146,     0,     0,
       0,   147,    77,     0,     0,    78,    79,    80,    81,    82,
       0,   150,   151,     0,   152,     0,     0,   153,     0,     0,
      84,    85,    86,    87,     0,    88,   246,    90,   247,     0,
       0,     0,    91,     0,     0,     0,     0,    92,    93,    94,
      95,   156,    97,    98,     0,    99,   100,     0,   157,   101,
     102,   103,     0,   104,     0,     0,   105,     0,     0,   106,
     107,   108,   109,     0,   110,   111,     0,   112,     0,   113,
     114,   115,     0,     0,   116,   117,   159,     0,   119,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,   162,     0,     0,   352,   353,     0,
     164,   245,   134,   135,   136,     0,   351,   138,     0,     0,
       0,     0,   140,     0,    67,     0,   141,   142,     0,     0,
      70,    71,    72,    73,     0,   144,    74,     0,     0,     0,
       0,    75,     0,     0,     0,    76,     0,     0,     0,     0,
       0,     0,     0,   146,     0,     0,     0,   147,    77,     0,
       0,    78,    79,    80,    81,    82,     0,   150,   151,     0,
     152,     0,     0,   153,     0,     0,    84,    85,    86,    87,
       0,    88,   246,    90,   247,     0,     0,     0,    91,     0,
       0,     0,     0,    92,    93,    94,    95,   156,    97,    98,
       0,    99,   100,     0,   157,   101,   102,   103,     0,   104,
       0,     0,   105,     0,     0,   106,   107,   108,   109,     0,
     110,   111,     0,   112,     0,   113,   114,   115,     0,     0,
     116,   117,   159,     0,   119,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,     0,     0,
     162,     0,     0,   163,   370,     0,   164,   245,   134,   135,
     136,     0,   137,   138,     0,     0,     0,     0,   140,     0,
      67,     0,   141,   142,     0,     0,    70,    71,   143,    73,
       0,   144,    74,     0,     0,     0,     0,    75,     0,     0,
       0,   145,     0,     0,     0,     0,     0,     0,     0,   146,
       0,     0,     0,   147,    77,     0,     0,    78,    79,    80,
     148,   149,     0,   150,   151,     0,   152,     0,     0,   153,
       0,     0,    84,    85,    86,    87,     0,    88,   154,    90,
     155,     0,     0,     0,    91,     0,     0,     0,     0,    92,
      93,    94,    95,   156,    97,    98,     0,    99,   100,     0,
     157,   101,   102,   103,     0,   104,     0,     0,   105,     0,
       0,   106,   107,   108,   158,     0,   110,   111,     0,   112,
       0,   113,   114,   115,     0,     0,   116,   117,   159,     0,
     119,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   160,   161,     0,     0,   162,     0,     0,   163,
       0,     0,   164,   133,   134,   135,   136,     0,   137,   138,
       0,     0,     0,     0,   140,     0,    67,     0,   141,   142,
       0,     0,    70,    71,   143,    73,     0,   144,    74,     0,
       0,     0,     0,    75,     0,     0,     0,   145,     0,     0,
       0,     0,     0,     0,     0,   146,     0,     0,     0,   147,
      77,     0,     0,    78,    79,    80,   148,   149,     0,   150,
     151,     0,   152,     0,     0,   153,     0,     0,    84,    85,
      86,    87,     0,    88,   154,    90,   155,     0,     0,     0,
      91,     0,     0,     0,     0,    92,    93,    94,    95,   156,
      97,    98,     0,    99,   100,     0,   157,   101,   102,   103,
       0,   104,     0,     0,   105,     0,     0,   106,   107,   108,
     158,     0,   110,   111,     0,   112,     0,   113,   114,   115,
       0,     0,   116,   117,   159,     0,   119,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   160,   161,
       0,     0,   162,     0,     0,   163,     0,     0,   164,   245,
     134,   135,   136,     0,   137,   138,     0,     0,     0,     0,
     140,     0,    67,     0,   141,   142,     0,     0,    70,    71,
      72,    73,     0,   144,    74,     0,     0,     0,     0,    75,
       0,     0,     0,    76,     0,     0,     0,     0,     0,     0,
       0,   146,     0,     0,     0,   147,    77,     0,     0,    78,
      79,    80,    81,    82,     0,   150,   151,     0,   152,     0,
       0,   153,     0,     0,    84,    85,    86,    87,     0,    88,
     246,    90,   247,     0,     0,     0,    91,     0,     0,     0,
       0,    92,    93,    94,    95,   156,    97,    98,     0,    99,
     100,     0,   157,   101,   102,   103,   498,   104,     0,     0,
     105,     0,     0,   106,   107,   108,   109,     0,   110,   111,
       0,   112,     0,   113,   114,   115,     0,     0,   116,   117,
     159,     0,   119,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   160,     0,     0,     0,   162,     0,
       0,   163,     0,     0,   164,   245,   134,   135,   136,     0,
     351,   138,     0,     0,     0,     0,   140,     0,    67,     0,
     141,   142,     0,     0,    70,    71,    72,    73,     0,   144,
      74,     0,     0,     0,     0,    75,     0,     0,     0,    76,
       0,     0,     0,     0,     0,     0,     0,   146,     0,     0,
       0,   147,    77,     0,     0,    78,    79,    80,    81,    82,
       0,   150,   151,     0,   152,     0,     0,   153,     0,     0,
      84,    85,    86,    87,     0,    88,   246,    90,   247,     0,
       0,     0,    91,     0,     0,     0,     0,    92,    93,    94,
      95,   156,    97,    98,     0,    99,   100,     0,   157,   101,
     102,   103,     0,   104,     0,     0,   105,     0,     0,   106,
     107,   108,   109,     0,   110,   111,     0,   112,     0,   113,
     114,   115,     0,     0,   116,   117,   159,     0,   119,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,   162,     0,     0,   163,   353,     0,
     164,   245,   134,   135,   136,     0,   137,   138,     0,     0,
       0,     0,   140,     0,    67,     0,   141,   142,     0,     0,
      70,    71,    72,    73,     0,   144,    74,     0,     0,     0,
       0,    75,     0,     0,     0,    76,     0,     0,     0,     0,
       0,     0,     0,   146,     0,     0,     0,   147,    77,     0,
       0,    78,    79,    80,    81,    82,     0,   150,   151,     0,
     152,     0,     0,   153,     0,     0,    84,    85,    86,    87,
       0,    88,   246,    90,   247,     0,     0,     0,    91,     0,
       0,     0,     0,    92,    93,    94,    95,   156,    97,    98,
       0,    99,   100,     0,   157,   101,   102,   103,     0,   104,
       0,     0,   105,     0,     0,   106,   107,   108,   109,     0,
     110,   111,     0,   112,     0,   113,   114,   115,     0,     0,
     116,   117,   159,     0,   119,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,     0,     0,
     162,     0,     0,   163,     0,     0,   164,   245,   134,   135,
     136,     0,   351,   138,     0,     0,     0,     0,   140,     0,
      67,     0,   141,   142,     0,     0,    70,    71,    72,    73,
       0,   144,    74,     0,     0,     0,     0,    75,     0,     0,
       0,    76,     0,     0,     0,     0,     0,     0,     0,   146,
       0,     0,     0,   147,    77,     0,     0,    78,    79,    80,
      81,    82,     0,   150,   151,     0,   152,     0,     0,   153,
       0,     0,    84,    85,    86,    87,     0,    88,   246,    90,
     247,     0,     0,     0,    91,     0,     0,     0,     0,    92,
      93,    94,    95,   156,    97,    98,     0,    99,   100,     0,
     157,   101,   102,   103,     0,   104,     0,     0,   105,     0,
       0,   106,   107,   108,   109,     0,   110,   111,     0,   112,
       0,   113,   114,   115,     0,     0,   116,   117,   159,     0,
     119,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   160,     0,     0,     0,   162,     0,     0,   163,
       0,     0,   164,   245,   134,   135,   136,     0,   382,   138,
       0,     0,     0,     0,   140,     0,    67,     0,   141,   142,
       0,     0,    70,    71,    72,    73,     0,   144,    74,     0,
       0,     0,     0,    75,     0,     0,     0,    76,     0,     0,
       0,     0,     0,     0,     0,   146,     0,     0,     0,   147,
      77,     0,     0,    78,    79,    80,    81,    82,     0,   150,
     151,     0,   152,     0,     0,   153,     0,     0,    84,    85,
      86,    87,     0,    88,   246,    90,   247,     0,     0,     0,
      91,     0,     0,     0,     0,    92,    93,    94,    95,   156,
      97,    98,     0,    99,   100,     0,   157,   101,   102,   103,
       0,   104,     0,     0,   105,     0,     0,   106,   107,   108,
     109,     0,   110,   111,     0,   112,     0,   113,   114,   115,
       0,     0,   116,   117,   159,     0,   119,     0,     0,     0,
       0,    66,   474,     0,     0,     0,     0,     0,   160,     0,
       0,     0,   162,     0,    67,   163,   475,   476,   164,     0,
      70,    71,    72,    73,     0,   477,    74,     0,     0,     0,
       0,    75,     0,     0,     0,   478,     0,     0,     0,     0,
       0,     0,     0,   479,     0,   480,     0,     0,    77,     0,
       0,    78,    79,    80,   481,    82,     0,   482,     0,     0,
      83,     0,     0,   483,     0,     0,    84,    85,    86,    87,
       0,   484,    89,    90,     0,     0,     0,     0,    91,     0,
       0,     0,     0,    92,    93,    94,    95,    96,    97,    98,
       0,    99,   100,     0,     0,   101,   102,   103,     0,   104,
       0,     0,   105,     0,     0,   106,   107,   108,   109,     0,
     110,   111,     0,   112,     0,   113,   114,   115,     0,     0,
     116,   117,   485,     0,   119,    66,     0,   627,     0,     0,
     628,     0,     0,     0,     0,     0,     0,     0,    67,     0,
      68,    69,     0,   486,    70,    71,    72,    73,     0,     0,
      74,     0,     0,     0,     0,    75,     0,     0,     0,    76,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,     0,    78,    79,    80,    81,    82,
       0,     0,     0,     0,    83,     0,     0,     0,     0,     0,
      84,    85,    86,    87,     0,    88,    89,    90,     0,     0,
       0,     0,    91,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,    98,     0,    99,   100,     0,     0,   101,
     102,   103,     0,   104,     0,     0,   105,     0,     0,   106,
     107,   108,   109,     0,   110,   111,    66,   112,     0,   113,
     114,   115,     0,     0,   116,   117,   118,     0,   119,    67,
       0,    68,    69,     0,     0,    70,    71,    72,    73,     0,
       0,    74,     0,     0,     0,     0,    75,   629,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    77,     0,     0,    78,    79,    80,    81,
      82,     0,     0,     0,     0,    83,     0,     0,     0,     0,
       0,    84,    85,    86,    87,     0,    88,    89,    90,     0,
       0,     0,     0,    91,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,    98,     0,    99,   100,     0,     0,
     101,   102,   103,     0,   104,     0,     0,   105,     0,     0,
     106,   107,   108,   109,     0,   110,   111,     0,   112,     0,
     113,   114,   115,     0,     0,   116,   117,   118,     0,   119,
      66,     0,   332,     0,     0,   419,     0,     0,     0,     0,
       0,     0,     0,    67,     0,    68,    69,     0,   385,    70,
      71,    72,    73,     0,     0,    74,     0,     0,     0,     0,
      75,     0,     0,     0,    76,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   420,     0,    77,     0,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,    84,    85,    86,    87,     0,
      88,    89,    90,     0,     0,     0,     0,    91,   421,     0,
       0,     0,    92,    93,    94,    95,    96,    97,    98,     0,
      99,   100,     0,     0,   101,   102,   103,     0,   104,     0,
       0,   105,     0,     0,   106,   107,   108,   109,     0,   110,
     111,     0,   112,   422,   113,   114,   115,     0,     0,   116,
     117,   118,     0,   119,    66,     0,   332,     0,     0,     0,
       0,     0,     0,     0,     0,   423,     0,    67,     0,    68,
      69,     0,     0,    70,    71,    72,    73,     0,     0,    74,
       0,     0,     0,     0,    75,     0,     0,     0,    76,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    77,     0,     0,    78,    79,    80,    81,    82,     0,
       0,     0,     0,    83,     0,     0,     0,     0,     0,    84,
      85,    86,    87,     0,    88,    89,    90,     0,     0,     0,
       0,    91,     0,     0,     0,     0,    92,    93,    94,    95,
      96,    97,    98,     0,    99,   100,     0,     0,   101,   102,
     103,     0,   104,     0,     0,   105,     0,     0,   106,   107,
     108,   109,     0,   110,   111,     0,   112,     0,   113,   114,
     115,    66,   474,   116,   117,   118,     0,   119,     0,     0,
       0,     0,     0,     0,    67,     0,    68,    69,     0,   423,
      70,    71,    72,    73,     0,   477,    74,     0,     0,     0,
       0,    75,     0,     0,     0,   478,     0,     0,     0,     0,
       0,     0,     0,   479,     0,   480,     0,     0,    77,     0,
       0,    78,    79,    80,   481,    82,     0,   482,     0,     0,
      83,     0,     0,   483,     0,     0,    84,    85,    86,    87,
       0,    88,    89,    90,     0,     0,     0,     0,    91,     0,
       0,     0,     0,    92,    93,    94,    95,   736,    97,    98,
       0,    99,   100,     0,     0,   101,   102,   103,     0,   104,
       0,     0,   105,     0,     0,   106,   107,   108,   109,     0,
     110,   111,     0,   112,     0,   113,   114,   115,    66,   474,
     116,   117,   485,     0,   119,     0,     0,     0,     0,     0,
       0,    67,     0,    68,    69,     0,     0,    70,    71,    72,
      73,     0,   477,    74,     0,     0,     0,     0,    75,     0,
       0,     0,   478,     0,     0,     0,     0,     0,     0,     0,
     479,     0,   480,     0,     0,    77,     0,     0,    78,    79,
      80,   481,    82,     0,   482,     0,     0,    83,     0,     0,
     483,     0,     0,    84,    85,    86,    87,     0,    88,    89,
      90,     0,     0,     0,     0,    91,     0,     0,     0,     0,
      92,    93,    94,    95,    96,    97,    98,     0,    99,   100,
       0,     0,   101,   102,   103,     0,   104,     0,     0,   105,
       0,     0,   106,   107,   108,   109,     0,   110,   111,     0,
     112,     0,   113,   114,   115,    66,   474,   116,   117,   485,
       0,   119,     0,     0,     0,     0,     0,     0,    67,     0,
     475,   476,     0,     0,    70,    71,    72,    73,     0,   477,
      74,     0,     0,     0,     0,    75,     0,     0,     0,   478,
       0,     0,     0,     0,     0,     0,     0,   479,     0,   480,
       0,     0,    77,     0,     0,    78,    79,    80,   481,    82,
       0,   482,     0,     0,    83,     0,     0,   483,     0,     0,
      84,    85,    86,    87,     0,    88,    89,    90,     0,     0,
       0,     0,    91,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,    98,     0,    99,   100,     0,     0,   101,
     102,   103,     0,   104,     0,     0,   105,     0,     0,   106,
     107,   108,   109,     0,   110,   111,     0,   112,     0,   113,
     114,   115,     0,     0,   116,   117,   485,    66,   119,   293,
     294,     0,   295,     0,     0,     0,     0,     0,     0,     0,
      67,     0,    68,    69,     0,     0,    70,    71,    72,    73,
       0,     0,    74,     0,     0,     0,     0,    75,     0,     0,
       0,    76,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,    78,    79,    80,
      81,    82,     0,     0,     0,     0,    83,     0,     0,     0,
       0,     0,    84,    85,    86,    87,     0,    88,    89,    90,
       0,     0,     0,     0,    91,   296,     0,     0,     0,    92,
      93,    94,    95,    96,    97,    98,     0,    99,   100,     0,
       0,   101,   102,   103,     0,   104,     0,     0,   105,     0,
       0,   106,   107,   108,   109,     0,   110,   111,    66,   112,
       0,   113,   114,   115,     0,   177,   116,   117,   118,     0,
     119,    67,     0,    68,    69,     0,     0,    70,    71,    72,
      73,     0,     0,    74,     0,     0,   178,     0,    75,     0,
       0,     0,    76,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    77,     0,     0,    78,   179,
      80,    81,    82,     0,     0,     0,   180,    83,     0,     0,
       0,     0,     0,    84,    85,    86,    87,     0,    88,    89,
      90,     0,     0,     0,     0,   181,     0,     0,     0,     0,
      92,    93,    94,    95,    96,    97,    98,     0,    99,   100,
       0,     0,   101,   102,   103,     0,   104,     0,     0,   182,
       0,     0,   106,   107,   108,   109,     0,   110,   111,     0,
       0,     0,   113,   114,   115,     0,     0,   116,   117,   118,
      66,   119,   533,     0,     0,   419,     0,     0,     0,     0,
       0,     0,     0,    67,     0,    68,    69,     0,     0,    70,
      71,    72,    73,     0,     0,    74,     0,     0,     0,     0,
      75,     0,     0,     0,    76,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    77,     0,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,    84,    85,    86,    87,     0,
      88,    89,    90,     0,     0,     0,     0,    91,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,    98,     0,
      99,   100,     0,     0,   101,   102,   103,     0,   104,     0,
       0,   105,     0,     0,   106,   107,   108,   109,     0,   110,
     111,    66,   112,   773,   113,   114,   115,     0,     0,   116,
     117,   118,     0,   119,    67,     0,    68,    69,     0,     0,
      70,    71,    72,    73,     0,     0,    74,     0,     0,     0,
       0,    75,     0,     0,     0,    76,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    77,     0,
       0,    78,    79,    80,    81,    82,     0,   774,     0,     0,
      83,     0,     0,     0,     0,     0,    84,    85,    86,    87,
       0,    88,    89,    90,     0,     0,     0,     0,    91,     0,
       0,     0,     0,    92,    93,    94,    95,    96,    97,    98,
       0,    99,   100,     0,     0,   101,   102,   103,     0,   104,
       0,     0,   105,     0,     0,   106,   107,   108,   109,     0,
     110,   111,    66,   112,     0,   113,   114,   115,     0,     0,
     116,   117,   118,     0,   119,    67,     0,    68,    69,     0,
       0,    70,    71,    72,    73,     0,     0,    74,     0,     0,
       0,     0,    75,     0,     0,     0,    76,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    77,
       0,     0,    78,    79,    80,    81,    82,     0,     0,   251,
       0,    83,     0,     0,     0,     0,     0,    84,    85,    86,
      87,     0,    88,    89,    90,     0,     0,     0,     0,    91,
       0,     0,     0,     0,    92,    93,    94,    95,    96,    97,
      98,     0,    99,   100,     0,     0,   101,   102,   103,     0,
     104,     0,     0,   105,     0,     0,   106,   107,   108,   109,
       0,   110,   111,    66,   112,     0,   113,   114,   115,     0,
       0,   116,   117,   118,     0,   119,    67,     0,    68,    69,
     262,     0,    70,    71,    72,    73,     0,     0,    74,     0,
       0,     0,     0,    75,     0,     0,     0,    76,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,     0,    78,    79,    80,    81,    82,     0,     0,
       0,     0,    83,     0,     0,     0,     0,     0,    84,    85,
      86,    87,     0,    88,    89,    90,     0,     0,     0,     0,
      91,     0,     0,     0,     0,    92,    93,    94,    95,    96,
      97,    98,     0,    99,   100,     0,     0,   101,   102,   103,
       0,   104,     0,     0,   105,     0,     0,   106,   107,   108,
     109,     0,   110,   111,     0,   112,    66,   113,   114,   115,
       0,   302,   116,   117,   118,     0,   119,     0,     0,    67,
       0,    68,    69,     0,     0,    70,    71,    72,    73,     0,
       0,    74,     0,     0,     0,     0,    75,     0,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    77,     0,     0,    78,    79,    80,    81,
      82,     0,     0,     0,     0,    83,     0,     0,     0,     0,
       0,    84,    85,    86,    87,     0,    88,    89,    90,     0,
       0,     0,     0,    91,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,    98,     0,    99,   100,     0,     0,
     101,   102,   103,     0,   104,     0,     0,   105,     0,     0,
     106,   303,   108,   109,     0,   110,   111,    66,   112,     0,
     113,   114,   115,     0,     0,   116,   117,   118,     0,   119,
      67,     0,    68,    69,     0,     0,    70,    71,    72,    73,
       0,     0,    74,     0,     0,     0,     0,    75,     0,     0,
       0,    76,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,    78,    79,    80,
      81,    82,     0,   558,     0,     0,    83,     0,     0,     0,
       0,     0,    84,    85,    86,    87,     0,    88,    89,    90,
       0,     0,     0,     0,    91,     0,     0,     0,     0,    92,
      93,    94,    95,    96,    97,    98,     0,    99,   100,     0,
       0,   101,   102,   103,     0,   104,     0,     0,   105,     0,
       0,   106,   107,   108,   109,     0,   110,   111,    66,   112,
       0,   113,   114,   115,     0,     0,   116,   117,   118,     0,
     119,    67,     0,    68,    69,     0,     0,    70,    71,    72,
      73,     0,     0,    74,     0,     0,     0,     0,    75,     0,
       0,     0,    76,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    77,     0,     0,    78,    79,
      80,    81,    82,     0,     0,     0,     0,    83,     0,     0,
       0,     0,     0,    84,    85,    86,    87,   747,    88,    89,
      90,     0,     0,     0,     0,    91,     0,     0,     0,     0,
      92,    93,    94,    95,    96,    97,    98,     0,    99,   100,
       0,     0,   101,   102,   103,     0,   104,     0,     0,   105,
       0,     0,   106,   107,   108,   109,     0,   110,   111,    66,
     112,     0,   113,   114,   115,     0,     0,   116,   117,   118,
       0,   119,    67,     0,    68,    69,     0,     0,    70,    71,
      72,    73,     0,     0,    74,     0,     0,     0,     0,    75,
       0,     0,     0,    76,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,     0,    78,
      79,    80,    81,    82,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,    84,    85,    86,    87,     0,    88,
      89,    90,     0,     0,     0,     0,    91,     0,     0,     0,
       0,    92,    93,    94,    95,    96,    97,    98,     0,    99,
     100,     0,     0,   101,   102,   103,     0,   104,     0,     0,
     105,     0,     0,   106,   107,   108,   109,     0,   110,   111,
      66,   112,     0,   113,   114,   115,   290,     0,   116,   117,
     118,     0,   119,    67,     0,    68,    69,     0,     0,    70,
      71,    72,    73,     0,     0,    74,     0,     0,     0,     0,
      75,     0,     0,     0,    76,     0,     0,     0,     0,     0,
       0,     0,   265,     0,     0,     0,     0,    77,     0,     0,
      78,    79,    80,    81,    82,     0,     0,     0,     0,    83,
       0,     0,     0,     0,   265,    84,    85,    86,    87,     0,
      88,    89,    90,     0,     0,   266,     0,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,    98,   265,
      99,   100,     0,     0,   101,   102,   103,   266,   104,     0,
       0,   105,     0,     0,   106,   107,   108,   109,     0,   110,
     111,     0,  -420,   265,   113,   114,   115,     0,     0,   116,
     117,   118,   266,   119,     0,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
       0,     0,     0,     0,   513,   514,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   265,     0,     0,     0,     0,   512,     0,     0,
       0,     0,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   265,     0,     0,
       0,     0,   514,     0,     0,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   265,     0,     0,     0,     0,   713,     0,     0,     0,
     266,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   265,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     265,     0,     0,     0,   378,   266,     0,     0,     0,     0,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,     0,     0,     0,     0,   502,
       0,     0,     0,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   265,
       0,     0,     0,   508,     0,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     265,     0,     0,     0,   517,     0,     0,   529,     0,     0,
       0,     0,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   265,     0,
       0,     0,   711,   266,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   265,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   266,     0,     0,
       0,   712,     0,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   265,
       0,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,     0,     0,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281
};

static const short yycheck[] =
{
       4,   231,   317,   233,   234,   466,   551,   237,   238,   555,
     549,   241,   641,   552,   217,   532,     5,     5,    22,   191,
       8,    18,     5,     6,   196,     8,   198,    29,     5,     9,
     345,    14,    22,     5,     5,     6,    40,     8,   486,     5,
       6,   213,     5,    14,     9,     8,    47,    52,    14,    52,
     589,    13,    14,   592,   593,   594,   595,   596,   597,    63,
      64,    65,    23,    36,    13,    14,    20,    13,    14,    81,
       5,     6,    82,     8,    27,    81,    71,     5,     6,    14,
       8,     5,     6,   102,   125,    71,    14,    61,   386,    26,
      14,    43,   125,   147,    14,   149,    49,   300,   128,    44,
      91,    52,    15,    94,    61,    67,   147,    26,   115,     0,
     101,    64,    32,   108,   147,    53,   146,   634,   122,   123,
     124,   125,   126,    60,   128,   129,   130,   131,   147,    90,
     142,   303,    77,    46,   124,    44,   142,   147,    99,   125,
     588,    60,   147,   144,   147,   147,   119,   445,    86,   138,
     138,     5,     6,    44,   783,   138,   109,   146,   146,    99,
     164,   138,   145,   680,   168,   145,   138,   138,    77,   146,
     799,   486,   138,    53,   145,   138,   180,   181,   381,   145,
     145,   185,   179,   700,   645,   131,    77,   139,   108,   151,
      61,   820,   146,   147,   139,   140,   141,    21,   202,   820,
     661,   205,   151,   138,   100,   151,    86,   724,   212,   105,
     138,     0,    99,    66,   138,   145,    87,    41,   847,   848,
     759,   760,    52,    16,   111,   229,    26,   848,   137,   138,
     139,   140,   141,   288,   289,   550,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   768,    26,   146,   147,   772,   260,   429,   262,   431,
      60,   146,   147,   466,   579,    72,   581,    67,    61,   584,
     146,   147,   145,   588,   146,   147,   146,   147,   146,   147,
     145,   827,    75,   150,   288,   289,    60,   146,   147,   146,
     147,   131,    85,   145,    87,    88,   300,   146,   147,   146,
     147,   146,   147,   145,   849,   146,   147,   311,   146,   147,
     146,   147,   145,   317,   318,   108,   145,   321,   111,   145,
     113,   146,   147,   146,   147,   145,   641,   145,    27,    28,
     123,   131,   132,   133,   134,   135,   136,   146,   147,    44,
     801,   345,   142,   146,   147,    67,   549,   146,   147,   552,
      49,   145,    67,   145,   145,   145,   360,   131,   132,   133,
     134,   135,   136,   145,   567,   145,    65,   145,   142,   145,
      69,   131,    77,   145,   145,   379,   145,   381,   102,    78,
      79,   115,    44,   131,   115,   102,   589,    71,    82,   592,
     593,   594,   595,   596,   597,   122,   599,    92,   601,   602,
     603,   604,   605,   606,   114,    97,   118,   722,    52,   118,
     109,   614,   416,   106,   418,    77,   146,   146,   131,   145,
     145,     5,   145,    44,   131,    17,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   441,    54,    68,
     131,   138,   645,   108,   108,     8,    34,   145,    34,    99,
     454,   147,   456,   457,   458,     8,    77,     8,   661,     8,
       8,   131,   152,   146,    81,   147,   145,     9,   783,     5,
     785,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      70,   147,   486,   131,   799,   103,     5,     8,   103,   131,
      83,   121,   146,   106,   697,   106,   145,   128,   145,   145,
     145,   100,   145,   131,   707,   820,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   145,   145,   145,    16,   529,   145,    55,     5,   131,
     131,   152,   847,   848,   146,   145,   540,   541,   146,    31,
     127,    33,    29,   147,     8,   145,   550,    39,     8,   779,
     147,     8,   146,     8,   139,   146,   759,   760,   147,     5,
     126,    60,    54,   146,   145,   145,   769,   146,   160,    61,
     162,   163,   145,   577,   578,   579,    91,   581,     8,   146,
     584,   146,   146,    75,   588,   146,   146,   146,   791,   146,
     128,   128,    81,    85,   128,    87,    88,    83,   801,   145,
      29,   146,   145,    55,    58,   131,     8,   147,     3,   812,
     614,    20,   104,   816,   145,   147,   108,   209,   147,   111,
     112,   113,   146,   128,   128,   629,    29,   857,    72,   633,
     147,   123,    83,   146,   146,   146,   131,   641,   230,   147,
     644,     8,     5,   146,   236,   147,    29,   239,   240,     8,
     242,   243,     5,   147,   147,   145,   660,   818,   146,   615,
     210,   321,   526,   352,   785,   257,   261,   768,   786,   261,
     799,   793,   541,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   501,   379,   614,   644,   287,   416,   418,   745,   837,
     466,   661,   550,    19,   633,   709,   722,   451,   453,   271,
      -1,   258,    -1,    -1,    -1,    17,    -1,    -1,   722,    -1,
      22,    -1,    -1,    25,    -1,    -1,    -1,    -1,    30,    -1,
      -1,   323,    -1,    35,    -1,    -1,    38,    44,    40,    41,
      42,    -1,    -1,    -1,    46,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
     352,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,   780,    -1,    -1,   783,
      44,   785,    84,    -1,   376,   377,    -1,    -1,    -1,   793,
      -1,    93,    77,    95,    -1,   799,    98,    -1,   100,    -1,
     102,    -1,    -1,   105,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,   818,   117,   820,    -1,   120,    -1,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,   146,
     147,    -1,    -1,   847,   848,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,    -1,    -1,   451,
      -1,   453,    -1,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,    -1,     3,
       4,     5,     6,   147,     8,     9,    10,    -1,    -1,    -1,
      14,    -1,    16,    -1,    18,    19,    -1,    -1,    22,    23,
      24,    25,    -1,    27,    28,    -1,   498,    -1,    -1,    33,
      -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
     512,    45,   514,    -1,   516,    49,    50,    -1,    -1,    53,
      54,    55,    56,    57,    -1,    59,    60,    -1,    62,    -1,
      -1,    65,    -1,    -1,    68,    69,    70,    71,    -1,    73,
      74,    75,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    85,    86,    87,    88,    89,    90,    91,    -1,    93,
      94,    -1,    96,    97,    98,    99,    -1,   101,    -1,    -1,
     104,    -1,    -1,   107,   108,   109,   110,    -1,   112,   113,
      -1,   115,    -1,   117,   118,   119,    -1,    -1,   122,   123,
     124,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,    -1,    -1,   142,    -1,
      -1,   145,    -1,   615,   148,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      18,    19,    -1,    -1,    22,    23,    24,    25,    -1,    27,
      28,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    49,    50,    -1,    -1,    53,    54,    55,    56,    57,
      -1,    59,    60,    -1,    62,    -1,    -1,    65,    -1,    -1,
      68,    69,    70,    71,    -1,    73,    74,    75,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    85,    86,    87,
      88,    89,    90,    91,    -1,    93,    94,    -1,    96,    97,
      98,    99,    -1,   101,    -1,    -1,   104,    -1,    -1,   107,
     108,   109,   110,    -1,   112,   113,    -1,   115,    -1,   117,
     118,   119,    -1,    -1,   122,   123,   124,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,   142,    -1,    -1,   145,   146,    -1,
     148,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    18,    19,    -1,    -1,
      22,    23,    24,    25,    -1,    27,    28,    -1,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    49,    50,    -1,
      -1,    53,    54,    55,    56,    57,    -1,    59,    60,    -1,
      62,    -1,    -1,    65,    -1,    -1,    68,    69,    70,    71,
      -1,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      -1,    93,    94,    -1,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,
     112,   113,    -1,   115,    -1,   117,   118,   119,    -1,    -1,
     122,   123,   124,    -1,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
     142,    -1,    -1,   145,   146,    -1,   148,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,
      16,    -1,    18,    19,    -1,    -1,    22,    23,    24,    25,
      -1,    27,    28,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    49,    50,    -1,    -1,    53,    54,    55,
      56,    57,    -1,    59,    60,    -1,    62,    -1,    -1,    65,
      -1,    -1,    68,    69,    70,    71,    -1,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    85,
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
      96,    97,    98,    99,    -1,   101,    -1,    -1,   104,    -1,
      -1,   107,   108,   109,   110,    -1,   112,   113,    -1,   115,
      -1,   117,   118,   119,    -1,    -1,   122,   123,   124,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,    -1,    -1,   142,    -1,    -1,   145,
      -1,    -1,   148,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    18,    19,
      -1,    -1,    22,    23,    24,    25,    -1,    27,    28,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    49,
      50,    -1,    -1,    53,    54,    55,    56,    57,    -1,    59,
      60,    -1,    62,    -1,    -1,    65,    -1,    -1,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    85,    86,    87,    88,    89,
      90,    91,    -1,    93,    94,    -1,    96,    97,    98,    99,
      -1,   101,    -1,    -1,   104,    -1,    -1,   107,   108,   109,
     110,    -1,   112,   113,    -1,   115,    -1,   117,   118,   119,
      -1,    -1,   122,   123,   124,    -1,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
      -1,    -1,   142,    -1,    -1,   145,    -1,    -1,   148,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    18,    19,    -1,    -1,    22,    23,
      24,    25,    -1,    27,    28,    -1,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    49,    50,    -1,    -1,    53,
      54,    55,    56,    57,    -1,    59,    60,    -1,    62,    -1,
      -1,    65,    -1,    -1,    68,    69,    70,    71,    -1,    73,
      74,    75,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    85,    86,    87,    88,    89,    90,    91,    -1,    93,
      94,    -1,    96,    97,    98,    99,   100,   101,    -1,    -1,
     104,    -1,    -1,   107,   108,   109,   110,    -1,   112,   113,
      -1,   115,    -1,   117,   118,   119,    -1,    -1,   122,   123,
     124,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,   142,    -1,
      -1,   145,    -1,    -1,   148,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      18,    19,    -1,    -1,    22,    23,    24,    25,    -1,    27,
      28,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    49,    50,    -1,    -1,    53,    54,    55,    56,    57,
      -1,    59,    60,    -1,    62,    -1,    -1,    65,    -1,    -1,
      68,    69,    70,    71,    -1,    73,    74,    75,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    85,    86,    87,
      88,    89,    90,    91,    -1,    93,    94,    -1,    96,    97,
      98,    99,    -1,   101,    -1,    -1,   104,    -1,    -1,   107,
     108,   109,   110,    -1,   112,   113,    -1,   115,    -1,   117,
     118,   119,    -1,    -1,   122,   123,   124,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,   142,    -1,    -1,   145,   146,    -1,
     148,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    18,    19,    -1,    -1,
      22,    23,    24,    25,    -1,    27,    28,    -1,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    49,    50,    -1,
      -1,    53,    54,    55,    56,    57,    -1,    59,    60,    -1,
      62,    -1,    -1,    65,    -1,    -1,    68,    69,    70,    71,
      -1,    73,    74,    75,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      -1,    93,    94,    -1,    96,    97,    98,    99,    -1,   101,
      -1,    -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,
     112,   113,    -1,   115,    -1,   117,   118,   119,    -1,    -1,
     122,   123,   124,    -1,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
     142,    -1,    -1,   145,    -1,    -1,   148,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,
      16,    -1,    18,    19,    -1,    -1,    22,    23,    24,    25,
      -1,    27,    28,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    49,    50,    -1,    -1,    53,    54,    55,
      56,    57,    -1,    59,    60,    -1,    62,    -1,    -1,    65,
      -1,    -1,    68,    69,    70,    71,    -1,    73,    74,    75,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    85,
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
      96,    97,    98,    99,    -1,   101,    -1,    -1,   104,    -1,
      -1,   107,   108,   109,   110,    -1,   112,   113,    -1,   115,
      -1,   117,   118,   119,    -1,    -1,   122,   123,   124,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    -1,    -1,    -1,   142,    -1,    -1,   145,
      -1,    -1,   148,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    18,    19,
      -1,    -1,    22,    23,    24,    25,    -1,    27,    28,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    49,
      50,    -1,    -1,    53,    54,    55,    56,    57,    -1,    59,
      60,    -1,    62,    -1,    -1,    65,    -1,    -1,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    85,    86,    87,    88,    89,
      90,    91,    -1,    93,    94,    -1,    96,    97,    98,    99,
      -1,   101,    -1,    -1,   104,    -1,    -1,   107,   108,   109,
     110,    -1,   112,   113,    -1,   115,    -1,   117,   118,   119,
      -1,    -1,   122,   123,   124,    -1,   126,    -1,    -1,    -1,
      -1,     3,     4,    -1,    -1,    -1,    -1,    -1,   138,    -1,
      -1,    -1,   142,    -1,    16,   145,    18,    19,   148,    -1,
      22,    23,    24,    25,    -1,    27,    28,    -1,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    47,    -1,    -1,    50,    -1,
      -1,    53,    54,    55,    56,    57,    -1,    59,    -1,    -1,
      62,    -1,    -1,    65,    -1,    -1,    68,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      -1,    93,    94,    -1,    -1,    97,    98,    99,    -1,   101,
      -1,    -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,
     112,   113,    -1,   115,    -1,   117,   118,   119,    -1,    -1,
     122,   123,   124,    -1,   126,     3,    -1,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      18,    19,    -1,   145,    22,    23,    24,    25,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    53,    54,    55,    56,    57,
      -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    85,    86,    87,
      88,    89,    90,    91,    -1,    93,    94,    -1,    -1,    97,
      98,    99,    -1,   101,    -1,    -1,   104,    -1,    -1,   107,
     108,   109,   110,    -1,   112,   113,     3,   115,    -1,   117,
     118,   119,    -1,    -1,   122,   123,   124,    -1,   126,    16,
      -1,    18,    19,    -1,    -1,    22,    23,    24,    25,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    33,   145,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    85,    86,
      87,    88,    89,    90,    91,    -1,    93,    94,    -1,    -1,
      97,    98,    99,    -1,   101,    -1,    -1,   104,    -1,    -1,
     107,   108,   109,   110,    -1,   112,   113,    -1,   115,    -1,
     117,   118,   119,    -1,    -1,   122,   123,   124,    -1,   126,
       3,    -1,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    18,    19,    -1,   145,    22,
      23,    24,    25,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    -1,    -1,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    -1,    -1,    80,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    90,    91,    -1,
      93,    94,    -1,    -1,    97,    98,    99,    -1,   101,    -1,
      -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,   112,
     113,    -1,   115,   116,   117,   118,   119,    -1,    -1,   122,
     123,   124,    -1,   126,     3,    -1,     5,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    16,    -1,    18,
      19,    -1,    -1,    22,    23,    24,    25,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    53,    54,    55,    56,    57,    -1,
      -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    -1,    85,    86,    87,    88,
      89,    90,    91,    -1,    93,    94,    -1,    -1,    97,    98,
      99,    -1,   101,    -1,    -1,   104,    -1,    -1,   107,   108,
     109,   110,    -1,   112,   113,    -1,   115,    -1,   117,   118,
     119,     3,     4,   122,   123,   124,    -1,   126,    -1,    -1,
      -1,    -1,    -1,    -1,    16,    -1,    18,    19,    -1,   138,
      22,    23,    24,    25,    -1,    27,    28,    -1,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    47,    -1,    -1,    50,    -1,
      -1,    53,    54,    55,    56,    57,    -1,    59,    -1,    -1,
      62,    -1,    -1,    65,    -1,    -1,    68,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      -1,    93,    94,    -1,    -1,    97,    98,    99,    -1,   101,
      -1,    -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,
     112,   113,    -1,   115,    -1,   117,   118,   119,     3,     4,
     122,   123,   124,    -1,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    18,    19,    -1,    -1,    22,    23,    24,
      25,    -1,    27,    28,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    47,    -1,    -1,    50,    -1,    -1,    53,    54,
      55,    56,    57,    -1,    59,    -1,    -1,    62,    -1,    -1,
      65,    -1,    -1,    68,    69,    70,    71,    -1,    73,    74,
      75,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      85,    86,    87,    88,    89,    90,    91,    -1,    93,    94,
      -1,    -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,
      -1,    -1,   107,   108,   109,   110,    -1,   112,   113,    -1,
     115,    -1,   117,   118,   119,     3,     4,   122,   123,   124,
      -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      18,    19,    -1,    -1,    22,    23,    24,    25,    -1,    27,
      28,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    47,
      -1,    -1,    50,    -1,    -1,    53,    54,    55,    56,    57,
      -1,    59,    -1,    -1,    62,    -1,    -1,    65,    -1,    -1,
      68,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    85,    86,    87,
      88,    89,    90,    91,    -1,    93,    94,    -1,    -1,    97,
      98,    99,    -1,   101,    -1,    -1,   104,    -1,    -1,   107,
     108,   109,   110,    -1,   112,   113,    -1,   115,    -1,   117,
     118,   119,    -1,    -1,   122,   123,   124,     3,   126,     5,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    18,    19,    -1,    -1,    22,    23,    24,    25,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    -1,    -1,    80,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
      -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,    -1,
      -1,   107,   108,   109,   110,    -1,   112,   113,     3,   115,
      -1,   117,   118,   119,    -1,    10,   122,   123,   124,    -1,
     126,    16,    -1,    18,    19,    -1,    -1,    22,    23,    24,
      25,    -1,    -1,    28,    -1,    -1,    31,    -1,    33,    -1,
      -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    61,    62,    -1,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    -1,    73,    74,
      75,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      85,    86,    87,    88,    89,    90,    91,    -1,    93,    94,
      -1,    -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,
      -1,    -1,   107,   108,   109,   110,    -1,   112,   113,    -1,
      -1,    -1,   117,   118,   119,    -1,    -1,   122,   123,   124,
       3,   126,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    18,    19,    -1,    -1,    22,
      23,    24,    25,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    90,    91,    -1,
      93,    94,    -1,    -1,    97,    98,    99,    -1,   101,    -1,
      -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,   112,
     113,     3,   115,     5,   117,   118,   119,    -1,    -1,   122,
     123,   124,    -1,   126,    16,    -1,    18,    19,    -1,    -1,
      22,    23,    24,    25,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    53,    54,    55,    56,    57,    -1,    59,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      -1,    93,    94,    -1,    -1,    97,    98,    99,    -1,   101,
      -1,    -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,
     112,   113,     3,   115,    -1,   117,   118,   119,    -1,    -1,
     122,   123,   124,    -1,   126,    16,    -1,    18,    19,    -1,
      -1,    22,    23,    24,    25,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    53,    54,    55,    56,    57,    -1,    -1,    60,
      -1,    62,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,    80,
      -1,    -1,    -1,    -1,    85,    86,    87,    88,    89,    90,
      91,    -1,    93,    94,    -1,    -1,    97,    98,    99,    -1,
     101,    -1,    -1,   104,    -1,    -1,   107,   108,   109,   110,
      -1,   112,   113,     3,   115,    -1,   117,   118,   119,    -1,
      -1,   122,   123,   124,    -1,   126,    16,    -1,    18,    19,
      20,    -1,    22,    23,    24,    25,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    53,    54,    55,    56,    57,    -1,    -1,
      -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    68,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    85,    86,    87,    88,    89,
      90,    91,    -1,    93,    94,    -1,    -1,    97,    98,    99,
      -1,   101,    -1,    -1,   104,    -1,    -1,   107,   108,   109,
     110,    -1,   112,   113,    -1,   115,     3,   117,   118,   119,
      -1,     8,   122,   123,   124,    -1,   126,    -1,    -1,    16,
      -1,    18,    19,    -1,    -1,    22,    23,    24,    25,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    85,    86,
      87,    88,    89,    90,    91,    -1,    93,    94,    -1,    -1,
      97,    98,    99,    -1,   101,    -1,    -1,   104,    -1,    -1,
     107,   108,   109,   110,    -1,   112,   113,     3,   115,    -1,
     117,   118,   119,    -1,    -1,   122,   123,   124,    -1,   126,
      16,    -1,    18,    19,    -1,    -1,    22,    23,    24,    25,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    53,    54,    55,
      56,    57,    -1,    59,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    85,
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
      -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,    -1,
      -1,   107,   108,   109,   110,    -1,   112,   113,     3,   115,
      -1,   117,   118,   119,    -1,    -1,   122,   123,   124,    -1,
     126,    16,    -1,    18,    19,    -1,    -1,    22,    23,    24,
      25,    -1,    -1,    28,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    62,    -1,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      85,    86,    87,    88,    89,    90,    91,    -1,    93,    94,
      -1,    -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,
      -1,    -1,   107,   108,   109,   110,    -1,   112,   113,     3,
     115,    -1,   117,   118,   119,    -1,    -1,   122,   123,   124,
      -1,   126,    16,    -1,    18,    19,    -1,    -1,    22,    23,
      24,    25,    -1,    -1,    28,    -1,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    -1,    73,
      74,    75,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    85,    86,    87,    88,    89,    90,    91,    -1,    93,
      94,    -1,    -1,    97,    98,    99,    -1,   101,    -1,    -1,
     104,    -1,    -1,   107,   108,   109,   110,    -1,   112,   113,
       3,   115,    -1,   117,   118,   119,     9,    -1,   122,   123,
     124,    -1,   126,    16,    -1,    18,    19,    -1,    -1,    22,
      23,    24,    25,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    -1,    -1,    44,    68,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    90,    91,    44,
      93,    94,    -1,    -1,    97,    98,    99,    77,   101,    -1,
      -1,   104,    -1,    -1,   107,   108,   109,   110,    -1,   112,
     113,    -1,   115,    44,   117,   118,   119,    -1,    -1,   122,
     123,   124,    77,   126,    -1,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,   146,   147,    77,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,    44,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,    44,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    77,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,    44,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    77,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      44,    -1,    -1,    -1,   146,    77,    -1,    -1,    -1,    -1,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,   146,
      -1,    -1,    -1,    77,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,    44,
      -1,    -1,    -1,   146,    -1,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      44,    -1,    -1,    -1,   146,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    77,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,    44,    -1,
      -1,    -1,   146,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,    77,    -1,    -1,
      -1,   146,    -1,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      -1,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,    -1,    -1,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    22,    25,    30,    35,    38,    40,    41,    42,
      46,    50,    63,    84,    93,    95,    98,   100,   102,   105,
     107,   117,   120,   154,   155,   158,   159,   160,   161,   168,
     213,   218,   219,   224,   225,   226,   227,   238,   239,   248,
     249,   250,   251,   252,   256,   257,   262,   263,   264,   267,
     269,   270,   271,   272,   273,   274,   276,   277,   278,   279,
     280,   281,   295,    99,   111,    61,     3,    16,    18,    19,
      22,    23,    24,    25,    28,    33,    37,    50,    53,    54,
      55,    56,    57,    62,    68,    69,    70,    71,    73,    74,
      75,    80,    85,    86,    87,    88,    89,    90,    91,    93,
      94,    97,    98,    99,   101,   104,   107,   108,   109,   110,
     112,   113,   115,   117,   118,   119,   122,   123,   124,   126,
     156,   157,    53,    86,    52,    53,    86,    23,    90,    99,
      61,    61,    87,     3,     4,     5,     6,     8,     9,    10,
      14,    18,    19,    24,    27,    37,    45,    49,    56,    57,
      59,    60,    62,    65,    74,    76,    89,    96,   110,   124,
     138,   139,   142,   145,   148,   157,   169,   170,   172,   205,
     206,   209,   275,   282,   283,   286,   287,    10,    31,    54,
      61,    80,   104,   156,   265,    16,    31,    33,    39,    54,
      61,    75,    85,    87,    88,   104,   108,   111,   112,   113,
     123,   215,   216,   265,   115,    99,   157,   173,     0,    47,
     144,   293,    66,   157,   157,   157,   157,   145,   157,   157,
     173,   157,   157,   157,   157,   157,   157,    52,   145,   150,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,     3,    74,    76,   205,   205,
     205,    60,   157,   210,   211,    13,    14,   151,   284,   285,
      52,   147,    20,   157,   171,    44,    77,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   145,    72,   197,   198,    67,   131,   131,   102,
       9,   156,   157,     5,     6,     8,    81,   157,   222,   223,
     131,   115,     8,   108,   157,   102,    71,   214,   214,   214,
     214,    82,   199,   200,   157,   122,   157,   102,   147,   205,
     290,   291,   292,   100,   105,   159,   157,   214,    92,    15,
      46,   114,     5,     6,     8,    14,   138,   145,   182,   183,
     234,   240,   241,    97,   118,   125,   175,   118,    52,   294,
     106,     8,   145,   146,   205,   207,   208,   157,   205,   207,
      43,   139,   207,   207,   146,   205,   207,   207,   205,   205,
     146,   207,   205,   205,   146,   145,   145,   145,   146,   147,
     149,   131,     8,   205,   285,   145,   173,   170,   205,   157,
     205,   205,   205,   205,   205,   205,   205,   287,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,     5,
      81,   142,   205,   222,   222,   131,   131,    54,   138,     8,
      48,    81,   116,   138,   157,   182,   220,   221,    68,   108,
     214,   108,     8,   157,   201,   202,    14,    32,   108,   217,
      71,   125,   258,   259,   157,   253,   254,   283,   157,   171,
      29,   147,   288,   289,   145,   228,    34,    34,    99,     5,
       6,     8,   146,   182,   184,   242,   147,   243,    27,    49,
      64,   109,   268,     8,     4,    18,    19,    27,    37,    45,
      47,    56,    59,    65,    73,   124,   145,   157,   176,   177,
     178,   179,   180,   181,   283,     8,     8,     8,   100,   161,
     146,   147,   146,   146,   157,   146,   146,   146,   146,   146,
     146,   146,   147,   146,   147,   146,   147,   146,   205,   205,
     211,   157,   182,   212,   152,   152,   165,   174,   175,    51,
     147,    81,   145,     5,   220,     9,   223,    70,   214,   214,
     131,   147,   103,     5,   103,     8,   157,   260,   261,   131,
     147,   175,   131,   290,    83,   193,   194,   292,    59,   157,
     229,   230,   121,   157,   157,   157,   146,   147,   146,   147,
     234,   241,   244,   245,   146,   106,   106,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   176,   128,    26,
      60,    67,   131,   132,   133,   134,   135,   136,   142,    26,
      60,   131,   132,   133,   134,   135,   136,   142,   146,   208,
     146,   205,   205,   205,   131,   100,   168,    55,   187,   157,
       5,   184,   131,    91,    94,   101,   266,     5,     8,   145,
     157,   202,   131,   127,   145,   182,   183,   254,   199,   182,
     183,    29,   197,   146,   147,   145,   231,   232,    27,    28,
      49,    65,    69,    78,    79,   109,   255,   182,     8,    20,
     246,   147,     8,     8,   157,   157,   157,   283,   139,   283,
     146,   146,   283,     8,   146,   146,   176,   182,   183,     9,
     145,    81,   142,     8,   182,   183,     8,   182,   183,   182,
     183,   182,   183,   182,   183,   182,   183,    26,    60,   182,
     145,   182,   182,   182,   182,   182,   182,    26,    60,   147,
     162,   146,   146,   147,   212,   169,   146,     5,   188,   126,
     191,   192,    60,   146,   145,    36,   119,    91,   157,   203,
     204,   145,     8,   261,   146,   184,    89,   180,   195,   196,
     229,   145,   233,   234,    82,   147,   235,    72,   157,   247,
     234,   245,   146,   146,   146,   146,   146,   146,   146,   128,
     128,     8,   184,   185,   186,    81,   182,     9,   145,   128,
     184,   182,   145,     5,    59,   157,   163,   164,   146,   145,
      52,    83,   166,    29,    55,    58,   190,   157,   282,   283,
     184,   131,   146,   147,     8,   146,   145,    21,    41,   147,
     146,   147,     3,   236,   237,   232,   182,   183,   182,   183,
     147,   146,   128,   186,   182,   146,   128,   184,   147,   207,
      29,    72,   167,   180,   189,    83,   178,   193,   146,   146,
     182,   204,   146,   146,   196,   234,   131,   147,     8,   182,
     146,   182,   146,   164,   146,   195,     5,   147,    29,   197,
       8,   237,   147,   147,   180,   195,   199,   145,     5,   207,
     146,   146
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

  case 89:

    { pParser->PushQuery(); ;}
    break;

  case 90:

    { pParser->PushQuery(); ;}
    break;

  case 91:

    { pParser->PushQuery(); ;}
    break;

  case 95:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 97:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 100:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 101:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 105:

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

  case 106:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 108:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 109:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 110:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 113:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 116:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 117:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 118:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 119:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 120:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 121:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 122:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 123:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 124:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 125:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 126:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 128:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 135:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 137:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 138:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 139:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_sRefString = pParser->m_pBuf;
		;}
    break;

  case 140:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
			pFilter->m_sRefString = pParser->m_pBuf;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-5], yyvsp[-3].m_iValue, yyvsp[-1].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 153:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 154:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 155:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 156:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 157:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 158:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 159:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 160:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 161:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 162:

    {
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-2], SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( yyvsp[-2].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 163:

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

  case 164:

    {
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-4], SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( yyvsp[-4].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			f->m_dValues.Uniq();
		;}
    break;

  case 165:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-5], SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( yyvsp[-5].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			f->m_dValues.Uniq();
		;}
    break;

  case 166:

    {
			AddMvaRange ( pParser, yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 167:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( yyvsp[-5], SPH_FILTER_RANGE );
			f->m_eMvaFunc = ( yyvsp[-5].m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_iMinValue = yyvsp[-2].m_iValue;
			f->m_iMaxValue = yyvsp[0].m_iValue;
		;}
    break;

  case 168:

    {
			AddMvaRange ( pParser, yyvsp[-2], INT64_MIN, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 169:

    {
			AddMvaRange ( pParser, yyvsp[-2], yyvsp[0].m_iValue+1, INT64_MAX );
		;}
    break;

  case 170:

    {
			AddMvaRange ( pParser, yyvsp[-2], INT64_MIN, yyvsp[0].m_iValue );
		;}
    break;

  case 171:

    {
			AddMvaRange ( pParser, yyvsp[-2], yyvsp[0].m_iValue, INT64_MAX );
		;}
    break;

  case 174:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 175:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 176:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 177:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 178:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 184:

    { yyval = yyvsp[-1]; yyval.m_iType = TOK_ANY; ;}
    break;

  case 185:

    { yyval = yyvsp[-1]; yyval.m_iType = TOK_ALL; ;}
    break;

  case 186:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 187:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 188:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 189:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 190:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 191:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 192:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 193:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 194:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 200:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 201:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 202:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 204:

    {
			pParser->AddHaving();
		;}
    break;

  case 207:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 210:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 211:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 213:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 215:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 216:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 219:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 220:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 226:

    {
			if ( !pParser->AddOption ( yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 227:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 228:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 229:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 230:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 231:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 232:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 233:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 234:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 236:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 237:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 242:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 243:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 244:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 245:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 247:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 248:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 249:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 250:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 251:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 252:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 254:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 255:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 256:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 257:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 258:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 259:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 260:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 261:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 262:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 266:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 267:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 268:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 269:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 270:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 271:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 272:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 273:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 274:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 275:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 276:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 277:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 278:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 279:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 280:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 281:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 289:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 290:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 297:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 298:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 299:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 300:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 301:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 302:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 303:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 304:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 305:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 306:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 307:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 308:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 309:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 310:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 314:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 315:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 316:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 317:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 318:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 319:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 320:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 321:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 322:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 323:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 324:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 327:

    { yyval.m_iValue = 1; ;}
    break;

  case 328:

    { yyval.m_iValue = 0; ;}
    break;

  case 329:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 337:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 338:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 339:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 342:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 343:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 344:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 349:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 350:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 353:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 354:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 355:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 356:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 357:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 358:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 359:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 360:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 365:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 366:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 367:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 368:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 369:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 371:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 372:

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

  case 373:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 376:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 378:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 383:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 386:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 387:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 388:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 391:

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

  case 392:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 393:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 394:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 395:

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

  case 396:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 397:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 398:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 399:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 400:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 401:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 402:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 403:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 404:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 405:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 406:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 407:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 408:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 409:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 416:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 417:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 418:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 426:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 427:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 428:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 429:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 430:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 431:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 432:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 433:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 434:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 435:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 436:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 439:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 440:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 441:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 442:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 443:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 444:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 447:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 449:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 450:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 451:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 452:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 453:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 454:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 455:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 459:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 461:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 464:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
		;}
    break;

  case 466:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 467:

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

