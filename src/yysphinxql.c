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
#define YYFINAL  131
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1418

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  140
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  120
/* YYNRULES -- Number of rules. */
#define YYNRULES  352
/* YYNRULES -- Number of states. */
#define YYNSTATES  665

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
      60,    62,    64,    68,    70,    72,    74,    83,    85,    95,
      96,    99,   101,   105,   107,   109,   111,   112,   116,   117,
     120,   125,   137,   139,   143,   145,   148,   149,   151,   154,
     156,   161,   166,   171,   176,   181,   186,   190,   196,   198,
     202,   203,   205,   208,   210,   214,   218,   223,   225,   229,
     233,   239,   246,   250,   255,   261,   265,   269,   273,   277,
     281,   283,   289,   295,   301,   305,   309,   313,   317,   321,
     325,   329,   334,   338,   340,   342,   347,   351,   355,   357,
     359,   364,   369,   374,   376,   379,   381,   384,   386,   388,
     392,   393,   398,   399,   401,   403,   407,   408,   411,   412,
     414,   420,   421,   423,   427,   433,   435,   439,   441,   444,
     447,   448,   450,   453,   458,   459,   461,   464,   466,   470,
     474,   478,   482,   488,   495,   502,   506,   510,   512,   516,
     520,   522,   524,   526,   528,   530,   532,   534,   537,   540,
     544,   548,   552,   556,   560,   564,   568,   572,   576,   580,
     584,   588,   592,   596,   600,   604,   608,   612,   616,   618,
     620,   622,   626,   631,   636,   641,   646,   651,   656,   661,
     665,   672,   679,   683,   692,   694,   698,   700,   702,   706,
     712,   714,   716,   718,   720,   723,   724,   727,   729,   732,
     735,   739,   741,   743,   748,   753,   757,   759,   761,   763,
     765,   767,   769,   773,   778,   783,   788,   792,   797,   802,
     810,   816,   818,   820,   822,   824,   826,   828,   830,   832,
     834,   837,   844,   846,   848,   849,   853,   855,   859,   861,
     865,   869,   871,   875,   877,   879,   881,   885,   888,   893,
     900,   902,   906,   908,   912,   914,   918,   919,   922,   924,
     928,   932,   933,   935,   937,   939,   943,   945,   947,   951,
     955,   962,   964,   968,   972,   976,   982,   987,   991,   995,
     997,   999,  1001,  1003,  1005,  1007,  1009,  1011,  1019,  1024,
    1030,  1031,  1033,  1036,  1038,  1042,  1046,  1049,  1053,  1060,
    1061,  1063,  1065,  1068,  1071,  1074,  1076,  1084,  1086,  1088,
    1090,  1092,  1098,  1102,  1106,  1113,  1117,  1121,  1125,  1127,
    1131,  1134,  1138,  1142,  1145,  1147,  1150,  1152,  1154,  1158,
    1162,  1166,  1170
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     141,     0,    -1,   142,    -1,   143,    -1,   143,   131,    -1,
     205,    -1,   213,    -1,   199,    -1,   200,    -1,   203,    -1,
     214,    -1,   223,    -1,   225,    -1,   226,    -1,   227,    -1,
     232,    -1,   237,    -1,   238,    -1,   242,    -1,   246,    -1,
     244,    -1,   245,    -1,   247,    -1,   248,    -1,   249,    -1,
     239,    -1,   250,    -1,   252,    -1,   253,    -1,   254,    -1,
     231,    -1,   144,    -1,   143,   131,   144,    -1,   145,    -1,
     194,    -1,   146,    -1,    90,     3,   132,   132,   146,   133,
     147,   133,    -1,   153,    -1,    90,   154,    47,   132,   150,
     153,   133,   151,   152,    -1,    -1,   134,   148,    -1,   149,
      -1,   148,   134,   149,    -1,     3,    -1,     5,    -1,    54,
      -1,    -1,    77,    26,   177,    -1,    -1,    66,     5,    -1,
      66,     5,   134,     5,    -1,    90,   154,    47,   158,   159,
     169,   173,   172,   175,   179,   181,    -1,   155,    -1,   154,
     134,   155,    -1,   126,    -1,   157,   156,    -1,    -1,     3,
      -1,    18,     3,    -1,   187,    -1,    21,   132,   187,   133,
      -1,    68,   132,   187,   133,    -1,    70,   132,   187,   133,
      -1,    99,   132,   187,   133,    -1,    52,   132,   187,   133,
      -1,    33,   132,   126,   133,    -1,    51,   132,   133,    -1,
      33,   132,    39,     3,   133,    -1,     3,    -1,   158,   134,
       3,    -1,    -1,   160,    -1,   112,   161,    -1,   162,    -1,
     161,   115,   161,    -1,   132,   161,   133,    -1,    67,   132,
       8,   133,    -1,   163,    -1,   165,   118,   166,    -1,   165,
     119,   166,    -1,   165,    55,   132,   168,   133,    -1,   165,
     129,    55,   132,   168,   133,    -1,   165,    55,     9,    -1,
     165,   129,    55,     9,    -1,   165,    23,   166,   115,   166,
      -1,   165,   121,   166,    -1,   165,   120,   166,    -1,   165,
     122,   166,    -1,   165,   123,   166,    -1,   165,   118,   167,
      -1,   164,    -1,   165,    23,   167,   115,   167,    -1,   165,
      23,   166,   115,   167,    -1,   165,    23,   167,   115,   166,
      -1,   165,   121,   167,    -1,   165,   120,   167,    -1,   165,
     122,   167,    -1,   165,   123,   167,    -1,   165,   118,     8,
      -1,   165,   119,     8,    -1,   165,    61,    75,    -1,   165,
      61,   129,    75,    -1,   165,   119,   167,    -1,     3,    -1,
       4,    -1,    33,   132,   126,   133,    -1,    51,   132,   133,
      -1,   111,   132,   133,    -1,    54,    -1,   255,    -1,    59,
     132,   255,   133,    -1,    41,   132,   255,   133,    -1,    24,
     132,   255,   133,    -1,     5,    -1,   125,     5,    -1,     6,
      -1,   125,     6,    -1,    14,    -1,   166,    -1,   168,   134,
     166,    -1,    -1,    50,   170,    26,   171,    -1,    -1,     5,
      -1,   165,    -1,   171,   134,   165,    -1,    -1,    53,   163,
      -1,    -1,   174,    -1,   113,    50,    77,    26,   177,    -1,
      -1,   176,    -1,    77,    26,   177,    -1,    77,    26,    81,
     132,   133,    -1,   178,    -1,   177,   134,   178,    -1,   165,
      -1,   165,    19,    -1,   165,    37,    -1,    -1,   180,    -1,
      66,     5,    -1,    66,     5,   134,     5,    -1,    -1,   182,
      -1,    76,   183,    -1,   184,    -1,   183,   134,   184,    -1,
       3,   118,     3,    -1,    83,   118,     3,    -1,     3,   118,
       5,    -1,     3,   118,   132,   185,   133,    -1,     3,   118,
       3,   132,     8,   133,    -1,    83,   118,     3,   132,     8,
     133,    -1,     3,   118,     8,    -1,    83,   118,     8,    -1,
     186,    -1,   185,   134,   186,    -1,     3,   118,   166,    -1,
       3,    -1,     4,    -1,    54,    -1,     5,    -1,     6,    -1,
      14,    -1,     9,    -1,   125,   187,    -1,   129,   187,    -1,
     187,   124,   187,    -1,   187,   125,   187,    -1,   187,   126,
     187,    -1,   187,   127,   187,    -1,   187,   120,   187,    -1,
     187,   121,   187,    -1,   187,   117,   187,    -1,   187,   116,
     187,    -1,   187,   128,   187,    -1,   187,    40,   187,    -1,
     187,    71,   187,    -1,   187,   123,   187,    -1,   187,   122,
     187,    -1,   187,   118,   187,    -1,   187,   119,   187,    -1,
     187,   115,   187,    -1,   187,   114,   187,    -1,   132,   187,
     133,    -1,   135,   191,   136,    -1,   188,    -1,   255,    -1,
     258,    -1,   255,    61,    75,    -1,   255,    61,   129,    75,
      -1,     3,   132,   189,   133,    -1,    55,   132,   189,   133,
      -1,    59,   132,   189,   133,    -1,    24,   132,   189,   133,
      -1,    44,   132,   189,   133,    -1,    41,   132,   189,   133,
      -1,     3,   132,   133,    -1,    70,   132,   187,   134,   187,
     133,    -1,    68,   132,   187,   134,   187,   133,    -1,   111,
     132,   133,    -1,     3,   132,   187,    46,     3,    55,   255,
     133,    -1,   190,    -1,   189,   134,   190,    -1,   187,    -1,
       8,    -1,   192,   118,   193,    -1,   191,   134,   192,   118,
     193,    -1,     3,    -1,    55,    -1,   166,    -1,     3,    -1,
      94,   196,    -1,    -1,    65,     8,    -1,   110,    -1,    97,
     195,    -1,    69,   195,    -1,    16,    97,   195,    -1,    80,
      -1,    79,    -1,    16,     8,    97,   195,    -1,    16,     3,
      97,   195,    -1,    56,     3,    97,    -1,     3,    -1,    75,
      -1,     8,    -1,     5,    -1,     6,    -1,   197,    -1,   198,
     125,   197,    -1,    92,     3,   118,   202,    -1,    92,     3,
     118,   201,    -1,    92,     3,   118,    75,    -1,    92,    74,
     198,    -1,    92,    10,   118,   198,    -1,    92,    28,    92,
     198,    -1,    92,    49,     9,   118,   132,   168,   133,    -1,
      92,    49,     3,   118,   201,    -1,     3,    -1,     8,    -1,
     104,    -1,    43,    -1,   166,    -1,    31,    -1,    88,    -1,
     204,    -1,    22,    -1,    96,   103,    -1,   206,    60,     3,
     207,   108,   209,    -1,    57,    -1,    86,    -1,    -1,   132,
     208,   133,    -1,   165,    -1,   208,   134,   165,    -1,   210,
      -1,   209,   134,   210,    -1,   132,   211,   133,    -1,   212,
      -1,   211,   134,   212,    -1,   166,    -1,   167,    -1,     8,
      -1,   132,   168,   133,    -1,   132,   133,    -1,    36,    47,
     158,   160,    -1,    27,     3,   132,   215,   218,   133,    -1,
     216,    -1,   215,   134,   216,    -1,   212,    -1,   132,   217,
     133,    -1,     8,    -1,   217,   134,     8,    -1,    -1,   134,
     219,    -1,   220,    -1,   219,   134,   220,    -1,   212,   221,
     222,    -1,    -1,    18,    -1,     3,    -1,    66,    -1,   224,
       3,   195,    -1,    38,    -1,    37,    -1,    94,   101,   195,
      -1,    94,    35,   195,    -1,   107,   158,    92,   228,   160,
     181,    -1,   229,    -1,   228,   134,   229,    -1,     3,   118,
     166,    -1,     3,   118,   167,    -1,     3,   118,   132,   168,
     133,    -1,     3,   118,   132,   133,    -1,   255,   118,   166,
      -1,   255,   118,   167,    -1,    59,    -1,    24,    -1,    44,
      -1,    25,    -1,    72,    -1,    73,    -1,    63,    -1,    98,
      -1,    17,   100,     3,    15,    30,     3,   230,    -1,    94,
     240,   109,   233,    -1,    94,   240,   109,    65,     8,    -1,
      -1,   234,    -1,   112,   235,    -1,   236,    -1,   235,   114,
     236,    -1,     3,   118,     8,    -1,    94,    29,    -1,    94,
      28,    92,    -1,    92,   240,   103,    62,    64,   241,    -1,
      -1,    49,    -1,    93,    -1,    84,   106,    -1,    84,    32,
      -1,    85,    84,    -1,    91,    -1,    34,    48,     3,    87,
     243,    95,     8,    -1,    58,    -1,    24,    -1,    44,    -1,
      98,    -1,    34,    83,     3,    95,     8,    -1,    42,    83,
       3,    -1,    42,    48,     3,    -1,    20,    56,     3,   102,
      89,     3,    -1,    45,    89,     3,    -1,    45,    82,     3,
      -1,    90,   251,   179,    -1,    10,    -1,    10,   137,     3,
      -1,    90,   187,    -1,   105,    89,     3,    -1,    78,    56,
       3,    -1,     3,   256,    -1,   257,    -1,   256,   257,    -1,
      13,    -1,    14,    -1,   138,   187,   139,    -1,   138,     8,
     139,    -1,   187,   118,   259,    -1,   259,   118,   187,    -1,
       8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   166,   166,   167,   168,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   203,   204,   208,   209,   213,   214,   222,   223,   230,
     232,   236,   240,   247,   248,   249,   253,   266,   273,   275,
     280,   289,   305,   306,   310,   311,   314,   316,   317,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   333,   334,
     337,   339,   343,   347,   348,   349,   353,   358,   362,   369,
     377,   385,   394,   399,   404,   409,   414,   419,   424,   429,
     434,   439,   444,   449,   454,   459,   464,   469,   474,   479,
     484,   489,   497,   501,   502,   507,   513,   519,   525,   531,
     532,   533,   534,   538,   539,   550,   551,   552,   556,   562,
     568,   570,   573,   575,   582,   586,   592,   594,   600,   602,
     606,   617,   619,   623,   627,   634,   635,   639,   640,   641,
     644,   646,   650,   655,   662,   664,   668,   672,   673,   677,
     682,   688,   693,   699,   704,   709,   714,   722,   727,   734,
     744,   745,   746,   747,   748,   749,   750,   751,   752,   754,
     755,   756,   757,   758,   759,   760,   761,   762,   763,   764,
     765,   766,   767,   768,   769,   770,   771,   772,   773,   774,
     775,   776,   777,   781,   782,   783,   784,   785,   786,   787,
     788,   789,   790,   791,   795,   796,   800,   801,   805,   806,
     810,   811,   815,   816,   822,   825,   827,   831,   832,   833,
     834,   835,   836,   837,   842,   847,   857,   858,   859,   860,
     861,   865,   866,   870,   875,   880,   885,   886,   887,   891,
     896,   904,   905,   909,   910,   911,   925,   926,   927,   931,
     932,   938,   946,   947,   950,   952,   956,   957,   961,   962,
     966,   970,   971,   975,   976,   977,   978,   979,   985,   995,
    1003,  1007,  1014,  1015,  1022,  1032,  1038,  1040,  1044,  1049,
    1053,  1060,  1062,  1066,  1067,  1073,  1081,  1082,  1088,  1092,
    1098,  1106,  1107,  1111,  1125,  1131,  1135,  1140,  1154,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1176,  1189,  1193,
    1200,  1201,  1205,  1209,  1210,  1214,  1218,  1225,  1232,  1238,
    1239,  1240,  1244,  1245,  1246,  1247,  1253,  1264,  1265,  1266,
    1267,  1271,  1281,  1290,  1301,  1313,  1322,  1333,  1341,  1342,
    1346,  1356,  1367,  1378,  1381,  1382,  1386,  1387,  1388,  1389,
    1393,  1394,  1398
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
  "$accept", "request", "statement", "multi_stmt_list", "multi_stmt", 
  "select", "select1", "opt_tablefunc_args", "tablefunc_args_list", 
  "tablefunc_arg", "subselect_start", "opt_outer_order", 
  "opt_outer_limit", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "filter_item", 
  "expr_float_unhandled", "expr_ident", "const_int", "const_float", 
  "const_list", "opt_group_clause", "opt_int", "group_items_list", 
  "opt_having_clause", "opt_group_order_clause", "group_order_clause", 
  "opt_order_clause", "order_clause", "order_items_list", "order_item", 
  "opt_limit_clause", "limit_clause", "opt_option_clause", 
  "option_clause", "option_list", "option_item", "named_const_list", 
  "named_const", "expr", "function", "arglist", "arg", "consthash", 
  "hash_key", "hash_val", "show_stmt", "like_filter", "show_what", 
  "simple_set_value", "set_value", "set_stmt", "set_global_stmt", 
  "set_string_value", "boolean_value", "transact_op", "start_transaction", 
  "insert_into", "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "show_databases", "update", "update_items_list", "update_item", 
  "alter_col_type", "alter", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "show_character_set", 
  "set_transaction", "opt_scope", "isolation_level", "create_function", 
  "udf_type", "create_ranker", "drop_ranker", "drop_function", 
  "attach_index", "flush_rtindex", "flush_ramchunk", "select_sysvar", 
  "sysvar_name", "select_dual", "truncate", "optimize_index", 
  "json_field", "subscript", "subkey", "streq", "strval", 0
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
     142,   143,   143,   144,   144,   145,   145,   146,   146,   147,
     147,   148,   148,   149,   149,   149,   150,   151,   152,   152,
     152,   153,   154,   154,   155,   155,   156,   156,   156,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   158,   158,
     159,   159,   160,   161,   161,   161,   162,   162,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   164,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   166,   166,   167,   167,   167,   168,   168,
     169,   169,   170,   170,   171,   171,   172,   172,   173,   173,
     174,   175,   175,   176,   176,   177,   177,   178,   178,   178,
     179,   179,   180,   180,   181,   181,   182,   183,   183,   184,
     184,   184,   184,   184,   184,   184,   184,   185,   185,   186,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   189,   189,   190,   190,   191,   191,
     192,   192,   193,   193,   194,   195,   195,   196,   196,   196,
     196,   196,   196,   196,   196,   196,   197,   197,   197,   197,
     197,   198,   198,   199,   199,   199,   199,   199,   199,   200,
     200,   201,   201,   202,   202,   202,   203,   203,   203,   204,
     204,   205,   206,   206,   207,   207,   208,   208,   209,   209,
     210,   211,   211,   212,   212,   212,   212,   212,   213,   214,
     215,   215,   216,   216,   217,   217,   218,   218,   219,   219,
     220,   221,   221,   222,   222,   223,   224,   224,   225,   226,
     227,   228,   228,   229,   229,   229,   229,   229,   229,   230,
     230,   230,   230,   230,   230,   230,   230,   231,   232,   232,
     233,   233,   234,   235,   235,   236,   237,   238,   239,   240,
     240,   240,   241,   241,   241,   241,   242,   243,   243,   243,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   251,
     252,   253,   254,   255,   256,   256,   257,   257,   257,   257,
     258,   258,   259
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
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
       3,     3,     5,     6,     6,     3,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     3,     4,     4,     4,     4,     4,     4,     4,     3,
       6,     6,     3,     8,     1,     3,     1,     1,     3,     5,
       1,     1,     1,     1,     2,     0,     2,     1,     2,     2,
       3,     1,     1,     4,     4,     3,     1,     1,     1,     1,
       1,     1,     3,     4,     4,     4,     3,     4,     4,     7,
       5,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     6,     1,     1,     0,     3,     1,     3,     1,     3,
       3,     1,     3,     1,     1,     1,     3,     2,     4,     6,
       1,     3,     1,     3,     1,     3,     0,     2,     1,     3,
       3,     0,     1,     1,     1,     3,     1,     1,     3,     3,
       6,     1,     3,     3,     3,     5,     4,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     7,     4,     5,
       0,     1,     2,     1,     3,     3,     2,     3,     6,     0,
       1,     1,     2,     2,     2,     1,     7,     1,     1,     1,
       1,     5,     3,     3,     6,     3,     3,     3,     1,     3,
       2,     3,     3,     2,     1,     2,     1,     1,     3,     3,
       3,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   249,     0,   246,     0,     0,   287,   286,
       0,     0,   252,     0,   253,   247,     0,   319,   319,     0,
       0,     0,     0,     2,     3,    31,    33,    35,    37,    34,
       7,     8,     9,   248,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    30,    15,    16,    17,    25,    18,    20,
      21,    19,    22,    23,    24,    26,    27,    28,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,   161,   163,   164,   352,   166,   338,   165,     0,     0,
       0,     0,     0,     0,     0,   162,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,    52,    56,
      59,   188,   140,   189,   190,     0,     0,     0,     0,   320,
       0,   321,     0,     0,     0,   316,   215,   320,     0,   215,
     222,   221,   215,   215,   217,   214,     0,   250,     0,    68,
       0,     1,     4,     0,   215,     0,     0,     0,     0,     0,
       0,   333,   332,   336,   335,   342,   346,   347,     0,     0,
     343,   344,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,     0,   167,
     168,     0,   210,   211,     0,     0,     0,     0,    57,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   337,
     141,     0,     0,     0,     0,     0,     0,     0,   226,   229,
     230,   228,   227,   231,   236,     0,     0,     0,   215,   317,
       0,   289,     0,   219,   218,   288,   310,   341,     0,     0,
       0,     0,    32,   254,   285,     0,     0,   113,   115,   265,
     117,     0,     0,   263,   264,   272,   276,   270,     0,     0,
       0,   268,   207,     0,   199,   206,     0,   204,   352,     0,
     345,   339,     0,   206,     0,     0,     0,     0,     0,    66,
       0,     0,     0,     0,     0,     0,   202,     0,     0,     0,
     186,     0,   187,     0,    46,    70,    53,    59,    58,   178,
     179,   185,   184,   176,   175,   182,   350,   183,   173,   174,
     181,   180,   169,   170,   171,   172,   177,   142,   191,     0,
     351,   241,   242,   244,   235,   243,     0,   245,   234,   233,
     237,   238,     0,     0,     0,     0,   215,   215,   220,   216,
     225,     0,     0,   308,   311,     0,     0,   291,     0,    69,
       0,     0,     0,     0,   114,   116,   274,   267,   118,     0,
       0,     0,     0,   328,   329,   327,   330,     0,   331,   103,
     104,     0,     0,     0,     0,   108,     0,     0,     0,     0,
      72,    73,    77,    90,     0,   109,     0,     0,     0,   193,
       0,   349,   348,    60,   196,     0,    65,   198,   197,    64,
     194,   195,    61,     0,    62,     0,    63,     0,     0,     0,
     213,   212,   208,     0,   120,    71,     0,   192,   240,     0,
     232,     0,   224,   223,   309,     0,   312,   313,     0,     0,
     144,     0,   256,     0,     0,     0,   334,   266,     0,   273,
       0,   272,   271,   277,   278,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    39,     0,   205,    67,
       0,     0,     0,     0,     0,   122,   128,   143,     0,     0,
       0,   325,   318,     0,     0,     0,   293,   294,   292,     0,
     290,   145,   297,   298,   255,     0,     0,   251,   258,   300,
     302,   301,   299,   305,   303,   304,   306,   307,   119,   275,
     282,     0,     0,   326,     0,     0,     0,     0,   106,     0,
       0,   107,    75,    74,     0,     0,    82,     0,   100,     0,
      98,    78,    89,    99,    79,   102,    86,    95,    85,    94,
      87,    96,    88,    97,     0,     0,     0,     0,   201,   200,
     209,     0,     0,   123,     0,     0,   126,   129,   239,   323,
     322,   324,   315,   314,   296,     0,     0,     0,   146,   147,
     257,     0,     0,   261,     0,   283,   284,   280,   281,   279,
     112,   105,   111,   110,    76,     0,     0,     0,   101,    83,
       0,    43,    44,    45,    40,    41,    36,     0,     0,     0,
      48,     0,     0,     0,   131,   295,     0,     0,     0,   260,
       0,   259,    84,    92,    93,    91,    80,     0,     0,   203,
       0,     0,    38,   124,   121,     0,   127,     0,   140,   132,
     149,   151,   155,     0,   150,   156,   148,   262,    81,    42,
     137,    47,   135,    49,     0,     0,     0,   144,     0,     0,
       0,   157,     0,   138,   139,     0,     0,   125,   130,     0,
     133,    51,     0,     0,   152,     0,     0,   136,    50,     0,
     153,   159,   158,   154,   134
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   536,   584,   585,
     403,   590,   612,    28,    97,    98,   180,    99,   285,   404,
     251,   370,   371,   372,   373,   374,   348,   244,   349,   466,
     544,   614,   594,   546,   547,   618,   619,   631,   632,   199,
     200,   480,   481,   558,   559,   640,   641,   263,   101,   256,
     257,   174,   175,   402,    29,   221,   125,   213,   214,    30,
      31,   318,   319,    32,    33,    34,    35,   341,   423,   487,
     488,   562,   245,    36,    37,   246,   247,   350,   352,   433,
     434,   501,   567,    38,    39,    40,    41,    42,   336,   337,
     497,    43,    44,   333,   334,   416,   417,    45,    46,    47,
     112,   472,    48,   357,    49,    50,    51,    52,    53,    54,
      55,   102,    56,    57,    58,   103,   150,   151,   104,   105
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -398
static const short yypact[] =
{
    1266,   -52,     1,  -398,    69,  -398,     2,    82,  -398,  -398,
     176,   179,  -398,   138,  -398,  -398,   342,   487,  1007,   -17,
     -22,   153,   210,  -398,   108,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,   230,  -398,  -398,  -398,   257,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,   300,
     302,   160,   305,   323,   153,   330,   340,   350,   352,   361,
      27,  -398,  -398,  -398,  -398,  -398,   162,  -398,   244,   253,
     260,   268,   279,   281,   292,  -398,   308,   313,   314,   318,
     324,   325,   606,  -398,   606,   606,    62,   -23,  -398,    53,
     720,  -398,   307,   310,  -398,   341,   344,   346,   366,   241,
     376,  -398,   357,    44,   374,  -398,   405,  -398,   470,   405,
    -398,  -398,   405,   405,  -398,  -398,   367,  -398,   472,  -398,
      99,  -398,   192,   478,   405,   469,   383,    84,   399,   396,
     -89,  -398,  -398,  -398,  -398,  -398,  -398,  -398,   199,   664,
      68,  -398,   489,   606,   698,   -13,   698,   698,   362,   606,
     698,   698,   606,   606,   606,   370,    61,   375,   377,  -398,
    -398,  1004,  -398,  -398,   149,   390,     0,   428,  -398,   508,
    -398,   606,   606,   606,   606,   606,   606,   606,   606,   606,
     606,   606,   606,   606,   606,   606,   606,   606,   507,  -398,
    -398,   -36,   606,   152,   376,   376,   395,   400,  -398,  -398,
    -398,  -398,  -398,  -398,   397,   459,   427,   429,   405,  -398,
     517,  -398,   431,  -398,  -398,  -398,   166,  -398,   527,   529,
     496,   409,  -398,   401,  -398,   504,   446,  -398,  -398,  -398,
    -398,   315,     6,  -398,  -398,  -398,   404,  -398,   221,   534,
     303,  -398,   425,    29,  -398,  1261,   189,  -398,   406,   736,
    -398,  -398,  1024,  1276,   196,   543,   419,   225,   234,  -398,
    1044,   255,   265,   778,   836,  1065,  -398,   564,   606,   606,
    -398,    62,  -398,   104,  -398,   -89,  -398,  1276,  -398,  -398,
    -398,  1290,   761,   518,   927,   623,  -398,   623,   146,   146,
     146,   146,   161,   161,  -398,  -398,  -398,   422,  -398,   484,
     623,  -398,  -398,  -398,  -398,  -398,   557,  -398,  -398,  -398,
     397,   397,   239,   433,   376,   510,   405,   405,  -398,  -398,
    -398,   567,   573,  -398,  -398,     7,   -58,  -398,   461,  -398,
     336,   473,   574,   579,  -398,  -398,  -398,  -398,  -398,   269,
     271,    84,   450,  -398,  -398,  -398,  -398,   491,  -398,    68,
    -398,   452,   453,   455,   458,  -398,   460,   462,   464,   303,
     476,  -398,  -398,  -398,   175,  -398,   428,   465,   590,  -398,
     698,  -398,  -398,  -398,  -398,   466,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,   606,  -398,   606,  -398,   868,   894,   479,
    -398,  -398,  -398,   512,   550,  -398,   598,  -398,  -398,     5,
    -398,   164,  -398,  -398,  -398,   486,   492,  -398,    11,   527,
     537,   121,  -398,   273,   485,  1265,  -398,  -398,     5,  -398,
     608,    46,  -398,   493,  -398,  -398,   616,   626,   500,   626,
     515,   626,   625,   516,   119,   303,   121,    -4,   -29,   100,
     110,   121,   121,   121,   121,   596,   519,   597,  -398,  -398,
    1133,  1154,   104,   428,   521,   650,   544,  -398,   275,   -10,
     572,  -398,  -398,   651,   573,    63,  -398,  -398,  -398,    39,
    -398,  -398,  -398,  -398,  -398,   336,    96,   524,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,    25,    96,  -398,    68,   531,   533,   538,  -398,   546,
     547,  -398,  -398,  -398,   562,   566,  -398,     5,  -398,   587,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,    20,   187,   549,   626,  -398,  -398,
    -398,   -11,   607,  -398,   657,   635,   633,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,   285,   569,   577,   556,  -398,
    -398,    64,   287,  -398,   485,  -398,  -398,  -398,   673,  -398,
    -398,  -398,  -398,  -398,  -398,   121,   121,   289,  -398,  -398,
       5,  -398,  -398,  -398,   558,  -398,  -398,   565,   153,   674,
     643,   336,   634,   336,   636,  -398,    10,   272,    39,  -398,
      96,  -398,  -398,  -398,  -398,  -398,  -398,   294,   187,  -398,
     336,   705,  -398,  -398,   580,   689,  -398,   690,   307,  -398,
     589,  -398,  -398,   721,   593,  -398,  -398,  -398,  -398,  -398,
     200,   592,  -398,   594,   336,   336,   490,   537,   719,   611,
     296,  -398,   722,  -398,  -398,   336,   728,  -398,   592,   604,
     592,  -398,   621,     5,  -398,   721,   622,  -398,  -398,   628,
    -398,  -398,  -398,  -398,  -398
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -398,  -398,  -398,  -398,   605,  -398,   503,  -398,  -398,   132,
    -398,  -398,  -398,   355,   299,   582,  -398,  -398,    30,  -398,
    -241,  -282,  -398,   170,  -398,  -334,  -137,  -391,  -397,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -192,   120,   151,
    -398,   127,  -398,  -398,   169,  -398,   115,   -15,  -398,   181,
     391,  -398,   497,   311,  -398,  -115,  -398,   448,   250,  -398,
    -398,   457,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
     213,  -398,  -349,  -398,  -398,  -398,   423,  -398,  -398,  -398,
     278,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,   363,
    -398,  -398,  -398,  -398,  -398,  -398,   309,  -398,  -398,  -398,
     763,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -219,  -398,   637,  -398,   599
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -353
static const short yytable[] =
{
     243,   100,   431,   129,   223,   516,   422,   224,   225,   338,
     237,   237,   468,   620,   346,   621,   237,   238,   622,   234,
     146,   147,   549,   250,   176,   240,   265,   477,   565,   579,
     483,   375,   166,    71,    72,    73,   588,    74,    75,   308,
     146,   147,   556,    77,   405,   229,   518,   216,    59,  -281,
      62,   130,   217,    79,   250,   515,   178,    60,   522,   525,
     527,   529,   531,   533,   500,   172,   317,   128,   237,   237,
      81,   179,    61,    82,   146,   147,   419,   169,   555,   170,
     171,   146,   147,    85,    86,    63,   127,   444,    87,   237,
     238,   566,   239,   309,   140,   420,   550,   167,   240,   168,
     519,   237,   238,   328,   239,   237,   238,   400,   520,   237,
     240,   177,  -281,   266,   240,   237,   238,   173,   523,   376,
     577,   375,   557,   177,   240,   418,   237,   238,   517,    64,
     316,   316,   284,   255,   259,   240,   241,   563,   262,   347,
      91,   218,   623,   475,   270,   149,   401,   273,   274,   275,
     375,   560,   580,   568,    92,   311,   129,   237,    94,   148,
     312,    95,   287,   513,    96,   149,   289,   290,   291,   292,
     293,   294,   295,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   607,   603,   605,   181,   310,   316,   316,
     581,   228,   582,   277,    69,   313,   554,   347,   446,   149,
     338,   181,   166,    71,    72,    73,   149,   252,    75,   241,
     131,   412,   413,    77,   243,   287,   242,   182,   505,   643,
     507,   241,   509,    79,    65,   241,   375,   314,   561,   316,
     447,   331,   182,   229,   445,   241,   448,   644,   171,   132,
      81,   583,   311,    82,   206,   353,   241,   312,   469,   470,
     207,   627,   512,    85,    86,   471,   315,   613,    87,    66,
     134,    67,   255,   397,   398,   354,   375,   167,    68,   168,
     193,   194,   195,   196,   197,   624,   630,   316,   332,   355,
     625,   476,   230,   281,   482,   282,   231,   195,   196,   197,
     133,   498,   137,   449,   450,   451,   452,   453,   454,   152,
     647,   630,   630,   135,   455,   136,   359,   360,   138,   514,
      91,   630,   521,   524,   526,   528,   530,   532,   587,   356,
     344,   345,   379,   380,    92,   401,   139,   361,    94,   384,
     380,   253,   254,   141,    96,   264,   362,   267,   268,   359,
     360,   271,   272,   142,   363,    70,    71,    72,    73,   243,
      74,    75,    76,   143,   364,   144,    77,   365,   387,   380,
     361,   287,   366,    78,   145,   243,    79,   388,   380,   362,
     367,   201,   375,   198,   375,    80,   153,   363,   460,   208,
     461,   209,   210,    81,   211,   154,    82,   364,   390,   380,
     365,   375,   155,    83,    84,   366,    85,    86,   391,   380,
     156,    87,   427,   428,   429,   430,   484,   485,   548,   428,
      88,   157,    89,   158,   368,   375,   375,   375,   595,   428,
     599,   600,   606,   428,   159,   113,   375,   628,   428,   654,
     655,   166,    71,    72,    73,   369,    74,    75,   602,   604,
     160,    90,    77,   648,   650,   161,   162,   368,   287,    78,
     163,   212,    79,    91,   320,   321,   164,   165,   205,   202,
     215,    80,   203,   243,   204,   118,   219,    92,    93,    81,
     220,    94,    82,   222,    95,   227,   226,    96,   119,    83,
      84,   233,    85,    86,   235,   236,   248,    87,   120,   121,
     106,   249,   261,   359,   360,   269,    88,   107,    89,    70,
      71,    72,    73,   276,    74,    75,   122,   278,   283,   279,
      77,   288,   307,   322,   361,   108,   661,    78,   323,   124,
      79,   325,   324,   362,   326,   329,   327,    90,   330,    80,
     335,   363,   339,   340,   342,   343,   109,    81,   351,    91,
      82,   364,   358,  -352,   365,   381,   385,    83,    84,   366,
      85,    86,   386,    92,    93,    87,   406,    94,   181,   407,
      95,   110,   344,    96,    88,   409,    89,   166,    71,    72,
      73,   649,   252,    75,   411,   414,   415,   425,    77,   421,
     111,   424,   426,   435,   437,   438,   436,   439,    79,   182,
     440,   445,   441,   457,   442,    90,   443,   462,   456,   459,
     465,   368,   463,   467,   473,    81,   474,    91,    82,   166,
      71,    72,    73,   479,    74,    75,   499,   486,    85,    86,
      77,    92,    93,    87,   503,    94,   506,   502,    95,   504,
      79,    96,   167,   510,   168,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    81,   508,   511,
      82,   534,   537,   535,   542,   543,   551,   545,   564,   552,
      85,    86,   578,   181,   570,    87,   571,   166,    71,    72,
      73,   572,   258,    75,   167,    91,   168,   575,    77,   573,
     574,   576,   586,   591,   589,   592,   593,   596,    79,    92,
     598,   500,   608,    94,   182,   597,    95,   254,   609,    96,
     610,   166,    71,    72,    73,    81,   252,    75,    82,   611,
     633,   615,    77,   617,   634,   635,   636,    91,    85,    86,
    -340,   638,    79,    87,   639,   642,   645,   652,   646,   653,
     656,    92,   167,   658,   168,    94,   659,   232,    95,    81,
     629,    96,    82,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    85,    86,   660,   663,   377,    87,   464,   286,
     181,   664,   541,   616,   651,   657,   167,   626,   168,   637,
     662,   458,   410,   540,   432,    91,   181,   601,   399,   408,
     569,   126,   478,   553,     0,     0,   296,   260,     0,    92,
       0,   182,     0,    94,     0,     0,    95,     0,     0,    96,
       0,   181,     0,     0,     0,     0,     0,   182,     0,    91,
       0,     0,     0,     0,     0,     0,     0,     0,   181,     0,
       0,     0,     0,    92,     0,     0,     0,    94,     0,     0,
      95,     0,   182,    96,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   382,   181,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
       0,     0,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   182,   181,     0,
       0,   392,   393,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,     0,     0,     0,     0,   182,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   182,     0,   181,     0,   394,
     395,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,     0,   182,     0,
       0,     0,   393,     0,     0,     0,     0,     0,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   113,     0,     0,     0,     0,   395,     0,
       0,     0,     0,     0,     0,   114,   115,     0,     0,     0,
       0,     0,   116,     0,   181,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   117,     0,     0,     0,
       0,     0,     0,   118,   181,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   182,   119,     0,     0,     0,
       0,     0,     0,     0,   181,     0,   120,   121,     0,     0,
       0,     0,     0,     0,     0,   182,     0,     0,     0,     0,
     111,     0,     0,     0,   122,   181,     0,     0,   123,     0,
       0,     0,     0,     0,     0,   182,     0,   124,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,     0,     0,     0,   182,   280,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,     0,     0,     0,     0,   383,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   181,     0,     0,     0,   389,     0,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   181,     0,     0,     0,   396,     0,
       0,     0,     0,     0,   182,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   182,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,     0,     0,     0,     0,   538,     0,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,     1,     0,     0,     2,   539,     3,   489,
     490,     0,     0,     4,     0,     0,     0,     5,     0,     0,
       6,   181,     7,     8,     9,     0,     0,   378,    10,   491,
       0,    11,     0,     0,     0,     0,   181,     0,     0,     0,
       0,     0,     0,    12,   492,     0,     0,     0,   493,     0,
     181,     0,   182,     0,     0,     0,     0,   494,   495,     0,
       0,     0,     0,     0,    13,     0,     0,   182,     0,     0,
       0,     0,    14,     0,    15,     0,    16,     0,    17,     0,
      18,   182,    19,   496,     0,     0,     0,     0,     0,     0,
       0,    20,     0,    21,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197
};

static const short yycheck[] =
{
     137,    16,   351,     3,   119,     9,   340,   122,   123,   228,
       5,     5,   409,     3,     8,     5,     5,     6,     8,   134,
      13,    14,    32,   112,    47,    14,    39,   418,     3,     9,
     421,   250,     3,     4,     5,     6,    47,     8,     9,    75,
      13,    14,     3,    14,   285,   134,    75,     3,   100,     3,
      48,    21,     8,    24,   112,   446,     3,    56,   449,   450,
     451,   452,   453,   454,    18,     3,   203,    89,     5,     5,
      41,    18,     3,    44,    13,    14,   134,    92,   475,    94,
      95,    13,    14,    54,    55,    83,   103,   369,    59,     5,
       6,    66,     8,   129,    64,   336,   106,    68,    14,    70,
     129,     5,     6,   218,     8,     5,     6,     3,     8,     5,
      14,   134,    66,   126,    14,     5,     6,    55,     8,    90,
     517,   340,    83,   134,    14,   118,     5,     6,   132,    47,
     125,   125,   132,   148,   149,    14,   125,   486,   153,   133,
     111,    97,   132,   132,   159,   138,   283,   162,   163,   164,
     369,   485,   132,   502,   125,     3,     3,     5,   129,   132,
       8,   132,   177,   445,   135,   138,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   580,   575,   576,    40,   202,   125,   125,
       3,    92,     5,   132,    56,    43,   133,   133,    23,   138,
     419,    40,     3,     4,     5,     6,   138,     8,     9,   125,
       0,   326,   327,    14,   351,   230,   132,    71,   437,    19,
     439,   125,   441,    24,    48,   125,   445,    75,   132,   125,
      55,    65,    71,   134,   115,   125,    61,    37,   253,   131,
      41,    54,     3,    44,     3,    24,   125,     8,    84,    85,
       9,   600,   133,    54,    55,    91,   104,   591,    59,    83,
       3,    82,   277,   278,   279,    44,   485,    68,    89,    70,
     124,   125,   126,   127,   128,     3,   610,   125,   112,    58,
       8,   418,    90,   134,   421,   136,    94,   126,   127,   128,
      60,   428,   132,   118,   119,   120,   121,   122,   123,   137,
     634,   635,   636,     3,   129,     3,     3,     4,     3,   446,
     111,   645,   449,   450,   451,   452,   453,   454,   537,    98,
       5,     6,   133,   134,   125,   462,     3,    24,   129,   133,
     134,   132,   133,     3,   135,   154,    33,   156,   157,     3,
       4,   160,   161,     3,    41,     3,     4,     5,     6,   486,
       8,     9,    10,     3,    51,     3,    14,    54,   133,   134,
      24,   376,    59,    21,     3,   502,    24,   133,   134,    33,
      67,    61,   591,    66,   593,    33,   132,    41,   393,     3,
     395,     5,     6,    41,     8,   132,    44,    51,   133,   134,
      54,   610,   132,    51,    52,    59,    54,    55,   133,   134,
     132,    59,   133,   134,   133,   134,   133,   134,   133,   134,
      68,   132,    70,   132,   111,   634,   635,   636,   133,   134,
     133,   134,   133,   134,   132,    16,   645,   133,   134,   133,
     134,     3,     4,     5,     6,   132,     8,     9,   575,   576,
     132,    99,    14,   635,   636,   132,   132,   111,   463,    21,
     132,    75,    24,   111,   204,   205,   132,   132,    92,   118,
     103,    33,   118,   600,   118,    56,    92,   125,   126,    41,
      65,   129,    44,     3,   132,     3,   109,   135,    69,    51,
      52,     3,    54,    55,    15,   102,    87,    59,    79,    80,
       3,    95,     3,     3,     4,   133,    68,    10,    70,     3,
       4,     5,     6,   133,     8,     9,    97,   132,   118,   132,
      14,     3,     5,   118,    24,    28,   653,    21,   118,   110,
      24,    62,   125,    33,    97,     8,    97,    99,    97,    33,
       3,    41,     3,   132,    30,    89,    49,    41,   134,   111,
      44,    51,     8,   118,    54,   139,     3,    51,    52,    59,
      54,    55,   133,   125,   126,    59,   134,   129,    40,    75,
     132,    74,     5,   135,    68,   132,    70,     3,     4,     5,
       6,    81,     8,     9,    64,     8,     3,     3,    14,   118,
      93,   108,     3,   133,   132,   132,    95,   132,    24,    71,
     132,   115,   132,     3,   132,    99,   132,   118,   133,   133,
      50,   111,    90,     5,   118,    41,   114,   111,    44,     3,
       4,     5,     6,    76,     8,     9,     8,   132,    54,    55,
      14,   125,   126,    59,     8,   129,   126,   134,   132,     3,
      24,   135,    68,     8,    70,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    41,   133,   133,
      44,    55,    55,   134,   133,     5,    84,   113,   134,     8,
      54,    55,    75,    40,   133,    59,   133,     3,     4,     5,
       6,   133,     8,     9,    68,   111,    70,   115,    14,   133,
     133,   115,   133,    26,    77,    50,    53,   118,    24,   125,
     134,    18,   134,   129,    71,   118,   132,   133,   133,   135,
      26,     3,     4,     5,     6,    41,     8,     9,    44,    66,
       5,    77,    14,    77,   134,    26,    26,   111,    54,    55,
       0,   132,    24,    59,     3,   132,   134,     8,   134,   118,
       8,   125,    68,     5,    70,   129,   132,   132,   132,    41,
     608,   135,    44,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    54,    55,   133,   133,   253,    59,   403,   177,
      40,   133,   463,   593,   637,   645,    68,   598,    70,   618,
     655,   380,   324,   462,   351,   111,    40,   564,   281,   322,
     502,    18,   419,   474,    -1,    -1,   187,   150,    -1,   125,
      -1,    71,    -1,   129,    -1,    -1,   132,    -1,    -1,   135,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    71,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    -1,    -1,   125,    -1,    -1,    -1,   129,    -1,    -1,
     132,    -1,    71,   135,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    71,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    40,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,    -1,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    71,    40,    -1,
      -1,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    71,    -1,    40,    -1,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,    71,    -1,
      -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    16,    -1,    -1,    -1,    -1,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    -1,    35,    -1,    40,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    97,    40,    -1,    -1,   101,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,   110,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,    71,   133,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,    -1,   133,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    40,    -1,    -1,    -1,   133,    -1,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    40,    -1,    -1,    -1,   133,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,    -1,    -1,    -1,   133,    -1,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    17,    -1,    -1,    20,   133,    22,    24,
      25,    -1,    -1,    27,    -1,    -1,    -1,    31,    -1,    -1,
      34,    40,    36,    37,    38,    -1,    -1,    46,    42,    44,
      -1,    45,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    59,    -1,    -1,    -1,    63,    -1,
      40,    -1,    71,    -1,    -1,    -1,    -1,    72,    73,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    86,    -1,    88,    -1,    90,    -1,    92,    -1,
      94,    71,    96,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,   107,    -1,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    78,    86,    88,    90,    92,    94,    96,
     105,   107,   141,   142,   143,   144,   145,   146,   153,   194,
     199,   200,   203,   204,   205,   206,   213,   214,   223,   224,
     225,   226,   227,   231,   232,   237,   238,   239,   242,   244,
     245,   246,   247,   248,   249,   250,   252,   253,   254,   100,
      56,     3,    48,    83,    47,    48,    83,    82,    89,    56,
       3,     4,     5,     6,     8,     9,    10,    14,    21,    24,
      33,    41,    44,    51,    52,    54,    55,    59,    68,    70,
      99,   111,   125,   126,   129,   132,   135,   154,   155,   157,
     187,   188,   251,   255,   258,   259,     3,    10,    28,    49,
      74,    93,   240,    16,    28,    29,    35,    49,    56,    69,
      79,    80,    97,   101,   110,   196,   240,   103,    89,     3,
     158,     0,   131,    60,     3,     3,     3,   132,     3,     3,
     158,     3,     3,     3,     3,     3,    13,    14,   132,   138,
     256,   257,   137,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,     3,    68,    70,   187,
     187,   187,     3,    55,   191,   192,    47,   134,     3,    18,
     156,    40,    71,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    66,   179,
     180,    61,   118,   118,   118,    92,     3,     9,     3,     5,
       6,     8,    75,   197,   198,   103,     3,     8,    97,    92,
      65,   195,     3,   195,   195,   195,   109,     3,    92,   134,
      90,    94,   144,     3,   195,    15,   102,     5,     6,     8,
      14,   125,   132,   166,   167,   212,   215,   216,    87,    95,
     112,   160,     8,   132,   133,   187,   189,   190,     8,   187,
     257,     3,   187,   187,   189,    39,   126,   189,   189,   133,
     187,   189,   189,   187,   187,   187,   133,   132,   132,   132,
     133,   134,   136,   118,   132,   158,   155,   187,     3,   187,
     187,   187,   187,   187,   187,   187,   259,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,     5,    75,   129,
     187,     3,     8,    43,    75,   104,   125,   166,   201,   202,
     198,   198,   118,   118,   125,    62,    97,    97,   195,     8,
      97,    65,   112,   233,   234,     3,   228,   229,   255,     3,
     132,   207,    30,    89,     5,     6,     8,   133,   166,   168,
     217,   134,   218,    24,    44,    58,    98,   243,     8,     3,
       4,    24,    33,    41,    51,    54,    59,    67,   111,   132,
     161,   162,   163,   164,   165,   255,    90,   146,    46,   133,
     134,   139,   139,   133,   133,     3,   133,   133,   133,   133,
     133,   133,   133,   134,   133,   134,   133,   187,   187,   192,
       3,   166,   193,   150,   159,   160,   134,    75,   201,   132,
     197,    64,   195,   195,     8,     3,   235,   236,   118,   134,
     160,   118,   165,   208,   108,     3,     3,   133,   134,   133,
     134,   212,   216,   219,   220,   133,    95,   132,   132,   132,
     132,   132,   132,   132,   161,   115,    23,    55,    61,   118,
     119,   120,   121,   122,   123,   129,   133,     3,   190,   133,
     187,   187,   118,    90,   153,    50,   169,     5,   168,    84,
      85,    91,   241,   118,   114,   132,   166,   167,   229,    76,
     181,   182,   166,   167,   133,   134,   132,   209,   210,    24,
      25,    44,    59,    63,    72,    73,    98,   230,   166,     8,
      18,   221,   134,     8,     3,   255,   126,   255,   133,   255,
       8,   133,   133,   161,   166,   167,     9,   132,    75,   129,
       8,   166,   167,     8,   166,   167,   166,   167,   166,   167,
     166,   167,   166,   167,    55,   134,   147,    55,   133,   133,
     193,   154,   133,     5,   170,   113,   173,   174,   133,    32,
     106,    84,     8,   236,   133,   168,     3,    83,   183,   184,
     165,   132,   211,   212,   134,     3,    66,   222,   212,   220,
     133,   133,   133,   133,   133,   115,   115,   168,    75,     9,
     132,     3,     5,    54,   148,   149,   133,   255,    47,    77,
     151,    26,    50,    53,   172,   133,   118,   118,   134,   133,
     134,   210,   166,   167,   166,   167,   133,   168,   134,   133,
      26,    66,   152,   165,   171,    77,   163,    77,   175,   176,
       3,     5,     8,   132,     3,     8,   184,   212,   133,   149,
     165,   177,   178,     5,   134,    26,    26,   179,   132,     3,
     185,   186,   132,    19,    37,   134,   134,   165,   177,    81,
     177,   181,     8,   118,   133,   134,     8,   178,     5,   132,
     133,   166,   186,   133,   133
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

  case 31:

    { pParser->PushQuery(); ;}
    break;

  case 32:

    { pParser->PushQuery(); ;}
    break;

  case 36:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 38:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 41:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 42:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 46:

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

  case 47:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 49:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 50:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 51:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 54:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 59:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 60:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 61:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 62:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 63:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 64:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 65:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 66:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 67:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 69:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 76:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 78:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 79:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 80:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 81:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 82:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 95:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 96:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 97:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 98:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 99:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 100:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 101:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 104:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 105:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 106:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 107:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 108:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 113:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 114:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 115:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 116:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 117:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 118:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 119:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 123:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 124:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 125:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 127:

    {
			pParser->AddHaving();
		;}
    break;

  case 130:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 133:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 134:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 136:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 138:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 139:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 142:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 143:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 153:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 154:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 155:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 156:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 157:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 158:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 159:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 161:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 162:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 167:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 168:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 169:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 170:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 171:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 172:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 173:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 174:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 175:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 176:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 177:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 178:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 179:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 180:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 181:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 182:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 183:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 186:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 187:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 191:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 192:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 193:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 194:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 195:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 196:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 197:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 198:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 199:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 200:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 201:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 202:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 203:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 208:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 209:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 216:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 217:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 218:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 219:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 220:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 221:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 222:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 223:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 224:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 225:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 233:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 234:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 235:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 236:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 237:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 238:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 239:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 240:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 243:

    { yyval.m_iValue = 1; ;}
    break;

  case 244:

    { yyval.m_iValue = 0; ;}
    break;

  case 245:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 246:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 247:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 248:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 251:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 252:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 253:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 256:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 257:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 260:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 261:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 262:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 263:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 264:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 265:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 266:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 267:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 268:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 269:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 270:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 271:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 273:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 274:

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

  case 275:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 278:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 280:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 285:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 288:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 289:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 290:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 293:

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

  case 294:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 295:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 296:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 297:

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

  case 298:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 299:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 300:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 301:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 302:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 303:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 304:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 305:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 306:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 307:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 308:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 309:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 316:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 317:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 318:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 326:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 327:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 328:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 329:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 330:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 331:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[-2] );
			pParser->ToStringUnescape ( tStmt.m_sUdrLib, yyvsp[0] );
		;}
    break;

  case 332:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[0] );
		;}
    break;

  case 333:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 334:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 335:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 336:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 337:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 340:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 341:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 342:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 343:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 345:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 346:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 347:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 348:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 349:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 350:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 351:

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

