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
     TOK_LEVEL = 318,
     TOK_LIKE = 319,
     TOK_LIMIT = 320,
     TOK_MATCH = 321,
     TOK_MAX = 322,
     TOK_META = 323,
     TOK_MIN = 324,
     TOK_MOD = 325,
     TOK_NAMES = 326,
     TOK_NULL = 327,
     TOK_OPTION = 328,
     TOK_ORDER = 329,
     TOK_OPTIMIZE = 330,
     TOK_PLAN = 331,
     TOK_PROFILE = 332,
     TOK_RAND = 333,
     TOK_RAMCHUNK = 334,
     TOK_RANKER = 335,
     TOK_READ = 336,
     TOK_REPEATABLE = 337,
     TOK_REPLACE = 338,
     TOK_RETURNS = 339,
     TOK_ROLLBACK = 340,
     TOK_RTINDEX = 341,
     TOK_SELECT = 342,
     TOK_SERIALIZABLE = 343,
     TOK_SET = 344,
     TOK_SESSION = 345,
     TOK_SHOW = 346,
     TOK_SONAME = 347,
     TOK_START = 348,
     TOK_STATUS = 349,
     TOK_STRING = 350,
     TOK_SUM = 351,
     TOK_TABLE = 352,
     TOK_TABLES = 353,
     TOK_TO = 354,
     TOK_TRANSACTION = 355,
     TOK_TRUE = 356,
     TOK_TRUNCATE = 357,
     TOK_UNCOMMITTED = 358,
     TOK_UPDATE = 359,
     TOK_VALUES = 360,
     TOK_VARIABLES = 361,
     TOK_WARNINGS = 362,
     TOK_WEIGHT = 363,
     TOK_WHERE = 364,
     TOK_WITHIN = 365,
     TOK_OR = 366,
     TOK_AND = 367,
     TOK_NE = 368,
     TOK_GTE = 369,
     TOK_LTE = 370,
     TOK_NOT = 371,
     TOK_NEG = 372
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
#define TOK_LEVEL 318
#define TOK_LIKE 319
#define TOK_LIMIT 320
#define TOK_MATCH 321
#define TOK_MAX 322
#define TOK_META 323
#define TOK_MIN 324
#define TOK_MOD 325
#define TOK_NAMES 326
#define TOK_NULL 327
#define TOK_OPTION 328
#define TOK_ORDER 329
#define TOK_OPTIMIZE 330
#define TOK_PLAN 331
#define TOK_PROFILE 332
#define TOK_RAND 333
#define TOK_RAMCHUNK 334
#define TOK_RANKER 335
#define TOK_READ 336
#define TOK_REPEATABLE 337
#define TOK_REPLACE 338
#define TOK_RETURNS 339
#define TOK_ROLLBACK 340
#define TOK_RTINDEX 341
#define TOK_SELECT 342
#define TOK_SERIALIZABLE 343
#define TOK_SET 344
#define TOK_SESSION 345
#define TOK_SHOW 346
#define TOK_SONAME 347
#define TOK_START 348
#define TOK_STATUS 349
#define TOK_STRING 350
#define TOK_SUM 351
#define TOK_TABLE 352
#define TOK_TABLES 353
#define TOK_TO 354
#define TOK_TRANSACTION 355
#define TOK_TRUE 356
#define TOK_TRUNCATE 357
#define TOK_UNCOMMITTED 358
#define TOK_UPDATE 359
#define TOK_VALUES 360
#define TOK_VARIABLES 361
#define TOK_WARNINGS 362
#define TOK_WEIGHT 363
#define TOK_WHERE 364
#define TOK_WITHIN 365
#define TOK_OR 366
#define TOK_AND 367
#define TOK_NE 368
#define TOK_GTE 369
#define TOK_LTE 370
#define TOK_NOT 371
#define TOK_NEG 372




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
#define YYLAST   1436

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  137
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  120
/* YYNRULES -- Number of rules. */
#define YYNRULES  348
/* YYNRULES -- Number of states. */
#define YYNSTATES  661

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   372

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   125,   114,     2,
     129,   130,   123,   121,   131,   122,   134,   124,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   128,
     117,   115,   118,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   135,     2,   136,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   132,   113,   133,     2,     2,     2,     2,
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
     105,   106,   107,   108,   109,   110,   111,   112,   116,   119,
     120,   126,   127
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
     997,   999,  1001,  1003,  1011,  1016,  1022,  1023,  1025,  1028,
    1030,  1034,  1038,  1041,  1045,  1052,  1053,  1055,  1057,  1060,
    1063,  1066,  1068,  1076,  1078,  1080,  1082,  1084,  1090,  1094,
    1098,  1105,  1109,  1113,  1117,  1119,  1123,  1126,  1130,  1134,
    1137,  1139,  1142,  1144,  1146,  1150,  1154,  1158,  1162
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     138,     0,    -1,   139,    -1,   140,    -1,   140,   128,    -1,
     202,    -1,   210,    -1,   196,    -1,   197,    -1,   200,    -1,
     211,    -1,   220,    -1,   222,    -1,   223,    -1,   224,    -1,
     229,    -1,   234,    -1,   235,    -1,   239,    -1,   243,    -1,
     241,    -1,   242,    -1,   244,    -1,   245,    -1,   246,    -1,
     236,    -1,   247,    -1,   249,    -1,   250,    -1,   251,    -1,
     228,    -1,   141,    -1,   140,   128,   141,    -1,   142,    -1,
     191,    -1,   143,    -1,    87,     3,   129,   129,   143,   130,
     144,   130,    -1,   150,    -1,    87,   151,    47,   129,   147,
     150,   130,   148,   149,    -1,    -1,   131,   145,    -1,   146,
      -1,   145,   131,   146,    -1,     3,    -1,     5,    -1,    54,
      -1,    -1,    74,    26,   174,    -1,    -1,    65,     5,    -1,
      65,     5,   131,     5,    -1,    87,   151,    47,   155,   156,
     166,   170,   169,   172,   176,   178,    -1,   152,    -1,   151,
     131,   152,    -1,   123,    -1,   154,   153,    -1,    -1,     3,
      -1,    18,     3,    -1,   184,    -1,    21,   129,   184,   130,
      -1,    67,   129,   184,   130,    -1,    69,   129,   184,   130,
      -1,    96,   129,   184,   130,    -1,    52,   129,   184,   130,
      -1,    33,   129,   123,   130,    -1,    51,   129,   130,    -1,
      33,   129,    39,     3,   130,    -1,     3,    -1,   155,   131,
       3,    -1,    -1,   157,    -1,   109,   158,    -1,   159,    -1,
     158,   112,   158,    -1,   129,   158,   130,    -1,    66,   129,
       8,   130,    -1,   160,    -1,   162,   115,   163,    -1,   162,
     116,   163,    -1,   162,    55,   129,   165,   130,    -1,   162,
     126,    55,   129,   165,   130,    -1,   162,    55,     9,    -1,
     162,   126,    55,     9,    -1,   162,    23,   163,   112,   163,
      -1,   162,   118,   163,    -1,   162,   117,   163,    -1,   162,
     119,   163,    -1,   162,   120,   163,    -1,   162,   115,   164,
      -1,   161,    -1,   162,    23,   164,   112,   164,    -1,   162,
      23,   163,   112,   164,    -1,   162,    23,   164,   112,   163,
      -1,   162,   118,   164,    -1,   162,   117,   164,    -1,   162,
     119,   164,    -1,   162,   120,   164,    -1,   162,   115,     8,
      -1,   162,   116,     8,    -1,   162,    61,    72,    -1,   162,
      61,   126,    72,    -1,   162,   116,   164,    -1,     3,    -1,
       4,    -1,    33,   129,   123,   130,    -1,    51,   129,   130,
      -1,   108,   129,   130,    -1,    54,    -1,   252,    -1,    59,
     129,   252,   130,    -1,    41,   129,   252,   130,    -1,    24,
     129,   252,   130,    -1,     5,    -1,   122,     5,    -1,     6,
      -1,   122,     6,    -1,    14,    -1,   163,    -1,   165,   131,
     163,    -1,    -1,    50,   167,    26,   168,    -1,    -1,     5,
      -1,   162,    -1,   168,   131,   162,    -1,    -1,    53,   160,
      -1,    -1,   171,    -1,   110,    50,    74,    26,   174,    -1,
      -1,   173,    -1,    74,    26,   174,    -1,    74,    26,    78,
     129,   130,    -1,   175,    -1,   174,   131,   175,    -1,   162,
      -1,   162,    19,    -1,   162,    37,    -1,    -1,   177,    -1,
      65,     5,    -1,    65,     5,   131,     5,    -1,    -1,   179,
      -1,    73,   180,    -1,   181,    -1,   180,   131,   181,    -1,
       3,   115,     3,    -1,    80,   115,     3,    -1,     3,   115,
       5,    -1,     3,   115,   129,   182,   130,    -1,     3,   115,
       3,   129,     8,   130,    -1,    80,   115,     3,   129,     8,
     130,    -1,     3,   115,     8,    -1,    80,   115,     8,    -1,
     183,    -1,   182,   131,   183,    -1,     3,   115,   163,    -1,
       3,    -1,     4,    -1,    54,    -1,     5,    -1,     6,    -1,
      14,    -1,     9,    -1,   122,   184,    -1,   126,   184,    -1,
     184,   121,   184,    -1,   184,   122,   184,    -1,   184,   123,
     184,    -1,   184,   124,   184,    -1,   184,   117,   184,    -1,
     184,   118,   184,    -1,   184,   114,   184,    -1,   184,   113,
     184,    -1,   184,   125,   184,    -1,   184,    40,   184,    -1,
     184,    70,   184,    -1,   184,   120,   184,    -1,   184,   119,
     184,    -1,   184,   115,   184,    -1,   184,   116,   184,    -1,
     184,   112,   184,    -1,   184,   111,   184,    -1,   129,   184,
     130,    -1,   132,   188,   133,    -1,   185,    -1,   252,    -1,
     255,    -1,   252,    61,    72,    -1,   252,    61,   126,    72,
      -1,     3,   129,   186,   130,    -1,    55,   129,   186,   130,
      -1,    59,   129,   186,   130,    -1,    24,   129,   186,   130,
      -1,    44,   129,   186,   130,    -1,    41,   129,   186,   130,
      -1,     3,   129,   130,    -1,    69,   129,   184,   131,   184,
     130,    -1,    67,   129,   184,   131,   184,   130,    -1,   108,
     129,   130,    -1,     3,   129,   184,    46,     3,    55,   252,
     130,    -1,   187,    -1,   186,   131,   187,    -1,   184,    -1,
       8,    -1,   189,   115,   190,    -1,   188,   131,   189,   115,
     190,    -1,     3,    -1,    55,    -1,   163,    -1,     3,    -1,
      91,   193,    -1,    -1,    64,     8,    -1,   107,    -1,    94,
     192,    -1,    68,   192,    -1,    16,    94,   192,    -1,    77,
      -1,    76,    -1,    16,     8,    94,   192,    -1,    16,     3,
      94,   192,    -1,    56,     3,    94,    -1,     3,    -1,    72,
      -1,     8,    -1,     5,    -1,     6,    -1,   194,    -1,   195,
     122,   194,    -1,    89,     3,   115,   199,    -1,    89,     3,
     115,   198,    -1,    89,     3,   115,    72,    -1,    89,    71,
     195,    -1,    89,    10,   115,   195,    -1,    89,    28,    89,
     195,    -1,    89,    49,     9,   115,   129,   165,   130,    -1,
      89,    49,     3,   115,   198,    -1,     3,    -1,     8,    -1,
     101,    -1,    43,    -1,   163,    -1,    31,    -1,    85,    -1,
     201,    -1,    22,    -1,    93,   100,    -1,   203,    60,     3,
     204,   105,   206,    -1,    57,    -1,    83,    -1,    -1,   129,
     205,   130,    -1,   162,    -1,   205,   131,   162,    -1,   207,
      -1,   206,   131,   207,    -1,   129,   208,   130,    -1,   209,
      -1,   208,   131,   209,    -1,   163,    -1,   164,    -1,     8,
      -1,   129,   165,   130,    -1,   129,   130,    -1,    36,    47,
     155,   157,    -1,    27,     3,   129,   212,   215,   130,    -1,
     213,    -1,   212,   131,   213,    -1,   209,    -1,   129,   214,
     130,    -1,     8,    -1,   214,   131,     8,    -1,    -1,   131,
     216,    -1,   217,    -1,   216,   131,   217,    -1,   209,   218,
     219,    -1,    -1,    18,    -1,     3,    -1,    65,    -1,   221,
       3,   192,    -1,    38,    -1,    37,    -1,    91,    98,   192,
      -1,    91,    35,   192,    -1,   104,   155,    89,   225,   157,
     178,    -1,   226,    -1,   225,   131,   226,    -1,     3,   115,
     163,    -1,     3,   115,   164,    -1,     3,   115,   129,   165,
     130,    -1,     3,   115,   129,   130,    -1,   252,   115,   163,
      -1,   252,   115,   164,    -1,    59,    -1,    24,    -1,    44,
      -1,    25,    -1,    17,    97,     3,    15,    30,     3,   227,
      -1,    91,   237,   106,   230,    -1,    91,   237,   106,    64,
       8,    -1,    -1,   231,    -1,   109,   232,    -1,   233,    -1,
     232,   111,   233,    -1,     3,   115,     8,    -1,    91,    29,
      -1,    91,    28,    89,    -1,    89,   237,   100,    62,    63,
     238,    -1,    -1,    49,    -1,    90,    -1,    81,   103,    -1,
      81,    32,    -1,    82,    81,    -1,    88,    -1,    34,    48,
       3,    84,   240,    92,     8,    -1,    58,    -1,    24,    -1,
      44,    -1,    95,    -1,    34,    80,     3,    92,     8,    -1,
      42,    80,     3,    -1,    42,    48,     3,    -1,    20,    56,
       3,    99,    86,     3,    -1,    45,    86,     3,    -1,    45,
      79,     3,    -1,    87,   248,   176,    -1,    10,    -1,    10,
     134,     3,    -1,    87,   184,    -1,   102,    86,     3,    -1,
      75,    56,     3,    -1,     3,   253,    -1,   254,    -1,   253,
     254,    -1,    13,    -1,    14,    -1,   135,   184,   136,    -1,
     135,     8,   136,    -1,   184,   115,   256,    -1,   256,   115,
     184,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   163,   163,   164,   165,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   200,   201,   205,   206,   210,   211,   219,   220,   227,
     229,   233,   237,   244,   245,   246,   250,   263,   270,   272,
     277,   286,   302,   303,   307,   308,   311,   313,   314,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   330,   331,
     334,   336,   340,   344,   345,   346,   350,   355,   359,   366,
     374,   382,   391,   396,   401,   406,   411,   416,   421,   426,
     431,   436,   441,   446,   451,   456,   461,   466,   471,   476,
     481,   486,   494,   498,   499,   504,   510,   516,   522,   528,
     529,   530,   531,   535,   536,   547,   548,   549,   553,   559,
     565,   567,   570,   572,   579,   583,   589,   591,   597,   599,
     603,   614,   616,   620,   624,   631,   632,   636,   637,   638,
     641,   643,   647,   652,   659,   661,   665,   669,   670,   674,
     679,   685,   690,   696,   701,   706,   711,   719,   724,   731,
     741,   742,   743,   744,   745,   746,   747,   748,   749,   751,
     752,   753,   754,   755,   756,   757,   758,   759,   760,   761,
     762,   763,   764,   765,   766,   767,   768,   769,   770,   771,
     772,   773,   774,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   792,   793,   797,   798,   802,   803,
     807,   808,   812,   813,   819,   822,   824,   828,   829,   830,
     831,   832,   833,   834,   839,   844,   854,   855,   856,   857,
     858,   862,   863,   867,   872,   877,   882,   883,   884,   888,
     893,   901,   902,   906,   907,   908,   922,   923,   924,   928,
     929,   935,   943,   944,   947,   949,   953,   954,   958,   959,
     963,   967,   968,   972,   973,   974,   975,   976,   982,   992,
    1000,  1004,  1011,  1012,  1019,  1029,  1035,  1037,  1041,  1046,
    1050,  1057,  1059,  1063,  1064,  1070,  1078,  1079,  1085,  1089,
    1095,  1103,  1104,  1108,  1122,  1128,  1132,  1137,  1151,  1162,
    1163,  1164,  1165,  1169,  1182,  1186,  1193,  1194,  1198,  1202,
    1203,  1207,  1211,  1218,  1225,  1231,  1232,  1233,  1237,  1238,
    1239,  1240,  1246,  1257,  1258,  1259,  1260,  1264,  1274,  1283,
    1294,  1306,  1315,  1326,  1334,  1335,  1339,  1349,  1360,  1371,
    1374,  1375,  1379,  1380,  1381,  1382,  1386,  1387,  1391
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
  "TOK_INTEGER", "TOK_INTO", "TOK_IS", "TOK_ISOLATION", "TOK_LEVEL", 
  "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", 
  "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", 
  "TOK_RANKER", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", 
  "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", 
  "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
  "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", 
  "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
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
     365,   366,   367,   124,    38,    61,   368,    60,    62,   369,
     370,    43,    45,    42,    47,    37,   371,   372,    59,    40,
      41,    44,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   137,   138,   138,   138,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   140,   140,   141,   141,   142,   142,   143,   143,   144,
     144,   145,   145,   146,   146,   146,   147,   148,   149,   149,
     149,   150,   151,   151,   152,   152,   153,   153,   153,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   155,   155,
     156,   156,   157,   158,   158,   158,   159,   159,   160,   160,
     160,   160,   160,   160,   160,   160,   160,   160,   160,   160,
     160,   160,   160,   160,   160,   160,   160,   160,   160,   160,
     160,   160,   161,   162,   162,   162,   162,   162,   162,   162,
     162,   162,   162,   163,   163,   164,   164,   164,   165,   165,
     166,   166,   167,   167,   168,   168,   169,   169,   170,   170,
     171,   172,   172,   173,   173,   174,   174,   175,   175,   175,
     176,   176,   177,   177,   178,   178,   179,   180,   180,   181,
     181,   181,   181,   181,   181,   181,   181,   182,   182,   183,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   186,   186,   187,   187,   188,   188,
     189,   189,   190,   190,   191,   192,   192,   193,   193,   193,
     193,   193,   193,   193,   193,   193,   194,   194,   194,   194,
     194,   195,   195,   196,   196,   196,   196,   196,   196,   197,
     197,   198,   198,   199,   199,   199,   200,   200,   200,   201,
     201,   202,   203,   203,   204,   204,   205,   205,   206,   206,
     207,   208,   208,   209,   209,   209,   209,   209,   210,   211,
     212,   212,   213,   213,   214,   214,   215,   215,   216,   216,
     217,   218,   218,   219,   219,   220,   221,   221,   222,   223,
     224,   225,   225,   226,   226,   226,   226,   226,   226,   227,
     227,   227,   227,   228,   229,   229,   230,   230,   231,   232,
     232,   233,   234,   235,   236,   237,   237,   237,   238,   238,
     238,   238,   239,   240,   240,   240,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   248,   249,   250,   251,   252,
     253,   253,   254,   254,   254,   254,   255,   255,   256
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
       1,     1,     1,     7,     4,     5,     0,     1,     2,     1,
       3,     3,     2,     3,     6,     0,     1,     1,     2,     2,
       2,     1,     7,     1,     1,     1,     1,     5,     3,     3,
       6,     3,     3,     3,     1,     3,     2,     3,     3,     2,
       1,     2,     1,     1,     3,     3,     3,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   249,     0,   246,     0,     0,   287,   286,
       0,     0,   252,     0,   253,   247,     0,   315,   315,     0,
       0,     0,     0,     2,     3,    31,    33,    35,    37,    34,
       7,     8,     9,   248,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    30,    15,    16,    17,    25,    18,    20,
      21,    19,    22,    23,    24,    26,    27,    28,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,   161,   163,   164,   348,   166,   334,   165,     0,     0,
       0,     0,     0,     0,     0,   162,     0,     0,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,    52,    56,
      59,   188,   140,   189,   190,     0,     0,     0,     0,   316,
       0,   317,     0,     0,     0,   312,   215,   316,     0,   215,
     222,   221,   215,   215,   217,   214,     0,   250,     0,    68,
       0,     1,     4,     0,   215,     0,     0,     0,     0,     0,
       0,   329,   328,   332,   331,   338,   342,   343,     0,     0,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,     0,   167,
     168,     0,   210,   211,     0,     0,     0,     0,    57,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   333,
     141,     0,     0,     0,     0,     0,     0,     0,   226,   229,
     230,   228,   227,   231,   236,     0,     0,     0,   215,   313,
       0,   289,     0,   219,   218,   288,   306,   337,     0,     0,
       0,     0,    32,   254,   285,     0,     0,   113,   115,   265,
     117,     0,     0,   263,   264,   272,   276,   270,     0,     0,
       0,   268,   207,     0,   199,   206,     0,   204,   348,     0,
     341,   335,     0,   206,     0,     0,     0,     0,     0,    66,
       0,     0,     0,     0,     0,     0,   202,     0,     0,     0,
     186,     0,   187,     0,    46,    70,    53,    59,    58,   178,
     179,   185,   184,   176,   175,   182,   346,   183,   173,   174,
     181,   180,   169,   170,   171,   172,   177,   142,   191,     0,
     347,   241,   242,   244,   235,   243,     0,   245,   234,   233,
     237,   238,     0,     0,     0,     0,   215,   215,   220,   216,
     225,     0,     0,   304,   307,     0,     0,   291,     0,    69,
       0,     0,     0,     0,   114,   116,   274,   267,   118,     0,
       0,     0,     0,   324,   325,   323,   326,     0,   327,   103,
     104,     0,     0,     0,     0,   108,     0,     0,     0,     0,
      72,    73,    77,    90,     0,   109,     0,     0,     0,   193,
       0,   345,   344,    60,   196,     0,    65,   198,   197,    64,
     194,   195,    61,     0,    62,     0,    63,     0,     0,     0,
     213,   212,   208,     0,   120,    71,     0,   192,   240,     0,
     232,     0,   224,   223,   305,     0,   308,   309,     0,     0,
     144,     0,   256,     0,     0,     0,   330,   266,     0,   273,
       0,   272,   271,   277,   278,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    39,     0,   205,    67,
       0,     0,     0,     0,     0,   122,   128,   143,     0,     0,
       0,   321,   314,     0,     0,     0,   293,   294,   292,     0,
     290,   145,   297,   298,   255,     0,     0,   251,   258,   300,
     302,   301,   299,   303,   119,   275,   282,     0,     0,   322,
       0,     0,     0,     0,   106,     0,     0,   107,    75,    74,
       0,     0,    82,     0,   100,     0,    98,    78,    89,    99,
      79,   102,    86,    95,    85,    94,    87,    96,    88,    97,
       0,     0,     0,     0,   201,   200,   209,     0,     0,   123,
       0,     0,   126,   129,   239,   319,   318,   320,   311,   310,
     296,     0,     0,     0,   146,   147,   257,     0,     0,   261,
       0,   283,   284,   280,   281,   279,   112,   105,   111,   110,
      76,     0,     0,     0,   101,    83,     0,    43,    44,    45,
      40,    41,    36,     0,     0,     0,    48,     0,     0,     0,
     131,   295,     0,     0,     0,   260,     0,   259,    84,    92,
      93,    91,    80,     0,     0,   203,     0,     0,    38,   124,
     121,     0,   127,     0,   140,   132,   149,   151,   155,     0,
     150,   156,   148,   262,    81,    42,   137,    47,   135,    49,
       0,     0,     0,   144,     0,     0,     0,   157,     0,   138,
     139,     0,     0,   125,   130,     0,   133,    51,     0,     0,
     152,     0,     0,   136,    50,     0,   153,   159,   158,   154,
     134
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   532,   580,   581,
     403,   586,   608,    28,    97,    98,   180,    99,   285,   404,
     251,   370,   371,   372,   373,   374,   348,   244,   349,   466,
     540,   610,   590,   542,   543,   614,   615,   627,   628,   199,
     200,   480,   481,   554,   555,   636,   637,   263,   101,   256,
     257,   174,   175,   402,    29,   221,   125,   213,   214,    30,
      31,   318,   319,    32,    33,    34,    35,   341,   423,   487,
     488,   558,   245,    36,    37,   246,   247,   350,   352,   433,
     434,   497,   563,    38,    39,    40,    41,    42,   336,   337,
     493,    43,    44,   333,   334,   416,   417,    45,    46,    47,
     112,   472,    48,   357,    49,    50,    51,    52,    53,    54,
      55,   102,    56,    57,    58,   103,   150,   151,   104,   105
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -408
static const short yypact[] =
{
    1295,   -30,    49,  -408,   116,  -408,   -21,   147,  -408,  -408,
     148,   154,  -408,   149,  -408,  -408,   334,   187,   461,   119,
     160,   245,   254,  -408,   129,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,   202,  -408,  -408,  -408,   277,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,   280,
     282,   142,   287,   302,   245,   326,   362,   370,   377,   381,
       9,  -408,  -408,  -408,  -408,  -408,   174,  -408,   258,   261,
     263,   271,   273,   278,   281,  -408,   284,   285,   289,   290,
     298,   299,   746,  -408,   746,   746,    30,   -19,  -408,   199,
     837,  -408,   341,   348,  -408,   296,   297,   314,   343,   234,
     195,  -408,   333,    92,   347,  -408,   373,  -408,   435,   373,
    -408,  -408,   373,   373,  -408,  -408,   335,  -408,   436,  -408,
     -15,  -408,   158,   437,   373,   428,   345,    84,   361,   354,
     111,  -408,  -408,  -408,  -408,  -408,  -408,  -408,   574,   815,
      24,  -408,   444,   746,   857,    18,   857,   857,   318,   746,
     857,   857,   746,   746,   746,   320,    27,   322,   323,  -408,
    -408,  1037,  -408,  -408,    77,   338,    -2,   464,  -408,   451,
    -408,   746,   746,   746,   746,   746,   746,   746,   746,   746,
     746,   746,   746,   746,   746,   746,   746,   746,   450,  -408,
    -408,   -38,   746,    21,   195,   195,   346,   349,  -408,  -408,
    -408,  -408,  -408,  -408,   336,   400,   371,   380,   373,  -408,
     463,  -408,   382,  -408,  -408,  -408,    -1,  -408,   472,   476,
     540,   179,  -408,   351,  -408,   452,   395,  -408,  -408,  -408,
    -408,   283,     7,  -408,  -408,  -408,   352,  -408,   183,   478,
      28,  -408,   369,   670,  -408,  1243,   162,  -408,   355,   893,
    -408,  -408,  1068,  1299,   164,   484,   363,   169,   171,  -408,
    1099,   201,   204,   924,   950,  1124,  -408,   704,   746,   746,
    -408,    30,  -408,    65,  -408,   111,  -408,  1299,  -408,  -408,
    -408,   728,   536,   852,  1311,   568,  -408,   568,   198,   198,
     198,   198,   151,   151,  -408,  -408,  -408,   364,  -408,   420,
     568,  -408,  -408,  -408,  -408,  -408,   489,  -408,  -408,  -408,
     336,   336,    83,   372,   195,   439,   373,   373,  -408,  -408,
    -408,   490,   496,  -408,  -408,    -4,   120,  -408,   385,  -408,
     300,   398,   501,   503,  -408,  -408,  -408,  -408,  -408,   216,
     233,    84,   379,  -408,  -408,  -408,  -408,   415,  -408,    24,
    -408,   384,   391,   392,   393,  -408,   396,   397,   399,    28,
     402,  -408,  -408,  -408,   305,  -408,   464,   394,   508,  -408,
     857,  -408,  -408,  -408,  -408,   404,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,   746,  -408,   746,  -408,   981,  1012,   412,
    -408,  -408,  -408,   443,   482,  -408,   530,  -408,  -408,    67,
    -408,   178,  -408,  -408,  -408,   421,   429,  -408,    70,   472,
     466,    44,  -408,   238,   413,   286,  -408,  -408,    67,  -408,
     533,    59,  -408,   416,  -408,  -408,   542,   549,   430,   549,
     426,   549,   550,   427,   -59,    28,    44,    -6,   -12,   101,
     112,    44,    44,    44,    44,   507,   432,   510,  -408,  -408,
    1155,  1186,    65,   464,   440,   561,   457,  -408,   240,    -7,
     488,  -408,  -408,   563,   496,    25,  -408,  -408,  -408,    17,
    -408,  -408,  -408,  -408,  -408,   300,    96,   454,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,    48,    96,  -408,
      24,   445,   459,   467,  -408,   470,   471,  -408,  -408,  -408,
     462,   491,  -408,    67,  -408,   532,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
      56,   155,   475,   549,  -408,  -408,  -408,     1,   528,  -408,
     584,   562,   558,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,   246,   498,   499,   485,  -408,  -408,    31,   252,  -408,
     413,  -408,  -408,  -408,   599,  -408,  -408,  -408,  -408,  -408,
    -408,    44,    44,   264,  -408,  -408,    67,  -408,  -408,  -408,
     492,  -408,  -408,   494,   245,   593,   555,   300,   547,   300,
     548,  -408,    13,   125,    17,  -408,    96,  -408,  -408,  -408,
    -408,  -408,  -408,   266,   155,  -408,   300,   620,  -408,  -408,
     495,   601,  -408,   604,   341,  -408,   502,  -408,  -408,   629,
     505,  -408,  -408,  -408,  -408,  -408,    36,   504,  -408,   506,
     300,   300,   228,   466,   631,   525,   268,  -408,   634,  -408,
    -408,   300,   639,  -408,   504,   516,   504,  -408,   517,    67,
    -408,   629,   534,  -408,  -408,   535,  -408,  -408,  -408,  -408,
    -408
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -408,  -408,  -408,  -408,   514,  -408,   414,  -408,  -408,    64,
    -408,  -408,  -408,   267,   208,   500,  -408,  -408,    35,  -408,
     -92,  -315,  -408,    91,  -408,  -334,  -137,  -407,  -392,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -305,    40,    69,
    -408,    62,  -408,  -408,   103,  -408,    47,   -14,  -408,   196,
     319,  -408,   424,   239,  -408,  -115,  -408,   378,   200,  -408,
    -408,   401,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
     156,  -408,  -346,  -408,  -408,  -408,   366,  -408,  -408,  -408,
     217,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,   301,
    -408,  -408,  -408,  -408,  -408,  -408,   247,  -408,  -408,  -408,
     701,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -215,  -408,   572,  -408,   539
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -349
static const short yytable[] =
{
     243,   129,   100,   512,   223,   431,   422,   224,   225,   146,
     147,   477,   237,   338,   483,   346,   616,   468,   617,   234,
     552,   618,   146,   147,   311,   545,   237,    62,   176,   312,
     237,   359,   360,   172,   308,   375,   237,   146,   147,   511,
     146,   147,   518,   521,   523,   525,   527,   529,   584,   237,
     238,   561,   361,   445,   444,   639,   130,   265,   240,    63,
     514,   362,  -281,   331,   313,   575,   317,    59,   400,   363,
     237,   508,   237,   640,   228,   237,   238,   496,   169,   364,
     170,   171,   365,   551,   240,   173,   311,   366,   309,   237,
     238,   312,   239,   314,   367,   216,   546,   553,   240,   140,
     217,   237,   238,   328,   239,    60,   237,   238,   332,   516,
     240,   418,   177,   562,   515,   240,   229,   237,   238,    61,
     519,   573,   315,   513,  -281,   375,   240,   284,   620,   316,
     509,   149,   177,   621,   255,   259,   368,   347,   148,   262,
     559,   266,   619,   316,   149,   270,   401,   316,   273,   274,
     275,   556,   564,   316,   375,   550,   277,   369,   577,   149,
     578,   347,   149,   287,   599,   601,   241,   289,   290,   291,
     292,   293,   294,   295,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   603,   576,   218,   316,   310,   316,
     106,   181,   241,   405,    64,   113,    65,   107,   208,   475,
     209,   210,   178,   211,   338,    69,   241,   353,   281,   579,
     282,   412,   413,   242,   243,   108,   287,   179,   241,   127,
     250,   182,   501,   241,   503,   557,   505,   354,    66,   250,
     375,   359,   360,    67,   241,   118,   109,   206,   181,   171,
      68,   355,   229,   207,   420,   230,   128,   119,   129,   231,
     623,   419,   361,   609,   131,   120,   121,   132,   110,   469,
     470,   362,   133,   255,   397,   398,   471,   212,   182,   363,
     375,   137,   626,   122,   195,   196,   197,   111,   356,   364,
     134,   476,   365,   135,   482,   136,   124,   366,   344,   345,
     138,   494,   379,   380,   384,   380,   643,   626,   626,   387,
     380,   388,   380,   359,   360,   139,   645,   626,   152,   510,
     489,   490,   517,   520,   522,   524,   526,   528,   583,   193,
     194,   195,   196,   197,   361,   401,   644,   646,   446,   141,
     491,   390,   380,   362,   391,   380,   368,    70,    71,    72,
      73,   363,    74,    75,    76,   492,   427,   428,    77,   243,
     264,   364,   267,   268,   365,    78,   271,   272,    79,   366,
     447,   243,   287,   429,   430,   142,   448,    80,   484,   485,
     544,   428,   375,   143,   375,    81,   591,   428,    82,   460,
     144,   461,   595,   596,   145,    83,    84,   153,    85,    86,
     154,   375,   155,    87,   602,   428,   624,   428,   650,   651,
     156,    88,   157,    89,   320,   321,   198,   158,   368,   201,
     159,   202,   203,   160,   161,   375,   375,   375,   162,   163,
     449,   450,   451,   452,   453,   454,   375,   164,   165,   204,
      90,   455,   205,   215,   598,   600,   219,   220,   222,   227,
     233,   226,    91,   235,   236,   248,   249,   261,   269,   287,
     276,   278,   279,   283,   288,   307,    92,    93,   324,   243,
      94,   322,   325,    95,   323,   326,    96,   166,    71,    72,
      73,   329,    74,    75,   327,   335,   330,   113,    77,   339,
     340,   343,   342,   351,  -348,    78,   358,   385,    79,   114,
     115,   381,   407,   386,   344,   406,   116,    80,   414,   415,
     421,   409,   411,   424,   425,    81,   426,   436,    82,   435,
     117,   457,   657,   437,   445,    83,    84,   118,    85,    86,
     438,   439,   440,    87,   456,   441,   442,   462,   443,   119,
     463,    88,   465,    89,   459,   467,   473,   120,   121,   479,
     474,   495,   486,    70,    71,    72,    73,   498,    74,    75,
     499,   111,   500,   502,    77,   122,   504,   507,   506,   123,
      90,    78,   530,   531,    79,   533,   539,   541,   124,   547,
     538,   548,    91,    80,   571,   566,   181,   166,    71,    72,
      73,    81,   252,    75,    82,   560,    92,    93,    77,   567,
      94,    83,    84,    95,    85,    86,    96,   568,    79,    87,
     569,   570,   585,   572,   574,   582,   182,    88,   181,    89,
     587,   589,   588,   592,   593,    81,   594,   496,    82,   606,
     607,   611,   613,   604,   605,   629,   630,   631,    85,    86,
     632,   634,   635,    87,   638,   641,    90,   642,   182,   648,
     649,   167,   652,   168,   654,   655,   232,   656,    91,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    92,    93,   659,   660,    94,   377,   625,    95,
     464,   537,    96,   166,    71,    72,    73,   286,    74,    75,
     612,   653,    91,   633,    77,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    79,   647,    92,   622,   658,   458,
      94,   536,   410,   253,   254,   399,    96,   166,    71,    72,
      73,    81,   252,    75,    82,   565,   597,   432,    77,   126,
     478,   549,   260,   408,    85,    86,   296,     0,    79,    87,
       0,     0,     0,     0,     0,     0,     0,   167,     0,   168,
       0,     0,     0,     0,     0,    81,     0,     0,    82,   166,
      71,    72,    73,     0,    74,    75,     0,   376,    85,    86,
      77,     0,     0,    87,     0,     0,     0,     0,   181,     0,
      79,   167,     0,   168,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,    81,     0,     0,
      82,     0,    92,     0,     0,     0,    94,     0,   182,    95,
      85,    86,    96,     0,     0,    87,     0,     0,     0,     0,
       0,     0,    91,   167,     0,   168,     0,     0,   166,    71,
      72,    73,     0,   258,    75,     0,    92,     0,     0,    77,
      94,     0,     0,    95,   254,     0,    96,  -336,     0,    79,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    91,     0,    81,     0,     0,    82,
     166,    71,    72,    73,     0,   252,    75,     0,    92,    85,
      86,    77,    94,     0,    87,    95,     0,   181,    96,     0,
       0,    79,   167,     0,   168,     0,     0,     0,     0,     0,
       0,     0,   181,     0,     0,     0,     0,     0,    81,     0,
       0,    82,     0,     0,     0,     0,     0,   182,     0,     0,
       0,    85,    86,     0,     0,     0,    87,     0,     0,     0,
       0,     0,   182,    91,   167,     0,   168,     0,     0,     0,
       0,     0,     0,   181,     0,     0,     0,    92,     0,     0,
       0,    94,     0,     0,    95,     0,     0,    96,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   182,   181,    91,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,     0,    92,
       0,     0,     0,    94,     0,     0,    95,     0,     0,    96,
     181,     0,     0,     0,   182,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,     0,
     182,   181,     0,     0,     0,     0,     0,     0,     0,   382,
       0,     0,     0,     0,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
       0,   182,   181,     0,   392,   393,     0,     0,     0,     0,
       0,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,     0,   181,     0,     0,
     394,   395,   182,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   182,   181,     0,
       0,     0,   393,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   182,   181,
       0,     0,     0,   395,     0,     0,     0,     0,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,     0,   181,     0,     0,   280,     0,   182,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   182,   181,     0,     0,   383,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   182,   181,     0,     0,   389,
       0,     0,     0,     0,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
       0,     0,     0,     0,   396,     0,   182,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,     0,     0,   181,     0,   534,     0,     0,     0,   378,
       0,     0,     0,     0,     0,     0,     0,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,     1,   182,     0,     2,   535,     3,     0,     0,
       0,     0,     4,     0,     0,     0,     5,     0,     0,     6,
       0,     7,     8,     9,     0,     0,     0,    10,     0,   181,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    12,     0,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   182,
      13,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,   182,    16,     0,    17,     0,    18,     0,    19,     0,
       0,     0,     0,     0,     0,     0,     0,    20,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,     0,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197
};

static const short yycheck[] =
{
     137,     3,    16,     9,   119,   351,   340,   122,   123,    13,
      14,   418,     5,   228,   421,     8,     3,   409,     5,   134,
       3,     8,    13,    14,     3,    32,     5,    48,    47,     8,
       5,     3,     4,     3,    72,   250,     5,    13,    14,   446,
      13,    14,   449,   450,   451,   452,   453,   454,    47,     5,
       6,     3,    24,   112,   369,    19,    21,    39,    14,    80,
      72,    33,     3,    64,    43,     9,   203,    97,     3,    41,
       5,   130,     5,    37,    89,     5,     6,    18,    92,    51,
      94,    95,    54,   475,    14,    55,     3,    59,   126,     5,
       6,     8,     8,    72,    66,     3,   103,    80,    14,    64,
       8,     5,     6,   218,     8,    56,     5,     6,   109,     8,
      14,   115,   131,    65,   126,    14,   131,     5,     6,     3,
       8,   513,   101,   129,    65,   340,    14,   129,     3,   122,
     445,   135,   131,     8,   148,   149,   108,   130,   129,   153,
     486,   123,   129,   122,   135,   159,   283,   122,   162,   163,
     164,   485,   498,   122,   369,   130,   129,   129,     3,   135,
       5,   130,   135,   177,   571,   572,   122,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   576,   129,    94,   122,   202,   122,
       3,    40,   122,   285,    47,    16,    48,    10,     3,   129,
       5,     6,     3,     8,   419,    56,   122,    24,   131,    54,
     133,   326,   327,   129,   351,    28,   230,    18,   122,   100,
     109,    70,   437,   122,   439,   129,   441,    44,    80,   109,
     445,     3,     4,    79,   122,    56,    49,     3,    40,   253,
      86,    58,   131,     9,   336,    87,    86,    68,     3,    91,
     596,   131,    24,   587,     0,    76,    77,   128,    71,    81,
      82,    33,    60,   277,   278,   279,    88,    72,    70,    41,
     485,   129,   606,    94,   123,   124,   125,    90,    95,    51,
       3,   418,    54,     3,   421,     3,   107,    59,     5,     6,
       3,   428,   130,   131,   130,   131,   630,   631,   632,   130,
     131,   130,   131,     3,     4,     3,    78,   641,   134,   446,
      24,    25,   449,   450,   451,   452,   453,   454,   533,   121,
     122,   123,   124,   125,    24,   462,   631,   632,    23,     3,
      44,   130,   131,    33,   130,   131,   108,     3,     4,     5,
       6,    41,     8,     9,    10,    59,   130,   131,    14,   486,
     154,    51,   156,   157,    54,    21,   160,   161,    24,    59,
      55,   498,   376,   130,   131,     3,    61,    33,   130,   131,
     130,   131,   587,     3,   589,    41,   130,   131,    44,   393,
       3,   395,   130,   131,     3,    51,    52,   129,    54,    55,
     129,   606,   129,    59,   130,   131,   130,   131,   130,   131,
     129,    67,   129,    69,   204,   205,    65,   129,   108,    61,
     129,   115,   115,   129,   129,   630,   631,   632,   129,   129,
     115,   116,   117,   118,   119,   120,   641,   129,   129,   115,
      96,   126,    89,   100,   571,   572,    89,    64,     3,     3,
       3,   106,   108,    15,    99,    84,    92,     3,   130,   463,
     130,   129,   129,   115,     3,     5,   122,   123,   122,   596,
     126,   115,    62,   129,   115,    94,   132,     3,     4,     5,
       6,     8,     8,     9,    94,     3,    94,    16,    14,     3,
     129,    86,    30,   131,   115,    21,     8,     3,    24,    28,
      29,   136,    72,   130,     5,   131,    35,    33,     8,     3,
     115,   129,    63,   105,     3,    41,     3,    92,    44,   130,
      49,     3,   649,   129,   112,    51,    52,    56,    54,    55,
     129,   129,   129,    59,   130,   129,   129,   115,   129,    68,
      87,    67,    50,    69,   130,     5,   115,    76,    77,    73,
     111,     8,   129,     3,     4,     5,     6,   131,     8,     9,
       8,    90,     3,   123,    14,    94,   130,   130,     8,    98,
      96,    21,    55,   131,    24,    55,     5,   110,   107,    81,
     130,     8,   108,    33,   112,   130,    40,     3,     4,     5,
       6,    41,     8,     9,    44,   131,   122,   123,    14,   130,
     126,    51,    52,   129,    54,    55,   132,   130,    24,    59,
     130,   130,    74,   112,    72,   130,    70,    67,    40,    69,
      26,    53,    50,   115,   115,    41,   131,    18,    44,    26,
      65,    74,    74,   131,   130,     5,   131,    26,    54,    55,
      26,   129,     3,    59,   129,   131,    96,   131,    70,     8,
     115,    67,     8,    69,     5,   129,   132,   130,   108,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   122,   123,   130,   130,   126,   253,   604,   129,
     403,   463,   132,     3,     4,     5,     6,   177,     8,     9,
     589,   641,   108,   614,    14,   117,   118,   119,   120,   121,
     122,   123,   124,   125,    24,   633,   122,   594,   651,   380,
     126,   462,   324,   129,   130,   281,   132,     3,     4,     5,
       6,    41,     8,     9,    44,   498,   560,   351,    14,    18,
     419,   474,   150,   322,    54,    55,   187,    -1,    24,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    44,     3,
       4,     5,     6,    -1,     8,     9,    -1,    87,    54,    55,
      14,    -1,    -1,    59,    -1,    -1,    -1,    -1,    40,    -1,
      24,    67,    -1,    69,    -1,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    -1,   122,    -1,    -1,    -1,   126,    -1,    70,   129,
      54,    55,   132,    -1,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    67,    -1,    69,    -1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    -1,   122,    -1,    -1,    14,
     126,    -1,    -1,   129,   130,    -1,   132,     0,    -1,    24,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   108,    -1,    41,    -1,    -1,    44,
       3,     4,     5,     6,    -1,     8,     9,    -1,   122,    54,
      55,    14,   126,    -1,    59,   129,    -1,    40,   132,    -1,
      -1,    24,    67,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    54,    55,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    70,   108,    67,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,   122,    -1,    -1,
      -1,   126,    -1,    -1,   129,    -1,    -1,   132,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,    70,    40,   108,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,    -1,   122,
      -1,    -1,    -1,   126,    -1,    -1,   129,    -1,    -1,   132,
      40,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,    -1,
      70,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
      -1,    70,    40,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,    -1,    40,    -1,    -1,
     130,   131,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,    70,    40,    -1,
      -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,    70,    40,
      -1,    -1,    -1,   131,    -1,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,    -1,    40,    -1,    -1,   130,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,    70,    40,    -1,    -1,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,    70,    40,    -1,    -1,   130,
      -1,    -1,    -1,    -1,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
      -1,    -1,    -1,    -1,   130,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,    -1,    -1,    40,    -1,   130,    -1,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,    17,    70,    -1,    20,   130,    22,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    31,    -1,    -1,    34,
      -1,    36,    37,    38,    -1,    -1,    -1,    42,    -1,    40,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    57,    -1,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,    70,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    -1,
      85,    70,    87,    -1,    89,    -1,    91,    -1,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,    -1,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    45,    57,    75,    83,    85,    87,    89,    91,    93,
     102,   104,   138,   139,   140,   141,   142,   143,   150,   191,
     196,   197,   200,   201,   202,   203,   210,   211,   220,   221,
     222,   223,   224,   228,   229,   234,   235,   236,   239,   241,
     242,   243,   244,   245,   246,   247,   249,   250,   251,    97,
      56,     3,    48,    80,    47,    48,    80,    79,    86,    56,
       3,     4,     5,     6,     8,     9,    10,    14,    21,    24,
      33,    41,    44,    51,    52,    54,    55,    59,    67,    69,
      96,   108,   122,   123,   126,   129,   132,   151,   152,   154,
     184,   185,   248,   252,   255,   256,     3,    10,    28,    49,
      71,    90,   237,    16,    28,    29,    35,    49,    56,    68,
      76,    77,    94,    98,   107,   193,   237,   100,    86,     3,
     155,     0,   128,    60,     3,     3,     3,   129,     3,     3,
     155,     3,     3,     3,     3,     3,    13,    14,   129,   135,
     253,   254,   134,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,     3,    67,    69,   184,
     184,   184,     3,    55,   188,   189,    47,   131,     3,    18,
     153,    40,    70,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,    65,   176,
     177,    61,   115,   115,   115,    89,     3,     9,     3,     5,
       6,     8,    72,   194,   195,   100,     3,     8,    94,    89,
      64,   192,     3,   192,   192,   192,   106,     3,    89,   131,
      87,    91,   141,     3,   192,    15,    99,     5,     6,     8,
      14,   122,   129,   163,   164,   209,   212,   213,    84,    92,
     109,   157,     8,   129,   130,   184,   186,   187,     8,   184,
     254,     3,   184,   184,   186,    39,   123,   186,   186,   130,
     184,   186,   186,   184,   184,   184,   130,   129,   129,   129,
     130,   131,   133,   115,   129,   155,   152,   184,     3,   184,
     184,   184,   184,   184,   184,   184,   256,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,     5,    72,   126,
     184,     3,     8,    43,    72,   101,   122,   163,   198,   199,
     195,   195,   115,   115,   122,    62,    94,    94,   192,     8,
      94,    64,   109,   230,   231,     3,   225,   226,   252,     3,
     129,   204,    30,    86,     5,     6,     8,   130,   163,   165,
     214,   131,   215,    24,    44,    58,    95,   240,     8,     3,
       4,    24,    33,    41,    51,    54,    59,    66,   108,   129,
     158,   159,   160,   161,   162,   252,    87,   143,    46,   130,
     131,   136,   136,   130,   130,     3,   130,   130,   130,   130,
     130,   130,   130,   131,   130,   131,   130,   184,   184,   189,
       3,   163,   190,   147,   156,   157,   131,    72,   198,   129,
     194,    63,   192,   192,     8,     3,   232,   233,   115,   131,
     157,   115,   162,   205,   105,     3,     3,   130,   131,   130,
     131,   209,   213,   216,   217,   130,    92,   129,   129,   129,
     129,   129,   129,   129,   158,   112,    23,    55,    61,   115,
     116,   117,   118,   119,   120,   126,   130,     3,   187,   130,
     184,   184,   115,    87,   150,    50,   166,     5,   165,    81,
      82,    88,   238,   115,   111,   129,   163,   164,   226,    73,
     178,   179,   163,   164,   130,   131,   129,   206,   207,    24,
      25,    44,    59,   227,   163,     8,    18,   218,   131,     8,
       3,   252,   123,   252,   130,   252,     8,   130,   130,   158,
     163,   164,     9,   129,    72,   126,     8,   163,   164,     8,
     163,   164,   163,   164,   163,   164,   163,   164,   163,   164,
      55,   131,   144,    55,   130,   130,   190,   151,   130,     5,
     167,   110,   170,   171,   130,    32,   103,    81,     8,   233,
     130,   165,     3,    80,   180,   181,   162,   129,   208,   209,
     131,     3,    65,   219,   209,   217,   130,   130,   130,   130,
     130,   112,   112,   165,    72,     9,   129,     3,     5,    54,
     145,   146,   130,   252,    47,    74,   148,    26,    50,    53,
     169,   130,   115,   115,   131,   130,   131,   207,   163,   164,
     163,   164,   130,   165,   131,   130,    26,    65,   149,   162,
     168,    74,   160,    74,   172,   173,     3,     5,     8,   129,
       3,     8,   181,   209,   130,   146,   162,   174,   175,     5,
     131,    26,    26,   176,   129,     3,   182,   183,   129,    19,
      37,   131,   131,   162,   174,    78,   174,   178,     8,   115,
     130,   131,     8,   175,     5,   129,   130,   163,   183,   130,
     130
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

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 304:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 305:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 312:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 313:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 314:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 322:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 323:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 324:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 325:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 326:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 327:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[-2] );
			pParser->ToStringUnescape ( tStmt.m_sUdrLib, yyvsp[0] );
		;}
    break;

  case 328:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_RANKER;
			pParser->ToString ( tStmt.m_sUdrName, yyvsp[0] );
		;}
    break;

  case 329:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 330:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 331:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 332:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 333:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 336:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 337:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 338:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 339:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 341:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 342:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 343:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 344:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 345:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 346:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 347:

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

