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
     TOK_PLUGIN = 335,
     TOK_PLUGINS = 336,
     TOK_PROFILE = 337,
     TOK_RAND = 338,
     TOK_RAMCHUNK = 339,
     TOK_READ = 340,
     TOK_REPEATABLE = 341,
     TOK_REPLACE = 342,
     TOK_RETURNS = 343,
     TOK_ROLLBACK = 344,
     TOK_RTINDEX = 345,
     TOK_SELECT = 346,
     TOK_SERIALIZABLE = 347,
     TOK_SET = 348,
     TOK_SESSION = 349,
     TOK_SHOW = 350,
     TOK_SONAME = 351,
     TOK_START = 352,
     TOK_STATUS = 353,
     TOK_STRING = 354,
     TOK_SUM = 355,
     TOK_TABLE = 356,
     TOK_TABLES = 357,
     TOK_THREADS = 358,
     TOK_TO = 359,
     TOK_TRANSACTION = 360,
     TOK_TRUE = 361,
     TOK_TRUNCATE = 362,
     TOK_TYPE = 363,
     TOK_UNCOMMITTED = 364,
     TOK_UPDATE = 365,
     TOK_VALUES = 366,
     TOK_VARIABLES = 367,
     TOK_WARNINGS = 368,
     TOK_WEIGHT = 369,
     TOK_WHERE = 370,
     TOK_WITHIN = 371,
     TOK_OR = 372,
     TOK_AND = 373,
     TOK_NE = 374,
     TOK_GTE = 375,
     TOK_LTE = 376,
     TOK_NOT = 377,
     TOK_NEG = 378
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
#define TOK_PLUGIN 335
#define TOK_PLUGINS 336
#define TOK_PROFILE 337
#define TOK_RAND 338
#define TOK_RAMCHUNK 339
#define TOK_READ 340
#define TOK_REPEATABLE 341
#define TOK_REPLACE 342
#define TOK_RETURNS 343
#define TOK_ROLLBACK 344
#define TOK_RTINDEX 345
#define TOK_SELECT 346
#define TOK_SERIALIZABLE 347
#define TOK_SET 348
#define TOK_SESSION 349
#define TOK_SHOW 350
#define TOK_SONAME 351
#define TOK_START 352
#define TOK_STATUS 353
#define TOK_STRING 354
#define TOK_SUM 355
#define TOK_TABLE 356
#define TOK_TABLES 357
#define TOK_THREADS 358
#define TOK_TO 359
#define TOK_TRANSACTION 360
#define TOK_TRUE 361
#define TOK_TRUNCATE 362
#define TOK_TYPE 363
#define TOK_UNCOMMITTED 364
#define TOK_UPDATE 365
#define TOK_VALUES 366
#define TOK_VARIABLES 367
#define TOK_WARNINGS 368
#define TOK_WEIGHT 369
#define TOK_WHERE 370
#define TOK_WITHIN 371
#define TOK_OR 372
#define TOK_AND 373
#define TOK_NE 374
#define TOK_GTE 375
#define TOK_LTE 376
#define TOK_NOT 377
#define TOK_NEG 378




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
#define YYFINAL  184
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4148

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  143
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  126
/* YYNRULES -- Number of rules. */
#define YYNRULES  410
/* YYNRULES -- Number of states. */
#define YYNSTATES  731

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   378

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   131,   120,     2,
     135,   136,   129,   127,   137,   128,   140,   130,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   134,
     123,   121,   124,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   141,     2,   142,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   138,   119,   139,     2,     2,     2,     2,
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
     115,   116,   117,   118,   122,   125,   126,   132,   133
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
     160,   162,   166,   168,   170,   172,   181,   183,   193,   194,
     197,   199,   203,   205,   207,   209,   210,   214,   215,   218,
     223,   235,   237,   241,   243,   246,   247,   249,   252,   254,
     259,   264,   269,   274,   279,   284,   288,   294,   296,   300,
     301,   303,   306,   308,   312,   316,   321,   323,   327,   331,
     337,   344,   348,   353,   359,   363,   367,   371,   375,   379,
     381,   387,   393,   399,   403,   407,   411,   415,   419,   423,
     427,   432,   436,   438,   440,   445,   449,   453,   455,   457,
     462,   467,   472,   474,   477,   479,   482,   484,   486,   490,
     491,   496,   497,   499,   501,   505,   506,   509,   510,   512,
     518,   519,   521,   525,   531,   533,   537,   539,   542,   545,
     546,   548,   551,   556,   557,   559,   562,   564,   568,   572,
     576,   582,   589,   593,   595,   599,   603,   605,   607,   609,
     611,   613,   615,   617,   620,   623,   627,   631,   635,   639,
     643,   647,   651,   655,   659,   663,   667,   671,   675,   679,
     683,   687,   691,   695,   699,   701,   703,   705,   709,   714,
     719,   724,   729,   734,   739,   744,   748,   755,   762,   766,
     775,   777,   781,   783,   785,   789,   795,   797,   799,   801,
     803,   806,   807,   810,   812,   815,   818,   822,   824,   826,
     828,   833,   838,   842,   844,   847,   852,   857,   862,   866,
     871,   876,   884,   890,   896,   898,   900,   902,   904,   906,
     908,   912,   914,   916,   918,   920,   922,   924,   926,   928,
     930,   933,   941,   943,   945,   946,   950,   952,   954,   956,
     960,   962,   966,   970,   972,   976,   978,   980,   982,   986,
     989,   990,   993,   995,   999,  1003,  1008,  1015,  1017,  1021,
    1023,  1027,  1029,  1033,  1034,  1037,  1039,  1043,  1047,  1048,
    1050,  1052,  1054,  1058,  1060,  1062,  1066,  1070,  1077,  1079,
    1083,  1087,  1091,  1097,  1102,  1106,  1110,  1112,  1114,  1116,
    1118,  1120,  1122,  1124,  1126,  1134,  1141,  1146,  1152,  1153,
    1155,  1158,  1160,  1164,  1168,  1171,  1175,  1182,  1183,  1185,
    1187,  1190,  1193,  1196,  1198,  1206,  1208,  1210,  1212,  1214,
    1218,  1225,  1229,  1233,  1237,  1239,  1243,  1246,  1250,  1254,
    1262,  1268,  1271,  1273,  1276,  1278,  1280,  1284,  1288,  1292,
    1296
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     144,     0,    -1,   145,    -1,   148,    -1,   148,   134,    -1,
     210,    -1,   222,    -1,   202,    -1,   203,    -1,   208,    -1,
     223,    -1,   232,    -1,   234,    -1,   235,    -1,   236,    -1,
     241,    -1,   246,    -1,   247,    -1,   251,    -1,   253,    -1,
     254,    -1,   255,    -1,   256,    -1,   248,    -1,   257,    -1,
     259,    -1,   260,    -1,   261,    -1,   240,    -1,   262,    -1,
     263,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    29,    -1,    33,    -1,    45,    -1,
      48,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      62,    -1,    64,    -1,    65,    -1,    67,    -1,    68,    -1,
      69,    -1,    79,    -1,    80,    -1,    81,    -1,    82,    -1,
      84,    -1,    83,    -1,    85,    -1,    86,    -1,    88,    -1,
      89,    -1,    90,    -1,    92,    -1,    94,    -1,    97,    -1,
      98,    -1,    99,    -1,   100,    -1,   102,    -1,   107,    -1,
     108,    -1,   109,    -1,   112,    -1,   113,    -1,   114,    -1,
     116,    -1,    63,    -1,   146,    -1,    74,    -1,   105,    -1,
     149,    -1,   148,   134,   149,    -1,   150,    -1,   199,    -1,
     151,    -1,    91,     3,   135,   135,   151,   136,   152,   136,
      -1,   158,    -1,    91,   159,    47,   135,   155,   158,   136,
     156,   157,    -1,    -1,   137,   153,    -1,   154,    -1,   153,
     137,   154,    -1,   147,    -1,     5,    -1,    54,    -1,    -1,
      77,    26,   182,    -1,    -1,    66,     5,    -1,    66,     5,
     137,     5,    -1,    91,   159,    47,   163,   164,   174,   178,
     177,   180,   184,   186,    -1,   160,    -1,   159,   137,   160,
      -1,   129,    -1,   162,   161,    -1,    -1,   147,    -1,    18,
     147,    -1,   192,    -1,    21,   135,   192,   136,    -1,    68,
     135,   192,   136,    -1,    70,   135,   192,   136,    -1,   100,
     135,   192,   136,    -1,    52,   135,   192,   136,    -1,    33,
     135,   129,   136,    -1,    51,   135,   136,    -1,    33,   135,
      39,   147,   136,    -1,   147,    -1,   163,   137,   147,    -1,
      -1,   165,    -1,   115,   166,    -1,   167,    -1,   166,   118,
     166,    -1,   135,   166,   136,    -1,    67,   135,     8,   136,
      -1,   168,    -1,   170,   121,   171,    -1,   170,   122,   171,
      -1,   170,    55,   135,   173,   136,    -1,   170,   132,    55,
     135,   173,   136,    -1,   170,    55,     9,    -1,   170,   132,
      55,     9,    -1,   170,    23,   171,   118,   171,    -1,   170,
     124,   171,    -1,   170,   123,   171,    -1,   170,   125,   171,
      -1,   170,   126,   171,    -1,   170,   121,   172,    -1,   169,
      -1,   170,    23,   172,   118,   172,    -1,   170,    23,   171,
     118,   172,    -1,   170,    23,   172,   118,   171,    -1,   170,
     124,   172,    -1,   170,   123,   172,    -1,   170,   125,   172,
      -1,   170,   126,   172,    -1,   170,   121,     8,    -1,   170,
     122,     8,    -1,   170,    61,    75,    -1,   170,    61,   132,
      75,    -1,   170,   122,   172,    -1,   147,    -1,     4,    -1,
      33,   135,   129,   136,    -1,    51,   135,   136,    -1,   114,
     135,   136,    -1,    54,    -1,   264,    -1,    59,   135,   264,
     136,    -1,    41,   135,   264,   136,    -1,    24,   135,   264,
     136,    -1,     5,    -1,   128,     5,    -1,     6,    -1,   128,
       6,    -1,    14,    -1,   171,    -1,   173,   137,   171,    -1,
      -1,    50,   175,    26,   176,    -1,    -1,     5,    -1,   170,
      -1,   176,   137,   170,    -1,    -1,    53,   168,    -1,    -1,
     179,    -1,   116,    50,    77,    26,   182,    -1,    -1,   181,
      -1,    77,    26,   182,    -1,    77,    26,    83,   135,   136,
      -1,   183,    -1,   182,   137,   183,    -1,   170,    -1,   170,
      19,    -1,   170,    37,    -1,    -1,   185,    -1,    66,     5,
      -1,    66,     5,   137,     5,    -1,    -1,   187,    -1,    76,
     188,    -1,   189,    -1,   188,   137,   189,    -1,   147,   121,
     147,    -1,   147,   121,     5,    -1,   147,   121,   135,   190,
     136,    -1,   147,   121,   147,   135,     8,   136,    -1,   147,
     121,     8,    -1,   191,    -1,   190,   137,   191,    -1,   147,
     121,   171,    -1,   147,    -1,     4,    -1,    54,    -1,     5,
      -1,     6,    -1,    14,    -1,     9,    -1,   128,   192,    -1,
     132,   192,    -1,   192,   127,   192,    -1,   192,   128,   192,
      -1,   192,   129,   192,    -1,   192,   130,   192,    -1,   192,
     123,   192,    -1,   192,   124,   192,    -1,   192,   120,   192,
      -1,   192,   119,   192,    -1,   192,   131,   192,    -1,   192,
      40,   192,    -1,   192,    71,   192,    -1,   192,   126,   192,
      -1,   192,   125,   192,    -1,   192,   121,   192,    -1,   192,
     122,   192,    -1,   192,   118,   192,    -1,   192,   117,   192,
      -1,   135,   192,   136,    -1,   138,   196,   139,    -1,   193,
      -1,   264,    -1,   267,    -1,   264,    61,    75,    -1,   264,
      61,   132,    75,    -1,     3,   135,   194,   136,    -1,    55,
     135,   194,   136,    -1,    59,   135,   194,   136,    -1,    24,
     135,   194,   136,    -1,    44,   135,   194,   136,    -1,    41,
     135,   194,   136,    -1,     3,   135,   136,    -1,    70,   135,
     192,   137,   192,   136,    -1,    68,   135,   192,   137,   192,
     136,    -1,   114,   135,   136,    -1,     3,   135,   192,    46,
     147,    55,   264,   136,    -1,   195,    -1,   194,   137,   195,
      -1,   192,    -1,     8,    -1,   197,   121,   198,    -1,   196,
     137,   197,   121,   198,    -1,   147,    -1,    55,    -1,   171,
      -1,   147,    -1,    95,   201,    -1,    -1,    65,     8,    -1,
     113,    -1,    98,   200,    -1,    69,   200,    -1,    16,    98,
     200,    -1,    82,    -1,    79,    -1,    81,    -1,    16,     8,
      98,   200,    -1,    16,   147,    98,   200,    -1,    56,   147,
      98,    -1,   103,    -1,   103,   171,    -1,    93,   146,   121,
     205,    -1,    93,   146,   121,   204,    -1,    93,   146,   121,
      75,    -1,    93,    74,   206,    -1,    93,    10,   121,   206,
      -1,    93,    28,    93,   206,    -1,    93,    49,     9,   121,
     135,   173,   136,    -1,    93,    49,   146,   121,   204,    -1,
      93,    49,   146,   121,     5,    -1,   147,    -1,     8,    -1,
     106,    -1,    43,    -1,   171,    -1,   207,    -1,   206,   128,
     207,    -1,   147,    -1,    75,    -1,     8,    -1,     5,    -1,
       6,    -1,    31,    -1,    89,    -1,   209,    -1,    22,    -1,
      97,   105,    -1,   211,    60,   147,   212,   111,   215,   219,
      -1,    57,    -1,    87,    -1,    -1,   135,   214,   136,    -1,
     147,    -1,    54,    -1,   213,    -1,   214,   137,   213,    -1,
     216,    -1,   215,   137,   216,    -1,   135,   217,   136,    -1,
     218,    -1,   217,   137,   218,    -1,   171,    -1,   172,    -1,
       8,    -1,   135,   173,   136,    -1,   135,   136,    -1,    -1,
      76,   220,    -1,   221,    -1,   220,   137,   221,    -1,     3,
     121,     8,    -1,    36,    47,   163,   165,    -1,    27,   147,
     135,   224,   227,   136,    -1,   225,    -1,   224,   137,   225,
      -1,   218,    -1,   135,   226,   136,    -1,     8,    -1,   226,
     137,     8,    -1,    -1,   137,   228,    -1,   229,    -1,   228,
     137,   229,    -1,   218,   230,   231,    -1,    -1,    18,    -1,
     147,    -1,    66,    -1,   233,   147,   200,    -1,    38,    -1,
      37,    -1,    95,   102,   200,    -1,    95,    35,   200,    -1,
     110,   163,    93,   237,   165,   186,    -1,   238,    -1,   237,
     137,   238,    -1,   147,   121,   171,    -1,   147,   121,   172,
      -1,   147,   121,   135,   173,   136,    -1,   147,   121,   135,
     136,    -1,   264,   121,   171,    -1,   264,   121,   172,    -1,
      59,    -1,    24,    -1,    44,    -1,    25,    -1,    72,    -1,
      73,    -1,    63,    -1,    99,    -1,    17,   101,   147,    15,
      30,   147,   239,    -1,    17,   101,   147,    42,    30,   147,
      -1,    95,   249,   112,   242,    -1,    95,   249,   112,    65,
       8,    -1,    -1,   243,    -1,   115,   244,    -1,   245,    -1,
     244,   117,   245,    -1,   147,   121,     8,    -1,    95,    29,
      -1,    95,    28,    93,    -1,    93,   249,   105,    62,    64,
     250,    -1,    -1,    49,    -1,    94,    -1,    85,   109,    -1,
      85,    32,    -1,    86,    85,    -1,    92,    -1,    34,    48,
     147,    88,   252,    96,     8,    -1,    58,    -1,    24,    -1,
      44,    -1,    99,    -1,    42,    48,   147,    -1,    20,    56,
     147,   104,    90,   147,    -1,    45,    90,   147,    -1,    45,
      84,   147,    -1,    91,   258,   184,    -1,    10,    -1,    10,
     140,   147,    -1,    91,   192,    -1,   107,    90,   147,    -1,
      78,    56,   147,    -1,    34,    80,   147,   108,     8,    96,
       8,    -1,    42,    80,   147,   108,     8,    -1,   147,   265,
      -1,   266,    -1,   265,   266,    -1,    13,    -1,    14,    -1,
     141,   192,   142,    -1,   141,     8,   142,    -1,   192,   121,
     268,    -1,   268,   121,   192,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   169,   169,   170,   171,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   211,   212,   212,   212,   212,   212,   213,   213,   213,
     213,   214,   214,   214,   214,   214,   215,   215,   215,   215,
     215,   216,   216,   216,   216,   216,   216,   216,   217,   217,
     217,   217,   218,   218,   218,   218,   218,   219,   219,   219,
     219,   219,   220,   220,   220,   220,   221,   225,   225,   225,
     231,   232,   236,   237,   241,   242,   250,   251,   258,   260,
     264,   268,   275,   276,   277,   281,   294,   301,   303,   308,
     317,   333,   334,   338,   339,   342,   344,   345,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   361,   362,   365,
     367,   371,   375,   376,   377,   381,   386,   390,   397,   405,
     413,   422,   427,   432,   437,   442,   447,   452,   457,   462,
     467,   472,   477,   482,   487,   492,   497,   502,   507,   512,
     517,   525,   529,   530,   535,   541,   547,   553,   559,   560,
     561,   562,   566,   567,   578,   579,   580,   584,   590,   596,
     598,   601,   603,   610,   614,   620,   622,   628,   630,   634,
     645,   647,   651,   655,   662,   663,   667,   668,   669,   672,
     674,   678,   683,   690,   692,   696,   700,   701,   705,   710,
     715,   721,   726,   734,   739,   746,   756,   757,   758,   759,
     760,   761,   762,   763,   764,   766,   767,   768,   769,   770,
     771,   772,   773,   774,   775,   776,   777,   778,   779,   780,
     781,   782,   783,   784,   785,   786,   787,   788,   789,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   803,
     807,   808,   812,   813,   817,   818,   822,   823,   827,   828,
     834,   837,   839,   843,   844,   845,   846,   847,   848,   849,
     850,   855,   860,   865,   866,   876,   881,   886,   891,   892,
     893,   897,   902,   907,   915,   916,   920,   921,   922,   934,
     935,   939,   940,   941,   942,   943,   950,   951,   952,   956,
     957,   963,   971,   972,   975,   977,   981,   982,   986,   987,
     991,   992,   996,  1000,  1001,  1005,  1006,  1007,  1008,  1009,
    1012,  1013,  1017,  1018,  1022,  1028,  1038,  1046,  1050,  1057,
    1058,  1065,  1075,  1081,  1083,  1087,  1092,  1096,  1103,  1105,
    1109,  1110,  1116,  1124,  1125,  1131,  1135,  1141,  1149,  1150,
    1154,  1168,  1174,  1178,  1183,  1197,  1208,  1209,  1210,  1211,
    1212,  1213,  1214,  1215,  1219,  1227,  1239,  1243,  1250,  1251,
    1255,  1259,  1260,  1264,  1268,  1275,  1282,  1288,  1289,  1290,
    1294,  1295,  1296,  1297,  1303,  1314,  1315,  1316,  1317,  1322,
    1333,  1345,  1354,  1365,  1373,  1374,  1378,  1388,  1399,  1410,
    1421,  1432,  1435,  1436,  1440,  1441,  1442,  1443,  1447,  1448,
    1452
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
  "TOK_PLAN", "TOK_PLUGIN", "TOK_PLUGINS", "TOK_PROFILE", "TOK_RAND", 
  "TOK_RAMCHUNK", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", 
  "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", 
  "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
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
  "show_what", "set_stmt", "set_global_stmt", "set_string_value", 
  "boolean_value", "set_value", "simple_set_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_ident", "column_list", "insert_rows_list", 
  "insert_row", "insert_vals_list", "insert_val", "opt_insert_options", 
  "insert_options_list", "insert_option", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "show_databases", "update", 
  "update_items_list", "update_item", "alter_col_type", "alter", 
  "show_variables", "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "flush_ramchunk", "select_sysvar", 
  "sysvar_name", "select_dual", "truncate", "optimize_index", 
  "create_plugin", "drop_plugin", "json_field", "subscript", "subkey", 
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
     365,   366,   367,   368,   369,   370,   371,   372,   373,   124,
      38,    61,   374,    60,    62,   375,   376,    43,    45,    42,
      47,    37,   377,   378,    59,    40,    41,    44,   123,   125,
      46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   143,   144,   144,   144,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   147,   147,   147,
     148,   148,   149,   149,   150,   150,   151,   151,   152,   152,
     153,   153,   154,   154,   154,   155,   156,   157,   157,   157,
     158,   159,   159,   160,   160,   161,   161,   161,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   163,   163,   164,
     164,   165,   166,   166,   166,   167,   167,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   169,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   171,   171,   172,   172,   172,   173,   173,   174,
     174,   175,   175,   176,   176,   177,   177,   178,   178,   179,
     180,   180,   181,   181,   182,   182,   183,   183,   183,   184,
     184,   185,   185,   186,   186,   187,   188,   188,   189,   189,
     189,   189,   189,   190,   190,   191,   192,   192,   192,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,   193,
     194,   194,   195,   195,   196,   196,   197,   197,   198,   198,
     199,   200,   200,   201,   201,   201,   201,   201,   201,   201,
     201,   201,   201,   201,   201,   202,   202,   202,   202,   202,
     202,   203,   203,   203,   204,   204,   205,   205,   205,   206,
     206,   207,   207,   207,   207,   207,   208,   208,   208,   209,
     209,   210,   211,   211,   212,   212,   213,   213,   214,   214,
     215,   215,   216,   217,   217,   218,   218,   218,   218,   218,
     219,   219,   220,   220,   221,   222,   223,   224,   224,   225,
     225,   226,   226,   227,   227,   228,   228,   229,   230,   230,
     231,   231,   232,   233,   233,   234,   235,   236,   237,   237,
     238,   238,   238,   238,   238,   238,   239,   239,   239,   239,
     239,   239,   239,   239,   240,   240,   241,   241,   242,   242,
     243,   244,   244,   245,   246,   247,   248,   249,   249,   249,
     250,   250,   250,   250,   251,   252,   252,   252,   252,   253,
     254,   255,   256,   257,   258,   258,   259,   260,   261,   262,
     263,   264,   265,   265,   266,   266,   266,   266,   267,   267,
     268
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
       1,     3,     1,     1,     1,     8,     1,     9,     0,     2,
       1,     3,     1,     1,     1,     0,     3,     0,     2,     4,
      11,     1,     3,     1,     2,     0,     1,     2,     1,     4,
       4,     4,     4,     4,     4,     3,     5,     1,     3,     0,
       1,     2,     1,     3,     3,     4,     1,     3,     3,     5,
       6,     3,     4,     5,     3,     3,     3,     3,     3,     1,
       5,     5,     5,     3,     3,     3,     3,     3,     3,     3,
       4,     3,     1,     1,     4,     3,     3,     1,     1,     4,
       4,     4,     1,     2,     1,     2,     1,     1,     3,     0,
       4,     0,     1,     1,     3,     0,     2,     0,     1,     5,
       0,     1,     3,     5,     1,     3,     1,     2,     2,     0,
       1,     2,     4,     0,     1,     2,     1,     3,     3,     3,
       5,     6,     3,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     1,     3,     4,     4,
       4,     4,     4,     4,     4,     3,     6,     6,     3,     8,
       1,     3,     1,     1,     3,     5,     1,     1,     1,     1,
       2,     0,     2,     1,     2,     2,     3,     1,     1,     1,
       4,     4,     3,     1,     2,     4,     4,     4,     3,     4,
       4,     7,     5,     5,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     7,     1,     1,     0,     3,     1,     1,     1,     3,
       1,     3,     3,     1,     3,     1,     1,     1,     3,     2,
       0,     2,     1,     3,     3,     4,     6,     1,     3,     1,
       3,     1,     3,     0,     2,     1,     3,     3,     0,     1,
       1,     1,     3,     1,     1,     3,     3,     6,     1,     3,
       3,     3,     5,     4,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     7,     6,     4,     5,     0,     1,
       2,     1,     3,     3,     2,     3,     6,     0,     1,     1,
       2,     2,     2,     1,     7,     1,     1,     1,     1,     3,
       6,     3,     3,     3,     1,     3,     2,     3,     3,     7,
       5,     2,     1,     2,     1,     1,     3,     3,     3,     3,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   299,     0,   296,     0,     0,   344,   343,
       0,     0,   302,     0,   303,   297,     0,   377,   377,     0,
       0,     0,     0,     2,     3,    80,    82,    84,    86,    83,
       7,     8,     9,   298,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,    29,    30,     0,
       0,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    76,    46,    47,    48,
      49,    50,    78,    51,    52,    53,    54,    56,    55,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    79,    69,    70,    71,    72,    73,    74,    75,    77,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
     207,   209,   210,   410,   212,   394,   211,    34,     0,    38,
       0,     0,    43,    44,   208,     0,     0,    49,     0,    67,
      74,     0,   103,     0,     0,     0,   206,     0,   101,   105,
     108,   234,   189,   235,   236,     0,     0,     0,    41,     0,
      63,     0,     0,     0,     0,   374,   261,   378,     0,   261,
     268,   269,   267,   379,   261,   261,   273,   263,   260,     0,
     300,     0,   117,     0,     1,     4,     0,   261,     0,     0,
       0,     0,     0,     0,   389,     0,   392,   391,   398,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    31,    49,     0,   213,   214,     0,
     257,   256,     0,     0,   404,   405,     0,   401,   402,     0,
       0,     0,   106,   104,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   393,   190,     0,     0,     0,     0,     0,     0,
     294,   295,   293,   292,   291,   278,   289,     0,     0,     0,
     261,     0,   375,     0,   346,     0,   265,   264,   345,   162,
       0,   274,   368,   397,     0,     0,     0,     0,    81,   304,
     342,     0,     0,     0,   164,   317,   166,     0,     0,   315,
     316,   329,   333,   327,     0,     0,     0,   325,     0,   253,
       0,   245,   252,     0,   250,   395,     0,   252,     0,     0,
       0,     0,     0,   115,     0,     0,     0,     0,     0,     0,
     248,     0,     0,     0,   232,     0,   233,     0,   410,     0,
     403,    95,   119,   102,   108,   107,   224,   225,   231,   230,
     222,   221,   228,   408,   229,   219,   220,   227,   226,   215,
     216,   217,   218,   223,   191,   237,     0,   409,   279,   280,
       0,     0,     0,   285,   287,   277,   286,   284,   288,   276,
     275,     0,   261,   266,   261,   262,   272,   163,     0,     0,
     366,   369,     0,     0,   348,     0,   118,     0,     0,     0,
       0,     0,   165,   331,   319,   167,     0,     0,     0,     0,
     386,   387,   385,   388,     0,     0,   153,     0,    38,     0,
      43,   157,     0,    48,    74,     0,   152,   121,   122,   126,
     139,     0,   158,   400,     0,     0,     0,   239,     0,   109,
     242,     0,   114,   244,   243,   113,   240,   241,   110,     0,
     111,     0,   112,     0,     0,     0,   259,   258,   254,   407,
     406,     0,   169,   120,     0,   238,     0,   283,   282,   290,
       0,   270,   271,   367,     0,   370,   371,     0,     0,   193,
       0,   307,   306,   308,     0,     0,     0,   365,   390,   318,
       0,   330,     0,   338,   328,   334,   335,   326,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    88,
       0,   251,   116,     0,     0,     0,     0,     0,   171,   177,
     192,     0,     0,     0,   383,   376,     0,     0,     0,   350,
     351,   349,     0,   347,   194,   354,   355,   305,     0,     0,
     320,   310,   357,   359,   358,   356,   362,   360,   361,   363,
     364,   168,   332,   339,     0,     0,   384,   399,     0,     0,
       0,     0,   155,     0,     0,   156,   124,   123,     0,     0,
     131,     0,   149,     0,   147,   127,   138,   148,   128,   151,
     135,   144,   134,   143,   136,   145,   137,   146,     0,     0,
       0,     0,   247,   246,   255,     0,     0,   172,     0,     0,
     175,   178,   281,   381,   380,   382,   373,   372,   353,     0,
       0,   195,   196,   309,     0,     0,   313,     0,     0,   301,
     341,   340,   337,   338,   336,   161,   154,   160,   159,   125,
       0,     0,     0,   150,   132,     0,    93,    94,    92,    89,
      90,    85,     0,     0,     0,    97,     0,     0,     0,   180,
     352,     0,     0,   312,     0,     0,   321,   322,   311,   133,
     141,   142,   140,   129,     0,     0,   249,     0,     0,    87,
     173,   170,     0,   176,     0,   189,   181,   199,   202,     0,
     198,   197,   314,     0,     0,   130,    91,   186,    96,   184,
      98,     0,     0,     0,   193,     0,     0,   203,     0,   324,
     323,   187,   188,     0,     0,   174,   179,    56,   182,   100,
       0,   200,     0,     0,   185,    99,     0,   205,   204,   201,
     183
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   109,   146,    24,    25,    26,    27,   600,
     649,   650,   461,   655,   679,    28,   147,   148,   233,   149,
     342,   462,   307,   427,   428,   429,   430,   431,   405,   300,
     406,   529,   608,   681,   659,   610,   611,   685,   686,   698,
     699,   252,   253,   543,   544,   621,   622,   706,   707,   317,
     151,   313,   314,   222,   223,   458,    29,   274,   178,    30,
      31,   379,   380,   265,   266,    32,    33,    34,    35,   398,
     483,   484,   550,   551,   625,   301,   629,   666,   667,    36,
      37,   302,   303,   407,   409,   495,   496,   564,   632,    38,
      39,    40,    41,    42,   393,   394,   560,    43,    44,   390,
     391,   475,   476,    45,    46,    47,   162,   535,    48,   414,
      49,    50,    51,    52,    53,   152,    54,    55,    56,    57,
      58,   153,   227,   228,   154,   155
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -483
static const short yypact[] =
{
    3980,   -47,    33,  -483,  3609,  -483,    12,    49,  -483,  -483,
      22,   -33,  -483,    83,  -483,  -483,   707,  3001,   565,     0,
       1,  3609,   115,  -483,    28,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,   105,  -483,  -483,  -483,  3609,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  3609,
    3609,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
      34,  3609,  3609,  3609,  3609,  3609,  3609,  3609,  3609,    43,
    -483,  -483,  -483,  -483,  -483,    46,  -483,    45,    82,   100,
     127,   166,   187,   189,  -483,   195,   197,   199,   209,   213,
     214,  1523,  -483,  1523,  1523,  3102,    16,   -20,  -483,  3203,
     120,  -483,   128,   148,  -483,    67,   198,   257,  3710,  2687,
     247,   232,   249,  3306,   262,  -483,   292,  -483,  3609,   292,
    -483,  -483,  -483,  -483,   292,   292,    21,  -483,  -483,   246,
    -483,  3609,  -483,    -3,  -483,   108,  3609,   292,    38,   256,
      26,   273,   254,    52,  -483,   255,  -483,  -483,  -483,   843,
    3609,  1523,  1659,    47,  1659,  1659,   228,  1523,  1659,  1659,
    1523,  1523,  1523,   229,   231,   234,   235,  -483,  -483,  3711,
    -483,  -483,   -71,   250,  -483,  -483,  1795,    16,  -483,  2128,
     979,  3609,  -483,  -483,  1523,  1523,  1523,  1523,  1523,  1523,
    1523,  1523,  1523,  1523,  1523,  1523,  1523,  1523,  1523,  1523,
    1523,   367,  -483,  -483,   -51,  1523,  2687,  2687,   252,   253,
    -483,  -483,  -483,  -483,  -483,   248,  -483,  2240,   315,   281,
      35,   283,  -483,   374,  -483,   286,  -483,  -483,  -483,  -483,
     378,  -483,    23,  -483,  3609,  3609,  1115,     3,  -483,   251,
    -483,   357,   359,   298,  -483,  -483,  -483,   223,     7,  -483,
    -483,  -483,   263,  -483,    74,   384,  1915,  -483,   386,   280,
    1251,  -483,  3975,    96,  -483,  -483,  3740,  3990,   119,  3609,
     282,   122,   124,  -483,  3773,   137,   140,   319,   358,  3803,
    -483,  1387,  1523,  1523,  -483,  3102,  -483,  2354,   275,   165,
    -483,  -483,    52,  -483,  3990,  -483,  -483,  -483,  4005,   482,
    4017,  3219,    -2,  -483,    -2,   183,   183,   183,   183,   141,
     141,  -483,  -483,  -483,   289,  -483,   344,    -2,   248,   248,
     295,  2799,  2687,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,   356,   292,  -483,   292,  -483,  -483,  -483,   419,  3609,
    -483,  -483,     9,    53,  -483,   310,  -483,  3407,   322,  3609,
    3609,  3609,  -483,  -483,  -483,  -483,   142,   161,    26,   316,
    -483,  -483,  -483,  -483,   338,   355,  -483,   318,   323,   325,
     326,  -483,   327,   328,   330,  1915,    16,   336,  -483,  -483,
    -483,   143,  -483,  -483,   979,   321,  3609,  -483,  1659,  -483,
    -483,   332,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  1523,
    -483,  1523,  -483,   285,   388,   343,  -483,  -483,  -483,  -483,
    -483,   380,   441,  -483,   464,  -483,    21,  -483,  -483,  -483,
     121,  -483,  -483,  -483,   371,   376,  -483,    57,  3609,   421,
      73,  -483,  -483,  -483,   163,   364,   592,  -483,  -483,  -483,
      21,  -483,   493,    15,  -483,   365,  -483,  -483,   495,   512,
    3609,   394,  3609,   390,  3609,   516,   391,    66,  1915,    73,
       2,   -38,    59,    69,    73,    73,    73,    73,   474,   395,
     476,  -483,  -483,  3835,  3865,  2354,   979,   398,   530,   426,
    -483,   168,   -14,   452,  -483,  -483,   535,  3609,     8,  -483,
    -483,  -483,  3609,  -483,  -483,  -483,  -483,  -483,  3407,    44,
     -40,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  3508,    44,  -483,  -483,    16,   409,
     411,   412,  -483,   413,   414,  -483,  -483,  -483,   433,   434,
    -483,    21,  -483,   479,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,     5,  2900,
     420,  3609,  -483,  -483,  -483,    -6,   480,  -483,   532,   511,
     509,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,   172,
     459,   445,  -483,  -483,    20,   181,  -483,   580,   364,  -483,
    -483,  -483,  -483,   567,  -483,  -483,  -483,  -483,  -483,  -483,
      73,    73,   184,  -483,  -483,    21,  -483,  -483,  -483,   449,
    -483,  -483,   451,  3609,   562,   523,  2468,   513,  2468,   514,
    -483,  2027,  3609,  -483,    44,   471,   461,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,   190,  2900,  -483,  2468,   591,  -483,
    -483,   462,   593,  -483,   594,   128,  -483,  -483,  -483,  3609,
     483,  -483,  -483,   614,   580,  -483,  -483,   156,   486,  -483,
     487,  2468,  2468,  2575,   421,   504,   192,  -483,   618,  -483,
    -483,  -483,  -483,  2468,   622,  -483,   486,   494,   486,  -483,
      21,  -483,  3609,   492,  -483,  -483,   496,  -483,  -483,  -483,
    -483
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -483,  -483,  -483,   -12,    -4,  -483,   446,  -483,   320,  -483,
    -483,   -42,  -483,  -483,  -483,   174,   111,   408,  -483,  -483,
      40,  -483,  -294,  -405,  -483,   -18,  -483,  -482,  -174,  -470,
    -462,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -366,
     -72,   -43,  -483,   -61,  -483,  -483,   -17,  -483,   -74,   329,
    -483,     6,   212,  -483,   331,   131,  -483,  -166,  -483,  -483,
    -483,   299,  -483,    90,   297,  -483,  -483,  -483,  -483,  -483,
     126,  -483,  -483,    25,  -483,  -407,  -483,  -483,   -22,  -483,
    -483,  -483,   267,  -483,  -483,  -483,   112,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,   201,  -483,  -483,  -483,  -483,
    -483,  -483,   139,  -483,  -483,  -483,   662,  -483,  -483,  -483,
    -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,  -483,
    -483,  -278,  -483,   454,  -483,   442
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -411
static const short yytable[] =
{
     110,   493,   281,   276,   531,   161,   395,   540,   277,   278,
     546,   580,   279,   279,   644,   403,   299,   182,   613,   163,
     507,   290,   224,   225,   365,   279,   279,   229,   432,   224,
     225,   279,   294,   563,   295,   187,   627,   582,   234,   579,
     296,   653,   586,   589,   591,   593,   595,   597,   463,   279,
     294,   116,   295,   291,    59,   188,   189,   117,   296,   168,
     111,   183,   279,   294,   279,   294,   335,   584,   336,   235,
     114,   296,   169,   296,   279,   294,   619,   587,   279,   294,
     292,   366,   170,   296,   171,   172,   319,   296,   388,    60,
     284,   181,   112,   378,   583,   614,   113,   628,   410,   479,
     273,   174,   115,   577,   383,   180,   176,   191,   192,   182,
     194,   195,   196,   197,   198,   184,   177,   230,   411,   642,
    -396,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     477,   230,   412,   -65,   285,   280,   280,   581,   389,   118,
     645,   221,   626,   404,   618,   232,   259,   432,   280,   280,
     226,  -329,  -329,   193,   297,   264,   404,   226,   633,   271,
     234,   298,   185,   457,   275,   186,   509,   306,   306,   190,
     670,   672,   297,   413,   680,   711,   320,   283,   199,   624,
     201,   234,   289,   674,   508,   297,   200,   297,   255,   285,
     478,   235,   538,   712,   251,   697,   315,   297,   510,   286,
     395,   297,   576,   287,   511,   234,   532,   533,   318,   254,
     321,   322,   235,   534,   325,   326,   471,   202,   472,   715,
     697,   697,   569,   234,   571,   182,   573,   345,   387,   402,
     432,   697,   437,   438,   299,   203,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   264,   264,   235,   440,   438,   692,   443,   438,
     444,   438,   204,   377,   512,   513,   514,   515,   516,   517,
     248,   249,   250,   446,   438,   518,   447,   438,   489,   490,
     392,   396,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   491,   492,   547,
     548,   205,   426,   539,   612,   490,   545,   460,   660,   490,
     246,   247,   248,   249,   250,   441,   561,   663,   664,   256,
     673,   490,   206,   652,   207,   234,   695,   490,   721,   722,
     208,   221,   209,   456,   210,   578,   716,   718,   585,   588,
     590,   592,   594,   596,   211,   150,   368,   369,   212,   213,
     257,   457,  -379,   267,   268,   272,   235,   273,   282,   234,
     293,   304,   305,   308,   323,   330,   331,   377,   264,   332,
     333,   337,   364,   370,   371,   299,   372,   381,   432,   382,
     432,   384,   385,   387,   386,   474,   397,   399,   401,   400,
     235,   299,   415,   482,   433,   486,   487,   488,   234,   432,
     408,  -410,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   459,   442,   465,
     470,   426,   449,   432,   432,   432,   464,   473,   234,   235,
     466,   480,   520,   485,   498,   432,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   499,   497,   500,   508,   448,   449,   519,   501,   235,
     502,   503,   504,   505,   525,   506,   669,   671,   522,   530,
     217,   526,   218,   219,   392,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     299,   528,   536,   537,   450,   451,   568,   542,   568,   549,
     568,   562,   565,   566,   426,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     567,   456,   234,   570,   574,   451,   572,   575,   312,   598,
     316,   601,   599,   474,   606,   607,   324,   615,   620,   327,
     328,   329,   609,   616,   482,   635,   727,   636,   637,   638,
     639,   640,   641,   235,   643,   339,   651,   654,   656,   344,
     631,   657,   658,   346,   347,   348,   349,   350,   351,   352,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     661,   163,   662,   665,   367,   563,   675,   676,   677,   678,
     682,   684,   693,   164,   165,   648,   700,   568,   694,   701,
     166,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   167,   344,   552,   553,   708,   702,
     703,   168,   709,   713,   714,   720,   723,   725,   729,   726,
     435,   288,   730,   696,   169,   527,   554,   605,   343,   219,
     683,   724,   704,   719,   170,   691,   171,   172,   728,   182,
     521,   555,   426,   668,   426,   556,   604,   690,   620,   173,
     312,   453,   454,   174,   557,   558,   455,   175,   176,   469,
     468,   648,   710,   426,   623,   494,   617,   634,   177,   541,
     179,   340,   353,     0,     0,   705,     0,     0,     0,     0,
       0,   559,     0,     0,     0,     0,     0,   426,   426,   426,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   426,
     119,   120,   121,   122,     0,   123,   124,   125,   705,     0,
       0,   126,     0,    62,     0,     0,     0,    63,   127,    65,
       0,   128,    66,     0,     0,     0,    67,     0,     0,     0,
     129,     0,     0,     0,     0,     0,     0,     0,   130,     0,
       0,   131,    69,     0,     0,    70,    71,    72,   132,   133,
       0,   134,   135,   344,     0,     0,   136,     0,     0,    75,
      76,    77,    78,     0,    79,   137,    81,   138,   523,     0,
     524,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,   139,     0,   100,
       0,     0,   101,     0,   102,   103,   104,     0,     0,   105,
     106,   140,     0,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   141,   142,     0,     0,   143,
       0,     0,   144,     0,     0,   145,   214,   120,   121,   122,
       0,   309,   124,     0,     0,   344,     0,   126,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   128,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,   130,     0,     0,   131,    69,     0,
       0,    70,    71,    72,    73,    74,     0,   134,   135,     0,
       0,     0,   136,     0,     0,    75,    76,    77,    78,     0,
      79,   215,    81,   216,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,   101,     0,
     102,   103,   104,     0,     0,   105,   106,   140,     0,   108,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   141,     0,     0,     0,   143,     0,     0,   310,   311,
       0,   145,   214,   120,   121,   122,     0,   123,   124,     0,
       0,     0,     0,   126,     0,    62,     0,     0,     0,    63,
     127,    65,     0,   128,    66,     0,     0,     0,    67,     0,
       0,     0,   129,     0,     0,     0,     0,     0,     0,     0,
     130,     0,     0,   131,    69,     0,     0,    70,    71,    72,
     132,   133,     0,   134,   135,     0,     0,     0,   136,     0,
       0,    75,    76,    77,    78,     0,    79,   137,    81,   138,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,   139,
       0,   100,     0,     0,   101,     0,   102,   103,   104,     0,
       0,   105,   106,   140,     0,   108,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   141,   142,     0,
       0,   143,     0,     0,   144,     0,     0,   145,   119,   120,
     121,   122,     0,   123,   124,     0,     0,     0,     0,   126,
       0,    62,     0,     0,     0,    63,   127,    65,     0,   128,
      66,     0,     0,     0,    67,     0,     0,     0,   129,     0,
       0,     0,     0,     0,     0,     0,   130,     0,     0,   131,
      69,     0,     0,    70,    71,    72,   132,   133,     0,   134,
     135,     0,     0,     0,   136,     0,     0,    75,    76,    77,
      78,     0,    79,   137,    81,   138,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,   139,     0,   100,     0,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   140,
       0,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,   142,     0,     0,   143,     0,     0,
     144,     0,     0,   145,   214,   120,   121,   122,     0,   123,
     124,     0,     0,     0,     0,   126,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   128,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,   130,     0,     0,   131,    69,     0,     0,    70,
      71,    72,    73,    74,     0,   134,   135,     0,     0,     0,
     136,     0,     0,    75,    76,    77,    78,     0,    79,   215,
      81,   216,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,   434,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,   101,     0,   102,   103,
     104,     0,     0,   105,   106,   140,     0,   108,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,     0,   143,     0,     0,   144,     0,     0,   145,
     214,   120,   121,   122,     0,   309,   124,     0,     0,     0,
       0,   126,     0,    62,     0,     0,     0,    63,    64,    65,
       0,   128,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,   130,     0,
       0,   131,    69,     0,     0,    70,    71,    72,    73,    74,
       0,   134,   135,     0,     0,     0,   136,     0,     0,    75,
      76,    77,    78,     0,    79,   215,    81,   216,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,   101,     0,   102,   103,   104,     0,     0,   105,
     106,   140,     0,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   141,     0,     0,     0,   143,
       0,     0,   144,   311,     0,   145,   214,   120,   121,   122,
       0,   123,   124,     0,     0,     0,     0,   126,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   128,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,   130,     0,     0,   131,    69,     0,
       0,    70,    71,    72,    73,    74,     0,   134,   135,     0,
       0,     0,   136,     0,     0,    75,    76,    77,    78,     0,
      79,   215,    81,   216,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,   101,     0,
     102,   103,   104,     0,     0,   105,   106,   140,     0,   108,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   141,     0,     0,     0,   143,     0,     0,   144,     0,
       0,   145,   214,   120,   121,   122,     0,   309,   124,     0,
       0,     0,     0,   126,     0,    62,     0,     0,     0,    63,
      64,    65,     0,   128,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
     130,     0,     0,   131,    69,     0,     0,    70,    71,    72,
      73,    74,     0,   134,   135,     0,     0,     0,   136,     0,
       0,    75,    76,    77,    78,     0,    79,   215,    81,   216,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,   101,     0,   102,   103,   104,     0,
       0,   105,   106,   140,     0,   108,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   141,     0,     0,
       0,   143,     0,     0,   144,     0,     0,   145,   214,   120,
     121,   122,     0,   338,   124,     0,     0,     0,     0,   126,
       0,    62,     0,     0,     0,    63,    64,    65,     0,   128,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,   130,     0,     0,   131,
      69,     0,     0,    70,    71,    72,    73,    74,     0,   134,
     135,     0,     0,     0,   136,     0,     0,    75,    76,    77,
      78,     0,    79,   215,    81,   216,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   140,
       0,   108,     0,     0,     0,     0,     0,     0,    61,   416,
       0,     0,     0,   141,     0,     0,     0,   143,     0,     0,
     144,    62,     0,   145,     0,    63,    64,    65,     0,   417,
      66,     0,     0,     0,    67,     0,     0,     0,   418,     0,
       0,     0,     0,     0,     0,     0,   419,     0,     0,     0,
      69,     0,     0,    70,    71,    72,   420,    74,     0,   421,
       0,     0,     0,     0,   422,     0,     0,    75,    76,    77,
      78,     0,   423,    80,    81,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   424,
      61,   108,   687,     0,     0,   688,     0,     0,     0,     0,
       0,     0,     0,    62,     0,     0,     0,    63,    64,    65,
     425,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
       0,    61,   101,     0,   102,   103,   104,     0,     0,   105,
     106,   107,     0,   108,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,   689,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,    91,    92,    93,     0,
      94,     0,    95,     0,     0,    96,    97,    98,    99,     0,
     100,     0,     0,   101,     0,   102,   103,   104,     0,     0,
     105,   106,   107,    61,   108,   279,     0,     0,   373,     0,
       0,     0,     0,     0,     0,     0,    62,     0,     0,     0,
      63,    64,    65,   341,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   374,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,     0,     0,    82,   375,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,    97,    98,
      99,     0,   100,     0,     0,   101,   376,   102,   103,   104,
       0,     0,   105,   106,   107,     0,   108,    61,     0,   279,
       0,     0,     0,     0,     0,     0,     0,     0,   280,     0,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,    91,    92,    93,     0,    94,     0,    95,     0,
       0,    96,    97,    98,    99,     0,   100,     0,     0,   101,
       0,   102,   103,   104,     0,     0,   105,   106,   107,     0,
     108,    61,   416,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,     0,    62,     0,     0,     0,    63,    64,
      65,     0,   417,    66,     0,     0,     0,    67,     0,     0,
       0,   418,     0,     0,     0,     0,     0,     0,     0,   419,
       0,     0,     0,    69,     0,     0,    70,    71,    72,   420,
      74,     0,   421,     0,     0,     0,     0,   422,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,    91,    92,    93,     0,
      94,     0,    95,     0,     0,    96,    97,    98,    99,     0,
     100,     0,     0,   101,     0,   102,   103,   104,    61,   416,
     105,   106,   424,     0,   108,     0,     0,     0,     0,     0,
       0,    62,     0,     0,     0,    63,    64,    65,     0,   417,
      66,     0,     0,     0,    67,     0,     0,     0,   418,     0,
       0,     0,     0,     0,     0,     0,   419,     0,     0,     0,
      69,     0,     0,    70,    71,    72,   420,    74,     0,   421,
       0,     0,     0,     0,   422,     0,     0,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,   717,    88,
      89,    90,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   424,
      61,   108,   260,   261,     0,   262,     0,     0,     0,     0,
       0,     0,     0,    62,     0,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,   263,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,   101,     0,   102,   103,   104,     0,     0,   105,
     106,   107,    61,   108,   467,     0,     0,   373,     0,     0,
       0,     0,     0,     0,     0,    62,     0,     0,     0,    63,
      64,    65,     0,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,     0,    61,   101,   646,   102,   103,   104,     0,
       0,   105,   106,   107,     0,   108,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,   647,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,    97,    98,
      99,     0,   100,     0,    61,   101,     0,   102,   103,   104,
       0,   156,   105,   106,   107,     0,   108,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,   157,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
     158,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,   159,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,   160,     0,     0,    96,    97,
      98,    99,     0,   100,     0,    61,     0,     0,   102,   103,
     104,     0,     0,   105,   106,   107,     0,   108,    62,     0,
       0,     0,    63,    64,    65,     0,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,     0,   220,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
      91,    92,    93,     0,    94,     0,    95,     0,     0,    96,
      97,    98,    99,     0,   100,     0,    61,   101,     0,   102,
     103,   104,     0,     0,   105,   106,   107,     0,   108,    62,
       0,   231,     0,    63,    64,    65,     0,     0,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,     0,     0,   234,
       0,     0,     0,     0,     0,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
     235,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,   101,    61,
     102,   103,   104,     0,   269,   105,   106,   107,     0,   108,
       0,     0,    62,     0,     0,     0,    63,    64,    65,     0,
       0,    66,     0,     0,     0,    67,     0,     0,     0,    68,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,    69,     0,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,   270,    98,    99,     0,   100,     0,
      61,   101,     0,   102,   103,   104,     0,     0,   105,   106,
     107,     0,   108,    62,     0,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,   481,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
       0,    61,   101,     0,   102,   103,   104,     0,     0,   105,
     106,   107,     0,   108,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,   630,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,    91,    92,    93,     0,
      94,     0,    95,     0,     0,    96,    97,    98,    99,     0,
     100,     0,    61,   101,     0,   102,   103,   104,     0,     0,
     105,   106,   107,     0,   108,    62,     0,     0,     0,    63,
      64,    65,     0,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,     0,    61,   101,     0,   102,   103,   104,   258,
       0,   105,   106,   107,     0,   108,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,   234,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,     0,    79,    80,    81,
     234,     0,   235,     0,     0,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,    97,    98,
      99,   235,   100,   234,     0,  -378,     0,   102,   103,   104,
       0,     0,   105,   106,   107,     0,   108,     0,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   234,   235,     0,     0,   334,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,     0,     0,   235,   234,   439,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   234,   235,     0,     0,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,     0,   235,     0,     0,   452,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,     0,     0,     0,
       0,   602,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,     1,     0,     0,
       2,   603,     3,     0,     0,     0,     0,     4,     0,     0,
       0,     5,     0,     0,     6,   234,     7,     8,     9,     0,
       0,   436,    10,     0,     0,    11,     0,     0,     0,     0,
     234,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,     0,     0,   234,   235,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   234,    13,     0,
       0,   235,     0,     0,     0,     0,     0,    14,     0,    15,
       0,    16,     0,    17,     0,    18,   235,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    20,   235,     0,
      21,     0,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,     0,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250
};

static const short yycheck[] =
{
       4,   408,   176,   169,   466,    17,   284,   477,   174,   175,
     480,     9,     5,     5,     9,     8,   190,    21,    32,    16,
     425,   187,    13,    14,    75,     5,     5,    47,   306,    13,
      14,     5,     6,    18,     8,    39,    76,    75,    40,   509,
      14,    47,   512,   513,   514,   515,   516,   517,   342,     5,
       6,    84,     8,    15,   101,    59,    60,    90,    14,    56,
      48,    21,     5,     6,     5,     6,   137,     8,   139,    71,
      48,    14,    69,    14,     5,     6,   538,     8,     5,     6,
      42,   132,    79,    14,    81,    82,    39,    14,    65,    56,
      93,    90,    80,   267,   132,   109,    47,   137,    24,   393,
      65,    98,    80,   508,   270,   105,   103,   111,   112,   113,
     114,   115,   116,   117,   118,     0,   113,   137,    44,   581,
       0,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     121,   137,    58,    98,   137,   128,   128,   135,   115,    56,
     135,   145,   549,   136,   136,   149,   158,   425,   128,   128,
     141,   136,   137,   113,   128,   159,   136,   141,   565,   163,
      40,   135,   134,   337,   168,    60,    23,   115,   115,   135,
     640,   641,   128,    99,   656,    19,   129,   181,   135,   135,
     135,    40,   186,   645,   118,   128,   140,   128,   121,   137,
     137,    71,   135,    37,    66,   677,   200,   128,    55,    91,
     478,   128,   136,    95,    61,    40,    85,    86,   202,    61,
     204,   205,    71,    92,   208,   209,   382,   135,   384,   701,
     702,   703,   500,    40,   502,   229,   504,   231,     5,     6,
     508,   713,   136,   137,   408,   135,    71,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   256,   257,    71,   136,   137,   664,   136,   137,
     136,   137,   135,   267,   121,   122,   123,   124,   125,   126,
     129,   130,   131,   136,   137,   132,   136,   137,   136,   137,
     284,   285,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   136,   137,   136,
     137,   135,   306,   477,   136,   137,   480,   142,   136,   137,
     127,   128,   129,   130,   131,   319,   490,   136,   137,   121,
     136,   137,   135,   601,   135,    40,   136,   137,   136,   137,
     135,   335,   135,   337,   135,   509,   702,   703,   512,   513,
     514,   515,   516,   517,   135,    16,   256,   257,   135,   135,
      93,   525,   105,   121,   105,    93,    71,    65,   112,    40,
     104,    88,   108,   108,   136,   136,   135,   371,   372,   135,
     135,   121,     5,   121,   121,   549,   128,    62,   656,    98,
     658,    98,     8,     5,    98,   389,   135,    30,    90,    30,
      71,   565,     8,   397,     8,   399,   400,   401,    40,   677,
     137,   121,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   142,   136,    75,
      64,   425,   137,   701,   702,   703,   137,     8,    40,    71,
     135,   121,   436,   111,    96,   713,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,    96,   136,   135,   118,   136,   137,   136,   135,    71,
     135,   135,   135,   135,   121,   135,   640,   641,   136,     5,
     141,    91,   143,   144,   478,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     664,    50,   121,   117,   136,   137,   500,    76,   502,   135,
     504,     8,   137,     8,   508,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       8,   525,    40,   129,     8,   137,   136,   136,   199,    55,
     201,    55,   137,   537,   136,     5,   207,    85,   542,   210,
     211,   212,   116,     8,   548,   136,   720,   136,   136,   136,
     136,   118,   118,    71,    75,   226,   136,    77,    26,   230,
     564,    50,    53,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     121,    16,   137,     3,   255,    18,   137,   136,    26,    66,
      77,    77,   121,    28,    29,   599,     5,   601,   137,   137,
      35,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,    49,   286,    24,    25,   135,    26,
      26,    56,     8,   137,   137,   121,     8,     5,   136,   135,
     310,   185,   136,   675,    69,   461,    44,   526,   230,   310,
     658,   713,   685,   704,    79,   662,    81,    82,   722,   653,
     438,    59,   656,   628,   658,    63,   525,   661,   662,    94,
     331,   332,   333,    98,    72,    73,   335,   102,   103,   372,
     371,   675,   694,   677,   548,   408,   537,   565,   113,   478,
      18,   227,   240,    -1,    -1,   689,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,   701,   702,   703,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   713,
       3,     4,     5,     6,    -1,     8,     9,    10,   722,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    44,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    54,    55,   434,    -1,    -1,    59,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,   449,    -1,
     451,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,   102,
      -1,    -1,   105,    -1,   107,   108,   109,    -1,    -1,   112,
     113,   114,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,   129,    -1,    -1,   132,
      -1,    -1,   135,    -1,    -1,   138,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,   526,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    54,    55,    -1,
      -1,    -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    98,    99,   100,    -1,   102,    -1,    -1,   105,    -1,
     107,   108,   109,    -1,    -1,   112,   113,   114,    -1,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,    -1,    -1,    -1,   132,    -1,    -1,   135,   136,
      -1,   138,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    44,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,   100,
      -1,   102,    -1,    -1,   105,    -1,   107,   108,   109,    -1,
      -1,   112,   113,   114,    -1,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,    -1,
      -1,   132,    -1,    -1,   135,    -1,    -1,   138,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    44,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,    -1,
     105,    -1,   107,   108,   109,    -1,    -1,   112,   113,   114,
      -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,   129,    -1,    -1,   132,    -1,    -1,
     135,    -1,    -1,   138,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    91,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,    -1,   105,    -1,   107,   108,
     109,    -1,    -1,   112,   113,   114,    -1,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,
      -1,    -1,    -1,   132,    -1,    -1,   135,    -1,    -1,   138,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    44,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,   102,
      -1,    -1,   105,    -1,   107,   108,   109,    -1,    -1,   112,
     113,   114,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,    -1,    -1,    -1,   132,
      -1,    -1,   135,   136,    -1,   138,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    54,    55,    -1,
      -1,    -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    98,    99,   100,    -1,   102,    -1,    -1,   105,    -1,
     107,   108,   109,    -1,    -1,   112,   113,   114,    -1,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,    -1,    -1,    -1,   132,    -1,    -1,   135,    -1,
      -1,   138,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    44,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,   100,
      -1,   102,    -1,    -1,   105,    -1,   107,   108,   109,    -1,
      -1,   112,   113,   114,    -1,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,    -1,    -1,
      -1,   132,    -1,    -1,   135,    -1,    -1,   138,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    44,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,    -1,
     105,    -1,   107,   108,   109,    -1,    -1,   112,   113,   114,
      -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,    -1,    -1,   128,    -1,    -1,    -1,   132,    -1,    -1,
     135,    16,    -1,   138,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,    -1,
     105,    -1,   107,   108,   109,    -1,    -1,   112,   113,   114,
       3,   116,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
     135,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,   102,
      -1,     3,   105,    -1,   107,   108,   109,    -1,    -1,   112,
     113,   114,    -1,   116,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,
     102,    -1,    -1,   105,    -1,   107,   108,   109,    -1,    -1,
     112,   113,   114,     3,   116,     5,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,   135,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,
     100,    -1,   102,    -1,    -1,   105,   106,   107,   108,   109,
      -1,    -1,   112,   113,   114,    -1,   116,     3,    -1,     5,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    88,    89,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    98,    99,   100,    -1,   102,    -1,    -1,   105,
      -1,   107,   108,   109,    -1,    -1,   112,   113,   114,    -1,
     116,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   128,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,
     102,    -1,    -1,   105,    -1,   107,   108,   109,     3,     4,
     112,   113,   114,    -1,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,    -1,
     105,    -1,   107,   108,   109,    -1,    -1,   112,   113,   114,
       3,   116,     5,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,   102,
      -1,    -1,   105,    -1,   107,   108,   109,    -1,    -1,   112,
     113,   114,     3,   116,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,   100,
      -1,   102,    -1,     3,   105,     5,   107,   108,   109,    -1,
      -1,   112,   113,   114,    -1,   116,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,
     100,    -1,   102,    -1,     3,   105,    -1,   107,   108,   109,
      -1,    10,   112,   113,   114,    -1,   116,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,     3,    -1,    -1,   107,   108,
     109,    -1,    -1,   112,   113,   114,    -1,   116,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      98,    99,   100,    -1,   102,    -1,     3,   105,    -1,   107,
     108,   109,    -1,    -1,   112,   113,   114,    -1,   116,    16,
      -1,    18,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      71,    88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    98,    99,   100,    -1,   102,    -1,    -1,   105,     3,
     107,   108,   109,    -1,     8,   112,   113,   114,    -1,   116,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,
       3,   105,    -1,   107,   108,   109,    -1,    -1,   112,   113,
     114,    -1,   116,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,   102,
      -1,     3,   105,    -1,   107,   108,   109,    -1,    -1,   112,
     113,   114,    -1,   116,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,
     102,    -1,     3,   105,    -1,   107,   108,   109,    -1,    -1,
     112,   113,   114,    -1,   116,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,   100,
      -1,   102,    -1,     3,   105,    -1,   107,   108,   109,     9,
      -1,   112,   113,   114,    -1,   116,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      40,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,
     100,    71,   102,    40,    -1,   105,    -1,   107,   108,   109,
      -1,    -1,   112,   113,   114,    -1,   116,    -1,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,    40,    71,    -1,    -1,   136,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,    -1,    -1,    71,    40,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,    40,    71,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,    -1,    71,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,    17,    -1,    -1,
      20,   136,    22,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    31,    -1,    -1,    34,    40,    36,    37,    38,    -1,
      -1,    46,    42,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    78,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    87,    -1,    89,
      -1,    91,    -1,    93,    -1,    95,    71,    97,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,    71,    -1,
     110,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,    -1,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    78,    87,    89,    91,    93,    95,    97,
     107,   110,   144,   145,   148,   149,   150,   151,   158,   199,
     202,   203,   208,   209,   210,   211,   222,   223,   232,   233,
     234,   235,   236,   240,   241,   246,   247,   248,   251,   253,
     254,   255,   256,   257,   259,   260,   261,   262,   263,   101,
      56,     3,    16,    20,    21,    22,    25,    29,    33,    45,
      48,    49,    50,    51,    52,    62,    63,    64,    65,    67,
      68,    69,    74,    79,    80,    81,    82,    83,    84,    85,
      86,    88,    89,    90,    92,    94,    97,    98,    99,   100,
     102,   105,   107,   108,   109,   112,   113,   114,   116,   146,
     147,    48,    80,    47,    48,    80,    84,    90,    56,     3,
       4,     5,     6,     8,     9,    10,    14,    21,    24,    33,
      41,    44,    51,    52,    54,    55,    59,    68,    70,   100,
     114,   128,   129,   132,   135,   138,   147,   159,   160,   162,
     192,   193,   258,   264,   267,   268,    10,    28,    49,    74,
      94,   146,   249,    16,    28,    29,    35,    49,    56,    69,
      79,    81,    82,    94,    98,   102,   103,   113,   201,   249,
     105,    90,   147,   163,     0,   134,    60,   147,   147,   147,
     135,   147,   147,   163,   147,   147,   147,   147,   147,   135,
     140,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,     3,    68,    70,   192,   192,   192,
      55,   147,   196,   197,    13,    14,   141,   265,   266,    47,
     137,    18,   147,   161,    40,    71,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,    66,   184,   185,    61,   121,   121,    93,     9,   146,
       5,     6,     8,    75,   147,   206,   207,   121,   105,     8,
      98,   147,    93,    65,   200,   147,   200,   200,   200,     5,
     128,   171,   112,   147,    93,   137,    91,    95,   149,   147,
     200,    15,    42,   104,     6,     8,    14,   128,   135,   171,
     172,   218,   224,   225,    88,   108,   115,   165,   108,     8,
     135,   136,   192,   194,   195,   147,   192,   192,   194,    39,
     129,   194,   194,   136,   192,   194,   194,   192,   192,   192,
     136,   135,   135,   135,   136,   137,   139,   121,     8,   192,
     266,   135,   163,   160,   192,   147,   192,   192,   192,   192,
     192,   192,   192,   268,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,     5,    75,   132,   192,   206,   206,
     121,   121,   128,     8,    43,    75,   106,   147,   171,   204,
     205,    62,    98,   200,    98,     8,    98,     5,    65,   115,
     242,   243,   147,   237,   238,   264,   147,   135,   212,    30,
      30,    90,     6,     8,   136,   171,   173,   226,   137,   227,
      24,    44,    58,    99,   252,     8,     4,    24,    33,    41,
      51,    54,    59,    67,   114,   135,   147,   166,   167,   168,
     169,   170,   264,     8,    91,   151,    46,   136,   137,   136,
     136,   147,   136,   136,   136,   136,   136,   136,   136,   137,
     136,   137,   136,   192,   192,   197,   147,   171,   198,   142,
     142,   155,   164,   165,   137,    75,   135,     5,   204,   207,
      64,   200,   200,     8,   147,   244,   245,   121,   137,   165,
     121,    54,   147,   213,   214,   111,   147,   147,   147,   136,
     137,   136,   137,   218,   225,   228,   229,   136,    96,    96,
     135,   135,   135,   135,   135,   135,   135,   166,   118,    23,
      55,    61,   121,   122,   123,   124,   125,   126,   132,   136,
     147,   195,   136,   192,   192,   121,    91,   158,    50,   174,
       5,   173,    85,    86,    92,   250,   121,   117,   135,   171,
     172,   238,    76,   186,   187,   171,   172,   136,   137,   135,
     215,   216,    24,    25,    44,    59,    63,    72,    73,    99,
     239,   171,     8,    18,   230,   137,     8,     8,   147,   264,
     129,   264,   136,   264,     8,   136,   136,   166,   171,   172,
       9,   135,    75,   132,     8,   171,   172,     8,   171,   172,
     171,   172,   171,   172,   171,   172,   171,   172,    55,   137,
     152,    55,   136,   136,   198,   159,   136,     5,   175,   116,
     178,   179,   136,    32,   109,    85,     8,   245,   136,   173,
     147,   188,   189,   213,   135,   217,   218,    76,   137,   219,
      66,   147,   231,   218,   229,   136,   136,   136,   136,   136,
     118,   118,   173,    75,     9,   135,     5,    54,   147,   153,
     154,   136,   264,    47,    77,   156,    26,    50,    53,   177,
     136,   121,   137,   136,   137,     3,   220,   221,   216,   171,
     172,   171,   172,   136,   173,   137,   136,    26,    66,   157,
     170,   176,    77,   168,    77,   180,   181,     5,     8,   135,
     147,   189,   218,   121,   137,   136,   154,   170,   182,   183,
       5,   137,    26,    26,   184,   147,   190,   191,   135,     8,
     221,    19,    37,   137,   137,   170,   182,    83,   182,   186,
     121,   136,   137,     8,   183,     5,   135,   171,   191,   136,
     136
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

  case 80:

    { pParser->PushQuery(); ;}
    break;

  case 81:

    { pParser->PushQuery(); ;}
    break;

  case 85:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 87:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 90:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 91:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 95:

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

  case 96:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 98:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 99:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 100:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 103:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 106:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 107:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 108:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 109:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 110:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 111:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 112:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 113:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 114:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 115:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 116:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 118:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 125:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 127:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 128:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 129:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 130:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 131:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 132:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 133:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 135:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 136:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 153:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 154:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 155:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 156:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 157:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 162:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 163:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 164:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 165:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 166:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 167:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 168:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 172:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 173:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 174:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 176:

    {
			pParser->AddHaving();
		;}
    break;

  case 179:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 182:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 183:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 187:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 188:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 191:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 192:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 198:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 199:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 200:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 201:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
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

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 270:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 271:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 272:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 273:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 274:

    {
		pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS;
	;}
    break;

  case 275:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 276:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 277:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 278:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 279:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 280:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 281:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 282:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 283:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 286:

    { yyval.m_iValue = 1; ;}
    break;

  case 287:

    { yyval.m_iValue = 0; ;}
    break;

  case 288:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 296:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 297:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 298:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 301:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 302:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 303:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 308:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 309:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 312:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 313:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 314:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 315:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 316:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 317:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 318:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 319:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 324:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 325:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 326:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 327:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 328:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 330:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 331:

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

  case 332:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 335:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 337:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 342:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 345:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 346:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 347:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 350:

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

  case 351:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 352:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 353:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 354:

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

  case 355:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 356:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 357:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 358:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 359:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 360:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 361:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 362:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 363:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 364:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 365:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 366:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 367:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 374:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 375:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 376:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 384:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 385:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 386:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 387:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 388:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 389:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 390:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 391:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 392:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 393:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 396:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 397:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 398:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 399:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 400:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 401:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 403:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 404:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 405:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 406:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 407:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 408:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 409:

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

