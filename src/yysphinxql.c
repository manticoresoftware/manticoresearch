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
     TOK_BY = 278,
     TOK_CALL = 279,
     TOK_CHARACTER = 280,
     TOK_COLLATION = 281,
     TOK_COLUMN = 282,
     TOK_COMMIT = 283,
     TOK_COMMITTED = 284,
     TOK_COUNT = 285,
     TOK_CREATE = 286,
     TOK_DELETE = 287,
     TOK_DESC = 288,
     TOK_DESCRIBE = 289,
     TOK_DISTINCT = 290,
     TOK_DIV = 291,
     TOK_DROP = 292,
     TOK_FALSE = 293,
     TOK_FLOAT = 294,
     TOK_FLUSH = 295,
     TOK_FROM = 296,
     TOK_FUNCTION = 297,
     TOK_GLOBAL = 298,
     TOK_GROUP = 299,
     TOK_GROUPBY = 300,
     TOK_GROUP_CONCAT = 301,
     TOK_ID = 302,
     TOK_IN = 303,
     TOK_INDEX = 304,
     TOK_INSERT = 305,
     TOK_INT = 306,
     TOK_INTEGER = 307,
     TOK_INTO = 308,
     TOK_ISOLATION = 309,
     TOK_LEVEL = 310,
     TOK_LIKE = 311,
     TOK_LIMIT = 312,
     TOK_MATCH = 313,
     TOK_MAX = 314,
     TOK_META = 315,
     TOK_MIN = 316,
     TOK_MOD = 317,
     TOK_NAMES = 318,
     TOK_NULL = 319,
     TOK_OPTION = 320,
     TOK_ORDER = 321,
     TOK_OPTIMIZE = 322,
     TOK_PLAN = 323,
     TOK_PROFILE = 324,
     TOK_RAND = 325,
     TOK_RAMCHUNK = 326,
     TOK_READ = 327,
     TOK_REPEATABLE = 328,
     TOK_REPLACE = 329,
     TOK_RETURNS = 330,
     TOK_ROLLBACK = 331,
     TOK_RTINDEX = 332,
     TOK_SELECT = 333,
     TOK_SERIALIZABLE = 334,
     TOK_SET = 335,
     TOK_SESSION = 336,
     TOK_SHOW = 337,
     TOK_SONAME = 338,
     TOK_START = 339,
     TOK_STATUS = 340,
     TOK_STRING = 341,
     TOK_SUM = 342,
     TOK_TABLE = 343,
     TOK_TABLES = 344,
     TOK_TO = 345,
     TOK_TRANSACTION = 346,
     TOK_TRUE = 347,
     TOK_TRUNCATE = 348,
     TOK_UNCOMMITTED = 349,
     TOK_UPDATE = 350,
     TOK_VALUES = 351,
     TOK_VARIABLES = 352,
     TOK_WARNINGS = 353,
     TOK_WEIGHT = 354,
     TOK_WHERE = 355,
     TOK_WITHIN = 356,
     TOK_OR = 357,
     TOK_AND = 358,
     TOK_NE = 359,
     TOK_GTE = 360,
     TOK_LTE = 361,
     TOK_NOT = 362,
     TOK_NEG = 363
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
#define TOK_BY 278
#define TOK_CALL 279
#define TOK_CHARACTER 280
#define TOK_COLLATION 281
#define TOK_COLUMN 282
#define TOK_COMMIT 283
#define TOK_COMMITTED 284
#define TOK_COUNT 285
#define TOK_CREATE 286
#define TOK_DELETE 287
#define TOK_DESC 288
#define TOK_DESCRIBE 289
#define TOK_DISTINCT 290
#define TOK_DIV 291
#define TOK_DROP 292
#define TOK_FALSE 293
#define TOK_FLOAT 294
#define TOK_FLUSH 295
#define TOK_FROM 296
#define TOK_FUNCTION 297
#define TOK_GLOBAL 298
#define TOK_GROUP 299
#define TOK_GROUPBY 300
#define TOK_GROUP_CONCAT 301
#define TOK_ID 302
#define TOK_IN 303
#define TOK_INDEX 304
#define TOK_INSERT 305
#define TOK_INT 306
#define TOK_INTEGER 307
#define TOK_INTO 308
#define TOK_ISOLATION 309
#define TOK_LEVEL 310
#define TOK_LIKE 311
#define TOK_LIMIT 312
#define TOK_MATCH 313
#define TOK_MAX 314
#define TOK_META 315
#define TOK_MIN 316
#define TOK_MOD 317
#define TOK_NAMES 318
#define TOK_NULL 319
#define TOK_OPTION 320
#define TOK_ORDER 321
#define TOK_OPTIMIZE 322
#define TOK_PLAN 323
#define TOK_PROFILE 324
#define TOK_RAND 325
#define TOK_RAMCHUNK 326
#define TOK_READ 327
#define TOK_REPEATABLE 328
#define TOK_REPLACE 329
#define TOK_RETURNS 330
#define TOK_ROLLBACK 331
#define TOK_RTINDEX 332
#define TOK_SELECT 333
#define TOK_SERIALIZABLE 334
#define TOK_SET 335
#define TOK_SESSION 336
#define TOK_SHOW 337
#define TOK_SONAME 338
#define TOK_START 339
#define TOK_STATUS 340
#define TOK_STRING 341
#define TOK_SUM 342
#define TOK_TABLE 343
#define TOK_TABLES 344
#define TOK_TO 345
#define TOK_TRANSACTION 346
#define TOK_TRUE 347
#define TOK_TRUNCATE 348
#define TOK_UNCOMMITTED 349
#define TOK_UPDATE 350
#define TOK_VALUES 351
#define TOK_VARIABLES 352
#define TOK_WARNINGS 353
#define TOK_WEIGHT 354
#define TOK_WHERE 355
#define TOK_WITHIN 356
#define TOK_OR 357
#define TOK_AND 358
#define TOK_NE 359
#define TOK_GTE 360
#define TOK_LTE 361
#define TOK_NOT 362
#define TOK_NEG 363




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif


