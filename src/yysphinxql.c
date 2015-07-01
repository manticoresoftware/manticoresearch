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
     TOK_ATTRIBUTES = 276,
     TOK_AVG = 277,
     TOK_BEGIN = 278,
     TOK_BETWEEN = 279,
     TOK_BIGINT = 280,
     TOK_BOOL = 281,
     TOK_BY = 282,
     TOK_CALL = 283,
     TOK_CHARACTER = 284,
     TOK_CHUNK = 285,
     TOK_COLLATION = 286,
     TOK_COLUMN = 287,
     TOK_COMMIT = 288,
     TOK_COMMITTED = 289,
     TOK_COUNT = 290,
     TOK_CREATE = 291,
     TOK_DATABASES = 292,
     TOK_DELETE = 293,
     TOK_DESC = 294,
     TOK_DESCRIBE = 295,
     TOK_DISTINCT = 296,
     TOK_DIV = 297,
     TOK_DOUBLE = 298,
     TOK_DROP = 299,
     TOK_FACET = 300,
     TOK_FALSE = 301,
     TOK_FLOAT = 302,
     TOK_FLUSH = 303,
     TOK_FOR = 304,
     TOK_FROM = 305,
     TOK_FUNCTION = 306,
     TOK_GLOBAL = 307,
     TOK_GROUP = 308,
     TOK_GROUPBY = 309,
     TOK_GROUP_CONCAT = 310,
     TOK_HAVING = 311,
     TOK_ID = 312,
     TOK_IN = 313,
     TOK_INDEX = 314,
     TOK_INSERT = 315,
     TOK_INT = 316,
     TOK_INTEGER = 317,
     TOK_INTO = 318,
     TOK_IS = 319,
     TOK_ISOLATION = 320,
     TOK_JSON = 321,
     TOK_LEVEL = 322,
     TOK_LIKE = 323,
     TOK_LIMIT = 324,
     TOK_MATCH = 325,
     TOK_MAX = 326,
     TOK_META = 327,
     TOK_MIN = 328,
     TOK_MOD = 329,
     TOK_MULTI = 330,
     TOK_MULTI64 = 331,
     TOK_NAMES = 332,
     TOK_NULL = 333,
     TOK_OPTION = 334,
     TOK_ORDER = 335,
     TOK_OPTIMIZE = 336,
     TOK_PLAN = 337,
     TOK_PLUGIN = 338,
     TOK_PLUGINS = 339,
     TOK_PROFILE = 340,
     TOK_RAND = 341,
     TOK_RAMCHUNK = 342,
     TOK_READ = 343,
     TOK_RECONFIGURE = 344,
     TOK_REPEATABLE = 345,
     TOK_REPLACE = 346,
     TOK_REMAP = 347,
     TOK_RETURNS = 348,
     TOK_ROLLBACK = 349,
     TOK_RTINDEX = 350,
     TOK_SELECT = 351,
     TOK_SERIALIZABLE = 352,
     TOK_SET = 353,
     TOK_SETTINGS = 354,
     TOK_SESSION = 355,
     TOK_SHOW = 356,
     TOK_SONAME = 357,
     TOK_START = 358,
     TOK_STATUS = 359,
     TOK_STRING = 360,
     TOK_SUM = 361,
     TOK_TABLE = 362,
     TOK_TABLES = 363,
     TOK_THREADS = 364,
     TOK_TO = 365,
     TOK_TRANSACTION = 366,
     TOK_TRUE = 367,
     TOK_TRUNCATE = 368,
     TOK_TYPE = 369,
     TOK_UNCOMMITTED = 370,
     TOK_UPDATE = 371,
     TOK_VALUES = 372,
     TOK_VARIABLES = 373,
     TOK_WARNINGS = 374,
     TOK_WEIGHT = 375,
     TOK_WHERE = 376,
     TOK_WITHIN = 377,
     TOK_OR = 378,
     TOK_AND = 379,
     TOK_NE = 380,
     TOK_GTE = 381,
     TOK_LTE = 382,
     TOK_NOT = 383,
     TOK_NEG = 384
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
#define TOK_ATTRIBUTES 276
#define TOK_AVG 277
#define TOK_BEGIN 278
#define TOK_BETWEEN 279
#define TOK_BIGINT 280
#define TOK_BOOL 281
#define TOK_BY 282
#define TOK_CALL 283
#define TOK_CHARACTER 284
#define TOK_CHUNK 285
#define TOK_COLLATION 286
#define TOK_COLUMN 287
#define TOK_COMMIT 288
#define TOK_COMMITTED 289
#define TOK_COUNT 290
#define TOK_CREATE 291
#define TOK_DATABASES 292
#define TOK_DELETE 293
#define TOK_DESC 294
#define TOK_DESCRIBE 295
#define TOK_DISTINCT 296
#define TOK_DIV 297
#define TOK_DOUBLE 298
#define TOK_DROP 299
#define TOK_FACET 300
#define TOK_FALSE 301
#define TOK_FLOAT 302
#define TOK_FLUSH 303
#define TOK_FOR 304
#define TOK_FROM 305
#define TOK_FUNCTION 306
#define TOK_GLOBAL 307
#define TOK_GROUP 308
#define TOK_GROUPBY 309
#define TOK_GROUP_CONCAT 310
#define TOK_HAVING 311
#define TOK_ID 312
#define TOK_IN 313
#define TOK_INDEX 314
#define TOK_INSERT 315
#define TOK_INT 316
#define TOK_INTEGER 317
#define TOK_INTO 318
#define TOK_IS 319
#define TOK_ISOLATION 320
#define TOK_JSON 321
#define TOK_LEVEL 322
#define TOK_LIKE 323
#define TOK_LIMIT 324
#define TOK_MATCH 325
#define TOK_MAX 326
#define TOK_META 327
#define TOK_MIN 328
#define TOK_MOD 329
#define TOK_MULTI 330
#define TOK_MULTI64 331
#define TOK_NAMES 332
#define TOK_NULL 333
#define TOK_OPTION 334
#define TOK_ORDER 335
#define TOK_OPTIMIZE 336
#define TOK_PLAN 337
#define TOK_PLUGIN 338
#define TOK_PLUGINS 339
#define TOK_PROFILE 340
#define TOK_RAND 341
#define TOK_RAMCHUNK 342
#define TOK_READ 343
#define TOK_RECONFIGURE 344
#define TOK_REPEATABLE 345
#define TOK_REPLACE 346
#define TOK_REMAP 347
#define TOK_RETURNS 348
#define TOK_ROLLBACK 349
#define TOK_RTINDEX 350
#define TOK_SELECT 351
#define TOK_SERIALIZABLE 352
#define TOK_SET 353
#define TOK_SETTINGS 354
#define TOK_SESSION 355
#define TOK_SHOW 356
#define TOK_SONAME 357
#define TOK_START 358
#define TOK_STATUS 359
#define TOK_STRING 360
#define TOK_SUM 361
#define TOK_TABLE 362
#define TOK_TABLES 363
#define TOK_THREADS 364
#define TOK_TO 365
#define TOK_TRANSACTION 366
#define TOK_TRUE 367
#define TOK_TRUNCATE 368
#define TOK_TYPE 369
#define TOK_UNCOMMITTED 370
#define TOK_UPDATE 371
#define TOK_VALUES 372
#define TOK_VARIABLES 373
#define TOK_WARNINGS 374
#define TOK_WEIGHT 375
#define TOK_WHERE 376
#define TOK_WITHIN 377
#define TOK_OR 378
#define TOK_AND 379
#define TOK_NE 380
#define TOK_GTE 381
#define TOK_LTE 382
#define TOK_NOT 383
#define TOK_NEG 384




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
#define YYFINAL  238
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4664

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  149
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  137
/* YYNRULES -- Number of rules. */
#define YYNRULES  483
/* YYNRULES -- Number of states. */
#define YYNSTATES  839

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   384

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   137,   126,     2,
     141,   142,   135,   133,   143,   134,   146,   136,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   140,
     129,   127,   130,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   147,     2,   148,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   144,   125,   145,     2,     2,     2,     2,
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
     128,   131,   132,   138,   139
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
     240,   242,   244,   246,   248,   250,   252,   254,   256,   260,
     263,   265,   267,   269,   278,   280,   290,   291,   294,   296,
     300,   302,   304,   306,   307,   311,   312,   315,   320,   332,
     334,   338,   340,   343,   344,   346,   349,   351,   356,   361,
     366,   371,   376,   381,   385,   391,   393,   397,   398,   400,
     403,   405,   409,   413,   418,   420,   424,   428,   434,   441,
     445,   450,   456,   460,   464,   468,   472,   476,   480,   486,
     492,   498,   502,   506,   510,   514,   518,   522,   526,   531,
     533,   535,   540,   544,   548,   550,   552,   557,   562,   567,
     571,   573,   576,   578,   581,   583,   585,   589,   591,   595,
     597,   599,   600,   605,   606,   608,   610,   614,   615,   618,
     619,   621,   627,   628,   630,   634,   640,   642,   646,   648,
     651,   654,   655,   657,   660,   665,   666,   668,   671,   673,
     677,   681,   685,   691,   698,   702,   704,   708,   712,   714,
     716,   718,   720,   722,   724,   726,   729,   732,   736,   740,
     744,   748,   752,   756,   760,   764,   768,   772,   776,   780,
     784,   788,   792,   796,   800,   804,   808,   810,   812,   814,
     818,   823,   828,   833,   838,   843,   848,   853,   857,   864,
     871,   875,   884,   899,   901,   905,   907,   909,   913,   919,
     921,   923,   925,   927,   930,   931,   934,   936,   939,   942,
     946,   948,   950,   952,   955,   960,   965,   969,   974,   979,
     981,   983,   984,   987,   992,   997,  1002,  1006,  1011,  1016,
    1024,  1030,  1036,  1046,  1048,  1050,  1052,  1054,  1056,  1058,
    1062,  1064,  1066,  1068,  1070,  1072,  1074,  1076,  1078,  1080,
    1083,  1091,  1093,  1095,  1096,  1100,  1102,  1104,  1106,  1110,
    1112,  1116,  1120,  1122,  1126,  1128,  1130,  1132,  1136,  1139,
    1140,  1143,  1145,  1149,  1153,  1158,  1165,  1167,  1171,  1173,
    1177,  1179,  1183,  1184,  1187,  1189,  1193,  1197,  1198,  1200,
    1202,  1204,  1208,  1210,  1212,  1216,  1220,  1227,  1229,  1233,
    1237,  1241,  1247,  1252,  1256,  1260,  1262,  1264,  1266,  1268,
    1270,  1272,  1274,  1276,  1278,  1286,  1293,  1298,  1303,  1309,
    1310,  1312,  1315,  1317,  1321,  1325,  1328,  1332,  1339,  1340,
    1342,  1344,  1347,  1350,  1353,  1355,  1363,  1365,  1367,  1369,
    1371,  1373,  1377,  1384,  1388,  1392,  1395,  1399,  1401,  1405,
    1408,  1412,  1416,  1424,  1430,  1432,  1434,  1437,  1439,  1442,
    1444,  1446,  1450,  1454,  1458,  1462,  1464,  1465,  1468,  1470,
    1473,  1475,  1477,  1481
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     150,     0,    -1,   151,    -1,   154,    -1,   154,   140,    -1,
     219,    -1,   231,    -1,   211,    -1,   212,    -1,   217,    -1,
     232,    -1,   241,    -1,   243,    -1,   244,    -1,   245,    -1,
     250,    -1,   255,    -1,   256,    -1,   260,    -1,   262,    -1,
     263,    -1,   264,    -1,   265,    -1,   266,    -1,   257,    -1,
     267,    -1,   269,    -1,   270,    -1,   271,    -1,   249,    -1,
     272,    -1,   273,    -1,     3,    -1,    15,    -1,    16,    -1,
      17,    -1,    19,    -1,    20,    -1,    21,    -1,    22,    -1,
      23,    -1,    24,    -1,    25,    -1,    26,    -1,    28,    -1,
      29,    -1,    30,    -1,    31,    -1,    32,    -1,    33,    -1,
      34,    -1,    35,    -1,    36,    -1,    37,    -1,    38,    -1,
      39,    -1,    40,    -1,    41,    -1,    43,    -1,    44,    -1,
      47,    -1,    48,    -1,    49,    -1,    51,    -1,    52,    -1,
      53,    -1,    55,    -1,    54,    -1,    56,    -1,    59,    -1,
      60,    -1,    61,    -1,    62,    -1,    63,    -1,    65,    -1,
      66,    -1,    67,    -1,    68,    -1,    70,    -1,    71,    -1,
      72,    -1,    73,    -1,    75,    -1,    76,    -1,    81,    -1,
      79,    -1,    82,    -1,    83,    -1,    84,    -1,    85,    -1,
      87,    -1,    86,    -1,    88,    -1,    89,    -1,    92,    -1,
      90,    -1,    91,    -1,    93,    -1,    94,    -1,    95,    -1,
      97,    -1,   100,    -1,    98,    -1,    99,    -1,   101,    -1,
     102,    -1,   103,    -1,   104,    -1,   105,    -1,   106,    -1,
     107,    -1,   108,    -1,   109,    -1,   110,    -1,   113,    -1,
     114,    -1,   115,    -1,   116,    -1,   117,    -1,   118,    -1,
     119,    -1,   120,    -1,   121,    -1,   122,    -1,   152,    -1,
      77,    -1,   111,    -1,   155,    -1,   154,   140,   155,    -1,
     154,   285,    -1,   156,    -1,   206,    -1,   157,    -1,    96,
       3,   141,   141,   157,   142,   158,   142,    -1,   164,    -1,
      96,   165,    50,   141,   161,   164,   142,   162,   163,    -1,
      -1,   143,   159,    -1,   160,    -1,   159,   143,   160,    -1,
     153,    -1,     5,    -1,    57,    -1,    -1,    80,    27,   189,
      -1,    -1,    69,     5,    -1,    69,     5,   143,     5,    -1,
      96,   165,    50,   169,   170,   181,   185,   184,   187,   191,
     193,    -1,   166,    -1,   165,   143,   166,    -1,   135,    -1,
     168,   167,    -1,    -1,   153,    -1,    18,   153,    -1,   199,
      -1,    22,   141,   199,   142,    -1,    71,   141,   199,   142,
      -1,    73,   141,   199,   142,    -1,   106,   141,   199,   142,
      -1,    55,   141,   199,   142,    -1,    35,   141,   135,   142,
      -1,    54,   141,   142,    -1,    35,   141,    41,   153,   142,
      -1,   153,    -1,   169,   143,   153,    -1,    -1,   171,    -1,
     121,   172,    -1,   173,    -1,   172,   124,   172,    -1,   141,
     172,   142,    -1,    70,   141,     8,   142,    -1,   174,    -1,
     175,   127,   176,    -1,   175,   128,   176,    -1,   175,    58,
     141,   180,   142,    -1,   175,   138,    58,   141,   180,   142,
      -1,   175,    58,     9,    -1,   175,   138,    58,     9,    -1,
     175,    24,   176,   124,   176,    -1,   175,   130,   176,    -1,
     175,   129,   176,    -1,   175,   131,   176,    -1,   175,   132,
     176,    -1,   175,   127,   177,    -1,   175,   128,   177,    -1,
     175,    24,   177,   124,   177,    -1,   175,    24,   176,   124,
     177,    -1,   175,    24,   177,   124,   176,    -1,   175,   130,
     177,    -1,   175,   129,   177,    -1,   175,   131,   177,    -1,
     175,   132,   177,    -1,   175,   127,     8,    -1,   175,   128,
       8,    -1,   175,    64,    78,    -1,   175,    64,   138,    78,
      -1,   153,    -1,     4,    -1,    35,   141,   135,   142,    -1,
      54,   141,   142,    -1,   120,   141,   142,    -1,    57,    -1,
     275,    -1,    62,   141,   275,   142,    -1,    43,   141,   275,
     142,    -1,    25,   141,   275,   142,    -1,    45,   141,   142,
      -1,     5,    -1,   134,     5,    -1,     6,    -1,   134,     6,
      -1,    14,    -1,   176,    -1,   178,   143,   176,    -1,     8,
      -1,   179,   143,     8,    -1,   178,    -1,   179,    -1,    -1,
      53,   182,    27,   183,    -1,    -1,     5,    -1,   175,    -1,
     183,   143,   175,    -1,    -1,    56,   174,    -1,    -1,   186,
      -1,   122,    53,    80,    27,   189,    -1,    -1,   188,    -1,
      80,    27,   189,    -1,    80,    27,    86,   141,   142,    -1,
     190,    -1,   189,   143,   190,    -1,   175,    -1,   175,    19,
      -1,   175,    39,    -1,    -1,   192,    -1,    69,     5,    -1,
      69,     5,   143,     5,    -1,    -1,   194,    -1,    79,   195,
      -1,   196,    -1,   195,   143,   196,    -1,   153,   127,   153,
      -1,   153,   127,     5,    -1,   153,   127,   141,   197,   142,
      -1,   153,   127,   153,   141,     8,   142,    -1,   153,   127,
       8,    -1,   198,    -1,   197,   143,   198,    -1,   153,   127,
     176,    -1,   153,    -1,     4,    -1,    57,    -1,     5,    -1,
       6,    -1,    14,    -1,     9,    -1,   134,   199,    -1,   138,
     199,    -1,   199,   133,   199,    -1,   199,   134,   199,    -1,
     199,   135,   199,    -1,   199,   136,   199,    -1,   199,   129,
     199,    -1,   199,   130,   199,    -1,   199,   126,   199,    -1,
     199,   125,   199,    -1,   199,   137,   199,    -1,   199,    42,
     199,    -1,   199,    74,   199,    -1,   199,   132,   199,    -1,
     199,   131,   199,    -1,   199,   127,   199,    -1,   199,   128,
     199,    -1,   199,   124,   199,    -1,   199,   123,   199,    -1,
     141,   199,   142,    -1,   144,   203,   145,    -1,   200,    -1,
     275,    -1,   278,    -1,   274,    64,    78,    -1,   274,    64,
     138,    78,    -1,     3,   141,   201,   142,    -1,    58,   141,
     201,   142,    -1,    62,   141,   201,   142,    -1,    25,   141,
     201,   142,    -1,    47,   141,   201,   142,    -1,    43,   141,
     201,   142,    -1,     3,   141,   142,    -1,    73,   141,   199,
     143,   199,   142,    -1,    71,   141,   199,   143,   199,   142,
      -1,   120,   141,   142,    -1,     3,   141,   199,    49,   153,
      58,   274,   142,    -1,    92,   141,   199,   143,   199,   143,
     141,   201,   142,   143,   141,   201,   142,   142,    -1,   202,
      -1,   201,   143,   202,    -1,   199,    -1,     8,    -1,   204,
     127,   205,    -1,   203,   143,   204,   127,   205,    -1,   153,
      -1,    58,    -1,   176,    -1,   153,    -1,   101,   208,    -1,
      -1,    68,     8,    -1,   119,    -1,   104,   207,    -1,    72,
     207,    -1,    16,   104,   207,    -1,    85,    -1,    82,    -1,
      84,    -1,   109,   193,    -1,    16,     8,   104,   207,    -1,
      16,   153,   104,   207,    -1,   209,   153,   104,    -1,   209,
     153,   210,    99,    -1,   209,   153,    14,    99,    -1,    59,
      -1,   107,    -1,    -1,    30,     5,    -1,    98,   152,   127,
     214,    -1,    98,   152,   127,   213,    -1,    98,   152,   127,
      78,    -1,    98,    77,   215,    -1,    98,    10,   127,   215,
      -1,    98,    29,    98,   215,    -1,    98,    52,     9,   127,
     141,   178,   142,    -1,    98,    52,   152,   127,   213,    -1,
      98,    52,   152,   127,     5,    -1,    98,    59,   153,    52,
       9,   127,   141,   178,   142,    -1,   153,    -1,     8,    -1,
     112,    -1,    46,    -1,   176,    -1,   216,    -1,   215,   134,
     216,    -1,   153,    -1,    78,    -1,     8,    -1,     5,    -1,
       6,    -1,    33,    -1,    94,    -1,   218,    -1,    23,    -1,
     103,   111,    -1,   220,    63,   153,   221,   117,   224,   228,
      -1,    60,    -1,    91,    -1,    -1,   141,   223,   142,    -1,
     153,    -1,    57,    -1,   222,    -1,   223,   143,   222,    -1,
     225,    -1,   224,   143,   225,    -1,   141,   226,   142,    -1,
     227,    -1,   226,   143,   227,    -1,   176,    -1,   177,    -1,
       8,    -1,   141,   178,   142,    -1,   141,   142,    -1,    -1,
      79,   229,    -1,   230,    -1,   229,   143,   230,    -1,     3,
     127,     8,    -1,    38,    50,   169,   171,    -1,    28,   153,
     141,   233,   236,   142,    -1,   234,    -1,   233,   143,   234,
      -1,   227,    -1,   141,   235,   142,    -1,     8,    -1,   235,
     143,     8,    -1,    -1,   143,   237,    -1,   238,    -1,   237,
     143,   238,    -1,   227,   239,   240,    -1,    -1,    18,    -1,
     153,    -1,    69,    -1,   242,   153,   207,    -1,    40,    -1,
      39,    -1,   101,   108,   207,    -1,   101,    37,   207,    -1,
     116,   169,    98,   246,   171,   193,    -1,   247,    -1,   246,
     143,   247,    -1,   153,   127,   176,    -1,   153,   127,   177,
      -1,   153,   127,   141,   178,   142,    -1,   153,   127,   141,
     142,    -1,   275,   127,   176,    -1,   275,   127,   177,    -1,
      62,    -1,    25,    -1,    47,    -1,    26,    -1,    75,    -1,
      76,    -1,    66,    -1,   105,    -1,    61,    -1,    17,   107,
     153,    15,    32,   153,   248,    -1,    17,   107,   153,    44,
      32,   153,    -1,    17,    95,   153,    89,    -1,   101,   258,
     118,   251,    -1,   101,   258,   118,    68,     8,    -1,    -1,
     252,    -1,   121,   253,    -1,   254,    -1,   253,   123,   254,
      -1,   153,   127,     8,    -1,   101,    31,    -1,   101,    29,
      98,    -1,    98,   258,   111,    65,    67,   259,    -1,    -1,
      52,    -1,   100,    -1,    88,   115,    -1,    88,    34,    -1,
      90,    88,    -1,    97,    -1,    36,    51,   153,    93,   261,
     102,     8,    -1,    61,    -1,    25,    -1,    47,    -1,   105,
      -1,    62,    -1,    44,    51,   153,    -1,    20,    59,   153,
     110,    95,   153,    -1,    48,    95,   153,    -1,    48,    87,
     153,    -1,    48,    21,    -1,    96,   268,   191,    -1,    10,
      -1,    10,   146,   153,    -1,    96,   199,    -1,   113,    95,
     153,    -1,    81,    59,   153,    -1,    36,    83,   153,   114,
       8,   102,     8,    -1,    44,    83,   153,   114,     8,    -1,
     275,    -1,   153,    -1,   153,   276,    -1,   277,    -1,   276,
     277,    -1,    13,    -1,    14,    -1,   147,   199,   148,    -1,
     147,     8,   148,    -1,   199,   127,   279,    -1,   279,   127,
     199,    -1,     8,    -1,    -1,   281,   284,    -1,    27,    -1,
     283,   167,    -1,   199,    -1,   282,    -1,   284,   143,   282,
      -1,    45,   284,   280,   187,   191,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   177,   177,   178,   179,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   224,   225,   225,   225,   225,   225,   225,   226,
     226,   226,   226,   226,   226,   227,   227,   227,   227,   227,
     228,   228,   228,   228,   228,   229,   229,   229,   229,   229,
     230,   230,   230,   230,   230,   230,   231,   231,   231,   231,
     231,   232,   232,   232,   232,   232,   232,   233,   233,   233,
     233,   233,   233,   234,   234,   234,   234,   234,   235,   235,
     235,   235,   235,   236,   236,   236,   236,   236,   237,   237,
     237,   237,   237,   238,   238,   238,   238,   238,   238,   239,
     239,   239,   239,   239,   239,   240,   240,   240,   240,   240,
     241,   241,   241,   241,   245,   245,   245,   251,   252,   253,
     257,   258,   262,   263,   271,   272,   279,   281,   285,   289,
     296,   297,   298,   302,   315,   322,   324,   329,   338,   354,
     355,   359,   360,   363,   365,   366,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   382,   383,   386,   388,   392,
     396,   397,   398,   402,   407,   411,   418,   426,   435,   445,
     450,   455,   460,   465,   470,   475,   480,   485,   490,   495,
     500,   505,   510,   515,   520,   525,   530,   535,   540,   548,
     549,   554,   560,   566,   572,   578,   579,   580,   581,   582,
     586,   587,   598,   599,   600,   604,   610,   617,   623,   630,
     631,   634,   636,   639,   641,   648,   652,   658,   660,   666,
     668,   672,   683,   685,   689,   693,   700,   701,   705,   706,
     707,   710,   712,   716,   721,   728,   730,   734,   738,   739,
     743,   748,   753,   759,   764,   772,   777,   784,   794,   795,
     796,   797,   798,   799,   800,   801,   802,   804,   805,   806,
     807,   808,   809,   810,   811,   812,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   823,   824,   825,   826,
     827,   831,   832,   833,   834,   835,   836,   837,   838,   839,
     840,   841,   842,   846,   847,   851,   852,   856,   857,   861,
     862,   866,   867,   873,   876,   878,   882,   883,   884,   885,
     886,   887,   888,   889,   890,   895,   900,   905,   910,   919,
     920,   923,   925,   933,   938,   943,   948,   949,   950,   954,
     959,   964,   969,   978,   979,   983,   984,   985,   997,   998,
    1002,  1003,  1004,  1005,  1006,  1013,  1014,  1015,  1019,  1020,
    1026,  1034,  1035,  1038,  1040,  1044,  1045,  1049,  1050,  1054,
    1055,  1059,  1063,  1064,  1068,  1069,  1070,  1071,  1072,  1075,
    1076,  1080,  1081,  1085,  1091,  1101,  1109,  1113,  1120,  1121,
    1128,  1138,  1144,  1146,  1150,  1155,  1159,  1166,  1168,  1172,
    1173,  1179,  1187,  1188,  1194,  1198,  1204,  1212,  1213,  1217,
    1231,  1237,  1241,  1246,  1260,  1271,  1272,  1273,  1274,  1275,
    1276,  1277,  1278,  1279,  1283,  1291,  1298,  1309,  1313,  1320,
    1321,  1325,  1329,  1330,  1334,  1338,  1345,  1352,  1358,  1359,
    1360,  1364,  1365,  1366,  1367,  1373,  1384,  1385,  1386,  1387,
    1388,  1393,  1404,  1416,  1425,  1434,  1444,  1452,  1453,  1457,
    1467,  1478,  1489,  1500,  1511,  1512,  1516,  1519,  1520,  1524,
    1525,  1526,  1527,  1531,  1532,  1536,  1541,  1543,  1547,  1556,
    1560,  1568,  1569,  1573
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
  "TOK_AGENT", "TOK_ALTER", "TOK_AS", "TOK_ASC", "TOK_ATTACH", 
  "TOK_ATTRIBUTES", "TOK_AVG", "TOK_BEGIN", "TOK_BETWEEN", "TOK_BIGINT", 
  "TOK_BOOL", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", "TOK_CHUNK", 
  "TOK_COLLATION", "TOK_COLUMN", "TOK_COMMIT", "TOK_COMMITTED", 
  "TOK_COUNT", "TOK_CREATE", "TOK_DATABASES", "TOK_DELETE", "TOK_DESC", 
  "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DOUBLE", "TOK_DROP", 
  "TOK_FACET", "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FOR", 
  "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", 
  "TOK_GROUP_CONCAT", "TOK_HAVING", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTEGER", "TOK_INTO", "TOK_IS", 
  "TOK_ISOLATION", "TOK_JSON", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", 
  "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_MULTI", 
  "TOK_MULTI64", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PLUGIN", "TOK_PLUGINS", "TOK_PROFILE", 
  "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", "TOK_RECONFIGURE", 
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
  "where_item", "filter_item", "expr_ident", "const_int", "const_float", 
  "const_list", "string_list", "const_list_or_string_list", 
  "opt_group_clause", "opt_int", "group_items_list", "opt_having_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "consthash", "hash_key", "hash_val", "show_stmt", 
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
  "drop_plugin", "json_field", "json_expr", "subscript", "subkey", 
  "streq", "strval", "opt_facet_by_items_list", "facet_by", "facet_item", 
  "facet_expr", "facet_items_list", "facet_stmt", 0
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
     375,   376,   377,   378,   379,   124,    38,    61,   380,    60,
      62,   381,   382,    43,    45,    42,    47,    37,   383,   384,
      59,    40,    41,    44,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   149,   150,   150,   150,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   153,   153,   153,   154,   154,   154,
     155,   155,   156,   156,   157,   157,   158,   158,   159,   159,
     160,   160,   160,   161,   162,   163,   163,   163,   164,   165,
     165,   166,   166,   167,   167,   167,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   169,   169,   170,   170,   171,
     172,   172,   172,   173,   173,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     176,   176,   177,   177,   177,   178,   178,   179,   179,   180,
     180,   181,   181,   182,   182,   183,   183,   184,   184,   185,
     185,   186,   187,   187,   188,   188,   189,   189,   190,   190,
     190,   191,   191,   192,   192,   193,   193,   194,   195,   195,
     196,   196,   196,   196,   196,   197,   197,   198,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   200,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   200,   201,   201,   202,   202,   203,   203,   204,
     204,   205,   205,   206,   207,   207,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   209,
     209,   210,   210,   211,   211,   211,   211,   211,   211,   212,
     212,   212,   212,   213,   213,   214,   214,   214,   215,   215,
     216,   216,   216,   216,   216,   217,   217,   217,   218,   218,
     219,   220,   220,   221,   221,   222,   222,   223,   223,   224,
     224,   225,   226,   226,   227,   227,   227,   227,   227,   228,
     228,   229,   229,   230,   231,   232,   233,   233,   234,   234,
     235,   235,   236,   236,   237,   237,   238,   239,   239,   240,
     240,   241,   242,   242,   243,   244,   245,   246,   246,   247,
     247,   247,   247,   247,   247,   248,   248,   248,   248,   248,
     248,   248,   248,   248,   249,   249,   249,   250,   250,   251,
     251,   252,   253,   253,   254,   255,   256,   257,   258,   258,
     258,   259,   259,   259,   259,   260,   261,   261,   261,   261,
     261,   262,   263,   264,   265,   266,   267,   268,   268,   269,
     270,   271,   272,   273,   274,   274,   275,   276,   276,   277,
     277,   277,   277,   278,   278,   279,   280,   280,   281,   282,
     283,   284,   284,   285
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
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     8,     1,     9,     0,     2,     1,     3,
       1,     1,     1,     0,     3,     0,     2,     4,    11,     1,
       3,     1,     2,     0,     1,     2,     1,     4,     4,     4,
       4,     4,     4,     3,     5,     1,     3,     0,     1,     2,
       1,     3,     3,     4,     1,     3,     3,     5,     6,     3,
       4,     5,     3,     3,     3,     3,     3,     3,     5,     5,
       5,     3,     3,     3,     3,     3,     3,     3,     4,     1,
       1,     4,     3,     3,     1,     1,     4,     4,     4,     3,
       1,     2,     1,     2,     1,     1,     3,     1,     3,     1,
       1,     0,     4,     0,     1,     1,     3,     0,     2,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     6,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     1,     3,
       4,     4,     4,     4,     4,     4,     4,     3,     6,     6,
       3,     8,    14,     1,     3,     1,     1,     3,     5,     1,
       1,     1,     1,     2,     0,     2,     1,     2,     2,     3,
       1,     1,     1,     2,     4,     4,     3,     4,     4,     1,
       1,     0,     2,     4,     4,     4,     3,     4,     4,     7,
       5,     5,     9,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       7,     1,     1,     0,     3,     1,     1,     1,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     3,     2,     0,
       2,     1,     3,     3,     4,     6,     1,     3,     1,     3,
       1,     3,     0,     2,     1,     3,     3,     0,     1,     1,
       1,     3,     1,     1,     3,     3,     6,     1,     3,     3,
       3,     5,     4,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     7,     6,     4,     4,     5,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     1,
       1,     3,     6,     3,     3,     2,     3,     1,     3,     2,
       3,     3,     7,     5,     1,     1,     2,     1,     2,     1,
       1,     3,     3,     3,     3,     1,     0,     2,     1,     2,
       1,     1,     3,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   358,     0,   355,     0,     0,   403,   402,
       0,     0,   361,     0,   362,   356,     0,   438,   438,     0,
       0,     0,     0,     2,     3,   127,   130,   132,   134,   131,
       7,     8,     9,   357,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    29,    15,    16,    17,    24,    18,    19,
      20,    21,    22,    23,    25,    26,    27,    28,    30,    31,
       0,     0,     0,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    67,    66,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,   125,    85,    84,    86,    87,
      88,    89,    91,    90,    92,    93,    95,    96,    94,    97,
      98,    99,   100,   102,   103,   101,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   126,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,     0,     0,
       0,     0,     0,     0,   455,     0,     0,     0,    32,   259,
     261,   262,   475,   264,   457,   263,    39,    42,    51,    58,
      60,    67,    66,   260,     0,    72,    79,    81,    94,   109,
     121,     0,   151,     0,     0,     0,   258,     0,   149,   153,
     156,   286,   241,     0,   287,   288,     0,     0,    45,    64,
      69,     0,   101,     0,     0,     0,     0,   435,   314,   439,
     329,   314,   321,   322,   320,   440,   314,   330,   314,   245,
     316,   313,     0,     0,   359,     0,   165,     0,     1,     0,
       4,   129,     0,   314,     0,     0,     0,     0,     0,     0,
       0,   451,     0,   454,   453,   461,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    32,    79,    81,   265,   266,     0,   310,   309,
       0,     0,   469,   470,     0,   466,   467,     0,     0,     0,
     154,   152,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   242,     0,     0,     0,     0,     0,     0,     0,   353,
     354,   352,   351,   350,   336,   348,     0,     0,     0,   314,
       0,   436,     0,   405,   318,   317,   404,     0,   323,   246,
     331,   429,   460,     0,     0,   480,   481,   153,   476,     0,
       0,   128,   363,   401,   426,     0,     0,     0,   210,   212,
     376,   214,     0,     0,   374,   375,   388,   392,   386,     0,
       0,     0,   384,     0,   306,     0,   297,   305,     0,   303,
     458,     0,   305,     0,     0,     0,     0,     0,   163,     0,
       0,     0,     0,     0,     0,     0,   300,     0,     0,     0,
     284,     0,   285,     0,   475,     0,   468,   143,   167,   150,
     156,   155,   276,   277,   283,   282,   274,   273,   280,   473,
     281,   271,   272,   279,   278,   267,   268,   269,   270,   275,
     243,   289,     0,   474,   337,   338,     0,     0,     0,     0,
     344,   346,   335,   345,     0,   343,   347,   334,   333,     0,
     314,   319,   314,   315,     0,   247,   248,     0,     0,   326,
       0,     0,     0,   427,   430,     0,     0,   407,     0,   166,
     479,   478,     0,   232,     0,     0,     0,     0,     0,     0,
     211,   213,   390,   378,   215,     0,     0,     0,     0,   447,
     448,   446,   450,   449,     0,     0,   200,    42,    51,    58,
       0,    67,   204,    72,    78,   121,     0,   199,   169,   170,
     174,     0,   205,   463,     0,     0,     0,   291,     0,   157,
     294,     0,   162,   296,   295,   161,   292,   293,   158,     0,
     159,     0,     0,   160,     0,     0,     0,   312,   311,   307,
     472,   471,     0,   221,   168,     0,   290,     0,   341,   340,
       0,   349,     0,   324,   325,     0,     0,   328,   332,   327,
     428,     0,   431,   432,     0,     0,   245,     0,   482,     0,
     241,   233,   477,   366,   365,   367,     0,     0,     0,   425,
     452,   377,     0,   389,     0,   397,   387,   393,   394,   385,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   136,     0,   304,   164,     0,     0,     0,     0,
       0,     0,   223,   229,   244,     0,     0,     0,     0,   444,
     437,   251,   254,     0,   250,   249,     0,     0,     0,   409,
     410,   408,   406,   413,   414,     0,   483,   364,     0,     0,
     379,   369,   416,   418,   417,   423,   415,   421,   419,   420,
     422,   424,   216,   391,   398,     0,     0,   445,   462,     0,
       0,     0,     0,   209,   202,     0,     0,   203,   172,   171,
       0,     0,   179,     0,   197,     0,   195,   175,   186,   196,
     176,   187,   183,   192,   182,   191,   184,   193,   185,   194,
       0,     0,     0,     0,   299,   298,     0,   308,     0,     0,
     224,     0,     0,   227,   230,   339,     0,   442,   441,   443,
       0,     0,   255,     0,   434,   433,   412,     0,    91,   238,
     234,   236,   368,     0,     0,   372,     0,     0,   360,   400,
     399,   396,   397,   395,   208,   201,   207,   206,   173,     0,
       0,   217,   219,   220,     0,   198,   180,     0,   141,   142,
     140,   137,   138,   133,   465,     0,   464,     0,     0,     0,
     145,     0,     0,     0,   232,     0,     0,   252,     0,     0,
     411,     0,   239,   240,     0,   371,     0,     0,   380,   381,
     370,   181,   189,   190,   188,     0,   177,     0,     0,   301,
       0,     0,     0,   135,   225,   222,     0,   228,   241,   342,
     257,   256,   253,   235,   237,   373,     0,     0,   218,   178,
     139,     0,   144,   146,     0,     0,   245,   383,   382,     0,
       0,   226,   231,   148,     0,   147,     0,     0,   302
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   157,   196,    24,    25,    26,    27,   702,
     761,   762,   542,   770,   803,    28,   197,   198,   291,   199,
     408,   543,   372,   508,   509,   510,   511,   484,   365,   485,
     753,   754,   623,   711,   805,   774,   713,   714,   570,   571,
     730,   731,   310,   311,   338,   339,   455,   456,   721,   722,
     382,   201,   378,   379,   280,   281,   539,    29,   333,   231,
     232,   460,    30,    31,   447,   448,   324,   325,    32,    33,
      34,    35,   476,   575,   576,   650,   651,   734,   366,   738,
     788,   789,    36,    37,   367,   368,   486,   488,   587,   588,
     665,   741,    38,    39,    40,    41,    42,   466,   467,   661,
      43,    44,   463,   464,   562,   563,    45,    46,    47,   214,
     630,    48,   494,    49,    50,    51,    52,    53,    54,   202,
      55,    56,    57,    58,    59,   203,   204,   285,   286,   205,
     206,   473,   474,   346,   347,   348,   241
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -724
static const short yypact[] =
{
    4497,     9,    33,  -724,  3830,  -724,    35,    49,  -724,  -724,
      47,     8,  -724,    75,  -724,  -724,   808,  3950,   223,    -6,
      36,  3830,   114,  -724,   -21,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,   112,  -724,  -724,  -724,  3830,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    3830,  3830,  3830,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,    -1,  3830,
    3830,  3830,  3830,  3830,  -724,  3830,  3830,  3830,     2,  -724,
    -724,  -724,  -724,  -724,    37,  -724,    53,    60,    71,    86,
      96,   102,   104,  -724,   110,   120,   126,   130,   139,   143,
     147,  1660,  -724,  1660,  1660,  3274,    19,    -5,  -724,  3383,
     539,  -724,    81,   133,   152,  -724,    91,    94,   208,  4070,
    3830,  2685,   198,   187,   204,  3503,   221,  -724,   252,  -724,
    -724,   252,  -724,  -724,  -724,  -724,   252,  -724,   252,   242,
    -724,  -724,  3830,   206,  -724,  3830,  -724,   -18,  -724,  1660,
      25,  -724,  3830,   252,   236,    67,   216,    13,   235,   215,
     -47,  -724,   220,  -724,  -724,  -724,   950,  3830,  1660,  1802,
     -13,  1802,  1802,   193,  1660,  1802,  1802,  1660,  1660,  1660,
    1660,   194,   196,   197,   200,  -724,  -724,  4215,  -724,  -724,
      55,   217,  -724,  -724,  1944,    27,  -724,  2322,  1092,  3830,
    -724,  -724,  1660,  1660,  1660,  1660,  1660,  1660,  1660,  1660,
    1660,  1660,  1660,  1660,  1660,  1660,  1660,  1660,  1660,   340,
    -724,  -724,   -29,  1660,  2685,  2685,   233,   238,   310,  -724,
    -724,  -724,  -724,  -724,   229,  -724,  2445,   303,   265,     4,
     266,  -724,   363,  -724,  -724,  -724,  -724,  3830,  -724,  -724,
      77,    -8,  -724,  3830,  3830,  4513,  -724,  3383,   -11,  1234,
     378,  -724,   232,  -724,  -724,   347,   349,   287,  -724,  -724,
    -724,  -724,   131,    26,  -724,  -724,  -724,   240,  -724,   256,
     376,  2076,  -724,   377,   259,  1376,  -724,  4498,   -15,  -724,
    -724,  4248,  4513,    72,  3830,   245,    80,    82,  -724,  4279,
      87,    90,   581,   618,  4073,  4312,  -724,  1518,  1660,  1660,
    -724,  3274,  -724,  2565,   241,   554,  -724,  -724,   -47,  -724,
    4513,  -724,  -724,  -724,  4527,   437,   222,   656,   385,  -724,
     385,   122,   122,   122,   122,    68,    68,  -724,  -724,  -724,
     247,  -724,   313,   385,   229,   229,   254,  3045,   379,  2685,
    -724,  -724,  -724,  -724,   388,  -724,  -724,  -724,  -724,   331,
     252,  -724,   252,  -724,   290,   275,  -724,   320,   415,  -724,
     323,   416,  3830,  -724,  -724,    63,   -31,  -724,   298,  -724,
    -724,  -724,  1660,   348,  1660,  3612,   312,  3830,  3830,  3830,
    -724,  -724,  -724,  -724,  -724,    92,    99,    13,   288,  -724,
    -724,  -724,  -724,  -724,   329,   330,  -724,   293,   295,   297,
     299,   302,  -724,   304,   305,   306,  2076,    27,   315,  -724,
    -724,   162,  -724,  -724,  1092,   307,  3830,  -724,  1802,  -724,
    -724,   309,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  1660,
    -724,  1660,  1660,  -724,  4108,  4151,   321,  -724,  -724,  -724,
    -724,  -724,   356,   400,  -724,   449,  -724,    46,  -724,  -724,
     343,  -724,    56,  -724,  -724,  2199,  3830,  -724,  -724,  -724,
    -724,   350,   333,  -724,    48,  3830,   242,    83,  -724,   453,
      81,  -724,   338,  -724,  -724,  -724,   105,   335,  4501,  -724,
    -724,  -724,    46,  -724,   476,    45,  -724,   345,  -724,  -724,
     478,   481,  3830,   355,  3830,   351,   352,  3830,   483,   357,
      -9,  2076,    83,    29,    -3,    65,    79,    83,    83,    83,
      83,   434,   358,   440,  -724,  -724,  4345,  4376,  4172,  2565,
    1092,   361,   495,   382,  -724,   107,   369,    14,   425,  -724,
    -724,  -724,  -724,  3830,   383,  -724,   515,  3830,     7,  -724,
    -724,  -724,  -724,  -724,  -724,  2805,  -724,  -724,  3612,    38,
     -49,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  3721,    38,  -724,  -724,    27,
     384,   386,   387,  -724,  -724,   389,   390,  -724,  -724,  -724,
     403,   406,  -724,    42,  -724,   455,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
      30,  3165,   392,  3830,  -724,  -724,   394,  -724,     5,   456,
    -724,   510,   485,   484,  -724,  -724,    46,  -724,  -724,  -724,
     414,   127,  -724,   534,  -724,  -724,  -724,   134,   402,    62,
     401,  -724,  -724,    31,   136,  -724,   542,   335,  -724,  -724,
    -724,  -724,   528,  -724,  -724,  -724,  -724,  -724,  -724,    83,
      83,  -724,   404,   405,   407,  -724,  -724,    42,  -724,  -724,
    -724,   410,  -724,  -724,    27,   408,  -724,  1802,  3830,   527,
     486,  2925,   477,  2925,   348,   144,    46,  -724,  3830,   417,
    -724,   418,  -724,  -724,  2925,  -724,    38,   429,   432,  -724,
    -724,  -724,  -724,  -724,  -724,   550,  -724,   435,  3165,  -724,
     155,  2925,   573,  -724,  -724,   436,   556,  -724,    81,  -724,
    -724,  -724,  -724,  -724,  -724,  -724,   576,   542,  -724,  -724,
    -724,   442,   401,   443,  2925,  2925,   242,  -724,  -724,   446,
     584,  -724,   401,  -724,  1802,  -724,   170,   450,  -724
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -724,  -724,  -724,    -7,    -4,  -724,   354,  -724,   224,  -724,
    -724,  -207,  -724,  -724,  -724,    58,   -25,   314,   251,  -724,
     -10,  -724,  -349,  -481,  -724,  -172,  -632,  -141,  -541,  -538,
    -724,  -154,  -724,  -724,  -724,  -724,  -724,  -724,  -170,  -724,
    -723,  -179,  -568,  -724,  -560,  -724,  -724,    50,  -724,  -171,
     108,  -724,  -258,    93,  -724,   209,    -2,  -724,  -206,  -724,
    -724,  -724,  -724,  -724,   175,  -724,   -41,   177,  -724,  -724,
    -724,  -724,  -724,   -34,  -724,  -724,  -119,  -724,  -482,  -724,
    -724,  -198,  -724,  -724,  -724,   137,  -724,  -724,  -724,   -46,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,    61,  -724,
    -724,  -724,  -724,  -724,  -724,   -16,  -724,  -724,  -724,   607,
    -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,  -724,
    -724,  -724,  -724,  -724,  -724,   -76,  -329,  -724,   346,  -724,
     332,  -724,  -724,   160,  -724,   164,  -724
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -476
static const short yytable[] =
{
     158,   383,   646,   386,   387,   585,   642,   390,   391,   625,
     213,   237,   358,   729,   468,   334,   471,   236,   358,   359,
     335,   360,   336,   640,   239,   600,   644,   361,   384,   164,
     736,   358,   282,   283,   482,   243,   358,   353,   682,   756,
     282,   283,   512,   358,   359,   287,   360,   358,   717,   431,
     751,   358,   361,   358,   359,   768,   244,   245,   246,   544,
     461,   681,   361,   664,   688,   691,   693,   695,   697,   699,
     358,   359,   332,   686,   371,   684,   282,   283,   822,   361,
     343,   782,   355,  -465,   358,   359,   159,   689,   358,   359,
     371,   457,    62,   361,   737,   165,   344,   361,   162,   161,
     727,   783,   832,   166,    60,   234,   364,   458,  -107,   432,
     292,   356,   565,   462,   238,   601,    61,   566,   160,   240,
     679,   349,   385,   451,   200,   344,   350,   517,   518,   718,
     163,   235,   472,   678,   167,   685,   480,   481,   288,   804,
     247,   444,   293,   256,   627,   752,   628,   362,   288,   726,
     309,   250,   729,   629,   363,   248,   249,   236,   251,   252,
     444,   253,   254,   255,   292,   444,   284,   735,   483,   729,
     683,   757,   362,   483,   284,   242,   444,   512,   775,   733,
     444,   459,   362,   257,   742,   446,   602,  -388,  -388,   638,
     564,   279,   831,   729,   258,   290,   293,   312,   401,   362,
     402,   259,   317,   306,   307,   308,   318,   323,   792,   794,
     284,   330,   260,   362,   520,   518,  -464,   362,   313,   752,
     603,   314,   523,   518,   524,   518,   604,   261,   340,   526,
     518,   342,   527,   518,   581,   582,   468,   262,   352,   215,
     826,   583,   584,   263,   553,   264,   554,   647,   648,   715,
     582,   265,   216,   380,   217,   304,   305,   306,   307,   308,
     218,   266,   538,   670,   292,   672,   833,   267,   675,   777,
     778,   268,   512,   434,   435,   219,   780,   582,   785,   786,
     269,   489,   220,   236,   270,   411,   809,   582,   271,   605,
     606,   607,   608,   609,   610,   221,   293,   821,   518,   275,
     611,   276,   277,   490,   815,   222,   315,   223,   224,  -440,
     323,   323,   837,   518,   326,   327,   512,   491,   492,   331,
     332,   337,   445,   225,   341,   354,   357,   226,   369,   370,
     227,   228,   229,   454,   373,   388,   396,   397,   398,   465,
     469,   399,   230,   290,   403,   430,   364,   345,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     436,   493,   438,   439,   377,   437,   381,   507,   449,   450,
     452,   453,   389,   475,   766,   392,   393,   394,   395,   477,
     521,   478,   479,   487,   495,   513,  -475,   522,   550,   540,
     545,   546,   405,   480,   215,   547,   410,   279,   552,   537,
     412,   413,   414,   415,   416,   417,   418,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   555,   556,   557,
     558,   433,   559,   639,   560,   567,   643,   292,   569,   577,
     589,   590,   591,   445,   592,   323,   593,   220,   594,   601,
     595,   662,   512,   596,   512,   597,   598,   599,   619,   612,
     221,   615,   620,   622,   624,   512,   637,   410,   561,   293,
     222,   680,   223,   224,   687,   690,   692,   694,   696,   698,
     626,   574,   512,   578,   579,   580,   649,   636,   538,   292,
     645,   472,   226,   277,   663,   227,   667,   229,   666,   668,
     671,   676,   700,   673,   674,   512,   512,   230,   703,   677,
     710,   701,   507,   709,   712,   377,   534,   535,   364,   800,
     716,   293,   613,   719,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   724,   723,   364,   744,   749,   745,   746,
     750,   747,   748,   755,   763,   767,   769,   771,   772,  -459,
     773,   776,   779,   781,   784,   787,   664,   582,   795,   796,
     799,   634,   454,   798,   801,   802,   816,   806,   818,   812,
     813,   465,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   817,   836,   819,   823,   824,
     345,   292,   345,   825,   827,   829,   830,   834,   669,   835,
     669,   820,   838,   669,   351,   708,   292,   507,   470,   515,
     621,   807,   409,   797,   808,   814,   635,   811,   791,   793,
     536,   614,   549,   293,   732,   537,   551,   707,   790,   828,
     743,   725,   410,   292,   586,   233,   641,   765,   293,   720,
     419,   406,   568,   561,     0,   810,     0,   616,   572,   617,
     618,   507,     0,     0,   574,   364,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   293,     0,     0,     0,     0,
     292,   740,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   293,     0,     0,     0,     0,   760,   292,   764,
       0,     0,   541,     0,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,     0,
       0,     0,     0,   528,   529,     0,     0,     0,   410,     0,
     293,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,     0,     0,     0,     0,
     530,   531,     0,     0,   236,     0,     0,   507,     0,   507,
       0,     0,     0,     0,   720,     0,     0,     0,     0,     0,
     507,     0,     0,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   760,     0,     0,   507,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   168,   169,   170,   171,     0,   172,   173,   174,     0,
     507,   507,   175,    64,    65,    66,     0,    67,    68,    69,
     176,    71,    72,   177,    74,     0,    75,    76,    77,    78,
      79,    80,    81,   178,    83,    84,    85,    86,    87,    88,
       0,   179,    90,     0,     0,   180,    92,    93,     0,    94,
      95,    96,   181,   182,    99,   183,   184,   100,   101,   102,
     185,   104,     0,   105,   106,   107,   108,     0,   109,   186,
     111,   187,     0,   113,   114,   115,     0,   116,     0,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     188,   129,   130,   131,     0,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   189,   142,   143,   144,   145,   146,
       0,   147,   148,   149,   150,   151,   152,   153,   190,   155,
     156,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   191,   192,     0,     0,   193,     0,     0,   194,
       0,     0,   195,   272,   169,   170,   171,     0,   374,   173,
       0,     0,     0,     0,   175,    64,    65,    66,     0,    67,
      68,    69,    70,    71,    72,   177,    74,     0,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,   179,    90,     0,     0,   180,    92,    93,
       0,    94,    95,    96,    97,    98,    99,   183,   184,   100,
     101,   102,   185,   104,     0,   105,   106,   107,   108,     0,
     109,   273,   111,   274,     0,   113,   114,   115,     0,   116,
       0,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   188,   129,   130,   131,     0,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,     0,   147,   148,   149,   150,   151,   152,   153,
     190,   155,   156,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   191,     0,     0,     0,   193,     0,
       0,   375,   376,     0,   195,   272,   169,   170,   171,     0,
     172,   173,     0,     0,     0,     0,   175,    64,    65,    66,
       0,    67,    68,    69,   176,    71,    72,   177,    74,     0,
      75,    76,    77,    78,    79,    80,    81,   178,    83,    84,
      85,    86,    87,    88,     0,   179,    90,     0,     0,   180,
      92,    93,     0,    94,    95,    96,   181,   182,    99,   183,
     184,   100,   101,   102,   185,   104,     0,   105,   106,   107,
     108,     0,   109,   186,   111,   187,     0,   113,   114,   115,
       0,   116,     0,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   188,   129,   130,   131,     0,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   189,   142,
     143,   144,   145,   146,     0,   147,   148,   149,   150,   151,
     152,   153,   190,   155,   156,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   191,   192,     0,     0,
     193,     0,     0,   194,     0,     0,   195,   168,   169,   170,
     171,     0,   172,   173,     0,     0,     0,     0,   175,    64,
      65,    66,     0,    67,    68,    69,   176,    71,    72,   177,
      74,     0,    75,    76,    77,    78,    79,    80,    81,   178,
      83,    84,    85,    86,    87,    88,     0,   179,    90,     0,
       0,   180,    92,    93,     0,    94,    95,    96,   181,   182,
      99,   183,   184,   100,   101,   102,   185,   104,     0,   105,
     106,   107,   108,     0,   109,   186,   111,   187,     0,   113,
     114,   115,     0,   116,     0,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   188,   129,   130,   131,
       0,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     189,   142,   143,   144,   145,   146,     0,   147,   148,   149,
     150,   151,   152,   153,   190,   155,   156,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   191,   192,
       0,     0,   193,     0,     0,   194,     0,     0,   195,   272,
     169,   170,   171,     0,   172,   173,     0,     0,     0,     0,
     175,    64,    65,    66,     0,    67,    68,    69,    70,    71,
      72,   177,    74,     0,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,     0,   179,
      90,     0,     0,   180,    92,    93,     0,    94,    95,    96,
      97,    98,    99,   183,   184,   100,   101,   102,   185,   104,
       0,   105,   106,   107,   108,     0,   109,   273,   111,   274,
       0,   113,   114,   115,     0,   116,     0,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   188,   129,
     130,   131,   514,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,     0,   147,
     148,   149,   150,   151,   152,   153,   190,   155,   156,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     191,     0,     0,     0,   193,     0,     0,   194,     0,     0,
     195,   272,   169,   170,   171,     0,   374,   173,     0,     0,
       0,     0,   175,    64,    65,    66,     0,    67,    68,    69,
      70,    71,    72,   177,    74,     0,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
       0,   179,    90,     0,     0,   180,    92,    93,     0,    94,
      95,    96,    97,    98,    99,   183,   184,   100,   101,   102,
     185,   104,     0,   105,   106,   107,   108,     0,   109,   273,
     111,   274,     0,   113,   114,   115,     0,   116,     0,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     188,   129,   130,   131,     0,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
       0,   147,   148,   149,   150,   151,   152,   153,   190,   155,
     156,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   191,     0,     0,     0,   193,     0,     0,   194,
     376,     0,   195,   272,   169,   170,   171,     0,   172,   173,
       0,     0,     0,     0,   175,    64,    65,    66,     0,    67,
      68,    69,    70,    71,    72,   177,    74,     0,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,   179,    90,     0,     0,   180,    92,    93,
       0,    94,    95,    96,    97,    98,    99,   183,   184,   100,
     101,   102,   185,   104,     0,   105,   106,   107,   108,     0,
     109,   273,   111,   274,     0,   113,   114,   115,     0,   116,
       0,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   188,   129,   130,   131,     0,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,     0,   147,   148,   149,   150,   151,   152,   153,
     190,   155,   156,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   191,     0,     0,     0,   193,     0,
       0,   194,     0,     0,   195,   272,   169,   170,   171,     0,
     374,   173,     0,     0,     0,     0,   175,    64,    65,    66,
       0,    67,    68,    69,    70,    71,    72,   177,    74,     0,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     0,   179,    90,     0,     0,   180,
      92,    93,     0,    94,    95,    96,    97,    98,    99,   183,
     184,   100,   101,   102,   185,   104,     0,   105,   106,   107,
     108,     0,   109,   273,   111,   274,     0,   113,   114,   115,
       0,   116,     0,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   188,   129,   130,   131,     0,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,     0,   147,   148,   149,   150,   151,
     152,   153,   190,   155,   156,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   191,     0,     0,     0,
     193,     0,     0,   194,     0,     0,   195,   272,   169,   170,
     171,     0,   404,   173,     0,     0,     0,     0,   175,    64,
      65,    66,     0,    67,    68,    69,    70,    71,    72,   177,
      74,     0,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,     0,   179,    90,     0,
       0,   180,    92,    93,     0,    94,    95,    96,    97,    98,
      99,   183,   184,   100,   101,   102,   185,   104,     0,   105,
     106,   107,   108,     0,   109,   273,   111,   274,     0,   113,
     114,   115,     0,   116,     0,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   188,   129,   130,   131,
       0,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,     0,   147,   148,   149,
     150,   151,   152,   153,   190,   155,   156,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   191,    63,
     496,     0,   193,     0,     0,   194,     0,     0,   195,     0,
       0,    64,    65,    66,     0,    67,    68,    69,    70,    71,
      72,   497,    74,     0,    75,    76,    77,    78,    79,    80,
      81,   498,    83,    84,    85,    86,    87,    88,     0,   499,
      90,   500,     0,    91,    92,    93,     0,    94,    95,    96,
     501,    98,    99,   502,     0,   100,   101,   102,   503,   104,
       0,   105,   106,   107,   108,     0,   504,   110,   111,   112,
       0,   113,   114,   115,     0,   116,     0,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,     0,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,     0,   147,
     148,   149,   150,   151,   152,   153,   505,   155,   156,     0,
       0,     0,    63,     0,   631,     0,     0,   632,     0,     0,
       0,     0,     0,     0,    64,    65,    66,   506,    67,    68,
      69,    70,    71,    72,    73,    74,     0,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,     0,    89,    90,     0,     0,    91,    92,    93,     0,
      94,    95,    96,    97,    98,    99,     0,     0,   100,   101,
     102,   103,   104,     0,   105,   106,   107,   108,     0,   109,
     110,   111,   112,     0,   113,   114,   115,     0,   116,     0,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,     0,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,     0,     0,     0,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    64,    65,    66,
     633,    67,    68,    69,    70,    71,    72,    73,    74,     0,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     0,    89,    90,     0,     0,    91,
      92,    93,     0,    94,    95,    96,    97,    98,    99,     0,
       0,   100,   101,   102,   103,   104,     0,   105,   106,   107,
     108,     0,   109,   110,   111,   112,     0,   113,   114,   115,
       0,   116,     0,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,     0,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,     0,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,     0,     0,     0,    63,     0,
     358,     0,     0,   440,     0,     0,     0,     0,     0,     0,
      64,    65,    66,   407,    67,    68,    69,    70,    71,    72,
      73,    74,     0,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,    89,    90,
       0,   441,    91,    92,    93,     0,    94,    95,    96,    97,
      98,    99,     0,     0,   100,   101,   102,   103,   104,     0,
     105,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     113,   114,   115,   442,   116,     0,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   443,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,    63,     0,
     358,     0,     0,     0,     0,     0,     0,     0,     0,   444,
      64,    65,    66,     0,    67,    68,    69,    70,    71,    72,
      73,    74,     0,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,    89,    90,
       0,     0,    91,    92,    93,     0,    94,    95,    96,    97,
      98,    99,     0,     0,   100,   101,   102,   103,   104,     0,
     105,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,     0,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,    63,     0,
     319,   320,     0,   321,     0,     0,     0,     0,     0,   444,
      64,    65,    66,     0,    67,    68,    69,    70,    71,    72,
      73,    74,     0,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,    89,    90,
       0,     0,    91,    92,    93,     0,    94,    95,    96,    97,
      98,    99,     0,     0,   100,   101,   102,   103,   104,     0,
     105,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     113,   114,   115,   322,   116,     0,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,     0,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,    63,   496,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      64,    65,    66,     0,    67,    68,    69,    70,    71,    72,
     497,    74,     0,    75,    76,    77,    78,    79,    80,    81,
     498,    83,    84,    85,    86,    87,    88,     0,   499,    90,
     500,     0,    91,    92,    93,     0,    94,    95,    96,   501,
      98,    99,   502,     0,   100,   101,   102,   503,   104,     0,
     105,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   120,
     121,   728,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,     0,   147,   148,
     149,   150,   151,   152,   153,   505,   155,   156,    63,   496,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      64,    65,    66,     0,    67,    68,    69,    70,    71,    72,
     497,    74,     0,    75,    76,    77,    78,    79,    80,    81,
     498,    83,    84,    85,    86,    87,    88,     0,   499,    90,
     500,     0,    91,    92,    93,     0,    94,    95,    96,   501,
      98,    99,   502,     0,   100,   101,   102,   503,   104,     0,
     105,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,     0,   147,   148,
     149,   150,   151,   152,   153,   505,   155,   156,    63,     0,
     548,     0,     0,   440,     0,     0,     0,     0,     0,     0,
      64,    65,    66,     0,    67,    68,    69,    70,    71,    72,
      73,    74,     0,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,    89,    90,
       0,     0,    91,    92,    93,     0,    94,    95,    96,    97,
      98,    99,     0,     0,   100,   101,   102,   103,   104,     0,
     105,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,     0,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,    63,     0,
     758,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      64,    65,    66,     0,    67,    68,    69,    70,    71,    72,
      73,    74,     0,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,    89,    90,
       0,     0,    91,    92,    93,     0,    94,    95,    96,    97,
      98,    99,   759,     0,   100,   101,   102,   103,   104,     0,
     105,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,    63,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,     0,    64,
      65,    66,     0,    67,    68,    69,    70,    71,    72,    73,
      74,     0,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,     0,    89,    90,     0,
       0,    91,    92,    93,     0,    94,    95,    96,    97,    98,
      99,     0,   278,   100,   101,   102,   103,   104,     0,   105,
     106,   107,   108,     0,   109,   110,   111,   112,     0,   113,
     114,   115,     0,   116,     0,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,    63,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,     0,    64,    65,
      66,   289,    67,    68,    69,    70,    71,    72,    73,    74,
       0,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,     0,    89,    90,     0,     0,
      91,    92,    93,     0,    94,    95,    96,    97,    98,    99,
       0,     0,   100,   101,   102,   103,   104,     0,   105,   106,
     107,   108,     0,   109,   110,   111,   112,     0,   113,   114,
     115,     0,   116,     0,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,     0,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,    63,     0,     0,     0,
       0,   328,     0,     0,     0,     0,     0,     0,    64,    65,
      66,     0,    67,    68,    69,    70,    71,    72,    73,    74,
       0,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,     0,    89,    90,     0,     0,
      91,    92,    93,     0,    94,    95,    96,    97,    98,    99,
       0,     0,   100,   101,   102,   103,   104,     0,   105,   106,
     107,   108,     0,   109,   110,   111,   112,     0,   113,   114,
     115,     0,   116,     0,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
     132,   133,   134,   135,   136,   137,   138,   329,   140,   141,
     142,   143,   144,   145,   146,    63,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,     0,    64,    65,    66,
       0,    67,    68,    69,    70,    71,    72,    73,    74,     0,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     0,    89,    90,     0,     0,    91,
      92,    93,     0,    94,    95,    96,    97,    98,    99,   573,
       0,   100,   101,   102,   103,   104,     0,   105,   106,   107,
     108,     0,   109,   110,   111,   112,     0,   113,   114,   115,
       0,   116,     0,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,     0,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,    63,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,     0,    64,    65,    66,     0,
      67,    68,    69,    70,    71,    72,    73,    74,     0,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,     0,    89,    90,     0,     0,    91,    92,
      93,     0,    94,    95,    96,    97,    98,    99,     0,     0,
     100,   101,   102,   103,   104,     0,   105,   106,   107,   108,
     739,   109,   110,   111,   112,     0,   113,   114,   115,     0,
     116,     0,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,     0,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,    63,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,     0,    64,    65,    66,     0,    67,
      68,    69,    70,    71,    72,    73,    74,     0,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,    89,    90,     0,     0,    91,    92,    93,
       0,    94,    95,    96,    97,    98,    99,     0,     0,   100,
     101,   102,   103,   104,     0,   105,   106,   107,   108,     0,
     109,   110,   111,   112,     0,   113,   114,   115,     0,   116,
       0,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,     0,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    63,     0,     0,     0,     0,     0,     0,
     207,     0,     0,     0,     0,    64,    65,    66,     0,    67,
      68,    69,    70,    71,    72,    73,    74,     0,    75,   208,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,    89,    90,     0,     0,    91,    92,    93,
       0,    94,   209,    96,    97,    98,    99,     0,     0,   210,
     101,   102,   103,   104,     0,   105,   106,   107,   108,     0,
     109,   110,   111,   112,     0,   113,   114,   211,     0,   116,
       0,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,   132,   133,   134,
     212,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,     0,     0,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    63,     0,     0,     0,     0,     0,   316,
       0,     0,     0,     0,     0,    64,    65,    66,     0,    67,
      68,    69,    70,    71,    72,    73,    74,     0,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,    89,    90,   292,     0,    91,    92,    93,
       0,    94,    95,    96,    97,    98,    99,     0,     0,   100,
     101,   102,   103,   104,     0,   105,   106,   107,   108,     0,
     109,   110,   111,   112,     0,   113,   114,   293,     0,   116,
     292,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,  -439,   293,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   292,     0,     0,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,     0,     0,     0,   292,     0,   532,     0,     0,     0,
       0,     0,     0,     0,     0,   293,     0,     0,     0,     0,
       0,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   293,     0,     0,     0,
       0,   529,     0,     0,     0,     0,     0,   292,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   293,
     292,     0,     0,     0,   531,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
       0,     0,     0,     0,     0,   706,     0,     0,     0,     0,
       0,   292,   293,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   293,   292,     0,     0,   400,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   293,   292,     0,     0,
     519,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,     0,   292,   293,
       0,   525,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     293,     0,     0,     0,   533,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,     0,     0,     0,     0,   704,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,     1,     0,     0,     2,   705,     0,
       3,     0,     0,     0,     0,     4,   652,   653,     0,     0,
       5,     0,     0,     6,     0,     7,     8,     9,     0,     0,
     292,    10,     0,     0,     0,    11,     0,   516,   654,     0,
       0,     0,     0,     0,     0,   292,     0,    12,     0,     0,
       0,     0,   655,   656,     0,     0,     0,   657,     0,   292,
       0,     0,   293,     0,     0,     0,   658,   659,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   293,    14,     0,
       0,    15,     0,    16,     0,    17,     0,     0,    18,     0,
      19,   293,     0,     0,     0,     0,   660,     0,     0,     0,
      20,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308
};

static const short yycheck[] =
{
       4,   259,   570,   261,   262,   487,   566,   265,   266,   547,
      17,    21,     5,   645,   343,   221,    27,    21,     5,     6,
     226,     8,   228,   564,    45,   506,   567,    14,    41,    21,
      79,     5,    13,    14,     8,    39,     5,   243,     9,     9,
      13,    14,   371,     5,     6,    50,     8,     5,    34,    78,
       8,     5,    14,     5,     6,    50,    60,    61,    62,   408,
      68,   602,    14,    18,   605,   606,   607,   608,   609,   610,
       5,     6,    68,     8,   121,    78,    13,    14,   801,    14,
      98,    19,    15,    64,     5,     6,    51,     8,     5,     6,
     121,    14,    59,    14,   143,    87,   143,    14,    51,    50,
     638,    39,   825,    95,    95,   111,   247,    30,   104,   138,
      42,    44,   143,   121,     0,   124,   107,   466,    83,   140,
     601,    96,   135,   329,    16,   143,   101,   142,   143,   115,
      83,    95,   143,   142,    59,   138,     5,     6,   143,   771,
     141,   134,    74,   141,    88,   683,    90,   134,   143,   142,
      69,   161,   784,    97,   141,   159,   160,   161,   162,   163,
     134,   165,   166,   167,    42,   134,   147,   649,   142,   801,
     141,   141,   134,   142,   147,    63,   134,   506,   716,   141,
     134,   104,   134,   146,   666,   326,    24,   142,   143,   141,
     127,   195,   824,   825,   141,   199,    74,    64,   143,   134,
     145,   141,   209,   135,   136,   137,   210,   211,   749,   750,
     147,   215,   141,   134,   142,   143,    64,   134,   127,   757,
      58,   127,   142,   143,   142,   143,    64,   141,   232,   142,
     143,   235,   142,   143,   142,   143,   565,   141,   242,    16,
     808,   142,   143,   141,   450,   141,   452,   142,   143,   142,
     143,   141,    29,   257,    31,   133,   134,   135,   136,   137,
      37,   141,   403,   592,    42,   594,   826,   141,   597,   142,
     143,   141,   601,   314,   315,    52,   142,   143,   142,   143,
     141,    25,    59,   287,   141,   289,   142,   143,   141,   127,
     128,   129,   130,   131,   132,    72,    74,   142,   143,   191,
     138,   193,   194,    47,   786,    82,    98,    84,    85,   111,
     314,   315,   142,   143,   127,   111,   645,    61,    62,    98,
      68,    79,   326,   100,   118,    89,   110,   104,    93,   114,
     107,   108,   109,   337,   114,   142,   142,   141,   141,   343,
     344,   141,   119,   347,   127,     5,   487,   239,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     127,   105,    52,   134,   256,   127,   258,   371,    65,   104,
     104,     8,   264,   141,   703,   267,   268,   269,   270,    32,
     384,    32,    95,   143,     8,     8,   127,   142,     9,   148,
     143,    78,   284,     5,    16,   141,   288,   401,    67,   403,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   127,   143,    99,
       5,   313,    99,   564,     8,   127,   567,    42,    80,   117,
     142,   102,   102,   437,   141,   439,   141,    59,   141,   124,
     141,   582,   771,   141,   773,   141,   141,   141,   127,   142,
      72,   142,    96,    53,     5,   784,   123,   349,   462,    74,
      82,   602,    84,    85,   605,   606,   607,   608,   609,   610,
     127,   475,   801,   477,   478,   479,   141,   127,   619,    42,
      27,   143,   104,   375,     8,   107,     8,   109,   143,     8,
     135,     8,    58,   142,   142,   824,   825,   119,    58,   142,
       5,   143,   506,   142,   122,   397,   398,   399,   649,   767,
     141,    74,   516,    88,   129,   130,   131,   132,   133,   134,
     135,   136,   137,     8,   141,   666,   142,   124,   142,   142,
     124,   142,   142,    78,   142,   141,    80,    27,    53,     0,
      56,   127,     8,   141,   143,     3,    18,   143,   143,   142,
     142,   555,   556,   143,    27,    69,   127,    80,     8,   142,
     142,   565,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   143,   834,   142,     5,   143,
     472,    42,   474,    27,     8,   143,   143,   141,   592,     5,
     594,   798,   142,   597,   240,   620,    42,   601,   347,   375,
     542,   773,   288,   757,   774,   784,   556,   778,   749,   750,
     401,   518,   437,    74,   648,   619,   439,   619,   737,   817,
     666,   637,   514,    42,   487,    18,   565,   703,    74,   633,
     298,   285,   472,   637,    -1,   776,    -1,   529,   474,   531,
     532,   645,    -1,    -1,   648,   786,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      42,   665,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    74,    -1,    -1,    -1,    -1,   701,    42,   703,
      -1,    -1,   148,    -1,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,   142,   143,    -1,    -1,    -1,   620,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
     142,   143,    -1,    -1,   768,    -1,    -1,   771,    -1,   773,
      -1,    -1,    -1,    -1,   778,    -1,    -1,    -1,    -1,    -1,
     784,    -1,    -1,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   798,    -1,    -1,   801,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,    -1,
     824,   825,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      -1,    43,    44,    -1,    -1,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    75,    76,    77,    -1,    79,    -1,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,    -1,   138,    -1,    -1,   141,
      -1,    -1,   144,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    43,    44,    -1,    -1,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    75,    76,    77,    -1,    79,
      -1,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,   138,    -1,
      -1,   141,   142,    -1,   144,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    43,    44,    -1,    -1,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    75,    76,    77,
      -1,    79,    -1,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
     138,    -1,    -1,   141,    -1,    -1,   144,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    43,    44,    -1,
      -1,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    75,
      76,    77,    -1,    79,    -1,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,    -1,   138,    -1,    -1,   141,    -1,    -1,   144,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    43,
      44,    -1,    -1,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    75,    76,    77,    -1,    79,    -1,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,    -1,    -1,    -1,   138,    -1,    -1,   141,    -1,    -1,
     144,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      -1,    43,    44,    -1,    -1,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    75,    76,    77,    -1,    79,    -1,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,    -1,    -1,   138,    -1,    -1,   141,
     142,    -1,   144,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    43,    44,    -1,    -1,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    75,    76,    77,    -1,    79,
      -1,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,   138,    -1,
      -1,   141,    -1,    -1,   144,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    43,    44,    -1,    -1,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    75,    76,    77,
      -1,    79,    -1,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
     138,    -1,    -1,   141,    -1,    -1,   144,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    43,    44,    -1,
      -1,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    75,
      76,    77,    -1,    79,    -1,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,     3,
       4,    -1,   138,    -1,    -1,   141,    -1,    -1,   144,    -1,
      -1,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    43,
      44,    45,    -1,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    -1,    59,    60,    61,    62,    63,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    75,    76,    77,    -1,    79,    -1,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,     3,    -1,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    16,    17,   141,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    -1,    43,    44,    -1,    -1,    47,    48,    49,    -1,
      51,    52,    53,    54,    55,    56,    -1,    -1,    59,    60,
      61,    62,    63,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    75,    76,    77,    -1,    79,    -1,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
     141,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    43,    44,    -1,    -1,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    75,    76,    77,
      -1,    79,    -1,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,     3,    -1,
       5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,   141,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    43,    44,
      -1,    46,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     3,    -1,
       5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    43,    44,
      -1,    -1,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      75,    76,    77,    -1,    79,    -1,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     3,    -1,
       5,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   134,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    43,    44,
      -1,    -1,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     3,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    43,    44,
      45,    -1,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    -1,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      75,    76,    77,    -1,    79,    -1,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     3,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    43,    44,
      45,    -1,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    -1,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      75,    76,    77,    -1,    79,    -1,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     3,    -1,
       5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    43,    44,
      -1,    -1,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      75,    76,    77,    -1,    79,    -1,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     3,    -1,
       5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    43,    44,
      -1,    -1,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    -1,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      75,    76,    77,    -1,    79,    -1,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     3,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    43,    44,    -1,
      -1,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    62,    63,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    75,
      76,    77,    -1,    79,    -1,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     3,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    43,    44,    -1,    -1,
      47,    48,    49,    -1,    51,    52,    53,    54,    55,    56,
      -1,    -1,    59,    60,    61,    62,    63,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    75,    76,
      77,    -1,    79,    -1,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,     3,    -1,    -1,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    43,    44,    -1,    -1,
      47,    48,    49,    -1,    51,    52,    53,    54,    55,    56,
      -1,    -1,    59,    60,    61,    62,    63,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    75,    76,
      77,    -1,    79,    -1,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     3,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    43,    44,    -1,    -1,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    75,    76,    77,
      -1,    79,    -1,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     3,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    43,    44,    -1,    -1,    47,    48,
      49,    -1,    51,    52,    53,    54,    55,    56,    -1,    -1,
      59,    60,    61,    62,    63,    -1,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    -1,
      79,    -1,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     3,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    43,    44,    -1,    -1,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    -1,    -1,    59,
      60,    61,    62,    63,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    75,    76,    77,    -1,    79,
      -1,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,     3,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    43,    44,    -1,    -1,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    -1,    -1,    59,
      60,    61,    62,    63,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    75,    76,    77,    -1,    79,
      -1,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    -1,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,     3,    -1,    -1,    -1,    -1,    -1,     9,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    43,    44,    42,    -1,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    -1,    -1,    59,
      60,    61,    62,    63,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    75,    76,    74,    -1,    79,
      42,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    74,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    42,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    42,    -1,   143,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    74,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    74,
      42,    -1,    -1,    -1,   143,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
      -1,    42,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    74,    42,    -1,    -1,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    74,    42,    -1,    -1,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,    -1,    42,    74,
      -1,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      74,    -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    17,    -1,    -1,    20,   142,    -1,
      23,    -1,    -1,    -1,    -1,    28,    25,    26,    -1,    -1,
      33,    -1,    -1,    36,    -1,    38,    39,    40,    -1,    -1,
      42,    44,    -1,    -1,    -1,    48,    -1,    49,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    60,    -1,    -1,
      -1,    -1,    61,    62,    -1,    -1,    -1,    66,    -1,    42,
      -1,    -1,    74,    -1,    -1,    -1,    75,    76,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    91,    -1,
      -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,    -1,
     103,    74,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
     113,    -1,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    23,    28,    33,    36,    38,    39,    40,
      44,    48,    60,    81,    91,    94,    96,    98,   101,   103,
     113,   116,   150,   151,   154,   155,   156,   157,   164,   206,
     211,   212,   217,   218,   219,   220,   231,   232,   241,   242,
     243,   244,   245,   249,   250,   255,   256,   257,   260,   262,
     263,   264,   265,   266,   267,   269,   270,   271,   272,   273,
      95,   107,    59,     3,    15,    16,    17,    19,    20,    21,
      22,    23,    24,    25,    26,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    43,
      44,    47,    48,    49,    51,    52,    53,    54,    55,    56,
      59,    60,    61,    62,    63,    65,    66,    67,    68,    70,
      71,    72,    73,    75,    76,    77,    79,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   152,   153,    51,
      83,    50,    51,    83,    21,    87,    95,    59,     3,     4,
       5,     6,     8,     9,    10,    14,    22,    25,    35,    43,
      47,    54,    55,    57,    58,    62,    71,    73,    92,   106,
     120,   134,   135,   138,   141,   144,   153,   165,   166,   168,
     199,   200,   268,   274,   275,   278,   279,    10,    29,    52,
      59,    77,   100,   152,   258,    16,    29,    31,    37,    52,
      59,    72,    82,    84,    85,   100,   104,   107,   108,   109,
     119,   208,   209,   258,   111,    95,   153,   169,     0,    45,
     140,   285,    63,   153,   153,   153,   153,   141,   153,   153,
     169,   153,   153,   153,   153,   153,   141,   146,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,     3,    71,    73,   199,   199,   199,    58,   153,
     203,   204,    13,    14,   147,   276,   277,    50,   143,    18,
     153,   167,    42,    74,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    69,
     191,   192,    64,   127,   127,    98,     9,   152,   153,     5,
       6,     8,    78,   153,   215,   216,   127,   111,     8,   104,
     153,    98,    68,   207,   207,   207,   207,    79,   193,   194,
     153,   118,   153,    98,   143,   199,   282,   283,   284,    96,
     101,   155,   153,   207,    89,    15,    44,   110,     5,     6,
       8,    14,   134,   141,   176,   177,   227,   233,   234,    93,
     114,   121,   171,   114,     8,   141,   142,   199,   201,   202,
     153,   199,   199,   201,    41,   135,   201,   201,   142,   199,
     201,   201,   199,   199,   199,   199,   142,   141,   141,   141,
     142,   143,   145,   127,     8,   199,   277,   141,   169,   166,
     199,   153,   199,   199,   199,   199,   199,   199,   199,   279,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
       5,    78,   138,   199,   215,   215,   127,   127,    52,   134,
       8,    46,    78,   112,   134,   153,   176,   213,   214,    65,
     104,   207,   104,     8,   153,   195,   196,    14,    30,   104,
     210,    68,   121,   251,   252,   153,   246,   247,   275,   153,
     167,    27,   143,   280,   281,   141,   221,    32,    32,    95,
       5,     6,     8,   142,   176,   178,   235,   143,   236,    25,
      47,    61,    62,   105,   261,     8,     4,    25,    35,    43,
      45,    54,    57,    62,    70,   120,   141,   153,   172,   173,
     174,   175,   275,     8,    96,   157,    49,   142,   143,   142,
     142,   153,   142,   142,   142,   142,   142,   142,   142,   143,
     142,   143,   143,   142,   199,   199,   204,   153,   176,   205,
     148,   148,   161,   170,   171,   143,    78,   141,     5,   213,
       9,   216,    67,   207,   207,   127,   143,    99,     5,    99,
       8,   153,   253,   254,   127,   143,   171,   127,   282,    80,
     187,   188,   284,    57,   153,   222,   223,   117,   153,   153,
     153,   142,   143,   142,   143,   227,   234,   237,   238,   142,
     102,   102,   141,   141,   141,   141,   141,   141,   141,   141,
     172,   124,    24,    58,    64,   127,   128,   129,   130,   131,
     132,   138,   142,   153,   202,   142,   199,   199,   199,   127,
      96,   164,    53,   181,     5,   178,   127,    88,    90,    97,
     259,     5,     8,   141,   153,   196,   127,   123,   141,   176,
     177,   247,   193,   176,   177,    27,   191,   142,   143,   141,
     224,   225,    25,    26,    47,    61,    62,    66,    75,    76,
     105,   248,   176,     8,    18,   239,   143,     8,     8,   153,
     275,   135,   275,   142,   142,   275,     8,   142,   142,   172,
     176,   177,     9,   141,    78,   138,     8,   176,   177,     8,
     176,   177,   176,   177,   176,   177,   176,   177,   176,   177,
      58,   143,   158,    58,   142,   142,   143,   205,   165,   142,
       5,   182,   122,   185,   186,   142,   141,    34,   115,    88,
     153,   197,   198,   141,     8,   254,   142,   178,    86,   175,
     189,   190,   222,   141,   226,   227,    79,   143,   228,    69,
     153,   240,   227,   238,   142,   142,   142,   142,   142,   124,
     124,     8,   178,   179,   180,    78,     9,   141,     5,    57,
     153,   159,   160,   142,   153,   274,   275,   141,    50,    80,
     162,    27,    53,    56,   184,   178,   127,   142,   143,     8,
     142,   141,    19,    39,   143,   142,   143,     3,   229,   230,
     225,   176,   177,   176,   177,   143,   142,   180,   143,   142,
     201,    27,    69,   163,   175,   183,    80,   174,   187,   142,
     176,   198,   142,   142,   190,   227,   127,   143,     8,   142,
     160,   142,   189,     5,   143,    27,   191,     8,   230,   143,
     143,   175,   189,   193,   141,     5,   201,   142,   142
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

  case 127:

    { pParser->PushQuery(); ;}
    break;

  case 128:

    { pParser->PushQuery(); ;}
    break;

  case 129:

    { pParser->PushQuery(); ;}
    break;

  case 133:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 135:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 138:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 139:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 143:

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

  case 144:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 146:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 147:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 148:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 151:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 154:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 155:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 156:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 157:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 158:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 159:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 160:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 161:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 162:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 163:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 164:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 166:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 173:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 175:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 176:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 177:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_sRefString = pParser->m_pBuf;
		;}
    break;

  case 178:

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

  case 179:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 180:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 181:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 182:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 183:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 184:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 185:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 186:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 187:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true, true ) )
				YYERROR;
		;}
    break;

  case 188:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 189:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 190:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 191:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 192:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 193:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 194:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 195:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 196:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 197:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 198:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 200:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 201:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 202:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 203:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 204:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 210:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 211:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 212:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 213:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 214:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 215:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 216:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 217:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 218:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 224:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 225:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 226:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 228:

    {
			pParser->AddHaving();
		;}
    break;

  case 231:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 234:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 235:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 237:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 239:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 240:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 243:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 244:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 250:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 251:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 252:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 253:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 254:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 255:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 256:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 257:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 259:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 260:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 265:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 266:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 267:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 268:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 269:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 270:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 271:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 272:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 273:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 274:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 275:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 276:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 277:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 278:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 279:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 280:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 281:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 282:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 283:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 284:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 285:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 289:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 290:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 291:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 292:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 293:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 294:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 295:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 296:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 297:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 298:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 299:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 300:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 301:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 302:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 307:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 308:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 315:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 316:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 317:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 318:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 319:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 320:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 321:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 322:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 323:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 324:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 325:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 326:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 327:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 328:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 332:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 333:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 334:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 335:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 336:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 337:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 338:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 339:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 340:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 341:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 342:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 345:

    { yyval.m_iValue = 1; ;}
    break;

  case 346:

    { yyval.m_iValue = 0; ;}
    break;

  case 347:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 355:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 356:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 357:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 360:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 361:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 362:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 367:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 368:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 371:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 372:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 373:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 374:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 375:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 376:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 377:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 378:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 383:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 384:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 385:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 386:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 387:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 389:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 390:

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

  case 391:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 394:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 396:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 401:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 404:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 405:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 406:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 409:

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

  case 410:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 411:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 412:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 413:

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

  case 414:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 415:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 416:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 417:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 418:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 419:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 420:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 421:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 422:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 423:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 424:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 425:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 426:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 427:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 428:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 435:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 436:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 437:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 445:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 446:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 447:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 448:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 449:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 450:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 451:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 452:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 453:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 454:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 455:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 456:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 459:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 460:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 461:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 462:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 463:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 466:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 468:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 469:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 470:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 471:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 472:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 473:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 474:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 478:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 480:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 483:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
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

