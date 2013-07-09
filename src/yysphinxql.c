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
#define YYLAST   1242

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  132
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  112
/* YYNRULES -- Number of rules. */
#define YYNRULES  319
/* YYNRULES -- Number of states. */
#define YYNSTATES  601

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
     632,   636,   642,   644,   646,   648,   650,   653,   654,   657,
     659,   662,   665,   669,   671,   673,   678,   683,   687,   689,
     691,   693,   695,   697,   699,   703,   708,   713,   718,   722,
     727,   735,   741,   743,   745,   747,   749,   751,   753,   755,
     757,   759,   762,   769,   771,   773,   774,   778,   780,   784,
     786,   790,   794,   796,   800,   802,   804,   806,   810,   813,
     821,   831,   838,   840,   844,   846,   850,   852,   856,   857,
     860,   862,   866,   870,   871,   873,   875,   877,   881,   883,
     885,   889,   896,   898,   902,   906,   910,   916,   921,   923,
     925,   927,   929,   937,   942,   943,   945,   948,   950,   954,
     958,   961,   965,   972,   973,   975,   977,   980,   983,   986,
     988,   996,   998,  1000,  1002,  1006,  1013,  1017,  1021,  1025,
    1027,  1031,  1035,  1039,  1042,  1044,  1047,  1049,  1051,  1055
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     133,     0,    -1,   134,    -1,   135,    -1,   135,   123,    -1,
     195,    -1,   203,    -1,   189,    -1,   190,    -1,   193,    -1,
     204,    -1,   213,    -1,   215,    -1,   216,    -1,   221,    -1,
     226,    -1,   227,    -1,   231,    -1,   233,    -1,   234,    -1,
     235,    -1,   236,    -1,   228,    -1,   237,    -1,   239,    -1,
     240,    -1,   220,    -1,   136,    -1,   135,   123,   136,    -1,
     137,    -1,   184,    -1,   138,    -1,    82,     3,   124,   124,
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
     103,   124,   125,    -1,    50,    -1,   241,    -1,     5,    -1,
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
      -1,   178,    -1,   241,    -1,     3,   124,   179,   125,    -1,
      51,   124,   179,   125,    -1,    55,   124,   179,   125,    -1,
      24,   124,   179,   125,    -1,    42,   124,   179,   125,    -1,
       3,   124,   125,    -1,    65,   124,   177,   126,   177,   125,
      -1,    63,   124,   177,   126,   177,   125,    -1,   103,   124,
     125,    -1,   180,    -1,   179,   126,   180,    -1,   177,    -1,
       8,    -1,   182,   110,   183,    -1,   181,   126,   182,   110,
     183,    -1,     3,    -1,    51,    -1,   157,    -1,     3,    -1,
      86,   186,    -1,    -1,    60,     8,    -1,   102,    -1,    89,
     185,    -1,    64,   185,    -1,    16,    89,   185,    -1,    73,
      -1,    72,    -1,    16,     8,    89,   185,    -1,    16,     3,
      89,   185,    -1,    52,     3,    89,    -1,     3,    -1,    68,
      -1,     8,    -1,     5,    -1,     6,    -1,   187,    -1,   188,
     117,   187,    -1,    84,     3,   110,   192,    -1,    84,     3,
     110,   191,    -1,    84,     3,   110,    68,    -1,    84,    67,
     188,    -1,    84,    10,   110,   188,    -1,    84,    46,     9,
     110,   124,   159,   125,    -1,    84,    46,     3,   110,   191,
      -1,     3,    -1,     8,    -1,    96,    -1,    41,    -1,   157,
      -1,    31,    -1,    80,    -1,   194,    -1,    22,    -1,    88,
      95,    -1,   196,    56,     3,   197,   100,   199,    -1,    53,
      -1,    78,    -1,    -1,   124,   198,   125,    -1,   156,    -1,
     198,   126,   156,    -1,   200,    -1,   199,   126,   200,    -1,
     124,   201,   125,    -1,   202,    -1,   201,   126,   202,    -1,
     157,    -1,   158,    -1,     8,    -1,   124,   159,   125,    -1,
     124,   125,    -1,    35,    44,   150,   104,    50,   110,   157,
      -1,    35,    44,   150,   104,    50,    51,   124,   159,   125,
      -1,    27,     3,   124,   205,   208,   125,    -1,   206,    -1,
     205,   126,   206,    -1,   202,    -1,   124,   207,   125,    -1,
       8,    -1,   207,   126,     8,    -1,    -1,   126,   209,    -1,
     210,    -1,   209,   126,   210,    -1,   202,   211,   212,    -1,
      -1,    18,    -1,     3,    -1,    61,    -1,   214,     3,   185,
      -1,    37,    -1,    36,    -1,    86,    93,   185,    -1,    99,
     150,    84,   217,   152,   171,    -1,   218,    -1,   217,   126,
     218,    -1,     3,   110,   157,    -1,     3,   110,   158,    -1,
       3,   110,   124,   159,   125,    -1,     3,   110,   124,   125,
      -1,    55,    -1,    24,    -1,    42,    -1,    25,    -1,    17,
      92,     3,    15,    30,     3,   219,    -1,    86,   229,   101,
     222,    -1,    -1,   223,    -1,   104,   224,    -1,   225,    -1,
     224,   106,   225,    -1,     3,   110,     8,    -1,    86,    29,
      -1,    86,    28,    84,    -1,    84,   229,    95,    58,    59,
     230,    -1,    -1,    46,    -1,    85,    -1,    76,    98,    -1,
      76,    32,    -1,    77,    76,    -1,    83,    -1,    34,    45,
       3,    79,   232,    87,     8,    -1,    54,    -1,    42,    -1,
      90,    -1,    40,    45,     3,    -1,    20,    52,     3,    94,
      81,     3,    -1,    43,    81,     3,    -1,    43,    75,     3,
      -1,    82,   238,   169,    -1,    10,    -1,    10,   129,     3,
      -1,    97,    81,     3,    -1,    71,    52,     3,    -1,     3,
     242,    -1,   243,    -1,   242,   243,    -1,    13,    -1,    14,
      -1,   130,   177,   131,    -1,   130,     8,   131,    -1
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
     756,   757,   761,   762,   766,   767,   773,   776,   778,   782,
     783,   784,   785,   786,   787,   788,   793,   798,   808,   809,
     810,   811,   812,   816,   817,   821,   826,   831,   836,   837,
     841,   846,   854,   855,   859,   860,   861,   875,   876,   877,
     881,   882,   888,   896,   897,   900,   902,   906,   907,   911,
     912,   916,   920,   921,   925,   926,   927,   928,   929,   935,
     943,   957,   965,   969,   976,   977,   984,   994,  1000,  1002,
    1006,  1011,  1015,  1022,  1024,  1028,  1029,  1035,  1043,  1044,
    1050,  1056,  1064,  1065,  1069,  1083,  1089,  1093,  1103,  1104,
    1105,  1106,  1110,  1123,  1129,  1130,  1134,  1138,  1139,  1143,
    1147,  1154,  1161,  1167,  1168,  1169,  1173,  1174,  1175,  1176,
    1182,  1193,  1194,  1195,  1199,  1210,  1222,  1231,  1242,  1250,
    1251,  1257,  1268,  1279,  1282,  1283,  1287,  1288,  1289,  1290
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
  "hash_key", "hash_val", "show_stmt", "like_filter", "show_what", 
  "simple_set_value", "set_value", "set_stmt", "set_global_stmt", 
  "set_string_value", "boolean_value", "transact_op", "start_transaction", 
  "insert_into", "insert_or_replace", "opt_column_list", "column_list", 
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
     181,   181,   182,   182,   183,   183,   184,   185,   185,   186,
     186,   186,   186,   186,   186,   186,   186,   186,   187,   187,
     187,   187,   187,   188,   188,   189,   189,   189,   189,   189,
     190,   190,   191,   191,   192,   192,   192,   193,   193,   193,
     194,   194,   195,   196,   196,   197,   197,   198,   198,   199,
     199,   200,   201,   201,   202,   202,   202,   202,   202,   203,
     203,   204,   205,   205,   206,   206,   207,   207,   208,   208,
     209,   209,   210,   211,   211,   212,   212,   213,   214,   214,
     215,   216,   217,   217,   218,   218,   218,   218,   219,   219,
     219,   219,   220,   221,   222,   222,   223,   224,   224,   225,
     226,   227,   228,   229,   229,   229,   230,   230,   230,   230,
     231,   232,   232,   232,   233,   234,   235,   236,   237,   238,
     238,   239,   240,   241,   242,   242,   243,   243,   243,   243
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
       3,     5,     1,     1,     1,     1,     2,     0,     2,     1,
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
       3,     3,     3,     2,     1,     2,     1,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   230,     0,   227,     0,     0,   269,   268,
       0,     0,   233,     0,   234,   228,     0,   293,   293,     0,
       0,     0,     0,     2,     3,    27,    29,    31,    33,    30,
       7,     8,     9,   229,     5,     0,     6,    10,    11,     0,
      12,    13,    26,    14,    15,    16,    22,    17,    18,    19,
      20,    21,    23,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   147,   148,   150,   151,   153,   309,
     152,     0,     0,     0,     0,     0,     0,   149,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
      48,    52,    55,   175,   130,   176,     0,     0,   294,     0,
     295,     0,     0,     0,   290,   294,     0,   197,   204,   203,
     197,   197,   199,   196,     0,   231,     0,    64,     0,     1,
       4,     0,   197,     0,     0,     0,     0,     0,   304,   307,
     306,   312,   316,   317,     0,     0,   313,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,     0,     0,   154,   155,     0,   192,   193,     0,
       0,     0,     0,    53,     0,    51,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   308,   131,     0,     0,     0,     0,
     208,   211,   212,   210,   209,   213,   218,     0,     0,     0,
     197,   291,     0,     0,   201,   200,   270,   284,   311,     0,
       0,     0,     0,    28,   235,   267,     0,     0,   105,   107,
     246,   109,     0,     0,   244,   245,   254,   258,   252,     0,
       0,   189,     0,   182,   188,     0,   186,     0,     0,   315,
     310,     0,     0,     0,     0,     0,    62,     0,     0,     0,
       0,     0,     0,   185,     0,     0,     0,   173,     0,   174,
       0,    42,    66,    49,    54,   165,   166,   172,   171,   163,
     162,   169,   170,   160,   161,   168,   167,   156,   157,   158,
     159,   164,   132,   222,   223,   225,   217,   224,     0,   226,
     216,   215,   219,     0,     0,     0,     0,   197,   197,   202,
     207,   198,     0,   283,   285,     0,     0,   272,    65,     0,
       0,     0,     0,   106,   108,   256,   248,   110,     0,     0,
       0,     0,   302,   301,   303,     0,     0,     0,     0,   177,
       0,   319,   318,    56,   180,     0,    61,   181,    60,   178,
     179,    57,     0,    58,     0,    59,     0,     0,     0,   195,
     194,   190,     0,     0,   112,    67,     0,   221,     0,   214,
       0,   206,   205,     0,   286,   287,     0,     0,   134,    98,
      99,     0,     0,   103,     0,   237,     0,   104,     0,     0,
     305,   247,     0,   255,     0,   254,   253,   259,   260,   251,
       0,     0,     0,    35,   187,    63,     0,     0,     0,     0,
       0,     0,     0,    68,    69,    85,     0,   114,   118,   133,
       0,     0,     0,   299,   292,     0,     0,     0,   274,   275,
     273,     0,   271,   135,     0,     0,     0,   236,     0,     0,
     232,   239,   279,   281,   280,   278,   282,   111,   257,   264,
       0,     0,   300,     0,   249,     0,     0,   184,   183,   191,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   115,     0,     0,   121,   119,
     220,   297,   296,   298,   289,   288,   277,     0,     0,   136,
     137,     0,   101,   102,   238,     0,     0,   242,     0,   265,
     266,   262,   263,   261,     0,    39,    40,    41,    36,    37,
      32,     0,     0,    44,     0,    71,    70,     0,     0,    77,
       0,    95,     0,    93,    73,    84,    94,    74,    97,    81,
      90,    80,    89,    82,    91,    83,    92,     0,     0,     0,
       0,   130,   122,   276,     0,     0,   100,   241,     0,   240,
     250,     0,     0,     0,    34,    72,     0,     0,     0,    96,
      78,     0,   116,   113,     0,     0,   134,   139,   140,   143,
       0,   138,   243,    38,   127,    43,   125,    45,    79,    87,
      88,    86,    75,     0,     0,     0,     0,   123,    47,     0,
       0,     0,   144,   128,   129,     0,     0,    76,   117,   120,
       0,     0,     0,   141,     0,   126,    46,   124,   142,   146,
     145
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   446,   498,   499,
     352,   503,   544,    28,    89,    90,   165,    91,   262,   354,
     355,   403,   404,   405,   564,   317,   225,   318,   408,   466,
     553,   468,   469,   531,   532,   565,   566,   184,   185,   422,
     423,   479,   480,   581,   582,   234,    93,   235,   236,   159,
     160,   351,    29,   204,   113,   195,   196,    30,    31,   290,
     291,    32,    33,    34,    35,   310,   376,   430,   431,   486,
     226,    36,    37,   227,   228,   319,   321,   387,   388,   440,
     491,    38,    39,    40,    41,   306,   307,   436,    42,    43,
     303,   304,   364,   365,    44,    45,    46,   101,   414,    47,
     325,    48,    49,    50,    51,    52,    94,    53,    54,    95,
     136,   137
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -510
static const short yypact[] =
{
    1002,   -23,    79,  -510,   117,  -510,   139,   160,  -510,  -510,
     162,   190,  -510,   165,  -510,  -510,   277,   208,   998,   131,
     157,   230,   263,  -510,   125,  -510,  -510,  -510,  -510,  -510,
    -510,  -510,  -510,  -510,  -510,   222,  -510,  -510,  -510,   289,
    -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,
    -510,  -510,  -510,  -510,  -510,   294,   303,   127,   306,   230,
     315,   317,   320,   328,    19,  -510,  -510,  -510,  -510,   215,
    -510,   233,   235,   243,   259,   264,   265,  -510,   275,   276,
     279,   282,   284,   285,   487,  -510,   487,   487,    36,   -19,
    -510,   224,  1001,  -510,   288,  -510,   240,   300,   299,    51,
    -510,   316,    26,   330,  -510,  -510,   409,   355,  -510,  -510,
     355,   355,  -510,  -510,   318,  -510,   413,  -510,   -34,  -510,
     225,   414,   355,   403,   326,    74,   344,   -52,  -510,  -510,
    -510,  -510,  -510,  -510,   444,   575,    -2,  -510,   421,   487,
     592,    20,   592,   305,   487,   592,   592,   487,   487,   487,
     307,    31,   310,   311,  -510,  -510,   753,  -510,  -510,   138,
     332,     1,   342,  -510,   441,  -510,   487,   487,   487,   487,
     487,   487,   487,   487,   487,   487,   487,   487,   487,   487,
     487,   487,   487,   446,  -510,  -510,   178,    51,   346,   347,
    -510,  -510,  -510,  -510,  -510,  -510,   337,   404,   372,   376,
     355,  -510,   385,   467,  -510,  -510,  -510,   374,  -510,   473,
     476,   422,   183,  -510,   356,  -510,   454,   407,  -510,  -510,
    -510,  -510,   152,    35,  -510,  -510,  -510,   371,  -510,   171,
     439,  -510,    13,  -510,  1001,   151,  -510,   367,   614,  -510,
    -510,   781,   164,   497,   377,   188,  -510,   809,   196,   227,
     640,   669,   837,  -510,   512,   487,   487,  -510,    36,  -510,
      68,  -510,   -37,  -510,  -510,  -510,  -510,  1015,  1029,  1097,
    1040,  1122,  1122,   321,   321,   321,   321,   363,   363,  -510,
    -510,  -510,   378,  -510,  -510,  -510,  -510,  -510,   498,  -510,
    -510,  -510,   337,    82,   381,    51,   447,   355,   355,  -510,
    -510,  -510,   505,  -510,  -510,   400,    76,  -510,  -510,   212,
     412,   511,   516,  -510,  -510,  -510,  -510,  -510,   229,   236,
      74,   397,  -510,  -510,  -510,   436,    11,   342,   399,  -510,
     592,  -510,  -510,  -510,  -510,   402,  -510,  -510,  -510,  -510,
    -510,  -510,   487,  -510,   487,  -510,   697,   725,   418,  -510,
    -510,  -510,   448,     3,   484,  -510,   527,  -510,    25,  -510,
     167,  -510,  -510,   423,   428,  -510,     9,   473,   466,    -2,
    -510,   417,   420,  -510,   424,  -510,   239,  -510,   427,   168,
    -510,  -510,    25,  -510,   537,    90,  -510,   429,  -510,  -510,
     545,   432,    25,   431,  -510,  -510,   865,   893,    68,   342,
     433,   435,     3,   453,  -510,  -510,   562,   559,   461,  -510,
     244,    16,   494,  -510,  -510,   564,   505,    37,  -510,  -510,
    -510,   570,  -510,  -510,   456,   451,   457,  -510,   212,   104,
     460,  -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,
     126,   104,  -510,    25,  -510,   234,   462,  -510,  -510,  -510,
      -9,   518,   583,   105,     3,   200,    15,   -30,   195,   226,
     200,   200,   200,   200,   541,  -510,   567,   547,   532,  -510,
    -510,  -510,  -510,  -510,  -510,  -510,  -510,   246,   493,   479,
    -510,   482,  -510,  -510,  -510,    65,   248,  -510,   427,  -510,
    -510,  -510,   591,  -510,   251,  -510,  -510,  -510,   486,  -510,
    -510,   230,   584,   557,   495,  -510,  -510,   514,   515,  -510,
      25,  -510,   555,  -510,  -510,  -510,  -510,  -510,  -510,  -510,
    -510,  -510,  -510,  -510,  -510,  -510,  -510,    32,   212,   554,
     601,   288,  -510,  -510,    23,   570,  -510,  -510,   104,  -510,
    -510,   234,   212,   623,  -510,  -510,   200,   200,   253,  -510,
    -510,    25,  -510,   506,   605,   255,   466,   517,  -510,  -510,
     632,  -510,  -510,  -510,    58,   519,  -510,   520,  -510,  -510,
    -510,  -510,  -510,   256,   212,   212,   524,   519,  -510,   636,
     539,   260,  -510,  -510,  -510,   212,   645,  -510,  -510,   519,
     526,   529,    25,  -510,   632,  -510,  -510,  -510,  -510,  -510,
    -510
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -510,  -510,  -510,  -510,   536,  -510,   426,  -510,  -510,   111,
    -510,  -510,  -510,   308,   262,   500,  -510,  -510,    54,  -510,
     353,  -382,  -510,  -510,  -304,  -125,  -358,  -357,  -510,  -510,
    -510,  -510,  -510,  -510,  -510,  -509,    78,   133,  -510,   109,
    -510,  -510,   132,  -510,    72,    -3,  -510,   154,   338,  -510,
     411,   272,  -510,  -101,  -510,   386,   499,  -510,  -510,   389,
    -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,   197,  -510,
    -318,  -510,  -510,  -510,   351,  -510,  -510,  -510,   247,  -510,
    -510,  -510,  -510,  -510,  -510,  -510,   322,  -510,  -510,  -510,
    -510,  -510,  -510,   268,  -510,  -510,  -510,   672,  -510,  -510,
    -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,  -510,  -306,
    -510,   551
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -264
static const short yytable[] =
{
     224,   410,   385,   377,   117,   375,   369,   370,   419,   205,
     206,   132,   133,    92,   218,   219,   151,    65,    66,    67,
     453,   215,    68,   221,   509,   161,   557,    70,   558,   198,
     218,   559,   132,   133,   199,   501,   371,    72,   511,   157,
     218,   550,   218,   315,   132,   133,   577,   377,   471,   406,
     209,   372,   230,   373,   190,    74,   191,   192,   243,   193,
     477,   289,   391,    77,    78,   401,   589,   353,    79,    55,
     218,   349,   506,   218,   210,   118,   152,   583,   153,   218,
     219,   154,   220,   155,   156,   283,   494,   158,   221,   210,
     284,   512,   210,  -263,   584,   327,   377,   508,   406,   299,
     515,   518,   520,   522,   524,   526,   374,   162,   439,   218,
     219,   487,   220,   127,   472,   200,    83,   162,   221,   194,
      57,   392,   377,   492,   484,   261,   222,   402,   135,   489,
      84,    56,   238,   417,    86,   350,   241,    87,   244,   510,
      88,   247,   288,   134,   250,   251,   252,   560,   377,   135,
     406,  -263,   288,   548,   288,   254,   551,   313,   314,    92,
     316,   135,   476,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     353,   283,   288,   218,    58,   288,   284,   490,   569,   571,
     316,   222,   432,   433,   573,   224,   361,   362,   223,   102,
     218,   219,   367,   513,    59,   218,   219,    60,    92,   221,
     434,    96,   454,   322,   221,   369,   370,    63,    97,   285,
     562,   222,   377,   435,   552,   323,   115,   163,   485,   156,
     505,   218,   219,   117,   516,   106,   377,   495,   116,   496,
     221,   418,   164,   411,   412,   371,   286,   107,   120,   377,
     413,   125,   346,   347,    98,   108,   109,   437,   369,   370,
     372,   324,   373,   119,   258,    61,   259,   444,   377,   377,
     588,    62,   110,   350,   287,    99,   329,   330,   121,   377,
      64,    65,    66,    67,   497,   112,    68,    69,   371,   334,
     330,    70,   122,   100,   242,   288,   245,   123,    71,   248,
     249,    72,   188,   372,   224,   373,   124,   211,   189,   126,
      73,   212,   222,   337,   330,   374,   224,   222,   128,    74,
     129,   339,   330,   130,    92,    75,    76,    77,    78,   576,
     507,   131,    79,   514,   517,   519,   521,   523,   525,   396,
      80,   397,    81,   222,   138,   151,    65,    66,    67,   183,
     186,    68,   340,   330,   381,   382,    70,   139,   374,   140,
     166,   383,   384,    71,   427,   428,    72,   141,    82,   470,
     382,   533,   382,   537,   538,    73,   540,   382,   572,   382,
      83,   587,   382,   142,    74,   593,   594,   167,   143,   144,
      75,    76,    77,    78,    84,    85,    92,    79,    86,   145,
     146,    87,   166,   147,    88,    80,   148,    81,   149,   150,
     187,   197,   202,   224,   201,   203,   208,   214,   216,   207,
     217,   568,   570,   229,   240,    64,    65,    66,    67,   167,
     246,    68,   253,    82,   255,   256,    70,   178,   179,   180,
     181,   182,   260,    71,   264,    83,    72,   151,    65,    66,
      67,   282,   231,    68,   295,    73,   293,   294,    70,    84,
      85,   297,   296,    86,    74,   298,    87,   599,    72,    88,
      75,    76,    77,    78,   300,   301,   305,    79,   302,   308,
     309,   180,   181,   182,   311,    80,    74,    81,   312,   326,
     151,    65,    66,    67,    77,    78,    68,   320,   331,    79,
     335,    70,   336,   313,   356,   358,   360,   152,   363,   153,
     366,    72,   378,    82,   379,   151,    65,    66,    67,   380,
     231,    68,   389,   390,   393,    83,    70,   395,   398,    74,
     399,   407,   409,   415,   416,   421,    72,    77,    78,    84,
      85,   424,    79,    86,   425,   438,    87,    83,   426,    88,
     152,   429,   153,   442,    74,   441,   443,   445,   451,   452,
     454,    84,    77,    78,   465,    86,   467,    79,   232,   233,
     473,    88,   474,   478,   481,   152,   482,   153,   151,    65,
      66,    67,   483,   237,    68,   455,   488,   500,   502,    70,
      83,   504,   527,   528,   529,   151,    65,    66,    67,    72,
     231,    68,   530,   534,    84,   535,    70,   536,    86,   439,
     542,    87,   541,   456,    88,    83,    72,    74,   543,   457,
     545,   546,   547,   549,   554,    77,    78,   555,   567,    84,
      79,   575,   574,    86,    74,   580,    87,   233,   152,    88,
     153,   579,    77,    78,   591,   585,   586,    79,   590,   592,
     596,   597,   563,   166,   598,   152,   213,   153,   328,   368,
     400,   450,   263,   595,   556,   578,   600,   561,   394,   348,
     449,   386,   458,   459,   460,   461,   462,   463,    83,   166,
     167,   359,   357,   464,   475,   539,   292,   239,   493,   420,
     114,     0,    84,     0,     0,    83,    86,     0,     0,    87,
       0,     0,    88,     0,     0,     0,   167,     0,   166,    84,
       0,     0,     0,    86,     0,     0,    87,     0,     0,    88,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   167,   166,     0,     0,     0,
       0,     0,     0,     0,     0,   332,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,     0,     0,   167,   166,   341,   342,     0,     0,     0,
       0,     0,     0,     0,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
       0,   167,   166,     0,   343,   344,     0,     0,     0,     0,
       0,     0,     0,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,     0,   167,
     166,     0,     0,   342,     0,     0,     0,     0,     0,     0,
       0,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,     0,   167,   166,     0,
       0,   344,     0,     0,     0,     0,     0,     0,     0,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,     0,   167,   166,     0,   257,     0,
       0,     0,     0,     0,     0,     0,     0,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,     0,   167,   166,     0,   333,     0,     0,     0,
       0,     0,     0,     0,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
       0,   167,   166,     0,   338,     0,     0,     0,     0,     0,
       0,     0,     0,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,     0,   167,
       0,     0,   345,     0,     0,     0,     0,     0,     0,     0,
       0,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,     0,     0,     0,     0,
     447,     0,     0,     0,     0,     0,     0,     0,     0,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   102,     0,     0,     0,   448,     1,
       0,     0,     2,     0,     3,     0,   103,   104,     0,     4,
       0,     0,     0,     5,     0,     0,     6,     7,     8,     9,
     166,     0,    10,     0,   105,    11,     0,     0,     0,     0,
     106,     0,     0,     0,   166,    12,     0,     0,     0,     0,
       0,     0,   107,     0,     0,     0,     0,   167,   166,     0,
     108,   109,     0,    13,     0,     0,     0,     0,     0,   166,
      14,   167,    15,   100,    16,     0,    17,   110,    18,     0,
      19,   111,     0,     0,     0,   167,     0,     0,     0,    20,
     112,    21,     0,     0,     0,     0,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   166,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   166,     0,   167,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   167,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   174,   175,   176,   177,   178,   179,
     180,   181,   182
};

static const short yycheck[] =
{
     125,   358,   320,   309,     3,   309,     3,     4,   366,   110,
     111,    13,    14,    16,     5,     6,     3,     4,     5,     6,
     402,   122,     9,    14,     9,    44,     3,    14,     5,     3,
       5,     8,    13,    14,     8,    44,    33,    24,    68,     3,
       5,     9,     5,     8,    13,    14,   555,   353,    32,   353,
      84,    48,   104,    50,     3,    42,     5,     6,    38,     8,
     417,   186,    51,    50,    51,    62,   575,   104,    55,    92,
       5,     3,   454,     5,   126,    21,    63,    19,    65,     5,
       6,    84,     8,    86,    87,     3,   443,    51,    14,   126,
       8,   121,   126,     3,    36,    82,   402,   455,   402,   200,
     458,   459,   460,   461,   462,   463,   103,   126,    18,     5,
       6,   429,     8,    59,    98,    89,   103,   126,    14,    68,
       3,   110,   428,   441,   428,   124,   117,   124,   130,     3,
     117,    52,   135,   124,   121,   260,   139,   124,   118,   124,
     127,   144,   117,   124,   147,   148,   149,   124,   454,   130,
     454,    61,   117,   510,   117,   124,   124,     5,     6,   162,
     125,   130,   125,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     104,     3,   117,     5,    45,   117,     8,    61,   546,   547,
     125,   117,    24,    25,   551,   320,   297,   298,   124,    16,
       5,     6,   126,     8,    44,     5,     6,    45,   211,    14,
      42,     3,   107,    42,    14,     3,     4,    52,    10,    41,
     538,   117,   528,    55,   528,    54,    95,     3,   124,   232,
     125,     5,     6,     3,     8,    52,   542,     3,    81,     5,
      14,   366,    18,    76,    77,    33,    68,    64,   123,   555,
      83,   124,   255,   256,    46,    72,    73,   382,     3,     4,
      48,    90,    50,     0,   126,    75,   128,   392,   574,   575,
     574,    81,    89,   398,    96,    67,   125,   126,    56,   585,
       3,     4,     5,     6,    50,   102,     9,    10,    33,   125,
     126,    14,     3,    85,   140,   117,   142,     3,    21,   145,
     146,    24,     3,    48,   429,    50,     3,    82,     9,     3,
      33,    86,   117,   125,   126,   103,   441,   117,     3,    42,
       3,   125,   126,     3,   327,    48,    49,    50,    51,    74,
     455,     3,    55,   458,   459,   460,   461,   462,   463,   342,
      63,   344,    65,   117,   129,     3,     4,     5,     6,    61,
     110,     9,   125,   126,   125,   126,    14,   124,   103,   124,
      39,   125,   126,    21,   125,   126,    24,   124,    91,   125,
     126,   125,   126,   125,   126,    33,   125,   126,   125,   126,
     103,   125,   126,   124,    42,   125,   126,    66,   124,   124,
      48,    49,    50,    51,   117,   118,   399,    55,   121,   124,
     124,   124,    39,   124,   127,    63,   124,    65,   124,   124,
     110,    95,     3,   538,    84,    60,     3,     3,    15,   101,
      94,   546,   547,    79,     3,     3,     4,     5,     6,    66,
     125,     9,   125,    91,   124,   124,    14,   116,   117,   118,
     119,   120,   110,    21,     3,   103,    24,     3,     4,     5,
       6,     5,     8,     9,   117,    33,   110,   110,    14,   117,
     118,    89,    58,   121,    42,    89,   124,   592,    24,   127,
      48,    49,    50,    51,    89,     8,     3,    55,   104,     3,
     124,   118,   119,   120,    30,    63,    42,    65,    81,    50,
       3,     4,     5,     6,    50,    51,     9,   126,   131,    55,
       3,    14,   125,     5,   126,   124,    59,    63,     3,    65,
     110,    24,   100,    91,     3,     3,     4,     5,     6,     3,
       8,     9,   125,    87,   125,   103,    14,   125,   110,    42,
      82,    47,     5,   110,   106,    69,    24,    50,    51,   117,
     118,   124,    55,   121,   124,     8,   124,   103,   124,   127,
      63,   124,    65,     8,    42,   126,   124,   126,   125,   124,
     107,   117,    50,    51,     5,   121,   105,    55,   124,   125,
      76,   127,     8,     3,   118,    63,   125,    65,     3,     4,
       5,     6,   125,     8,     9,    23,   126,   125,    70,    14,
     103,     8,    51,    26,    47,     3,     4,     5,     6,    24,
       8,     9,    70,   110,   117,   126,    14,   125,   121,    18,
      26,   124,   126,    51,   127,   103,    24,    42,    61,    57,
     125,   107,   107,    68,    70,    50,    51,    26,     5,   117,
      55,    26,   126,   121,    42,     3,   124,   125,    63,   127,
      65,   124,    50,    51,     8,   126,   126,    55,   124,   110,
       5,   125,   541,    39,   125,    63,   120,    65,   232,   306,
     352,   399,   162,   585,   531,   556,   594,   535,   330,   258,
     398,   320,   110,   111,   112,   113,   114,   115,   103,    39,
      66,   295,   293,   121,   416,   488,   187,   136,   441,   367,
      18,    -1,   117,    -1,    -1,   103,   121,    -1,    -1,   124,
      -1,    -1,   127,    -1,    -1,    -1,    66,    -1,    39,   117,
      -1,    -1,    -1,   121,    -1,    -1,   124,    -1,    -1,   127,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    66,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,    66,    39,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    66,    39,    -1,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    66,
      39,    -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    66,    39,    -1,
      -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    66,    39,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    -1,    66,    39,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    66,    39,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    66,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    16,    -1,    -1,    -1,   125,    17,
      -1,    -1,    20,    -1,    22,    -1,    28,    29,    -1,    27,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      39,    -1,    40,    -1,    46,    43,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    39,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,    66,    39,    -1,
      72,    73,    -1,    71,    -1,    -1,    -1,    -1,    -1,    39,
      78,    66,    80,    85,    82,    -1,    84,    89,    86,    -1,
      88,    93,    -1,    -1,    -1,    66,    -1,    -1,    -1,    97,
     102,    99,    -1,    -1,    -1,    -1,    66,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    39,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    39,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
     118,   119,   120
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    35,    36,    37,
      40,    43,    53,    71,    78,    80,    82,    84,    86,    88,
      97,    99,   133,   134,   135,   136,   137,   138,   145,   184,
     189,   190,   193,   194,   195,   196,   203,   204,   213,   214,
     215,   216,   220,   221,   226,   227,   228,   231,   233,   234,
     235,   236,   237,   239,   240,    92,    52,     3,    45,    44,
      45,    75,    81,    52,     3,     4,     5,     6,     9,    10,
      14,    21,    24,    33,    42,    48,    49,    50,    51,    55,
      63,    65,    91,   103,   117,   118,   121,   124,   127,   146,
     147,   149,   177,   178,   238,   241,     3,    10,    46,    67,
      85,   229,    16,    28,    29,    46,    52,    64,    72,    73,
      89,    93,   102,   186,   229,    95,    81,     3,   150,     0,
     123,    56,     3,     3,     3,   124,     3,   150,     3,     3,
       3,     3,    13,    14,   124,   130,   242,   243,   129,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,     3,    63,    65,   177,   177,   177,     3,    51,   181,
     182,    44,   126,     3,    18,   148,    39,    66,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    61,   169,   170,   110,   110,     3,     9,
       3,     5,     6,     8,    68,   187,   188,    95,     3,     8,
      89,    84,     3,    60,   185,   185,   185,   101,     3,    84,
     126,    82,    86,   136,     3,   185,    15,    94,     5,     6,
       8,    14,   117,   124,   157,   158,   202,   205,   206,    79,
     104,     8,   124,   125,   177,   179,   180,     8,   177,   243,
       3,   177,   179,    38,   118,   179,   125,   177,   179,   179,
     177,   177,   177,   125,   124,   124,   124,   125,   126,   128,
     110,   124,   150,   147,     3,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,     5,     3,     8,    41,    68,    96,   117,   157,
     191,   192,   188,   110,   110,   117,    58,    89,    89,   185,
      89,     8,   104,   222,   223,     3,   217,   218,     3,   124,
     197,    30,    81,     5,     6,     8,   125,   157,   159,   207,
     126,   208,    42,    54,    90,   232,    50,    82,   138,   125,
     126,   131,   131,   125,   125,     3,   125,   125,   125,   125,
     125,   125,   126,   125,   126,   125,   177,   177,   182,     3,
     157,   183,   142,   104,   151,   152,   126,   191,   124,   187,
      59,   185,   185,     3,   224,   225,   110,   126,   152,     3,
       4,    33,    48,    50,   103,   156,   198,   241,   100,     3,
       3,   125,   126,   125,   126,   202,   206,   209,   210,   125,
      87,    51,   110,   125,   180,   125,   177,   177,   110,    82,
     145,    62,   124,   153,   154,   155,   156,    47,   160,     5,
     159,    76,    77,    83,   230,   110,   106,   124,   157,   158,
     218,    69,   171,   172,   124,   124,   124,   125,   126,   124,
     199,   200,    24,    25,    42,    55,   219,   157,     8,    18,
     211,   126,     8,   124,   157,   126,   139,   125,   125,   183,
     146,   125,   124,   153,   107,    23,    51,    57,   110,   111,
     112,   113,   114,   115,   121,     5,   161,   105,   163,   164,
     125,    32,    98,    76,     8,   225,   125,   159,     3,   173,
     174,   118,   125,   125,   156,   124,   201,   202,   126,     3,
      61,   212,   202,   210,   159,     3,     5,    50,   140,   141,
     125,    44,    70,   143,     8,   125,   153,   157,   158,     9,
     124,    68,   121,     8,   157,   158,     8,   157,   158,   157,
     158,   157,   158,   157,   158,   157,   158,    51,    26,    47,
      70,   165,   166,   125,   110,   126,   125,   125,   126,   200,
     125,   126,    26,    61,   144,   125,   107,   107,   159,    68,
       9,   124,   156,   162,    70,    26,   169,     3,     5,     8,
     124,   174,   202,   141,   156,   167,   168,     5,   157,   158,
     157,   158,   125,   159,   126,    26,    74,   167,   171,   124,
       3,   175,   176,    19,    36,   126,   126,   125,   156,   167,
     124,     8,   110,   125,   126,   168,     5,   125,   125,   157,
     176
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

  case 198:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 199:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 200:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 205:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 206:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 207:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 215:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 216:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 217:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 218:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 219:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 220:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 221:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 224:

    { yyval.m_iValue = 1; ;}
    break;

  case 225:

    { yyval.m_iValue = 0; ;}
    break;

  case 226:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 227:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 228:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 229:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 232:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 233:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 234:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 237:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 238:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 241:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 242:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 243:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 244:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 245:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 246:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 247:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 248:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 249:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 250:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 251:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 252:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 253:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 255:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 256:

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

  case 257:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 260:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 262:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 267:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 270:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 271:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 274:

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

  case 275:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 276:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 277:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 278:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 279:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 280:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 281:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 282:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 283:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 290:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 291:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 292:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 300:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 301:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 302:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 303:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 304:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 305:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 306:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 307:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 308:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 311:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 312:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 313:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 315:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 316:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 317:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 318:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 319:

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

