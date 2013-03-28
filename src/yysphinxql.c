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
     TOK_BY = 277,
     TOK_CALL = 278,
     TOK_CHARACTER = 279,
     TOK_COLLATION = 280,
     TOK_COLUMN = 281,
     TOK_COMMIT = 282,
     TOK_COMMITTED = 283,
     TOK_COUNT = 284,
     TOK_CREATE = 285,
     TOK_DELETE = 286,
     TOK_DESC = 287,
     TOK_DESCRIBE = 288,
     TOK_DISTINCT = 289,
     TOK_DIV = 290,
     TOK_DROP = 291,
     TOK_FALSE = 292,
     TOK_FLOAT = 293,
     TOK_FLUSH = 294,
     TOK_FROM = 295,
     TOK_FUNCTION = 296,
     TOK_GLOBAL = 297,
     TOK_GROUP = 298,
     TOK_GROUPBY = 299,
     TOK_GROUP_CONCAT = 300,
     TOK_ID = 301,
     TOK_IN = 302,
     TOK_INDEX = 303,
     TOK_INSERT = 304,
     TOK_INT = 305,
     TOK_INTO = 306,
     TOK_ISOLATION = 307,
     TOK_LEVEL = 308,
     TOK_LIKE = 309,
     TOK_LIMIT = 310,
     TOK_MATCH = 311,
     TOK_MAX = 312,
     TOK_META = 313,
     TOK_MIN = 314,
     TOK_MOD = 315,
     TOK_NAMES = 316,
     TOK_NULL = 317,
     TOK_OPTION = 318,
     TOK_ORDER = 319,
     TOK_OPTIMIZE = 320,
     TOK_PROFILE = 321,
     TOK_RAND = 322,
     TOK_READ = 323,
     TOK_REPEATABLE = 324,
     TOK_REPLACE = 325,
     TOK_RETURNS = 326,
     TOK_ROLLBACK = 327,
     TOK_RTINDEX = 328,
     TOK_SELECT = 329,
     TOK_SERIALIZABLE = 330,
     TOK_SET = 331,
     TOK_SESSION = 332,
     TOK_SHOW = 333,
     TOK_SONAME = 334,
     TOK_START = 335,
     TOK_STATUS = 336,
     TOK_STRING = 337,
     TOK_SUM = 338,
     TOK_TABLE = 339,
     TOK_TABLES = 340,
     TOK_TO = 341,
     TOK_TRANSACTION = 342,
     TOK_TRUE = 343,
     TOK_TRUNCATE = 344,
     TOK_UNCOMMITTED = 345,
     TOK_UPDATE = 346,
     TOK_VALUES = 347,
     TOK_VARIABLES = 348,
     TOK_WARNINGS = 349,
     TOK_WEIGHT = 350,
     TOK_WHERE = 351,
     TOK_WITHIN = 352,
     TOK_OR = 353,
     TOK_AND = 354,
     TOK_NE = 355,
     TOK_GTE = 356,
     TOK_LTE = 357,
     TOK_NOT = 358,
     TOK_NEG = 359
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
#define TOK_BY 277
#define TOK_CALL 278
#define TOK_CHARACTER 279
#define TOK_COLLATION 280
#define TOK_COLUMN 281
#define TOK_COMMIT 282
#define TOK_COMMITTED 283
#define TOK_COUNT 284
#define TOK_CREATE 285
#define TOK_DELETE 286
#define TOK_DESC 287
#define TOK_DESCRIBE 288
#define TOK_DISTINCT 289
#define TOK_DIV 290
#define TOK_DROP 291
#define TOK_FALSE 292
#define TOK_FLOAT 293
#define TOK_FLUSH 294
#define TOK_FROM 295
#define TOK_FUNCTION 296
#define TOK_GLOBAL 297
#define TOK_GROUP 298
#define TOK_GROUPBY 299
#define TOK_GROUP_CONCAT 300
#define TOK_ID 301
#define TOK_IN 302
#define TOK_INDEX 303
#define TOK_INSERT 304
#define TOK_INT 305
#define TOK_INTO 306
#define TOK_ISOLATION 307
#define TOK_LEVEL 308
#define TOK_LIKE 309
#define TOK_LIMIT 310
#define TOK_MATCH 311
#define TOK_MAX 312
#define TOK_META 313
#define TOK_MIN 314
#define TOK_MOD 315
#define TOK_NAMES 316
#define TOK_NULL 317
#define TOK_OPTION 318
#define TOK_ORDER 319
#define TOK_OPTIMIZE 320
#define TOK_PROFILE 321
#define TOK_RAND 322
#define TOK_READ 323
#define TOK_REPEATABLE 324
#define TOK_REPLACE 325
#define TOK_RETURNS 326
#define TOK_ROLLBACK 327
#define TOK_RTINDEX 328
#define TOK_SELECT 329
#define TOK_SERIALIZABLE 330
#define TOK_SET 331
#define TOK_SESSION 332
#define TOK_SHOW 333
#define TOK_SONAME 334
#define TOK_START 335
#define TOK_STATUS 336
#define TOK_STRING 337
#define TOK_SUM 338
#define TOK_TABLE 339
#define TOK_TABLES 340
#define TOK_TO 341
#define TOK_TRANSACTION 342
#define TOK_TRUE 343
#define TOK_TRUNCATE 344
#define TOK_UNCOMMITTED 345
#define TOK_UPDATE 346
#define TOK_VALUES 347
#define TOK_VARIABLES 348
#define TOK_WARNINGS 349
#define TOK_WEIGHT 350
#define TOK_WHERE 351
#define TOK_WITHIN 352
#define TOK_OR 353
#define TOK_AND 354
#define TOK_NE 355
#define TOK_GTE 356
#define TOK_LTE 357
#define TOK_NOT 358
#define TOK_NEG 359




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
#define YYFINAL  110
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   931

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  122
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  100
/* YYNRULES -- Number of rules. */
#define YYNRULES  279
/* YYNRULES -- Number of states. */
#define YYNSTATES  539

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   359

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   112,   101,     2,
     116,   117,   110,   108,   118,   109,   121,   111,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   115,
     104,   102,   105,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   119,   100,   120,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   103,   106,   107,   113,   114
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
     527,   534,   541,   545,   547,   551,   553,   555,   559,   565,
     568,   569,   572,   574,   577,   580,   584,   586,   591,   596,
     600,   602,   604,   606,   608,   610,   612,   616,   621,   626,
     631,   635,   640,   648,   654,   656,   658,   660,   662,   664,
     666,   668,   670,   672,   675,   682,   684,   686,   687,   691,
     693,   697,   699,   703,   707,   709,   713,   715,   717,   719,
     723,   726,   734,   744,   751,   753,   757,   759,   763,   765,
     769,   770,   773,   775,   779,   783,   784,   786,   788,   790,
     794,   796,   798,   802,   809,   811,   815,   819,   823,   829,
     834,   842,   847,   848,   850,   853,   855,   859,   863,   866,
     870,   877,   878,   880,   882,   885,   888,   891,   893,   901,
     903,   905,   907,   911,   918,   922,   926,   928,   932,   936
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     123,     0,    -1,   124,    -1,   125,    -1,   125,   115,    -1,
     178,    -1,   186,    -1,   172,    -1,   173,    -1,   176,    -1,
     187,    -1,   196,    -1,   198,    -1,   199,    -1,   203,    -1,
     208,    -1,   209,    -1,   213,    -1,   215,    -1,   216,    -1,
     217,    -1,   210,    -1,   218,    -1,   220,    -1,   221,    -1,
     202,    -1,   126,    -1,   125,   115,   126,    -1,   127,    -1,
     167,    -1,   131,    -1,    74,   132,    40,   116,   128,   131,
     117,   129,   130,    -1,    -1,    64,    22,   152,    -1,    -1,
      55,     5,    -1,    55,     5,   118,     5,    -1,    74,   132,
      40,   136,   137,   146,   148,   150,   154,   156,    -1,   133,
      -1,   132,   118,   133,    -1,   110,    -1,   135,   134,    -1,
      -1,     3,    -1,    16,     3,    -1,   162,    -1,    19,   116,
     162,   117,    -1,    57,   116,   162,   117,    -1,    59,   116,
     162,   117,    -1,    83,   116,   162,   117,    -1,    45,   116,
     162,   117,    -1,    29,   116,   110,   117,    -1,    44,   116,
     117,    -1,    29,   116,    34,     3,   117,    -1,     3,    -1,
     136,   118,     3,    -1,    -1,   138,    -1,    96,   139,    -1,
     140,    -1,   139,    99,   139,    -1,   116,   139,   117,    -1,
      56,   116,     8,   117,    -1,   142,   102,   143,    -1,   142,
     103,   143,    -1,   142,    47,   116,   145,   117,    -1,   142,
     113,    47,   116,   145,   117,    -1,   142,    47,     9,    -1,
     142,   113,    47,     9,    -1,   142,    21,   143,    99,   143,
      -1,   142,   105,   143,    -1,   142,   104,   143,    -1,   142,
     106,   143,    -1,   142,   107,   143,    -1,   142,   102,   144,
      -1,   141,    -1,   142,    21,   144,    99,   144,    -1,   142,
     105,   144,    -1,   142,   104,   144,    -1,   142,   106,   144,
      -1,   142,   107,   144,    -1,   142,   102,     8,    -1,   142,
     103,     8,    -1,   142,   103,   144,    -1,     3,    -1,     4,
      -1,    29,   116,   110,   117,    -1,    44,   116,   117,    -1,
      95,   116,   117,    -1,    46,    -1,     5,    -1,   109,     5,
      -1,     6,    -1,   109,     6,    -1,   143,    -1,   145,   118,
     143,    -1,    -1,    43,    22,   147,    -1,   142,    -1,   147,
     118,   142,    -1,    -1,   149,    -1,    97,    43,    64,    22,
     152,    -1,    -1,   151,    -1,    64,    22,   152,    -1,    64,
      22,    67,   116,   117,    -1,   153,    -1,   152,   118,   153,
      -1,   142,    -1,   142,    17,    -1,   142,    32,    -1,    -1,
     155,    -1,    55,     5,    -1,    55,     5,   118,     5,    -1,
      -1,   157,    -1,    63,   158,    -1,   159,    -1,   158,   118,
     159,    -1,     3,   102,     3,    -1,     3,   102,     5,    -1,
       3,   102,   116,   160,   117,    -1,     3,   102,     3,   116,
       8,   117,    -1,     3,   102,     8,    -1,   161,    -1,   160,
     118,   161,    -1,     3,   102,   143,    -1,     3,    -1,     4,
      -1,    46,    -1,     5,    -1,     6,    -1,     9,    -1,   109,
     162,    -1,   113,   162,    -1,   162,   108,   162,    -1,   162,
     109,   162,    -1,   162,   110,   162,    -1,   162,   111,   162,
      -1,   162,   104,   162,    -1,   162,   105,   162,    -1,   162,
     101,   162,    -1,   162,   100,   162,    -1,   162,   112,   162,
      -1,   162,    35,   162,    -1,   162,    60,   162,    -1,   162,
     107,   162,    -1,   162,   106,   162,    -1,   162,   102,   162,
      -1,   162,   103,   162,    -1,   162,    99,   162,    -1,   162,
      98,   162,    -1,   116,   162,   117,    -1,   119,   166,   120,
      -1,   163,    -1,     3,   116,   164,   117,    -1,    47,   116,
     164,   117,    -1,     3,   116,   117,    -1,    59,   116,   162,
     118,   162,   117,    -1,    57,   116,   162,   118,   162,   117,
      -1,    95,   116,   117,    -1,   165,    -1,   164,   118,   165,
      -1,   162,    -1,     8,    -1,     3,   102,   143,    -1,   166,
     118,     3,   102,   143,    -1,    78,   169,    -1,    -1,    54,
       8,    -1,    94,    -1,    81,   168,    -1,    58,   168,    -1,
      14,    81,   168,    -1,    66,    -1,    14,     8,    81,   168,
      -1,    14,     3,    81,   168,    -1,    48,     3,    81,    -1,
       3,    -1,    62,    -1,     8,    -1,     5,    -1,     6,    -1,
     170,    -1,   171,   109,   170,    -1,    76,     3,   102,   175,
      -1,    76,     3,   102,   174,    -1,    76,     3,   102,    62,
      -1,    76,    61,   171,    -1,    76,    10,   102,   171,    -1,
      76,    42,     9,   102,   116,   145,   117,    -1,    76,    42,
       3,   102,   174,    -1,     3,    -1,     8,    -1,    88,    -1,
      37,    -1,   143,    -1,    27,    -1,    72,    -1,   177,    -1,
      20,    -1,    80,    87,    -1,   179,    51,     3,   180,    92,
     182,    -1,    49,    -1,    70,    -1,    -1,   116,   181,   117,
      -1,   142,    -1,   181,   118,   142,    -1,   183,    -1,   182,
     118,   183,    -1,   116,   184,   117,    -1,   185,    -1,   184,
     118,   185,    -1,   143,    -1,   144,    -1,     8,    -1,   116,
     145,   117,    -1,   116,   117,    -1,    31,    40,   136,    96,
      46,   102,   143,    -1,    31,    40,   136,    96,    46,    47,
     116,   145,   117,    -1,    23,     3,   116,   188,   191,   117,
      -1,   189,    -1,   188,   118,   189,    -1,   185,    -1,   116,
     190,   117,    -1,     8,    -1,   190,   118,     8,    -1,    -1,
     118,   192,    -1,   193,    -1,   192,   118,   193,    -1,   185,
     194,   195,    -1,    -1,    16,    -1,     3,    -1,    55,    -1,
     197,     3,   168,    -1,    33,    -1,    32,    -1,    78,    85,
     168,    -1,    91,   136,    76,   200,   138,   156,    -1,   201,
      -1,   200,   118,   201,    -1,     3,   102,   143,    -1,     3,
     102,   144,    -1,     3,   102,   116,   145,   117,    -1,     3,
     102,   116,   117,    -1,    15,    84,     3,    13,    26,     3,
       3,    -1,    78,   211,    93,   204,    -1,    -1,   205,    -1,
      96,   206,    -1,   207,    -1,   206,    98,   207,    -1,     3,
     102,     8,    -1,    78,    25,    -1,    78,    24,    76,    -1,
      76,   211,    87,    52,    53,   212,    -1,    -1,    42,    -1,
      77,    -1,    68,    90,    -1,    68,    28,    -1,    69,    68,
      -1,    75,    -1,    30,    41,     3,    71,   214,    79,     8,
      -1,    50,    -1,    38,    -1,    82,    -1,    36,    41,     3,
      -1,    18,    48,     3,    86,    73,     3,    -1,    39,    73,
       3,    -1,    74,   219,   154,    -1,    10,    -1,    10,   121,
       3,    -1,    89,    73,     3,    -1,    65,    48,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   133,   133,   134,   135,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   165,   166,   170,   171,
     175,   176,   184,   197,   205,   207,   212,   221,   237,   238,
     242,   243,   246,   248,   249,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   265,   266,   269,   271,   275,   279,
     280,   281,   285,   290,   297,   305,   313,   322,   327,   332,
     337,   342,   347,   352,   357,   362,   367,   372,   377,   382,
     387,   392,   397,   405,   409,   410,   415,   421,   427,   433,
     442,   443,   454,   455,   459,   465,   471,   473,   477,   481,
     487,   489,   493,   504,   506,   510,   514,   521,   522,   526,
     527,   528,   531,   533,   537,   542,   549,   551,   555,   559,
     560,   564,   569,   574,   580,   585,   593,   598,   605,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     626,   627,   628,   629,   630,   631,   632,   633,   634,   635,
     636,   637,   638,   639,   640,   641,   642,   646,   647,   648,
     649,   650,   651,   655,   656,   660,   661,   665,   666,   672,
     675,   677,   681,   682,   683,   684,   685,   686,   691,   696,
     706,   707,   708,   709,   710,   714,   715,   719,   724,   729,
     734,   735,   739,   744,   752,   753,   757,   758,   759,   773,
     774,   775,   779,   780,   786,   794,   795,   798,   800,   804,
     805,   809,   810,   814,   818,   819,   823,   824,   825,   826,
     827,   833,   841,   855,   863,   867,   874,   875,   882,   892,
     898,   900,   904,   909,   913,   920,   922,   926,   927,   933,
     941,   942,   948,   954,   962,   963,   967,   971,   975,   979,
     989,  1002,  1008,  1009,  1013,  1017,  1018,  1022,  1026,  1033,
    1040,  1046,  1047,  1048,  1052,  1053,  1054,  1055,  1061,  1072,
    1073,  1074,  1078,  1089,  1101,  1112,  1120,  1121,  1130,  1141
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
  "TOK_BY", "TOK_CALL", "TOK_CHARACTER", "TOK_COLLATION", "TOK_COLUMN", 
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", 
  "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTO", 
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_PROFILE", "TOK_RAND", 
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
  "statement", "multi_stmt_list", "multi_stmt", "select", 
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
  "update", "update_items_list", "update_item", "alter", "show_variables", 
  "opt_show_variables_where", "show_variables_where", 
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
     124,    38,    61,   355,    60,    62,   356,   357,    43,    45,
      42,    47,    37,   358,   359,    59,    40,    41,    44,   123,
     125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   122,   123,   123,   123,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   125,   125,   126,   126,
     127,   127,   128,   129,   130,   130,   130,   131,   132,   132,
     133,   133,   134,   134,   134,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   136,   136,   137,   137,   138,   139,
     139,   139,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   141,   142,   142,   142,   142,   142,   142,
     143,   143,   144,   144,   145,   145,   146,   146,   147,   147,
     148,   148,   149,   150,   150,   151,   151,   152,   152,   153,
     153,   153,   154,   154,   155,   155,   156,   156,   157,   158,
     158,   159,   159,   159,   159,   159,   160,   160,   161,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   163,   163,   163,
     163,   163,   163,   164,   164,   165,   165,   166,   166,   167,
     168,   168,   169,   169,   169,   169,   169,   169,   169,   169,
     170,   170,   170,   170,   170,   171,   171,   172,   172,   172,
     172,   172,   173,   173,   174,   174,   175,   175,   175,   176,
     176,   176,   177,   177,   178,   179,   179,   180,   180,   181,
     181,   182,   182,   183,   184,   184,   185,   185,   185,   185,
     185,   186,   186,   187,   188,   188,   189,   189,   190,   190,
     191,   191,   192,   192,   193,   194,   194,   195,   195,   196,
     197,   197,   198,   199,   200,   200,   201,   201,   201,   201,
     202,   203,   204,   204,   205,   206,   206,   207,   208,   209,
     210,   211,   211,   211,   212,   212,   212,   212,   213,   214,
     214,   214,   215,   216,   217,   218,   219,   219,   220,   221
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
       3,     3,     3,     3,     3,     3,     1,     4,     4,     3,
       6,     6,     3,     1,     3,     1,     1,     3,     5,     2,
       0,     2,     1,     2,     2,     3,     1,     4,     4,     3,
       1,     1,     1,     1,     1,     1,     3,     4,     4,     4,
       3,     4,     7,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     6,     1,     1,     0,     3,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     1,     3,
       2,     7,     9,     6,     1,     3,     1,     3,     1,     3,
       0,     2,     1,     3,     3,     0,     1,     1,     1,     3,
       1,     1,     3,     6,     1,     3,     3,     3,     5,     4,
       7,     4,     0,     1,     2,     1,     3,     3,     2,     3,
       6,     0,     1,     1,     2,     2,     2,     1,     7,     1,
       1,     1,     3,     6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   202,     0,   199,     0,     0,   241,   240,
       0,     0,   205,     0,   206,   200,     0,   261,   261,     0,
       0,     0,     0,     2,     3,    26,    28,    30,    29,     7,
       8,     9,   201,     5,     0,     6,    10,    11,     0,    12,
      13,    25,    14,    15,    16,    21,    17,    18,    19,    20,
      22,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,   129,   130,   132,   133,   134,   276,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,    38,    42,    45,   156,   112,     0,     0,
     262,     0,   263,     0,     0,     0,   258,   262,     0,   170,
     176,   170,   170,   172,   169,     0,   203,     0,    54,     0,
       1,     4,     0,   170,     0,     0,     0,     0,     0,   272,
     274,   279,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   135,   136,     0,     0,     0,
       0,     0,    43,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   275,   113,     0,     0,     0,     0,   180,
     183,   184,   182,   181,   185,   190,     0,     0,     0,   170,
     259,     0,     0,   174,   173,   242,   252,   278,     0,     0,
       0,     0,    27,   207,   239,     0,     0,    90,    92,   218,
       0,     0,   216,   217,   226,   230,   224,     0,     0,   166,
     159,   165,     0,   163,   277,     0,     0,     0,    52,     0,
       0,     0,     0,     0,   162,     0,     0,   154,     0,     0,
     155,    32,    56,    39,    44,   146,   147,   153,   152,   144,
     143,   150,   151,   141,   142,   149,   148,   137,   138,   139,
     140,   145,   114,   194,   195,   197,   189,   196,     0,   198,
     188,   187,   191,     0,     0,     0,     0,   170,   170,   175,
     179,   171,     0,   251,   253,     0,     0,   244,    55,     0,
       0,     0,     0,    91,    93,   228,   220,    94,     0,     0,
       0,     0,   270,   269,   271,     0,     0,   157,     0,    46,
       0,    51,    50,   158,    47,     0,    48,     0,    49,     0,
       0,   167,     0,     0,     0,    96,    57,     0,   193,     0,
     186,     0,   178,   177,     0,   254,   255,     0,     0,   116,
      84,    85,     0,     0,    89,     0,   209,     0,     0,     0,
     273,   219,     0,   227,     0,   226,   225,   231,   232,   223,
       0,     0,     0,   164,    53,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    75,     0,     0,   100,   115,     0,
       0,     0,   267,   260,     0,     0,     0,   246,   247,   245,
       0,   243,   117,     0,     0,     0,   208,     0,     0,   204,
     211,   250,    95,   229,   236,     0,     0,   268,     0,   221,
     161,   160,   168,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   103,
     101,   192,   265,   264,   266,   257,   256,   249,     0,     0,
     118,   119,     0,    87,    88,   210,     0,     0,   214,     0,
     237,   238,   234,   235,   233,     0,     0,     0,    34,     0,
      61,    60,     0,     0,    67,     0,    81,    63,    74,    82,
      64,    83,    71,    78,    70,    77,    72,    79,    73,    80,
       0,    98,    97,     0,     0,   112,   104,   248,     0,     0,
      86,   213,     0,   212,   222,     0,     0,    31,    62,     0,
       0,     0,    68,     0,     0,     0,     0,   116,   121,   122,
     125,     0,   120,   215,   109,    33,   107,    35,    69,     0,
      76,    65,     0,    99,     0,     0,   105,    37,     0,     0,
       0,   126,   110,   111,     0,     0,    66,   102,     0,     0,
       0,   123,     0,   108,    36,   106,   124,   128,   127
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,   313,   448,   487,    27,
      82,    83,   144,    84,   232,   315,   316,   362,   363,   364,
     504,   287,   203,   288,   367,   472,   419,   420,   475,   476,
     505,   506,   163,   164,   381,   382,   430,   431,   520,   521,
      85,    86,   212,   213,   139,    28,   183,   104,   174,   175,
      29,    30,   260,   261,    31,    32,    33,    34,   280,   337,
     389,   390,   437,   204,    35,    36,   205,   206,   289,   291,
     347,   348,   395,   442,    37,    38,    39,    40,   276,   277,
      41,    42,   273,   274,   325,   326,    43,    44,    45,    93,
     373,    46,   295,    47,    48,    49,    50,    87,    51,    52
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -325
static const short yypact[] =
{
     840,   -55,    19,  -325,    87,  -325,    70,    21,  -325,  -325,
      82,    62,  -325,    96,  -325,  -325,   151,   179,   681,    84,
     112,   176,   192,  -325,    91,  -325,  -325,  -325,  -325,  -325,
    -325,  -325,  -325,  -325,   203,  -325,  -325,  -325,   232,  -325,
    -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,
    -325,  -325,  -325,   259,   265,   100,   266,   176,   268,   275,
     283,   190,  -325,  -325,  -325,  -325,   191,   212,   221,   235,
     238,  -325,   240,   246,   255,   260,   267,   318,  -325,   318,
     318,   355,   -14,  -325,   149,   609,  -325,   327,   282,   284,
     172,   310,  -325,   300,    24,   313,  -325,  -325,   387,   337,
    -325,   337,   337,  -325,  -325,   301,  -325,   390,  -325,    40,
    -325,   131,   393,   337,   384,   314,    25,   331,   -53,  -325,
    -325,  -325,   244,   401,   318,     0,   288,   318,   279,   318,
     318,   318,   290,   292,   293,  -325,  -325,   450,   308,    73,
       3,   198,  -325,   408,  -325,   318,   318,   318,   318,   318,
     318,   318,   318,   318,   318,   318,   318,   318,   318,   318,
     318,   318,   407,  -325,  -325,   297,   310,   315,   316,  -325,
    -325,  -325,  -325,  -325,  -325,   307,   363,   338,   339,   337,
    -325,   340,   414,  -325,  -325,  -325,   329,  -325,   420,   425,
     198,   118,  -325,   317,  -325,   400,   356,  -325,  -325,  -325,
     209,    20,  -325,  -325,  -325,   312,  -325,    71,   389,  -325,
    -325,   609,   101,  -325,  -325,   476,   433,   321,  -325,   503,
     106,   343,   364,   529,  -325,   318,   318,  -325,    13,   436,
    -325,  -325,    44,  -325,  -325,  -325,  -325,   689,   703,   716,
     742,   725,   725,   165,   165,   165,   165,   269,   269,  -325,
    -325,  -325,   322,  -325,  -325,  -325,  -325,  -325,   451,  -325,
    -325,  -325,   307,   156,   361,   310,   406,   337,   337,  -325,
    -325,  -325,   475,  -325,  -325,   377,    51,  -325,  -325,   306,
     388,   481,   483,  -325,  -325,  -325,  -325,  -325,   120,   141,
      25,   370,  -325,  -325,  -325,   409,   -10,  -325,   279,  -325,
     372,  -325,  -325,  -325,  -325,   318,  -325,   318,  -325,   397,
     423,  -325,   391,   416,    12,   448,  -325,   487,  -325,    13,
    -325,   164,  -325,  -325,   392,   415,  -325,    15,   420,   449,
    -325,  -325,   398,   402,  -325,   403,  -325,   148,   404,   513,
    -325,  -325,    13,  -325,   509,    83,  -325,   419,  -325,  -325,
     531,   424,    13,  -325,  -325,   556,   582,    13,   198,   426,
     428,    12,   443,  -325,  -325,   242,   523,   468,  -325,   202,
      11,   478,  -325,  -325,   539,   475,     8,  -325,  -325,  -325,
     563,  -325,  -325,   458,   452,   453,  -325,   306,    30,   454,
    -325,  -325,  -325,  -325,  -325,    47,    30,  -325,    13,  -325,
    -325,  -325,  -325,     2,   507,   565,    69,    12,    58,    -2,
      39,    54,    58,    58,    58,    58,   543,   306,   549,   530,
    -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,   213,   493,
     479,  -325,   482,  -325,  -325,  -325,     9,   215,  -325,   404,
    -325,  -325,  -325,   580,  -325,   223,   176,   576,   545,   501,
    -325,  -325,   520,   522,  -325,    13,  -325,  -325,  -325,  -325,
    -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,
      -1,  -325,   504,   559,   602,   327,  -325,  -325,    14,   563,
    -325,  -325,    30,  -325,  -325,   306,   620,  -325,  -325,    13,
      18,   225,  -325,    13,   306,   604,   184,   449,   527,  -325,
    -325,   642,  -325,  -325,   119,   532,  -325,   533,  -325,   641,
    -325,  -325,   250,  -325,   306,   536,   532,  -325,   640,   547,
     252,  -325,  -325,  -325,   306,   648,  -325,   532,   553,   554,
      13,  -325,   642,  -325,  -325,  -325,  -325,  -325,  -325
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -325,  -325,  -325,  -325,   561,  -325,  -325,  -325,  -325,   362,
     319,   535,  -325,  -325,    85,  -325,   421,  -304,  -325,  -325,
    -274,  -116,  -317,  -310,  -325,  -325,  -325,  -325,  -325,  -325,
    -324,   150,   204,  -325,   181,  -325,  -325,   217,  -325,   166,
     -76,  -325,   572,   405,  -325,  -325,   -90,  -325,   437,   538,
    -325,  -325,   438,  -325,  -325,  -325,  -325,  -325,  -325,  -325,
    -325,   286,  -325,  -288,  -325,  -325,  -325,   432,  -325,  -325,
    -325,   330,  -325,  -325,  -325,  -325,  -325,  -325,  -325,   399,
    -325,  -325,  -325,  -325,  -325,   353,  -325,  -325,  -325,   712,
    -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325,  -325
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -236
static const short yytable[] =
{
     202,   135,   345,   136,   137,   336,   108,   454,   492,   369,
     378,   184,   185,   197,   197,   330,   331,   498,   197,   499,
     197,   198,   500,   194,   198,   197,   140,   177,   285,    53,
     197,   198,   178,   199,   216,   197,   198,   351,   199,   422,
     365,   332,   446,   208,   197,   198,   211,   456,   215,   259,
     440,   219,   211,   221,   222,   223,   333,   406,   334,   197,
     198,    57,   459,   197,   198,   189,   428,    54,   360,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,  -235,   365,   445,   269,
      55,   453,   352,   458,   461,   463,   465,   467,   469,   394,
     438,   423,   441,   451,   141,   179,   109,   335,   443,   292,
     217,    56,   311,   435,   455,   493,   188,   258,   258,   231,
     141,   293,   258,    58,   200,   427,   286,   509,   361,   258,
     501,   376,    94,   365,   200,    59,   522,   286,  -235,   200,
     314,   201,   118,   471,    60,   491,   436,   314,   200,   309,
     310,   523,   142,   294,    61,    62,    63,    64,   189,   253,
      65,    66,   189,   200,   254,   143,    98,   200,   407,   328,
      67,   106,   516,   510,   202,   167,    99,   322,   323,   108,
      68,   168,    88,   512,   100,   107,   450,   330,   331,    89,
     527,   229,   110,   230,   503,    69,    70,    71,    72,   101,
     145,    61,    62,    63,    64,   190,   111,    65,    73,   191,
      74,   377,   103,   332,   283,   284,   116,    67,   297,   298,
     513,    90,   211,   303,   298,   146,   392,    68,   333,   355,
     334,   356,   370,   371,    75,   113,   399,   341,   342,   372,
      91,   402,    69,    70,    71,    72,    76,    61,    62,    63,
      64,   515,   209,    65,   112,    73,    92,    74,   343,   344,
      77,    78,   114,   408,    79,   386,   387,    80,   115,   117,
      81,   119,   202,   157,   158,   159,   160,   161,   120,   335,
     202,    75,    61,    62,    63,    64,   121,   209,    65,   409,
      71,    72,   452,    76,   457,   460,   462,   464,   466,   468,
     253,   133,   197,   134,   145,   254,   122,    77,    78,   330,
     331,    79,   123,   169,    80,   170,   171,    81,   172,   421,
     342,    61,    62,    63,    64,    71,    72,    65,   124,   146,
     477,   342,   481,   482,   255,   332,   133,   125,   134,    76,
     484,   342,   511,   342,   410,   411,   412,   413,   414,   415,
     333,   126,   334,    77,   127,   416,   128,    79,   138,   256,
      80,   210,   129,    81,    71,    72,   202,   526,   342,   531,
     532,   130,   173,   508,    76,   133,   131,   134,   145,   159,
     160,   161,   162,   132,   165,   257,   166,   176,    77,   180,
     181,   182,    79,   187,   186,    80,   193,   195,    81,   145,
     196,   335,   207,   146,   214,   218,   258,   224,   225,   226,
     228,   234,   252,    76,   537,   266,   265,   263,   264,   267,
     268,   270,   271,   275,   146,   272,   281,    77,   278,   282,
     290,    79,   145,   279,    80,   296,   300,    81,   301,   312,
     317,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   283,   146,   145,   321,
     304,   305,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   319,   324,   327,
     338,   306,   307,   146,   339,   145,   340,   349,   350,   354,
     358,   366,   368,   357,   374,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     146,   145,   380,   375,   383,   305,   391,   393,   384,   385,
     388,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   146,   396,   145,   397,
     398,   307,   407,   404,   405,   417,   424,   425,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   146,   145,   418,   429,   227,   432,   433,
     434,   447,   439,   449,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   146,
     470,   145,   473,   299,   474,   478,   394,   479,   485,   480,
     486,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   146,   145,   488,   489,
     302,   490,   494,   495,   496,   507,   514,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   146,   518,   145,   519,   308,   284,   529,   530,
     524,   525,   528,   534,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   146,
     535,   536,   192,   400,   533,   359,   233,   403,   517,   497,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,    94,   502,   329,   538,   401,
     220,   318,   320,   353,   262,    95,    96,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   346,    97,   145,   483,   444,   379,   426,    98,
     105,     0,     0,     0,     0,     0,     0,     0,   145,    99,
       0,     0,     0,     0,     0,     0,     0,   100,     0,   146,
       0,   145,     0,     0,     0,     0,     0,     0,    92,     0,
     145,     0,   101,   146,     0,     0,   102,     0,     0,     0,
       0,     0,     0,     0,     0,   103,   146,   145,     0,     0,
       0,     0,     0,     0,     0,   146,     0,     0,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   146,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,     0,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   153,
     154,   155,   156,   157,   158,   159,   160,   161,     0,     0,
       0,     0,     0,     0,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,     1,     0,     0,     2,     0,
       3,     0,     0,     4,     0,     0,     0,     5,     0,     0,
       6,     7,     8,     9,     0,     0,    10,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,     0,     0,     0,     0,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
      19,     0,     0,     0,     0,     0,     0,     0,     0,    20,
       0,    21
};

static const short yycheck[] =
{
     116,    77,   290,    79,    80,   279,     3,     9,     9,   319,
     327,   101,   102,     5,     5,     3,     4,     3,     5,     5,
       5,     6,     8,   113,     6,     5,    40,     3,     8,    84,
       5,     6,     8,     8,    34,     5,     6,    47,     8,    28,
     314,    29,    40,    96,     5,     6,   122,     8,   124,   165,
       3,   127,   128,   129,   130,   131,    44,   361,    46,     5,
       6,    40,     8,     5,     6,   118,   376,    48,    56,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,     3,   361,   398,   179,
       3,   408,   102,   410,   411,   412,   413,   414,   415,    16,
     388,    90,    55,   407,   118,    81,    21,    95,   396,    38,
     110,    41,   228,   387,   116,   116,    76,   109,   109,   116,
     118,    50,   109,    41,   109,   117,   117,   109,   116,   109,
     116,   116,    14,   407,   109,    73,    17,   117,    55,   109,
      96,   116,    57,   417,    48,   455,   116,    96,   109,   225,
     226,    32,     3,    82,     3,     4,     5,     6,   118,     3,
       9,    10,   118,   109,     8,    16,    48,   109,    99,   118,
      19,    87,   496,   490,   290,     3,    58,   267,   268,     3,
      29,     9,     3,   493,    66,    73,   117,     3,     4,    10,
     514,   118,     0,   120,   482,    44,    45,    46,    47,    81,
      35,     3,     4,     5,     6,    74,   115,     9,    57,    78,
      59,   327,    94,    29,     5,     6,   116,    19,   117,   118,
     494,    42,   298,   117,   118,    60,   342,    29,    44,   305,
      46,   307,    68,    69,    83,     3,   352,   117,   118,    75,
      61,   357,    44,    45,    46,    47,    95,     3,     4,     5,
       6,    67,     8,     9,    51,    57,    77,    59,   117,   118,
     109,   110,     3,    21,   113,   117,   118,   116,     3,     3,
     119,     3,   388,   108,   109,   110,   111,   112,     3,    95,
     396,    83,     3,     4,     5,     6,     3,     8,     9,    47,
      46,    47,   408,    95,   410,   411,   412,   413,   414,   415,
       3,    57,     5,    59,    35,     8,   116,   109,   110,     3,
       4,   113,   121,     3,   116,     5,     6,   119,     8,   117,
     118,     3,     4,     5,     6,    46,    47,     9,   116,    60,
     117,   118,   117,   118,    37,    29,    57,   116,    59,    95,
     117,   118,   117,   118,   102,   103,   104,   105,   106,   107,
      44,   116,    46,   109,   116,   113,   116,   113,     3,    62,
     116,   117,   116,   119,    46,    47,   482,   117,   118,   117,
     118,   116,    62,   489,    95,    57,   116,    59,    35,   110,
     111,   112,    55,   116,   102,    88,   102,    87,   109,    76,
       3,    54,   113,     3,    93,   116,     3,    13,   119,    35,
      86,    95,    71,    60,     3,   117,   109,   117,   116,   116,
     102,     3,     5,    95,   530,    52,   109,   102,   102,    81,
      81,    81,     8,     3,    60,    96,    26,   109,     3,    73,
     118,   113,    35,   116,   116,    46,     3,   119,   117,     3,
     118,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     5,    60,    35,    53,
     117,   118,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   116,     3,   102,
      92,   117,   118,    60,     3,    35,     3,   117,    79,   117,
      74,    43,     5,   102,   102,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      60,    35,    63,    98,   116,   118,     3,     8,   116,   116,
     116,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    60,   118,    35,     8,
     116,   118,    99,   117,   116,    22,    68,     8,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    60,    35,    97,     3,   117,   110,   117,
     117,    64,   118,     8,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    60,
      47,    35,    43,   117,    64,   102,    16,   118,    22,   117,
      55,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    60,    35,   117,    99,
     117,    99,   118,    64,    22,     5,    22,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    60,   116,    35,     3,   117,     6,     8,   102,
     118,   118,   116,     5,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    60,
     117,   117,   111,   117,   524,   313,   141,   358,   497,   475,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    14,   479,   276,   532,   117,
     128,   263,   265,   298,   166,    24,    25,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   290,    42,    35,   439,   396,   328,   375,    48,
      18,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    60,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      35,    -1,    81,    60,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    60,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    60,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    -1,    -1,
      -1,    -1,    -1,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    15,    -1,    -1,    18,    -1,
      20,    -1,    -1,    23,    -1,    -1,    -1,    27,    -1,    -1,
      30,    31,    32,    33,    -1,    -1,    36,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      70,    -1,    72,    -1,    74,    -1,    76,    -1,    78,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      -1,    91
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    23,    27,    30,    31,    32,    33,
      36,    39,    49,    65,    70,    72,    74,    76,    78,    80,
      89,    91,   123,   124,   125,   126,   127,   131,   167,   172,
     173,   176,   177,   178,   179,   186,   187,   196,   197,   198,
     199,   202,   203,   208,   209,   210,   213,   215,   216,   217,
     218,   220,   221,    84,    48,     3,    41,    40,    41,    73,
      48,     3,     4,     5,     6,     9,    10,    19,    29,    44,
      45,    46,    47,    57,    59,    83,    95,   109,   110,   113,
     116,   119,   132,   133,   135,   162,   163,   219,     3,    10,
      42,    61,    77,   211,    14,    24,    25,    42,    48,    58,
      66,    81,    85,    94,   169,   211,    87,    73,     3,   136,
       0,   115,    51,     3,     3,     3,   116,     3,   136,     3,
       3,     3,   116,   121,   116,   116,   116,   116,   116,   116,
     116,   116,   116,    57,    59,   162,   162,   162,     3,   166,
      40,   118,     3,    16,   134,    35,    60,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    55,   154,   155,   102,   102,     3,     9,     3,
       5,     6,     8,    62,   170,   171,    87,     3,     8,    81,
      76,     3,    54,   168,   168,   168,    93,     3,    76,   118,
      74,    78,   126,     3,   168,    13,    86,     5,     6,     8,
     109,   116,   143,   144,   185,   188,   189,    71,    96,     8,
     117,   162,   164,   165,     3,   162,    34,   110,   117,   162,
     164,   162,   162,   162,   117,   116,   116,   117,   102,   118,
     120,   116,   136,   133,     3,   162,   162,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     162,   162,     5,     3,     8,    37,    62,    88,   109,   143,
     174,   175,   171,   102,   102,   109,    52,    81,    81,   168,
      81,     8,    96,   204,   205,     3,   200,   201,     3,   116,
     180,    26,    73,     5,     6,     8,   117,   143,   145,   190,
     118,   191,    38,    50,    82,   214,    46,   117,   118,   117,
       3,   117,   117,   117,   117,   118,   117,   118,   117,   162,
     162,   143,     3,   128,    96,   137,   138,   118,   174,   116,
     170,    53,   168,   168,     3,   206,   207,   102,   118,   138,
       3,     4,    29,    44,    46,    95,   142,   181,    92,     3,
       3,   117,   118,   117,   118,   185,   189,   192,   193,   117,
      79,    47,   102,   165,   117,   162,   162,   102,    74,   131,
      56,   116,   139,   140,   141,   142,    43,   146,     5,   145,
      68,    69,    75,   212,   102,    98,   116,   143,   144,   201,
      63,   156,   157,   116,   116,   116,   117,   118,   116,   182,
     183,     3,   143,     8,    16,   194,   118,     8,   116,   143,
     117,   117,   143,   132,   117,   116,   139,    99,    21,    47,
     102,   103,   104,   105,   106,   107,   113,    22,    97,   148,
     149,   117,    28,    90,    68,     8,   207,   117,   145,     3,
     158,   159,   110,   117,   117,   142,   116,   184,   185,   118,
       3,    55,   195,   185,   193,   145,    40,    64,   129,     8,
     117,   139,   143,   144,     9,   116,     8,   143,   144,     8,
     143,   144,   143,   144,   143,   144,   143,   144,   143,   144,
      47,   142,   147,    43,    64,   150,   151,   117,   102,   118,
     117,   117,   118,   183,   117,    22,    55,   130,   117,    99,
      99,   145,     9,   116,   118,    64,    22,   154,     3,     5,
       8,   116,   159,   185,   142,   152,   153,     5,   143,   109,
     144,   117,   145,   142,    22,    67,   152,   156,   116,     3,
     160,   161,    17,    32,   118,   118,   117,   152,   116,     8,
     102,   117,   118,   153,     5,   117,   117,   143,   161
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

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 160:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 161:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 162:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 167:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 168:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 171:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 175:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 176:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 177:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 178:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 179:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 187:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 188:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 189:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 190:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 191:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 192:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 193:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 196:

    { yyval.m_iValue = 1; ;}
    break;

  case 197:

    { yyval.m_iValue = 0; ;}
    break;

  case 198:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 199:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 200:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 204:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 205:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 206:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 209:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 210:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 213:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 214:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 215:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 216:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 217:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 218:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 219:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 220:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 221:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 222:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 223:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 224:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 225:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 227:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 228:

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

  case 229:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 232:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 234:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 238:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 239:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 242:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 243:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 246:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 247:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 248:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 249:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 250:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			tStmt.m_sIndex = yyvsp[-4].m_sValue;
			tStmt.m_sAlterAttr = yyvsp[-1].m_sValue;
			tStmt.m_sAlterColType = yyvsp[0].m_sValue;
		;}
    break;

  case 251:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 258:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 259:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 260:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 268:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 269:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 270:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 271:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 272:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 273:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 274:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 275:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 277:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 278:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 279:

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

