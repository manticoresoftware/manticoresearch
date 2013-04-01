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
     TOK_PROFILE = 323,
     TOK_RAND = 324,
     TOK_READ = 325,
     TOK_REPEATABLE = 326,
     TOK_REPLACE = 327,
     TOK_RETURNS = 328,
     TOK_ROLLBACK = 329,
     TOK_RTINDEX = 330,
     TOK_SELECT = 331,
     TOK_SERIALIZABLE = 332,
     TOK_SET = 333,
     TOK_SESSION = 334,
     TOK_SHOW = 335,
     TOK_SONAME = 336,
     TOK_START = 337,
     TOK_STATUS = 338,
     TOK_STRING = 339,
     TOK_SUM = 340,
     TOK_TABLE = 341,
     TOK_TABLES = 342,
     TOK_TO = 343,
     TOK_TRANSACTION = 344,
     TOK_TRUE = 345,
     TOK_TRUNCATE = 346,
     TOK_UNCOMMITTED = 347,
     TOK_UPDATE = 348,
     TOK_VALUES = 349,
     TOK_VARIABLES = 350,
     TOK_WARNINGS = 351,
     TOK_WEIGHT = 352,
     TOK_WHERE = 353,
     TOK_WITHIN = 354,
     TOK_OR = 355,
     TOK_AND = 356,
     TOK_NE = 357,
     TOK_GTE = 358,
     TOK_LTE = 359,
     TOK_NOT = 360,
     TOK_NEG = 361
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
#define TOK_PROFILE 323
#define TOK_RAND 324
#define TOK_READ 325
#define TOK_REPEATABLE 326
#define TOK_REPLACE 327
#define TOK_RETURNS 328
#define TOK_ROLLBACK 329
#define TOK_RTINDEX 330
#define TOK_SELECT 331
#define TOK_SERIALIZABLE 332
#define TOK_SET 333
#define TOK_SESSION 334
#define TOK_SHOW 335
#define TOK_SONAME 336
#define TOK_START 337
#define TOK_STATUS 338
#define TOK_STRING 339
#define TOK_SUM 340
#define TOK_TABLE 341
#define TOK_TABLES 342
#define TOK_TO 343
#define TOK_TRANSACTION 344
#define TOK_TRUE 345
#define TOK_TRUNCATE 346
#define TOK_UNCOMMITTED 347
#define TOK_UPDATE 348
#define TOK_VALUES 349
#define TOK_VARIABLES 350
#define TOK_WARNINGS 351
#define TOK_WEIGHT 352
#define TOK_WHERE 353
#define TOK_WITHIN 354
#define TOK_OR 355
#define TOK_AND 356
#define TOK_NE 357
#define TOK_GTE 358
#define TOK_LTE 359
#define TOK_NOT 360
#define TOK_NEG 361




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
#define YYFINAL  113
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   980

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  124
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  101
/* YYNRULES -- Number of rules. */
#define YYNRULES  285
/* YYNRULES -- Number of states. */
#define YYNSTATES  554

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   361

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   114,   103,     2,
     118,   119,   112,   110,   120,   111,   123,   113,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   117,
     106,   104,   107,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   121,   102,   122,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   101,   105,   108,   109,
     115,   116
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    58,    60,
      62,    64,    74,    75,    79,    80,    83,    88,    99,   101,
     105,   107,   110,   111,   113,   116,   118,   123,   128,   133,
     138,   143,   148,   152,   158,   160,   164,   165,   167,   170,
     172,   176,   180,   185,   189,   193,   199,   206,   210,   215,
     221,   225,   229,   233,   237,   241,   243,   249,   253,   257,
     261,   265,   269,   273,   277,   279,   281,   286,   290,   294,
     296,   298,   301,   303,   306,   308,   312,   313,   317,   319,
     323,   324,   326,   332,   333,   335,   339,   345,   347,   351,
     353,   356,   359,   360,   362,   365,   370,   371,   373,   376,
     378,   382,   386,   390,   396,   403,   407,   409,   413,   417,
     419,   421,   423,   425,   427,   429,   432,   435,   439,   443,
     447,   451,   455,   459,   463,   467,   471,   475,   479,   483,
     487,   491,   495,   499,   503,   507,   511,   513,   518,   523,
     528,   533,   538,   542,   549,   556,   560,   562,   566,   568,
     570,   574,   580,   583,   584,   587,   589,   592,   595,   599,
     601,   606,   611,   615,   617,   619,   621,   623,   625,   627,
     631,   636,   641,   646,   650,   655,   663,   669,   671,   673,
     675,   677,   679,   681,   683,   685,   687,   690,   697,   699,
     701,   702,   706,   708,   712,   714,   718,   722,   724,   728,
     730,   732,   734,   738,   741,   749,   759,   766,   768,   772,
     774,   778,   780,   784,   785,   788,   790,   794,   798,   799,
     801,   803,   805,   809,   811,   813,   817,   824,   826,   830,
     834,   838,   844,   849,   851,   853,   855,   863,   868,   869,
     871,   874,   876,   880,   884,   887,   891,   898,   899,   901,
     903,   906,   909,   912,   914,   922,   924,   926,   928,   932,
     939,   943,   947,   949,   953,   957
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     125,     0,    -1,   126,    -1,   127,    -1,   127,   117,    -1,
     180,    -1,   188,    -1,   174,    -1,   175,    -1,   178,    -1,
     189,    -1,   198,    -1,   200,    -1,   201,    -1,   206,    -1,
     211,    -1,   212,    -1,   216,    -1,   218,    -1,   219,    -1,
     220,    -1,   213,    -1,   221,    -1,   223,    -1,   224,    -1,
     205,    -1,   128,    -1,   127,   117,   128,    -1,   129,    -1,
     169,    -1,   133,    -1,    76,   134,    41,   118,   130,   133,
     119,   131,   132,    -1,    -1,    66,    23,   154,    -1,    -1,
      57,     5,    -1,    57,     5,   120,     5,    -1,    76,   134,
      41,   138,   139,   148,   150,   152,   156,   158,    -1,   135,
      -1,   134,   120,   135,    -1,   112,    -1,   137,   136,    -1,
      -1,     3,    -1,    16,     3,    -1,   164,    -1,    19,   118,
     164,   119,    -1,    59,   118,   164,   119,    -1,    61,   118,
     164,   119,    -1,    85,   118,   164,   119,    -1,    46,   118,
     164,   119,    -1,    30,   118,   112,   119,    -1,    45,   118,
     119,    -1,    30,   118,    35,     3,   119,    -1,     3,    -1,
     138,   120,     3,    -1,    -1,   140,    -1,    98,   141,    -1,
     142,    -1,   141,   101,   141,    -1,   118,   141,   119,    -1,
      58,   118,     8,   119,    -1,   144,   104,   145,    -1,   144,
     105,   145,    -1,   144,    48,   118,   147,   119,    -1,   144,
     115,    48,   118,   147,   119,    -1,   144,    48,     9,    -1,
     144,   115,    48,     9,    -1,   144,    21,   145,   101,   145,
      -1,   144,   107,   145,    -1,   144,   106,   145,    -1,   144,
     108,   145,    -1,   144,   109,   145,    -1,   144,   104,   146,
      -1,   143,    -1,   144,    21,   146,   101,   146,    -1,   144,
     107,   146,    -1,   144,   106,   146,    -1,   144,   108,   146,
      -1,   144,   109,   146,    -1,   144,   104,     8,    -1,   144,
     105,     8,    -1,   144,   105,   146,    -1,     3,    -1,     4,
      -1,    30,   118,   112,   119,    -1,    45,   118,   119,    -1,
      97,   118,   119,    -1,    47,    -1,     5,    -1,   111,     5,
      -1,     6,    -1,   111,     6,    -1,   145,    -1,   147,   120,
     145,    -1,    -1,    44,    23,   149,    -1,   144,    -1,   149,
     120,   144,    -1,    -1,   151,    -1,    99,    44,    66,    23,
     154,    -1,    -1,   153,    -1,    66,    23,   154,    -1,    66,
      23,    69,   118,   119,    -1,   155,    -1,   154,   120,   155,
      -1,   144,    -1,   144,    17,    -1,   144,    33,    -1,    -1,
     157,    -1,    57,     5,    -1,    57,     5,   120,     5,    -1,
      -1,   159,    -1,    65,   160,    -1,   161,    -1,   160,   120,
     161,    -1,     3,   104,     3,    -1,     3,   104,     5,    -1,
       3,   104,   118,   162,   119,    -1,     3,   104,     3,   118,
       8,   119,    -1,     3,   104,     8,    -1,   163,    -1,   162,
     120,   163,    -1,     3,   104,   145,    -1,     3,    -1,     4,
      -1,    47,    -1,     5,    -1,     6,    -1,     9,    -1,   111,
     164,    -1,   115,   164,    -1,   164,   110,   164,    -1,   164,
     111,   164,    -1,   164,   112,   164,    -1,   164,   113,   164,
      -1,   164,   106,   164,    -1,   164,   107,   164,    -1,   164,
     103,   164,    -1,   164,   102,   164,    -1,   164,   114,   164,
      -1,   164,    36,   164,    -1,   164,    62,   164,    -1,   164,
     109,   164,    -1,   164,   108,   164,    -1,   164,   104,   164,
      -1,   164,   105,   164,    -1,   164,   101,   164,    -1,   164,
     100,   164,    -1,   118,   164,   119,    -1,   121,   168,   122,
      -1,   165,    -1,     3,   118,   166,   119,    -1,    48,   118,
     166,   119,    -1,    52,   118,   166,   119,    -1,    22,   118,
     166,   119,    -1,    39,   118,   166,   119,    -1,     3,   118,
     119,    -1,    61,   118,   164,   120,   164,   119,    -1,    59,
     118,   164,   120,   164,   119,    -1,    97,   118,   119,    -1,
     167,    -1,   166,   120,   167,    -1,   164,    -1,     8,    -1,
       3,   104,   145,    -1,   168,   120,     3,   104,   145,    -1,
      80,   171,    -1,    -1,    56,     8,    -1,    96,    -1,    83,
     170,    -1,    60,   170,    -1,    14,    83,   170,    -1,    68,
      -1,    14,     8,    83,   170,    -1,    14,     3,    83,   170,
      -1,    49,     3,    83,    -1,     3,    -1,    64,    -1,     8,
      -1,     5,    -1,     6,    -1,   172,    -1,   173,   111,   172,
      -1,    78,     3,   104,   177,    -1,    78,     3,   104,   176,
      -1,    78,     3,   104,    64,    -1,    78,    63,   173,    -1,
      78,    10,   104,   173,    -1,    78,    43,     9,   104,   118,
     147,   119,    -1,    78,    43,     3,   104,   176,    -1,     3,
      -1,     8,    -1,    90,    -1,    38,    -1,   145,    -1,    28,
      -1,    74,    -1,   179,    -1,    20,    -1,    82,    89,    -1,
     181,    53,     3,   182,    94,   184,    -1,    50,    -1,    72,
      -1,    -1,   118,   183,   119,    -1,   144,    -1,   183,   120,
     144,    -1,   185,    -1,   184,   120,   185,    -1,   118,   186,
     119,    -1,   187,    -1,   186,   120,   187,    -1,   145,    -1,
     146,    -1,     8,    -1,   118,   147,   119,    -1,   118,   119,
      -1,    32,    41,   138,    98,    47,   104,   145,    -1,    32,
      41,   138,    98,    47,    48,   118,   147,   119,    -1,    24,
       3,   118,   190,   193,   119,    -1,   191,    -1,   190,   120,
     191,    -1,   187,    -1,   118,   192,   119,    -1,     8,    -1,
     192,   120,     8,    -1,    -1,   120,   194,    -1,   195,    -1,
     194,   120,   195,    -1,   187,   196,   197,    -1,    -1,    16,
      -1,     3,    -1,    57,    -1,   199,     3,   170,    -1,    34,
      -1,    33,    -1,    80,    87,   170,    -1,    93,   138,    78,
     202,   140,   158,    -1,   203,    -1,   202,   120,   203,    -1,
       3,   104,   145,    -1,     3,   104,   146,    -1,     3,   104,
     118,   147,   119,    -1,     3,   104,   118,   119,    -1,    52,
      -1,    22,    -1,    39,    -1,    15,    86,     3,    13,    27,
       3,   204,    -1,    80,   214,    95,   207,    -1,    -1,   208,
      -1,    98,   209,    -1,   210,    -1,   209,   100,   210,    -1,
       3,   104,     8,    -1,    80,    26,    -1,    80,    25,    78,
      -1,    78,   214,    89,    54,    55,   215,    -1,    -1,    43,
      -1,    79,    -1,    70,    92,    -1,    70,    29,    -1,    71,
      70,    -1,    77,    -1,    31,    42,     3,    73,   217,    81,
       8,    -1,    51,    -1,    39,    -1,    84,    -1,    37,    42,
       3,    -1,    18,    49,     3,    88,    75,     3,    -1,    40,
      75,     3,    -1,    76,   222,   156,    -1,    10,    -1,    10,
     123,     3,    -1,    91,    75,     3,    -1,    67,    49,     3,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   135,   135,   136,   137,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   167,   168,   172,   173,
     177,   178,   186,   199,   207,   209,   214,   223,   239,   240,
     244,   245,   248,   250,   251,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   267,   268,   271,   273,   277,   281,
     282,   283,   287,   292,   299,   307,   315,   324,   329,   334,
     339,   344,   349,   354,   359,   364,   369,   374,   379,   384,
     389,   394,   399,   407,   411,   412,   417,   423,   429,   435,
     444,   445,   456,   457,   461,   467,   473,   475,   479,   483,
     489,   491,   495,   506,   508,   512,   516,   523,   524,   528,
     529,   530,   533,   535,   539,   544,   551,   553,   557,   561,
     562,   566,   571,   576,   582,   587,   595,   600,   607,   617,
     618,   619,   620,   621,   622,   623,   624,   625,   626,   627,
     628,   629,   630,   631,   632,   633,   634,   635,   636,   637,
     638,   639,   640,   641,   642,   643,   644,   648,   649,   650,
     651,   652,   653,   654,   655,   656,   660,   661,   665,   666,
     670,   671,   677,   680,   682,   686,   687,   688,   689,   690,
     691,   696,   701,   711,   712,   713,   714,   715,   719,   720,
     724,   729,   734,   739,   740,   744,   749,   757,   758,   762,
     763,   764,   778,   779,   780,   784,   785,   791,   799,   800,
     803,   805,   809,   810,   814,   815,   819,   823,   824,   828,
     829,   830,   831,   832,   838,   846,   860,   868,   872,   879,
     880,   887,   897,   903,   905,   909,   914,   918,   925,   927,
     931,   932,   938,   946,   947,   953,   959,   967,   968,   972,
     976,   980,   984,   994,   995,   996,  1000,  1013,  1019,  1020,
    1024,  1028,  1029,  1033,  1037,  1044,  1051,  1057,  1058,  1059,
    1063,  1064,  1065,  1066,  1072,  1083,  1084,  1085,  1089,  1100,
    1112,  1123,  1131,  1132,  1141,  1152
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
  "TOK_OPTIMIZE", "TOK_PROFILE", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", 
  "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", 
  "TOK_TABLE", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", 
  "TOK_TRUNCATE", "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", 
  "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", 
  "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", 
  "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", 
  "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", "'}'", "'.'", 
  "$accept", "request", "statement", "multi_stmt_list", "multi_stmt", 
  "select", "subselect_start", "opt_outer_order", "opt_outer_limit", 
  "select_from", "select_items_list", "select_item", "opt_alias", 
  "select_expr", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "where_item", "expr_float_unhandled", "expr_ident", 
  "const_int", "const_float", "const_list", "opt_group_clause", 
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
     355,   356,   124,    38,    61,   357,    60,    62,   358,   359,
      43,    45,    42,    47,    37,   360,   361,    59,    40,    41,
      44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   124,   125,   125,   125,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   127,   127,   128,   128,
     129,   129,   130,   131,   132,   132,   132,   133,   134,   134,
     135,   135,   136,   136,   136,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   138,   138,   139,   139,   140,   141,
     141,   141,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   143,   144,   144,   144,   144,   144,   144,
     145,   145,   146,   146,   147,   147,   148,   148,   149,   149,
     150,   150,   151,   152,   152,   153,   153,   154,   154,   155,
     155,   155,   156,   156,   157,   157,   158,   158,   159,   160,
     160,   161,   161,   161,   161,   161,   162,   162,   163,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   166,   166,   167,   167,
     168,   168,   169,   170,   170,   171,   171,   171,   171,   171,
     171,   171,   171,   172,   172,   172,   172,   172,   173,   173,
     174,   174,   174,   174,   174,   175,   175,   176,   176,   177,
     177,   177,   178,   178,   178,   179,   179,   180,   181,   181,
     182,   182,   183,   183,   184,   184,   185,   186,   186,   187,
     187,   187,   187,   187,   188,   188,   189,   190,   190,   191,
     191,   192,   192,   193,   193,   194,   194,   195,   196,   196,
     197,   197,   198,   199,   199,   200,   201,   202,   202,   203,
     203,   203,   203,   204,   204,   204,   205,   206,   207,   207,
     208,   209,   209,   210,   211,   212,   213,   214,   214,   214,
     215,   215,   215,   215,   216,   217,   217,   217,   218,   219,
     220,   221,   222,   222,   223,   224
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     9,     0,     3,     0,     2,     4,    10,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     4,     3,     5,     1,     3,     0,     1,     2,     1,
       3,     3,     4,     3,     3,     5,     6,     3,     4,     5,
       3,     3,     3,     3,     3,     1,     5,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     4,     3,     3,     1,
       1,     2,     1,     2,     1,     3,     0,     3,     1,     3,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     4,     4,     4,
       4,     4,     3,     6,     6,     3,     1,     3,     1,     1,
       3,     5,     2,     0,     2,     1,     2,     2,     3,     1,
       4,     4,     3,     1,     1,     1,     1,     1,     1,     3,
       4,     4,     4,     3,     4,     7,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     6,     1,     1,
       0,     3,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     3,     2,     7,     9,     6,     1,     3,     1,
       3,     1,     3,     0,     2,     1,     3,     3,     0,     1,
       1,     1,     3,     1,     1,     3,     6,     1,     3,     3,
       3,     5,     4,     1,     1,     1,     7,     4,     0,     1,
       2,     1,     3,     3,     2,     3,     6,     0,     1,     1,
       2,     2,     2,     1,     7,     1,     1,     1,     3,     6,
       3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   205,     0,   202,     0,     0,   244,   243,
       0,     0,   208,     0,   209,   203,     0,   267,   267,     0,
       0,     0,     0,     2,     3,    26,    28,    30,    29,     7,
       8,     9,   204,     5,     0,     6,    10,    11,     0,    12,
      13,    25,    14,    15,    16,    21,    17,    18,    19,    20,
      22,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,   129,   130,   132,   133,   134,   282,     0,     0,     0,
       0,     0,     0,   131,     0,     0,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,    38,    42,    45,   156,
     112,     0,     0,   268,     0,   269,     0,     0,     0,   264,
     268,     0,   173,   179,   173,   173,   175,   172,     0,   206,
       0,    54,     0,     1,     4,     0,   173,     0,     0,     0,
       0,     0,   278,   280,   285,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   135,   136,     0,     0,     0,     0,     0,    43,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   281,
     113,     0,     0,     0,     0,   183,   186,   187,   185,   184,
     188,   193,     0,     0,     0,   173,   265,     0,     0,   177,
     176,   245,   258,   284,     0,     0,     0,     0,    27,   210,
     242,     0,     0,    90,    92,   221,     0,     0,   219,   220,
     229,   233,   227,     0,     0,   169,   162,   168,     0,   166,
     283,     0,     0,     0,     0,     0,    52,     0,     0,     0,
       0,     0,     0,   165,     0,     0,   154,     0,     0,   155,
      32,    56,    39,    44,   146,   147,   153,   152,   144,   143,
     150,   151,   141,   142,   149,   148,   137,   138,   139,   140,
     145,   114,   197,   198,   200,   192,   199,     0,   201,   191,
     190,   194,     0,     0,     0,     0,   173,   173,   178,   182,
     174,     0,   257,   259,     0,     0,   247,    55,     0,     0,
       0,     0,    91,    93,   231,   223,    94,     0,     0,     0,
       0,   276,   275,   277,     0,     0,   157,     0,    46,   160,
       0,    51,   161,    50,   158,   159,    47,     0,    48,     0,
      49,     0,     0,   170,     0,     0,     0,    96,    57,     0,
     196,     0,   189,     0,   181,   180,     0,   260,   261,     0,
       0,   116,    84,    85,     0,     0,    89,     0,   212,     0,
       0,     0,   279,   222,     0,   230,     0,   229,   228,   234,
     235,   226,     0,     0,     0,   167,    53,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    75,     0,     0,   100,
     115,     0,     0,     0,   273,   266,     0,     0,     0,   249,
     250,   248,     0,   246,   117,     0,     0,     0,   211,     0,
       0,   207,   214,   254,   255,   253,   256,    95,   232,   239,
       0,     0,   274,     0,   224,   164,   163,   171,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   103,   101,   195,   271,   270,   272,
     263,   262,   252,     0,     0,   118,   119,     0,    87,    88,
     213,     0,     0,   217,     0,   240,   241,   237,   238,   236,
       0,     0,     0,    34,     0,    61,    60,     0,     0,    67,
       0,    81,    63,    74,    82,    64,    83,    71,    78,    70,
      77,    72,    79,    73,    80,     0,    98,    97,     0,     0,
     112,   104,   251,     0,     0,    86,   216,     0,   215,   225,
       0,     0,    31,    62,     0,     0,     0,    68,     0,     0,
       0,     0,   116,   121,   122,   125,     0,   120,   218,   109,
      33,   107,    35,    69,     0,    76,    65,     0,    99,     0,
       0,   105,    37,     0,     0,     0,   126,   110,   111,     0,
       0,    66,   102,     0,     0,     0,   123,     0,   108,    36,
     106,   124,   128,   127
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,   325,   463,   502,    27,
      85,    86,   150,    87,   241,   327,   328,   374,   375,   376,
     519,   296,   209,   297,   379,   487,   434,   435,   490,   491,
     520,   521,   169,   170,   393,   394,   445,   446,   535,   536,
     217,    89,   218,   219,   145,    28,   189,   107,   180,   181,
      29,    30,   269,   270,    31,    32,    33,    34,   289,   349,
     401,   402,   452,   210,    35,    36,   211,   212,   298,   300,
     359,   360,   410,   457,    37,    38,    39,    40,   285,   286,
     406,    41,    42,   282,   283,   337,   338,    43,    44,    45,
      96,   385,    46,   304,    47,    48,    49,    50,    90,    51,
      52
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -334
static const short yypact[] =
{
     757,   -13,    30,  -334,   117,  -334,   182,    67,  -334,  -334,
     185,    64,  -334,   139,  -334,  -334,   164,   319,   378,   150,
     156,   241,   246,  -334,   131,  -334,  -334,  -334,  -334,  -334,
    -334,  -334,  -334,  -334,   215,  -334,  -334,  -334,   274,  -334,
    -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,
    -334,  -334,  -334,   275,   277,   172,   293,   241,   298,   312,
     318,   214,  -334,  -334,  -334,  -334,   200,   216,   218,   220,
     225,   227,   236,  -334,   242,   250,   257,   258,   259,   262,
     419,  -334,   419,   419,   381,   -32,  -334,    38,   751,  -334,
     329,   284,   285,    40,    42,  -334,   301,    26,   316,  -334,
    -334,   392,   340,  -334,   340,   340,  -334,  -334,   302,  -334,
     397,  -334,    58,  -334,   128,   398,   340,   389,   323,    96,
     334,   -31,  -334,  -334,  -334,    22,   410,   419,   322,   -12,
     322,   295,   419,   322,   322,   419,   419,   419,   297,   299,
     311,  -334,  -334,   522,   326,    76,     5,   294,  -334,   417,
    -334,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   426,  -334,
    -334,   194,    42,   328,   330,  -334,  -334,  -334,  -334,  -334,
    -334,   324,   385,   359,   362,   340,  -334,   369,   445,  -334,
    -334,  -334,   356,  -334,   452,   453,   294,   187,  -334,   341,
    -334,   436,   393,  -334,  -334,  -334,   105,     2,  -334,  -334,
    -334,   344,  -334,   142,   422,  -334,  -334,   751,   118,  -334,
    -334,   549,   123,   467,   353,   132,  -334,   576,   134,   137,
     382,   441,   603,  -334,   419,   419,  -334,    13,   470,  -334,
    -334,    86,  -334,  -334,  -334,  -334,   765,   778,   857,   400,
     866,   866,   337,   337,   337,   337,   205,   205,  -334,  -334,
    -334,   355,  -334,  -334,  -334,  -334,  -334,   471,  -334,  -334,
    -334,   324,    50,   361,    42,   442,   340,   340,  -334,  -334,
    -334,   478,  -334,  -334,   395,    97,  -334,  -334,   363,   406,
     512,   514,  -334,  -334,  -334,  -334,  -334,   140,   143,    96,
     399,  -334,  -334,  -334,   438,    -6,  -334,   322,  -334,  -334,
     401,  -334,  -334,  -334,  -334,  -334,  -334,   419,  -334,   419,
    -334,   462,   495,  -334,   418,   447,    10,   477,  -334,   520,
    -334,    13,  -334,   163,  -334,  -334,   423,   428,  -334,   110,
     452,   461,  -334,  -334,   411,   414,  -334,   415,  -334,   146,
     420,    24,  -334,  -334,    13,  -334,   527,    48,  -334,   416,
    -334,  -334,   531,   459,    13,  -334,  -334,   630,   657,    13,
     294,   437,   460,    10,   458,  -334,  -334,   243,   556,   481,
    -334,   152,    -7,   511,  -334,  -334,   575,   478,    60,  -334,
    -334,  -334,   583,  -334,  -334,   475,   469,   472,  -334,   363,
     184,   473,  -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,
      34,   184,  -334,    13,  -334,  -334,  -334,  -334,   -21,   523,
     582,   -63,    10,    54,     7,    27,   158,    54,    54,    54,
      54,   544,   363,   550,   547,  -334,  -334,  -334,  -334,  -334,
    -334,  -334,  -334,   154,   506,   494,  -334,   497,  -334,  -334,
    -334,    66,   167,  -334,   420,  -334,  -334,  -334,   601,  -334,
     169,   241,   595,   562,   501,  -334,  -334,   536,   539,  -334,
      13,  -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,
    -334,  -334,  -334,  -334,  -334,     8,  -334,   524,   555,   619,
     329,  -334,  -334,    16,   583,  -334,  -334,   184,  -334,  -334,
     363,   638,  -334,  -334,    13,     6,   237,  -334,    13,   363,
     622,   290,   461,   528,  -334,  -334,   644,  -334,  -334,    49,
     551,  -334,   552,  -334,   642,  -334,  -334,   244,  -334,   363,
     546,   551,  -334,   659,   565,   252,  -334,  -334,  -334,   363,
     665,  -334,   551,   554,   572,    13,  -334,   644,  -334,  -334,
    -334,  -334,  -334,  -334
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -334,  -334,  -334,  -334,   560,  -334,  -334,  -334,  -334,   350,
     327,   553,  -334,  -334,    15,  -334,   409,  -293,  -334,  -334,
    -287,  -119,  -333,  -326,  -334,  -334,  -334,  -334,  -334,  -334,
    -324,   157,   208,  -334,   189,  -334,  -334,   224,  -334,   155,
      -5,  -334,    85,   413,  -334,  -334,  -101,  -334,   425,   557,
    -334,  -334,   449,  -334,  -334,  -334,  -334,  -334,  -334,  -334,
    -334,   269,  -334,  -297,  -334,  -334,  -334,   427,  -334,  -334,
    -334,   313,  -334,  -334,  -334,  -334,  -334,  -334,  -334,   387,
    -334,  -334,  -334,  -334,  -334,  -334,   338,  -334,  -334,  -334,
     710,  -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,  -334,
    -334
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -239
static const short yytable[] =
{
     208,   348,   357,   190,   191,   381,   390,   203,   111,   146,
     294,    88,   204,   342,   343,   200,   469,   507,   203,   513,
     461,   514,   437,   223,   515,    61,    62,    63,    64,   183,
     215,    65,   203,   204,   184,   471,   112,   455,   422,   377,
     344,   148,   363,   173,    68,   175,   403,   176,   177,   174,
     178,  -238,   268,   262,   149,   345,   465,   346,   263,   203,
     204,    70,   443,   404,   409,   203,   537,   214,   372,    73,
      74,   203,   121,    53,    75,   141,   405,   142,   143,    54,
     421,   139,   538,   140,   278,   438,   377,   460,   147,   195,
     468,   456,   473,   476,   478,   480,   482,   484,   364,   147,
     224,   203,   204,   453,   205,  -238,   179,   347,    57,   185,
     292,   293,   450,   267,   458,   203,   204,   524,   323,    79,
      55,   295,   221,   240,   267,   470,   508,   227,   373,   466,
     230,   231,   232,    80,   516,   377,   194,    82,   206,    59,
      83,   216,    88,    84,   506,   486,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   203,   204,   206,   474,    61,    62,    63,
      64,   267,   525,    65,    66,   334,   335,   267,   195,   442,
     208,   301,   527,    67,   326,   295,    68,   531,    60,   203,
     204,    88,   205,   302,    69,   326,   238,   262,   239,   203,
     518,    97,   263,    70,   196,   542,   195,   206,   197,    71,
      72,    73,    74,   222,   207,   225,    75,   340,   228,   229,
     389,   206,   528,    76,    56,    77,   303,    58,   388,   321,
     322,   110,   264,   382,   383,   407,   101,   306,   307,   109,
     384,   151,   309,   307,   111,   414,   113,   102,   114,    78,
     417,   312,   307,   314,   307,   103,   315,   307,   265,   353,
     354,    79,   355,   356,   423,   398,   399,   152,   115,   206,
     104,   436,   354,   492,   354,    80,    81,   116,   117,    82,
     118,   208,    83,   106,   266,    84,   496,   497,   499,   354,
     119,   424,   208,   342,   343,   206,   120,    61,    62,    63,
      64,   122,   451,    65,   467,   267,   472,   475,   477,   479,
     481,   483,   367,    67,   368,   123,    68,   165,   166,   167,
     344,   124,    91,   126,    69,    61,    62,    63,    64,    92,
     215,    65,   125,    70,   127,   345,   128,   346,   129,    71,
      72,    73,    74,   130,    68,   131,    75,   425,   426,   427,
     428,   429,   430,    76,   132,    77,   526,   354,   431,   530,
     133,    70,    93,   541,   354,    88,   342,   343,   134,    73,
      74,   546,   547,   151,    75,   135,   136,   137,   208,    78,
     138,   139,    94,   140,   144,   523,   168,   347,   171,   172,
     182,    79,    97,   344,   186,   187,   188,   192,    95,   152,
     193,   199,   201,    98,    99,    80,    81,   213,   345,    82,
     346,   202,    83,   220,   226,    84,   233,   234,   151,    79,
     243,   100,    61,    62,    63,    64,   552,   101,    65,   235,
     237,   261,   272,    80,   273,   274,   151,    82,   102,   275,
      83,    68,   276,    84,   152,   277,   103,   163,   164,   165,
     166,   167,   279,   280,   281,   284,   287,    95,    70,   288,
     347,   104,   152,   290,   299,   105,    73,    74,   291,   305,
     310,    75,   311,   324,   106,   329,   292,   151,   139,   331,
     140,   336,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   333,   151,   339,
     350,   316,   317,   152,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   351,    79,   352,   361,   362,
     366,   378,   369,   370,   152,   380,   392,   386,   387,   395,
      80,   151,   396,   397,    82,   408,   411,    83,   400,   412,
      84,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   419,   152,   151,   422,
     318,   319,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   413,   420,   432,
     433,   439,   317,   440,   152,   151,   444,   447,   448,   462,
     464,   449,   485,   454,   488,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     493,   152,   151,   489,   494,   319,   495,   409,   500,   501,
     503,   510,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   504,   152,   151,
     505,   236,   511,   522,   509,   529,   533,   534,   293,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   543,   152,   151,   544,   308,   545,
     549,   539,   540,   550,   198,   371,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   551,   152,   151,   341,   313,   548,   418,   512,   332,
     242,   532,   553,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   517,   152,
     365,   330,   320,   498,   459,   441,   358,   391,   108,   271,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,     0,     0,     0,     0,   415,
       0,     0,     0,     0,     0,     0,     0,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,     1,     0,     0,     2,   416,     3,     0,     0,
       0,     4,     0,     0,     0,     5,     0,   151,     6,     7,
       8,     9,     0,     0,    10,     0,     0,    11,     0,     0,
       0,   151,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,   152,   151,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,     0,     0,   152,     0,    14,
       0,    15,     0,    16,     0,    17,     0,    18,     0,    19,
     152,     0,     0,     0,     0,     0,     0,     0,    20,     0,
      21,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   151,     0,     0,     0,     0,     0,     0,
       0,     0,   151,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   152,
       0,     0,     0,     0,     0,     0,     0,     0,   152,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   159,   160,   161,   162,   163,   164,   165,   166,
     167
};

static const short yycheck[] =
{
     119,   288,   299,   104,   105,   331,   339,     5,     3,    41,
       8,    16,     6,     3,     4,   116,     9,     9,     5,     3,
      41,     5,    29,    35,     8,     3,     4,     5,     6,     3,
       8,     9,     5,     6,     8,     8,    21,     3,   101,   326,
      30,     3,    48,     3,    22,     3,    22,     5,     6,     9,
       8,     3,   171,     3,    16,    45,   119,    47,     8,     5,
       6,    39,   388,    39,    16,     5,    17,    98,    58,    47,
      48,     5,    57,    86,    52,    80,    52,    82,    83,    49,
     373,    59,    33,    61,   185,    92,   373,   413,   120,   120,
     423,    57,   425,   426,   427,   428,   429,   430,   104,   120,
     112,     5,     6,   400,     8,    57,    64,    97,    41,    83,
       5,     6,   399,   111,   411,     5,     6,   111,   237,    97,
       3,   119,   127,   118,   111,   118,   118,   132,   118,   422,
     135,   136,   137,   111,   118,   422,    78,   115,   111,    75,
     118,   119,   147,   121,   470,   432,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,     5,     6,   111,     8,     3,     4,     5,
       6,   111,   505,     9,    10,   276,   277,   111,   120,   119,
     299,    39,   508,    19,    98,   119,    22,   511,    49,     5,
       6,   196,     8,    51,    30,    98,   120,     3,   122,     5,
     497,    14,     8,    39,    76,   529,   120,   111,    80,    45,
      46,    47,    48,   128,   118,   130,    52,   120,   133,   134,
     339,   111,   509,    59,    42,    61,    84,    42,   118,   234,
     235,    75,    38,    70,    71,   354,    49,   119,   120,    89,
      77,    36,   119,   120,     3,   364,     0,    60,   117,    85,
     369,   119,   120,   119,   120,    68,   119,   120,    64,   119,
     120,    97,   119,   120,    21,   119,   120,    62,    53,   111,
      83,   119,   120,   119,   120,   111,   112,     3,     3,   115,
       3,   400,   118,    96,    90,   121,   119,   120,   119,   120,
     118,    48,   411,     3,     4,   111,     3,     3,     4,     5,
       6,     3,   118,     9,   423,   111,   425,   426,   427,   428,
     429,   430,   317,    19,   319,     3,    22,   112,   113,   114,
      30,     3,     3,   123,    30,     3,     4,     5,     6,    10,
       8,     9,   118,    39,   118,    45,   118,    47,   118,    45,
      46,    47,    48,   118,    22,   118,    52,   104,   105,   106,
     107,   108,   109,    59,   118,    61,   119,   120,   115,    69,
     118,    39,    43,   119,   120,   370,     3,     4,   118,    47,
      48,   119,   120,    36,    52,   118,   118,   118,   497,    85,
     118,    59,    63,    61,     3,   504,    57,    97,   104,   104,
      89,    97,    14,    30,    78,     3,    56,    95,    79,    62,
       3,     3,    13,    25,    26,   111,   112,    73,    45,   115,
      47,    88,   118,     3,   119,   121,   119,   118,    36,    97,
       3,    43,     3,     4,     5,     6,   545,    49,     9,   118,
     104,     5,   104,   111,   104,   111,    36,   115,    60,    54,
     118,    22,    83,   121,    62,    83,    68,   110,   111,   112,
     113,   114,    83,     8,    98,     3,     3,    79,    39,   118,
      97,    83,    62,    27,   120,    87,    47,    48,    75,    47,
       3,    52,   119,     3,    96,   120,     5,    36,    59,   118,
      61,     3,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    55,    36,   104,
      94,   119,   120,    62,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     3,    97,     3,   119,    81,
     119,    44,   104,    76,    62,     5,    65,   104,   100,   118,
     111,    36,   118,   118,   115,     8,   120,   118,   118,     8,
     121,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   119,    62,    36,   101,
     119,   120,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   118,   118,    23,
      99,    70,   120,     8,    62,    36,     3,   112,   119,    66,
       8,   119,    48,   120,    44,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     104,    62,    36,    66,   120,   120,   119,    16,    23,    57,
     119,    66,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   101,    62,    36,
     101,   119,    23,     5,   120,    23,   118,     3,     6,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   118,    62,    36,     8,   119,   104,
       5,   120,   120,   119,   114,   325,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   119,    62,    36,   285,   119,   539,   370,   490,   274,
     147,   512,   547,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   494,    62,
     307,   272,   119,   454,   411,   387,   299,   340,    18,   172,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    15,    -1,    -1,    18,   119,    20,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    28,    -1,    36,    31,    32,
      33,    34,    -1,    -1,    37,    -1,    -1,    40,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    62,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    62,    -1,    72,
      -1,    74,    -1,    76,    -1,    78,    -1,    80,    -1,    82,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   106,   107,   108,   109,   110,   111,   112,   113,
     114
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    24,    28,    31,    32,    33,    34,
      37,    40,    50,    67,    72,    74,    76,    78,    80,    82,
      91,    93,   125,   126,   127,   128,   129,   133,   169,   174,
     175,   178,   179,   180,   181,   188,   189,   198,   199,   200,
     201,   205,   206,   211,   212,   213,   216,   218,   219,   220,
     221,   223,   224,    86,    49,     3,    42,    41,    42,    75,
      49,     3,     4,     5,     6,     9,    10,    19,    22,    30,
      39,    45,    46,    47,    48,    52,    59,    61,    85,    97,
     111,   112,   115,   118,   121,   134,   135,   137,   164,   165,
     222,     3,    10,    43,    63,    79,   214,    14,    25,    26,
      43,    49,    60,    68,    83,    87,    96,   171,   214,    89,
      75,     3,   138,     0,   117,    53,     3,     3,     3,   118,
       3,   138,     3,     3,     3,   118,   123,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,    59,
      61,   164,   164,   164,     3,   168,    41,   120,     3,    16,
     136,    36,    62,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    57,   156,
     157,   104,   104,     3,     9,     3,     5,     6,     8,    64,
     172,   173,    89,     3,     8,    83,    78,     3,    56,   170,
     170,   170,    95,     3,    78,   120,    76,    80,   128,     3,
     170,    13,    88,     5,     6,     8,   111,   118,   145,   146,
     187,   190,   191,    73,    98,     8,   119,   164,   166,   167,
       3,   164,   166,    35,   112,   166,   119,   164,   166,   166,
     164,   164,   164,   119,   118,   118,   119,   104,   120,   122,
     118,   138,   135,     3,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,     5,     3,     8,    38,    64,    90,   111,   145,   176,
     177,   173,   104,   104,   111,    54,    83,    83,   170,    83,
       8,    98,   207,   208,     3,   202,   203,     3,   118,   182,
      27,    75,     5,     6,     8,   119,   145,   147,   192,   120,
     193,    39,    51,    84,   217,    47,   119,   120,   119,   119,
       3,   119,   119,   119,   119,   119,   119,   120,   119,   120,
     119,   164,   164,   145,     3,   130,    98,   139,   140,   120,
     176,   118,   172,    55,   170,   170,     3,   209,   210,   104,
     120,   140,     3,     4,    30,    45,    47,    97,   144,   183,
      94,     3,     3,   119,   120,   119,   120,   187,   191,   194,
     195,   119,    81,    48,   104,   167,   119,   164,   164,   104,
      76,   133,    58,   118,   141,   142,   143,   144,    44,   148,
       5,   147,    70,    71,    77,   215,   104,   100,   118,   145,
     146,   203,    65,   158,   159,   118,   118,   118,   119,   120,
     118,   184,   185,    22,    39,    52,   204,   145,     8,    16,
     196,   120,     8,   118,   145,   119,   119,   145,   134,   119,
     118,   141,   101,    21,    48,   104,   105,   106,   107,   108,
     109,   115,    23,    99,   150,   151,   119,    29,    92,    70,
       8,   210,   119,   147,     3,   160,   161,   112,   119,   119,
     144,   118,   186,   187,   120,     3,    57,   197,   187,   195,
     147,    41,    66,   131,     8,   119,   141,   145,   146,     9,
     118,     8,   145,   146,     8,   145,   146,   145,   146,   145,
     146,   145,   146,   145,   146,    48,   144,   149,    44,    66,
     152,   153,   119,   104,   120,   119,   119,   120,   185,   119,
      23,    57,   132,   119,   101,   101,   147,     9,   118,   120,
      66,    23,   156,     3,     5,     8,   118,   161,   187,   144,
     154,   155,     5,   145,   111,   146,   119,   147,   144,    23,
      69,   154,   158,   118,     3,   162,   163,    17,    33,   120,
     120,   119,   154,   118,     8,   104,   119,   120,   155,     5,
     119,   119,   145,   163
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
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 32:

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

  case 33:

    {
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 35:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 36:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 37:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 40:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 43:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 44:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 45:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 46:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 47:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 48:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 49:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 50:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 51:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 52:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 53:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 55:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 62:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 63:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 64:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 65:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 66:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 67:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 73:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 76:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 86:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 87:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 88:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 89:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 90:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 91:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 92:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 93:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 94:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 95:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 98:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 99:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 102:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 105:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 106:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 108:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 110:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 111:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 114:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 115:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 121:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 122:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 123:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 124:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 125:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 126:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 127:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 128:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 130:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 131:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 135:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 137:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 138:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 139:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 140:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 141:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 142:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 143:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 144:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 145:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 146:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 147:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 157:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 158:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 159:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 160:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 161:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 162:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 163:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 164:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 165:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 170:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 171:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 175:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 176:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 177:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 178:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 179:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 180:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 181:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 182:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 190:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 191:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 192:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 193:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 194:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 195:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 196:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 199:

    { yyval.m_iValue = 1; ;}
    break;

  case 200:

    { yyval.m_iValue = 0; ;}
    break;

  case 201:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 207:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 208:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 209:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 212:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 213:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 216:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 217:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 218:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 219:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 220:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 221:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 222:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 223:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 224:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 225:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 226:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 227:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 228:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 230:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 231:

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

  case 232:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 235:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 237:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 241:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 242:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 245:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 246:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 249:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 250:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 251:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 252:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 253:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 254:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 255:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 256:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			tStmt.m_sIndex = yyvsp[-4].m_sValue;
			tStmt.m_sAlterAttr = yyvsp[-1].m_sValue;
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 264:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 265:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 266:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 274:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 275:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 276:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 277:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 278:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 279:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 280:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 281:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 283:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 284:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 285:

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

