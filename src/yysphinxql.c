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
     TOK_REMAP = 343,
     TOK_RETURNS = 344,
     TOK_ROLLBACK = 345,
     TOK_RTINDEX = 346,
     TOK_SELECT = 347,
     TOK_SERIALIZABLE = 348,
     TOK_SET = 349,
     TOK_SESSION = 350,
     TOK_SHOW = 351,
     TOK_SONAME = 352,
     TOK_START = 353,
     TOK_STATUS = 354,
     TOK_STRING = 355,
     TOK_SUM = 356,
     TOK_TABLE = 357,
     TOK_TABLES = 358,
     TOK_THREADS = 359,
     TOK_TO = 360,
     TOK_TRANSACTION = 361,
     TOK_TRUE = 362,
     TOK_TRUNCATE = 363,
     TOK_TYPE = 364,
     TOK_UNCOMMITTED = 365,
     TOK_UPDATE = 366,
     TOK_VALUES = 367,
     TOK_VARIABLES = 368,
     TOK_WARNINGS = 369,
     TOK_WEIGHT = 370,
     TOK_WHERE = 371,
     TOK_WITHIN = 372,
     TOK_OR = 373,
     TOK_AND = 374,
     TOK_NE = 375,
     TOK_GTE = 376,
     TOK_LTE = 377,
     TOK_NOT = 378,
     TOK_NEG = 379
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
#define TOK_REMAP 343
#define TOK_RETURNS 344
#define TOK_ROLLBACK 345
#define TOK_RTINDEX 346
#define TOK_SELECT 347
#define TOK_SERIALIZABLE 348
#define TOK_SET 349
#define TOK_SESSION 350
#define TOK_SHOW 351
#define TOK_SONAME 352
#define TOK_START 353
#define TOK_STATUS 354
#define TOK_STRING 355
#define TOK_SUM 356
#define TOK_TABLE 357
#define TOK_TABLES 358
#define TOK_THREADS 359
#define TOK_TO 360
#define TOK_TRANSACTION 361
#define TOK_TRUE 362
#define TOK_TRUNCATE 363
#define TOK_TYPE 364
#define TOK_UNCOMMITTED 365
#define TOK_UPDATE 366
#define TOK_VALUES 367
#define TOK_VARIABLES 368
#define TOK_WARNINGS 369
#define TOK_WEIGHT 370
#define TOK_WHERE 371
#define TOK_WITHIN 372
#define TOK_OR 373
#define TOK_AND 374
#define TOK_NE 375
#define TOK_GTE 376
#define TOK_LTE 377
#define TOK_NOT 378
#define TOK_NEG 379




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
#define YYFINAL  186
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4359

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  144
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  126
/* YYNRULES -- Number of rules. */
#define YYNRULES  411
/* YYNRULES -- Number of states. */
#define YYNSTATES  746

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   379

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   132,   121,     2,
     136,   137,   130,   128,   138,   129,   141,   131,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   135,
     124,   122,   125,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   142,     2,   143,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   139,   120,   140,     2,     2,     2,     2,
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
     115,   116,   117,   118,   119,   123,   126,   127,   133,   134
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
     160,   162,   164,   168,   170,   172,   174,   183,   185,   195,
     196,   199,   201,   205,   207,   209,   211,   212,   216,   217,
     220,   225,   237,   239,   243,   245,   248,   249,   251,   254,
     256,   261,   266,   271,   276,   281,   286,   290,   296,   298,
     302,   303,   305,   308,   310,   314,   318,   323,   325,   329,
     333,   339,   346,   350,   355,   361,   365,   369,   373,   377,
     381,   383,   389,   395,   401,   405,   409,   413,   417,   421,
     425,   429,   434,   438,   440,   442,   447,   451,   455,   457,
     459,   464,   469,   474,   476,   479,   481,   484,   486,   488,
     492,   493,   498,   499,   501,   503,   507,   508,   511,   512,
     514,   520,   521,   523,   527,   533,   535,   539,   541,   544,
     547,   548,   550,   553,   558,   559,   561,   564,   566,   570,
     574,   578,   584,   591,   595,   597,   601,   605,   607,   609,
     611,   613,   615,   617,   619,   622,   625,   629,   633,   637,
     641,   645,   649,   653,   657,   661,   665,   669,   673,   677,
     681,   685,   689,   693,   697,   701,   703,   705,   707,   711,
     716,   721,   726,   731,   736,   741,   746,   750,   757,   764,
     768,   777,   792,   794,   798,   800,   802,   806,   812,   814,
     816,   818,   820,   823,   824,   827,   829,   832,   835,   839,
     841,   843,   845,   848,   853,   858,   862,   867,   872,   877,
     881,   886,   891,   899,   905,   911,   913,   915,   917,   919,
     921,   923,   927,   929,   931,   933,   935,   937,   939,   941,
     943,   945,   948,   956,   958,   960,   961,   965,   967,   969,
     971,   975,   977,   981,   985,   987,   991,   993,   995,   997,
    1001,  1004,  1005,  1008,  1010,  1014,  1018,  1023,  1030,  1032,
    1036,  1038,  1042,  1044,  1048,  1049,  1052,  1054,  1058,  1062,
    1063,  1065,  1067,  1069,  1073,  1075,  1077,  1081,  1085,  1092,
    1094,  1098,  1102,  1106,  1112,  1117,  1121,  1125,  1127,  1129,
    1131,  1133,  1135,  1137,  1139,  1141,  1149,  1156,  1161,  1167,
    1168,  1170,  1173,  1175,  1179,  1183,  1186,  1190,  1197,  1198,
    1200,  1202,  1205,  1208,  1211,  1213,  1221,  1223,  1225,  1227,
    1229,  1233,  1240,  1244,  1248,  1252,  1254,  1258,  1261,  1265,
    1269,  1277,  1283,  1286,  1288,  1291,  1293,  1295,  1299,  1303,
    1307,  1311
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     145,     0,    -1,   146,    -1,   149,    -1,   149,   135,    -1,
     211,    -1,   223,    -1,   203,    -1,   204,    -1,   209,    -1,
     224,    -1,   233,    -1,   235,    -1,   236,    -1,   237,    -1,
     242,    -1,   247,    -1,   248,    -1,   252,    -1,   254,    -1,
     255,    -1,   256,    -1,   257,    -1,   249,    -1,   258,    -1,
     260,    -1,   261,    -1,   262,    -1,   241,    -1,   263,    -1,
     264,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    29,    -1,    33,    -1,    45,    -1,
      48,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      62,    -1,    64,    -1,    65,    -1,    67,    -1,    68,    -1,
      69,    -1,    79,    -1,    80,    -1,    81,    -1,    82,    -1,
      84,    -1,    83,    -1,    85,    -1,    86,    -1,    89,    -1,
      90,    -1,    91,    -1,    93,    -1,    95,    -1,    98,    -1,
      99,    -1,   100,    -1,   101,    -1,   103,    -1,   104,    -1,
     108,    -1,   109,    -1,   110,    -1,   113,    -1,   114,    -1,
     115,    -1,   117,    -1,    63,    -1,   147,    -1,    74,    -1,
     106,    -1,   150,    -1,   149,   135,   150,    -1,   151,    -1,
     200,    -1,   152,    -1,    92,     3,   136,   136,   152,   137,
     153,   137,    -1,   159,    -1,    92,   160,    47,   136,   156,
     159,   137,   157,   158,    -1,    -1,   138,   154,    -1,   155,
      -1,   154,   138,   155,    -1,   148,    -1,     5,    -1,    54,
      -1,    -1,    77,    26,   183,    -1,    -1,    66,     5,    -1,
      66,     5,   138,     5,    -1,    92,   160,    47,   164,   165,
     175,   179,   178,   181,   185,   187,    -1,   161,    -1,   160,
     138,   161,    -1,   130,    -1,   163,   162,    -1,    -1,   148,
      -1,    18,   148,    -1,   193,    -1,    21,   136,   193,   137,
      -1,    68,   136,   193,   137,    -1,    70,   136,   193,   137,
      -1,   101,   136,   193,   137,    -1,    52,   136,   193,   137,
      -1,    33,   136,   130,   137,    -1,    51,   136,   137,    -1,
      33,   136,    39,   148,   137,    -1,   148,    -1,   164,   138,
     148,    -1,    -1,   166,    -1,   116,   167,    -1,   168,    -1,
     167,   119,   167,    -1,   136,   167,   137,    -1,    67,   136,
       8,   137,    -1,   169,    -1,   171,   122,   172,    -1,   171,
     123,   172,    -1,   171,    55,   136,   174,   137,    -1,   171,
     133,    55,   136,   174,   137,    -1,   171,    55,     9,    -1,
     171,   133,    55,     9,    -1,   171,    23,   172,   119,   172,
      -1,   171,   125,   172,    -1,   171,   124,   172,    -1,   171,
     126,   172,    -1,   171,   127,   172,    -1,   171,   122,   173,
      -1,   170,    -1,   171,    23,   173,   119,   173,    -1,   171,
      23,   172,   119,   173,    -1,   171,    23,   173,   119,   172,
      -1,   171,   125,   173,    -1,   171,   124,   173,    -1,   171,
     126,   173,    -1,   171,   127,   173,    -1,   171,   122,     8,
      -1,   171,   123,     8,    -1,   171,    61,    75,    -1,   171,
      61,   133,    75,    -1,   171,   123,   173,    -1,   148,    -1,
       4,    -1,    33,   136,   130,   137,    -1,    51,   136,   137,
      -1,   115,   136,   137,    -1,    54,    -1,   265,    -1,    59,
     136,   265,   137,    -1,    41,   136,   265,   137,    -1,    24,
     136,   265,   137,    -1,     5,    -1,   129,     5,    -1,     6,
      -1,   129,     6,    -1,    14,    -1,   172,    -1,   174,   138,
     172,    -1,    -1,    50,   176,    26,   177,    -1,    -1,     5,
      -1,   171,    -1,   177,   138,   171,    -1,    -1,    53,   169,
      -1,    -1,   180,    -1,   117,    50,    77,    26,   183,    -1,
      -1,   182,    -1,    77,    26,   183,    -1,    77,    26,    83,
     136,   137,    -1,   184,    -1,   183,   138,   184,    -1,   171,
      -1,   171,    19,    -1,   171,    37,    -1,    -1,   186,    -1,
      66,     5,    -1,    66,     5,   138,     5,    -1,    -1,   188,
      -1,    76,   189,    -1,   190,    -1,   189,   138,   190,    -1,
     148,   122,   148,    -1,   148,   122,     5,    -1,   148,   122,
     136,   191,   137,    -1,   148,   122,   148,   136,     8,   137,
      -1,   148,   122,     8,    -1,   192,    -1,   191,   138,   192,
      -1,   148,   122,   172,    -1,   148,    -1,     4,    -1,    54,
      -1,     5,    -1,     6,    -1,    14,    -1,     9,    -1,   129,
     193,    -1,   133,   193,    -1,   193,   128,   193,    -1,   193,
     129,   193,    -1,   193,   130,   193,    -1,   193,   131,   193,
      -1,   193,   124,   193,    -1,   193,   125,   193,    -1,   193,
     121,   193,    -1,   193,   120,   193,    -1,   193,   132,   193,
      -1,   193,    40,   193,    -1,   193,    71,   193,    -1,   193,
     127,   193,    -1,   193,   126,   193,    -1,   193,   122,   193,
      -1,   193,   123,   193,    -1,   193,   119,   193,    -1,   193,
     118,   193,    -1,   136,   193,   137,    -1,   139,   197,   140,
      -1,   194,    -1,   265,    -1,   268,    -1,   265,    61,    75,
      -1,   265,    61,   133,    75,    -1,     3,   136,   195,   137,
      -1,    55,   136,   195,   137,    -1,    59,   136,   195,   137,
      -1,    24,   136,   195,   137,    -1,    44,   136,   195,   137,
      -1,    41,   136,   195,   137,    -1,     3,   136,   137,    -1,
      70,   136,   193,   138,   193,   137,    -1,    68,   136,   193,
     138,   193,   137,    -1,   115,   136,   137,    -1,     3,   136,
     193,    46,   148,    55,   265,   137,    -1,    88,   136,   193,
     138,   193,   138,   136,   195,   137,   138,   136,   195,   137,
     137,    -1,   196,    -1,   195,   138,   196,    -1,   193,    -1,
       8,    -1,   198,   122,   199,    -1,   197,   138,   198,   122,
     199,    -1,   148,    -1,    55,    -1,   172,    -1,   148,    -1,
      96,   202,    -1,    -1,    65,     8,    -1,   114,    -1,    99,
     201,    -1,    69,   201,    -1,    16,    99,   201,    -1,    82,
      -1,    79,    -1,    81,    -1,   104,   187,    -1,    16,     8,
      99,   201,    -1,    16,   148,    99,   201,    -1,    56,   148,
      99,    -1,    94,   147,   122,   206,    -1,    94,   147,   122,
     205,    -1,    94,   147,   122,    75,    -1,    94,    74,   207,
      -1,    94,    10,   122,   207,    -1,    94,    28,    94,   207,
      -1,    94,    49,     9,   122,   136,   174,   137,    -1,    94,
      49,   147,   122,   205,    -1,    94,    49,   147,   122,     5,
      -1,   148,    -1,     8,    -1,   107,    -1,    43,    -1,   172,
      -1,   208,    -1,   207,   129,   208,    -1,   148,    -1,    75,
      -1,     8,    -1,     5,    -1,     6,    -1,    31,    -1,    90,
      -1,   210,    -1,    22,    -1,    98,   106,    -1,   212,    60,
     148,   213,   112,   216,   220,    -1,    57,    -1,    87,    -1,
      -1,   136,   215,   137,    -1,   148,    -1,    54,    -1,   214,
      -1,   215,   138,   214,    -1,   217,    -1,   216,   138,   217,
      -1,   136,   218,   137,    -1,   219,    -1,   218,   138,   219,
      -1,   172,    -1,   173,    -1,     8,    -1,   136,   174,   137,
      -1,   136,   137,    -1,    -1,    76,   221,    -1,   222,    -1,
     221,   138,   222,    -1,     3,   122,     8,    -1,    36,    47,
     164,   166,    -1,    27,   148,   136,   225,   228,   137,    -1,
     226,    -1,   225,   138,   226,    -1,   219,    -1,   136,   227,
     137,    -1,     8,    -1,   227,   138,     8,    -1,    -1,   138,
     229,    -1,   230,    -1,   229,   138,   230,    -1,   219,   231,
     232,    -1,    -1,    18,    -1,   148,    -1,    66,    -1,   234,
     148,   201,    -1,    38,    -1,    37,    -1,    96,   103,   201,
      -1,    96,    35,   201,    -1,   111,   164,    94,   238,   166,
     187,    -1,   239,    -1,   238,   138,   239,    -1,   148,   122,
     172,    -1,   148,   122,   173,    -1,   148,   122,   136,   174,
     137,    -1,   148,   122,   136,   137,    -1,   265,   122,   172,
      -1,   265,   122,   173,    -1,    59,    -1,    24,    -1,    44,
      -1,    25,    -1,    72,    -1,    73,    -1,    63,    -1,   100,
      -1,    17,   102,   148,    15,    30,   148,   240,    -1,    17,
     102,   148,    42,    30,   148,    -1,    96,   250,   113,   243,
      -1,    96,   250,   113,    65,     8,    -1,    -1,   244,    -1,
     116,   245,    -1,   246,    -1,   245,   118,   246,    -1,   148,
     122,     8,    -1,    96,    29,    -1,    96,    28,    94,    -1,
      94,   250,   106,    62,    64,   251,    -1,    -1,    49,    -1,
      95,    -1,    85,   110,    -1,    85,    32,    -1,    86,    85,
      -1,    93,    -1,    34,    48,   148,    89,   253,    97,     8,
      -1,    58,    -1,    24,    -1,    44,    -1,   100,    -1,    42,
      48,   148,    -1,    20,    56,   148,   105,    91,   148,    -1,
      45,    91,   148,    -1,    45,    84,   148,    -1,    92,   259,
     185,    -1,    10,    -1,    10,   141,   148,    -1,    92,   193,
      -1,   108,    91,   148,    -1,    78,    56,   148,    -1,    34,
      80,   148,   109,     8,    97,     8,    -1,    42,    80,   148,
     109,     8,    -1,   148,   266,    -1,   267,    -1,   266,   267,
      -1,    13,    -1,    14,    -1,   142,   193,   143,    -1,   142,
       8,   143,    -1,   193,   122,   269,    -1,   269,   122,   193,
      -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   170,   170,   171,   172,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   212,   213,   213,   213,   213,   213,   214,   214,   214,
     214,   215,   215,   215,   215,   215,   216,   216,   216,   216,
     216,   217,   217,   217,   217,   217,   217,   217,   218,   218,
     218,   218,   219,   219,   219,   219,   219,   220,   220,   220,
     220,   220,   220,   221,   221,   221,   221,   222,   226,   226,
     226,   232,   233,   237,   238,   242,   243,   251,   252,   259,
     261,   265,   269,   276,   277,   278,   282,   295,   302,   304,
     309,   318,   334,   335,   339,   340,   343,   345,   346,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   362,   363,
     366,   368,   372,   376,   377,   378,   382,   387,   391,   398,
     406,   414,   423,   428,   433,   438,   443,   448,   453,   458,
     463,   468,   473,   478,   483,   488,   493,   498,   503,   508,
     513,   518,   526,   530,   531,   536,   542,   548,   554,   560,
     561,   562,   563,   567,   568,   579,   580,   581,   585,   591,
     597,   599,   602,   604,   611,   615,   621,   623,   629,   631,
     635,   646,   648,   652,   656,   663,   664,   668,   669,   670,
     673,   675,   679,   684,   691,   693,   697,   701,   702,   706,
     711,   716,   722,   727,   735,   740,   747,   757,   758,   759,
     760,   761,   762,   763,   764,   765,   767,   768,   769,   770,
     771,   772,   773,   774,   775,   776,   777,   778,   779,   780,
     781,   782,   783,   784,   785,   786,   787,   788,   789,   790,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   803,
     804,   805,   809,   810,   814,   815,   819,   820,   824,   825,
     829,   830,   836,   839,   841,   845,   846,   847,   848,   849,
     850,   851,   852,   853,   858,   863,   873,   878,   883,   888,
     889,   890,   894,   899,   904,   912,   913,   917,   918,   919,
     931,   932,   936,   937,   938,   939,   940,   947,   948,   949,
     953,   954,   960,   968,   969,   972,   974,   978,   979,   983,
     984,   988,   989,   993,   997,   998,  1002,  1003,  1004,  1005,
    1006,  1009,  1010,  1014,  1015,  1019,  1025,  1035,  1043,  1047,
    1054,  1055,  1062,  1072,  1078,  1080,  1084,  1089,  1093,  1100,
    1102,  1106,  1107,  1113,  1121,  1122,  1128,  1132,  1138,  1146,
    1147,  1151,  1165,  1171,  1175,  1180,  1194,  1205,  1206,  1207,
    1208,  1209,  1210,  1211,  1212,  1216,  1224,  1236,  1240,  1247,
    1248,  1252,  1256,  1257,  1261,  1265,  1272,  1279,  1285,  1286,
    1287,  1291,  1292,  1293,  1294,  1300,  1311,  1312,  1313,  1314,
    1319,  1330,  1342,  1351,  1362,  1370,  1371,  1375,  1385,  1396,
    1407,  1418,  1429,  1432,  1433,  1437,  1438,  1439,  1440,  1444,
    1445,  1449
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
  "TOK_REMAP", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", 
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
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     124,    38,    61,   375,    60,    62,   376,   377,    43,    45,
      42,    47,    37,   378,   379,    59,    40,    41,    44,   123,
     125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   144,   145,   145,   145,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   148,   148,
     148,   149,   149,   150,   150,   151,   151,   152,   152,   153,
     153,   154,   154,   155,   155,   155,   156,   157,   158,   158,
     158,   159,   160,   160,   161,   161,   162,   162,   162,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   164,   164,
     165,   165,   166,   167,   167,   167,   168,   168,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   170,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   172,   172,   173,   173,   173,   174,   174,
     175,   175,   176,   176,   177,   177,   178,   178,   179,   179,
     180,   181,   181,   182,   182,   183,   183,   184,   184,   184,
     185,   185,   186,   186,   187,   187,   188,   189,   189,   190,
     190,   190,   190,   190,   191,   191,   192,   193,   193,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,   193,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   195,   195,   196,   196,   197,   197,   198,   198,
     199,   199,   200,   201,   201,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   203,   203,   203,   203,
     203,   203,   204,   204,   204,   205,   205,   206,   206,   206,
     207,   207,   208,   208,   208,   208,   208,   209,   209,   209,
     210,   210,   211,   212,   212,   213,   213,   214,   214,   215,
     215,   216,   216,   217,   218,   218,   219,   219,   219,   219,
     219,   220,   220,   221,   221,   222,   223,   224,   225,   225,
     226,   226,   227,   227,   228,   228,   229,   229,   230,   231,
     231,   232,   232,   233,   234,   234,   235,   236,   237,   238,
     238,   239,   239,   239,   239,   239,   239,   240,   240,   240,
     240,   240,   240,   240,   240,   241,   241,   242,   242,   243,
     243,   244,   245,   245,   246,   247,   248,   249,   250,   250,
     250,   251,   251,   251,   251,   252,   253,   253,   253,   253,
     254,   255,   256,   257,   258,   259,   259,   260,   261,   262,
     263,   264,   265,   266,   266,   267,   267,   267,   267,   268,
     268,   269
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
       1,     1,     3,     1,     1,     1,     8,     1,     9,     0,
       2,     1,     3,     1,     1,     1,     0,     3,     0,     2,
       4,    11,     1,     3,     1,     2,     0,     1,     2,     1,
       4,     4,     4,     4,     4,     4,     3,     5,     1,     3,
       0,     1,     2,     1,     3,     3,     4,     1,     3,     3,
       5,     6,     3,     4,     5,     3,     3,     3,     3,     3,
       1,     5,     5,     5,     3,     3,     3,     3,     3,     3,
       3,     4,     3,     1,     1,     4,     3,     3,     1,     1,
       4,     4,     4,     1,     2,     1,     2,     1,     1,     3,
       0,     4,     0,     1,     1,     3,     0,     2,     0,     1,
       5,     0,     1,     3,     5,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     6,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     3,     4,
       4,     4,     4,     4,     4,     4,     3,     6,     6,     3,
       8,    14,     1,     3,     1,     1,     3,     5,     1,     1,
       1,     1,     2,     0,     2,     1,     2,     2,     3,     1,
       1,     1,     2,     4,     4,     3,     4,     4,     4,     3,
       4,     4,     7,     5,     5,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     7,     1,     1,     0,     3,     1,     1,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     1,     3,
       2,     0,     2,     1,     3,     3,     4,     6,     1,     3,
       1,     3,     1,     3,     0,     2,     1,     3,     3,     0,
       1,     1,     1,     3,     1,     1,     3,     3,     6,     1,
       3,     3,     3,     5,     4,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     7,     6,     4,     5,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     1,
       3,     6,     3,     3,     3,     1,     3,     2,     3,     3,
       7,     5,     2,     1,     2,     1,     1,     3,     3,     3,
       3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   300,     0,   297,     0,     0,   345,   344,
       0,     0,   303,     0,   304,   298,     0,   378,   378,     0,
       0,     0,     0,     2,     3,    81,    83,    85,    87,    84,
       7,     8,     9,   299,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,    29,    30,     0,
       0,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    77,    46,    47,    48,
      49,    50,    79,    51,    52,    53,    54,    56,    55,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    80,    70,    71,    72,    73,    74,    75,    76,
      78,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      31,   208,   210,   211,   411,   213,   395,   212,    34,     0,
      38,     0,     0,    43,    44,   209,     0,     0,    49,     0,
       0,    67,    75,     0,   104,     0,     0,     0,   207,     0,
     102,   106,   109,   235,   190,   236,   237,     0,     0,     0,
      41,     0,    63,     0,     0,     0,     0,   375,   263,   379,
       0,   263,   270,   271,   269,   380,   263,   263,   194,   265,
     262,     0,   301,     0,   118,     0,     1,     4,     0,   263,
       0,     0,     0,     0,     0,     0,   390,     0,   393,   392,
     399,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    31,    49,     0,
     214,   215,     0,   259,   258,     0,     0,   405,   406,     0,
     402,   403,     0,     0,     0,   107,   105,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   394,   191,     0,     0,     0,
       0,     0,     0,   295,   296,   294,   293,   292,   279,   290,
       0,     0,     0,   263,     0,   376,     0,   347,     0,   267,
     266,   346,     0,   272,   195,   369,   398,     0,     0,     0,
       0,    82,   305,   343,     0,     0,     0,   163,   165,   318,
     167,     0,     0,   316,   317,   330,   334,   328,     0,     0,
       0,   326,     0,   255,     0,   246,   254,     0,   252,   396,
       0,   254,     0,     0,     0,     0,     0,   116,     0,     0,
       0,     0,     0,     0,     0,   249,     0,     0,     0,   233,
       0,   234,     0,   411,     0,   404,    96,   120,   103,   109,
     108,   225,   226,   232,   231,   223,   222,   229,   409,   230,
     220,   221,   228,   227,   216,   217,   218,   219,   224,   192,
     238,     0,   410,   280,   281,     0,     0,     0,   286,   288,
     278,   287,     0,   285,   289,   277,   276,     0,   263,   268,
     263,   264,   275,     0,   196,   197,     0,     0,   367,   370,
       0,     0,   349,     0,   119,     0,     0,     0,     0,     0,
     164,   166,   332,   320,   168,     0,     0,     0,     0,   387,
     388,   386,   389,     0,     0,   154,     0,    38,     0,    43,
     158,     0,    48,    75,     0,   153,   122,   123,   127,   140,
       0,   159,   401,     0,     0,     0,   240,     0,   110,   243,
       0,   115,   245,   244,   114,   241,   242,   111,     0,   112,
       0,     0,   113,     0,     0,     0,   261,   260,   256,   408,
     407,     0,   170,   121,     0,   239,     0,   284,   283,   291,
       0,   273,   274,     0,     0,   368,     0,   371,   372,     0,
       0,   194,     0,   308,   307,   309,     0,     0,     0,   366,
     391,   319,     0,   331,     0,   339,   329,   335,   336,   327,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    89,     0,   253,   117,     0,     0,     0,     0,     0,
       0,   172,   178,   193,     0,     0,     0,   384,   377,   200,
     203,     0,   199,   198,     0,     0,     0,   351,   352,   350,
     348,   355,   356,   306,     0,     0,   321,   311,   358,   360,
     359,   357,   363,   361,   362,   364,   365,   169,   333,   340,
       0,     0,   385,   400,     0,     0,     0,     0,   156,     0,
       0,   157,   125,   124,     0,     0,   132,     0,   150,     0,
     148,   128,   139,   149,   129,   152,   136,   145,   135,   144,
     137,   146,   138,   147,     0,     0,     0,     0,   248,   247,
       0,   257,     0,     0,   173,     0,     0,   176,   179,   282,
     382,   381,   383,     0,     0,   204,     0,   374,   373,   354,
       0,   310,     0,     0,   314,     0,     0,   302,   342,   341,
     338,   339,   337,   162,   155,   161,   160,   126,     0,     0,
       0,   151,   133,     0,    94,    95,    93,    90,    91,    86,
       0,     0,     0,     0,    98,     0,     0,     0,   181,     0,
     201,     0,     0,   353,   313,     0,     0,   322,   323,   312,
     134,   142,   143,   141,   130,     0,     0,   250,     0,     0,
       0,    88,   174,   171,     0,   177,     0,   190,   182,   206,
     205,   202,   315,     0,     0,   131,    92,     0,   187,    97,
     185,    99,     0,     0,     0,   194,   325,   324,     0,   188,
     189,     0,     0,   175,   180,    56,   183,   101,     0,   186,
     100,     0,     0,   184,     0,   251
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   110,   148,    24,    25,    26,    27,   616,
     667,   668,   471,   674,   701,    28,   149,   150,   236,   151,
     347,   472,   311,   436,   437,   438,   439,   440,   414,   304,
     415,   542,   625,   703,   678,   627,   628,   707,   708,   719,
     720,   255,   256,   283,   284,   394,   395,   634,   635,   321,
     153,   317,   318,   225,   226,   468,    29,   277,   180,    30,
      31,   385,   386,   268,   269,    32,    33,    34,    35,   406,
     495,   496,   566,   567,   643,   305,   647,   687,   688,    36,
      37,   306,   307,   416,   418,   507,   508,   580,   650,    38,
      39,    40,    41,    42,   401,   402,   576,    43,    44,   398,
     399,   487,   488,    45,    46,    47,   164,   548,    48,   423,
      49,    50,    51,    52,    53,   154,    54,    55,    56,    57,
      58,   155,   230,   231,   156,   157
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -505
static const short yypact[] =
{
    4203,   -20,    29,  -505,  3526,  -505,    25,    43,  -505,  -505,
      27,    44,  -505,    62,  -505,  -505,   701,  2912,  3934,   -10,
      33,  3526,   104,  -505,     6,  -505,  -505,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,    70,  -505,  -505,  -505,  3526,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  3526,
    3526,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,    23,  3526,  3526,  3526,  3526,  3526,  3526,  3526,  3526,
      53,  -505,  -505,  -505,  -505,  -505,     8,  -505,    91,    96,
      98,   101,   112,   129,   132,  -505,   139,   143,   145,   157,
     169,   173,   175,  1523,  -505,  1523,  1523,  3014,    20,   -18,
    -505,  3116,    83,  -505,    97,   111,  -505,    58,    69,   106,
    3628,  2595,   207,   196,   215,  3220,   228,  -505,   259,  -505,
    3526,   259,  -505,  -505,  -505,  -505,   259,   259,   249,  -505,
    -505,   216,  -505,  3526,  -505,   -16,  -505,    -5,  3526,   259,
      64,   221,    22,   239,   223,   -57,  -505,   224,  -505,  -505,
    -505,   838,  3526,  1523,  1660,    -1,  1660,  1660,   193,  1523,
    1660,  1660,  1523,  1523,  1523,  1523,   197,   199,   201,   204,
    -505,  -505,  3931,  -505,  -505,    38,   219,  -505,  -505,  1797,
      20,  -505,   536,   975,  3526,  -505,  -505,  1523,  1523,  1523,
    1523,  1523,  1523,  1523,  1523,  1523,  1523,  1523,  1523,  1523,
    1523,  1523,  1523,  1523,   344,  -505,  -505,   -33,  1523,  2595,
    2595,   230,   231,  -505,  -505,  -505,  -505,  -505,   225,  -505,
    2144,   293,   271,     0,   272,  -505,   366,  -505,   276,  -505,
    -505,  -505,  3526,  -505,  -505,   -19,  -505,  3526,  3526,  1112,
     213,  -505,   240,  -505,   348,   349,   289,  -505,  -505,  -505,
    -505,   127,     7,  -505,  -505,  -505,   243,  -505,    40,   374,
    1918,  -505,   386,   273,  1249,  -505,  4198,    59,  -505,  -505,
    3951,  4213,    80,  3526,   261,   107,   109,  -505,  3971,   114,
     120,  3659,  3693,  3722,  3994,  -505,  1386,  1523,  1523,  -505,
    3014,  -505,  2259,   253,  3629,  -505,  -505,   -57,  -505,  4213,
    -505,  -505,  -505,  4227,   237,   399,   557,   260,  -505,   260,
      94,    94,    94,    94,    55,    55,  -505,  -505,  -505,   262,
    -505,   322,   260,   225,   225,   266,  2708,  2595,  -505,  -505,
    -505,  -505,   402,  -505,  -505,  -505,  -505,   345,   259,  -505,
     259,  -505,  -505,   288,   274,  -505,   403,  3526,  -505,  -505,
      48,   -49,  -505,   291,  -505,  3322,   303,  3526,  3526,  3526,
    -505,  -505,  -505,  -505,  -505,   122,   124,    22,   279,  -505,
    -505,  -505,  -505,   324,   325,  -505,   287,   290,   292,   295,
    -505,   296,   297,   300,  1918,    20,   305,  -505,  -505,  -505,
     116,  -505,  -505,   975,   323,  3526,  -505,  1660,  -505,  -505,
     327,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  1523,  -505,
    1523,  1523,  -505,  3755,  3787,   307,  -505,  -505,  -505,  -505,
    -505,   333,   377,  -505,   435,  -505,    16,  -505,  -505,  -505,
      82,  -505,  -505,  2031,  3526,  -505,   339,   347,  -505,    63,
    3526,   249,    87,  -505,  -505,  -505,   126,   326,   749,  -505,
    -505,  -505,    16,  -505,   458,    36,  -505,   329,  -505,  -505,
     461,   463,  3526,   342,  3526,   336,  3526,   466,   338,   -43,
    1918,    87,     2,   -31,    17,    66,    87,    87,    87,    87,
     423,   343,   430,  -505,  -505,  4064,  4087,  3817,  2259,   975,
     350,   483,   372,  -505,   133,    11,   405,  -505,  -505,  -505,
    -505,  3526,   356,  -505,   485,  3526,    13,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  3322,    52,   -35,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    3424,    52,  -505,  -505,    20,   358,   359,   361,  -505,   362,
     363,  -505,  -505,  -505,   382,   383,  -505,    16,  -505,   429,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,    28,  2810,   368,  3526,  -505,  -505,
     370,  -505,    -7,   432,  -505,   481,   464,   460,  -505,  -505,
    -505,  -505,  -505,   389,   148,  -505,   507,  -505,  -505,  -505,
     150,  -505,    19,   152,  -505,   514,   326,  -505,  -505,  -505,
    -505,   500,  -505,  -505,  -505,  -505,  -505,  -505,    87,    87,
     159,  -505,  -505,    16,  -505,  -505,  -505,   394,  -505,  -505,
     396,  1660,  3526,   510,   471,  2374,   467,  2374,   468,    16,
    -505,  3526,   401,  -505,  -505,    52,   418,   408,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,   161,  2810,  -505,   164,  2374,
     543,  -505,  -505,   411,   524,  -505,   527,    97,  -505,  -505,
    -505,  -505,  -505,   546,   514,  -505,  -505,   417,   118,   421,
    -505,   424,  2374,  2374,  2482,   249,  -505,  -505,   427,  -505,
    -505,  2374,   559,  -505,   421,   431,   421,  -505,  1660,  -505,
    -505,   433,   166,  -505,   434,  -505
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -505,  -505,  -505,    -8,    -4,  -505,   379,  -505,   254,  -505,
    -505,  -124,  -505,  -505,  -505,   102,    35,   346,  -505,  -505,
       5,  -505,  -284,  -395,  -505,  -102,  -505,  -408,  -182,  -476,
    -470,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -504,
    -154,  -129,  -505,  -489,  -505,  -505,    99,  -505,  -101,   205,
    -505,  -203,   135,  -505,   250,    51,  -505,  -157,  -505,  -505,
    -505,   217,  -505,    -6,   214,  -505,  -505,  -505,  -505,  -505,
      30,  -505,  -505,   -54,  -505,  -412,  -505,  -505,  -119,  -505,
    -505,  -505,   179,  -505,  -505,  -505,    21,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,   117,  -505,  -505,  -505,  -505,
    -505,  -505,    54,  -505,  -505,  -505,   588,  -505,  -505,  -505,
    -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,  -505,
    -505,  -240,  -505,   378,  -505,   369
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -412
static const short yytable[] =
{
     111,   322,   560,   325,   326,   505,   544,   329,   330,   163,
     303,   596,   297,   558,   279,   412,   562,   184,   297,   280,
     281,   297,   297,   298,   297,   600,   185,   297,   298,   232,
     299,   300,   293,   227,   228,   189,   300,   662,   323,   519,
     672,   645,   370,   630,   598,   595,   396,   403,   602,   605,
     607,   609,   611,   613,   579,   190,   191,   297,   298,   310,
     299,   227,   228,   473,   419,   276,   300,   310,   297,   298,
     441,   297,   298,   112,   603,   115,   520,   300,   287,   294,
     300,   288,    59,  -397,   420,    60,   640,   289,   384,   490,
     114,   290,   297,   298,   592,   237,   182,   397,   421,   -65,
     371,   300,   599,   646,   186,   113,   295,   116,   193,   194,
     184,   196,   197,   198,   199,   200,   389,   491,   119,   195,
     233,   631,   288,   237,   183,   593,   238,   660,   117,   324,
     188,   233,   410,   411,   237,   118,   382,   729,   597,   521,
     422,   187,   382,   224,   413,   382,   301,   235,   382,   202,
     639,   301,   262,   644,   238,   730,   413,   267,   302,   192,
     467,   274,   229,   254,   663,   238,   278,   545,   546,   651,
     489,   522,   257,  -330,  -330,   547,   340,   523,   341,   286,
     258,   301,   691,   693,   292,   251,   252,   253,   642,   201,
     229,   259,   301,   695,   441,   301,   446,   447,   319,   556,
     260,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   301,   449,   447,   734,
     736,   152,   249,   250,   251,   252,   253,   203,   184,   165,
     350,   481,   204,   482,   205,   303,   737,   206,   524,   525,
     526,   527,   528,   529,   452,   447,   453,   447,   207,   530,
     403,   455,   447,   373,   374,   267,   267,   456,   447,   501,
     502,   503,   504,   563,   564,   208,   383,   702,   209,   170,
     629,   502,   585,   712,   587,   210,   589,   237,   393,   211,
     441,   212,   171,   400,   404,   680,   681,   683,   502,   684,
     685,   718,   172,   213,   173,   174,   694,   502,   715,   502,
     237,   717,   447,   744,   447,   214,   435,   557,   238,   215,
     561,   216,   176,  -380,   733,   718,   718,   178,   270,   450,
     577,   271,   275,   718,   276,   282,   296,   179,   308,   285,
     327,   238,   309,   312,   335,   336,   224,   337,   466,   594,
     338,   342,   601,   604,   606,   608,   610,   612,   220,   369,
     221,   222,   375,   376,   377,   387,   467,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     388,   390,   383,   267,   391,   392,   405,   670,   407,   408,
     409,   417,   424,   303,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   486,   442,  -411,   469,   475,   451,   303,
     474,   494,   476,   498,   499,   500,   316,   410,   320,   480,
     483,   485,   484,   492,   328,   497,   509,   331,   332,   333,
     334,   510,   511,   512,   520,   539,   513,   541,   514,   538,
     435,   515,   516,   517,   344,   441,   518,   441,   349,   237,
     543,   532,   351,   352,   353,   354,   355,   356,   357,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   441,
     531,   554,   565,   372,   534,   555,   578,   581,   698,   582,
     238,   583,   586,   588,   590,   591,   690,   692,   614,   552,
     393,   615,   441,   441,   441,   617,   400,   623,   624,   626,
     632,   441,   636,   637,   349,   653,   654,   709,   655,   656,
     657,   658,   659,   303,   661,   669,   671,   675,   584,   673,
     584,   679,   584,   677,   676,   682,   435,   686,   579,   222,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   696,   697,   466,   742,   699,   700,   711,    61,
     713,   316,   463,   464,   704,   706,   714,   633,   721,   722,
     723,   486,    62,   724,   726,   728,    63,    64,    65,   731,
     494,    66,   732,   738,   740,    67,   291,   741,   444,    68,
     743,   745,   716,   540,   622,   705,   649,   739,   725,   348,
     710,    69,   533,   553,    70,    71,    72,    73,    74,   621,
     465,   479,   689,   478,   641,   727,   506,   237,    75,    76,
      77,    78,   652,    79,    80,    81,   181,   559,   345,   638,
      82,   666,   358,   584,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,     0,    91,    92,    93,   238,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
     101,     0,   102,     0,   103,   104,   105,     0,   349,   106,
     107,   108,     0,   109,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,     0,   536,   537,     0,   184,     0,
       0,   435,   346,   435,     0,     0,     0,   633,     0,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
       0,     0,   666,     0,     0,   435,     0,     0,     0,     0,
       0,     0,     0,     0,   120,   121,   122,   123,     0,   124,
     125,   126,     0,     0,     0,   127,     0,    62,   435,   435,
     435,    63,   128,    65,     0,   129,    66,   435,     0,     0,
      67,     0,     0,     0,   130,     0,     0,     0,     0,     0,
       0,     0,   131,     0,   349,   132,    69,     0,     0,    70,
      71,    72,   133,   134,     0,   135,   136,     0,     0,     0,
     137,     0,     0,    75,    76,    77,    78,     0,    79,   138,
      81,   139,     0,   568,   569,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,   140,
      91,    92,    93,   570,    94,     0,    95,     0,     0,    96,
      97,    98,   141,     0,   100,   101,     0,   102,   571,   103,
     104,   105,   572,     0,   106,   107,   142,     0,   109,     0,
       0,   573,   574,     0,     0,     0,     0,     0,     0,     0,
     143,   144,     0,     0,   145,     0,     0,   146,     0,     0,
     147,   217,   121,   122,   123,     0,   313,   125,     0,   575,
       0,     0,   127,     0,    62,     0,     0,     0,    63,    64,
      65,     0,   129,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,   131,
       0,     0,   132,    69,     0,     0,    70,    71,    72,    73,
      74,     0,   135,   136,     0,     0,     0,   137,     0,     0,
      75,    76,    77,    78,     0,    79,   218,    81,   219,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,   140,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,   101,     0,   102,     0,   103,   104,   105,     0,
       0,   106,   107,   142,     0,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,     0,     0,
       0,   145,     0,     0,   314,   315,     0,   147,   217,   121,
     122,   123,     0,   124,   125,     0,     0,     0,     0,   127,
       0,    62,     0,     0,     0,    63,   128,    65,     0,   129,
      66,     0,     0,     0,    67,     0,     0,     0,   130,     0,
       0,     0,     0,     0,     0,     0,   131,     0,     0,   132,
      69,     0,     0,    70,    71,    72,   133,   134,     0,   135,
     136,     0,     0,     0,   137,     0,     0,    75,    76,    77,
      78,     0,    79,   138,    81,   139,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,   140,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,   141,     0,   100,   101,
       0,   102,     0,   103,   104,   105,     0,     0,   106,   107,
     142,     0,   109,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,     0,     0,   145,     0,
       0,   146,     0,     0,   147,   120,   121,   122,   123,     0,
     124,   125,     0,     0,     0,     0,   127,     0,    62,     0,
       0,     0,    63,   128,    65,     0,   129,    66,     0,     0,
       0,    67,     0,     0,     0,   130,     0,     0,     0,     0,
       0,     0,     0,   131,     0,     0,   132,    69,     0,     0,
      70,    71,    72,   133,   134,     0,   135,   136,     0,     0,
       0,   137,     0,     0,    75,    76,    77,    78,     0,    79,
     138,    81,   139,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
     140,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,   141,     0,   100,   101,     0,   102,     0,
     103,   104,   105,     0,     0,   106,   107,   142,     0,   109,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,     0,     0,   145,     0,     0,   146,     0,
       0,   147,   217,   121,   122,   123,     0,   124,   125,     0,
       0,     0,     0,   127,     0,    62,     0,     0,     0,    63,
      64,    65,     0,   129,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
     131,     0,     0,   132,    69,     0,     0,    70,    71,    72,
      73,    74,     0,   135,   136,     0,     0,     0,   137,     0,
       0,    75,    76,    77,    78,     0,    79,   218,    81,   219,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,   140,    91,    92,
      93,   443,    94,     0,    95,     0,     0,    96,    97,    98,
      99,     0,   100,   101,     0,   102,     0,   103,   104,   105,
       0,     0,   106,   107,   142,     0,   109,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,     0,
       0,     0,   145,     0,     0,   146,     0,     0,   147,   217,
     121,   122,   123,     0,   313,   125,     0,     0,     0,     0,
     127,     0,    62,     0,     0,     0,    63,    64,    65,     0,
     129,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,   131,     0,     0,
     132,    69,     0,     0,    70,    71,    72,    73,    74,     0,
     135,   136,     0,     0,     0,   137,     0,     0,    75,    76,
      77,    78,     0,    79,   218,    81,   219,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,   140,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
     101,     0,   102,     0,   103,   104,   105,     0,     0,   106,
     107,   142,     0,   109,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   143,     0,     0,     0,   145,
       0,     0,   146,   315,     0,   147,   217,   121,   122,   123,
       0,   124,   125,     0,     0,     0,     0,   127,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   129,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,   131,     0,     0,   132,    69,     0,
       0,    70,    71,    72,    73,    74,     0,   135,   136,     0,
       0,     0,   137,     0,     0,    75,    76,    77,    78,     0,
      79,   218,    81,   219,     0,     0,     0,    82,     0,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
       0,   140,    91,    92,    93,     0,    94,     0,    95,     0,
       0,    96,    97,    98,    99,     0,   100,   101,     0,   102,
       0,   103,   104,   105,     0,     0,   106,   107,   142,     0,
     109,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   143,     0,     0,     0,   145,     0,     0,   146,
       0,     0,   147,   217,   121,   122,   123,     0,   313,   125,
       0,     0,     0,     0,   127,     0,    62,     0,     0,     0,
      63,    64,    65,     0,   129,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,   132,    69,     0,     0,    70,    71,
      72,    73,    74,     0,   135,   136,     0,     0,     0,   137,
       0,     0,    75,    76,    77,    78,     0,    79,   218,    81,
     219,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,   140,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,   101,     0,   102,     0,   103,   104,
     105,     0,     0,   106,   107,   142,     0,   109,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   143,
       0,     0,     0,   145,     0,     0,   146,     0,     0,   147,
     217,   121,   122,   123,     0,   343,   125,     0,     0,     0,
       0,   127,     0,    62,     0,     0,     0,    63,    64,    65,
       0,   129,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,   131,     0,
       0,   132,    69,     0,     0,    70,    71,    72,    73,    74,
       0,   135,   136,     0,     0,     0,   137,     0,     0,    75,
      76,    77,    78,     0,    79,   218,    81,   219,     0,     0,
       0,    82,     0,     0,     0,     0,    83,    84,    85,    86,
      87,    88,    89,    90,     0,   140,    91,    92,    93,     0,
      94,     0,    95,     0,     0,    96,    97,    98,    99,     0,
     100,   101,     0,   102,     0,   103,   104,   105,     0,     0,
     106,   107,   142,     0,   109,     0,     0,     0,     0,     0,
       0,    61,   425,     0,     0,     0,   143,     0,     0,     0,
     145,     0,     0,   146,    62,     0,   147,     0,    63,    64,
      65,     0,   426,    66,     0,     0,     0,    67,     0,     0,
       0,   427,     0,     0,     0,     0,     0,     0,     0,   428,
       0,     0,     0,    69,     0,     0,    70,    71,    72,   429,
      74,     0,   430,     0,     0,     0,     0,   431,     0,     0,
      75,    76,    77,    78,     0,   432,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,   101,     0,   102,     0,   103,   104,   105,     0,
       0,   106,   107,   433,    61,   109,   549,     0,     0,   550,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,    63,    64,    65,   434,     0,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,     0,
      91,    92,    93,     0,    94,     0,    95,     0,     0,    96,
      97,    98,    99,     0,   100,   101,     0,   102,     0,   103,
     104,   105,     0,     0,   106,   107,   108,    61,   109,   297,
       0,     0,   378,     0,     0,     0,     0,     0,     0,     0,
      62,     0,     0,     0,    63,    64,    65,   551,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   379,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,    82,   380,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,     0,
     102,   381,   103,   104,   105,     0,     0,   106,   107,   108,
       0,   109,    61,     0,   297,     0,     0,     0,     0,     0,
       0,     0,     0,   382,     0,    62,     0,     0,     0,    63,
      64,    65,     0,     0,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,    97,    98,
      99,     0,   100,   101,     0,   102,     0,   103,   104,   105,
       0,     0,   106,   107,   108,     0,   109,    61,   425,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   382,     0,
      62,     0,     0,     0,    63,    64,    65,     0,   426,    66,
       0,     0,     0,    67,     0,     0,     0,   427,     0,     0,
       0,     0,     0,     0,     0,   428,     0,     0,     0,    69,
       0,     0,    70,    71,    72,   429,    74,     0,   430,     0,
       0,     0,     0,   431,     0,     0,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,     0,
     102,     0,   103,   104,   105,    61,   425,   106,   107,   433,
       0,   109,     0,     0,     0,     0,     0,     0,    62,     0,
       0,     0,    63,    64,    65,     0,   426,    66,     0,     0,
       0,    67,     0,     0,     0,   427,     0,     0,     0,     0,
       0,     0,     0,   428,     0,     0,     0,    69,     0,     0,
      70,    71,    72,   429,    74,     0,   430,     0,     0,     0,
       0,   431,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,   735,    88,    89,    90,     0,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,   101,     0,   102,     0,
     103,   104,   105,     0,     0,   106,   107,   433,    61,   109,
     263,   264,     0,   265,     0,     0,     0,     0,     0,     0,
       0,    62,     0,     0,     0,    63,    64,    65,     0,     0,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,     0,     0,    82,
     266,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,    99,     0,   100,   101,
       0,   102,     0,   103,   104,   105,     0,     0,   106,   107,
     108,    61,   109,   477,     0,     0,   378,     0,     0,     0,
       0,     0,     0,     0,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,   101,    61,   102,   664,   103,   104,   105,     0,
       0,   106,   107,   108,     0,   109,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,   665,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,   101,    61,   102,     0,   103,   104,
     105,     0,   158,   106,   107,   108,     0,   109,    62,     0,
       0,     0,    63,    64,    65,     0,     0,    66,     0,     0,
     159,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,   160,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,   161,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
       0,    91,    92,    93,     0,    94,     0,   162,     0,     0,
      96,    97,    98,    99,     0,   100,   101,    61,     0,     0,
     103,   104,   105,     0,     0,   106,   107,   108,     0,   109,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,     0,   223,
       0,     0,     0,     0,     0,     0,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,    61,
     102,     0,   103,   104,   105,     0,     0,   106,   107,   108,
       0,   109,    62,     0,   234,     0,    63,    64,    65,     0,
       0,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
     101,     0,   102,    61,   103,   104,   105,     0,   272,   106,
     107,   108,     0,   109,     0,     0,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,     0,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,   273,
      98,    99,     0,   100,   101,    61,   102,     0,   103,   104,
     105,     0,     0,   106,   107,   108,     0,   109,    62,     0,
       0,     0,    63,    64,    65,     0,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,   493,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,   101,    61,   102,     0,
     103,   104,   105,     0,     0,   106,   107,   108,     0,   109,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,    76,    77,    78,
     648,    79,    80,    81,     0,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,    61,
     102,     0,   103,   104,   105,     0,     0,   106,   107,   108,
       0,   109,    62,     0,     0,     0,    63,    64,    65,     0,
       0,    66,     0,     0,     0,    67,     0,     0,     0,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
     101,    61,   102,     0,   103,   104,   105,   261,     0,   106,
     107,   108,     0,   109,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,   237,
     238,     0,     0,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
     238,   100,   101,   237,  -379,     0,   103,   104,   105,     0,
       0,   106,   107,   108,     0,   109,     0,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   237,     0,   238,     0,     0,     0,     0,     0,
       0,     0,   470,     0,     0,     0,     0,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,     0,   238,     0,   237,   457,   458,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   238,   237,     0,     0,
     459,   460,     0,     0,     0,     0,     0,     0,     0,     0,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,     0,     0,   237,   238,     0,
     461,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   238,     0,
       0,     0,     0,   458,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
       0,     0,     0,     0,     0,   460,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     165,     0,     0,     0,     0,   620,     0,     0,     0,     0,
       0,     0,   166,   167,     0,     0,     0,     0,     0,   168,
       0,   237,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   169,     0,     0,     0,     0,     0,     0,
     170,   237,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   238,   171,     0,     0,     0,     0,     0,     0,
       0,   237,     0,   172,     0,   173,   174,     0,     0,     0,
       0,     0,   238,     0,     0,     0,     0,     0,     0,   175,
       0,     0,     0,   176,   237,     0,     0,   177,   178,     0,
       0,     0,   238,     0,     0,     0,     0,     0,   179,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,     0,   238,     0,     0,   339,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,     0,     0,     0,     0,   448,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   237,     0,     0,     0,   454,     0,
       0,     0,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   237,     0,     0,
       0,   462,     0,     0,     0,   238,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   238,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,     0,     0,     0,
       0,   618,     0,     0,     0,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
       1,     0,     0,     2,   619,     3,     0,     0,     0,     0,
       4,     0,     0,     0,     5,     0,     0,     6,   237,     7,
       8,     9,     0,     0,   445,    10,     0,     0,    11,     0,
       0,     0,     0,   237,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,   237,     0,   238,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,     0,     0,   238,     0,     0,     0,     0,     0,
      14,     0,     0,    15,     0,    16,     0,    17,   238,    18,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    20,     0,     0,    21,     0,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253
};

static const short yycheck[] =
{
       4,   204,   491,   206,   207,   417,   476,   210,   211,    17,
     192,     9,     5,   489,   171,     8,   492,    21,     5,   176,
     177,     5,     5,     6,     5,     8,    21,     5,     6,    47,
       8,    14,   189,    13,    14,    39,    14,     9,    39,   434,
      47,    76,    75,    32,    75,   521,    65,   287,   524,   525,
     526,   527,   528,   529,    18,    59,    60,     5,     6,   116,
       8,    13,    14,   347,    24,    65,    14,   116,     5,     6,
     310,     5,     6,    48,     8,    48,   119,    14,    94,    15,
      14,   138,   102,     0,    44,    56,   556,    92,   270,   138,
      47,    96,     5,     6,   137,    40,   106,   116,    58,    99,
     133,    14,   133,   138,     0,    80,    42,    80,   112,   113,
     114,   115,   116,   117,   118,   119,   273,   401,    56,   114,
     138,   110,   138,    40,    91,   520,    71,   597,    84,   130,
      60,   138,     5,     6,    40,    91,   129,    19,   136,    23,
     100,   135,   129,   147,   137,   129,   129,   151,   129,   141,
     137,   129,   160,   565,    71,    37,   137,   161,   136,   136,
     342,   165,   142,    66,   136,    71,   170,    85,    86,   581,
     122,    55,    61,   137,   138,    93,   138,    61,   140,   183,
     122,   129,   658,   659,   188,   130,   131,   132,   136,   136,
     142,   122,   129,   663,   434,   129,   137,   138,   202,   136,
      94,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   129,   137,   138,   723,
     724,    16,   128,   129,   130,   131,   132,   136,   232,    16,
     234,   388,   136,   390,   136,   417,   725,   136,   122,   123,
     124,   125,   126,   127,   137,   138,   137,   138,   136,   133,
     490,   137,   138,   259,   260,   259,   260,   137,   138,   137,
     138,   137,   138,   137,   138,   136,   270,   675,   136,    56,
     137,   138,   512,   685,   514,   136,   516,    40,   282,   136,
     520,   136,    69,   287,   288,   137,   138,   137,   138,   137,
     138,   699,    79,   136,    81,    82,   137,   138,   137,   138,
      40,   137,   138,   137,   138,   136,   310,   489,    71,   136,
     492,   136,    99,   106,   722,   723,   724,   104,   122,   323,
     502,   106,    94,   731,    65,    76,   105,   114,    89,   113,
     137,    71,   109,   109,   137,   136,   340,   136,   342,   521,
     136,   122,   524,   525,   526,   527,   528,   529,   143,     5,
     145,   146,   122,   122,   129,    62,   538,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
      99,    99,   376,   377,     8,    99,   136,   617,    30,    30,
      91,   138,     8,   565,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   397,     8,   122,   143,    75,   137,   581,
     138,   405,   136,   407,   408,   409,   201,     5,   203,    64,
     122,     8,   138,   122,   209,   112,   137,   212,   213,   214,
     215,    97,    97,   136,   119,    92,   136,    50,   136,   122,
     434,   136,   136,   136,   229,   675,   136,   677,   233,    40,
       5,   445,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   699,
     137,   122,   136,   258,   137,   118,     8,   138,   671,     8,
      71,     8,   130,   137,     8,   137,   658,   659,    55,   483,
     484,   138,   722,   723,   724,    55,   490,   137,     5,   117,
      85,   731,   136,     8,   289,   137,   137,   679,   137,   137,
     137,   119,   119,   685,    75,   137,   136,    26,   512,    77,
     514,   122,   516,    53,    50,     8,   520,     3,    18,   314,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   138,   137,   538,   738,    26,    66,   137,     3,
     122,   336,   337,   338,    77,    77,   138,   551,     5,   138,
      26,   555,    16,    26,     8,   138,    20,    21,    22,   138,
     564,    25,   138,   136,     5,    29,   187,   136,   314,    33,
     137,   137,   696,   471,   539,   677,   580,   731,   707,   233,
     681,    45,   447,   484,    48,    49,    50,    51,    52,   538,
     340,   377,   646,   376,   564,   714,   417,    40,    62,    63,
      64,    65,   581,    67,    68,    69,    18,   490,   230,   555,
      74,   615,   243,   617,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    89,    90,    91,    71,    93,
      -1,    95,    -1,    -1,    98,    99,   100,   101,    -1,   103,
     104,    -1,   106,    -1,   108,   109,   110,    -1,   443,   113,
     114,   115,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   458,    -1,   460,   461,    -1,   672,    -1,
      -1,   675,   136,   677,    -1,    -1,    -1,   681,    -1,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
      -1,    -1,   696,    -1,    -1,   699,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    10,    -1,    -1,    -1,    14,    -1,    16,   722,   723,
     724,    20,    21,    22,    -1,    24,    25,   731,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,   539,    44,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    24,    25,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      89,    90,    91,    44,    93,    -1,    95,    -1,    -1,    98,
      99,   100,   101,    -1,   103,   104,    -1,   106,    59,   108,
     109,   110,    63,    -1,   113,   114,   115,    -1,   117,    -1,
      -1,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,    -1,    -1,   133,    -1,    -1,   136,    -1,    -1,
     139,     3,     4,     5,     6,    -1,     8,     9,    -1,   100,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    44,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,   100,   101,
      -1,   103,   104,    -1,   106,    -1,   108,   109,   110,    -1,
      -1,   113,   114,   115,    -1,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,
      -1,   133,    -1,    -1,   136,   137,    -1,   139,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    44,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    54,
      55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    99,   100,   101,    -1,   103,   104,
      -1,   106,    -1,   108,   109,   110,    -1,    -1,   113,   114,
     115,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,    -1,   133,    -1,
      -1,   136,    -1,    -1,   139,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    54,    55,    -1,    -1,
      -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      88,    89,    90,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,   100,   101,    -1,   103,   104,    -1,   106,    -1,
     108,   109,   110,    -1,    -1,   113,   114,   115,    -1,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,    -1,   133,    -1,    -1,   136,    -1,
      -1,   139,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    44,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    54,    55,    -1,    -1,    -1,    59,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    89,    90,
      91,    92,    93,    -1,    95,    -1,    -1,    98,    99,   100,
     101,    -1,   103,   104,    -1,   106,    -1,   108,   109,   110,
      -1,    -1,   113,   114,   115,    -1,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,    -1,
      -1,    -1,   133,    -1,    -1,   136,    -1,    -1,   139,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,   100,   101,    -1,   103,
     104,    -1,   106,    -1,   108,   109,   110,    -1,    -1,   113,
     114,   115,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,   133,
      -1,    -1,   136,   137,    -1,   139,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    44,    45,    -1,
      -1,    48,    49,    50,    51,    52,    -1,    54,    55,    -1,
      -1,    -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    99,   100,   101,    -1,   103,   104,    -1,   106,
      -1,   108,   109,   110,    -1,    -1,   113,   114,   115,    -1,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,    -1,    -1,    -1,   133,    -1,    -1,   136,
      -1,    -1,   139,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    -1,    -1,    44,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    55,    -1,    -1,    -1,    59,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    88,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
     100,   101,    -1,   103,   104,    -1,   106,    -1,   108,   109,
     110,    -1,    -1,   113,   114,   115,    -1,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
      -1,    -1,    -1,   133,    -1,    -1,   136,    -1,    -1,   139,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    44,    45,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    99,   100,   101,    -1,
     103,   104,    -1,   106,    -1,   108,   109,   110,    -1,    -1,
     113,   114,   115,    -1,   117,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,    -1,    -1,   129,    -1,    -1,    -1,
     133,    -1,    -1,   136,    16,    -1,   139,    -1,    20,    21,
      22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,   100,   101,
      -1,   103,   104,    -1,   106,    -1,   108,   109,   110,    -1,
      -1,   113,   114,   115,     3,   117,     5,    -1,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,   136,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    -1,    98,
      99,   100,   101,    -1,   103,   104,    -1,   106,    -1,   108,
     109,   110,    -1,    -1,   113,   114,   115,     3,   117,     5,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,   136,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,   100,   101,    -1,   103,   104,    -1,
     106,   107,   108,   109,   110,    -1,    -1,   113,   114,   115,
      -1,   117,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,   100,
     101,    -1,   103,   104,    -1,   106,    -1,   108,   109,   110,
      -1,    -1,   113,   114,   115,    -1,   117,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    59,    -1,    -1,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,   100,   101,    -1,   103,   104,    -1,
     106,    -1,   108,   109,   110,     3,     4,   113,   114,   115,
      -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    59,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    89,    90,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,   100,   101,    -1,   103,   104,    -1,   106,    -1,
     108,   109,   110,    -1,    -1,   113,   114,   115,     3,   117,
       5,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,    99,   100,   101,    -1,   103,   104,
      -1,   106,    -1,   108,   109,   110,    -1,    -1,   113,   114,
     115,     3,   117,     5,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,   100,   101,
      -1,   103,   104,     3,   106,     5,   108,   109,   110,    -1,
      -1,   113,   114,   115,    -1,   117,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
     100,   101,    -1,   103,   104,     3,   106,    -1,   108,   109,
     110,    -1,    10,   113,   114,   115,    -1,   117,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      28,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    89,    90,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,   100,   101,    -1,   103,   104,     3,    -1,    -1,
     108,   109,   110,    -1,    -1,   113,   114,   115,    -1,   117,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,   100,   101,    -1,   103,   104,     3,
     106,    -1,   108,   109,   110,    -1,    -1,   113,   114,   115,
      -1,   117,    16,    -1,    18,    -1,    20,    21,    22,    -1,
      -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,   100,   101,    -1,   103,
     104,    -1,   106,     3,   108,   109,   110,    -1,     8,   113,
     114,   115,    -1,   117,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
     100,   101,    -1,   103,   104,     3,   106,    -1,   108,   109,
     110,    -1,    -1,   113,   114,   115,    -1,   117,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    89,    90,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,   100,   101,    -1,   103,   104,     3,   106,    -1,
     108,   109,   110,    -1,    -1,   113,   114,   115,    -1,   117,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,   100,   101,    -1,   103,   104,     3,
     106,    -1,   108,   109,   110,    -1,    -1,   113,   114,   115,
      -1,   117,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,   100,   101,    -1,   103,
     104,     3,   106,    -1,   108,   109,   110,     9,    -1,   113,
     114,   115,    -1,   117,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    45,    -1,    -1,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    40,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,   100,   101,
      71,   103,   104,    40,   106,    -1,   108,   109,   110,    -1,
      -1,   113,   114,   115,    -1,   117,    -1,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,    40,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,    -1,    71,    -1,    40,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,    71,    40,    -1,    -1,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,    -1,    -1,    40,    71,    -1,
     138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,    71,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
      16,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    35,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    79,    -1,    81,    82,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      -1,    -1,    -1,    99,    40,    -1,    -1,   103,   104,    -1,
      -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,   114,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,    -1,    71,    -1,    -1,   137,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,   137,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,    40,    -1,    -1,    -1,   137,    -1,
      -1,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,    40,    -1,    -1,
      -1,   137,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,   137,    -1,    -1,    -1,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
      17,    -1,    -1,    20,   137,    22,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    31,    -1,    -1,    34,    40,    36,
      37,    38,    -1,    -1,    46,    42,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    90,    -1,    92,    -1,    94,    71,    96,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,   111,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    78,    87,    90,    92,    94,    96,    98,
     108,   111,   145,   146,   149,   150,   151,   152,   159,   200,
     203,   204,   209,   210,   211,   212,   223,   224,   233,   234,
     235,   236,   237,   241,   242,   247,   248,   249,   252,   254,
     255,   256,   257,   258,   260,   261,   262,   263,   264,   102,
      56,     3,    16,    20,    21,    22,    25,    29,    33,    45,
      48,    49,    50,    51,    52,    62,    63,    64,    65,    67,
      68,    69,    74,    79,    80,    81,    82,    83,    84,    85,
      86,    89,    90,    91,    93,    95,    98,    99,   100,   101,
     103,   104,   106,   108,   109,   110,   113,   114,   115,   117,
     147,   148,    48,    80,    47,    48,    80,    84,    91,    56,
       3,     4,     5,     6,     8,     9,    10,    14,    21,    24,
      33,    41,    44,    51,    52,    54,    55,    59,    68,    70,
      88,   101,   115,   129,   130,   133,   136,   139,   148,   160,
     161,   163,   193,   194,   259,   265,   268,   269,    10,    28,
      49,    74,    95,   147,   250,    16,    28,    29,    35,    49,
      56,    69,    79,    81,    82,    95,    99,   103,   104,   114,
     202,   250,   106,    91,   148,   164,     0,   135,    60,   148,
     148,   148,   136,   148,   148,   164,   148,   148,   148,   148,
     148,   136,   141,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,     3,    68,    70,
     193,   193,   193,    55,   148,   197,   198,    13,    14,   142,
     266,   267,    47,   138,    18,   148,   162,    40,    71,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,    66,   185,   186,    61,   122,   122,
      94,     9,   147,     5,     6,     8,    75,   148,   207,   208,
     122,   106,     8,    99,   148,    94,    65,   201,   148,   201,
     201,   201,    76,   187,   188,   113,   148,    94,   138,    92,
      96,   150,   148,   201,    15,    42,   105,     5,     6,     8,
      14,   129,   136,   172,   173,   219,   225,   226,    89,   109,
     116,   166,   109,     8,   136,   137,   193,   195,   196,   148,
     193,   193,   195,    39,   130,   195,   195,   137,   193,   195,
     195,   193,   193,   193,   193,   137,   136,   136,   136,   137,
     138,   140,   122,     8,   193,   267,   136,   164,   161,   193,
     148,   193,   193,   193,   193,   193,   193,   193,   269,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,     5,
      75,   133,   193,   207,   207,   122,   122,   129,     8,    43,
      75,   107,   129,   148,   172,   205,   206,    62,    99,   201,
      99,     8,    99,   148,   189,   190,    65,   116,   243,   244,
     148,   238,   239,   265,   148,   136,   213,    30,    30,    91,
       5,     6,     8,   137,   172,   174,   227,   138,   228,    24,
      44,    58,   100,   253,     8,     4,    24,    33,    41,    51,
      54,    59,    67,   115,   136,   148,   167,   168,   169,   170,
     171,   265,     8,    92,   152,    46,   137,   138,   137,   137,
     148,   137,   137,   137,   137,   137,   137,   137,   138,   137,
     138,   138,   137,   193,   193,   198,   148,   172,   199,   143,
     143,   156,   165,   166,   138,    75,   136,     5,   205,   208,
      64,   201,   201,   122,   138,     8,   148,   245,   246,   122,
     138,   166,   122,    54,   148,   214,   215,   112,   148,   148,
     148,   137,   138,   137,   138,   219,   226,   229,   230,   137,
      97,    97,   136,   136,   136,   136,   136,   136,   136,   167,
     119,    23,    55,    61,   122,   123,   124,   125,   126,   127,
     133,   137,   148,   196,   137,   193,   193,   193,   122,    92,
     159,    50,   175,     5,   174,    85,    86,    93,   251,     5,
       8,   136,   148,   190,   122,   118,   136,   172,   173,   239,
     187,   172,   173,   137,   138,   136,   216,   217,    24,    25,
      44,    59,    63,    72,    73,   100,   240,   172,     8,    18,
     231,   138,     8,     8,   148,   265,   130,   265,   137,   265,
       8,   137,   137,   167,   172,   173,     9,   136,    75,   133,
       8,   172,   173,     8,   172,   173,   172,   173,   172,   173,
     172,   173,   172,   173,    55,   138,   153,    55,   137,   137,
     138,   199,   160,   137,     5,   176,   117,   179,   180,   137,
      32,   110,    85,   148,   191,   192,   136,     8,   246,   137,
     174,   214,   136,   218,   219,    76,   138,   220,    66,   148,
     232,   219,   230,   137,   137,   137,   137,   137,   119,   119,
     174,    75,     9,   136,     5,    54,   148,   154,   155,   137,
     265,   136,    47,    77,   157,    26,    50,    53,   178,   122,
     137,   138,     8,   137,   137,   138,     3,   221,   222,   217,
     172,   173,   172,   173,   137,   174,   138,   137,   195,    26,
      66,   158,   171,   177,    77,   169,    77,   181,   182,   172,
     192,   137,   219,   122,   138,   137,   155,   137,   171,   183,
     184,     5,   138,    26,    26,   185,     8,   222,   138,    19,
      37,   138,   138,   171,   183,    83,   183,   187,   136,   184,
       5,   136,   195,   137,   137,   137
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

  case 81:

    { pParser->PushQuery(); ;}
    break;

  case 82:

    { pParser->PushQuery(); ;}
    break;

  case 86:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 88:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 91:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 92:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 96:

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

  case 97:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 99:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 100:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 101:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 104:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 107:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 108:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 109:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 110:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 111:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 112:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 113:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 114:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 115:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 116:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 117:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 119:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 126:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 128:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 129:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 130:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 131:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 132:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 133:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 135:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 136:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 154:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 155:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 156:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 157:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 158:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 163:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 164:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 165:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 166:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 167:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 168:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 169:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 173:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 174:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 175:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 177:

    {
			pParser->AddHaving();
		;}
    break;

  case 180:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 183:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 184:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 186:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 188:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 189:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 192:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 193:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 201:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 202:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 205:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 206:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 208:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 209:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 214:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 215:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 234:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 238:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
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

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 247:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 248:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 249:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 250:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 251:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 256:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 257:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 264:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 265:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 266:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 267:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 268:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 269:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 270:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 271:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 272:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 273:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 274:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 275:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 276:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 277:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 278:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 279:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 280:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 281:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 282:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 283:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 284:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 287:

    { yyval.m_iValue = 1; ;}
    break;

  case 288:

    { yyval.m_iValue = 0; ;}
    break;

  case 289:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 297:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 298:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 299:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 302:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 303:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 304:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 309:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 310:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 313:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 314:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 315:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 316:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 317:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 318:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 319:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 320:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 325:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 326:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 327:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 328:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 329:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 331:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 332:

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

  case 333:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 336:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 338:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 343:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 346:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 347:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 348:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 351:

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

  case 352:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 353:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 354:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 355:

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

  case 356:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 357:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 358:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 359:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 360:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 361:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 362:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 363:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 364:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 365:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 366:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 367:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 368:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 375:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 376:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 377:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 385:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 386:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 387:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 388:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 389:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 390:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 391:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 392:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 393:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 394:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 397:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 398:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 399:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 400:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 401:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 402:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 404:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 405:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 406:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 407:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 408:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 409:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 410:

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

