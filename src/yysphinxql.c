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
     TOK_AGENT = 267,
     TOK_AS = 268,
     TOK_ASC = 269,
     TOK_ATTACH = 270,
     TOK_AVG = 271,
     TOK_BEGIN = 272,
     TOK_BETWEEN = 273,
     TOK_BY = 274,
     TOK_CALL = 275,
     TOK_CHARACTER = 276,
     TOK_COLLATION = 277,
     TOK_COMMIT = 278,
     TOK_COMMITTED = 279,
     TOK_COUNT = 280,
     TOK_CREATE = 281,
     TOK_DELETE = 282,
     TOK_DESC = 283,
     TOK_DESCRIBE = 284,
     TOK_DISTINCT = 285,
     TOK_DIV = 286,
     TOK_DROP = 287,
     TOK_FALSE = 288,
     TOK_FLOAT = 289,
     TOK_FLUSH = 290,
     TOK_FROM = 291,
     TOK_FUNCTION = 292,
     TOK_GLOBAL = 293,
     TOK_GROUP = 294,
     TOK_GROUPBY = 295,
     TOK_GROUP_CONCAT = 296,
     TOK_ID = 297,
     TOK_IN = 298,
     TOK_INDEX = 299,
     TOK_INSERT = 300,
     TOK_INT = 301,
     TOK_INTO = 302,
     TOK_ISOLATION = 303,
     TOK_LEVEL = 304,
     TOK_LIKE = 305,
     TOK_LIMIT = 306,
     TOK_MATCH = 307,
     TOK_MAX = 308,
     TOK_META = 309,
     TOK_MIN = 310,
     TOK_MOD = 311,
     TOK_NAMES = 312,
     TOK_NULL = 313,
     TOK_OPTION = 314,
     TOK_ORDER = 315,
     TOK_OPTIMIZE = 316,
     TOK_RAND = 317,
     TOK_READ = 318,
     TOK_REPEATABLE = 319,
     TOK_REPLACE = 320,
     TOK_RETURNS = 321,
     TOK_ROLLBACK = 322,
     TOK_RTINDEX = 323,
     TOK_SELECT = 324,
     TOK_SERIALIZABLE = 325,
     TOK_SET = 326,
     TOK_SESSION = 327,
     TOK_SHOW = 328,
     TOK_SONAME = 329,
     TOK_START = 330,
     TOK_STATUS = 331,
     TOK_STRING = 332,
     TOK_SUM = 333,
     TOK_TABLES = 334,
     TOK_TO = 335,
     TOK_TRANSACTION = 336,
     TOK_TRUE = 337,
     TOK_TRUNCATE = 338,
     TOK_UNCOMMITTED = 339,
     TOK_UPDATE = 340,
     TOK_VALUES = 341,
     TOK_VARIABLES = 342,
     TOK_WARNINGS = 343,
     TOK_WEIGHT = 344,
     TOK_WHERE = 345,
     TOK_WITHIN = 346,
     TOK_OR = 347,
     TOK_AND = 348,
     TOK_NE = 349,
     TOK_GTE = 350,
     TOK_LTE = 351,
     TOK_NOT = 352,
     TOK_NEG = 353
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
#define TOK_AGENT 267
#define TOK_AS 268
#define TOK_ASC 269
#define TOK_ATTACH 270
#define TOK_AVG 271
#define TOK_BEGIN 272
#define TOK_BETWEEN 273
#define TOK_BY 274
#define TOK_CALL 275
#define TOK_CHARACTER 276
#define TOK_COLLATION 277
#define TOK_COMMIT 278
#define TOK_COMMITTED 279
#define TOK_COUNT 280
#define TOK_CREATE 281
#define TOK_DELETE 282
#define TOK_DESC 283
#define TOK_DESCRIBE 284
#define TOK_DISTINCT 285
#define TOK_DIV 286
#define TOK_DROP 287
#define TOK_FALSE 288
#define TOK_FLOAT 289
#define TOK_FLUSH 290
#define TOK_FROM 291
#define TOK_FUNCTION 292
#define TOK_GLOBAL 293
#define TOK_GROUP 294
#define TOK_GROUPBY 295
#define TOK_GROUP_CONCAT 296
#define TOK_ID 297
#define TOK_IN 298
#define TOK_INDEX 299
#define TOK_INSERT 300
#define TOK_INT 301
#define TOK_INTO 302
#define TOK_ISOLATION 303
#define TOK_LEVEL 304
#define TOK_LIKE 305
#define TOK_LIMIT 306
#define TOK_MATCH 307
#define TOK_MAX 308
#define TOK_META 309
#define TOK_MIN 310
#define TOK_MOD 311
#define TOK_NAMES 312
#define TOK_NULL 313
#define TOK_OPTION 314
#define TOK_ORDER 315
#define TOK_OPTIMIZE 316
#define TOK_RAND 317
#define TOK_READ 318
#define TOK_REPEATABLE 319
#define TOK_REPLACE 320
#define TOK_RETURNS 321
#define TOK_ROLLBACK 322
#define TOK_RTINDEX 323
#define TOK_SELECT 324
#define TOK_SERIALIZABLE 325
#define TOK_SET 326
#define TOK_SESSION 327
#define TOK_SHOW 328
#define TOK_SONAME 329
#define TOK_START 330
#define TOK_STATUS 331
#define TOK_STRING 332
#define TOK_SUM 333
#define TOK_TABLES 334
#define TOK_TO 335
#define TOK_TRANSACTION 336
#define TOK_TRUE 337
#define TOK_TRUNCATE 338
#define TOK_UNCOMMITTED 339
#define TOK_UPDATE 340
#define TOK_VALUES 341
#define TOK_VARIABLES 342
#define TOK_WARNINGS 343
#define TOK_WEIGHT 344
#define TOK_WHERE 345
#define TOK_WITHIN 346
#define TOK_OR 347
#define TOK_AND 348
#define TOK_NE 349
#define TOK_GTE 350
#define TOK_LTE 351
#define TOK_NOT 352
#define TOK_NEG 353




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
#define YYFINAL  104
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   882

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  93
/* YYNRULES -- Number of rules. */
#define YYNRULES  261
/* YYNRULES -- Number of states. */
#define YYNSTATES  499

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   353

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   106,    95,     2,
     111,   112,   104,   102,   110,   103,   115,   105,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   109,
      98,    96,    99,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   113,    94,   114,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    97,
     100,   101,   107,   108
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    56,    58,    60,
      71,    73,    77,    79,    82,    83,    85,    88,    90,    95,
     100,   105,   110,   115,   120,   124,   128,   134,   136,   140,
     141,   143,   146,   148,   152,   157,   161,   165,   171,   178,
     182,   187,   193,   197,   201,   205,   209,   213,   217,   221,
     225,   231,   235,   239,   241,   243,   248,   252,   256,   258,
     260,   263,   265,   268,   270,   274,   275,   277,   281,   282,
     284,   290,   291,   293,   297,   303,   305,   309,   311,   314,
     317,   318,   320,   323,   328,   329,   331,   334,   336,   340,
     344,   348,   354,   361,   365,   367,   371,   375,   377,   379,
     381,   383,   385,   387,   390,   393,   397,   401,   405,   409,
     413,   417,   421,   425,   429,   433,   437,   441,   445,   449,
     453,   457,   461,   465,   469,   471,   476,   481,   485,   492,
     499,   501,   505,   507,   509,   513,   519,   522,   523,   526,
     528,   531,   534,   538,   543,   548,   550,   552,   554,   556,
     558,   563,   568,   573,   577,   582,   590,   596,   598,   600,
     602,   604,   606,   608,   610,   612,   614,   617,   624,   626,
     628,   629,   633,   635,   639,   641,   645,   649,   651,   655,
     657,   659,   661,   665,   668,   676,   686,   693,   695,   699,
     701,   705,   707,   711,   712,   715,   717,   721,   725,   726,
     728,   730,   732,   736,   738,   740,   744,   751,   753,   757,
     761,   765,   771,   776,   781,   782,   784,   787,   789,   793,
     797,   800,   804,   811,   812,   814,   816,   819,   822,   825,
     827,   835,   837,   839,   841,   845,   852,   856,   860,   862,
     866,   870
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     117,     0,    -1,   118,    -1,   119,    -1,   119,   109,    -1,
     166,    -1,   174,    -1,   160,    -1,   161,    -1,   164,    -1,
     175,    -1,   184,    -1,   186,    -1,   187,    -1,   190,    -1,
     195,    -1,   196,    -1,   200,    -1,   202,    -1,   203,    -1,
     204,    -1,   197,    -1,   205,    -1,   207,    -1,   208,    -1,
     120,    -1,   119,   109,   120,    -1,   121,    -1,   156,    -1,
      69,   122,    36,   126,   127,   135,   137,   139,   143,   145,
      -1,   123,    -1,   122,   110,   123,    -1,   104,    -1,   125,
     124,    -1,    -1,     3,    -1,    13,     3,    -1,   151,    -1,
      16,   111,   151,   112,    -1,    53,   111,   151,   112,    -1,
      55,   111,   151,   112,    -1,    78,   111,   151,   112,    -1,
      41,   111,   151,   112,    -1,    25,   111,   104,   112,    -1,
      89,   111,   112,    -1,    40,   111,   112,    -1,    25,   111,
      30,     3,   112,    -1,     3,    -1,   126,   110,     3,    -1,
      -1,   128,    -1,    90,   129,    -1,   130,    -1,   129,    93,
     130,    -1,    52,   111,     8,   112,    -1,   131,    96,   132,
      -1,   131,    97,   132,    -1,   131,    43,   111,   134,   112,
      -1,   131,   107,    43,   111,   134,   112,    -1,   131,    43,
       9,    -1,   131,   107,    43,     9,    -1,   131,    18,   132,
      93,   132,    -1,   131,    99,   132,    -1,   131,    98,   132,
      -1,   131,   100,   132,    -1,   131,   101,   132,    -1,   131,
      96,   133,    -1,   131,    97,   133,    -1,   131,    99,   133,
      -1,   131,    98,   133,    -1,   131,    18,   133,    93,   133,
      -1,   131,   100,   133,    -1,   131,   101,   133,    -1,     3,
      -1,     4,    -1,    25,   111,   104,   112,    -1,    40,   111,
     112,    -1,    89,   111,   112,    -1,    42,    -1,     5,    -1,
     103,     5,    -1,     6,    -1,   103,     6,    -1,   132,    -1,
     134,   110,   132,    -1,    -1,   136,    -1,    39,    19,   131,
      -1,    -1,   138,    -1,    91,    39,    60,    19,   141,    -1,
      -1,   140,    -1,    60,    19,   141,    -1,    60,    19,    62,
     111,   112,    -1,   142,    -1,   141,   110,   142,    -1,   131,
      -1,   131,    14,    -1,   131,    28,    -1,    -1,   144,    -1,
      51,     5,    -1,    51,     5,   110,     5,    -1,    -1,   146,
      -1,    59,   147,    -1,   148,    -1,   147,   110,   148,    -1,
       3,    96,     3,    -1,     3,    96,     5,    -1,     3,    96,
     111,   149,   112,    -1,     3,    96,     3,   111,     8,   112,
      -1,     3,    96,     8,    -1,   150,    -1,   149,   110,   150,
      -1,     3,    96,   132,    -1,     3,    -1,     4,    -1,    42,
      -1,     5,    -1,     6,    -1,     9,    -1,   103,   151,    -1,
     107,   151,    -1,   151,   102,   151,    -1,   151,   103,   151,
      -1,   151,   104,   151,    -1,   151,   105,   151,    -1,   151,
      98,   151,    -1,   151,    99,   151,    -1,   151,    95,   151,
      -1,   151,    94,   151,    -1,   151,   106,   151,    -1,   151,
      31,   151,    -1,   151,    56,   151,    -1,   151,   101,   151,
      -1,   151,   100,   151,    -1,   151,    96,   151,    -1,   151,
      97,   151,    -1,   151,    93,   151,    -1,   151,    92,   151,
      -1,   111,   151,   112,    -1,   113,   155,   114,    -1,   152,
      -1,     3,   111,   153,   112,    -1,    43,   111,   153,   112,
      -1,     3,   111,   112,    -1,    55,   111,   151,   110,   151,
     112,    -1,    53,   111,   151,   110,   151,   112,    -1,   154,
      -1,   153,   110,   154,    -1,   151,    -1,     8,    -1,     3,
      96,   132,    -1,   155,   110,     3,    96,   132,    -1,    73,
     158,    -1,    -1,    50,     8,    -1,    88,    -1,    76,   157,
      -1,    54,   157,    -1,    12,    76,   157,    -1,    12,     8,
      76,   157,    -1,    12,     3,    76,   157,    -1,     3,    -1,
      58,    -1,     8,    -1,     5,    -1,     6,    -1,    71,     3,
      96,   163,    -1,    71,     3,    96,   162,    -1,    71,     3,
      96,    58,    -1,    71,    57,   159,    -1,    71,    10,    96,
     159,    -1,    71,    38,     9,    96,   111,   134,   112,    -1,
      71,    38,     3,    96,   162,    -1,     3,    -1,     8,    -1,
      82,    -1,    33,    -1,   132,    -1,    23,    -1,    67,    -1,
     165,    -1,    17,    -1,    75,    81,    -1,   167,    47,     3,
     168,    86,   170,    -1,    45,    -1,    65,    -1,    -1,   111,
     169,   112,    -1,   131,    -1,   169,   110,   131,    -1,   171,
      -1,   170,   110,   171,    -1,   111,   172,   112,    -1,   173,
      -1,   172,   110,   173,    -1,   132,    -1,   133,    -1,     8,
      -1,   111,   134,   112,    -1,   111,   112,    -1,    27,    36,
     126,    90,    42,    96,   132,    -1,    27,    36,   126,    90,
      42,    43,   111,   134,   112,    -1,    20,     3,   111,   176,
     179,   112,    -1,   177,    -1,   176,   110,   177,    -1,   173,
      -1,   111,   178,   112,    -1,     8,    -1,   178,   110,     8,
      -1,    -1,   110,   180,    -1,   181,    -1,   180,   110,   181,
      -1,   173,   182,   183,    -1,    -1,    13,    -1,     3,    -1,
      51,    -1,   185,     3,   157,    -1,    29,    -1,    28,    -1,
      73,    79,   157,    -1,    85,   126,    71,   188,   128,   145,
      -1,   189,    -1,   188,   110,   189,    -1,     3,    96,   132,
      -1,     3,    96,   133,    -1,     3,    96,   111,   134,   112,
      -1,     3,    96,   111,   112,    -1,    73,   198,    87,   191,
      -1,    -1,   192,    -1,    90,   193,    -1,   194,    -1,   193,
      92,   194,    -1,     3,    96,     8,    -1,    73,    22,    -1,
      73,    21,    71,    -1,    71,   198,    81,    48,    49,   199,
      -1,    -1,    38,    -1,    72,    -1,    63,    84,    -1,    63,
      24,    -1,    64,    63,    -1,    70,    -1,    26,    37,     3,
      66,   201,    74,     8,    -1,    46,    -1,    34,    -1,    77,
      -1,    32,    37,     3,    -1,    15,    44,     3,    80,    68,
       3,    -1,    35,    68,     3,    -1,    69,   206,   143,    -1,
      10,    -1,    10,   115,     3,    -1,    83,    68,     3,    -1,
      61,    44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   127,   127,   128,   129,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   158,   159,   163,   164,   168,
     183,   184,   188,   189,   192,   194,   195,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   212,   213,   216,
     218,   222,   226,   227,   231,   236,   243,   251,   259,   268,
     273,   278,   283,   288,   293,   298,   303,   304,   305,   306,
     311,   316,   321,   329,   330,   335,   341,   347,   353,   362,
     363,   374,   375,   379,   385,   391,   393,   397,   404,   406,
     410,   416,   418,   422,   426,   433,   434,   438,   439,   440,
     443,   445,   449,   454,   461,   463,   467,   471,   472,   476,
     481,   486,   492,   497,   505,   510,   517,   527,   528,   529,
     530,   531,   532,   533,   534,   535,   536,   537,   538,   539,
     540,   541,   542,   543,   544,   545,   546,   547,   548,   549,
     550,   551,   552,   553,   554,   558,   559,   560,   561,   562,
     566,   567,   571,   572,   576,   577,   583,   586,   588,   592,
     593,   594,   595,   596,   601,   611,   612,   613,   614,   615,
     619,   624,   629,   634,   635,   639,   644,   652,   653,   657,
     658,   659,   673,   674,   675,   679,   680,   686,   694,   695,
     698,   700,   704,   705,   709,   710,   714,   718,   719,   723,
     724,   725,   726,   727,   733,   741,   755,   763,   767,   774,
     775,   782,   792,   798,   800,   804,   809,   813,   820,   822,
     826,   827,   833,   841,   842,   848,   854,   862,   863,   867,
     871,   875,   879,   889,   895,   896,   900,   904,   905,   909,
     913,   920,   927,   933,   934,   935,   939,   940,   941,   942,
     948,   959,   960,   961,   965,   976,   988,   999,  1007,  1008,
    1017,  1028
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
  "TOK_AGENT", "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", "TOK_BEGIN", 
  "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", "TOK_COLLATION", 
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", 
  "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTO", 
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_RAND", "TOK_READ", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_STRING", "TOK_SUM", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", 
  "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", "')'", "'{'", 
  "'}'", "'.'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "expr_ident", "const_int", 
  "const_float", "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "consthash", "show_stmt", "like_filter", "show_what", 
  "set_value", "set_stmt", "set_global_stmt", "set_string_value", 
  "boolean_value", "transact_op", "start_transaction", "insert_into", 
  "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", "show_variables", 
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
     345,   346,   347,   348,   124,    38,    61,   349,    60,    62,
     350,   351,    43,    45,    42,    47,    37,   352,   353,    59,
      44,    40,    41,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   116,   117,   117,   117,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   119,   119,   120,   120,   121,
     122,   122,   123,   123,   124,   124,   124,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   126,   126,   127,
     127,   128,   129,   129,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   131,   131,   131,   131,   131,   131,   132,
     132,   133,   133,   134,   134,   135,   135,   136,   137,   137,
     138,   139,   139,   140,   140,   141,   141,   142,   142,   142,
     143,   143,   144,   144,   145,   145,   146,   147,   147,   148,
     148,   148,   148,   148,   149,   149,   150,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   152,   152,   152,   152,   152,
     153,   153,   154,   154,   155,   155,   156,   157,   157,   158,
     158,   158,   158,   158,   158,   159,   159,   159,   159,   159,
     160,   160,   160,   160,   160,   161,   161,   162,   162,   163,
     163,   163,   164,   164,   164,   165,   165,   166,   167,   167,
     168,   168,   169,   169,   170,   170,   171,   172,   172,   173,
     173,   173,   173,   173,   174,   174,   175,   176,   176,   177,
     177,   178,   178,   179,   179,   180,   180,   181,   182,   182,
     183,   183,   184,   185,   185,   186,   187,   188,   188,   189,
     189,   189,   189,   190,   191,   191,   192,   193,   193,   194,
     195,   196,   197,   198,   198,   198,   199,   199,   199,   199,
     200,   201,   201,   201,   202,   203,   204,   205,   206,   206,
     207,   208
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,    10,
       1,     3,     1,     2,     0,     1,     2,     1,     4,     4,
       4,     4,     4,     4,     3,     3,     5,     1,     3,     0,
       1,     2,     1,     3,     4,     3,     3,     5,     6,     3,
       4,     5,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     3,     3,     1,     1,     4,     3,     3,     1,     1,
       2,     1,     2,     1,     3,     0,     1,     3,     0,     1,
       5,     0,     1,     3,     5,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     6,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     4,     4,     3,     6,     6,
       1,     3,     1,     1,     3,     5,     2,     0,     2,     1,
       2,     2,     3,     4,     4,     1,     1,     1,     1,     1,
       4,     4,     4,     3,     4,     7,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     6,     1,     1,
       0,     3,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     3,     2,     7,     9,     6,     1,     3,     1,
       3,     1,     3,     0,     2,     1,     3,     3,     0,     1,
       1,     1,     3,     1,     1,     3,     6,     1,     3,     3,
       3,     5,     4,     4,     0,     1,     2,     1,     3,     3,
       2,     3,     6,     0,     1,     1,     2,     2,     2,     1,
       7,     1,     1,     1,     3,     6,     3,     3,     1,     3,
       3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   185,     0,   182,     0,     0,   224,   223,     0,
       0,   188,     0,   189,   183,     0,   243,   243,     0,     0,
       0,     0,     2,     3,    25,    27,    28,     7,     8,     9,
     184,     5,     0,     6,    10,    11,     0,    12,    13,    14,
      15,    16,    21,    17,    18,    19,    20,    22,    23,    24,
       0,     0,     0,     0,     0,     0,     0,   117,   118,   120,
     121,   122,   258,     0,     0,     0,     0,   119,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,    30,
      34,    37,   144,   100,     0,     0,   244,     0,   245,     0,
       0,     0,   240,   244,   157,   157,   157,   159,   156,     0,
     186,     0,    47,     0,     1,     4,     0,   157,     0,     0,
       0,     0,   254,   256,   261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   123,   124,
       0,     0,     0,     0,     0,    35,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   257,   101,     0,     0,
       0,     0,   165,   168,   169,   167,   166,   173,     0,     0,
       0,   157,   241,     0,   161,   160,   225,   234,   260,     0,
       0,     0,     0,    26,   190,   222,     0,    79,    81,   201,
       0,     0,   199,   200,   209,   213,   207,     0,     0,   153,
     147,   152,     0,   150,   259,     0,     0,     0,    45,     0,
       0,     0,     0,     0,    44,     0,     0,   142,     0,     0,
     143,    49,    31,    36,   134,   135,   141,   140,   132,   131,
     138,   139,   129,   130,   137,   136,   125,   126,   127,   128,
     133,   102,   177,   178,   180,   172,   179,     0,   181,   171,
     170,   174,     0,     0,     0,   157,   157,   162,   158,     0,
     233,   235,     0,     0,   227,    48,     0,     0,     0,    80,
      82,   211,   203,    83,     0,     0,     0,     0,   252,   251,
     253,     0,     0,     0,   145,    38,     0,    43,    42,   146,
       0,    39,     0,    40,    41,     0,     0,   154,     0,     0,
      85,    50,     0,   176,     0,     0,   164,   163,     0,   236,
     237,     0,     0,   104,    73,    74,     0,     0,    78,     0,
     192,     0,     0,   255,     0,   202,     0,   210,   209,   208,
     214,   215,   206,     0,     0,     0,   151,    46,     0,     0,
       0,     0,    51,    52,     0,     0,    88,    86,   103,     0,
       0,     0,   249,   242,     0,     0,     0,   229,   230,   228,
       0,   226,   105,     0,     0,     0,     0,   191,     0,   187,
     194,    84,   212,   219,     0,     0,   250,     0,   204,   149,
     148,   155,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    91,    89,   175,   247,   246,
     248,   239,   238,   232,     0,     0,   106,   107,     0,    76,
      77,   193,     0,     0,   197,     0,   220,   221,   217,   218,
     216,     0,     0,    53,     0,     0,    59,     0,    55,    66,
      56,    67,    63,    69,    62,    68,    64,    71,    65,    72,
       0,    87,     0,     0,   100,    92,   231,     0,     0,    75,
       0,   196,   195,   205,    54,     0,     0,     0,    60,     0,
       0,     0,   104,   109,   110,   113,     0,   108,   198,    61,
       0,    70,    57,     0,     0,     0,    97,    93,    95,    29,
       0,     0,     0,   114,    58,    90,     0,    98,    99,     0,
       0,     0,     0,   111,    94,    96,   112,   116,   115
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,    78,    79,   137,    80,
     103,   300,   301,   342,   343,   476,   273,   193,   274,   346,
     347,   395,   396,   444,   445,   477,   478,   156,   157,   361,
     362,   406,   407,   482,   483,    81,    82,   202,   203,   132,
      26,   174,    98,   167,    27,    28,   249,   250,    29,    30,
      31,    32,   267,   321,   369,   370,   413,   194,    33,    34,
     195,   196,   275,   277,   330,   331,   374,   418,    35,    36,
      37,    38,   263,   264,    39,   260,   261,   309,   310,    40,
      41,    42,    89,   353,    43,   281,    44,    45,    46,    47,
      83,    48,    49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -293
