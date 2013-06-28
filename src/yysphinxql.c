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
     TOK_ADD = 268,
     TOK_AGENT = 269,
     TOK_ALTER = 270,
     TOK_AS = 271,
     TOK_ASC = 272,
     TOK_ATTACH = 273,
     TOK_AVG = 274,
     TOK_BEGIN = 275,
     TOK_BETWEEN = 276,
     TOK_BIGINT = 277,
     TOK_BOOL = 278,
     TOK_BY = 279,
     TOK_CALL = 280,
     TOK_CHARACTER = 281,
     TOK_COLLATION = 282,
     TOK_COLUMN = 283,
     TOK_COMMIT = 284,
     TOK_COMMITTED = 285,
     TOK_COUNT = 286,
     TOK_CREATE = 287,
     TOK_DELETE = 288,
     TOK_DESC = 289,
     TOK_DESCRIBE = 290,
     TOK_DISTINCT = 291,
     TOK_DIV = 292,
     TOK_DROP = 293,
     TOK_FALSE = 294,
     TOK_FLOAT = 295,
     TOK_FLUSH = 296,
     TOK_FROM = 297,
     TOK_FUNCTION = 298,
     TOK_GLOBAL = 299,
     TOK_GROUP = 300,
     TOK_GROUPBY = 301,
     TOK_GROUP_CONCAT = 302,
     TOK_ID = 303,
     TOK_IN = 304,
     TOK_INDEX = 305,
     TOK_INSERT = 306,
     TOK_INT = 307,
     TOK_INTEGER = 308,
     TOK_INTO = 309,
     TOK_ISOLATION = 310,
     TOK_LEVEL = 311,
     TOK_LIKE = 312,
     TOK_LIMIT = 313,
     TOK_MATCH = 314,
     TOK_MAX = 315,
     TOK_META = 316,
     TOK_MIN = 317,
     TOK_MOD = 318,
     TOK_NAMES = 319,
     TOK_NULL = 320,
     TOK_OPTION = 321,
     TOK_ORDER = 322,
     TOK_OPTIMIZE = 323,
     TOK_PLAN = 324,
     TOK_PROFILE = 325,
     TOK_RAND = 326,
     TOK_RAMCHUNK = 327,
     TOK_READ = 328,
     TOK_REPEATABLE = 329,
     TOK_REPLACE = 330,
     TOK_RETURNS = 331,
     TOK_ROLLBACK = 332,
     TOK_RTINDEX = 333,
     TOK_SELECT = 334,
     TOK_SERIALIZABLE = 335,
     TOK_SET = 336,
     TOK_SESSION = 337,
     TOK_SHOW = 338,
     TOK_SONAME = 339,
     TOK_START = 340,
     TOK_STATUS = 341,
     TOK_STRING = 342,
     TOK_SUM = 343,
     TOK_TABLE = 344,
     TOK_TABLES = 345,
     TOK_TO = 346,
     TOK_TRANSACTION = 347,
     TOK_TRUE = 348,
     TOK_TRUNCATE = 349,
     TOK_UNCOMMITTED = 350,
     TOK_UPDATE = 351,
     TOK_VALUES = 352,
     TOK_VARIABLES = 353,
     TOK_WARNINGS = 354,
     TOK_WEIGHT = 355,
     TOK_WHERE = 356,
     TOK_WITHIN = 357,
     TOK_OR = 358,
     TOK_AND = 359,
     TOK_NE = 360,
     TOK_GTE = 361,
     TOK_LTE = 362,
     TOK_NOT = 363,
     TOK_NEG = 364
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
#define TOK_ADD 268
#define TOK_AGENT 269
#define TOK_ALTER 270
#define TOK_AS 271
#define TOK_ASC 272
#define TOK_ATTACH 273
#define TOK_AVG 274
#define TOK_BEGIN 275
#define TOK_BETWEEN 276
#define TOK_BIGINT 277
#define TOK_BOOL 278
#define TOK_BY 279
#define TOK_CALL 280
#define TOK_CHARACTER 281
#define TOK_COLLATION 282
#define TOK_COLUMN 283
#define TOK_COMMIT 284
#define TOK_COMMITTED 285
#define TOK_COUNT 286
#define TOK_CREATE 287
#define TOK_DELETE 288
#define TOK_DESC 289
#define TOK_DESCRIBE 290
#define TOK_DISTINCT 291
#define TOK_DIV 292
#define TOK_DROP 293
#define TOK_FALSE 294
#define TOK_FLOAT 295
#define TOK_FLUSH 296
#define TOK_FROM 297
#define TOK_FUNCTION 298
#define TOK_GLOBAL 299
#define TOK_GROUP 300
#define TOK_GROUPBY 301
#define TOK_GROUP_CONCAT 302
#define TOK_ID 303
#define TOK_IN 304
#define TOK_INDEX 305
#define TOK_INSERT 306
#define TOK_INT 307
#define TOK_INTEGER 308
#define TOK_INTO 309
#define TOK_ISOLATION 310
#define TOK_LEVEL 311
#define TOK_LIKE 312
#define TOK_LIMIT 313
#define TOK_MATCH 314
#define TOK_MAX 315
#define TOK_META 316
#define TOK_MIN 317
#define TOK_MOD 318
#define TOK_NAMES 319
#define TOK_NULL 320
#define TOK_OPTION 321
#define TOK_ORDER 322
#define TOK_OPTIMIZE 323
#define TOK_PLAN 324
#define TOK_PROFILE 325
#define TOK_RAND 326
#define TOK_RAMCHUNK 327
#define TOK_READ 328
#define TOK_REPEATABLE 329
#define TOK_REPLACE 330
#define TOK_RETURNS 331
#define TOK_ROLLBACK 332
#define TOK_RTINDEX 333
#define TOK_SELECT 334
#define TOK_SERIALIZABLE 335
#define TOK_SET 336
#define TOK_SESSION 337
#define TOK_SHOW 338
#define TOK_SONAME 339
#define TOK_START 340
#define TOK_STATUS 341
#define TOK_STRING 342
#define TOK_SUM 343
#define TOK_TABLE 344
#define TOK_TABLES 345
#define TOK_TO 346
#define TOK_TRANSACTION 347
#define TOK_TRUE 348
#define TOK_TRUNCATE 349
#define TOK_UNCOMMITTED 350
#define TOK_UPDATE 351
#define TOK_VALUES 352
#define TOK_VARIABLES 353
#define TOK_WARNINGS 354
#define TOK_WEIGHT 355
#define TOK_WHERE 356
#define TOK_WITHIN 357
#define TOK_OR 358
#define TOK_AND 359
#define TOK_NE 360
#define TOK_GTE 361
#define TOK_LTE 362
#define TOK_NOT 363
#define TOK_NEG 364




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
#define YYFINAL  117
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1210

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  127
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  107
/* YYNRULES -- Number of rules. */
#define YYNRULES  302
/* YYNRULES -- Number of states. */
#define YYNSTATES  579

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   364

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   117,   106,     2,
     121,   122,   115,   113,   123,   114,   126,   116,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   120,
     109,   107,   110,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   124,   105,   125,     2,     2,     2,     2,
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
     108,   111,   112,   118,   119
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    56,    60,
      62,    64,    66,    75,    77,    87,    88,    91,    93,    97,
      99,   101,   103,   104,   108,   109,   112,   117,   128,   130,
     134,   136,   139,   140,   142,   145,   147,   152,   157,   162,
     167,   172,   177,   181,   187,   189,   193,   194,   196,   199,
     201,   205,   209,   214,   218,   222,   228,   235,   239,   244,
     250,   254,   258,   262,   266,   270,   272,   278,   284,   290,
     294,   298,   302,   306,   310,   314,   318,   320,   322,   327,
     331,   335,   337,   339,   342,   344,   347,   349,   353,   354,
     359,   360,   362,   364,   368,   369,   371,   377,   378,   380,
     384,   390,   392,   396,   398,   401,   404,   405,   407,   410,
     415,   416,   418,   421,   423,   427,   431,   435,   441,   448,
     452,   454,   458,   462,   464,   466,   468,   470,   472,   474,
     477,   480,   484,   488,   492,   496,   500,   504,   508,   512,
     516,   520,   524,   528,   532,   536,   540,   544,   548,   552,
     556,   558,   563,   568,   573,   578,   583,   587,   594,   601,
     605,   607,   611,   613,   615,   619,   625,   628,   629,   632,
     634,   637,   640,   644,   646,   648,   653,   658,   662,   664,
     666,   668,   670,   672,   674,   678,   683,   688,   693,   697,
     702,   710,   716,   718,   720,   722,   724,   726,   728,   730,
     732,   734,   737,   744,   746,   748,   749,   753,   755,   759,
     761,   765,   769,   771,   775,   777,   779,   781,   785,   788,
     796,   806,   813,   815,   819,   821,   825,   827,   831,   832,
     835,   837,   841,   845,   846,   848,   850,   852,   856,   858,
     860,   864,   871,   873,   877,   881,   885,   891,   896,   898,
     900,   902,   904,   912,   917,   918,   920,   923,   925,   929,
     933,   936,   940,   947,   948,   950,   952,   955,   958,   961,
     963,   971,   973,   975,   977,   981,   988,   992,   996,  1000,
    1002,  1006,  1010
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     128,     0,    -1,   129,    -1,   130,    -1,   130,   120,    -1,
     188,    -1,   196,    -1,   182,    -1,   183,    -1,   186,    -1,
     197,    -1,   206,    -1,   208,    -1,   209,    -1,   214,    -1,
     219,    -1,   220,    -1,   224,    -1,   226,    -1,   227,    -1,
     228,    -1,   229,    -1,   221,    -1,   230,    -1,   232,    -1,
     233,    -1,   213,    -1,   131,    -1,   130,   120,   131,    -1,
     132,    -1,   177,    -1,   133,    -1,    79,     3,   121,   121,
     133,   122,   134,   122,    -1,   140,    -1,    79,   141,    42,
     121,   137,   140,   122,   138,   139,    -1,    -1,   123,   135,
      -1,   136,    -1,   135,   123,   136,    -1,     3,    -1,     5,
      -1,    48,    -1,    -1,    67,    24,   162,    -1,    -1,    58,
       5,    -1,    58,     5,   123,     5,    -1,    79,   141,    42,
     145,   146,   155,   158,   160,   164,   166,    -1,   142,    -1,
     141,   123,   142,    -1,   115,    -1,   144,   143,    -1,    -1,
       3,    -1,    16,     3,    -1,   172,    -1,    19,   121,   172,
     122,    -1,    60,   121,   172,   122,    -1,    62,   121,   172,
     122,    -1,    88,   121,   172,   122,    -1,    47,   121,   172,
     122,    -1,    31,   121,   115,   122,    -1,    46,   121,   122,
      -1,    31,   121,    36,     3,   122,    -1,     3,    -1,   145,
     123,     3,    -1,    -1,   147,    -1,   101,   148,    -1,   149,
      -1,   148,   104,   148,    -1,   121,   148,   122,    -1,    59,
     121,     8,   122,    -1,   151,   107,   152,    -1,   151,   108,
     152,    -1,   151,    49,   121,   154,   122,    -1,   151,   118,
      49,   121,   154,   122,    -1,   151,    49,     9,    -1,   151,
     118,    49,     9,    -1,   151,    21,   152,   104,   152,    -1,
     151,   110,   152,    -1,   151,   109,   152,    -1,   151,   111,
     152,    -1,   151,   112,   152,    -1,   151,   107,   153,    -1,
     150,    -1,   151,    21,   153,   104,   153,    -1,   151,    21,
     152,   104,   153,    -1,   151,    21,   153,   104,   152,    -1,
     151,   110,   153,    -1,   151,   109,   153,    -1,   151,   111,
     153,    -1,   151,   112,   153,    -1,   151,   107,     8,    -1,
     151,   108,     8,    -1,   151,   108,   153,    -1,     3,    -1,
       4,    -1,    31,   121,   115,   122,    -1,    46,   121,   122,
      -1,   100,   121,   122,    -1,    48,    -1,     5,    -1,   114,
       5,    -1,     6,    -1,   114,     6,    -1,   152,    -1,   154,
     123,   152,    -1,    -1,    45,   156,    24,   157,    -1,    -1,
       5,    -1,   151,    -1,   157,   123,   151,    -1,    -1,   159,
      -1,   102,    45,    67,    24,   162,    -1,    -1,   161,    -1,
      67,    24,   162,    -1,    67,    24,    71,   121,   122,    -1,
     163,    -1,   162,   123,   163,    -1,   151,    -1,   151,    17,
      -1,   151,    34,    -1,    -1,   165,    -1,    58,     5,    -1,
      58,     5,   123,     5,    -1,    -1,   167,    -1,    66,   168,
      -1,   169,    -1,   168,   123,   169,    -1,     3,   107,     3,
      -1,     3,   107,     5,    -1,     3,   107,   121,   170,   122,
      -1,     3,   107,     3,   121,     8,   122,    -1,     3,   107,
       8,    -1,   171,    -1,   170,   123,   171,    -1,     3,   107,
     152,    -1,     3,    -1,     4,    -1,    48,    -1,     5,    -1,
       6,    -1,     9,    -1,   114,   172,    -1,   118,   172,    -1,
     172,   113,   172,    -1,   172,   114,   172,    -1,   172,   115,
     172,    -1,   172,   116,   172,    -1,   172,   109,   172,    -1,
     172,   110,   172,    -1,   172,   106,   172,    -1,   172,   105,
     172,    -1,   172,   117,   172,    -1,   172,    37,   172,    -1,
     172,    63,   172,    -1,   172,   112,   172,    -1,   172,   111,
     172,    -1,   172,   107,   172,    -1,   172,   108,   172,    -1,
     172,   104,   172,    -1,   172,   103,   172,    -1,   121,   172,
     122,    -1,   124,   176,   125,    -1,   173,    -1,     3,   121,
     174,   122,    -1,    49,   121,   174,   122,    -1,    53,   121,
     174,   122,    -1,    22,   121,   174,   122,    -1,    40,   121,
     174,   122,    -1,     3,   121,   122,    -1,    62,   121,   172,
     123,   172,   122,    -1,    60,   121,   172,   123,   172,   122,
      -1,   100,   121,   122,    -1,   175,    -1,   174,   123,   175,
      -1,   172,    -1,     8,    -1,     3,   107,   152,    -1,   176,
     123,     3,   107,   152,    -1,    83,   179,    -1,    -1,    57,
       8,    -1,    99,    -1,    86,   178,    -1,    61,   178,    -1,
      14,    86,   178,    -1,    70,    -1,    69,    -1,    14,     8,
      86,   178,    -1,    14,     3,    86,   178,    -1,    50,     3,
      86,    -1,     3,    -1,    65,    -1,     8,    -1,     5,    -1,
       6,    -1,   180,    -1,   181,   114,   180,    -1,    81,     3,
     107,   185,    -1,    81,     3,   107,   184,    -1,    81,     3,
     107,    65,    -1,    81,    64,   181,    -1,    81,    10,   107,
     181,    -1,    81,    44,     9,   107,   121,   154,   122,    -1,
      81,    44,     3,   107,   184,    -1,     3,    -1,     8,    -1,
      93,    -1,    39,    -1,   152,    -1,    29,    -1,    77,    -1,
     187,    -1,    20,    -1,    85,    92,    -1,   189,    54,     3,
     190,    97,   192,    -1,    51,    -1,    75,    -1,    -1,   121,
     191,   122,    -1,   151,    -1,   191,   123,   151,    -1,   193,
      -1,   192,   123,   193,    -1,   121,   194,   122,    -1,   195,
      -1,   194,   123,   195,    -1,   152,    -1,   153,    -1,     8,
      -1,   121,   154,   122,    -1,   121,   122,    -1,    33,    42,
     145,   101,    48,   107,   152,    -1,    33,    42,   145,   101,
      48,    49,   121,   154,   122,    -1,    25,     3,   121,   198,
     201,   122,    -1,   199,    -1,   198,   123,   199,    -1,   195,
      -1,   121,   200,   122,    -1,     8,    -1,   200,   123,     8,
      -1,    -1,   123,   202,    -1,   203,    -1,   202,   123,   203,
      -1,   195,   204,   205,    -1,    -1,    16,    -1,     3,    -1,
      58,    -1,   207,     3,   178,    -1,    35,    -1,    34,    -1,
      83,    90,   178,    -1,    96,   145,    81,   210,   147,   166,
      -1,   211,    -1,   210,   123,   211,    -1,     3,   107,   152,
      -1,     3,   107,   153,    -1,     3,   107,   121,   154,   122,
      -1,     3,   107,   121,   122,    -1,    53,    -1,    22,    -1,
      40,    -1,    23,    -1,    15,    89,     3,    13,    28,     3,
     212,    -1,    83,   222,    98,   215,    -1,    -1,   216,    -1,
     101,   217,    -1,   218,    -1,   217,   103,   218,    -1,     3,
     107,     8,    -1,    83,    27,    -1,    83,    26,    81,    -1,
      81,   222,    92,    55,    56,   223,    -1,    -1,    44,    -1,
      82,    -1,    73,    95,    -1,    73,    30,    -1,    74,    73,
      -1,    80,    -1,    32,    43,     3,    76,   225,    84,     8,
      -1,    52,    -1,    40,    -1,    87,    -1,    38,    43,     3,
      -1,    18,    50,     3,    91,    78,     3,    -1,    41,    78,
       3,    -1,    41,    72,     3,    -1,    79,   231,   164,    -1,
      10,    -1,    10,   126,     3,    -1,    94,    78,     3,    -1,
      68,    50,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   155,   155,   156,   157,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   188,   189,   193,
     194,   198,   199,   207,   208,   215,   217,   221,   225,   232,
     233,   234,   238,   251,   258,   260,   265,   274,   289,   290,
     294,   295,   298,   300,   301,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   317,   318,   321,   323,   327,   331,
     332,   333,   337,   342,   349,   357,   365,   374,   379,   384,
     389,   394,   399,   404,   409,   414,   419,   424,   429,   434,
     439,   444,   449,   454,   459,   467,   471,   472,   477,   483,
     489,   495,   504,   505,   516,   517,   521,   527,   533,   535,
     538,   540,   547,   551,   557,   559,   563,   574,   576,   580,
     584,   591,   592,   596,   597,   598,   601,   603,   607,   612,
     619,   621,   625,   629,   630,   634,   639,   644,   650,   655,
     663,   668,   675,   685,   686,   687,   688,   689,   690,   691,
     692,   694,   695,   696,   697,   698,   699,   700,   701,   702,
     703,   704,   705,   706,   707,   708,   709,   710,   711,   712,
     713,   717,   718,   719,   720,   721,   722,   723,   724,   725,
     729,   730,   734,   735,   739,   740,   746,   749,   751,   755,
     756,   757,   758,   759,   760,   761,   766,   771,   781,   782,
     783,   784,   785,   789,   790,   794,   799,   804,   809,   810,
     814,   819,   827,   828,   832,   833,   834,   848,   849,   850,
     854,   855,   861,   869,   870,   873,   875,   879,   880,   884,
     885,   889,   893,   894,   898,   899,   900,   901,   902,   908,
     916,   930,   938,   942,   949,   950,   957,   967,   973,   975,
     979,   984,   988,   995,   997,  1001,  1002,  1008,  1016,  1017,
    1023,  1029,  1037,  1038,  1042,  1056,  1062,  1066,  1076,  1077,
    1078,  1079,  1083,  1096,  1102,  1103,  1107,  1111,  1112,  1116,
    1120,  1127,  1134,  1140,  1141,  1142,  1146,  1147,  1148,  1149,
    1155,  1166,  1167,  1168,  1172,  1183,  1195,  1204,  1215,  1223,
    1224,  1230,  1241
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
  "TOK_BAD_NUMERIC", "TOK_ADD", "TOK_AGENT", "TOK_ALTER", "TOK_AS", 
  "TOK_ASC", "TOK_ATTACH", "TOK_AVG", "TOK_BEGIN", "TOK_BETWEEN", 
  "TOK_BIGINT", "TOK_BOOL", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", 
  "TOK_COLLATION", "TOK_COLUMN", "TOK_COMMIT", "TOK_COMMITTED", 
  "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", 
  "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", 
  "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTEGER", "TOK_INTO", "TOK_ISOLATION", 
  "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", 
  "TOK_ORDER", "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", 
  "TOK_RAMCHUNK", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", 
  "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", 
  "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
  "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", 
  "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", 
  "';'", "'('", "')'", "','", "'{'", "'}'", "'.'", "$accept", "request", 
  "statement", "multi_stmt_list", "multi_stmt", "select", "select1", 
  "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg", 
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "expr_float_unhandled", "expr_ident", "const_int", 
  "const_float", "const_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_group_order_clause", "group_order_clause", 
  "opt_order_clause", "order_clause", "order_items_list", "order_item", 
  "opt_limit_clause", "limit_clause", "opt_option_clause", 
  "option_clause", "option_list", "option_item", "named_const_list", 
  "named_const", "expr", "function", "arglist", "arg", "consthash", 
  "show_stmt", "like_filter", "show_what", "simple_set_value", 
  "set_value", "set_stmt", "set_global_stmt", "set_string_value", 
  "boolean_value", "transact_op", "start_transaction", "insert_into", 
  "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", "alter_col_type", "alter", 
  "show_variables", "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "flush_ramchunk", "select_sysvar", 
  "sysvar_name", "truncate", "optimize_index", 0
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
     355,   356,   357,   358,   359,   124,    38,    61,   360,    60,
      62,   361,   362,    43,    45,    42,    47,    37,   363,   364,
      59,    40,    41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   127,   128,   128,   128,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   130,   130,   131,
     131,   132,   132,   133,   133,   134,   134,   135,   135,   136,
     136,   136,   137,   138,   139,   139,   139,   140,   141,   141,
     142,   142,   143,   143,   143,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   145,   145,   146,   146,   147,   148,
     148,   148,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   150,   151,   151,   151,   151,
     151,   151,   152,   152,   153,   153,   154,   154,   155,   155,
     156,   156,   157,   157,   158,   158,   159,   160,   160,   161,
     161,   162,   162,   163,   163,   163,   164,   164,   165,   165,
     166,   166,   167,   168,   168,   169,   169,   169,   169,   169,
     170,   170,   171,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   173,   173,   173,   173,   173,   173,   173,   173,   173,
     174,   174,   175,   175,   176,   176,   177,   178,   178,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   180,   180,
     180,   180,   180,   181,   181,   182,   182,   182,   182,   182,
     183,   183,   184,   184,   185,   185,   185,   186,   186,   186,
     187,   187,   188,   189,   189,   190,   190,   191,   191,   192,
     192,   193,   194,   194,   195,   195,   195,   195,   195,   196,
     196,   197,   198,   198,   199,   199,   200,   200,   201,   201,
     202,   202,   203,   204,   204,   205,   205,   206,   207,   207,
     208,   209,   210,   210,   211,   211,   211,   211,   212,   212,
     212,   212,   213,   214,   215,   215,   216,   217,   217,   218,
     219,   220,   221,   222,   222,   222,   223,   223,   223,   223,
     224,   225,   225,   225,   226,   227,   228,   229,   230,   231,
     231,   232,   233
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     8,     1,     9,     0,     2,     1,     3,     1,
       1,     1,     0,     3,     0,     2,     4,    10,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     4,     3,     5,     1,     3,     0,     1,     2,     1,
       3,     3,     4,     3,     3,     5,     6,     3,     4,     5,
       3,     3,     3,     3,     3,     1,     5,     5,     5,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     4,     3,
       3,     1,     1,     2,     1,     2,     1,     3,     0,     4,
       0,     1,     1,     3,     0,     1,     5,     0,     1,     3,
       5,     1,     3,     1,     2,     2,     0,     1,     2,     4,
       0,     1,     2,     1,     3,     3,     3,     5,     6,     3,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     4,     4,     4,     4,     4,     3,     6,     6,     3,
       1,     3,     1,     1,     3,     5,     2,     0,     2,     1,
       2,     2,     3,     1,     1,     4,     4,     3,     1,     1,
       1,     1,     1,     1,     3,     4,     4,     4,     3,     4,
       7,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     6,     1,     1,     0,     3,     1,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     3,     2,     7,
       9,     6,     1,     3,     1,     3,     1,     3,     0,     2,
       1,     3,     3,     0,     1,     1,     1,     3,     1,     1,
       3,     6,     1,     3,     3,     3,     5,     4,     1,     1,
       1,     1,     7,     4,     0,     1,     2,     1,     3,     3,
       2,     3,     6,     0,     1,     1,     2,     2,     2,     1,
       7,     1,     1,     1,     3,     6,     3,     3,     3,     1,
       3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   220,     0,   217,     0,     0,   259,   258,
       0,     0,   223,     0,   224,   218,     0,   283,   283,     0,
       0,     0,     0,     2,     3,    27,    29,    31,    33,    30,
       7,     8,     9,   219,     5,     0,     6,    10,    11,     0,
      12,    13,    26,    14,    15,    16,    22,    17,    18,    19,
      20,    21,    23,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   146,   147,   148,   299,
       0,     0,     0,     0,     0,     0,   145,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,    48,
      52,    55,   170,   126,     0,     0,   284,     0,   285,     0,
       0,     0,   280,   284,     0,   187,   194,   193,   187,   187,
     189,   186,     0,   221,     0,    64,     0,     1,     4,     0,
     187,     0,     0,     0,     0,     0,   294,   297,   296,   302,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,     0,     0,   149,   150,     0,
       0,     0,     0,     0,    53,     0,    51,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   298,   127,     0,     0,     0,
       0,   198,   201,   202,   200,   199,   203,   208,     0,     0,
       0,   187,   281,     0,     0,   191,   190,   260,   274,   301,
       0,     0,     0,     0,    28,   225,   257,     0,     0,   102,
     104,   236,     0,     0,   234,   235,   244,   248,   242,     0,
       0,   183,     0,   176,   182,     0,   180,   300,     0,     0,
       0,     0,     0,    62,     0,     0,     0,     0,     0,     0,
     179,     0,     0,     0,   168,     0,     0,   169,    42,    66,
      49,    54,   160,   161,   167,   166,   158,   157,   164,   165,
     155,   156,   163,   162,   151,   152,   153,   154,   159,   128,
     212,   213,   215,   207,   214,     0,   216,   206,   205,   209,
       0,     0,     0,     0,   187,   187,   192,   197,   188,     0,
     273,   275,     0,     0,   262,    65,     0,     0,     0,     0,
     103,   105,   246,   238,   106,     0,     0,     0,     0,   292,
     291,   293,     0,     0,     0,     0,   171,     0,    56,   174,
       0,    61,   175,    60,   172,   173,    57,     0,    58,     0,
      59,     0,     0,   184,     0,     0,     0,   108,    67,     0,
     211,     0,   204,     0,   196,   195,     0,   276,   277,     0,
       0,   130,    96,    97,     0,     0,   101,     0,   227,     0,
       0,     0,   295,   237,     0,   245,     0,   244,   243,   249,
     250,   241,     0,     0,     0,    35,   181,    63,     0,     0,
       0,     0,     0,     0,     0,    68,    69,    85,     0,   110,
     114,   129,     0,     0,     0,   289,   282,     0,     0,     0,
     264,   265,   263,     0,   261,   131,     0,     0,     0,   226,
       0,     0,   222,   229,   269,   271,   270,   268,   272,   107,
     247,   254,     0,     0,   290,     0,   239,     0,     0,   178,
     177,   185,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   111,     0,     0,   117,
     115,   210,   287,   286,   288,   279,   278,   267,     0,     0,
     132,   133,     0,    99,   100,   228,     0,     0,   232,     0,
     255,   256,   252,   253,   251,     0,    39,    40,    41,    36,
      37,    32,     0,     0,    44,     0,    71,    70,     0,     0,
      77,     0,    93,    73,    84,    94,    74,    95,    81,    90,
      80,    89,    82,    91,    83,    92,     0,     0,     0,     0,
     126,   118,   266,     0,     0,    98,   231,     0,   230,   240,
       0,     0,     0,    34,    72,     0,     0,     0,    78,     0,
     112,   109,     0,     0,   130,   135,   136,   139,     0,   134,
     233,    38,   123,    43,   121,    45,    79,    87,    88,    86,
      75,     0,     0,     0,     0,   119,    47,     0,     0,     0,
     140,   124,   125,     0,     0,    76,   113,   116,     0,     0,
       0,   137,     0,   122,    46,   120,   138,   142,   141
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   428,   479,   480,
     335,   484,   523,    28,    88,    89,   156,    90,   249,   337,
     338,   385,   386,   387,   542,   304,   215,   305,   390,   447,
     531,   449,   450,   510,   511,   543,   544,   175,   176,   404,
     405,   460,   461,   559,   560,   224,    92,   225,   226,   151,
      29,   195,   111,   186,   187,    30,    31,   277,   278,    32,
      33,    34,    35,   297,   359,   412,   413,   467,   216,    36,
      37,   217,   218,   306,   308,   369,   370,   422,   472,    38,
      39,    40,    41,   293,   294,   418,    42,    43,   290,   291,
     347,   348,    44,    45,    46,    99,   396,    47,   312,    48,
      49,    50,    51,    52,    93,    53,    54
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -478
static const short yypact[] =
{
     980,   -69,    -4,  -478,    80,  -478,    43,    70,  -478,  -478,
      72,   -29,  -478,   125,  -478,  -478,   183,   292,   635,    93,
     142,   231,   239,  -478,   108,  -478,  -478,  -478,  -478,  -478,
    -478,  -478,  -478,  -478,  -478,   186,  -478,  -478,  -478,   243,
    -478,  -478,  -478,  -478,  -478,  -478,  -478,  -478,  -478,  -478,
    -478,  -478,  -478,  -478,  -478,   246,   253,   145,   265,   231,
     266,   274,   278,   283,   161,  -478,  -478,  -478,  -478,   168,
     182,   184,   185,   191,   194,   202,  -478,   209,   226,   238,
     240,   242,   245,   633,  -478,   633,   633,   296,   -24,  -478,
     117,   974,  -478,   250,   257,   260,   195,    79,  -478,   268,
      24,   271,  -478,  -478,   369,   316,  -478,  -478,   316,   316,
    -478,  -478,   281,  -478,   374,  -478,   -34,  -478,    40,   377,
     316,   368,   304,    20,   307,   -50,  -478,  -478,  -478,  -478,
     465,   393,   633,   551,   -13,   551,   275,   633,   551,   551,
     633,   633,   633,   277,   280,   288,   291,  -478,  -478,   739,
     293,    83,    10,   322,  -478,   395,  -478,   633,   633,   633,
     633,   633,   633,   633,   633,   633,   633,   633,   633,   633,
     633,   633,   633,   633,   399,  -478,  -478,    36,    79,   306,
     308,  -478,  -478,  -478,  -478,  -478,  -478,   300,   361,   331,
     332,   316,  -478,   333,   412,  -478,  -478,  -478,   324,  -478,
     420,   423,   402,   279,  -478,   309,  -478,   400,   349,  -478,
    -478,  -478,   166,    16,  -478,  -478,  -478,   311,  -478,   163,
     381,  -478,   592,  -478,   974,   130,  -478,  -478,   767,   132,
     428,   310,   157,  -478,   794,   162,   167,   512,   657,   822,
    -478,   488,   633,   633,  -478,    31,   432,  -478,  -478,   -44,
    -478,  -478,  -478,  -478,   988,  1001,  1082,  1093,   276,   276,
     159,   159,   159,   159,    -8,    -8,  -478,  -478,  -478,   315,
    -478,  -478,  -478,  -478,  -478,   434,  -478,  -478,  -478,   300,
      45,   320,    79,   388,   316,   316,  -478,  -478,  -478,   442,
    -478,  -478,   345,    17,  -478,  -478,   187,   356,   451,   453,
    -478,  -478,  -478,  -478,  -478,   169,   188,    20,   335,  -478,
    -478,  -478,   375,   -15,   322,   336,  -478,   551,  -478,  -478,
     338,  -478,  -478,  -478,  -478,  -478,  -478,   633,  -478,   633,
    -478,   678,   712,  -478,   354,   384,     4,   421,  -478,   460,
    -478,    31,  -478,   136,  -478,  -478,   360,   372,  -478,    62,
     420,   406,  -478,  -478,   355,   357,  -478,   358,  -478,   210,
     359,   225,  -478,  -478,    31,  -478,   469,    42,  -478,   362,
    -478,  -478,   473,   363,    31,   365,  -478,  -478,   849,   877,
      31,   322,   364,   378,     4,   379,  -478,  -478,   152,   477,
     387,  -478,   212,   -21,   422,  -478,  -478,   490,   442,    -1,
    -478,  -478,  -478,   497,  -478,  -478,   386,   382,   385,  -478,
     187,    56,   380,  -478,  -478,  -478,  -478,  -478,  -478,  -478,
    -478,  -478,    13,    56,  -478,    31,  -478,   146,   389,  -478,
    -478,  -478,    -9,   439,   500,     2,     4,    60,     5,    32,
      64,    60,    60,    60,    60,   463,  -478,   485,   470,   452,
    -478,  -478,  -478,  -478,  -478,  -478,  -478,  -478,   215,   414,
     401,  -478,   407,  -478,  -478,  -478,    25,   220,  -478,   359,
    -478,  -478,  -478,   506,  -478,   223,  -478,  -478,  -478,   408,
    -478,  -478,   231,   508,   472,   411,  -478,  -478,   430,   431,
    -478,    31,  -478,  -478,  -478,  -478,  -478,  -478,  -478,  -478,
    -478,  -478,  -478,  -478,  -478,  -478,    22,   187,   471,   515,
     250,  -478,  -478,     7,   497,  -478,  -478,    56,  -478,  -478,
     146,   187,   535,  -478,  -478,    60,    60,   228,  -478,    31,
    -478,   419,   519,   196,   406,   424,  -478,  -478,   541,  -478,
    -478,  -478,   178,   429,  -478,   435,  -478,  -478,  -478,  -478,
    -478,   232,   187,   187,   425,   429,  -478,   539,   444,   235,
    -478,  -478,  -478,   187,   548,  -478,  -478,   429,   440,   441,
      31,  -478,   541,  -478,  -478,  -478,  -478,  -478,  -478
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -478,  -478,  -478,  -478,   443,  -478,   342,  -478,  -478,    46,
    -478,  -478,  -478,   233,   189,   416,  -478,  -478,     1,  -478,
     284,  -325,  -478,  -478,  -294,  -123,  -346,  -322,  -478,  -478,
    -478,  -478,  -478,  -478,  -478,  -477,     8,    57,  -478,    38,
    -478,  -478,    66,  -478,     6,    -5,  -478,    86,   259,  -478,
    -478,  -103,  -478,   299,   396,  -478,  -478,   302,  -478,  -478,
    -478,  -478,  -478,  -478,  -478,  -478,   115,  -478,  -306,  -478,
    -478,  -478,   285,  -478,  -478,  -478,   170,  -478,  -478,  -478,
    -478,  -478,  -478,  -478,   244,  -478,  -478,  -478,  -478,  -478,
    -478,   192,  -478,  -478,  -478,   567,  -478,  -478,  -478,  -478,
    -478,  -478,  -478,  -478,  -478,  -478,  -478
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -254
static const short yytable[] =
{
     214,   367,   358,   401,   209,   196,   197,   352,   353,   452,
     535,    91,   536,   115,   490,   537,   470,   206,   152,   392,
      55,   209,   116,   230,   302,   209,   210,   189,   211,   157,
     209,   528,   190,   482,   373,   354,   209,   209,   210,   270,
     492,   209,   388,    61,   271,  -253,    56,   200,   270,    62,
     355,   220,   356,   271,   276,   158,   555,   336,   421,   435,
     125,   209,   210,   383,   211,   209,   210,   209,   210,   209,
     210,   471,   495,   201,   453,   272,   567,   458,   147,   201,
     148,   149,   181,    57,   182,   183,    58,   184,   286,   201,
     388,   489,   374,   494,   497,   499,   501,   503,   505,   153,
    -253,   273,   231,   475,   357,   468,   436,   171,   172,   173,
     191,   487,    59,   275,   153,    60,   465,   473,   336,   202,
     154,   457,   333,   203,   486,   384,   491,   228,   538,   274,
     275,   248,   234,   155,   212,   237,   238,   239,   303,   275,
     350,   213,   388,   529,   185,   275,   212,   303,    91,   476,
     275,   477,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   527,
     212,   300,   301,   437,   212,    63,   212,   466,   212,   547,
     549,   344,   345,   399,   214,   113,    64,    65,    66,    67,
     352,   353,    68,    69,   478,   561,   157,    91,   179,   352,
     353,   438,    70,   309,   180,    71,   246,   551,   247,   393,
     394,   540,   562,   530,    72,   310,   395,   149,   354,   229,
     114,   232,   158,    73,   235,   236,   400,   354,   118,    74,
      75,    76,    77,   355,   115,   356,    78,   331,   332,   117,
     119,   419,   355,    79,   356,    80,   120,   414,   415,   121,
     311,   426,   316,   317,   319,   317,   122,   431,   566,   439,
     440,   441,   442,   443,   444,   416,   123,   554,   124,   126,
     445,    81,   169,   170,   171,   172,   173,   127,   417,   322,
     317,   128,   130,    82,   324,   317,   129,   357,   214,   325,
     317,   363,   364,   100,   131,    94,   357,    83,    84,   150,
     214,    85,    95,   132,    86,   133,   134,    87,   174,    91,
     365,   366,   135,   157,   488,   136,   493,   496,   498,   500,
     502,   504,   378,   137,   379,   144,    65,    66,    67,   104,
     138,    68,   409,   410,   451,   364,    96,   512,   364,   158,
     105,    70,   516,   517,    71,   519,   364,   139,   106,   107,
     550,   364,   192,    72,   565,   364,    97,   571,   572,   140,
     188,   141,    73,   142,   177,   108,   143,   178,    74,    75,
      76,    77,   193,   194,    98,    78,    91,   199,   110,   198,
     205,   207,    79,   219,    80,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   214,   208,   227,   233,   251,   240,
     245,   241,   546,   548,   269,    64,    65,    66,    67,   242,
      81,    68,   243,   280,   282,   281,   283,   284,   285,   287,
     288,    70,    82,   292,    71,   289,   295,   299,   298,   313,
     296,   320,   321,    72,   307,   334,    83,    84,   339,   300,
      85,   341,    73,    86,   343,   346,    87,   577,    74,    75,
      76,    77,   349,   360,   361,    78,   362,   371,   375,   372,
     377,   380,    79,   381,    80,   391,   389,   397,   144,    65,
      66,    67,   403,   221,    68,   398,   406,   420,   407,   408,
     411,   424,   446,   436,   425,   423,   433,    71,   427,   448,
      81,   144,    65,    66,    67,   454,   221,    68,   455,   434,
     459,   462,    82,   469,   463,    73,   483,   464,   485,   507,
      71,   481,   506,    76,    77,   508,    83,    84,    78,   509,
      85,   513,   421,    86,   514,   145,    87,   146,    73,   515,
     522,   520,   521,   524,   525,   526,    76,    77,   532,   533,
     545,    78,   552,   553,   558,   557,   568,   569,   145,   157,
     146,   570,   563,   574,   144,    65,    66,    67,   564,   221,
      68,   204,   575,   576,   315,    82,   541,   534,   382,   250,
     432,   573,   556,    71,   279,   158,   376,   351,   578,    83,
     539,   342,   340,    85,   518,   112,   222,   223,    82,    87,
     456,    73,   368,   474,   402,   144,    65,    66,    67,    76,
      77,    68,    83,     0,    78,     0,    85,     0,     0,    86,
     223,   145,    87,   146,    71,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
       0,     0,    73,     0,   326,   327,   144,    65,    66,    67,
      76,    77,    68,     0,     0,    78,     0,     0,     0,   100,
       0,    82,   145,     0,   146,    71,     0,     0,     0,     0,
       0,   101,   102,     0,     0,    83,     0,     0,     0,    85,
       0,   314,    86,    73,     0,    87,     0,     0,     0,   103,
       0,    76,    77,     0,     0,   104,    78,     0,     0,     0,
       0,     0,    82,   145,   157,   146,   105,     0,     0,     0,
       0,     0,     0,     0,   106,   107,    83,     0,     0,     0,
      85,     0,     0,    86,     0,   157,    87,    98,     0,     0,
     158,   108,     0,     0,     0,   109,     0,     0,     0,     0,
       0,     0,     0,    82,   110,     0,     0,     0,     0,     0,
       0,   158,     0,     0,     0,     0,     0,    83,     0,   157,
       0,    85,     0,     0,    86,     0,     0,    87,     0,     0,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   158,   157,     0,     0,   328,
     329,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,     0,     0,     0,     0,
       0,   327,   158,     0,   157,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     158,   157,     0,     0,     0,   329,     0,     0,     0,     0,
       0,     0,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   158,     0,   157,
       0,   244,     0,     0,     0,     0,     0,     0,     0,     0,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   158,   157,     0,     0,   318,
       0,     0,     0,     0,     0,     0,     0,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   158,     0,   157,     0,   323,     0,     0,     0,
       0,     0,     0,     0,     0,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     158,     0,     0,     0,   330,     0,     0,     0,     0,     0,
       0,     0,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,     0,     0,     0,
       0,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,     1,     0,     0,     2,   430,
       3,     0,     0,     0,     0,     4,     0,     0,     0,     5,
       0,   157,     6,     7,     8,     9,     0,     0,    10,     0,
       0,    11,     0,     0,     0,   157,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,   158,   157,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,     0,
       0,   158,     0,     0,     0,    14,     0,    15,     0,    16,
       0,    17,     0,    18,   158,    19,     0,     0,     0,     0,
       0,     0,     0,     0,    20,     0,    21,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   157,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     157,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   158,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   158,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173
};

static const short yycheck[] =
{
     123,   307,   296,   349,     5,   108,   109,     3,     4,    30,
       3,    16,     5,     3,     9,     8,     3,   120,    42,   341,
      89,     5,    21,    36,     8,     5,     6,     3,     8,    37,
       5,     9,     8,    42,    49,    31,     5,     5,     6,     3,
       8,     5,   336,    72,     8,     3,    50,    81,     3,    78,
      46,   101,    48,     8,   177,    63,   533,   101,    16,   384,
      59,     5,     6,    59,     8,     5,     6,     5,     6,     5,
       6,    58,     8,   123,    95,    39,   553,   399,    83,   123,
      85,    86,     3,     3,     5,     6,    43,     8,   191,   123,
     384,   437,   107,   439,   440,   441,   442,   443,   444,   123,
      58,    65,   115,   425,   100,   411,   104,   115,   116,   117,
      86,   436,    42,   114,   123,    43,   410,   423,   101,    79,
       3,   122,   245,    83,   122,   121,   121,   132,   121,    93,
     114,   121,   137,    16,   114,   140,   141,   142,   122,   114,
     123,   121,   436,   121,    65,   114,   114,   122,   153,     3,
     114,     5,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   491,
     114,     5,     6,    21,   114,    50,   114,   121,   114,   525,
     526,   284,   285,   121,   307,    92,     3,     4,     5,     6,
       3,     4,     9,    10,    48,    17,    37,   202,     3,     3,
       4,    49,    19,    40,     9,    22,   123,   529,   125,    73,
      74,   517,    34,   507,    31,    52,    80,   222,    31,   133,
      78,   135,    63,    40,   138,   139,   349,    31,   120,    46,
      47,    48,    49,    46,     3,    48,    53,   242,   243,     0,
      54,   364,    46,    60,    48,    62,     3,    22,    23,     3,
      87,   374,   122,   123,   122,   123,     3,   380,   552,   107,
     108,   109,   110,   111,   112,    40,   121,    71,     3,     3,
     118,    88,   113,   114,   115,   116,   117,     3,    53,   122,
     123,     3,   121,   100,   122,   123,     3,   100,   411,   122,
     123,   122,   123,    14,   126,     3,   100,   114,   115,     3,
     423,   118,    10,   121,   121,   121,   121,   124,    58,   314,
     122,   123,   121,    37,   437,   121,   439,   440,   441,   442,
     443,   444,   327,   121,   329,     3,     4,     5,     6,    50,
     121,     9,   122,   123,   122,   123,    44,   122,   123,    63,
      61,    19,   122,   123,    22,   122,   123,   121,    69,    70,
     122,   123,    81,    31,   122,   123,    64,   122,   123,   121,
      92,   121,    40,   121,   107,    86,   121,   107,    46,    47,
      48,    49,     3,    57,    82,    53,   381,     3,    99,    98,
       3,    13,    60,    76,    62,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   517,    91,     3,   122,     3,   122,
     107,   121,   525,   526,     5,     3,     4,     5,     6,   121,
      88,     9,   121,   107,   114,   107,    55,    86,    86,    86,
       8,    19,   100,     3,    22,   101,     3,    78,    28,    48,
     121,     3,   122,    31,   123,     3,   114,   115,   123,     5,
     118,   121,    40,   121,    56,     3,   124,   570,    46,    47,
      48,    49,   107,    97,     3,    53,     3,   122,   122,    84,
     122,   107,    60,    79,    62,     5,    45,   107,     3,     4,
       5,     6,    66,     8,     9,   103,   121,     8,   121,   121,
     121,     8,     5,   104,   121,   123,   122,    22,   123,   102,
      88,     3,     4,     5,     6,    73,     8,     9,     8,   121,
       3,   115,   100,   123,   122,    40,    67,   122,     8,    24,
      22,   122,    49,    48,    49,    45,   114,   115,    53,    67,
     118,   107,    16,   121,   123,    60,   124,    62,    40,   122,
      58,   123,    24,   122,   104,   104,    48,    49,    67,    24,
       5,    53,   123,    24,     3,   121,   121,     8,    60,    37,
      62,   107,   123,     5,     3,     4,     5,     6,   123,     8,
       9,   118,   122,   122,   222,   100,   520,   510,   335,   153,
     381,   563,   534,    22,   178,    63,   317,   293,   572,   114,
     514,   282,   280,   118,   469,    18,   121,   122,   100,   124,
     398,    40,   307,   423,   350,     3,     4,     5,     6,    48,
      49,     9,   114,    -1,    53,    -1,   118,    -1,    -1,   121,
     122,    60,   124,    62,    22,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,    -1,    40,    -1,   122,   123,     3,     4,     5,     6,
      48,    49,     9,    -1,    -1,    53,    -1,    -1,    -1,    14,
      -1,   100,    60,    -1,    62,    22,    -1,    -1,    -1,    -1,
      -1,    26,    27,    -1,    -1,   114,    -1,    -1,    -1,   118,
      -1,    79,   121,    40,    -1,   124,    -1,    -1,    -1,    44,
      -1,    48,    49,    -1,    -1,    50,    53,    -1,    -1,    -1,
      -1,    -1,   100,    60,    37,    62,    61,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,   114,    -1,    -1,    -1,
     118,    -1,    -1,   121,    -1,    37,   124,    82,    -1,    -1,
      63,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,    -1,    -1,    -1,   114,    -1,    37,
      -1,   118,    -1,    -1,   121,    -1,    -1,   124,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,    63,    37,    -1,    -1,   122,
     123,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,    -1,
      -1,   123,    63,    -1,    37,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      63,    37,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,
      -1,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    63,    -1,    37,
      -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,    63,    37,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,    63,    -1,    37,    -1,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      63,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,    -1,    -1,    -1,
      -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,    15,    -1,    -1,    18,   122,
      20,    -1,    -1,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    37,    32,    33,    34,    35,    -1,    -1,    38,    -1,
      -1,    41,    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    63,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    63,    -1,    -1,    -1,    75,    -1,    77,    -1,    79,
      -1,    81,    -1,    83,    63,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    -1,    96,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    25,    29,    32,    33,    34,    35,
      38,    41,    51,    68,    75,    77,    79,    81,    83,    85,
      94,    96,   128,   129,   130,   131,   132,   133,   140,   177,
     182,   183,   186,   187,   188,   189,   196,   197,   206,   207,
     208,   209,   213,   214,   219,   220,   221,   224,   226,   227,
     228,   229,   230,   232,   233,    89,    50,     3,    43,    42,
      43,    72,    78,    50,     3,     4,     5,     6,     9,    10,
      19,    22,    31,    40,    46,    47,    48,    49,    53,    60,
      62,    88,   100,   114,   115,   118,   121,   124,   141,   142,
     144,   172,   173,   231,     3,    10,    44,    64,    82,   222,
      14,    26,    27,    44,    50,    61,    69,    70,    86,    90,
      99,   179,   222,    92,    78,     3,   145,     0,   120,    54,
       3,     3,     3,   121,     3,   145,     3,     3,     3,     3,
     121,   126,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,     3,    60,    62,   172,   172,   172,
       3,   176,    42,   123,     3,    16,   143,    37,    63,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    58,   164,   165,   107,   107,     3,
       9,     3,     5,     6,     8,    65,   180,   181,    92,     3,
       8,    86,    81,     3,    57,   178,   178,   178,    98,     3,
      81,   123,    79,    83,   131,     3,   178,    13,    91,     5,
       6,     8,   114,   121,   152,   153,   195,   198,   199,    76,
     101,     8,   121,   122,   172,   174,   175,     3,   172,   174,
      36,   115,   174,   122,   172,   174,   174,   172,   172,   172,
     122,   121,   121,   121,   122,   107,   123,   125,   121,   145,
     142,     3,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,     5,
       3,     8,    39,    65,    93,   114,   152,   184,   185,   181,
     107,   107,   114,    55,    86,    86,   178,    86,     8,   101,
     215,   216,     3,   210,   211,     3,   121,   190,    28,    78,
       5,     6,     8,   122,   152,   154,   200,   123,   201,    40,
      52,    87,   225,    48,    79,   133,   122,   123,   122,   122,
       3,   122,   122,   122,   122,   122,   122,   123,   122,   123,
     122,   172,   172,   152,     3,   137,   101,   146,   147,   123,
     184,   121,   180,    56,   178,   178,     3,   217,   218,   107,
     123,   147,     3,     4,    31,    46,    48,   100,   151,   191,
      97,     3,     3,   122,   123,   122,   123,   195,   199,   202,
     203,   122,    84,    49,   107,   122,   175,   122,   172,   172,
     107,    79,   140,    59,   121,   148,   149,   150,   151,    45,
     155,     5,   154,    73,    74,    80,   223,   107,   103,   121,
     152,   153,   211,    66,   166,   167,   121,   121,   121,   122,
     123,   121,   192,   193,    22,    23,    40,    53,   212,   152,
       8,    16,   204,   123,     8,   121,   152,   123,   134,   122,
     122,   152,   141,   122,   121,   148,   104,    21,    49,   107,
     108,   109,   110,   111,   112,   118,     5,   156,   102,   158,
     159,   122,    30,    95,    73,     8,   218,   122,   154,     3,
     168,   169,   115,   122,   122,   151,   121,   194,   195,   123,
       3,    58,   205,   195,   203,   154,     3,     5,    48,   135,
     136,   122,    42,    67,   138,     8,   122,   148,   152,   153,
       9,   121,     8,   152,   153,     8,   152,   153,   152,   153,
     152,   153,   152,   153,   152,   153,    49,    24,    45,    67,
     160,   161,   122,   107,   123,   122,   122,   123,   193,   122,
     123,    24,    58,   139,   122,   104,   104,   154,     9,   121,
     151,   157,    67,    24,   164,     3,     5,     8,   121,   169,
     195,   136,   151,   162,   163,     5,   152,   153,   152,   153,
     122,   154,   123,    24,    71,   162,   166,   121,     3,   170,
     171,    17,    34,   123,   123,   122,   151,   162,   121,     8,
     107,   122,   123,   163,     5,   122,   122,   152,   171
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

  case 27:

    { pParser->PushQuery(); ;}
    break;

  case 28:

    { pParser->PushQuery(); ;}
    break;

  case 32:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 34:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 37:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 38:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 42:

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

  case 43:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 45:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 46:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 47:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-6] );
		;}
    break;

  case 50:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 53:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 54:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 55:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 56:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 59:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 60:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 61:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 62:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 63:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 65:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 72:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 73:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 74:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 75:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 76:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 77:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 97:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 98:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 99:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 100:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 101:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 102:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 103:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 104:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 105:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 106:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 107:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 111:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 112:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 113:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 116:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 119:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 120:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 122:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 124:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 125:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 128:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 129:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 135:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 136:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 138:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 141:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 142:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 144:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 145:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 149:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 150:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 151:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 152:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 153:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 154:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 155:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 156:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 157:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 158:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 159:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 160:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 161:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 162:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 163:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 164:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 165:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 166:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 167:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 168:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 169:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 171:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 172:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 173:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 174:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 175:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 176:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 177:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 178:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 179:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 188:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 190:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 191:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 192:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 193:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 194:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 195:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 197:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 205:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 206:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 207:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 208:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 209:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 210:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 211:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 214:

    { yyval.m_iValue = 1; ;}
    break;

  case 215:

    { yyval.m_iValue = 0; ;}
    break;

  case 216:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 217:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 218:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 219:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 222:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 223:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 224:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 227:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 228:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 231:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 232:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 233:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 234:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 235:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 236:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 237:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 238:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 239:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 240:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 241:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 242:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 243:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 245:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 246:

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

  case 247:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 250:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 252:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 260:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 261:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 264:

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

  case 265:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 266:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 267:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 268:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 269:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 270:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 271:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 272:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 273:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 280:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 281:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 282:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 290:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 291:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 292:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 293:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 294:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 295:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 296:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 297:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 298:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 301:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 302:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
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

