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
#define YYFINAL  194
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4494

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  149
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  138
/* YYNRULES -- Number of rules. */
#define YYNRULES  440
/* YYNRULES -- Number of states. */
#define YYNSTATES  796

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
     160,   162,   164,   166,   168,   172,   175,   177,   179,   181,
     190,   192,   202,   203,   206,   208,   212,   214,   216,   218,
     219,   223,   224,   227,   232,   244,   246,   250,   252,   255,
     256,   258,   261,   263,   268,   273,   278,   283,   288,   293,
     297,   303,   305,   309,   310,   312,   315,   317,   321,   325,
     330,   332,   336,   340,   346,   353,   357,   362,   368,   372,
     376,   380,   384,   388,   390,   396,   402,   408,   412,   416,
     420,   424,   428,   432,   436,   441,   445,   447,   449,   454,
     458,   462,   464,   466,   471,   476,   481,   485,   487,   490,
     492,   495,   497,   499,   503,   505,   509,   511,   513,   514,
     519,   520,   522,   524,   528,   529,   532,   533,   535,   541,
     542,   544,   548,   554,   556,   560,   562,   565,   568,   569,
     571,   574,   579,   580,   582,   585,   587,   591,   595,   599,
     605,   612,   616,   618,   622,   626,   628,   630,   632,   634,
     636,   638,   640,   643,   646,   650,   654,   658,   662,   666,
     670,   674,   678,   682,   686,   690,   694,   698,   702,   706,
     710,   714,   718,   722,   724,   726,   728,   732,   737,   742,
     747,   752,   757,   762,   767,   771,   778,   785,   789,   798,
     813,   815,   819,   821,   823,   827,   833,   835,   837,   839,
     841,   844,   845,   848,   850,   853,   856,   860,   862,   864,
     866,   869,   874,   879,   883,   888,   893,   895,   897,   898,
     901,   906,   911,   916,   920,   925,   930,   938,   944,   950,
     960,   962,   964,   966,   968,   970,   972,   976,   978,   980,
     982,   984,   986,   988,   990,   992,   994,   997,  1005,  1007,
    1009,  1010,  1014,  1016,  1018,  1020,  1024,  1026,  1030,  1034,
    1036,  1040,  1042,  1044,  1046,  1050,  1053,  1054,  1057,  1059,
    1063,  1067,  1072,  1079,  1081,  1085,  1087,  1091,  1093,  1097,
    1098,  1101,  1103,  1107,  1111,  1112,  1114,  1116,  1118,  1122,
    1124,  1126,  1130,  1134,  1141,  1143,  1147,  1151,  1155,  1161,
    1166,  1170,  1174,  1176,  1178,  1180,  1182,  1184,  1186,  1188,
    1190,  1192,  1200,  1207,  1212,  1217,  1223,  1224,  1226,  1229,
    1231,  1235,  1239,  1242,  1246,  1253,  1254,  1256,  1258,  1261,
    1264,  1267,  1269,  1277,  1279,  1281,  1283,  1285,  1287,  1291,
    1298,  1302,  1306,  1309,  1313,  1315,  1319,  1322,  1326,  1330,
    1338,  1344,  1346,  1348,  1351,  1353,  1356,  1358,  1360,  1364,
    1368,  1372,  1376,  1378,  1379,  1382,  1384,  1387,  1389,  1391,
    1395
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     150,     0,    -1,   151,    -1,   154,    -1,   154,   140,    -1,
     220,    -1,   232,    -1,   212,    -1,   213,    -1,   218,    -1,
     233,    -1,   242,    -1,   244,    -1,   245,    -1,   246,    -1,
     251,    -1,   256,    -1,   257,    -1,   261,    -1,   263,    -1,
     264,    -1,   265,    -1,   266,    -1,   267,    -1,   258,    -1,
     268,    -1,   270,    -1,   271,    -1,   272,    -1,   250,    -1,
     273,    -1,   274,    -1,     3,    -1,    16,    -1,    20,    -1,
      21,    -1,    22,    -1,    23,    -1,    26,    -1,    31,    -1,
      35,    -1,    48,    -1,    51,    -1,    52,    -1,    53,    -1,
      54,    -1,    55,    -1,    65,    -1,    67,    -1,    68,    -1,
      70,    -1,    71,    -1,    72,    -1,    82,    -1,    83,    -1,
      84,    -1,    85,    -1,    87,    -1,    86,    -1,    88,    -1,
      90,    -1,    93,    -1,    94,    -1,    95,    -1,    97,    -1,
     100,    -1,   103,    -1,   104,    -1,   105,    -1,   106,    -1,
     108,    -1,   109,    -1,   113,    -1,   114,    -1,   115,    -1,
     118,    -1,   119,    -1,   120,    -1,   122,    -1,    66,    -1,
     152,    -1,    77,    -1,   111,    -1,   155,    -1,   154,   140,
     155,    -1,   154,   286,    -1,   156,    -1,   207,    -1,   157,
      -1,    96,     3,   141,   141,   157,   142,   158,   142,    -1,
     164,    -1,    96,   165,    50,   141,   161,   164,   142,   162,
     163,    -1,    -1,   143,   159,    -1,   160,    -1,   159,   143,
     160,    -1,   153,    -1,     5,    -1,    57,    -1,    -1,    80,
      27,   190,    -1,    -1,    69,     5,    -1,    69,     5,   143,
       5,    -1,    96,   165,    50,   169,   170,   182,   186,   185,
     188,   192,   194,    -1,   166,    -1,   165,   143,   166,    -1,
     135,    -1,   168,   167,    -1,    -1,   153,    -1,    18,   153,
      -1,   200,    -1,    22,   141,   200,   142,    -1,    71,   141,
     200,   142,    -1,    73,   141,   200,   142,    -1,   106,   141,
     200,   142,    -1,    55,   141,   200,   142,    -1,    35,   141,
     135,   142,    -1,    54,   141,   142,    -1,    35,   141,    41,
     153,   142,    -1,   153,    -1,   169,   143,   153,    -1,    -1,
     171,    -1,   121,   172,    -1,   173,    -1,   172,   124,   172,
      -1,   141,   172,   142,    -1,    70,   141,     8,   142,    -1,
     174,    -1,   176,   127,   177,    -1,   176,   128,   177,    -1,
     176,    58,   141,   181,   142,    -1,   176,   138,    58,   141,
     181,   142,    -1,   176,    58,     9,    -1,   176,   138,    58,
       9,    -1,   176,    24,   177,   124,   177,    -1,   176,   130,
     177,    -1,   176,   129,   177,    -1,   176,   131,   177,    -1,
     176,   132,   177,    -1,   176,   127,   178,    -1,   175,    -1,
     176,    24,   178,   124,   178,    -1,   176,    24,   177,   124,
     178,    -1,   176,    24,   178,   124,   177,    -1,   176,   130,
     178,    -1,   176,   129,   178,    -1,   176,   131,   178,    -1,
     176,   132,   178,    -1,   176,   127,     8,    -1,   176,   128,
       8,    -1,   176,    64,    78,    -1,   176,    64,   138,    78,
      -1,   176,   128,   178,    -1,   153,    -1,     4,    -1,    35,
     141,   135,   142,    -1,    54,   141,   142,    -1,   120,   141,
     142,    -1,    57,    -1,   276,    -1,    62,   141,   276,   142,
      -1,    43,   141,   276,   142,    -1,    25,   141,   276,   142,
      -1,    45,   141,   142,    -1,     5,    -1,   134,     5,    -1,
       6,    -1,   134,     6,    -1,    14,    -1,   177,    -1,   179,
     143,   177,    -1,     8,    -1,   180,   143,     8,    -1,   179,
      -1,   180,    -1,    -1,    53,   183,    27,   184,    -1,    -1,
       5,    -1,   176,    -1,   184,   143,   176,    -1,    -1,    56,
     174,    -1,    -1,   187,    -1,   122,    53,    80,    27,   190,
      -1,    -1,   189,    -1,    80,    27,   190,    -1,    80,    27,
      86,   141,   142,    -1,   191,    -1,   190,   143,   191,    -1,
     176,    -1,   176,    19,    -1,   176,    39,    -1,    -1,   193,
      -1,    69,     5,    -1,    69,     5,   143,     5,    -1,    -1,
     195,    -1,    79,   196,    -1,   197,    -1,   196,   143,   197,
      -1,   153,   127,   153,    -1,   153,   127,     5,    -1,   153,
     127,   141,   198,   142,    -1,   153,   127,   153,   141,     8,
     142,    -1,   153,   127,     8,    -1,   199,    -1,   198,   143,
     199,    -1,   153,   127,   177,    -1,   153,    -1,     4,    -1,
      57,    -1,     5,    -1,     6,    -1,    14,    -1,     9,    -1,
     134,   200,    -1,   138,   200,    -1,   200,   133,   200,    -1,
     200,   134,   200,    -1,   200,   135,   200,    -1,   200,   136,
     200,    -1,   200,   129,   200,    -1,   200,   130,   200,    -1,
     200,   126,   200,    -1,   200,   125,   200,    -1,   200,   137,
     200,    -1,   200,    42,   200,    -1,   200,    74,   200,    -1,
     200,   132,   200,    -1,   200,   131,   200,    -1,   200,   127,
     200,    -1,   200,   128,   200,    -1,   200,   124,   200,    -1,
     200,   123,   200,    -1,   141,   200,   142,    -1,   144,   204,
     145,    -1,   201,    -1,   276,    -1,   279,    -1,   275,    64,
      78,    -1,   275,    64,   138,    78,    -1,     3,   141,   202,
     142,    -1,    58,   141,   202,   142,    -1,    62,   141,   202,
     142,    -1,    25,   141,   202,   142,    -1,    47,   141,   202,
     142,    -1,    43,   141,   202,   142,    -1,     3,   141,   142,
      -1,    73,   141,   200,   143,   200,   142,    -1,    71,   141,
     200,   143,   200,   142,    -1,   120,   141,   142,    -1,     3,
     141,   200,    49,   153,    58,   275,   142,    -1,    92,   141,
     200,   143,   200,   143,   141,   202,   142,   143,   141,   202,
     142,   142,    -1,   203,    -1,   202,   143,   203,    -1,   200,
      -1,     8,    -1,   205,   127,   206,    -1,   204,   143,   205,
     127,   206,    -1,   153,    -1,    58,    -1,   177,    -1,   153,
      -1,   101,   209,    -1,    -1,    68,     8,    -1,   119,    -1,
     104,   208,    -1,    72,   208,    -1,    16,   104,   208,    -1,
      85,    -1,    82,    -1,    84,    -1,   109,   194,    -1,    16,
       8,   104,   208,    -1,    16,   153,   104,   208,    -1,   210,
     153,   104,    -1,   210,   153,   211,    99,    -1,   210,   153,
      14,    99,    -1,    59,    -1,   107,    -1,    -1,    30,     5,
      -1,    98,   152,   127,   215,    -1,    98,   152,   127,   214,
      -1,    98,   152,   127,    78,    -1,    98,    77,   216,    -1,
      98,    10,   127,   216,    -1,    98,    29,    98,   216,    -1,
      98,    52,     9,   127,   141,   179,   142,    -1,    98,    52,
     152,   127,   214,    -1,    98,    52,   152,   127,     5,    -1,
      98,    59,   153,    52,     9,   127,   141,   179,   142,    -1,
     153,    -1,     8,    -1,   112,    -1,    46,    -1,   177,    -1,
     217,    -1,   216,   134,   217,    -1,   153,    -1,    78,    -1,
       8,    -1,     5,    -1,     6,    -1,    33,    -1,    94,    -1,
     219,    -1,    23,    -1,   103,   111,    -1,   221,    63,   153,
     222,   117,   225,   229,    -1,    60,    -1,    91,    -1,    -1,
     141,   224,   142,    -1,   153,    -1,    57,    -1,   223,    -1,
     224,   143,   223,    -1,   226,    -1,   225,   143,   226,    -1,
     141,   227,   142,    -1,   228,    -1,   227,   143,   228,    -1,
     177,    -1,   178,    -1,     8,    -1,   141,   179,   142,    -1,
     141,   142,    -1,    -1,    79,   230,    -1,   231,    -1,   230,
     143,   231,    -1,     3,   127,     8,    -1,    38,    50,   169,
     171,    -1,    28,   153,   141,   234,   237,   142,    -1,   235,
      -1,   234,   143,   235,    -1,   228,    -1,   141,   236,   142,
      -1,     8,    -1,   236,   143,     8,    -1,    -1,   143,   238,
      -1,   239,    -1,   238,   143,   239,    -1,   228,   240,   241,
      -1,    -1,    18,    -1,   153,    -1,    69,    -1,   243,   153,
     208,    -1,    40,    -1,    39,    -1,   101,   108,   208,    -1,
     101,    37,   208,    -1,   116,   169,    98,   247,   171,   194,
      -1,   248,    -1,   247,   143,   248,    -1,   153,   127,   177,
      -1,   153,   127,   178,    -1,   153,   127,   141,   179,   142,
      -1,   153,   127,   141,   142,    -1,   276,   127,   177,    -1,
     276,   127,   178,    -1,    62,    -1,    25,    -1,    47,    -1,
      26,    -1,    75,    -1,    76,    -1,    66,    -1,   105,    -1,
      61,    -1,    17,   107,   153,    15,    32,   153,   249,    -1,
      17,   107,   153,    44,    32,   153,    -1,    17,    95,   153,
      89,    -1,   101,   259,   118,   252,    -1,   101,   259,   118,
      68,     8,    -1,    -1,   253,    -1,   121,   254,    -1,   255,
      -1,   254,   123,   255,    -1,   153,   127,     8,    -1,   101,
      31,    -1,   101,    29,    98,    -1,    98,   259,   111,    65,
      67,   260,    -1,    -1,    52,    -1,   100,    -1,    88,   115,
      -1,    88,    34,    -1,    90,    88,    -1,    97,    -1,    36,
      51,   153,    93,   262,   102,     8,    -1,    61,    -1,    25,
      -1,    47,    -1,   105,    -1,    62,    -1,    44,    51,   153,
      -1,    20,    59,   153,   110,    95,   153,    -1,    48,    95,
     153,    -1,    48,    87,   153,    -1,    48,    21,    -1,    96,
     269,   192,    -1,    10,    -1,    10,   146,   153,    -1,    96,
     200,    -1,   113,    95,   153,    -1,    81,    59,   153,    -1,
      36,    83,   153,   114,     8,   102,     8,    -1,    44,    83,
     153,   114,     8,    -1,   276,    -1,   153,    -1,   153,   277,
      -1,   278,    -1,   277,   278,    -1,    13,    -1,    14,    -1,
     147,   200,   148,    -1,   147,     8,   148,    -1,   200,   127,
     280,    -1,   280,   127,   200,    -1,     8,    -1,    -1,   282,
     285,    -1,    27,    -1,   284,   167,    -1,   200,    -1,   283,
      -1,   285,   143,   283,    -1,    45,   285,   281,   188,   192,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   177,   177,   178,   179,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   220,   221,   221,   221,   221,   221,   221,   222,
     222,   222,   222,   223,   223,   223,   223,   223,   224,   224,
     224,   224,   224,   225,   225,   225,   225,   225,   225,   225,
     226,   226,   226,   226,   227,   227,   227,   227,   227,   228,
     228,   228,   228,   228,   228,   229,   229,   229,   229,   230,
     234,   234,   234,   240,   241,   242,   246,   247,   251,   252,
     260,   261,   268,   270,   274,   278,   285,   286,   287,   291,
     304,   311,   313,   318,   327,   343,   344,   348,   349,   352,
     354,   355,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   371,   372,   375,   377,   381,   385,   386,   387,   391,
     396,   400,   407,   415,   424,   434,   439,   444,   449,   454,
     459,   464,   469,   474,   479,   484,   489,   494,   499,   504,
     509,   514,   519,   524,   529,   537,   541,   542,   547,   553,
     559,   565,   571,   572,   573,   574,   575,   579,   580,   591,
     592,   593,   597,   603,   610,   616,   623,   624,   627,   629,
     632,   634,   641,   645,   651,   653,   659,   661,   665,   676,
     678,   682,   686,   693,   694,   698,   699,   700,   703,   705,
     709,   714,   721,   723,   727,   731,   732,   736,   741,   746,
     752,   757,   765,   770,   777,   787,   788,   789,   790,   791,
     792,   793,   794,   795,   797,   798,   799,   800,   801,   802,
     803,   804,   805,   806,   807,   808,   809,   810,   811,   812,
     813,   814,   815,   816,   817,   818,   819,   820,   824,   825,
     826,   827,   828,   829,   830,   831,   832,   833,   834,   835,
     839,   840,   844,   845,   849,   850,   854,   855,   859,   860,
     866,   869,   871,   875,   876,   877,   878,   879,   880,   881,
     882,   883,   888,   893,   898,   903,   912,   913,   916,   918,
     926,   931,   936,   941,   942,   943,   947,   952,   957,   962,
     971,   972,   976,   977,   978,   990,   991,   995,   996,   997,
     998,   999,  1006,  1007,  1008,  1012,  1013,  1019,  1027,  1028,
    1031,  1033,  1037,  1038,  1042,  1043,  1047,  1048,  1052,  1056,
    1057,  1061,  1062,  1063,  1064,  1065,  1068,  1069,  1073,  1074,
    1078,  1084,  1094,  1102,  1106,  1113,  1114,  1121,  1131,  1137,
    1139,  1143,  1148,  1152,  1159,  1161,  1165,  1166,  1172,  1180,
    1181,  1187,  1191,  1197,  1205,  1206,  1210,  1224,  1230,  1234,
    1239,  1253,  1264,  1265,  1266,  1267,  1268,  1269,  1270,  1271,
    1272,  1276,  1284,  1291,  1302,  1306,  1313,  1314,  1318,  1322,
    1323,  1327,  1331,  1338,  1345,  1351,  1352,  1353,  1357,  1358,
    1359,  1360,  1366,  1377,  1378,  1379,  1380,  1381,  1386,  1397,
    1409,  1418,  1427,  1437,  1445,  1446,  1450,  1460,  1471,  1482,
    1493,  1504,  1505,  1509,  1512,  1513,  1517,  1518,  1519,  1520,
    1524,  1525,  1529,  1534,  1536,  1540,  1549,  1553,  1561,  1562,
    1566
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
     153,   153,   153,   154,   154,   154,   155,   155,   156,   156,
     157,   157,   158,   158,   159,   159,   160,   160,   160,   161,
     162,   163,   163,   163,   164,   165,   165,   166,   166,   167,
     167,   167,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   169,   169,   170,   170,   171,   172,   172,   172,   173,
     173,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   175,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   177,   177,   178,
     178,   178,   179,   179,   180,   180,   181,   181,   182,   182,
     183,   183,   184,   184,   185,   185,   186,   186,   187,   188,
     188,   189,   189,   190,   190,   191,   191,   191,   192,   192,
     193,   193,   194,   194,   195,   196,   196,   197,   197,   197,
     197,   197,   198,   198,   199,   200,   200,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     202,   202,   203,   203,   204,   204,   205,   205,   206,   206,
     207,   208,   208,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   210,   210,   211,   211,
     212,   212,   212,   212,   212,   212,   213,   213,   213,   213,
     214,   214,   215,   215,   215,   216,   216,   217,   217,   217,
     217,   217,   218,   218,   218,   219,   219,   220,   221,   221,
     222,   222,   223,   223,   224,   224,   225,   225,   226,   227,
     227,   228,   228,   228,   228,   228,   229,   229,   230,   230,
     231,   232,   233,   234,   234,   235,   235,   236,   236,   237,
     237,   238,   238,   239,   240,   240,   241,   241,   242,   243,
     243,   244,   245,   246,   247,   247,   248,   248,   248,   248,
     248,   248,   249,   249,   249,   249,   249,   249,   249,   249,
     249,   250,   250,   250,   251,   251,   252,   252,   253,   254,
     254,   255,   256,   257,   258,   259,   259,   259,   260,   260,
     260,   260,   261,   262,   262,   262,   262,   262,   263,   264,
     265,   266,   267,   268,   269,   269,   270,   271,   272,   273,
     274,   275,   275,   276,   277,   277,   278,   278,   278,   278,
     279,   279,   280,   281,   281,   282,   283,   284,   285,   285,
     286
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
       1,     1,     1,     1,     3,     2,     1,     1,     1,     8,
       1,     9,     0,     2,     1,     3,     1,     1,     1,     0,
       3,     0,     2,     4,    11,     1,     3,     1,     2,     0,
       1,     2,     1,     4,     4,     4,     4,     4,     4,     3,
       5,     1,     3,     0,     1,     2,     1,     3,     3,     4,
       1,     3,     3,     5,     6,     3,     4,     5,     3,     3,
       3,     3,     3,     1,     5,     5,     5,     3,     3,     3,
       3,     3,     3,     3,     4,     3,     1,     1,     4,     3,
       3,     1,     1,     4,     4,     4,     3,     1,     2,     1,
       2,     1,     1,     3,     1,     3,     1,     1,     0,     4,
       0,     1,     1,     3,     0,     2,     0,     1,     5,     0,
       1,     3,     5,     1,     3,     1,     2,     2,     0,     1,
       2,     4,     0,     1,     2,     1,     3,     3,     3,     5,
       6,     3,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     3,     4,     4,     4,
       4,     4,     4,     4,     3,     6,     6,     3,     8,    14,
       1,     3,     1,     1,     3,     5,     1,     1,     1,     1,
       2,     0,     2,     1,     2,     2,     3,     1,     1,     1,
       2,     4,     4,     3,     4,     4,     1,     1,     0,     2,
       4,     4,     4,     3,     4,     4,     7,     5,     5,     9,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     7,     1,     1,
       0,     3,     1,     1,     1,     3,     1,     3,     3,     1,
       3,     1,     1,     1,     3,     2,     0,     2,     1,     3,
       3,     4,     6,     1,     3,     1,     3,     1,     3,     0,
       2,     1,     3,     3,     0,     1,     1,     1,     3,     1,
       1,     3,     3,     6,     1,     3,     3,     3,     5,     4,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     7,     6,     4,     4,     5,     0,     1,     2,     1,
       3,     3,     2,     3,     6,     0,     1,     1,     2,     2,
       2,     1,     7,     1,     1,     1,     1,     1,     3,     6,
       3,     3,     2,     3,     1,     3,     2,     3,     3,     7,
       5,     1,     1,     2,     1,     2,     1,     1,     3,     3,
       3,     3,     1,     0,     2,     1,     2,     1,     1,     3,
       5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   315,     0,   312,     0,     0,   360,   359,
       0,     0,   318,     0,   319,   313,     0,   395,   395,     0,
       0,     0,     0,     2,     3,    83,    86,    88,    90,    87,
       7,     8,     9,   314,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    29,    15,    16,    17,    24,    18,    19,
      20,    21,    22,    23,    25,    26,    27,    28,    30,    31,
       0,     0,     0,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    79,
      48,    49,    50,    51,    52,    81,    53,    54,    55,    56,
      58,    57,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    82,    72,    73,    74,    75,
      76,    77,    78,    80,     0,     0,     0,     0,     0,     0,
     412,     0,     0,     0,    32,   216,   218,   219,   432,   221,
     414,   220,    36,     0,    40,     0,     0,    45,    46,   217,
       0,     0,    51,     0,     0,    69,    77,     0,   107,     0,
       0,     0,   215,     0,   105,   109,   112,   243,   198,     0,
     244,   245,     0,     0,     0,    43,     0,     0,    65,     0,
       0,     0,     0,   392,   271,   396,   286,   271,   278,   279,
     277,   397,   271,   287,   271,   202,   273,   270,     0,     0,
     316,     0,   121,     0,     1,     0,     4,    85,     0,   271,
       0,     0,     0,     0,     0,     0,     0,   408,     0,   411,
     410,   418,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    32,    51,
       0,   222,   223,     0,   267,   266,     0,     0,   426,   427,
       0,   423,   424,     0,     0,     0,   110,   108,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   413,   199,     0,     0,
       0,     0,     0,     0,     0,   310,   311,   309,   308,   307,
     293,   305,     0,     0,     0,   271,     0,   393,     0,   362,
     275,   274,   361,     0,   280,   203,   288,   386,   417,     0,
       0,   437,   438,   109,   433,     0,     0,    84,   320,   358,
     383,     0,     0,     0,   167,   169,   333,   171,     0,     0,
     331,   332,   345,   349,   343,     0,     0,     0,   341,     0,
     263,     0,   254,   262,     0,   260,   415,     0,   262,     0,
       0,     0,     0,     0,   119,     0,     0,     0,     0,     0,
       0,     0,   257,     0,     0,     0,   241,     0,   242,     0,
     432,     0,   425,    99,   123,   106,   112,   111,   233,   234,
     240,   239,   231,   230,   237,   430,   238,   228,   229,   236,
     235,   224,   225,   226,   227,   232,   200,   246,     0,   431,
     294,   295,     0,     0,     0,     0,   301,   303,   292,   302,
       0,   300,   304,   291,   290,     0,   271,   276,   271,   272,
       0,   204,   205,     0,     0,   283,     0,     0,     0,   384,
     387,     0,     0,   364,     0,   122,   436,   435,     0,   189,
       0,     0,     0,     0,     0,     0,   168,   170,   347,   335,
     172,     0,     0,     0,     0,   404,   405,   403,   407,   406,
       0,     0,   157,     0,    40,     0,     0,    45,   161,     0,
      50,    77,     0,   156,   125,   126,   130,   143,     0,   162,
     420,     0,     0,     0,   248,     0,   113,   251,     0,   118,
     253,   252,   117,   249,   250,   114,     0,   115,     0,     0,
     116,     0,     0,     0,   269,   268,   264,   429,   428,     0,
     178,   124,     0,   247,     0,   298,   297,     0,   306,     0,
     281,   282,     0,     0,   285,   289,   284,   385,     0,   388,
     389,     0,     0,   202,     0,   439,     0,   198,   190,   434,
     323,   322,   324,     0,     0,     0,   382,   409,   334,     0,
     346,     0,   354,   344,   350,   351,   342,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    92,
       0,   261,   120,     0,     0,     0,     0,     0,     0,   180,
     186,   201,     0,     0,     0,     0,   401,   394,   208,   211,
       0,   207,   206,     0,     0,     0,   366,   367,   365,   363,
     370,   371,     0,   440,   321,     0,     0,   336,   326,   373,
     375,   374,   380,   372,   378,   376,   377,   379,   381,   173,
     348,   355,     0,     0,   402,   419,     0,     0,     0,     0,
     166,   159,     0,     0,   160,   128,   127,     0,     0,   135,
       0,   153,     0,   151,   131,   142,   152,   132,   155,   139,
     148,   138,   147,   140,   149,   141,   150,     0,     0,     0,
       0,   256,   255,     0,   265,     0,     0,   181,     0,     0,
     184,   187,   296,     0,   399,   398,   400,     0,     0,   212,
       0,   391,   390,   369,     0,    58,   195,   191,   193,   325,
       0,     0,   329,     0,     0,   317,   357,   356,   353,   354,
     352,   165,   158,   164,   163,   129,     0,     0,   174,   176,
     177,     0,   154,   136,     0,    97,    98,    96,    93,    94,
      89,   422,     0,   421,     0,     0,     0,   101,     0,     0,
       0,   189,     0,     0,   209,     0,     0,   368,     0,   196,
     197,     0,   328,     0,     0,   337,   338,   327,   137,   145,
     146,   144,     0,   133,     0,     0,   258,     0,     0,     0,
      91,   182,   179,     0,   185,   198,   299,   214,   213,   210,
     192,   194,   330,     0,     0,   175,   134,    95,     0,   100,
     102,     0,     0,   202,   340,   339,     0,     0,   183,   188,
     104,     0,   103,     0,     0,   259
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   113,   152,    24,    25,    26,    27,   659,
     718,   719,   499,   727,   760,    28,   153,   154,   247,   155,
     364,   500,   328,   464,   465,   466,   467,   468,   440,   321,
     441,   710,   711,   580,   668,   762,   731,   670,   671,   527,
     528,   687,   688,   266,   267,   294,   295,   411,   412,   678,
     679,   338,   157,   334,   335,   236,   237,   496,    29,   289,
     187,   188,   416,    30,    31,   403,   404,   280,   281,    32,
      33,    34,    35,   432,   532,   533,   607,   608,   691,   322,
     695,   745,   746,    36,    37,   323,   324,   442,   444,   544,
     545,   622,   698,    38,    39,    40,    41,    42,   422,   423,
     618,    43,    44,   419,   420,   519,   520,    45,    46,    47,
     170,   587,    48,   450,    49,    50,    51,    52,    53,    54,
     158,    55,    56,    57,    58,    59,   159,   160,   241,   242,
     161,   162,   429,   430,   302,   303,   304,   197
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -673
static const short yypact[] =
{
    4327,   139,    42,  -673,  3796,  -673,    43,    93,  -673,  -673,
      48,    38,  -673,    87,  -673,  -673,   736,  2927,   532,   -23,
       0,  3796,   161,  -673,   -19,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,   111,  -673,  -673,  -673,  3796,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    3796,  3796,  3796,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,    69,  3796,  3796,  3796,  3796,  3796,
    -673,  3796,  3796,  3796,    73,  -673,  -673,  -673,  -673,  -673,
      40,  -673,    97,    99,   101,   103,   107,   113,   120,  -673,
     123,   130,   136,   138,   147,   149,   151,  1588,  -673,  1588,
    1588,  3259,    26,   -16,  -673,  3366,    96,  -673,   187,   230,
     240,  -673,   124,   194,   228,  3903,  3796,  2820,   216,   202,
     219,  3475,   237,  -673,   269,  -673,  -673,   269,  -673,  -673,
    -673,  -673,   269,  -673,   269,   259,  -673,  -673,  3796,   221,
    -673,  3796,  -673,   -43,  -673,  1588,    36,  -673,  3796,   269,
     251,    62,   231,    24,   249,   229,   -36,  -673,   232,  -673,
    -673,  -673,   878,  3796,  1588,  1730,    13,  1730,  1730,   203,
    1588,  1730,  1730,  1588,  1588,  1588,  1588,   205,   209,   210,
     234,  -673,  -673,  4046,  -673,  -673,   -54,   217,  -673,  -673,
    1872,    35,  -673,  2227,  1020,  3796,  -673,  -673,  1588,  1588,
    1588,  1588,  1588,  1588,  1588,  1588,  1588,  1588,  1588,  1588,
    1588,  1588,  1588,  1588,  1588,   368,  -673,  -673,    -3,  1588,
    2820,  2820,   252,   254,   324,  -673,  -673,  -673,  -673,  -673,
     244,  -673,  2349,   317,   280,    19,   283,  -673,   380,  -673,
    -673,  -673,  -673,  3796,  -673,  -673,    79,    82,  -673,  3796,
    3796,  4343,  -673,  3366,   -15,  1162,   369,  -673,   253,  -673,
    -673,   358,   360,   298,  -673,  -673,  -673,  -673,    98,    10,
    -673,  -673,  -673,   272,  -673,   188,   387,  1998,  -673,   388,
     270,  1304,  -673,  4328,    58,  -673,  -673,  4067,  4343,   110,
    3796,   274,   117,   127,  -673,  4087,   131,   133,   534,   563,
    3904,  4110,  -673,  1446,  1588,  1588,  -673,  3259,  -673,  2469,
     271,   396,  -673,  -673,   -36,  -673,  4343,  -673,  -673,  -673,
     275,  4357,   493,  2137,   513,  -673,   513,   312,   312,   312,
     312,    56,    56,  -673,  -673,  -673,   277,  -673,   339,   513,
     244,   244,   291,  3045,   424,  2820,  -673,  -673,  -673,  -673,
     429,  -673,  -673,  -673,  -673,   370,   269,  -673,   269,  -673,
     309,   297,  -673,   344,   445,  -673,   353,   448,  3796,  -673,
    -673,    33,    94,  -673,   330,  -673,  -673,  -673,  1588,   379,
    1588,  3582,   343,  3796,  3796,  3796,  -673,  -673,  -673,  -673,
    -673,   144,   155,    24,   322,  -673,  -673,  -673,  -673,  -673,
     364,   365,  -673,   327,   333,   334,   336,   338,  -673,   340,
     342,   345,  1998,    35,   347,  -673,  -673,  -673,   153,  -673,
    -673,  1020,   348,  3796,  -673,  1730,  -673,  -673,   349,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  1588,  -673,  1588,  1588,
    -673,  3925,  3950,   357,  -673,  -673,  -673,  -673,  -673,   384,
     432,  -673,   482,  -673,    45,  -673,  -673,   362,  -673,   109,
    -673,  -673,  2120,  3796,  -673,  -673,  -673,  -673,   366,   371,
    -673,    47,  3796,   259,    78,  -673,   465,   187,  -673,   354,
    -673,  -673,  -673,   158,   359,   791,  -673,  -673,  -673,    45,
    -673,   490,    53,  -673,   356,  -673,  -673,   494,   495,  3796,
     372,  3796,   363,   373,  3796,   496,   374,   -22,  1998,    78,
      14,     2,    64,    68,    78,    78,    78,    78,   443,   391,
     453,  -673,  -673,  4183,  4206,  3971,  2469,  1020,   375,   501,
     390,  -673,   164,   398,   -10,   425,  -673,  -673,  -673,  -673,
    3796,   399,  -673,   506,  3796,    11,  -673,  -673,  -673,  -673,
    -673,  -673,  2589,  -673,  -673,  3582,    37,   -35,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  3689,    37,  -673,  -673,    35,   395,   400,   401,
    -673,  -673,   404,   407,  -673,  -673,  -673,   417,   427,  -673,
      23,  -673,   474,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,  -673,    28,  3152,   411,
    3796,  -673,  -673,   415,  -673,    29,   477,  -673,   531,   507,
     503,  -673,  -673,    45,  -673,  -673,  -673,   435,   166,  -673,
     556,  -673,  -673,  -673,   168,   430,   115,   431,  -673,  -673,
      22,   170,  -673,   565,   359,  -673,  -673,  -673,  -673,   552,
    -673,  -673,  -673,  -673,  -673,  -673,    78,    78,  -673,   437,
     438,   433,  -673,  -673,    23,  -673,  -673,  -673,   439,  -673,
    -673,    35,   436,  -673,  1730,  3796,   546,   514,  2702,   505,
    2702,   379,   172,    45,  -673,  3796,   446,  -673,   447,  -673,
    -673,  2702,  -673,    37,   466,   452,  -673,  -673,  -673,  -673,
    -673,  -673,   591,  -673,   458,  3152,  -673,   176,  2702,   598,
    -673,  -673,   463,   580,  -673,   187,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,   601,   565,  -673,  -673,  -673,   467,   431,
     468,  2702,  2702,   259,  -673,  -673,   471,   608,  -673,   431,
    -673,  1730,  -673,   182,   473,  -673
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -673,  -673,  -673,    -6,    -4,  -673,   442,  -673,   300,  -673,
    -673,  -122,  -673,  -673,  -673,   135,    75,   409,   332,  -673,
      -1,  -673,  -283,  -429,  -673,   -75,  -673,  -592,  -141,  -499,
    -498,  -673,   -42,  -673,  -673,  -673,  -673,  -673,  -673,   -58,
    -673,  -672,   -67,  -518,  -673,  -521,  -673,  -673,   162,  -673,
     -57,   108,  -673,  -214,   204,  -673,   323,   105,  -673,  -163,
    -673,  -673,  -673,  -673,  -673,   289,  -673,   -66,   288,  -673,
    -673,  -673,  -673,  -673,   102,  -673,  -673,     7,  -673,  -438,
    -673,  -673,   -90,  -673,  -673,  -673,   260,  -673,  -673,  -673,
      81,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,   180,
    -673,  -673,  -673,  -673,  -673,  -673,   114,  -673,  -673,  -673,
     691,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,  -673,
    -673,  -673,  -673,  -673,  -673,  -673,    50,  -286,  -673,   470,
    -673,   459,  -673,  -673,   284,  -673,   285,  -673
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -433
static const short yytable[] =
{
     114,   339,   599,   342,   343,   542,   582,   346,   347,   603,
     686,   169,   427,   424,   290,   314,   314,   192,   438,   291,
     193,   292,   597,   639,   674,   601,   195,   314,   314,   314,
     315,   708,   316,   557,   243,   199,   309,   713,   317,   238,
     239,   469,   314,   315,   693,   316,   238,   239,   238,   239,
     314,   317,   314,   315,   340,   299,   200,   201,   202,   120,
     638,   317,   320,   645,   648,   650,   652,   654,   656,   314,
     315,   621,   643,   314,   315,   387,   646,   311,   317,   725,
     641,   501,   317,   314,   315,   327,   779,   288,   190,   357,
    -422,   358,   317,   413,   115,   191,  -416,   684,   248,   118,
     300,    62,   558,   436,   437,   675,   312,   300,   694,   414,
     789,   204,   205,   192,   207,   208,   206,   209,   210,   211,
     635,   196,   407,   -67,   156,   121,   116,   244,   428,   636,
     249,   119,   305,   122,   739,   388,   761,   306,   248,   523,
     642,   402,   709,   117,   400,   400,   123,   235,   341,   686,
     417,   246,   439,   683,   740,   640,   400,   400,   318,   273,
     521,   194,   274,   279,   439,   319,   686,   286,   692,   714,
     249,   318,   244,   240,   198,   732,   469,   559,   690,   400,
     240,   318,   240,   415,   296,   699,   213,   298,   595,   788,
     686,   262,   263,   264,   308,  -345,  -345,   584,   318,   585,
     474,   475,   318,   418,   390,   391,   586,   749,   751,   336,
     203,   560,   318,   445,   212,   327,   709,   561,   495,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,    60,   446,   424,   522,   214,   192,
     215,   367,   216,   510,   217,   511,    61,   783,   218,   447,
     448,   269,   477,   475,   219,   231,   265,   232,   233,   480,
     475,   220,   790,   627,   221,   629,   279,   279,   632,   481,
     475,   222,   469,   483,   475,   484,   475,   223,   401,   224,
     562,   563,   564,   565,   566,   567,   538,   539,   225,   410,
     226,   568,   227,   449,   268,   421,   425,   540,   541,   246,
     604,   605,   320,   301,  -421,   772,   672,   539,   734,   735,
     737,   539,   742,   743,   766,   539,   469,   248,   778,   475,
     333,   270,   337,   463,   794,   475,   271,  -397,   345,   282,
     283,   348,   349,   350,   351,   287,   478,   288,   293,   297,
     310,   313,   325,   326,   359,   344,   329,   352,   361,   249,
     353,   354,   366,   235,   248,   494,   368,   369,   370,   371,
     372,   373,   374,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   723,   355,   394,   389,   395,   392,
     596,   393,   405,   600,   406,   171,   249,   408,   409,   401,
     433,   279,   434,   435,   431,   451,   470,  -432,   619,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   366,   518,   443,   479,   503,   637,   497,
     502,   644,   647,   649,   651,   653,   655,   531,   176,   535,
     536,   537,   504,   507,   436,   495,   512,   509,   248,   233,
     513,   177,   469,   514,   469,   260,   261,   262,   263,   264,
     515,   178,   516,   179,   180,   469,   517,   524,   463,   526,
     534,   333,   491,   492,   546,   320,   547,   548,   549,   570,
     249,   558,   469,   182,   550,   551,   183,   552,   185,   553,
     577,   554,   320,   555,   576,   579,   556,   581,   186,   583,
     569,   572,   602,   593,   594,   469,   469,   428,   620,   623,
     606,   657,   624,   625,   633,   630,   667,   628,   591,   410,
     757,   660,   669,   676,   681,   631,   634,   666,   421,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   658,   248,   301,   701,   301,   673,
     680,   706,   702,   703,   498,   626,   704,   626,   171,   705,
     626,   707,   712,   720,   463,   248,   724,   726,   728,   730,
     729,   172,   733,   173,   736,   748,   750,   249,   744,   174,
     621,   738,   494,   758,   741,   753,   248,   793,   756,   366,
     539,   752,   755,   759,   175,   763,   677,   249,   769,   770,
     518,   176,   767,   773,   573,   774,   574,   575,   463,   775,
     776,   531,   320,   780,   177,   248,   781,   782,   249,   784,
     786,   787,   791,   792,   178,   795,   179,   180,   697,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   472,   181,   777,   578,   426,   182,   249,   307,   183,
     184,   185,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   186,   665,   365,   717,   764,   721,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   754,   765,   771,   592,   485,   486,   768,   571,
     493,   664,   506,   508,   785,   366,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   747,   598,   543,   700,   487,   488,   689,   682,   189,
     722,   362,   525,   375,     0,   529,     0,     0,     0,     0,
       0,   192,     0,     0,   463,     0,   463,     0,     0,     0,
       0,   677,     0,     0,     0,     0,     0,   463,     0,   124,
     125,   126,   127,     0,   128,   129,   130,     0,     0,     0,
     131,   717,    64,     0,   463,     0,    65,    66,   132,    68,
       0,   133,    69,     0,     0,     0,     0,    70,     0,     0,
       0,   134,     0,     0,     0,     0,     0,   463,   463,   135,
       0,     0,     0,   136,    72,     0,     0,    73,    74,    75,
     137,   138,     0,   139,   140,     0,     0,     0,   141,     0,
       0,    78,    79,    80,    81,     0,    82,   142,    84,   143,
       0,     0,     0,    85,     0,     0,   609,   610,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,   144,    94,
      95,    96,     0,    97,     0,     0,    98,     0,   611,    99,
     100,   101,   145,     0,   103,   104,     0,   105,     0,   106,
     107,   108,   612,   613,   109,   110,   146,   614,   112,     0,
       0,     0,     0,     0,     0,     0,   615,   616,     0,     0,
     147,   148,     0,     0,   149,     0,     0,   150,     0,     0,
     151,   228,   125,   126,   127,     0,   330,   129,     0,     0,
       0,     0,   131,     0,    64,     0,   617,     0,    65,    66,
      67,    68,     0,   133,    69,     0,     0,     0,     0,    70,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
       0,   135,     0,     0,     0,   136,    72,     0,     0,    73,
      74,    75,    76,    77,     0,   139,   140,     0,     0,     0,
     141,     0,     0,    78,    79,    80,    81,     0,    82,   229,
      84,   230,     0,     0,     0,    85,     0,     0,     0,     0,
      86,    87,    88,    89,    90,    91,    92,     0,    93,     0,
     144,    94,    95,    96,     0,    97,     0,     0,    98,     0,
       0,    99,   100,   101,   102,     0,   103,   104,     0,   105,
       0,   106,   107,   108,     0,     0,   109,   110,   146,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   147,     0,     0,     0,   149,     0,     0,   331,
     332,     0,   151,   228,   125,   126,   127,     0,   128,   129,
       0,     0,     0,     0,   131,     0,    64,     0,     0,     0,
      65,    66,   132,    68,     0,   133,    69,     0,     0,     0,
       0,    70,     0,     0,     0,   134,     0,     0,     0,     0,
       0,     0,     0,   135,     0,     0,     0,   136,    72,     0,
       0,    73,    74,    75,   137,   138,     0,   139,   140,     0,
       0,     0,   141,     0,     0,    78,    79,    80,    81,     0,
      82,   142,    84,   143,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,   144,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   145,     0,   103,   104,
       0,   105,     0,   106,   107,   108,     0,     0,   109,   110,
     146,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   147,   148,     0,     0,   149,     0,
       0,   150,     0,     0,   151,   124,   125,   126,   127,     0,
     128,   129,     0,     0,     0,     0,   131,     0,    64,     0,
       0,     0,    65,    66,   132,    68,     0,   133,    69,     0,
       0,     0,     0,    70,     0,     0,     0,   134,     0,     0,
       0,     0,     0,     0,     0,   135,     0,     0,     0,   136,
      72,     0,     0,    73,    74,    75,   137,   138,     0,   139,
     140,     0,     0,     0,   141,     0,     0,    78,    79,    80,
      81,     0,    82,   142,    84,   143,     0,     0,     0,    85,
       0,     0,     0,     0,    86,    87,    88,    89,    90,    91,
      92,     0,    93,     0,   144,    94,    95,    96,     0,    97,
       0,     0,    98,     0,     0,    99,   100,   101,   145,     0,
     103,   104,     0,   105,     0,   106,   107,   108,     0,     0,
     109,   110,   146,     0,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   147,   148,     0,     0,
     149,     0,     0,   150,     0,     0,   151,   228,   125,   126,
     127,     0,   128,   129,     0,     0,     0,     0,   131,     0,
      64,     0,     0,     0,    65,    66,    67,    68,     0,   133,
      69,     0,     0,     0,     0,    70,     0,     0,     0,    71,
       0,     0,     0,     0,     0,     0,     0,   135,     0,     0,
       0,   136,    72,     0,     0,    73,    74,    75,    76,    77,
       0,   139,   140,     0,     0,     0,   141,     0,     0,    78,
      79,    80,    81,     0,    82,   229,    84,   230,     0,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,     0,   144,    94,    95,    96,
     471,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     102,     0,   103,   104,     0,   105,     0,   106,   107,   108,
       0,     0,   109,   110,   146,     0,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   147,     0,
       0,     0,   149,     0,     0,   150,     0,     0,   151,   228,
     125,   126,   127,     0,   330,   129,     0,     0,     0,     0,
     131,     0,    64,     0,     0,     0,    65,    66,    67,    68,
       0,   133,    69,     0,     0,     0,     0,    70,     0,     0,
       0,    71,     0,     0,     0,     0,     0,     0,     0,   135,
       0,     0,     0,   136,    72,     0,     0,    73,    74,    75,
      76,    77,     0,   139,   140,     0,     0,     0,   141,     0,
       0,    78,    79,    80,    81,     0,    82,   229,    84,   230,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,   144,    94,
      95,    96,     0,    97,     0,     0,    98,     0,     0,    99,
     100,   101,   102,     0,   103,   104,     0,   105,     0,   106,
     107,   108,     0,     0,   109,   110,   146,     0,   112,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     147,     0,     0,     0,   149,     0,     0,   150,   332,     0,
     151,   228,   125,   126,   127,     0,   128,   129,     0,     0,
       0,     0,   131,     0,    64,     0,     0,     0,    65,    66,
      67,    68,     0,   133,    69,     0,     0,     0,     0,    70,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
       0,   135,     0,     0,     0,   136,    72,     0,     0,    73,
      74,    75,    76,    77,     0,   139,   140,     0,     0,     0,
     141,     0,     0,    78,    79,    80,    81,     0,    82,   229,
      84,   230,     0,     0,     0,    85,     0,     0,     0,     0,
      86,    87,    88,    89,    90,    91,    92,     0,    93,     0,
     144,    94,    95,    96,     0,    97,     0,     0,    98,     0,
       0,    99,   100,   101,   102,     0,   103,   104,     0,   105,
       0,   106,   107,   108,     0,     0,   109,   110,   146,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   147,     0,     0,     0,   149,     0,     0,   150,
       0,     0,   151,   228,   125,   126,   127,     0,   330,   129,
       0,     0,     0,     0,   131,     0,    64,     0,     0,     0,
      65,    66,    67,    68,     0,   133,    69,     0,     0,     0,
       0,    70,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,   135,     0,     0,     0,   136,    72,     0,
       0,    73,    74,    75,    76,    77,     0,   139,   140,     0,
       0,     0,   141,     0,     0,    78,    79,    80,    81,     0,
      82,   229,    84,   230,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,   144,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
       0,   105,     0,   106,   107,   108,     0,     0,   109,   110,
     146,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   147,     0,     0,     0,   149,     0,
       0,   150,     0,     0,   151,   228,   125,   126,   127,     0,
     360,   129,     0,     0,     0,     0,   131,     0,    64,     0,
       0,     0,    65,    66,    67,    68,     0,   133,    69,     0,
       0,     0,     0,    70,     0,     0,     0,    71,     0,     0,
       0,     0,     0,     0,     0,   135,     0,     0,     0,   136,
      72,     0,     0,    73,    74,    75,    76,    77,     0,   139,
     140,     0,     0,     0,   141,     0,     0,    78,    79,    80,
      81,     0,    82,   229,    84,   230,     0,     0,     0,    85,
       0,     0,     0,     0,    86,    87,    88,    89,    90,    91,
      92,     0,    93,     0,   144,    94,    95,    96,     0,    97,
       0,     0,    98,     0,     0,    99,   100,   101,   102,     0,
     103,   104,     0,   105,     0,   106,   107,   108,     0,     0,
     109,   110,   146,     0,   112,     0,     0,     0,     0,     0,
       0,    63,   452,     0,     0,     0,   147,     0,     0,     0,
     149,     0,     0,   150,    64,     0,   151,     0,    65,    66,
      67,    68,     0,   453,    69,     0,     0,     0,     0,    70,
       0,     0,     0,   454,     0,     0,     0,     0,     0,     0,
       0,   455,     0,   456,     0,     0,    72,     0,     0,    73,
      74,    75,   457,    77,     0,   458,     0,     0,     0,     0,
     459,     0,     0,    78,    79,    80,    81,     0,   460,    83,
      84,     0,     0,     0,     0,    85,     0,     0,     0,     0,
      86,    87,    88,    89,    90,    91,    92,     0,    93,     0,
       0,    94,    95,    96,     0,    97,     0,     0,    98,     0,
       0,    99,   100,   101,   102,     0,   103,   104,     0,   105,
       0,   106,   107,   108,     0,     0,   109,   110,   461,     0,
     112,     0,     0,    63,     0,   588,     0,     0,   589,     0,
       0,     0,     0,     0,     0,     0,    64,     0,     0,   462,
      65,    66,    67,    68,     0,     0,    69,     0,     0,     0,
       0,    70,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,     0,   248,
       0,     0,     0,     0,     0,    78,    79,    80,    81,     0,
      82,    83,    84,     0,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,   249,     0,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
      63,   105,     0,   106,   107,   108,     0,     0,   109,   110,
     111,     0,   112,    64,     0,     0,     0,    65,    66,    67,
      68,     0,     0,    69,     0,     0,     0,     0,    70,     0,
       0,   590,    71,     0,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,     0,    82,    83,    84,
       0,     0,     0,     0,    85,     0,     0,     0,     0,    86,
      87,    88,    89,    90,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,    98,     0,     0,
      99,   100,   101,   102,     0,   103,   104,     0,   105,     0,
     106,   107,   108,     0,     0,   109,   110,   111,     0,   112,
       0,     0,    63,     0,   314,     0,     0,   396,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,   363,    65,
      66,    67,    68,     0,     0,    69,     0,     0,     0,     0,
      70,     0,     0,     0,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   397,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,     0,    82,
      83,    84,     0,     0,     0,     0,    85,   398,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   100,   101,   102,     0,   103,   104,     0,
     105,   399,   106,   107,   108,     0,     0,   109,   110,   111,
       0,   112,    63,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,   400,     0,    64,     0,     0,     0,    65,
      66,    67,    68,     0,     0,    69,     0,     0,     0,     0,
      70,     0,     0,     0,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,     0,    82,
      83,    84,     0,     0,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   100,   101,   102,     0,   103,   104,     0,
     105,     0,   106,   107,   108,     0,     0,   109,   110,   111,
       0,   112,    63,   452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   400,     0,    64,     0,     0,     0,    65,
      66,    67,    68,     0,   453,    69,     0,     0,     0,     0,
      70,     0,     0,     0,   454,     0,     0,     0,     0,     0,
       0,     0,   455,     0,   456,     0,     0,    72,     0,     0,
      73,    74,    75,   457,    77,     0,   458,     0,     0,     0,
       0,   459,     0,     0,    78,    79,    80,    81,     0,    82,
      83,    84,     0,     0,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,   685,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   100,   101,   102,     0,   103,   104,     0,
     105,     0,   106,   107,   108,    63,   452,   109,   110,   461,
       0,   112,     0,     0,     0,     0,     0,     0,    64,     0,
       0,     0,    65,    66,    67,    68,     0,   453,    69,     0,
       0,     0,     0,    70,     0,     0,     0,   454,     0,     0,
       0,     0,     0,     0,     0,   455,     0,   456,     0,     0,
      72,     0,     0,    73,    74,    75,   457,    77,     0,   458,
       0,     0,     0,     0,   459,     0,     0,    78,    79,    80,
      81,     0,    82,    83,    84,     0,     0,     0,     0,    85,
       0,     0,     0,     0,    86,    87,    88,    89,    90,    91,
      92,     0,    93,     0,     0,    94,    95,    96,     0,    97,
       0,     0,    98,     0,     0,    99,   100,   101,   102,     0,
     103,   104,     0,   105,     0,   106,   107,   108,     0,     0,
     109,   110,   461,    63,   112,   275,   276,     0,   277,     0,
       0,     0,     0,     0,     0,     0,    64,     0,     0,     0,
      65,    66,    67,    68,     0,     0,    69,     0,     0,     0,
       0,    70,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    78,    79,    80,    81,     0,
      82,    83,    84,     0,     0,     0,     0,    85,   278,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,     0,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
      63,   105,     0,   106,   107,   108,     0,   163,   109,   110,
     111,     0,   112,    64,     0,     0,     0,    65,    66,    67,
      68,     0,     0,    69,     0,     0,   164,     0,    70,     0,
       0,     0,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,     0,     0,    73,   165,
      75,    76,    77,     0,     0,     0,   166,     0,     0,     0,
       0,     0,    78,    79,    80,    81,     0,    82,    83,    84,
       0,     0,     0,     0,   167,     0,     0,     0,     0,    86,
      87,    88,    89,    90,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,   168,     0,     0,
      99,   100,   101,   102,     0,   103,   104,     0,     0,     0,
     106,   107,   108,     0,     0,   109,   110,   111,    63,   112,
     505,     0,     0,   396,     0,     0,     0,     0,     0,     0,
       0,    64,     0,     0,     0,    65,    66,    67,    68,     0,
       0,    69,     0,     0,     0,     0,    70,     0,     0,     0,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,     0,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      78,    79,    80,    81,     0,    82,    83,    84,     0,     0,
       0,     0,    85,     0,     0,     0,     0,    86,    87,    88,
      89,    90,    91,    92,     0,    93,     0,     0,    94,    95,
      96,     0,    97,     0,     0,    98,     0,     0,    99,   100,
     101,   102,     0,   103,   104,    63,   105,   715,   106,   107,
     108,     0,     0,   109,   110,   111,     0,   112,    64,     0,
       0,     0,    65,    66,    67,    68,     0,     0,    69,     0,
       0,     0,     0,    70,     0,     0,     0,    71,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,     0,     0,    73,    74,    75,    76,    77,     0,   716,
       0,     0,     0,     0,     0,     0,     0,    78,    79,    80,
      81,     0,    82,    83,    84,     0,     0,     0,     0,    85,
       0,     0,     0,     0,    86,    87,    88,    89,    90,    91,
      92,     0,    93,     0,     0,    94,    95,    96,     0,    97,
       0,     0,    98,     0,     0,    99,   100,   101,   102,     0,
     103,   104,    63,   105,     0,   106,   107,   108,     0,     0,
     109,   110,   111,     0,   112,    64,     0,     0,     0,    65,
      66,    67,    68,     0,     0,    69,     0,     0,     0,     0,
      70,     0,     0,     0,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,   234,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,     0,    82,
      83,    84,     0,     0,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   100,   101,   102,     0,   103,   104,    63,
     105,     0,   106,   107,   108,     0,     0,   109,   110,   111,
       0,   112,    64,     0,   245,     0,    65,    66,    67,    68,
       0,     0,    69,     0,     0,     0,     0,    70,     0,     0,
       0,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,     0,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    78,    79,    80,    81,     0,    82,    83,    84,     0,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,     0,    94,
      95,    96,     0,    97,     0,     0,    98,     0,     0,    99,
     100,   101,   102,     0,   103,   104,     0,   105,    63,   106,
     107,   108,     0,   284,   109,   110,   111,     0,   112,     0,
       0,    64,     0,     0,     0,    65,    66,    67,    68,     0,
       0,    69,     0,     0,     0,     0,    70,     0,     0,     0,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,     0,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      78,    79,    80,    81,     0,    82,    83,    84,     0,     0,
       0,     0,    85,     0,     0,     0,     0,    86,    87,    88,
      89,    90,    91,    92,     0,    93,     0,     0,    94,    95,
      96,     0,    97,     0,     0,    98,     0,     0,    99,   285,
     101,   102,     0,   103,   104,    63,   105,     0,   106,   107,
     108,     0,     0,   109,   110,   111,     0,   112,    64,     0,
       0,     0,    65,    66,    67,    68,     0,     0,    69,     0,
       0,     0,     0,    70,     0,     0,     0,    71,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,     0,     0,    73,    74,    75,    76,    77,     0,   530,
       0,     0,     0,     0,     0,     0,     0,    78,    79,    80,
      81,     0,    82,    83,    84,     0,     0,     0,     0,    85,
       0,     0,     0,     0,    86,    87,    88,    89,    90,    91,
      92,     0,    93,     0,     0,    94,    95,    96,     0,    97,
       0,     0,    98,     0,     0,    99,   100,   101,   102,     0,
     103,   104,    63,   105,     0,   106,   107,   108,     0,     0,
     109,   110,   111,     0,   112,    64,     0,     0,     0,    65,
      66,    67,    68,     0,     0,    69,     0,     0,     0,     0,
      70,     0,     0,     0,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,   696,    82,
      83,    84,     0,     0,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   100,   101,   102,     0,   103,   104,    63,
     105,     0,   106,   107,   108,     0,     0,   109,   110,   111,
       0,   112,    64,     0,     0,     0,    65,    66,    67,    68,
       0,     0,    69,     0,     0,     0,     0,    70,     0,     0,
       0,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,     0,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    78,    79,    80,    81,     0,    82,    83,    84,     0,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,     0,    94,
      95,    96,     0,    97,     0,     0,    98,     0,     0,    99,
     100,   101,   102,     0,   103,   104,    63,   105,     0,   106,
     107,   108,   272,     0,   109,   110,   111,     0,   112,    64,
       0,     0,     0,    65,    66,    67,    68,     0,     0,    69,
       0,     0,     0,     0,    70,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,     0,   248,     0,     0,     0,
       0,    72,     0,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,     0,     0,   248,    78,    79,
      80,    81,     0,    82,    83,    84,     0,     0,   249,     0,
       0,     0,     0,     0,     0,    86,    87,    88,    89,    90,
      91,    92,   248,    93,     0,     0,    94,    95,    96,   249,
      97,     0,     0,    98,     0,     0,    99,   100,   101,   102,
       0,   103,   104,   248,  -396,     0,   106,   107,   108,     0,
       0,   109,   110,   111,   249,   112,     0,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,     0,     0,     0,   249,     0,   489,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,     0,     0,     0,     0,     0,   486,     0,
       0,     0,     0,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   248,     0,
       0,     0,     0,   488,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   248,
       0,     0,     0,     0,   663,     0,     0,     0,     0,     0,
     249,     0,     0,     0,     0,     0,     0,     0,     0,   248,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   249,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   248,     0,     0,     0,     0,     0,     0,     0,
       0,   249,     0,     0,     0,     0,     0,     0,     0,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   249,     0,     0,     0,   356,     0,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,     0,     0,     0,     0,   476,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   248,     0,     0,     0,   482,
       0,     0,     0,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   248,     0,
       0,     0,   490,     0,     0,     0,     0,   249,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     249,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,     0,     0,     0,     0,   661,     0,     0,     0,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,     1,     0,     0,     2,   662,     0,
       3,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       5,     0,     0,     6,     0,     7,     8,     9,     0,     0,
     248,    10,     0,     0,     0,    11,     0,   473,     0,     0,
       0,     0,     0,     0,     0,   248,     0,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   248,
       0,     0,   249,     0,     0,     0,     0,     0,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   249,    14,     0,
       0,    15,     0,    16,     0,    17,     0,     0,    18,     0,
      19,   249,     0,     0,     0,     0,     0,     0,     0,     0,
      20,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,     0,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264
};

static const short yycheck[] =
{
       4,   215,   523,   217,   218,   443,   504,   221,   222,   527,
     602,    17,    27,   299,   177,     5,     5,    21,     8,   182,
      21,   184,   521,     9,    34,   524,    45,     5,     5,     5,
       6,     8,     8,   462,    50,    39,   199,     9,    14,    13,
      14,   327,     5,     6,    79,     8,    13,    14,    13,    14,
       5,    14,     5,     6,    41,    98,    60,    61,    62,    21,
     559,    14,   203,   562,   563,   564,   565,   566,   567,     5,
       6,    18,     8,     5,     6,    78,     8,    15,    14,    50,
      78,   364,    14,     5,     6,   121,   758,    68,   111,   143,
      64,   145,    14,    14,    51,    95,     0,   595,    42,    51,
     143,    59,   124,     5,     6,   115,    44,   143,   143,    30,
     782,   115,   116,   117,   118,   119,   117,   121,   122,   123,
     142,   140,   285,   104,    16,    87,    83,   143,   143,   558,
      74,    83,    96,    95,    19,   138,   728,   101,    42,   422,
     138,   282,   640,    50,   134,   134,    59,   151,   135,   741,
      68,   155,   142,   142,    39,   141,   134,   134,   134,   165,
     127,     0,   166,   167,   142,   141,   758,   171,   606,   141,
      74,   134,   143,   147,    63,   673,   462,    24,   141,   134,
     147,   134,   147,   104,   188,   623,   146,   191,   141,   781,
     782,   135,   136,   137,   198,   142,   143,    88,   134,    90,
     142,   143,   134,   121,   270,   271,    97,   706,   707,   213,
     141,    58,   134,    25,   141,   121,   714,    64,   359,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    95,    47,   522,   143,   141,   243,
     141,   245,   141,   406,   141,   408,   107,   765,   141,    61,
      62,   127,   142,   143,   141,   147,    69,   149,   150,   142,
     143,   141,   783,   549,   141,   551,   270,   271,   554,   142,
     143,   141,   558,   142,   143,   142,   143,   141,   282,   141,
     127,   128,   129,   130,   131,   132,   142,   143,   141,   293,
     141,   138,   141,   105,    64,   299,   300,   142,   143,   303,
     142,   143,   443,   195,    64,   743,   142,   143,   142,   143,
     142,   143,   142,   143,   142,   143,   602,    42,   142,   143,
     212,   127,   214,   327,   142,   143,    98,   111,   220,   127,
     111,   223,   224,   225,   226,    98,   340,    68,    79,   118,
      89,   110,    93,   114,   127,   142,   114,   142,   240,    74,
     141,   141,   244,   357,    42,   359,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,     5,   660,   141,    52,   269,   134,   127,
     521,   127,    65,   524,   104,    16,    74,   104,     8,   393,
      32,   395,    32,    95,   141,     8,     8,   127,   539,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   305,   418,   143,   142,    78,   559,   148,
     143,   562,   563,   564,   565,   566,   567,   431,    59,   433,
     434,   435,   141,     9,     5,   576,   127,    67,    42,   331,
     143,    72,   728,    99,   730,   133,   134,   135,   136,   137,
       5,    82,    99,    84,    85,   741,     8,   127,   462,    80,
     117,   353,   354,   355,   142,   606,   102,   102,   141,   473,
      74,   124,   758,   104,   141,   141,   107,   141,   109,   141,
      96,   141,   623,   141,   127,    53,   141,     5,   119,   127,
     142,   142,    27,   127,   123,   781,   782,   143,     8,   143,
     141,    58,     8,     8,     8,   142,     5,   135,   512,   513,
     724,    58,   122,    88,     8,   142,   142,   142,   522,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   143,    42,   428,   142,   430,   141,
     141,   124,   142,   142,   148,   549,   142,   551,    16,   142,
     554,   124,    78,   142,   558,    42,   141,    80,    27,    56,
      53,    29,   127,    31,     8,   706,   707,    74,     3,    37,
      18,   141,   576,    27,   143,   142,    42,   791,   142,   471,
     143,   143,   143,    69,    52,    80,   590,    74,   142,   142,
     594,    59,   733,   127,   486,   143,   488,   489,   602,     8,
     142,   605,   743,     5,    72,    42,   143,    27,    74,     8,
     143,   143,   141,     5,    82,   142,    84,    85,   622,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   331,   100,   755,   499,   303,   104,    74,   196,   107,
     108,   109,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   119,   577,   244,   658,   730,   660,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   714,   731,   741,   513,   142,   143,   735,   475,
     357,   576,   393,   395,   774,   577,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   694,   522,   443,   623,   142,   143,   605,   594,    18,
     660,   241,   428,   254,    -1,   430,    -1,    -1,    -1,    -1,
      -1,   725,    -1,    -1,   728,    -1,   730,    -1,    -1,    -1,
      -1,   735,    -1,    -1,    -1,    -1,    -1,   741,    -1,     3,
       4,     5,     6,    -1,     8,     9,    10,    -1,    -1,    -1,
      14,   755,    16,    -1,   758,    -1,    20,    21,    22,    23,
      -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,   781,   782,    43,
      -1,    -1,    -1,    47,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    58,    -1,    -1,    -1,    62,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    77,    -1,    -1,    25,    26,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    -1,    47,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,    61,    62,   118,   119,   120,    66,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    -1,    -1,
     134,   135,    -1,    -1,   138,    -1,    -1,   141,    -1,    -1,
     144,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,   105,    -1,    20,    21,
      22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    47,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    57,    58,    -1,    -1,    -1,
      62,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,    -1,    -1,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,    -1,    -1,   138,    -1,    -1,   141,
     142,    -1,   144,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    47,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    57,    58,    -1,
      -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    92,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,    -1,    -1,   118,   119,
     120,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,    -1,   138,    -1,
      -1,   141,    -1,    -1,   144,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    25,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    47,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      58,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,    -1,    -1,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
     138,    -1,    -1,   141,    -1,    -1,   144,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,    25,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    47,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    57,    58,    -1,    -1,    -1,    62,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    -1,    92,    93,    94,    95,
      96,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
      -1,    -1,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
      -1,    -1,   138,    -1,    -1,   141,    -1,    -1,   144,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    47,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    58,    -1,    -1,    -1,    62,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    92,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,    -1,    -1,   118,   119,   120,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,    -1,    -1,    -1,   138,    -1,    -1,   141,   142,    -1,
     144,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    47,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    57,    58,    -1,    -1,    -1,
      62,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,    -1,    -1,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,    -1,    -1,   138,    -1,    -1,   141,
      -1,    -1,   144,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    47,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    57,    58,    -1,
      -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    92,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,    -1,    -1,   118,   119,
     120,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,   138,    -1,
      -1,   141,    -1,    -1,   144,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    25,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    47,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      58,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    -1,    92,    93,    94,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,    -1,    -1,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,    -1,    -1,   134,    -1,    -1,    -1,
     138,    -1,    -1,   141,    16,    -1,   144,    -1,    20,    21,
      22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,    -1,    -1,   118,   119,   120,    -1,
     122,    -1,    -1,     3,    -1,     5,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,   141,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,    42,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    74,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
       3,   111,    -1,   113,   114,   115,    -1,    -1,   118,   119,
     120,    -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,   141,    35,    -1,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,    -1,    -1,   118,   119,   120,    -1,   122,
      -1,    -1,     3,    -1,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,   141,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,   112,   113,   114,   115,    -1,    -1,   118,   119,   120,
      -1,   122,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,    -1,   113,   114,   115,    -1,    -1,   118,   119,   120,
      -1,   122,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    -1,    45,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,    -1,
     111,    -1,   113,   114,   115,     3,     4,   118,   119,   120,
      -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    25,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,    -1,    -1,
     118,   119,   120,     3,   122,     5,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    78,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
       3,   111,    -1,   113,   114,   115,    -1,    10,   118,   119,
     120,    -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    29,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,    -1,    -1,
     113,   114,   115,    -1,    -1,   118,   119,   120,     3,   122,
       5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    -1,    -1,    93,    94,
      95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,     3,   111,     5,   113,   114,
     115,    -1,    -1,   118,   119,   120,    -1,   122,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,     3,   111,    -1,   113,   114,   115,    -1,    -1,
     118,   119,   120,    -1,   122,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,     3,
     111,    -1,   113,   114,   115,    -1,    -1,   118,   119,   120,
      -1,   122,    16,    -1,    18,    -1,    20,    21,    22,    23,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,     3,   113,
     114,   115,    -1,     8,   118,   119,   120,    -1,   122,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    -1,    -1,    93,    94,
      95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,     3,   111,    -1,   113,   114,
     115,    -1,    -1,   118,   119,   120,    -1,   122,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,     3,   111,    -1,   113,   114,   115,    -1,    -1,
     118,   119,   120,    -1,   122,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,     3,
     111,    -1,   113,   114,   115,    -1,    -1,   118,   119,   120,
      -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,     3,   111,    -1,   113,
     114,   115,     9,    -1,   118,   119,   120,    -1,   122,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    42,    90,    -1,    -1,    93,    94,    95,    74,
      97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    42,   111,    -1,   113,   114,   115,    -1,
      -1,   118,   119,   120,    74,   122,    -1,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    74,    -1,   143,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    42,    -1,
      -1,    -1,    -1,   143,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    42,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    74,    -1,    -1,    -1,   142,    -1,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,   142,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,    42,    -1,    -1,    -1,   142,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    42,    -1,
      -1,    -1,   142,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    17,    -1,    -1,    20,   142,    -1,
      23,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      33,    -1,    -1,    36,    -1,    38,    39,    40,    -1,    -1,
      42,    44,    -1,    -1,    -1,    48,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    91,    -1,
      -1,    94,    -1,    96,    -1,    98,    -1,    -1,   101,    -1,
     103,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,    -1,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    23,    28,    33,    36,    38,    39,    40,
      44,    48,    60,    81,    91,    94,    96,    98,   101,   103,
     113,   116,   150,   151,   154,   155,   156,   157,   164,   207,
     212,   213,   218,   219,   220,   221,   232,   233,   242,   243,
     244,   245,   246,   250,   251,   256,   257,   258,   261,   263,
     264,   265,   266,   267,   268,   270,   271,   272,   273,   274,
      95,   107,    59,     3,    16,    20,    21,    22,    23,    26,
      31,    35,    48,    51,    52,    53,    54,    55,    65,    66,
      67,    68,    70,    71,    72,    77,    82,    83,    84,    85,
      86,    87,    88,    90,    93,    94,    95,    97,   100,   103,
     104,   105,   106,   108,   109,   111,   113,   114,   115,   118,
     119,   120,   122,   152,   153,    51,    83,    50,    51,    83,
      21,    87,    95,    59,     3,     4,     5,     6,     8,     9,
      10,    14,    22,    25,    35,    43,    47,    54,    55,    57,
      58,    62,    71,    73,    92,   106,   120,   134,   135,   138,
     141,   144,   153,   165,   166,   168,   200,   201,   269,   275,
     276,   279,   280,    10,    29,    52,    59,    77,   100,   152,
     259,    16,    29,    31,    37,    52,    59,    72,    82,    84,
      85,   100,   104,   107,   108,   109,   119,   209,   210,   259,
     111,    95,   153,   169,     0,    45,   140,   286,    63,   153,
     153,   153,   153,   141,   153,   153,   169,   153,   153,   153,
     153,   153,   141,   146,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,     3,    71,
      73,   200,   200,   200,    58,   153,   204,   205,    13,    14,
     147,   277,   278,    50,   143,    18,   153,   167,    42,    74,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,    69,   192,   193,    64,   127,
     127,    98,     9,   152,   153,     5,     6,     8,    78,   153,
     216,   217,   127,   111,     8,   104,   153,    98,    68,   208,
     208,   208,   208,    79,   194,   195,   153,   118,   153,    98,
     143,   200,   283,   284,   285,    96,   101,   155,   153,   208,
      89,    15,    44,   110,     5,     6,     8,    14,   134,   141,
     177,   178,   228,   234,   235,    93,   114,   121,   171,   114,
       8,   141,   142,   200,   202,   203,   153,   200,   200,   202,
      41,   135,   202,   202,   142,   200,   202,   202,   200,   200,
     200,   200,   142,   141,   141,   141,   142,   143,   145,   127,
       8,   200,   278,   141,   169,   166,   200,   153,   200,   200,
     200,   200,   200,   200,   200,   280,   200,   200,   200,   200,
     200,   200,   200,   200,   200,   200,     5,    78,   138,   200,
     216,   216,   127,   127,    52,   134,     8,    46,    78,   112,
     134,   153,   177,   214,   215,    65,   104,   208,   104,     8,
     153,   196,   197,    14,    30,   104,   211,    68,   121,   252,
     253,   153,   247,   248,   276,   153,   167,    27,   143,   281,
     282,   141,   222,    32,    32,    95,     5,     6,     8,   142,
     177,   179,   236,   143,   237,    25,    47,    61,    62,   105,
     262,     8,     4,    25,    35,    43,    45,    54,    57,    62,
      70,   120,   141,   153,   172,   173,   174,   175,   176,   276,
       8,    96,   157,    49,   142,   143,   142,   142,   153,   142,
     142,   142,   142,   142,   142,   142,   143,   142,   143,   143,
     142,   200,   200,   205,   153,   177,   206,   148,   148,   161,
     170,   171,   143,    78,   141,     5,   214,     9,   217,    67,
     208,   208,   127,   143,    99,     5,    99,     8,   153,   254,
     255,   127,   143,   171,   127,   283,    80,   188,   189,   285,
      57,   153,   223,   224,   117,   153,   153,   153,   142,   143,
     142,   143,   228,   235,   238,   239,   142,   102,   102,   141,
     141,   141,   141,   141,   141,   141,   141,   172,   124,    24,
      58,    64,   127,   128,   129,   130,   131,   132,   138,   142,
     153,   203,   142,   200,   200,   200,   127,    96,   164,    53,
     182,     5,   179,   127,    88,    90,    97,   260,     5,     8,
     141,   153,   197,   127,   123,   141,   177,   178,   248,   194,
     177,   178,    27,   192,   142,   143,   141,   225,   226,    25,
      26,    47,    61,    62,    66,    75,    76,   105,   249,   177,
       8,    18,   240,   143,     8,     8,   153,   276,   135,   276,
     142,   142,   276,     8,   142,   142,   172,   177,   178,     9,
     141,    78,   138,     8,   177,   178,     8,   177,   178,   177,
     178,   177,   178,   177,   178,   177,   178,    58,   143,   158,
      58,   142,   142,   143,   206,   165,   142,     5,   183,   122,
     186,   187,   142,   141,    34,   115,    88,   153,   198,   199,
     141,     8,   255,   142,   179,    86,   176,   190,   191,   223,
     141,   227,   228,    79,   143,   229,    69,   153,   241,   228,
     239,   142,   142,   142,   142,   142,   124,   124,     8,   179,
     180,   181,    78,     9,   141,     5,    57,   153,   159,   160,
     142,   153,   275,   276,   141,    50,    80,   162,    27,    53,
      56,   185,   179,   127,   142,   143,     8,   142,   141,    19,
      39,   143,   142,   143,     3,   230,   231,   226,   177,   178,
     177,   178,   143,   142,   181,   143,   142,   202,    27,    69,
     163,   176,   184,    80,   174,   188,   142,   177,   199,   142,
     142,   191,   228,   127,   143,     8,   142,   160,   142,   190,
       5,   143,    27,   192,     8,   231,   143,   143,   176,   190,
     194,   141,     5,   202,   142,   142
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

  case 83:

    { pParser->PushQuery(); ;}
    break;

  case 84:

    { pParser->PushQuery(); ;}
    break;

  case 85:

    { pParser->PushQuery(); ;}
    break;

  case 89:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 91:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 94:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 95:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 99:

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

  case 100:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 102:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 103:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 104:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 107:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 110:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 111:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 112:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 113:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 114:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 115:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 116:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 117:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 118:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 119:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 120:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 122:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 129:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 131:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 132:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 133:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_sRefString = pParser->m_pBuf;
		;}
    break;

  case 134:

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

  case 135:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 136:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 153:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 154:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 157:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 158:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 159:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 160:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 161:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 167:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 168:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 169:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 170:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 171:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 172:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 173:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
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

  case 181:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 182:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 183:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 185:

    {
			pParser->AddHaving();
		;}
    break;

  case 188:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 191:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 192:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 194:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 196:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 197:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 200:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 201:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 207:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 208:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 209:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 210:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 213:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 214:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 216:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 217:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 222:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 223:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 241:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 242:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
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

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 254:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 255:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 256:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 257:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 258:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 259:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 264:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 265:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 272:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 273:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 275:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 276:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 277:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 278:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 279:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 280:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 281:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 282:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 283:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 284:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 285:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 289:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 290:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 291:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 292:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 293:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 294:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 295:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 296:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 297:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 298:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 299:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 302:

    { yyval.m_iValue = 1; ;}
    break;

  case 303:

    { yyval.m_iValue = 0; ;}
    break;

  case 304:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 312:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 313:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 314:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 317:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 318:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 319:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 324:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 325:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 328:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 329:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 330:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 331:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 332:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 333:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 334:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 335:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 340:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 341:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 342:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 343:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 344:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 346:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 347:

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

  case 348:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 351:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 353:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 358:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 361:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 362:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 363:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 366:

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

  case 367:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 368:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 369:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 370:

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

  case 371:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 372:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 373:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 374:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 375:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 376:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 377:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 378:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 379:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 380:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 381:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 382:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 383:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 384:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 385:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 392:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 393:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 394:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 402:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 403:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 404:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 405:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 406:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 407:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 408:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 409:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 410:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 411:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 412:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 413:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 416:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 417:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 418:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 419:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 420:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 423:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 425:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 426:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 427:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 428:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 429:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 430:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 431:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 435:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 437:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 440:

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

