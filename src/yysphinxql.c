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
#define YYFINAL  178
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4227

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  140
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  122
/* YYNRULES -- Number of rules. */
#define YYNRULES  397
/* YYNRULES -- Number of states. */
#define YYNSTATES  712

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
     140,   142,   144,   146,   148,   150,   152,   154,   158,   160,
     162,   164,   173,   175,   185,   186,   189,   191,   195,   197,
     199,   201,   202,   206,   207,   210,   215,   227,   229,   233,
     235,   238,   239,   241,   244,   246,   251,   256,   261,   266,
     271,   276,   280,   286,   288,   292,   293,   295,   298,   300,
     304,   308,   313,   315,   319,   323,   329,   336,   340,   345,
     351,   355,   359,   363,   367,   371,   373,   379,   385,   391,
     395,   399,   403,   407,   411,   415,   419,   424,   428,   430,
     432,   437,   441,   445,   447,   449,   454,   459,   464,   466,
     469,   471,   474,   476,   478,   482,   483,   488,   489,   491,
     493,   497,   498,   501,   502,   504,   510,   511,   513,   517,
     523,   525,   529,   531,   534,   537,   538,   540,   543,   548,
     549,   551,   554,   556,   560,   564,   568,   572,   578,   585,
     592,   596,   600,   602,   606,   610,   612,   614,   616,   618,
     620,   622,   624,   627,   630,   634,   638,   642,   646,   650,
     654,   658,   662,   666,   670,   674,   678,   682,   686,   690,
     694,   698,   702,   706,   708,   710,   712,   716,   721,   726,
     731,   736,   741,   746,   751,   755,   762,   769,   773,   782,
     784,   788,   790,   792,   796,   802,   804,   806,   808,   810,
     813,   814,   817,   819,   822,   825,   829,   831,   833,   838,
     843,   847,   852,   857,   862,   866,   871,   876,   884,   890,
     892,   894,   896,   898,   900,   902,   906,   908,   910,   912,
     914,   916,   918,   920,   922,   924,   927,   934,   936,   938,
     939,   943,   945,   949,   951,   955,   959,   961,   965,   967,
     969,   971,   975,   978,   983,   990,   992,   996,   998,  1002,
    1004,  1008,  1009,  1012,  1014,  1018,  1022,  1023,  1025,  1027,
    1029,  1033,  1035,  1037,  1041,  1045,  1052,  1054,  1058,  1062,
    1066,  1072,  1077,  1081,  1085,  1087,  1089,  1091,  1093,  1095,
    1097,  1099,  1101,  1109,  1114,  1120,  1121,  1123,  1126,  1128,
    1132,  1136,  1139,  1143,  1150,  1151,  1153,  1155,  1158,  1161,
    1164,  1166,  1174,  1176,  1178,  1180,  1182,  1188,  1192,  1196,
    1203,  1207,  1211,  1215,  1217,  1221,  1224,  1228,  1232,  1235,
    1237,  1240,  1242,  1244,  1248,  1252,  1256,  1260
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     141,     0,    -1,   142,    -1,   145,    -1,   145,   131,    -1,
     207,    -1,   215,    -1,   199,    -1,   200,    -1,   205,    -1,
     216,    -1,   225,    -1,   227,    -1,   228,    -1,   229,    -1,
     234,    -1,   239,    -1,   240,    -1,   244,    -1,   248,    -1,
     246,    -1,   247,    -1,   249,    -1,   250,    -1,   251,    -1,
     241,    -1,   252,    -1,   254,    -1,   255,    -1,   256,    -1,
     233,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    29,    -1,    33,    -1,    45,    -1,
      48,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      62,    -1,    64,    -1,    65,    -1,    67,    -1,    68,    -1,
      69,    -1,    79,    -1,    80,    -1,    82,    -1,    81,    -1,
      84,    -1,    85,    -1,    87,    -1,    88,    -1,    89,    -1,
      91,    -1,    93,    -1,    96,    -1,    97,    -1,    98,    -1,
      99,    -1,   101,    -1,   105,    -1,   106,    -1,   109,    -1,
     110,    -1,   111,    -1,   113,    -1,   143,    -1,    74,    -1,
     103,    -1,   146,    -1,   145,   131,   146,    -1,   147,    -1,
     196,    -1,   148,    -1,    90,     3,   132,   132,   148,   133,
     149,   133,    -1,   155,    -1,    90,   156,    47,   132,   152,
     155,   133,   153,   154,    -1,    -1,   134,   150,    -1,   151,
      -1,   150,   134,   151,    -1,   144,    -1,     5,    -1,    54,
      -1,    -1,    77,    26,   179,    -1,    -1,    66,     5,    -1,
      66,     5,   134,     5,    -1,    90,   156,    47,   160,   161,
     171,   175,   174,   177,   181,   183,    -1,   157,    -1,   156,
     134,   157,    -1,   126,    -1,   159,   158,    -1,    -1,   144,
      -1,    18,   144,    -1,   189,    -1,    21,   132,   189,   133,
      -1,    68,   132,   189,   133,    -1,    70,   132,   189,   133,
      -1,    99,   132,   189,   133,    -1,    52,   132,   189,   133,
      -1,    33,   132,   126,   133,    -1,    51,   132,   133,    -1,
      33,   132,    39,   144,   133,    -1,   144,    -1,   160,   134,
     144,    -1,    -1,   162,    -1,   112,   163,    -1,   164,    -1,
     163,   115,   163,    -1,   132,   163,   133,    -1,    67,   132,
       8,   133,    -1,   165,    -1,   167,   118,   168,    -1,   167,
     119,   168,    -1,   167,    55,   132,   170,   133,    -1,   167,
     129,    55,   132,   170,   133,    -1,   167,    55,     9,    -1,
     167,   129,    55,     9,    -1,   167,    23,   168,   115,   168,
      -1,   167,   121,   168,    -1,   167,   120,   168,    -1,   167,
     122,   168,    -1,   167,   123,   168,    -1,   167,   118,   169,
      -1,   166,    -1,   167,    23,   169,   115,   169,    -1,   167,
      23,   168,   115,   169,    -1,   167,    23,   169,   115,   168,
      -1,   167,   121,   169,    -1,   167,   120,   169,    -1,   167,
     122,   169,    -1,   167,   123,   169,    -1,   167,   118,     8,
      -1,   167,   119,     8,    -1,   167,    61,    75,    -1,   167,
      61,   129,    75,    -1,   167,   119,   169,    -1,   144,    -1,
       4,    -1,    33,   132,   126,   133,    -1,    51,   132,   133,
      -1,   111,   132,   133,    -1,    54,    -1,   257,    -1,    59,
     132,   257,   133,    -1,    41,   132,   257,   133,    -1,    24,
     132,   257,   133,    -1,     5,    -1,   125,     5,    -1,     6,
      -1,   125,     6,    -1,    14,    -1,   168,    -1,   170,   134,
     168,    -1,    -1,    50,   172,    26,   173,    -1,    -1,     5,
      -1,   167,    -1,   173,   134,   167,    -1,    -1,    53,   165,
      -1,    -1,   176,    -1,   113,    50,    77,    26,   179,    -1,
      -1,   178,    -1,    77,    26,   179,    -1,    77,    26,    81,
     132,   133,    -1,   180,    -1,   179,   134,   180,    -1,   167,
      -1,   167,    19,    -1,   167,    37,    -1,    -1,   182,    -1,
      66,     5,    -1,    66,     5,   134,     5,    -1,    -1,   184,
      -1,    76,   185,    -1,   186,    -1,   185,   134,   186,    -1,
     144,   118,   144,    -1,    83,   118,   144,    -1,   144,   118,
       5,    -1,   144,   118,   132,   187,   133,    -1,   144,   118,
     144,   132,     8,   133,    -1,    83,   118,   144,   132,     8,
     133,    -1,   144,   118,     8,    -1,    83,   118,     8,    -1,
     188,    -1,   187,   134,   188,    -1,   144,   118,   168,    -1,
     144,    -1,     4,    -1,    54,    -1,     5,    -1,     6,    -1,
      14,    -1,     9,    -1,   125,   189,    -1,   129,   189,    -1,
     189,   124,   189,    -1,   189,   125,   189,    -1,   189,   126,
     189,    -1,   189,   127,   189,    -1,   189,   120,   189,    -1,
     189,   121,   189,    -1,   189,   117,   189,    -1,   189,   116,
     189,    -1,   189,   128,   189,    -1,   189,    40,   189,    -1,
     189,    71,   189,    -1,   189,   123,   189,    -1,   189,   122,
     189,    -1,   189,   118,   189,    -1,   189,   119,   189,    -1,
     189,   115,   189,    -1,   189,   114,   189,    -1,   132,   189,
     133,    -1,   135,   193,   136,    -1,   190,    -1,   257,    -1,
     260,    -1,   257,    61,    75,    -1,   257,    61,   129,    75,
      -1,     3,   132,   191,   133,    -1,    55,   132,   191,   133,
      -1,    59,   132,   191,   133,    -1,    24,   132,   191,   133,
      -1,    44,   132,   191,   133,    -1,    41,   132,   191,   133,
      -1,     3,   132,   133,    -1,    70,   132,   189,   134,   189,
     133,    -1,    68,   132,   189,   134,   189,   133,    -1,   111,
     132,   133,    -1,     3,   132,   189,    46,   144,    55,   257,
     133,    -1,   192,    -1,   191,   134,   192,    -1,   189,    -1,
       8,    -1,   194,   118,   195,    -1,   193,   134,   194,   118,
     195,    -1,   144,    -1,    55,    -1,   168,    -1,   144,    -1,
      94,   198,    -1,    -1,    65,     8,    -1,   110,    -1,    97,
     197,    -1,    69,   197,    -1,    16,    97,   197,    -1,    80,
      -1,    79,    -1,    16,     8,    97,   197,    -1,    16,   144,
      97,   197,    -1,    56,   144,    97,    -1,    92,   143,   118,
     202,    -1,    92,   143,   118,   201,    -1,    92,   143,   118,
      75,    -1,    92,    74,   203,    -1,    92,    10,   118,   203,
      -1,    92,    28,    92,   203,    -1,    92,    49,     9,   118,
     132,   170,   133,    -1,    92,    49,   143,   118,   201,    -1,
     144,    -1,     8,    -1,   104,    -1,    43,    -1,   168,    -1,
     204,    -1,   203,   125,   204,    -1,   144,    -1,    75,    -1,
       8,    -1,     5,    -1,     6,    -1,    31,    -1,    88,    -1,
     206,    -1,    22,    -1,    96,   103,    -1,   208,    60,   144,
     209,   108,   211,    -1,    57,    -1,    86,    -1,    -1,   132,
     210,   133,    -1,   144,    -1,   210,   134,   144,    -1,   212,
      -1,   211,   134,   212,    -1,   132,   213,   133,    -1,   214,
      -1,   213,   134,   214,    -1,   168,    -1,   169,    -1,     8,
      -1,   132,   170,   133,    -1,   132,   133,    -1,    36,    47,
     160,   162,    -1,    27,   144,   132,   217,   220,   133,    -1,
     218,    -1,   217,   134,   218,    -1,   214,    -1,   132,   219,
     133,    -1,     8,    -1,   219,   134,     8,    -1,    -1,   134,
     221,    -1,   222,    -1,   221,   134,   222,    -1,   214,   223,
     224,    -1,    -1,    18,    -1,   144,    -1,    66,    -1,   226,
     144,   197,    -1,    38,    -1,    37,    -1,    94,   101,   197,
      -1,    94,    35,   197,    -1,   107,   160,    92,   230,   162,
     183,    -1,   231,    -1,   230,   134,   231,    -1,   144,   118,
     168,    -1,   144,   118,   169,    -1,   144,   118,   132,   170,
     133,    -1,   144,   118,   132,   133,    -1,   257,   118,   168,
      -1,   257,   118,   169,    -1,    59,    -1,    24,    -1,    44,
      -1,    25,    -1,    72,    -1,    73,    -1,    63,    -1,    98,
      -1,    17,   100,   144,    15,    30,   144,   232,    -1,    94,
     242,   109,   235,    -1,    94,   242,   109,    65,     8,    -1,
      -1,   236,    -1,   112,   237,    -1,   238,    -1,   237,   114,
     238,    -1,   144,   118,     8,    -1,    94,    29,    -1,    94,
      28,    92,    -1,    92,   242,   103,    62,    64,   243,    -1,
      -1,    49,    -1,    93,    -1,    84,   106,    -1,    84,    32,
      -1,    85,    84,    -1,    91,    -1,    34,    48,   144,    87,
     245,    95,     8,    -1,    58,    -1,    24,    -1,    44,    -1,
      98,    -1,    34,    83,   144,    95,     8,    -1,    42,    83,
     144,    -1,    42,    48,   144,    -1,    20,    56,   144,   102,
      89,   144,    -1,    45,    89,   144,    -1,    45,    82,   144,
      -1,    90,   253,   181,    -1,    10,    -1,    10,   137,   144,
      -1,    90,   189,    -1,   105,    89,   144,    -1,    78,    56,
     144,    -1,   144,   258,    -1,   259,    -1,   258,   259,    -1,
      13,    -1,    14,    -1,   138,   189,   139,    -1,   138,     8,
     139,    -1,   189,   118,   261,    -1,   261,   118,   189,    -1,
       8,    -1
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
     217,   217,   217,   221,   221,   221,   227,   228,   232,   233,
     237,   238,   246,   247,   254,   256,   260,   264,   271,   272,
     273,   277,   290,   297,   299,   304,   313,   329,   330,   334,
     335,   338,   340,   341,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   357,   358,   361,   363,   367,   371,   372,
     373,   377,   382,   386,   393,   401,   409,   418,   423,   428,
     433,   438,   443,   448,   453,   458,   463,   468,   473,   478,
     483,   488,   493,   498,   503,   508,   513,   521,   525,   526,
     531,   537,   543,   549,   555,   556,   557,   558,   562,   563,
     574,   575,   576,   580,   586,   592,   594,   597,   599,   606,
     610,   616,   618,   624,   626,   630,   641,   643,   647,   651,
     658,   659,   663,   664,   665,   668,   670,   674,   679,   686,
     688,   692,   696,   697,   701,   706,   712,   717,   723,   728,
     733,   738,   746,   751,   758,   768,   769,   770,   771,   772,
     773,   774,   775,   776,   778,   779,   780,   781,   782,   783,
     784,   785,   786,   787,   788,   789,   790,   791,   792,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   805,   806,
     807,   808,   809,   810,   811,   812,   813,   814,   815,   819,
     820,   824,   825,   829,   830,   834,   835,   839,   840,   846,
     849,   851,   855,   856,   857,   858,   859,   860,   861,   866,
     871,   881,   886,   891,   896,   897,   898,   902,   907,   915,
     916,   920,   921,   922,   934,   935,   939,   940,   941,   942,
     943,   950,   951,   952,   956,   957,   963,   971,   972,   975,
     977,   981,   982,   986,   987,   991,   995,   996,  1000,  1001,
    1002,  1003,  1004,  1010,  1020,  1028,  1032,  1039,  1040,  1047,
    1057,  1063,  1065,  1069,  1074,  1078,  1085,  1087,  1091,  1092,
    1098,  1106,  1107,  1113,  1117,  1123,  1131,  1132,  1136,  1150,
    1156,  1160,  1165,  1179,  1190,  1191,  1192,  1193,  1194,  1195,
    1196,  1197,  1201,  1214,  1218,  1225,  1226,  1230,  1234,  1235,
    1239,  1243,  1250,  1257,  1263,  1264,  1265,  1269,  1270,  1271,
    1272,  1278,  1289,  1290,  1291,  1292,  1296,  1306,  1315,  1326,
    1338,  1347,  1358,  1366,  1367,  1371,  1381,  1392,  1403,  1406,
    1407,  1411,  1412,  1413,  1414,  1418,  1419,  1423
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
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "show_databases", "update", 
  "update_items_list", "update_item", "alter_col_type", "alter", 
  "show_variables", "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "create_ranker", 
  "drop_ranker", "drop_function", "attach_index", "flush_rtindex", 
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
     143,   143,   143,   144,   144,   144,   145,   145,   146,   146,
     147,   147,   148,   148,   149,   149,   150,   150,   151,   151,
     151,   152,   153,   154,   154,   154,   155,   156,   156,   157,
     157,   158,   158,   158,   159,   159,   159,   159,   159,   159,
     159,   159,   159,   160,   160,   161,   161,   162,   163,   163,
     163,   164,   164,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   166,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   168,   168,
     169,   169,   169,   170,   170,   171,   171,   172,   172,   173,
     173,   174,   174,   175,   175,   176,   177,   177,   178,   178,
     179,   179,   180,   180,   180,   181,   181,   182,   182,   183,
     183,   184,   185,   185,   186,   186,   186,   186,   186,   186,
     186,   186,   187,   187,   188,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   190,   190,
     190,   190,   190,   190,   190,   190,   190,   190,   190,   191,
     191,   192,   192,   193,   193,   194,   194,   195,   195,   196,
     197,   197,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   199,   199,   199,   199,   199,   199,   200,   200,   201,
     201,   202,   202,   202,   203,   203,   204,   204,   204,   204,
     204,   205,   205,   205,   206,   206,   207,   208,   208,   209,
     209,   210,   210,   211,   211,   212,   213,   213,   214,   214,
     214,   214,   214,   215,   216,   217,   217,   218,   218,   219,
     219,   220,   220,   221,   221,   222,   223,   223,   224,   224,
     225,   226,   226,   227,   228,   229,   230,   230,   231,   231,
     231,   231,   231,   231,   232,   232,   232,   232,   232,   232,
     232,   232,   233,   234,   234,   235,   235,   236,   237,   237,
     238,   239,   240,   241,   242,   242,   242,   243,   243,   243,
     243,   244,   245,   245,   245,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   253,   254,   255,   256,   257,   258,
     258,   259,   259,   259,   259,   260,   260,   261
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
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     8,     1,     9,     0,     2,     1,     3,     1,     1,
       1,     0,     3,     0,     2,     4,    11,     1,     3,     1,
       2,     0,     1,     2,     1,     4,     4,     4,     4,     4,
       4,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       3,     4,     1,     3,     3,     5,     6,     3,     4,     5,
       3,     3,     3,     3,     3,     1,     5,     5,     5,     3,
       3,     3,     3,     3,     3,     3,     4,     3,     1,     1,
       4,     3,     3,     1,     1,     4,     4,     4,     1,     2,
       1,     2,     1,     1,     3,     0,     4,     0,     1,     1,
       3,     0,     2,     0,     1,     5,     0,     1,     3,     5,
       1,     3,     1,     2,     2,     0,     1,     2,     4,     0,
       1,     2,     1,     3,     3,     3,     3,     5,     6,     6,
       3,     3,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     3,     4,     4,     4,
       4,     4,     4,     4,     3,     6,     6,     3,     8,     1,
       3,     1,     1,     3,     5,     1,     1,     1,     1,     2,
       0,     2,     1,     2,     2,     3,     1,     1,     4,     4,
       3,     4,     4,     4,     3,     4,     4,     7,     5,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     6,     1,     1,     0,
       3,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     3,     2,     4,     6,     1,     3,     1,     3,     1,
       3,     0,     2,     1,     3,     3,     0,     1,     1,     1,
       3,     1,     1,     3,     3,     6,     1,     3,     3,     3,
       5,     4,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     7,     4,     5,     0,     1,     2,     1,     3,
       3,     2,     3,     6,     0,     1,     1,     2,     2,     2,
       1,     7,     1,     1,     1,     1,     5,     3,     3,     6,
       3,     3,     3,     1,     3,     2,     3,     3,     2,     1,
       2,     1,     1,     3,     3,     3,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   294,     0,   291,     0,     0,   332,   331,
       0,     0,   297,     0,   298,   292,     0,   364,   364,     0,
       0,     0,     0,     2,     3,    76,    78,    80,    82,    79,
       7,     8,     9,   293,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    30,    15,    16,    17,    25,    18,    20,
      21,    19,    22,    23,    24,    26,    27,    28,    29,     0,
       0,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    74,    51,    52,    54,    53,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    75,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    31,   206,   208,   209,   397,
     211,   383,   210,    34,     0,    38,     0,     0,    43,    44,
     207,     0,     0,    49,     0,    65,    71,     0,    99,     0,
       0,     0,   205,     0,    97,   101,   104,   233,   185,   234,
     235,     0,     0,     0,    41,     0,    61,     0,     0,     0,
       0,   361,   260,   365,     0,   260,   267,   266,   366,   260,
     260,   262,   259,     0,   295,     0,   113,     0,     1,     4,
       0,   260,     0,     0,     0,     0,     0,     0,   378,   377,
     381,   380,   387,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    31,    49,
       0,   212,   213,     0,   256,   255,     0,     0,   391,   392,
       0,   388,   389,     0,     0,     0,   102,   100,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   382,   186,     0,     0,
       0,     0,     0,     0,   289,   290,   288,   287,   286,   274,
     284,     0,     0,     0,   260,     0,   362,     0,   334,     0,
     264,   263,   333,   355,   386,     0,     0,     0,     0,    77,
     299,   330,     0,     0,   158,   160,   310,   162,     0,     0,
     308,   309,   317,   321,   315,     0,     0,     0,   313,   252,
       0,   244,   251,     0,   249,   384,     0,   251,     0,     0,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
     247,     0,     0,     0,   231,     0,   232,     0,   397,     0,
     390,    91,   115,    98,   104,   103,   223,   224,   230,   229,
     221,   220,   227,   395,   228,   218,   219,   226,   225,   214,
     215,   216,   217,   222,   187,   236,     0,   396,   275,   276,
       0,     0,     0,   280,   282,   273,   281,     0,   279,   283,
     272,   271,     0,   260,   265,   260,   261,   270,     0,     0,
     353,   356,     0,     0,   336,     0,   114,     0,     0,     0,
       0,   159,   161,   319,   312,   163,     0,     0,     0,     0,
     373,   374,   372,   375,     0,   376,   149,     0,    38,     0,
      43,   153,     0,    48,    71,     0,   148,   117,   118,   122,
     135,     0,   154,     0,     0,     0,   238,     0,   105,   241,
       0,   110,   243,   242,   109,   239,   240,   106,     0,   107,
       0,   108,     0,     0,     0,   258,   257,   253,   394,   393,
       0,   165,   116,     0,   237,     0,   278,   285,     0,   268,
     269,   354,     0,   357,   358,     0,     0,   189,     0,   301,
       0,     0,     0,   379,   311,     0,   318,     0,   326,   316,
     322,   323,   314,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,   250,   112,     0,     0,     0,
       0,     0,   167,   173,   188,     0,     0,     0,   370,   363,
       0,     0,     0,   338,   339,   337,     0,   335,   190,   342,
     343,   300,     0,     0,   296,   303,   345,   347,   346,   344,
     350,   348,   349,   351,   352,   164,   320,   327,     0,     0,
     371,     0,     0,     0,     0,   151,     0,     0,   152,   120,
     119,     0,     0,   127,     0,   145,     0,   143,   123,   134,
     144,   124,   147,   131,   140,   130,   139,   132,   141,   133,
     142,     0,     0,     0,     0,   246,   245,   254,     0,     0,
     168,     0,     0,   171,   174,   277,   368,   367,   369,   360,
     359,   341,     0,     0,     0,   191,   192,   302,     0,     0,
     306,     0,   329,   328,   325,   326,   324,   157,   150,   156,
     155,   121,     0,     0,     0,   146,   128,     0,    89,    90,
      88,    85,    86,    81,     0,     0,     0,    93,     0,     0,
       0,   176,   340,     0,     0,     0,   305,     0,   304,   129,
     137,   138,   136,   125,     0,     0,   248,     0,     0,    83,
     169,   166,     0,   172,     0,   185,   177,   201,   195,   196,
     200,     0,   194,   193,   307,   126,    87,   182,    92,   180,
      94,     0,     0,     0,   189,     0,     0,     0,   202,     0,
     183,   184,     0,     0,   170,   175,    54,   178,    96,     0,
       0,   197,     0,     0,   181,    95,     0,   199,   204,   203,
     198,   179
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   105,   142,    24,    25,    26,    27,   583,
     631,   632,   450,   637,   659,    28,   143,   144,   227,   145,
     332,   451,   298,   417,   418,   419,   420,   421,   395,   291,
     396,   513,   591,   661,   641,   593,   594,   665,   666,   678,
     679,   246,   247,   527,   528,   605,   606,   687,   688,   307,
     147,   303,   304,   216,   217,   447,    29,   268,   172,    30,
      31,   370,   371,   259,   260,    32,    33,    34,    35,   388,
     470,   534,   535,   609,   292,    36,    37,   293,   294,   397,
     399,   480,   481,   548,   614,    38,    39,    40,    41,    42,
     383,   384,   544,    43,    44,   380,   381,   463,   464,    45,
      46,    47,   158,   519,    48,   404,    49,    50,    51,    52,
      53,    54,    55,   148,    56,    57,    58,   149,   221,   222,
     150,   151
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -475
static const short yypact[] =
{
    3973,   -47,    17,  -475,  3590,  -475,    28,    31,  -475,  -475,
      42,    34,  -475,    27,  -475,  -475,   660,  2798,   338,   -14,
      10,  3590,   122,  -475,   -29,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,    70,  -475,  -475,  -475,  3590,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  3590,
    3590,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,     3,  3590,  3590,  3590,
    3590,  3590,  3590,  3590,  3590,    29,  -475,  -475,  -475,  -475,
    -475,    37,  -475,    49,    54,    79,    84,   104,   131,   135,
    -475,   160,   170,   172,   178,   179,   181,  1458,  -475,  1458,
    1458,  2897,    32,   -34,  -475,  2996,    77,  -475,    67,    98,
    -475,   196,   206,   148,  3689,  2595,    64,   207,   224,  3095,
     237,  -475,    73,  -475,  3590,    73,  -475,  -475,  -475,    73,
      73,  -475,  -475,   238,  -475,  3590,  -475,   -42,  -475,    68,
    3590,    73,   333,   247,    18,   263,   258,   -52,  -475,  -475,
    -475,  -475,  -475,   793,  3590,  1458,  1591,   -11,  1591,  1591,
     222,  1458,  1591,  1591,  1458,  1458,  1458,   226,   230,   231,
     232,  -475,  -475,  3712,  -475,  -475,   -66,   251,  -475,  -475,
    1724,    32,  -475,  2051,   926,  3590,  -475,  -475,  1458,  1458,
    1458,  1458,  1458,  1458,  1458,  1458,  1458,  1458,  1458,  1458,
    1458,  1458,  1458,  1458,  1458,   360,  -475,  -475,   -54,  1458,
    2595,  2595,   252,   253,  -475,  -475,  -475,  -475,  -475,   249,
    -475,  2160,   310,   279,     1,   280,  -475,   370,  -475,   284,
    -475,  -475,  -475,   -28,  -475,  3590,  3590,  1059,   199,  -475,
     250,  -475,   354,   299,  -475,  -475,  -475,  -475,   167,     7,
    -475,  -475,  -475,   255,  -475,    30,   382,  1841,  -475,   273,
    1192,  -475,  3968,    51,  -475,  -475,  3732,  3983,    80,  3590,
     259,    89,    91,  -475,  3801,    99,   101,   433,   467,  3821,
    -475,  1325,  1458,  1458,  -475,  2897,  -475,  2271,   254,   375,
    -475,  -475,   -52,  -475,  3983,  -475,  -475,  -475,  3997,  4010,
    4099,  2610,   534,  -475,   534,   149,   149,   149,   149,   117,
     117,  -475,  -475,  -475,   262,  -475,   322,   534,   249,   249,
     266,  3194,  2595,  -475,  -475,  -475,  -475,   395,  -475,  -475,
    -475,  -475,   337,    73,  -475,    73,  -475,  -475,   396,  3590,
    -475,  -475,     6,   -21,  -475,   285,  -475,  3590,   297,  3590,
    3590,  -475,  -475,  -475,  -475,  -475,   115,   118,    18,   281,
    -475,  -475,  -475,  -475,   311,  -475,  -475,   287,   288,   290,
     294,  -475,   295,   296,   298,  1841,    32,   301,  -475,  -475,
    -475,   108,  -475,   926,   300,  3590,  -475,  1591,  -475,  -475,
     303,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  1458,  -475,
    1458,  -475,   499,  3690,   314,  -475,  -475,  -475,  -475,  -475,
     323,   387,  -475,   435,  -475,     4,  -475,  -475,    35,  -475,
    -475,  -475,   320,   329,  -475,    24,  3590,   368,    81,  -475,
     120,   313,   336,  -475,  -475,     4,  -475,   439,    46,  -475,
     315,  -475,  -475,   442,  3590,   325,  3590,   319,  3590,   448,
     324,   -36,  1841,    81,    -5,   -48,    53,    57,    81,    81,
      81,    81,   403,   326,   404,  -475,  -475,  3841,  3861,  2271,
     926,   328,   458,   352,  -475,   136,   -10,   383,  -475,  -475,
     460,  3590,     9,  -475,  -475,  -475,  3293,  -475,  -475,  -475,
    -475,  -475,  3590,    43,   335,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  3392,    43,
    -475,    32,   339,   341,   342,  -475,   343,   344,  -475,  -475,
    -475,   355,   356,  -475,     4,  -475,   406,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,    22,  2694,   345,  3590,  -475,  -475,  -475,     5,   402,
    -475,   457,   436,   432,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,   147,   369,   388,   374,  -475,  -475,    20,   152,
    -475,   313,  -475,  -475,  -475,   491,  -475,  -475,  -475,  -475,
    -475,  -475,    81,    81,   155,  -475,  -475,     4,  -475,  -475,
    -475,   376,  -475,  -475,   378,  3590,   486,   447,  2382,   438,
    2382,   441,  -475,  3491,  1950,  3293,  -475,    43,  -475,  -475,
    -475,  -475,  -475,  -475,   157,  2694,  -475,  2382,   511,  -475,
    -475,   386,   495,  -475,   497,    67,  -475,  -475,   392,  -475,
    -475,  3590,   394,  -475,  -475,  -475,  -475,    75,   393,  -475,
     397,  2382,  2382,  2486,   368,   521,   412,   165,  -475,   524,
    -475,  -475,  2382,   528,  -475,   393,   405,   393,  -475,   401,
       4,  -475,  3590,   408,  -475,  -475,   410,  -475,  -475,  -475,
    -475,  -475
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -475,  -475,  -475,    -7,    -4,  -475,   357,  -475,   235,  -475,
    -475,  -110,  -475,  -475,  -475,    96,    52,   340,  -475,  -475,
      12,  -475,  -298,  -399,  -475,   -77,  -475,  -474,  -181,  -457,
    -450,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -475,  -400,
    -127,   -97,  -475,  -115,  -475,  -475,   -74,  -475,  -130,   102,
    -475,    63,   146,  -475,   271,    66,  -475,  -163,  -475,  -475,
    -475,   215,  -475,    50,   217,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,   -13,  -475,  -397,  -475,  -475,  -475,   201,  -475,
    -475,  -475,    48,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,   111,  -475,  -475,  -475,  -475,  -475,  -475,    82,  -475,
    -475,  -475,   584,  -475,  -475,  -475,  -475,  -475,  -475,  -475,
    -475,  -475,  -475,  -475,  -475,  -475,  -475,  -228,  -475,   385,
    -475,   373
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -398
static const short yytable[] =
{
     106,   478,   270,   290,   563,   515,   271,   272,   524,   284,
     157,   530,   284,   223,   284,   393,   491,   176,   281,   218,
     219,   355,   596,   284,   285,   284,   286,   565,   309,   284,
     285,   626,   287,   177,   452,   181,   562,   378,   287,   569,
     572,   574,   576,   578,   580,   218,   219,   385,   284,   285,
     275,   286,   635,    59,   400,   182,   183,   287,   284,   285,
     297,   567,   284,   285,   547,   570,   267,   287,   325,   422,
     326,   287,   602,    60,   401,   356,   107,  -385,   109,   492,
     369,   566,   276,   114,   379,   467,   284,   285,   402,   174,
     110,   297,   276,   560,   690,   287,   597,   559,   -63,   175,
     224,   374,   179,   185,   186,   176,   188,   189,   190,   191,
     192,   108,   691,   466,   624,   310,   112,   228,   146,   516,
     517,   187,   178,   113,   465,   111,   518,   564,   403,   367,
     180,   493,   367,   245,   367,   184,   610,   215,   267,   224,
     394,   226,   601,   288,   220,   367,   446,   253,   229,   288,
     289,   258,   615,   394,   627,   265,   522,   228,   277,   248,
     269,   193,   278,   494,   660,   650,   652,  -366,   288,   495,
     220,   274,   391,   392,   194,   608,   280,   654,   288,  -317,
    -317,   195,   288,   677,   426,   427,   196,   422,   229,   228,
     305,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   288,   694,   677,   677,
     459,   197,   460,   429,   427,   159,   198,   290,   677,   176,
     229,   335,   432,   427,   433,   427,   496,   497,   498,   499,
     500,   501,   435,   427,   436,   427,   199,   502,   385,   211,
     251,   212,   213,   242,   243,   244,   258,   258,   474,   475,
     674,   476,   477,   531,   532,   164,   552,   368,   554,   308,
     556,   311,   312,   200,   422,   315,   316,   201,   165,   595,
     475,   382,   386,   240,   241,   242,   243,   244,   166,   167,
     642,   475,   695,   697,   523,   646,   647,   529,   653,   475,
     675,   475,   202,   416,   545,   302,   169,   306,   701,   702,
     358,   359,   203,   314,   204,   430,   317,   318,   319,   171,
     205,   206,   561,   207,   249,   568,   571,   573,   575,   577,
     579,   215,   329,   445,   250,   261,   334,   262,   446,   266,
     336,   337,   338,   339,   340,   341,   342,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   273,   282,   283,
     295,   357,   290,   296,   159,   313,   634,   368,   258,   320,
     536,   537,   321,   322,   323,   354,   160,   161,   290,   327,
     360,   361,   372,   162,   362,   462,   373,   375,   376,   334,
     538,   377,   387,   469,   389,   472,   473,   163,   390,   398,
     405,  -397,   431,   448,   164,   539,   453,   454,   455,   540,
     391,   458,   213,   468,   461,   471,   483,   165,   541,   542,
     422,   416,   422,   510,   482,   228,   492,   166,   167,   484,
     485,   504,   486,   302,   442,   443,   487,   488,   489,   422,
     490,   168,   509,   503,   543,   169,   506,   512,   520,   170,
     514,   649,   651,   521,   526,   533,   229,   546,   171,   549,
     550,   553,   555,   422,   422,   422,   557,   558,   581,   584,
     582,   589,   382,   590,   422,   592,   290,   598,   599,   611,
     622,   623,   617,   228,   618,   619,   620,   621,   633,   636,
     551,   625,   551,   638,   551,   640,   639,   643,   416,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   229,   445,   644,   228,   645,   547,
     655,   656,   657,   658,   449,   662,   680,   462,   664,   708,
     681,   682,   604,   683,   685,   334,   689,   692,   607,   699,
     700,   693,   703,   705,   707,   424,   279,   706,   229,   228,
     507,   710,   508,   711,   613,   676,   511,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   588,   663,   333,   704,   437,   438,   684,   698,
     229,   673,   709,   505,   228,   587,   456,   525,   630,   457,
     551,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   444,   616,   648,   479,
     439,   440,   173,   600,     0,   229,   330,   343,     0,     0,
       0,     0,   334,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,     0,     0,
       0,   176,     0,   438,   416,     0,   416,     0,     0,   668,
     672,   604,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   630,     0,   416,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   115,   116,   117,   118,   686,   119,   120,
     121,     0,     0,     0,   122,     0,    62,   416,   416,   416,
      63,   123,    65,     0,   124,    66,     0,     0,   416,    67,
       0,     0,     0,   125,     0,     0,     0,     0,   686,     0,
       0,   126,     0,     0,   127,    69,     0,     0,    70,    71,
      72,   128,   129,     0,   130,   131,     0,     0,     0,   132,
       0,     0,    75,     0,    76,    77,     0,    78,   133,    80,
     134,     0,     0,     0,    81,     0,     0,     0,     0,    82,
      83,    84,    85,     0,    86,    87,     0,    88,    89,    90,
       0,    91,     0,    92,     0,     0,    93,    94,    95,   135,
       0,    97,     0,    98,     0,    99,   100,     0,     0,   101,
     102,   136,     0,   104,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   137,   138,     0,     0,   139,
       0,     0,   140,     0,     0,   141,   208,   116,   117,   118,
       0,   299,   120,     0,     0,     0,     0,   122,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   124,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,   126,     0,     0,   127,    69,     0,
       0,    70,    71,    72,    73,    74,     0,   130,   131,     0,
       0,     0,   132,     0,     0,    75,     0,    76,    77,     0,
      78,   209,    80,   210,     0,     0,     0,    81,     0,     0,
       0,     0,    82,    83,    84,    85,     0,    86,    87,     0,
      88,    89,    90,     0,    91,     0,    92,     0,     0,    93,
      94,    95,    96,     0,    97,     0,    98,     0,    99,   100,
       0,     0,   101,   102,   136,     0,   104,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   137,     0,
       0,     0,   139,     0,     0,   300,   301,     0,   141,   208,
     116,   117,   118,     0,   119,   120,     0,     0,     0,     0,
     122,     0,    62,     0,     0,     0,    63,   123,    65,     0,
     124,    66,     0,     0,     0,    67,     0,     0,     0,   125,
       0,     0,     0,     0,     0,     0,     0,   126,     0,     0,
     127,    69,     0,     0,    70,    71,    72,   128,   129,     0,
     130,   131,     0,     0,     0,   132,     0,     0,    75,     0,
      76,    77,     0,    78,   133,    80,   134,     0,     0,     0,
      81,     0,     0,     0,     0,    82,    83,    84,    85,     0,
      86,    87,     0,    88,    89,    90,     0,    91,     0,    92,
       0,     0,    93,    94,    95,   135,     0,    97,     0,    98,
       0,    99,   100,     0,     0,   101,   102,   136,     0,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   137,   138,     0,     0,   139,     0,     0,   140,     0,
       0,   141,   115,   116,   117,   118,     0,   119,   120,     0,
       0,     0,     0,   122,     0,    62,     0,     0,     0,    63,
     123,    65,     0,   124,    66,     0,     0,     0,    67,     0,
       0,     0,   125,     0,     0,     0,     0,     0,     0,     0,
     126,     0,     0,   127,    69,     0,     0,    70,    71,    72,
     128,   129,     0,   130,   131,     0,     0,     0,   132,     0,
       0,    75,     0,    76,    77,     0,    78,   133,    80,   134,
       0,     0,     0,    81,     0,     0,     0,     0,    82,    83,
      84,    85,     0,    86,    87,     0,    88,    89,    90,     0,
      91,     0,    92,     0,     0,    93,    94,    95,   135,     0,
      97,     0,    98,     0,    99,   100,     0,     0,   101,   102,
     136,     0,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   137,   138,     0,     0,   139,     0,
       0,   140,     0,     0,   141,   208,   116,   117,   118,     0,
     119,   120,     0,     0,     0,     0,   122,     0,    62,     0,
       0,     0,    63,    64,    65,     0,   124,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,   126,     0,     0,   127,    69,     0,     0,
      70,    71,    72,    73,    74,     0,   130,   131,     0,     0,
       0,   132,     0,     0,    75,     0,    76,    77,     0,    78,
     209,    80,   210,     0,     0,     0,    81,     0,     0,     0,
       0,    82,    83,    84,    85,     0,    86,    87,     0,    88,
      89,    90,   423,    91,     0,    92,     0,     0,    93,    94,
      95,    96,     0,    97,     0,    98,     0,    99,   100,     0,
       0,   101,   102,   136,     0,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   137,     0,     0,
       0,   139,     0,     0,   140,     0,     0,   141,   208,   116,
     117,   118,     0,   299,   120,     0,     0,     0,     0,   122,
       0,    62,     0,     0,     0,    63,    64,    65,     0,   124,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,   126,     0,     0,   127,
      69,     0,     0,    70,    71,    72,    73,    74,     0,   130,
     131,     0,     0,     0,   132,     0,     0,    75,     0,    76,
      77,     0,    78,   209,    80,   210,     0,     0,     0,    81,
       0,     0,     0,     0,    82,    83,    84,    85,     0,    86,
      87,     0,    88,    89,    90,     0,    91,     0,    92,     0,
       0,    93,    94,    95,    96,     0,    97,     0,    98,     0,
      99,   100,     0,     0,   101,   102,   136,     0,   104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     137,     0,     0,     0,   139,     0,     0,   140,   301,     0,
     141,   208,   116,   117,   118,     0,   119,   120,     0,     0,
       0,     0,   122,     0,    62,     0,     0,     0,    63,    64,
      65,     0,   124,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,   126,
       0,     0,   127,    69,     0,     0,    70,    71,    72,    73,
      74,     0,   130,   131,     0,     0,     0,   132,     0,     0,
      75,     0,    76,    77,     0,    78,   209,    80,   210,     0,
       0,     0,    81,     0,     0,     0,     0,    82,    83,    84,
      85,     0,    86,    87,     0,    88,    89,    90,     0,    91,
       0,    92,     0,     0,    93,    94,    95,    96,     0,    97,
       0,    98,     0,    99,   100,     0,     0,   101,   102,   136,
       0,   104,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   137,     0,     0,     0,   139,     0,     0,
     140,     0,     0,   141,   208,   116,   117,   118,     0,   299,
     120,     0,     0,     0,     0,   122,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   124,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,   126,     0,     0,   127,    69,     0,     0,    70,
      71,    72,    73,    74,     0,   130,   131,     0,     0,     0,
     132,     0,     0,    75,     0,    76,    77,     0,    78,   209,
      80,   210,     0,     0,     0,    81,     0,     0,     0,     0,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,     0,    91,     0,    92,     0,     0,    93,    94,    95,
      96,     0,    97,     0,    98,     0,    99,   100,     0,     0,
     101,   102,   136,     0,   104,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   137,     0,     0,     0,
     139,     0,     0,   140,     0,     0,   141,   208,   116,   117,
     118,     0,   328,   120,     0,     0,     0,     0,   122,     0,
      62,     0,     0,     0,    63,    64,    65,     0,   124,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,   126,     0,     0,   127,    69,
       0,     0,    70,    71,    72,    73,    74,     0,   130,   131,
       0,     0,     0,   132,     0,     0,    75,     0,    76,    77,
       0,    78,   209,    80,   210,     0,     0,     0,    81,     0,
       0,     0,     0,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,     0,    91,     0,    92,     0,     0,
      93,    94,    95,    96,     0,    97,     0,    98,     0,    99,
     100,     0,     0,   101,   102,   136,     0,   104,     0,     0,
       0,     0,     0,     0,    61,   406,     0,     0,     0,   137,
       0,     0,     0,   139,     0,     0,   140,    62,     0,   141,
       0,    63,    64,    65,     0,   407,    66,     0,     0,     0,
      67,     0,     0,     0,   408,     0,     0,     0,     0,     0,
       0,     0,   409,     0,     0,     0,    69,     0,     0,    70,
      71,    72,   410,    74,     0,   411,     0,     0,     0,     0,
     412,     0,     0,    75,     0,    76,    77,     0,   413,    79,
      80,     0,     0,     0,     0,    81,     0,     0,     0,     0,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,     0,    91,     0,    92,     0,     0,    93,    94,    95,
      96,     0,    97,     0,    98,     0,    99,   100,     0,     0,
     101,   102,   414,    61,   104,   669,     0,     0,   670,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     0,     0,
      63,    64,    65,   415,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,     0,    76,    77,     0,    78,    79,    80,
       0,     0,     0,     0,    81,     0,     0,     0,     0,    82,
      83,    84,    85,     0,    86,    87,     0,    88,    89,    90,
       0,    91,     0,    92,     0,     0,    93,    94,    95,    96,
       0,    97,     0,    98,    61,    99,   100,     0,     0,   101,
     102,   103,     0,   104,     0,     0,     0,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,     0,
      67,     0,   671,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,     0,     0,    81,     0,     0,     0,     0,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,     0,    91,     0,    92,     0,     0,    93,    94,    95,
      96,     0,    97,     0,    98,     0,    99,   100,     0,     0,
     101,   102,   103,    61,   104,   284,     0,     0,   363,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     0,     0,
      63,    64,    65,   331,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   364,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,     0,    76,    77,     0,    78,    79,    80,
       0,     0,     0,     0,    81,   365,     0,     0,     0,    82,
      83,    84,    85,     0,    86,    87,     0,    88,    89,    90,
       0,    91,     0,    92,     0,     0,    93,    94,    95,    96,
       0,    97,     0,    98,   366,    99,   100,     0,     0,   101,
     102,   103,     0,   104,    61,     0,   284,     0,     0,     0,
       0,     0,     0,     0,     0,   367,     0,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,     0,     0,    81,     0,     0,     0,     0,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,     0,    91,     0,    92,     0,     0,    93,    94,    95,
      96,     0,    97,     0,    98,     0,    99,   100,     0,     0,
     101,   102,   103,     0,   104,    61,   406,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   367,     0,    62,     0,
       0,     0,    63,    64,    65,     0,   407,    66,     0,     0,
       0,    67,     0,     0,     0,   408,     0,     0,     0,     0,
       0,     0,     0,   409,     0,     0,     0,    69,     0,     0,
      70,    71,    72,   410,    74,     0,   411,     0,     0,     0,
       0,   412,     0,     0,    75,     0,    76,    77,     0,    78,
      79,    80,     0,     0,     0,     0,    81,     0,     0,     0,
       0,    82,    83,    84,    85,     0,    86,    87,     0,    88,
      89,    90,     0,    91,     0,    92,     0,     0,    93,    94,
      95,    96,     0,    97,     0,    98,     0,    99,   100,    61,
     406,   101,   102,   414,     0,   104,     0,     0,     0,     0,
       0,     0,    62,     0,     0,     0,    63,    64,    65,     0,
     407,    66,     0,     0,     0,    67,     0,     0,     0,   408,
       0,     0,     0,     0,     0,     0,     0,   409,     0,     0,
       0,    69,     0,     0,    70,    71,    72,   410,    74,     0,
     411,     0,     0,     0,     0,   412,     0,     0,    75,     0,
      76,    77,     0,    78,    79,    80,     0,     0,     0,     0,
      81,     0,     0,     0,     0,    82,    83,   696,    85,     0,
      86,    87,     0,    88,    89,    90,     0,    91,     0,    92,
       0,     0,    93,    94,    95,    96,     0,    97,     0,    98,
       0,    99,   100,     0,     0,   101,   102,   414,    61,   104,
     254,   255,     0,   256,     0,     0,     0,     0,     0,     0,
       0,    62,     0,     0,     0,    63,    64,    65,     0,     0,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,     0,
     228,     0,     0,     0,     0,     0,     0,    75,     0,    76,
      77,     0,    78,    79,    80,     0,     0,     0,     0,    81,
     257,     0,     0,     0,    82,    83,    84,    85,     0,    86,
      87,   229,    88,    89,    90,     0,    91,     0,    92,     0,
       0,    93,    94,    95,    96,     0,    97,    61,    98,   628,
      99,   100,     0,     0,   101,   102,   103,     0,   104,     0,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,    69,
       0,     0,    70,    71,    72,    73,    74,     0,   629,     0,
       0,     0,     0,     0,     0,     0,    75,     0,    76,    77,
       0,    78,    79,    80,     0,     0,     0,     0,    81,     0,
       0,     0,     0,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,     0,    91,     0,    92,     0,     0,
      93,    94,    95,    96,     0,    97,     0,    98,     0,    99,
     100,    61,     0,   101,   102,   103,     0,   104,   152,     0,
       0,     0,     0,     0,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,   153,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,   154,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,     0,    76,    77,     0,    78,    79,    80,     0,     0,
       0,     0,   155,     0,     0,     0,     0,    82,    83,    84,
      85,     0,    86,    87,     0,    88,    89,    90,     0,    91,
       0,   156,     0,     0,    93,    94,    95,    96,     0,    97,
      61,     0,     0,    99,   100,     0,     0,   101,   102,   103,
       0,   104,     0,    62,     0,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,     0,   214,     0,     0,     0,     0,     0,     0,    75,
       0,    76,    77,     0,    78,    79,    80,     0,     0,     0,
       0,    81,     0,     0,     0,     0,    82,    83,    84,    85,
       0,    86,    87,     0,    88,    89,    90,     0,    91,     0,
      92,     0,     0,    93,    94,    95,    96,     0,    97,    61,
      98,     0,    99,   100,     0,     0,   101,   102,   103,     0,
     104,     0,    62,     0,   225,     0,    63,    64,    65,     0,
       0,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,     0,
      76,    77,     0,    78,    79,    80,     0,     0,     0,     0,
      81,     0,     0,     0,     0,    82,    83,    84,    85,     0,
      86,    87,     0,    88,    89,    90,     0,    91,     0,    92,
       0,     0,    93,    94,    95,    96,     0,    97,    61,    98,
       0,    99,   100,   263,     0,   101,   102,   103,     0,   104,
       0,    62,     0,     0,     0,    63,    64,    65,     0,     0,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,     0,    76,
      77,     0,    78,    79,    80,     0,     0,     0,     0,    81,
       0,     0,     0,     0,    82,    83,    84,    85,     0,    86,
      87,     0,    88,    89,    90,     0,    91,     0,    92,     0,
       0,    93,   264,    95,    96,     0,    97,    61,    98,     0,
      99,   100,   363,     0,   101,   102,   103,     0,   104,     0,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,     0,    76,    77,
       0,    78,    79,    80,     0,     0,     0,     0,    81,     0,
       0,     0,     0,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,     0,    91,     0,    92,     0,     0,
      93,    94,    95,    96,     0,    97,    61,    98,     0,    99,
     100,     0,     0,   101,   102,   103,     0,   104,     0,    62,
       0,     0,     0,    63,    64,    65,     0,     0,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,     0,    76,    77,     0,
      78,    79,    80,     0,     0,     0,     0,    81,     0,     0,
       0,     0,    82,    83,    84,    85,   603,    86,    87,     0,
      88,    89,    90,     0,    91,     0,    92,     0,     0,    93,
      94,    95,    96,     0,    97,    61,    98,     0,    99,   100,
       0,     0,   101,   102,   103,     0,   104,     0,    62,     0,
       0,     0,    63,    64,    65,     0,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,     0,    76,    77,   612,    78,
      79,    80,     0,     0,     0,     0,    81,     0,     0,     0,
       0,    82,    83,    84,    85,     0,    86,    87,     0,    88,
      89,    90,     0,    91,     0,    92,     0,     0,    93,    94,
      95,    96,     0,    97,    61,    98,     0,    99,   100,   667,
       0,   101,   102,   103,     0,   104,     0,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,     0,     0,    81,     0,     0,     0,     0,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,     0,    91,     0,    92,     0,     0,    93,    94,    95,
      96,     0,    97,    61,    98,     0,    99,   100,     0,     0,
     101,   102,   103,     0,   104,     0,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,     0,    76,    77,     0,    78,    79,    80,
       0,     0,     0,     0,    81,     0,     0,     0,     0,    82,
      83,    84,    85,     0,    86,    87,     0,    88,    89,    90,
       0,    91,     0,    92,     0,     0,    93,    94,    95,    96,
       0,    97,    61,    98,     0,    99,   100,     0,   252,   101,
     102,   103,     0,   104,     0,    62,     0,     0,     0,    63,
      64,    65,     0,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
     228,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   228,    76,    77,     0,    78,    79,    80,     0,
       0,   229,     0,     0,     0,     0,     0,     0,    82,    83,
      84,    85,   228,    86,    87,     0,    88,    89,    90,     0,
      91,     0,    92,   229,     0,    93,    94,    95,    96,     0,
      97,     0,  -365,     0,    99,   100,     0,     0,   101,   102,
     103,     0,   104,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,     0,
       0,     0,     0,     0,   440,     0,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   228,     0,     0,     0,   324,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   228,     0,     0,     0,   428,     0,     0,     0,     0,
       0,     0,   229,     0,     0,     0,     0,     0,     0,     0,
       0,   228,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   229,     0,     0,     0,     0,     0,     0,     0,
       0,   228,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   229,     0,     0,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
       0,     0,   229,     0,   434,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
       0,     0,     0,     0,   441,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
       0,     0,     0,     0,   585,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
       1,     0,     0,     2,   586,     3,     0,     0,     0,     0,
       4,     0,     0,     0,     5,     0,     0,     6,   228,     7,
       8,     9,     0,     0,   425,    10,     0,     0,    11,     0,
       0,     0,     0,   228,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,   228,     0,   229,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     228,    13,     0,     0,   229,     0,     0,     0,     0,    14,
       0,    15,     0,    16,     0,    17,     0,    18,   229,    19,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
      21,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   228,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     229,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244
};

static const short yycheck[] =
{
       4,   398,   165,   184,     9,   455,   169,   170,   465,     5,
      17,   468,     5,    47,     5,     8,   415,    21,   181,    13,
      14,    75,    32,     5,     6,     5,     8,    75,    39,     5,
       6,     9,    14,    21,   332,    39,   493,    65,    14,   496,
     497,   498,   499,   500,   501,    13,    14,   275,     5,     6,
      92,     8,    47,   100,    24,    59,    60,    14,     5,     6,
     112,     8,     5,     6,    18,     8,    65,    14,   134,   297,
     136,    14,   522,    56,    44,   129,    48,     0,    47,   115,
     261,   129,   134,    56,   112,   383,     5,     6,    58,   103,
      48,   112,   134,   492,    19,    14,   106,   133,    97,    89,
     134,   264,   131,   107,   108,   109,   110,   111,   112,   113,
     114,    83,    37,   134,   564,   126,    82,    40,    16,    84,
      85,   109,     0,    89,   118,    83,    91,   132,    98,   125,
      60,    23,   125,    66,   125,   132,   533,   141,    65,   134,
     133,   145,   133,   125,   138,   125,   327,   154,    71,   125,
     132,   155,   549,   133,   132,   159,   132,    40,    90,    61,
     164,   132,    94,    55,   638,   622,   623,   103,   125,    61,
     138,   175,     5,     6,   137,   132,   180,   627,   125,   133,
     134,   132,   125,   657,   133,   134,   132,   415,    71,    40,
     194,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   125,   681,   682,   683,
     373,   132,   375,   133,   134,    16,   132,   398,   692,   223,
      71,   225,   133,   134,   133,   134,   118,   119,   120,   121,
     122,   123,   133,   134,   133,   134,   132,   129,   466,   137,
      92,   139,   140,   126,   127,   128,   250,   251,   133,   134,
     647,   133,   134,   133,   134,    56,   484,   261,   486,   196,
     488,   198,   199,   132,   492,   202,   203,   132,    69,   133,
     134,   275,   276,   124,   125,   126,   127,   128,    79,    80,
     133,   134,   682,   683,   465,   133,   134,   468,   133,   134,
     133,   134,   132,   297,   475,   193,    97,   195,   133,   134,
     250,   251,   132,   201,   132,   309,   204,   205,   206,   110,
     132,   132,   493,   132,   118,   496,   497,   498,   499,   500,
     501,   325,   220,   327,   118,   118,   224,   103,   509,    92,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   109,    15,   102,
      87,   249,   533,    95,    16,   133,   584,   361,   362,   133,
      24,    25,   132,   132,   132,     5,    28,    29,   549,   118,
     118,   118,    62,    35,   125,   379,    97,    97,     8,   277,
      44,    97,   132,   387,    30,   389,   390,    49,    89,   134,
       8,   118,   133,   139,    56,    59,   134,    75,   132,    63,
       5,    64,   300,   118,     8,   108,    95,    69,    72,    73,
     638,   415,   640,    90,   133,    40,   115,    79,    80,   132,
     132,   425,   132,   321,   322,   323,   132,   132,   132,   657,
     132,    93,   118,   133,    98,    97,   133,    50,   118,   101,
       5,   622,   623,   114,    76,   132,    71,     8,   110,   134,
       8,   126,   133,   681,   682,   683,     8,   133,    55,    55,
     134,   133,   466,     5,   692,   113,   647,    84,     8,   134,
     115,   115,   133,    40,   133,   133,   133,   133,   133,    77,
     484,    75,   486,    26,   488,    53,    50,   118,   492,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    71,   509,   118,    40,   134,    18,
     134,   133,    26,    66,   139,    77,     5,   521,    77,   700,
     134,    26,   526,    26,   132,   423,   132,   134,   532,     8,
     118,   134,     8,     5,   133,   300,   179,   132,    71,    40,
     438,   133,   440,   133,   548,   655,   450,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   510,   640,   224,   692,   133,   134,   665,   684,
      71,   645,   702,   427,    40,   509,   361,   466,   582,   362,
     584,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   325,   549,   611,   398,
     133,   134,    18,   521,    -1,    71,   221,   234,    -1,    -1,
      -1,    -1,   510,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,    -1,
      -1,   635,    -1,   134,   638,    -1,   640,    -1,    -1,   643,
     644,   645,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   655,    -1,   657,   120,   121,   122,   123,   124,   125,
     126,   127,   128,     3,     4,     5,     6,   671,     8,     9,
      10,    -1,    -1,    -1,    14,    -1,    16,   681,   682,   683,
      20,    21,    22,    -1,    24,    25,    -1,    -1,   692,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,   702,    -1,
      -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,    59,
      -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    -1,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,    99,
      -1,   101,    -1,   103,    -1,   105,   106,    -1,    -1,   109,
     110,   111,    -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,    -1,   129,
      -1,    -1,   132,    -1,    -1,   135,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    54,    55,    -1,
      -1,    -1,    59,    -1,    -1,    62,    -1,    64,    65,    -1,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    85,    -1,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,
      97,    98,    99,    -1,   101,    -1,   103,    -1,   105,   106,
      -1,    -1,   109,   110,   111,    -1,   113,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,   129,    -1,    -1,   132,   133,    -1,   135,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,    -1,
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
      -1,    62,    -1,    64,    65,    -1,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    85,    -1,    87,    88,    89,    -1,
      91,    -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,
     101,    -1,   103,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,    -1,   129,    -1,
      -1,   132,    -1,    -1,   135,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    54,    55,    -1,    -1,
      -1,    59,    -1,    -1,    62,    -1,    64,    65,    -1,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    85,    -1,    87,
      88,    89,    90,    91,    -1,    93,    -1,    -1,    96,    97,
      98,    99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,
      -1,   109,   110,   111,    -1,   113,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,
      -1,   129,    -1,    -1,   132,    -1,    -1,   135,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    44,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      55,    -1,    -1,    -1,    59,    -1,    -1,    62,    -1,    64,
      65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      85,    -1,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      -1,    96,    97,    98,    99,    -1,   101,    -1,   103,    -1,
     105,   106,    -1,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,    -1,    -1,    -1,   129,    -1,    -1,   132,   133,    -1,
     135,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    44,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,
      62,    -1,    64,    65,    -1,    67,    68,    69,    70,    -1,
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
      59,    -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    -1,   113,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,
     129,    -1,    -1,   132,    -1,    -1,   135,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    44,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    54,    55,
      -1,    -1,    -1,    59,    -1,    -1,    62,    -1,    64,    65,
      -1,    67,    68,    69,    70,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    85,
      -1,    87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,
      96,    97,    98,    99,    -1,   101,    -1,   103,    -1,   105,
     106,    -1,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,    -1,    -1,   125,
      -1,    -1,    -1,   129,    -1,    -1,   132,    16,    -1,   135,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,    -1,
     109,   110,   111,     3,   113,     5,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,   132,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    -1,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,    99,
      -1,   101,    -1,   103,     3,   105,   106,    -1,    -1,   109,
     110,   111,    -1,   113,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,   132,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,    -1,
     109,   110,   111,     3,   113,     5,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,   132,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    -1,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,    99,
      -1,   101,    -1,   103,   104,   105,   106,    -1,    -1,   109,
     110,   111,    -1,   113,     3,    -1,     5,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,    -1,   103,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    -1,   113,     3,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    59,    -1,    -1,    62,    -1,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    85,    -1,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,
      98,    99,    -1,   101,    -1,   103,    -1,   105,   106,     3,
       4,   109,   110,   111,    -1,   113,    -1,    -1,    -1,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    59,    -1,    -1,    62,    -1,
      64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    85,    -1,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    -1,    96,    97,    98,    99,    -1,   101,    -1,   103,
      -1,   105,   106,    -1,    -1,   109,   110,   111,     3,   113,
       5,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      85,    71,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      -1,    96,    97,    98,    99,    -1,   101,     3,   103,     5,
     105,   106,    -1,    -1,   109,   110,   111,    -1,   113,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    85,
      -1,    87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,
      96,    97,    98,    99,    -1,   101,    -1,   103,    -1,   105,
     106,     3,    -1,   109,   110,   111,    -1,   113,    10,    -1,
      -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    28,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    85,    -1,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    -1,    96,    97,    98,    99,    -1,   101,
       3,    -1,    -1,   105,   106,    -1,    -1,   109,   110,   111,
      -1,   113,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      -1,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    85,    -1,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    -1,    96,    97,    98,    99,    -1,   101,     3,
     103,    -1,   105,   106,    -1,    -1,   109,   110,   111,    -1,
     113,    -1,    16,    -1,    18,    -1,    20,    21,    22,    -1,
      -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    85,    -1,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    -1,    96,    97,    98,    99,    -1,   101,     3,   103,
      -1,   105,   106,     8,    -1,   109,   110,   111,    -1,   113,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      85,    -1,    87,    88,    89,    -1,    91,    -1,    93,    -1,
      -1,    96,    97,    98,    99,    -1,   101,     3,   103,    -1,
     105,   106,     8,    -1,   109,   110,   111,    -1,   113,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    85,
      -1,    87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,
      96,    97,    98,    99,    -1,   101,     3,   103,    -1,   105,
     106,    -1,    -1,   109,   110,   111,    -1,   113,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    -1,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    -1,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,
      97,    98,    99,    -1,   101,     3,   103,    -1,   105,   106,
      -1,    -1,   109,   110,   111,    -1,   113,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    -1,    64,    65,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    85,    -1,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,
      98,    99,    -1,   101,     3,   103,    -1,   105,   106,     8,
      -1,   109,   110,   111,    -1,   113,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    -1,    87,    88,
      89,    -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,
      99,    -1,   101,     3,   103,    -1,   105,   106,    -1,    -1,
     109,   110,   111,    -1,   113,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    -1,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    -1,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    -1,    96,    97,    98,    99,
      -1,   101,     3,   103,    -1,   105,   106,    -1,     9,   109,
     110,   111,    -1,   113,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    40,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    40,    84,    85,    -1,    87,    88,    89,    -1,
      91,    -1,    93,    71,    -1,    96,    97,    98,    99,    -1,
     101,    -1,   103,    -1,   105,   106,    -1,    -1,   109,   110,
     111,    -1,   113,    71,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,   134,    -1,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    40,    -1,    -1,    -1,   133,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    40,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    -1,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,    -1,    71,    -1,   133,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,    -1,    -1,    -1,   133,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,    -1,    -1,    -1,   133,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      17,    -1,    -1,    20,   133,    22,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    31,    -1,    -1,    34,    40,    36,
      37,    38,    -1,    -1,    46,    42,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    78,    -1,    -1,    71,    -1,    -1,    -1,    -1,    86,
      -1,    88,    -1,    90,    -1,    92,    -1,    94,    71,    96,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
     107,    71,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    78,    86,    88,    90,    92,    94,    96,
     105,   107,   141,   142,   145,   146,   147,   148,   155,   196,
     199,   200,   205,   206,   207,   208,   215,   216,   225,   226,
     227,   228,   229,   233,   234,   239,   240,   241,   244,   246,
     247,   248,   249,   250,   251,   252,   254,   255,   256,   100,
      56,     3,    16,    20,    21,    22,    25,    29,    33,    45,
      48,    49,    50,    51,    52,    62,    64,    65,    67,    68,
      69,    74,    79,    80,    81,    82,    84,    85,    87,    88,
      89,    91,    93,    96,    97,    98,    99,   101,   103,   105,
     106,   109,   110,   111,   113,   143,   144,    48,    83,    47,
      48,    83,    82,    89,    56,     3,     4,     5,     6,     8,
       9,    10,    14,    21,    24,    33,    41,    44,    51,    52,
      54,    55,    59,    68,    70,    99,   111,   125,   126,   129,
     132,   135,   144,   156,   157,   159,   189,   190,   253,   257,
     260,   261,    10,    28,    49,    74,    93,   143,   242,    16,
      28,    29,    35,    49,    56,    69,    79,    80,    93,    97,
     101,   110,   198,   242,   103,    89,   144,   160,     0,   131,
      60,   144,   144,   144,   132,   144,   144,   160,   144,   144,
     144,   144,   144,   132,   137,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,     3,    68,
      70,   189,   189,   189,    55,   144,   193,   194,    13,    14,
     138,   258,   259,    47,   134,    18,   144,   158,    40,    71,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    66,   181,   182,    61,   118,
     118,    92,     9,   143,     5,     6,     8,    75,   144,   203,
     204,   118,   103,     8,    97,   144,    92,    65,   197,   144,
     197,   197,   197,   109,   144,    92,   134,    90,    94,   146,
     144,   197,    15,   102,     5,     6,     8,    14,   125,   132,
     168,   169,   214,   217,   218,    87,    95,   112,   162,     8,
     132,   133,   189,   191,   192,   144,   189,   189,   191,    39,
     126,   191,   191,   133,   189,   191,   191,   189,   189,   189,
     133,   132,   132,   132,   133,   134,   136,   118,     8,   189,
     259,   132,   160,   157,   189,   144,   189,   189,   189,   189,
     189,   189,   189,   261,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,     5,    75,   129,   189,   203,   203,
     118,   118,   125,     8,    43,    75,   104,   125,   144,   168,
     201,   202,    62,    97,   197,    97,     8,    97,    65,   112,
     235,   236,   144,   230,   231,   257,   144,   132,   209,    30,
      89,     5,     6,     8,   133,   168,   170,   219,   134,   220,
      24,    44,    58,    98,   245,     8,     4,    24,    33,    41,
      51,    54,    59,    67,   111,   132,   144,   163,   164,   165,
     166,   167,   257,    90,   148,    46,   133,   134,   133,   133,
     144,   133,   133,   133,   133,   133,   133,   133,   134,   133,
     134,   133,   189,   189,   194,   144,   168,   195,   139,   139,
     152,   161,   162,   134,    75,   132,   201,   204,    64,   197,
     197,     8,   144,   237,   238,   118,   134,   162,   118,   144,
     210,   108,   144,   144,   133,   134,   133,   134,   214,   218,
     221,   222,   133,    95,   132,   132,   132,   132,   132,   132,
     132,   163,   115,    23,    55,    61,   118,   119,   120,   121,
     122,   123,   129,   133,   144,   192,   133,   189,   189,   118,
      90,   155,    50,   171,     5,   170,    84,    85,    91,   243,
     118,   114,   132,   168,   169,   231,    76,   183,   184,   168,
     169,   133,   134,   132,   211,   212,    24,    25,    44,    59,
      63,    72,    73,    98,   232,   168,     8,    18,   223,   134,
       8,   144,   257,   126,   257,   133,   257,     8,   133,   133,
     163,   168,   169,     9,   132,    75,   129,     8,   168,   169,
       8,   168,   169,   168,   169,   168,   169,   168,   169,   168,
     169,    55,   134,   149,    55,   133,   133,   195,   156,   133,
       5,   172,   113,   175,   176,   133,    32,   106,    84,     8,
     238,   133,   170,    83,   144,   185,   186,   144,   132,   213,
     214,   134,    66,   144,   224,   214,   222,   133,   133,   133,
     133,   133,   115,   115,   170,    75,     9,   132,     5,    54,
     144,   150,   151,   133,   257,    47,    77,   153,    26,    50,
      53,   174,   133,   118,   118,   134,   133,   134,   212,   168,
     169,   168,   169,   133,   170,   134,   133,    26,    66,   154,
     167,   173,    77,   165,    77,   177,   178,     8,   144,     5,
       8,   132,   144,   186,   214,   133,   151,   167,   179,   180,
       5,   134,    26,    26,   181,   132,   144,   187,   188,   132,
      19,    37,   134,   134,   167,   179,    81,   179,   183,     8,
     118,   133,   134,     8,   180,     5,   132,   133,   168,   188,
     133,   133
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

  case 76:

    { pParser->PushQuery(); ;}
    break;

  case 77:

    { pParser->PushQuery(); ;}
    break;

  case 81:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 83:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 86:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 87:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 91:

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

  case 92:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 94:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 95:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 96:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 99:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 102:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 103:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 104:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 105:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 106:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 107:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 108:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 109:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 110:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 111:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 112:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 114:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 121:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 123:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 124:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 125:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 126:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 127:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 128:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 129:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 130:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 131:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 132:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 133:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 135:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 136:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 150:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 151:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 152:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 153:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 158:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 159:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 160:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 161:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 162:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 163:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 164:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 168:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 169:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 170:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 172:

    {
			pParser->AddHaving();
		;}
    break;

  case 175:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 178:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 179:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 181:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 183:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 187:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 188:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 194:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
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
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 198:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 203:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 204:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 206:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 207:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 212:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 213:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 214:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
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

  case 236:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 237:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
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

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 245:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 247:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 248:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 254:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 261:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 262:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 263:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 264:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 265:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 266:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 267:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 268:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 269:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 270:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 271:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 272:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 273:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 275:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 276:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 277:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 278:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 281:

    { yyval.m_iValue = 1; ;}
    break;

  case 282:

    { yyval.m_iValue = 0; ;}
    break;

  case 283:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 291:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 292:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 293:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 296:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 297:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 298:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 301:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 302:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 305:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 306:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 307:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 308:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 309:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 310:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 311:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 312:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 313:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 314:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 315:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 316:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 318:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 319:

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

  case 320:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 323:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 325:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 330:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 333:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 334:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 335:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 338:

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

  case 339:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 340:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 341:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
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

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 345:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 346:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 347:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 348:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 349:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 350:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 351:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 352:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 353:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 354:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 361:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 362:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 363:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 371:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
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

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 376:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[-2] );
			pParser->ToStringUnescape ( tStmt.m_sUdrLib, yyvsp[0] );
		;}
    break;

  case 377:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[0] );
		;}
    break;

  case 378:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 379:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 380:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 381:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 382:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 385:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 386:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 387:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 388:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 390:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 391:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 392:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 393:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 394:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 395:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 396:

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
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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

