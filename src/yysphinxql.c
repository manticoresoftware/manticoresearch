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
     TOK_FROM = 300,
     TOK_FUNCTION = 301,
     TOK_GLOBAL = 302,
     TOK_GROUP = 303,
     TOK_GROUPBY = 304,
     TOK_GROUP_CONCAT = 305,
     TOK_ID = 306,
     TOK_IN = 307,
     TOK_INDEX = 308,
     TOK_INSERT = 309,
     TOK_INT = 310,
     TOK_INTEGER = 311,
     TOK_INTO = 312,
     TOK_IS = 313,
     TOK_ISOLATION = 314,
     TOK_LEVEL = 315,
     TOK_LIKE = 316,
     TOK_LIMIT = 317,
     TOK_MATCH = 318,
     TOK_MAX = 319,
     TOK_META = 320,
     TOK_MIN = 321,
     TOK_MOD = 322,
     TOK_NAMES = 323,
     TOK_NULL = 324,
     TOK_OPTION = 325,
     TOK_ORDER = 326,
     TOK_OPTIMIZE = 327,
     TOK_PLAN = 328,
     TOK_PROFILE = 329,
     TOK_RAND = 330,
     TOK_RAMCHUNK = 331,
     TOK_READ = 332,
     TOK_REPEATABLE = 333,
     TOK_REPLACE = 334,
     TOK_RETURNS = 335,
     TOK_ROLLBACK = 336,
     TOK_RTINDEX = 337,
     TOK_SELECT = 338,
     TOK_SERIALIZABLE = 339,
     TOK_SET = 340,
     TOK_SESSION = 341,
     TOK_SHOW = 342,
     TOK_SONAME = 343,
     TOK_START = 344,
     TOK_STATUS = 345,
     TOK_STRING = 346,
     TOK_SUM = 347,
     TOK_TABLE = 348,
     TOK_TABLES = 349,
     TOK_TO = 350,
     TOK_TRANSACTION = 351,
     TOK_TRUE = 352,
     TOK_TRUNCATE = 353,
     TOK_UNCOMMITTED = 354,
     TOK_UPDATE = 355,
     TOK_VALUES = 356,
     TOK_VARIABLES = 357,
     TOK_WARNINGS = 358,
     TOK_WEIGHT = 359,
     TOK_WHERE = 360,
     TOK_WITHIN = 361,
     TOK_OR = 362,
     TOK_AND = 363,
     TOK_NE = 364,
     TOK_GTE = 365,
     TOK_LTE = 366,
     TOK_NOT = 367,
     TOK_NEG = 368
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
#define TOK_FROM 300
#define TOK_FUNCTION 301
#define TOK_GLOBAL 302
#define TOK_GROUP 303
#define TOK_GROUPBY 304
#define TOK_GROUP_CONCAT 305
#define TOK_ID 306
#define TOK_IN 307
#define TOK_INDEX 308
#define TOK_INSERT 309
#define TOK_INT 310
#define TOK_INTEGER 311
#define TOK_INTO 312
#define TOK_IS 313
#define TOK_ISOLATION 314
#define TOK_LEVEL 315
#define TOK_LIKE 316
#define TOK_LIMIT 317
#define TOK_MATCH 318
#define TOK_MAX 319
#define TOK_META 320
#define TOK_MIN 321
#define TOK_MOD 322
#define TOK_NAMES 323
#define TOK_NULL 324
#define TOK_OPTION 325
#define TOK_ORDER 326
#define TOK_OPTIMIZE 327
#define TOK_PLAN 328
#define TOK_PROFILE 329
#define TOK_RAND 330
#define TOK_RAMCHUNK 331
#define TOK_READ 332
#define TOK_REPEATABLE 333
#define TOK_REPLACE 334
#define TOK_RETURNS 335
#define TOK_ROLLBACK 336
#define TOK_RTINDEX 337
#define TOK_SELECT 338
#define TOK_SERIALIZABLE 339
#define TOK_SET 340
#define TOK_SESSION 341
#define TOK_SHOW 342
#define TOK_SONAME 343
#define TOK_START 344
#define TOK_STATUS 345
#define TOK_STRING 346
#define TOK_SUM 347
#define TOK_TABLE 348
#define TOK_TABLES 349
#define TOK_TO 350
#define TOK_TRANSACTION 351
#define TOK_TRUE 352
#define TOK_TRUNCATE 353
#define TOK_UNCOMMITTED 354
#define TOK_UPDATE 355
#define TOK_VALUES 356
#define TOK_VARIABLES 357
#define TOK_WARNINGS 358
#define TOK_WEIGHT 359
#define TOK_WHERE 360
#define TOK_WITHIN 361
#define TOK_OR 362
#define TOK_AND 363
#define TOK_NE 364
#define TOK_GTE 365
#define TOK_LTE 366
#define TOK_NOT 367
#define TOK_NEG 368




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
#define YYLAST   1359

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  133
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  114
/* YYNRULES -- Number of rules. */
#define YYNRULES  328
/* YYNRULES -- Number of states. */
#define YYNSTATES  616

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   368

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   121,   110,     2,
     125,   126,   119,   117,   127,   118,   130,   120,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   124,
     113,   111,   114,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   131,     2,   132,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   128,   109,   129,     2,     2,     2,     2,
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
     105,   106,   107,   108,   112,   115,   116,   122,   123
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
     584,   589,   594,   599,   604,   608,   615,   622,   626,   628,
     632,   634,   636,   640,   646,   648,   650,   652,   654,   657,
     658,   661,   663,   666,   669,   673,   675,   677,   682,   687,
     691,   693,   695,   697,   699,   701,   703,   707,   712,   717,
     722,   726,   731,   736,   744,   750,   752,   754,   756,   758,
     760,   762,   764,   766,   768,   771,   778,   780,   782,   783,
     787,   789,   793,   795,   799,   803,   805,   809,   811,   813,
     815,   819,   822,   830,   840,   847,   849,   853,   855,   859,
     861,   865,   866,   869,   871,   875,   879,   880,   882,   884,
     886,   890,   892,   894,   898,   902,   909,   911,   915,   919,
     923,   929,   934,   938,   942,   944,   946,   948,   950,   958,
     963,   969,   970,   972,   975,   977,   981,   985,   988,   992,
     999,  1000,  1002,  1004,  1007,  1010,  1013,  1015,  1023,  1025,
    1027,  1029,  1031,  1035,  1042,  1046,  1050,  1054,  1056,  1060,
    1063,  1067,  1071,  1074,  1076,  1079,  1081,  1083,  1087
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     134,     0,    -1,   135,    -1,   136,    -1,   136,   124,    -1,
     196,    -1,   204,    -1,   190,    -1,   191,    -1,   194,    -1,
     205,    -1,   214,    -1,   216,    -1,   217,    -1,   218,    -1,
     223,    -1,   228,    -1,   229,    -1,   233,    -1,   235,    -1,
     236,    -1,   237,    -1,   238,    -1,   230,    -1,   239,    -1,
     241,    -1,   242,    -1,   243,    -1,   222,    -1,   137,    -1,
     136,   124,   137,    -1,   138,    -1,   185,    -1,   139,    -1,
      83,     3,   125,   125,   139,   126,   140,   126,    -1,   146,
      -1,    83,   147,    45,   125,   143,   146,   126,   144,   145,
      -1,    -1,   127,   141,    -1,   142,    -1,   141,   127,   142,
      -1,     3,    -1,     5,    -1,    51,    -1,    -1,    71,    26,
     168,    -1,    -1,    62,     5,    -1,    62,     5,   127,     5,
      -1,    83,   147,    45,   151,   152,   161,   164,   166,   170,
     172,    -1,   148,    -1,   147,   127,   148,    -1,   119,    -1,
     150,   149,    -1,    -1,     3,    -1,    18,     3,    -1,   178,
      -1,    21,   125,   178,   126,    -1,    64,   125,   178,   126,
      -1,    66,   125,   178,   126,    -1,    92,   125,   178,   126,
      -1,    50,   125,   178,   126,    -1,    33,   125,   119,   126,
      -1,    49,   125,   126,    -1,    33,   125,    39,     3,   126,
      -1,     3,    -1,   151,   127,     3,    -1,    -1,   153,    -1,
     105,   154,    -1,   155,    -1,   154,   108,   154,    -1,   125,
     154,   126,    -1,    63,   125,     8,   126,    -1,   157,   111,
     158,    -1,   157,   112,   158,    -1,   157,    52,   125,   160,
     126,    -1,   157,   122,    52,   125,   160,   126,    -1,   157,
      52,     9,    -1,   157,   122,    52,     9,    -1,   157,    23,
     158,   108,   158,    -1,   157,   114,   158,    -1,   157,   113,
     158,    -1,   157,   115,   158,    -1,   157,   116,   158,    -1,
     157,   111,   159,    -1,   156,    -1,   157,    23,   159,   108,
     159,    -1,   157,    23,   158,   108,   159,    -1,   157,    23,
     159,   108,   158,    -1,   157,   114,   159,    -1,   157,   113,
     159,    -1,   157,   115,   159,    -1,   157,   116,   159,    -1,
     157,   111,     8,    -1,   157,   112,     8,    -1,   157,    58,
      69,    -1,   157,    58,   122,    69,    -1,   157,   112,   159,
      -1,     3,    -1,     4,    -1,    33,   125,   119,   126,    -1,
      49,   125,   126,    -1,   104,   125,   126,    -1,    51,    -1,
     244,    -1,     5,    -1,   118,     5,    -1,     6,    -1,   118,
       6,    -1,    14,    -1,   158,    -1,   160,   127,   158,    -1,
      -1,    48,   162,    26,   163,    -1,    -1,     5,    -1,   157,
      -1,   163,   127,   157,    -1,    -1,   165,    -1,   106,    48,
      71,    26,   168,    -1,    -1,   167,    -1,    71,    26,   168,
      -1,    71,    26,    75,   125,   126,    -1,   169,    -1,   168,
     127,   169,    -1,   157,    -1,   157,    19,    -1,   157,    37,
      -1,    -1,   171,    -1,    62,     5,    -1,    62,     5,   127,
       5,    -1,    -1,   173,    -1,    70,   174,    -1,   175,    -1,
     174,   127,   175,    -1,     3,   111,     3,    -1,     3,   111,
       5,    -1,     3,   111,   125,   176,   126,    -1,     3,   111,
       3,   125,     8,   126,    -1,     3,   111,     8,    -1,   177,
      -1,   176,   127,   177,    -1,     3,   111,   158,    -1,     3,
      -1,     4,    -1,    51,    -1,     5,    -1,     6,    -1,    14,
      -1,     9,    -1,   118,   178,    -1,   122,   178,    -1,   178,
     117,   178,    -1,   178,   118,   178,    -1,   178,   119,   178,
      -1,   178,   120,   178,    -1,   178,   113,   178,    -1,   178,
     114,   178,    -1,   178,   110,   178,    -1,   178,   109,   178,
      -1,   178,   121,   178,    -1,   178,    40,   178,    -1,   178,
      67,   178,    -1,   178,   116,   178,    -1,   178,   115,   178,
      -1,   178,   111,   178,    -1,   178,   112,   178,    -1,   178,
     108,   178,    -1,   178,   107,   178,    -1,   125,   178,   126,
      -1,   128,   182,   129,    -1,   179,    -1,   244,    -1,     3,
     125,   180,   126,    -1,    52,   125,   180,   126,    -1,    56,
     125,   180,   126,    -1,    24,   125,   180,   126,    -1,    43,
     125,   180,   126,    -1,     3,   125,   126,    -1,    66,   125,
     178,   127,   178,   126,    -1,    64,   125,   178,   127,   178,
     126,    -1,   104,   125,   126,    -1,   181,    -1,   180,   127,
     181,    -1,   178,    -1,     8,    -1,   183,   111,   184,    -1,
     182,   127,   183,   111,   184,    -1,     3,    -1,    52,    -1,
     158,    -1,     3,    -1,    87,   187,    -1,    -1,    61,     8,
      -1,   103,    -1,    90,   186,    -1,    65,   186,    -1,    16,
      90,   186,    -1,    74,    -1,    73,    -1,    16,     8,    90,
     186,    -1,    16,     3,    90,   186,    -1,    53,     3,    90,
      -1,     3,    -1,    69,    -1,     8,    -1,     5,    -1,     6,
      -1,   188,    -1,   189,   118,   188,    -1,    85,     3,   111,
     193,    -1,    85,     3,   111,   192,    -1,    85,     3,   111,
      69,    -1,    85,    68,   189,    -1,    85,    10,   111,   189,
      -1,    85,    28,    85,   189,    -1,    85,    47,     9,   111,
     125,   160,   126,    -1,    85,    47,     3,   111,   192,    -1,
       3,    -1,     8,    -1,    97,    -1,    42,    -1,   158,    -1,
      31,    -1,    81,    -1,   195,    -1,    22,    -1,    89,    96,
      -1,   197,    57,     3,   198,   101,   200,    -1,    54,    -1,
      79,    -1,    -1,   125,   199,   126,    -1,   157,    -1,   199,
     127,   157,    -1,   201,    -1,   200,   127,   201,    -1,   125,
     202,   126,    -1,   203,    -1,   202,   127,   203,    -1,   158,
      -1,   159,    -1,     8,    -1,   125,   160,   126,    -1,   125,
     126,    -1,    36,    45,   151,   105,    51,   111,   158,    -1,
      36,    45,   151,   105,    51,    52,   125,   160,   126,    -1,
      27,     3,   125,   206,   209,   126,    -1,   207,    -1,   206,
     127,   207,    -1,   203,    -1,   125,   208,   126,    -1,     8,
      -1,   208,   127,     8,    -1,    -1,   127,   210,    -1,   211,
      -1,   210,   127,   211,    -1,   203,   212,   213,    -1,    -1,
      18,    -1,     3,    -1,    62,    -1,   215,     3,   186,    -1,
      38,    -1,    37,    -1,    87,    94,   186,    -1,    87,    35,
     186,    -1,   100,   151,    85,   219,   153,   172,    -1,   220,
      -1,   219,   127,   220,    -1,     3,   111,   158,    -1,     3,
     111,   159,    -1,     3,   111,   125,   160,   126,    -1,     3,
     111,   125,   126,    -1,   244,   111,   158,    -1,   244,   111,
     159,    -1,    56,    -1,    24,    -1,    43,    -1,    25,    -1,
      17,    93,     3,    15,    30,     3,   221,    -1,    87,   231,
     102,   224,    -1,    87,   231,   102,    61,     8,    -1,    -1,
     225,    -1,   105,   226,    -1,   227,    -1,   226,   107,   227,
      -1,     3,   111,     8,    -1,    87,    29,    -1,    87,    28,
      85,    -1,    85,   231,    96,    59,    60,   232,    -1,    -1,
      47,    -1,    86,    -1,    77,    99,    -1,    77,    32,    -1,
      78,    77,    -1,    84,    -1,    34,    46,     3,    80,   234,
      88,     8,    -1,    55,    -1,    24,    -1,    43,    -1,    91,
      -1,    41,    46,     3,    -1,    20,    53,     3,    95,    82,
       3,    -1,    44,    82,     3,    -1,    44,    76,     3,    -1,
      83,   240,   170,    -1,    10,    -1,    10,   130,     3,    -1,
      83,   178,    -1,    98,    82,     3,    -1,    72,    53,     3,
      -1,     3,   245,    -1,   246,    -1,   245,   246,    -1,    13,
      -1,    14,    -1,   131,   178,   132,    -1,   131,     8,   132,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   159,   159,   160,   161,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   194,
     195,   199,   200,   204,   205,   213,   214,   221,   223,   227,
     231,   238,   239,   240,   244,   257,   264,   266,   271,   280,
     295,   296,   300,   301,   304,   306,   307,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   323,   324,   327,   329,
     333,   337,   338,   339,   343,   348,   355,   363,   371,   380,
     385,   390,   395,   400,   405,   410,   415,   420,   425,   430,
     435,   440,   445,   450,   455,   460,   465,   470,   475,   483,
     487,   488,   493,   499,   505,   511,   517,   521,   522,   533,
     534,   535,   539,   545,   551,   553,   556,   558,   565,   569,
     575,   577,   581,   592,   594,   598,   602,   609,   610,   614,
     615,   616,   619,   621,   625,   630,   637,   639,   643,   647,
     648,   652,   657,   662,   668,   673,   681,   686,   693,   703,
     704,   705,   706,   707,   708,   709,   710,   711,   713,   714,
     715,   716,   717,   718,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   729,   730,   731,   732,   733,   737,
     738,   739,   740,   741,   742,   743,   744,   745,   749,   750,
     754,   755,   759,   760,   764,   765,   769,   770,   776,   779,
     781,   785,   786,   787,   788,   789,   790,   791,   796,   801,
     811,   812,   813,   814,   815,   819,   820,   824,   829,   834,
     839,   840,   841,   845,   850,   858,   859,   863,   864,   865,
     879,   880,   881,   885,   886,   892,   900,   901,   904,   906,
     910,   911,   915,   916,   920,   924,   925,   929,   930,   931,
     932,   933,   939,   947,   961,   969,   973,   980,   981,   988,
     998,  1004,  1006,  1010,  1015,  1019,  1026,  1028,  1032,  1033,
    1039,  1047,  1048,  1054,  1058,  1064,  1072,  1073,  1077,  1091,
    1097,  1101,  1106,  1120,  1131,  1132,  1133,  1134,  1138,  1151,
    1155,  1162,  1163,  1167,  1171,  1172,  1176,  1180,  1187,  1194,
    1200,  1201,  1202,  1206,  1207,  1208,  1209,  1215,  1226,  1227,
    1228,  1229,  1233,  1244,  1256,  1265,  1276,  1284,  1285,  1289,
    1299,  1310,  1321,  1324,  1325,  1329,  1330,  1331,  1332
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
  "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", 
  "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTEGER", "TOK_INTO", "TOK_IS", 
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PROFILE", 
  "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", 
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
  "where_clause", "where_expr", "where_item", "expr_float_unhandled", 
  "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "opt_int", "group_items_list", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "consthash", "hash_key", "hash_val", "show_stmt", 
  "like_filter", "show_what", "simple_set_value", "set_value", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
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
  "json_field", "subscript", "subkey", 0
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
     355,   356,   357,   358,   359,   360,   361,   362,   363,   124,
      38,    61,   364,    60,    62,   365,   366,    43,    45,    42,
      47,    37,   367,   368,    59,    40,    41,    44,   123,   125,
      46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   133,   134,   134,   134,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   141,
     141,   142,   142,   142,   143,   144,   145,   145,   145,   146,
     147,   147,   148,   148,   149,   149,   149,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   151,   151,   152,   152,
     153,   154,   154,   154,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   156,
     157,   157,   157,   157,   157,   157,   157,   158,   158,   159,
     159,   159,   160,   160,   161,   161,   162,   162,   163,   163,
     164,   164,   165,   166,   166,   167,   167,   168,   168,   169,
     169,   169,   170,   170,   171,   171,   172,   172,   173,   174,
     174,   175,   175,   175,   175,   175,   176,   176,   177,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   180,   180,
     181,   181,   182,   182,   183,   183,   184,   184,   185,   186,
     186,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     188,   188,   188,   188,   188,   189,   189,   190,   190,   190,
     190,   190,   190,   191,   191,   192,   192,   193,   193,   193,
     194,   194,   194,   195,   195,   196,   197,   197,   198,   198,
     199,   199,   200,   200,   201,   202,   202,   203,   203,   203,
     203,   203,   204,   204,   205,   206,   206,   207,   207,   208,
     208,   209,   209,   210,   210,   211,   212,   212,   213,   213,
     214,   215,   215,   216,   217,   218,   219,   219,   220,   220,
     220,   220,   220,   220,   221,   221,   221,   221,   222,   223,
     223,   224,   224,   225,   226,   226,   227,   228,   229,   230,
     231,   231,   231,   232,   232,   232,   232,   233,   234,   234,
     234,   234,   235,   236,   237,   238,   239,   240,   240,   241,
     242,   243,   244,   245,   245,   246,   246,   246,   246
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
       4,     4,     4,     4,     3,     6,     6,     3,     1,     3,
       1,     1,     3,     5,     1,     1,     1,     1,     2,     0,
       2,     1,     2,     2,     3,     1,     1,     4,     4,     3,
       1,     1,     1,     1,     1,     1,     3,     4,     4,     4,
       3,     4,     4,     7,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     6,     1,     1,     0,     3,
       1,     3,     1,     3,     3,     1,     3,     1,     1,     1,
       3,     2,     7,     9,     6,     1,     3,     1,     3,     1,
       3,     0,     2,     1,     3,     3,     0,     1,     1,     1,
       3,     1,     1,     3,     3,     6,     1,     3,     3,     3,
       5,     4,     3,     3,     1,     1,     1,     1,     7,     4,
       5,     0,     1,     2,     1,     3,     3,     2,     3,     6,
       0,     1,     1,     2,     2,     2,     1,     7,     1,     1,
       1,     1,     3,     6,     3,     3,     3,     1,     3,     2,
       3,     3,     2,     1,     2,     1,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   233,     0,   230,     0,     0,   272,   271,
       0,     0,   236,     0,   237,   231,     0,   300,   300,     0,
       0,     0,     0,     2,     3,    29,    31,    33,    35,    32,
       7,     8,     9,   232,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   149,   150,   152,   153,
     155,   317,   154,     0,     0,     0,     0,     0,     0,   151,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,    50,    54,    57,   177,   132,   178,     0,     0,
       0,   301,     0,   302,     0,     0,     0,   297,   199,   301,
       0,   199,   206,   205,   199,   199,   201,   198,     0,   234,
       0,    66,     0,     1,     4,     0,   199,     0,     0,     0,
       0,     0,   312,   315,   314,   321,   325,   326,     0,     0,
     322,   323,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   149,     0,     0,   156,   157,
       0,   194,   195,     0,     0,     0,     0,    55,     0,    53,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   316,   133,
       0,     0,     0,     0,     0,   210,   213,   214,   212,   211,
     215,   220,     0,     0,     0,   199,   298,     0,   274,     0,
     203,   202,   273,   291,   320,     0,     0,     0,     0,    30,
     238,   270,     0,     0,   107,   109,   249,   111,     0,     0,
     247,   248,   257,   261,   255,     0,     0,   191,     0,   184,
     190,     0,   188,     0,     0,   324,   318,     0,     0,     0,
       0,     0,    64,     0,     0,     0,     0,     0,     0,   187,
       0,     0,     0,   175,     0,   176,     0,    44,    68,    51,
      57,    56,   167,   168,   174,   173,   165,   164,   171,   172,
     162,   163,   170,   169,   158,   159,   160,   161,   166,   134,
     225,   226,   228,   219,   227,     0,   229,   218,   217,   221,
     222,     0,     0,     0,     0,   199,   199,   204,   200,   209,
       0,     0,   289,   292,     0,     0,   276,     0,    67,     0,
       0,     0,     0,   108,   110,   259,   251,   112,     0,     0,
       0,     0,   309,   310,   308,   311,     0,     0,     0,     0,
     179,     0,   328,   327,    58,   182,     0,    63,   183,    62,
     180,   181,    59,     0,    60,     0,    61,     0,     0,     0,
     197,   196,   192,     0,     0,   114,    69,     0,   224,     0,
     216,     0,   208,   207,   290,     0,   293,   294,     0,     0,
     136,     0,   100,   101,     0,     0,   105,     0,   240,     0,
     106,     0,     0,   313,   250,     0,   258,     0,   257,   256,
     262,   263,   254,     0,     0,     0,    37,   189,    65,     0,
       0,     0,     0,     0,     0,     0,    70,    71,    87,     0,
     116,   120,   135,     0,     0,     0,   306,   299,     0,     0,
       0,   278,   279,   277,     0,   275,   137,   282,   283,     0,
       0,     0,   239,     0,     0,   235,   242,   285,   287,   286,
     284,   288,   113,   260,   267,     0,     0,   307,     0,   252,
       0,     0,   186,   185,   193,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     117,     0,     0,   123,   121,   223,   304,   303,   305,   296,
     295,   281,     0,     0,   138,   139,     0,   103,   104,   241,
       0,     0,   245,     0,   268,   269,   265,   266,   264,     0,
      41,    42,    43,    38,    39,    34,     0,     0,    46,     0,
      73,    72,     0,     0,    79,     0,    97,     0,    95,    75,
      86,    96,    76,    99,    83,    92,    82,    91,    84,    93,
      85,    94,     0,     0,     0,     0,   132,   124,   280,     0,
       0,   102,   244,     0,   243,   253,     0,     0,     0,    36,
      74,     0,     0,     0,    98,    80,     0,   118,   115,     0,
       0,   136,   141,   142,   145,     0,   140,   246,    40,   129,
      45,   127,    47,    81,    89,    90,    88,    77,     0,     0,
       0,     0,   125,    49,     0,     0,     0,   146,   130,   131,
       0,     0,    78,   119,   122,     0,     0,     0,   143,     0,
     128,    48,   126,   144,   148,   147
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   461,   513,   514,
     363,   518,   559,    28,    91,    92,   169,    93,   268,   365,
     366,   416,   417,   418,   579,   327,   231,   328,   421,   481,
     568,   483,   484,   546,   547,   580,   581,   188,   189,   435,
     436,   494,   495,   596,   597,   240,    95,   241,   242,   163,
     164,   362,    29,   208,   117,   200,   201,    30,    31,   297,
     298,    32,    33,    34,    35,   320,   389,   445,   446,   501,
     232,    36,    37,   233,   234,   329,   331,   400,   401,   455,
     506,    38,    39,    40,    41,    42,   315,   316,   451,    43,
      44,   312,   313,   376,   377,    45,    46,    47,   104,   427,
      48,   336,    49,    50,    51,    52,    53,    96,    54,    55,
      56,    97,   140,   141
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -450
static const short yypact[] =
{
    1147,   -20,    99,  -450,   158,  -450,   118,    49,  -450,  -450,
     171,   146,  -450,   176,  -450,  -450,   274,   286,   679,   123,
     172,   257,   267,  -450,   141,  -450,  -450,  -450,  -450,  -450,
    -450,  -450,  -450,  -450,  -450,   215,  -450,  -450,  -450,   278,
    -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,
    -450,  -450,  -450,  -450,  -450,  -450,  -450,   290,   294,   175,
     310,   257,   326,   328,   331,   336,    34,  -450,  -450,  -450,
    -450,   221,  -450,   237,   242,   250,   252,   258,   262,  -450,
     266,   269,   272,   275,   276,   283,   667,  -450,   667,   667,
      72,   -12,  -450,   203,   694,  -450,   327,  -450,   299,   302,
     335,    87,    64,  -450,   316,    52,   341,  -450,   370,  -450,
     433,   370,  -450,  -450,   370,   370,  -450,  -450,   342,  -450,
     434,  -450,   -53,  -450,   144,   435,   370,   431,   353,    21,
     369,   -39,  -450,  -450,  -450,  -450,  -450,  -450,   458,   564,
      25,  -450,   448,   667,   632,   -15,   632,   329,   667,   632,
     632,   667,   667,   667,   330,    63,   333,   340,  -450,  -450,
     902,  -450,  -450,   126,   343,     3,   355,  -450,   449,  -450,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   463,  -450,  -450,
     204,    64,    64,   359,   360,  -450,  -450,  -450,  -450,  -450,
    -450,   357,   394,   386,   391,   370,  -450,   476,  -450,   399,
    -450,  -450,  -450,    55,  -450,   487,   488,   436,   300,  -450,
     373,  -450,   469,   421,  -450,  -450,  -450,  -450,   264,    17,
    -450,  -450,  -450,   377,  -450,   196,   454,  -450,   181,  -450,
    1142,   -69,  -450,   374,   709,  -450,  -450,   930,   148,   504,
     382,   164,  -450,   958,   185,   192,   791,   812,   986,  -450,
     539,   667,   667,  -450,    72,  -450,    90,  -450,   -27,  -450,
    1142,  -450,  -450,  -450,  1156,  1170,  1238,  1181,   719,   719,
     376,   376,   376,   376,   265,   265,  -450,  -450,  -450,   384,
    -450,  -450,  -450,  -450,  -450,   507,  -450,  -450,  -450,   357,
     357,    84,   388,    64,   455,   370,   370,  -450,  -450,  -450,
     508,   514,  -450,  -450,    23,    91,  -450,   407,  -450,   259,
     418,   517,   518,  -450,  -450,  -450,  -450,  -450,   194,   216,
      21,   397,  -450,  -450,  -450,  -450,   437,    16,   355,   400,
    -450,   632,  -450,  -450,  -450,  -450,   401,  -450,  -450,  -450,
    -450,  -450,  -450,   667,  -450,   667,  -450,   846,   874,   419,
    -450,  -450,  -450,   446,     1,   483,  -450,   527,  -450,     2,
    -450,   166,  -450,  -450,  -450,   422,   427,  -450,    75,   487,
     465,    14,    25,  -450,   411,   412,  -450,   413,  -450,   229,
    -450,   414,   167,  -450,  -450,     2,  -450,   533,    41,  -450,
     423,  -450,  -450,   538,   424,     2,   425,  -450,  -450,  1014,
    1042,    90,   355,   430,   426,     1,   451,  -450,  -450,   590,
     552,   459,  -450,   244,   -17,   489,  -450,  -450,   563,   514,
       4,  -450,  -450,  -450,   557,  -450,  -450,  -450,  -450,   456,
     453,   461,  -450,   259,    37,   447,  -450,  -450,  -450,  -450,
    -450,  -450,  -450,  -450,  -450,    46,    37,  -450,     2,  -450,
     210,   466,  -450,  -450,  -450,    26,   506,   573,   108,     1,
      14,    22,    -4,    48,    93,    14,    14,    14,    14,   537,
    -450,   559,   545,   523,  -450,  -450,  -450,  -450,  -450,  -450,
    -450,  -450,   254,   485,   470,  -450,   472,  -450,  -450,  -450,
       5,   291,  -450,   414,  -450,  -450,  -450,   581,  -450,   296,
    -450,  -450,  -450,   473,  -450,  -450,   257,   575,   540,   478,
    -450,  -450,   498,   500,  -450,     2,  -450,   541,  -450,  -450,
    -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,
    -450,  -450,    32,   259,   543,   583,   327,  -450,  -450,    13,
     557,  -450,  -450,    37,  -450,  -450,   210,   259,   606,  -450,
    -450,    14,    14,   301,  -450,  -450,     2,  -450,   490,   586,
     253,   465,   493,  -450,  -450,   616,  -450,  -450,  -450,   211,
     494,  -450,   495,  -450,  -450,  -450,  -450,  -450,   303,   259,
     259,   499,   494,  -450,   615,   515,   308,  -450,  -450,  -450,
     259,   620,  -450,  -450,   494,   501,   503,     2,  -450,   616,
    -450,  -450,  -450,  -450,  -450,  -450
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -450,  -450,  -450,  -450,   509,  -450,   393,  -450,  -450,    76,
    -450,  -450,  -450,   271,   227,   479,  -450,  -450,    58,  -450,
     332,  -385,  -450,  -450,  -318,  -129,  -364,  -367,  -450,  -450,
    -450,  -450,  -450,  -450,  -450,  -449,    44,   103,  -450,    79,
    -450,  -450,   101,  -450,    43,    -3,  -450,    92,   312,  -450,
     390,   247,  -450,  -103,  -450,   352,   145,  -450,  -450,   358,
    -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,   157,  -450,
    -327,  -450,  -450,  -450,   339,  -450,  -450,  -450,   206,  -450,
    -450,  -450,  -450,  -450,  -450,  -450,  -450,   284,  -450,  -450,
    -450,  -450,  -450,  -450,   245,  -450,  -450,  -450,   648,  -450,
    -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,  -450,
    -450,  -175,  -450,   547
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -320
static const short yytable[] =
{
     230,   388,   423,   398,   382,   383,   121,   224,   210,   224,
     224,   211,   212,    94,   432,   486,   572,   438,   573,   224,
     225,   574,   224,   221,   249,   325,   224,   225,   227,   226,
     468,   524,   215,   165,   384,   227,   136,   137,   136,   137,
     317,   565,   224,   225,  -266,   226,   419,   136,   137,   504,
     385,   227,   386,   224,   225,   203,   528,   340,   341,   454,
     204,   296,   227,   492,   414,   526,   236,   195,   404,   196,
     197,   516,   198,    57,   216,   161,   136,   137,   364,   122,
     224,   225,   487,   158,   521,   159,   160,   290,   216,   227,
     193,   509,   291,   360,    61,   224,   194,   419,   224,   225,
     216,   531,   307,  -266,   250,   387,   523,   227,   505,   530,
     533,   535,   537,   539,   541,   166,   310,   502,   527,   131,
     295,   592,   295,   295,   162,   499,   415,   405,   267,   507,
     491,   326,   228,   199,   378,   295,   244,   361,   575,   228,
     247,   604,   205,   326,   390,   253,   229,   525,   256,   257,
     258,   419,    58,   166,   139,   228,   139,   566,   563,   138,
     311,    59,   500,   270,    60,   139,   228,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   155,    67,    68,    69,   260,   390,
      70,   447,   448,   228,   139,    72,   364,   584,   586,   588,
     430,   230,   372,   373,   317,    74,   167,   290,   295,   224,
     449,   228,   291,   510,   270,   511,   469,    62,   379,   119,
     332,   168,    63,   450,    76,   567,   577,   217,    64,    65,
     598,   218,    79,    80,   520,   160,   248,    81,   251,   333,
     390,   254,   255,   424,   425,   156,   292,   157,   599,   431,
     426,   334,   437,   264,   120,   265,   382,   383,   357,   358,
     121,   512,   382,   383,   338,   124,   452,   123,   390,   323,
     324,   603,   125,   293,   345,   341,   459,    66,    67,    68,
      69,   126,   361,    70,    71,    85,   384,   335,    72,    98,
     348,   341,   384,   127,   390,    73,    99,   128,    74,    86,
     129,   294,   385,    88,   386,   170,    89,    75,   385,    90,
     386,   350,   341,   130,   100,   230,   105,    76,   351,   341,
     394,   395,   295,    77,    78,    79,    80,   230,   591,   132,
      81,   133,   171,   101,   134,   270,   299,   300,    82,   135,
      83,   522,   396,   397,   529,   532,   534,   536,   538,   540,
     409,   142,   410,   110,   102,   442,   443,   387,   155,    67,
      68,    69,   143,   387,    70,   111,    84,   144,   390,    72,
     485,   395,   103,   112,   113,   145,    73,   146,    85,    74,
     548,   395,   390,   147,   184,   185,   186,   148,    75,   187,
     114,   149,    86,    87,   150,   390,    88,   151,    76,    89,
     152,   153,    90,   116,    77,    78,    79,    80,   154,   270,
     190,    81,   202,   191,   390,   390,   170,   552,   553,    82,
     192,    83,   555,   395,   230,   390,   206,   587,   395,   602,
     395,   207,   583,   585,   608,   609,   209,   214,   220,    66,
      67,    68,    69,   171,   213,    70,   222,    84,   223,   235,
      72,   246,   271,   304,   266,   252,   259,    73,   261,    85,
      74,   155,    67,    68,    69,   262,   237,    70,   289,    75,
     301,   302,    72,    86,    87,   303,   305,    88,   614,    76,
      89,   306,    74,    90,   308,    77,    78,    79,    80,   309,
     314,   318,    81,   182,   183,   184,   185,   186,   319,   321,
      82,    76,    83,   322,   330,   337,   342,   346,   347,    79,
      80,   367,   323,   369,    81,   371,   374,   375,   381,   391,
     392,   393,   156,   402,   157,   403,   406,   408,    84,   412,
     411,   420,   422,   428,   429,   434,   439,   440,   441,   444,
      85,   453,   155,    67,    68,    69,   457,   237,    70,   458,
     456,   467,   460,    72,    86,    87,   466,   480,    88,   469,
     493,    89,    85,    74,    90,   482,   488,   155,    67,    68,
      69,   489,   243,    70,   503,   496,    86,   517,    72,   497,
      88,   519,    76,   238,   239,   543,    90,   498,    74,   542,
      79,    80,   515,   544,   545,    81,   549,   550,   551,   454,
     556,   557,   558,   156,   560,   157,   561,    76,   562,   570,
     564,   582,   590,   470,   569,    79,    80,   589,   594,   595,
      81,   600,   601,   606,   605,   611,   607,   612,   156,   613,
     157,   339,   578,   219,   413,   155,    67,    68,    69,   465,
     237,    70,   471,    85,   610,   269,    72,   380,   472,   571,
     593,   576,   615,   407,   359,   370,    74,    86,   464,   368,
     554,    88,   508,   433,    89,   239,   118,    90,    85,   399,
     155,    67,    68,    69,   490,    76,    70,     0,     0,     0,
       0,    72,    86,    79,    80,     0,    88,   245,    81,    89,
       0,    74,    90,     0,  -319,   105,   156,     0,   157,     0,
       0,   473,   474,   475,   476,   477,   478,   106,   107,     0,
      76,     0,   479,     0,   108,     0,     0,     0,    79,    80,
       0,     0,     0,    81,     0,     0,   109,     0,     0,     0,
       0,   156,   110,   157,   170,     0,    85,     0,     0,     0,
       0,     0,     0,     0,   111,     0,     0,     0,     0,   170,
      86,     0,   112,   113,    88,     0,     0,    89,     0,   170,
      90,   171,     0,     0,     0,   103,     0,     0,     0,   114,
       0,    85,     0,   115,     0,     0,   171,     0,     0,     0,
       0,     0,   116,     0,     0,    86,   171,     0,     0,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   170,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   343,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   170,     0,     0,     0,     0,     0,   171,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,     0,     0,   170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   171,   170,     0,     0,   352,   353,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,     0,     0,     0,     0,   354,   355,
       0,   171,   170,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,     0,   171,
     170,     0,     0,   353,     0,     0,     0,     0,     0,     0,
       0,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,     0,   171,   170,     0,
       0,   355,     0,     0,     0,     0,     0,     0,     0,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,     0,   171,   170,     0,   263,     0,
       0,     0,     0,     0,     0,     0,     0,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,     0,   171,   170,     0,   344,     0,     0,     0,
       0,     0,     0,     0,     0,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
       0,   171,   170,     0,   349,     0,     0,     0,     0,     0,
       0,     0,     0,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,     0,   171,
       0,     0,   356,     0,     0,     0,     0,     0,     0,     0,
       0,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,     0,     0,     0,     0,
     462,     0,     0,     0,     0,     0,     0,     0,     0,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,     1,     0,     0,     2,   463,     3,
       0,     0,     0,     0,     4,     0,     0,     0,     5,     0,
       0,     6,   170,     7,     8,     9,     0,     0,    10,     0,
       0,    11,     0,     0,     0,     0,   170,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,   171,
     170,     0,     0,     0,     0,     0,     0,     0,     0,    13,
       0,   170,     0,   171,     0,     0,    14,     0,    15,     0,
      16,     0,    17,     0,    18,     0,    19,   171,     0,     0,
       0,     0,     0,     0,     0,    20,     0,    21,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   170,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,     0,     0,   171,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186
};

static const short yycheck[] =
{
     129,   319,   369,   330,     3,     4,     3,     5,   111,     5,
       5,   114,   115,    16,   378,    32,     3,   381,     5,     5,
       6,     8,     5,   126,    39,     8,     5,     6,    14,     8,
     415,     9,    85,    45,    33,    14,    13,    14,    13,    14,
     215,     9,     5,     6,     3,     8,   364,    13,    14,     3,
      49,    14,    51,     5,     6,     3,     8,   126,   127,    18,
       8,   190,    14,   430,    63,    69,   105,     3,    52,     5,
       6,    45,     8,    93,   127,     3,    13,    14,   105,    21,
       5,     6,    99,    86,   469,    88,    89,     3,   127,    14,
       3,   458,     8,     3,    45,     5,     9,   415,     5,     6,
     127,     8,   205,    62,   119,   104,   470,    14,    62,   473,
     474,   475,   476,   477,   478,   127,    61,   444,   122,    61,
     118,   570,   118,   118,    52,   443,   125,   111,   125,   456,
     126,   126,   118,    69,   111,   118,   139,   266,   125,   118,
     143,   590,    90,   126,   319,   148,   125,   125,   151,   152,
     153,   469,    53,   127,   131,   118,   131,   125,   525,   125,
     105,     3,   125,   166,    46,   131,   118,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,     3,     4,     5,     6,   125,   364,
       9,    24,    25,   118,   131,    14,   105,   561,   562,   566,
     125,   330,   305,   306,   379,    24,     3,     3,   118,     5,
      43,   118,     8,     3,   217,     5,   108,    46,   127,    96,
      24,    18,    76,    56,    43,   543,   553,    83,    82,    53,
      19,    87,    51,    52,   126,   238,   144,    56,   146,    43,
     415,   149,   150,    77,    78,    64,    42,    66,    37,   378,
      84,    55,   381,   127,    82,   129,     3,     4,   261,   262,
       3,    51,     3,     4,    83,   124,   395,     0,   443,     5,
       6,   589,    57,    69,   126,   127,   405,     3,     4,     5,
       6,     3,   411,     9,    10,   104,    33,    91,    14,     3,
     126,   127,    33,     3,   469,    21,    10,     3,    24,   118,
     125,    97,    49,   122,    51,    40,   125,    33,    49,   128,
      51,   126,   127,     3,    28,   444,    16,    43,   126,   127,
     126,   127,   118,    49,    50,    51,    52,   456,    75,     3,
      56,     3,    67,    47,     3,   338,   191,   192,    64,     3,
      66,   470,   126,   127,   473,   474,   475,   476,   477,   478,
     353,   130,   355,    53,    68,   126,   127,   104,     3,     4,
       5,     6,   125,   104,     9,    65,    92,   125,   543,    14,
     126,   127,    86,    73,    74,   125,    21,   125,   104,    24,
     126,   127,   557,   125,   119,   120,   121,   125,    33,    62,
      90,   125,   118,   119,   125,   570,   122,   125,    43,   125,
     125,   125,   128,   103,    49,    50,    51,    52,   125,   412,
     111,    56,    96,   111,   589,   590,    40,   126,   127,    64,
      85,    66,   126,   127,   553,   600,    85,   126,   127,   126,
     127,    61,   561,   562,   126,   127,     3,     3,     3,     3,
       4,     5,     6,    67,   102,     9,    15,    92,    95,    80,
      14,     3,     3,    59,   111,   126,   126,    21,   125,   104,
      24,     3,     4,     5,     6,   125,     8,     9,     5,    33,
     111,   111,    14,   118,   119,   118,    90,   122,   607,    43,
     125,    90,    24,   128,     8,    49,    50,    51,    52,    90,
       3,     3,    56,   117,   118,   119,   120,   121,   125,    30,
      64,    43,    66,    82,   127,    51,   132,     3,   126,    51,
      52,   127,     5,   125,    56,    60,     8,     3,   111,   101,
       3,     3,    64,   126,    66,    88,   126,   126,    92,    83,
     111,    48,     5,   111,   107,    70,   125,   125,   125,   125,
     104,     8,     3,     4,     5,     6,     8,     8,     9,   125,
     127,   125,   127,    14,   118,   119,   126,     5,   122,   108,
       3,   125,   104,    24,   128,   106,    77,     3,     4,     5,
       6,     8,     8,     9,   127,   119,   118,    71,    14,   126,
     122,     8,    43,   125,   126,    26,   128,   126,    24,    52,
      51,    52,   126,    48,    71,    56,   111,   127,   126,    18,
     127,    26,    62,    64,   126,    66,   108,    43,   108,    26,
      69,     5,    26,    23,    71,    51,    52,   127,   125,     3,
      56,   127,   127,     8,   125,     5,   111,   126,    64,   126,
      66,   238,   556,   124,   363,     3,     4,     5,     6,   412,
       8,     9,    52,   104,   600,   166,    14,   315,    58,   546,
     571,   550,   609,   341,   264,   303,    24,   118,   411,   301,
     503,   122,   456,   379,   125,   126,    18,   128,   104,   330,
       3,     4,     5,     6,   429,    43,     9,    -1,    -1,    -1,
      -1,    14,   118,    51,    52,    -1,   122,   140,    56,   125,
      -1,    24,   128,    -1,     0,    16,    64,    -1,    66,    -1,
      -1,   111,   112,   113,   114,   115,   116,    28,    29,    -1,
      43,    -1,   122,    -1,    35,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    -1,    -1,    47,    -1,    -1,    -1,
      -1,    64,    53,    66,    40,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    40,
     118,    -1,    73,    74,   122,    -1,    -1,   125,    -1,    40,
     128,    67,    -1,    -1,    -1,    86,    -1,    -1,    -1,    90,
      -1,   104,    -1,    94,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,   118,    67,    -1,    -1,   122,
      -1,    -1,   125,    -1,    -1,   128,    -1,    -1,    -1,    -1,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,    40,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,    67,    40,    -1,    -1,   126,   127,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,    -1,    -1,    -1,    -1,   126,   127,
      -1,    67,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,    -1,    67,
      40,    -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,    67,    40,    -1,
      -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,    -1,    67,    40,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,    -1,    67,    40,    -1,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      -1,    67,    40,    -1,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,    -1,    67,
      -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,    17,    -1,    -1,    20,   126,    22,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    31,    -1,
      -1,    34,    40,    36,    37,    38,    -1,    -1,    41,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      -1,    40,    -1,    67,    -1,    -1,    79,    -1,    81,    -1,
      83,    -1,    85,    -1,    87,    -1,    89,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    67,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,    40,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      41,    44,    54,    72,    79,    81,    83,    85,    87,    89,
      98,   100,   134,   135,   136,   137,   138,   139,   146,   185,
     190,   191,   194,   195,   196,   197,   204,   205,   214,   215,
     216,   217,   218,   222,   223,   228,   229,   230,   233,   235,
     236,   237,   238,   239,   241,   242,   243,    93,    53,     3,
      46,    45,    46,    76,    82,    53,     3,     4,     5,     6,
       9,    10,    14,    21,    24,    33,    43,    49,    50,    51,
      52,    56,    64,    66,    92,   104,   118,   119,   122,   125,
     128,   147,   148,   150,   178,   179,   240,   244,     3,    10,
      28,    47,    68,    86,   231,    16,    28,    29,    35,    47,
      53,    65,    73,    74,    90,    94,   103,   187,   231,    96,
      82,     3,   151,     0,   124,    57,     3,     3,     3,   125,
       3,   151,     3,     3,     3,     3,    13,    14,   125,   131,
     245,   246,   130,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,     3,    64,    66,   178,   178,
     178,     3,    52,   182,   183,    45,   127,     3,    18,   149,
      40,    67,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    62,   170,   171,
     111,   111,    85,     3,     9,     3,     5,     6,     8,    69,
     188,   189,    96,     3,     8,    90,    85,    61,   186,     3,
     186,   186,   186,   102,     3,    85,   127,    83,    87,   137,
       3,   186,    15,    95,     5,     6,     8,    14,   118,   125,
     158,   159,   203,   206,   207,    80,   105,     8,   125,   126,
     178,   180,   181,     8,   178,   246,     3,   178,   180,    39,
     119,   180,   126,   178,   180,   180,   178,   178,   178,   126,
     125,   125,   125,   126,   127,   129,   111,   125,   151,   148,
     178,     3,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,     5,
       3,     8,    42,    69,    97,   118,   158,   192,   193,   189,
     189,   111,   111,   118,    59,    90,    90,   186,     8,    90,
      61,   105,   224,   225,     3,   219,   220,   244,     3,   125,
     198,    30,    82,     5,     6,     8,   126,   158,   160,   208,
     127,   209,    24,    43,    55,    91,   234,    51,    83,   139,
     126,   127,   132,   132,   126,   126,     3,   126,   126,   126,
     126,   126,   126,   127,   126,   127,   126,   178,   178,   183,
       3,   158,   184,   143,   105,   152,   153,   127,   192,   125,
     188,    60,   186,   186,     8,     3,   226,   227,   111,   127,
     153,   111,     3,     4,    33,    49,    51,   104,   157,   199,
     244,   101,     3,     3,   126,   127,   126,   127,   203,   207,
     210,   211,   126,    88,    52,   111,   126,   181,   126,   178,
     178,   111,    83,   146,    63,   125,   154,   155,   156,   157,
      48,   161,     5,   160,    77,    78,    84,   232,   111,   107,
     125,   158,   159,   220,    70,   172,   173,   158,   159,   125,
     125,   125,   126,   127,   125,   200,   201,    24,    25,    43,
      56,   221,   158,     8,    18,   212,   127,     8,   125,   158,
     127,   140,   126,   126,   184,   147,   126,   125,   154,   108,
      23,    52,    58,   111,   112,   113,   114,   115,   116,   122,
       5,   162,   106,   164,   165,   126,    32,    99,    77,     8,
     227,   126,   160,     3,   174,   175,   119,   126,   126,   157,
     125,   202,   203,   127,     3,    62,   213,   203,   211,   160,
       3,     5,    51,   141,   142,   126,    45,    71,   144,     8,
     126,   154,   158,   159,     9,   125,    69,   122,     8,   158,
     159,     8,   158,   159,   158,   159,   158,   159,   158,   159,
     158,   159,    52,    26,    48,    71,   166,   167,   126,   111,
     127,   126,   126,   127,   201,   126,   127,    26,    62,   145,
     126,   108,   108,   160,    69,     9,   125,   157,   163,    71,
      26,   170,     3,     5,     8,   125,   175,   203,   142,   157,
     168,   169,     5,   158,   159,   158,   159,   126,   160,   127,
      26,    75,   168,   172,   125,     3,   176,   177,    19,    37,
     127,   127,   126,   157,   168,   125,     8,   111,   126,   127,
     169,     5,   126,   126,   158,   177
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

  case 192:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 193:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 200:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 205:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 206:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 207:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 208:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 209:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 217:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 218:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 219:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 220:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 221:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 222:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 223:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 224:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 227:

    { yyval.m_iValue = 1; ;}
    break;

  case 228:

    { yyval.m_iValue = 0; ;}
    break;

  case 229:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 230:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 231:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 232:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 235:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-3] );
		;}
    break;

  case 236:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 237:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 240:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 241:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 244:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 245:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 246:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 247:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 248:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 249:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 250:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 251:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 252:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 253:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 254:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 255:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 256:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 258:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 259:

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

  case 260:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 263:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 265:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 270:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 273:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 275:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 278:

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

  case 279:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 280:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 281:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 282:

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

  case 283:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 284:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 285:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 286:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 287:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 288:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 289:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 290:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 297:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 298:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 299:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 307:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 308:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 309:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 310:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 311:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 312:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 313:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 314:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 315:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 316:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 319:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 320:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 321:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 322:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 324:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 325:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 326:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 327:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 328:

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

