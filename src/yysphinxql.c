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
#define YYLAST   1353

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  134
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  114
/* YYNRULES -- Number of rules. */
#define YYNRULES  330
/* YYNRULES -- Number of states. */
#define YYNSTATES  630

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
     646,   648,   652,   654,   656,   660,   666,   668,   670,   672,
     674,   677,   678,   681,   683,   686,   689,   693,   695,   697,
     702,   707,   711,   713,   715,   717,   719,   721,   723,   727,
     732,   737,   742,   746,   751,   756,   764,   770,   772,   774,
     776,   778,   780,   782,   784,   786,   788,   791,   798,   800,
     802,   803,   807,   809,   813,   815,   819,   823,   825,   829,
     831,   833,   835,   839,   842,   850,   860,   867,   869,   873,
     875,   879,   881,   885,   886,   889,   891,   895,   899,   900,
     902,   904,   906,   910,   912,   914,   918,   922,   929,   931,
     935,   939,   943,   949,   954,   958,   962,   964,   966,   968,
     970,   978,   983,   989,   990,   992,   995,   997,  1001,  1005,
    1008,  1012,  1019,  1020,  1022,  1024,  1027,  1030,  1033,  1035,
    1043,  1045,  1047,  1049,  1051,  1055,  1062,  1066,  1070,  1074,
    1076,  1080,  1083,  1087,  1091,  1094,  1096,  1099,  1101,  1103,
    1107
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
       3,    53,   245,   127,    -1,     3,   126,   179,   112,     8,
      45,     3,    53,   245,   127,    -1,   182,    -1,   181,   128,
     182,    -1,   179,    -1,     8,    -1,   184,   112,   185,    -1,
     183,   128,   184,   112,   185,    -1,     3,    -1,    53,    -1,
     159,    -1,     3,    -1,    88,   188,    -1,    -1,    62,     8,
      -1,   104,    -1,    91,   187,    -1,    66,   187,    -1,    16,
      91,   187,    -1,    75,    -1,    74,    -1,    16,     8,    91,
     187,    -1,    16,     3,    91,   187,    -1,    54,     3,    91,
      -1,     3,    -1,    70,    -1,     8,    -1,     5,    -1,     6,
      -1,   189,    -1,   190,   119,   189,    -1,    86,     3,   112,
     194,    -1,    86,     3,   112,   193,    -1,    86,     3,   112,
      70,    -1,    86,    69,   190,    -1,    86,    10,   112,   190,
      -1,    86,    28,    86,   190,    -1,    86,    48,     9,   112,
     126,   161,   127,    -1,    86,    48,     3,   112,   193,    -1,
       3,    -1,     8,    -1,    98,    -1,    42,    -1,   159,    -1,
      31,    -1,    82,    -1,   196,    -1,    22,    -1,    90,    97,
      -1,   198,    58,     3,   199,   102,   201,    -1,    55,    -1,
      80,    -1,    -1,   126,   200,   127,    -1,   158,    -1,   200,
     128,   158,    -1,   202,    -1,   201,   128,   202,    -1,   126,
     203,   127,    -1,   204,    -1,   203,   128,   204,    -1,   159,
      -1,   160,    -1,     8,    -1,   126,   161,   127,    -1,   126,
     127,    -1,    36,    46,   152,   106,    52,   112,   159,    -1,
      36,    46,   152,   106,    52,    53,   126,   161,   127,    -1,
      27,     3,   126,   207,   210,   127,    -1,   208,    -1,   207,
     128,   208,    -1,   204,    -1,   126,   209,   127,    -1,     8,
      -1,   209,   128,     8,    -1,    -1,   128,   211,    -1,   212,
      -1,   211,   128,   212,    -1,   204,   213,   214,    -1,    -1,
      18,    -1,     3,    -1,    63,    -1,   216,     3,   187,    -1,
      38,    -1,    37,    -1,    88,    95,   187,    -1,    88,    35,
     187,    -1,   101,   152,    86,   220,   154,   173,    -1,   221,
      -1,   220,   128,   221,    -1,     3,   112,   159,    -1,     3,
     112,   160,    -1,     3,   112,   126,   161,   127,    -1,     3,
     112,   126,   127,    -1,   245,   112,   159,    -1,   245,   112,
     160,    -1,    57,    -1,    24,    -1,    43,    -1,    25,    -1,
      17,    94,     3,    15,    30,     3,   222,    -1,    88,   232,
     103,   225,    -1,    88,   232,   103,    62,     8,    -1,    -1,
     226,    -1,   106,   227,    -1,   228,    -1,   227,   108,   228,
      -1,     3,   112,     8,    -1,    88,    29,    -1,    88,    28,
      86,    -1,    86,   232,    97,    60,    61,   233,    -1,    -1,
      48,    -1,    87,    -1,    78,   100,    -1,    78,    32,    -1,
      79,    78,    -1,    85,    -1,    34,    47,     3,    81,   235,
      89,     8,    -1,    56,    -1,    24,    -1,    43,    -1,    92,
      -1,    41,    47,     3,    -1,    20,    54,     3,    96,    83,
       3,    -1,    44,    83,     3,    -1,    44,    77,     3,    -1,
      84,   241,   171,    -1,    10,    -1,    10,   131,     3,    -1,
      84,   179,    -1,    99,    83,     3,    -1,    73,    54,     3,
      -1,     3,   246,    -1,   247,    -1,   246,   247,    -1,    13,
      -1,    14,    -1,   132,   179,   133,    -1,   132,     8,   133,
      -1
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
     739,   740,   741,   742,   743,   744,   745,   746,   747,   748,
     752,   753,   757,   758,   762,   763,   767,   768,   772,   773,
     779,   782,   784,   788,   789,   790,   791,   792,   793,   794,
     799,   804,   814,   815,   816,   817,   818,   822,   823,   827,
     832,   837,   842,   843,   844,   848,   853,   861,   862,   866,
     867,   868,   882,   883,   884,   888,   889,   895,   903,   904,
     907,   909,   913,   914,   918,   919,   923,   927,   928,   932,
     933,   934,   935,   936,   942,   950,   964,   972,   976,   983,
     984,   991,  1001,  1007,  1009,  1013,  1018,  1022,  1029,  1031,
    1035,  1036,  1042,  1050,  1051,  1057,  1061,  1067,  1075,  1076,
    1080,  1094,  1100,  1104,  1109,  1123,  1134,  1135,  1136,  1137,
    1141,  1154,  1158,  1165,  1166,  1170,  1174,  1175,  1179,  1183,
    1190,  1197,  1203,  1204,  1205,  1209,  1210,  1211,  1212,  1218,
    1229,  1230,  1231,  1232,  1236,  1247,  1259,  1268,  1279,  1287,
    1288,  1292,  1302,  1313,  1324,  1327,  1328,  1332,  1333,  1334,
    1335
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
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     181,   181,   182,   182,   183,   183,   184,   184,   185,   185,
     186,   187,   187,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   189,   189,   189,   189,   189,   190,   190,   191,
     191,   191,   191,   191,   191,   192,   192,   193,   193,   194,
     194,   194,   195,   195,   195,   196,   196,   197,   198,   198,
     199,   199,   200,   200,   201,   201,   202,   203,   203,   204,
     204,   204,   204,   204,   205,   205,   206,   207,   207,   208,
     208,   209,   209,   210,   210,   211,   211,   212,   213,   213,
     214,   214,   215,   216,   216,   217,   218,   219,   220,   220,
     221,   221,   221,   221,   221,   221,   222,   222,   222,   222,
     223,   224,   224,   225,   225,   226,   227,   227,   228,   229,
     230,   231,   232,   232,   232,   233,   233,   233,   233,   234,
     235,   235,   235,   235,   236,   237,   238,   239,   240,   241,
     241,   242,   243,   244,   245,   246,   246,   247,   247,   247,
     247
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
       4,     4,     4,     4,     3,     6,     6,     3,     8,    10,
       1,     3,     1,     1,     3,     5,     1,     1,     1,     1,
       2,     0,     2,     1,     2,     2,     3,     1,     1,     4,
       4,     3,     1,     1,     1,     1,     1,     1,     3,     4,
       4,     4,     3,     4,     4,     7,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     6,     1,     1,
       0,     3,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     3,     2,     7,     9,     6,     1,     3,     1,
       3,     1,     3,     0,     2,     1,     3,     3,     0,     1,
       1,     1,     3,     1,     1,     3,     3,     6,     1,     3,
       3,     3,     5,     4,     3,     3,     1,     1,     1,     1,
       7,     4,     5,     0,     1,     2,     1,     3,     3,     2,
       3,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     1,     1,     3,     6,     3,     3,     3,     1,
       3,     2,     3,     3,     2,     1,     2,     1,     1,     3,
       3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   235,     0,   232,     0,     0,   274,   273,
       0,     0,   238,     0,   239,   233,     0,   302,   302,     0,
       0,     0,     0,     2,     3,    29,    31,    33,    35,    32,
       7,     8,     9,   234,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   149,   150,   152,   153,
     155,   319,   154,     0,     0,     0,     0,     0,     0,   151,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,    50,    54,    57,   177,   132,   178,     0,     0,
       0,   303,     0,   304,     0,     0,     0,   299,   201,   303,
       0,   201,   208,   207,   201,   201,   203,   200,     0,   236,
       0,    66,     0,     1,     4,     0,   201,     0,     0,     0,
       0,     0,   314,   317,   316,   323,   327,   328,     0,     0,
     324,   325,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   149,     0,     0,   156,   157,
       0,   196,   197,     0,     0,     0,     0,    55,     0,    53,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   318,   133,
       0,     0,     0,     0,     0,   212,   215,   216,   214,   213,
     217,   222,     0,     0,     0,   201,   300,     0,   276,     0,
     205,   204,   275,   293,   322,     0,     0,     0,     0,    30,
     240,   272,     0,     0,   107,   109,   251,   111,     0,     0,
     249,   250,   259,   263,   257,     0,     0,   193,     0,   184,
     192,     0,   190,     0,     0,   326,   320,     0,   192,     0,
       0,     0,     0,    64,     0,     0,     0,     0,     0,     0,
     187,     0,     0,     0,   175,     0,   176,     0,    44,    68,
      51,    57,    56,   167,   168,   174,   173,   165,   164,   171,
     172,   162,   163,   170,   169,   158,   159,   160,   161,   166,
     134,   227,   228,   230,   221,   229,     0,   231,   220,   219,
     223,   224,     0,     0,     0,     0,   201,   201,   206,   202,
     211,     0,     0,   291,   294,     0,     0,   278,     0,    67,
       0,     0,     0,     0,   108,   110,   261,   253,   112,     0,
       0,     0,     0,   311,   312,   310,   313,     0,     0,     0,
       0,     0,     0,   179,     0,   330,   329,    58,   182,     0,
      63,   183,    62,   180,   181,    59,     0,    60,     0,    61,
       0,     0,     0,   199,   198,   194,     0,     0,   114,    69,
       0,   226,     0,   218,     0,   210,   209,   292,     0,   295,
     296,     0,     0,   136,     0,   100,   101,     0,     0,   105,
       0,   242,     0,   106,     0,     0,   315,   252,     0,   260,
       0,   259,   258,   264,   265,   256,     0,     0,     0,    37,
       0,     0,   191,    65,     0,     0,     0,     0,     0,     0,
       0,    70,    71,    87,     0,   116,   120,   135,     0,     0,
       0,   308,   301,     0,     0,     0,   280,   281,   279,     0,
     277,   137,   284,   285,     0,     0,     0,   241,     0,     0,
     237,   244,   287,   289,   288,   286,   290,   113,   262,   269,
       0,     0,   309,     0,   254,     0,     0,     0,     0,   186,
     185,   195,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   117,     0,     0,
     123,   121,   225,   306,   305,   307,   298,   297,   283,     0,
       0,   138,   139,     0,   103,   104,   243,     0,     0,   247,
       0,   270,   271,   267,   268,   266,     0,    41,    42,    43,
      38,    39,    34,     0,     0,     0,     0,     0,    46,     0,
      73,    72,     0,     0,    79,     0,    97,     0,    95,    75,
      86,    96,    76,    99,    83,    92,    82,    91,    84,    93,
      85,    94,     0,     0,     0,     0,   132,   124,   282,     0,
       0,   102,   246,     0,   245,   255,     0,   188,     0,     0,
       0,    36,    74,     0,     0,     0,    98,    80,     0,   118,
     115,     0,     0,   136,   141,   142,   145,     0,   140,   248,
      40,     0,   129,    45,   127,    47,    81,    89,    90,    88,
      77,     0,     0,     0,     0,   125,    49,     0,     0,     0,
     146,   189,   130,   131,     0,     0,    78,   119,   122,     0,
       0,     0,   143,     0,   128,    48,   126,   144,   148,   147
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   466,   520,   521,
     366,   528,   571,    28,    91,    92,   169,    93,   269,   368,
     369,   421,   422,   423,   592,   328,   231,   329,   426,   488,
     580,   490,   491,   556,   557,   593,   594,   188,   189,   440,
     441,   501,   502,   609,   610,   248,    95,   241,   242,   163,
     164,   365,    29,   208,   117,   200,   201,    30,    31,   298,
     299,    32,    33,    34,    35,   321,   392,   450,   451,   508,
     232,    36,    37,   233,   234,   330,   332,   403,   404,   460,
     513,    38,    39,    40,    41,    42,   316,   317,   456,    43,
      44,   313,   314,   379,   380,    45,    46,    47,   104,   432,
      48,   337,    49,    50,    51,    52,    53,    96,    54,    55,
      56,    97,   140,   141
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -481
static const short yypact[] =
{
    1169,   -69,    -1,  -481,    42,  -481,    58,    68,  -481,  -481,
      70,   137,  -481,   101,  -481,  -481,   307,   437,   927,    98,
     136,   227,   260,  -481,    92,  -481,  -481,  -481,  -481,  -481,
    -481,  -481,  -481,  -481,  -481,   213,  -481,  -481,  -481,   237,
    -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,
    -481,  -481,  -481,  -481,  -481,  -481,  -481,   274,   277,   156,
     290,   227,   295,   305,   311,   323,    50,  -481,  -481,  -481,
    -481,   204,  -481,   211,   221,   223,   254,   257,   263,  -481,
     265,   270,   275,   278,   280,   287,   675,  -481,   675,   675,
      46,   -30,  -481,   178,   652,  -481,   276,  -481,   316,   320,
     279,   239,   376,  -481,   338,   117,   352,  -481,   386,  -481,
     451,   386,  -481,  -481,   386,   386,  -481,  -481,   353,  -481,
     463,  -481,   -15,  -481,   143,   465,   386,   456,   384,   110,
     391,   -82,  -481,  -481,  -481,  -481,  -481,  -481,   180,    27,
      45,  -481,   478,   675,   528,   -29,   528,   359,   675,   528,
     528,   675,   675,   675,   361,   131,   356,   368,  -481,  -481,
     924,  -481,  -481,    82,   383,    11,   364,  -481,   497,  -481,
     675,   675,   675,   675,   675,   675,   675,   675,   675,   675,
     675,   675,   675,   675,   675,   675,   675,   496,  -481,  -481,
     273,   376,   376,   390,   392,  -481,  -481,  -481,  -481,  -481,
    -481,   388,   445,   417,   418,   386,  -481,   502,  -481,   421,
    -481,  -481,  -481,   -18,  -481,   515,   518,   446,   387,  -481,
     396,  -481,   493,   443,  -481,  -481,  -481,  -481,   253,    60,
    -481,  -481,  -481,   399,  -481,   182,   476,  -481,   593,  -481,
    1119,   140,  -481,   397,   708,  -481,  -481,   944,  1175,   173,
     526,   411,   177,  -481,   979,   191,   195,   735,   765,  1008,
    -481,   511,   675,   675,  -481,    46,  -481,    90,  -481,    -9,
    -481,  1175,  -481,  -481,  -481,  1204,   671,  1231,   588,  1186,
    1186,   355,   355,   355,   355,   298,   298,  -481,  -481,  -481,
     412,  -481,  -481,  -481,  -481,  -481,   536,  -481,  -481,  -481,
     388,   388,    54,   419,   376,   482,   386,   386,  -481,  -481,
    -481,   538,   541,  -481,  -481,   134,    85,  -481,   435,  -481,
     240,   447,   545,   547,  -481,  -481,  -481,  -481,  -481,   197,
     206,   110,   426,  -481,  -481,  -481,  -481,   466,   -26,   364,
     429,   554,   618,  -481,   528,  -481,  -481,  -481,  -481,   431,
    -481,  -481,  -481,  -481,  -481,  -481,   675,  -481,   675,  -481,
     791,   820,   448,  -481,  -481,  -481,   475,    17,   512,  -481,
     557,  -481,    61,  -481,   217,  -481,  -481,  -481,   455,   462,
    -481,    23,   515,   503,    33,    45,  -481,   453,   457,  -481,
     458,  -481,   214,  -481,   460,   229,  -481,  -481,    61,  -481,
     569,    37,  -481,   454,  -481,  -481,   579,   464,    61,   461,
     535,   546,  -481,  -481,  1034,  1063,    90,   364,   467,   474,
      17,   483,  -481,  -481,   550,   596,   498,  -481,   235,   -13,
     530,  -481,  -481,   598,   541,    80,  -481,  -481,  -481,   601,
    -481,  -481,  -481,  -481,   490,   484,   485,  -481,   240,   210,
     486,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,
      10,   210,  -481,    61,  -481,    51,   488,   610,   615,  -481,
    -481,  -481,     6,   548,   611,   -67,    17,    33,     9,   -48,
     256,   283,    33,    33,    33,    33,   572,  -481,   603,   582,
     563,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,   259,
     527,   513,  -481,   516,  -481,  -481,  -481,    84,   266,  -481,
     460,  -481,  -481,  -481,   626,  -481,   271,  -481,  -481,  -481,
     520,  -481,  -481,    45,   522,   600,   227,   629,   605,   532,
    -481,  -481,   560,   565,  -481,    61,  -481,   602,  -481,  -481,
    -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,
    -481,  -481,    25,   240,   604,   656,   276,  -481,  -481,     4,
     601,  -481,  -481,   210,  -481,  -481,    51,  -481,   610,   240,
     681,  -481,  -481,    33,    33,   281,  -481,  -481,    61,  -481,
     559,   662,   189,   503,   564,  -481,  -481,   688,  -481,  -481,
    -481,   566,    53,   567,  -481,   568,  -481,  -481,  -481,  -481,
    -481,   297,   240,   240,   571,   567,  -481,   686,   609,   331,
    -481,  -481,  -481,  -481,   240,   709,  -481,  -481,   567,   586,
     590,    61,  -481,   688,  -481,  -481,  -481,  -481,  -481,  -481
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -481,  -481,  -481,  -481,   591,  -481,   487,  -481,  -481,   158,
    -481,  -481,  -481,   360,   312,   570,  -481,  -481,    22,  -481,
     414,  -394,  -481,  -481,  -319,  -129,  -373,  -357,  -481,  -481,
    -481,  -481,  -481,  -481,  -481,  -480,   119,   175,  -481,   151,
    -481,  -481,   183,  -481,   112,   -12,  -481,   293,   394,  -481,
     480,   330,  -481,  -109,  -481,   449,   272,  -481,  -481,   450,
    -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,   241,  -481,
    -328,  -481,  -481,  -481,   423,  -481,  -481,  -481,   288,  -481,
    -481,  -481,  -481,  -481,  -481,  -481,  -481,   373,  -481,  -481,
    -481,  -481,  -481,  -481,   322,  -481,  -481,  -481,   732,  -481,
    -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,  -481,
    -481,  -192,  -481,   617
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -322
static const short yytable[] =
{
     230,   391,   210,   401,    94,   211,   212,   584,   437,   585,
     250,   443,   586,   511,   121,   428,   165,   221,   534,   493,
     385,   386,   536,   318,   236,    57,   475,   407,   224,   225,
     155,    67,    68,    69,   577,   243,    70,   227,   224,   225,
    -268,    72,   476,   122,   311,    59,   216,   227,   424,   161,
     387,    74,   526,    58,   517,   459,   518,   291,   136,   137,
     530,   297,   292,   136,   137,   224,   224,   388,   326,   389,
      76,   215,   612,   512,   158,   537,   159,   160,   499,    79,
      80,   419,   531,   131,    81,   224,   408,   494,   312,   224,
     613,   251,   156,   363,   157,   224,   308,   367,   166,   162,
    -268,   424,   605,   519,   533,    60,   516,   540,   543,   545,
     547,   549,   551,   216,    61,   224,   225,    62,   226,   216,
     203,   509,   390,   618,   227,   204,   240,   244,   393,   506,
     587,   247,    85,   514,   166,   535,   254,   268,   364,   257,
     258,   259,   228,   420,   136,   137,    86,   136,   137,   435,
      88,   578,   228,    89,   271,    65,    90,   424,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   393,   138,   139,   575,   296,
     296,   167,   139,   155,    67,    68,    69,   327,   237,    70,
     318,   367,   385,   386,    72,   119,   168,   375,   376,   296,
     597,   599,   230,   296,    74,   271,   333,   498,   205,   296,
     265,   327,   266,   382,    63,   224,   225,   124,   226,   120,
      64,   601,   387,    76,   227,   334,   160,   217,   393,   228,
     121,   218,    79,    80,   579,   589,   229,    81,   335,   388,
     126,   389,   193,   385,   386,   156,   381,   157,   194,   240,
     360,   361,   436,   452,   453,   442,   393,   261,   324,   325,
     123,   224,   225,   139,   538,   604,   139,   343,   344,   457,
     227,   125,   454,   387,   336,   524,   291,   127,   224,   464,
     128,   292,   129,   617,   393,    85,   455,   364,   224,   225,
     388,   541,   389,   130,   390,   429,   430,   227,   132,    86,
     348,   344,   431,    88,   351,   344,   238,   239,   133,    90,
      66,    67,    68,    69,   134,   293,    70,    71,   353,   344,
     230,    72,   354,   344,   397,   398,   135,   271,    73,   228,
     279,    74,   230,   399,   400,   142,   507,   143,   170,   187,
      75,   447,   448,   294,   414,   390,   415,   144,   532,   145,
      76,   539,   542,   544,   546,   548,   550,    77,    78,    79,
      80,   393,   492,   398,    81,   192,   171,   155,    67,    68,
      69,   295,    82,    70,    83,   228,   591,   393,    72,   195,
     146,   196,   197,   147,   198,    73,   558,   398,    74,   148,
     393,   149,   296,   562,   563,   170,   150,    75,   565,   398,
      84,   151,   228,   105,   152,   271,   153,    76,   600,   398,
     393,   393,    85,   154,    77,    78,    79,    80,   184,   185,
     186,    81,   393,   171,   616,   398,    86,    87,   190,    82,
      88,    83,   191,    89,   230,   202,    90,   249,   206,   252,
      98,   110,   255,   256,   596,   598,   199,    99,   207,    66,
      67,    68,    69,   111,   209,    70,   213,    84,   622,   623,
      72,   112,   113,   300,   301,   100,   214,    73,   220,    85,
      74,   222,   235,   182,   183,   184,   185,   186,   114,    75,
     223,   246,   262,    86,    87,   101,   253,    88,   260,    76,
      89,   116,   628,    90,   263,   267,    77,    78,    79,    80,
     272,   290,   302,    81,   303,   305,   102,   304,   306,   307,
     309,    82,   310,    83,   155,    67,    68,    69,   315,   237,
      70,   319,   320,   322,   103,    72,   323,   331,   338,   349,
     345,   155,    67,    68,    69,    74,   237,    70,   350,    84,
     370,   324,    72,   374,   378,   372,   377,   384,   395,   394,
     396,    85,    74,   405,    76,   406,   409,   410,   413,   417,
     416,   425,   427,    79,    80,    86,    87,   433,    81,    88,
     434,    76,    89,   477,   439,    90,   156,   458,   157,   444,
      79,    80,   461,   445,   446,    81,   449,   462,   467,   465,
     463,   468,   476,   156,   473,   157,   155,    67,    68,    69,
     474,   487,    70,   478,   500,   489,   496,    72,   495,   479,
     503,   504,   505,   523,   510,   522,    85,    74,   525,   529,
     527,   155,    67,    68,    69,   552,   411,    70,   170,   553,
      86,   554,    72,    85,    88,   555,    76,    89,   239,   559,
      90,   560,    74,   561,   459,    79,    80,    86,   566,   567,
      81,    88,  -321,   568,    89,   569,   171,    90,   156,   572,
     157,    76,   480,   481,   482,   483,   484,   485,   570,   573,
      79,    80,   576,   486,   574,    81,   581,   339,   155,    67,
      68,    69,   582,   156,    70,   157,   595,   602,   603,    72,
     607,   608,   170,   611,   620,   614,   615,   619,    85,    74,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   170,    86,   626,   625,   219,    88,   627,    76,    89,
     171,   621,    90,    85,   590,   340,   418,    79,    80,   472,
     383,   583,    81,   624,   606,   629,   270,    86,   412,   171,
     156,    88,   157,   588,    89,   362,   471,    90,   170,   515,
     118,   564,   371,   373,   402,   438,   497,   245,     0,     0,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   170,   171,     0,     0,     0,
      85,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,    86,     0,     0,     0,    88,     0,
       0,    89,     0,   171,    90,   170,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   170,     0,   171,     0,     0,     0,     0,     0,     0,
       0,   346,     0,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,     0,   171,
     170,     0,   355,   356,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   171,     0,
       0,     0,   357,   358,     0,     0,     0,     0,     0,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,     0,     0,     0,     0,     0,   356,
       0,     0,     0,     0,     0,     0,     0,     0,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   105,     0,     0,     0,     0,   358,     0,
       0,     0,     0,     0,     0,   106,   107,     0,     0,     0,
       0,     0,   108,     0,   170,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   109,     0,     0,     0,     0,
       0,   110,     0,     0,   170,     0,     0,     0,     0,     0,
       0,     0,   171,   111,     0,     0,     0,     0,     0,     0,
       0,   112,   113,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   171,     0,   103,     0,     0,     0,   114,   170,
       0,     0,   115,     0,     0,     0,     0,     0,     0,     0,
       0,   116,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   171,   170,     0,
       0,   264,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,     0,     0,     0,
       0,   347,     0,     0,   170,     0,   171,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   171,   170,     0,     0,   352,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   171,     0,     0,     0,   359,     0,     0,     0,     0,
       0,     0,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,     0,     0,   170,
       0,   469,     0,     0,   341,     0,     0,     0,     0,     0,
       0,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,     1,   171,     0,     2,
     470,     3,     0,     0,     0,     0,     4,     0,     0,     0,
       5,     0,     0,     6,     0,     7,     8,     9,     0,     0,
      10,     0,     0,    11,     0,   170,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,   170,   172,   173,   174,
     175,   342,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,    13,   171,   170,     0,     0,     0,     0,    14,
       0,    15,     0,    16,   171,    17,     0,    18,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
      21,   170,   171,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,     0,   171,
     178,   179,   180,   181,   182,   183,   184,   185,   186,     0,
       0,     0,     0,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186
};

static const short yycheck[] =
{
     129,   320,   111,   331,    16,   114,   115,     3,   381,     5,
      39,   384,     8,     3,     3,   372,    46,   126,     9,    32,
       3,     4,    70,   215,   106,    94,   420,    53,     5,     6,
       3,     4,     5,     6,     9,     8,     9,    14,     5,     6,
       3,    14,   109,    21,    62,     3,   128,    14,   367,     3,
      33,    24,    46,    54,     3,    18,     5,     3,    13,    14,
     127,   190,     8,    13,    14,     5,     5,    50,     8,    52,
      43,    86,    19,    63,    86,   123,    88,    89,   435,    52,
      53,    64,   476,    61,    57,     5,   112,   100,   106,     5,
      37,   120,    65,     3,    67,     5,   205,   106,   128,    53,
      63,   420,   582,    52,   477,    47,   463,   480,   481,   482,
     483,   484,   485,   128,    46,     5,     6,    47,     8,   128,
       3,   449,   105,   603,    14,     8,   138,   139,   320,   448,
     126,   143,   105,   461,   128,   126,   148,   126,   267,   151,
     152,   153,   119,   126,    13,    14,   119,    13,    14,   126,
     123,   126,   119,   126,   166,    54,   129,   476,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   367,   126,   132,   535,   119,
     119,     3,   132,     3,     4,     5,     6,   127,     8,     9,
     382,   106,     3,     4,    14,    97,    18,   306,   307,   119,
     573,   574,   331,   119,    24,   217,    24,   127,    91,   119,
     128,   127,   130,   128,    77,     5,     6,   125,     8,    83,
      83,   578,    33,    43,    14,    43,   238,    84,   420,   119,
       3,    88,    52,    53,   553,   563,   126,    57,    56,    50,
       3,    52,     3,     3,     4,    65,   112,    67,     9,   261,
     262,   263,   381,    24,    25,   384,   448,   126,     5,     6,
       0,     5,     6,   132,     8,    76,   132,   127,   128,   398,
      14,    58,    43,    33,    92,   467,     3,     3,     5,   408,
       3,     8,   126,   602,   476,   105,    57,   416,     5,     6,
      50,     8,    52,     3,   105,    78,    79,    14,     3,   119,
     127,   128,    85,   123,   127,   128,   126,   127,     3,   129,
       3,     4,     5,     6,     3,    42,     9,    10,   127,   128,
     449,    14,   127,   128,   127,   128,     3,   339,    21,   119,
     342,    24,   461,   127,   128,   131,   126,   126,    40,    63,
      33,   127,   128,    70,   356,   105,   358,   126,   477,   126,
      43,   480,   481,   482,   483,   484,   485,    50,    51,    52,
      53,   553,   127,   128,    57,    86,    68,     3,     4,     5,
       6,    98,    65,     9,    67,   119,   568,   569,    14,     3,
     126,     5,     6,   126,     8,    21,   127,   128,    24,   126,
     582,   126,   119,   127,   128,    40,   126,    33,   127,   128,
      93,   126,   119,    16,   126,   417,   126,    43,   127,   128,
     602,   603,   105,   126,    50,    51,    52,    53,   120,   121,
     122,    57,   614,    68,   127,   128,   119,   120,   112,    65,
     123,    67,   112,   126,   563,    97,   129,   144,    86,   146,
       3,    54,   149,   150,   573,   574,    70,    10,    62,     3,
       4,     5,     6,    66,     3,     9,   103,    93,   127,   128,
      14,    74,    75,   191,   192,    28,     3,    21,     3,   105,
      24,    15,    81,   118,   119,   120,   121,   122,    91,    33,
      96,     3,   126,   119,   120,    48,   127,   123,   127,    43,
     126,   104,   621,   129,   126,   112,    50,    51,    52,    53,
       3,     5,   112,    57,   112,    60,    69,   119,    91,    91,
       8,    65,    91,    67,     3,     4,     5,     6,     3,     8,
       9,     3,   126,    30,    87,    14,    83,   128,    52,     3,
     133,     3,     4,     5,     6,    24,     8,     9,   127,    93,
     128,     5,    14,    61,     3,   126,     8,   112,     3,   102,
       3,   105,    24,   127,    43,    89,   127,     3,   127,    84,
     112,    49,     5,    52,    53,   119,   120,   112,    57,   123,
     108,    43,   126,    23,    71,   129,    65,     8,    67,   126,
      52,    53,   128,   126,   126,    57,   126,     8,    53,   128,
     126,    45,   109,    65,   127,    67,     3,     4,     5,     6,
     126,     5,     9,    53,     3,   107,     8,    14,    78,    59,
     120,   127,   127,     3,   128,   127,   105,    24,     3,     8,
      72,     3,     4,     5,     6,    53,     8,     9,    40,    26,
     119,    49,    14,   105,   123,    72,    43,   126,   127,   112,
     129,   128,    24,   127,    18,    52,    53,   119,   128,   127,
      57,   123,     0,    53,   126,    26,    68,   129,    65,   127,
      67,    43,   112,   113,   114,   115,   116,   117,    63,   109,
      52,    53,    70,   123,   109,    57,    72,    84,     3,     4,
       5,     6,    26,    65,     9,    67,     5,   128,    26,    14,
     126,     3,    40,   127,     8,   128,   128,   126,   105,    24,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    40,   119,   127,     5,   124,   123,   127,    43,   126,
      68,   112,   129,   105,   566,   238,   366,    52,    53,   417,
     316,   556,    57,   614,   583,   623,   166,   119,   344,    68,
      65,   123,    67,   560,   126,   265,   416,   129,    40,   461,
      18,   510,   302,   304,   331,   382,   434,   140,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    40,    68,    -1,    -1,    -1,
     105,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   119,    -1,    -1,    -1,   123,    -1,
      -1,   126,    -1,    68,   129,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    40,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    68,
      40,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    68,    -1,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    16,    -1,    -1,    -1,    -1,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    -1,    35,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    87,    -1,    -1,    -1,    91,    40,
      -1,    -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    68,    40,    -1,
      -1,   127,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
      -1,   127,    -1,    -1,    40,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    68,    40,    -1,    -1,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    68,    -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    40,
      -1,   127,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    17,    68,    -1,    20,
     127,    22,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      31,    -1,    -1,    34,    -1,    36,    37,    38,    -1,    -1,
      41,    -1,    -1,    44,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    40,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    73,    68,    40,    -1,    -1,    -1,    -1,    80,
      -1,    82,    -1,    84,    68,    86,    -1,    88,    -1,    90,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
     101,    40,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    68,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122
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
     140,    45,   112,   127,   128,   133,   133,   127,   127,     3,
     127,   127,   127,   127,   127,   127,   128,   127,   128,   127,
     179,   179,   184,     3,   159,   185,   144,   106,   153,   154,
     128,   193,   126,   189,    61,   187,   187,     8,     3,   227,
     228,   112,   128,   154,   112,     3,     4,    33,    50,    52,
     105,   158,   200,   245,   102,     3,     3,   127,   128,   127,
     128,   204,   208,   211,   212,   127,    89,    53,   112,   127,
       3,     8,   182,   127,   179,   179,   112,    84,   147,    64,
     126,   155,   156,   157,   158,    49,   162,     5,   161,    78,
      79,    85,   233,   112,   108,   126,   159,   160,   221,    71,
     173,   174,   159,   160,   126,   126,   126,   127,   128,   126,
     201,   202,    24,    25,    43,    57,   222,   159,     8,    18,
     213,   128,     8,   126,   159,   128,   141,    53,    45,   127,
     127,   185,   148,   127,   126,   155,   109,    23,    53,    59,
     112,   113,   114,   115,   116,   117,   123,     5,   163,   107,
     165,   166,   127,    32,   100,    78,     8,   228,   127,   161,
       3,   175,   176,   120,   127,   127,   158,   126,   203,   204,
     128,     3,    63,   214,   204,   212,   161,     3,     5,    52,
     142,   143,   127,     3,   245,     3,    46,    72,   145,     8,
     127,   155,   159,   160,     9,   126,    70,   123,     8,   159,
     160,     8,   159,   160,   159,   160,   159,   160,   159,   160,
     159,   160,    53,    26,    49,    72,   167,   168,   127,   112,
     128,   127,   127,   128,   202,   127,   128,   127,    53,    26,
      63,   146,   127,   109,   109,   161,    70,     9,   126,   158,
     164,    72,    26,   171,     3,     5,     8,   126,   176,   204,
     143,   245,   158,   169,   170,     5,   159,   160,   159,   160,
     127,   161,   128,    26,    76,   169,   173,   126,     3,   177,
     178,   127,    19,    37,   128,   128,   127,   158,   169,   126,
       8,   112,   127,   128,   170,     5,   127,   127,   159,   178
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

  case 189:

    { TRACK_BOUNDS ( yyval, yyvsp[-9], yyvsp[0] ); ;}
    break;

  case 194:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 195:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 202:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 205:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 206:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 207:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 208:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 209:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 210:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 211:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 219:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 220:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 221:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 222:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 223:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 224:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 225:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 226:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 229:

    { yyval.m_iValue = 1; ;}
    break;

  case 230:

    { yyval.m_iValue = 0; ;}
    break;

  case 231:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 232:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 233:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 234:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 237:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 238:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 239:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 242:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 243:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 246:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 247:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 248:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 249:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 250:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 251:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 252:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 253:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 254:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 255:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 256:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 257:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 258:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 260:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 261:

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

  case 262:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 265:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 267:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 272:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 275:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 276:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 277:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 280:

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

  case 281:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 282:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 283:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 284:

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

  case 285:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 286:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 287:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 288:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 289:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 290:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 291:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 292:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 299:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 300:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 301:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 309:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 310:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 311:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 312:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 313:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 314:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 315:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 316:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 317:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 318:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 321:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 322:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 323:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 324:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 326:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 327:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 328:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 329:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 330:

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