static const short yypact[] =
{
     797,   -19,  -293,    29,  -293,   -15,    55,  -293,  -293,    20,
      -4,  -293,    23,  -293,  -293,   101,   236,   613,    22,    81,
     118,   155,  -293,    41,  -293,  -293,  -293,  -293,  -293,  -293,
    -293,  -293,   140,  -293,  -293,  -293,   162,  -293,  -293,  -293,
    -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,
     185,    90,   214,   118,   221,   227,   229,   130,  -293,  -293,
    -293,  -293,   129,   137,   143,   146,   173,  -293,   184,   188,
     192,   193,   195,   282,  -293,   282,   282,   260,     2,  -293,
      53,   576,  -293,   222,   180,   200,   144,   323,  -293,   235,
     149,   239,  -293,  -293,   277,   277,   277,  -293,  -293,   246,
    -293,   331,  -293,   -47,  -293,   120,   336,   277,   268,    21,
     290,    61,  -293,  -293,  -293,   247,   354,   282,    30,   253,
     282,   309,   282,   282,   282,   254,   256,   257,  -293,  -293,
     363,   273,   150,   118,   194,  -293,   367,  -293,   282,   282,
     282,   282,   282,   282,   282,   282,   282,   282,   282,   282,
     282,   282,   282,   282,   282,   366,  -293,  -293,   158,   323,
     283,   284,  -293,  -293,  -293,  -293,  -293,  -293,   330,   308,
     310,   277,  -293,   379,  -293,  -293,  -293,   300,  -293,   388,
     389,   194,   104,  -293,   285,  -293,   345,  -293,  -293,  -293,
      35,    10,  -293,  -293,  -293,   304,  -293,    12,   381,  -293,
    -293,   576,   -22,  -293,  -293,   387,   418,   327,  -293,   414,
      50,   305,   332,   440,  -293,   282,   282,  -293,    32,   437,
    -293,   128,  -293,  -293,  -293,  -293,   627,   653,   679,   705,
     688,   688,   165,   165,   165,   165,    77,    77,  -293,  -293,
    -293,   337,  -293,  -293,  -293,  -293,  -293,   436,  -293,  -293,
    -293,  -293,    36,   335,   399,   277,   277,  -293,  -293,   446,
    -293,  -293,   355,   152,  -293,  -293,   169,   364,   449,  -293,
    -293,  -293,  -293,  -293,   117,   199,    21,   341,  -293,  -293,
    -293,   380,   -35,   309,  -293,  -293,   360,  -293,  -293,  -293,
     282,  -293,   282,  -293,  -293,   518,   544,  -293,   377,   134,
     435,  -293,   471,  -293,    32,   114,  -293,  -293,   382,   385,
    -293,    37,   388,   439,  -293,  -293,   383,   384,  -293,   390,
    -293,   210,   391,  -293,    32,  -293,   495,  -293,   182,  -293,
     394,  -293,  -293,   497,   410,    32,  -293,  -293,   466,   492,
      32,   413,   407,  -293,   276,   506,   438,  -293,  -293,   211,
      31,   464,  -293,  -293,   520,   446,    11,  -293,  -293,  -293,
     527,  -293,  -293,   427,   441,   442,   169,  -293,    28,   445,
    -293,  -293,  -293,  -293,     8,    28,  -293,    32,  -293,  -293,
    -293,  -293,   539,   134,    25,    14,    25,    25,    25,    25,
      25,    25,   507,   169,   512,   496,  -293,  -293,  -293,  -293,
    -293,  -293,  -293,  -293,   220,   461,   463,  -293,   465,  -293,
    -293,  -293,    15,   228,  -293,   391,  -293,  -293,  -293,   563,
    -293,   232,   467,  -293,   487,   488,  -293,    32,  -293,  -293,
    -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,  -293,
      19,  -293,   522,   564,   222,  -293,  -293,     9,   527,  -293,
      28,  -293,  -293,  -293,  -293,    32,    -2,   233,  -293,    32,
     580,   203,   439,   490,  -293,  -293,   599,  -293,  -293,  -293,
     597,  -293,  -293,   237,   169,   494,    34,   498,  -293,  -293,
     598,   513,   243,  -293,  -293,   498,   514,  -293,  -293,   169,
     515,    32,   599,  -293,  -293,  -293,  -293,  -293,  -293
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -293,  -293,  -293,  -293,   524,  -293,  -293,   499,  -293,  -293,
     -46,  -293,   368,  -293,   269,  -264,  -109,  -292,  -291,  -293,
    -293,  -293,  -293,  -293,  -293,   156,   164,   212,  -293,   197,
    -293,  -293,   207,  -293,   168,   -70,  -293,   536,   378,  -293,
    -293,   -86,  -293,   503,  -293,  -293,   411,  -293,  -293,  -293,
    -293,  -293,  -293,  -293,  -293,   249,  -293,  -275,  -293,  -293,
    -293,   412,  -293,  -293,  -293,   291,  -293,  -293,  -293,  -293,
    -293,  -293,  -293,   353,  -293,  -293,  -293,  -293,   338,  -293,
    -293,  -293,   669,  -293,  -293,  -293,  -293,  -293,  -293,  -293,
    -293,  -293,  -293
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -219
static const short yytable[] =
{
     192,   328,   320,   128,   188,   129,   130,   111,   334,   175,
     176,   416,   463,   349,   464,   187,   187,   465,   271,   358,
     187,   185,    52,   426,   179,    50,   187,   188,   458,   189,
     187,   188,    51,   187,   188,   344,   189,   187,   133,   242,
     269,   270,   187,   188,   243,   201,   278,   205,   487,   248,
     209,   201,   211,   212,   213,   398,   135,    54,   279,   417,
     206,   335,   488,   180,    55,   404,   136,    56,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   257,   421,   221,   283,   280,
     284,    53,   425,   414,   429,   431,   433,   435,   437,   439,
     419,   470,   411,   100,    57,    58,    59,    60,   138,   297,
      61,    62,   134,   247,   247,   399,    90,    63,   247,   344,
     466,   102,   272,   403,   190,   427,    64,   272,   190,   441,
     459,   190,   191,   139,   207,   247,   457,   314,   315,   412,
     190,    65,    66,    67,    68,   295,   296,   160,   356,   101,
     105,   198,   169,   161,    69,   104,    70,   170,    94,   316,
     283,   242,   289,   187,   471,   107,   243,   192,   473,   306,
     307,   180,   314,   315,   317,   468,   318,   350,   351,    71,
      95,   152,   153,   154,   352,  -218,   341,   106,   108,   181,
      72,   244,    97,   182,   316,   373,   138,    57,    58,    59,
      60,   109,   357,    61,    73,    74,   314,   315,    75,   317,
      63,   318,    76,   201,    77,   371,   245,   110,   299,    64,
     338,   139,   339,   319,   112,   171,   378,   324,   316,   325,
     113,   381,   114,  -218,    65,    66,    67,    68,   180,    84,
     246,   115,   299,   317,   116,   318,    85,    69,   117,    70,
      57,    58,    59,    60,   118,   199,    61,   119,   319,   192,
     219,   247,   312,   131,   220,   475,   192,   150,   151,   152,
     153,   154,    71,   155,    86,   424,   158,   428,   430,   432,
     434,   436,   438,    72,   120,    57,    58,    59,    60,    67,
      68,    61,   319,    87,   384,   121,   159,    73,    74,   122,
     126,    75,   127,   123,   124,    76,   125,    77,    88,   326,
     172,   327,    57,    58,    59,    60,   168,   199,    61,   385,
     366,   324,   367,   397,    67,    68,   162,   173,   163,   164,
     324,   165,   446,   177,   178,   126,   138,   127,   450,   184,
     451,   192,   324,   324,   453,   472,   469,   324,   186,   484,
      73,    67,    68,   492,    75,   493,   197,   204,    76,   200,
      77,   139,   126,   138,   127,   208,   214,   215,   216,   218,
     223,   241,   386,   387,   388,   389,   390,   391,   254,   252,
     253,   166,   497,   392,   255,    73,   256,   258,   139,    75,
     259,   262,   265,    76,   138,    77,   266,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,    73,   268,   276,   290,    75,   291,   138,   139,
      76,   286,    77,   282,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   287,
     298,   269,   292,   139,   293,   138,   304,   302,   305,   308,
     322,   311,   323,   332,   333,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     139,   138,   337,   340,   345,   217,   348,   355,   354,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   363,   364,   139,   138,   360,   285,
     383,   365,   368,   372,   375,   376,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   377,   139,   138,   382,   393,   288,   400,   401,   394,
     405,   408,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   422,   139,   138,
     440,   442,   294,   409,   410,   415,   443,   447,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   448,   139,   138,   373,   449,   379,   454,
     455,   456,   460,   461,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   474,
     139,   480,   481,   270,   380,   486,   490,   138,   489,   491,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,    90,   494,   496,   290,   183,
     485,   313,   139,   222,    91,    92,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    93,   423,   495,   292,   467,   462,   210,   138,   479,
     498,   336,   251,   303,   452,   359,   420,    94,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   139,   138,    88,    99,     0,   329,    95,
       0,     0,    96,   402,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,     0,     0,     0,   139,
     138,     0,     0,     0,     0,     0,     0,     0,     0,   138,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   139,   138,     0,     0,     0,
       0,     0,     0,     0,   139,     0,     0,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   139,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,     0,     0,     0,     0,
       0,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     1,     0,     2,     0,     0,     3,     0,     0,
       4,     0,     0,     5,     6,     7,     8,     0,     0,     9,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,    13,     0,    14,     0,    15,     0,    16,     0,
      17,     0,    18,     0,     0,     0,     0,     0,     0,     0,
      19,     0,    20
};

static const short yycheck[] =
{
     109,   276,   266,    73,     6,    75,    76,    53,    43,    95,
      96,     3,     3,   304,     5,     5,     5,     8,     8,   311,
       5,   107,    37,     9,    71,    44,     5,     6,     9,     8,
       5,     6,     3,     5,     6,   299,     8,     5,    36,     3,
       5,     6,     5,     6,     8,   115,    34,   117,    14,   158,
     120,   121,   122,   123,   124,    24,     3,    37,    46,    51,
      30,    96,    28,   110,    68,   356,    13,    44,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   171,   377,   133,   110,    77,
     112,    36,   384,   368,   386,   387,   388,   389,   390,   391,
     375,   103,   366,    81,     3,     4,     5,     6,    31,   218,
       9,    10,   110,   103,   103,    84,    12,    16,   103,   383,
     111,     3,   112,   112,   103,   111,    25,   112,   103,   393,
     111,   103,   111,    56,   104,   103,   427,     3,     4,   111,
     103,    40,    41,    42,    43,   215,   216,     3,   111,    68,
     109,    90,     3,     9,    53,     0,    55,     8,    54,    25,
     110,     3,   112,     5,   456,     3,     8,   276,   459,   255,
     256,   110,     3,     4,    40,   450,    42,    63,    64,    78,
      76,   104,   105,   106,    70,     3,    52,    47,     3,    69,
      89,    33,    88,    73,    25,    13,    31,     3,     4,     5,
       6,   111,   311,     9,   103,   104,     3,     4,   107,    40,
      16,    42,   111,   283,   113,   324,    58,     3,    90,    25,
     290,    56,   292,    89,     3,    76,   335,   110,    25,   112,
       3,   340,     3,    51,    40,    41,    42,    43,   110,     3,
      82,   111,    90,    40,   115,    42,    10,    53,   111,    55,
       3,     4,     5,     6,   111,     8,     9,   111,    89,   368,
     110,   103,   110,     3,   114,    62,   375,   102,   103,   104,
     105,   106,    78,    51,    38,   384,    96,   386,   387,   388,
     389,   390,   391,    89,   111,     3,     4,     5,     6,    42,
      43,     9,    89,    57,    18,   111,    96,   103,   104,   111,
      53,   107,    55,   111,   111,   111,   111,   113,    72,   110,
      71,   112,     3,     4,     5,     6,    81,     8,     9,    43,
     110,   110,   112,   112,    42,    43,     3,    50,     5,     6,
     110,     8,   112,    87,     3,    53,    31,    55,   110,     3,
     112,   450,   110,   110,   112,   112,   455,   110,    80,   112,
     103,    42,    43,   110,   107,   112,    66,     3,   111,   112,
     113,    56,    53,    31,    55,   112,   112,   111,   111,    96,
       3,     5,    96,    97,    98,    99,   100,   101,    48,    96,
      96,    58,   491,   107,    76,   103,    76,     8,    56,   107,
      90,     3,     3,   111,    31,   113,   111,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   103,    68,   110,   110,   107,   112,    31,    56,
     111,     3,   113,    42,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   112,
       3,     5,   110,    56,   112,    31,   111,   110,    49,     3,
      86,    96,     3,   112,    74,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      56,    31,   112,    96,    39,   112,     5,    92,    96,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   111,   111,    56,    31,    59,   112,
      93,   111,   111,     8,   110,     8,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   111,    56,    31,   111,    19,   112,    63,     8,    91,
       3,   104,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,     8,    56,    31,
      43,    39,   112,   112,   112,   110,    60,    96,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   110,    56,    31,    13,   112,   112,   112,
      93,    93,    60,    19,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    19,
      56,   111,     3,     6,   112,   111,     8,    31,   110,    96,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    12,   112,   112,   110,   105,
     474,   263,    56,   134,    21,    22,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    38,   383,   489,   110,   448,   444,   121,    31,   462,
     492,   283,   159,   252,   415,   312,   375,    54,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,    56,    31,    72,    17,    -1,   276,    76,
      -1,    -1,    79,   355,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    -1,    56,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,    15,    -1,    17,    -1,    -1,    20,    -1,    -1,
      23,    -1,    -1,    26,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    -1,    65,    -1,    67,    -1,    69,    -1,    71,    -1,
      73,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    -1,    85
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    61,    65,    67,    69,    71,    73,    75,    83,
      85,   117,   118,   119,   120,   121,   156,   160,   161,   164,
     165,   166,   167,   174,   175,   184,   185,   186,   187,   190,
     195,   196,   197,   200,   202,   203,   204,   205,   207,   208,
      44,     3,    37,    36,    37,    68,    44,     3,     4,     5,
       6,     9,    10,    16,    25,    40,    41,    42,    43,    53,
      55,    78,    89,   103,   104,   107,   111,   113,   122,   123,
     125,   151,   152,   206,     3,    10,    38,    57,    72,   198,
      12,    21,    22,    38,    54,    76,    79,    88,   158,   198,
      81,    68,     3,   126,     0,   109,    47,     3,     3,   111,
       3,   126,     3,     3,     3,   111,   115,   111,   111,   111,
     111,   111,   111,   111,   111,   111,    53,    55,   151,   151,
     151,     3,   155,    36,   110,     3,    13,   124,    31,    56,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    51,   143,   144,    96,    96,
       3,     9,     3,     5,     6,     8,    58,   159,    81,     3,
       8,    76,    71,    50,   157,   157,   157,    87,     3,    71,
     110,    69,    73,   120,     3,   157,    80,     5,     6,     8,
     103,   111,   132,   133,   173,   176,   177,    66,    90,     8,
     112,   151,   153,   154,     3,   151,    30,   104,   112,   151,
     153,   151,   151,   151,   112,   111,   111,   112,    96,   110,
     114,   126,   123,     3,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,     5,     3,     8,    33,    58,    82,   103,   132,   162,
     163,   159,    96,    96,    48,    76,    76,   157,     8,    90,
     191,   192,     3,   188,   189,     3,   111,   168,    68,     5,
       6,     8,   112,   132,   134,   178,   110,   179,    34,    46,
      77,   201,    42,   110,   112,   112,     3,   112,   112,   112,
     110,   112,   110,   112,   112,   151,   151,   132,     3,    90,
     127,   128,   110,   162,   111,    49,   157,   157,     3,   193,
     194,    96,   110,   128,     3,     4,    25,    40,    42,    89,
     131,   169,    86,     3,   110,   112,   110,   112,   173,   177,
     180,   181,   112,    74,    43,    96,   154,   112,   151,   151,
      96,    52,   129,   130,   131,    39,   135,   136,     5,   134,
      63,    64,    70,   199,    96,    92,   111,   132,   133,   189,
      59,   145,   146,   111,   111,   111,   110,   112,   111,   170,
     171,   132,     8,    13,   182,   110,     8,   111,   132,   112,
     112,   132,   111,    93,    18,    43,    96,    97,    98,    99,
     100,   101,   107,    19,    91,   137,   138,   112,    24,    84,
      63,     8,   194,   112,   134,     3,   147,   148,   104,   112,
     112,   131,   111,   172,   173,   110,     3,    51,   183,   173,
     181,   134,     8,   130,   132,   133,     9,   111,   132,   133,
     132,   133,   132,   133,   132,   133,   132,   133,   132,   133,
      43,   131,    39,    60,   139,   140,   112,    96,   110,   112,
     110,   112,   171,   112,   112,    93,    93,   134,     9,   111,
      60,    19,   143,     3,     5,     8,   111,   148,   173,   132,
     103,   133,   112,   134,    19,    62,   131,   141,   142,   145,
     111,     3,   149,   150,   112,   141,   111,    14,    28,   110,
       8,    96,   110,   112,   112,   142,   112,   132,   150
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

  case 25:

    { pParser->PushQuery(); ;}
    break;

  case 26:

    { pParser->PushQuery(); ;}
    break;

  case 29:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 32:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 36:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 37:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 38:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 39:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 40:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 41:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 42:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 43:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 44:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 45:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 46:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 48:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 54:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 55:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 56:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 57:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 58:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 59:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 60:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddIntFilterGTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1 ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddIntFilterLTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 64:

    {
			if ( !pParser->AddIntFilterGTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 65:

    {
			if ( !pParser->AddIntFilterLTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 76:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 77:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 78:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 79:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 80:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 81:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 82:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 83:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 84:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 87:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 90:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 93:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 94:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 96:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 98:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 99:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 102:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 103:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 109:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 110:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 111:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 112:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 113:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 114:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 115:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 116:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 118:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 119:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 123:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 125:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 126:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 127:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 128:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 129:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 130:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 131:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 132:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 133:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 134:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 135:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 145:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 146:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 147:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 148:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 149:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 154:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 155:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 158:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 159:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 160:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 161:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 162:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS; ;}
    break;

  case 163:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 164:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 170:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 171:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 172:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 175:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 176:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 179:

    { yyval.m_iValue = 1; ;}
    break;

  case 180:

    { yyval.m_iValue = 0; ;}
    break;

  case 181:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 182:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 183:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 184:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 187:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 188:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 192:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 193:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 196:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 197:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 198:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 199:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 200:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 201:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 202:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 203:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 204:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 205:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 206:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 207:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 208:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 210:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 211:

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

  case 212:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 215:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 217:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 221:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 222:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 225:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 226:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 229:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 230:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 231:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 232:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 233:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 240:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 241:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 242:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 250:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 251:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 252:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 253:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 254:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 255:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 256:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 259:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 260:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 261:

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

