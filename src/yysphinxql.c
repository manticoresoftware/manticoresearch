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
#define YYLAST   4510

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  149
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  136
/* YYNRULES -- Number of rules. */
#define YYNRULES  434
/* YYNRULES -- Number of states. */
#define YYNSTATES  789

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
     492,   495,   497,   499,   503,   504,   509,   510,   512,   514,
     518,   519,   522,   523,   525,   531,   532,   534,   538,   544,
     546,   550,   552,   555,   558,   559,   561,   564,   569,   570,
     572,   575,   577,   581,   585,   589,   595,   602,   606,   608,
     612,   616,   618,   620,   622,   624,   626,   628,   630,   633,
     636,   640,   644,   648,   652,   656,   660,   664,   668,   672,
     676,   680,   684,   688,   692,   696,   700,   704,   708,   712,
     714,   716,   718,   722,   727,   732,   737,   742,   747,   752,
     757,   761,   768,   775,   779,   788,   803,   805,   809,   811,
     813,   817,   823,   825,   827,   829,   831,   834,   835,   838,
     840,   843,   846,   850,   852,   854,   856,   859,   864,   869,
     873,   878,   883,   885,   887,   888,   891,   896,   901,   906,
     910,   915,   920,   928,   934,   940,   950,   952,   954,   956,
     958,   960,   962,   966,   968,   970,   972,   974,   976,   978,
     980,   982,   984,   987,   995,   997,   999,  1000,  1004,  1006,
    1008,  1010,  1014,  1016,  1020,  1024,  1026,  1030,  1032,  1034,
    1036,  1040,  1043,  1044,  1047,  1049,  1053,  1057,  1062,  1069,
    1071,  1075,  1077,  1081,  1083,  1087,  1088,  1091,  1093,  1097,
    1101,  1102,  1104,  1106,  1108,  1112,  1114,  1116,  1120,  1124,
    1131,  1133,  1137,  1141,  1145,  1151,  1156,  1160,  1164,  1166,
    1168,  1170,  1172,  1174,  1176,  1178,  1180,  1188,  1195,  1200,
    1205,  1211,  1212,  1214,  1217,  1219,  1223,  1227,  1230,  1234,
    1241,  1242,  1244,  1246,  1249,  1252,  1255,  1257,  1265,  1267,
    1269,  1271,  1273,  1277,  1284,  1288,  1292,  1295,  1299,  1301,
    1305,  1308,  1312,  1316,  1324,  1330,  1332,  1334,  1337,  1339,
    1342,  1344,  1346,  1350,  1354,  1358,  1362,  1364,  1365,  1368,
    1370,  1373,  1375,  1377,  1381
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     150,     0,    -1,   151,    -1,   154,    -1,   154,   140,    -1,
     218,    -1,   230,    -1,   210,    -1,   211,    -1,   216,    -1,
     231,    -1,   240,    -1,   242,    -1,   243,    -1,   244,    -1,
     249,    -1,   254,    -1,   255,    -1,   259,    -1,   261,    -1,
     262,    -1,   263,    -1,   264,    -1,   265,    -1,   256,    -1,
     266,    -1,   268,    -1,   269,    -1,   270,    -1,   248,    -1,
     271,    -1,   272,    -1,     3,    -1,    16,    -1,    20,    -1,
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
     155,    -1,   154,   284,    -1,   156,    -1,   205,    -1,   157,
      -1,    96,     3,   141,   141,   157,   142,   158,   142,    -1,
     164,    -1,    96,   165,    50,   141,   161,   164,   142,   162,
     163,    -1,    -1,   143,   159,    -1,   160,    -1,   159,   143,
     160,    -1,   153,    -1,     5,    -1,    57,    -1,    -1,    80,
      27,   188,    -1,    -1,    69,     5,    -1,    69,     5,   143,
       5,    -1,    96,   165,    50,   169,   170,   180,   184,   183,
     186,   190,   192,    -1,   166,    -1,   165,   143,   166,    -1,
     135,    -1,   168,   167,    -1,    -1,   153,    -1,    18,   153,
      -1,   198,    -1,    22,   141,   198,   142,    -1,    71,   141,
     198,   142,    -1,    73,   141,   198,   142,    -1,   106,   141,
     198,   142,    -1,    55,   141,   198,   142,    -1,    35,   141,
     135,   142,    -1,    54,   141,   142,    -1,    35,   141,    41,
     153,   142,    -1,   153,    -1,   169,   143,   153,    -1,    -1,
     171,    -1,   121,   172,    -1,   173,    -1,   172,   124,   172,
      -1,   141,   172,   142,    -1,    70,   141,     8,   142,    -1,
     174,    -1,   176,   127,   177,    -1,   176,   128,   177,    -1,
     176,    58,   141,   179,   142,    -1,   176,   138,    58,   141,
     179,   142,    -1,   176,    58,     9,    -1,   176,   138,    58,
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
     142,    -1,    57,    -1,   274,    -1,    62,   141,   274,   142,
      -1,    43,   141,   274,   142,    -1,    25,   141,   274,   142,
      -1,    45,   141,   142,    -1,     5,    -1,   134,     5,    -1,
       6,    -1,   134,     6,    -1,    14,    -1,   177,    -1,   179,
     143,   177,    -1,    -1,    53,   181,    27,   182,    -1,    -1,
       5,    -1,   176,    -1,   182,   143,   176,    -1,    -1,    56,
     174,    -1,    -1,   185,    -1,   122,    53,    80,    27,   188,
      -1,    -1,   187,    -1,    80,    27,   188,    -1,    80,    27,
      86,   141,   142,    -1,   189,    -1,   188,   143,   189,    -1,
     176,    -1,   176,    19,    -1,   176,    39,    -1,    -1,   191,
      -1,    69,     5,    -1,    69,     5,   143,     5,    -1,    -1,
     193,    -1,    79,   194,    -1,   195,    -1,   194,   143,   195,
      -1,   153,   127,   153,    -1,   153,   127,     5,    -1,   153,
     127,   141,   196,   142,    -1,   153,   127,   153,   141,     8,
     142,    -1,   153,   127,     8,    -1,   197,    -1,   196,   143,
     197,    -1,   153,   127,   177,    -1,   153,    -1,     4,    -1,
      57,    -1,     5,    -1,     6,    -1,    14,    -1,     9,    -1,
     134,   198,    -1,   138,   198,    -1,   198,   133,   198,    -1,
     198,   134,   198,    -1,   198,   135,   198,    -1,   198,   136,
     198,    -1,   198,   129,   198,    -1,   198,   130,   198,    -1,
     198,   126,   198,    -1,   198,   125,   198,    -1,   198,   137,
     198,    -1,   198,    42,   198,    -1,   198,    74,   198,    -1,
     198,   132,   198,    -1,   198,   131,   198,    -1,   198,   127,
     198,    -1,   198,   128,   198,    -1,   198,   124,   198,    -1,
     198,   123,   198,    -1,   141,   198,   142,    -1,   144,   202,
     145,    -1,   199,    -1,   274,    -1,   277,    -1,   273,    64,
      78,    -1,   273,    64,   138,    78,    -1,     3,   141,   200,
     142,    -1,    58,   141,   200,   142,    -1,    62,   141,   200,
     142,    -1,    25,   141,   200,   142,    -1,    47,   141,   200,
     142,    -1,    43,   141,   200,   142,    -1,     3,   141,   142,
      -1,    73,   141,   198,   143,   198,   142,    -1,    71,   141,
     198,   143,   198,   142,    -1,   120,   141,   142,    -1,     3,
     141,   198,    49,   153,    58,   273,   142,    -1,    92,   141,
     198,   143,   198,   143,   141,   200,   142,   143,   141,   200,
     142,   142,    -1,   201,    -1,   200,   143,   201,    -1,   198,
      -1,     8,    -1,   203,   127,   204,    -1,   202,   143,   203,
     127,   204,    -1,   153,    -1,    58,    -1,   177,    -1,   153,
      -1,   101,   207,    -1,    -1,    68,     8,    -1,   119,    -1,
     104,   206,    -1,    72,   206,    -1,    16,   104,   206,    -1,
      85,    -1,    82,    -1,    84,    -1,   109,   192,    -1,    16,
       8,   104,   206,    -1,    16,   153,   104,   206,    -1,   208,
     153,   104,    -1,   208,   153,   209,    99,    -1,   208,   153,
      14,    99,    -1,    59,    -1,   107,    -1,    -1,    30,     5,
      -1,    98,   152,   127,   213,    -1,    98,   152,   127,   212,
      -1,    98,   152,   127,    78,    -1,    98,    77,   214,    -1,
      98,    10,   127,   214,    -1,    98,    29,    98,   214,    -1,
      98,    52,     9,   127,   141,   179,   142,    -1,    98,    52,
     152,   127,   212,    -1,    98,    52,   152,   127,     5,    -1,
      98,    59,   153,    52,     9,   127,   141,   179,   142,    -1,
     153,    -1,     8,    -1,   112,    -1,    46,    -1,   177,    -1,
     215,    -1,   214,   134,   215,    -1,   153,    -1,    78,    -1,
       8,    -1,     5,    -1,     6,    -1,    33,    -1,    94,    -1,
     217,    -1,    23,    -1,   103,   111,    -1,   219,    63,   153,
     220,   117,   223,   227,    -1,    60,    -1,    91,    -1,    -1,
     141,   222,   142,    -1,   153,    -1,    57,    -1,   221,    -1,
     222,   143,   221,    -1,   224,    -1,   223,   143,   224,    -1,
     141,   225,   142,    -1,   226,    -1,   225,   143,   226,    -1,
     177,    -1,   178,    -1,     8,    -1,   141,   179,   142,    -1,
     141,   142,    -1,    -1,    79,   228,    -1,   229,    -1,   228,
     143,   229,    -1,     3,   127,     8,    -1,    38,    50,   169,
     171,    -1,    28,   153,   141,   232,   235,   142,    -1,   233,
      -1,   232,   143,   233,    -1,   226,    -1,   141,   234,   142,
      -1,     8,    -1,   234,   143,     8,    -1,    -1,   143,   236,
      -1,   237,    -1,   236,   143,   237,    -1,   226,   238,   239,
      -1,    -1,    18,    -1,   153,    -1,    69,    -1,   241,   153,
     206,    -1,    40,    -1,    39,    -1,   101,   108,   206,    -1,
     101,    37,   206,    -1,   116,   169,    98,   245,   171,   192,
      -1,   246,    -1,   245,   143,   246,    -1,   153,   127,   177,
      -1,   153,   127,   178,    -1,   153,   127,   141,   179,   142,
      -1,   153,   127,   141,   142,    -1,   274,   127,   177,    -1,
     274,   127,   178,    -1,    62,    -1,    25,    -1,    47,    -1,
      26,    -1,    75,    -1,    76,    -1,    66,    -1,   105,    -1,
      17,   107,   153,    15,    32,   153,   247,    -1,    17,   107,
     153,    44,    32,   153,    -1,    17,    95,   153,    89,    -1,
     101,   257,   118,   250,    -1,   101,   257,   118,    68,     8,
      -1,    -1,   251,    -1,   121,   252,    -1,   253,    -1,   252,
     123,   253,    -1,   153,   127,     8,    -1,   101,    31,    -1,
     101,    29,    98,    -1,    98,   257,   111,    65,    67,   258,
      -1,    -1,    52,    -1,   100,    -1,    88,   115,    -1,    88,
      34,    -1,    90,    88,    -1,    97,    -1,    36,    51,   153,
      93,   260,   102,     8,    -1,    61,    -1,    25,    -1,    47,
      -1,   105,    -1,    44,    51,   153,    -1,    20,    59,   153,
     110,    95,   153,    -1,    48,    95,   153,    -1,    48,    87,
     153,    -1,    48,    21,    -1,    96,   267,   190,    -1,    10,
      -1,    10,   146,   153,    -1,    96,   198,    -1,   113,    95,
     153,    -1,    81,    59,   153,    -1,    36,    83,   153,   114,
       8,   102,     8,    -1,    44,    83,   153,   114,     8,    -1,
     274,    -1,   153,    -1,   153,   275,    -1,   276,    -1,   275,
     276,    -1,    13,    -1,    14,    -1,   147,   198,   148,    -1,
     147,     8,   148,    -1,   198,   127,   278,    -1,   278,   127,
     198,    -1,     8,    -1,    -1,   280,   283,    -1,    27,    -1,
     282,   167,    -1,   198,    -1,   281,    -1,   283,   143,   281,
      -1,    45,   283,   279,   186,   190,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   175,   175,   176,   177,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   218,   219,   219,   219,   219,   219,   219,   220,
     220,   220,   220,   221,   221,   221,   221,   221,   222,   222,
     222,   222,   222,   223,   223,   223,   223,   223,   223,   223,
     224,   224,   224,   224,   225,   225,   225,   225,   225,   226,
     226,   226,   226,   226,   226,   227,   227,   227,   227,   228,
     232,   232,   232,   238,   239,   240,   244,   245,   249,   250,
     258,   259,   266,   268,   272,   276,   283,   284,   285,   289,
     302,   309,   311,   316,   325,   341,   342,   346,   347,   350,
     352,   353,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   369,   370,   373,   375,   379,   383,   384,   385,   389,
     394,   398,   405,   413,   421,   430,   435,   440,   445,   450,
     455,   460,   465,   470,   475,   480,   485,   490,   495,   500,
     505,   510,   515,   520,   525,   533,   537,   538,   543,   549,
     555,   561,   567,   568,   569,   570,   571,   575,   576,   587,
     588,   589,   593,   599,   605,   607,   610,   612,   619,   623,
     629,   631,   637,   639,   643,   654,   656,   660,   664,   671,
     672,   676,   677,   678,   681,   683,   687,   692,   699,   701,
     705,   709,   710,   714,   719,   724,   730,   735,   743,   748,
     755,   765,   766,   767,   768,   769,   770,   771,   772,   773,
     775,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   797,   798,   802,   803,   804,   805,   806,   807,
     808,   809,   810,   811,   812,   813,   817,   818,   822,   823,
     827,   828,   832,   833,   837,   838,   844,   847,   849,   853,
     854,   855,   856,   857,   858,   859,   860,   861,   866,   871,
     876,   881,   890,   891,   894,   896,   904,   909,   914,   919,
     920,   921,   925,   930,   935,   940,   949,   950,   954,   955,
     956,   968,   969,   973,   974,   975,   976,   977,   984,   985,
     986,   990,   991,   997,  1005,  1006,  1009,  1011,  1015,  1016,
    1020,  1021,  1025,  1026,  1030,  1034,  1035,  1039,  1040,  1041,
    1042,  1043,  1046,  1047,  1051,  1052,  1056,  1062,  1072,  1080,
    1084,  1091,  1092,  1099,  1109,  1115,  1117,  1121,  1126,  1130,
    1137,  1139,  1143,  1144,  1150,  1158,  1159,  1165,  1169,  1175,
    1183,  1184,  1188,  1202,  1208,  1212,  1217,  1231,  1242,  1243,
    1244,  1245,  1246,  1247,  1248,  1249,  1253,  1261,  1268,  1279,
    1283,  1290,  1291,  1295,  1299,  1300,  1304,  1308,  1315,  1322,
    1328,  1329,  1330,  1334,  1335,  1336,  1337,  1343,  1354,  1355,
    1356,  1357,  1362,  1373,  1385,  1394,  1403,  1413,  1421,  1422,
    1426,  1436,  1447,  1458,  1469,  1480,  1481,  1485,  1488,  1489,
    1493,  1494,  1495,  1496,  1500,  1501,  1505,  1510,  1512,  1516,
    1525,  1529,  1537,  1538,  1542
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
     183,   183,   184,   184,   185,   186,   186,   187,   187,   188,
     188,   189,   189,   189,   190,   190,   191,   191,   192,   192,
     193,   194,   194,   195,   195,   195,   195,   195,   196,   196,
     197,   198,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   198,   198,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   200,   200,   201,   201,
     202,   202,   203,   203,   204,   204,   205,   206,   206,   207,
     207,   207,   207,   207,   207,   207,   207,   207,   207,   207,
     207,   207,   208,   208,   209,   209,   210,   210,   210,   210,
     210,   210,   211,   211,   211,   211,   212,   212,   213,   213,
     213,   214,   214,   215,   215,   215,   215,   215,   216,   216,
     216,   217,   217,   218,   219,   219,   220,   220,   221,   221,
     222,   222,   223,   223,   224,   225,   225,   226,   226,   226,
     226,   226,   227,   227,   228,   228,   229,   230,   231,   232,
     232,   233,   233,   234,   234,   235,   235,   236,   236,   237,
     238,   238,   239,   239,   240,   241,   241,   242,   243,   244,
     245,   245,   246,   246,   246,   246,   246,   246,   247,   247,
     247,   247,   247,   247,   247,   247,   248,   248,   248,   249,
     249,   250,   250,   251,   252,   252,   253,   254,   255,   256,
     257,   257,   257,   258,   258,   258,   258,   259,   260,   260,
     260,   260,   261,   262,   263,   264,   265,   266,   267,   267,
     268,   269,   270,   271,   272,   273,   273,   274,   275,   275,
     276,   276,   276,   276,   277,   277,   278,   279,   279,   280,
     281,   282,   283,   283,   284
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
       2,     1,     1,     3,     0,     4,     0,     1,     1,     3,
       0,     2,     0,     1,     5,     0,     1,     3,     5,     1,
       3,     1,     2,     2,     0,     1,     2,     4,     0,     1,
       2,     1,     3,     3,     3,     5,     6,     3,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     1,     3,     4,     4,     4,     4,     4,     4,     4,
       3,     6,     6,     3,     8,    14,     1,     3,     1,     1,
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
       2,     3,     3,     7,     5,     1,     1,     2,     1,     2,
       1,     1,     3,     3,     3,     3,     1,     0,     2,     1,
       2,     1,     1,     3,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   311,     0,   308,     0,     0,   356,   355,
       0,     0,   314,     0,   315,   309,     0,   390,   390,     0,
       0,     0,     0,     2,     3,    83,    86,    88,    90,    87,
       7,     8,     9,   310,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    29,    15,    16,    17,    24,    18,    19,
      20,    21,    22,    23,    25,    26,    27,    28,    30,    31,
       0,     0,     0,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    79,
      48,    49,    50,    51,    52,    81,    53,    54,    55,    56,
      58,    57,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    82,    72,    73,    74,    75,
      76,    77,    78,    80,     0,     0,     0,     0,     0,     0,
     406,     0,     0,     0,    32,   212,   214,   215,   426,   217,
     408,   216,    36,     0,    40,     0,     0,    45,    46,   213,
       0,     0,    51,     0,     0,    69,    77,     0,   107,     0,
       0,     0,   211,     0,   105,   109,   112,   239,   194,     0,
     240,   241,     0,     0,     0,    43,     0,     0,    65,     0,
       0,     0,     0,   387,   267,   391,   282,   267,   274,   275,
     273,   392,   267,   283,   267,   198,   269,   266,     0,     0,
     312,     0,   121,     0,     1,     0,     4,    85,     0,   267,
       0,     0,     0,     0,     0,     0,     0,   402,     0,   405,
     404,   412,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    32,    51,
       0,   218,   219,     0,   263,   262,     0,     0,   420,   421,
       0,   417,   418,     0,     0,     0,   110,   108,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   407,   195,     0,     0,
       0,     0,     0,     0,     0,   306,   307,   305,   304,   303,
     289,   301,     0,     0,     0,   267,     0,   388,     0,   358,
     271,   270,   357,     0,   276,   199,   284,   381,   411,     0,
       0,   431,   432,   109,   427,     0,     0,    84,   316,   354,
     378,     0,     0,     0,   167,   169,   329,   171,     0,     0,
     327,   328,   341,   345,   339,     0,     0,     0,   337,     0,
     259,     0,   250,   258,     0,   256,   409,     0,   258,     0,
       0,     0,     0,     0,   119,     0,     0,     0,     0,     0,
       0,     0,   253,     0,     0,     0,   237,     0,   238,     0,
     426,     0,   419,    99,   123,   106,   112,   111,   229,   230,
     236,   235,   227,   226,   233,   424,   234,   224,   225,   232,
     231,   220,   221,   222,   223,   228,   196,   242,     0,   425,
     290,   291,     0,     0,     0,     0,   297,   299,   288,   298,
       0,   296,   300,   287,   286,     0,   267,   272,   267,   268,
       0,   200,   201,     0,     0,   279,     0,     0,     0,   379,
     382,     0,     0,   360,     0,   122,   430,   429,     0,   185,
       0,     0,     0,     0,     0,     0,   168,   170,   343,   331,
     172,     0,     0,     0,     0,   399,   400,   398,   401,     0,
       0,   157,     0,    40,     0,     0,    45,   161,     0,    50,
      77,     0,   156,   125,   126,   130,   143,     0,   162,   414,
       0,     0,     0,   244,     0,   113,   247,     0,   118,   249,
     248,   117,   245,   246,   114,     0,   115,     0,     0,   116,
       0,     0,     0,   265,   264,   260,   423,   422,     0,   174,
     124,     0,   243,     0,   294,   293,     0,   302,     0,   277,
     278,     0,     0,   281,   285,   280,   380,     0,   383,   384,
       0,     0,   198,     0,   433,     0,   194,   186,   428,   319,
     318,   320,     0,     0,     0,   377,   403,   330,     0,   342,
       0,   350,   340,   346,   347,   338,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    92,     0,
     257,   120,     0,     0,     0,     0,     0,     0,   176,   182,
     197,     0,     0,     0,     0,   396,   389,   204,   207,     0,
     203,   202,     0,     0,     0,   362,   363,   361,   359,   366,
     367,     0,   434,   317,     0,     0,   332,   322,   369,   371,
     370,   368,   374,   372,   373,   375,   376,   173,   344,   351,
       0,     0,   397,   413,     0,     0,     0,     0,   166,   159,
       0,     0,   160,   128,   127,     0,     0,   135,     0,   153,
       0,   151,   131,   142,   152,   132,   155,   139,   148,   138,
     147,   140,   149,   141,   150,     0,     0,     0,     0,   252,
     251,     0,   261,     0,     0,   177,     0,     0,   180,   183,
     292,     0,   394,   393,   395,     0,     0,   208,     0,   386,
     385,   365,     0,    58,   191,   187,   189,   321,     0,     0,
     325,     0,     0,   313,   353,   352,   349,   350,   348,   165,
     158,   164,   163,   129,     0,     0,     0,   154,   136,     0,
      97,    98,    96,    93,    94,    89,   416,     0,   415,     0,
       0,     0,   101,     0,     0,     0,   185,     0,     0,   205,
       0,     0,   364,     0,   192,   193,     0,   324,     0,     0,
     333,   334,   323,   137,   145,   146,   144,   133,     0,     0,
     254,     0,     0,     0,    91,   178,   175,     0,   181,   194,
     295,   210,   209,   206,   188,   190,   326,     0,     0,   134,
      95,     0,   100,   102,     0,     0,   198,   336,   335,     0,
       0,   179,   184,   104,     0,   103,     0,     0,   255
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   113,   152,    24,    25,    26,    27,   657,
     713,   714,   498,   722,   754,    28,   153,   154,   247,   155,
     364,   499,   328,   463,   464,   465,   466,   467,   440,   321,
     441,   579,   666,   756,   726,   668,   669,   526,   527,   685,
     686,   266,   267,   294,   295,   411,   412,   676,   677,   338,
     157,   334,   335,   236,   237,   495,    29,   289,   187,   188,
     416,    30,    31,   403,   404,   280,   281,    32,    33,    34,
      35,   432,   531,   532,   606,   607,   689,   322,   693,   740,
     741,    36,    37,   323,   324,   442,   444,   543,   544,   620,
     696,    38,    39,    40,    41,    42,   422,   423,   616,    43,
      44,   419,   420,   518,   519,    45,    46,    47,   170,   586,
      48,   449,    49,    50,    51,    52,    53,    54,   158,    55,
      56,    57,    58,    59,   159,   160,   241,   242,   161,   162,
     429,   430,   302,   303,   304,   197
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -700
static const short yypact[] =
{
    4330,    30,    -8,  -700,  3670,  -700,    49,    20,  -700,  -700,
      52,    51,  -700,    24,  -700,  -700,   730,  2801,  3871,   -20,
      50,  3670,   136,  -700,   -18,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,   114,  -700,  -700,  -700,  3670,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    3670,  3670,  3670,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,    40,  3670,  3670,  3670,  3670,  3670,
    -700,  3670,  3670,  3670,    62,  -700,  -700,  -700,  -700,  -700,
      44,  -700,    67,    73,    99,   104,   107,   109,   112,  -700,
     119,   121,   123,   147,   149,   157,   161,  1582,  -700,  1582,
    1582,  3133,    41,    -3,  -700,  3240,    92,  -700,   187,   170,
     242,  -700,   188,   192,    94,  3777,  3670,  2694,   210,   198,
     216,  3349,   231,  -700,   262,  -700,  -700,   262,  -700,  -700,
    -700,  -700,   262,  -700,   262,   256,  -700,  -700,  3670,   219,
    -700,  3670,  -700,   -37,  -700,  1582,    77,  -700,  3670,   262,
     258,    72,   240,    38,   261,   237,    12,  -700,   260,  -700,
    -700,  -700,   872,  3670,  1582,  1724,    -5,  1724,  1724,   236,
    1582,  1724,  1724,  1582,  1582,  1582,  1582,   248,   251,   252,
     253,  -700,  -700,  4049,  -700,  -700,    46,   268,  -700,  -700,
    1866,    60,  -700,  2221,  1014,  3670,  -700,  -700,  1582,  1582,
    1582,  1582,  1582,  1582,  1582,  1582,  1582,  1582,  1582,  1582,
    1582,  1582,  1582,  1582,  1582,   391,  -700,  -700,   -50,  1582,
    2694,  2694,   271,   272,   348,  -700,  -700,  -700,  -700,  -700,
     267,  -700,  2343,   337,   300,   -19,   301,  -700,   399,  -700,
    -700,  -700,  -700,  3670,  -700,  -700,     4,   -27,  -700,  3670,
    3670,  4346,  -700,  3240,   -15,  1156,   438,  -700,   269,  -700,
    -700,   376,   377,   316,  -700,  -700,  -700,  -700,    93,     8,
    -700,  -700,  -700,   274,  -700,   186,   407,  1992,  -700,   410,
     292,  1298,  -700,  4331,    43,  -700,  -700,  4070,  4346,    53,
    3670,   280,    89,    95,  -700,  4145,   128,   131,  3868,  3889,
    3910,  4166,  -700,  1440,  1582,  1582,  -700,  3133,  -700,   609,
     275,   346,  -700,  -700,    12,  -700,  4346,  -700,  -700,  -700,
    4360,  4373,  2131,  2603,   250,  -700,   250,    65,    65,    65,
      65,   209,   209,  -700,  -700,  -700,   281,  -700,   354,   250,
     267,   267,   287,  2919,   424,  2694,  -700,  -700,  -700,  -700,
     429,  -700,  -700,  -700,  -700,   368,   262,  -700,   262,  -700,
     309,   294,  -700,   343,   445,  -700,   344,   444,  3670,  -700,
    -700,    66,    28,  -700,   326,  -700,  -700,  -700,  1582,   379,
    1582,  3456,   339,  3670,  3670,  3670,  -700,  -700,  -700,  -700,
    -700,   134,   137,    38,   318,  -700,  -700,  -700,  -700,   362,
     363,  -700,   325,   345,   351,   352,   355,  -700,   357,   358,
     359,  1992,    60,   360,  -700,  -700,  -700,   211,  -700,  -700,
    1014,   349,  3670,  -700,  1724,  -700,  -700,   353,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,  1582,  -700,  1582,  1582,  -700,
    3932,  4006,   374,  -700,  -700,  -700,  -700,  -700,   389,   434,
    -700,   497,  -700,    18,  -700,  -700,   382,  -700,   164,  -700,
    -700,  2114,  3670,  -700,  -700,  -700,  -700,   384,   380,  -700,
      34,  3670,   256,   115,  -700,   479,   187,  -700,   369,  -700,
    -700,  -700,   139,   372,   350,  -700,  -700,  -700,    18,  -700,
     506,    11,  -700,   373,  -700,  -700,   507,   510,  3670,   386,
    3670,   383,   385,  3670,   511,   387,   -64,  1992,   115,    16,
     -45,    76,    96,   115,   115,   115,   115,   466,   388,   468,
    -700,  -700,  4186,  4209,  4028,   609,  1014,   390,   523,   408,
    -700,   142,   392,    55,   446,  -700,  -700,  -700,  -700,  3670,
     394,  -700,   529,  3670,    14,  -700,  -700,  -700,  -700,  -700,
    -700,  2463,  -700,  -700,  3456,    63,   -34,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    3563,    63,  -700,  -700,    60,   397,   398,   401,  -700,  -700,
     406,   409,  -700,  -700,  -700,   417,   426,  -700,    18,  -700,
     474,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,  -700,    17,  3026,   412,  3670,  -700,
    -700,   414,  -700,     0,   476,  -700,   531,   508,   503,  -700,
    -700,    18,  -700,  -700,  -700,   433,   144,  -700,   554,  -700,
    -700,  -700,   151,   422,    56,   421,  -700,  -700,    27,   158,
    -700,   562,   372,  -700,  -700,  -700,  -700,   548,  -700,  -700,
    -700,  -700,  -700,  -700,   115,   115,   162,  -700,  -700,    18,
    -700,  -700,  -700,   425,  -700,  -700,    60,   427,  -700,  1724,
    3670,   540,   504,  2576,   492,  2576,   379,   165,    18,  -700,
    3670,   432,  -700,   435,  -700,  -700,  2576,  -700,    63,   448,
     436,  -700,  -700,  -700,  -700,  -700,  -700,  -700,   167,  3026,
    -700,   169,  2576,   571,  -700,  -700,   437,   555,  -700,   187,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,   573,   562,  -700,
    -700,   440,   421,   441,  2576,  2576,   256,  -700,  -700,   449,
     581,  -700,   421,  -700,  1724,  -700,   171,   450,  -700
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -700,  -700,  -700,    -6,    -4,  -700,   395,  -700,   263,  -700,
    -700,  -151,  -700,  -700,  -700,   101,    25,   361,   299,  -700,
       9,  -700,  -326,  -430,  -700,  -122,  -700,  -592,  -117,  -499,
    -497,  -700,  -700,  -700,  -700,  -700,  -700,  -120,  -700,  -699,
    -132,  -516,  -700,  -517,  -700,  -700,    97,  -700,  -123,   108,
    -700,  -214,   141,  -700,   265,    33,  -700,  -162,  -700,  -700,
    -700,  -700,  -700,   217,  -700,    47,   218,  -700,  -700,  -700,
    -700,  -700,    13,  -700,  -700,   -74,  -700,  -441,  -700,  -700,
    -149,  -700,  -700,  -700,   177,  -700,  -700,  -700,     2,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,   103,  -700,  -700,
    -700,  -700,  -700,  -700,    35,  -700,  -700,  -700,   608,  -700,
    -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,  -700,
    -700,  -700,  -700,  -700,   -31,  -285,  -700,   393,  -700,   396,
    -700,  -700,   205,  -700,   206,  -700
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -427
static const short yytable[] =
{
     114,   339,   541,   342,   343,   598,   581,   346,   347,   684,
     602,   169,   427,   314,   424,   290,   438,   192,   413,   314,
     291,   596,   292,   314,   600,   637,   708,   195,   387,   619,
     193,   556,   314,   639,   414,   199,   340,   309,   500,   314,
     315,   417,   468,   314,   315,   691,   316,   243,   317,   288,
     720,    62,   317,   772,   238,   239,   200,   201,   202,   636,
     557,   299,   643,   646,   648,   650,   652,   654,   314,   315,
     117,   316,   120,   238,   239,   734,   782,   317,   633,   238,
     239,   314,   315,   123,   641,   -67,   320,   311,   388,   672,
     317,   190,  -410,   640,   418,   735,   522,   682,   436,   437,
     115,   314,   315,   118,   644,  -416,   300,   248,   415,   692,
     317,   204,   205,   192,   207,   208,   312,   209,   210,   211,
     314,   315,   196,   407,   156,    60,   206,   634,   428,   317,
     341,   755,   116,   327,   248,   119,   194,    61,   121,   249,
     244,   706,   400,   244,   684,   191,   122,   235,   400,   327,
     439,   246,   400,  -341,  -341,   300,   681,   638,   709,   273,
     684,   400,   274,   279,   690,   402,   249,   286,   318,   439,
     673,   521,   318,   305,   727,   594,   468,   198,   306,   319,
     697,   203,   781,   684,   296,   473,   474,   298,   240,   357,
     213,   358,   271,   520,   308,   476,   474,   318,   260,   261,
     262,   263,   264,   212,   688,   744,   746,   240,   214,   336,
     318,   445,   748,   240,   215,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     318,   479,   474,   446,   268,   558,   424,   480,   474,   192,
     216,   367,   494,   776,   509,   217,   510,   447,   218,   318,
     219,   248,   583,   220,   584,   231,   265,   232,   233,   783,
     221,   585,   222,   625,   223,   627,   279,   279,   630,   559,
     482,   474,   468,   483,   474,   560,   537,   538,   401,   539,
     540,   603,   604,   249,   670,   538,   729,   730,   224,   410,
     225,   448,   248,   732,   538,   421,   425,   766,   226,   246,
     737,   738,   227,   301,   747,   538,  -415,   760,   538,   769,
     538,   771,   474,   787,   474,   269,   468,   390,   391,   270,
     333,  -392,   337,   462,   249,   282,   320,   283,   345,   287,
     288,   348,   349,   350,   351,   293,   477,   297,   561,   562,
     563,   564,   565,   566,   262,   263,   264,   310,   361,   567,
     313,   326,   366,   235,   325,   493,   368,   369,   370,   371,
     372,   373,   374,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   718,   329,   608,   609,   389,   344,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   248,   401,
     352,   279,   353,   354,   355,   359,   386,   610,   392,   393,
     394,   395,   405,   595,   406,   408,   599,   409,   433,   434,
     431,   435,   611,   366,   517,   450,   612,   443,   469,  -426,
     249,   617,   478,   496,   501,   613,   614,   530,   503,   534,
     535,   536,   502,   506,   436,   508,   511,   512,   468,   233,
     468,   635,   513,   515,   642,   645,   647,   649,   651,   653,
     514,   468,   516,   523,   171,   615,   533,   462,   494,   525,
     545,   333,   490,   491,   546,   547,   548,   468,   569,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   557,   576,   549,   578,   320,   468,
     468,   568,   550,   551,   497,   571,   552,   176,   553,   554,
     555,   575,   580,   593,   320,   751,   601,   590,   410,   582,
     177,   592,   428,   605,   618,   622,   621,   421,   623,   631,
     178,   626,   179,   180,   655,   628,   658,   629,   665,   632,
     667,   656,   664,   671,   674,   678,   301,   679,   301,   699,
     700,   704,   182,   701,   624,   183,   624,   185,   702,   624,
     705,   703,   707,   462,   715,   719,   721,   186,   723,   725,
     728,   724,   731,   733,   736,   739,   619,   752,   749,   750,
     786,   493,   757,   753,   763,   767,   773,   764,   366,   768,
     774,   777,   775,   779,   780,   675,   785,   743,   745,   517,
     784,   307,   788,   572,   471,   573,   574,   462,   770,   577,
     530,   663,   426,   758,   765,   365,   759,   762,   662,   591,
     505,   761,    63,   507,   314,   570,   695,   687,   742,   778,
     542,   320,   492,   698,   597,    64,   189,   717,   680,    65,
      66,    67,    68,   524,   362,    69,   528,     0,     0,     0,
      70,     0,     0,     0,    71,     0,     0,     0,     0,     0,
     375,     0,   712,     0,   716,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,     0,    82,
      83,    84,     0,     0,   366,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   100,   101,   102,   192,   103,   104,   462,
     105,   462,   106,   107,   108,     0,   675,   109,   110,   111,
       0,   112,   462,   124,   125,   126,   127,     0,   128,   129,
     130,     0,     0,   400,   131,   712,    64,     0,   462,     0,
      65,    66,   132,    68,     0,   133,    69,     0,     0,     0,
       0,    70,     0,     0,     0,   134,     0,     0,     0,     0,
     462,   462,     0,   135,     0,     0,     0,   136,    72,     0,
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
     330,   129,     0,     0,     0,     0,   131,     0,    64,     0,
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
       0,     0,     0,     0,     0,     0,   147,     0,     0,     0,
     149,     0,     0,   331,   332,     0,   151,   228,   125,   126,
     127,     0,   128,   129,     0,     0,     0,     0,   131,     0,
      64,     0,     0,     0,    65,    66,   132,    68,     0,   133,
      69,     0,     0,     0,     0,    70,     0,     0,     0,   134,
       0,     0,     0,     0,     0,     0,     0,   135,     0,     0,
       0,   136,    72,     0,     0,    73,    74,    75,   137,   138,
       0,   139,   140,     0,     0,     0,   141,     0,     0,    78,
      79,    80,    81,     0,    82,   142,    84,   143,     0,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,     0,   144,    94,    95,    96,
       0,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     145,     0,   103,   104,     0,   105,     0,   106,   107,   108,
       0,     0,   109,   110,   146,     0,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   147,   148,
       0,     0,   149,     0,     0,   150,     0,     0,   151,   124,
     125,   126,   127,     0,   128,   129,     0,     0,     0,     0,
     131,     0,    64,     0,     0,     0,    65,    66,   132,    68,
       0,   133,    69,     0,     0,     0,     0,    70,     0,     0,
       0,   134,     0,     0,     0,     0,     0,     0,     0,   135,
       0,     0,     0,   136,    72,     0,     0,    73,    74,    75,
     137,   138,     0,   139,   140,     0,     0,     0,   141,     0,
       0,    78,    79,    80,    81,     0,    82,   142,    84,   143,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,   144,    94,
      95,    96,     0,    97,     0,     0,    98,     0,     0,    99,
     100,   101,   145,     0,   103,   104,     0,   105,     0,   106,
     107,   108,     0,     0,   109,   110,   146,     0,   112,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     147,   148,     0,     0,   149,     0,     0,   150,     0,     0,
     151,   228,   125,   126,   127,     0,   128,   129,     0,     0,
       0,     0,   131,     0,    64,     0,     0,     0,    65,    66,
      67,    68,     0,   133,    69,     0,     0,     0,     0,    70,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
       0,   135,     0,     0,     0,   136,    72,     0,     0,    73,
      74,    75,    76,    77,     0,   139,   140,     0,     0,     0,
     141,     0,     0,    78,    79,    80,    81,     0,    82,   229,
      84,   230,     0,     0,     0,    85,     0,     0,     0,     0,
      86,    87,    88,    89,    90,    91,    92,     0,    93,     0,
     144,    94,    95,    96,   470,    97,     0,     0,    98,     0,
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
       0,   150,   332,     0,   151,   228,   125,   126,   127,     0,
     128,   129,     0,     0,     0,     0,   131,     0,    64,     0,
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
       0,     0,   149,     0,     0,   150,     0,     0,   151,   228,
     125,   126,   127,     0,   360,   129,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    63,   451,     0,     0,     0,
     147,     0,     0,     0,   149,     0,     0,   150,    64,     0,
     151,     0,    65,    66,    67,    68,     0,   452,    69,     0,
       0,     0,     0,    70,     0,     0,     0,   453,     0,     0,
       0,     0,     0,     0,     0,   454,     0,   455,     0,     0,
      72,     0,     0,    73,    74,    75,   456,    77,     0,   457,
       0,     0,     0,     0,   458,     0,     0,    78,    79,    80,
      81,     0,   459,    83,    84,     0,     0,     0,     0,    85,
       0,     0,     0,     0,    86,    87,    88,    89,    90,    91,
      92,     0,    93,     0,     0,    94,    95,    96,     0,    97,
       0,     0,    98,     0,     0,    99,   100,   101,   102,     0,
     103,   104,     0,   105,     0,   106,   107,   108,     0,     0,
     109,   110,   460,     0,   112,     0,     0,    63,     0,   587,
       0,     0,   588,     0,     0,     0,     0,     0,     0,     0,
      64,     0,     0,   461,    65,    66,    67,    68,     0,     0,
      69,     0,     0,     0,     0,    70,     0,     0,     0,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,     0,     0,    73,    74,    75,    76,    77,
       0,     0,     0,   248,     0,     0,     0,     0,     0,    78,
      79,    80,    81,     0,    82,    83,    84,     0,     0,     0,
       0,    85,     0,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,   249,     0,    94,    95,    96,
       0,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     102,     0,   103,   104,    63,   105,     0,   106,   107,   108,
       0,     0,   109,   110,   111,     0,   112,    64,     0,     0,
       0,    65,    66,    67,    68,     0,     0,    69,     0,     0,
       0,     0,    70,     0,     0,   589,    71,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    72,
       0,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    79,    80,    81,
       0,    82,    83,    84,     0,     0,     0,     0,    85,     0,
       0,     0,     0,    86,    87,    88,    89,    90,    91,    92,
       0,    93,     0,     0,    94,    95,    96,     0,    97,     0,
       0,    98,     0,     0,    99,   100,   101,   102,     0,   103,
     104,     0,   105,     0,   106,   107,   108,     0,     0,   109,
     110,   111,     0,   112,     0,     0,    63,     0,   314,     0,
       0,   396,     0,     0,     0,     0,     0,     0,     0,    64,
       0,     0,   363,    65,    66,    67,    68,     0,     0,    69,
       0,     0,     0,     0,    70,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   397,
       0,    72,     0,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,     0,    82,    83,    84,     0,     0,     0,     0,
      85,   398,     0,     0,     0,    86,    87,    88,    89,    90,
      91,    92,     0,    93,     0,     0,    94,    95,    96,     0,
      97,     0,     0,    98,     0,     0,    99,   100,   101,   102,
       0,   103,   104,     0,   105,   399,   106,   107,   108,     0,
       0,   109,   110,   111,     0,   112,    63,   451,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   400,     0,    64,
       0,     0,     0,    65,    66,    67,    68,     0,   452,    69,
       0,     0,     0,     0,    70,     0,     0,     0,   453,     0,
       0,     0,     0,     0,     0,     0,   454,     0,   455,     0,
       0,    72,     0,     0,    73,    74,    75,   456,    77,     0,
     457,     0,     0,     0,     0,   458,     0,     0,    78,    79,
      80,    81,     0,    82,    83,    84,     0,     0,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,   683,
      91,    92,     0,    93,     0,     0,    94,    95,    96,     0,
      97,     0,     0,    98,     0,     0,    99,   100,   101,   102,
       0,   103,   104,     0,   105,     0,   106,   107,   108,    63,
     451,   109,   110,   460,     0,   112,     0,     0,     0,     0,
       0,     0,    64,     0,     0,     0,    65,    66,    67,    68,
       0,   452,    69,     0,     0,     0,     0,    70,     0,     0,
       0,   453,     0,     0,     0,     0,     0,     0,     0,   454,
       0,   455,     0,     0,    72,     0,     0,    73,    74,    75,
     456,    77,     0,   457,     0,     0,     0,     0,   458,     0,
       0,    78,    79,    80,    81,   248,    82,    83,    84,     0,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,     0,    94,
      95,    96,     0,    97,     0,     0,    98,   249,     0,    99,
     100,   101,   102,     0,   103,   104,     0,   105,     0,   106,
     107,   108,     0,     0,   109,   110,   460,    63,   112,   275,
     276,     0,   277,     0,     0,     0,     0,     0,     0,     0,
      64,     0,     0,     0,    65,    66,    67,    68,     0,     0,
      69,     0,     0,     0,     0,    70,     0,     0,     0,    71,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,     0,    72,     0,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    78,
      79,    80,    81,     0,    82,    83,    84,     0,     0,     0,
       0,    85,   278,     0,     0,     0,    86,    87,    88,    89,
      90,    91,    92,     0,    93,     0,     0,    94,    95,    96,
       0,    97,     0,     0,    98,     0,     0,    99,   100,   101,
     102,     0,   103,   104,    63,   105,     0,   106,   107,   108,
       0,   163,   109,   110,   111,     0,   112,    64,     0,     0,
       0,    65,    66,    67,    68,     0,     0,    69,     0,     0,
     164,     0,    70,     0,     0,     0,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
       0,     0,    73,   165,    75,    76,    77,     0,     0,     0,
     166,     0,     0,     0,     0,     0,    78,    79,    80,    81,
       0,    82,    83,    84,     0,     0,     0,     0,   167,     0,
       0,     0,     0,    86,    87,    88,    89,    90,    91,    92,
       0,    93,     0,     0,    94,    95,    96,     0,    97,     0,
       0,   168,     0,     0,    99,   100,   101,   102,     0,   103,
     104,     0,     0,     0,   106,   107,   108,     0,     0,   109,
     110,   111,    63,   112,   504,     0,     0,   396,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,     0,    65,
      66,    67,    68,     0,     0,    69,     0,     0,     0,     0,
      70,     0,     0,     0,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,     0,    82,
      83,    84,     0,     0,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   100,   101,   102,     0,   103,   104,    63,
     105,   710,   106,   107,   108,     0,     0,   109,   110,   111,
       0,   112,    64,     0,     0,     0,    65,    66,    67,    68,
       0,     0,    69,     0,     0,     0,     0,    70,     0,     0,
       0,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,     0,     0,    73,    74,    75,
      76,    77,     0,   711,     0,     0,     0,     0,     0,     0,
       0,    78,    79,    80,    81,     0,    82,    83,    84,     0,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,     0,    94,
      95,    96,     0,    97,     0,     0,    98,     0,     0,    99,
     100,   101,   102,     0,   103,   104,    63,   105,     0,   106,
     107,   108,     0,     0,   109,   110,   111,     0,   112,    64,
       0,     0,     0,    65,    66,    67,    68,     0,     0,    69,
       0,     0,     0,     0,    70,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,     0,     0,    73,    74,    75,    76,    77,     0,
       0,   234,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,     0,    82,    83,    84,     0,     0,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,    90,
      91,    92,     0,    93,     0,     0,    94,    95,    96,     0,
      97,     0,     0,    98,     0,     0,    99,   100,   101,   102,
       0,   103,   104,    63,   105,     0,   106,   107,   108,     0,
       0,   109,   110,   111,     0,   112,    64,     0,   245,     0,
      65,    66,    67,    68,     0,     0,    69,     0,     0,     0,
       0,    70,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    78,    79,    80,    81,     0,
      82,    83,    84,     0,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,     0,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
       0,   105,    63,   106,   107,   108,     0,   284,   109,   110,
     111,     0,   112,     0,     0,    64,     0,     0,     0,    65,
      66,    67,    68,     0,     0,    69,     0,     0,     0,     0,
      70,     0,     0,     0,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78,    79,    80,    81,     0,    82,
      83,    84,     0,     0,     0,     0,    85,     0,     0,     0,
       0,    86,    87,    88,    89,    90,    91,    92,     0,    93,
       0,     0,    94,    95,    96,     0,    97,     0,     0,    98,
       0,     0,    99,   285,   101,   102,     0,   103,   104,    63,
     105,     0,   106,   107,   108,     0,     0,   109,   110,   111,
       0,   112,    64,     0,     0,     0,    65,    66,    67,    68,
       0,     0,    69,     0,     0,     0,     0,    70,     0,     0,
       0,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,     0,     0,    73,    74,    75,
      76,    77,     0,   529,     0,     0,     0,     0,     0,     0,
       0,    78,    79,    80,    81,     0,    82,    83,    84,     0,
       0,     0,     0,    85,     0,     0,     0,     0,    86,    87,
      88,    89,    90,    91,    92,     0,    93,     0,     0,    94,
      95,    96,     0,    97,     0,     0,    98,     0,     0,    99,
     100,   101,   102,     0,   103,   104,    63,   105,     0,   106,
     107,   108,     0,     0,   109,   110,   111,     0,   112,    64,
       0,     0,     0,    65,    66,    67,    68,     0,     0,    69,
       0,     0,     0,     0,    70,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,     0,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,   694,    82,    83,    84,     0,     0,     0,     0,
      85,     0,     0,     0,     0,    86,    87,    88,    89,    90,
      91,    92,     0,    93,     0,     0,    94,    95,    96,     0,
      97,     0,     0,    98,     0,     0,    99,   100,   101,   102,
       0,   103,   104,    63,   105,     0,   106,   107,   108,     0,
       0,   109,   110,   111,     0,   112,    64,     0,     0,     0,
      65,    66,    67,    68,     0,     0,    69,     0,     0,     0,
       0,    70,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    78,    79,    80,    81,     0,
      82,    83,    84,     0,     0,     0,     0,    85,     0,     0,
       0,     0,    86,    87,    88,    89,    90,    91,    92,     0,
      93,     0,     0,    94,    95,    96,     0,    97,     0,     0,
      98,     0,     0,    99,   100,   101,   102,     0,   103,   104,
      63,   105,     0,   106,   107,   108,   272,     0,   109,   110,
     111,     0,   112,    64,     0,     0,     0,    65,    66,    67,
      68,     0,     0,    69,     0,     0,     0,     0,    70,     0,
       0,     0,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    78,    79,    80,    81,     0,    82,    83,    84,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    86,
      87,    88,    89,    90,    91,    92,     0,    93,     0,     0,
      94,    95,    96,     0,    97,     0,     0,    98,     0,     0,
      99,   100,   101,   102,     0,   103,   104,   171,  -391,     0,
     106,   107,   108,     0,     0,   109,   110,   111,     0,   112,
     172,     0,   173,     0,     0,     0,     0,     0,   174,     0,
     248,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   175,     0,     0,     0,     0,     0,     0,
     176,   248,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   249,   177,     0,     0,     0,     0,     0,     0,
       0,     0,   248,   178,     0,   179,   180,     0,     0,     0,
       0,     0,     0,   249,     0,     0,     0,     0,     0,     0,
       0,   181,     0,     0,   248,   182,     0,     0,   183,   184,
     185,     0,     0,     0,   249,     0,     0,     0,     0,     0,
     186,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   249,     0,     0,     0,
     484,   485,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,     0,     0,     0,
       0,   486,   487,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   248,     0,
       0,     0,     0,   488,     0,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     248,     0,     0,     0,     0,   485,     0,     0,     0,     0,
     249,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   248,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   249,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   248,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   249,     0,     0,     0,     0,     0,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   249,     0,     0,     0,     0,   487,
       0,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,     0,     0,     0,     0,
       0,   661,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   248,     0,     0,
       0,   356,     0,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   248,     0,
       0,     0,   475,     0,     0,     0,     0,     0,     0,   249,
       0,     0,     0,     0,     0,     0,     0,     0,   248,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     249,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   248,     0,     0,     0,     0,     0,     0,     0,     0,
     249,     0,     0,     0,     0,     0,     0,     0,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   249,     0,     0,     0,   481,     0,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,     0,     0,     0,     0,   489,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,     0,     0,     0,     0,   659,     0,
       0,     0,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,     1,     0,     0,
       2,   660,     0,     3,     0,     0,     0,     0,     4,     0,
       0,     0,     0,     5,     0,     0,     6,     0,     7,     8,
       9,     0,     0,   248,    10,     0,     0,     0,    11,     0,
     472,     0,     0,     0,     0,     0,     0,     0,   248,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   248,     0,     0,   249,     0,     0,     0,     0,
       0,    13,     0,     0,     0,   248,     0,     0,     0,     0,
     249,    14,     0,     0,    15,     0,    16,     0,    17,     0,
       0,    18,     0,    19,   249,     0,     0,     0,     0,     0,
       0,     0,     0,    20,     0,     0,    21,   249,     0,     0,
       0,     0,     0,     0,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264
};

static const short yycheck[] =
{
       4,   215,   443,   217,   218,   522,   503,   221,   222,   601,
     526,    17,    27,     5,   299,   177,     8,    21,    14,     5,
     182,   520,   184,     5,   523,     9,     9,    45,    78,    18,
      21,   461,     5,    78,    30,    39,    41,   199,   364,     5,
       6,    68,   327,     5,     6,    79,     8,    50,    14,    68,
      50,    59,    14,   752,    13,    14,    60,    61,    62,   558,
     124,    98,   561,   562,   563,   564,   565,   566,     5,     6,
      50,     8,    21,    13,    14,    19,   775,    14,   142,    13,
      14,     5,     6,    59,     8,   104,   203,    15,   138,    34,
      14,   111,     0,   138,   121,    39,   422,   594,     5,     6,
      51,     5,     6,    51,     8,    64,   143,    42,   104,   143,
      14,   115,   116,   117,   118,   119,    44,   121,   122,   123,
       5,     6,   140,   285,    16,    95,   117,   557,   143,    14,
     135,   723,    83,   121,    42,    83,     0,   107,    87,    74,
     143,   638,   134,   143,   736,    95,    95,   151,   134,   121,
     142,   155,   134,   142,   143,   143,   142,   141,   141,   165,
     752,   134,   166,   167,   605,   282,    74,   171,   134,   142,
     115,   143,   134,    96,   671,   141,   461,    63,   101,   141,
     621,   141,   774,   775,   188,   142,   143,   191,   147,   143,
     146,   145,    98,   127,   198,   142,   143,   134,   133,   134,
     135,   136,   137,   141,   141,   704,   705,   147,   141,   213,
     134,    25,   709,   147,   141,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     134,   142,   143,    47,    64,    24,   521,   142,   143,   243,
     141,   245,   359,   759,   406,   141,   408,    61,   141,   134,
     141,    42,    88,   141,    90,   147,    69,   149,   150,   776,
     141,    97,   141,   548,   141,   550,   270,   271,   553,    58,
     142,   143,   557,   142,   143,    64,   142,   143,   282,   142,
     143,   142,   143,    74,   142,   143,   142,   143,   141,   293,
     141,   105,    42,   142,   143,   299,   300,   738,   141,   303,
     142,   143,   141,   195,   142,   143,    64,   142,   143,   142,
     143,   142,   143,   142,   143,   127,   601,   270,   271,   127,
     212,   111,   214,   327,    74,   127,   443,   111,   220,    98,
      68,   223,   224,   225,   226,    79,   340,   118,   127,   128,
     129,   130,   131,   132,   135,   136,   137,    89,   240,   138,
     110,   114,   244,   357,    93,   359,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   658,   114,    25,    26,   269,   142,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    42,   393,
     142,   395,   141,   141,   141,   127,     5,    47,   127,   127,
      52,   134,    65,   520,   104,   104,   523,     8,    32,    32,
     141,    95,    62,   305,   418,     8,    66,   143,     8,   127,
      74,   538,   142,   148,   143,    75,    76,   431,   141,   433,
     434,   435,    78,     9,     5,    67,   127,   143,   723,   331,
     725,   558,    99,    99,   561,   562,   563,   564,   565,   566,
       5,   736,     8,   127,    16,   105,   117,   461,   575,    80,
     142,   353,   354,   355,   102,   102,   141,   752,   472,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   124,    96,   141,    53,   605,   774,
     775,   142,   141,   141,   148,   142,   141,    59,   141,   141,
     141,   127,     5,   123,   621,   719,    27,   511,   512,   127,
      72,   127,   143,   141,     8,     8,   143,   521,     8,     8,
      82,   135,    84,    85,    58,   142,    58,   142,     5,   142,
     122,   143,   142,   141,    88,   141,   428,     8,   430,   142,
     142,   124,   104,   142,   548,   107,   550,   109,   142,   553,
     124,   142,    78,   557,   142,   141,    80,   119,    27,    56,
     127,    53,     8,   141,   143,     3,    18,    27,   143,   142,
     784,   575,    80,    69,   142,   127,     5,   142,   470,   143,
     143,     8,    27,   143,   143,   589,     5,   704,   705,   593,
     141,   196,   142,   485,   331,   487,   488,   601,   749,   498,
     604,   576,   303,   725,   736,   244,   726,   730,   575,   512,
     393,   728,     3,   395,     5,   474,   620,   604,   692,   768,
     443,   738,   357,   621,   521,    16,    18,   658,   593,    20,
      21,    22,    23,   428,   241,    26,   430,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
     254,    -1,   656,    -1,   658,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,   576,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,   720,   108,   109,   723,
     111,   725,   113,   114,   115,    -1,   730,   118,   119,   120,
      -1,   122,   736,     3,     4,     5,     6,    -1,     8,     9,
      10,    -1,    -1,   134,    14,   749,    16,    -1,   752,    -1,
      20,    21,    22,    23,    -1,    25,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
     774,   775,    -1,    43,    -1,    -1,    -1,    47,    48,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
     138,    -1,    -1,   141,   142,    -1,   144,     3,     4,     5,
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
     134,   135,    -1,    -1,   138,    -1,    -1,   141,    -1,    -1,
     144,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    -1,    25,    26,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    47,    48,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    57,    58,    -1,    -1,    -1,
      62,    -1,    -1,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    -1,    90,    -1,
      92,    93,    94,    95,    96,    97,    -1,    -1,   100,    -1,
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
      -1,   141,   142,    -1,   144,     3,     4,     5,     6,    -1,
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
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,    -1,    -1,
     134,    -1,    -1,    -1,   138,    -1,    -1,   141,    16,    -1,
     144,    -1,    20,    21,    22,    23,    -1,    25,    26,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,    67,
      68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,    -1,    -1,
     118,   119,   120,    -1,   122,    -1,    -1,     3,    -1,     5,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,   141,    20,    21,    22,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    74,    -1,    93,    94,    95,
      -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,     3,   111,    -1,   113,   114,   115,
      -1,    -1,   118,   119,   120,    -1,   122,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,   141,    35,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,
      -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,   111,    -1,   113,   114,   115,    -1,    -1,   118,
     119,   120,    -1,   122,    -1,    -1,     3,    -1,     5,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      -1,    -1,   141,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    78,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,   111,   112,   113,   114,   115,    -1,
      -1,   118,   119,   120,    -1,   122,     3,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    25,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    45,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    62,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,    -1,   111,    -1,   113,   114,   115,     3,
       4,   118,   119,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      -1,    25,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      -1,    45,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    65,    66,    67,    68,    42,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    74,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,    -1,    -1,   118,   119,   120,     3,   122,     5,
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,    -1,    48,    -1,    -1,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      -1,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,     3,   111,    -1,   113,   114,   115,
      -1,    10,   118,   119,   120,    -1,   122,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,
      29,    -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,
      -1,   100,    -1,    -1,   103,   104,   105,   106,    -1,   108,
     109,    -1,    -1,    -1,   113,   114,   115,    -1,    -1,   118,
     119,   120,     3,   122,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,     3,
     111,     5,   113,   114,   115,    -1,    -1,   118,   119,   120,
      -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,     3,   111,    -1,   113,
     114,   115,    -1,    -1,   118,   119,   120,    -1,   122,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,     3,   111,    -1,   113,   114,   115,    -1,
      -1,   118,   119,   120,    -1,   122,    16,    -1,    18,    -1,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,     3,   113,   114,   115,    -1,     8,   118,   119,
     120,    -1,   122,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,    -1,   108,   109,     3,
     111,    -1,   113,   114,   115,    -1,    -1,   118,   119,   120,
      -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    67,    68,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,     3,   111,    -1,   113,
     114,   115,    -1,    -1,   118,   119,   120,    -1,   122,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      67,    68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,   106,
      -1,   108,   109,     3,   111,    -1,   113,   114,   115,    -1,
      -1,   118,   119,   120,    -1,   122,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
       3,   111,    -1,   113,   114,   115,     9,    -1,   118,   119,
     120,    -1,   122,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
     103,   104,   105,   106,    -1,   108,   109,    16,   111,    -1,
     113,   114,   115,    -1,    -1,   118,   119,   120,    -1,   122,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    37,    -1,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    82,    -1,    84,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    42,   104,    -1,    -1,   107,   108,
     109,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,
     119,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    74,    -1,    -1,    -1,
     142,   143,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,   142,   143,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    42,    -1,
      -1,    -1,    -1,   143,    -1,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      42,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    74,    -1,    -1,    -1,    -1,   143,
      -1,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,   143,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,    42,    -1,    -1,
      -1,   142,    -1,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    42,    -1,
      -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    74,    -1,    -1,    -1,   142,    -1,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,   142,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,   142,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,    17,    -1,    -1,
      20,   142,    -1,    23,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    36,    -1,    38,    39,
      40,    -1,    -1,    42,    44,    -1,    -1,    -1,    48,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      74,    91,    -1,    -1,    94,    -1,    96,    -1,    98,    -1,
      -1,   101,    -1,   103,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    -1,    -1,   116,    74,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    23,    28,    33,    36,    38,    39,    40,
      44,    48,    60,    81,    91,    94,    96,    98,   101,   103,
     113,   116,   150,   151,   154,   155,   156,   157,   164,   205,
     210,   211,   216,   217,   218,   219,   230,   231,   240,   241,
     242,   243,   244,   248,   249,   254,   255,   256,   259,   261,
     262,   263,   264,   265,   266,   268,   269,   270,   271,   272,
      95,   107,    59,     3,    16,    20,    21,    22,    23,    26,
      31,    35,    48,    51,    52,    53,    54,    55,    65,    66,
      67,    68,    70,    71,    72,    77,    82,    83,    84,    85,
      86,    87,    88,    90,    93,    94,    95,    97,   100,   103,
     104,   105,   106,   108,   109,   111,   113,   114,   115,   118,
     119,   120,   122,   152,   153,    51,    83,    50,    51,    83,
      21,    87,    95,    59,     3,     4,     5,     6,     8,     9,
      10,    14,    22,    25,    35,    43,    47,    54,    55,    57,
      58,    62,    71,    73,    92,   106,   120,   134,   135,   138,
     141,   144,   153,   165,   166,   168,   198,   199,   267,   273,
     274,   277,   278,    10,    29,    52,    59,    77,   100,   152,
     257,    16,    29,    31,    37,    52,    59,    72,    82,    84,
      85,   100,   104,   107,   108,   109,   119,   207,   208,   257,
     111,    95,   153,   169,     0,    45,   140,   284,    63,   153,
     153,   153,   153,   141,   153,   153,   169,   153,   153,   153,
     153,   153,   141,   146,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,     3,    71,
      73,   198,   198,   198,    58,   153,   202,   203,    13,    14,
     147,   275,   276,    50,   143,    18,   153,   167,    42,    74,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,    69,   190,   191,    64,   127,
     127,    98,     9,   152,   153,     5,     6,     8,    78,   153,
     214,   215,   127,   111,     8,   104,   153,    98,    68,   206,
     206,   206,   206,    79,   192,   193,   153,   118,   153,    98,
     143,   198,   281,   282,   283,    96,   101,   155,   153,   206,
      89,    15,    44,   110,     5,     6,     8,    14,   134,   141,
     177,   178,   226,   232,   233,    93,   114,   121,   171,   114,
       8,   141,   142,   198,   200,   201,   153,   198,   198,   200,
      41,   135,   200,   200,   142,   198,   200,   200,   198,   198,
     198,   198,   142,   141,   141,   141,   142,   143,   145,   127,
       8,   198,   276,   141,   169,   166,   198,   153,   198,   198,
     198,   198,   198,   198,   198,   278,   198,   198,   198,   198,
     198,   198,   198,   198,   198,   198,     5,    78,   138,   198,
     214,   214,   127,   127,    52,   134,     8,    46,    78,   112,
     134,   153,   177,   212,   213,    65,   104,   206,   104,     8,
     153,   194,   195,    14,    30,   104,   209,    68,   121,   250,
     251,   153,   245,   246,   274,   153,   167,    27,   143,   279,
     280,   141,   220,    32,    32,    95,     5,     6,     8,   142,
     177,   179,   234,   143,   235,    25,    47,    61,   105,   260,
       8,     4,    25,    35,    43,    45,    54,    57,    62,    70,
     120,   141,   153,   172,   173,   174,   175,   176,   274,     8,
      96,   157,    49,   142,   143,   142,   142,   153,   142,   142,
     142,   142,   142,   142,   142,   143,   142,   143,   143,   142,
     198,   198,   203,   153,   177,   204,   148,   148,   161,   170,
     171,   143,    78,   141,     5,   212,     9,   215,    67,   206,
     206,   127,   143,    99,     5,    99,     8,   153,   252,   253,
     127,   143,   171,   127,   281,    80,   186,   187,   283,    57,
     153,   221,   222,   117,   153,   153,   153,   142,   143,   142,
     143,   226,   233,   236,   237,   142,   102,   102,   141,   141,
     141,   141,   141,   141,   141,   141,   172,   124,    24,    58,
      64,   127,   128,   129,   130,   131,   132,   138,   142,   153,
     201,   142,   198,   198,   198,   127,    96,   164,    53,   180,
       5,   179,   127,    88,    90,    97,   258,     5,     8,   141,
     153,   195,   127,   123,   141,   177,   178,   246,   192,   177,
     178,    27,   190,   142,   143,   141,   223,   224,    25,    26,
      47,    62,    66,    75,    76,   105,   247,   177,     8,    18,
     238,   143,     8,     8,   153,   274,   135,   274,   142,   142,
     274,     8,   142,   142,   172,   177,   178,     9,   141,    78,
     138,     8,   177,   178,     8,   177,   178,   177,   178,   177,
     178,   177,   178,   177,   178,    58,   143,   158,    58,   142,
     142,   143,   204,   165,   142,     5,   181,   122,   184,   185,
     142,   141,    34,   115,    88,   153,   196,   197,   141,     8,
     253,   142,   179,    86,   176,   188,   189,   221,   141,   225,
     226,    79,   143,   227,    69,   153,   239,   226,   237,   142,
     142,   142,   142,   142,   124,   124,   179,    78,     9,   141,
       5,    57,   153,   159,   160,   142,   153,   273,   274,   141,
      50,    80,   162,    27,    53,    56,   183,   179,   127,   142,
     143,     8,   142,   141,    19,    39,   143,   142,   143,     3,
     228,   229,   224,   177,   178,   177,   178,   142,   179,   143,
     142,   200,    27,    69,   163,   176,   182,    80,   174,   186,
     142,   177,   197,   142,   142,   189,   226,   127,   143,   142,
     160,   142,   188,     5,   143,    27,   190,     8,   229,   143,
     143,   176,   188,   192,   141,     5,   200,   142,   142
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

  case 177:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 178:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 179:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 181:

    {
			pParser->AddHaving();
		;}
    break;

  case 184:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 187:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 188:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 190:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 192:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 193:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 196:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 197:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 203:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 204:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 205:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 206:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 209:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 210:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 212:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 213:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 218:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 219:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 242:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 243:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 244:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
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

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 251:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 252:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 254:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 255:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 260:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 261:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 268:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 269:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 270:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 271:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 272:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 273:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 275:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 276:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 277:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 278:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 279:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 280:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 281:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 285:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 286:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 287:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 288:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 289:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 290:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 291:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 292:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 293:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 294:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 295:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 298:

    { yyval.m_iValue = 1; ;}
    break;

  case 299:

    { yyval.m_iValue = 0; ;}
    break;

  case 300:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 308:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 309:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 310:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 313:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 314:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 315:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 320:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 321:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 324:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 325:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 326:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 327:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 328:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 329:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 330:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 331:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 336:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 337:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 338:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 339:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 340:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 342:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 343:

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

  case 344:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 347:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 349:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 354:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 357:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 358:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 359:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 362:

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

  case 363:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 364:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 365:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
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

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 369:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 370:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 371:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 372:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 373:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 374:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 375:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 376:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 377:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 378:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 379:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 380:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 387:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 388:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 389:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 397:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 398:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 399:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 400:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 401:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 402:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 403:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 404:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 405:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 406:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		;}
    break;

  case 407:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 410:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 411:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 412:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 413:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 414:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 417:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 419:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 420:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 421:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 422:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 423:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 424:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 425:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 429:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 431:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 434:

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

