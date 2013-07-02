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
     TOK_DELETE = 290,
     TOK_DESC = 291,
     TOK_DESCRIBE = 292,
     TOK_DISTINCT = 293,
     TOK_DIV = 294,
     TOK_DROP = 295,
     TOK_FALSE = 296,
     TOK_FLOAT = 297,
     TOK_FLUSH = 298,
     TOK_FROM = 299,
     TOK_FUNCTION = 300,
     TOK_GLOBAL = 301,
     TOK_GROUP = 302,
     TOK_GROUPBY = 303,
     TOK_GROUP_CONCAT = 304,
     TOK_ID = 305,
     TOK_IN = 306,
     TOK_INDEX = 307,
     TOK_INSERT = 308,
     TOK_INT = 309,
     TOK_INTEGER = 310,
     TOK_INTO = 311,
     TOK_IS = 312,
     TOK_ISOLATION = 313,
     TOK_LEVEL = 314,
     TOK_LIKE = 315,
     TOK_LIMIT = 316,
     TOK_MATCH = 317,
     TOK_MAX = 318,
     TOK_META = 319,
     TOK_MIN = 320,
     TOK_MOD = 321,
     TOK_NAMES = 322,
     TOK_NULL = 323,
     TOK_OPTION = 324,
     TOK_ORDER = 325,
     TOK_OPTIMIZE = 326,
     TOK_PLAN = 327,
     TOK_PROFILE = 328,
     TOK_RAND = 329,
     TOK_RAMCHUNK = 330,
     TOK_READ = 331,
     TOK_REPEATABLE = 332,
     TOK_REPLACE = 333,
     TOK_RETURNS = 334,
     TOK_ROLLBACK = 335,
     TOK_RTINDEX = 336,
     TOK_SELECT = 337,
     TOK_SERIALIZABLE = 338,
     TOK_SET = 339,
     TOK_SESSION = 340,
     TOK_SHOW = 341,
     TOK_SONAME = 342,
     TOK_START = 343,
     TOK_STATUS = 344,
     TOK_STRING = 345,
     TOK_SUM = 346,
     TOK_TABLE = 347,
     TOK_TABLES = 348,
     TOK_TO = 349,
     TOK_TRANSACTION = 350,
     TOK_TRUE = 351,
     TOK_TRUNCATE = 352,
     TOK_UNCOMMITTED = 353,
     TOK_UPDATE = 354,
     TOK_VALUES = 355,
     TOK_VARIABLES = 356,
     TOK_WARNINGS = 357,
     TOK_WEIGHT = 358,
     TOK_WHERE = 359,
     TOK_WITHIN = 360,
     TOK_OR = 361,
     TOK_AND = 362,
     TOK_NE = 363,
     TOK_GTE = 364,
     TOK_LTE = 365,
     TOK_NOT = 366,
     TOK_NEG = 367
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
#define TOK_DELETE 290
#define TOK_DESC 291
#define TOK_DESCRIBE 292
#define TOK_DISTINCT 293
#define TOK_DIV 294
#define TOK_DROP 295
#define TOK_FALSE 296
#define TOK_FLOAT 297
#define TOK_FLUSH 298
#define TOK_FROM 299
#define TOK_FUNCTION 300
#define TOK_GLOBAL 301
#define TOK_GROUP 302
#define TOK_GROUPBY 303
#define TOK_GROUP_CONCAT 304
#define TOK_ID 305
#define TOK_IN 306
#define TOK_INDEX 307
#define TOK_INSERT 308
#define TOK_INT 309
#define TOK_INTEGER 310
#define TOK_INTO 311
#define TOK_IS 312
#define TOK_ISOLATION 313
#define TOK_LEVEL 314
#define TOK_LIKE 315
#define TOK_LIMIT 316
#define TOK_MATCH 317
#define TOK_MAX 318
#define TOK_META 319
#define TOK_MIN 320
#define TOK_MOD 321
#define TOK_NAMES 322
#define TOK_NULL 323
#define TOK_OPTION 324
#define TOK_ORDER 325
#define TOK_OPTIMIZE 326
#define TOK_PLAN 327
#define TOK_PROFILE 328
#define TOK_RAND 329
#define TOK_RAMCHUNK 330
#define TOK_READ 331
#define TOK_REPEATABLE 332
#define TOK_REPLACE 333
#define TOK_RETURNS 334
#define TOK_ROLLBACK 335
#define TOK_RTINDEX 336
#define TOK_SELECT 337
#define TOK_SERIALIZABLE 338
#define TOK_SET 339
#define TOK_SESSION 340
#define TOK_SHOW 341
#define TOK_SONAME 342
#define TOK_START 343
#define TOK_STATUS 344
#define TOK_STRING 345
#define TOK_SUM 346
#define TOK_TABLE 347
#define TOK_TABLES 348
#define TOK_TO 349
#define TOK_TRANSACTION 350
#define TOK_TRUE 351
#define TOK_TRUNCATE 352
#define TOK_UNCOMMITTED 353
#define TOK_UPDATE 354
#define TOK_VALUES 355
#define TOK_VARIABLES 356
#define TOK_WARNINGS 357
#define TOK_WEIGHT 358
#define TOK_WHERE 359
#define TOK_WITHIN 360
#define TOK_OR 361
#define TOK_AND 362
#define TOK_NE 363
#define TOK_GTE 364
#define TOK_LTE 365
#define TOK_NOT 366
#define TOK_NEG 367




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
#define YYFINAL  119
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1245

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  132
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  110
/* YYNRULES -- Number of rules. */
#define YYNRULES  315
/* YYNRULES -- Number of states. */
#define YYNSTATES  597

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   367

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   120,   109,     2,
     124,   125,   118,   116,   126,   117,   129,   119,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   123,
     112,   110,   113,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   130,     2,   131,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   127,   108,   128,     2,     2,     2,     2,
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
     105,   106,   107,   111,   114,   115,   121,   122
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
     294,   298,   302,   306,   310,   314,   318,   323,   327,   329,
     331,   336,   340,   344,   346,   348,   350,   353,   355,   358,
     360,   362,   366,   367,   372,   373,   375,   377,   381,   382,
     384,   390,   391,   393,   397,   403,   405,   409,   411,   414,
     417,   418,   420,   423,   428,   429,   431,   434,   436,   440,
     444,   448,   454,   461,   465,   467,   471,   475,   477,   479,
     481,   483,   485,   487,   489,   492,   495,   499,   503,   507,
     511,   515,   519,   523,   527,   531,   535,   539,   543,   547,
     551,   555,   559,   563,   567,   571,   573,   575,   580,   585,
     590,   595,   600,   604,   611,   618,   622,   624,   628,   630,
     632,   636,   642,   645,   646,   649,   651,   654,   657,   661,
     663,   665,   670,   675,   679,   681,   683,   685,   687,   689,
     691,   695,   700,   705,   710,   714,   719,   727,   733,   735,
     737,   739,   741,   743,   745,   747,   749,   751,   754,   761,
     763,   765,   766,   770,   772,   776,   778,   782,   786,   788,
     792,   794,   796,   798,   802,   805,   813,   823,   830,   832,
     836,   838,   842,   844,   848,   849,   852,   854,   858,   862,
     863,   865,   867,   869,   873,   875,   877,   881,   888,   890,
     894,   898,   902,   908,   913,   915,   917,   919,   921,   929,
     934,   935,   937,   940,   942,   946,   950,   953,   957,   964,
     965,   967,   969,   972,   975,   978,   980,   988,   990,   992,
     994,   998,  1005,  1009,  1013,  1017,  1019,  1023,  1027,  1031,
    1034,  1036,  1039,  1041,  1043,  1047
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     133,     0,    -1,   134,    -1,   135,    -1,   135,   123,    -1,
     193,    -1,   201,    -1,   187,    -1,   188,    -1,   191,    -1,
     202,    -1,   211,    -1,   213,    -1,   214,    -1,   219,    -1,
     224,    -1,   225,    -1,   229,    -1,   231,    -1,   232,    -1,
     233,    -1,   234,    -1,   226,    -1,   235,    -1,   237,    -1,
     238,    -1,   218,    -1,   136,    -1,   135,   123,   136,    -1,
     137,    -1,   182,    -1,   138,    -1,    82,     3,   124,   124,
     138,   125,   139,   125,    -1,   145,    -1,    82,   146,    44,
     124,   142,   145,   125,   143,   144,    -1,    -1,   126,   140,
      -1,   141,    -1,   140,   126,   141,    -1,     3,    -1,     5,
      -1,    50,    -1,    -1,    70,    26,   167,    -1,    -1,    61,
       5,    -1,    61,     5,   126,     5,    -1,    82,   146,    44,
     150,   151,   160,   163,   165,   169,   171,    -1,   147,    -1,
     146,   126,   147,    -1,   118,    -1,   149,   148,    -1,    -1,
       3,    -1,    18,     3,    -1,   177,    -1,    21,   124,   177,
     125,    -1,    63,   124,   177,   125,    -1,    65,   124,   177,
     125,    -1,    91,   124,   177,   125,    -1,    49,   124,   177,
     125,    -1,    33,   124,   118,   125,    -1,    48,   124,   125,
      -1,    33,   124,    38,     3,   125,    -1,     3,    -1,   150,
     126,     3,    -1,    -1,   152,    -1,   104,   153,    -1,   154,
      -1,   153,   107,   153,    -1,   124,   153,   125,    -1,    62,
     124,     8,   125,    -1,   156,   110,   157,    -1,   156,   111,
     157,    -1,   156,    51,   124,   159,   125,    -1,   156,   121,
      51,   124,   159,   125,    -1,   156,    51,     9,    -1,   156,
     121,    51,     9,    -1,   156,    23,   157,   107,   157,    -1,
     156,   113,   157,    -1,   156,   112,   157,    -1,   156,   114,
     157,    -1,   156,   115,   157,    -1,   156,   110,   158,    -1,
     155,    -1,   156,    23,   158,   107,   158,    -1,   156,    23,
     157,   107,   158,    -1,   156,    23,   158,   107,   157,    -1,
     156,   113,   158,    -1,   156,   112,   158,    -1,   156,   114,
     158,    -1,   156,   115,   158,    -1,   156,   110,     8,    -1,
     156,   111,     8,    -1,   156,    57,    68,    -1,   156,    57,
     121,    68,    -1,   156,   111,   158,    -1,     3,    -1,     4,
      -1,    33,   124,   118,   125,    -1,    48,   124,   125,    -1,
     103,   124,   125,    -1,    50,    -1,   239,    -1,     5,    -1,
     117,     5,    -1,     6,    -1,   117,     6,    -1,    14,    -1,
     157,    -1,   159,   126,   157,    -1,    -1,    47,   161,    26,
     162,    -1,    -1,     5,    -1,   156,    -1,   162,   126,   156,
      -1,    -1,   164,    -1,   105,    47,    70,    26,   167,    -1,
      -1,   166,    -1,    70,    26,   167,    -1,    70,    26,    74,
     124,   125,    -1,   168,    -1,   167,   126,   168,    -1,   156,
      -1,   156,    19,    -1,   156,    36,    -1,    -1,   170,    -1,
      61,     5,    -1,    61,     5,   126,     5,    -1,    -1,   172,
      -1,    69,   173,    -1,   174,    -1,   173,   126,   174,    -1,
       3,   110,     3,    -1,     3,   110,     5,    -1,     3,   110,
     124,   175,   125,    -1,     3,   110,     3,   124,     8,   125,
      -1,     3,   110,     8,    -1,   176,    -1,   175,   126,   176,
      -1,     3,   110,   157,    -1,     3,    -1,     4,    -1,    50,
      -1,     5,    -1,     6,    -1,    14,    -1,     9,    -1,   117,
     177,    -1,   121,   177,    -1,   177,   116,   177,    -1,   177,
     117,   177,    -1,   177,   118,   177,    -1,   177,   119,   177,
      -1,   177,   112,   177,    -1,   177,   113,   177,    -1,   177,
     109,   177,    -1,   177,   108,   177,    -1,   177,   120,   177,
      -1,   177,    39,   177,    -1,   177,    66,   177,    -1,   177,
     115,   177,    -1,   177,   114,   177,    -1,   177,   110,   177,
      -1,   177,   111,   177,    -1,   177,   107,   177,    -1,   177,
     106,   177,    -1,   124,   177,   125,    -1,   127,   181,   128,
      -1,   178,    -1,   239,    -1,     3,   124,   179,   125,    -1,
      51,   124,   179,   125,    -1,    55,   124,   179,   125,    -1,
      24,   124,   179,   125,    -1,    42,   124,   179,   125,    -1,
       3,   124,   125,    -1,    65,   124,   177,   126,   177,   125,
      -1,    63,   124,   177,   126,   177,   125,    -1,   103,   124,
     125,    -1,   180,    -1,   179,   126,   180,    -1,   177,    -1,
       8,    -1,     3,   110,   157,    -1,   181,   126,     3,   110,
     157,    -1,    86,   184,    -1,    -1,    60,     8,    -1,   102,
      -1,    89,   183,    -1,    64,   183,    -1,    16,    89,   183,
      -1,    73,    -1,    72,    -1,    16,     8,    89,   183,    -1,
      16,     3,    89,   183,    -1,    52,     3,    89,    -1,     3,
      -1,    68,    -1,     8,    -1,     5,    -1,     6,    -1,   185,
      -1,   186,   117,   185,    -1,    84,     3,   110,   190,    -1,
      84,     3,   110,   189,    -1,    84,     3,   110,    68,    -1,
      84,    67,   186,    -1,    84,    10,   110,   186,    -1,    84,
      46,     9,   110,   124,   159,   125,    -1,    84,    46,     3,
     110,   189,    -1,     3,    -1,     8,    -1,    96,    -1,    41,
      -1,   157,    -1,    31,    -1,    80,    -1,   192,    -1,    22,
      -1,    88,    95,    -1,   194,    56,     3,   195,   100,   197,
      -1,    53,    -1,    78,    -1,    -1,   124,   196,   125,    -1,
     156,    -1,   196,   126,   156,    -1,   198,    -1,   197,   126,
     198,    -1,   124,   199,   125,    -1,   200,    -1,   199,   126,
     200,    -1,   157,    -1,   158,    -1,     8,    -1,   124,   159,
     125,    -1,   124,   125,    -1,    35,    44,   150,   104,    50,
     110,   157,    -1,    35,    44,   150,   104,    50,    51,   124,
     159,   125,    -1,    27,     3,   124,   203,   206,   125,    -1,
     204,    -1,   203,   126,   204,    -1,   200,    -1,   124,   205,
     125,    -1,     8,    -1,   205,   126,     8,    -1,    -1,   126,
     207,    -1,   208,    -1,   207,   126,   208,    -1,   200,   209,
     210,    -1,    -1,    18,    -1,     3,    -1,    61,    -1,   212,
       3,   183,    -1,    37,    -1,    36,    -1,    86,    93,   183,
      -1,    99,   150,    84,   215,   152,   171,    -1,   216,    -1,
     215,   126,   216,    -1,     3,   110,   157,    -1,     3,   110,
     158,    -1,     3,   110,   124,   159,   125,    -1,     3,   110,
     124,   125,    -1,    55,    -1,    24,    -1,    42,    -1,    25,
      -1,    17,    92,     3,    15,    30,     3,   217,    -1,    86,
     227,   101,   220,    -1,    -1,   221,    -1,   104,   222,    -1,
     223,    -1,   222,   106,   223,    -1,     3,   110,     8,    -1,
      86,    29,    -1,    86,    28,    84,    -1,    84,   227,    95,
      58,    59,   228,    -1,    -1,    46,    -1,    85,    -1,    76,
      98,    -1,    76,    32,    -1,    77,    76,    -1,    83,    -1,
      34,    45,     3,    79,   230,    87,     8,    -1,    54,    -1,
      42,    -1,    90,    -1,    40,    45,     3,    -1,    20,    52,
       3,    94,    81,     3,    -1,    43,    81,     3,    -1,    43,
      75,     3,    -1,    82,   236,   169,    -1,    10,    -1,    10,
     129,     3,    -1,    97,    81,     3,    -1,    71,    52,     3,
      -1,     3,   240,    -1,   241,    -1,   240,   241,    -1,    13,
      -1,    14,    -1,   130,   177,   131,    -1,   130,     8,   131,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   158,   158,   159,   160,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   191,   192,   196,
     197,   201,   202,   210,   211,   218,   220,   224,   228,   235,
     236,   237,   241,   254,   261,   263,   268,   277,   292,   293,
     297,   298,   301,   303,   304,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   320,   321,   324,   326,   330,   334,
     335,   336,   340,   345,   352,   360,   368,   377,   382,   387,
     392,   397,   402,   407,   412,   417,   422,   427,   432,   437,
     442,   447,   452,   457,   462,   467,   472,   480,   484,   485,
     490,   496,   502,   508,   514,   518,   519,   530,   531,   532,
     536,   542,   548,   550,   553,   555,   562,   566,   572,   574,
     578,   589,   591,   595,   599,   606,   607,   611,   612,   613,
     616,   618,   622,   627,   634,   636,   640,   644,   645,   649,
     654,   659,   665,   670,   678,   683,   690,   700,   701,   702,
     703,   704,   705,   706,   707,   708,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   720,   721,   722,   723,
     724,   725,   726,   727,   728,   729,   730,   734,   735,   736,
     737,   738,   739,   740,   741,   742,   746,   747,   751,   752,
     756,   757,   763,   766,   768,   772,   773,   774,   775,   776,
     777,   778,   783,   788,   798,   799,   800,   801,   802,   806,
     807,   811,   816,   821,   826,   827,   831,   836,   844,   845,
     849,   850,   851,   865,   866,   867,   871,   872,   878,   886,
     887,   890,   892,   896,   897,   901,   902,   906,   910,   911,
     915,   916,   917,   918,   919,   925,   933,   947,   955,   959,
     966,   967,   974,   984,   990,   992,   996,  1001,  1005,  1012,
    1014,  1018,  1019,  1025,  1033,  1034,  1040,  1046,  1054,  1055,
    1059,  1073,  1079,  1083,  1093,  1094,  1095,  1096,  1100,  1113,
    1119,  1120,  1124,  1128,  1129,  1133,  1137,  1144,  1151,  1157,
    1158,  1159,  1163,  1164,  1165,  1166,  1172,  1183,  1184,  1185,
    1189,  1200,  1212,  1221,  1232,  1240,  1241,  1247,  1258,  1269,
    1272,  1273,  1277,  1278,  1279,  1280
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
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", 
  "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTEGER", 
  "TOK_INTO", "TOK_IS", "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", 
  "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", 
  "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", 
  "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_STRING", "TOK_SUM", "TOK_TABLE", "TOK_TABLES", "TOK_TO", 
  "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", 
  "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", 
  "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", 
  "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", 
  "','", "'{'", "'}'", "'.'", "'['", "']'", "$accept", "request", 
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
  "sysvar_name", "truncate", "optimize_index", "json_field", "subscript", 
  "subkey", 0
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
     355,   356,   357,   358,   359,   360,   361,   362,   124,    38,
      61,   363,    60,    62,   364,   365,    43,    45,    42,    47,
      37,   366,   367,    59,    40,    41,    44,   123,   125,    46,
      91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   132,   133,   133,   133,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   135,   135,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   141,
     141,   141,   142,   143,   144,   144,   144,   145,   146,   146,
     147,   147,   148,   148,   148,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   150,   150,   151,   151,   152,   153,
     153,   153,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   155,   156,   156,
     156,   156,   156,   156,   156,   157,   157,   158,   158,   158,
     159,   159,   160,   160,   161,   161,   162,   162,   163,   163,
     164,   165,   165,   166,   166,   167,   167,   168,   168,   168,
     169,   169,   170,   170,   171,   171,   172,   173,   173,   174,
     174,   174,   174,   174,   175,   175,   176,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   179,   179,   180,   180,
     181,   181,   182,   183,   183,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   185,   185,   185,   185,   185,   186,
     186,   187,   187,   187,   187,   187,   188,   188,   189,   189,
     190,   190,   190,   191,   191,   191,   192,   192,   193,   194,
     194,   195,   195,   196,   196,   197,   197,   198,   199,   199,
     200,   200,   200,   200,   200,   201,   201,   202,   203,   203,
     204,   204,   205,   205,   206,   206,   207,   207,   208,   209,
     209,   210,   210,   211,   212,   212,   213,   214,   215,   215,
     216,   216,   216,   216,   217,   217,   217,   217,   218,   219,
     220,   220,   221,   222,   222,   223,   224,   225,   226,   227,
     227,   227,   228,   228,   228,   228,   229,   230,   230,   230,
     231,   232,   233,   234,   235,   236,   236,   237,   238,   239,
     240,   240,   241,   241,   241,   241
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
       3,     3,     3,     3,     3,     3,     4,     3,     1,     1,
       4,     3,     3,     1,     1,     1,     2,     1,     2,     1,
       1,     3,     0,     4,     0,     1,     1,     3,     0,     1,
       5,     0,     1,     3,     5,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     6,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     4,     4,     4,
       4,     4,     3,     6,     6,     3,     1,     3,     1,     1,
       3,     5,     2,     0,     2,     1,     2,     2,     3,     1,
       1,     4,     4,     3,     1,     1,     1,     1,     1,     1,
       3,     4,     4,     4,     3,     4,     7,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     6,     1,
       1,     0,     3,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     3,     2,     7,     9,     6,     1,     3,
       1,     3,     1,     3,     0,     2,     1,     3,     3,     0,
       1,     1,     1,     3,     1,     1,     3,     6,     1,     3,
       3,     3,     5,     4,     1,     1,     1,     1,     7,     4,
       0,     1,     2,     1,     3,     3,     2,     3,     6,     0,
       1,     1,     2,     2,     2,     1,     7,     1,     1,     1,
       3,     6,     3,     3,     3,     1,     3,     3,     3,     2,
       1,     2,     1,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   226,     0,   223,     0,     0,   265,   264,
       0,     0,   229,     0,   230,   224,     0,   289,   289,     0,
       0,     0,     0,     2,     3,    27,    29,    31,    33,    30,
       7,     8,     9,   225,     5,     0,     6,    10,    11,     0,
      12,    13,    26,    14,    15,    16,    22,    17,    18,    19,
      20,    21,    23,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   147,   148,   150,   151,   153,   305,
     152,     0,     0,     0,     0,     0,     0,   149,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
      48,    52,    55,   175,   130,   176,     0,     0,   290,     0,
     291,     0,     0,     0,   286,   290,     0,   193,   200,   199,
     193,   193,   195,   192,     0,   227,     0,    64,     0,     1,
       4,     0,   193,     0,     0,     0,     0,     0,   300,   303,
     302,   308,   312,   313,     0,     0,   309,   310,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,     0,     0,   154,   155,     0,     0,     0,     0,
       0,    53,     0,    51,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   304,   131,     0,     0,     0,     0,   204,   207,
     208,   206,   205,   209,   214,     0,     0,     0,   193,   287,
       0,     0,   197,   196,   266,   280,   307,     0,     0,     0,
       0,    28,   231,   263,     0,     0,   105,   107,   242,   109,
       0,     0,   240,   241,   250,   254,   248,     0,     0,   189,
       0,   182,   188,     0,   186,     0,     0,   311,   306,     0,
       0,     0,     0,     0,    62,     0,     0,     0,     0,     0,
       0,   185,     0,     0,     0,   173,     0,     0,   174,    42,
      66,    49,    54,   165,   166,   172,   171,   163,   162,   169,
     170,   160,   161,   168,   167,   156,   157,   158,   159,   164,
     132,   218,   219,   221,   213,   220,     0,   222,   212,   211,
     215,     0,     0,     0,     0,   193,   193,   198,   203,   194,
       0,   279,   281,     0,     0,   268,    65,     0,     0,     0,
       0,   106,   108,   252,   244,   110,     0,     0,     0,     0,
     298,   297,   299,     0,     0,     0,     0,   177,     0,   315,
     314,    56,   180,     0,    61,   181,    60,   178,   179,    57,
       0,    58,     0,    59,     0,     0,   190,     0,     0,     0,
     112,    67,     0,   217,     0,   210,     0,   202,   201,     0,
     282,   283,     0,     0,   134,    98,    99,     0,     0,   103,
       0,   233,     0,   104,     0,     0,   301,   243,     0,   251,
       0,   250,   249,   255,   256,   247,     0,     0,     0,    35,
     187,    63,     0,     0,     0,     0,     0,     0,     0,    68,
      69,    85,     0,   114,   118,   133,     0,     0,     0,   295,
     288,     0,     0,     0,   270,   271,   269,     0,   267,   135,
       0,     0,     0,   232,     0,     0,   228,   235,   275,   277,
     276,   274,   278,   111,   253,   260,     0,     0,   296,     0,
     245,     0,     0,   184,   183,   191,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   115,     0,     0,   121,   119,   216,   293,   292,   294,
     285,   284,   273,     0,     0,   136,   137,     0,   101,   102,
     234,     0,     0,   238,     0,   261,   262,   258,   259,   257,
       0,    39,    40,    41,    36,    37,    32,     0,     0,    44,
       0,    71,    70,     0,     0,    77,     0,    95,     0,    93,
      73,    84,    94,    74,    97,    81,    90,    80,    89,    82,
      91,    83,    92,     0,     0,     0,     0,   130,   122,   272,
       0,     0,   100,   237,     0,   236,   246,     0,     0,     0,
      34,    72,     0,     0,     0,    96,    78,     0,   116,   113,
       0,     0,   134,   139,   140,   143,     0,   138,   239,    38,
     127,    43,   125,    45,    79,    87,    88,    86,    75,     0,
       0,     0,     0,   123,    47,     0,     0,     0,   144,   128,
     129,     0,     0,    76,   117,   120,     0,     0,     0,   141,
       0,   126,    46,   124,   142,   146,   145
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   442,   494,   495,
     348,   499,   540,    28,    89,    90,   163,    91,   260,   350,
     351,   399,   400,   401,   560,   315,   223,   316,   404,   462,
     549,   464,   465,   527,   528,   561,   562,   182,   183,   418,
     419,   475,   476,   577,   578,   232,    93,   233,   234,   158,
      29,   202,   113,   193,   194,    30,    31,   288,   289,    32,
      33,    34,    35,   308,   372,   426,   427,   482,   224,    36,
      37,   225,   226,   317,   319,   383,   384,   436,   487,    38,
      39,    40,    41,   304,   305,   432,    42,    43,   301,   302,
     360,   361,    44,    45,    46,   101,   410,    47,   323,    48,
      49,    50,    51,    52,    94,    53,    54,    95,   136,   137
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -433
static const short yypact[] =
{
    1030,   -42,     9,  -433,    86,  -433,    73,    98,  -433,  -433,
      81,   122,  -433,    64,  -433,  -433,   333,   261,  1026,    90,
     117,   208,   216,  -433,   111,  -433,  -433,  -433,  -433,  -433,
    -433,  -433,  -433,  -433,  -433,   171,  -433,  -433,  -433,   239,
    -433,  -433,  -433,  -433,  -433,  -433,  -433,  -433,  -433,  -433,
    -433,  -433,  -433,  -433,  -433,   254,   259,   130,   263,   208,
     267,   277,   282,   290,    71,  -433,  -433,  -433,  -433,   182,
    -433,   177,   200,   211,   217,   221,   224,  -433,   229,   237,
     243,   245,   255,   262,   645,  -433,   645,   645,   347,   -23,
    -433,    31,  1029,  -433,   317,  -433,   285,   287,   223,    37,
    -433,   292,    28,   315,  -433,  -433,   398,   343,  -433,  -433,
     343,   343,  -433,  -433,   304,  -433,   404,  -433,   -71,  -433,
      29,   405,   343,   395,   318,    24,   332,   -14,  -433,  -433,
    -433,  -433,  -433,  -433,   178,   241,     5,  -433,   410,   645,
     565,   -10,   565,   289,   645,   565,   565,   645,   645,   645,
     291,    88,   299,   303,  -433,  -433,   781,   305,   -68,    -1,
     416,  -433,   425,  -433,   645,   645,   645,   645,   645,   645,
     645,   645,   645,   645,   645,   645,   645,   645,   645,   645,
     645,   424,  -433,  -433,   253,    37,   321,   322,  -433,  -433,
    -433,  -433,  -433,  -433,   316,   376,   346,   349,   343,  -433,
     350,   437,  -433,  -433,  -433,   344,  -433,   450,   453,   481,
     475,  -433,   337,  -433,   439,   387,  -433,  -433,  -433,  -433,
     225,    19,  -433,  -433,  -433,   348,  -433,    -7,   420,  -433,
     582,  -433,  1029,   126,  -433,   341,   613,  -433,  -433,   809,
     149,   470,   351,   151,  -433,   837,   164,   172,   668,   697,
     865,  -433,   438,   645,   645,  -433,    20,   472,  -433,  -433,
      84,  -433,  -433,  -433,  -433,  1043,  1057,  1125,   639,  1066,
    1066,   274,   274,   274,   274,   104,   104,  -433,  -433,  -433,
     357,  -433,  -433,  -433,  -433,  -433,   473,  -433,  -433,  -433,
     316,    45,   353,    37,   423,   343,   343,  -433,  -433,  -433,
     489,  -433,  -433,   384,   110,  -433,  -433,   356,   396,   494,
     495,  -433,  -433,  -433,  -433,  -433,   183,   189,    24,   374,
    -433,  -433,  -433,   413,   -18,   416,   379,  -433,   565,  -433,
    -433,  -433,  -433,   381,  -433,  -433,  -433,  -433,  -433,  -433,
     645,  -433,   645,  -433,   725,   753,  -433,   399,   426,     4,
     463,  -433,   506,  -433,    20,  -433,   196,  -433,  -433,   402,
     407,  -433,    82,   450,   446,     5,  -433,   392,   393,  -433,
     394,  -433,   192,  -433,   397,    39,  -433,  -433,    20,  -433,
     512,    38,  -433,   400,  -433,  -433,   514,   401,    20,   409,
    -433,  -433,   893,   921,    20,   416,   403,   414,     4,   417,
    -433,  -433,   560,   537,   440,  -433,   194,   -16,   474,  -433,
    -433,   541,   489,    21,  -433,  -433,  -433,   548,  -433,  -433,
     434,   428,   429,  -433,   356,    72,   430,  -433,  -433,  -433,
    -433,  -433,  -433,  -433,  -433,  -433,    48,    72,  -433,    20,
    -433,   175,   432,  -433,  -433,  -433,   -22,   488,   552,   -63,
       4,    92,     1,   -48,    62,   199,    92,    92,    92,    92,
     510,  -433,   540,   520,   505,  -433,  -433,  -433,  -433,  -433,
    -433,  -433,  -433,   197,   466,   452,  -433,   455,  -433,  -433,
    -433,    52,   226,  -433,   397,  -433,  -433,  -433,   563,  -433,
     238,  -433,  -433,  -433,   456,  -433,  -433,   208,   564,   531,
     468,  -433,  -433,   487,   490,  -433,    20,  -433,   527,  -433,
    -433,  -433,  -433,  -433,  -433,  -433,  -433,  -433,  -433,  -433,
    -433,  -433,  -433,     3,   356,   530,   575,   317,  -433,  -433,
       6,   548,  -433,  -433,    72,  -433,  -433,   175,   356,   598,
    -433,  -433,    92,    92,   246,  -433,  -433,    20,  -433,   478,
     583,   352,   446,   486,  -433,  -433,   609,  -433,  -433,  -433,
     181,   492,  -433,   493,  -433,  -433,  -433,  -433,  -433,   248,
     356,   356,   497,   492,  -433,   605,   504,   251,  -433,  -433,
    -433,   356,   617,  -433,  -433,   492,   498,   500,    20,  -433,
     609,  -433,  -433,  -433,  -433,  -433,  -433
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -433,  -433,  -433,  -433,   507,  -433,   406,  -433,  -433,    89,
    -433,  -433,  -433,   281,   236,   479,  -433,  -433,    54,  -433,
     330,  -359,  -433,  -433,  -284,  -125,  -283,  -339,  -433,  -433,
    -433,  -433,  -433,  -433,  -433,  -432,    57,   108,  -433,    91,
    -433,  -433,   109,  -433,    51,   -15,  -433,   142,   314,  -433,
    -433,  -105,  -433,   360,   459,  -433,  -433,   355,  -433,  -433,
    -433,  -433,  -433,  -433,  -433,  -433,   173,  -433,  -315,  -433,
    -433,  -433,   338,  -433,  -433,  -433,   218,  -433,  -433,  -433,
    -433,  -433,  -433,  -433,   295,  -433,  -433,  -433,  -433,  -433,
    -433,   249,  -433,  -433,  -433,   642,  -433,  -433,  -433,  -433,
    -433,  -433,  -433,  -433,  -433,  -433,  -433,  -303,  -433,   526
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -260
static const short yytable[] =
{
     222,    92,   117,   381,   373,   203,   204,   365,   366,   553,
     505,   554,   546,   207,   555,   406,   467,   213,   132,   133,
     507,   159,   497,   371,   216,   216,   216,   313,   241,   216,
     217,   196,   218,   387,   161,   320,   197,   367,   219,   449,
     188,  -259,   189,   190,   450,   191,   373,   321,   281,   162,
      55,   485,   368,   282,   369,   208,   435,   216,   257,   287,
     258,    56,   501,   428,   429,   402,   397,   216,   217,   154,
     509,   155,   156,   508,   473,   118,   219,   216,   217,   415,
     218,   430,   468,   322,   132,   133,   219,   216,   217,    57,
     228,   502,   388,   297,   431,   373,   219,   216,   217,  -259,
     490,   132,   133,   160,   160,   192,   219,   370,   242,   486,
     483,   209,   208,   127,   402,   210,    63,   198,    58,   573,
     236,   373,   488,   259,   239,   506,    60,   547,   398,   245,
     556,   346,   248,   249,   250,   135,   286,   286,   286,   585,
     480,   220,    59,   164,   314,    92,   472,   373,   221,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   402,   544,   504,   286,
     165,   511,   514,   516,   518,   520,   522,   314,   491,   220,
     492,   151,    65,    66,    67,   115,   229,    68,   349,   220,
     357,   358,    70,   222,    92,   134,   481,    61,   116,   220,
     579,   135,    72,    62,   216,   217,   413,   512,   569,   220,
     208,   117,   252,   219,   349,   156,   119,   580,   135,   558,
      74,   373,   178,   179,   180,   493,   186,   121,    77,    78,
     311,   312,   187,    79,   120,   373,   363,   414,   344,   345,
     548,   152,   122,   153,   151,    65,    66,    67,   373,   235,
      68,   327,   328,   433,   125,    70,   281,   123,   216,   565,
     567,   282,   124,   440,    96,    72,   126,   373,   373,   445,
     128,    97,   407,   408,   332,   328,   335,   328,   373,   409,
     129,    83,   240,    74,   243,   130,   584,   246,   247,   337,
     328,    77,    78,   131,   283,    84,    79,   338,   328,    86,
     222,   139,   230,   231,   152,    88,   153,    98,   377,   378,
      92,   138,   222,   164,   379,   380,   220,   423,   424,   466,
     378,   284,   529,   378,   140,   392,   503,   393,    99,   510,
     513,   515,   517,   519,   521,   141,    64,    65,    66,    67,
     165,   142,    68,    69,    83,   143,   100,    70,   144,   285,
     157,   533,   534,   145,    71,   365,   366,    72,    84,   365,
     366,   146,    86,   536,   378,    87,    73,   147,    88,   148,
     286,   568,   378,   583,   378,    74,   589,   590,   181,   149,
      92,    75,    76,    77,    78,   367,   150,   195,    79,   367,
     176,   177,   178,   179,   180,   184,    80,   185,    81,   199,
     368,   200,   369,   201,   368,   205,   369,   206,   212,   222,
     214,   227,   215,   238,   244,   256,   251,   564,   566,   151,
      65,    66,    67,   253,    82,    68,   572,   254,   262,   280,
      70,   291,   292,   293,   294,   295,    83,    71,   296,   298,
      72,   151,    65,    66,    67,   299,   229,    68,   300,    73,
      84,    85,    70,   303,    86,   370,   306,    87,    74,   370,
      88,   307,    72,   595,    75,    76,    77,    78,   310,   309,
     324,    79,   329,   333,   318,   347,   334,   354,   311,    80,
      74,    81,   356,   352,    64,    65,    66,    67,    77,    78,
      68,   102,   359,    79,   362,    70,   374,   375,   376,   385,
     386,   152,    71,   153,   389,    72,   391,    82,   395,   394,
     403,   405,   411,   412,    73,   417,   420,   421,   422,    83,
     434,   425,   438,    74,   450,   439,   437,   106,   447,    75,
      76,    77,    78,    84,    85,   441,    79,    86,   448,   107,
      87,    83,   461,    88,    80,   463,    81,   108,   109,   470,
     469,   474,   477,   478,   479,    84,   484,   496,   498,    86,
     500,   523,    87,   231,   110,    88,   524,   525,   151,    65,
      66,    67,    82,   229,    68,   526,   530,   112,   531,    70,
     532,   435,   537,   451,    83,   151,    65,    66,    67,    72,
     538,    68,   539,   541,   542,   545,    70,   543,    84,    85,
     550,   551,    86,   563,   570,    87,    72,    74,    88,   571,
     575,   452,   576,   587,   588,    77,    78,   453,   581,   582,
      79,   586,   592,   593,    74,   594,   559,   211,   152,   396,
     153,   446,    77,    78,   364,   552,   326,    79,   591,   261,
     557,   596,   390,   574,   290,   152,   353,   153,   151,    65,
      66,    67,   164,   355,    68,   489,   382,   535,   416,    70,
     114,   471,   237,     0,   325,     0,     0,     0,    83,    72,
     454,   455,   456,   457,   458,   459,     0,     0,   164,   165,
       0,   460,    84,     0,     0,    83,    86,    74,     0,    87,
       0,     0,    88,     0,     0,    77,    78,     0,     0,    84,
      79,     0,     0,    86,     0,   165,    87,   164,   152,    88,
     153,     0,     0,     0,     0,     0,     0,     0,     0,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   165,     0,   164,     0,     0,     0,
       0,     0,     0,     0,   330,     0,     0,     0,    83,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
       0,     0,    84,   165,   164,     0,    86,     0,     0,    87,
       0,     0,    88,     0,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,     0,
       0,   165,   164,   339,   340,     0,     0,     0,     0,     0,
       0,     0,     0,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,     0,   165,
     164,     0,   341,   342,     0,     0,     0,     0,     0,     0,
       0,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,     0,   165,   164,     0,
       0,   340,     0,     0,     0,     0,     0,     0,     0,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,     0,   165,   164,     0,     0,   342,
       0,     0,     0,     0,     0,     0,     0,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,     0,   165,   164,     0,   255,     0,     0,     0,
       0,     0,     0,     0,     0,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
       0,   165,   164,     0,   331,     0,     0,     0,     0,     0,
       0,     0,     0,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,     0,   165,
     164,     0,   336,     0,     0,     0,     0,     0,     0,     0,
       0,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,     0,   165,     0,     0,
     343,     0,     0,     0,     0,     0,     0,     0,     0,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,     0,     0,     0,     0,   443,     0,
       0,     0,     0,     0,     0,     0,     0,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   102,     0,     0,     0,   444,     1,     0,     0,
       2,     0,     3,     0,   103,   104,     0,     4,     0,     0,
       0,     5,     0,     0,     6,     7,     8,     9,   164,     0,
      10,     0,   105,    11,     0,     0,     0,     0,   106,     0,
       0,     0,   164,    12,     0,     0,     0,     0,     0,     0,
     107,     0,     0,     0,     0,   165,   164,     0,   108,   109,
       0,    13,     0,     0,     0,   164,     0,     0,    14,   165,
      15,   100,    16,     0,    17,   110,    18,     0,    19,   111,
       0,     0,     0,   165,     0,     0,     0,    20,   112,    21,
       0,     0,   165,     0,     0,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   164,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   172,   173,
     174,   175,   176,   177,   178,   179,   180,     0,     0,     0,
       0,   165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180
};

static const short yycheck[] =
{
     125,    16,     3,   318,   307,   110,   111,     3,     4,     3,
       9,     5,     9,    84,     8,   354,    32,   122,    13,    14,
      68,    44,    44,   307,     5,     5,     5,     8,    38,     5,
       6,     3,     8,    51,     3,    42,     8,    33,    14,   398,
       3,     3,     5,     6,   107,     8,   349,    54,     3,    18,
      92,     3,    48,     8,    50,   126,    18,     5,   126,   184,
     128,    52,   125,    24,    25,   349,    62,     5,     6,    84,
       8,    86,    87,   121,   413,    21,    14,     5,     6,   362,
       8,    42,    98,    90,    13,    14,    14,     5,     6,     3,
     104,   450,   110,   198,    55,   398,    14,     5,     6,    61,
     439,    13,    14,   126,   126,    68,    14,   103,   118,    61,
     425,    82,   126,    59,   398,    86,    52,    89,    45,   551,
     135,   424,   437,   124,   139,   124,    45,   124,   124,   144,
     124,   256,   147,   148,   149,   130,   117,   117,   117,   571,
     424,   117,    44,    39,   125,   160,   125,   450,   124,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   450,   506,   451,   117,
      66,   454,   455,   456,   457,   458,   459,   125,     3,   117,
       5,     3,     4,     5,     6,    95,     8,     9,   104,   117,
     295,   296,    14,   318,   209,   124,   124,    75,    81,   117,
      19,   130,    24,    81,     5,     6,   124,     8,   547,   117,
     126,     3,   124,    14,   104,   230,     0,    36,   130,   534,
      42,   524,   118,   119,   120,    50,     3,    56,    50,    51,
       5,     6,     9,    55,   123,   538,   126,   362,   253,   254,
     524,    63,     3,    65,     3,     4,     5,     6,   551,     8,
       9,   125,   126,   378,   124,    14,     3,     3,     5,   542,
     543,     8,     3,   388,     3,    24,     3,   570,   571,   394,
       3,    10,    76,    77,   125,   126,   125,   126,   581,    83,
       3,   103,   140,    42,   142,     3,   570,   145,   146,   125,
     126,    50,    51,     3,    41,   117,    55,   125,   126,   121,
     425,   124,   124,   125,    63,   127,    65,    46,   125,   126,
     325,   129,   437,    39,   125,   126,   117,   125,   126,   125,
     126,    68,   125,   126,   124,   340,   451,   342,    67,   454,
     455,   456,   457,   458,   459,   124,     3,     4,     5,     6,
      66,   124,     9,    10,   103,   124,    85,    14,   124,    96,
       3,   125,   126,   124,    21,     3,     4,    24,   117,     3,
       4,   124,   121,   125,   126,   124,    33,   124,   127,   124,
     117,   125,   126,   125,   126,    42,   125,   126,    61,   124,
     395,    48,    49,    50,    51,    33,   124,    95,    55,    33,
     116,   117,   118,   119,   120,   110,    63,   110,    65,    84,
      48,     3,    50,    60,    48,   101,    50,     3,     3,   534,
      15,    79,    94,     3,   125,   110,   125,   542,   543,     3,
       4,     5,     6,   124,    91,     9,    74,   124,     3,     5,
      14,   110,   110,   117,    58,    89,   103,    21,    89,    89,
      24,     3,     4,     5,     6,     8,     8,     9,   104,    33,
     117,   118,    14,     3,   121,   103,     3,   124,    42,   103,
     127,   124,    24,   588,    48,    49,    50,    51,    81,    30,
      50,    55,   131,     3,   126,     3,   125,   124,     5,    63,
      42,    65,    59,   126,     3,     4,     5,     6,    50,    51,
       9,    16,     3,    55,   110,    14,   100,     3,     3,   125,
      87,    63,    21,    65,   125,    24,   125,    91,    82,   110,
      47,     5,   110,   106,    33,    69,   124,   124,   124,   103,
       8,   124,     8,    42,   107,   124,   126,    52,   125,    48,
      49,    50,    51,   117,   118,   126,    55,   121,   124,    64,
     124,   103,     5,   127,    63,   105,    65,    72,    73,     8,
      76,     3,   118,   125,   125,   117,   126,   125,    70,   121,
       8,    51,   124,   125,    89,   127,    26,    47,     3,     4,
       5,     6,    91,     8,     9,    70,   110,   102,   126,    14,
     125,    18,   126,    23,   103,     3,     4,     5,     6,    24,
      26,     9,    61,   125,   107,    68,    14,   107,   117,   118,
      70,    26,   121,     5,   126,   124,    24,    42,   127,    26,
     124,    51,     3,     8,   110,    50,    51,    57,   126,   126,
      55,   124,     5,   125,    42,   125,   537,   120,    63,   348,
      65,   395,    50,    51,   304,   527,   230,    55,   581,   160,
     531,   590,   328,   552,   185,    63,   291,    65,     3,     4,
       5,     6,    39,   293,     9,   437,   318,   484,   363,    14,
      18,   412,   136,    -1,    82,    -1,    -1,    -1,   103,    24,
     110,   111,   112,   113,   114,   115,    -1,    -1,    39,    66,
      -1,   121,   117,    -1,    -1,   103,   121,    42,    -1,   124,
      -1,    -1,   127,    -1,    -1,    50,    51,    -1,    -1,   117,
      55,    -1,    -1,   121,    -1,    66,   124,    39,    63,   127,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    66,    -1,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,    -1,    -1,    -1,   103,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,   117,    66,    39,    -1,   121,    -1,    -1,   124,
      -1,    -1,   127,    -1,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,    66,    39,   125,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    66,
      39,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    66,    39,    -1,
      -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    66,    39,    -1,    -1,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    -1,    66,    39,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    66,    39,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    66,
      39,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    66,    -1,    -1,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    16,    -1,    -1,    -1,   125,    17,    -1,    -1,
      20,    -1,    22,    -1,    28,    29,    -1,    27,    -1,    -1,
      -1,    31,    -1,    -1,    34,    35,    36,    37,    39,    -1,
      40,    -1,    46,    43,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    39,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    66,    39,    -1,    72,    73,
      -1,    71,    -1,    -1,    -1,    39,    -1,    -1,    78,    66,
      80,    85,    82,    -1,    84,    89,    86,    -1,    88,    93,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    97,   102,    99,
      -1,    -1,    66,    -1,    -1,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    39,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    35,    36,    37,
      40,    43,    53,    71,    78,    80,    82,    84,    86,    88,
      97,    99,   133,   134,   135,   136,   137,   138,   145,   182,
     187,   188,   191,   192,   193,   194,   201,   202,   211,   212,
     213,   214,   218,   219,   224,   225,   226,   229,   231,   232,
     233,   234,   235,   237,   238,    92,    52,     3,    45,    44,
      45,    75,    81,    52,     3,     4,     5,     6,     9,    10,
      14,    21,    24,    33,    42,    48,    49,    50,    51,    55,
      63,    65,    91,   103,   117,   118,   121,   124,   127,   146,
     147,   149,   177,   178,   236,   239,     3,    10,    46,    67,
      85,   227,    16,    28,    29,    46,    52,    64,    72,    73,
      89,    93,   102,   184,   227,    95,    81,     3,   150,     0,
     123,    56,     3,     3,     3,   124,     3,   150,     3,     3,
       3,     3,    13,    14,   124,   130,   240,   241,   129,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,     3,    63,    65,   177,   177,   177,     3,   181,    44,
     126,     3,    18,   148,    39,    66,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    61,   169,   170,   110,   110,     3,     9,     3,     5,
       6,     8,    68,   185,   186,    95,     3,     8,    89,    84,
       3,    60,   183,   183,   183,   101,     3,    84,   126,    82,
      86,   136,     3,   183,    15,    94,     5,     6,     8,    14,
     117,   124,   157,   158,   200,   203,   204,    79,   104,     8,
     124,   125,   177,   179,   180,     8,   177,   241,     3,   177,
     179,    38,   118,   179,   125,   177,   179,   179,   177,   177,
     177,   125,   124,   124,   124,   125,   110,   126,   128,   124,
     150,   147,     3,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
       5,     3,     8,    41,    68,    96,   117,   157,   189,   190,
     186,   110,   110,   117,    58,    89,    89,   183,    89,     8,
     104,   220,   221,     3,   215,   216,     3,   124,   195,    30,
      81,     5,     6,     8,   125,   157,   159,   205,   126,   206,
      42,    54,    90,   230,    50,    82,   138,   125,   126,   131,
     131,   125,   125,     3,   125,   125,   125,   125,   125,   125,
     126,   125,   126,   125,   177,   177,   157,     3,   142,   104,
     151,   152,   126,   189,   124,   185,    59,   183,   183,     3,
     222,   223,   110,   126,   152,     3,     4,    33,    48,    50,
     103,   156,   196,   239,   100,     3,     3,   125,   126,   125,
     126,   200,   204,   207,   208,   125,    87,    51,   110,   125,
     180,   125,   177,   177,   110,    82,   145,    62,   124,   153,
     154,   155,   156,    47,   160,     5,   159,    76,    77,    83,
     228,   110,   106,   124,   157,   158,   216,    69,   171,   172,
     124,   124,   124,   125,   126,   124,   197,   198,    24,    25,
      42,    55,   217,   157,     8,    18,   209,   126,     8,   124,
     157,   126,   139,   125,   125,   157,   146,   125,   124,   153,
     107,    23,    51,    57,   110,   111,   112,   113,   114,   115,
     121,     5,   161,   105,   163,   164,   125,    32,    98,    76,
       8,   223,   125,   159,     3,   173,   174,   118,   125,   125,
     156,   124,   199,   200,   126,     3,    61,   210,   200,   208,
     159,     3,     5,    50,   140,   141,   125,    44,    70,   143,
       8,   125,   153,   157,   158,     9,   124,    68,   121,     8,
     157,   158,     8,   157,   158,   157,   158,   157,   158,   157,
     158,   157,   158,    51,    26,    47,    70,   165,   166,   125,
     110,   126,   125,   125,   126,   198,   125,   126,    26,    61,
     144,   125,   107,   107,   159,    68,     9,   124,   156,   162,
      70,    26,   169,     3,     5,     8,   124,   174,   200,   141,
     156,   167,   168,     5,   157,   158,   157,   158,   125,   159,
     126,    26,    74,   167,   171,   124,     3,   175,   176,    19,
      36,   126,   126,   125,   156,   167,   124,     8,   110,   125,
     126,   168,     5,   125,   125,   157,   176
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

  case 95:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 96:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 99:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 100:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 101:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 102:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 103:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 105:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 106:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 107:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 108:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 109:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 110:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 111:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 115:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 116:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 117:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 120:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 123:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 124:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 126:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 128:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 129:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 132:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 133:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 142:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 145:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 146:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 148:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 149:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 154:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 155:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
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

  case 177:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 178:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
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

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 183:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 184:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 185:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 190:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 191:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 194:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 195:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 196:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 197:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 198:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 199:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 200:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 201:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 202:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 203:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 211:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 212:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 213:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 214:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 215:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 216:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 217:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 220:

    { yyval.m_iValue = 1; ;}
    break;

  case 221:

    { yyval.m_iValue = 0; ;}
    break;

  case 222:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 223:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 224:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 225:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 228:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 229:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 230:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 233:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 234:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 237:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 238:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 239:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 240:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 241:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 242:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 243:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 244:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 245:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 246:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 247:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 248:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 249:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 251:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 252:

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

  case 253:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 256:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 258:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 263:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 266:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 267:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 270:

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

  case 271:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 272:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 273:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 274:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 275:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 276:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 277:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 278:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 279:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 286:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 287:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 288:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 296:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 297:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 298:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 299:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 300:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 301:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 302:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 303:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 304:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 307:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 308:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 309:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 311:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 312:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 313:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 314:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 315:

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

