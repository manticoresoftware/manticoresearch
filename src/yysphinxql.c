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
#define YYLAST   1161

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  125
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  105
/* YYNRULES -- Number of rules. */
#define YYNRULES  297
/* YYNRULES -- Number of states. */
#define YYNSTATES  573

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
     333,   335,   337,   340,   342,   345,   347,   351,   352,   356,
     358,   362,   363,   365,   371,   372,   374,   378,   384,   386,
     390,   392,   395,   398,   399,   401,   404,   409,   410,   412,
     415,   417,   421,   425,   429,   435,   442,   446,   448,   452,
     456,   458,   460,   462,   464,   466,   468,   471,   474,   478,
     482,   486,   490,   494,   498,   502,   506,   510,   514,   518,
     522,   526,   530,   534,   538,   542,   546,   550,   552,   557,
     562,   567,   572,   577,   581,   588,   595,   599,   601,   605,
     607,   609,   613,   619,   622,   623,   626,   628,   631,   634,
     638,   640,   642,   647,   652,   656,   658,   660,   662,   664,
     666,   668,   672,   677,   682,   687,   691,   696,   704,   710,
     712,   714,   716,   718,   720,   722,   724,   726,   728,   731,
     738,   740,   742,   743,   747,   749,   753,   755,   759,   763,
     765,   769,   771,   773,   775,   779,   782,   790,   800,   807,
     809,   813,   815,   819,   821,   825,   826,   829,   831,   835,
     839,   840,   842,   844,   846,   850,   852,   854,   858,   865,
     867,   871,   875,   879,   885,   890,   892,   894,   896,   904,
     909,   910,   912,   915,   917,   921,   925,   928,   932,   939,
     940,   942,   944,   947,   950,   953,   955,   963,   965,   967,
     969,   973,   980,   984,   988,   990,   994,   998
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     126,     0,    -1,   127,    -1,   128,    -1,   128,   118,    -1,
     185,    -1,   193,    -1,   179,    -1,   180,    -1,   183,    -1,
     194,    -1,   203,    -1,   205,    -1,   206,    -1,   211,    -1,
     216,    -1,   217,    -1,   221,    -1,   223,    -1,   224,    -1,
     225,    -1,   218,    -1,   226,    -1,   228,    -1,   229,    -1,
     210,    -1,   129,    -1,   128,   118,   129,    -1,   130,    -1,
     174,    -1,   131,    -1,    77,     3,   119,   119,   131,   120,
     132,   120,    -1,   138,    -1,    77,   139,    41,   119,   135,
     138,   120,   136,   137,    -1,    -1,   121,   133,    -1,   134,
      -1,   133,   121,   134,    -1,     3,    -1,     5,    -1,    47,
      -1,    -1,    66,    23,   159,    -1,    -1,    57,     5,    -1,
      57,     5,   121,     5,    -1,    77,   139,    41,   143,   144,
     153,   155,   157,   161,   163,    -1,   140,    -1,   139,   121,
     140,    -1,   113,    -1,   142,   141,    -1,    -1,     3,    -1,
      16,     3,    -1,   169,    -1,    19,   119,   169,   120,    -1,
      59,   119,   169,   120,    -1,    61,   119,   169,   120,    -1,
      86,   119,   169,   120,    -1,    46,   119,   169,   120,    -1,
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
      -1,    -1,    44,    23,   154,    -1,   149,    -1,   154,   121,
     149,    -1,    -1,   156,    -1,   100,    44,    66,    23,   159,
      -1,    -1,   158,    -1,    66,    23,   159,    -1,    66,    23,
      70,   119,   120,    -1,   160,    -1,   159,   121,   160,    -1,
     149,    -1,   149,    17,    -1,   149,    33,    -1,    -1,   162,
      -1,    57,     5,    -1,    57,     5,   121,     5,    -1,    -1,
     164,    -1,    65,   165,    -1,   166,    -1,   165,   121,   166,
      -1,     3,   105,     3,    -1,     3,   105,     5,    -1,     3,
     105,   119,   167,   120,    -1,     3,   105,     3,   119,     8,
     120,    -1,     3,   105,     8,    -1,   168,    -1,   167,   121,
     168,    -1,     3,   105,   150,    -1,     3,    -1,     4,    -1,
      47,    -1,     5,    -1,     6,    -1,     9,    -1,   112,   169,
      -1,   116,   169,    -1,   169,   111,   169,    -1,   169,   112,
     169,    -1,   169,   113,   169,    -1,   169,   114,   169,    -1,
     169,   107,   169,    -1,   169,   108,   169,    -1,   169,   104,
     169,    -1,   169,   103,   169,    -1,   169,   115,   169,    -1,
     169,    36,   169,    -1,   169,    62,   169,    -1,   169,   110,
     169,    -1,   169,   109,   169,    -1,   169,   105,   169,    -1,
     169,   106,   169,    -1,   169,   102,   169,    -1,   169,   101,
     169,    -1,   119,   169,   120,    -1,   122,   173,   123,    -1,
     170,    -1,     3,   119,   171,   120,    -1,    48,   119,   171,
     120,    -1,    52,   119,   171,   120,    -1,    22,   119,   171,
     120,    -1,    39,   119,   171,   120,    -1,     3,   119,   120,
      -1,    61,   119,   169,   121,   169,   120,    -1,    59,   119,
     169,   121,   169,   120,    -1,    98,   119,   120,    -1,   172,
      -1,   171,   121,   172,    -1,   169,    -1,     8,    -1,     3,
     105,   150,    -1,   173,   121,     3,   105,   150,    -1,    81,
     176,    -1,    -1,    56,     8,    -1,    97,    -1,    84,   175,
      -1,    60,   175,    -1,    14,    84,   175,    -1,    69,    -1,
      68,    -1,    14,     8,    84,   175,    -1,    14,     3,    84,
     175,    -1,    49,     3,    84,    -1,     3,    -1,    64,    -1,
       8,    -1,     5,    -1,     6,    -1,   177,    -1,   178,   112,
     177,    -1,    79,     3,   105,   182,    -1,    79,     3,   105,
     181,    -1,    79,     3,   105,    64,    -1,    79,    63,   178,
      -1,    79,    10,   105,   178,    -1,    79,    43,     9,   105,
     119,   152,   120,    -1,    79,    43,     3,   105,   181,    -1,
       3,    -1,     8,    -1,    91,    -1,    38,    -1,   150,    -1,
      28,    -1,    75,    -1,   184,    -1,    20,    -1,    83,    90,
      -1,   186,    53,     3,   187,    95,   189,    -1,    50,    -1,
      73,    -1,    -1,   119,   188,   120,    -1,   149,    -1,   188,
     121,   149,    -1,   190,    -1,   189,   121,   190,    -1,   119,
     191,   120,    -1,   192,    -1,   191,   121,   192,    -1,   150,
      -1,   151,    -1,     8,    -1,   119,   152,   120,    -1,   119,
     120,    -1,    32,    41,   143,    99,    47,   105,   150,    -1,
      32,    41,   143,    99,    47,    48,   119,   152,   120,    -1,
      24,     3,   119,   195,   198,   120,    -1,   196,    -1,   195,
     121,   196,    -1,   192,    -1,   119,   197,   120,    -1,     8,
      -1,   197,   121,     8,    -1,    -1,   121,   199,    -1,   200,
      -1,   199,   121,   200,    -1,   192,   201,   202,    -1,    -1,
      16,    -1,     3,    -1,    57,    -1,   204,     3,   175,    -1,
      34,    -1,    33,    -1,    81,    88,   175,    -1,    94,   143,
      79,   207,   145,   163,    -1,   208,    -1,   207,   121,   208,
      -1,     3,   105,   150,    -1,     3,   105,   151,    -1,     3,
     105,   119,   152,   120,    -1,     3,   105,   119,   120,    -1,
      52,    -1,    22,    -1,    39,    -1,    15,    87,     3,    13,
      27,     3,   209,    -1,    81,   219,    96,   212,    -1,    -1,
     213,    -1,    99,   214,    -1,   215,    -1,   214,   101,   215,
      -1,     3,   105,     8,    -1,    81,    26,    -1,    81,    25,
      79,    -1,    79,   219,    90,    54,    55,   220,    -1,    -1,
      43,    -1,    80,    -1,    71,    93,    -1,    71,    29,    -1,
      72,    71,    -1,    78,    -1,    31,    42,     3,    74,   222,
      82,     8,    -1,    51,    -1,    39,    -1,    85,    -1,    37,
      42,     3,    -1,    18,    49,     3,    89,    76,     3,    -1,
      40,    76,     3,    -1,    77,   227,   161,    -1,    10,    -1,
      10,   124,     3,    -1,    92,    76,     3,    -1,    67,    49,
       3,    -1
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
     477,   486,   487,   498,   499,   503,   509,   515,   517,   521,
     525,   531,   533,   537,   548,   550,   554,   558,   565,   566,
     570,   571,   572,   575,   577,   581,   586,   593,   595,   599,
     603,   604,   608,   613,   618,   624,   629,   637,   642,   649,
     659,   660,   661,   662,   663,   664,   665,   666,   667,   668,
     669,   670,   671,   672,   673,   674,   675,   676,   677,   678,
     679,   680,   681,   682,   683,   684,   685,   686,   690,   691,
     692,   693,   694,   695,   696,   697,   698,   702,   703,   707,
     708,   712,   713,   719,   722,   724,   728,   729,   730,   731,
     732,   733,   734,   739,   744,   754,   755,   756,   757,   758,
     762,   763,   767,   772,   777,   782,   783,   787,   792,   800,
     801,   805,   806,   807,   821,   822,   823,   827,   828,   834,
     842,   843,   846,   848,   852,   853,   857,   858,   862,   866,
     867,   871,   872,   873,   874,   875,   881,   889,   903,   911,
     915,   922,   923,   930,   940,   946,   948,   952,   957,   961,
     968,   970,   974,   975,   981,   989,   990,   996,  1002,  1010,
    1011,  1015,  1019,  1023,  1027,  1037,  1038,  1039,  1043,  1056,
    1062,  1063,  1067,  1071,  1072,  1076,  1080,  1087,  1094,  1100,
    1101,  1102,  1106,  1107,  1108,  1109,  1115,  1126,  1127,  1128,
    1132,  1143,  1155,  1166,  1174,  1175,  1184,  1195
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
  "const_float", "const_list", "opt_group_clause", "group_items_list", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "consthash", "show_stmt", "like_filter", "show_what", 
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
     154,   155,   155,   156,   157,   157,   158,   158,   159,   159,
     160,   160,   160,   161,   161,   162,   162,   163,   163,   164,
     165,   165,   166,   166,   166,   166,   166,   167,   167,   168,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   171,   171,   172,
     172,   173,   173,   174,   175,   175,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   177,   177,   177,   177,   177,
     178,   178,   179,   179,   179,   179,   179,   180,   180,   181,
     181,   182,   182,   182,   183,   183,   183,   184,   184,   185,
     186,   186,   187,   187,   188,   188,   189,   189,   190,   191,
     191,   192,   192,   192,   192,   192,   193,   193,   194,   195,
     195,   196,   196,   197,   197,   198,   198,   199,   199,   200,
     201,   201,   202,   202,   203,   204,   204,   205,   206,   207,
     207,   208,   208,   208,   208,   209,   209,   209,   210,   211,
     212,   212,   213,   214,   214,   215,   216,   217,   218,   219,
     219,   219,   220,   220,   220,   220,   221,   222,   222,   222,
     223,   224,   225,   226,   227,   227,   228,   229
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
       1,     1,     2,     1,     2,     1,     3,     0,     3,     1,
       3,     0,     1,     5,     0,     1,     3,     5,     1,     3,
       1,     2,     2,     0,     1,     2,     4,     0,     1,     2,
       1,     3,     3,     3,     5,     6,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     4,     4,
       4,     4,     4,     3,     6,     6,     3,     1,     3,     1,
       1,     3,     5,     2,     0,     2,     1,     2,     2,     3,
       1,     1,     4,     4,     3,     1,     1,     1,     1,     1,
       1,     3,     4,     4,     4,     3,     4,     7,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     6,
       1,     1,     0,     3,     1,     3,     1,     3,     3,     1,
       3,     1,     1,     1,     3,     2,     7,     9,     6,     1,
       3,     1,     3,     1,     3,     0,     2,     1,     3,     3,
       0,     1,     1,     1,     3,     1,     1,     3,     6,     1,
       3,     3,     3,     5,     4,     1,     1,     1,     7,     4,
       0,     1,     2,     1,     3,     3,     2,     3,     6,     0,
       1,     1,     2,     2,     2,     1,     7,     1,     1,     1,
       3,     6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   217,     0,   214,     0,     0,   256,   255,
       0,     0,   220,     0,   221,   215,     0,   279,   279,     0,
       0,     0,     0,     2,     3,    26,    28,    30,    32,    29,
       7,     8,     9,   216,     5,     0,     6,    10,    11,     0,
      12,    13,    25,    14,    15,    16,    21,    17,    18,    19,
      20,    22,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,   140,   141,   143,   144,   145,   294,     0,     0,
       0,     0,     0,     0,   142,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,    47,    51,    54,
     167,   123,     0,     0,   280,     0,   281,     0,     0,     0,
     276,   280,     0,   184,   191,   190,   184,   184,   186,   183,
       0,   218,     0,    63,     0,     1,     4,     0,   184,     0,
       0,     0,     0,     0,   290,   292,   297,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   140,     0,     0,   146,   147,     0,     0,     0,     0,
       0,    52,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   293,   124,     0,     0,     0,     0,   195,   198,
     199,   197,   196,   200,   205,     0,     0,     0,   184,   277,
       0,     0,   188,   187,   257,   270,   296,     0,     0,     0,
       0,    27,   222,   254,     0,     0,   101,   103,   233,     0,
       0,   231,   232,   241,   245,   239,     0,     0,   180,     0,
     173,   179,     0,   177,   295,     0,     0,     0,     0,     0,
      61,     0,     0,     0,     0,     0,     0,   176,     0,     0,
       0,   165,     0,     0,   166,    41,    65,    48,    53,   157,
     158,   164,   163,   155,   154,   161,   162,   152,   153,   160,
     159,   148,   149,   150,   151,   156,   125,   209,   210,   212,
     204,   211,     0,   213,   203,   202,   206,     0,     0,     0,
       0,   184,   184,   189,   194,   185,     0,   269,   271,     0,
       0,   259,    64,     0,     0,     0,     0,   102,   104,   243,
     235,   105,     0,     0,     0,     0,   288,   287,   289,     0,
       0,     0,     0,   168,     0,    55,   171,     0,    60,   172,
      59,   169,   170,    56,     0,    57,     0,    58,     0,     0,
     181,     0,     0,     0,   107,    66,     0,   208,     0,   201,
       0,   193,   192,     0,   272,   273,     0,     0,   127,    95,
      96,     0,     0,   100,     0,   224,     0,     0,     0,   291,
     234,     0,   242,     0,   241,   240,   246,   247,   238,     0,
       0,     0,    34,   178,    62,     0,     0,     0,     0,     0,
       0,     0,    67,    68,    84,     0,     0,   111,   126,     0,
       0,     0,   285,   278,     0,     0,     0,   261,   262,   260,
       0,   258,   128,     0,     0,     0,   223,     0,     0,   219,
     226,   266,   267,   265,   268,   106,   244,   251,     0,     0,
     286,     0,   236,     0,     0,   175,   174,   182,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   114,   112,   207,   283,   282,   284,
     275,   274,   264,     0,     0,   129,   130,     0,    98,    99,
     225,     0,     0,   229,     0,   252,   253,   249,   250,   248,
       0,    38,    39,    40,    35,    36,    31,     0,     0,    43,
       0,    70,    69,     0,     0,    76,     0,    92,    72,    83,
      93,    73,    94,    80,    89,    79,    88,    81,    90,    82,
      91,     0,   109,   108,     0,     0,   123,   115,   263,     0,
       0,    97,   228,     0,   227,   237,     0,     0,     0,    33,
      71,     0,     0,     0,    77,     0,     0,     0,     0,   127,
     132,   133,   136,     0,   131,   230,    37,   120,    42,   118,
      44,    78,    86,    87,    85,    74,     0,   110,     0,     0,
     116,    46,     0,     0,     0,   137,   121,   122,     0,     0,
      75,   113,     0,     0,     0,   134,     0,   119,    45,   117,
     135,   139,   138
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   424,   474,   475,
     332,   479,   519,    28,    86,    87,   153,    88,   246,   334,
     335,   382,   383,   384,   537,   301,   212,   302,   387,   503,
     444,   445,   506,   507,   538,   539,   172,   173,   401,   402,
     455,   456,   554,   555,   221,    90,   222,   223,   148,    29,
     192,   109,   183,   184,    30,    31,   274,   275,    32,    33,
      34,    35,   294,   356,   409,   410,   462,   213,    36,    37,
     214,   215,   303,   305,   366,   367,   418,   467,    38,    39,
      40,    41,   290,   291,   414,    42,    43,   287,   288,   344,
     345,    44,    45,    46,    97,   393,    47,   309,    48,    49,
      50,    51,    91,    52,    53
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -411
static const short yypact[] =
{
     958,   -45,    52,  -411,    48,  -411,    68,    87,  -411,  -411,
     144,   155,  -411,   190,  -411,  -411,   223,    64,   610,   154,
     173,   243,   251,  -411,   136,  -411,  -411,  -411,  -411,  -411,
    -411,  -411,  -411,  -411,  -411,   202,  -411,  -411,  -411,   256,
    -411,  -411,  -411,  -411,  -411,  -411,  -411,  -411,  -411,  -411,
    -411,  -411,  -411,  -411,   260,   264,   161,   287,   243,   294,
     298,   303,   189,  -411,  -411,  -411,  -411,   209,   191,   194,
     219,   222,   224,   227,  -411,   231,   235,   239,   250,   257,
     259,   590,  -411,   590,   590,   376,   -30,  -411,   184,   952,
    -411,   323,   276,   277,   238,   188,  -411,   293,    18,   306,
    -411,  -411,   384,   333,  -411,  -411,   333,   333,  -411,  -411,
     295,  -411,   387,  -411,    -6,  -411,   129,   390,   333,   381,
     307,    70,   321,   -38,  -411,  -411,  -411,   465,   394,   590,
      10,    -1,    10,   278,   590,    10,    10,   590,   590,   590,
     279,   283,   284,   286,  -411,  -411,   722,   305,   -10,     1,
     325,  -411,   410,  -411,   590,   590,   590,   590,   590,   590,
     590,   590,   590,   590,   590,   590,   590,   590,   590,   590,
     590,   412,  -411,  -411,   166,   188,   313,   314,  -411,  -411,
    -411,  -411,  -411,  -411,   308,   367,   340,   342,   333,  -411,
     343,   420,  -411,  -411,  -411,   330,  -411,   428,   429,   403,
     291,  -411,   315,  -411,   408,   360,  -411,  -411,  -411,   170,
      19,  -411,  -411,  -411,   318,  -411,   134,   393,  -411,   550,
    -411,   952,    77,  -411,  -411,   749,   101,   442,   326,   145,
    -411,   776,   152,   156,   614,   635,   803,  -411,   488,   590,
     590,  -411,     2,   449,  -411,  -411,   -17,  -411,  -411,  -411,
    -411,   966,   980,  1046,   508,   368,   368,   212,   212,   212,
     212,   301,   301,  -411,  -411,  -411,   332,  -411,  -411,  -411,
    -411,  -411,   451,  -411,  -411,  -411,   308,   205,   335,   188,
     402,   333,   333,  -411,  -411,  -411,   455,  -411,  -411,   354,
      69,  -411,  -411,   234,   365,   458,   460,  -411,  -411,  -411,
    -411,  -411,   165,   168,    70,   345,  -411,  -411,  -411,   385,
       0,   325,   346,  -411,    10,  -411,  -411,   352,  -411,  -411,
    -411,  -411,  -411,  -411,   590,  -411,   590,  -411,   668,   695,
    -411,   379,   409,     5,   441,  -411,   483,  -411,     2,  -411,
     131,  -411,  -411,   395,   389,  -411,    93,   428,   430,  -411,
    -411,   380,   383,  -411,   386,  -411,   179,   388,   162,  -411,
    -411,     2,  -411,   490,     7,  -411,   382,  -411,  -411,   498,
     392,     2,   397,  -411,  -411,   830,   857,     2,   325,   400,
     404,     5,   406,  -411,  -411,   186,   486,   414,  -411,   183,
      26,   450,  -411,  -411,   520,   455,    25,  -411,  -411,  -411,
     526,  -411,  -411,   417,   411,   413,  -411,   234,    80,   416,
    -411,  -411,  -411,  -411,  -411,  -411,  -411,  -411,    43,    80,
    -411,     2,  -411,    42,   418,  -411,  -411,  -411,   -12,   466,
     531,   -64,     5,    60,    22,    31,    35,    60,    60,    60,
      60,   493,   234,   499,   468,  -411,  -411,  -411,  -411,  -411,
    -411,  -411,  -411,   228,   437,   424,  -411,   426,  -411,  -411,
    -411,    28,   232,  -411,   388,  -411,  -411,  -411,   532,  -411,
     236,  -411,  -411,  -411,   431,  -411,  -411,   243,   527,   494,
     438,  -411,  -411,   459,   462,  -411,     2,  -411,  -411,  -411,
    -411,  -411,  -411,  -411,  -411,  -411,  -411,  -411,  -411,  -411,
    -411,    51,  -411,   436,   496,   537,   323,  -411,  -411,    17,
     526,  -411,  -411,    80,  -411,  -411,    42,   234,   560,  -411,
    -411,    60,    60,   241,  -411,     2,   234,   543,   213,   430,
     448,  -411,  -411,   565,  -411,  -411,  -411,    37,   452,  -411,
     453,  -411,  -411,  -411,  -411,  -411,   245,  -411,   234,   456,
     452,  -411,   561,   471,   247,  -411,  -411,  -411,   234,   566,
    -411,   452,   463,   470,     2,  -411,   565,  -411,  -411,  -411,
    -411,  -411,  -411
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -411,  -411,  -411,  -411,   464,  -411,   359,  -411,  -411,    63,
    -411,  -411,  -411,   269,   204,   475,  -411,  -411,    23,  -411,
     302,  -309,  -411,  -411,  -265,  -121,  -343,  -337,  -411,  -411,
    -411,  -411,  -411,  -411,  -410,    30,    85,  -411,    74,  -411,
    -411,    95,  -411,    40,    -4,  -411,    88,   312,  -411,  -411,
    -101,  -411,   349,   457,  -411,  -411,   353,  -411,  -411,  -411,
    -411,  -411,  -411,  -411,  -411,   167,  -411,  -302,  -411,  -411,
    -411,   329,  -411,  -411,  -411,   215,  -411,  -411,  -411,  -411,
    -411,  -411,  -411,   292,  -411,  -411,  -411,  -411,  -411,  -411,
     246,  -411,  -411,  -411,   622,  -411,  -411,  -411,  -411,  -411,
    -411,  -411,  -411,  -411,  -411
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -251
static const short yytable[] =
{
     211,   389,   364,   398,   113,   193,   194,   206,   349,   350,
    -250,   149,    89,   141,    63,    64,    65,   203,   218,    66,
     530,   186,   531,   417,   206,   532,   187,   299,   355,   477,
     206,   485,    69,   206,   227,   351,   206,   207,   432,   487,
     206,   207,    54,   490,   114,   471,   465,   472,   370,    71,
     352,    56,   353,   273,   556,   447,   481,    74,    75,   453,
     524,   217,    76,   380,  -250,   206,   207,    92,   385,   142,
     557,   143,   431,   197,    93,   206,   207,   144,   208,   145,
     146,   123,   333,   198,   470,   206,   207,   283,   208,   473,
     484,   150,   489,   492,   494,   496,   498,   500,   206,   207,
     466,    55,   188,   354,   198,   371,   463,    94,    80,   150,
      57,   243,   228,   244,   272,   198,   385,   468,   550,   448,
     245,   330,    81,   482,   381,   225,    83,    95,    58,    84,
     231,   272,    85,   234,   235,   236,   533,   272,   561,   300,
     272,   486,   460,   209,    96,   452,    89,   209,   300,   523,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   385,   333,   267,
     525,   206,   209,   306,   268,   297,   298,   502,   542,   544,
     341,   342,   209,   211,   411,   307,    59,   151,   546,   210,
     347,   178,   209,   179,   180,    89,   181,   313,   314,   461,
     152,   412,   390,   391,   269,   209,   199,   433,   267,   392,
     200,   535,   396,   268,   413,   146,   349,   350,   226,   308,
     229,   316,   314,   232,   233,   397,    62,    63,    64,    65,
     270,    60,    66,    67,   434,   328,   329,   349,   350,    61,
     415,   176,    68,   351,   111,    69,   113,   177,   154,   112,
     422,   115,   182,    70,   116,   117,   427,   271,   352,   118,
     353,   547,    71,   119,   351,   319,   314,   120,    72,    73,
      74,    75,   321,   314,   155,    76,   322,   314,   272,   352,
     121,   353,    77,   549,    78,   360,   361,   211,   362,   363,
     122,   435,   436,   437,   438,   439,   440,   124,   211,   406,
     407,   125,   441,   446,   361,    98,   126,    89,   127,    79,
     129,   354,   483,   130,   488,   491,   493,   495,   497,   499,
     375,    80,   376,   166,   167,   168,   169,   170,   141,    63,
      64,    65,   354,   128,    66,    81,    82,   154,   131,    83,
     102,   132,    84,   133,    68,    85,   134,    69,   508,   361,
     135,   103,   512,   513,   136,    70,   515,   361,   137,   104,
     105,   545,   361,   155,    71,   560,   361,   565,   566,   138,
      72,    73,    74,    75,    89,   106,   139,    76,   140,   147,
     171,   174,   175,   185,    77,   189,    78,   190,   108,   191,
     196,   195,   211,   202,   204,   216,   205,   224,   230,   237,
     541,   543,   238,   239,   154,   240,    62,    63,    64,    65,
     242,    79,    66,   248,   168,   169,   170,   266,   277,   278,
     279,   280,    68,    80,   281,    69,   282,   284,   285,   286,
     155,   289,   292,    70,   293,   295,   296,    81,    82,   304,
     310,    83,    71,   571,    84,   317,   318,    85,    72,    73,
      74,    75,   331,   336,   338,    76,   297,   340,   343,   346,
     357,   358,    77,   359,    78,   368,   372,   369,   141,    63,
      64,    65,   374,   218,    66,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   377,   386,   378,    69,   388,    79,
     395,   141,    63,    64,    65,   400,   218,    66,   416,   403,
     394,    80,   404,   419,    71,   405,   420,   408,   432,   442,
      69,   421,    74,    75,   443,    81,    82,    76,   423,    83,
     429,   449,    84,   430,   142,    85,   143,    71,   450,   454,
     457,   458,   478,   459,   505,    74,    75,   464,   476,   480,
      76,   501,   509,   504,   154,   510,   511,   142,   417,   143,
     517,   518,   516,   141,    63,    64,    65,   526,   520,    66,
     528,   521,   527,    80,   522,   540,   548,   552,   553,   563,
     155,   568,    69,   558,   559,   562,   564,    81,   312,   536,
     201,    83,   428,   569,   219,   220,    80,    85,   567,    71,
     570,   529,   348,   141,    63,    64,    65,    74,    75,    66,
      81,   379,    76,   551,    83,   534,   572,    84,   220,   142,
      85,   143,    69,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,    98,   247,   373,   311,   339,    71,
     337,   514,   276,   365,   469,    99,   100,    74,    75,   399,
     110,   451,    76,     0,     0,     0,     0,     0,    80,   142,
     154,   143,     0,   101,     0,     0,     0,     0,     0,   102,
       0,     0,    81,     0,     0,     0,    83,     0,     0,    84,
     103,   154,    85,     0,     0,     0,   155,     0,   104,   105,
       0,     0,     0,     0,     0,     0,     0,     0,    80,     0,
      96,     0,     0,     0,   106,     0,     0,   155,   107,     0,
       0,     0,    81,     0,   154,     0,    83,   108,     0,    84,
       0,     0,    85,     0,     0,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     155,   154,     0,     0,   323,   324,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,     0,     0,     0,     0,   325,   326,   155,   154,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   155,   154,     0,     0,     0,   324,
       0,     0,     0,     0,     0,     0,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   155,   154,     0,     0,     0,   326,     0,     0,     0,
       0,     0,     0,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   155,   154,
       0,     0,   241,     0,     0,     0,     0,     0,     0,     0,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   155,   154,     0,     0,   315,
       0,     0,     0,     0,     0,     0,     0,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   155,   154,     0,     0,   320,     0,     0,     0,
       0,     0,     0,     0,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   155,
       0,     0,     0,   327,     0,     0,     0,     0,     0,     0,
       0,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,     0,     0,     0,     0,
     425,     0,     0,     0,     0,     0,     0,     0,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,     1,     0,     0,     2,   426,     3,     0,
       0,     0,     4,     0,     0,     0,     5,     0,   154,     6,
       7,     8,     9,     0,     0,    10,     0,     0,    11,     0,
       0,     0,   154,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,   155,     0,   154,     0,     0,     0,
       0,     0,     0,     0,     0,    13,     0,     0,   155,     0,
       0,    14,     0,    15,     0,    16,     0,    17,     0,    18,
       0,    19,   155,     0,     0,     0,     0,     0,     0,     0,
      20,     0,    21,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   154,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   155,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170
};

static const short yycheck[] =
{
     121,   338,   304,   346,     3,   106,   107,     5,     3,     4,
       3,    41,    16,     3,     4,     5,     6,   118,     8,     9,
       3,     3,     5,    16,     5,     8,     8,     8,   293,    41,
       5,     9,    22,     5,    35,    30,     5,     6,   102,     8,
       5,     6,    87,     8,    21,     3,     3,     5,    48,    39,
      45,     3,    47,   174,    17,    29,   120,    47,    48,   396,
       9,    99,    52,    58,    57,     5,     6,     3,   333,    59,
      33,    61,   381,    79,    10,     5,     6,    81,     8,    83,
      84,    58,    99,   121,   421,     5,     6,   188,     8,    47,
     433,   121,   435,   436,   437,   438,   439,   440,     5,     6,
      57,    49,    84,    98,   121,   105,   408,    43,    98,   121,
      42,   121,   113,   123,   112,   121,   381,   419,   528,    93,
     119,   242,   112,   432,   119,   129,   116,    63,    41,   119,
     134,   112,   122,   137,   138,   139,   119,   112,   548,   120,
     112,   119,   407,   112,    80,   120,   150,   112,   120,   486,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   432,    99,     3,
     119,     5,   112,    39,     8,     5,     6,   442,   521,   522,
     281,   282,   112,   304,    22,    51,    42,     3,   525,   119,
     121,     3,   112,     5,     6,   199,     8,   120,   121,   119,
      16,    39,    71,    72,    38,   112,    77,    21,     3,    78,
      81,   513,   119,     8,    52,   219,     3,     4,   130,    85,
     132,   120,   121,   135,   136,   346,     3,     4,     5,     6,
      64,    76,     9,    10,    48,   239,   240,     3,     4,    49,
     361,     3,    19,    30,    90,    22,     3,     9,    36,    76,
     371,     0,    64,    30,   118,    53,   377,    91,    45,     3,
      47,   526,    39,     3,    30,   120,   121,     3,    45,    46,
      47,    48,   120,   121,    62,    52,   120,   121,   112,    45,
     119,    47,    59,    70,    61,   120,   121,   408,   120,   121,
       3,   105,   106,   107,   108,   109,   110,     3,   419,   120,
     121,     3,   116,   120,   121,    14,     3,   311,   119,    86,
     119,    98,   433,   119,   435,   436,   437,   438,   439,   440,
     324,    98,   326,   111,   112,   113,   114,   115,     3,     4,
       5,     6,    98,   124,     9,   112,   113,    36,   119,   116,
      49,   119,   119,   119,    19,   122,   119,    22,   120,   121,
     119,    60,   120,   121,   119,    30,   120,   121,   119,    68,
      69,   120,   121,    62,    39,   120,   121,   120,   121,   119,
      45,    46,    47,    48,   378,    84,   119,    52,   119,     3,
      57,   105,   105,    90,    59,    79,    61,     3,    97,    56,
       3,    96,   513,     3,    13,    74,    89,     3,   120,   120,
     521,   522,   119,   119,    36,   119,     3,     4,     5,     6,
     105,    86,     9,     3,   113,   114,   115,     5,   105,   105,
     112,    54,    19,    98,    84,    22,    84,    84,     8,    99,
      62,     3,     3,    30,   119,    27,    76,   112,   113,   121,
      47,   116,    39,   564,   119,     3,   120,   122,    45,    46,
      47,    48,     3,   121,   119,    52,     5,    55,     3,   105,
      95,     3,    59,     3,    61,   120,   120,    82,     3,     4,
       5,     6,   120,     8,     9,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   105,    44,    77,    22,     5,    86,
     101,     3,     4,     5,     6,    65,     8,     9,     8,   119,
     105,    98,   119,   121,    39,   119,     8,   119,   102,    23,
      22,   119,    47,    48,   100,   112,   113,    52,   121,   116,
     120,    71,   119,   119,    59,   122,    61,    39,     8,     3,
     113,   120,    66,   120,    66,    47,    48,   121,   120,     8,
      52,    48,   105,    44,    36,   121,   120,    59,    16,    61,
      23,    57,   121,     3,     4,     5,     6,   121,   120,     9,
      23,   102,    66,    98,   102,     5,    23,   119,     3,     8,
      62,     5,    22,   121,   121,   119,   105,   112,   219,   516,
     116,   116,   378,   120,   119,   120,    98,   122,   558,    39,
     120,   506,   290,     3,     4,     5,     6,    47,    48,     9,
     112,   332,    52,   529,   116,   510,   566,   119,   120,    59,
     122,    61,    22,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    14,   150,   314,    77,   279,    39,
     277,   464,   175,   304,   419,    25,    26,    47,    48,   347,
      18,   395,    52,    -1,    -1,    -1,    -1,    -1,    98,    59,
      36,    61,    -1,    43,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,   112,    -1,    -1,    -1,   116,    -1,    -1,   119,
      60,    36,   122,    -1,    -1,    -1,    62,    -1,    68,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      80,    -1,    -1,    -1,    84,    -1,    -1,    62,    88,    -1,
      -1,    -1,   112,    -1,    36,    -1,   116,    97,    -1,   119,
      -1,    -1,   122,    -1,    -1,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
      62,    36,    -1,    -1,   120,   121,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    -1,    -1,    -1,    -1,   120,   121,    62,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    62,    36,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    62,    36,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    62,    36,
      -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,    62,    36,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    62,    36,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    62,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    15,    -1,    -1,    18,   120,    20,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    28,    -1,    36,    31,
      32,    33,    34,    -1,    -1,    37,    -1,    -1,    40,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    62,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    62,    -1,
      -1,    73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,
      -1,    83,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    -1,    94,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    36,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    24,    28,    31,    32,    33,    34,
      37,    40,    50,    67,    73,    75,    77,    79,    81,    83,
      92,    94,   126,   127,   128,   129,   130,   131,   138,   174,
     179,   180,   183,   184,   185,   186,   193,   194,   203,   204,
     205,   206,   210,   211,   216,   217,   218,   221,   223,   224,
     225,   226,   228,   229,    87,    49,     3,    42,    41,    42,
      76,    49,     3,     4,     5,     6,     9,    10,    19,    22,
      30,    39,    45,    46,    47,    48,    52,    59,    61,    86,
      98,   112,   113,   116,   119,   122,   139,   140,   142,   169,
     170,   227,     3,    10,    43,    63,    80,   219,    14,    25,
      26,    43,    49,    60,    68,    69,    84,    88,    97,   176,
     219,    90,    76,     3,   143,     0,   118,    53,     3,     3,
       3,   119,     3,   143,     3,     3,     3,   119,   124,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,     3,    59,    61,   169,   169,   169,     3,   173,    41,
     121,     3,    16,   141,    36,    62,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,    57,   161,   162,   105,   105,     3,     9,     3,     5,
       6,     8,    64,   177,   178,    90,     3,     8,    84,    79,
       3,    56,   175,   175,   175,    96,     3,    79,   121,    77,
      81,   129,     3,   175,    13,    89,     5,     6,     8,   112,
     119,   150,   151,   192,   195,   196,    74,    99,     8,   119,
     120,   169,   171,   172,     3,   169,   171,    35,   113,   171,
     120,   169,   171,   171,   169,   169,   169,   120,   119,   119,
     119,   120,   105,   121,   123,   119,   143,   140,     3,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,     5,     3,     8,    38,
      64,    91,   112,   150,   181,   182,   178,   105,   105,   112,
      54,    84,    84,   175,    84,     8,    99,   212,   213,     3,
     207,   208,     3,   119,   187,    27,    76,     5,     6,     8,
     120,   150,   152,   197,   121,   198,    39,    51,    85,   222,
      47,    77,   131,   120,   121,   120,   120,     3,   120,   120,
     120,   120,   120,   120,   121,   120,   121,   120,   169,   169,
     150,     3,   135,    99,   144,   145,   121,   181,   119,   177,
      55,   175,   175,     3,   214,   215,   105,   121,   145,     3,
       4,    30,    45,    47,    98,   149,   188,    95,     3,     3,
     120,   121,   120,   121,   192,   196,   199,   200,   120,    82,
      48,   105,   120,   172,   120,   169,   169,   105,    77,   138,
      58,   119,   146,   147,   148,   149,    44,   153,     5,   152,
      71,    72,    78,   220,   105,   101,   119,   150,   151,   208,
      65,   163,   164,   119,   119,   119,   120,   121,   119,   189,
     190,    22,    39,    52,   209,   150,     8,    16,   201,   121,
       8,   119,   150,   121,   132,   120,   120,   150,   139,   120,
     119,   146,   102,    21,    48,   105,   106,   107,   108,   109,
     110,   116,    23,   100,   155,   156,   120,    29,    93,    71,
       8,   215,   120,   152,     3,   165,   166,   113,   120,   120,
     149,   119,   191,   192,   121,     3,    57,   202,   192,   200,
     152,     3,     5,    47,   133,   134,   120,    41,    66,   136,
       8,   120,   146,   150,   151,     9,   119,     8,   150,   151,
       8,   150,   151,   150,   151,   150,   151,   150,   151,   150,
     151,    48,   149,   154,    44,    66,   157,   158,   120,   105,
     121,   120,   120,   121,   190,   120,   121,    23,    57,   137,
     120,   102,   102,   152,     9,   119,   121,    66,    23,   161,
       3,     5,     8,   119,   166,   192,   134,   149,   159,   160,
       5,   150,   151,   150,   151,   120,   152,   149,    23,    70,
     159,   163,   119,     3,   167,   168,    17,    33,   121,   121,
     120,   159,   119,     8,   105,   120,   121,   160,     5,   120,
     120,   150,   168
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

  case 109:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 110:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 113:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 116:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 117:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 119:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 121:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 122:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 125:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 126:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 132:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 133:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 135:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 138:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 139:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 141:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 142:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 146:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 147:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 148:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 149:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 168:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 169:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 174:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 175:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 176:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 181:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 182:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 185:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 186:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 187:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 188:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 190:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 191:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 192:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 193:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 194:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 202:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 203:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 204:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 205:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 206:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 207:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 208:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 211:

    { yyval.m_iValue = 1; ;}
    break;

  case 212:

    { yyval.m_iValue = 0; ;}
    break;

  case 213:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 214:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 215:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 216:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 219:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 220:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 221:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 224:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 225:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 228:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 229:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 230:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 231:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 232:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 233:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 234:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 235:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 236:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 237:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 238:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 239:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 240:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 242:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 243:

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

  case 244:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 247:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 249:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 253:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 254:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 257:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 258:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 261:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 262:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 263:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 264:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 265:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 266:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 267:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 268:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			tStmt.m_sIndex = yyvsp[-4].m_sValue;
			tStmt.m_sAlterAttr = yyvsp[-1].m_sValue;
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 269:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 276:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 277:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 278:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 286:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 287:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 288:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 289:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 290:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 291:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 292:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 293:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 295:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 296:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 297:

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

