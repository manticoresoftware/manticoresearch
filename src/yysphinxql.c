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
     TOK_TO = 358,
     TOK_TRANSACTION = 359,
     TOK_TRUE = 360,
     TOK_TRUNCATE = 361,
     TOK_TYPE = 362,
     TOK_UNCOMMITTED = 363,
     TOK_UPDATE = 364,
     TOK_VALUES = 365,
     TOK_VARIABLES = 366,
     TOK_WARNINGS = 367,
     TOK_WEIGHT = 368,
     TOK_WHERE = 369,
     TOK_WITHIN = 370,
     TOK_OR = 371,
     TOK_AND = 372,
     TOK_NE = 373,
     TOK_GTE = 374,
     TOK_LTE = 375,
     TOK_NOT = 376,
     TOK_NEG = 377
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
#define TOK_TO 358
#define TOK_TRANSACTION 359
#define TOK_TRUE 360
#define TOK_TRUNCATE 361
#define TOK_TYPE 362
#define TOK_UNCOMMITTED 363
#define TOK_UPDATE 364
#define TOK_VALUES 365
#define TOK_VARIABLES 366
#define TOK_WARNINGS 367
#define TOK_WEIGHT 368
#define TOK_WHERE 369
#define TOK_WITHIN 370
#define TOK_OR 371
#define TOK_AND 372
#define TOK_NE 373
#define TOK_GTE 374
#define TOK_LTE 375
#define TOK_NOT 376
#define TOK_NEG 377




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
#define YYFINAL  183
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4124

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  142
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  126
/* YYNRULES -- Number of rules. */
#define YYNRULES  407
/* YYNRULES -- Number of states. */
#define YYNSTATES  726

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   377

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   130,   119,     2,
     134,   135,   128,   126,   136,   127,   139,   129,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   133,
     122,   120,   123,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   140,     2,   141,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   137,   118,   138,     2,     2,     2,     2,
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
     115,   116,   117,   121,   124,   125,   131,   132
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
     828,   833,   838,   842,   847,   852,   857,   861,   866,   871,
     879,   885,   891,   893,   895,   897,   899,   901,   903,   907,
     909,   911,   913,   915,   917,   919,   921,   923,   925,   928,
     936,   938,   940,   941,   945,   947,   949,   951,   955,   957,
     961,   965,   967,   971,   973,   975,   977,   981,   984,   985,
     988,   990,   994,   998,  1003,  1010,  1012,  1016,  1018,  1022,
    1024,  1028,  1029,  1032,  1034,  1038,  1042,  1043,  1045,  1047,
    1049,  1053,  1055,  1057,  1061,  1065,  1072,  1074,  1078,  1082,
    1086,  1092,  1097,  1101,  1105,  1107,  1109,  1111,  1113,  1115,
    1117,  1119,  1121,  1129,  1134,  1140,  1141,  1143,  1146,  1148,
    1152,  1156,  1159,  1163,  1170,  1171,  1173,  1175,  1178,  1181,
    1184,  1186,  1194,  1196,  1198,  1200,  1202,  1206,  1213,  1217,
    1221,  1225,  1227,  1231,  1234,  1238,  1242,  1250,  1256,  1259,
    1261,  1264,  1266,  1268,  1272,  1276,  1280,  1284
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     143,     0,    -1,   144,    -1,   147,    -1,   147,   133,    -1,
     209,    -1,   221,    -1,   201,    -1,   202,    -1,   207,    -1,
     222,    -1,   231,    -1,   233,    -1,   234,    -1,   235,    -1,
     240,    -1,   245,    -1,   246,    -1,   250,    -1,   252,    -1,
     253,    -1,   254,    -1,   255,    -1,   247,    -1,   256,    -1,
     258,    -1,   259,    -1,   260,    -1,   239,    -1,   261,    -1,
     262,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    29,    -1,    33,    -1,    45,    -1,
      48,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      62,    -1,    64,    -1,    65,    -1,    67,    -1,    68,    -1,
      69,    -1,    79,    -1,    80,    -1,    81,    -1,    82,    -1,
      84,    -1,    83,    -1,    85,    -1,    86,    -1,    88,    -1,
      89,    -1,    90,    -1,    92,    -1,    94,    -1,    97,    -1,
      98,    -1,    99,    -1,   100,    -1,   102,    -1,   106,    -1,
     107,    -1,   108,    -1,   111,    -1,   112,    -1,   113,    -1,
     115,    -1,    63,    -1,   145,    -1,    74,    -1,   104,    -1,
     148,    -1,   147,   133,   148,    -1,   149,    -1,   198,    -1,
     150,    -1,    91,     3,   134,   134,   150,   135,   151,   135,
      -1,   157,    -1,    91,   158,    47,   134,   154,   157,   135,
     155,   156,    -1,    -1,   136,   152,    -1,   153,    -1,   152,
     136,   153,    -1,   146,    -1,     5,    -1,    54,    -1,    -1,
      77,    26,   181,    -1,    -1,    66,     5,    -1,    66,     5,
     136,     5,    -1,    91,   158,    47,   162,   163,   173,   177,
     176,   179,   183,   185,    -1,   159,    -1,   158,   136,   159,
      -1,   128,    -1,   161,   160,    -1,    -1,   146,    -1,    18,
     146,    -1,   191,    -1,    21,   134,   191,   135,    -1,    68,
     134,   191,   135,    -1,    70,   134,   191,   135,    -1,   100,
     134,   191,   135,    -1,    52,   134,   191,   135,    -1,    33,
     134,   128,   135,    -1,    51,   134,   135,    -1,    33,   134,
      39,   146,   135,    -1,   146,    -1,   162,   136,   146,    -1,
      -1,   164,    -1,   114,   165,    -1,   166,    -1,   165,   117,
     165,    -1,   134,   165,   135,    -1,    67,   134,     8,   135,
      -1,   167,    -1,   169,   120,   170,    -1,   169,   121,   170,
      -1,   169,    55,   134,   172,   135,    -1,   169,   131,    55,
     134,   172,   135,    -1,   169,    55,     9,    -1,   169,   131,
      55,     9,    -1,   169,    23,   170,   117,   170,    -1,   169,
     123,   170,    -1,   169,   122,   170,    -1,   169,   124,   170,
      -1,   169,   125,   170,    -1,   169,   120,   171,    -1,   168,
      -1,   169,    23,   171,   117,   171,    -1,   169,    23,   170,
     117,   171,    -1,   169,    23,   171,   117,   170,    -1,   169,
     123,   171,    -1,   169,   122,   171,    -1,   169,   124,   171,
      -1,   169,   125,   171,    -1,   169,   120,     8,    -1,   169,
     121,     8,    -1,   169,    61,    75,    -1,   169,    61,   131,
      75,    -1,   169,   121,   171,    -1,   146,    -1,     4,    -1,
      33,   134,   128,   135,    -1,    51,   134,   135,    -1,   113,
     134,   135,    -1,    54,    -1,   263,    -1,    59,   134,   263,
     135,    -1,    41,   134,   263,   135,    -1,    24,   134,   263,
     135,    -1,     5,    -1,   127,     5,    -1,     6,    -1,   127,
       6,    -1,    14,    -1,   170,    -1,   172,   136,   170,    -1,
      -1,    50,   174,    26,   175,    -1,    -1,     5,    -1,   169,
      -1,   175,   136,   169,    -1,    -1,    53,   167,    -1,    -1,
     178,    -1,   115,    50,    77,    26,   181,    -1,    -1,   180,
      -1,    77,    26,   181,    -1,    77,    26,    83,   134,   135,
      -1,   182,    -1,   181,   136,   182,    -1,   169,    -1,   169,
      19,    -1,   169,    37,    -1,    -1,   184,    -1,    66,     5,
      -1,    66,     5,   136,     5,    -1,    -1,   186,    -1,    76,
     187,    -1,   188,    -1,   187,   136,   188,    -1,   146,   120,
     146,    -1,   146,   120,     5,    -1,   146,   120,   134,   189,
     135,    -1,   146,   120,   146,   134,     8,   135,    -1,   146,
     120,     8,    -1,   190,    -1,   189,   136,   190,    -1,   146,
     120,   170,    -1,   146,    -1,     4,    -1,    54,    -1,     5,
      -1,     6,    -1,    14,    -1,     9,    -1,   127,   191,    -1,
     131,   191,    -1,   191,   126,   191,    -1,   191,   127,   191,
      -1,   191,   128,   191,    -1,   191,   129,   191,    -1,   191,
     122,   191,    -1,   191,   123,   191,    -1,   191,   119,   191,
      -1,   191,   118,   191,    -1,   191,   130,   191,    -1,   191,
      40,   191,    -1,   191,    71,   191,    -1,   191,   125,   191,
      -1,   191,   124,   191,    -1,   191,   120,   191,    -1,   191,
     121,   191,    -1,   191,   117,   191,    -1,   191,   116,   191,
      -1,   134,   191,   135,    -1,   137,   195,   138,    -1,   192,
      -1,   263,    -1,   266,    -1,   263,    61,    75,    -1,   263,
      61,   131,    75,    -1,     3,   134,   193,   135,    -1,    55,
     134,   193,   135,    -1,    59,   134,   193,   135,    -1,    24,
     134,   193,   135,    -1,    44,   134,   193,   135,    -1,    41,
     134,   193,   135,    -1,     3,   134,   135,    -1,    70,   134,
     191,   136,   191,   135,    -1,    68,   134,   191,   136,   191,
     135,    -1,   113,   134,   135,    -1,     3,   134,   191,    46,
     146,    55,   263,   135,    -1,   194,    -1,   193,   136,   194,
      -1,   191,    -1,     8,    -1,   196,   120,   197,    -1,   195,
     136,   196,   120,   197,    -1,   146,    -1,    55,    -1,   170,
      -1,   146,    -1,    95,   200,    -1,    -1,    65,     8,    -1,
     112,    -1,    98,   199,    -1,    69,   199,    -1,    16,    98,
     199,    -1,    82,    -1,    79,    -1,    81,    -1,    16,     8,
      98,   199,    -1,    16,   146,    98,   199,    -1,    56,   146,
      98,    -1,    93,   145,   120,   204,    -1,    93,   145,   120,
     203,    -1,    93,   145,   120,    75,    -1,    93,    74,   205,
      -1,    93,    10,   120,   205,    -1,    93,    28,    93,   205,
      -1,    93,    49,     9,   120,   134,   172,   135,    -1,    93,
      49,   145,   120,   203,    -1,    93,    49,   145,   120,     5,
      -1,   146,    -1,     8,    -1,   105,    -1,    43,    -1,   170,
      -1,   206,    -1,   205,   127,   206,    -1,   146,    -1,    75,
      -1,     8,    -1,     5,    -1,     6,    -1,    31,    -1,    89,
      -1,   208,    -1,    22,    -1,    97,   104,    -1,   210,    60,
     146,   211,   110,   214,   218,    -1,    57,    -1,    87,    -1,
      -1,   134,   213,   135,    -1,   146,    -1,    54,    -1,   212,
      -1,   213,   136,   212,    -1,   215,    -1,   214,   136,   215,
      -1,   134,   216,   135,    -1,   217,    -1,   216,   136,   217,
      -1,   170,    -1,   171,    -1,     8,    -1,   134,   172,   135,
      -1,   134,   135,    -1,    -1,    76,   219,    -1,   220,    -1,
     219,   136,   220,    -1,     3,   120,     8,    -1,    36,    47,
     162,   164,    -1,    27,   146,   134,   223,   226,   135,    -1,
     224,    -1,   223,   136,   224,    -1,   217,    -1,   134,   225,
     135,    -1,     8,    -1,   225,   136,     8,    -1,    -1,   136,
     227,    -1,   228,    -1,   227,   136,   228,    -1,   217,   229,
     230,    -1,    -1,    18,    -1,   146,    -1,    66,    -1,   232,
     146,   199,    -1,    38,    -1,    37,    -1,    95,   102,   199,
      -1,    95,    35,   199,    -1,   109,   162,    93,   236,   164,
     185,    -1,   237,    -1,   236,   136,   237,    -1,   146,   120,
     170,    -1,   146,   120,   171,    -1,   146,   120,   134,   172,
     135,    -1,   146,   120,   134,   135,    -1,   263,   120,   170,
      -1,   263,   120,   171,    -1,    59,    -1,    24,    -1,    44,
      -1,    25,    -1,    72,    -1,    73,    -1,    63,    -1,    99,
      -1,    17,   101,   146,    15,    30,   146,   238,    -1,    95,
     248,   111,   241,    -1,    95,   248,   111,    65,     8,    -1,
      -1,   242,    -1,   114,   243,    -1,   244,    -1,   243,   116,
     244,    -1,   146,   120,     8,    -1,    95,    29,    -1,    95,
      28,    93,    -1,    93,   248,   104,    62,    64,   249,    -1,
      -1,    49,    -1,    94,    -1,    85,   108,    -1,    85,    32,
      -1,    86,    85,    -1,    92,    -1,    34,    48,   146,    88,
     251,    96,     8,    -1,    58,    -1,    24,    -1,    44,    -1,
      99,    -1,    42,    48,   146,    -1,    20,    56,   146,   103,
      90,   146,    -1,    45,    90,   146,    -1,    45,    84,   146,
      -1,    91,   257,   183,    -1,    10,    -1,    10,   139,   146,
      -1,    91,   191,    -1,   106,    90,   146,    -1,    78,    56,
     146,    -1,    34,    80,   146,   107,     8,    96,     8,    -1,
      42,    80,   146,   107,     8,    -1,   146,   264,    -1,   265,
      -1,   264,   265,    -1,    13,    -1,    14,    -1,   140,   191,
     141,    -1,   140,     8,   141,    -1,   191,   120,   267,    -1,
     267,   120,   191,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   168,   168,   169,   170,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   210,   211,   211,   211,   211,   211,   212,   212,   212,
     212,   213,   213,   213,   213,   213,   214,   214,   214,   214,
     214,   215,   215,   215,   215,   215,   215,   215,   216,   216,
     216,   216,   217,   217,   217,   217,   217,   218,   218,   218,
     218,   218,   219,   219,   219,   219,   220,   224,   224,   224,
     230,   231,   235,   236,   240,   241,   249,   250,   257,   259,
     263,   267,   274,   275,   276,   280,   293,   300,   302,   307,
     316,   332,   333,   337,   338,   341,   343,   344,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   360,   361,   364,
     366,   370,   374,   375,   376,   380,   385,   389,   396,   404,
     412,   421,   426,   431,   436,   441,   446,   451,   456,   461,
     466,   471,   476,   481,   486,   491,   496,   501,   506,   511,
     516,   524,   528,   529,   534,   540,   546,   552,   558,   559,
     560,   561,   565,   566,   577,   578,   579,   583,   589,   595,
     597,   600,   602,   609,   613,   619,   621,   627,   629,   633,
     644,   646,   650,   654,   661,   662,   666,   667,   668,   671,
     673,   677,   682,   689,   691,   695,   699,   700,   704,   709,
     714,   720,   725,   733,   738,   745,   755,   756,   757,   758,
     759,   760,   761,   762,   763,   765,   766,   767,   768,   769,
     770,   771,   772,   773,   774,   775,   776,   777,   778,   779,
     780,   781,   782,   783,   784,   785,   786,   787,   788,   792,
     793,   794,   795,   796,   797,   798,   799,   800,   801,   802,
     806,   807,   811,   812,   816,   817,   821,   822,   826,   827,
     833,   836,   838,   842,   843,   844,   845,   846,   847,   848,
     849,   854,   859,   869,   874,   879,   884,   885,   886,   890,
     895,   900,   908,   909,   913,   914,   915,   927,   928,   932,
     933,   934,   935,   936,   943,   944,   945,   949,   950,   956,
     964,   965,   968,   970,   974,   975,   979,   980,   984,   985,
     989,   993,   994,   998,   999,  1000,  1001,  1002,  1005,  1006,
    1010,  1011,  1015,  1021,  1031,  1039,  1043,  1050,  1051,  1058,
    1068,  1074,  1076,  1080,  1085,  1089,  1096,  1098,  1102,  1103,
    1109,  1117,  1118,  1124,  1128,  1134,  1142,  1143,  1147,  1161,
    1167,  1171,  1176,  1190,  1201,  1202,  1203,  1204,  1205,  1206,
    1207,  1208,  1212,  1225,  1229,  1236,  1237,  1241,  1245,  1246,
    1250,  1254,  1261,  1268,  1274,  1275,  1276,  1280,  1281,  1282,
    1283,  1289,  1300,  1301,  1302,  1303,  1308,  1319,  1331,  1340,
    1351,  1359,  1360,  1364,  1374,  1385,  1396,  1407,  1418,  1421,
    1422,  1426,  1427,  1428,  1429,  1433,  1434,  1438
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
  "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_TYPE", "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", 
  "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", 
  "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", 
  "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", 
  "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", "'}'", "'.'", 
  "'['", "']'", "$accept", "request", "statement", "ident_set", "ident", 
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
     365,   366,   367,   368,   369,   370,   371,   372,   124,    38,
      61,   373,    60,    62,   374,   375,    43,    45,    42,    47,
      37,   376,   377,    59,    40,    41,    44,   123,   125,    46,
      91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   142,   143,   143,   143,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   146,   146,   146,
     147,   147,   148,   148,   149,   149,   150,   150,   151,   151,
     152,   152,   153,   153,   153,   154,   155,   156,   156,   156,
     157,   158,   158,   159,   159,   160,   160,   160,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   162,   162,   163,
     163,   164,   165,   165,   165,   166,   166,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   168,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   170,   170,   171,   171,   171,   172,   172,   173,
     173,   174,   174,   175,   175,   176,   176,   177,   177,   178,
     179,   179,   180,   180,   181,   181,   182,   182,   182,   183,
     183,   184,   184,   185,   185,   186,   187,   187,   188,   188,
     188,   188,   188,   189,   189,   190,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     193,   193,   194,   194,   195,   195,   196,   196,   197,   197,
     198,   199,   199,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   200,   201,   201,   201,   201,   201,   201,   202,
     202,   202,   203,   203,   204,   204,   204,   205,   205,   206,
     206,   206,   206,   206,   207,   207,   207,   208,   208,   209,
     210,   210,   211,   211,   212,   212,   213,   213,   214,   214,
     215,   216,   216,   217,   217,   217,   217,   217,   218,   218,
     219,   219,   220,   221,   222,   223,   223,   224,   224,   225,
     225,   226,   226,   227,   227,   228,   229,   229,   230,   230,
     231,   232,   232,   233,   234,   235,   236,   236,   237,   237,
     237,   237,   237,   237,   238,   238,   238,   238,   238,   238,
     238,   238,   239,   240,   240,   241,   241,   242,   243,   243,
     244,   245,   246,   247,   248,   248,   248,   249,   249,   249,
     249,   250,   251,   251,   251,   251,   252,   253,   254,   255,
     256,   257,   257,   258,   259,   260,   261,   262,   263,   264,
     264,   265,   265,   265,   265,   266,   266,   267
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
       4,     4,     3,     4,     4,     4,     3,     4,     4,     7,
       5,     5,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     7,
       1,     1,     0,     3,     1,     1,     1,     3,     1,     3,
       3,     1,     3,     1,     1,     1,     3,     2,     0,     2,
       1,     3,     3,     4,     6,     1,     3,     1,     3,     1,
       3,     0,     2,     1,     3,     3,     0,     1,     1,     1,
       3,     1,     1,     3,     3,     6,     1,     3,     3,     3,
       5,     4,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     7,     4,     5,     0,     1,     2,     1,     3,
       3,     2,     3,     6,     0,     1,     1,     2,     2,     2,
       1,     7,     1,     1,     1,     1,     3,     6,     3,     3,
       3,     1,     3,     2,     3,     3,     7,     5,     2,     1,
       2,     1,     1,     3,     3,     3,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   297,     0,   294,     0,     0,   342,   341,
       0,     0,   300,     0,   301,   295,     0,   374,   374,     0,
       0,     0,     0,     2,     3,    80,    82,    84,    86,    83,
       7,     8,     9,   296,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,    29,    30,     0,
       0,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    76,    46,    47,    48,
      49,    50,    78,    51,    52,    53,    54,    56,    55,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    79,    69,    70,    71,    72,    73,    74,    75,    77,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
     207,   209,   210,   407,   212,   391,   211,    34,     0,    38,
       0,     0,    43,    44,   208,     0,     0,    49,     0,    67,
      74,     0,   103,     0,     0,     0,   206,     0,   101,   105,
     108,   234,   189,   235,   236,     0,     0,     0,    41,     0,
      63,     0,     0,     0,     0,   371,   261,   375,     0,   261,
     268,   269,   267,   376,   261,   261,   263,   260,     0,   298,
       0,   117,     0,     1,     4,     0,   261,     0,     0,     0,
       0,     0,     0,   386,     0,   389,   388,   395,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    31,    49,     0,   213,   214,     0,   257,
     256,     0,     0,   401,   402,     0,   398,   399,     0,     0,
       0,   106,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   390,   190,     0,     0,     0,     0,     0,     0,   292,
     293,   291,   290,   289,   276,   287,     0,     0,     0,   261,
       0,   372,     0,   344,     0,   265,   264,   343,   365,   394,
       0,     0,     0,     0,    81,   302,   340,     0,     0,   162,
     164,   315,   166,     0,     0,   313,   314,   327,   331,   325,
       0,     0,     0,   323,     0,   253,     0,   245,   252,     0,
     250,   392,     0,   252,     0,     0,     0,     0,     0,   115,
       0,     0,     0,     0,     0,     0,   248,     0,     0,     0,
     232,     0,   233,     0,   407,     0,   400,    95,   119,   102,
     108,   107,   224,   225,   231,   230,   222,   221,   228,   405,
     229,   219,   220,   227,   226,   215,   216,   217,   218,   223,
     191,   237,     0,   406,   277,   278,     0,     0,     0,   283,
     285,   275,   284,     0,   282,   286,   274,   273,     0,   261,
     266,   261,   262,   272,     0,     0,   363,   366,     0,     0,
     346,     0,   118,     0,     0,     0,     0,   163,   165,   329,
     317,   167,     0,     0,     0,     0,   383,   384,   382,   385,
       0,     0,   153,     0,    38,     0,    43,   157,     0,    48,
      74,     0,   152,   121,   122,   126,   139,     0,   158,   397,
       0,     0,     0,   239,     0,   109,   242,     0,   114,   244,
     243,   113,   240,   241,   110,     0,   111,     0,   112,     0,
       0,     0,   259,   258,   254,   404,   403,     0,   169,   120,
       0,   238,     0,   281,   280,   288,     0,   270,   271,   364,
       0,   367,   368,     0,     0,   193,     0,   305,   304,   306,
       0,     0,     0,   387,   316,     0,   328,     0,   336,   326,
     332,   333,   324,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    88,     0,   251,   116,     0,     0,
       0,     0,     0,   171,   177,   192,     0,     0,     0,   380,
     373,     0,     0,     0,   348,   349,   347,     0,   345,   194,
     352,   353,   303,     0,     0,   318,   308,   355,   357,   356,
     354,   360,   358,   359,   361,   362,   168,   330,   337,     0,
       0,   381,   396,     0,     0,     0,     0,   155,     0,     0,
     156,   124,   123,     0,     0,   131,     0,   149,     0,   147,
     127,   138,   148,   128,   151,   135,   144,   134,   143,   136,
     145,   137,   146,     0,     0,     0,     0,   247,   246,   255,
       0,     0,   172,     0,     0,   175,   178,   279,   378,   377,
     379,   370,   369,   351,     0,     0,   195,   196,   307,     0,
       0,   311,     0,     0,   299,   339,   338,   335,   336,   334,
     161,   154,   160,   159,   125,     0,     0,     0,   150,   132,
       0,    93,    94,    92,    89,    90,    85,     0,     0,     0,
      97,     0,     0,     0,   180,   350,     0,     0,   310,     0,
       0,   319,   320,   309,   133,   141,   142,   140,   129,     0,
       0,   249,     0,     0,    87,   173,   170,     0,   176,     0,
     189,   181,   199,   202,     0,   198,   197,   312,     0,     0,
     130,    91,   186,    96,   184,    98,     0,     0,     0,   193,
       0,     0,   203,     0,   322,   321,   187,   188,     0,     0,
     174,   179,    56,   182,   100,     0,   200,     0,     0,   185,
      99,     0,   205,   204,   201,   183
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   109,   146,    24,    25,    26,    27,   595,
     644,   645,   457,   650,   674,    28,   147,   148,   232,   149,
     338,   458,   303,   423,   424,   425,   426,   427,   401,   296,
     402,   524,   603,   676,   654,   605,   606,   680,   681,   693,
     694,   251,   252,   538,   539,   616,   617,   701,   702,   313,
     151,   309,   310,   221,   222,   454,    29,   273,   177,    30,
      31,   376,   377,   264,   265,    32,    33,    34,    35,   394,
     479,   480,   545,   546,   620,   297,   624,   661,   662,    36,
      37,   298,   299,   403,   405,   490,   491,   559,   627,    38,
      39,    40,    41,    42,   389,   390,   555,    43,    44,   386,
     387,   471,   472,    45,    46,    47,   162,   530,    48,   410,
      49,    50,    51,    52,    53,   152,    54,    55,    56,    57,
      58,   153,   226,   227,   154,   155
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -529
static const short yypact[] =
{
    3954,   -72,    13,  -529,  3433,  -529,    -2,    45,  -529,  -529,
      36,    33,  -529,    44,  -529,  -529,   681,  2841,   350,    57,
      30,  3433,   125,  -529,    32,  -529,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,   112,  -529,  -529,  -529,  3433,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  3433,
    3433,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
      41,  3433,  3433,  3433,  3433,  3433,  3433,  3433,  3433,    43,
    -529,  -529,  -529,  -529,  -529,    49,  -529,    60,    72,    78,
      83,    89,    91,    94,  -529,    97,   103,   107,   114,   119,
     129,  1491,  -529,  1491,  1491,  2939,     6,   -34,  -529,  3037,
      10,  -529,   194,   204,  -529,   151,   160,   174,  3533,  2532,
     187,   185,   203,  3139,   215,  -529,   244,  -529,  3433,   244,
    -529,  -529,  -529,  -529,   244,   244,  -529,  -529,   199,  -529,
    3433,  -529,   -63,  -529,   -57,  3433,   244,   298,   211,    46,
     231,   213,   -83,  -529,   221,  -529,  -529,  -529,   816,  3433,
    1491,  1626,   -24,  1626,  1626,   195,  1491,  1626,  1626,  1491,
    1491,  1491,   197,   202,   222,   223,  -529,  -529,  3687,  -529,
    -529,   -17,   217,  -529,  -529,  1761,     6,  -529,  1978,   951,
    3433,  -529,  -529,  1491,  1491,  1491,  1491,  1491,  1491,  1491,
    1491,  1491,  1491,  1491,  1491,  1491,  1491,  1491,  1491,  1491,
     328,  -529,  -529,   -52,  1491,  2532,  2532,   218,   239,  -529,
    -529,  -529,  -529,  -529,   234,  -529,  2089,   300,   269,     0,
     270,  -529,   361,  -529,   272,  -529,  -529,  -529,   -40,  -529,
    3433,  3433,  1086,   163,  -529,   238,  -529,   343,   285,  -529,
    -529,  -529,  -529,    52,    16,  -529,  -529,  -529,   246,  -529,
     147,   372,  1880,  -529,   375,   264,  1221,  -529,  3901,   -39,
    -529,  -529,  3717,  3962,    63,  3433,   251,    65,    67,  -529,
    3749,    74,    79,  3563,  3596,  3778,  -529,  1356,  1491,  1491,
    -529,  2939,  -529,  2202,   249,  3534,  -529,  -529,   -83,  -529,
    3962,  -529,  -529,  -529,  3994,  3975,   535,  3053,   331,  -529,
     331,   167,   167,   167,   167,   126,   126,  -529,  -529,  -529,
     257,  -529,   312,   331,   234,   234,   260,  2643,  2532,  -529,
    -529,  -529,  -529,   390,  -529,  -529,  -529,  -529,   333,   244,
    -529,   244,  -529,  -529,   392,  3433,  -529,  -529,    34,   -37,
    -529,   278,  -529,  3237,   291,  3433,  3433,  -529,  -529,  -529,
    -529,  -529,    85,   100,    46,   268,  -529,  -529,  -529,  -529,
     308,   309,  -529,   273,   274,   275,   276,  -529,   279,   280,
     282,  1880,     6,   301,  -529,  -529,  -529,   161,  -529,  -529,
     951,   286,  3433,  -529,  1626,  -529,  -529,   288,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  1491,  -529,  1491,  -529,  3625,
    3657,   306,  -529,  -529,  -529,  -529,  -529,   336,   380,  -529,
     431,  -529,    22,  -529,  -529,  -529,   101,  -529,  -529,  -529,
     317,   322,  -529,    62,  3433,   363,    56,  -529,  -529,  -529,
     104,   311,   352,  -529,  -529,    22,  -529,   432,    27,  -529,
     307,  -529,  -529,   434,   438,  3433,   319,  3433,   329,  3433,
     460,   334,   -11,  1880,    56,    19,   -16,    77,    81,    56,
      56,    56,    56,   416,   338,   417,  -529,  -529,  3810,  3840,
    2202,   951,   340,   471,   364,  -529,   123,   -18,   393,  -529,
    -529,   472,  3433,    17,  -529,  -529,  -529,  3433,  -529,  -529,
    -529,  -529,  -529,  3237,    58,   -43,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  3335,
      58,  -529,  -529,     6,   346,   347,   348,  -529,   351,   353,
    -529,  -529,  -529,   368,   370,  -529,    22,  -529,   414,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,    23,  2741,   355,  3433,  -529,  -529,  -529,
     -31,   415,  -529,   468,   446,   444,  -529,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,   134,   378,   365,  -529,  -529,    21,
     138,  -529,   500,   311,  -529,  -529,  -529,  -529,   486,  -529,
    -529,  -529,  -529,  -529,  -529,    56,    56,   143,  -529,  -529,
      22,  -529,  -529,  -529,   371,  -529,  -529,   376,  3433,   480,
     448,  2315,   435,  2315,   447,  -529,   497,  3433,  -529,    58,
     395,   385,  -529,  -529,  -529,  -529,  -529,  -529,  -529,   153,
    2741,  -529,  2315,   518,  -529,  -529,   389,   501,  -529,   505,
     194,  -529,  -529,  -529,  3433,   398,  -529,  -529,   526,   500,
    -529,  -529,   141,   399,  -529,   401,  2315,  2315,  2421,   363,
     418,   165,  -529,   532,  -529,  -529,  -529,  -529,  2315,   536,
    -529,   399,   409,   399,  -529,    22,  -529,  3433,   419,  -529,
    -529,   421,  -529,  -529,  -529,  -529
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -529,  -529,  -529,    -8,    -4,  -529,   360,  -529,   252,  -529,
    -529,  -120,  -529,  -529,  -529,    95,    42,   339,  -529,  -529,
     -10,  -529,  -301,  -409,  -529,   -96,  -529,  -188,  -186,  -468,
    -458,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -528,
    -141,  -111,  -529,  -129,  -529,  -529,   -85,  -529,  -144,   106,
    -529,    26,   140,  -529,   253,    68,  -529,  -168,  -529,  -529,
    -529,   226,  -529,    47,   230,  -529,  -529,  -529,  -529,  -529,
      59,  -529,  -529,   -23,  -529,  -402,  -529,  -529,   -82,  -529,
    -529,  -529,   207,  -529,  -529,  -529,    53,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,   142,  -529,  -529,  -529,  -529,
    -529,  -529,    82,  -529,  -529,  -529,   597,  -529,  -529,  -529,
    -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,  -529,
    -529,  -231,  -529,   391,  -529,   379
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -408
static const short yytable[] =
{
     110,   275,   488,   295,   526,   535,   276,   277,   541,   161,
    -393,   182,   502,   228,   608,   315,   648,   181,   286,   223,
     224,   289,   289,   361,   399,   384,   289,   289,   575,    59,
     280,   302,   639,   622,   282,   186,   574,   459,   283,   581,
     584,   586,   588,   590,   592,   558,   111,   223,   224,   391,
     233,   289,   290,   281,   291,   187,   188,   397,   398,   577,
     292,   289,   290,   289,   290,   272,   291,   289,   290,    60,
     292,   428,   292,   281,   385,   614,   292,   302,   112,   362,
     375,   234,   289,   290,   114,   579,   289,   290,   475,   582,
     609,   292,   113,   623,   572,   292,   433,   434,   -65,   474,
     118,   380,   229,   192,   316,   229,   503,   190,   191,   181,
     193,   194,   195,   196,   197,   578,   115,   116,   637,   331,
     180,   332,   150,   117,   571,   183,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   220,   621,   373,   373,   231,   225,   453,   373,   373,
     258,   400,   613,   576,   473,   263,   400,   640,   628,   270,
     706,   179,  -327,  -327,   274,   184,   233,   665,   667,   711,
     713,   406,   185,   293,   225,   189,   279,   198,   707,   163,
     294,   285,   669,   293,   504,   293,   527,   528,   199,   293,
     428,   407,   619,   529,   200,   311,   533,   234,   436,   434,
     439,   434,   440,   434,   293,   408,   201,   233,   293,   442,
     434,   467,   202,   468,   443,   434,   505,   203,   295,   168,
     484,   485,   506,   204,   181,   205,   341,   314,   206,   317,
     318,   207,   169,   321,   322,   486,   487,   208,   234,   542,
     543,   209,   170,   391,   171,   172,   409,   216,   210,   217,
     218,   263,   263,   211,   247,   248,   249,   687,   607,   485,
     250,   174,   374,   212,   564,   253,   566,   256,   568,   655,
     485,   254,   428,   658,   659,   176,   388,   392,   668,   485,
     255,   507,   508,   509,   510,   511,   512,   534,   690,   485,
     540,  -376,   513,   245,   246,   247,   248,   249,   422,   556,
     716,   717,   364,   365,   308,   266,   312,   267,   271,   272,
     278,   437,   320,   287,   288,   323,   324,   325,   573,   300,
     301,   580,   583,   585,   587,   589,   591,   220,   304,   452,
     319,   335,   326,   360,   453,   340,   327,   333,   366,   342,
     343,   344,   345,   346,   347,   348,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   328,   329,   295,   367,
     363,   368,   378,   374,   263,   647,   163,   379,   381,   382,
     383,   233,   393,   395,   295,   396,   547,   548,   164,   165,
     411,   470,   404,   429,  -407,   166,   438,   461,   340,   478,
     455,   482,   483,   460,   462,   397,   549,   466,   476,   167,
     469,   481,   234,   492,   493,   494,   168,   495,   496,   497,
     498,   550,   218,   499,   500,   551,   501,   422,   503,   169,
     428,   514,   428,   517,   552,   553,   520,   521,   515,   170,
     523,   171,   172,   308,   449,   450,   525,   531,   532,   537,
     557,   428,   561,   560,   173,   544,   562,   565,   174,   664,
     666,   554,   175,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   176,   675,   567,   428,   428,   428,   569,   570,
     388,   593,   596,   295,   594,   601,   602,   428,   610,   604,
     611,   630,   631,   632,   692,   635,   633,   636,   634,   638,
     646,   563,   649,   563,   651,   563,   652,   653,   656,   422,
      61,   657,   682,   660,   558,   683,   672,   670,   710,   692,
     692,   671,   677,    62,   673,   688,   452,    63,    64,    65,
     692,   689,    66,   695,   679,   696,    67,   697,   470,   722,
      68,   698,   703,   615,   704,   708,   340,   709,   715,   478,
     718,   720,    69,   721,   284,    70,    71,    72,    73,    74,
     691,   518,   522,   519,   724,   626,   725,   678,   431,    75,
      76,    77,    78,   600,    79,    80,    81,   719,   339,   699,
     714,    82,   686,   723,   516,   233,    83,    84,    85,    86,
      87,    88,    89,    90,   451,    91,    92,    93,   599,    94,
     643,    95,   563,   464,    96,    97,    98,    99,   465,   100,
     663,   101,   618,   102,   103,   104,   234,   705,   105,   106,
     107,   489,   108,   629,   612,   178,   536,   336,   349,     0,
       0,     0,     0,     0,     0,     0,     0,   340,     0,     0,
       0,   684,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,     0,     0,   422,     0,   422,
       0,     0,   685,   615,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   643,     0,   422,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     700,     0,     0,     0,   119,   120,   121,   122,     0,   123,
     124,   125,   422,   422,   422,   126,     0,    62,     0,     0,
       0,    63,   127,    65,   422,   128,    66,     0,     0,     0,
      67,     0,     0,   700,   129,     0,     0,     0,     0,     0,
       0,     0,   130,     0,     0,   131,    69,     0,     0,    70,
      71,    72,   132,   133,     0,   134,   135,     0,     0,     0,
     136,     0,     0,    75,    76,    77,    78,     0,    79,   137,
      81,   138,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,   139,     0,   100,     0,   101,     0,   102,   103,   104,
       0,     0,   105,   106,   140,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,   142,
       0,     0,   143,     0,     0,   144,     0,     0,   145,   213,
     120,   121,   122,     0,   305,   124,     0,     0,     0,     0,
     126,     0,    62,     0,     0,     0,    63,    64,    65,     0,
     128,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,   130,     0,     0,
     131,    69,     0,     0,    70,    71,    72,    73,    74,     0,
     134,   135,     0,     0,     0,   136,     0,     0,    75,    76,
      77,    78,     0,    79,   214,    81,   215,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,    99,     0,   100,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   140,
       0,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,     0,     0,     0,   143,     0,     0,
     306,   307,     0,   145,   213,   120,   121,   122,     0,   123,
     124,     0,     0,     0,     0,   126,     0,    62,     0,     0,
       0,    63,   127,    65,     0,   128,    66,     0,     0,     0,
      67,     0,     0,     0,   129,     0,     0,     0,     0,     0,
       0,     0,   130,     0,     0,   131,    69,     0,     0,    70,
      71,    72,   132,   133,     0,   134,   135,     0,     0,     0,
     136,     0,     0,    75,    76,    77,    78,     0,    79,   137,
      81,   138,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,   139,     0,   100,     0,   101,     0,   102,   103,   104,
       0,     0,   105,   106,   140,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,   142,
       0,     0,   143,     0,     0,   144,     0,     0,   145,   119,
     120,   121,   122,     0,   123,   124,     0,     0,     0,     0,
     126,     0,    62,     0,     0,     0,    63,   127,    65,     0,
     128,    66,     0,     0,     0,    67,     0,     0,     0,   129,
       0,     0,     0,     0,     0,     0,     0,   130,     0,     0,
     131,    69,     0,     0,    70,    71,    72,   132,   133,     0,
     134,   135,     0,     0,     0,   136,     0,     0,    75,    76,
      77,    78,     0,    79,   137,    81,   138,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,   139,     0,   100,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   140,
       0,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,   142,     0,     0,   143,     0,     0,
     144,     0,     0,   145,   213,   120,   121,   122,     0,   123,
     124,     0,     0,     0,     0,   126,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   128,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,   130,     0,     0,   131,    69,     0,     0,    70,
      71,    72,    73,    74,     0,   134,   135,     0,     0,     0,
     136,     0,     0,    75,    76,    77,    78,     0,    79,   214,
      81,   215,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,   430,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,     0,   101,     0,   102,   103,   104,
       0,     0,   105,   106,   140,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,     0,
       0,     0,   143,     0,     0,   144,     0,     0,   145,   213,
     120,   121,   122,     0,   305,   124,     0,     0,     0,     0,
     126,     0,    62,     0,     0,     0,    63,    64,    65,     0,
     128,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,   130,     0,     0,
     131,    69,     0,     0,    70,    71,    72,    73,    74,     0,
     134,   135,     0,     0,     0,   136,     0,     0,    75,    76,
      77,    78,     0,    79,   214,    81,   215,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,    99,     0,   100,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   140,
       0,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,     0,     0,     0,   143,     0,     0,
     144,   307,     0,   145,   213,   120,   121,   122,     0,   123,
     124,     0,     0,     0,     0,   126,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   128,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,   130,     0,     0,   131,    69,     0,     0,    70,
      71,    72,    73,    74,     0,   134,   135,     0,     0,     0,
     136,     0,     0,    75,    76,    77,    78,     0,    79,   214,
      81,   215,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,     0,   101,     0,   102,   103,   104,
       0,     0,   105,   106,   140,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,     0,
       0,     0,   143,     0,     0,   144,     0,     0,   145,   213,
     120,   121,   122,     0,   305,   124,     0,     0,     0,     0,
     126,     0,    62,     0,     0,     0,    63,    64,    65,     0,
     128,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,   130,     0,     0,
     131,    69,     0,     0,    70,    71,    72,    73,    74,     0,
     134,   135,     0,     0,     0,   136,     0,     0,    75,    76,
      77,    78,     0,    79,   214,    81,   215,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,    99,     0,   100,     0,
     101,     0,   102,   103,   104,     0,     0,   105,   106,   140,
       0,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,     0,     0,     0,   143,     0,     0,
     144,     0,     0,   145,   213,   120,   121,   122,     0,   334,
     124,     0,     0,     0,     0,   126,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   128,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,   130,     0,     0,   131,    69,     0,     0,    70,
      71,    72,    73,    74,     0,   134,   135,     0,     0,     0,
     136,     0,     0,    75,    76,    77,    78,     0,    79,   214,
      81,   215,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,     0,   101,     0,   102,   103,   104,
       0,     0,   105,   106,   140,     0,   108,     0,     0,     0,
       0,     0,     0,    61,   412,     0,     0,     0,   141,     0,
       0,     0,   143,     0,     0,   144,    62,     0,   145,     0,
      63,    64,    65,     0,   413,    66,     0,     0,     0,    67,
       0,     0,     0,   414,     0,     0,     0,     0,     0,     0,
       0,   415,     0,     0,     0,    69,     0,     0,    70,    71,
      72,   416,    74,     0,   417,     0,     0,     0,     0,   418,
       0,     0,    75,    76,    77,    78,     0,   419,    80,    81,
       0,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,    97,    98,
      99,    61,   100,     0,   101,     0,   102,   103,   104,     0,
       0,   105,   106,   420,    62,   108,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,   421,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,    91,    92,    93,     0,
      94,     0,    95,     0,     0,    96,    97,    98,    99,     0,
     100,     0,   101,     0,   102,   103,   104,     0,     0,   105,
     106,   107,    61,   108,   289,     0,     0,   369,     0,     0,
       0,     0,     0,     0,     0,    62,     0,     0,     0,    63,
      64,    65,   337,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   370,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,   371,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,     0,   101,   372,   102,   103,   104,     0,     0,
     105,   106,   107,     0,   108,    61,     0,   289,     0,     0,
       0,     0,     0,     0,     0,     0,   373,     0,    62,     0,
       0,     0,    63,    64,    65,     0,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
      91,    92,    93,     0,    94,     0,    95,     0,     0,    96,
      97,    98,    99,     0,   100,     0,   101,     0,   102,   103,
     104,     0,     0,   105,   106,   107,     0,   108,    61,   412,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   373,
       0,    62,     0,     0,     0,    63,    64,    65,     0,   413,
      66,     0,     0,     0,    67,     0,     0,     0,   414,     0,
       0,     0,     0,     0,     0,     0,   415,     0,     0,     0,
      69,     0,     0,    70,    71,    72,   416,    74,     0,   417,
       0,     0,     0,     0,   418,     0,     0,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,     0,   101,
       0,   102,   103,   104,    61,   412,   105,   106,   420,     0,
     108,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   413,    66,     0,     0,     0,
      67,     0,     0,     0,   414,     0,     0,     0,     0,     0,
       0,     0,   415,     0,     0,     0,    69,     0,     0,    70,
      71,    72,   416,    74,     0,   417,     0,     0,     0,     0,
     418,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,   712,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,     0,   101,     0,   102,   103,   104,
       0,     0,   105,   106,   420,    61,   108,   259,   260,     0,
     261,     0,     0,     0,     0,     0,     0,     0,    62,     0,
       0,     0,    63,    64,    65,     0,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,   262,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
      91,    92,    93,     0,    94,     0,    95,     0,     0,    96,
      97,    98,    99,     0,   100,     0,   101,     0,   102,   103,
     104,     0,     0,   105,   106,   107,    61,   108,   463,     0,
       0,   369,     0,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,    63,    64,    65,     0,     0,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,    61,   100,   641,   101,     0,   102,
     103,   104,     0,     0,   105,   106,   107,    62,   108,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,   642,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,    61,   101,     0,   102,   103,   104,
       0,   156,   105,   106,   107,     0,   108,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,   157,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
     158,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,   159,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,    91,
      92,    93,     0,    94,     0,   160,     0,     0,    96,    97,
      98,    99,    61,   100,     0,     0,     0,   102,   103,   104,
       0,     0,   105,   106,   107,    62,   108,     0,     0,    63,
      64,    65,     0,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,   219,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
      61,   100,     0,   101,     0,   102,   103,   104,     0,     0,
     105,   106,   107,    62,   108,   230,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,     0,     0,   233,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,   234,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
       0,   101,    61,   102,   103,   104,     0,   268,   105,   106,
     107,     0,   108,     0,     0,    62,     0,     0,     0,    63,
      64,    65,     0,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,   269,    98,    99,
      61,   100,     0,   101,     0,   102,   103,   104,     0,     0,
     105,   106,   107,    62,   108,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,   477,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,    61,   100,
       0,   101,     0,   102,   103,   104,     0,     0,   105,   106,
     107,    62,   108,     0,     0,    63,    64,    65,     0,     0,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,   625,    79,    80,    81,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,    61,   100,     0,   101,
       0,   102,   103,   104,     0,     0,   105,   106,   107,    62,
     108,     0,     0,    63,    64,    65,     0,     0,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,    61,   101,     0,   102,
     103,   104,   257,     0,   105,   106,   107,     0,   108,    62,
       0,     0,     0,    63,    64,    65,     0,     0,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,   233,     0,     0,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,    76,    77,    78,     0,
      79,    80,    81,   233,     0,   234,     0,     0,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,   234,   100,   233,  -375,     0,   102,
     103,   104,     0,     0,   105,   106,   107,     0,   108,     0,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   233,     0,   234,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,     0,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,     0,     0,   234,   233,   444,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   233,   234,     0,
       0,   446,   447,     0,     0,     0,     0,     0,     0,     0,
       0,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   233,   234,     0,
       0,   445,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   234,   233,
       0,     0,     0,   447,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   233,     0,
     234,     0,   330,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,     0,   234,
     233,     0,   435,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     233,   234,     0,     0,   441,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,     0,
       0,   234,     0,   448,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   233,     0,     0,     0,   597,     0,   432,     0,     0,
       0,     0,     0,     0,     0,     0,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,     1,   234,     0,     2,   598,     3,     0,     0,     0,
       0,     4,     0,     0,     0,     5,     0,     0,     6,     0,
       7,     8,     9,     0,     0,     0,    10,     0,     0,    11,
       0,     0,   233,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,   233,     0,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,    13,   234,   233,     0,     0,     0,     0,     0,
       0,    14,     0,    15,     0,    16,   234,    17,     0,    18,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
      20,     0,     0,    21,     0,   234,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,     0,     0,     0,
       0,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249
};

static const short yycheck[] =
{
       4,   169,   404,   189,   462,   473,   174,   175,   476,    17,
       0,    21,   421,    47,    32,    39,    47,    21,   186,    13,
      14,     5,     5,    75,     8,    65,     5,     5,     9,   101,
      93,   114,     9,    76,    91,    39,   504,   338,    95,   507,
     508,   509,   510,   511,   512,    18,    48,    13,    14,   280,
      40,     5,     6,   136,     8,    59,    60,     5,     6,    75,
      14,     5,     6,     5,     6,    65,     8,     5,     6,    56,
      14,   302,    14,   136,   114,   533,    14,   114,    80,   131,
     266,    71,     5,     6,    48,     8,     5,     6,   389,     8,
     108,    14,    47,   136,   503,    14,   135,   136,    98,   136,
      56,   269,   136,   113,   128,   136,   117,   111,   112,   113,
     114,   115,   116,   117,   118,   131,    80,    84,   576,   136,
      90,   138,    16,    90,   135,     0,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   145,   544,   127,   127,   149,   140,   333,   127,   127,
     158,   135,   135,   134,   120,   159,   135,   134,   560,   163,
      19,   104,   135,   136,   168,   133,    40,   635,   636,   697,
     698,    24,    60,   127,   140,   134,   180,   134,    37,    16,
     134,   185,   640,   127,    23,   127,    85,    86,   139,   127,
     421,    44,   134,    92,   134,   199,   134,    71,   135,   136,
     135,   136,   135,   136,   127,    58,   134,    40,   127,   135,
     136,   379,   134,   381,   135,   136,    55,   134,   404,    56,
     135,   136,    61,   134,   228,   134,   230,   201,   134,   203,
     204,   134,    69,   207,   208,   135,   136,   134,    71,   135,
     136,   134,    79,   474,    81,    82,    99,   141,   134,   143,
     144,   255,   256,   134,   128,   129,   130,   659,   135,   136,
      66,    98,   266,   134,   495,    61,   497,    93,   499,   135,
     136,   120,   503,   135,   136,   112,   280,   281,   135,   136,
     120,   120,   121,   122,   123,   124,   125,   473,   135,   136,
     476,   104,   131,   126,   127,   128,   129,   130,   302,   485,
     135,   136,   255,   256,   198,   120,   200,   104,    93,    65,
     111,   315,   206,    15,   103,   209,   210,   211,   504,    88,
     107,   507,   508,   509,   510,   511,   512,   331,   107,   333,
     135,   225,   135,     5,   520,   229,   134,   120,   120,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   134,   134,   544,   120,
     254,   127,    62,   367,   368,   596,    16,    98,    98,     8,
      98,    40,   134,    30,   560,    90,    24,    25,    28,    29,
       8,   385,   136,     8,   120,    35,   135,    75,   282,   393,
     141,   395,   396,   136,   134,     5,    44,    64,   120,    49,
       8,   110,    71,   135,    96,    96,    56,   134,   134,   134,
     134,    59,   306,   134,   134,    63,   134,   421,   117,    69,
     651,   135,   653,   135,    72,    73,   120,    91,   432,    79,
      50,    81,    82,   327,   328,   329,     5,   120,   116,    76,
       8,   672,     8,   136,    94,   134,     8,   128,    98,   635,
     636,    99,   102,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   112,   651,   135,   696,   697,   698,     8,   135,
     474,    55,    55,   659,   136,   135,     5,   708,    85,   115,
       8,   135,   135,   135,   672,   117,   135,   117,   135,    75,
     135,   495,    77,   497,    26,   499,    50,    53,   120,   503,
       3,   136,     5,     3,    18,     8,    26,   136,   696,   697,
     698,   135,    77,    16,    66,   120,   520,    20,    21,    22,
     708,   136,    25,     5,    77,   136,    29,    26,   532,   715,
      33,    26,   134,   537,     8,   136,   430,   136,   120,   543,
       8,     5,    45,   134,   184,    48,    49,    50,    51,    52,
     670,   445,   457,   447,   135,   559,   135,   653,   306,    62,
      63,    64,    65,   521,    67,    68,    69,   708,   229,   680,
     699,    74,   657,   717,   434,    40,    79,    80,    81,    82,
      83,    84,    85,    86,   331,    88,    89,    90,   520,    92,
     594,    94,   596,   367,    97,    98,    99,   100,   368,   102,
     623,   104,   543,   106,   107,   108,    71,   689,   111,   112,
     113,   404,   115,   560,   532,    18,   474,   226,   239,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   521,    -1,    -1,
      -1,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   648,    -1,    -1,   651,    -1,   653,
      -1,    -1,   656,   657,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   670,    -1,   672,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     684,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    10,   696,   697,   698,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,   708,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,   717,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,   106,   107,   108,
      -1,    -1,   111,   112,   113,    -1,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,
      -1,    -1,   131,    -1,    -1,   134,    -1,    -1,   137,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,
     104,    -1,   106,   107,   108,    -1,    -1,   111,   112,   113,
      -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,    -1,    -1,    -1,   131,    -1,    -1,
     134,   135,    -1,   137,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,   106,   107,   108,
      -1,    -1,   111,   112,   113,    -1,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,
      -1,    -1,   131,    -1,    -1,   134,    -1,    -1,   137,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,
     104,    -1,   106,   107,   108,    -1,    -1,   111,   112,   113,
      -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,    -1,    -1,   131,    -1,    -1,
     134,    -1,    -1,   137,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    91,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,   106,   107,   108,
      -1,    -1,   111,   112,   113,    -1,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,    -1,
      -1,    -1,   131,    -1,    -1,   134,    -1,    -1,   137,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,
     104,    -1,   106,   107,   108,    -1,    -1,   111,   112,   113,
      -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,    -1,    -1,    -1,   131,    -1,    -1,
     134,   135,    -1,   137,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,   106,   107,   108,
      -1,    -1,   111,   112,   113,    -1,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,    -1,
      -1,    -1,   131,    -1,    -1,   134,    -1,    -1,   137,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,
     104,    -1,   106,   107,   108,    -1,    -1,   111,   112,   113,
      -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,    -1,    -1,    -1,   131,    -1,    -1,
     134,    -1,    -1,   137,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,   106,   107,   108,
      -1,    -1,   111,   112,   113,    -1,   115,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,    -1,    -1,   127,    -1,
      -1,    -1,   131,    -1,    -1,   134,    16,    -1,   137,    -1,
      20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    59,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,
     100,     3,   102,    -1,   104,    -1,   106,   107,   108,    -1,
      -1,   111,   112,   113,    16,   115,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,
     102,    -1,   104,    -1,   106,   107,   108,    -1,    -1,   111,
     112,   113,     3,   115,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,   134,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,   100,
      -1,   102,    -1,   104,   105,   106,   107,   108,    -1,    -1,
     111,   112,   113,    -1,   115,     3,    -1,     5,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   127,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      98,    99,   100,    -1,   102,    -1,   104,    -1,   106,   107,
     108,    -1,    -1,   111,   112,   113,    -1,   115,     3,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    98,    99,   100,    -1,   102,    -1,   104,
      -1,   106,   107,   108,     3,     4,   111,   112,   113,    -1,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,   106,   107,   108,
      -1,    -1,   111,   112,   113,     3,   115,     5,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      98,    99,   100,    -1,   102,    -1,   104,    -1,   106,   107,
     108,    -1,    -1,   111,   112,   113,     3,   115,     5,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    98,    99,   100,     3,   102,     5,   104,    -1,   106,
     107,   108,    -1,    -1,   111,   112,   113,    16,   115,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,    -1,   102,     3,   104,    -1,   106,   107,   108,
      -1,    10,   111,   112,   113,    -1,   115,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    98,
      99,   100,     3,   102,    -1,    -1,    -1,   106,   107,   108,
      -1,    -1,   111,   112,   113,    16,   115,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,   100,
       3,   102,    -1,   104,    -1,   106,   107,   108,    -1,    -1,
     111,   112,   113,    16,   115,    18,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    71,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    98,    99,   100,    -1,   102,
      -1,   104,     3,   106,   107,   108,    -1,     8,   111,   112,
     113,    -1,   115,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    98,    99,   100,
       3,   102,    -1,   104,    -1,   106,   107,   108,    -1,    -1,
     111,   112,   113,    16,   115,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    98,    99,   100,     3,   102,
      -1,   104,    -1,   106,   107,   108,    -1,    -1,   111,   112,
     113,    16,   115,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    98,    99,   100,     3,   102,    -1,   104,
      -1,   106,   107,   108,    -1,    -1,   111,   112,   113,    16,
     115,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    98,    99,   100,    -1,   102,     3,   104,    -1,   106,
     107,   108,     9,    -1,   111,   112,   113,    -1,   115,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    40,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    98,    99,   100,    71,   102,    40,   104,    -1,   106,
     107,   108,    -1,    -1,   111,   112,   113,    -1,   115,    -1,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,    40,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,    -1,    -1,    71,    40,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,    40,    71,    -1,
      -1,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,    -1,    40,    71,    -1,
      -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,    71,    40,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,    40,    -1,
      71,    -1,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,    -1,    71,
      40,    -1,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
      40,    71,    -1,    -1,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,    -1,
      -1,    71,    -1,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,    40,    -1,    -1,    -1,   135,    -1,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,    17,    71,    -1,    20,   135,    22,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    31,    -1,    -1,    34,    -1,
      36,    37,    38,    -1,    -1,    -1,    42,    -1,    -1,    45,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    40,    -1,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,    78,    71,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    89,    -1,    91,    71,    93,    -1,    95,
      -1,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,   109,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    78,    87,    89,    91,    93,    95,    97,
     106,   109,   143,   144,   147,   148,   149,   150,   157,   198,
     201,   202,   207,   208,   209,   210,   221,   222,   231,   232,
     233,   234,   235,   239,   240,   245,   246,   247,   250,   252,
     253,   254,   255,   256,   258,   259,   260,   261,   262,   101,
      56,     3,    16,    20,    21,    22,    25,    29,    33,    45,
      48,    49,    50,    51,    52,    62,    63,    64,    65,    67,
      68,    69,    74,    79,    80,    81,    82,    83,    84,    85,
      86,    88,    89,    90,    92,    94,    97,    98,    99,   100,
     102,   104,   106,   107,   108,   111,   112,   113,   115,   145,
     146,    48,    80,    47,    48,    80,    84,    90,    56,     3,
       4,     5,     6,     8,     9,    10,    14,    21,    24,    33,
      41,    44,    51,    52,    54,    55,    59,    68,    70,   100,
     113,   127,   128,   131,   134,   137,   146,   158,   159,   161,
     191,   192,   257,   263,   266,   267,    10,    28,    49,    74,
      94,   145,   248,    16,    28,    29,    35,    49,    56,    69,
      79,    81,    82,    94,    98,   102,   112,   200,   248,   104,
      90,   146,   162,     0,   133,    60,   146,   146,   146,   134,
     146,   146,   162,   146,   146,   146,   146,   146,   134,   139,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,     3,    68,    70,   191,   191,   191,    55,
     146,   195,   196,    13,    14,   140,   264,   265,    47,   136,
      18,   146,   160,    40,    71,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
      66,   183,   184,    61,   120,   120,    93,     9,   145,     5,
       6,     8,    75,   146,   205,   206,   120,   104,     8,    98,
     146,    93,    65,   199,   146,   199,   199,   199,   111,   146,
      93,   136,    91,    95,   148,   146,   199,    15,   103,     5,
       6,     8,    14,   127,   134,   170,   171,   217,   223,   224,
      88,   107,   114,   164,   107,     8,   134,   135,   191,   193,
     194,   146,   191,   191,   193,    39,   128,   193,   193,   135,
     191,   193,   193,   191,   191,   191,   135,   134,   134,   134,
     135,   136,   138,   120,     8,   191,   265,   134,   162,   159,
     191,   146,   191,   191,   191,   191,   191,   191,   191,   267,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
       5,    75,   131,   191,   205,   205,   120,   120,   127,     8,
      43,    75,   105,   127,   146,   170,   203,   204,    62,    98,
     199,    98,     8,    98,    65,   114,   241,   242,   146,   236,
     237,   263,   146,   134,   211,    30,    90,     5,     6,     8,
     135,   170,   172,   225,   136,   226,    24,    44,    58,    99,
     251,     8,     4,    24,    33,    41,    51,    54,    59,    67,
     113,   134,   146,   165,   166,   167,   168,   169,   263,     8,
      91,   150,    46,   135,   136,   135,   135,   146,   135,   135,
     135,   135,   135,   135,   135,   136,   135,   136,   135,   191,
     191,   196,   146,   170,   197,   141,   141,   154,   163,   164,
     136,    75,   134,     5,   203,   206,    64,   199,   199,     8,
     146,   243,   244,   120,   136,   164,   120,    54,   146,   212,
     213,   110,   146,   146,   135,   136,   135,   136,   217,   224,
     227,   228,   135,    96,    96,   134,   134,   134,   134,   134,
     134,   134,   165,   117,    23,    55,    61,   120,   121,   122,
     123,   124,   125,   131,   135,   146,   194,   135,   191,   191,
     120,    91,   157,    50,   173,     5,   172,    85,    86,    92,
     249,   120,   116,   134,   170,   171,   237,    76,   185,   186,
     170,   171,   135,   136,   134,   214,   215,    24,    25,    44,
      59,    63,    72,    73,    99,   238,   170,     8,    18,   229,
     136,     8,     8,   146,   263,   128,   263,   135,   263,     8,
     135,   135,   165,   170,   171,     9,   134,    75,   131,     8,
     170,   171,     8,   170,   171,   170,   171,   170,   171,   170,
     171,   170,   171,    55,   136,   151,    55,   135,   135,   197,
     158,   135,     5,   174,   115,   177,   178,   135,    32,   108,
      85,     8,   244,   135,   172,   146,   187,   188,   212,   134,
     216,   217,    76,   136,   218,    66,   146,   230,   217,   228,
     135,   135,   135,   135,   135,   117,   117,   172,    75,     9,
     134,     5,    54,   146,   152,   153,   135,   263,    47,    77,
     155,    26,    50,    53,   176,   135,   120,   136,   135,   136,
       3,   219,   220,   215,   170,   171,   170,   171,   135,   172,
     136,   135,    26,    66,   156,   169,   175,    77,   167,    77,
     179,   180,     5,     8,   134,   146,   188,   217,   120,   136,
     135,   153,   169,   181,   182,     5,   136,    26,    26,   183,
     146,   189,   190,   134,     8,   220,    19,    37,   136,   136,
     169,   181,    83,   181,   185,   120,   135,   136,     8,   182,
       5,   134,   170,   190,   135,   135
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

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 274:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 275:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 276:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 277:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 278:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 279:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 280:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 281:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 284:

    { yyval.m_iValue = 1; ;}
    break;

  case 285:

    { yyval.m_iValue = 0; ;}
    break;

  case 286:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 294:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 295:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 296:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 299:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 300:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 301:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 306:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 307:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 310:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 311:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 312:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 313:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 314:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 315:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 316:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 317:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 322:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 323:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 324:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 325:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 326:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 328:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 329:

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

  case 330:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 333:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 335:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 340:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 343:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 344:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 345:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 348:

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

  case 349:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 350:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 351:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 352:

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

  case 353:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 354:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 355:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 356:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 357:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 358:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 359:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 360:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 361:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 362:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 363:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 364:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 371:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 372:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 373:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 381:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 382:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 383:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 384:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 385:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 386:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 387:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 388:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 389:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 390:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 393:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 394:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 395:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 396:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 397:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 398:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 400:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 401:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 402:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 403:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 404:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 405:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 406:

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

