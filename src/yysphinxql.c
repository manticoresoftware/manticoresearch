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
#define YYLAST   4512

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  149
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  138
/* YYNRULES -- Number of rules. */
#define YYNRULES  438
/* YYNRULES -- Number of states. */
#define YYNSTATES  794

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
    1190,  1198,  1205,  1210,  1215,  1221,  1222,  1224,  1227,  1229,
    1233,  1237,  1240,  1244,  1251,  1252,  1254,  1256,  1259,  1262,
    1265,  1267,  1275,  1277,  1279,  1281,  1283,  1287,  1294,  1298,
    1302,  1305,  1309,  1311,  1315,  1318,  1322,  1326,  1334,  1340,
    1342,  1344,  1347,  1349,  1352,  1354,  1356,  1360,  1364,  1368,
    1372,  1374,  1375,  1378,  1380,  1383,  1385,  1387,  1391
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
      17,   107,   153,    15,    32,   153,   249,    -1,    17,   107,
     153,    44,    32,   153,    -1,    17,    95,   153,    89,    -1,
     101,   259,   118,   252,    -1,   101,   259,   118,    68,     8,
      -1,    -1,   253,    -1,   121,   254,    -1,   255,    -1,   254,
     123,   255,    -1,   153,   127,     8,    -1,   101,    31,    -1,
     101,    29,    98,    -1,    98,   259,   111,    65,    67,   260,
      -1,    -1,    52,    -1,   100,    -1,    88,   115,    -1,    88,
      34,    -1,    90,    88,    -1,    97,    -1,    36,    51,   153,
      93,   262,   102,     8,    -1,    61,    -1,    25,    -1,    47,
      -1,   105,    -1,    44,    51,   153,    -1,    20,    59,   153,
     110,    95,   153,    -1,    48,    95,   153,    -1,    48,    87,
     153,    -1,    48,    21,    -1,    96,   269,   192,    -1,    10,
      -1,    10,   146,   153,    -1,    96,   200,    -1,   113,    95,
     153,    -1,    81,    59,   153,    -1,    36,    83,   153,   114,
       8,   102,     8,    -1,    44,    83,   153,   114,     8,    -1,
     276,    -1,   153,    -1,   153,   277,    -1,   278,    -1,   277,
     278,    -1,    13,    -1,    14,    -1,   147,   200,   148,    -1,
     147,     8,   148,    -1,   200,   127,   280,    -1,   280,   127,
     200,    -1,     8,    -1,    -1,   282,   285,    -1,    27,    -1,
     284,   167,    -1,   200,    -1,   283,    -1,   285,   143,   283,
      -1,    45,   285,   281,   188,   192,    -1
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
    1275,  1283,  1290,  1301,  1305,  1312,  1313,  1317,  1321,  1322,
    1326,  1330,  1337,  1344,  1350,  1351,  1352,  1356,  1357,  1358,
    1359,  1365,  1376,  1377,  1378,  1379,  1384,  1395,  1407,  1416,
    1425,  1435,  1443,  1444,  1448,  1458,  1469,  1480,  1491,  1502,
    1503,  1507,  1510,  1511,  1515,  1516,  1517,  1518,  1522,  1523,
    1527,  1532,  1534,  1538,  1547,  1551,  1559,  1560,  1564
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
     250,   250,   250,   251,   251,   252,   252,   253,   254,   254,
     255,   256,   257,   258,   259,   259,   259,   260,   260,   260,
     260,   261,   262,   262,   262,   262,   263,   264,   265,   266,
     267,   268,   269,   269,   270,   271,   272,   273,   274,   275,
     275,   276,   277,   277,   278,   278,   278,   278,   279,   279,
     280,   281,   281,   282,   283,   284,   285,   285,   286
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
       7,     6,     4,     4,     5,     0,     1,     2,     1,     3,
       3,     2,     3,     6,     0,     1,     1,     2,     2,     2,
       1,     7,     1,     1,     1,     1,     3,     6,     3,     3,
       2,     3,     1,     3,     2,     3,     3,     7,     5,     1,
       1,     2,     1,     2,     1,     1,     3,     3,     3,     3,
       1,     0,     2,     1,     2,     1,     1,     3,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   315,     0,   312,     0,     0,   360,   359,
       0,     0,   318,     0,   319,   313,     0,   394,   394,     0,
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
     410,     0,     0,     0,    32,   216,   218,   219,   430,   221,
     412,   220,    36,     0,    40,     0,     0,    45,    46,   217,
       0,     0,    51,     0,     0,    69,    77,     0,   107,     0,
       0,     0,   215,     0,   105,   109,   112,   243,   198,     0,
     244,   245,     0,     0,     0,    43,     0,     0,    65,     0,
       0,     0,     0,   391,   271,   395,   286,   271,   278,   279,
     277,   396,   271,   287,   271,   202,   273,   270,     0,     0,
     316,     0,   121,     0,     1,     0,     4,    85,     0,   271,
       0,     0,     0,     0,     0,     0,     0,   406,     0,   409,
     408,   416,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    32,    51,
       0,   222,   223,     0,   267,   266,     0,     0,   424,   425,
       0,   421,   422,     0,     0,     0,   110,   108,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   411,   199,     0,     0,
       0,     0,     0,     0,     0,   310,   311,   309,   308,   307,
     293,   305,     0,     0,     0,   271,     0,   392,     0,   362,
     275,   274,   361,     0,   280,   203,   288,   385,   415,     0,
       0,   435,   436,   109,   431,     0,     0,    84,   320,   358,
     382,     0,     0,     0,   167,   169,   333,   171,     0,     0,
     331,   332,   345,   349,   343,     0,     0,     0,   341,     0,
     263,     0,   254,   262,     0,   260,   413,     0,   262,     0,
       0,     0,     0,     0,   119,     0,     0,     0,     0,     0,
       0,     0,   257,     0,     0,     0,   241,     0,   242,     0,
     430,     0,   423,    99,   123,   106,   112,   111,   233,   234,
     240,   239,   231,   230,   237,   428,   238,   228,   229,   236,
     235,   224,   225,   226,   227,   232,   200,   246,     0,   429,
     294,   295,     0,     0,     0,     0,   301,   303,   292,   302,
       0,   300,   304,   291,   290,     0,   271,   276,   271,   272,
       0,   204,   205,     0,     0,   283,     0,     0,     0,   383,
     386,     0,     0,   364,     0,   122,   434,   433,     0,   189,
       0,     0,     0,     0,     0,     0,   168,   170,   347,   335,
     172,     0,     0,     0,     0,   403,   404,   402,   405,     0,
       0,   157,     0,    40,     0,     0,    45,   161,     0,    50,
      77,     0,   156,   125,   126,   130,   143,     0,   162,   418,
       0,     0,     0,   248,     0,   113,   251,     0,   118,   253,
     252,   117,   249,   250,   114,     0,   115,     0,     0,   116,
       0,     0,     0,   269,   268,   264,   427,   426,     0,   178,
     124,     0,   247,     0,   298,   297,     0,   306,     0,   281,
     282,     0,     0,   285,   289,   284,   384,     0,   387,   388,
       0,     0,   202,     0,   437,     0,   198,   190,   432,   323,
     322,   324,     0,     0,     0,   381,   407,   334,     0,   346,
       0,   354,   344,   350,   351,   342,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    92,     0,
     261,   120,     0,     0,     0,     0,     0,     0,   180,   186,
     201,     0,     0,     0,     0,   400,   393,   208,   211,     0,
     207,   206,     0,     0,     0,   366,   367,   365,   363,   370,
     371,     0,   438,   321,     0,     0,   336,   326,   373,   375,
     374,   372,   378,   376,   377,   379,   380,   173,   348,   355,
       0,     0,   401,   417,     0,     0,     0,     0,   166,   159,
       0,     0,   160,   128,   127,     0,     0,   135,     0,   153,
       0,   151,   131,   142,   152,   132,   155,   139,   148,   138,
     147,   140,   149,   141,   150,     0,     0,     0,     0,   256,
     255,     0,   265,     0,     0,   181,     0,     0,   184,   187,
     296,     0,   398,   397,   399,     0,     0,   212,     0,   390,
     389,   369,     0,    58,   195,   191,   193,   325,     0,     0,
     329,     0,     0,   317,   357,   356,   353,   354,   352,   165,
     158,   164,   163,   129,     0,     0,   174,   176,   177,     0,
     154,   136,     0,    97,    98,    96,    93,    94,    89,   420,
       0,   419,     0,     0,     0,   101,     0,     0,     0,   189,
       0,     0,   209,     0,     0,   368,     0,   196,   197,     0,
     328,     0,     0,   337,   338,   327,   137,   145,   146,   144,
       0,   133,     0,     0,   258,     0,     0,     0,    91,   182,
     179,     0,   185,   198,   299,   214,   213,   210,   192,   194,
     330,     0,     0,   175,   134,    95,     0,   100,   102,     0,
       0,   202,   340,   339,     0,     0,   183,   188,   104,     0,
     103,     0,     0,   259
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   113,   152,    24,    25,    26,    27,   657,
     716,   717,   498,   725,   758,    28,   153,   154,   247,   155,
     364,   499,   328,   463,   464,   465,   466,   467,   440,   321,
     441,   708,   709,   579,   666,   760,   729,   668,   669,   526,
     527,   685,   686,   266,   267,   294,   295,   411,   412,   676,
     677,   338,   157,   334,   335,   236,   237,   495,    29,   289,
     187,   188,   416,    30,    31,   403,   404,   280,   281,    32,
      33,    34,    35,   432,   531,   532,   606,   607,   689,   322,
     693,   743,   744,    36,    37,   323,   324,   442,   444,   543,
     544,   620,   696,    38,    39,    40,    41,    42,   422,   423,
     616,    43,    44,   419,   420,   518,   519,    45,    46,    47,
     170,   586,    48,   449,    49,    50,    51,    52,    53,    54,
     158,    55,    56,    57,    58,    59,   159,   160,   241,   242,
     161,   162,   429,   430,   302,   303,   304,   197
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -650
static const short yypact[] =
{
    4345,    -6,    26,  -650,  3794,  -650,    27,    44,  -650,  -650,
      40,    53,  -650,    43,  -650,  -650,   734,  2925,   553,    24,
       3,  3794,   104,  -650,   -13,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,    70,  -650,  -650,  -650,  3794,
    -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    3794,  3794,  3794,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,    -2,  3794,  3794,  3794,  3794,  3794,
    -650,  3794,  3794,  3794,    36,  -650,  -650,  -650,  -650,  -650,
       0,  -650,    54,    61,    73,   115,   120,   136,   147,  -650,
     161,   165,   176,   178,   183,   186,   194,  1586,  -650,  1586,
    1586,  3257,    13,    -9,  -650,  3364,    96,  -650,   122,   146,
     217,  -650,   110,   214,   223,  3901,  3794,  2818,   243,   263,
     282,  3473,   246,  -650,   332,  -650,  -650,   332,  -650,  -650,
    -650,  -650,   332,  -650,   332,   322,  -650,  -650,  3794,   286,
    -650,  3794,  -650,   -22,  -650,  1586,   150,  -650,  3794,   332,
     318,    85,   298,    23,   316,   296,   -37,  -650,   297,  -650,
    -650,  -650,   876,  3794,  1586,  1728,    -3,  1728,  1728,   270,
    1586,  1728,  1728,  1586,  1586,  1586,  1586,   274,   278,   279,
     281,  -650,  -650,  4065,  -650,  -650,   -50,   299,  -650,  -650,
    1870,    41,  -650,  2225,  1018,  3794,  -650,  -650,  1586,  1586,
    1586,  1586,  1586,  1586,  1586,  1586,  1586,  1586,  1586,  1586,
    1586,  1586,  1586,  1586,  1586,   418,  -650,  -650,   -29,  1586,
    2818,  2818,   301,   305,   372,  -650,  -650,  -650,  -650,  -650,
     291,  -650,  2347,   368,   330,    22,   331,  -650,   428,  -650,
    -650,  -650,  -650,  3794,  -650,  -650,    78,    35,  -650,  3794,
    3794,  4361,  -650,  3364,   -15,  1160,   333,  -650,   302,  -650,
    -650,   406,   419,   355,  -650,  -650,  -650,  -650,   212,     8,
    -650,  -650,  -650,   310,  -650,   237,   446,  1996,  -650,   447,
     329,  1302,  -650,  4346,    93,  -650,  -650,  4085,  4361,   111,
    3794,   317,   137,   140,  -650,  4108,   144,   148,   562,  3902,
    3923,  4181,  -650,  1444,  1586,  1586,  -650,  3257,  -650,  2467,
     312,   431,  -650,  -650,   -37,  -650,  4361,  -650,  -650,  -650,
    4375,   581,  2135,   251,   139,  -650,   139,    63,    63,    63,
      63,   112,   112,  -650,  -650,  -650,   321,  -650,   387,   139,
     291,   291,   325,  3043,   458,  2818,  -650,  -650,  -650,  -650,
     464,  -650,  -650,  -650,  -650,   404,   332,  -650,   332,  -650,
     347,   334,  -650,   376,   471,  -650,   379,   472,  3794,  -650,
    -650,    56,    50,  -650,   352,  -650,  -650,  -650,  1586,   401,
    1586,  3580,   365,  3794,  3794,  3794,  -650,  -650,  -650,  -650,
    -650,   173,   187,    23,   342,  -650,  -650,  -650,  -650,   383,
     384,  -650,   346,   348,   349,   350,   351,  -650,   353,   354,
     356,  1996,    41,   369,  -650,  -650,  -650,   180,  -650,  -650,
    1018,   357,  3794,  -650,  1728,  -650,  -650,   359,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  1586,  -650,  1586,  1586,  -650,
    3948,  3969,   371,  -650,  -650,  -650,  -650,  -650,   400,   449,
    -650,   498,  -650,    42,  -650,  -650,   385,  -650,   162,  -650,
    -650,  2118,  3794,  -650,  -650,  -650,  -650,   386,   388,  -650,
      39,  3794,   322,    45,  -650,   479,   122,  -650,   367,  -650,
    -650,  -650,   195,   373,   238,  -650,  -650,  -650,    42,  -650,
     507,    69,  -650,   375,  -650,  -650,   508,   511,  3794,   390,
    3794,   378,   380,  3794,   513,   389,    92,  1996,    45,    14,
       5,    67,    74,    45,    45,    45,    45,   468,   391,   469,
    -650,  -650,  4204,  4224,  4044,  2467,  1018,   393,   523,   407,
    -650,   197,   392,   -16,   442,  -650,  -650,  -650,  -650,  3794,
     396,  -650,   524,  3794,    10,  -650,  -650,  -650,  -650,  -650,
    -650,  2587,  -650,  -650,  3580,    34,   -18,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    3687,    34,  -650,  -650,    41,   397,   398,   399,  -650,  -650,
     403,   405,  -650,  -650,  -650,   424,   426,  -650,    25,  -650,
     465,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,    37,  3150,   409,  3794,  -650,
    -650,   411,  -650,     2,   462,  -650,   543,   519,   517,  -650,
    -650,    42,  -650,  -650,  -650,   448,   204,  -650,   566,  -650,
    -650,  -650,   208,   436,   153,   437,  -650,  -650,    19,   231,
    -650,   578,   373,  -650,  -650,  -650,  -650,   565,  -650,  -650,
    -650,  -650,  -650,  -650,    45,    45,  -650,   443,   451,   450,
    -650,  -650,    25,  -650,  -650,  -650,   455,  -650,  -650,    41,
     457,  -650,  1728,  3794,   564,   532,  2700,   522,  2700,   401,
     233,    42,  -650,  3794,   461,  -650,   466,  -650,  -650,  2700,
    -650,    34,   480,   463,  -650,  -650,  -650,  -650,  -650,  -650,
     601,  -650,   473,  3150,  -650,   252,  2700,   605,  -650,  -650,
     470,   584,  -650,   122,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,   609,   578,  -650,  -650,  -650,   475,   437,   476,  2700,
    2700,   322,  -650,  -650,   481,   615,  -650,   437,  -650,  1728,
    -650,   254,   484,  -650
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -650,  -650,  -650,    -7,    -4,  -650,   425,  -650,   300,  -650,
    -650,  -126,  -650,  -650,  -650,   130,    57,   395,   326,  -650,
      -1,  -650,  -302,  -427,  -650,   -98,  -650,  -590,  -117,  -498,
    -497,  -650,   -80,  -650,  -650,  -650,  -650,  -650,  -650,   -95,
    -650,  -649,   -99,  -523,  -650,  -521,  -650,  -650,   129,  -650,
     -91,   108,  -650,  -213,   169,  -650,   287,    71,  -650,  -163,
    -650,  -650,  -650,  -650,  -650,   255,  -650,   128,   250,  -650,
    -650,  -650,  -650,  -650,    46,  -650,  -650,   -45,  -650,  -436,
    -650,  -650,  -123,  -650,  -650,  -650,   213,  -650,  -650,  -650,
      30,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,   138,
    -650,  -650,  -650,  -650,  -650,  -650,    65,  -650,  -650,  -650,
     645,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,  -650,
    -650,  -650,  -650,  -650,  -650,  -650,     6,  -256,  -650,   427,
    -650,   412,  -650,  -650,   239,  -650,   235,  -650
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -431
static const short yytable[] =
{
     114,   598,   339,   602,   342,   343,   581,   541,   346,   347,
     169,   684,   427,   314,   290,   314,   438,   192,   672,   291,
     193,   292,   596,   637,   314,   600,   238,   239,   314,   315,
     314,   316,   195,   706,   556,   199,   309,   317,   340,   314,
     315,   243,   316,   424,   314,   315,   711,   314,   317,   387,
     314,   315,   723,   317,   238,   239,   200,   201,   202,   317,
     636,   691,   500,   643,   646,   648,   650,   652,   654,   238,
     239,   468,   314,   315,   120,   641,   299,  -420,   115,   314,
     315,   317,   644,   639,   327,    62,   320,   619,   317,    60,
     288,   118,   413,   357,   117,   358,  -414,   682,   191,   673,
     311,    61,   123,   417,   194,   248,   300,   777,   414,   388,
     116,   204,   205,   192,   207,   208,   206,   209,   210,   211,
     522,   300,   407,   119,   156,   692,   -67,   196,   428,   312,
     634,   787,   341,   198,   244,   190,   759,   249,   248,   203,
     121,   707,   400,   640,   400,   244,   213,   235,   122,   684,
     439,   246,   681,   400,   248,   638,   418,   318,   273,   400,
     240,   439,   274,   279,   319,   402,   684,   286,   318,   690,
     249,   327,   737,   318,   730,   688,   400,   212,   712,   318,
     594,   248,   415,   520,   296,   697,   249,   298,   240,   786,
     684,   265,   738,   521,   308,   214,   260,   261,   262,   263,
     264,   318,   215,   240,   558,   468,   747,   749,   318,   336,
     268,  -345,  -345,   249,   216,   707,   557,   436,   437,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   633,   473,   474,   269,   559,   192,
     781,   367,   494,   509,   560,   510,   305,   262,   263,   264,
     583,   306,   584,   476,   474,   231,   217,   232,   233,   585,
     788,   218,   445,   608,   609,   424,   279,   279,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   219,   401,   479,
     474,  -419,   480,   474,   446,   610,   482,   474,   220,   410,
     483,   474,   625,   248,   627,   421,   425,   630,   447,   246,
     611,   468,   221,   301,   612,   770,   222,   561,   562,   563,
     564,   565,   566,   613,   614,   537,   538,   223,   567,   224,
     333,   271,   337,   462,   225,   249,   320,   226,   345,   539,
     540,   348,   349,   350,   351,   227,   477,   603,   604,   670,
     538,   270,   448,   615,   287,   468,   732,   733,   361,   171,
     735,   538,   366,   235,  -396,   493,   368,   369,   370,   371,
     372,   373,   374,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   740,   741,   764,   538,   389,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   401,
     282,   279,   176,   283,   776,   474,   792,   474,   390,   391,
     288,   293,   721,   595,   297,   177,   599,   310,   313,   325,
     326,   329,   344,   366,   517,   178,   352,   179,   180,   353,
     354,   617,   355,   386,   394,   395,   359,   530,   392,   534,
     535,   536,   393,   405,   406,   408,   409,   182,   433,   233,
     183,   635,   185,   431,   642,   645,   647,   649,   651,   653,
     435,   434,   186,   443,   450,   469,  -430,   462,   494,   478,
     496,   333,   490,   491,   501,   502,   503,   506,   569,   436,
     468,   508,   468,   248,   511,   513,   514,   512,   515,   523,
     516,   525,   533,   468,   545,   546,   547,   548,   320,   549,
     550,   551,   552,   557,   553,   554,   576,   555,   575,   568,
     468,   571,   578,   580,   320,   249,   601,   590,   410,   755,
     428,   593,   582,   592,   605,   618,   622,   421,   621,   623,
     628,   631,   629,   468,   468,   626,   655,   658,   665,   667,
     674,   632,   679,   671,   656,   664,   301,   678,   301,   699,
     700,   701,   724,   710,   624,   702,   624,   703,   704,   624,
     705,   718,   722,   462,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   171,
     726,   493,   727,   728,   734,   731,   791,   736,   366,   497,
     739,   742,   172,   619,   173,   675,   538,   746,   748,   517,
     174,   756,   751,   572,   750,   573,   574,   462,   753,   754,
     530,   757,   761,   767,   248,   175,   772,   771,   768,   773,
     778,   780,   176,   779,   765,   774,   695,   782,   784,   785,
     790,   307,   789,   248,   320,   177,   793,   775,   577,   426,
     762,   471,   752,   663,   763,   178,   249,   179,   180,   365,
     769,   591,   766,   570,   492,   507,   662,   745,   505,   783,
     687,   698,   715,   181,   719,   249,   542,   182,   680,   597,
     183,   184,   185,   189,   720,   528,   375,   524,   362,     0,
       0,     0,   186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   366,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
       0,     0,     0,     0,   484,   485,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   192,
       0,     0,   462,     0,   462,     0,     0,     0,     0,   675,
       0,     0,     0,     0,     0,   462,     0,   124,   125,   126,
     127,     0,   128,   129,   130,     0,     0,     0,   131,   715,
      64,     0,   462,     0,    65,    66,   132,    68,     0,   133,
      69,     0,     0,     0,     0,    70,     0,     0,     0,   134,
       0,     0,     0,     0,     0,   462,   462,   135,     0,     0,
       0,   136,    72,     0,     0,    73,    74,    75,   137,   138,
       0,   139,   140,     0,     0,     0,   141,     0,     0,    78,
      79,    80,    81,     0,    82,   142,    84,   143,     0,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,     0,   144,    94,    95,    96,
       0,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     145,     0,   103,   104,     0,   105,     0,   106,   107,   108,
       0,     0,   109,   110,   146,     0,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   147,   148,
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
     147,     0,     0,     0,   149,     0,     0,   331,   332,     0,
     151,   228,   125,   126,   127,     0,   128,   129,     0,     0,
       0,     0,   131,     0,    64,     0,     0,     0,    65,    66,
     132,    68,     0,   133,    69,     0,     0,     0,     0,    70,
       0,     0,     0,   134,     0,     0,     0,     0,     0,     0,
       0,   135,     0,     0,     0,   136,    72,     0,     0,    73,
      74,    75,   137,   138,     0,   139,   140,     0,     0,     0,
     141,     0,     0,    78,    79,    80,    81,     0,    82,   142,
      84,   143,     0,     0,     0,    85,     0,     0,     0,     0,
      86,    87,    88,    89,    90,    91,    92,     0,    93,     0,
     144,    94,    95,    96,     0,    97,     0,     0,    98,     0,
       0,    99,   100,   101,   145,     0,   103,   104,     0,   105,
       0,   106,   107,   108,     0,     0,   109,   110,   146,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   147,   148,     0,     0,   149,     0,     0,   150,
       0,     0,   151,   124,   125,   126,   127,     0,   128,   129,
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
       0,   150,     0,     0,   151,   228,   125,   126,   127,     0,
     128,   129,     0,     0,     0,     0,   131,     0,    64,     0,
       0,     0,    65,    66,    67,    68,     0,   133,    69,     0,
       0,     0,     0,    70,     0,     0,     0,    71,     0,     0,
       0,     0,     0,     0,     0,   135,     0,     0,     0,   136,
      72,     0,     0,    73,    74,    75,    76,    77,     0,   139,
     140,     0,     0,     0,   141,     0,     0,    78,    79,    80,
      81,     0,    82,   229,    84,   230,     0,     0,     0,    85,
       0,     0,     0,     0,    86,    87,    88,    89,    90,    91,
      92,     0,    93,     0,   144,    94,    95,    96,   470,    97,
       0,     0,    98,     0,     0,    99,   100,   101,   102,     0,
     103,   104,     0,   105,     0,   106,   107,   108,     0,     0,
     109,   110,   146,     0,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   147,     0,     0,     0,
     149,     0,     0,   150,     0,     0,   151,   228,   125,   126,
     127,     0,   330,   129,     0,     0,     0,     0,   131,     0,
      64,     0,     0,     0,    65,    66,    67,    68,     0,   133,
      69,     0,     0,     0,     0,    70,     0,     0,     0,    71,
       0,     0,     0,     0,     0,     0,     0,   135,     0,     0,
       0,   136,    72,     0,     0,    73,    74,    75,    76,    77,
       0,   139,   140,     0,     0,     0,   141,     0,     0,    78,
      79,    80,    81,     0,    82,   229,    84,   230,     0,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,     0,   144,    94,    95,    96,
       0,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     102,     0,   103,   104,     0,   105,     0,   106,   107,   108,
       0,     0,   109,   110,   146,     0,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   147,     0,
       0,     0,   149,     0,     0,   150,   332,     0,   151,   228,
     125,   126,   127,     0,   128,   129,     0,     0,     0,     0,
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
     147,     0,     0,     0,   149,     0,     0,   150,     0,     0,
     151,   228,   125,   126,   127,     0,   330,   129,     0,     0,
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
       0,     0,   151,   228,   125,   126,   127,     0,   360,   129,
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
     146,     0,   112,     0,     0,     0,     0,     0,     0,    63,
     451,     0,     0,     0,   147,     0,     0,     0,   149,     0,
       0,   150,    64,     0,   151,     0,    65,    66,    67,    68,
       0,   452,    69,     0,     0,     0,     0,    70,     0,     0,
       0,   453,     0,     0,     0,     0,     0,     0,     0,   454,
       0,   455,     0,     0,    72,     0,     0,    73,    74,    75,
     456,    77,     0,   457,     0,     0,     0,     0,   458,     0,
       0,    78,    79,    80,    81,     0,   459,    83,    84,     0,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,     0,    94,
      95,    96,     0,    97,     0,     0,    98,     0,     0,    99,
     100,   101,   102,     0,   103,   104,     0,   105,     0,   106,
     107,   108,     0,     0,   109,   110,   460,     0,   112,     0,
       0,    63,     0,   587,     0,     0,   588,     0,     0,     0,
       0,     0,     0,     0,    64,     0,     0,   461,    65,    66,
      67,    68,     0,     0,    69,     0,     0,     0,     0,    70,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,     0,     0,    73,
      74,    75,    76,    77,     0,     0,     0,   248,     0,     0,
       0,     0,     0,    78,    79,    80,    81,     0,    82,    83,
      84,     0,     0,     0,     0,    85,     0,     0,     0,     0,
      86,    87,    88,    89,    90,    91,    92,     0,    93,   249,
       0,    94,    95,    96,     0,    97,     0,     0,    98,     0,
       0,    99,   100,   101,   102,     0,   103,   104,    63,   105,
       0,   106,   107,   108,     0,     0,   109,   110,   111,     0,
     112,    64,     0,     0,     0,    65,    66,    67,    68,     0,
       0,    69,     0,     0,     0,     0,    70,     0,     0,   589,
      71,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    72,     0,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      78,    79,    80,    81,     0,    82,    83,    84,     0,     0,
       0,     0,    85,     0,     0,     0,     0,    86,    87,    88,
      89,    90,    91,    92,     0,    93,     0,     0,    94,    95,
      96,     0,    97,     0,     0,    98,     0,     0,    99,   100,
     101,   102,     0,   103,   104,     0,   105,     0,   106,   107,
     108,     0,     0,   109,   110,   111,     0,   112,     0,     0,
      63,     0,   314,     0,     0,   396,     0,     0,     0,     0,
       0,     0,     0,    64,     0,     0,   363,    65,    66,    67,
      68,     0,     0,    69,     0,     0,     0,     0,    70,     0,
       0,     0,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   397,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,     0,    82,    83,    84,
       0,     0,     0,     0,    85,   398,     0,     0,     0,    86,
      87,    88,    89,    90,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,    98,     0,     0,
      99,   100,   101,   102,     0,   103,   104,     0,   105,   399,
     106,   107,   108,     0,     0,   109,   110,   111,     0,   112,
      63,     0,   314,     0,     0,     0,     0,     0,     0,     0,
       0,   400,     0,    64,     0,     0,     0,    65,    66,    67,
      68,     0,     0,    69,     0,     0,     0,     0,    70,     0,
       0,     0,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,     0,    82,    83,    84,
       0,     0,     0,     0,    85,     0,     0,     0,     0,    86,
      87,    88,    89,    90,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,    98,     0,     0,
      99,   100,   101,   102,     0,   103,   104,     0,   105,     0,
     106,   107,   108,     0,     0,   109,   110,   111,     0,   112,
      63,   451,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   400,     0,    64,     0,     0,     0,    65,    66,    67,
      68,     0,   452,    69,     0,     0,     0,     0,    70,     0,
       0,     0,   453,     0,     0,     0,     0,     0,     0,     0,
     454,     0,   455,     0,     0,    72,     0,     0,    73,    74,
      75,   456,    77,     0,   457,     0,     0,     0,     0,   458,
       0,     0,    78,    79,    80,    81,     0,    82,    83,    84,
       0,     0,     0,     0,    85,     0,     0,     0,     0,    86,
      87,    88,    89,   683,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,    98,     0,     0,
      99,   100,   101,   102,     0,   103,   104,     0,   105,     0,
     106,   107,   108,    63,   451,   109,   110,   460,     0,   112,
       0,     0,     0,     0,     0,     0,    64,     0,     0,     0,
      65,    66,    67,    68,     0,   452,    69,     0,     0,     0,
       0,    70,     0,     0,     0,   453,     0,     0,     0,     0,
       0,     0,     0,   454,     0,   455,     0,     0,    72,     0,
       0,    73,    74,    75,   456,    77,     0,   457,     0,     0,
       0,     0,   458,     0,     0,    78,    79,    80,    81,     0,
      82,    83,    84,     0,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,     0,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
       0,   105,     0,   106,   107,   108,     0,     0,   109,   110,
     460,    63,   112,   275,   276,     0,   277,     0,     0,     0,
       0,     0,     0,     0,    64,     0,     0,     0,    65,    66,
      67,    68,     0,     0,    69,     0,     0,     0,     0,    70,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,     0,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    78,    79,    80,    81,     0,    82,    83,
      84,     0,     0,     0,     0,    85,   278,     0,     0,     0,
      86,    87,    88,    89,    90,    91,    92,     0,    93,     0,
       0,    94,    95,    96,     0,    97,     0,     0,    98,     0,
       0,    99,   100,   101,   102,     0,   103,   104,    63,   105,
       0,   106,   107,   108,     0,   163,   109,   110,   111,     0,
     112,    64,     0,     0,     0,    65,    66,    67,    68,     0,
       0,    69,     0,     0,   164,     0,    70,     0,     0,     0,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,     0,     0,    73,   165,    75,    76,
      77,     0,     0,     0,   166,     0,     0,     0,     0,     0,
      78,    79,    80,    81,     0,    82,    83,    84,     0,     0,
       0,     0,   167,     0,     0,     0,     0,    86,    87,    88,
      89,    90,    91,    92,     0,    93,     0,     0,    94,    95,
      96,     0,    97,     0,     0,   168,     0,     0,    99,   100,
     101,   102,     0,   103,   104,     0,     0,     0,   106,   107,
     108,     0,     0,   109,   110,   111,    63,   112,   504,     0,
       0,   396,     0,     0,     0,     0,     0,     0,     0,    64,
       0,     0,     0,    65,    66,    67,    68,     0,     0,    69,
       0,     0,     0,     0,    70,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,     0,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,     0,    82,    83,    84,     0,     0,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,    90,
      91,    92,     0,    93,     0,     0,    94,    95,    96,     0,
      97,     0,     0,    98,     0,     0,    99,   100,   101,   102,
       0,   103,   104,    63,   105,   713,   106,   107,   108,     0,
       0,   109,   110,   111,     0,   112,    64,     0,     0,     0,
      65,    66,    67,    68,     0,     0,    69,     0,     0,     0,
       0,    70,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,   714,     0,     0,
       0,     0,     0,     0,     0,    78,    79,    80,    81,     0,
      82,    83,    84,     0,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,     0,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
      63,   105,     0,   106,   107,   108,     0,     0,   109,   110,
     111,     0,   112,    64,     0,     0,     0,    65,    66,    67,
      68,     0,     0,    69,     0,     0,     0,     0,    70,     0,
       0,     0,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,   234,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,     0,    82,    83,    84,
       0,     0,     0,     0,    85,     0,     0,     0,     0,    86,
      87,    88,    89,    90,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,    98,     0,     0,
      99,   100,   101,   102,     0,   103,   104,    63,   105,     0,
     106,   107,   108,     0,     0,   109,   110,   111,     0,   112,
      64,     0,   245,     0,    65,    66,    67,    68,     0,     0,
      69,     0,     0,     0,     0,    70,     0,     0,     0,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,     0,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    78,
      79,    80,    81,     0,    82,    83,    84,     0,     0,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,     0,     0,    94,    95,    96,
       0,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     102,     0,   103,   104,     0,   105,    63,   106,   107,   108,
       0,   284,   109,   110,   111,     0,   112,     0,     0,    64,
       0,     0,     0,    65,    66,    67,    68,     0,     0,    69,
       0,     0,     0,     0,    70,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,     0,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,     0,    82,    83,    84,     0,     0,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,    90,
      91,    92,     0,    93,     0,     0,    94,    95,    96,     0,
      97,     0,     0,    98,     0,     0,    99,   285,   101,   102,
       0,   103,   104,    63,   105,     0,   106,   107,   108,     0,
       0,   109,   110,   111,     0,   112,    64,     0,     0,     0,
      65,    66,    67,    68,     0,     0,    69,     0,     0,     0,
       0,    70,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,   529,     0,     0,
       0,     0,     0,     0,     0,    78,    79,    80,    81,     0,
      82,    83,    84,     0,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,     0,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
      63,   105,     0,   106,   107,   108,     0,     0,   109,   110,
     111,     0,   112,    64,     0,     0,     0,    65,    66,    67,
      68,     0,     0,    69,     0,     0,     0,     0,    70,     0,
       0,     0,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,   694,    82,    83,    84,
       0,     0,     0,     0,    85,     0,     0,     0,     0,    86,
      87,    88,    89,    90,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,    98,     0,     0,
      99,   100,   101,   102,     0,   103,   104,    63,   105,     0,
     106,   107,   108,     0,     0,   109,   110,   111,     0,   112,
      64,     0,     0,     0,    65,    66,    67,    68,     0,     0,
      69,     0,     0,     0,     0,    70,     0,     0,     0,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,     0,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    78,
      79,    80,    81,     0,    82,    83,    84,     0,     0,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,     0,     0,    94,    95,    96,
       0,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     102,     0,   103,   104,    63,   105,     0,   106,   107,   108,
     272,     0,   109,   110,   111,     0,   112,    64,     0,     0,
       0,    65,    66,    67,    68,     0,     0,    69,     0,     0,
       0,     0,    70,     0,     0,     0,    71,     0,     0,     0,
       0,     0,     0,     0,   248,     0,     0,     0,     0,    72,
       0,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,     0,     0,   248,    78,    79,    80,    81,
       0,    82,    83,    84,     0,     0,   249,     0,     0,     0,
       0,     0,     0,    86,    87,    88,    89,    90,    91,    92,
     248,    93,     0,     0,    94,    95,    96,   249,    97,     0,
       0,    98,     0,     0,    99,   100,   101,   102,     0,   103,
     104,   248,  -395,     0,   106,   107,   108,     0,     0,   109,
     110,   111,   249,   112,     0,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
       0,     0,     0,   249,   486,   487,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,     0,     0,     0,     0,     0,   488,     0,     0,     0,
       0,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   248,     0,     0,     0,
       0,   485,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   248,     0,     0,
       0,     0,   487,     0,     0,     0,     0,     0,   249,     0,
       0,     0,     0,     0,     0,     0,     0,   248,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   249,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     248,     0,     0,     0,     0,     0,     0,     0,     0,   249,
       0,     0,     0,     0,     0,     0,     0,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   249,     0,     0,     0,     0,   661,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,     0,     0,     0,     0,   356,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   248,     0,     0,     0,   475,     0,     0,
       0,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   248,     0,     0,     0,
     481,     0,     0,     0,     0,   249,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   248,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   249,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   249,     0,
       0,     0,     0,     0,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,     0,
       0,     0,     0,   489,     0,     0,     0,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,     0,     0,     0,     0,   659,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,     1,     0,     0,     2,   660,     0,     3,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     5,     0,
       0,     6,     0,     7,     8,     9,     0,     0,   248,    10,
       0,     0,     0,    11,     0,   472,     0,     0,     0,     0,
       0,     0,     0,   248,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   248,     0,     0,
     249,     0,     0,     0,     0,     0,    13,     0,     0,     0,
       0,     0,     0,     0,     0,   249,    14,     0,     0,    15,
       0,    16,     0,    17,     0,     0,    18,     0,    19,   249,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264
};

static const short yycheck[] =
{
       4,   522,   215,   526,   217,   218,   503,   443,   221,   222,
      17,   601,    27,     5,   177,     5,     8,    21,    34,   182,
      21,   184,   520,     9,     5,   523,    13,    14,     5,     6,
       5,     8,    45,     8,   461,    39,   199,    14,    41,     5,
       6,    50,     8,   299,     5,     6,     9,     5,    14,    78,
       5,     6,    50,    14,    13,    14,    60,    61,    62,    14,
     558,    79,   364,   561,   562,   563,   564,   565,   566,    13,
      14,   327,     5,     6,    21,     8,    98,    64,    51,     5,
       6,    14,     8,    78,   121,    59,   203,    18,    14,    95,
      68,    51,    14,   143,    50,   145,     0,   594,    95,   115,
      15,   107,    59,    68,     0,    42,   143,   756,    30,   138,
      83,   115,   116,   117,   118,   119,   117,   121,   122,   123,
     422,   143,   285,    83,    16,   143,   104,   140,   143,    44,
     557,   780,   135,    63,   143,   111,   726,    74,    42,   141,
      87,   638,   134,   138,   134,   143,   146,   151,    95,   739,
     142,   155,   142,   134,    42,   141,   121,   134,   165,   134,
     147,   142,   166,   167,   141,   282,   756,   171,   134,   605,
      74,   121,    19,   134,   671,   141,   134,   141,   141,   134,
     141,    42,   104,   127,   188,   621,    74,   191,   147,   779,
     780,    69,    39,   143,   198,   141,   133,   134,   135,   136,
     137,   134,   141,   147,    24,   461,   704,   705,   134,   213,
      64,   142,   143,    74,   141,   712,   124,     5,     6,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   142,   142,   143,   127,    58,   243,
     763,   245,   359,   406,    64,   408,    96,   135,   136,   137,
      88,   101,    90,   142,   143,   147,   141,   149,   150,    97,
     781,   141,    25,    25,    26,   521,   270,   271,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   141,   282,   142,
     143,    64,   142,   143,    47,    47,   142,   143,   141,   293,
     142,   143,   548,    42,   550,   299,   300,   553,    61,   303,
      62,   557,   141,   195,    66,   741,   141,   127,   128,   129,
     130,   131,   132,    75,    76,   142,   143,   141,   138,   141,
     212,    98,   214,   327,   141,    74,   443,   141,   220,   142,
     143,   223,   224,   225,   226,   141,   340,   142,   143,   142,
     143,   127,   105,   105,    98,   601,   142,   143,   240,    16,
     142,   143,   244,   357,   111,   359,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   142,   143,   142,   143,   269,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   393,
     127,   395,    59,   111,   142,   143,   142,   143,   270,   271,
      68,    79,   658,   520,   118,    72,   523,    89,   110,    93,
     114,   114,   142,   305,   418,    82,   142,    84,    85,   141,
     141,   538,   141,     5,    52,   134,   127,   431,   127,   433,
     434,   435,   127,    65,   104,   104,     8,   104,    32,   331,
     107,   558,   109,   141,   561,   562,   563,   564,   565,   566,
      95,    32,   119,   143,     8,     8,   127,   461,   575,   142,
     148,   353,   354,   355,   143,    78,   141,     9,   472,     5,
     726,    67,   728,    42,   127,    99,     5,   143,    99,   127,
       8,    80,   117,   739,   142,   102,   102,   141,   605,   141,
     141,   141,   141,   124,   141,   141,    96,   141,   127,   142,
     756,   142,    53,     5,   621,    74,    27,   511,   512,   722,
     143,   123,   127,   127,   141,     8,     8,   521,   143,     8,
     142,     8,   142,   779,   780,   135,    58,    58,     5,   122,
      88,   142,     8,   141,   143,   142,   428,   141,   430,   142,
     142,   142,    80,    78,   548,   142,   550,   142,   124,   553,
     124,   142,   141,   557,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    16,
      27,   575,    53,    56,     8,   127,   789,   141,   470,   148,
     143,     3,    29,    18,    31,   589,   143,   704,   705,   593,
      37,    27,   142,   485,   143,   487,   488,   601,   143,   142,
     604,    69,    80,   142,    42,    52,   143,   127,   142,     8,
       5,    27,    59,   143,   731,   142,   620,     8,   143,   143,
       5,   196,   141,    42,   741,    72,   142,   753,   498,   303,
     728,   331,   712,   576,   729,    82,    74,    84,    85,   244,
     739,   512,   733,   474,   357,   395,   575,   692,   393,   772,
     604,   621,   656,   100,   658,    74,   443,   104,   593,   521,
     107,   108,   109,    18,   658,   430,   254,   428,   241,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   576,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,   142,   143,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   723,
      -1,    -1,   726,    -1,   728,    -1,    -1,    -1,    -1,   733,
      -1,    -1,    -1,    -1,    -1,   739,    -1,     3,     4,     5,
       6,    -1,     8,     9,    10,    -1,    -1,    -1,    14,   753,
      16,    -1,   756,    -1,    20,    21,    22,    23,    -1,    25,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,   779,   780,    43,    -1,    -1,
      -1,    47,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    57,    58,    -1,    -1,    -1,    62,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    -1,    92,    93,    94,    95,
      -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
      -1,    -1,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
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
      -1,    -1,   134,   135,    -1,    -1,   138,    -1,    -1,   141,
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
      88,    -1,    90,    -1,    92,    93,    94,    95,    96,    97,
      -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,    -1,    -1,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
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
      -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
      -1,    -1,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
      -1,    -1,   138,    -1,    -1,   141,   142,    -1,   144,     3,
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
     134,    -1,    -1,    -1,   138,    -1,    -1,   141,    -1,    -1,
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
     120,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,    -1,    -1,   134,    -1,    -1,    -1,   138,    -1,
      -1,   141,    16,    -1,   144,    -1,    20,    21,    22,    23,
      -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      -1,    45,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,    -1,    -1,   118,   119,   120,    -1,   122,    -1,
      -1,     3,    -1,     5,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    16,    -1,    -1,   141,    20,    21,
      22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    74,
      -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,     3,   111,
      -1,   113,   114,   115,    -1,    -1,   118,   119,   120,    -1,
     122,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,   141,
      35,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    -1,    -1,    93,    94,
      95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,   111,    -1,   113,   114,
     115,    -1,    -1,   118,   119,   120,    -1,   122,    -1,    -1,
       3,    -1,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,   141,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,   112,
     113,   114,   115,    -1,    -1,   118,   119,   120,    -1,   122,
       3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,    -1,    -1,   118,   119,   120,    -1,   122,
       3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    45,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    -1,   111,    -1,
     113,   114,   115,     3,     4,   118,   119,   120,    -1,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    62,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,    -1,    -1,   118,   119,
     120,     3,   122,     5,     6,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,     3,   111,
      -1,   113,   114,   115,    -1,    10,   118,   119,   120,    -1,
     122,    16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    29,    -1,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    -1,    -1,    93,    94,
      95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,
     105,   106,    -1,   108,   109,    -1,    -1,    -1,   113,   114,
     115,    -1,    -1,   118,   119,   120,     3,   122,     5,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,     3,   111,     5,   113,   114,   115,    -1,
      -1,   118,   119,   120,    -1,   122,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
       3,   111,    -1,   113,   114,   115,    -1,    -1,   118,   119,
     120,    -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,     3,   111,    -1,
     113,   114,   115,    -1,    -1,   118,   119,   120,    -1,   122,
      16,    -1,    18,    -1,    20,    21,    22,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,     3,   113,   114,   115,
      -1,     8,   118,   119,   120,    -1,   122,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,     3,   111,    -1,   113,   114,   115,    -1,
      -1,   118,   119,   120,    -1,   122,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
       3,   111,    -1,   113,   114,   115,    -1,    -1,   118,   119,
     120,    -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,     3,   111,    -1,
     113,   114,   115,    -1,    -1,   118,   119,   120,    -1,   122,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,     3,   111,    -1,   113,   114,   115,
       9,    -1,   118,   119,   120,    -1,   122,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    65,    66,    67,    68,
      -1,    70,    71,    72,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      42,    90,    -1,    -1,    93,    94,    95,    74,    97,    -1,
      -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    42,   111,    -1,   113,   114,   115,    -1,    -1,   118,
     119,   120,    74,   122,    -1,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    74,   142,   143,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    42,    -1,    -1,    -1,
      -1,   143,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,    42,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    74,    -1,    -1,    -1,    -1,   143,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,   142,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    42,    -1,    -1,    -1,   142,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    42,    -1,    -1,    -1,
     142,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,   142,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,   142,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    17,    -1,    -1,    20,   142,    -1,    23,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    36,    -1,    38,    39,    40,    -1,    -1,    42,    44,
      -1,    -1,    -1,    48,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    91,    -1,    -1,    94,
      -1,    96,    -1,    98,    -1,    -1,   101,    -1,   103,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137
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
     177,   179,   236,   143,   237,    25,    47,    61,   105,   262,
       8,     4,    25,    35,    43,    45,    54,    57,    62,    70,
     120,   141,   153,   172,   173,   174,   175,   176,   276,     8,
      96,   157,    49,   142,   143,   142,   142,   153,   142,   142,
     142,   142,   142,   142,   142,   143,   142,   143,   143,   142,
     200,   200,   205,   153,   177,   206,   148,   148,   161,   170,
     171,   143,    78,   141,     5,   214,     9,   217,    67,   208,
     208,   127,   143,    99,     5,    99,     8,   153,   254,   255,
     127,   143,   171,   127,   283,    80,   188,   189,   285,    57,
     153,   223,   224,   117,   153,   153,   153,   142,   143,   142,
     143,   228,   235,   238,   239,   142,   102,   102,   141,   141,
     141,   141,   141,   141,   141,   141,   172,   124,    24,    58,
      64,   127,   128,   129,   130,   131,   132,   138,   142,   153,
     203,   142,   200,   200,   200,   127,    96,   164,    53,   182,
       5,   179,   127,    88,    90,    97,   260,     5,     8,   141,
     153,   197,   127,   123,   141,   177,   178,   248,   194,   177,
     178,    27,   192,   142,   143,   141,   225,   226,    25,    26,
      47,    62,    66,    75,    76,   105,   249,   177,     8,    18,
     240,   143,     8,     8,   153,   276,   135,   276,   142,   142,
     276,     8,   142,   142,   172,   177,   178,     9,   141,    78,
     138,     8,   177,   178,     8,   177,   178,   177,   178,   177,
     178,   177,   178,   177,   178,    58,   143,   158,    58,   142,
     142,   143,   206,   165,   142,     5,   183,   122,   186,   187,
     142,   141,    34,   115,    88,   153,   198,   199,   141,     8,
     255,   142,   179,    86,   176,   190,   191,   223,   141,   227,
     228,    79,   143,   229,    69,   153,   241,   228,   239,   142,
     142,   142,   142,   142,   124,   124,     8,   179,   180,   181,
      78,     9,   141,     5,    57,   153,   159,   160,   142,   153,
     275,   276,   141,    50,    80,   162,    27,    53,    56,   185,
     179,   127,   142,   143,     8,   142,   141,    19,    39,   143,
     142,   143,     3,   230,   231,   226,   177,   178,   177,   178,
     143,   142,   181,   143,   142,   202,    27,    69,   163,   176,
     184,    80,   174,   188,   142,   177,   199,   142,   142,   191,
     228,   127,   143,     8,   142,   160,   142,   190,     5,   143,
      27,   192,     8,   231,   143,   143,   176,   190,   194,   141,
       5,   202,   142,   142
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

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 381:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 382:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 383:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 384:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 391:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 392:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 393:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 401:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 402:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 403:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 404:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 405:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 406:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 407:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 408:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 409:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 410:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 411:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 414:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 415:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 416:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 417:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 418:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 421:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 423:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 424:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 425:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 426:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 427:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 428:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 429:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 433:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 435:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 438:

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

