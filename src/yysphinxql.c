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
     TOK_DROP = 296,
     TOK_FALSE = 297,
     TOK_FLOAT = 298,
     TOK_FLUSH = 299,
     TOK_FOR = 300,
     TOK_FROM = 301,
     TOK_FUNCTION = 302,
     TOK_GLOBAL = 303,
     TOK_GROUP = 304,
     TOK_GROUPBY = 305,
     TOK_GROUP_CONCAT = 306,
     TOK_ID = 307,
     TOK_IN = 308,
     TOK_INDEX = 309,
     TOK_INSERT = 310,
     TOK_INT = 311,
     TOK_INTEGER = 312,
     TOK_INTO = 313,
     TOK_IS = 314,
     TOK_ISOLATION = 315,
     TOK_LEVEL = 316,
     TOK_LIKE = 317,
     TOK_LIMIT = 318,
     TOK_MATCH = 319,
     TOK_MAX = 320,
     TOK_META = 321,
     TOK_MIN = 322,
     TOK_MOD = 323,
     TOK_NAMES = 324,
     TOK_NULL = 325,
     TOK_OPTION = 326,
     TOK_ORDER = 327,
     TOK_OPTIMIZE = 328,
     TOK_PLAN = 329,
     TOK_PROFILE = 330,
     TOK_RAND = 331,
     TOK_RAMCHUNK = 332,
     TOK_READ = 333,
     TOK_REPEATABLE = 334,
     TOK_REPLACE = 335,
     TOK_RETURNS = 336,
     TOK_ROLLBACK = 337,
     TOK_RTINDEX = 338,
     TOK_SELECT = 339,
     TOK_SERIALIZABLE = 340,
     TOK_SET = 341,
     TOK_SESSION = 342,
     TOK_SHOW = 343,
     TOK_SONAME = 344,
     TOK_START = 345,
     TOK_STATUS = 346,
     TOK_STRING = 347,
     TOK_SUM = 348,
     TOK_TABLE = 349,
     TOK_TABLES = 350,
     TOK_TO = 351,
     TOK_TRANSACTION = 352,
     TOK_TRUE = 353,
     TOK_TRUNCATE = 354,
     TOK_UNCOMMITTED = 355,
     TOK_UPDATE = 356,
     TOK_VALUES = 357,
     TOK_VARIABLES = 358,
     TOK_WARNINGS = 359,
     TOK_WEIGHT = 360,
     TOK_WHERE = 361,
     TOK_WITHIN = 362,
     TOK_OR = 363,
     TOK_AND = 364,
     TOK_NE = 365,
     TOK_GTE = 366,
     TOK_LTE = 367,
     TOK_NOT = 368,
     TOK_NEG = 369
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
#define TOK_DROP 296
#define TOK_FALSE 297
#define TOK_FLOAT 298
#define TOK_FLUSH 299
#define TOK_FOR 300
#define TOK_FROM 301
#define TOK_FUNCTION 302
#define TOK_GLOBAL 303
#define TOK_GROUP 304
#define TOK_GROUPBY 305
#define TOK_GROUP_CONCAT 306
#define TOK_ID 307
#define TOK_IN 308
#define TOK_INDEX 309
#define TOK_INSERT 310
#define TOK_INT 311
#define TOK_INTEGER 312
#define TOK_INTO 313
#define TOK_IS 314
#define TOK_ISOLATION 315
#define TOK_LEVEL 316
#define TOK_LIKE 317
#define TOK_LIMIT 318
#define TOK_MATCH 319
#define TOK_MAX 320
#define TOK_META 321
#define TOK_MIN 322
#define TOK_MOD 323
#define TOK_NAMES 324
#define TOK_NULL 325
#define TOK_OPTION 326
#define TOK_ORDER 327
#define TOK_OPTIMIZE 328
#define TOK_PLAN 329
#define TOK_PROFILE 330
#define TOK_RAND 331
#define TOK_RAMCHUNK 332
#define TOK_READ 333
#define TOK_REPEATABLE 334
#define TOK_REPLACE 335
#define TOK_RETURNS 336
#define TOK_ROLLBACK 337
#define TOK_RTINDEX 338
#define TOK_SELECT 339
#define TOK_SERIALIZABLE 340
#define TOK_SET 341
#define TOK_SESSION 342
#define TOK_SHOW 343
#define TOK_SONAME 344
#define TOK_START 345
#define TOK_STATUS 346
#define TOK_STRING 347
#define TOK_SUM 348
#define TOK_TABLE 349
#define TOK_TABLES 350
#define TOK_TO 351
#define TOK_TRANSACTION 352
#define TOK_TRUE 353
#define TOK_TRUNCATE 354
#define TOK_UNCOMMITTED 355
#define TOK_UPDATE 356
#define TOK_VALUES 357
#define TOK_VARIABLES 358
#define TOK_WARNINGS 359
#define TOK_WEIGHT 360
#define TOK_WHERE 361
#define TOK_WITHIN 362
#define TOK_OR 363
#define TOK_AND 364
#define TOK_NE 365
#define TOK_GTE 366
#define TOK_LTE 367
#define TOK_NOT 368
#define TOK_NEG 369




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
#define YYFINAL  123
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1335

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  134
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  114
/* YYNRULES -- Number of rules. */
#define YYNRULES  329
/* YYNRULES -- Number of states. */
#define YYNSTATES  623

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   369

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   122,   111,     2,
     126,   127,   120,   118,   128,   119,   131,   121,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   125,
     114,   112,   115,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   132,     2,   133,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   129,   110,   130,     2,     2,     2,     2,
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
     105,   106,   107,   108,   109,   113,   116,   117,   123,   124
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    56,    58,
      60,    64,    66,    68,    70,    79,    81,    91,    92,    95,
      97,   101,   103,   105,   107,   108,   112,   113,   116,   121,
     132,   134,   138,   140,   143,   144,   146,   149,   151,   156,
     161,   166,   171,   176,   181,   185,   191,   193,   197,   198,
     200,   203,   205,   209,   213,   218,   222,   226,   232,   239,
     243,   248,   254,   258,   262,   266,   270,   274,   276,   282,
     288,   294,   298,   302,   306,   310,   314,   318,   322,   327,
     331,   333,   335,   340,   344,   348,   350,   352,   354,   357,
     359,   362,   364,   366,   370,   371,   376,   377,   379,   381,
     385,   386,   388,   394,   395,   397,   401,   407,   409,   413,
     415,   418,   421,   422,   424,   427,   432,   433,   435,   438,
     440,   444,   448,   452,   458,   465,   469,   471,   475,   479,
     481,   483,   485,   487,   489,   491,   493,   496,   499,   503,
     507,   511,   515,   519,   523,   527,   531,   535,   539,   543,
     547,   551,   555,   559,   563,   567,   571,   575,   577,   579,
     584,   589,   594,   599,   604,   608,   615,   622,   626,   635,
     637,   641,   643,   645,   649,   655,   657,   659,   661,   663,
     666,   667,   670,   672,   675,   678,   682,   684,   686,   691,
     696,   700,   702,   704,   706,   708,   710,   712,   716,   721,
     726,   731,   735,   740,   745,   753,   759,   761,   763,   765,
     767,   769,   771,   773,   775,   777,   780,   787,   789,   791,
     792,   796,   798,   802,   804,   808,   812,   814,   818,   820,
     822,   824,   828,   831,   839,   849,   856,   858,   862,   864,
     868,   870,   874,   875,   878,   880,   884,   888,   889,   891,
     893,   895,   899,   901,   903,   907,   911,   918,   920,   924,
     928,   932,   938,   943,   947,   951,   953,   955,   957,   959,
     967,   972,   978,   979,   981,   984,   986,   990,   994,   997,
    1001,  1008,  1009,  1011,  1013,  1016,  1019,  1022,  1024,  1032,
    1034,  1036,  1038,  1040,  1044,  1051,  1055,  1059,  1063,  1065,
    1069,  1072,  1076,  1080,  1083,  1085,  1088,  1090,  1092,  1096
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     135,     0,    -1,   136,    -1,   137,    -1,   137,   125,    -1,
     197,    -1,   205,    -1,   191,    -1,   192,    -1,   195,    -1,
     206,    -1,   215,    -1,   217,    -1,   218,    -1,   219,    -1,
     224,    -1,   229,    -1,   230,    -1,   234,    -1,   236,    -1,
     237,    -1,   238,    -1,   239,    -1,   231,    -1,   240,    -1,
     242,    -1,   243,    -1,   244,    -1,   223,    -1,   138,    -1,
     137,   125,   138,    -1,   139,    -1,   186,    -1,   140,    -1,
      84,     3,   126,   126,   140,   127,   141,   127,    -1,   147,
      -1,    84,   148,    46,   126,   144,   147,   127,   145,   146,
      -1,    -1,   128,   142,    -1,   143,    -1,   142,   128,   143,
      -1,     3,    -1,     5,    -1,    52,    -1,    -1,    72,    26,
     169,    -1,    -1,    63,     5,    -1,    63,     5,   128,     5,
      -1,    84,   148,    46,   152,   153,   162,   165,   167,   171,
     173,    -1,   149,    -1,   148,   128,   149,    -1,   120,    -1,
     151,   150,    -1,    -1,     3,    -1,    18,     3,    -1,   179,
      -1,    21,   126,   179,   127,    -1,    65,   126,   179,   127,
      -1,    67,   126,   179,   127,    -1,    93,   126,   179,   127,
      -1,    51,   126,   179,   127,    -1,    33,   126,   120,   127,
      -1,    50,   126,   127,    -1,    33,   126,    39,     3,   127,
      -1,     3,    -1,   152,   128,     3,    -1,    -1,   154,    -1,
     106,   155,    -1,   156,    -1,   155,   109,   155,    -1,   126,
     155,   127,    -1,    64,   126,     8,   127,    -1,   158,   112,
     159,    -1,   158,   113,   159,    -1,   158,    53,   126,   161,
     127,    -1,   158,   123,    53,   126,   161,   127,    -1,   158,
      53,     9,    -1,   158,   123,    53,     9,    -1,   158,    23,
     159,   109,   159,    -1,   158,   115,   159,    -1,   158,   114,
     159,    -1,   158,   116,   159,    -1,   158,   117,   159,    -1,
     158,   112,   160,    -1,   157,    -1,   158,    23,   160,   109,
     160,    -1,   158,    23,   159,   109,   160,    -1,   158,    23,
     160,   109,   159,    -1,   158,   115,   160,    -1,   158,   114,
     160,    -1,   158,   116,   160,    -1,   158,   117,   160,    -1,
     158,   112,     8,    -1,   158,   113,     8,    -1,   158,    59,
      70,    -1,   158,    59,   123,    70,    -1,   158,   113,   160,
      -1,     3,    -1,     4,    -1,    33,   126,   120,   127,    -1,
      50,   126,   127,    -1,   105,   126,   127,    -1,    52,    -1,
     245,    -1,     5,    -1,   119,     5,    -1,     6,    -1,   119,
       6,    -1,    14,    -1,   159,    -1,   161,   128,   159,    -1,
      -1,    49,   163,    26,   164,    -1,    -1,     5,    -1,   158,
      -1,   164,   128,   158,    -1,    -1,   166,    -1,   107,    49,
      72,    26,   169,    -1,    -1,   168,    -1,    72,    26,   169,
      -1,    72,    26,    76,   126,   127,    -1,   170,    -1,   169,
     128,   170,    -1,   158,    -1,   158,    19,    -1,   158,    37,
      -1,    -1,   172,    -1,    63,     5,    -1,    63,     5,   128,
       5,    -1,    -1,   174,    -1,    71,   175,    -1,   176,    -1,
     175,   128,   176,    -1,     3,   112,     3,    -1,     3,   112,
       5,    -1,     3,   112,   126,   177,   127,    -1,     3,   112,
       3,   126,     8,   127,    -1,     3,   112,     8,    -1,   178,
      -1,   177,   128,   178,    -1,     3,   112,   159,    -1,     3,
      -1,     4,    -1,    52,    -1,     5,    -1,     6,    -1,    14,
      -1,     9,    -1,   119,   179,    -1,   123,   179,    -1,   179,
     118,   179,    -1,   179,   119,   179,    -1,   179,   120,   179,
      -1,   179,   121,   179,    -1,   179,   114,   179,    -1,   179,
     115,   179,    -1,   179,   111,   179,    -1,   179,   110,   179,
      -1,   179,   122,   179,    -1,   179,    40,   179,    -1,   179,
      68,   179,    -1,   179,   117,   179,    -1,   179,   116,   179,
      -1,   179,   112,   179,    -1,   179,   113,   179,    -1,   179,
     109,   179,    -1,   179,   108,   179,    -1,   126,   179,   127,
      -1,   129,   183,   130,    -1,   180,    -1,   245,    -1,     3,
     126,   181,   127,    -1,    53,   126,   181,   127,    -1,    57,
     126,   181,   127,    -1,    24,   126,   181,   127,    -1,    43,
     126,   181,   127,    -1,     3,   126,   127,    -1,    67,   126,
     179,   128,   179,   127,    -1,    65,   126,   179,   128,   179,
     127,    -1,   105,   126,   127,    -1,     3,   126,   179,    45,
       3,    53,   245,   127,    -1,   182,    -1,   181,   128,   182,
      -1,   179,    -1,     8,    -1,   184,   112,   185,    -1,   183,
     128,   184,   112,   185,    -1,     3,    -1,    53,    -1,   159,
      -1,     3,    -1,    88,   188,    -1,    -1,    62,     8,    -1,
     104,    -1,    91,   187,    -1,    66,   187,    -1,    16,    91,
     187,    -1,    75,    -1,    74,    -1,    16,     8,    91,   187,
      -1,    16,     3,    91,   187,    -1,    54,     3,    91,    -1,
       3,    -1,    70,    -1,     8,    -1,     5,    -1,     6,    -1,
     189,    -1,   190,   119,   189,    -1,    86,     3,   112,   194,
      -1,    86,     3,   112,   193,    -1,    86,     3,   112,    70,
      -1,    86,    69,   190,    -1,    86,    10,   112,   190,    -1,
      86,    28,    86,   190,    -1,    86,    48,     9,   112,   126,
     161,   127,    -1,    86,    48,     3,   112,   193,    -1,     3,
      -1,     8,    -1,    98,    -1,    42,    -1,   159,    -1,    31,
      -1,    82,    -1,   196,    -1,    22,    -1,    90,    97,    -1,
     198,    58,     3,   199,   102,   201,    -1,    55,    -1,    80,
      -1,    -1,   126,   200,   127,    -1,   158,    -1,   200,   128,
     158,    -1,   202,    -1,   201,   128,   202,    -1,   126,   203,
     127,    -1,   204,    -1,   203,   128,   204,    -1,   159,    -1,
     160,    -1,     8,    -1,   126,   161,   127,    -1,   126,   127,
      -1,    36,    46,   152,   106,    52,   112,   159,    -1,    36,
      46,   152,   106,    52,    53,   126,   161,   127,    -1,    27,
       3,   126,   207,   210,   127,    -1,   208,    -1,   207,   128,
     208,    -1,   204,    -1,   126,   209,   127,    -1,     8,    -1,
     209,   128,     8,    -1,    -1,   128,   211,    -1,   212,    -1,
     211,   128,   212,    -1,   204,   213,   214,    -1,    -1,    18,
      -1,     3,    -1,    63,    -1,   216,     3,   187,    -1,    38,
      -1,    37,    -1,    88,    95,   187,    -1,    88,    35,   187,
      -1,   101,   152,    86,   220,   154,   173,    -1,   221,    -1,
     220,   128,   221,    -1,     3,   112,   159,    -1,     3,   112,
     160,    -1,     3,   112,   126,   161,   127,    -1,     3,   112,
     126,   127,    -1,   245,   112,   159,    -1,   245,   112,   160,
      -1,    57,    -1,    24,    -1,    43,    -1,    25,    -1,    17,
      94,     3,    15,    30,     3,   222,    -1,    88,   232,   103,
     225,    -1,    88,   232,   103,    62,     8,    -1,    -1,   226,
      -1,   106,   227,    -1,   228,    -1,   227,   108,   228,    -1,
       3,   112,     8,    -1,    88,    29,    -1,    88,    28,    86,
      -1,    86,   232,    97,    60,    61,   233,    -1,    -1,    48,
      -1,    87,    -1,    78,   100,    -1,    78,    32,    -1,    79,
      78,    -1,    85,    -1,    34,    47,     3,    81,   235,    89,
       8,    -1,    56,    -1,    24,    -1,    43,    -1,    92,    -1,
      41,    47,     3,    -1,    20,    54,     3,    96,    83,     3,
      -1,    44,    83,     3,    -1,    44,    77,     3,    -1,    84,
     241,   171,    -1,    10,    -1,    10,   131,     3,    -1,    84,
     179,    -1,    99,    83,     3,    -1,    73,    54,     3,    -1,
       3,   246,    -1,   247,    -1,   246,   247,    -1,    13,    -1,
      14,    -1,   132,   179,   133,    -1,   132,     8,   133,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   160,   160,   161,   162,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   195,
     196,   200,   201,   205,   206,   214,   215,   222,   224,   228,
     232,   239,   240,   241,   245,   258,   265,   267,   272,   281,
     296,   297,   301,   302,   305,   307,   308,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   324,   325,   328,   330,
     334,   338,   339,   340,   344,   349,   356,   364,   372,   381,
     386,   391,   396,   401,   406,   411,   416,   421,   426,   431,
     436,   441,   446,   451,   456,   461,   466,   471,   476,   484,
     488,   489,   494,   500,   506,   512,   518,   522,   523,   534,
     535,   536,   540,   546,   552,   554,   557,   559,   566,   570,
     576,   578,   582,   593,   595,   599,   603,   610,   611,   615,
     616,   617,   620,   622,   626,   631,   638,   640,   644,   648,
     649,   653,   658,   663,   669,   674,   682,   687,   694,   704,
     705,   706,   707,   708,   709,   710,   711,   712,   714,   715,
     716,   717,   718,   719,   720,   721,   722,   723,   724,   725,
     726,   727,   728,   729,   730,   731,   732,   733,   734,   738,
     739,   740,   741,   742,   743,   744,   745,   746,   747,   751,
     752,   756,   757,   761,   762,   766,   767,   771,   772,   778,
     781,   783,   787,   788,   789,   790,   791,   792,   793,   798,
     803,   813,   814,   815,   816,   817,   821,   822,   826,   831,
     836,   841,   842,   843,   847,   852,   860,   861,   865,   866,
     867,   881,   882,   883,   887,   888,   894,   902,   903,   906,
     908,   912,   913,   917,   918,   922,   926,   927,   931,   932,
     933,   934,   935,   941,   949,   963,   971,   975,   982,   983,
     990,  1000,  1006,  1008,  1012,  1017,  1021,  1028,  1030,  1034,
    1035,  1041,  1049,  1050,  1056,  1060,  1066,  1074,  1075,  1079,
    1093,  1099,  1103,  1108,  1122,  1133,  1134,  1135,  1136,  1140,
    1153,  1157,  1164,  1165,  1169,  1173,  1174,  1178,  1182,  1189,
    1196,  1202,  1203,  1204,  1208,  1209,  1210,  1211,  1217,  1228,
    1229,  1230,  1231,  1235,  1246,  1258,  1267,  1278,  1286,  1287,
    1291,  1301,  1312,  1323,  1326,  1327,  1331,  1332,  1333,  1334
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
  "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", 
  "TOK_FLUSH", "TOK_FOR", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", 
  "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", "TOK_IN", 
  "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTEGER", "TOK_INTO", 
  "TOK_IS", "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", 
  "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", 
  "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_PLAN", 
  "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", "TOK_REPEATABLE", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", 
  "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", 
  "TOK_TABLE", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", 
  "TOK_TRUNCATE", "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", 
  "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", 
  "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", 
  "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", 
  "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", "'}'", "'.'", 
  "'['", "']'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select", "select1", "opt_tablefunc_args", 
  "tablefunc_args_list", "tablefunc_arg", "subselect_start", 
  "opt_outer_order", "opt_outer_limit", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "expr_float_unhandled", "expr_ident", "const_int", 
  "const_float", "const_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_group_order_clause", "group_order_clause", 
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
  "udf_type", "drop_function", "attach_index", "flush_rtindex", 
  "flush_ramchunk", "select_sysvar", "sysvar_name", "select_dual", 
  "truncate", "optimize_index", "json_field", "subscript", "subkey", 0
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
     124,    38,    61,   365,    60,    62,   366,   367,    43,    45,
      42,    47,    37,   368,   369,    59,    40,    41,    44,   123,
     125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   134,   135,   135,   135,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   137,
     137,   138,   138,   139,   139,   140,   140,   141,   141,   142,
     142,   143,   143,   143,   144,   145,   146,   146,   146,   147,
     148,   148,   149,   149,   150,   150,   150,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   152,   152,   153,   153,
     154,   155,   155,   155,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   157,
     158,   158,   158,   158,   158,   158,   158,   159,   159,   160,
     160,   160,   161,   161,   162,   162,   163,   163,   164,   164,
     165,   165,   166,   167,   167,   168,   168,   169,   169,   170,
     170,   170,   171,   171,   172,   172,   173,   173,   174,   175,
     175,   176,   176,   176,   176,   176,   177,   177,   178,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   181,
     181,   182,   182,   183,   183,   184,   184,   185,   185,   186,
     187,   187,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   189,   189,   189,   189,   189,   190,   190,   191,   191,
     191,   191,   191,   191,   192,   192,   193,   193,   194,   194,
     194,   195,   195,   195,   196,   196,   197,   198,   198,   199,
     199,   200,   200,   201,   201,   202,   203,   203,   204,   204,
     204,   204,   204,   205,   205,   206,   207,   207,   208,   208,
     209,   209,   210,   210,   211,   211,   212,   213,   213,   214,
     214,   215,   216,   216,   217,   218,   219,   220,   220,   221,
     221,   221,   221,   221,   221,   222,   222,   222,   222,   223,
     224,   224,   225,   225,   226,   227,   227,   228,   229,   230,
     231,   232,   232,   232,   233,   233,   233,   233,   234,   235,
     235,   235,   235,   236,   237,   238,   239,   240,   241,   241,
     242,   243,   244,   245,   246,   246,   247,   247,   247,   247
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     8,     1,     9,     0,     2,     1,
       3,     1,     1,     1,     0,     3,     0,     2,     4,    10,
       1,     3,     1,     2,     0,     1,     2,     1,     4,     4,
       4,     4,     4,     4,     3,     5,     1,     3,     0,     1,
       2,     1,     3,     3,     4,     3,     3,     5,     6,     3,
       4,     5,     3,     3,     3,     3,     3,     1,     5,     5,
       5,     3,     3,     3,     3,     3,     3,     3,     4,     3,
       1,     1,     4,     3,     3,     1,     1,     1,     2,     1,
       2,     1,     1,     3,     0,     4,     0,     1,     1,     3,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     4,
       4,     4,     4,     4,     3,     6,     6,     3,     8,     1,
       3,     1,     1,     3,     5,     1,     1,     1,     1,     2,
       0,     2,     1,     2,     2,     3,     1,     1,     4,     4,
       3,     1,     1,     1,     1,     1,     1,     3,     4,     4,
       4,     3,     4,     4,     7,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     6,     1,     1,     0,
       3,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     3,     2,     7,     9,     6,     1,     3,     1,     3,
       1,     3,     0,     2,     1,     3,     3,     0,     1,     1,
       1,     3,     1,     1,     3,     3,     6,     1,     3,     3,
       3,     5,     4,     3,     3,     1,     1,     1,     1,     7,
       4,     5,     0,     1,     2,     1,     3,     3,     2,     3,
       6,     0,     1,     1,     2,     2,     2,     1,     7,     1,
       1,     1,     1,     3,     6,     3,     3,     3,     1,     3,
       2,     3,     3,     2,     1,     2,     1,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   234,     0,   231,     0,     0,   273,   272,
       0,     0,   237,     0,   238,   232,     0,   301,   301,     0,
       0,     0,     0,     2,     3,    29,    31,    33,    35,    32,
       7,     8,     9,   233,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   149,   150,   152,   153,
     155,   318,   154,     0,     0,     0,     0,     0,     0,   151,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,    50,    54,    57,   177,   132,   178,     0,     0,
       0,   302,     0,   303,     0,     0,     0,   298,   200,   302,
       0,   200,   207,   206,   200,   200,   202,   199,     0,   235,
       0,    66,     0,     1,     4,     0,   200,     0,     0,     0,
       0,     0,   313,   316,   315,   322,   326,   327,     0,     0,
     323,   324,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   149,     0,     0,   156,   157,
       0,   195,   196,     0,     0,     0,     0,    55,     0,    53,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   317,   133,
       0,     0,     0,     0,     0,   211,   214,   215,   213,   212,
     216,   221,     0,     0,     0,   200,   299,     0,   275,     0,
     204,   203,   274,   292,   321,     0,     0,     0,     0,    30,
     239,   271,     0,     0,   107,   109,   250,   111,     0,     0,
     248,   249,   258,   262,   256,     0,     0,   192,     0,   184,
     191,     0,   189,     0,     0,   325,   319,     0,   191,     0,
       0,     0,     0,    64,     0,     0,     0,     0,     0,     0,
     187,     0,     0,     0,   175,     0,   176,     0,    44,    68,
      51,    57,    56,   167,   168,   174,   173,   165,   164,   171,
     172,   162,   163,   170,   169,   158,   159,   160,   161,   166,
     134,   226,   227,   229,   220,   228,     0,   230,   219,   218,
     222,   223,     0,     0,     0,     0,   200,   200,   205,   201,
     210,     0,     0,   290,   293,     0,     0,   277,     0,    67,
       0,     0,     0,     0,   108,   110,   260,   252,   112,     0,
       0,     0,     0,   310,   311,   309,   312,     0,     0,     0,
       0,     0,   179,     0,   329,   328,    58,   182,     0,    63,
     183,    62,   180,   181,    59,     0,    60,     0,    61,     0,
       0,     0,   198,   197,   193,     0,     0,   114,    69,     0,
     225,     0,   217,     0,   209,   208,   291,     0,   294,   295,
       0,     0,   136,     0,   100,   101,     0,     0,   105,     0,
     241,     0,   106,     0,     0,   314,   251,     0,   259,     0,
     258,   257,   263,   264,   255,     0,     0,     0,    37,     0,
     190,    65,     0,     0,     0,     0,     0,     0,     0,    70,
      71,    87,     0,   116,   120,   135,     0,     0,     0,   307,
     300,     0,     0,     0,   279,   280,   278,     0,   276,   137,
     283,   284,     0,     0,     0,   240,     0,     0,   236,   243,
     286,   288,   287,   285,   289,   113,   261,   268,     0,     0,
     308,     0,   253,     0,     0,     0,   186,   185,   194,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   117,     0,     0,   123,   121,   224,
     305,   304,   306,   297,   296,   282,     0,     0,   138,   139,
       0,   103,   104,   242,     0,     0,   246,     0,   269,   270,
     266,   267,   265,     0,    41,    42,    43,    38,    39,    34,
       0,     0,     0,     0,    46,     0,    73,    72,     0,     0,
      79,     0,    97,     0,    95,    75,    86,    96,    76,    99,
      83,    92,    82,    91,    84,    93,    85,    94,     0,     0,
       0,     0,   132,   124,   281,     0,     0,   102,   245,     0,
     244,   254,     0,   188,     0,     0,    36,    74,     0,     0,
       0,    98,    80,     0,   118,   115,     0,     0,   136,   141,
     142,   145,     0,   140,   247,    40,   129,    45,   127,    47,
      81,    89,    90,    88,    77,     0,     0,     0,     0,   125,
      49,     0,     0,     0,   146,   130,   131,     0,     0,    78,
     119,   122,     0,     0,     0,   143,     0,   128,    48,   126,
     144,   148,   147
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   464,   517,   518,
     365,   524,   566,    28,    91,    92,   169,    93,   269,   367,
     368,   419,   420,   421,   586,   328,   231,   329,   424,   485,
     575,   487,   488,   552,   553,   587,   588,   188,   189,   438,
     439,   498,   499,   603,   604,   248,    95,   241,   242,   163,
     164,   364,    29,   208,   117,   200,   201,    30,    31,   298,
     299,    32,    33,    34,    35,   321,   391,   448,   449,   505,
     232,    36,    37,   233,   234,   330,   332,   402,   403,   458,
     510,    38,    39,    40,    41,    42,   316,   317,   454,    43,
      44,   313,   314,   378,   379,    45,    46,    47,   104,   430,
      48,   337,    49,    50,    51,    52,    53,    96,    54,    55,
      56,    97,   140,   141
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -496
static const short yypact[] =
{
    1083,   -68,    17,  -496,    61,  -496,    78,   114,  -496,  -496,
     170,   115,  -496,   181,  -496,  -496,   333,   201,   697,   118,
     141,   225,   240,  -496,   123,  -496,  -496,  -496,  -496,  -496,
    -496,  -496,  -496,  -496,  -496,   216,  -496,  -496,  -496,   254,
    -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,
    -496,  -496,  -496,  -496,  -496,  -496,  -496,   256,   277,   156,
     281,   225,   286,   294,   296,   322,    25,  -496,  -496,  -496,
    -496,   204,  -496,   207,   215,   218,   220,   229,   239,  -496,
     247,   261,   262,   271,   273,   288,   654,  -496,   654,   654,
      33,   -19,  -496,   242,   193,  -496,   312,  -496,   295,   305,
     332,   249,   326,  -496,   323,    71,   338,  -496,   357,  -496,
     424,   357,  -496,  -496,   357,   357,  -496,  -496,   325,  -496,
     426,  -496,   -40,  -496,   124,   428,   357,   418,   339,    29,
     353,    -9,  -496,  -496,  -496,  -496,  -496,  -496,   505,   547,
      56,  -496,   433,   654,   612,   -10,   612,   310,   654,   612,
     612,   654,   654,   654,   314,    37,   316,   321,  -496,  -496,
     838,  -496,  -496,   -76,   336,     2,   358,  -496,   447,  -496,
     654,   654,   654,   654,   654,   654,   654,   654,   654,   654,
     654,   654,   654,   654,   654,   654,   654,   450,  -496,  -496,
     202,   326,   326,   345,   346,  -496,  -496,  -496,  -496,  -496,
    -496,   341,   405,   375,   376,   357,  -496,   460,  -496,   378,
    -496,  -496,  -496,   128,  -496,   467,   468,   440,   531,  -496,
     348,  -496,   442,   392,  -496,  -496,  -496,  -496,   260,    16,
    -496,  -496,  -496,   351,  -496,    24,   430,  -496,   677,  -496,
    1078,   149,  -496,   343,   699,  -496,  -496,   867,  1093,   164,
     477,   359,   189,  -496,   893,   192,   196,   725,   754,   922,
    -496,   522,   654,   654,  -496,    33,  -496,    27,  -496,   103,
    -496,  1093,  -496,  -496,  -496,  1107,  1190,  1202,  1213,  1117,
    1117,   -26,   -26,   -26,   -26,   174,   174,  -496,  -496,  -496,
     360,  -496,  -496,  -496,  -496,  -496,   484,  -496,  -496,  -496,
     341,   341,    75,   368,   326,   434,   357,   357,  -496,  -496,
    -496,   488,   495,  -496,  -496,     9,   110,  -496,   387,  -496,
     223,   398,   498,   500,  -496,  -496,  -496,  -496,  -496,   199,
     232,    29,   377,  -496,  -496,  -496,  -496,   417,    15,   358,
     385,   512,  -496,   612,  -496,  -496,  -496,  -496,   389,  -496,
    -496,  -496,  -496,  -496,  -496,   654,  -496,   654,  -496,   783,
     809,   406,  -496,  -496,  -496,   436,     8,   472,  -496,   517,
    -496,    28,  -496,   168,  -496,  -496,  -496,   411,   409,  -496,
      70,   467,   453,   117,    56,  -496,   408,   413,  -496,   414,
    -496,   241,  -496,   415,   198,  -496,  -496,    28,  -496,   527,
      63,  -496,   416,  -496,  -496,   529,   423,    28,   439,   489,
    -496,  -496,   951,   977,    27,   358,   427,   438,     8,   459,
    -496,  -496,   479,   538,   466,  -496,   243,    -1,   499,  -496,
    -496,   568,   495,     5,  -496,  -496,  -496,   575,  -496,  -496,
    -496,  -496,   461,   455,   456,  -496,   223,    39,   452,  -496,
    -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,    22,    39,
    -496,    28,  -496,   161,   457,   583,  -496,  -496,  -496,    31,
     516,   590,   -54,     8,   117,     7,    45,    51,   106,   117,
     117,   117,   117,   548,  -496,   577,   558,   536,  -496,  -496,
    -496,  -496,  -496,  -496,  -496,  -496,   250,   497,   483,  -496,
     486,  -496,  -496,  -496,    23,   253,  -496,   415,  -496,  -496,
    -496,   601,  -496,   265,  -496,  -496,  -496,   501,  -496,  -496,
      56,   496,   225,   599,   567,   506,  -496,  -496,   528,   530,
    -496,    28,  -496,   570,  -496,  -496,  -496,  -496,  -496,  -496,
    -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,    11,   223,
     566,   616,   312,  -496,  -496,    10,   575,  -496,  -496,    39,
    -496,  -496,   161,  -496,   223,   638,  -496,  -496,   117,   117,
     267,  -496,  -496,    28,  -496,   518,   618,   217,   453,   521,
    -496,  -496,   647,  -496,  -496,  -496,   182,   525,  -496,   526,
    -496,  -496,  -496,  -496,  -496,   276,   223,   223,   535,   525,
    -496,   648,   550,   278,  -496,  -496,  -496,   223,   662,  -496,
    -496,   525,   544,   545,    28,  -496,   647,  -496,  -496,  -496,
    -496,  -496,  -496
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -496,  -496,  -496,  -496,   551,  -496,   446,  -496,  -496,   112,
    -496,  -496,  -496,   320,   272,   523,  -496,  -496,   176,  -496,
     372,  -356,  -496,  -496,  -317,  -129,  -374,  -370,  -496,  -496,
    -496,  -496,  -496,  -496,  -496,  -495,    83,   140,  -496,   116,
    -496,  -496,   139,  -496,    77,     1,  -496,   137,   355,  -496,
     431,   285,  -496,  -107,  -496,   396,   221,  -496,  -496,   400,
    -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,   197,  -496,
    -329,  -496,  -496,  -496,   374,  -496,  -496,  -496,   244,  -496,
    -496,  -496,  -496,  -496,  -496,  -496,  -496,   327,  -496,  -496,
    -496,  -496,  -496,  -496,   280,  -496,  -496,  -496,   691,  -496,
    -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,  -496,
    -496,  -175,  -496,   574
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -321
static const short yytable[] =
{
     230,   426,   400,   390,   210,   121,   435,   211,   212,   441,
     224,   384,   385,   579,   170,   580,   530,    94,   581,   221,
     572,   224,   136,   137,   326,   508,    57,   165,   224,   250,
     362,   490,   224,   224,   224,   225,   161,   226,   136,   137,
     318,   386,   171,   227,   224,   225,   215,   226,   333,   422,
     136,   137,   265,   227,   266,   473,   224,   225,   387,   534,
     388,   297,   472,   496,    59,   227,  -267,   334,   406,   136,
     137,    58,   417,   526,   203,   224,   225,   522,   291,   204,
     335,   457,   599,   292,   227,   509,   162,   158,   216,   159,
     160,   513,   182,   183,   184,   185,   186,   236,   308,   491,
     529,   422,   611,   536,   539,   541,   543,   545,   547,   166,
     251,   224,   225,   389,   537,   532,   336,   527,   506,   216,
     227,   380,   224,   225,   296,    60,  -267,   407,   268,   503,
     511,   227,   495,   531,   418,   296,   582,   573,   363,   240,
     244,   139,   296,   327,   247,   392,   296,   296,   228,   254,
     327,   138,   257,   258,   259,   229,   422,   139,   228,   166,
      61,   570,   205,   261,   514,   504,   515,   271,   533,   139,
     228,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   139,   228,
     311,   392,    63,  -320,   591,   593,   433,   122,    64,   374,
     375,   605,   230,   595,    98,   291,   318,   224,   217,   366,
     292,    99,   218,   516,   170,   119,   366,    62,   271,   606,
     384,   385,   450,   451,   120,   228,   384,   385,   121,   100,
     584,   216,   574,   170,   312,    65,   228,   131,   381,   160,
     123,   452,   171,   392,   293,   167,   427,   428,   124,   101,
     386,   434,   193,   429,   440,   453,   386,   126,   194,   127,
     168,   171,   240,   359,   360,   324,   325,   387,   455,   388,
     102,   392,   294,   387,   125,   388,   342,   343,   462,   610,
     128,   249,   129,   252,   130,   363,   255,   256,   103,   132,
     521,   347,   343,   598,   184,   185,   186,   133,   392,   134,
     295,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   350,   343,   230,   352,
     343,   296,   389,   353,   343,   135,   396,   397,   389,   195,
     230,   196,   197,   143,   198,   142,    66,    67,    68,    69,
     271,   144,    70,    71,   145,   528,   146,    72,   535,   538,
     540,   542,   544,   546,    73,   147,   412,    74,   413,   398,
     399,   155,    67,    68,    69,   148,    75,    70,   445,   446,
     489,   397,    72,   149,   392,   187,    76,   554,   397,    73,
     558,   559,    74,    77,    78,    79,    80,   150,   151,   392,
      81,    75,   561,   397,   594,   397,   199,   152,    82,   153,
      83,    76,   392,   609,   397,   615,   616,   190,    77,    78,
      79,    80,   300,   301,   154,    81,   271,   191,   192,   207,
     202,   392,   392,    82,   206,    83,    84,   209,   213,   214,
     230,   220,   392,   222,   235,   223,   246,   253,    85,   590,
     592,   260,   262,    66,    67,    68,    69,   263,   267,    70,
     272,    84,    86,    87,    72,   290,    88,   302,   303,    89,
     304,    73,    90,    85,    74,   305,   306,   307,   309,   310,
     315,   319,   322,    75,   320,   323,   344,    86,    87,   331,
     348,    88,   338,    76,    89,   621,   349,    90,   369,   324,
      77,    78,    79,    80,   371,   373,   376,    81,   377,   383,
     393,   394,   474,   395,   404,    82,   405,    83,   155,    67,
      68,    69,   408,   237,    70,   409,   411,   432,   414,    72,
     415,   423,   425,   431,   437,   155,    67,    68,    69,    74,
     237,    70,   475,    84,   442,   456,    72,   460,   476,   443,
     444,   447,   465,   484,   459,    85,    74,   105,    76,   461,
     155,    67,    68,    69,   470,   243,    70,    79,    80,    86,
      87,    72,    81,    88,   471,    76,    89,   463,   473,    90,
     156,    74,   157,   486,    79,    80,   493,   492,   497,    81,
     507,   500,   501,   502,   519,   110,   520,   156,   523,   157,
      76,   477,   478,   479,   480,   481,   482,   111,   525,    79,
      80,   548,   483,   549,    81,   112,   113,   550,   551,   555,
      85,   556,   156,   557,   157,   155,    67,    68,    69,   457,
     237,    70,   114,   563,    86,   564,    72,    85,    88,   562,
     565,   238,   239,   567,    90,   116,    74,   568,   576,   569,
     571,    86,   577,   589,   597,    88,   596,   601,    89,   239,
     602,    90,    85,   607,   608,    76,   613,   155,    67,    68,
      69,   612,   614,    70,    79,    80,    86,   618,    72,    81,
      88,   619,   620,    89,   585,   219,    90,   156,    74,   157,
     155,    67,    68,    69,   340,   416,    70,   469,   382,   270,
     617,    72,   578,   622,   600,   583,   361,    76,   410,   468,
     372,    74,   370,   512,   560,   401,    79,    80,   436,   118,
       0,    81,   494,   105,   245,     0,     0,    85,     0,   156,
      76,   157,     0,     0,     0,   106,   107,     0,     0,    79,
      80,    86,   108,     0,    81,    88,     0,     0,    89,   170,
       0,    90,   156,     0,   157,   109,     0,     0,     0,     0,
       0,   110,     0,     0,     0,     0,     0,     0,     0,    85,
       0,   339,     0,   111,     0,   170,     0,   171,     0,     0,
       0,   112,   113,    86,     0,     0,     0,    88,     0,     0,
      89,     0,    85,    90,   103,     0,     0,     0,   114,     0,
       0,     0,   115,   171,   170,     0,    86,     0,     0,     0,
      88,   116,     0,    89,     0,     0,    90,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   171,   170,     0,     0,     0,     0,     0,     0,
       0,     0,   345,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,     0,   170,
       0,   171,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   171,   170,     0,
       0,   356,   357,     0,     0,     0,     0,     0,     0,     0,
       0,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   171,   170,     0,     0,
       0,   355,     0,     0,     0,     0,     0,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,     0,   170,     0,   171,     0,   357,     0,     0,
       0,     0,     0,     0,     0,     0,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   171,   170,     0,     0,   264,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     171,   170,     0,     0,   346,     0,     0,     0,     0,     0,
       0,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,     0,   170,     0,   171,
     351,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   171,     0,     0,     0,   358,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,     0,     0,     0,     0,   466,     0,
       0,     0,     0,     0,     0,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
       1,     0,     0,     2,   467,     3,     0,     0,     0,     0,
       4,     0,     0,     0,     5,     0,     0,     6,   170,     7,
       8,     9,     0,   341,    10,     0,     0,    11,     0,     0,
       0,     0,     0,   170,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,     0,     0,   171,   170,     0,     0,
       0,     0,     0,     0,     0,     0,    13,   170,     0,     0,
       0,   171,     0,    14,     0,    15,     0,    16,     0,    17,
       0,    18,     0,    19,     0,   171,     0,     0,     0,     0,
       0,     0,    20,     0,    21,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     170,   178,   179,   180,   181,   182,   183,   184,   185,   186,
       0,     0,   170,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   170,     0,     0,     0,     0,   171,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     171,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   171,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186
};

static const short yycheck[] =
{
     129,   371,   331,   320,   111,     3,   380,   114,   115,   383,
       5,     3,     4,     3,    40,     5,     9,    16,     8,   126,
       9,     5,    13,    14,     8,     3,    94,    46,     5,    39,
       3,    32,     5,     5,     5,     6,     3,     8,    13,    14,
     215,    33,    68,    14,     5,     6,    86,     8,    24,   366,
      13,    14,   128,    14,   130,   109,     5,     6,    50,     8,
      52,   190,   418,   433,     3,    14,     3,    43,    53,    13,
      14,    54,    64,   127,     3,     5,     6,    46,     3,     8,
      56,    18,   577,     8,    14,    63,    53,    86,   128,    88,
      89,   461,   118,   119,   120,   121,   122,   106,   205,   100,
     474,   418,   597,   477,   478,   479,   480,   481,   482,   128,
     120,     5,     6,   105,     8,    70,    92,   473,   447,   128,
      14,   112,     5,     6,   119,    47,    63,   112,   126,   446,
     459,    14,   127,   126,   126,   119,   126,   126,   267,   138,
     139,   132,   119,   127,   143,   320,   119,   119,   119,   148,
     127,   126,   151,   152,   153,   126,   473,   132,   119,   128,
      46,   531,    91,   126,     3,   126,     5,   166,   123,   132,
     119,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   132,   119,
      62,   366,    77,     0,   568,   569,   126,    21,    83,   306,
     307,    19,   331,   573,     3,     3,   381,     5,    84,   106,
       8,    10,    88,    52,    40,    97,   106,    47,   217,    37,
       3,     4,    24,    25,    83,   119,     3,     4,     3,    28,
     559,   128,   549,    40,   106,    54,   119,    61,   128,   238,
       0,    43,    68,   418,    42,     3,    78,    79,   125,    48,
      33,   380,     3,    85,   383,    57,    33,     3,     9,     3,
      18,    68,   261,   262,   263,     5,     6,    50,   397,    52,
      69,   446,    70,    50,    58,    52,   127,   128,   407,   596,
       3,   144,   126,   146,     3,   414,   149,   150,    87,     3,
     465,   127,   128,    76,   120,   121,   122,     3,   473,     3,
      98,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   127,   128,   447,   127,
     128,   119,   105,   127,   128,     3,   127,   128,   105,     3,
     459,     5,     6,   126,     8,   131,     3,     4,     5,     6,
     339,   126,     9,    10,   126,   474,   126,    14,   477,   478,
     479,   480,   481,   482,    21,   126,   355,    24,   357,   127,
     128,     3,     4,     5,     6,   126,    33,     9,   127,   128,
     127,   128,    14,   126,   549,    63,    43,   127,   128,    21,
     127,   128,    24,    50,    51,    52,    53,   126,   126,   564,
      57,    33,   127,   128,   127,   128,    70,   126,    65,   126,
      67,    43,   577,   127,   128,   127,   128,   112,    50,    51,
      52,    53,   191,   192,   126,    57,   415,   112,    86,    62,
      97,   596,   597,    65,    86,    67,    93,     3,   103,     3,
     559,     3,   607,    15,    81,    96,     3,   127,   105,   568,
     569,   127,   126,     3,     4,     5,     6,   126,   112,     9,
       3,    93,   119,   120,    14,     5,   123,   112,   112,   126,
     119,    21,   129,   105,    24,    60,    91,    91,     8,    91,
       3,     3,    30,    33,   126,    83,   133,   119,   120,   128,
       3,   123,    52,    43,   126,   614,   127,   129,   128,     5,
      50,    51,    52,    53,   126,    61,     8,    57,     3,   112,
     102,     3,    23,     3,   127,    65,    89,    67,     3,     4,
       5,     6,   127,     8,     9,     3,   127,   108,   112,    14,
      84,    49,     5,   112,    71,     3,     4,     5,     6,    24,
       8,     9,    53,    93,   126,     8,    14,     8,    59,   126,
     126,   126,    53,     5,   128,   105,    24,    16,    43,   126,
       3,     4,     5,     6,   127,     8,     9,    52,    53,   119,
     120,    14,    57,   123,   126,    43,   126,   128,   109,   129,
      65,    24,    67,   107,    52,    53,     8,    78,     3,    57,
     128,   120,   127,   127,   127,    54,     3,    65,    72,    67,
      43,   112,   113,   114,   115,   116,   117,    66,     8,    52,
      53,    53,   123,    26,    57,    74,    75,    49,    72,   112,
     105,   128,    65,   127,    67,     3,     4,     5,     6,    18,
       8,     9,    91,   127,   119,    26,    14,   105,   123,   128,
      63,   126,   127,   127,   129,   104,    24,   109,    72,   109,
      70,   119,    26,     5,    26,   123,   128,   126,   126,   127,
       3,   129,   105,   128,   128,    43,     8,     3,     4,     5,
       6,   126,   112,     9,    52,    53,   119,     5,    14,    57,
     123,   127,   127,   126,   562,   124,   129,    65,    24,    67,
       3,     4,     5,     6,   238,   365,     9,   415,   316,   166,
     607,    14,   552,   616,   578,   556,   265,    43,   343,   414,
     304,    24,   302,   459,   507,   331,    52,    53,   381,    18,
      -1,    57,   432,    16,   140,    -1,    -1,   105,    -1,    65,
      43,    67,    -1,    -1,    -1,    28,    29,    -1,    -1,    52,
      53,   119,    35,    -1,    57,   123,    -1,    -1,   126,    40,
      -1,   129,    65,    -1,    67,    48,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
      -1,    84,    -1,    66,    -1,    40,    -1,    68,    -1,    -1,
      -1,    74,    75,   119,    -1,    -1,    -1,   123,    -1,    -1,
     126,    -1,   105,   129,    87,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    95,    68,    40,    -1,   119,    -1,    -1,    -1,
     123,   104,    -1,   126,    -1,    -1,   129,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    68,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    40,
      -1,    68,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    68,    40,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    68,    40,    -1,    -1,
      -1,   128,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    40,    -1,    68,    -1,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    68,    40,    -1,    -1,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      68,    40,    -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    40,    -1,    68,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    68,    -1,    -1,    -1,   127,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      17,    -1,    -1,    20,   127,    22,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    31,    -1,    -1,    34,    40,    36,
      37,    38,    -1,    45,    41,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    40,    -1,    -1,
      -1,    68,    -1,    80,    -1,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,   101,    68,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      40,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      41,    44,    55,    73,    80,    82,    84,    86,    88,    90,
      99,   101,   135,   136,   137,   138,   139,   140,   147,   186,
     191,   192,   195,   196,   197,   198,   205,   206,   215,   216,
     217,   218,   219,   223,   224,   229,   230,   231,   234,   236,
     237,   238,   239,   240,   242,   243,   244,    94,    54,     3,
      47,    46,    47,    77,    83,    54,     3,     4,     5,     6,
       9,    10,    14,    21,    24,    33,    43,    50,    51,    52,
      53,    57,    65,    67,    93,   105,   119,   120,   123,   126,
     129,   148,   149,   151,   179,   180,   241,   245,     3,    10,
      28,    48,    69,    87,   232,    16,    28,    29,    35,    48,
      54,    66,    74,    75,    91,    95,   104,   188,   232,    97,
      83,     3,   152,     0,   125,    58,     3,     3,     3,   126,
       3,   152,     3,     3,     3,     3,    13,    14,   126,   132,
     246,   247,   131,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,     3,    65,    67,   179,   179,
     179,     3,    53,   183,   184,    46,   128,     3,    18,   150,
      40,    68,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    63,   171,   172,
     112,   112,    86,     3,     9,     3,     5,     6,     8,    70,
     189,   190,    97,     3,     8,    91,    86,    62,   187,     3,
     187,   187,   187,   103,     3,    86,   128,    84,    88,   138,
       3,   187,    15,    96,     5,     6,     8,    14,   119,   126,
     159,   160,   204,   207,   208,    81,   106,     8,   126,   127,
     179,   181,   182,     8,   179,   247,     3,   179,   179,   181,
      39,   120,   181,   127,   179,   181,   181,   179,   179,   179,
     127,   126,   126,   126,   127,   128,   130,   112,   126,   152,
     149,   179,     3,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
       5,     3,     8,    42,    70,    98,   119,   159,   193,   194,
     190,   190,   112,   112,   119,    60,    91,    91,   187,     8,
      91,    62,   106,   225,   226,     3,   220,   221,   245,     3,
     126,   199,    30,    83,     5,     6,     8,   127,   159,   161,
     209,   128,   210,    24,    43,    56,    92,   235,    52,    84,
     140,    45,   127,   128,   133,   133,   127,   127,     3,   127,
     127,   127,   127,   127,   127,   128,   127,   128,   127,   179,
     179,   184,     3,   159,   185,   144,   106,   153,   154,   128,
     193,   126,   189,    61,   187,   187,     8,     3,   227,   228,
     112,   128,   154,   112,     3,     4,    33,    50,    52,   105,
     158,   200,   245,   102,     3,     3,   127,   128,   127,   128,
     204,   208,   211,   212,   127,    89,    53,   112,   127,     3,
     182,   127,   179,   179,   112,    84,   147,    64,   126,   155,
     156,   157,   158,    49,   162,     5,   161,    78,    79,    85,
     233,   112,   108,   126,   159,   160,   221,    71,   173,   174,
     159,   160,   126,   126,   126,   127,   128,   126,   201,   202,
      24,    25,    43,    57,   222,   159,     8,    18,   213,   128,
       8,   126,   159,   128,   141,    53,   127,   127,   185,   148,
     127,   126,   155,   109,    23,    53,    59,   112,   113,   114,
     115,   116,   117,   123,     5,   163,   107,   165,   166,   127,
      32,   100,    78,     8,   228,   127,   161,     3,   175,   176,
     120,   127,   127,   158,   126,   203,   204,   128,     3,    63,
     214,   204,   212,   161,     3,     5,    52,   142,   143,   127,
       3,   245,    46,    72,   145,     8,   127,   155,   159,   160,
       9,   126,    70,   123,     8,   159,   160,     8,   159,   160,
     159,   160,   159,   160,   159,   160,   159,   160,    53,    26,
      49,    72,   167,   168,   127,   112,   128,   127,   127,   128,
     202,   127,   128,   127,    26,    63,   146,   127,   109,   109,
     161,    70,     9,   126,   158,   164,    72,    26,   171,     3,
       5,     8,   126,   176,   204,   143,   158,   169,   170,     5,
     159,   160,   159,   160,   127,   161,   128,    26,    76,   169,
     173,   126,     3,   177,   178,    19,    37,   128,   128,   127,
     158,   169,   126,     8,   112,   127,   128,   170,     5,   127,
     127,   159,   178
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

  case 29:

    { pParser->PushQuery(); ;}
    break;

  case 30:

    { pParser->PushQuery(); ;}
    break;

  case 34:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 36:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 39:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 40:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 44:

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

  case 45:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 47:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 48:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 49:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-6] );
		;}
    break;

  case 52:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 55:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 56:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 59:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 60:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 61:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 62:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 63:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 64:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 65:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 67:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 74:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 75:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 76:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 77:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 78:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 79:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 95:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 96:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 97:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 98:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 101:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 102:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 103:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 104:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 105:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 107:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 108:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 109:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 110:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 111:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 112:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 113:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 117:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 118:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 119:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 122:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 125:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 126:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 128:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 130:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 131:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 134:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 135:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 144:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 147:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 148:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 150:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 151:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 156:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 157:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 179:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 180:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 181:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 182:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 183:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 186:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 187:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 188:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 193:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 194:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 201:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 205:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 206:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 207:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 208:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 209:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 210:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 218:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 219:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 220:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 221:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 222:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 223:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 224:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 225:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 228:

    { yyval.m_iValue = 1; ;}
    break;

  case 229:

    { yyval.m_iValue = 0; ;}
    break;

  case 230:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 231:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 232:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 233:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 236:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 237:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 238:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 241:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 242:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 245:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 246:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 247:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 248:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 249:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 250:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 251:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 252:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 253:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 254:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 255:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 256:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 257:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 259:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 260:

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

  case 261:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 264:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 266:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 271:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 275:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 276:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 279:

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

  case 280:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 281:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 282:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 283:

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

  case 284:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 285:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 286:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 287:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 288:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 289:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 290:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 291:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 298:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 299:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 300:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 308:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 309:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 310:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 311:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 312:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 313:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 314:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 315:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 316:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 317:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 320:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 321:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 322:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 323:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 325:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 326:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 327:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 328:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 329:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

