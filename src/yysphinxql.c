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
     TOK_RELOAD = 345,
     TOK_REPEATABLE = 346,
     TOK_REPLACE = 347,
     TOK_REMAP = 348,
     TOK_RETURNS = 349,
     TOK_ROLLBACK = 350,
     TOK_RTINDEX = 351,
     TOK_SELECT = 352,
     TOK_SERIALIZABLE = 353,
     TOK_SET = 354,
     TOK_SETTINGS = 355,
     TOK_SESSION = 356,
     TOK_SHOW = 357,
     TOK_SONAME = 358,
     TOK_START = 359,
     TOK_STATUS = 360,
     TOK_STRING = 361,
     TOK_SUM = 362,
     TOK_TABLE = 363,
     TOK_TABLES = 364,
     TOK_THREADS = 365,
     TOK_TO = 366,
     TOK_TRANSACTION = 367,
     TOK_TRUE = 368,
     TOK_TRUNCATE = 369,
     TOK_TYPE = 370,
     TOK_UNCOMMITTED = 371,
     TOK_UPDATE = 372,
     TOK_VALUES = 373,
     TOK_VARIABLES = 374,
     TOK_WARNINGS = 375,
     TOK_WEIGHT = 376,
     TOK_WHERE = 377,
     TOK_WITHIN = 378,
     TOK_OR = 379,
     TOK_AND = 380,
     TOK_NE = 381,
     TOK_GTE = 382,
     TOK_LTE = 383,
     TOK_NOT = 384,
     TOK_NEG = 385
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
#define TOK_RELOAD 345
#define TOK_REPEATABLE 346
#define TOK_REPLACE 347
#define TOK_REMAP 348
#define TOK_RETURNS 349
#define TOK_ROLLBACK 350
#define TOK_RTINDEX 351
#define TOK_SELECT 352
#define TOK_SERIALIZABLE 353
#define TOK_SET 354
#define TOK_SETTINGS 355
#define TOK_SESSION 356
#define TOK_SHOW 357
#define TOK_SONAME 358
#define TOK_START 359
#define TOK_STATUS 360
#define TOK_STRING 361
#define TOK_SUM 362
#define TOK_TABLE 363
#define TOK_TABLES 364
#define TOK_THREADS 365
#define TOK_TO 366
#define TOK_TRANSACTION 367
#define TOK_TRUE 368
#define TOK_TRUNCATE 369
#define TOK_TYPE 370
#define TOK_UNCOMMITTED 371
#define TOK_UPDATE 372
#define TOK_VALUES 373
#define TOK_VARIABLES 374
#define TOK_WARNINGS 375
#define TOK_WEIGHT 376
#define TOK_WHERE 377
#define TOK_WITHIN 378
#define TOK_OR 379
#define TOK_AND 380
#define TOK_NE 381
#define TOK_GTE 382
#define TOK_LTE 383
#define TOK_NOT 384
#define TOK_NEG 385




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
#define YYFINAL  201
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4634

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  150
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  141
/* YYNRULES -- Number of rules. */
#define YYNRULES  448
/* YYNRULES -- Number of states. */
#define YYNSTATES  812

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   385

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   138,   127,     2,
     142,   143,   136,   134,   144,   135,   147,   137,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   141,
     130,   128,   131,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   148,     2,   149,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   145,   126,   146,     2,     2,     2,     2,
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
     125,   129,   132,   133,   139,   140
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
     160,   162,   164,   166,   168,   170,   172,   174,   178,   181,
     183,   185,   187,   196,   198,   208,   209,   212,   214,   218,
     220,   222,   224,   225,   229,   230,   233,   238,   250,   252,
     256,   258,   261,   262,   264,   267,   269,   274,   279,   284,
     289,   294,   299,   303,   309,   311,   315,   316,   318,   321,
     323,   327,   331,   336,   338,   342,   346,   352,   359,   363,
     368,   374,   378,   382,   386,   390,   394,   396,   402,   408,
     414,   418,   422,   426,   430,   434,   438,   442,   447,   451,
     453,   455,   460,   464,   468,   470,   472,   477,   482,   487,
     491,   493,   496,   498,   501,   503,   505,   509,   511,   515,
     517,   519,   520,   525,   526,   528,   530,   534,   535,   538,
     539,   541,   547,   548,   550,   554,   560,   562,   566,   568,
     571,   574,   575,   577,   580,   585,   586,   588,   591,   593,
     597,   599,   603,   607,   613,   620,   624,   626,   630,   634,
     636,   638,   640,   642,   644,   646,   648,   651,   654,   658,
     662,   666,   670,   674,   678,   682,   686,   690,   694,   698,
     702,   706,   710,   714,   718,   722,   726,   730,   732,   734,
     736,   740,   745,   750,   755,   760,   765,   770,   775,   779,
     786,   793,   797,   806,   821,   825,   830,   832,   836,   838,
     840,   844,   850,   852,   854,   856,   858,   861,   862,   865,
     867,   870,   873,   877,   879,   881,   883,   886,   891,   896,
     900,   905,   910,   912,   914,   915,   918,   923,   928,   933,
     937,   942,   947,   955,   961,   967,   977,   979,   981,   983,
     985,   987,   989,   993,   995,   997,   999,  1001,  1003,  1005,
    1007,  1009,  1011,  1014,  1022,  1024,  1026,  1027,  1031,  1033,
    1035,  1037,  1041,  1043,  1047,  1051,  1053,  1057,  1059,  1061,
    1063,  1067,  1070,  1071,  1074,  1076,  1080,  1084,  1089,  1096,
    1098,  1102,  1104,  1108,  1110,  1114,  1115,  1118,  1120,  1124,
    1128,  1129,  1131,  1133,  1135,  1139,  1141,  1143,  1147,  1151,
    1158,  1160,  1164,  1168,  1172,  1178,  1183,  1187,  1191,  1193,
    1195,  1197,  1199,  1201,  1203,  1205,  1207,  1215,  1222,  1227,
    1232,  1238,  1239,  1241,  1244,  1246,  1250,  1254,  1257,  1261,
    1268,  1269,  1271,  1273,  1276,  1279,  1282,  1284,  1292,  1294,
    1296,  1298,  1300,  1304,  1311,  1315,  1319,  1322,  1326,  1328,
    1332,  1335,  1339,  1343,  1351,  1357,  1363,  1365,  1367,  1370,
    1372,  1375,  1377,  1379,  1383,  1387,  1391,  1395,  1397,  1398,
    1401,  1403,  1406,  1408,  1410,  1414,  1420,  1421,  1424
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     151,     0,    -1,   152,    -1,   155,    -1,   155,   141,    -1,
     221,    -1,   233,    -1,   213,    -1,   214,    -1,   219,    -1,
     234,    -1,   243,    -1,   245,    -1,   246,    -1,   247,    -1,
     252,    -1,   257,    -1,   258,    -1,   262,    -1,   264,    -1,
     265,    -1,   266,    -1,   267,    -1,   268,    -1,   259,    -1,
     269,    -1,   271,    -1,   272,    -1,   273,    -1,   251,    -1,
     274,    -1,   275,    -1,   276,    -1,   290,    -1,     3,    -1,
      16,    -1,    20,    -1,    21,    -1,    22,    -1,    23,    -1,
      26,    -1,    31,    -1,    35,    -1,    48,    -1,    51,    -1,
      52,    -1,    53,    -1,    54,    -1,    55,    -1,    65,    -1,
      67,    -1,    68,    -1,    70,    -1,    71,    -1,    72,    -1,
      82,    -1,    83,    -1,    84,    -1,    85,    -1,    87,    -1,
      86,    -1,    88,    -1,    90,    -1,    91,    -1,    94,    -1,
      95,    -1,    96,    -1,    98,    -1,   101,    -1,   104,    -1,
     105,    -1,   106,    -1,   107,    -1,   109,    -1,   110,    -1,
     114,    -1,   115,    -1,   116,    -1,   119,    -1,   120,    -1,
     121,    -1,   123,    -1,    66,    -1,   153,    -1,    77,    -1,
     112,    -1,   156,    -1,   155,   141,   156,    -1,   155,   288,
      -1,   157,    -1,   208,    -1,   158,    -1,    97,     3,   142,
     142,   158,   143,   159,   143,    -1,   165,    -1,    97,   166,
      50,   142,   162,   165,   143,   163,   164,    -1,    -1,   144,
     160,    -1,   161,    -1,   160,   144,   161,    -1,   154,    -1,
       5,    -1,    57,    -1,    -1,    80,    27,   191,    -1,    -1,
      69,     5,    -1,    69,     5,   144,     5,    -1,    97,   166,
      50,   170,   171,   183,   187,   186,   189,   193,   195,    -1,
     167,    -1,   166,   144,   167,    -1,   136,    -1,   169,   168,
      -1,    -1,   154,    -1,    18,   154,    -1,   201,    -1,    22,
     142,   201,   143,    -1,    71,   142,   201,   143,    -1,    73,
     142,   201,   143,    -1,   107,   142,   201,   143,    -1,    55,
     142,   201,   143,    -1,    35,   142,   136,   143,    -1,    54,
     142,   143,    -1,    35,   142,    41,   154,   143,    -1,   154,
      -1,   170,   144,   154,    -1,    -1,   172,    -1,   122,   173,
      -1,   174,    -1,   173,   125,   173,    -1,   142,   173,   143,
      -1,    70,   142,     8,   143,    -1,   175,    -1,   177,   128,
     178,    -1,   177,   129,   178,    -1,   177,    58,   142,   182,
     143,    -1,   177,   139,    58,   142,   182,   143,    -1,   177,
      58,     9,    -1,   177,   139,    58,     9,    -1,   177,    24,
     178,   125,   178,    -1,   177,   131,   178,    -1,   177,   130,
     178,    -1,   177,   132,   178,    -1,   177,   133,   178,    -1,
     177,   128,   179,    -1,   176,    -1,   177,    24,   179,   125,
     179,    -1,   177,    24,   178,   125,   179,    -1,   177,    24,
     179,   125,   178,    -1,   177,   131,   179,    -1,   177,   130,
     179,    -1,   177,   132,   179,    -1,   177,   133,   179,    -1,
     177,   128,     8,    -1,   177,   129,     8,    -1,   177,    64,
      78,    -1,   177,    64,   139,    78,    -1,   177,   129,   179,
      -1,   154,    -1,     4,    -1,    35,   142,   136,   143,    -1,
      54,   142,   143,    -1,   121,   142,   143,    -1,    57,    -1,
     278,    -1,    62,   142,   278,   143,    -1,    43,   142,   278,
     143,    -1,    25,   142,   278,   143,    -1,    45,   142,   143,
      -1,     5,    -1,   135,     5,    -1,     6,    -1,   135,     6,
      -1,    14,    -1,   178,    -1,   180,   144,   178,    -1,     8,
      -1,   181,   144,     8,    -1,   180,    -1,   181,    -1,    -1,
      53,   184,    27,   185,    -1,    -1,     5,    -1,   177,    -1,
     185,   144,   177,    -1,    -1,    56,   175,    -1,    -1,   188,
      -1,   123,    53,    80,    27,   191,    -1,    -1,   190,    -1,
      80,    27,   191,    -1,    80,    27,    86,   142,   143,    -1,
     192,    -1,   191,   144,   192,    -1,   177,    -1,   177,    19,
      -1,   177,    39,    -1,    -1,   194,    -1,    69,     5,    -1,
      69,     5,   144,     5,    -1,    -1,   196,    -1,    79,   197,
      -1,   198,    -1,   197,   144,   198,    -1,   154,    -1,   154,
     128,   154,    -1,   154,   128,     5,    -1,   154,   128,   142,
     199,   143,    -1,   154,   128,   154,   142,     8,   143,    -1,
     154,   128,     8,    -1,   200,    -1,   199,   144,   200,    -1,
     154,   128,   178,    -1,   154,    -1,     4,    -1,    57,    -1,
       5,    -1,     6,    -1,    14,    -1,     9,    -1,   135,   201,
      -1,   139,   201,    -1,   201,   134,   201,    -1,   201,   135,
     201,    -1,   201,   136,   201,    -1,   201,   137,   201,    -1,
     201,   130,   201,    -1,   201,   131,   201,    -1,   201,   127,
     201,    -1,   201,   126,   201,    -1,   201,   138,   201,    -1,
     201,    42,   201,    -1,   201,    74,   201,    -1,   201,   133,
     201,    -1,   201,   132,   201,    -1,   201,   128,   201,    -1,
     201,   129,   201,    -1,   201,   125,   201,    -1,   201,   124,
     201,    -1,   142,   201,   143,    -1,   145,   205,   146,    -1,
     202,    -1,   278,    -1,   281,    -1,   277,    64,    78,    -1,
     277,    64,   139,    78,    -1,     3,   142,   203,   143,    -1,
      58,   142,   203,   143,    -1,    62,   142,   203,   143,    -1,
      25,   142,   203,   143,    -1,    47,   142,   203,   143,    -1,
      43,   142,   203,   143,    -1,     3,   142,   143,    -1,    73,
     142,   201,   144,   201,   143,    -1,    71,   142,   201,   144,
     201,   143,    -1,   121,   142,   143,    -1,     3,   142,   201,
      49,   154,    58,   277,   143,    -1,    93,   142,   201,   144,
     201,   144,   142,   203,   143,   144,   142,   203,   143,   143,
      -1,    86,   142,   143,    -1,    86,   142,   203,   143,    -1,
     204,    -1,   203,   144,   204,    -1,   201,    -1,     8,    -1,
     206,   128,   207,    -1,   205,   144,   206,   128,   207,    -1,
     154,    -1,    58,    -1,   178,    -1,   154,    -1,   102,   210,
      -1,    -1,    68,     8,    -1,   120,    -1,   105,   209,    -1,
      72,   209,    -1,    16,   105,   209,    -1,    85,    -1,    82,
      -1,    84,    -1,   110,   195,    -1,    16,     8,   105,   209,
      -1,    16,   154,   105,   209,    -1,   211,   154,   105,    -1,
     211,   154,   212,   100,    -1,   211,   154,    14,   100,    -1,
      59,    -1,   108,    -1,    -1,    30,     5,    -1,    99,   153,
     128,   216,    -1,    99,   153,   128,   215,    -1,    99,   153,
     128,    78,    -1,    99,    77,   217,    -1,    99,    10,   128,
     217,    -1,    99,    29,    99,   217,    -1,    99,    52,     9,
     128,   142,   180,   143,    -1,    99,    52,   153,   128,   215,
      -1,    99,    52,   153,   128,     5,    -1,    99,    59,   154,
      52,     9,   128,   142,   180,   143,    -1,   154,    -1,     8,
      -1,   113,    -1,    46,    -1,   178,    -1,   218,    -1,   217,
     135,   218,    -1,   154,    -1,    78,    -1,     8,    -1,     5,
      -1,     6,    -1,    33,    -1,    95,    -1,   220,    -1,    23,
      -1,   104,   112,    -1,   222,    63,   154,   223,   118,   226,
     230,    -1,    60,    -1,    92,    -1,    -1,   142,   225,   143,
      -1,   154,    -1,    57,    -1,   224,    -1,   225,   144,   224,
      -1,   227,    -1,   226,   144,   227,    -1,   142,   228,   143,
      -1,   229,    -1,   228,   144,   229,    -1,   178,    -1,   179,
      -1,     8,    -1,   142,   180,   143,    -1,   142,   143,    -1,
      -1,    79,   231,    -1,   232,    -1,   231,   144,   232,    -1,
       3,   128,     8,    -1,    38,    50,   170,   172,    -1,    28,
     154,   142,   235,   238,   143,    -1,   236,    -1,   235,   144,
     236,    -1,   229,    -1,   142,   237,   143,    -1,     8,    -1,
     237,   144,     8,    -1,    -1,   144,   239,    -1,   240,    -1,
     239,   144,   240,    -1,   229,   241,   242,    -1,    -1,    18,
      -1,   154,    -1,    69,    -1,   244,   154,   209,    -1,    40,
      -1,    39,    -1,   102,   109,   209,    -1,   102,    37,   209,
      -1,   117,   170,    99,   248,   172,   195,    -1,   249,    -1,
     248,   144,   249,    -1,   154,   128,   178,    -1,   154,   128,
     179,    -1,   154,   128,   142,   180,   143,    -1,   154,   128,
     142,   143,    -1,   278,   128,   178,    -1,   278,   128,   179,
      -1,    62,    -1,    25,    -1,    47,    -1,    26,    -1,    75,
      -1,    76,    -1,    66,    -1,   106,    -1,    17,   108,   154,
      15,    32,   154,   250,    -1,    17,   108,   154,    44,    32,
     154,    -1,    17,    96,   154,    89,    -1,   102,   260,   119,
     253,    -1,   102,   260,   119,    68,     8,    -1,    -1,   254,
      -1,   122,   255,    -1,   256,    -1,   255,   124,   256,    -1,
     154,   128,     8,    -1,   102,    31,    -1,   102,    29,    99,
      -1,    99,   260,   112,    65,    67,   261,    -1,    -1,    52,
      -1,   101,    -1,    88,   116,    -1,    88,    34,    -1,    91,
      88,    -1,    98,    -1,    36,    51,   154,    94,   263,   103,
       8,    -1,    61,    -1,    25,    -1,    47,    -1,   106,    -1,
      44,    51,   154,    -1,    20,    59,   154,   111,    96,   154,
      -1,    48,    96,   154,    -1,    48,    87,   154,    -1,    48,
      21,    -1,    97,   270,   193,    -1,    10,    -1,    10,   147,
     154,    -1,    97,   201,    -1,   114,    96,   154,    -1,    81,
      59,   154,    -1,    36,    83,   154,   115,     8,   103,     8,
      -1,    44,    83,   154,   115,     8,    -1,    90,    84,    50,
     103,     8,    -1,   278,    -1,   154,    -1,   154,   279,    -1,
     280,    -1,   279,   280,    -1,    13,    -1,    14,    -1,   148,
     201,   149,    -1,   148,     8,   149,    -1,   201,   128,   282,
      -1,   282,   128,   201,    -1,     8,    -1,    -1,   284,   287,
      -1,    27,    -1,   286,   168,    -1,   201,    -1,   285,    -1,
     287,   144,   285,    -1,    45,   287,   283,   189,   193,    -1,
      -1,    50,     8,    -1,    90,    59,   154,   289,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   176,   176,   177,   178,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   221,   222,   222,   222,   222,   222,
     222,   223,   223,   223,   223,   224,   224,   224,   224,   224,
     225,   225,   225,   225,   225,   226,   226,   226,   226,   226,
     226,   226,   227,   227,   227,   227,   227,   228,   228,   228,
     228,   228,   229,   229,   229,   229,   229,   229,   230,   230,
     230,   230,   231,   235,   235,   235,   241,   242,   243,   247,
     248,   252,   253,   261,   262,   269,   271,   275,   279,   286,
     287,   288,   292,   305,   312,   314,   319,   328,   344,   345,
     349,   350,   353,   355,   356,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   372,   373,   376,   378,   382,   386,
     387,   388,   392,   397,   401,   408,   416,   425,   435,   440,
     445,   450,   455,   460,   465,   470,   475,   480,   485,   490,
     495,   500,   505,   510,   515,   520,   525,   530,   538,   542,
     543,   548,   554,   560,   566,   572,   573,   574,   575,   576,
     580,   581,   592,   593,   594,   598,   604,   611,   617,   624,
     625,   628,   630,   633,   635,   642,   646,   652,   654,   660,
     662,   666,   677,   679,   683,   687,   694,   695,   699,   700,
     701,   704,   706,   710,   715,   722,   724,   728,   732,   733,
     737,   742,   747,   752,   758,   763,   771,   776,   783,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   803,   804,
     805,   806,   807,   808,   809,   810,   811,   812,   813,   814,
     815,   816,   817,   818,   819,   820,   821,   822,   823,   824,
     825,   826,   830,   831,   832,   833,   834,   835,   836,   837,
     838,   839,   840,   841,   842,   843,   847,   848,   852,   853,
     857,   858,   862,   863,   867,   868,   874,   877,   879,   883,
     884,   885,   886,   887,   888,   889,   890,   891,   896,   901,
     906,   911,   920,   921,   924,   926,   934,   939,   944,   949,
     950,   951,   955,   960,   965,   970,   979,   980,   984,   985,
     986,   998,   999,  1003,  1004,  1005,  1006,  1007,  1014,  1015,
    1016,  1020,  1021,  1027,  1035,  1036,  1039,  1041,  1045,  1046,
    1050,  1051,  1055,  1056,  1060,  1064,  1065,  1069,  1070,  1071,
    1072,  1073,  1076,  1077,  1081,  1082,  1086,  1092,  1102,  1110,
    1114,  1121,  1122,  1129,  1139,  1145,  1147,  1151,  1156,  1160,
    1167,  1169,  1173,  1174,  1180,  1188,  1189,  1195,  1199,  1205,
    1213,  1214,  1218,  1232,  1238,  1242,  1247,  1261,  1272,  1273,
    1274,  1275,  1276,  1277,  1278,  1279,  1283,  1291,  1298,  1309,
    1313,  1320,  1321,  1325,  1329,  1330,  1334,  1338,  1345,  1352,
    1358,  1359,  1360,  1364,  1365,  1366,  1367,  1373,  1384,  1385,
    1386,  1387,  1392,  1403,  1415,  1424,  1433,  1443,  1451,  1452,
    1456,  1466,  1477,  1488,  1499,  1509,  1520,  1521,  1525,  1528,
    1529,  1533,  1534,  1535,  1536,  1540,  1541,  1545,  1550,  1552,
    1556,  1565,  1569,  1577,  1578,  1582,  1593,  1595,  1602
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
  "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", "TOK_RECONFIGURE", "TOK_RELOAD", 
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
  "const_int", "const_float", "const_list", "string_list", 
  "const_list_or_string_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_having_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "hash_key", "hash_val", "show_stmt", "like_filter", 
  "show_what", "index_or_table", "opt_chunk", "set_stmt", 
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
     375,   376,   377,   378,   379,   380,   124,    38,    61,   381,
      60,    62,   382,   383,    43,    45,    42,    47,    37,   384,
     385,    59,    40,    41,    44,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   150,   151,   151,   151,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   154,   154,   154,   155,   155,   155,   156,
     156,   157,   157,   158,   158,   159,   159,   160,   160,   161,
     161,   161,   162,   163,   164,   164,   164,   165,   166,   166,
     167,   167,   168,   168,   168,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   170,   170,   171,   171,   172,   173,
     173,   173,   174,   174,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   176,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     178,   178,   179,   179,   179,   180,   180,   181,   181,   182,
     182,   183,   183,   184,   184,   185,   185,   186,   186,   187,
     187,   188,   189,   189,   190,   190,   191,   191,   192,   192,
     192,   193,   193,   194,   194,   195,   195,   196,   197,   197,
     198,   198,   198,   198,   198,   198,   199,   199,   200,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   201,   202,   202,   202,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   203,   203,   204,   204,
     205,   205,   206,   206,   207,   207,   208,   209,   209,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   211,   211,   212,   212,   213,   213,   213,   213,
     213,   213,   214,   214,   214,   214,   215,   215,   216,   216,
     216,   217,   217,   218,   218,   218,   218,   218,   219,   219,
     219,   220,   220,   221,   222,   222,   223,   223,   224,   224,
     225,   225,   226,   226,   227,   228,   228,   229,   229,   229,
     229,   229,   230,   230,   231,   231,   232,   233,   234,   235,
     235,   236,   236,   237,   237,   238,   238,   239,   239,   240,
     241,   241,   242,   242,   243,   244,   244,   245,   246,   247,
     248,   248,   249,   249,   249,   249,   249,   249,   250,   250,
     250,   250,   250,   250,   250,   250,   251,   251,   251,   252,
     252,   253,   253,   254,   255,   255,   256,   257,   258,   259,
     260,   260,   260,   261,   261,   261,   261,   262,   263,   263,
     263,   263,   264,   265,   266,   267,   268,   269,   270,   270,
     271,   272,   273,   274,   275,   276,   277,   277,   278,   279,
     279,   280,   280,   280,   280,   281,   281,   282,   283,   283,
     284,   285,   286,   287,   287,   288,   289,   289,   290
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
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     8,     1,     9,     0,     2,     1,     3,     1,
       1,     1,     0,     3,     0,     2,     4,    11,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     4,     3,     5,     1,     3,     0,     1,     2,     1,
       3,     3,     4,     1,     3,     3,     5,     6,     3,     4,
       5,     3,     3,     3,     3,     3,     1,     5,     5,     5,
       3,     3,     3,     3,     3,     3,     3,     4,     3,     1,
       1,     4,     3,     3,     1,     1,     4,     4,     4,     3,
       1,     2,     1,     2,     1,     1,     3,     1,     3,     1,
       1,     0,     4,     0,     1,     1,     3,     0,     2,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       1,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       3,     4,     4,     4,     4,     4,     4,     4,     3,     6,
       6,     3,     8,    14,     3,     4,     1,     3,     1,     1,
       3,     5,     1,     1,     1,     1,     2,     0,     2,     1,
       2,     2,     3,     1,     1,     1,     2,     4,     4,     3,
       4,     4,     1,     1,     0,     2,     4,     4,     4,     3,
       4,     4,     7,     5,     5,     9,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     7,     1,     1,     0,     3,     1,     1,
       1,     3,     1,     3,     3,     1,     3,     1,     1,     1,
       3,     2,     0,     2,     1,     3,     3,     4,     6,     1,
       3,     1,     3,     1,     3,     0,     2,     1,     3,     3,
       0,     1,     1,     1,     3,     1,     1,     3,     3,     6,
       1,     3,     3,     3,     5,     4,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     7,     6,     4,     4,
       5,     0,     1,     2,     1,     3,     3,     2,     3,     6,
       0,     1,     1,     2,     2,     2,     1,     7,     1,     1,
       1,     1,     3,     6,     3,     3,     2,     3,     1,     3,
       2,     3,     3,     7,     5,     5,     1,     1,     2,     1,
       2,     1,     1,     3,     3,     3,     3,     1,     0,     2,
       1,     2,     1,     1,     3,     5,     0,     2,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   321,     0,   318,     0,     0,   366,   365,
       0,     0,   324,     0,     0,   325,   319,     0,   400,   400,
       0,     0,     0,     0,     2,     3,    86,    89,    91,    93,
      90,     7,     8,     9,   320,     5,     0,     6,    10,    11,
       0,    12,    13,    14,    29,    15,    16,    17,    24,    18,
      19,    20,    21,    22,    23,    25,    26,    27,    28,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    82,    50,    51,    52,    53,    54,    84,    55,
      56,    57,    58,    60,    59,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    85,
      75,    76,    77,    78,    79,    80,    81,    83,     0,     0,
       0,     0,     0,     0,   416,     0,     0,     0,     0,     0,
      34,   220,   222,   223,   437,   225,   418,   224,    38,     0,
      42,     0,     0,    47,    48,   221,     0,     0,    53,     0,
      60,     0,    72,    80,     0,   110,     0,     0,     0,   219,
       0,   108,   112,   115,   247,   201,     0,   248,   249,     0,
       0,     0,    45,     0,     0,    68,     0,     0,     0,     0,
     397,   277,   401,   292,   277,   284,   285,   283,   402,   277,
     293,   277,   205,   279,   276,     0,     0,   322,     0,   124,
       0,     1,     0,     4,    88,     0,   277,     0,     0,     0,
       0,     0,     0,     0,   412,     0,   415,   414,   422,   446,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    34,    53,
       0,   226,   227,     0,   273,   272,     0,     0,   431,   432,
       0,   428,   429,     0,     0,     0,   113,   111,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   417,   202,     0,     0,
       0,     0,     0,     0,     0,   316,   317,   315,   314,   313,
     299,   311,     0,     0,     0,   277,     0,   398,     0,   368,
     281,   280,   367,     0,   286,   206,   294,   391,   421,     0,
       0,   442,   443,   112,   438,     0,     0,    87,   326,   364,
     388,     0,     0,     0,   170,   172,   339,   174,     0,     0,
     337,   338,   351,   355,   349,     0,     0,     0,   347,     0,
       0,   448,     0,   269,     0,   258,   268,     0,   266,   419,
       0,   268,     0,     0,     0,     0,     0,   122,     0,     0,
       0,     0,     0,   264,     0,     0,     0,   261,     0,     0,
       0,   245,     0,   246,     0,   437,     0,   430,   102,   126,
     109,   115,   114,   237,   238,   244,   243,   235,   234,   241,
     435,   242,   232,   233,   240,   239,   228,   229,   230,   231,
     236,   203,   250,     0,   436,   300,   301,     0,     0,     0,
       0,   307,   309,   298,   308,     0,   306,   310,   297,   296,
       0,   277,   282,   277,   278,   210,   207,   208,     0,     0,
     289,     0,     0,     0,   389,   392,     0,     0,   370,     0,
     125,   441,   440,     0,   192,     0,     0,     0,     0,     0,
       0,   171,   173,   353,   341,   175,     0,     0,     0,     0,
     409,   410,   408,   411,     0,     0,   160,     0,    42,     0,
       0,    47,   164,     0,    52,    80,     0,   159,   128,   129,
     133,   146,     0,   165,   424,   447,   425,     0,     0,     0,
     252,     0,   116,   255,     0,   121,   257,   256,   120,   253,
     254,   117,     0,   118,     0,   265,     0,   119,     0,     0,
       0,   275,   274,   270,   434,   433,     0,   181,   127,     0,
     251,     0,   304,   303,     0,   312,     0,   287,   288,     0,
       0,   291,   295,   290,   390,     0,   393,   394,     0,     0,
     205,     0,   444,     0,   201,   193,   439,   329,   328,   330,
       0,     0,     0,   387,   413,   340,     0,   352,     0,   360,
     350,   356,   357,   348,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    95,     0,   267,   123,
       0,     0,     0,     0,     0,     0,   183,   189,   204,     0,
       0,     0,     0,   406,   399,   212,   215,     0,   211,   209,
       0,     0,     0,   372,   373,   371,   369,   376,   377,     0,
     445,   327,     0,     0,   342,   332,   379,   381,   380,   378,
     384,   382,   383,   385,   386,   176,   354,   361,     0,     0,
     407,   423,     0,     0,     0,     0,   169,   162,     0,     0,
     163,   131,   130,     0,     0,   138,     0,   156,     0,   154,
     134,   145,   155,   135,   158,   142,   151,   141,   150,   143,
     152,   144,   153,     0,     0,     0,     0,   260,   259,     0,
     271,     0,     0,   184,     0,     0,   187,   190,   302,     0,
     404,   403,   405,     0,     0,   216,     0,   396,   395,   375,
       0,    60,   198,   194,   196,   331,     0,     0,   335,     0,
       0,   323,   363,   362,   359,   360,   358,   168,   161,   167,
     166,   132,     0,     0,   177,   179,   180,     0,   157,   139,
       0,   100,   101,    99,    96,    97,    92,   427,     0,   426,
       0,     0,     0,   104,     0,     0,     0,   192,     0,     0,
     213,     0,     0,   374,     0,   199,   200,     0,   334,     0,
       0,   343,   344,   333,   140,   148,   149,   147,     0,   136,
       0,     0,   262,     0,     0,     0,    94,   185,   182,     0,
     188,   201,   305,   218,   217,   214,   195,   197,   336,     0,
       0,   178,   137,    98,     0,   103,   105,     0,     0,   205,
     346,   345,     0,     0,   186,   191,   107,     0,   106,     0,
       0,   263
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    23,    24,   117,   159,    25,    26,    27,    28,   675,
     734,   735,   516,   743,   776,    29,   160,   161,   257,   162,
     379,   517,   338,   478,   479,   480,   481,   482,   455,   331,
     456,   726,   727,   597,   684,   778,   747,   686,   687,   544,
     545,   703,   704,   276,   277,   304,   305,   426,   427,   694,
     695,   351,   164,   347,   348,   246,   247,   513,    30,   299,
     194,   195,   431,    31,    32,   418,   419,   290,   291,    33,
      34,    35,    36,   447,   549,   550,   624,   625,   707,   332,
     711,   761,   762,    37,    38,   333,   334,   457,   459,   561,
     562,   638,   714,    39,    40,    41,    42,    43,   437,   438,
     634,    44,    45,   434,   435,   536,   537,    46,    47,    48,
     177,   604,    49,   464,    50,    51,    52,    53,    54,    55,
     165,    56,    57,    58,    59,    60,    61,   166,   167,   251,
     252,   168,   169,   444,   445,   312,   313,   314,   204,   341,
      62
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -692
static const short yypact[] =
{
    4452,    -2,    33,  -692,  3857,  -692,    31,    91,  -692,  -692,
      47,   119,  -692,   191,    20,  -692,  -692,   752,  2980,   468,
      -9,   159,  3857,   162,  -692,     7,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,   147,  -692,  -692,  -692,
    3857,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  3857,  3857,  3857,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,   116,  3857,
    3857,  3857,  3857,  3857,  -692,  3857,  3857,  3857,  3857,   212,
     121,  -692,  -692,  -692,  -692,  -692,   120,  -692,   145,   149,
     158,   160,   166,   168,   170,  -692,   190,   192,   196,   197,
     206,   211,   214,   215,  1753,  -692,  1753,  1753,  3315,    12,
      -7,  -692,  3423,    97,  -692,   235,   219,   294,  -692,   231,
     232,   263,  3965,  3857,  2872,   253,   239,   257,  3533,   274,
    -692,   306,  -692,  -692,   306,  -692,  -692,  -692,  -692,   306,
    -692,   306,   297,  -692,  -692,  3857,   258,  -692,  3857,  -692,
     -36,  -692,  1753,   103,  -692,  3857,   306,   295,    69,   278,
      26,   291,   276,   -34,  -692,   280,  -692,  -692,  -692,   346,
     299,   895,  3857,  1753,  1896,    48,  1896,  1896,   262,  1753,
    1896,  1896,  1753,  1753,  1038,  1753,  1753,   264,   266,   268,
     271,  -692,  -692,  4165,  -692,  -692,    53,   272,  -692,  -692,
    2039,    15,  -692,  2274,  1181,  3857,  -692,  -692,  1753,  1753,
    1753,  1753,  1753,  1753,  1753,  1753,  1753,  1753,  1753,  1753,
    1753,  1753,  1753,  1753,  1753,   409,  -692,  -692,    54,  1753,
    2872,  2872,   287,   289,   364,  -692,  -692,  -692,  -692,  -692,
     283,  -692,  2397,   355,   316,    22,   318,  -692,   416,  -692,
    -692,  -692,  -692,  3857,  -692,  -692,    98,   -11,  -692,  3857,
    3857,  4469,  -692,  3423,     6,  1324,   327,  -692,   286,  -692,
    -692,   398,   399,   337,  -692,  -692,  -692,  -692,    90,     8,
    -692,  -692,  -692,   290,  -692,   195,   428,  2166,  -692,   431,
     432,  -692,   435,   340,  1467,  -692,  4453,   -44,  -692,  -692,
    4198,  4469,   -10,  3857,   326,    73,    94,  -692,  4231,   101,
     104,  3966,  3999,  -692,   117,  4034,  4264,  -692,  1610,  1753,
    1753,  -692,  3315,  -692,  2518,   321,   324,  -692,  -692,   -34,
    -692,  4469,  -692,  -692,  -692,  4483,  4496,  3442,  3983,   345,
    -692,   345,   329,   329,   329,   329,    51,    51,  -692,  -692,
    -692,   330,  -692,   393,   345,   283,   283,   344,  3099,   478,
    2872,  -692,  -692,  -692,  -692,   483,  -692,  -692,  -692,  -692,
     422,   306,  -692,   306,  -692,   362,   347,  -692,   392,   488,
    -692,   394,   487,  3857,  -692,  -692,    28,    68,  -692,   368,
    -692,  -692,  -692,  1753,   421,  1753,  3641,   385,  3857,  3857,
    3857,  -692,  -692,  -692,  -692,  -692,   122,   131,    26,   361,
    -692,  -692,  -692,  -692,   403,   404,  -692,   366,   367,   369,
     370,   371,  -692,   372,   373,   374,  2166,    15,   405,  -692,
    -692,  -692,   140,  -692,  -692,  -692,  -692,  1181,   375,  3857,
    -692,  1896,  -692,  -692,   376,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  1753,  -692,  1753,  -692,  1753,  -692,  4064,  4099,
     382,  -692,  -692,  -692,  -692,  -692,   426,   471,  -692,   523,
    -692,    18,  -692,  -692,   401,  -692,   148,  -692,  -692,   609,
    3857,  -692,  -692,  -692,  -692,   406,   407,  -692,    44,  3857,
     297,    50,  -692,   505,   235,  -692,   389,  -692,  -692,  -692,
     138,   395,   288,  -692,  -692,  -692,    18,  -692,   528,     3,
    -692,   397,  -692,  -692,   530,   531,  3857,   410,  3857,   400,
     402,  3857,   534,   408,   -23,  2166,    50,    35,   114,    67,
      72,    50,    50,    50,    50,   489,   411,   491,  -692,  -692,
    4297,  4330,  4132,  2518,  1181,   413,   549,   434,  -692,   141,
     418,    19,   477,  -692,  -692,  -692,  -692,  3857,   430,  -692,
     562,  3857,     9,  -692,  -692,  -692,  -692,  -692,  -692,  2639,
    -692,  -692,  3641,    40,    -8,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  3749,    40,
    -692,  -692,    15,   437,   438,   439,  -692,  -692,   440,   442,
    -692,  -692,  -692,   449,   450,  -692,    30,  -692,   508,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,    38,  3207,   444,  3857,  -692,  -692,   448,
    -692,    37,   511,  -692,   565,   540,   538,  -692,  -692,    18,
    -692,  -692,  -692,   467,   151,  -692,   589,  -692,  -692,  -692,
     153,   456,    66,   455,  -692,  -692,    14,   198,  -692,   597,
     395,  -692,  -692,  -692,  -692,   583,  -692,  -692,  -692,  -692,
    -692,  -692,    50,    50,  -692,   458,   460,   462,  -692,  -692,
      30,  -692,  -692,  -692,   464,  -692,  -692,    15,   463,  -692,
    1896,  3857,   582,   541,  2753,   533,  2753,   421,   201,    18,
    -692,  3857,   473,  -692,   479,  -692,  -692,  2753,  -692,    40,
     493,   480,  -692,  -692,  -692,  -692,  -692,  -692,   603,  -692,
     484,  3207,  -692,   203,  2753,   618,  -692,  -692,   482,   601,
    -692,   235,  -692,  -692,  -692,  -692,  -692,  -692,  -692,   625,
     597,  -692,  -692,  -692,   492,   455,   494,  2753,  2753,   297,
    -692,  -692,   495,   634,  -692,   455,  -692,  1896,  -692,   208,
     498,  -692
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -692,  -692,  -692,    -6,    -4,  -692,   443,  -692,   298,  -692,
    -692,  -128,  -692,  -692,  -692,   129,    55,   396,   334,  -692,
      17,  -692,  -328,  -446,  -692,   -98,  -692,  -602,  -201,  -514,
    -511,  -692,   -78,  -692,  -692,  -692,  -692,  -692,  -692,   -94,
    -692,  -691,  -103,  -538,  -692,  -535,  -692,  -692,   125,  -692,
     -95,    57,  -692,  -223,   167,  -692,   293,    74,  -692,  -169,
    -692,  -692,  -692,  -692,  -692,   251,  -692,   -40,   256,  -692,
    -692,  -692,  -692,  -692,    46,  -692,  -692,   -41,  -692,  -456,
    -692,  -692,  -119,  -692,  -692,  -692,   220,  -692,  -692,  -692,
      34,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,   143,
    -692,  -692,  -692,  -692,  -692,  -692,    76,  -692,  -692,  -692,
     664,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,  -692,
    -692,  -692,  -692,  -692,  -692,  -692,  -692,    13,  -178,  -692,
     433,  -692,   424,  -692,  -692,   242,  -692,   245,  -692,  -692,
    -692
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -438
static const short yytable[] =
{
     118,   352,   559,   355,   356,   616,   620,   359,   360,   330,
     599,   364,   176,   324,   324,   300,   453,   702,   199,   324,
     301,   637,   302,   324,   614,   248,   249,   618,   248,   249,
     574,   324,   325,   442,   326,   324,   206,   319,   724,   200,
     327,   248,   249,   253,   655,   324,   325,   729,   326,   324,
     325,   518,   202,   690,   327,   324,   325,   432,   327,   207,
     208,   209,   654,   309,   327,   661,   664,   666,   668,   670,
     672,   709,   324,   325,   163,   659,  -427,   324,   325,   128,
     662,   327,   119,   795,   321,   755,   327,   741,   337,   353,
     298,   417,    65,   258,    63,   451,   452,  -420,   122,   490,
     491,   700,   575,   197,   129,   756,    64,   805,   310,   540,
     310,   433,   428,   322,   120,   211,   212,   199,   214,   215,
     651,   216,   217,   218,   219,   259,   422,   -70,   429,   652,
     123,   439,   402,   493,   491,   691,   710,   254,   213,   258,
     124,   121,   777,   415,   415,   725,  -351,  -351,   203,   415,
     443,   454,   699,   415,   245,   702,   538,   454,   256,   483,
     250,   328,   201,   250,   576,   415,   283,   708,   329,   284,
     289,   259,   702,   512,   296,   328,   250,   656,   748,   328,
     730,   254,   706,   715,   354,   328,   612,   272,   273,   274,
     337,   306,   657,   403,   308,   804,   702,   372,   577,   373,
     315,   318,   328,   430,   578,   316,   125,   328,   765,   767,
     205,   241,   539,   242,   243,   126,   496,   491,   349,   725,
     460,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   601,   497,   491,   602,
     405,   406,   461,   799,   499,   491,   603,   500,   491,   199,
     127,   382,   527,   658,   528,   198,   462,   330,   210,   311,
     505,   491,   220,   221,   806,   555,   556,   222,   579,   580,
     581,   582,   583,   584,   557,   558,   289,   289,   346,   585,
     350,   621,   622,   278,   688,   556,   358,   223,   416,   361,
     362,   224,   365,   366,   750,   751,   753,   556,   483,   425,
     225,   463,   226,   788,   275,   436,   440,   376,   227,   256,
     228,   381,   229,   626,   627,   383,   384,   385,   386,   387,
     388,   389,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   230,   477,   231,   628,   404,   613,   232,   233,
     617,   758,   759,   178,   782,   556,   794,   491,   234,   494,
     629,   810,   491,   235,   630,   635,   236,   237,  -426,   279,
     280,   439,   281,   631,   632,  -402,   258,   292,   245,   293,
     511,   258,   381,   297,   298,   653,   303,   307,   660,   663,
     665,   667,   669,   671,   320,   335,   183,   258,   643,   323,
     645,   336,   512,   648,   633,   339,   340,   483,   259,   184,
     374,   243,   342,   259,   416,   357,   289,   367,   368,   185,
     369,   186,   187,   370,   401,   407,   409,   408,   410,   259,
     420,   421,   330,   423,   424,   346,   508,   509,   446,   535,
     448,   449,   189,   450,   458,   190,   465,   192,   330,   484,
     485,   483,   548,   486,   552,   553,   554,   193,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   270,   271,   272,   273,   274,  -437,   495,
     514,   520,   477,   515,   519,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   178,   587,   521,   524,   451,   526,
     529,   530,   531,   532,   533,   534,   541,   179,   739,   180,
     311,   543,   311,   551,   563,   181,   564,   565,   566,   567,
     593,   568,   569,   570,   571,   572,   573,   773,   586,   589,
     182,   764,   766,   594,   596,   608,   425,   183,   598,   600,
     575,   611,   619,   443,   610,   436,   636,   623,   640,   641,
     184,   639,   649,   646,   381,   647,   644,   673,   783,   676,
     185,   650,   186,   187,   683,   674,   682,   685,   330,   590,
     689,   591,   642,   592,   642,   692,   483,   642,   483,   188,
     697,   477,   696,   189,   722,   723,   190,   191,   192,   483,
     717,   718,   719,   720,   809,   721,   728,   736,   193,   511,
     740,   742,   744,   745,   746,   749,   483,   752,   754,   757,
     760,   637,   556,   693,   768,   769,   772,   535,   771,   774,
     775,   791,    66,   779,   605,   477,   785,   606,   548,   483,
     483,   789,   786,   796,   790,    67,   797,   792,   798,    68,
      69,    70,    71,   800,   713,    72,   802,   807,   803,   808,
      73,   811,   488,   793,    74,   595,   317,   441,   780,   681,
     380,   381,   770,   781,   787,   609,   784,    75,   588,   523,
      76,    77,    78,    79,    80,   510,   525,   680,   705,   763,
     733,   801,   737,   716,    81,    82,    83,    84,   560,    85,
      86,    87,   615,   196,   377,   542,    88,   698,   390,   738,
     546,    89,    90,    91,    92,    93,    94,    95,     0,    96,
      97,     0,     0,    98,    99,   100,     0,   101,     0,     0,
     102,     0,     0,   103,   104,   105,   106,     0,   107,   108,
       0,   109,     0,   110,   111,   112,     0,     0,   113,   114,
     115,     0,   116,     0,     0,     0,     0,   199,     0,     0,
     477,     0,   477,     0,     0,     0,     0,   693,     0,     0,
       0,   607,     0,   477,     0,   130,   131,   132,   133,     0,
     134,   135,   136,     0,     0,     0,   137,   733,    67,     0,
     477,     0,    68,    69,   138,    71,     0,   139,    72,     0,
       0,     0,     0,    73,     0,     0,     0,   140,     0,     0,
       0,     0,     0,   477,   477,   141,     0,     0,     0,   142,
      75,     0,     0,    76,    77,    78,   143,   144,     0,   145,
     146,     0,     0,     0,   147,     0,     0,    81,    82,    83,
      84,     0,    85,   148,    87,   149,     0,     0,     0,    88,
       0,     0,     0,     0,    89,    90,    91,    92,   150,    94,
      95,     0,    96,    97,     0,   151,    98,    99,   100,     0,
     101,     0,     0,   102,     0,     0,   103,   104,   105,   152,
       0,   107,   108,     0,   109,     0,   110,   111,   112,     0,
       0,   113,   114,   153,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   154,   155,     0,
       0,   156,     0,     0,   157,     0,     0,   158,   238,   131,
     132,   133,     0,   343,   135,     0,     0,     0,     0,   137,
       0,    67,     0,     0,     0,    68,    69,    70,    71,     0,
     139,    72,     0,     0,     0,     0,    73,     0,     0,     0,
      74,     0,     0,     0,     0,     0,     0,     0,   141,     0,
       0,     0,   142,    75,     0,     0,    76,    77,    78,    79,
      80,     0,   145,   146,     0,     0,     0,   147,     0,     0,
      81,    82,    83,    84,     0,    85,   239,    87,   240,     0,
       0,     0,    88,     0,     0,     0,     0,    89,    90,    91,
      92,   150,    94,    95,     0,    96,    97,     0,   151,    98,
      99,   100,     0,   101,     0,     0,   102,     0,     0,   103,
     104,   105,   106,     0,   107,   108,     0,   109,     0,   110,
     111,   112,     0,     0,   113,   114,   153,     0,   116,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     154,     0,     0,     0,   156,     0,     0,   344,   345,     0,
     158,   238,   131,   132,   133,     0,   343,   135,     0,     0,
       0,     0,   137,     0,    67,     0,     0,     0,    68,    69,
      70,    71,     0,   139,    72,     0,     0,     0,     0,    73,
       0,     0,     0,    74,     0,     0,     0,     0,     0,     0,
       0,   141,     0,     0,     0,   142,    75,     0,     0,    76,
      77,    78,    79,    80,     0,   145,   146,     0,     0,     0,
     147,     0,     0,    81,    82,    83,    84,     0,    85,   239,
      87,   240,     0,     0,     0,    88,     0,     0,     0,     0,
      89,    90,    91,    92,   150,    94,    95,     0,    96,    97,
       0,   151,    98,    99,   100,     0,   101,     0,     0,   102,
       0,     0,   103,   104,   105,   106,     0,   107,   108,     0,
     109,     0,   110,   111,   112,     0,     0,   113,   114,   153,
       0,   116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   154,     0,     0,     0,   156,     0,     0,
     157,   363,     0,   158,   238,   131,   132,   133,     0,   134,
     135,     0,     0,     0,     0,   137,     0,    67,     0,     0,
       0,    68,    69,   138,    71,     0,   139,    72,     0,     0,
       0,     0,    73,     0,     0,     0,   140,     0,     0,     0,
       0,     0,     0,     0,   141,     0,     0,     0,   142,    75,
       0,     0,    76,    77,    78,   143,   144,     0,   145,   146,
       0,     0,     0,   147,     0,     0,    81,    82,    83,    84,
       0,    85,   148,    87,   149,     0,     0,     0,    88,     0,
       0,     0,     0,    89,    90,    91,    92,   150,    94,    95,
       0,    96,    97,     0,   151,    98,    99,   100,     0,   101,
       0,     0,   102,     0,     0,   103,   104,   105,   152,     0,
     107,   108,     0,   109,     0,   110,   111,   112,     0,     0,
     113,   114,   153,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   154,   155,     0,     0,
     156,     0,     0,   157,     0,     0,   158,   130,   131,   132,
     133,     0,   134,   135,     0,     0,     0,     0,   137,     0,
      67,     0,     0,     0,    68,    69,   138,    71,     0,   139,
      72,     0,     0,     0,     0,    73,     0,     0,     0,   140,
       0,     0,     0,     0,     0,     0,     0,   141,     0,     0,
       0,   142,    75,     0,     0,    76,    77,    78,   143,   144,
       0,   145,   146,     0,     0,     0,   147,     0,     0,    81,
      82,    83,    84,     0,    85,   148,    87,   149,     0,     0,
       0,    88,     0,     0,     0,     0,    89,    90,    91,    92,
     150,    94,    95,     0,    96,    97,     0,   151,    98,    99,
     100,     0,   101,     0,     0,   102,     0,     0,   103,   104,
     105,   152,     0,   107,   108,     0,   109,     0,   110,   111,
     112,     0,     0,   113,   114,   153,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   154,
     155,     0,     0,   156,     0,     0,   157,     0,     0,   158,
     238,   131,   132,   133,     0,   134,   135,     0,     0,     0,
       0,   137,     0,    67,     0,     0,     0,    68,    69,    70,
      71,     0,   139,    72,     0,     0,     0,     0,    73,     0,
       0,     0,    74,     0,     0,     0,     0,     0,     0,     0,
     141,     0,     0,     0,   142,    75,     0,     0,    76,    77,
      78,    79,    80,     0,   145,   146,     0,     0,     0,   147,
       0,     0,    81,    82,    83,    84,     0,    85,   239,    87,
     240,     0,     0,     0,    88,     0,     0,     0,     0,    89,
      90,    91,    92,   150,    94,    95,     0,    96,    97,     0,
     151,    98,    99,   100,   487,   101,     0,     0,   102,     0,
       0,   103,   104,   105,   106,     0,   107,   108,     0,   109,
       0,   110,   111,   112,     0,     0,   113,   114,   153,     0,
     116,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   154,     0,     0,     0,   156,     0,     0,   157,
       0,     0,   158,   238,   131,   132,   133,     0,   343,   135,
       0,     0,     0,     0,   137,     0,    67,     0,     0,     0,
      68,    69,    70,    71,     0,   139,    72,     0,     0,     0,
       0,    73,     0,     0,     0,    74,     0,     0,     0,     0,
       0,     0,     0,   141,     0,     0,     0,   142,    75,     0,
       0,    76,    77,    78,    79,    80,     0,   145,   146,     0,
       0,     0,   147,     0,     0,    81,    82,    83,    84,     0,
      85,   239,    87,   240,     0,     0,     0,    88,     0,     0,
       0,     0,    89,    90,    91,    92,   150,    94,    95,     0,
      96,    97,     0,   151,    98,    99,   100,     0,   101,     0,
       0,   102,     0,     0,   103,   104,   105,   106,     0,   107,
     108,     0,   109,     0,   110,   111,   112,     0,     0,   113,
     114,   153,     0,   116,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   154,     0,     0,     0,   156,
       0,     0,   157,   345,     0,   158,   238,   131,   132,   133,
       0,   134,   135,     0,     0,     0,     0,   137,     0,    67,
       0,     0,     0,    68,    69,    70,    71,     0,   139,    72,
       0,     0,     0,     0,    73,     0,     0,     0,    74,     0,
       0,     0,     0,     0,     0,     0,   141,     0,     0,     0,
     142,    75,     0,     0,    76,    77,    78,    79,    80,     0,
     145,   146,     0,     0,     0,   147,     0,     0,    81,    82,
      83,    84,     0,    85,   239,    87,   240,     0,     0,     0,
      88,     0,     0,     0,     0,    89,    90,    91,    92,   150,
      94,    95,     0,    96,    97,     0,   151,    98,    99,   100,
       0,   101,     0,     0,   102,     0,     0,   103,   104,   105,
     106,     0,   107,   108,     0,   109,     0,   110,   111,   112,
       0,     0,   113,   114,   153,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   154,     0,
       0,     0,   156,     0,     0,   157,     0,     0,   158,   238,
     131,   132,   133,     0,   343,   135,     0,     0,     0,     0,
     137,     0,    67,     0,     0,     0,    68,    69,    70,    71,
       0,   139,    72,     0,     0,     0,     0,    73,     0,     0,
       0,    74,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,     0,   142,    75,     0,     0,    76,    77,    78,
      79,    80,     0,   145,   146,     0,     0,     0,   147,     0,
       0,    81,    82,    83,    84,     0,    85,   239,    87,   240,
       0,     0,     0,    88,     0,     0,     0,     0,    89,    90,
      91,    92,   150,    94,    95,     0,    96,    97,     0,   151,
      98,    99,   100,     0,   101,     0,     0,   102,     0,     0,
     103,   104,   105,   106,     0,   107,   108,     0,   109,     0,
     110,   111,   112,     0,     0,   113,   114,   153,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   154,     0,     0,     0,   156,     0,     0,   157,     0,
       0,   158,   238,   131,   132,   133,     0,   375,   135,     0,
       0,     0,     0,   137,     0,    67,     0,     0,     0,    68,
      69,    70,    71,     0,   139,    72,     0,     0,     0,     0,
      73,     0,     0,     0,    74,     0,     0,     0,     0,     0,
       0,     0,   141,     0,     0,     0,   142,    75,     0,     0,
      76,    77,    78,    79,    80,     0,   145,   146,     0,     0,
       0,   147,     0,     0,    81,    82,    83,    84,     0,    85,
     239,    87,   240,     0,     0,     0,    88,     0,     0,     0,
       0,    89,    90,    91,    92,   150,    94,    95,     0,    96,
      97,     0,   151,    98,    99,   100,     0,   101,     0,     0,
     102,     0,     0,   103,   104,   105,   106,     0,   107,   108,
       0,   109,     0,   110,   111,   112,     0,     0,   113,   114,
     153,     0,   116,     0,     0,     0,     0,     0,     0,    66,
     466,     0,     0,     0,   154,     0,     0,     0,   156,     0,
       0,   157,    67,     0,   158,     0,    68,    69,    70,    71,
       0,   467,    72,     0,     0,     0,     0,    73,     0,     0,
       0,   468,     0,     0,     0,     0,     0,     0,     0,   469,
       0,   470,     0,     0,    75,     0,     0,    76,    77,    78,
     471,    80,     0,   472,     0,     0,     0,     0,   473,     0,
       0,    81,    82,    83,    84,     0,   474,    86,    87,     0,
       0,     0,     0,    88,     0,     0,     0,     0,    89,    90,
      91,    92,    93,    94,    95,     0,    96,    97,     0,     0,
      98,    99,   100,     0,   101,     0,     0,   102,     0,     0,
     103,   104,   105,   106,     0,   107,   108,    66,   109,     0,
     110,   111,   112,     0,     0,   113,   114,   475,     0,   116,
      67,     0,     0,     0,    68,    69,    70,    71,     0,     0,
      72,     0,     0,     0,     0,    73,     0,     0,   476,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,     0,     0,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    81,
      82,    83,    84,     0,    85,    86,    87,     0,     0,     0,
       0,    88,     0,     0,     0,     0,    89,    90,    91,    92,
      93,    94,    95,     0,    96,    97,     0,     0,    98,    99,
     100,     0,   101,     0,     0,   102,     0,     0,   103,   104,
     105,   106,     0,   107,   108,     0,   109,     0,   110,   111,
     112,     0,     0,   113,   114,   115,     0,   116,     0,     0,
      66,     0,   324,     0,     0,   411,     0,     0,     0,     0,
       0,     0,     0,    67,     0,     0,   378,    68,    69,    70,
      71,     0,     0,    72,     0,     0,     0,     0,    73,     0,
       0,     0,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   412,     0,    75,     0,     0,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,    82,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,    88,   413,     0,     0,     0,    89,
      90,    91,    92,    93,    94,    95,     0,    96,    97,     0,
       0,    98,    99,   100,     0,   101,     0,     0,   102,     0,
       0,   103,   104,   105,   106,     0,   107,   108,     0,   109,
     414,   110,   111,   112,     0,     0,   113,   114,   115,     0,
     116,    66,     0,   324,     0,     0,     0,     0,     0,     0,
       0,     0,   415,     0,    67,     0,     0,     0,    68,    69,
      70,    71,     0,     0,    72,     0,     0,     0,     0,    73,
       0,     0,     0,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,     0,     0,    76,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    81,    82,    83,    84,     0,    85,    86,
      87,     0,     0,     0,     0,    88,     0,     0,     0,     0,
      89,    90,    91,    92,    93,    94,    95,     0,    96,    97,
       0,     0,    98,    99,   100,     0,   101,     0,     0,   102,
       0,     0,   103,   104,   105,   106,     0,   107,   108,     0,
     109,     0,   110,   111,   112,     0,     0,   113,   114,   115,
       0,   116,    66,   466,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   415,     0,    67,     0,     0,     0,    68,
      69,    70,    71,     0,   467,    72,     0,     0,     0,     0,
      73,     0,     0,     0,   468,     0,     0,     0,     0,     0,
       0,     0,   469,     0,   470,     0,     0,    75,     0,     0,
      76,    77,    78,   471,    80,     0,   472,     0,     0,     0,
       0,   473,     0,     0,    81,    82,    83,    84,     0,    85,
      86,    87,     0,     0,     0,     0,    88,     0,     0,     0,
       0,    89,    90,    91,    92,   701,    94,    95,     0,    96,
      97,     0,     0,    98,    99,   100,     0,   101,     0,     0,
     102,     0,     0,   103,   104,   105,   106,     0,   107,   108,
       0,   109,     0,   110,   111,   112,    66,   466,   113,   114,
     475,     0,   116,     0,     0,     0,     0,     0,     0,    67,
       0,     0,     0,    68,    69,    70,    71,     0,   467,    72,
       0,     0,     0,     0,    73,     0,     0,     0,   468,     0,
       0,     0,     0,     0,     0,     0,   469,     0,   470,     0,
       0,    75,     0,     0,    76,    77,    78,   471,    80,     0,
     472,     0,     0,     0,     0,   473,     0,     0,    81,    82,
      83,    84,     0,    85,    86,    87,     0,     0,     0,     0,
      88,     0,     0,     0,     0,    89,    90,    91,    92,    93,
      94,    95,     0,    96,    97,     0,     0,    98,    99,   100,
       0,   101,     0,     0,   102,     0,     0,   103,   104,   105,
     106,     0,   107,   108,     0,   109,     0,   110,   111,   112,
       0,     0,   113,   114,   475,    66,   116,   285,   286,     0,
     287,     0,     0,     0,     0,     0,     0,     0,    67,     0,
       0,     0,    68,    69,    70,    71,     0,     0,    72,     0,
       0,     0,     0,    73,     0,     0,     0,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,     0,     0,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    81,    82,    83,
      84,     0,    85,    86,    87,     0,     0,     0,     0,    88,
     288,     0,     0,     0,    89,    90,    91,    92,    93,    94,
      95,     0,    96,    97,     0,     0,    98,    99,   100,     0,
     101,     0,     0,   102,     0,     0,   103,   104,   105,   106,
       0,   107,   108,    66,   109,     0,   110,   111,   112,     0,
     170,   113,   114,   115,     0,   116,    67,     0,     0,     0,
      68,    69,    70,    71,     0,     0,    72,     0,     0,   171,
       0,    73,     0,     0,     0,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,     0,
       0,    76,   172,    78,    79,    80,     0,     0,     0,   173,
       0,     0,     0,     0,     0,    81,    82,    83,    84,     0,
      85,    86,    87,     0,     0,     0,     0,   174,     0,     0,
       0,     0,    89,    90,    91,    92,    93,    94,    95,     0,
      96,    97,     0,     0,    98,    99,   100,     0,   101,     0,
       0,   175,     0,     0,   103,   104,   105,   106,     0,   107,
     108,     0,     0,     0,   110,   111,   112,     0,     0,   113,
     114,   115,    66,   116,   522,     0,     0,   411,     0,     0,
       0,     0,     0,     0,     0,    67,     0,     0,     0,    68,
      69,    70,    71,     0,     0,    72,     0,     0,     0,     0,
      73,     0,     0,     0,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,     0,     0,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,    82,    83,    84,     0,    85,
      86,    87,     0,     0,     0,     0,    88,     0,     0,     0,
       0,    89,    90,    91,    92,    93,    94,    95,     0,    96,
      97,     0,     0,    98,    99,   100,     0,   101,     0,     0,
     102,     0,     0,   103,   104,   105,   106,     0,   107,   108,
      66,   109,   731,   110,   111,   112,     0,     0,   113,   114,
     115,     0,   116,    67,     0,     0,     0,    68,    69,    70,
      71,     0,     0,    72,     0,     0,     0,     0,    73,     0,
       0,     0,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,     0,     0,    76,    77,
      78,    79,    80,     0,   732,     0,     0,     0,     0,     0,
       0,     0,    81,    82,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,    88,     0,     0,     0,     0,    89,
      90,    91,    92,    93,    94,    95,     0,    96,    97,     0,
       0,    98,    99,   100,     0,   101,     0,     0,   102,     0,
       0,   103,   104,   105,   106,     0,   107,   108,    66,   109,
       0,   110,   111,   112,     0,     0,   113,   114,   115,     0,
     116,    67,     0,     0,     0,    68,    69,    70,    71,     0,
       0,    72,     0,     0,     0,     0,    73,     0,     0,     0,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,     0,    76,    77,    78,    79,
      80,     0,     0,   244,     0,     0,     0,     0,     0,     0,
      81,    82,    83,    84,     0,    85,    86,    87,     0,     0,
       0,     0,    88,     0,     0,     0,     0,    89,    90,    91,
      92,    93,    94,    95,     0,    96,    97,     0,     0,    98,
      99,   100,     0,   101,     0,     0,   102,     0,     0,   103,
     104,   105,   106,     0,   107,   108,    66,   109,     0,   110,
     111,   112,     0,     0,   113,   114,   115,     0,   116,    67,
       0,   255,     0,    68,    69,    70,    71,     0,     0,    72,
       0,     0,     0,     0,    73,     0,     0,     0,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,     0,     0,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,   258,     0,     0,     0,    81,    82,
      83,    84,     0,    85,    86,    87,     0,     0,     0,     0,
      88,     0,     0,     0,     0,    89,    90,    91,    92,    93,
      94,    95,     0,    96,    97,     0,   259,    98,    99,   100,
       0,   101,     0,     0,   102,     0,     0,   103,   104,   105,
     106,     0,   107,   108,     0,   109,    66,   110,   111,   112,
       0,   294,   113,   114,   115,     0,   116,     0,     0,    67,
       0,     0,     0,    68,    69,    70,    71,     0,     0,    72,
       0,     0,     0,     0,    73,     0,     0,     0,    74,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,    75,     0,     0,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    81,    82,
      83,    84,     0,    85,    86,    87,     0,     0,     0,     0,
      88,     0,     0,     0,     0,    89,    90,    91,    92,    93,
      94,    95,     0,    96,    97,     0,     0,    98,    99,   100,
       0,   101,     0,     0,   102,     0,     0,   103,   295,   105,
     106,     0,   107,   108,    66,   109,     0,   110,   111,   112,
       0,     0,   113,   114,   115,     0,   116,    67,     0,     0,
       0,    68,    69,    70,    71,     0,     0,    72,     0,     0,
       0,     0,    73,     0,     0,     0,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,     0,    76,    77,    78,    79,    80,     0,   547,     0,
       0,     0,     0,     0,     0,     0,    81,    82,    83,    84,
       0,    85,    86,    87,     0,     0,     0,     0,    88,     0,
       0,     0,     0,    89,    90,    91,    92,    93,    94,    95,
       0,    96,    97,     0,     0,    98,    99,   100,     0,   101,
       0,     0,   102,     0,     0,   103,   104,   105,   106,     0,
     107,   108,    66,   109,     0,   110,   111,   112,     0,     0,
     113,   114,   115,     0,   116,    67,     0,     0,     0,    68,
      69,    70,    71,     0,     0,    72,     0,     0,     0,     0,
      73,     0,     0,     0,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,     0,     0,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,    82,    83,    84,   712,    85,
      86,    87,     0,     0,     0,     0,    88,     0,     0,     0,
       0,    89,    90,    91,    92,    93,    94,    95,     0,    96,
      97,     0,     0,    98,    99,   100,     0,   101,     0,     0,
     102,     0,     0,   103,   104,   105,   106,     0,   107,   108,
      66,   109,     0,   110,   111,   112,     0,     0,   113,   114,
     115,     0,   116,    67,     0,     0,     0,    68,    69,    70,
      71,     0,     0,    72,     0,     0,     0,     0,    73,     0,
       0,     0,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,     0,     0,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,    82,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,    88,     0,     0,     0,     0,    89,
      90,    91,    92,    93,    94,    95,     0,    96,    97,     0,
       0,    98,    99,   100,     0,   101,     0,     0,   102,     0,
       0,   103,   104,   105,   106,     0,   107,   108,    66,   109,
       0,   110,   111,   112,   282,     0,   113,   114,   115,     0,
     116,    67,     0,     0,     0,    68,    69,    70,    71,     0,
       0,    72,     0,     0,     0,     0,    73,     0,     0,     0,
      74,     0,     0,     0,     0,     0,     0,     0,   258,     0,
       0,     0,     0,    75,     0,     0,    76,    77,    78,    79,
      80,     0,     0,     0,     0,   258,     0,     0,     0,     0,
      81,    82,    83,    84,     0,    85,    86,    87,     0,     0,
     259,   258,     0,     0,     0,     0,     0,    89,    90,    91,
      92,    93,    94,    95,     0,    96,    97,   259,     0,    98,
      99,   100,     0,   101,     0,     0,   102,     0,     0,   103,
     104,   105,   106,   259,   107,   108,   258,  -401,     0,   110,
     111,   112,     0,     0,   113,   114,   115,     0,   116,     0,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,     0,   258,     0,   259,   501,
     502,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,     0,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   259,     0,
       0,   258,   503,   504,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   259,   258,     0,     0,     0,   506,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,     0,     0,     0,   259,   258,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,     0,   259,
     258,     0,     0,   504,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,     0,   259,   258,     0,     0,   679,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,     0,   259,   258,     0,   371,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,     0,   259,   258,
       0,   492,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
       0,   259,   258,     0,   498,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,     0,   259,     0,     0,   507,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,     0,     0,     0,     0,
     677,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,     1,
       0,     0,     2,   678,     0,     3,     0,     0,     0,     0,
       4,     0,     0,     0,     0,     5,     0,     0,     6,     0,
       7,     8,     9,     0,     0,   258,    10,     0,     0,     0,
      11,     0,   489,     0,     0,     0,     0,     0,     0,     0,
       0,   258,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   258,     0,   259,     0,     0,
       0,     0,     0,    13,     0,     0,     0,     0,   258,     0,
       0,     0,    14,   259,    15,     0,     0,    16,     0,    17,
       0,    18,     0,     0,    19,     0,    20,   259,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,     0,    22,
     259,     0,     0,     0,     0,     0,     0,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,     0,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274
};

static const short yycheck[] =
{
       4,   224,   458,   226,   227,   540,   544,   230,   231,   210,
     521,   234,    18,     5,     5,   184,     8,   619,    22,     5,
     189,    18,   191,     5,   538,    13,    14,   541,    13,    14,
     476,     5,     6,    27,     8,     5,    40,   206,     8,    22,
      14,    13,    14,    50,     9,     5,     6,     9,     8,     5,
       6,   379,    45,    34,    14,     5,     6,    68,    14,    63,
      64,    65,   576,    99,    14,   579,   580,   581,   582,   583,
     584,    79,     5,     6,    17,     8,    64,     5,     6,    59,
       8,    14,    51,   774,    15,    19,    14,    50,   122,    41,
      68,   292,    59,    42,    96,     5,     6,     0,    51,   143,
     144,   612,   125,   112,    84,    39,   108,   798,   144,   437,
     144,   122,    14,    44,    83,   119,   120,   121,   122,   123,
     143,   125,   126,   127,   128,    74,   295,   105,    30,   575,
      83,   309,    78,   143,   144,   116,   144,   144,   121,    42,
      21,    50,   744,   135,   135,   656,   143,   144,   141,   135,
     144,   143,   143,   135,   158,   757,   128,   143,   162,   337,
     148,   135,     0,   148,    24,   135,   172,   623,   142,   173,
     174,    74,   774,   374,   178,   135,   148,   142,   689,   135,
     142,   144,   142,   639,   136,   135,   142,   136,   137,   138,
     122,   195,    78,   139,   198,   797,   798,   144,    58,   146,
      97,   205,   135,   105,    64,   102,    87,   135,   722,   723,
      63,   154,   144,   156,   157,    96,   143,   144,   222,   730,
      25,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    88,   143,   144,    91,
     280,   281,    47,   781,   143,   144,    98,   143,   144,   253,
      59,   255,   421,   139,   423,    96,    61,   458,   142,   202,
     143,   144,    50,   142,   799,   143,   144,   147,   128,   129,
     130,   131,   132,   133,   143,   144,   280,   281,   221,   139,
     223,   143,   144,    64,   143,   144,   229,   142,   292,   232,
     233,   142,   235,   236,   143,   144,   143,   144,   476,   303,
     142,   106,   142,   759,    69,   309,   310,   250,   142,   313,
     142,   254,   142,    25,    26,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   142,   337,   142,    47,   279,   538,   142,   142,
     541,   143,   144,    16,   143,   144,   143,   144,   142,   353,
      62,   143,   144,   142,    66,   556,   142,   142,    64,   128,
     128,   539,    99,    75,    76,   112,    42,   128,   372,   112,
     374,    42,   315,    99,    68,   576,    79,   119,   579,   580,
     581,   582,   583,   584,    89,    94,    59,    42,   566,   111,
     568,   115,   593,   571,   106,   115,    50,   575,    74,    72,
     128,   344,   103,    74,   408,   143,   410,   143,   142,    82,
     142,    84,    85,   142,     5,   128,    52,   128,   135,    74,
      65,   105,   623,   105,     8,   368,   369,   370,   142,   433,
      32,    32,   105,    96,   144,   108,     8,   110,   639,     8,
       8,   619,   446,     8,   448,   449,   450,   120,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   134,   135,   136,   137,   138,   128,   143,
     149,    78,   476,   149,   144,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    16,   489,   142,     9,     5,    67,
     128,   144,   100,     5,   100,     8,   128,    29,   676,    31,
     443,    80,   445,   118,   143,    37,   103,   103,   142,   142,
     128,   142,   142,   142,   142,   142,   142,   740,   143,   143,
      52,   722,   723,    97,    53,   529,   530,    59,     5,   128,
     125,   124,    27,   144,   128,   539,     8,   142,     8,     8,
      72,   144,     8,   143,   487,   143,   136,    58,   749,    58,
      82,   143,    84,    85,     5,   144,   143,   123,   759,   502,
     142,   504,   566,   506,   568,    88,   744,   571,   746,   101,
       8,   575,   142,   105,   125,   125,   108,   109,   110,   757,
     143,   143,   143,   143,   807,   143,    78,   143,   120,   593,
     142,    80,    27,    53,    56,   128,   774,     8,   142,   144,
       3,    18,   144,   607,   144,   143,   143,   611,   144,    27,
      69,     8,     3,    80,     5,   619,   143,     8,   622,   797,
     798,   128,   143,     5,   144,    16,   144,   143,    27,    20,
      21,    22,    23,     8,   638,    26,   144,   142,   144,     5,
      31,   143,   344,   771,    35,   516,   203,   313,   746,   594,
     254,   594,   730,   747,   757,   530,   751,    48,   491,   408,
      51,    52,    53,    54,    55,   372,   410,   593,   622,   710,
     674,   790,   676,   639,    65,    66,    67,    68,   458,    70,
      71,    72,   539,    19,   251,   443,    77,   611,   264,   676,
     445,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      91,    -1,    -1,    94,    95,    96,    -1,    98,    -1,    -1,
     101,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
      -1,   112,    -1,   114,   115,   116,    -1,    -1,   119,   120,
     121,    -1,   123,    -1,    -1,    -1,    -1,   741,    -1,    -1,
     744,    -1,   746,    -1,    -1,    -1,    -1,   751,    -1,    -1,
      -1,   142,    -1,   757,    -1,     3,     4,     5,     6,    -1,
       8,     9,    10,    -1,    -1,    -1,    14,   771,    16,    -1,
     774,    -1,    20,    21,    22,    23,    -1,    25,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,   797,   798,    43,    -1,    -1,    -1,    47,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      58,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    91,    -1,    93,    94,    95,    96,    -1,
      98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,    -1,   112,    -1,   114,   115,   116,    -1,
      -1,   119,   120,   121,    -1,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,    -1,
      -1,   139,    -1,    -1,   142,    -1,    -1,   145,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      25,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,
      -1,    -1,    47,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    57,    58,    -1,    -1,    -1,    62,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    -1,    93,    94,
      95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,    -1,   112,    -1,   114,
     115,   116,    -1,    -1,   119,   120,   121,    -1,   123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     135,    -1,    -1,    -1,   139,    -1,    -1,   142,   143,    -1,
     145,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    47,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    57,    58,    -1,    -1,    -1,
      62,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    91,
      -1,    93,    94,    95,    96,    -1,    98,    -1,    -1,   101,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,    -1,
     112,    -1,   114,   115,   116,    -1,    -1,   119,   120,   121,
      -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,    -1,    -1,    -1,   139,    -1,    -1,
     142,   143,    -1,   145,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    -1,    25,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    47,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    57,    58,
      -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    91,    -1,    93,    94,    95,    96,    -1,    98,
      -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,    -1,   112,    -1,   114,   115,   116,    -1,    -1,
     119,   120,   121,    -1,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   135,   136,    -1,    -1,
     139,    -1,    -1,   142,    -1,    -1,   145,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,    25,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    47,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    57,    58,    -1,    -1,    -1,    62,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    91,    -1,    93,    94,    95,
      96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,
     106,   107,    -1,   109,   110,    -1,   112,    -1,   114,   115,
     116,    -1,    -1,   119,   120,   121,    -1,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,    -1,   139,    -1,    -1,   142,    -1,    -1,   145,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    -1,    -1,    47,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    57,    58,    -1,    -1,    -1,    62,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    -1,
      93,    94,    95,    96,    97,    98,    -1,    -1,   101,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,    -1,    -1,   119,   120,   121,    -1,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,    -1,    -1,    -1,   139,    -1,    -1,   142,
      -1,    -1,   145,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    47,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    57,    58,    -1,
      -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    91,    -1,    93,    94,    95,    96,    -1,    98,    -1,
      -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,    -1,   112,    -1,   114,   115,   116,    -1,    -1,   119,
     120,   121,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   135,    -1,    -1,    -1,   139,
      -1,    -1,   142,   143,    -1,   145,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    25,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,
      47,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      57,    58,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    91,    -1,    93,    94,    95,    96,
      -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,    -1,   114,   115,   116,
      -1,    -1,   119,   120,   121,    -1,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,
      -1,    -1,   139,    -1,    -1,   142,    -1,    -1,   145,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    47,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    58,    -1,    -1,    -1,    62,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    91,    -1,    93,
      94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,
     104,   105,   106,   107,    -1,   109,   110,    -1,   112,    -1,
     114,   115,   116,    -1,    -1,   119,   120,   121,    -1,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   135,    -1,    -1,    -1,   139,    -1,    -1,   142,    -1,
      -1,   145,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    -1,    -1,    -1,    47,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    57,    58,    -1,    -1,
      -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      91,    -1,    93,    94,    95,    96,    -1,    98,    -1,    -1,
     101,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
      -1,   112,    -1,   114,   115,   116,    -1,    -1,   119,   120,
     121,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,    -1,    -1,   135,    -1,    -1,    -1,   139,    -1,
      -1,   142,    16,    -1,   145,    -1,    20,    21,    22,    23,
      -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      -1,    45,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    91,    -1,    -1,
      94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,
     104,   105,   106,   107,    -1,   109,   110,     3,   112,    -1,
     114,   115,   116,    -1,    -1,   119,   120,   121,    -1,   123,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,   142,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    91,    -1,    -1,    94,    95,
      96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,
     106,   107,    -1,   109,   110,    -1,   112,    -1,   114,   115,
     116,    -1,    -1,   119,   120,   121,    -1,   123,    -1,    -1,
       3,    -1,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,   142,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    -1,
      -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
     113,   114,   115,   116,    -1,    -1,   119,   120,   121,    -1,
     123,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    91,
      -1,    -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,    -1,
     112,    -1,   114,   115,   116,    -1,    -1,   119,   120,   121,
      -1,   123,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    -1,    45,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      91,    -1,    -1,    94,    95,    96,    -1,    98,    -1,    -1,
     101,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
      -1,   112,    -1,   114,   115,   116,     3,     4,   119,   120,
     121,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    25,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    45,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    91,    -1,    -1,    94,    95,    96,
      -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,    -1,   114,   115,   116,
      -1,    -1,   119,   120,   121,     3,   123,     5,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,
      78,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    91,    -1,    -1,    94,    95,    96,    -1,
      98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,     3,   112,    -1,   114,   115,   116,    -1,
      10,   119,   120,   121,    -1,   123,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    29,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,    59,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    91,    -1,    -1,    94,    95,    96,    -1,    98,    -1,
      -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,    -1,    -1,    -1,   114,   115,   116,    -1,    -1,   119,
     120,   121,     3,   123,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      91,    -1,    -1,    94,    95,    96,    -1,    98,    -1,    -1,
     101,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
       3,   112,     5,   114,   115,   116,    -1,    -1,   119,   120,
     121,    -1,   123,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    -1,
      -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,     3,   112,
      -1,   114,   115,   116,    -1,    -1,   119,   120,   121,    -1,
     123,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    -1,    -1,    94,
      95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,     3,   112,    -1,   114,
     115,   116,    -1,    -1,   119,   120,   121,    -1,   123,    16,
      -1,    18,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    91,    -1,    74,    94,    95,    96,
      -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,     3,   114,   115,   116,
      -1,     8,   119,   120,   121,    -1,   123,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    91,    -1,    -1,    94,    95,    96,
      -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,     3,   112,    -1,   114,   115,   116,
      -1,    -1,   119,   120,   121,    -1,   123,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    91,    -1,    -1,    94,    95,    96,    -1,    98,
      -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,     3,   112,    -1,   114,   115,   116,    -1,    -1,
     119,   120,   121,    -1,   123,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      91,    -1,    -1,    94,    95,    96,    -1,    98,    -1,    -1,
     101,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
       3,   112,    -1,   114,   115,   116,    -1,    -1,   119,   120,
     121,    -1,   123,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    -1,
      -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,     3,   112,
      -1,   114,   115,   116,     9,    -1,   119,   120,   121,    -1,
     123,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      74,    42,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    74,    -1,    94,
      95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,
     105,   106,   107,    74,   109,   110,    42,   112,    -1,   114,
     115,   116,    -1,    -1,   119,   120,   121,    -1,   123,    -1,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,    -1,    42,    -1,    74,   143,
     144,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,    -1,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    74,    -1,
      -1,    42,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    74,    42,    -1,    -1,    -1,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    74,    42,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,    74,
      42,    -1,    -1,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    -1,    74,    42,    -1,    -1,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    -1,    74,    42,    -1,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,    -1,    74,    42,
      -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
      -1,    74,    42,    -1,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    -1,    74,    -1,    -1,   143,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,    17,
      -1,    -1,    20,   143,    -1,    23,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    33,    -1,    -1,    36,    -1,
      38,    39,    40,    -1,    -1,    42,    44,    -1,    -1,    -1,
      48,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    90,    74,    92,    -1,    -1,    95,    -1,    97,
      -1,    99,    -1,    -1,   102,    -1,   104,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
      74,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,    -1,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    23,    28,    33,    36,    38,    39,    40,
      44,    48,    60,    81,    90,    92,    95,    97,    99,   102,
     104,   114,   117,   151,   152,   155,   156,   157,   158,   165,
     208,   213,   214,   219,   220,   221,   222,   233,   234,   243,
     244,   245,   246,   247,   251,   252,   257,   258,   259,   262,
     264,   265,   266,   267,   268,   269,   271,   272,   273,   274,
     275,   276,   290,    96,   108,    59,     3,    16,    20,    21,
      22,    23,    26,    31,    35,    48,    51,    52,    53,    54,
      55,    65,    66,    67,    68,    70,    71,    72,    77,    82,
      83,    84,    85,    86,    87,    88,    90,    91,    94,    95,
      96,    98,   101,   104,   105,   106,   107,   109,   110,   112,
     114,   115,   116,   119,   120,   121,   123,   153,   154,    51,
      83,    50,    51,    83,    21,    87,    96,    59,    59,    84,
       3,     4,     5,     6,     8,     9,    10,    14,    22,    25,
      35,    43,    47,    54,    55,    57,    58,    62,    71,    73,
      86,    93,   107,   121,   135,   136,   139,   142,   145,   154,
     166,   167,   169,   201,   202,   270,   277,   278,   281,   282,
      10,    29,    52,    59,    77,   101,   153,   260,    16,    29,
      31,    37,    52,    59,    72,    82,    84,    85,   101,   105,
     108,   109,   110,   120,   210,   211,   260,   112,    96,   154,
     170,     0,    45,   141,   288,    63,   154,   154,   154,   154,
     142,   154,   154,   170,   154,   154,   154,   154,   154,   154,
      50,   142,   147,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,     3,    71,
      73,   201,   201,   201,    58,   154,   205,   206,    13,    14,
     148,   279,   280,    50,   144,    18,   154,   168,    42,    74,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,    69,   193,   194,    64,   128,
     128,    99,     9,   153,   154,     5,     6,     8,    78,   154,
     217,   218,   128,   112,     8,   105,   154,    99,    68,   209,
     209,   209,   209,    79,   195,   196,   154,   119,   154,    99,
     144,   201,   285,   286,   287,    97,   102,   156,   154,   209,
      89,    15,    44,   111,     5,     6,     8,    14,   135,   142,
     178,   179,   229,   235,   236,    94,   115,   122,   172,   115,
      50,   289,   103,     8,   142,   143,   201,   203,   204,   154,
     201,   201,   203,    41,   136,   203,   203,   143,   201,   203,
     203,   201,   201,   143,   203,   201,   201,   143,   142,   142,
     142,   143,   144,   146,   128,     8,   201,   280,   142,   170,
     167,   201,   154,   201,   201,   201,   201,   201,   201,   201,
     282,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,     5,    78,   139,   201,   217,   217,   128,   128,    52,
     135,     8,    46,    78,   113,   135,   154,   178,   215,   216,
      65,   105,   209,   105,     8,   154,   197,   198,    14,    30,
     105,   212,    68,   122,   253,   254,   154,   248,   249,   278,
     154,   168,    27,   144,   283,   284,   142,   223,    32,    32,
      96,     5,     6,     8,   143,   178,   180,   237,   144,   238,
      25,    47,    61,   106,   263,     8,     4,    25,    35,    43,
      45,    54,    57,    62,    70,   121,   142,   154,   173,   174,
     175,   176,   177,   278,     8,     8,     8,    97,   158,    49,
     143,   144,   143,   143,   154,   143,   143,   143,   143,   143,
     143,   143,   144,   143,   144,   143,   144,   143,   201,   201,
     206,   154,   178,   207,   149,   149,   162,   171,   172,   144,
      78,   142,     5,   215,     9,   218,    67,   209,   209,   128,
     144,   100,     5,   100,     8,   154,   255,   256,   128,   144,
     172,   128,   285,    80,   189,   190,   287,    57,   154,   224,
     225,   118,   154,   154,   154,   143,   144,   143,   144,   229,
     236,   239,   240,   143,   103,   103,   142,   142,   142,   142,
     142,   142,   142,   142,   173,   125,    24,    58,    64,   128,
     129,   130,   131,   132,   133,   139,   143,   154,   204,   143,
     201,   201,   201,   128,    97,   165,    53,   183,     5,   180,
     128,    88,    91,    98,   261,     5,     8,   142,   154,   198,
     128,   124,   142,   178,   179,   249,   195,   178,   179,    27,
     193,   143,   144,   142,   226,   227,    25,    26,    47,    62,
      66,    75,    76,   106,   250,   178,     8,    18,   241,   144,
       8,     8,   154,   278,   136,   278,   143,   143,   278,     8,
     143,   143,   173,   178,   179,     9,   142,    78,   139,     8,
     178,   179,     8,   178,   179,   178,   179,   178,   179,   178,
     179,   178,   179,    58,   144,   159,    58,   143,   143,   144,
     207,   166,   143,     5,   184,   123,   187,   188,   143,   142,
      34,   116,    88,   154,   199,   200,   142,     8,   256,   143,
     180,    86,   177,   191,   192,   224,   142,   228,   229,    79,
     144,   230,    69,   154,   242,   229,   240,   143,   143,   143,
     143,   143,   125,   125,     8,   180,   181,   182,    78,     9,
     142,     5,    57,   154,   160,   161,   143,   154,   277,   278,
     142,    50,    80,   163,    27,    53,    56,   186,   180,   128,
     143,   144,     8,   143,   142,    19,    39,   144,   143,   144,
       3,   231,   232,   227,   178,   179,   178,   179,   144,   143,
     182,   144,   143,   203,    27,    69,   164,   177,   185,    80,
     175,   189,   143,   178,   200,   143,   143,   192,   229,   128,
     144,     8,   143,   161,   143,   191,     5,   144,    27,   193,
       8,   232,   144,   144,   177,   191,   195,   142,     5,   203,
     143,   143
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

  case 86:

    { pParser->PushQuery(); ;}
    break;

  case 87:

    { pParser->PushQuery(); ;}
    break;

  case 88:

    { pParser->PushQuery(); ;}
    break;

  case 92:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 94:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 97:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 98:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 102:

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

  case 103:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 105:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 106:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 107:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 110:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 113:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 114:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 115:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 116:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 117:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 118:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 119:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 120:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 121:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 122:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 123:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 125:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 132:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 134:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 135:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 136:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_sRefString = pParser->m_pBuf;
		;}
    break;

  case 137:

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

  case 138:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 153:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 154:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 155:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 156:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 157:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 160:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 161:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 162:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 163:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 164:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 170:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 171:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 172:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 173:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 174:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 175:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 176:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 177:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 178:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 184:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 185:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 186:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 188:

    {
			pParser->AddHaving();
		;}
    break;

  case 191:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 194:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 195:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 197:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 199:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 200:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 203:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 204:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 210:

    {
			if ( !pParser->AddOption ( yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 211:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 212:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 213:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 214:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 215:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 216:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 217:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 218:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 220:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 221:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 226:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 227:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 234:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 235:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 236:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 237:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 238:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 239:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 240:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 241:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 242:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 243:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
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

  case 250:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 251:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 252:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 254:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 255:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 256:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 257:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 258:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 259:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 260:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 261:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 262:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 263:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 264:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 265:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 270:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 271:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 278:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 279:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 280:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 281:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 282:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 283:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 284:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 285:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 286:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 287:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 288:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 289:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 290:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 291:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 295:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 296:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 297:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 298:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 299:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 300:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 301:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 302:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 303:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 304:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 305:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 308:

    { yyval.m_iValue = 1; ;}
    break;

  case 309:

    { yyval.m_iValue = 0; ;}
    break;

  case 310:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 318:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 319:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 320:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 323:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 324:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 325:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 330:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 331:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 334:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 335:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 336:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 337:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 338:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 339:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 340:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 341:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 346:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 347:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 348:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 349:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 350:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 352:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 353:

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

  case 354:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 357:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 359:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 364:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 367:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 368:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 369:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 372:

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

  case 373:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 374:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 375:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 376:

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

  case 377:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 378:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 379:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 380:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 381:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 382:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 383:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 384:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 385:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 386:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 387:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 388:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 389:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 390:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 397:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 398:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 399:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 407:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 408:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 409:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 410:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 411:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 412:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 413:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 414:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 415:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 416:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 417:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 420:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 421:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 422:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 423:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 424:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 425:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 428:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 430:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 431:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 432:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 433:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 434:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 435:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 436:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 440:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 442:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 445:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
		;}
    break;

  case 447:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 448:

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