// some helpers
#include <float.h> // for FLT_MAX



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
#define YYLAST   1171

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  126
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  107
/* YYNRULES -- Number of rules. */
#define YYNRULES  301
/* YYNRULES -- Number of states. */
#define YYNSTATES  578

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   363

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   116,   105,     2,
     120,   121,   114,   112,   122,   113,   125,   115,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   119,
     108,   106,   109,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   123,   104,   124,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   101,   102,   103,   107,
     110,   111,   117,   118
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
     900,   902,   910,   915,   916,   918,   921,   923,   927,   931,
     934,   938,   945,   946,   948,   950,   953,   956,   959,   961,
     969,   971,   973,   975,   979,   986,   990,   994,   998,  1000,
    1004,  1008
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     127,     0,    -1,   128,    -1,   129,    -1,   129,   119,    -1,
     187,    -1,   195,    -1,   181,    -1,   182,    -1,   185,    -1,
     196,    -1,   205,    -1,   207,    -1,   208,    -1,   213,    -1,
     218,    -1,   219,    -1,   223,    -1,   225,    -1,   226,    -1,
     227,    -1,   228,    -1,   220,    -1,   229,    -1,   231,    -1,
     232,    -1,   212,    -1,   130,    -1,   129,   119,   130,    -1,
     131,    -1,   176,    -1,   132,    -1,    78,     3,   120,   120,
     132,   121,   133,   121,    -1,   139,    -1,    78,   140,    41,
     120,   136,   139,   121,   137,   138,    -1,    -1,   122,   134,
      -1,   135,    -1,   134,   122,   135,    -1,     3,    -1,     5,
      -1,    47,    -1,    -1,    66,    23,   161,    -1,    -1,    57,
       5,    -1,    57,     5,   122,     5,    -1,    78,   140,    41,
     144,   145,   154,   157,   159,   163,   165,    -1,   141,    -1,
     140,   122,   141,    -1,   114,    -1,   143,   142,    -1,    -1,
       3,    -1,    16,     3,    -1,   171,    -1,    19,   120,   171,
     121,    -1,    59,   120,   171,   121,    -1,    61,   120,   171,
     121,    -1,    87,   120,   171,   121,    -1,    46,   120,   171,
     121,    -1,    30,   120,   114,   121,    -1,    45,   120,   121,
      -1,    30,   120,    35,     3,   121,    -1,     3,    -1,   144,
     122,     3,    -1,    -1,   146,    -1,   100,   147,    -1,   148,
      -1,   147,   103,   147,    -1,   120,   147,   121,    -1,    58,
     120,     8,   121,    -1,   150,   106,   151,    -1,   150,   107,
     151,    -1,   150,    48,   120,   153,   121,    -1,   150,   117,
      48,   120,   153,   121,    -1,   150,    48,     9,    -1,   150,
     117,    48,     9,    -1,   150,    21,   151,   103,   151,    -1,
     150,   109,   151,    -1,   150,   108,   151,    -1,   150,   110,
     151,    -1,   150,   111,   151,    -1,   150,   106,   152,    -1,
     149,    -1,   150,    21,   152,   103,   152,    -1,   150,    21,
     151,   103,   152,    -1,   150,    21,   152,   103,   151,    -1,
     150,   109,   152,    -1,   150,   108,   152,    -1,   150,   110,
     152,    -1,   150,   111,   152,    -1,   150,   106,     8,    -1,
     150,   107,     8,    -1,   150,   107,   152,    -1,     3,    -1,
       4,    -1,    30,   120,   114,   121,    -1,    45,   120,   121,
      -1,    99,   120,   121,    -1,    47,    -1,     5,    -1,   113,
       5,    -1,     6,    -1,   113,     6,    -1,   151,    -1,   153,
     122,   151,    -1,    -1,    44,   155,    23,   156,    -1,    -1,
       5,    -1,   150,    -1,   156,   122,   150,    -1,    -1,   158,
      -1,   101,    44,    66,    23,   161,    -1,    -1,   160,    -1,
      66,    23,   161,    -1,    66,    23,    70,   120,   121,    -1,
     162,    -1,   161,   122,   162,    -1,   150,    -1,   150,    17,
      -1,   150,    33,    -1,    -1,   164,    -1,    57,     5,    -1,
      57,     5,   122,     5,    -1,    -1,   166,    -1,    65,   167,
      -1,   168,    -1,   167,   122,   168,    -1,     3,   106,     3,
      -1,     3,   106,     5,    -1,     3,   106,   120,   169,   121,
      -1,     3,   106,     3,   120,     8,   121,    -1,     3,   106,
       8,    -1,   170,    -1,   169,   122,   170,    -1,     3,   106,
     151,    -1,     3,    -1,     4,    -1,    47,    -1,     5,    -1,
       6,    -1,     9,    -1,   113,   171,    -1,   117,   171,    -1,
     171,   112,   171,    -1,   171,   113,   171,    -1,   171,   114,
     171,    -1,   171,   115,   171,    -1,   171,   108,   171,    -1,
     171,   109,   171,    -1,   171,   105,   171,    -1,   171,   104,
     171,    -1,   171,   116,   171,    -1,   171,    36,   171,    -1,
     171,    62,   171,    -1,   171,   111,   171,    -1,   171,   110,
     171,    -1,   171,   106,   171,    -1,   171,   107,   171,    -1,
     171,   103,   171,    -1,   171,   102,   171,    -1,   120,   171,
     121,    -1,   123,   175,   124,    -1,   172,    -1,     3,   120,
     173,   121,    -1,    48,   120,   173,   121,    -1,    52,   120,
     173,   121,    -1,    22,   120,   173,   121,    -1,    39,   120,
     173,   121,    -1,     3,   120,   121,    -1,    61,   120,   171,
     122,   171,   121,    -1,    59,   120,   171,   122,   171,   121,
      -1,    99,   120,   121,    -1,   174,    -1,   173,   122,   174,
      -1,   171,    -1,     8,    -1,     3,   106,   151,    -1,   175,
     122,     3,   106,   151,    -1,    82,   178,    -1,    -1,    56,
       8,    -1,    98,    -1,    85,   177,    -1,    60,   177,    -1,
      14,    85,   177,    -1,    69,    -1,    68,    -1,    14,     8,
      85,   177,    -1,    14,     3,    85,   177,    -1,    49,     3,
      85,    -1,     3,    -1,    64,    -1,     8,    -1,     5,    -1,
       6,    -1,   179,    -1,   180,   113,   179,    -1,    80,     3,
     106,   184,    -1,    80,     3,   106,   183,    -1,    80,     3,
     106,    64,    -1,    80,    63,   180,    -1,    80,    10,   106,
     180,    -1,    80,    43,     9,   106,   120,   153,   121,    -1,
      80,    43,     3,   106,   183,    -1,     3,    -1,     8,    -1,
      92,    -1,    38,    -1,   151,    -1,    28,    -1,    76,    -1,
     186,    -1,    20,    -1,    84,    91,    -1,   188,    53,     3,
     189,    96,   191,    -1,    50,    -1,    74,    -1,    -1,   120,
     190,   121,    -1,   150,    -1,   190,   122,   150,    -1,   192,
      -1,   191,   122,   192,    -1,   120,   193,   121,    -1,   194,
      -1,   193,   122,   194,    -1,   151,    -1,   152,    -1,     8,
      -1,   120,   153,   121,    -1,   120,   121,    -1,    32,    41,
     144,   100,    47,   106,   151,    -1,    32,    41,   144,   100,
      47,    48,   120,   153,   121,    -1,    24,     3,   120,   197,
     200,   121,    -1,   198,    -1,   197,   122,   198,    -1,   194,
      -1,   120,   199,   121,    -1,     8,    -1,   199,   122,     8,
      -1,    -1,   122,   201,    -1,   202,    -1,   201,   122,   202,
      -1,   194,   203,   204,    -1,    -1,    16,    -1,     3,    -1,
      57,    -1,   206,     3,   177,    -1,    34,    -1,    33,    -1,
      82,    89,   177,    -1,    95,   144,    80,   209,   146,   165,
      -1,   210,    -1,   209,   122,   210,    -1,     3,   106,   151,
      -1,     3,   106,   152,    -1,     3,   106,   120,   153,   121,
      -1,     3,   106,   120,   121,    -1,    52,    -1,    22,    -1,
      39,    -1,    15,    88,     3,    13,    27,     3,   211,    -1,
      82,   221,    97,   214,    -1,    -1,   215,    -1,   100,   216,
      -1,   217,    -1,   216,   102,   217,    -1,     3,   106,     8,
      -1,    82,    26,    -1,    82,    25,    80,    -1,    80,   221,
      91,    54,    55,   222,    -1,    -1,    43,    -1,    81,    -1,
      72,    94,    -1,    72,    29,    -1,    73,    72,    -1,    79,
      -1,    31,    42,     3,    75,   224,    83,     8,    -1,    51,
      -1,    39,    -1,    86,    -1,    37,    42,     3,    -1,    18,
      49,     3,    90,    77,     3,    -1,    40,    77,     3,    -1,
      40,    71,     3,    -1,    78,   230,   163,    -1,    10,    -1,
      10,   125,     3,    -1,    93,    77,     3,    -1,    67,    49,
       3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   137,   137,   138,   139,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   170,   171,   175,
     176,   180,   181,   189,   190,   197,   199,   203,   207,   214,
     215,   216,   220,   233,   241,   243,   248,   257,   273,   274,
     278,   279,   282,   284,   285,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   301,   302,   305,   307,   311,   315,
     316,   317,   321,   326,   333,   341,   349,   358,   363,   368,
     373,   378,   383,   388,   393,   398,   403,   408,   413,   418,
     423,   428,   433,   438,   443,   451,   455,   456,   461,   467,
     473,   479,   488,   489,   500,   501,   505,   511,   517,   519,
     522,   524,   531,   535,   541,   543,   547,   558,   560,   564,
     568,   575,   576,   580,   581,   582,   585,   587,   591,   596,
     603,   605,   609,   613,   614,   618,   623,   628,   634,   639,
     647,   652,   659,   669,   670,   671,   672,   673,   674,   675,
     676,   677,   678,   679,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   693,   694,   695,
     696,   700,   701,   702,   703,   704,   705,   706,   707,   708,
     712,   713,   717,   718,   722,   723,   729,   732,   734,   738,
     739,   740,   741,   742,   743,   744,   749,   754,   764,   765,
     766,   767,   768,   772,   773,   777,   782,   787,   792,   793,
     797,   802,   810,   811,   815,   816,   817,   831,   832,   833,
     837,   838,   844,   852,   853,   856,   858,   862,   863,   867,
     868,   872,   876,   877,   881,   882,   883,   884,   885,   891,
     899,   913,   921,   925,   932,   933,   940,   950,   956,   958,
     962,   967,   971,   978,   980,   984,   985,   991,   999,  1000,
    1006,  1012,  1020,  1021,  1025,  1029,  1033,  1037,  1047,  1048,
    1049,  1053,  1066,  1072,  1073,  1077,  1081,  1082,  1086,  1090,
    1097,  1104,  1110,  1111,  1112,  1116,  1117,  1118,  1119,  1125,
    1136,  1137,  1138,  1142,  1153,  1165,  1174,  1185,  1193,  1194,
    1203,  1214
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
  "TOK_BIGINT", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", "TOK_COLLATION", 
  "TOK_COLUMN", "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", 
  "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", 
  "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", 
  "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", 
  "TOK_GROUP_CONCAT", "TOK_ID", "TOK_IN", "TOK_INDEX", "TOK_INSERT", 
  "TOK_INT", "TOK_INTEGER", "TOK_INTO", "TOK_ISOLATION", "TOK_LEVEL", 
  "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", 
  "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", 
  "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", "TOK_TABLES", 
  "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
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
     355,   356,   357,   358,   124,    38,    61,   359,    60,    62,
     360,   361,    43,    45,    42,    47,    37,   362,   363,    59,
      40,    41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   126,   127,   127,   127,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   129,   129,   130,
     130,   131,   131,   132,   132,   133,   133,   134,   134,   135,
     135,   135,   136,   137,   138,   138,   138,   139,   140,   140,
     141,   141,   142,   142,   142,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   144,   144,   145,   145,   146,   147,
     147,   147,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   149,   150,   150,   150,   150,
     150,   150,   151,   151,   152,   152,   153,   153,   154,   154,
     155,   155,   156,   156,   157,   157,   158,   159,   159,   160,
     160,   161,   161,   162,   162,   162,   163,   163,   164,   164,
     165,   165,   166,   167,   167,   168,   168,   168,   168,   168,
     169,   169,   170,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     173,   173,   174,   174,   175,   175,   176,   177,   177,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   179,   179,
     179,   179,   179,   180,   180,   181,   181,   181,   181,   181,
     182,   182,   183,   183,   184,   184,   184,   185,   185,   185,
     186,   186,   187,   188,   188,   189,   189,   190,   190,   191,
     191,   192,   193,   193,   194,   194,   194,   194,   194,   195,
     195,   196,   197,   197,   198,   198,   199,   199,   200,   200,
     201,   201,   202,   203,   203,   204,   204,   205,   206,   206,
     207,   208,   209,   209,   210,   210,   210,   210,   211,   211,
     211,   212,   213,   214,   214,   215,   216,   216,   217,   218,
     219,   220,   221,   221,   221,   222,   222,   222,   222,   223,
     224,   224,   224,   225,   226,   227,   228,   229,   230,   230,
     231,   232
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
       1,     7,     4,     0,     1,     2,     1,     3,     3,     2,
       3,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     1,     3,     6,     3,     3,     3,     1,     3,
       3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   220,     0,   217,     0,     0,   259,   258,
       0,     0,   223,     0,   224,   218,     0,   282,   282,     0,
       0,     0,     0,     2,     3,    27,    29,    31,    33,    30,
       7,     8,     9,   219,     5,     0,     6,    10,    11,     0,
      12,    13,    26,    14,    15,    16,    22,    17,    18,    19,
      20,    21,    23,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   146,   147,   148,   298,
       0,     0,     0,     0,     0,     0,   145,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,    48,
      52,    55,   170,   126,     0,     0,   283,     0,   284,     0,
       0,     0,   279,   283,     0,   187,   194,   193,   187,   187,
     189,   186,     0,   221,     0,    64,     0,     1,     4,     0,
     187,     0,     0,     0,     0,     0,   293,   296,   295,   301,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,     0,     0,   149,   150,     0,
       0,     0,     0,     0,    53,     0,    51,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   297,   127,     0,     0,     0,
       0,   198,   201,   202,   200,   199,   203,   208,     0,     0,
       0,   187,   280,     0,     0,   191,   190,   260,   273,   300,
       0,     0,     0,     0,    28,   225,   257,     0,     0,   102,
     104,   236,     0,     0,   234,   235,   244,   248,   242,     0,
       0,   183,     0,   176,   182,     0,   180,   299,     0,     0,
       0,     0,     0,    62,     0,     0,     0,     0,     0,     0,
     179,     0,     0,     0,   168,     0,     0,   169,    42,    66,
      49,    54,   160,   161,   167,   166,   158,   157,   164,   165,
     155,   156,   163,   162,   151,   152,   153,   154,   159,   128,
     212,   213,   215,   207,   214,     0,   216,   206,   205,   209,
       0,     0,     0,     0,   187,   187,   192,   197,   188,     0,
     272,   274,     0,     0,   262,    65,     0,     0,     0,     0,
     103,   105,   246,   238,   106,     0,     0,     0,     0,   291,
     290,   292,     0,     0,     0,     0,   171,     0,    56,   174,
       0,    61,   175,    60,   172,   173,    57,     0,    58,     0,
      59,     0,     0,   184,     0,     0,     0,   108,    67,     0,
     211,     0,   204,     0,   196,   195,     0,   275,   276,     0,
       0,   130,    96,    97,     0,     0,   101,     0,   227,     0,
       0,     0,   294,   237,     0,   245,     0,   244,   243,   249,
     250,   241,     0,     0,     0,    35,   181,    63,     0,     0,
       0,     0,     0,     0,     0,    68,    69,    85,     0,   110,
     114,   129,     0,     0,     0,   288,   281,     0,     0,     0,
     264,   265,   263,     0,   261,   131,     0,     0,     0,   226,
       0,     0,   222,   229,   269,   270,   268,   271,   107,   247,
     254,     0,     0,   289,     0,   239,     0,     0,   178,   177,
     185,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   111,     0,     0,   117,   115,
     210,   286,   285,   287,   278,   277,   267,     0,     0,   132,
     133,     0,    99,   100,   228,     0,     0,   232,     0,   255,
     256,   252,   253,   251,     0,    39,    40,    41,    36,    37,
      32,     0,     0,    44,     0,    71,    70,     0,     0,    77,
       0,    93,    73,    84,    94,    74,    95,    81,    90,    80,
      89,    82,    91,    83,    92,     0,     0,     0,     0,   126,
     118,   266,     0,     0,    98,   231,     0,   230,   240,     0,
       0,     0,    34,    72,     0,     0,     0,    78,     0,   112,
     109,     0,     0,   130,   135,   136,   139,     0,   134,   233,
      38,   123,    43,   121,    45,    79,    87,    88,    86,    75,
       0,     0,     0,     0,   119,    47,     0,     0,     0,   140,
     124,   125,     0,     0,    76,   113,   116,     0,     0,     0,
     137,     0,   122,    46,   120,   138,   142,   141
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   427,   478,   479,
     335,   483,   522,    28,    88,    89,   156,    90,   249,   337,
     338,   385,   386,   387,   541,   304,   215,   305,   390,   446,
     530,   448,   449,   509,   510,   542,   543,   175,   176,   404,
     405,   459,   460,   558,   559,   224,    92,   225,   226,   151,
      29,   195,   111,   186,   187,    30,    31,   277,   278,    32,
      33,    34,    35,   297,   359,   412,   413,   466,   216,    36,
      37,   217,   218,   306,   308,   369,   370,   421,   471,    38,
      39,    40,    41,   293,   294,   417,    42,    43,   290,   291,
     347,   348,    44,    45,    46,    99,   396,    47,   312,    48,
      49,    50,    51,    52,    93,    53,    54
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -425
static const short yypact[] =
{
    1018,   -62,    -2,  -425,    54,  -425,    43,    25,  -425,  -425,
      53,    56,  -425,    99,  -425,  -425,   183,   217,  1014,   -21,
      94,   182,   218,  -425,   104,  -425,  -425,  -425,  -425,  -425,
    -425,  -425,  -425,  -425,  -425,   185,  -425,  -425,  -425,   247,
    -425,  -425,  -425,  -425,  -425,  -425,  -425,  -425,  -425,  -425,
    -425,  -425,  -425,  -425,  -425,   265,   268,   125,   280,   182,
     281,   288,   290,   292,   184,  -425,  -425,  -425,  -425,   189,
     187,   202,   250,   251,   252,   254,  -425,   256,   257,   259,
     261,   263,   264,   641,  -425,   641,   641,   302,   -30,  -425,
      46,  1017,  -425,   287,   203,   272,   258,    68,  -425,   294,
      20,   283,  -425,  -425,   383,   331,  -425,  -425,   331,   331,
    -425,  -425,   291,  -425,   386,  -425,   -32,  -425,   -23,   387,
     331,   378,   304,    26,   317,   -47,  -425,  -425,  -425,  -425,
     469,   392,   641,   555,    -8,   555,   275,   641,   555,   555,
     641,   641,   641,   276,   278,   279,   284,  -425,  -425,   774,
     297,   124,    -1,   321,  -425,   397,  -425,   641,   641,   641,
     641,   641,   641,   641,   641,   641,   641,   641,   641,   641,
     641,   641,   641,   641,   400,  -425,  -425,    78,    68,   300,
     301,  -425,  -425,  -425,  -425,  -425,  -425,   303,   359,   329,
     332,   331,  -425,   333,   411,  -425,  -425,  -425,   322,  -425,
     418,   420,   406,   194,  -425,   306,  -425,   402,   347,  -425,
    -425,  -425,   204,    17,  -425,  -425,  -425,   305,  -425,     7,
     384,  -425,   618,  -425,  1017,   112,  -425,  -425,   801,   165,
     427,   311,   168,  -425,   829,   180,   207,   663,   684,   856,
    -425,   492,   641,   641,  -425,    10,   430,  -425,  -425,    21,
    -425,  -425,  -425,  -425,   600,  1031,  1044,  1055,   223,   223,
     162,   162,   162,   162,   196,   196,  -425,  -425,  -425,   315,
    -425,  -425,  -425,  -425,  -425,   434,  -425,  -425,  -425,   303,
     191,   320,    68,   388,   331,   331,  -425,  -425,  -425,   439,
    -425,  -425,   341,   103,  -425,  -425,   174,   352,   446,   447,
    -425,  -425,  -425,  -425,  -425,   220,   224,    26,   334,  -425,
    -425,  -425,   373,   -18,   321,   336,  -425,   555,  -425,  -425,
     338,  -425,  -425,  -425,  -425,  -425,  -425,   641,  -425,   641,
    -425,   719,   746,  -425,   354,   385,     5,   417,  -425,   457,
    -425,    10,  -425,   193,  -425,  -425,   358,   364,  -425,    55,
     418,   403,  -425,  -425,   349,   350,  -425,   351,  -425,   226,
     356,   242,  -425,  -425,    10,  -425,   471,   198,  -425,   360,
    -425,  -425,   472,   361,    10,   362,  -425,  -425,   884,   911,
      10,   321,   365,   363,     5,   382,  -425,  -425,     3,   482,
     389,  -425,   228,   -10,   416,  -425,  -425,   481,   439,    24,
    -425,  -425,  -425,   489,  -425,  -425,   380,   381,   390,  -425,
     174,    59,   377,  -425,  -425,  -425,  -425,  -425,  -425,  -425,
    -425,    15,    59,  -425,    10,  -425,   192,   391,  -425,  -425,
    -425,   -20,   437,   496,   -65,     5,    63,    -3,    31,    37,
      63,    63,    63,    63,   458,  -425,   484,   465,   444,  -425,
    -425,  -425,  -425,  -425,  -425,  -425,  -425,   231,   407,   393,
    -425,   401,  -425,  -425,  -425,    28,   233,  -425,   356,  -425,
    -425,  -425,   502,  -425,   235,  -425,  -425,  -425,   405,  -425,
    -425,   182,   501,   468,   412,  -425,  -425,   429,   431,  -425,
      10,  -425,  -425,  -425,  -425,  -425,  -425,  -425,  -425,  -425,
    -425,  -425,  -425,  -425,  -425,     4,   174,   470,   512,   287,
    -425,  -425,     9,   489,  -425,  -425,    59,  -425,  -425,   192,
     174,   532,  -425,  -425,    63,    63,   237,  -425,    10,  -425,
     419,   515,   170,   403,   422,  -425,  -425,   540,  -425,  -425,
    -425,   173,   423,  -425,   424,  -425,  -425,  -425,  -425,  -425,
     240,   174,   174,   428,   423,  -425,   539,   443,   243,  -425,
    -425,  -425,   174,   545,  -425,  -425,   423,   433,   435,    10,
    -425,   540,  -425,  -425,  -425,  -425,  -425,  -425
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -425,  -425,  -425,  -425,   448,  -425,   330,  -425,  -425,    36,
    -425,  -425,  -425,   222,   181,   414,  -425,  -425,    23,  -425,
     277,  -344,  -425,  -425,  -295,  -123,  -342,  -321,  -425,  -425,
    -425,  -425,  -425,  -425,  -425,  -424,    11,    60,  -425,    32,
    -425,  -425,    58,  -425,     1,    -6,  -425,   114,   262,  -425,
    -425,  -104,  -425,   293,   396,  -425,  -425,   296,  -425,  -425,
    -425,  -425,  -425,  -425,  -425,  -425,   110,  -425,  -304,  -425,
    -425,  -425,   273,  -425,  -425,  -425,   159,  -425,  -425,  -425,
    -425,  -425,  -425,  -425,   234,  -425,  -425,  -425,  -425,  -425,
    -425,   190,  -425,  -425,  -425,   565,  -425,  -425,  -425,  -425,
    -425,  -425,  -425,  -425,  -425,  -425,  -425
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -254
static const short yytable[] =
{
     214,   358,   115,   367,   196,   197,   489,   401,   352,   353,
      91,   152,   534,   527,   535,   209,   206,   536,   469,   451,
     392,   481,   209,   189,   436,   302,    55,   230,   190,   209,
     373,   209,   210,   209,   211,   354,   209,   210,   435,   491,
     434,   388,   209,   210,   116,   494,   309,    56,   200,   154,
     355,   437,   356,   220,   276,   202,   485,    57,   310,   203,
     209,   210,   155,   383,   209,   210,    59,   211,   209,   210,
     113,   181,   470,   182,   183,   201,   184,   147,   457,   148,
     149,   270,   125,   209,   452,    58,   271,   286,   374,   388,
     201,   486,   153,   311,   488,    60,   493,   496,   498,   500,
     502,   504,   153,   474,   357,   191,   231,   467,   554,   438,
     439,   440,   441,   442,   443,   464,   272,   490,   472,   248,
     444,   336,   333,   275,   528,   384,   228,    61,   566,   537,
     275,   234,   185,    62,   237,   238,   239,   275,   303,   212,
     388,   275,   273,   201,   212,   456,   213,    91,    63,   303,
     212,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   212,   526,
     274,   114,   212,   352,   353,   399,   212,   352,   353,   465,
     344,   345,   546,   548,   214,   115,    64,    65,    66,    67,
     560,   275,    68,    69,   270,   475,    91,   476,   157,   271,
     354,  -253,    70,   336,   354,    71,   561,   550,   100,   300,
     301,   529,   539,    72,   420,   355,   149,   356,   117,   355,
      94,   356,    73,   118,   158,   350,   400,    95,    74,    75,
      76,    77,   157,   316,   317,    78,   331,   332,   119,   477,
     553,   418,    79,   104,    80,   123,   246,   229,   247,   232,
     120,   425,   235,   236,   105,  -253,   565,   430,   158,   157,
      96,   179,   106,   107,   414,   393,   394,   180,   121,   357,
      81,   122,   395,   357,   169,   170,   171,   172,   173,   108,
      97,   415,    82,   124,   126,   158,   319,   317,   214,   322,
     317,   127,   110,   128,   416,   129,    83,    84,    98,   214,
      85,   324,   317,    86,   130,   150,    87,   132,    91,   177,
     171,   172,   173,   487,   131,   492,   495,   497,   499,   501,
     503,   378,   133,   379,   144,    65,    66,    67,   325,   317,
      68,   165,   166,   167,   168,   169,   170,   171,   172,   173,
      70,   363,   364,    71,   174,   365,   366,   409,   410,   450,
     364,    72,   511,   364,   515,   516,   518,   364,   549,   364,
      73,   564,   364,   192,   570,   571,    74,    75,    76,    77,
     134,   135,   136,    78,   137,    91,   138,   139,   178,   140,
      79,   141,    80,   142,   143,   188,   193,   194,   198,   199,
     205,   207,   219,   214,   208,   227,   233,   240,   241,   242,
     251,   545,   547,   245,   243,   269,   280,   281,    81,    64,
      65,    66,    67,   283,   284,    68,   282,   285,   287,   288,
      82,   292,   289,   295,   299,    70,   296,   307,    71,   298,
     320,   313,   321,   334,    83,    84,    72,   339,    85,   300,
     341,    86,   346,   343,    87,    73,   576,   349,   360,   361,
     362,    74,    75,    76,    77,   371,   372,   375,    78,   377,
     380,   389,   391,   381,   397,    79,   398,    80,   403,   406,
     407,   408,   144,    65,    66,    67,   411,   221,    68,   419,
     423,   424,   422,   433,   426,   435,   432,   445,   453,   454,
     447,    71,   458,    81,   461,   144,    65,    66,    67,   468,
     221,    68,   462,   482,   484,    82,   505,   506,    73,   507,
     508,   463,   480,   512,    71,   513,    76,    77,   420,    83,
      84,    78,   514,    85,   520,   521,    86,   519,   145,    87,
     146,    73,   524,   523,   525,   532,   531,   544,   552,    76,
      77,   551,   556,   557,    78,   562,   563,   568,   567,   569,
     573,   145,   315,   146,   574,   540,   575,   382,   144,    65,
      66,    67,   431,   221,    68,   555,   204,   250,    82,   533,
     351,   538,   577,   572,   279,   342,   340,    71,   517,   376,
     368,   473,    83,   112,   402,     0,    85,     0,   455,   222,
     223,    82,    87,     0,    73,     0,     0,     0,     0,     0,
       0,     0,    76,    77,     0,    83,     0,    78,     0,    85,
       0,     0,    86,   223,   145,    87,   146,     0,     0,     0,
       0,   144,    65,    66,    67,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   157,     0,     0,     0,
      71,     0,     0,     0,   144,    65,    66,    67,     0,     0,
      68,     0,     0,     0,    82,     0,     0,    73,     0,     0,
       0,     0,   158,    71,     0,    76,    77,     0,    83,     0,
      78,     0,    85,     0,     0,    86,     0,   145,    87,   146,
      73,     0,     0,     0,     0,     0,     0,     0,    76,    77,
       0,     0,     0,    78,     0,     0,   314,     0,     0,   157,
     145,     0,   146,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,    82,     0,     0,
     157,     0,     0,     0,     0,   158,     0,     0,     0,     0,
       0,    83,     0,     0,     0,    85,     0,     0,    86,     0,
      82,    87,     0,     0,     0,     0,   158,     0,     0,     0,
       0,     0,     0,     0,    83,   157,     0,     0,    85,     0,
       0,    86,     0,     0,    87,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
       0,   158,   157,     0,   326,   327,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,     0,     0,     0,     0,   328,   329,     0,   158,     0,
     157,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   158,   157,     0,     0,
       0,   327,     0,     0,     0,     0,     0,     0,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   158,     0,   157,     0,     0,   329,     0,
       0,     0,     0,     0,     0,     0,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   158,   157,     0,     0,   244,     0,     0,     0,     0,
       0,     0,     0,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   158,     0,
     157,     0,   318,     0,     0,     0,     0,     0,     0,     0,
       0,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   158,   157,     0,     0,
     323,     0,     0,     0,     0,     0,     0,     0,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   158,     0,     0,     0,   330,     0,     0,
       0,     0,     0,     0,     0,     0,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,     0,     0,     0,     0,   428,     0,     0,     0,     0,
       0,     0,     0,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   100,     0,
       0,     0,   429,     1,     0,     0,     2,     0,     3,   101,
     102,     0,     4,     0,     0,     0,     5,     0,     0,     6,
       7,     8,     9,   157,     0,    10,     0,   103,    11,     0,
       0,     0,     0,   104,     0,     0,     0,   157,    12,     0,
       0,     0,     0,     0,   105,     0,     0,     0,     0,   158,
     157,     0,   106,   107,     0,    13,     0,     0,     0,     0,
       0,   157,    14,   158,    15,    98,    16,     0,    17,   108,
      18,     0,    19,   109,     0,     0,   158,     0,     0,     0,
       0,    20,   110,    21,     0,     0,     0,   158,     0,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,     0,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,     0,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173
};

static const short yycheck[] =
{
     123,   296,     3,   307,   108,   109,     9,   349,     3,     4,
      16,    41,     3,     9,     5,     5,   120,     8,     3,    29,
     341,    41,     5,     3,    21,     8,    88,    35,     8,     5,
      48,     5,     6,     5,     8,    30,     5,     6,   103,     8,
     384,   336,     5,     6,    21,     8,    39,    49,    80,     3,
      45,    48,    47,   100,   177,    78,   121,     3,    51,    82,
       5,     6,    16,    58,     5,     6,    41,     8,     5,     6,
      91,     3,    57,     5,     6,   122,     8,    83,   399,    85,
      86,     3,    59,     5,    94,    42,     8,   191,   106,   384,
     122,   435,   122,    86,   436,    42,   438,   439,   440,   441,
     442,   443,   122,   424,    99,    85,   114,   411,   532,   106,
     107,   108,   109,   110,   111,   410,    38,   120,   422,   120,
     117,   100,   245,   113,   120,   120,   132,    71,   552,   120,
     113,   137,    64,    77,   140,   141,   142,   113,   121,   113,
     435,   113,    64,   122,   113,   121,   120,   153,    49,   121,
     113,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   113,   490,
      92,    77,   113,     3,     4,   120,   113,     3,     4,   120,
     284,   285,   524,   525,   307,     3,     3,     4,     5,     6,
      17,   113,     9,    10,     3,     3,   202,     5,    36,     8,
      30,     3,    19,   100,    30,    22,    33,   528,    14,     5,
       6,   506,   516,    30,    16,    45,   222,    47,     0,    45,
       3,    47,    39,   119,    62,   122,   349,    10,    45,    46,
      47,    48,    36,   121,   122,    52,   242,   243,    53,    47,
      70,   364,    59,    49,    61,   120,   122,   133,   124,   135,
       3,   374,   138,   139,    60,    57,   551,   380,    62,    36,
      43,     3,    68,    69,    22,    72,    73,     9,     3,    99,
      87,     3,    79,    99,   112,   113,   114,   115,   116,    85,
      63,    39,    99,     3,     3,    62,   121,   122,   411,   121,
     122,     3,    98,     3,    52,     3,   113,   114,    81,   422,
     117,   121,   122,   120,   120,     3,   123,   120,   314,   106,
     114,   115,   116,   436,   125,   438,   439,   440,   441,   442,
     443,   327,   120,   329,     3,     4,     5,     6,   121,   122,
       9,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      19,   121,   122,    22,    57,   121,   122,   121,   122,   121,
     122,    30,   121,   122,   121,   122,   121,   122,   121,   122,
      39,   121,   122,    80,   121,   122,    45,    46,    47,    48,
     120,   120,   120,    52,   120,   381,   120,   120,   106,   120,
      59,   120,    61,   120,   120,    91,     3,    56,    97,     3,
       3,    13,    75,   516,    90,     3,   121,   121,   120,   120,
       3,   524,   525,   106,   120,     5,   106,   106,    87,     3,
       4,     5,     6,    54,    85,     9,   113,    85,    85,     8,
      99,     3,   100,     3,    77,    19,   120,   122,    22,    27,
       3,    47,   121,     3,   113,   114,    30,   122,   117,     5,
     120,   120,     3,    55,   123,    39,   569,   106,    96,     3,
       3,    45,    46,    47,    48,   121,    83,   121,    52,   121,
     106,    44,     5,    78,   106,    59,   102,    61,    65,   120,
     120,   120,     3,     4,     5,     6,   120,     8,     9,     8,
       8,   120,   122,   120,   122,   103,   121,     5,    72,     8,
     101,    22,     3,    87,   114,     3,     4,     5,     6,   122,
       8,     9,   121,    66,     8,    99,    48,    23,    39,    44,
      66,   121,   121,   106,    22,   122,    47,    48,    16,   113,
     114,    52,   121,   117,    23,    57,   120,   122,    59,   123,
      61,    39,   103,   121,   103,    23,    66,     5,    23,    47,
      48,   122,   120,     3,    52,   122,   122,     8,   120,   106,
       5,    59,   222,    61,   121,   519,   121,   335,     3,     4,
       5,     6,   381,     8,     9,   533,   118,   153,    99,   509,
     293,   513,   571,   562,   178,   282,   280,    22,   468,   317,
     307,   422,   113,    18,   350,    -1,   117,    -1,   398,   120,
     121,    99,   123,    -1,    39,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,   113,    -1,    52,    -1,   117,
      -1,    -1,   120,   121,    59,   123,    61,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    -1,     9,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      22,    -1,    -1,    -1,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    -1,    -1,    99,    -1,    -1,    39,    -1,    -1,
      -1,    -1,    62,    22,    -1,    47,    48,    -1,   113,    -1,
      52,    -1,   117,    -1,    -1,   120,    -1,    59,   123,    61,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    -1,    -1,    52,    -1,    -1,    78,    -1,    -1,    36,
      59,    -1,    61,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    99,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,   117,    -1,    -1,   120,    -1,
      99,   123,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    36,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,   123,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    62,    36,    -1,   121,   122,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,   121,   122,    -1,    62,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    62,    36,    -1,    -1,
      -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    62,    -1,    36,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    62,    36,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    62,    -1,
      36,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    62,    36,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    62,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    14,    -1,
      -1,    -1,   121,    15,    -1,    -1,    18,    -1,    20,    25,
      26,    -1,    24,    -1,    -1,    -1,    28,    -1,    -1,    31,
      32,    33,    34,    36,    -1,    37,    -1,    43,    40,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    36,    50,    -1,
      -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    62,
      36,    -1,    68,    69,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    36,    74,    62,    76,    81,    78,    -1,    80,    85,
      82,    -1,    84,    89,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    93,    98,    95,    -1,    -1,    -1,    62,    -1,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    24,    28,    31,    32,    33,    34,
      37,    40,    50,    67,    74,    76,    78,    80,    82,    84,
      93,    95,   127,   128,   129,   130,   131,   132,   139,   176,
     181,   182,   185,   186,   187,   188,   195,   196,   205,   206,
     207,   208,   212,   213,   218,   219,   220,   223,   225,   226,
     227,   228,   229,   231,   232,    88,    49,     3,    42,    41,
      42,    71,    77,    49,     3,     4,     5,     6,     9,    10,
      19,    22,    30,    39,    45,    46,    47,    48,    52,    59,
      61,    87,    99,   113,   114,   117,   120,   123,   140,   141,
     143,   171,   172,   230,     3,    10,    43,    63,    81,   221,
      14,    25,    26,    43,    49,    60,    68,    69,    85,    89,
      98,   178,   221,    91,    77,     3,   144,     0,   119,    53,
       3,     3,     3,   120,     3,   144,     3,     3,     3,     3,
     120,   125,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,     3,    59,    61,   171,   171,   171,
       3,   175,    41,   122,     3,    16,   142,    36,    62,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    57,   163,   164,   106,   106,     3,
       9,     3,     5,     6,     8,    64,   179,   180,    91,     3,
       8,    85,    80,     3,    56,   177,   177,   177,    97,     3,
      80,   122,    78,    82,   130,     3,   177,    13,    90,     5,
       6,     8,   113,   120,   151,   152,   194,   197,   198,    75,
     100,     8,   120,   121,   171,   173,   174,     3,   171,   173,
      35,   114,   173,   121,   171,   173,   173,   171,   171,   171,
     121,   120,   120,   120,   121,   106,   122,   124,   120,   144,
     141,     3,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,     5,
       3,     8,    38,    64,    92,   113,   151,   183,   184,   180,
     106,   106,   113,    54,    85,    85,   177,    85,     8,   100,
     214,   215,     3,   209,   210,     3,   120,   189,    27,    77,
       5,     6,     8,   121,   151,   153,   199,   122,   200,    39,
      51,    86,   224,    47,    78,   132,   121,   122,   121,   121,
       3,   121,   121,   121,   121,   121,   121,   122,   121,   122,
     121,   171,   171,   151,     3,   136,   100,   145,   146,   122,
     183,   120,   179,    55,   177,   177,     3,   216,   217,   106,
     122,   146,     3,     4,    30,    45,    47,    99,   150,   190,
      96,     3,     3,   121,   122,   121,   122,   194,   198,   201,
     202,   121,    83,    48,   106,   121,   174,   121,   171,   171,
     106,    78,   139,    58,   120,   147,   148,   149,   150,    44,
     154,     5,   153,    72,    73,    79,   222,   106,   102,   120,
     151,   152,   210,    65,   165,   166,   120,   120,   120,   121,
     122,   120,   191,   192,    22,    39,    52,   211,   151,     8,
      16,   203,   122,     8,   120,   151,   122,   133,   121,   121,
     151,   140,   121,   120,   147,   103,    21,    48,   106,   107,
     108,   109,   110,   111,   117,     5,   155,   101,   157,   158,
     121,    29,    94,    72,     8,   217,   121,   153,     3,   167,
     168,   114,   121,   121,   150,   120,   193,   194,   122,     3,
      57,   204,   194,   202,   153,     3,     5,    47,   134,   135,
     121,    41,    66,   137,     8,   121,   147,   151,   152,     9,
     120,     8,   151,   152,     8,   151,   152,   151,   152,   151,
     152,   151,   152,   151,   152,    48,    23,    44,    66,   159,
     160,   121,   106,   122,   121,   121,   122,   192,   121,   122,
      23,    57,   138,   121,   103,   103,   153,     9,   120,   150,
     156,    66,    23,   163,     3,     5,     8,   120,   168,   194,
     135,   150,   161,   162,     5,   151,   152,   151,   152,   121,
     153,   122,    23,    70,   161,   165,   120,     3,   169,   170,
      17,    33,   122,   122,   121,   150,   161,   120,     8,   106,
     121,   122,   162,     5,   121,   121,   151,   170
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
			pParser->m_pStmt->m_sTableFunc = yyvsp[-6].m_sValue;
		;}
    break;

  case 34:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 37:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 38:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 40:

    { yyval.m_sValue.SetSprintf ( "%lld", INT64(yyvsp[0].m_iValue) ); ;}
    break;

  case 41:

    { yyval.m_sValue = "id"; ;}
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
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
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
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
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

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
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
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
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
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 99:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 100:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 101:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 102:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 103:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 104:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 105:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
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
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 113:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 116:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 119:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 120:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 122:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 125:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
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
			yyval.m_sValue = yyvsp[-2].m_sValue;
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

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 150:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 151:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 152:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 153:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 154:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 155:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 156:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 157:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 158:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 159:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 160:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 161:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 162:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 163:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 164:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 165:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 166:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 167:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 168:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 169:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 171:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 172:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 173:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 174:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 175:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 176:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 177:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 178:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 179:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 184:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 185:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 188:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
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
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 197:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
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
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
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
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
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
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
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

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 233:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 234:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 235:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 236:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 237:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 238:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 239:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 240:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 241:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 242:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 243:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 245:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
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
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 247:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
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
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 256:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
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
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 265:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 266:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 267:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
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

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			tStmt.m_sIndex = yyvsp[-4].m_sValue;
			tStmt.m_sAlterAttr = yyvsp[-1].m_sValue;
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 272:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 279:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 280:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 281:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 289:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 290:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 291:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 292:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 293:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 294:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 295:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 296:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 297:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 299:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 300:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 301:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
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

