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
     TOK_READ = 326,
     TOK_REPEATABLE = 327,
     TOK_REPLACE = 328,
     TOK_RETURNS = 329,
     TOK_ROLLBACK = 330,
     TOK_RTINDEX = 331,
     TOK_SELECT = 332,
     TOK_SERIALIZABLE = 333,
     TOK_SET = 334,
     TOK_SESSION = 335,
     TOK_SHOW = 336,
     TOK_SONAME = 337,
     TOK_START = 338,
     TOK_STATUS = 339,
     TOK_STRING = 340,
     TOK_SUM = 341,
     TOK_TABLE = 342,
     TOK_TABLES = 343,
     TOK_TO = 344,
     TOK_TRANSACTION = 345,
     TOK_TRUE = 346,
     TOK_TRUNCATE = 347,
     TOK_UNCOMMITTED = 348,
     TOK_UPDATE = 349,
     TOK_VALUES = 350,
     TOK_VARIABLES = 351,
     TOK_WARNINGS = 352,
     TOK_WEIGHT = 353,
     TOK_WHERE = 354,
     TOK_WITHIN = 355,
     TOK_OR = 356,
     TOK_AND = 357,
     TOK_NE = 358,
     TOK_GTE = 359,
     TOK_LTE = 360,
     TOK_NOT = 361,
     TOK_NEG = 362
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
#define TOK_READ 326
#define TOK_REPEATABLE 327
#define TOK_REPLACE 328
#define TOK_RETURNS 329
#define TOK_ROLLBACK 330
#define TOK_RTINDEX 331
#define TOK_SELECT 332
#define TOK_SERIALIZABLE 333
#define TOK_SET 334
#define TOK_SESSION 335
#define TOK_SHOW 336
#define TOK_SONAME 337
#define TOK_START 338
#define TOK_STATUS 339
#define TOK_STRING 340
#define TOK_SUM 341
#define TOK_TABLE 342
#define TOK_TABLES 343
#define TOK_TO 344
#define TOK_TRANSACTION 345
#define TOK_TRUE 346
#define TOK_TRUNCATE 347
#define TOK_UNCOMMITTED 348
#define TOK_UPDATE 349
#define TOK_VALUES 350
#define TOK_VARIABLES 351
#define TOK_WARNINGS 352
#define TOK_WEIGHT 353
#define TOK_WHERE 354
#define TOK_WITHIN 355
#define TOK_OR 356
#define TOK_AND 357
#define TOK_NE 358
#define TOK_GTE 359
#define TOK_LTE 360
#define TOK_NOT 361
#define TOK_NEG 362




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
#define YYFINAL  115
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1115

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  125
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  106
/* YYNRULES -- Number of rules. */
#define YYNRULES  299
/* YYNRULES -- Number of states. */
#define YYNSTATES  575

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   362

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   115,   104,     2,
     119,   120,   113,   111,   121,   112,   124,   114,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   118,
     107,   105,   108,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   122,   103,   123,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   101,   102,   106,   109,
     110,   116,   117
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    58,    60,
      62,    64,    73,    75,    85,    86,    89,    91,    95,    97,
      99,   101,   102,   106,   107,   110,   115,   126,   128,   132,
     134,   137,   138,   140,   143,   145,   150,   155,   160,   165,
     170,   175,   179,   185,   187,   191,   192,   194,   197,   199,
     203,   207,   212,   216,   220,   226,   233,   237,   242,   248,
     252,   256,   260,   264,   268,   270,   276,   282,   288,   292,
     296,   300,   304,   308,   312,   316,   318,   320,   325,   329,
     333,   335,   337,   340,   342,   345,   347,   351,   352,   357,
     358,   360,   362,   366,   367,   369,   375,   376,   378,   382,
     388,   390,   394,   396,   399,   402,   403,   405,   408,   413,
     414,   416,   419,   421,   425,   429,   433,   439,   446,   450,
     452,   456,   460,   462,   464,   466,   468,   470,   472,   475,
     478,   482,   486,   490,   494,   498,   502,   506,   510,   514,
     518,   522,   526,   530,   534,   538,   542,   546,   550,   554,
     556,   561,   566,   571,   576,   581,   585,   592,   599,   603,
     605,   609,   611,   613,   617,   623,   626,   627,   630,   632,
     635,   638,   642,   644,   646,   651,   656,   660,   662,   664,
     666,   668,   670,   672,   676,   681,   686,   691,   695,   700,
     708,   714,   716,   718,   720,   722,   724,   726,   728,   730,
     732,   735,   742,   744,   746,   747,   751,   753,   757,   759,
     763,   767,   769,   773,   775,   777,   779,   783,   786,   794,
     804,   811,   813,   817,   819,   823,   825,   829,   830,   833,
     835,   839,   843,   844,   846,   848,   850,   854,   856,   858,
     862,   869,   871,   875,   879,   883,   889,   894,   896,   898,
     900,   908,   913,   914,   916,   919,   921,   925,   929,   932,
     936,   943,   944,   946,   948,   951,   954,   957,   959,   967,
     969,   971,   973,   977,   984,   988,   992,   994,   998,  1002
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     126,     0,    -1,   127,    -1,   128,    -1,   128,   118,    -1,
     186,    -1,   194,    -1,   180,    -1,   181,    -1,   184,    -1,
     195,    -1,   204,    -1,   206,    -1,   207,    -1,   212,    -1,
     217,    -1,   218,    -1,   222,    -1,   224,    -1,   225,    -1,
     226,    -1,   219,    -1,   227,    -1,   229,    -1,   230,    -1,
     211,    -1,   129,    -1,   128,   118,   129,    -1,   130,    -1,
     175,    -1,   131,    -1,    77,     3,   119,   119,   131,   120,
     132,   120,    -1,   138,    -1,    77,   139,    41,   119,   135,
     138,   120,   136,   137,    -1,    -1,   121,   133,    -1,   134,
      -1,   133,   121,   134,    -1,     3,    -1,     5,    -1,    47,
      -1,    -1,    66,    23,   160,    -1,    -1,    57,     5,    -1,
      57,     5,   121,     5,    -1,    77,   139,    41,   143,   144,
     153,   156,   158,   162,   164,    -1,   140,    -1,   139,   121,
     140,    -1,   113,    -1,   142,   141,    -1,    -1,     3,    -1,
      16,     3,    -1,   170,    -1,    19,   119,   170,   120,    -1,
      59,   119,   170,   120,    -1,    61,   119,   170,   120,    -1,
      86,   119,   170,   120,    -1,    46,   119,   170,   120,    -1,
      30,   119,   113,   120,    -1,    45,   119,   120,    -1,    30,
     119,    35,     3,   120,    -1,     3,    -1,   143,   121,     3,
      -1,    -1,   145,    -1,    99,   146,    -1,   147,    -1,   146,
     102,   146,    -1,   119,   146,   120,    -1,    58,   119,     8,
     120,    -1,   149,   105,   150,    -1,   149,   106,   150,    -1,
     149,    48,   119,   152,   120,    -1,   149,   116,    48,   119,
     152,   120,    -1,   149,    48,     9,    -1,   149,   116,    48,
       9,    -1,   149,    21,   150,   102,   150,    -1,   149,   108,
     150,    -1,   149,   107,   150,    -1,   149,   109,   150,    -1,
     149,   110,   150,    -1,   149,   105,   151,    -1,   148,    -1,
     149,    21,   151,   102,   151,    -1,   149,    21,   150,   102,
     151,    -1,   149,    21,   151,   102,   150,    -1,   149,   108,
     151,    -1,   149,   107,   151,    -1,   149,   109,   151,    -1,
     149,   110,   151,    -1,   149,   105,     8,    -1,   149,   106,
       8,    -1,   149,   106,   151,    -1,     3,    -1,     4,    -1,
      30,   119,   113,   120,    -1,    45,   119,   120,    -1,    98,
     119,   120,    -1,    47,    -1,     5,    -1,   112,     5,    -1,
       6,    -1,   112,     6,    -1,   150,    -1,   152,   121,   150,
      -1,    -1,    44,   154,    23,   155,    -1,    -1,     5,    -1,
     149,    -1,   155,   121,   149,    -1,    -1,   157,    -1,   100,
      44,    66,    23,   160,    -1,    -1,   159,    -1,    66,    23,
     160,    -1,    66,    23,    70,   119,   120,    -1,   161,    -1,
     160,   121,   161,    -1,   149,    -1,   149,    17,    -1,   149,
      33,    -1,    -1,   163,    -1,    57,     5,    -1,    57,     5,
     121,     5,    -1,    -1,   165,    -1,    65,   166,    -1,   167,
      -1,   166,   121,   167,    -1,     3,   105,     3,    -1,     3,
     105,     5,    -1,     3,   105,   119,   168,   120,    -1,     3,
     105,     3,   119,     8,   120,    -1,     3,   105,     8,    -1,
     169,    -1,   168,   121,   169,    -1,     3,   105,   150,    -1,
       3,    -1,     4,    -1,    47,    -1,     5,    -1,     6,    -1,
       9,    -1,   112,   170,    -1,   116,   170,    -1,   170,   111,
     170,    -1,   170,   112,   170,    -1,   170,   113,   170,    -1,
     170,   114,   170,    -1,   170,   107,   170,    -1,   170,   108,
     170,    -1,   170,   104,   170,    -1,   170,   103,   170,    -1,
     170,   115,   170,    -1,   170,    36,   170,    -1,   170,    62,
     170,    -1,   170,   110,   170,    -1,   170,   109,   170,    -1,
     170,   105,   170,    -1,   170,   106,   170,    -1,   170,   102,
     170,    -1,   170,   101,   170,    -1,   119,   170,   120,    -1,
     122,   174,   123,    -1,   171,    -1,     3,   119,   172,   120,
      -1,    48,   119,   172,   120,    -1,    52,   119,   172,   120,
      -1,    22,   119,   172,   120,    -1,    39,   119,   172,   120,
      -1,     3,   119,   120,    -1,    61,   119,   170,   121,   170,
     120,    -1,    59,   119,   170,   121,   170,   120,    -1,    98,
     119,   120,    -1,   173,    -1,   172,   121,   173,    -1,   170,
      -1,     8,    -1,     3,   105,   150,    -1,   174,   121,     3,
     105,   150,    -1,    81,   177,    -1,    -1,    56,     8,    -1,
      97,    -1,    84,   176,    -1,    60,   176,    -1,    14,    84,
     176,    -1,    69,    -1,    68,    -1,    14,     8,    84,   176,
      -1,    14,     3,    84,   176,    -1,    49,     3,    84,    -1,
       3,    -1,    64,    -1,     8,    -1,     5,    -1,     6,    -1,
     178,    -1,   179,   112,   178,    -1,    79,     3,   105,   183,
      -1,    79,     3,   105,   182,    -1,    79,     3,   105,    64,
      -1,    79,    63,   179,    -1,    79,    10,   105,   179,    -1,
      79,    43,     9,   105,   119,   152,   120,    -1,    79,    43,
       3,   105,   182,    -1,     3,    -1,     8,    -1,    91,    -1,
      38,    -1,   150,    -1,    28,    -1,    75,    -1,   185,    -1,
      20,    -1,    83,    90,    -1,   187,    53,     3,   188,    95,
     190,    -1,    50,    -1,    73,    -1,    -1,   119,   189,   120,
      -1,   149,    -1,   189,   121,   149,    -1,   191,    -1,   190,
     121,   191,    -1,   119,   192,   120,    -1,   193,    -1,   192,
     121,   193,    -1,   150,    -1,   151,    -1,     8,    -1,   119,
     152,   120,    -1,   119,   120,    -1,    32,    41,   143,    99,
      47,   105,   150,    -1,    32,    41,   143,    99,    47,    48,
     119,   152,   120,    -1,    24,     3,   119,   196,   199,   120,
      -1,   197,    -1,   196,   121,   197,    -1,   193,    -1,   119,
     198,   120,    -1,     8,    -1,   198,   121,     8,    -1,    -1,
     121,   200,    -1,   201,    -1,   200,   121,   201,    -1,   193,
     202,   203,    -1,    -1,    16,    -1,     3,    -1,    57,    -1,
     205,     3,   176,    -1,    34,    -1,    33,    -1,    81,    88,
     176,    -1,    94,   143,    79,   208,   145,   164,    -1,   209,
      -1,   208,   121,   209,    -1,     3,   105,   150,    -1,     3,
     105,   151,    -1,     3,   105,   119,   152,   120,    -1,     3,
     105,   119,   120,    -1,    52,    -1,    22,    -1,    39,    -1,
      15,    87,     3,    13,    27,     3,   210,    -1,    81,   220,
      96,   213,    -1,    -1,   214,    -1,    99,   215,    -1,   216,
      -1,   215,   101,   216,    -1,     3,   105,     8,    -1,    81,
      26,    -1,    81,    25,    79,    -1,    79,   220,    90,    54,
      55,   221,    -1,    -1,    43,    -1,    80,    -1,    71,    93,
      -1,    71,    29,    -1,    72,    71,    -1,    78,    -1,    31,
      42,     3,    74,   223,    82,     8,    -1,    51,    -1,    39,
      -1,    85,    -1,    37,    42,     3,    -1,    18,    49,     3,
      89,    76,     3,    -1,    40,    76,     3,    -1,    77,   228,
     162,    -1,    10,    -1,    10,   124,     3,    -1,    92,    76,
       3,    -1,    67,    49,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   136,   136,   137,   138,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   168,   169,   173,   174,
     178,   179,   187,   188,   195,   197,   201,   205,   212,   213,
     214,   218,   231,   239,   241,   246,   255,   271,   272,   276,
     277,   280,   282,   283,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   299,   300,   303,   305,   309,   313,   314,
     315,   319,   324,   331,   339,   347,   356,   361,   366,   371,
     376,   381,   386,   391,   396,   401,   406,   411,   416,   421,
     426,   431,   436,   441,   449,   453,   454,   459,   465,   471,
     477,   486,   487,   498,   499,   503,   509,   515,   517,   520,
     522,   529,   533,   539,   541,   545,   556,   558,   562,   566,
     573,   574,   578,   579,   580,   583,   585,   589,   594,   601,
     603,   607,   611,   612,   616,   621,   626,   632,   637,   645,
     650,   657,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   683,   684,
     685,   686,   687,   688,   689,   690,   691,   692,   693,   694,
     698,   699,   700,   701,   702,   703,   704,   705,   706,   710,
     711,   715,   716,   720,   721,   727,   730,   732,   736,   737,
     738,   739,   740,   741,   742,   747,   752,   762,   763,   764,
     765,   766,   770,   771,   775,   780,   785,   790,   791,   795,
     800,   808,   809,   813,   814,   815,   829,   830,   831,   835,
     836,   842,   850,   851,   854,   856,   860,   861,   865,   866,
     870,   874,   875,   879,   880,   881,   882,   883,   889,   897,
     911,   919,   923,   930,   931,   938,   948,   954,   956,   960,
     965,   969,   976,   978,   982,   983,   989,   997,   998,  1004,
    1010,  1018,  1019,  1023,  1027,  1031,  1035,  1045,  1046,  1047,
    1051,  1064,  1070,  1071,  1075,  1079,  1080,  1084,  1088,  1095,
    1102,  1108,  1109,  1110,  1114,  1115,  1116,  1117,  1123,  1134,
    1135,  1136,  1140,  1151,  1163,  1174,  1182,  1183,  1192,  1203
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
  "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PROFILE", "TOK_RAND", "TOK_READ", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_STRING", "TOK_SUM", "TOK_TABLE", "TOK_TABLES", "TOK_TO", 
  "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", 
  "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", 
  "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", 
  "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", 
  "','", "'{'", "'}'", "'.'", "$accept", "request", "statement", 
  "multi_stmt_list", "multi_stmt", "select", "select1", 
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
  "attach_index", "flush_rtindex", "select_sysvar", "sysvar_name", 
  "truncate", "optimize_index", 0
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
     355,   356,   357,   124,    38,    61,   358,    60,    62,   359,
     360,    43,    45,    42,    47,    37,   361,   362,    59,    40,
      41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   125,   126,   126,   126,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   128,   128,   129,   129,
     130,   130,   131,   131,   132,   132,   133,   133,   134,   134,
     134,   135,   136,   137,   137,   137,   138,   139,   139,   140,
     140,   141,   141,   141,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   143,   143,   144,   144,   145,   146,   146,
     146,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   148,   149,   149,   149,   149,   149,
     149,   150,   150,   151,   151,   152,   152,   153,   153,   154,
     154,   155,   155,   156,   156,   157,   158,   158,   159,   159,
     160,   160,   161,   161,   161,   162,   162,   163,   163,   164,
     164,   165,   166,   166,   167,   167,   167,   167,   167,   168,
     168,   169,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   172,
     172,   173,   173,   174,   174,   175,   176,   176,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   178,   178,   178,
     178,   178,   179,   179,   180,   180,   180,   180,   180,   181,
     181,   182,   182,   183,   183,   183,   184,   184,   184,   185,
     185,   186,   187,   187,   188,   188,   189,   189,   190,   190,
     191,   192,   192,   193,   193,   193,   193,   193,   194,   194,
     195,   196,   196,   197,   197,   198,   198,   199,   199,   200,
     200,   201,   202,   202,   203,   203,   204,   205,   205,   206,
     207,   208,   208,   209,   209,   209,   209,   210,   210,   210,
     211,   212,   213,   213,   214,   215,   215,   216,   217,   218,
     219,   220,   220,   220,   221,   221,   221,   221,   222,   223,
     223,   223,   224,   225,   226,   227,   228,   228,   229,   230
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     8,     1,     9,     0,     2,     1,     3,     1,     1,
       1,     0,     3,     0,     2,     4,    10,     1,     3,     1,
       2,     0,     1,     2,     1,     4,     4,     4,     4,     4,
       4,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       3,     4,     3,     3,     5,     6,     3,     4,     5,     3,
       3,     3,     3,     3,     1,     5,     5,     5,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     4,     3,     3,
       1,     1,     2,     1,     2,     1,     3,     0,     4,     0,
       1,     1,     3,     0,     1,     5,     0,     1,     3,     5,
       1,     3,     1,     2,     2,     0,     1,     2,     4,     0,
       1,     2,     1,     3,     3,     3,     5,     6,     3,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       4,     4,     4,     4,     4,     3,     6,     6,     3,     1,
       3,     1,     1,     3,     5,     2,     0,     2,     1,     2,
       2,     3,     1,     1,     4,     4,     3,     1,     1,     1,
       1,     1,     1,     3,     4,     4,     4,     3,     4,     7,
       5,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     6,     1,     1,     0,     3,     1,     3,     1,     3,
       3,     1,     3,     1,     1,     1,     3,     2,     7,     9,
       6,     1,     3,     1,     3,     1,     3,     0,     2,     1,
       3,     3,     0,     1,     1,     1,     3,     1,     1,     3,
       6,     1,     3,     3,     3,     5,     4,     1,     1,     1,
       7,     4,     0,     1,     2,     1,     3,     3,     2,     3,
       6,     0,     1,     1,     2,     2,     2,     1,     7,     1,
       1,     1,     3,     6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   219,     0,   216,     0,     0,   258,   257,
       0,     0,   222,     0,   223,   217,     0,   281,   281,     0,
       0,     0,     0,     2,     3,    26,    28,    30,    32,    29,
       7,     8,     9,   218,     5,     0,     6,    10,    11,     0,
      12,    13,    25,    14,    15,    16,    21,    17,    18,    19,
      20,    22,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,   142,   143,   145,   146,   147,   296,     0,     0,
       0,     0,     0,     0,   144,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,    47,    51,    54,
     169,   125,     0,     0,   282,     0,   283,     0,     0,     0,
     278,   282,     0,   186,   193,   192,   186,   186,   188,   185,
       0,   220,     0,    63,     0,     1,     4,     0,   186,     0,
       0,     0,     0,     0,   292,   294,   299,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   142,     0,     0,   148,   149,     0,     0,     0,     0,
       0,    52,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   295,   126,     0,     0,     0,     0,   197,   200,
     201,   199,   198,   202,   207,     0,     0,     0,   186,   279,
       0,     0,   190,   189,   259,   272,   298,     0,     0,     0,
       0,    27,   224,   256,     0,     0,   101,   103,   235,     0,
       0,   233,   234,   243,   247,   241,     0,     0,   182,     0,
     175,   181,     0,   179,   297,     0,     0,     0,     0,     0,
      61,     0,     0,     0,     0,     0,     0,   178,     0,     0,
       0,   167,     0,     0,   168,    41,    65,    48,    53,   159,
     160,   166,   165,   157,   156,   163,   164,   154,   155,   162,
     161,   150,   151,   152,   153,   158,   127,   211,   212,   214,
     206,   213,     0,   215,   205,   204,   208,     0,     0,     0,
       0,   186,   186,   191,   196,   187,     0,   271,   273,     0,
       0,   261,    64,     0,     0,     0,     0,   102,   104,   245,
     237,   105,     0,     0,     0,     0,   290,   289,   291,     0,
       0,     0,     0,   170,     0,    55,   173,     0,    60,   174,
      59,   171,   172,    56,     0,    57,     0,    58,     0,     0,
     183,     0,     0,     0,   107,    66,     0,   210,     0,   203,
       0,   195,   194,     0,   274,   275,     0,     0,   129,    95,
      96,     0,     0,   100,     0,   226,     0,     0,     0,   293,
     236,     0,   244,     0,   243,   242,   248,   249,   240,     0,
       0,     0,    34,   180,    62,     0,     0,     0,     0,     0,
       0,     0,    67,    68,    84,     0,   109,   113,   128,     0,
       0,     0,   287,   280,     0,     0,     0,   263,   264,   262,
       0,   260,   130,     0,     0,     0,   225,     0,     0,   221,
     228,   268,   269,   267,   270,   106,   246,   253,     0,     0,
     288,     0,   238,     0,     0,   177,   176,   184,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   110,     0,     0,   116,   114,   209,   285,   284,
     286,   277,   276,   266,     0,     0,   131,   132,     0,    98,
      99,   227,     0,     0,   231,     0,   254,   255,   251,   252,
     250,     0,    38,    39,    40,    35,    36,    31,     0,     0,
      43,     0,    70,    69,     0,     0,    76,     0,    92,    72,
      83,    93,    73,    94,    80,    89,    79,    88,    81,    90,
      82,    91,     0,     0,     0,     0,   125,   117,   265,     0,
       0,    97,   230,     0,   229,   239,     0,     0,     0,    33,
      71,     0,     0,     0,    77,     0,   111,   108,     0,     0,
     129,   134,   135,   138,     0,   133,   232,    37,   122,    42,
     120,    44,    78,    86,    87,    85,    74,     0,     0,     0,
       0,   118,    46,     0,     0,     0,   139,   123,   124,     0,
       0,    75,   112,   115,     0,     0,     0,   136,     0,   121,
      45,   119,   137,   141,   140
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   424,   475,   476,
     332,   480,   519,    28,    86,    87,   153,    88,   246,   334,
     335,   382,   383,   384,   538,   301,   212,   302,   387,   443,
     527,   445,   446,   506,   507,   539,   540,   172,   173,   401,
     402,   456,   457,   555,   556,   221,    90,   222,   223,   148,
      29,   192,   109,   183,   184,    30,    31,   274,   275,    32,
      33,    34,    35,   294,   356,   409,   410,   463,   213,    36,
      37,   214,   215,   303,   305,   366,   367,   418,   468,    38,
      39,    40,    41,   290,   291,   414,    42,    43,   287,   288,
     344,   345,    44,    45,    46,    97,   393,    47,   309,    48,
      49,    50,    51,    91,    52,    53
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -507
static const short yypact[] =
{
     887,   -23,    21,  -507,   102,  -507,   127,    18,  -507,  -507,
     167,   135,  -507,   165,  -507,  -507,    26,   188,   883,   126,
     154,   225,   238,  -507,   124,  -507,  -507,  -507,  -507,  -507,
    -507,  -507,  -507,  -507,  -507,   202,  -507,  -507,  -507,   246,
    -507,  -507,  -507,  -507,  -507,  -507,  -507,  -507,  -507,  -507,
    -507,  -507,  -507,  -507,   249,   274,   139,   276,   225,   287,
     289,   305,   192,  -507,  -507,  -507,  -507,   206,   214,   216,
     217,   226,   245,   254,  -507,   261,   265,   270,   277,   278,
     279,   569,  -507,   569,   569,   335,   -28,  -507,    23,   886,
    -507,   338,   294,   297,    11,    49,  -507,   313,    16,   326,
    -507,  -507,   404,   356,  -507,  -507,   356,   356,  -507,  -507,
     319,  -507,   413,  -507,   -32,  -507,    34,   414,   356,   406,
     331,    78,   348,    19,  -507,  -507,  -507,   382,   420,   569,
     467,    15,   467,   304,   569,   467,   467,   569,   569,   569,
     306,   309,   312,   316,  -507,  -507,   646,   332,    83,     8,
     215,  -507,   422,  -507,   569,   569,   569,   569,   569,   569,
     569,   569,   569,   569,   569,   569,   569,   569,   569,   569,
     569,   433,  -507,  -507,   184,    49,   341,   342,  -507,  -507,
    -507,  -507,  -507,  -507,   328,   394,   365,   366,   356,  -507,
     367,   446,  -507,  -507,  -507,   357,  -507,   452,   455,   320,
     272,  -507,   340,  -507,   434,   384,  -507,  -507,  -507,   259,
      17,  -507,  -507,  -507,   346,  -507,   208,   415,  -507,   507,
    -507,   886,   150,  -507,  -507,   673,   152,   460,   349,   168,
    -507,   700,   179,   189,   429,   531,   727,  -507,   405,   569,
     569,  -507,    10,   465,  -507,  -507,    80,  -507,  -507,  -507,
    -507,   903,   551,   920,   940,  1000,  1000,   191,   191,   191,
     191,   233,   233,  -507,  -507,  -507,   353,  -507,  -507,  -507,
    -507,  -507,   472,  -507,  -507,  -507,   328,   185,   359,    49,
     424,   356,   356,  -507,  -507,  -507,   478,  -507,  -507,   377,
      87,  -507,  -507,   199,   388,   481,   482,  -507,  -507,  -507,
    -507,  -507,   223,   232,    78,   368,  -507,  -507,  -507,   408,
      27,   215,   372,  -507,   467,  -507,  -507,   373,  -507,  -507,
    -507,  -507,  -507,  -507,   569,  -507,   569,  -507,   591,   612,
    -507,   381,   410,     4,   451,  -507,   491,  -507,    10,  -507,
     161,  -507,  -507,   392,   398,  -507,    63,   452,   435,  -507,
    -507,   386,   389,  -507,   390,  -507,   237,   399,   155,  -507,
    -507,    10,  -507,   499,    25,  -507,   401,  -507,  -507,   512,
     426,    10,   402,  -507,  -507,   754,   781,    10,   215,   427,
     432,     4,   450,  -507,  -507,   175,   543,   453,  -507,   240,
     -12,   485,  -507,  -507,   549,   478,    56,  -507,  -507,  -507,
     555,  -507,  -507,   447,   441,   442,  -507,   199,    98,   443,
    -507,  -507,  -507,  -507,  -507,  -507,  -507,  -507,    57,    98,
    -507,    10,  -507,    41,   449,  -507,  -507,  -507,   -14,   497,
     562,    29,     4,    61,     7,    32,   166,    61,    61,    61,
      61,   523,  -507,   553,   533,   514,  -507,  -507,  -507,  -507,
    -507,  -507,  -507,  -507,   242,   476,   461,  -507,   468,  -507,
    -507,  -507,    58,   250,  -507,   399,  -507,  -507,  -507,   574,
    -507,   255,  -507,  -507,  -507,   464,  -507,  -507,   225,   571,
     535,   475,  -507,  -507,   494,   495,  -507,    10,  -507,  -507,
    -507,  -507,  -507,  -507,  -507,  -507,  -507,  -507,  -507,  -507,
    -507,  -507,    24,   199,   532,   576,   338,  -507,  -507,    -2,
     555,  -507,  -507,    98,  -507,  -507,    41,   199,   595,  -507,
    -507,    61,    61,   257,  -507,    10,  -507,   480,   579,   196,
     435,   484,  -507,  -507,   601,  -507,  -507,  -507,   103,   486,
    -507,   488,  -507,  -507,  -507,  -507,  -507,   262,   199,   199,
     487,   486,  -507,   602,   506,   273,  -507,  -507,  -507,   199,
     607,  -507,  -507,   486,   498,   500,    10,  -507,   601,  -507,
    -507,  -507,  -507,  -507,  -507
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -507,  -507,  -507,  -507,   508,  -507,   395,  -507,  -507,    99,
    -507,  -507,  -507,   290,   247,   518,  -507,  -507,    89,  -507,
     360,  -323,  -507,  -507,  -291,  -121,  -341,  -320,  -507,  -507,
    -507,  -507,  -507,  -507,  -507,  -506,    72,   141,  -507,   119,
    -507,  -507,   159,  -507,   104,    -4,  -507,   219,   361,  -507,
    -507,   -97,  -507,   391,   496,  -507,  -507,   396,  -507,  -507,
    -507,  -507,  -507,  -507,  -507,  -507,   211,  -507,  -300,  -507,
    -507,  -507,   374,  -507,  -507,  -507,   258,  -507,  -507,  -507,
    -507,  -507,  -507,  -507,   333,  -507,  -507,  -507,  -507,  -507,
    -507,   284,  -507,  -507,  -507,   665,  -507,  -507,  -507,  -507,
    -507,  -507,  -507,  -507,  -507,  -507
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -253
static const short yytable[] =
{
     211,   531,   355,   532,   364,   398,   533,   349,   350,   193,
     194,   113,    89,   149,   176,   206,   486,   448,   389,   186,
     177,   203,   206,   551,   187,   299,   151,   478,  -252,    62,
      63,    64,    65,   524,   351,    66,    67,   206,   207,   152,
     488,   417,   385,   563,   472,    68,   473,   197,    69,   352,
     227,   353,   178,   273,   179,   180,    70,   181,   431,    58,
     466,   206,   380,   206,    54,    71,   206,   207,   206,   207,
      55,    72,    73,    74,    75,   370,   454,   144,    76,   145,
     146,   449,  -252,   206,   207,    77,   208,    78,   474,   198,
     385,   283,   485,   150,   490,   493,   495,   497,   499,   501,
     188,   471,   354,   206,   207,    56,   208,   150,   464,   483,
     114,   199,    79,   182,   467,   200,   461,   534,   217,   469,
     557,   330,   272,   381,    80,   225,   487,   245,   228,   272,
     231,   432,   371,   234,   235,   236,   558,   300,    81,    82,
     198,   385,    83,   525,   209,    84,    89,   123,    85,   482,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   523,   272,    57,
     272,   206,   207,   209,   491,   209,   453,   411,   300,   333,
     543,   545,   396,   211,   341,   342,   333,   267,   267,   206,
     209,    92,   268,   268,   412,    89,   433,   210,    93,   349,
     350,   198,   349,   350,   243,   547,   244,   413,   347,    59,
     209,    60,   526,   536,    61,   146,   111,   462,   141,    63,
      64,    65,   269,   434,    66,   397,   351,   154,   113,   351,
     112,    94,   390,   391,    68,   328,   329,    69,   115,   392,
     415,   352,   116,   353,   352,    70,   353,   306,   270,   118,
     422,    95,   119,   155,    71,   117,   427,   562,   121,   307,
      72,    73,    74,    75,   297,   298,   550,    76,    96,   154,
     313,   314,   316,   314,    77,   271,    78,   120,   209,   122,
     435,   436,   437,   438,   439,   440,    98,   211,   319,   314,
     124,   441,   125,   308,   354,   155,   272,   354,   211,   321,
     314,    79,   166,   167,   168,   169,   170,    89,   126,   322,
     314,   127,   484,    80,   489,   492,   494,   496,   498,   500,
     375,   102,   376,    62,    63,    64,    65,    81,    82,    66,
     128,    83,   103,   129,    84,   130,   131,    85,   147,    68,
     104,   105,    69,   360,   361,   132,   168,   169,   170,   226,
      70,   229,   362,   363,   232,   233,   106,   406,   407,    71,
     447,   361,   508,   361,   133,    72,    73,    74,    75,   108,
     512,   513,    76,   134,    89,   515,   361,   546,   361,    77,
     135,    78,   561,   361,   136,   141,    63,    64,    65,   137,
     218,    66,   211,   567,   568,   171,   138,   139,   140,   174,
     542,   544,   175,   185,    69,   189,    79,   190,   141,    63,
      64,    65,   191,   218,    66,   195,   196,   202,    80,   204,
     205,    71,   216,   224,   230,   248,   237,    69,   238,    74,
      75,   239,    81,    82,    76,   240,    83,   242,   266,    84,
     279,   142,    85,   143,    71,   573,   277,   278,   280,   281,
     282,   284,    74,    75,   285,   289,   286,    76,   292,   293,
     296,   295,   310,   317,   142,   154,   143,   304,   331,   318,
     141,    63,    64,    65,   336,   218,    66,   297,   338,   340,
      80,   343,   346,   357,   358,   359,   377,   378,   368,    69,
     369,   155,   372,   374,    81,   386,   388,   394,    83,   395,
     400,   219,   220,    80,    85,   403,    71,   416,   404,   405,
     141,    63,    64,    65,    74,    75,    66,    81,   408,    76,
     420,    83,   419,   423,    84,   220,   142,    85,   143,    69,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   421,    71,   429,   442,   323,
     324,   430,   432,   444,    74,    75,   450,   451,   455,    76,
     458,   459,   460,   479,   465,    80,   142,   154,   143,   477,
     481,   502,   141,    63,    64,    65,   503,   504,    66,    81,
     505,   509,   510,    83,   311,   516,    84,   154,   511,    85,
     417,    69,   518,   155,   517,   520,   521,   522,   528,   529,
     541,   548,   549,   553,   554,    80,   564,   559,    71,   560,
     565,   566,   570,   155,   312,   537,    74,    75,   571,    81,
     572,    76,   379,    83,   201,   428,    84,   154,   142,    85,
     143,   569,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   530,   154,   552,
     348,   325,   326,   155,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,    80,   247,   535,
     339,   276,   574,   337,   155,   373,   514,   470,   365,   452,
     399,    81,   154,   110,     0,    83,     0,     0,    84,     0,
       0,    85,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,     0,   155,   154,
       0,     0,   324,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,     0,     0,
       0,     0,     0,   326,     0,   155,   154,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   155,   154,     0,     0,   241,     0,     0,     0,
       0,     0,     0,     0,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   155,
     154,     0,     0,   315,     0,     0,     0,     0,     0,     0,
       0,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   155,   154,     0,     0,
     320,     0,     0,     0,     0,     0,     0,     0,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   155,     0,     0,     0,   327,     0,     0,
       0,     0,     0,     0,     0,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
       0,     0,     0,     0,   425,     0,     0,     0,     0,     0,
       0,     0,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,    98,     0,     0,
       0,   426,     1,     0,     0,     2,     0,     3,    99,   100,
       0,     4,     0,     0,     0,     5,     0,     0,     6,     7,
       8,     9,   154,     0,    10,     0,   101,    11,     0,     0,
       0,     0,   102,     0,     0,     0,     0,    12,     0,   154,
       0,     0,     0,   103,     0,     0,     0,     0,   155,     0,
       0,   104,   105,     0,    13,     0,   154,     0,     0,     0,
      14,     0,    15,    96,    16,   155,    17,   106,    18,     0,
      19,   107,     0,     0,     0,     0,   154,     0,     0,    20,
     108,    21,   155,     0,     0,     0,     0,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   155,     0,     0,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,     0,
       0,     0,     0,     0,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   154,     0,     0,     0,
       0,     0,     0,     0,     0,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,     0,     0,     0,     0,
       0,     0,   155,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   162,   163,   164,
     165,   166,   167,   168,   169,   170
};

static const short yycheck[] =
{
     121,     3,   293,     5,   304,   346,     8,     3,     4,   106,
     107,     3,    16,    41,     3,     5,     9,    29,   338,     3,
       9,   118,     5,   529,     8,     8,     3,    41,     3,     3,
       4,     5,     6,     9,    30,     9,    10,     5,     6,    16,
       8,    16,   333,   549,     3,    19,     5,    79,    22,    45,
      35,    47,     3,   174,     5,     6,    30,     8,   381,    41,
       3,     5,    58,     5,    87,    39,     5,     6,     5,     6,
      49,    45,    46,    47,    48,    48,   396,    81,    52,    83,
      84,    93,    57,     5,     6,    59,     8,    61,    47,   121,
     381,   188,   433,   121,   435,   436,   437,   438,   439,   440,
      84,   421,    98,     5,     6,     3,     8,   121,   408,   432,
      21,    77,    86,    64,    57,    81,   407,   119,    99,   419,
      17,   242,   112,   119,    98,   129,   119,   119,   113,   112,
     134,   102,   105,   137,   138,   139,    33,   120,   112,   113,
     121,   432,   116,   119,   112,   119,   150,    58,   122,   120,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   487,   112,    42,
     112,     5,     6,   112,     8,   112,   120,    22,   120,    99,
     521,   522,   119,   304,   281,   282,    99,     3,     3,     5,
     112,     3,     8,     8,    39,   199,    21,   119,    10,     3,
       4,   121,     3,     4,   121,   525,   123,    52,   121,    42,
     112,    76,   503,   513,    49,   219,    90,   119,     3,     4,
       5,     6,    38,    48,     9,   346,    30,    36,     3,    30,
      76,    43,    71,    72,    19,   239,   240,    22,     0,    78,
     361,    45,   118,    47,    45,    30,    47,    39,    64,     3,
     371,    63,     3,    62,    39,    53,   377,   548,   119,    51,
      45,    46,    47,    48,     5,     6,    70,    52,    80,    36,
     120,   121,   120,   121,    59,    91,    61,     3,   112,     3,
     105,   106,   107,   108,   109,   110,    14,   408,   120,   121,
       3,   116,     3,    85,    98,    62,   112,    98,   419,   120,
     121,    86,   111,   112,   113,   114,   115,   311,     3,   120,
     121,   119,   433,    98,   435,   436,   437,   438,   439,   440,
     324,    49,   326,     3,     4,     5,     6,   112,   113,     9,
     124,   116,    60,   119,   119,   119,   119,   122,     3,    19,
      68,    69,    22,   120,   121,   119,   113,   114,   115,   130,
      30,   132,   120,   121,   135,   136,    84,   120,   121,    39,
     120,   121,   120,   121,   119,    45,    46,    47,    48,    97,
     120,   121,    52,   119,   378,   120,   121,   120,   121,    59,
     119,    61,   120,   121,   119,     3,     4,     5,     6,   119,
       8,     9,   513,   120,   121,    57,   119,   119,   119,   105,
     521,   522,   105,    90,    22,    79,    86,     3,     3,     4,
       5,     6,    56,     8,     9,    96,     3,     3,    98,    13,
      89,    39,    74,     3,   120,     3,   120,    22,   119,    47,
      48,   119,   112,   113,    52,   119,   116,   105,     5,   119,
     112,    59,   122,    61,    39,   566,   105,   105,    54,    84,
      84,    84,    47,    48,     8,     3,    99,    52,     3,   119,
      76,    27,    47,     3,    59,    36,    61,   121,     3,   120,
       3,     4,     5,     6,   121,     8,     9,     5,   119,    55,
      98,     3,   105,    95,     3,     3,   105,    77,   120,    22,
      82,    62,   120,   120,   112,    44,     5,   105,   116,   101,
      65,   119,   120,    98,   122,   119,    39,     8,   119,   119,
       3,     4,     5,     6,    47,    48,     9,   112,   119,    52,
       8,   116,   121,   121,   119,   120,    59,   122,    61,    22,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   119,    39,   120,     5,   120,
     121,   119,   102,   100,    47,    48,    71,     8,     3,    52,
     113,   120,   120,    66,   121,    98,    59,    36,    61,   120,
       8,    48,     3,     4,     5,     6,    23,    44,     9,   112,
      66,   105,   121,   116,    77,   121,   119,    36,   120,   122,
      16,    22,    57,    62,    23,   120,   102,   102,    66,    23,
       5,   121,    23,   119,     3,    98,   119,   121,    39,   121,
       8,   105,     5,    62,   219,   516,    47,    48,   120,   112,
     120,    52,   332,   116,   116,   378,   119,    36,    59,   122,
      61,   559,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   506,    36,   530,
     290,   120,   121,    62,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    98,   150,   510,
     279,   175,   568,   277,    62,   314,   465,   419,   304,   395,
     347,   112,    36,    18,    -1,   116,    -1,    -1,   119,    -1,
      -1,   122,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    -1,    62,    36,
      -1,    -1,   121,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    62,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    62,    36,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    62,
      36,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    62,    36,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    62,    -1,    -1,    -1,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
      -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    14,    -1,    -1,
      -1,   120,    15,    -1,    -1,    18,    -1,    20,    25,    26,
      -1,    24,    -1,    -1,    -1,    28,    -1,    -1,    31,    32,
      33,    34,    36,    -1,    37,    -1,    43,    40,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    50,    -1,    36,
      -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    68,    69,    -1,    67,    -1,    36,    -1,    -1,    -1,
      73,    -1,    75,    80,    77,    62,    79,    84,    81,    -1,
      83,    88,    -1,    -1,    -1,    -1,    36,    -1,    -1,    92,
      97,    94,    62,    -1,    -1,    -1,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    62,    -1,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    24,    28,    31,    32,    33,    34,
      37,    40,    50,    67,    73,    75,    77,    79,    81,    83,
      92,    94,   126,   127,   128,   129,   130,   131,   138,   175,
     180,   181,   184,   185,   186,   187,   194,   195,   204,   205,
     206,   207,   211,   212,   217,   218,   219,   222,   224,   225,
     226,   227,   229,   230,    87,    49,     3,    42,    41,    42,
      76,    49,     3,     4,     5,     6,     9,    10,    19,    22,
      30,    39,    45,    46,    47,    48,    52,    59,    61,    86,
      98,   112,   113,   116,   119,   122,   139,   140,   142,   170,
     171,   228,     3,    10,    43,    63,    80,   220,    14,    25,
      26,    43,    49,    60,    68,    69,    84,    88,    97,   177,
     220,    90,    76,     3,   143,     0,   118,    53,     3,     3,
       3,   119,     3,   143,     3,     3,     3,   119,   124,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,     3,    59,    61,   170,   170,   170,     3,   174,    41,
     121,     3,    16,   141,    36,    62,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    57,   162,   163,   105,   105,     3,     9,     3,     5,
       6,     8,    64,   178,   179,    90,     3,     8,    84,    79,
       3,    56,   176,   176,   176,    96,     3,    79,   121,    77,
      81,   129,     3,   176,    13,    89,     5,     6,     8,   112,
     119,   150,   151,   193,   196,   197,    74,    99,     8,   119,
     120,   170,   172,   173,     3,   170,   172,    35,   113,   172,
     120,   170,   172,   172,   170,   170,   170,   120,   119,   119,
     119,   120,   105,   121,   123,   119,   143,   140,     3,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,     5,     3,     8,    38,
      64,    91,   112,   150,   182,   183,   179,   105,   105,   112,
      54,    84,    84,   176,    84,     8,    99,   213,   214,     3,
     208,   209,     3,   119,   188,    27,    76,     5,     6,     8,
     120,   150,   152,   198,   121,   199,    39,    51,    85,   223,
      47,    77,   131,   120,   121,   120,   120,     3,   120,   120,
     120,   120,   120,   120,   121,   120,   121,   120,   170,   170,
     150,     3,   135,    99,   144,   145,   121,   182,   119,   178,
      55,   176,   176,     3,   215,   216,   105,   121,   145,     3,
       4,    30,    45,    47,    98,   149,   189,    95,     3,     3,
     120,   121,   120,   121,   193,   197,   200,   201,   120,    82,
      48,   105,   120,   173,   120,   170,   170,   105,    77,   138,
      58,   119,   146,   147,   148,   149,    44,   153,     5,   152,
      71,    72,    78,   221,   105,   101,   119,   150,   151,   209,
      65,   164,   165,   119,   119,   119,   120,   121,   119,   190,
     191,    22,    39,    52,   210,   150,     8,    16,   202,   121,
       8,   119,   150,   121,   132,   120,   120,   150,   139,   120,
     119,   146,   102,    21,    48,   105,   106,   107,   108,   109,
     110,   116,     5,   154,   100,   156,   157,   120,    29,    93,
      71,     8,   216,   120,   152,     3,   166,   167,   113,   120,
     120,   149,   119,   192,   193,   121,     3,    57,   203,   193,
     201,   152,     3,     5,    47,   133,   134,   120,    41,    66,
     136,     8,   120,   146,   150,   151,     9,   119,     8,   150,
     151,     8,   150,   151,   150,   151,   150,   151,   150,   151,
     150,   151,    48,    23,    44,    66,   158,   159,   120,   105,
     121,   120,   120,   121,   191,   120,   121,    23,    57,   137,
     120,   102,   102,   152,     9,   119,   149,   155,    66,    23,
     162,     3,     5,     8,   119,   167,   193,   134,   149,   160,
     161,     5,   150,   151,   150,   151,   120,   152,   121,    23,
      70,   160,   164,   119,     3,   168,   169,    17,    33,   121,
     121,   120,   149,   160,   119,     8,   105,   120,   121,   161,
       5,   120,   120,   150,   169
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

  case 26:

    { pParser->PushQuery(); ;}
    break;

  case 27:

    { pParser->PushQuery(); ;}
    break;

  case 31:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->m_pStmt->m_sTableFunc = yyvsp[-6].m_sValue;
		;}
    break;

  case 33:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 36:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 37:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 39:

    { yyval.m_sValue.SetSprintf ( "%d", yyvsp[0].m_iValue ); ;}
    break;

  case 40:

    { yyval.m_sValue = "id"; ;}
    break;

  case 41:

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

  case 42:

    {
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 44:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 45:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 46:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 49:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 52:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 53:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 54:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 55:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 56:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 59:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 60:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 61:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 62:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 64:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 71:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 72:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 73:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 74:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 75:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 76:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 96:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 97:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 98:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 99:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 100:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 101:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 102:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 103:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 104:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 105:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 106:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 110:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 111:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 112:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 115:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 118:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 119:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 121:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 123:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 127:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 128:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
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
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 137:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 140:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 141:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 143:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 144:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 148:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 149:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 150:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 170:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 176:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 177:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 178:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 183:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 184:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 187:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 188:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 190:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 191:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 192:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 193:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 194:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 195:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 204:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 205:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 206:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 207:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 208:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 209:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 210:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 213:

    { yyval.m_iValue = 1; ;}
    break;

  case 214:

    { yyval.m_iValue = 0; ;}
    break;

  case 215:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 216:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 217:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 218:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 221:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 222:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 223:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 226:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 227:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 230:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 231:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 232:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 233:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 234:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 235:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 236:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 237:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 238:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 239:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 240:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 241:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 242:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 244:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 245:

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

  case 246:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 249:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 251:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 255:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 256:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 259:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 260:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 263:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 264:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 265:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 266:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 267:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 268:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 269:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 270:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			tStmt.m_sIndex = yyvsp[-4].m_sValue;
			tStmt.m_sAlterAttr = yyvsp[-1].m_sValue;
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 271:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 278:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 279:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 280:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 288:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 289:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 290:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 291:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 292:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 293:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 294:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 295:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 297:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 298:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 299:

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

