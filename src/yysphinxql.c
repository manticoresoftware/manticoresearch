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
#define YYFINAL  198
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4536

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  150
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  137
/* YYNRULES -- Number of rules. */
#define YYNRULES  437
/* YYNRULES -- Number of states. */
#define YYNSTATES  796

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
     160,   162,   164,   166,   168,   170,   172,   176,   179,   181,
     183,   185,   194,   196,   206,   207,   210,   212,   216,   218,
     220,   222,   223,   227,   228,   231,   236,   248,   250,   254,
     256,   259,   260,   262,   265,   267,   272,   277,   282,   287,
     292,   297,   301,   307,   309,   313,   314,   316,   319,   321,
     325,   329,   334,   336,   340,   344,   350,   357,   361,   366,
     372,   376,   380,   384,   388,   392,   394,   400,   406,   412,
     416,   420,   424,   428,   432,   436,   440,   445,   449,   451,
     453,   458,   462,   466,   468,   470,   475,   480,   485,   489,
     491,   494,   496,   499,   501,   503,   507,   508,   513,   514,
     516,   518,   522,   523,   526,   527,   529,   535,   536,   538,
     542,   548,   550,   554,   556,   559,   562,   563,   565,   568,
     573,   574,   576,   579,   581,   585,   589,   593,   599,   606,
     610,   612,   616,   620,   622,   624,   626,   628,   630,   632,
     634,   637,   640,   644,   648,   652,   656,   660,   664,   668,
     672,   676,   680,   684,   688,   692,   696,   700,   704,   708,
     712,   716,   718,   720,   722,   726,   731,   736,   741,   746,
     751,   756,   761,   765,   772,   779,   783,   792,   807,   809,
     813,   815,   817,   821,   827,   829,   831,   833,   835,   838,
     839,   842,   844,   847,   850,   854,   856,   858,   860,   863,
     868,   873,   877,   882,   887,   889,   891,   892,   895,   900,
     905,   910,   914,   919,   924,   932,   938,   944,   954,   956,
     958,   960,   962,   964,   966,   970,   972,   974,   976,   978,
     980,   982,   984,   986,   988,   991,   999,  1001,  1003,  1004,
    1008,  1010,  1012,  1014,  1018,  1020,  1024,  1028,  1030,  1034,
    1036,  1038,  1040,  1044,  1047,  1048,  1051,  1053,  1057,  1061,
    1066,  1073,  1075,  1079,  1081,  1085,  1087,  1091,  1092,  1095,
    1097,  1101,  1105,  1106,  1108,  1110,  1112,  1116,  1118,  1120,
    1124,  1128,  1135,  1137,  1141,  1145,  1149,  1155,  1160,  1164,
    1168,  1170,  1172,  1174,  1176,  1178,  1180,  1182,  1184,  1192,
    1199,  1204,  1209,  1215,  1216,  1218,  1221,  1223,  1227,  1231,
    1234,  1238,  1245,  1246,  1248,  1250,  1253,  1256,  1259,  1261,
    1269,  1271,  1273,  1275,  1277,  1281,  1288,  1292,  1296,  1299,
    1303,  1305,  1309,  1312,  1316,  1320,  1328,  1334,  1340,  1342,
    1344,  1347,  1349,  1352,  1354,  1356,  1360,  1364,  1368,  1372,
    1374,  1375,  1378,  1380,  1383,  1385,  1387,  1391
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     151,     0,    -1,   152,    -1,   155,    -1,   155,   141,    -1,
     219,    -1,   231,    -1,   211,    -1,   212,    -1,   217,    -1,
     232,    -1,   241,    -1,   243,    -1,   244,    -1,   245,    -1,
     250,    -1,   255,    -1,   256,    -1,   260,    -1,   262,    -1,
     263,    -1,   264,    -1,   265,    -1,   266,    -1,   257,    -1,
     267,    -1,   269,    -1,   270,    -1,   271,    -1,   249,    -1,
     272,    -1,   273,    -1,   274,    -1,     3,    -1,    16,    -1,
      20,    -1,    21,    -1,    22,    -1,    23,    -1,    26,    -1,
      31,    -1,    35,    -1,    48,    -1,    51,    -1,    52,    -1,
      53,    -1,    54,    -1,    55,    -1,    65,    -1,    67,    -1,
      68,    -1,    70,    -1,    71,    -1,    72,    -1,    82,    -1,
      83,    -1,    84,    -1,    85,    -1,    87,    -1,    86,    -1,
      88,    -1,    90,    -1,    91,    -1,    94,    -1,    95,    -1,
      96,    -1,    98,    -1,   101,    -1,   104,    -1,   105,    -1,
     106,    -1,   107,    -1,   109,    -1,   110,    -1,   114,    -1,
     115,    -1,   116,    -1,   119,    -1,   120,    -1,   121,    -1,
     123,    -1,    66,    -1,   153,    -1,    77,    -1,   112,    -1,
     156,    -1,   155,   141,   156,    -1,   155,   286,    -1,   157,
      -1,   206,    -1,   158,    -1,    97,     3,   142,   142,   158,
     143,   159,   143,    -1,   165,    -1,    97,   166,    50,   142,
     162,   165,   143,   163,   164,    -1,    -1,   144,   160,    -1,
     161,    -1,   160,   144,   161,    -1,   154,    -1,     5,    -1,
      57,    -1,    -1,    80,    27,   189,    -1,    -1,    69,     5,
      -1,    69,     5,   144,     5,    -1,    97,   166,    50,   170,
     171,   181,   185,   184,   187,   191,   193,    -1,   167,    -1,
     166,   144,   167,    -1,   136,    -1,   169,   168,    -1,    -1,
     154,    -1,    18,   154,    -1,   199,    -1,    22,   142,   199,
     143,    -1,    71,   142,   199,   143,    -1,    73,   142,   199,
     143,    -1,   107,   142,   199,   143,    -1,    55,   142,   199,
     143,    -1,    35,   142,   136,   143,    -1,    54,   142,   143,
      -1,    35,   142,    41,   154,   143,    -1,   154,    -1,   170,
     144,   154,    -1,    -1,   172,    -1,   122,   173,    -1,   174,
      -1,   173,   125,   173,    -1,   142,   173,   143,    -1,    70,
     142,     8,   143,    -1,   175,    -1,   177,   128,   178,    -1,
     177,   129,   178,    -1,   177,    58,   142,   180,   143,    -1,
     177,   139,    58,   142,   180,   143,    -1,   177,    58,     9,
      -1,   177,   139,    58,     9,    -1,   177,    24,   178,   125,
     178,    -1,   177,   131,   178,    -1,   177,   130,   178,    -1,
     177,   132,   178,    -1,   177,   133,   178,    -1,   177,   128,
     179,    -1,   176,    -1,   177,    24,   179,   125,   179,    -1,
     177,    24,   178,   125,   179,    -1,   177,    24,   179,   125,
     178,    -1,   177,   131,   179,    -1,   177,   130,   179,    -1,
     177,   132,   179,    -1,   177,   133,   179,    -1,   177,   128,
       8,    -1,   177,   129,     8,    -1,   177,    64,    78,    -1,
     177,    64,   139,    78,    -1,   177,   129,   179,    -1,   154,
      -1,     4,    -1,    35,   142,   136,   143,    -1,    54,   142,
     143,    -1,   121,   142,   143,    -1,    57,    -1,   276,    -1,
      62,   142,   276,   143,    -1,    43,   142,   276,   143,    -1,
      25,   142,   276,   143,    -1,    45,   142,   143,    -1,     5,
      -1,   135,     5,    -1,     6,    -1,   135,     6,    -1,    14,
      -1,   178,    -1,   180,   144,   178,    -1,    -1,    53,   182,
      27,   183,    -1,    -1,     5,    -1,   177,    -1,   183,   144,
     177,    -1,    -1,    56,   175,    -1,    -1,   186,    -1,   123,
      53,    80,    27,   189,    -1,    -1,   188,    -1,    80,    27,
     189,    -1,    80,    27,    86,   142,   143,    -1,   190,    -1,
     189,   144,   190,    -1,   177,    -1,   177,    19,    -1,   177,
      39,    -1,    -1,   192,    -1,    69,     5,    -1,    69,     5,
     144,     5,    -1,    -1,   194,    -1,    79,   195,    -1,   196,
      -1,   195,   144,   196,    -1,   154,   128,   154,    -1,   154,
     128,     5,    -1,   154,   128,   142,   197,   143,    -1,   154,
     128,   154,   142,     8,   143,    -1,   154,   128,     8,    -1,
     198,    -1,   197,   144,   198,    -1,   154,   128,   178,    -1,
     154,    -1,     4,    -1,    57,    -1,     5,    -1,     6,    -1,
      14,    -1,     9,    -1,   135,   199,    -1,   139,   199,    -1,
     199,   134,   199,    -1,   199,   135,   199,    -1,   199,   136,
     199,    -1,   199,   137,   199,    -1,   199,   130,   199,    -1,
     199,   131,   199,    -1,   199,   127,   199,    -1,   199,   126,
     199,    -1,   199,   138,   199,    -1,   199,    42,   199,    -1,
     199,    74,   199,    -1,   199,   133,   199,    -1,   199,   132,
     199,    -1,   199,   128,   199,    -1,   199,   129,   199,    -1,
     199,   125,   199,    -1,   199,   124,   199,    -1,   142,   199,
     143,    -1,   145,   203,   146,    -1,   200,    -1,   276,    -1,
     279,    -1,   275,    64,    78,    -1,   275,    64,   139,    78,
      -1,     3,   142,   201,   143,    -1,    58,   142,   201,   143,
      -1,    62,   142,   201,   143,    -1,    25,   142,   201,   143,
      -1,    47,   142,   201,   143,    -1,    43,   142,   201,   143,
      -1,     3,   142,   143,    -1,    73,   142,   199,   144,   199,
     143,    -1,    71,   142,   199,   144,   199,   143,    -1,   121,
     142,   143,    -1,     3,   142,   199,    49,   154,    58,   275,
     143,    -1,    93,   142,   199,   144,   199,   144,   142,   201,
     143,   144,   142,   201,   143,   143,    -1,   202,    -1,   201,
     144,   202,    -1,   199,    -1,     8,    -1,   204,   128,   205,
      -1,   203,   144,   204,   128,   205,    -1,   154,    -1,    58,
      -1,   178,    -1,   154,    -1,   102,   208,    -1,    -1,    68,
       8,    -1,   120,    -1,   105,   207,    -1,    72,   207,    -1,
      16,   105,   207,    -1,    85,    -1,    82,    -1,    84,    -1,
     110,   193,    -1,    16,     8,   105,   207,    -1,    16,   154,
     105,   207,    -1,   209,   154,   105,    -1,   209,   154,   210,
     100,    -1,   209,   154,    14,   100,    -1,    59,    -1,   108,
      -1,    -1,    30,     5,    -1,    99,   153,   128,   214,    -1,
      99,   153,   128,   213,    -1,    99,   153,   128,    78,    -1,
      99,    77,   215,    -1,    99,    10,   128,   215,    -1,    99,
      29,    99,   215,    -1,    99,    52,     9,   128,   142,   180,
     143,    -1,    99,    52,   153,   128,   213,    -1,    99,    52,
     153,   128,     5,    -1,    99,    59,   154,    52,     9,   128,
     142,   180,   143,    -1,   154,    -1,     8,    -1,   113,    -1,
      46,    -1,   178,    -1,   216,    -1,   215,   135,   216,    -1,
     154,    -1,    78,    -1,     8,    -1,     5,    -1,     6,    -1,
      33,    -1,    95,    -1,   218,    -1,    23,    -1,   104,   112,
      -1,   220,    63,   154,   221,   118,   224,   228,    -1,    60,
      -1,    92,    -1,    -1,   142,   223,   143,    -1,   154,    -1,
      57,    -1,   222,    -1,   223,   144,   222,    -1,   225,    -1,
     224,   144,   225,    -1,   142,   226,   143,    -1,   227,    -1,
     226,   144,   227,    -1,   178,    -1,   179,    -1,     8,    -1,
     142,   180,   143,    -1,   142,   143,    -1,    -1,    79,   229,
      -1,   230,    -1,   229,   144,   230,    -1,     3,   128,     8,
      -1,    38,    50,   170,   172,    -1,    28,   154,   142,   233,
     236,   143,    -1,   234,    -1,   233,   144,   234,    -1,   227,
      -1,   142,   235,   143,    -1,     8,    -1,   235,   144,     8,
      -1,    -1,   144,   237,    -1,   238,    -1,   237,   144,   238,
      -1,   227,   239,   240,    -1,    -1,    18,    -1,   154,    -1,
      69,    -1,   242,   154,   207,    -1,    40,    -1,    39,    -1,
     102,   109,   207,    -1,   102,    37,   207,    -1,   117,   170,
      99,   246,   172,   193,    -1,   247,    -1,   246,   144,   247,
      -1,   154,   128,   178,    -1,   154,   128,   179,    -1,   154,
     128,   142,   180,   143,    -1,   154,   128,   142,   143,    -1,
     276,   128,   178,    -1,   276,   128,   179,    -1,    62,    -1,
      25,    -1,    47,    -1,    26,    -1,    75,    -1,    76,    -1,
      66,    -1,   106,    -1,    17,   108,   154,    15,    32,   154,
     248,    -1,    17,   108,   154,    44,    32,   154,    -1,    17,
      96,   154,    89,    -1,   102,   258,   119,   251,    -1,   102,
     258,   119,    68,     8,    -1,    -1,   252,    -1,   122,   253,
      -1,   254,    -1,   253,   124,   254,    -1,   154,   128,     8,
      -1,   102,    31,    -1,   102,    29,    99,    -1,    99,   258,
     112,    65,    67,   259,    -1,    -1,    52,    -1,   101,    -1,
      88,   116,    -1,    88,    34,    -1,    91,    88,    -1,    98,
      -1,    36,    51,   154,    94,   261,   103,     8,    -1,    61,
      -1,    25,    -1,    47,    -1,   106,    -1,    44,    51,   154,
      -1,    20,    59,   154,   111,    96,   154,    -1,    48,    96,
     154,    -1,    48,    87,   154,    -1,    48,    21,    -1,    97,
     268,   191,    -1,    10,    -1,    10,   147,   154,    -1,    97,
     199,    -1,   114,    96,   154,    -1,    81,    59,   154,    -1,
      36,    83,   154,   115,     8,   103,     8,    -1,    44,    83,
     154,   115,     8,    -1,    90,    84,    50,   103,     8,    -1,
     276,    -1,   154,    -1,   154,   277,    -1,   278,    -1,   277,
     278,    -1,    13,    -1,    14,    -1,   148,   199,   149,    -1,
     148,     8,   149,    -1,   199,   128,   280,    -1,   280,   128,
     199,    -1,     8,    -1,    -1,   282,   285,    -1,    27,    -1,
     284,   168,    -1,   199,    -1,   283,    -1,   285,   144,   283,
      -1,    45,   285,   281,   187,   191,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   176,   176,   177,   178,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   220,   221,   221,   221,   221,   221,   221,
     222,   222,   222,   222,   223,   223,   223,   223,   223,   224,
     224,   224,   224,   224,   225,   225,   225,   225,   225,   225,
     225,   226,   226,   226,   226,   226,   227,   227,   227,   227,
     227,   228,   228,   228,   228,   228,   228,   229,   229,   229,
     229,   230,   234,   234,   234,   240,   241,   242,   246,   247,
     251,   252,   260,   261,   268,   270,   274,   278,   285,   286,
     287,   291,   304,   311,   313,   318,   327,   343,   344,   348,
     349,   352,   354,   355,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   371,   372,   375,   377,   381,   385,   386,
     387,   391,   396,   400,   407,   415,   423,   432,   437,   442,
     447,   452,   457,   462,   467,   472,   477,   482,   487,   492,
     497,   502,   507,   512,   517,   522,   527,   535,   539,   540,
     545,   551,   557,   563,   569,   570,   571,   572,   573,   577,
     578,   589,   590,   591,   595,   601,   607,   609,   612,   614,
     621,   625,   631,   633,   639,   641,   645,   656,   658,   662,
     666,   673,   674,   678,   679,   680,   683,   685,   689,   694,
     701,   703,   707,   711,   712,   716,   721,   726,   732,   737,
     745,   750,   757,   767,   768,   769,   770,   771,   772,   773,
     774,   775,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   797,   798,   799,   800,   804,   805,   806,   807,
     808,   809,   810,   811,   812,   813,   814,   815,   819,   820,
     824,   825,   829,   830,   834,   835,   839,   840,   846,   849,
     851,   855,   856,   857,   858,   859,   860,   861,   862,   863,
     868,   873,   878,   883,   892,   893,   896,   898,   906,   911,
     916,   921,   922,   923,   927,   932,   937,   942,   951,   952,
     956,   957,   958,   970,   971,   975,   976,   977,   978,   979,
     986,   987,   988,   992,   993,   999,  1007,  1008,  1011,  1013,
    1017,  1018,  1022,  1023,  1027,  1028,  1032,  1036,  1037,  1041,
    1042,  1043,  1044,  1045,  1048,  1049,  1053,  1054,  1058,  1064,
    1074,  1082,  1086,  1093,  1094,  1101,  1111,  1117,  1119,  1123,
    1128,  1132,  1139,  1141,  1145,  1146,  1152,  1160,  1161,  1167,
    1171,  1177,  1185,  1186,  1190,  1204,  1210,  1214,  1219,  1233,
    1244,  1245,  1246,  1247,  1248,  1249,  1250,  1251,  1255,  1263,
    1270,  1281,  1285,  1292,  1293,  1297,  1301,  1302,  1306,  1310,
    1317,  1324,  1330,  1331,  1332,  1336,  1337,  1338,  1339,  1345,
    1356,  1357,  1358,  1359,  1364,  1375,  1387,  1396,  1405,  1415,
    1423,  1424,  1428,  1438,  1449,  1460,  1471,  1481,  1492,  1493,
    1497,  1500,  1501,  1505,  1506,  1507,  1508,  1512,  1513,  1517,
    1522,  1524,  1528,  1537,  1541,  1549,  1550,  1554
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
  "const_int", "const_float", "const_list", "opt_group_clause", "opt_int", 
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
  "facet_item", "facet_expr", "facet_items_list", "facet_stmt", 0
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
     152,   152,   152,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   154,   154,   154,   155,   155,   155,   156,   156,
     157,   157,   158,   158,   159,   159,   160,   160,   161,   161,
     161,   162,   163,   164,   164,   164,   165,   166,   166,   167,
     167,   168,   168,   168,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   170,   170,   171,   171,   172,   173,   173,
     173,   174,   174,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   176,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   178,
     178,   179,   179,   179,   180,   180,   181,   181,   182,   182,
     183,   183,   184,   184,   185,   185,   186,   187,   187,   188,
     188,   189,   189,   190,   190,   190,   191,   191,   192,   192,
     193,   193,   194,   195,   195,   196,   196,   196,   196,   196,
     197,   197,   198,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   200,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   201,   201,
     202,   202,   203,   203,   204,   204,   205,   205,   206,   207,
     207,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   209,   209,   210,   210,   211,   211,
     211,   211,   211,   211,   212,   212,   212,   212,   213,   213,
     214,   214,   214,   215,   215,   216,   216,   216,   216,   216,
     217,   217,   217,   218,   218,   219,   220,   220,   221,   221,
     222,   222,   223,   223,   224,   224,   225,   226,   226,   227,
     227,   227,   227,   227,   228,   228,   229,   229,   230,   231,
     232,   233,   233,   234,   234,   235,   235,   236,   236,   237,
     237,   238,   239,   239,   240,   240,   241,   242,   242,   243,
     244,   245,   246,   246,   247,   247,   247,   247,   247,   247,
     248,   248,   248,   248,   248,   248,   248,   248,   249,   249,
     249,   250,   250,   251,   251,   252,   253,   253,   254,   255,
     256,   257,   258,   258,   258,   259,   259,   259,   259,   260,
     261,   261,   261,   261,   262,   263,   264,   265,   266,   267,
     268,   268,   269,   270,   271,   272,   273,   274,   275,   275,
     276,   277,   277,   278,   278,   278,   278,   279,   279,   280,
     281,   281,   282,   283,   284,   285,   285,   286
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
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     8,     1,     9,     0,     2,     1,     3,     1,     1,
       1,     0,     3,     0,     2,     4,    11,     1,     3,     1,
       2,     0,     1,     2,     1,     4,     4,     4,     4,     4,
       4,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       3,     4,     1,     3,     3,     5,     6,     3,     4,     5,
       3,     3,     3,     3,     3,     1,     5,     5,     5,     3,
       3,     3,     3,     3,     3,     3,     4,     3,     1,     1,
       4,     3,     3,     1,     1,     4,     4,     4,     3,     1,
       2,     1,     2,     1,     1,     3,     0,     4,     0,     1,
       1,     3,     0,     2,     0,     1,     5,     0,     1,     3,
       5,     1,     3,     1,     2,     2,     0,     1,     2,     4,
       0,     1,     2,     1,     3,     3,     3,     5,     6,     3,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     3,     4,     4,     4,     4,     4,
       4,     4,     3,     6,     6,     3,     8,    14,     1,     3,
       1,     1,     3,     5,     1,     1,     1,     1,     2,     0,
       2,     1,     2,     2,     3,     1,     1,     1,     2,     4,
       4,     3,     4,     4,     1,     1,     0,     2,     4,     4,
       4,     3,     4,     4,     7,     5,     5,     9,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     7,     1,     1,     0,     3,
       1,     1,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     3,     2,     0,     2,     1,     3,     3,     4,
       6,     1,     3,     1,     3,     1,     3,     0,     2,     1,
       3,     3,     0,     1,     1,     1,     3,     1,     1,     3,
       3,     6,     1,     3,     3,     3,     5,     4,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     7,     6,
       4,     4,     5,     0,     1,     2,     1,     3,     3,     2,
       3,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     1,     1,     3,     6,     3,     3,     2,     3,
       1,     3,     2,     3,     3,     7,     5,     5,     1,     1,
       2,     1,     2,     1,     1,     3,     3,     3,     3,     1,
       0,     2,     1,     2,     1,     1,     3,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   313,     0,   310,     0,     0,   358,   357,
       0,     0,   316,     0,     0,   317,   311,     0,   392,   392,
       0,     0,     0,     0,     2,     3,    85,    88,    90,    92,
      89,     7,     8,     9,   312,     5,     0,     6,    10,    11,
       0,    12,    13,    14,    29,    15,    16,    17,    24,    18,
      19,    20,    21,    22,    23,    25,    26,    27,    28,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    81,    49,    50,    51,    52,    53,    83,    54,    55,
      56,    57,    59,    58,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    84,    74,
      75,    76,    77,    78,    79,    80,    82,     0,     0,     0,
       0,     0,     0,   408,     0,     0,     0,     0,    33,   214,
     216,   217,   429,   219,   410,   218,    37,     0,    41,     0,
       0,    46,    47,   215,     0,     0,    52,     0,     0,    71,
      79,     0,   109,     0,     0,     0,   213,     0,   107,   111,
     114,   241,   196,     0,   242,   243,     0,     0,     0,    44,
       0,     0,    67,     0,     0,     0,     0,   389,   269,   393,
     284,   269,   276,   277,   275,   394,   269,   285,   269,   200,
     271,   268,     0,     0,   314,     0,   123,     0,     1,     0,
       4,    87,     0,   269,     0,     0,     0,     0,     0,     0,
       0,   404,     0,   407,   406,   414,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    33,    52,     0,   220,   221,     0,   265,
     264,     0,     0,   423,   424,     0,   420,   421,     0,     0,
       0,   112,   110,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   409,   197,     0,     0,     0,     0,     0,     0,     0,
     308,   309,   307,   306,   305,   291,   303,     0,     0,     0,
     269,     0,   390,     0,   360,   273,   272,   359,     0,   278,
     201,   286,   383,   413,     0,     0,   434,   435,   111,   430,
       0,     0,    86,   318,   356,   380,     0,     0,     0,   169,
     171,   331,   173,     0,     0,   329,   330,   343,   347,   341,
       0,     0,     0,   339,     0,     0,   261,     0,   252,   260,
       0,   258,   411,     0,   260,     0,     0,     0,     0,     0,
     121,     0,     0,     0,     0,     0,     0,     0,   255,     0,
       0,     0,   239,     0,   240,     0,   429,     0,   422,   101,
     125,   108,   114,   113,   231,   232,   238,   237,   229,   228,
     235,   427,   236,   226,   227,   234,   233,   222,   223,   224,
     225,   230,   198,   244,     0,   428,   292,   293,     0,     0,
       0,     0,   299,   301,   290,   300,     0,   298,   302,   289,
     288,     0,   269,   274,   269,   270,     0,   202,   203,     0,
       0,   281,     0,     0,     0,   381,   384,     0,     0,   362,
       0,   124,   433,   432,     0,   187,     0,     0,     0,     0,
       0,     0,   170,   172,   345,   333,   174,     0,     0,     0,
       0,   401,   402,   400,   403,     0,     0,   159,     0,    41,
       0,     0,    46,   163,     0,    51,    79,     0,   158,   127,
     128,   132,   145,     0,   164,   416,   417,     0,     0,     0,
     246,     0,   115,   249,     0,   120,   251,   250,   119,   247,
     248,   116,     0,   117,     0,     0,   118,     0,     0,     0,
     267,   266,   262,   426,   425,     0,   176,   126,     0,   245,
       0,   296,   295,     0,   304,     0,   279,   280,     0,     0,
     283,   287,   282,   382,     0,   385,   386,     0,     0,   200,
       0,   436,     0,   196,   188,   431,   321,   320,   322,     0,
       0,     0,   379,   405,   332,     0,   344,     0,   352,   342,
     348,   349,   340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    94,     0,   259,   122,     0,
       0,     0,     0,     0,     0,   178,   184,   199,     0,     0,
       0,     0,   398,   391,   206,   209,     0,   205,   204,     0,
       0,     0,   364,   365,   363,   361,   368,   369,     0,   437,
     319,     0,     0,   334,   324,   371,   373,   372,   370,   376,
     374,   375,   377,   378,   175,   346,   353,     0,     0,   399,
     415,     0,     0,     0,     0,   168,   161,     0,     0,   162,
     130,   129,     0,     0,   137,     0,   155,     0,   153,   133,
     144,   154,   134,   157,   141,   150,   140,   149,   142,   151,
     143,   152,     0,     0,     0,     0,   254,   253,     0,   263,
       0,     0,   179,     0,     0,   182,   185,   294,     0,   396,
     395,   397,     0,     0,   210,     0,   388,   387,   367,     0,
      59,   193,   189,   191,   323,     0,     0,   327,     0,     0,
     315,   355,   354,   351,   352,   350,   167,   160,   166,   165,
     131,     0,     0,     0,   156,   138,     0,    99,   100,    98,
      95,    96,    91,   419,     0,   418,     0,     0,     0,   103,
       0,     0,     0,   187,     0,     0,   207,     0,     0,   366,
       0,   194,   195,     0,   326,     0,     0,   335,   336,   325,
     139,   147,   148,   146,   135,     0,     0,   256,     0,     0,
       0,    93,   180,   177,     0,   183,   196,   297,   212,   211,
     208,   190,   192,   328,     0,     0,   136,    97,     0,   102,
     104,     0,     0,   200,   338,   337,     0,     0,   181,   186,
     106,     0,   105,     0,     0,   257
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    23,    24,   116,   156,    25,    26,    27,    28,   664,
     720,   721,   505,   729,   761,    29,   157,   158,   252,   159,
     370,   506,   333,   469,   470,   471,   472,   473,   446,   326,
     447,   586,   673,   763,   733,   675,   676,   533,   534,   692,
     693,   271,   272,   299,   300,   417,   418,   683,   684,   344,
     161,   340,   341,   241,   242,   502,    30,   294,   191,   192,
     422,    31,    32,   409,   410,   285,   286,    33,    34,    35,
      36,   438,   538,   539,   613,   614,   696,   327,   700,   747,
     748,    37,    38,   328,   329,   448,   450,   550,   551,   627,
     703,    39,    40,    41,    42,    43,   428,   429,   623,    44,
      45,   425,   426,   525,   526,    46,    47,    48,   174,   593,
      49,   455,    50,    51,    52,    53,    54,    55,   162,    56,
      57,    58,    59,    60,    61,   163,   164,   246,   247,   165,
     166,   435,   436,   307,   308,   309,   201
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -676
static const short yypact[] =
{
    4354,   101,    49,  -676,  3825,  -676,    13,    39,  -676,  -676,
      16,    36,  -676,    72,    29,  -676,  -676,   740,  2948,   453,
      78,   103,  3825,   172,  -676,   -11,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,    63,  -676,  -676,  -676,
    3825,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  3825,  3825,  3825,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,    62,  3825,  3825,
    3825,  3825,  3825,  -676,  3825,  3825,  3825,   158,    94,  -676,
    -676,  -676,  -676,  -676,    64,  -676,   106,   117,   123,   140,
     146,   150,   155,  -676,   157,   161,   163,   180,   181,   182,
     183,  1598,  -676,  1598,  1598,  3283,    30,    31,  -676,  3391,
      95,  -676,   252,   152,   262,  -676,   199,   201,   231,  3933,
    3825,  2840,   133,   203,   221,  3501,   235,  -676,   267,  -676,
    -676,   267,  -676,  -676,  -676,  -676,   267,  -676,   267,   258,
    -676,  -676,  3825,   222,  -676,  3825,  -676,   -43,  -676,  1598,
       1,  -676,  3825,   267,   251,    67,   232,    19,   250,   233,
     -39,  -676,   234,  -676,  -676,  -676,   244,   883,  3825,  1598,
    1741,     7,  1741,  1741,   207,  1598,  1741,  1741,  1598,  1598,
    1598,  1598,   208,   210,   211,   212,  -676,  -676,  4067,  -676,
    -676,    69,   228,  -676,  -676,  1884,    48,  -676,  2242,  1026,
    3825,  -676,  -676,  1598,  1598,  1598,  1598,  1598,  1598,  1598,
    1598,  1598,  1598,  1598,  1598,  1598,  1598,  1598,  1598,  1598,
     355,  -676,  -676,   -15,  1598,  2840,  2840,   254,   257,   310,
    -676,  -676,  -676,  -676,  -676,   255,  -676,  2365,   314,   281,
      34,   282,  -676,   381,  -676,  -676,  -676,  -676,  3825,  -676,
    -676,    74,    18,  -676,  3825,  3825,  4371,  -676,  3391,    12,
    1169,   273,  -676,   249,  -676,  -676,   360,   362,   300,  -676,
    -676,  -676,  -676,   129,     9,  -676,  -676,  -676,   256,  -676,
      81,   390,  2011,  -676,   391,   393,   277,  1312,  -676,  4355,
      91,  -676,  -676,  4100,  4371,    96,  3825,   263,    98,   107,
    -676,  4133,   109,   111,   557,   578,  3934,  4166,  -676,  1455,
    1598,  1598,  -676,  3283,  -676,  2486,   259,   239,  -676,  -676,
     -39,  -676,  4371,  -676,  -676,  -676,  4385,  4398,   452,  3409,
     878,  -676,   878,   126,   126,   126,   126,    45,    45,  -676,
    -676,  -676,   265,  -676,   329,   878,   255,   255,   269,  3067,
     403,  2840,  -676,  -676,  -676,  -676,   412,  -676,  -676,  -676,
    -676,   351,   267,  -676,   267,  -676,   291,   278,  -676,   321,
     418,  -676,   324,   417,  3825,  -676,  -676,    32,   -32,  -676,
     298,  -676,  -676,  -676,  1598,   347,  1598,  3609,   311,  3825,
    3825,  3825,  -676,  -676,  -676,  -676,  -676,   114,   124,    19,
     285,  -676,  -676,  -676,  -676,   328,   352,  -676,   290,   315,
     318,   320,   322,  -676,   323,   325,   330,  2011,    48,   331,
    -676,  -676,  -676,   179,  -676,  -676,  -676,  1026,   334,  3825,
    -676,  1741,  -676,  -676,   335,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  1598,  -676,  1598,  1598,  -676,  3967,  4002,   338,
    -676,  -676,  -676,  -676,  -676,   374,   420,  -676,   469,  -676,
      15,  -676,  -676,   357,  -676,   119,  -676,  -676,  2134,  3825,
    -676,  -676,  -676,  -676,   358,   356,  -676,    35,  3825,   258,
      66,  -676,   456,   252,  -676,   343,  -676,  -676,  -676,   130,
     337,   550,  -676,  -676,  -676,    15,  -676,   480,     4,  -676,
     345,  -676,  -676,   483,   484,  3825,   370,  3825,   350,   365,
    3825,   490,   366,   -34,  2011,    66,    17,    -3,    60,    71,
      66,    66,    66,    66,   458,   367,   459,  -676,  -676,  4199,
    4232,  4032,  2486,  1026,   375,   505,   398,  -676,   132,   385,
      76,   435,  -676,  -676,  -676,  -676,  3825,   386,  -676,   521,
    3825,    10,  -676,  -676,  -676,  -676,  -676,  -676,  2607,  -676,
    -676,  3609,    23,   -44,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  3717,    23,  -676,
    -676,    48,   387,   388,   389,  -676,  -676,   396,   397,  -676,
    -676,  -676,   408,   409,  -676,    15,  -676,   463,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,    21,  3175,   399,  3825,  -676,  -676,   394,  -676,
      42,   467,  -676,   522,   495,   494,  -676,  -676,    15,  -676,
    -676,  -676,   424,   134,  -676,   547,  -676,  -676,  -676,   136,
     415,    54,   421,  -676,  -676,    14,   141,  -676,   561,   337,
    -676,  -676,  -676,  -676,   548,  -676,  -676,  -676,  -676,  -676,
    -676,    66,    66,   143,  -676,  -676,    15,  -676,  -676,  -676,
     423,  -676,  -676,    48,   425,  -676,  1741,  3825,   542,   501,
    2721,   491,  2721,   347,   147,    15,  -676,  3825,   431,  -676,
     434,  -676,  -676,  2721,  -676,    23,   465,   447,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,   171,  3175,  -676,   173,  2721,
     589,  -676,  -676,   451,   571,  -676,   252,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,   592,   561,  -676,  -676,   457,   421,
     461,  2721,  2721,   258,  -676,  -676,   460,   598,  -676,   421,
    -676,  1741,  -676,   176,   466,  -676
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -676,  -676,  -676,    -5,    -4,  -676,   406,  -676,   271,  -676,
    -676,  -146,  -676,  -676,  -676,   108,    28,   368,   306,  -676,
      56,  -676,  -301,  -435,  -676,  -117,  -676,  -597,   -69,  -518,
    -504,  -676,  -676,  -676,  -676,  -676,  -676,  -115,  -676,  -675,
    -121,  -528,  -676,  -527,  -676,  -676,   105,  -676,  -110,   185,
    -676,  -219,   148,  -676,   270,    46,  -676,  -165,  -676,  -676,
    -676,  -676,  -676,   236,  -676,    -6,   229,  -676,  -676,  -676,
    -676,  -676,    25,  -676,  -676,   -67,  -676,  -439,  -676,  -676,
    -141,  -676,  -676,  -676,   188,  -676,  -676,  -676,    11,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,   110,  -676,  -676,
    -676,  -676,  -676,  -676,    40,  -676,  -676,  -676,   622,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,   -21,  -262,  -676,   400,  -676,
     392,  -676,  -676,   213,  -676,   209,  -676
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -430
static const short yytable[] =
{
     117,   345,   605,   348,   349,   609,   588,   352,   353,   603,
     548,   691,   607,   173,   319,   319,   295,   444,   196,   319,
     319,   296,   626,   297,   319,   320,   644,   321,   319,   320,
     715,   321,   563,   322,   199,   698,   203,   322,   314,   433,
     319,   320,   430,   243,   244,   243,   244,   643,   346,   322,
     650,   653,   655,   657,   659,   661,   304,   123,   204,   205,
     206,   243,   244,   393,   118,   319,   320,   121,   648,   507,
     474,   319,   320,   741,   322,   646,   319,   320,   197,   651,
     322,   248,   316,   332,   779,   322,   423,   253,   419,   120,
     332,   564,   727,   742,  -419,  -412,   119,   689,   310,   122,
     699,   305,   293,   311,   420,   305,   451,   789,    64,   640,
     679,   317,   528,   127,   208,   209,   196,   211,   212,   254,
     213,   214,   215,   124,   394,   413,   202,   529,   452,   641,
     200,   126,   125,   762,   442,   443,   647,   253,   325,   -69,
     424,   713,   453,   347,   406,   406,   691,  -343,  -343,   406,
     406,   240,   445,   688,   323,   251,   434,   445,   323,   645,
     527,   324,   691,   716,   278,   695,   279,   284,   253,   254,
     323,   291,   198,   697,   734,   249,   210,   601,   245,   421,
     245,   267,   268,   269,   788,   691,   249,   454,   301,   704,
     194,   303,   680,   751,   753,   323,   245,    62,   313,   195,
     254,   323,   160,   565,   207,   474,   323,   590,   216,    63,
     591,   218,   755,   363,   342,   364,   273,   592,   408,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   480,   481,   217,   566,   783,   483,
     481,   486,   481,   567,   196,  -394,   373,   516,   219,   517,
     487,   481,   489,   481,   490,   481,   790,   544,   545,   220,
     265,   266,   267,   268,   269,   221,   430,   546,   547,   396,
     397,   284,   284,   610,   611,   677,   545,   736,   737,   739,
     545,   253,   222,   407,   744,   745,   754,   545,   223,   175,
     767,   545,   224,   632,   416,   634,   501,   225,   637,   226,
     427,   431,   474,   227,   251,   228,   773,   568,   569,   570,
     571,   572,   573,   254,   776,   545,   778,   481,   574,   794,
     481,   270,   229,   230,   231,   232,  -418,   274,   468,   275,
     276,   287,   180,   288,   292,   293,   236,   298,   237,   238,
     315,   302,   484,   318,   330,   181,   474,   335,   331,   334,
     350,   358,   359,   360,   361,   182,   365,   183,   184,   240,
     392,   500,   400,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   186,   411,
     325,   187,   398,   189,   306,   399,   412,   414,   504,   415,
     401,   437,   439,   190,   440,   407,   441,   284,   456,   475,
     449,   476,   339,   725,   343,  -429,   485,   509,   503,   508,
     351,   510,   513,   354,   355,   356,   357,   442,   515,   518,
     524,   520,   519,   521,   522,   523,   530,   532,   552,   540,
     367,   553,   555,   537,   372,   541,   542,   543,   374,   375,
     376,   377,   378,   379,   380,   382,   383,   384,   385,   386,
     387,   388,   389,   390,   391,   554,   564,   556,   602,   395,
     557,   606,   558,   468,   559,   560,   582,   561,   474,   175,
     474,   583,   562,   585,   587,   576,   624,   575,   578,   612,
     600,   474,   176,   608,   177,   589,   599,   434,   625,   628,
     178,   629,   630,   635,   253,   372,   642,   474,   638,   649,
     652,   654,   656,   658,   660,   179,   633,   758,   636,   639,
     672,   663,   180,   501,   597,   416,   662,   665,   671,   474,
     474,   674,   238,   681,   427,   181,   254,   678,   685,   686,
     706,   707,   708,   711,   712,   182,   726,   183,   184,   709,
     710,   714,   722,   325,   339,   497,   498,   728,   731,   730,
     732,   631,   735,   631,   185,   738,   631,   740,   186,   325,
     468,   187,   188,   189,   746,   743,   626,   756,   757,   759,
     760,   764,   793,   190,   770,   615,   616,   771,   500,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   775,   682,   774,   780,   781,   524,   617,   782,   253,
     784,   786,   791,   792,   468,   787,   312,   537,   478,   795,
     777,   670,   618,   584,   432,   765,   619,   371,   766,   306,
     253,   306,   772,   702,   598,   620,   621,   769,   669,   577,
     514,   254,   749,   499,   785,   512,   694,   549,   604,   705,
     687,   193,   750,   752,   724,   535,   368,   531,     0,     0,
       0,   381,   254,     0,     0,     0,   622,     0,     0,   719,
       0,   723,   372,     0,     0,     0,   768,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   325,   579,     0,   580,
     581,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,     0,     0,     0,     0,
     491,   492,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
       0,   493,   494,   196,     0,     0,   468,     0,   468,     0,
       0,     0,     0,   682,     0,     0,     0,     0,     0,   468,
       0,     0,     0,   128,   129,   130,   131,     0,   132,   133,
     134,     0,   719,     0,   135,   468,    66,     0,     0,     0,
      67,    68,   136,    70,     0,   137,    71,     0,   372,     0,
       0,    72,     0,     0,     0,   138,     0,   468,   468,     0,
       0,     0,     0,   139,     0,     0,     0,   140,    74,     0,
       0,    75,    76,    77,   141,   142,     0,   143,   144,     0,
       0,     0,   145,     0,     0,    80,    81,    82,    83,     0,
      84,   146,    86,   147,     0,     0,     0,    87,     0,     0,
       0,     0,    88,    89,    90,    91,    92,    93,    94,     0,
      95,    96,     0,   148,    97,    98,    99,     0,   100,     0,
       0,   101,     0,     0,   102,   103,   104,   149,     0,   106,
     107,     0,   108,     0,   109,   110,   111,     0,     0,   112,
     113,   150,     0,   115,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   151,   152,     0,     0,   153,
       0,     0,   154,     0,     0,   155,   233,   129,   130,   131,
       0,   336,   133,     0,     0,     0,     0,   135,     0,    66,
       0,     0,     0,    67,    68,    69,    70,     0,   137,    71,
       0,     0,     0,     0,    72,     0,     0,     0,    73,     0,
     253,     0,     0,     0,     0,     0,   139,     0,     0,     0,
     140,    74,     0,     0,    75,    76,    77,    78,    79,     0,
     143,   144,     0,     0,     0,   145,     0,     0,    80,    81,
      82,    83,   254,    84,   234,    86,   235,     0,     0,     0,
      87,     0,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,   148,    97,    98,    99,
       0,   100,     0,     0,   101,     0,     0,   102,   103,   104,
     105,     0,   106,   107,     0,   108,     0,   109,   110,   111,
       0,     0,   112,   113,   150,     0,   115,     0,   261,   262,
     263,   264,   265,   266,   267,   268,   269,     0,   151,     0,
       0,     0,   153,     0,     0,   337,   338,     0,   155,   233,
     129,   130,   131,     0,   132,   133,     0,     0,     0,     0,
     135,     0,    66,     0,     0,     0,    67,    68,   136,    70,
       0,   137,    71,     0,     0,     0,     0,    72,     0,     0,
       0,   138,     0,     0,     0,     0,     0,     0,     0,   139,
       0,     0,     0,   140,    74,     0,     0,    75,    76,    77,
     141,   142,     0,   143,   144,     0,     0,     0,   145,     0,
       0,    80,    81,    82,    83,     0,    84,   146,    86,   147,
       0,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,    93,    94,     0,    95,    96,     0,   148,
      97,    98,    99,     0,   100,     0,     0,   101,     0,     0,
     102,   103,   104,   149,     0,   106,   107,     0,   108,     0,
     109,   110,   111,     0,     0,   112,   113,   150,     0,   115,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   151,   152,     0,     0,   153,     0,     0,   154,     0,
       0,   155,   128,   129,   130,   131,     0,   132,   133,     0,
       0,     0,     0,   135,     0,    66,     0,     0,     0,    67,
      68,   136,    70,     0,   137,    71,     0,     0,     0,     0,
      72,     0,     0,     0,   138,     0,     0,     0,     0,     0,
       0,     0,   139,     0,     0,     0,   140,    74,     0,     0,
      75,    76,    77,   141,   142,     0,   143,   144,     0,     0,
       0,   145,     0,     0,    80,    81,    82,    83,     0,    84,
     146,    86,   147,     0,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,     0,    95,
      96,     0,   148,    97,    98,    99,     0,   100,     0,     0,
     101,     0,     0,   102,   103,   104,   149,     0,   106,   107,
       0,   108,     0,   109,   110,   111,     0,     0,   112,   113,
     150,     0,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   151,   152,     0,     0,   153,     0,
       0,   154,     0,     0,   155,   233,   129,   130,   131,     0,
     132,   133,     0,     0,     0,     0,   135,     0,    66,     0,
       0,     0,    67,    68,    69,    70,     0,   137,    71,     0,
       0,     0,     0,    72,     0,     0,     0,    73,     0,     0,
       0,     0,     0,     0,     0,   139,     0,     0,     0,   140,
      74,     0,     0,    75,    76,    77,    78,    79,     0,   143,
     144,     0,     0,     0,   145,     0,     0,    80,    81,    82,
      83,     0,    84,   234,    86,   235,     0,     0,     0,    87,
       0,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,     0,    95,    96,     0,   148,    97,    98,    99,   477,
     100,     0,     0,   101,     0,     0,   102,   103,   104,   105,
       0,   106,   107,     0,   108,     0,   109,   110,   111,     0,
       0,   112,   113,   150,     0,   115,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   151,     0,     0,
       0,   153,     0,     0,   154,     0,     0,   155,   233,   129,
     130,   131,     0,   336,   133,     0,     0,     0,     0,   135,
       0,    66,     0,     0,     0,    67,    68,    69,    70,     0,
     137,    71,     0,     0,     0,     0,    72,     0,     0,     0,
      73,     0,     0,     0,     0,     0,     0,     0,   139,     0,
       0,     0,   140,    74,     0,     0,    75,    76,    77,    78,
      79,     0,   143,   144,     0,     0,     0,   145,     0,     0,
      80,    81,    82,    83,     0,    84,   234,    86,   235,     0,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,   148,    97,
      98,    99,     0,   100,     0,     0,   101,     0,     0,   102,
     103,   104,   105,     0,   106,   107,     0,   108,     0,   109,
     110,   111,     0,     0,   112,   113,   150,     0,   115,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     151,     0,     0,     0,   153,     0,     0,   154,   338,     0,
     155,   233,   129,   130,   131,     0,   132,   133,     0,     0,
       0,     0,   135,     0,    66,     0,     0,     0,    67,    68,
      69,    70,     0,   137,    71,     0,     0,     0,     0,    72,
       0,     0,     0,    73,     0,     0,     0,     0,     0,     0,
       0,   139,     0,     0,     0,   140,    74,     0,     0,    75,
      76,    77,    78,    79,     0,   143,   144,     0,     0,     0,
     145,     0,     0,    80,    81,    82,    83,     0,    84,   234,
      86,   235,     0,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,    93,    94,     0,    95,    96,
       0,   148,    97,    98,    99,     0,   100,     0,     0,   101,
       0,     0,   102,   103,   104,   105,     0,   106,   107,     0,
     108,     0,   109,   110,   111,     0,     0,   112,   113,   150,
       0,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   151,     0,     0,     0,   153,     0,     0,
     154,     0,     0,   155,   233,   129,   130,   131,     0,   336,
     133,     0,     0,     0,     0,   135,     0,    66,     0,     0,
       0,    67,    68,    69,    70,     0,   137,    71,     0,     0,
       0,     0,    72,     0,     0,     0,    73,     0,     0,     0,
       0,     0,     0,     0,   139,     0,     0,     0,   140,    74,
       0,     0,    75,    76,    77,    78,    79,     0,   143,   144,
       0,     0,     0,   145,     0,     0,    80,    81,    82,    83,
       0,    84,   234,    86,   235,     0,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,   148,    97,    98,    99,     0,   100,
       0,     0,   101,     0,     0,   102,   103,   104,   105,     0,
     106,   107,     0,   108,     0,   109,   110,   111,     0,     0,
     112,   113,   150,     0,   115,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   151,     0,     0,     0,
     153,     0,     0,   154,     0,     0,   155,   233,   129,   130,
     131,     0,   366,   133,     0,     0,     0,     0,   135,     0,
      66,     0,     0,     0,    67,    68,    69,    70,     0,   137,
      71,     0,     0,     0,     0,    72,     0,     0,     0,    73,
       0,     0,     0,     0,     0,     0,     0,   139,     0,     0,
       0,   140,    74,     0,     0,    75,    76,    77,    78,    79,
       0,   143,   144,     0,     0,     0,   145,     0,     0,    80,
      81,    82,    83,     0,    84,   234,    86,   235,     0,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,     0,    95,    96,     0,   148,    97,    98,
      99,     0,   100,     0,     0,   101,     0,     0,   102,   103,
     104,   105,     0,   106,   107,     0,   108,     0,   109,   110,
     111,     0,     0,   112,   113,   150,     0,   115,     0,     0,
       0,     0,     0,     0,    65,   457,     0,     0,     0,   151,
       0,     0,     0,   153,     0,     0,   154,    66,     0,   155,
       0,    67,    68,    69,    70,     0,   458,    71,     0,     0,
       0,     0,    72,     0,     0,     0,   459,     0,     0,     0,
       0,     0,     0,     0,   460,     0,   461,     0,     0,    74,
       0,     0,    75,    76,    77,   462,    79,     0,   463,     0,
       0,     0,     0,   464,     0,     0,    80,    81,    82,    83,
       0,   465,    85,    86,     0,     0,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
       0,     0,   101,     0,     0,   102,   103,   104,   105,     0,
     106,   107,     0,   108,     0,   109,   110,   111,     0,     0,
     112,   113,   466,     0,   115,     0,     0,    65,     0,   594,
       0,     0,   595,     0,     0,     0,     0,     0,     0,     0,
      66,     0,     0,   467,    67,    68,    69,    70,     0,     0,
      71,     0,     0,     0,     0,    72,     0,     0,     0,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,     0,     0,    75,    76,    77,    78,    79,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
      81,    82,    83,     0,    84,    85,    86,     0,     0,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,     0,    95,    96,     0,     0,    97,    98,
      99,     0,   100,     0,     0,   101,     0,     0,   102,   103,
     104,   105,     0,   106,   107,    65,   108,     0,   109,   110,
     111,     0,     0,   112,   113,   114,     0,   115,    66,     0,
       0,     0,    67,    68,    69,    70,     0,     0,    71,     0,
       0,     0,     0,    72,     0,     0,   596,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,     0,     0,    75,    76,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    80,    81,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,    87,
       0,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,     0,    95,    96,     0,     0,    97,    98,    99,     0,
     100,     0,     0,   101,     0,     0,   102,   103,   104,   105,
       0,   106,   107,     0,   108,     0,   109,   110,   111,     0,
       0,   112,   113,   114,     0,   115,     0,     0,    65,     0,
     319,     0,     0,   402,     0,     0,     0,     0,     0,     0,
       0,    66,     0,     0,   369,    67,    68,    69,    70,     0,
       0,    71,     0,     0,     0,     0,    72,     0,     0,     0,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   403,     0,    74,     0,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,    81,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,    87,   404,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,     0,     0,   101,     0,     0,   102,
     103,   104,   105,     0,   106,   107,     0,   108,   405,   109,
     110,   111,     0,     0,   112,   113,   114,     0,   115,    65,
       0,   319,     0,     0,     0,     0,     0,     0,     0,     0,
     406,     0,    66,     0,     0,     0,    67,    68,    69,    70,
       0,     0,    71,     0,     0,     0,     0,    72,     0,     0,
       0,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,     0,     0,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,    81,    82,    83,     0,    84,    85,    86,     0,
       0,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,    93,    94,     0,    95,    96,     0,     0,
      97,    98,    99,     0,   100,     0,     0,   101,     0,     0,
     102,   103,   104,   105,     0,   106,   107,     0,   108,     0,
     109,   110,   111,     0,     0,   112,   113,   114,     0,   115,
      65,   457,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   406,     0,    66,     0,     0,     0,    67,    68,    69,
      70,     0,   458,    71,     0,     0,     0,     0,    72,     0,
       0,     0,   459,     0,     0,     0,     0,     0,     0,     0,
     460,     0,   461,     0,     0,    74,     0,     0,    75,    76,
      77,   462,    79,     0,   463,     0,     0,     0,     0,   464,
       0,     0,    80,    81,    82,    83,     0,    84,    85,    86,
       0,     0,     0,     0,    87,     0,     0,     0,     0,    88,
      89,    90,    91,   690,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,     0,     0,   101,     0,
       0,   102,   103,   104,   105,     0,   106,   107,     0,   108,
       0,   109,   110,   111,    65,   457,   112,   113,   466,     0,
     115,     0,     0,     0,     0,     0,     0,    66,     0,     0,
       0,    67,    68,    69,    70,     0,   458,    71,     0,     0,
       0,     0,    72,     0,     0,     0,   459,     0,     0,     0,
       0,     0,     0,     0,   460,     0,   461,     0,     0,    74,
       0,     0,    75,    76,    77,   462,    79,     0,   463,     0,
       0,     0,     0,   464,     0,     0,    80,    81,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
       0,     0,   101,     0,     0,   102,   103,   104,   105,     0,
     106,   107,     0,   108,     0,   109,   110,   111,     0,     0,
     112,   113,   466,    65,   115,   280,   281,     0,   282,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
      67,    68,    69,    70,     0,     0,    71,     0,     0,     0,
       0,    72,     0,     0,     0,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,     0,
       0,    75,    76,    77,    78,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,    81,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,    87,   283,     0,
       0,     0,    88,    89,    90,    91,    92,    93,    94,     0,
      95,    96,     0,     0,    97,    98,    99,     0,   100,     0,
       0,   101,     0,     0,   102,   103,   104,   105,     0,   106,
     107,    65,   108,     0,   109,   110,   111,     0,   167,   112,
     113,   114,     0,   115,    66,     0,     0,     0,    67,    68,
      69,    70,     0,     0,    71,     0,     0,   168,     0,    72,
       0,     0,     0,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,     0,     0,    75,
     169,    77,    78,    79,     0,     0,     0,   170,     0,     0,
       0,     0,     0,    80,    81,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,   171,     0,     0,     0,     0,
      88,    89,    90,    91,    92,    93,    94,     0,    95,    96,
       0,     0,    97,    98,    99,     0,   100,     0,     0,   172,
       0,     0,   102,   103,   104,   105,     0,   106,   107,     0,
       0,     0,   109,   110,   111,     0,     0,   112,   113,   114,
      65,   115,   511,     0,     0,   402,     0,     0,     0,     0,
       0,     0,     0,    66,     0,     0,     0,    67,    68,    69,
      70,     0,     0,    71,     0,     0,     0,     0,    72,     0,
       0,     0,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,     0,     0,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,    81,    82,    83,     0,    84,    85,    86,
       0,     0,     0,     0,    87,     0,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,     0,     0,   101,     0,
       0,   102,   103,   104,   105,     0,   106,   107,    65,   108,
     717,   109,   110,   111,     0,     0,   112,   113,   114,     0,
     115,    66,     0,     0,     0,    67,    68,    69,    70,     0,
       0,    71,     0,     0,     0,     0,    72,     0,     0,     0,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,     0,     0,    75,    76,    77,    78,
      79,     0,   718,     0,     0,     0,     0,     0,     0,     0,
      80,    81,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,     0,     0,   101,     0,     0,   102,
     103,   104,   105,     0,   106,   107,    65,   108,     0,   109,
     110,   111,     0,     0,   112,   113,   114,     0,   115,    66,
       0,     0,     0,    67,    68,    69,    70,     0,     0,    71,
       0,     0,     0,     0,    72,     0,     0,     0,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,     0,     0,    75,    76,    77,    78,    79,     0,
       0,   239,     0,     0,     0,     0,     0,     0,    80,    81,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
      87,     0,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,     0,     0,   101,     0,     0,   102,   103,   104,
     105,     0,   106,   107,    65,   108,     0,   109,   110,   111,
       0,     0,   112,   113,   114,     0,   115,    66,     0,   250,
       0,    67,    68,    69,    70,     0,     0,    71,     0,     0,
       0,     0,    72,     0,     0,     0,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
       0,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,   253,     0,     0,     0,     0,    80,    81,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,   254,     0,    97,    98,    99,     0,   100,
       0,     0,   101,     0,     0,   102,   103,   104,   105,     0,
     106,   107,     0,   108,    65,   109,   110,   111,     0,   289,
     112,   113,   114,     0,   115,     0,     0,    66,     0,     0,
       0,    67,    68,    69,    70,     0,     0,    71,     0,     0,
       0,     0,    72,     0,     0,     0,    73,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,     0,    74,
       0,     0,    75,    76,    77,    78,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,    81,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
       0,     0,   101,     0,     0,   102,   290,   104,   105,     0,
     106,   107,    65,   108,     0,   109,   110,   111,     0,     0,
     112,   113,   114,     0,   115,    66,     0,     0,     0,    67,
      68,    69,    70,     0,     0,    71,     0,     0,     0,     0,
      72,     0,     0,     0,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,     0,     0,
      75,    76,    77,    78,    79,     0,   536,     0,     0,     0,
       0,     0,     0,     0,    80,    81,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,     0,    95,
      96,     0,     0,    97,    98,    99,     0,   100,     0,     0,
     101,     0,     0,   102,   103,   104,   105,     0,   106,   107,
      65,   108,     0,   109,   110,   111,     0,     0,   112,   113,
     114,     0,   115,    66,     0,     0,     0,    67,    68,    69,
      70,     0,     0,    71,     0,     0,     0,     0,    72,     0,
       0,     0,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,     0,     0,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,    81,    82,    83,   701,    84,    85,    86,
       0,     0,     0,     0,    87,     0,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,     0,     0,   101,     0,
       0,   102,   103,   104,   105,     0,   106,   107,    65,   108,
       0,   109,   110,   111,     0,     0,   112,   113,   114,     0,
     115,    66,     0,     0,     0,    67,    68,    69,    70,     0,
       0,    71,     0,     0,     0,     0,    72,     0,     0,     0,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,     0,     0,    75,    76,    77,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,    81,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,     0,     0,   101,     0,     0,   102,
     103,   104,   105,     0,   106,   107,    65,   108,     0,   109,
     110,   111,   277,     0,   112,   113,   114,     0,   115,    66,
       0,     0,     0,    67,    68,    69,    70,     0,     0,    71,
       0,     0,     0,     0,    72,     0,     0,     0,    73,     0,
       0,     0,     0,     0,     0,     0,   253,     0,     0,     0,
       0,    74,     0,     0,    75,    76,    77,    78,    79,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    80,    81,
      82,    83,     0,    84,    85,    86,     0,     0,   254,   253,
       0,     0,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,     0,     0,   101,     0,     0,   102,   103,   104,
     105,   254,   106,   107,   253,  -393,     0,   109,   110,   111,
       0,     0,   112,   113,   114,     0,   115,     0,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,     0,   253,     0,   254,     0,   495,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   254,     0,     0,   253,
       0,   492,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   254,   253,     0,     0,     0,   494,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,     0,     0,     0,   254,   253,   668,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,     0,   254,   253,     0,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,     0,
     254,   253,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,     0,   254,   253,     0,   488,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,     0,   254,     0,     0,   496,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,     0,     0,
       0,     0,   666,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,     1,     0,     0,     2,   667,     0,     3,     0,     0,
       0,     0,     4,     0,     0,     0,     0,     5,     0,     0,
       6,     0,     7,     8,     9,     0,     0,   253,    10,     0,
       0,     0,    11,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,   253,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   253,     0,   254,
       0,     0,     0,     0,     0,    13,     0,     0,     0,     0,
     253,     0,     0,     0,    14,   254,    15,     0,     0,    16,
       0,    17,     0,    18,     0,     0,    19,     0,    20,   254,
       0,     0,     0,     0,     0,     0,     0,     0,    21,     0,
       0,    22,   254,     0,     0,     0,     0,     0,     0,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,     0,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269
};

static const short yycheck[] =
{
       4,   220,   529,   222,   223,   533,   510,   226,   227,   527,
     449,   608,   530,    18,     5,     5,   181,     8,    22,     5,
       5,   186,    18,   188,     5,     6,     9,     8,     5,     6,
       9,     8,   467,    14,    45,    79,    40,    14,   203,    27,
       5,     6,   304,    13,    14,    13,    14,   565,    41,    14,
     568,   569,   570,   571,   572,   573,    99,    21,    62,    63,
      64,    13,    14,    78,    51,     5,     6,    51,     8,   370,
     332,     5,     6,    19,    14,    78,     5,     6,    22,     8,
      14,    50,    15,   122,   759,    14,    68,    42,    14,    50,
     122,   125,    50,    39,    64,     0,    83,   601,    97,    83,
     144,   144,    68,   102,    30,   144,    25,   782,    59,   143,
      34,    44,   144,    84,   118,   119,   120,   121,   122,    74,
     124,   125,   126,    87,   139,   290,    63,   428,    47,   564,
     141,    59,    96,   730,     5,     6,   139,    42,   207,   105,
     122,   645,    61,   136,   135,   135,   743,   143,   144,   135,
     135,   155,   143,   143,   135,   159,   144,   143,   135,   142,
     128,   142,   759,   142,   169,   142,   170,   171,    42,    74,
     135,   175,     0,   612,   678,   144,   120,   142,   148,   105,
     148,   136,   137,   138,   781,   782,   144,   106,   192,   628,
     112,   195,   116,   711,   712,   135,   148,    96,   202,    96,
      74,   135,    17,    24,   142,   467,   135,    88,    50,   108,
      91,   147,   716,   144,   218,   146,    64,    98,   287,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   143,   144,   142,    58,   766,   143,
     144,   143,   144,    64,   248,   112,   250,   412,   142,   414,
     143,   144,   143,   144,   143,   144,   783,   143,   144,   142,
     134,   135,   136,   137,   138,   142,   528,   143,   144,   275,
     276,   275,   276,   143,   144,   143,   144,   143,   144,   143,
     144,    42,   142,   287,   143,   144,   143,   144,   142,    16,
     143,   144,   142,   555,   298,   557,   365,   142,   560,   142,
     304,   305,   564,   142,   308,   142,   745,   128,   129,   130,
     131,   132,   133,    74,   143,   144,   143,   144,   139,   143,
     144,    69,   142,   142,   142,   142,    64,   128,   332,   128,
      99,   128,    59,   112,    99,    68,   151,    79,   153,   154,
      89,   119,   346,   111,    94,    72,   608,   103,   115,   115,
     143,   143,   142,   142,   142,    82,   128,    84,    85,   363,
       5,   365,    52,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   105,    65,
     449,   108,   128,   110,   199,   128,   105,   105,   149,     8,
     135,   142,    32,   120,    32,   399,    96,   401,     8,     8,
     144,     8,   217,   665,   219,   128,   143,    78,   149,   144,
     225,   142,     9,   228,   229,   230,   231,     5,    67,   128,
     424,   100,   144,     5,   100,     8,   128,    80,   143,   118,
     245,   103,   142,   437,   249,   439,   440,   441,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   103,   125,   142,   527,   274,
     142,   530,   142,   467,   142,   142,   128,   142,   730,    16,
     732,    97,   142,    53,     5,   479,   545,   143,   143,   142,
     124,   743,    29,    27,    31,   128,   128,   144,     8,   144,
      37,     8,     8,   143,    42,   310,   565,   759,     8,   568,
     569,   570,   571,   572,   573,    52,   136,   726,   143,   143,
       5,   144,    59,   582,   518,   519,    58,    58,   143,   781,
     782,   123,   337,    88,   528,    72,    74,   142,   142,     8,
     143,   143,   143,   125,   125,    82,   142,    84,    85,   143,
     143,    78,   143,   612,   359,   360,   361,    80,    53,    27,
      56,   555,   128,   557,   101,     8,   560,   142,   105,   628,
     564,   108,   109,   110,     3,   144,    18,   144,   143,    27,
      69,    80,   791,   120,   143,    25,    26,   143,   582,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   144,   596,   128,     5,   144,   600,    47,    27,    42,
       8,   144,   142,     5,   608,   144,   200,   611,   337,   143,
     756,   583,    62,   505,   308,   732,    66,   249,   733,   434,
      42,   436,   743,   627,   519,    75,    76,   737,   582,   481,
     401,    74,   699,   363,   775,   399,   611,   449,   528,   628,
     600,    19,   711,   712,   665,   436,   246,   434,    -1,    -1,
      -1,   259,    74,    -1,    -1,    -1,   106,    -1,    -1,   663,
      -1,   665,   477,    -1,    -1,    -1,   735,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   745,   492,    -1,   494,
     495,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
     143,   144,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,   143,   144,   727,    -1,    -1,   730,    -1,   732,    -1,
      -1,    -1,    -1,   737,    -1,    -1,    -1,    -1,    -1,   743,
      -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,     9,
      10,    -1,   756,    -1,    14,   759,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    25,    26,    -1,   583,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,   781,   782,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    47,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    57,    58,    -1,
      -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    91,    -1,    93,    94,    95,    96,    -1,    98,    -1,
      -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,    -1,   112,    -1,   114,   115,   116,    -1,    -1,   119,
     120,   121,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   135,   136,    -1,    -1,   139,
      -1,    -1,   142,    -1,    -1,   145,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    25,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      42,    -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,
      47,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      57,    58,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,
      67,    68,    74,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    91,    -1,    93,    94,    95,    96,
      -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,    -1,   114,   115,   116,
      -1,    -1,   119,   120,   121,    -1,   123,    -1,   130,   131,
     132,   133,   134,   135,   136,   137,   138,    -1,   135,    -1,
      -1,    -1,   139,    -1,    -1,   142,   143,    -1,   145,     3,
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
      -1,   135,   136,    -1,    -1,   139,    -1,    -1,   142,    -1,
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
     121,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,    -1,   139,    -1,
      -1,   142,    -1,    -1,   145,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    25,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    47,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      58,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    91,    -1,    93,    94,    95,    96,    97,
      98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,    -1,   112,    -1,   114,   115,   116,    -1,
      -1,   119,   120,   121,    -1,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,    -1,
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
     142,    -1,    -1,   145,     3,     4,     5,     6,    -1,     8,
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
      -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,     3,     4,    -1,    -1,    -1,   135,
      -1,    -1,    -1,   139,    -1,    -1,   142,    16,    -1,   145,
      -1,    20,    21,    22,    23,    -1,    25,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    91,    -1,    -1,    94,    95,    96,    -1,    98,
      -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,    -1,   112,    -1,   114,   115,   116,    -1,    -1,
     119,   120,   121,    -1,   123,    -1,    -1,     3,    -1,     5,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,   142,    20,    21,    22,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    91,    -1,    -1,    94,    95,
      96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,
     106,   107,    -1,   109,   110,     3,   112,    -1,   114,   115,
     116,    -1,    -1,   119,   120,   121,    -1,   123,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,   142,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    91,    -1,    -1,    94,    95,    96,    -1,
      98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,    -1,   112,    -1,   114,   115,   116,    -1,
      -1,   119,   120,   121,    -1,   123,    -1,    -1,     3,    -1,
       5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    -1,   142,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    78,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    -1,    -1,    94,
      95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,    -1,   112,   113,   114,
     115,   116,    -1,    -1,   119,   120,   121,    -1,   123,     3,
      -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     135,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    91,    -1,    -1,
      94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,
     104,   105,   106,   107,    -1,   109,   110,    -1,   112,    -1,
     114,   115,   116,    -1,    -1,   119,   120,   121,    -1,   123,
       3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   135,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    -1,
      -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,     3,     4,   119,   120,   121,    -1,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    -1,    25,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    91,    -1,    -1,    94,    95,    96,    -1,    98,
      -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,    -1,   112,    -1,   114,   115,   116,    -1,    -1,
     119,   120,   121,     3,   123,     5,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    78,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    91,    -1,    -1,    94,    95,    96,    -1,    98,    -1,
      -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,     3,   112,    -1,   114,   115,   116,    -1,    10,   119,
     120,   121,    -1,   123,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    -1,    -1,    26,    -1,    -1,    29,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    91,
      -1,    -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,    -1,
      -1,    -1,   114,   115,   116,    -1,    -1,   119,   120,   121,
       3,   123,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    -1,
      -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,     3,   112,
       5,   114,   115,   116,    -1,    -1,   119,   120,   121,    -1,
     123,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    -1,    -1,    94,
      95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,     3,   112,    -1,   114,
     115,   116,    -1,    -1,   119,   120,   121,    -1,   123,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    91,    -1,    -1,    94,    95,    96,
      -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,     3,   112,    -1,   114,   115,   116,
      -1,    -1,   119,   120,   121,    -1,   123,    16,    -1,    18,
      -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    -1,    -1,    65,    66,    67,    68,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    91,    74,    -1,    94,    95,    96,    -1,    98,
      -1,    -1,   101,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,    -1,   112,     3,   114,   115,   116,    -1,     8,
     119,   120,   121,    -1,   123,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,
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
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
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
      -1,    -1,    65,    66,    67,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    -1,
      -1,    94,    95,    96,    -1,    98,    -1,    -1,   101,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,     3,   112,
      -1,   114,   115,   116,    -1,    -1,   119,   120,   121,    -1,
     123,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    -1,    -1,    94,
      95,    96,    -1,    98,    -1,    -1,   101,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,     3,   112,    -1,   114,
     115,   116,     9,    -1,   119,   120,   121,    -1,   123,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    74,    42,
      -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    91,    -1,    -1,    94,    95,    96,
      -1,    98,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
     107,    74,   109,   110,    42,   112,    -1,   114,   115,   116,
      -1,    -1,   119,   120,   121,    -1,   123,    -1,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    -1,    42,    -1,    74,    -1,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    74,    -1,    -1,    42,
      -1,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    74,    42,    -1,    -1,    -1,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    74,    42,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    -1,    74,    42,    -1,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,    -1,
      74,    42,    -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,    -1,    74,    42,    -1,   143,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,    -1,    74,    -1,    -1,   143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    17,    -1,    -1,    20,   143,    -1,    23,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      36,    -1,    38,    39,    40,    -1,    -1,    42,    44,    -1,
      -1,    -1,    48,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    74,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      42,    -1,    -1,    -1,    90,    74,    92,    -1,    -1,    95,
      -1,    97,    -1,    99,    -1,    -1,   102,    -1,   104,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    74,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    -1,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    23,    28,    33,    36,    38,    39,    40,
      44,    48,    60,    81,    90,    92,    95,    97,    99,   102,
     104,   114,   117,   151,   152,   155,   156,   157,   158,   165,
     206,   211,   212,   217,   218,   219,   220,   231,   232,   241,
     242,   243,   244,   245,   249,   250,   255,   256,   257,   260,
     262,   263,   264,   265,   266,   267,   269,   270,   271,   272,
     273,   274,    96,   108,    59,     3,    16,    20,    21,    22,
      23,    26,    31,    35,    48,    51,    52,    53,    54,    55,
      65,    66,    67,    68,    70,    71,    72,    77,    82,    83,
      84,    85,    86,    87,    88,    90,    91,    94,    95,    96,
      98,   101,   104,   105,   106,   107,   109,   110,   112,   114,
     115,   116,   119,   120,   121,   123,   153,   154,    51,    83,
      50,    51,    83,    21,    87,    96,    59,    84,     3,     4,
       5,     6,     8,     9,    10,    14,    22,    25,    35,    43,
      47,    54,    55,    57,    58,    62,    71,    73,    93,   107,
     121,   135,   136,   139,   142,   145,   154,   166,   167,   169,
     199,   200,   268,   275,   276,   279,   280,    10,    29,    52,
      59,    77,   101,   153,   258,    16,    29,    31,    37,    52,
      59,    72,    82,    84,    85,   101,   105,   108,   109,   110,
     120,   208,   209,   258,   112,    96,   154,   170,     0,    45,
     141,   286,    63,   154,   154,   154,   154,   142,   154,   154,
     170,   154,   154,   154,   154,   154,    50,   142,   147,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,     3,    71,    73,   199,   199,   199,    58,
     154,   203,   204,    13,    14,   148,   277,   278,    50,   144,
      18,   154,   168,    42,    74,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
      69,   191,   192,    64,   128,   128,    99,     9,   153,   154,
       5,     6,     8,    78,   154,   215,   216,   128,   112,     8,
     105,   154,    99,    68,   207,   207,   207,   207,    79,   193,
     194,   154,   119,   154,    99,   144,   199,   283,   284,   285,
      97,   102,   156,   154,   207,    89,    15,    44,   111,     5,
       6,     8,    14,   135,   142,   178,   179,   227,   233,   234,
      94,   115,   122,   172,   115,   103,     8,   142,   143,   199,
     201,   202,   154,   199,   199,   201,    41,   136,   201,   201,
     143,   199,   201,   201,   199,   199,   199,   199,   143,   142,
     142,   142,   143,   144,   146,   128,     8,   199,   278,   142,
     170,   167,   199,   154,   199,   199,   199,   199,   199,   199,
     199,   280,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,     5,    78,   139,   199,   215,   215,   128,   128,
      52,   135,     8,    46,    78,   113,   135,   154,   178,   213,
     214,    65,   105,   207,   105,     8,   154,   195,   196,    14,
      30,   105,   210,    68,   122,   251,   252,   154,   246,   247,
     276,   154,   168,    27,   144,   281,   282,   142,   221,    32,
      32,    96,     5,     6,     8,   143,   178,   180,   235,   144,
     236,    25,    47,    61,   106,   261,     8,     4,    25,    35,
      43,    45,    54,    57,    62,    70,   121,   142,   154,   173,
     174,   175,   176,   177,   276,     8,     8,    97,   158,    49,
     143,   144,   143,   143,   154,   143,   143,   143,   143,   143,
     143,   143,   144,   143,   144,   144,   143,   199,   199,   204,
     154,   178,   205,   149,   149,   162,   171,   172,   144,    78,
     142,     5,   213,     9,   216,    67,   207,   207,   128,   144,
     100,     5,   100,     8,   154,   253,   254,   128,   144,   172,
     128,   283,    80,   187,   188,   285,    57,   154,   222,   223,
     118,   154,   154,   154,   143,   144,   143,   144,   227,   234,
     237,   238,   143,   103,   103,   142,   142,   142,   142,   142,
     142,   142,   142,   173,   125,    24,    58,    64,   128,   129,
     130,   131,   132,   133,   139,   143,   154,   202,   143,   199,
     199,   199,   128,    97,   165,    53,   181,     5,   180,   128,
      88,    91,    98,   259,     5,     8,   142,   154,   196,   128,
     124,   142,   178,   179,   247,   193,   178,   179,    27,   191,
     143,   144,   142,   224,   225,    25,    26,    47,    62,    66,
      75,    76,   106,   248,   178,     8,    18,   239,   144,     8,
       8,   154,   276,   136,   276,   143,   143,   276,     8,   143,
     143,   173,   178,   179,     9,   142,    78,   139,     8,   178,
     179,     8,   178,   179,   178,   179,   178,   179,   178,   179,
     178,   179,    58,   144,   159,    58,   143,   143,   144,   205,
     166,   143,     5,   182,   123,   185,   186,   143,   142,    34,
     116,    88,   154,   197,   198,   142,     8,   254,   143,   180,
      86,   177,   189,   190,   222,   142,   226,   227,    79,   144,
     228,    69,   154,   240,   227,   238,   143,   143,   143,   143,
     143,   125,   125,   180,    78,     9,   142,     5,    57,   154,
     160,   161,   143,   154,   275,   276,   142,    50,    80,   163,
      27,    53,    56,   184,   180,   128,   143,   144,     8,   143,
     142,    19,    39,   144,   143,   144,     3,   229,   230,   225,
     178,   179,   178,   179,   143,   180,   144,   143,   201,    27,
      69,   164,   177,   183,    80,   175,   187,   143,   178,   198,
     143,   143,   190,   227,   128,   144,   143,   161,   143,   189,
       5,   144,    27,   191,     8,   230,   144,   144,   177,   189,
     193,   142,     5,   201,   143,   143
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

  case 85:

    { pParser->PushQuery(); ;}
    break;

  case 86:

    { pParser->PushQuery(); ;}
    break;

  case 87:

    { pParser->PushQuery(); ;}
    break;

  case 91:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 93:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 96:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 97:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 101:

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

  case 102:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 104:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 105:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 106:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 109:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 112:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 113:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 114:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 115:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 116:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 117:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 118:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 119:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 120:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 121:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 122:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 124:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 131:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 133:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 134:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 135:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 136:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 137:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 153:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 154:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 155:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 156:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 159:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 160:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 161:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 162:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 163:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 169:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 170:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 171:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 172:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 173:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 174:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 175:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 179:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 180:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 181:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 183:

    {
			pParser->AddHaving();
		;}
    break;

  case 186:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 189:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 190:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 192:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 194:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 195:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 198:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 199:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 205:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 206:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 207:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 208:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 209:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 210:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 211:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 212:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 214:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 215:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 220:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 221:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 244:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 245:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 247:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 248:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 249:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 250:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 251:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 252:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 254:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 255:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 256:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 257:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 262:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 263:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 270:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 271:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 272:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 273:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 275:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 276:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 277:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 278:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 279:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 280:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 281:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 282:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 283:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 287:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 288:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 289:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 290:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 291:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 292:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 293:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 294:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 295:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 296:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 297:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 300:

    { yyval.m_iValue = 1; ;}
    break;

  case 301:

    { yyval.m_iValue = 0; ;}
    break;

  case 302:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 310:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 311:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 312:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 315:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 316:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 317:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 322:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 323:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 326:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 327:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 328:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 329:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 330:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 331:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 332:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 333:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 338:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 339:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 340:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 341:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 342:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 344:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 345:

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

  case 346:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 349:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 351:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 356:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 359:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 360:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 361:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 364:

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

  case 365:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 366:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 367:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 368:

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

  case 369:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 370:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 371:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 372:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 373:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 374:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 375:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 376:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 377:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 378:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 379:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 380:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 381:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 382:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 389:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 390:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 391:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 399:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 400:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 401:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 402:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 403:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 404:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 405:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 406:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 407:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 408:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 409:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 412:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 413:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 414:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 415:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 416:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 417:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 420:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 422:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 423:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 424:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 425:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 426:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 427:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 428:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 432:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 434:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 437:

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

