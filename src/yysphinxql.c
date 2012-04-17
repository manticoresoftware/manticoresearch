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
     TOK_AS = 267,
     TOK_ASC = 268,
     TOK_ATTACH = 269,
     TOK_AVG = 270,
     TOK_BEGIN = 271,
     TOK_BETWEEN = 272,
     TOK_BY = 273,
     TOK_CALL = 274,
     TOK_COLLATION = 275,
     TOK_COMMIT = 276,
     TOK_COMMITTED = 277,
     TOK_COUNT = 278,
     TOK_CREATE = 279,
     TOK_DELETE = 280,
     TOK_DESC = 281,
     TOK_DESCRIBE = 282,
     TOK_DISTINCT = 283,
     TOK_DIV = 284,
     TOK_DROP = 285,
     TOK_FALSE = 286,
     TOK_FLOAT = 287,
     TOK_FLUSH = 288,
     TOK_FROM = 289,
     TOK_FUNCTION = 290,
     TOK_GLOBAL = 291,
     TOK_GROUP = 292,
     TOK_GROUPBY = 293,
     TOK_GROUP_CONCAT = 294,
     TOK_ID = 295,
     TOK_IN = 296,
     TOK_INDEX = 297,
     TOK_INSERT = 298,
     TOK_INT = 299,
     TOK_INTO = 300,
     TOK_ISOLATION = 301,
     TOK_LEVEL = 302,
     TOK_LIMIT = 303,
     TOK_MATCH = 304,
     TOK_MAX = 305,
     TOK_META = 306,
     TOK_MIN = 307,
     TOK_MOD = 308,
     TOK_NAMES = 309,
     TOK_NULL = 310,
     TOK_OPTION = 311,
     TOK_ORDER = 312,
     TOK_RAND = 313,
     TOK_READ = 314,
     TOK_REPEATABLE = 315,
     TOK_REPLACE = 316,
     TOK_RETURNS = 317,
     TOK_ROLLBACK = 318,
     TOK_RTINDEX = 319,
     TOK_SELECT = 320,
     TOK_SERIALIZABLE = 321,
     TOK_SET = 322,
     TOK_SESSION = 323,
     TOK_SHOW = 324,
     TOK_SONAME = 325,
     TOK_START = 326,
     TOK_STATUS = 327,
     TOK_SUM = 328,
     TOK_TABLES = 329,
     TOK_TO = 330,
     TOK_TRANSACTION = 331,
     TOK_TRUE = 332,
     TOK_TRUNCATE = 333,
     TOK_UNCOMMITTED = 334,
     TOK_UPDATE = 335,
     TOK_VALUES = 336,
     TOK_VARIABLES = 337,
     TOK_WARNINGS = 338,
     TOK_WEIGHT = 339,
     TOK_WHERE = 340,
     TOK_WITHIN = 341,
     TOK_OR = 342,
     TOK_AND = 343,
     TOK_NE = 344,
     TOK_GTE = 345,
     TOK_LTE = 346,
     TOK_NOT = 347,
     TOK_NEG = 348
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
#define TOK_AS 267
#define TOK_ASC 268
#define TOK_ATTACH 269
#define TOK_AVG 270
#define TOK_BEGIN 271
#define TOK_BETWEEN 272
#define TOK_BY 273
#define TOK_CALL 274
#define TOK_COLLATION 275
#define TOK_COMMIT 276
#define TOK_COMMITTED 277
#define TOK_COUNT 278
#define TOK_CREATE 279
#define TOK_DELETE 280
#define TOK_DESC 281
#define TOK_DESCRIBE 282
#define TOK_DISTINCT 283
#define TOK_DIV 284
#define TOK_DROP 285
#define TOK_FALSE 286
#define TOK_FLOAT 287
#define TOK_FLUSH 288
#define TOK_FROM 289
#define TOK_FUNCTION 290
#define TOK_GLOBAL 291
#define TOK_GROUP 292
#define TOK_GROUPBY 293
#define TOK_GROUP_CONCAT 294
#define TOK_ID 295
#define TOK_IN 296
#define TOK_INDEX 297
#define TOK_INSERT 298
#define TOK_INT 299
#define TOK_INTO 300
#define TOK_ISOLATION 301
#define TOK_LEVEL 302
#define TOK_LIMIT 303
#define TOK_MATCH 304
#define TOK_MAX 305
#define TOK_META 306
#define TOK_MIN 307
#define TOK_MOD 308
#define TOK_NAMES 309
#define TOK_NULL 310
#define TOK_OPTION 311
#define TOK_ORDER 312
#define TOK_RAND 313
#define TOK_READ 314
#define TOK_REPEATABLE 315
#define TOK_REPLACE 316
#define TOK_RETURNS 317
#define TOK_ROLLBACK 318
#define TOK_RTINDEX 319
#define TOK_SELECT 320
#define TOK_SERIALIZABLE 321
#define TOK_SET 322
#define TOK_SESSION 323
#define TOK_SHOW 324
#define TOK_SONAME 325
#define TOK_START 326
#define TOK_STATUS 327
#define TOK_SUM 328
#define TOK_TABLES 329
#define TOK_TO 330
#define TOK_TRANSACTION 331
#define TOK_TRUE 332
#define TOK_TRUNCATE 333
#define TOK_UNCOMMITTED 334
#define TOK_UPDATE 335
#define TOK_VALUES 336
#define TOK_VARIABLES 337
#define TOK_WARNINGS 338
#define TOK_WEIGHT 339
#define TOK_WHERE 340
#define TOK_WITHIN 341
#define TOK_OR 342
#define TOK_AND 343
#define TOK_NE 344
#define TOK_GTE 345
#define TOK_LTE 346
#define TOK_NOT 347
#define TOK_NEG 348




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
#define YYFINAL  97
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   808

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  109
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  89
/* YYNRULES -- Number of rules. */
#define YYNRULES  248
/* YYNRULES -- Number of states. */
#define YYNSTATES  465

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   348

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   101,    90,     2,
     106,   107,    99,    97,   105,    98,   108,   100,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   104,
      93,    91,    94,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    89,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    92,    95,    96,   102,   103
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    52,    54,    56,    67,    69,
      73,    75,    78,    79,    81,    84,    86,    91,    96,   101,
     106,   111,   116,   120,   124,   130,   132,   136,   137,   139,
     142,   144,   148,   153,   157,   161,   167,   174,   178,   183,
     189,   193,   197,   201,   205,   209,   213,   217,   221,   227,
     231,   235,   237,   239,   244,   248,   252,   254,   256,   259,
     261,   264,   266,   270,   271,   273,   277,   278,   280,   286,
     287,   289,   293,   299,   301,   305,   307,   310,   313,   314,
     316,   319,   324,   325,   327,   330,   332,   336,   340,   344,
     350,   357,   361,   363,   367,   371,   373,   375,   377,   379,
     381,   383,   386,   389,   393,   397,   401,   405,   409,   413,
     417,   421,   425,   429,   433,   437,   441,   445,   449,   453,
     457,   461,   463,   468,   473,   477,   484,   491,   493,   497,
     499,   501,   504,   506,   508,   510,   512,   514,   516,   518,
     520,   525,   530,   535,   539,   544,   552,   558,   560,   562,
     564,   566,   568,   570,   572,   574,   576,   579,   586,   588,
     590,   591,   595,   597,   601,   603,   607,   611,   613,   617,
     619,   621,   623,   627,   630,   638,   648,   655,   657,   661,
     663,   667,   669,   673,   674,   677,   679,   683,   687,   688,
     690,   692,   694,   697,   699,   701,   704,   710,   712,   716,
     720,   724,   730,   735,   740,   741,   743,   746,   748,   752,
     756,   759,   766,   767,   769,   771,   774,   777,   780,   782,
     790,   792,   794,   798,   805,   809,   813,   815,   819
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     110,     0,    -1,   111,    -1,   112,    -1,   112,   104,    -1,
     157,    -1,   165,    -1,   151,    -1,   152,    -1,   155,    -1,
     166,    -1,   175,    -1,   177,    -1,   178,    -1,   181,    -1,
     186,    -1,   190,    -1,   192,    -1,   193,    -1,   194,    -1,
     187,    -1,   195,    -1,   197,    -1,   113,    -1,   112,   104,
     113,    -1,   114,    -1,   148,    -1,    65,   115,    34,   119,
     120,   128,   130,   132,   136,   138,    -1,   116,    -1,   115,
     105,   116,    -1,    99,    -1,   118,   117,    -1,    -1,     3,
      -1,    12,     3,    -1,   144,    -1,    15,   106,   144,   107,
      -1,    50,   106,   144,   107,    -1,    52,   106,   144,   107,
      -1,    73,   106,   144,   107,    -1,    39,   106,   144,   107,
      -1,    23,   106,    99,   107,    -1,    84,   106,   107,    -1,
      38,   106,   107,    -1,    23,   106,    28,     3,   107,    -1,
       3,    -1,   119,   105,     3,    -1,    -1,   121,    -1,    85,
     122,    -1,   123,    -1,   122,    88,   123,    -1,    49,   106,
       8,   107,    -1,   124,    91,   125,    -1,   124,    92,   125,
      -1,   124,    41,   106,   127,   107,    -1,   124,   102,    41,
     106,   127,   107,    -1,   124,    41,     9,    -1,   124,   102,
      41,     9,    -1,   124,    17,   125,    88,   125,    -1,   124,
      94,   125,    -1,   124,    93,   125,    -1,   124,    95,   125,
      -1,   124,    96,   125,    -1,   124,    91,   126,    -1,   124,
      92,   126,    -1,   124,    94,   126,    -1,   124,    93,   126,
      -1,   124,    17,   126,    88,   126,    -1,   124,    95,   126,
      -1,   124,    96,   126,    -1,     3,    -1,     4,    -1,    23,
     106,    99,   107,    -1,    38,   106,   107,    -1,    84,   106,
     107,    -1,    40,    -1,     5,    -1,    98,     5,    -1,     6,
      -1,    98,     6,    -1,   125,    -1,   127,   105,   125,    -1,
      -1,   129,    -1,    37,    18,   124,    -1,    -1,   131,    -1,
      86,    37,    57,    18,   134,    -1,    -1,   133,    -1,    57,
      18,   134,    -1,    57,    18,    58,   106,   107,    -1,   135,
      -1,   134,   105,   135,    -1,   124,    -1,   124,    13,    -1,
     124,    26,    -1,    -1,   137,    -1,    48,     5,    -1,    48,
       5,   105,     5,    -1,    -1,   139,    -1,    56,   140,    -1,
     141,    -1,   140,   105,   141,    -1,     3,    91,     3,    -1,
       3,    91,     5,    -1,     3,    91,   106,   142,   107,    -1,
       3,    91,     3,   106,     8,   107,    -1,     3,    91,     8,
      -1,   143,    -1,   142,   105,   143,    -1,     3,    91,   125,
      -1,     3,    -1,     4,    -1,    40,    -1,     5,    -1,     6,
      -1,     9,    -1,    98,   144,    -1,   102,   144,    -1,   144,
      97,   144,    -1,   144,    98,   144,    -1,   144,    99,   144,
      -1,   144,   100,   144,    -1,   144,    93,   144,    -1,   144,
      94,   144,    -1,   144,    90,   144,    -1,   144,    89,   144,
      -1,   144,   101,   144,    -1,   144,    29,   144,    -1,   144,
      53,   144,    -1,   144,    96,   144,    -1,   144,    95,   144,
      -1,   144,    91,   144,    -1,   144,    92,   144,    -1,   144,
      88,   144,    -1,   144,    87,   144,    -1,   106,   144,   107,
      -1,   145,    -1,     3,   106,   146,   107,    -1,    41,   106,
     146,   107,    -1,     3,   106,   107,    -1,    52,   106,   144,
     105,   144,   107,    -1,    50,   106,   144,   105,   144,   107,
      -1,   147,    -1,   146,   105,   147,    -1,   144,    -1,     8,
      -1,    69,   149,    -1,    83,    -1,    72,    -1,    51,    -1,
       3,    -1,    55,    -1,     8,    -1,     5,    -1,     6,    -1,
      67,     3,    91,   154,    -1,    67,     3,    91,   153,    -1,
      67,     3,    91,    55,    -1,    67,    54,   150,    -1,    67,
      10,    91,   150,    -1,    67,    36,     9,    91,   106,   127,
     107,    -1,    67,    36,     3,    91,   153,    -1,     3,    -1,
       8,    -1,    77,    -1,    31,    -1,   125,    -1,    21,    -1,
      63,    -1,   156,    -1,    16,    -1,    71,    76,    -1,   158,
      45,     3,   159,    81,   161,    -1,    43,    -1,    61,    -1,
      -1,   106,   160,   107,    -1,   124,    -1,   160,   105,   124,
      -1,   162,    -1,   161,   105,   162,    -1,   106,   163,   107,
      -1,   164,    -1,   163,   105,   164,    -1,   125,    -1,   126,
      -1,     8,    -1,   106,   127,   107,    -1,   106,   107,    -1,
      25,    34,   119,    85,    40,    91,   125,    -1,    25,    34,
     119,    85,    40,    41,   106,   127,   107,    -1,    19,     3,
     106,   167,   170,   107,    -1,   168,    -1,   167,   105,   168,
      -1,   164,    -1,   106,   169,   107,    -1,     8,    -1,   169,
     105,     8,    -1,    -1,   105,   171,    -1,   172,    -1,   171,
     105,   172,    -1,   164,   173,   174,    -1,    -1,    12,    -1,
       3,    -1,    48,    -1,   176,     3,    -1,    27,    -1,    26,
      -1,    69,    74,    -1,    80,   119,    67,   179,   121,    -1,
     180,    -1,   179,   105,   180,    -1,     3,    91,   125,    -1,
       3,    91,   126,    -1,     3,    91,   106,   127,   107,    -1,
       3,    91,   106,   107,    -1,    69,   188,    82,   182,    -1,
      -1,   183,    -1,    85,   184,    -1,   185,    -1,   184,    87,
     185,    -1,     3,    91,     8,    -1,    69,    20,    -1,    67,
     188,    76,    46,    47,   189,    -1,    -1,    36,    -1,    68,
      -1,    59,    79,    -1,    59,    22,    -1,    60,    59,    -1,
      66,    -1,    24,    35,     3,    62,   191,    70,     8,    -1,
      44,    -1,    32,    -1,    30,    35,     3,    -1,    14,    42,
       3,    75,    64,     3,    -1,    33,    64,     3,    -1,    65,
     196,   136,    -1,    10,    -1,    10,   108,     3,    -1,    78,
      64,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   122,   122,   123,   124,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   151,   152,   156,   157,   161,   176,   177,
     181,   182,   185,   187,   188,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   205,   206,   209,   211,   215,
     219,   220,   224,   229,   236,   244,   252,   261,   266,   271,
     276,   281,   286,   291,   296,   297,   298,   299,   304,   309,
     314,   322,   323,   328,   334,   340,   346,   355,   356,   360,
     361,   365,   371,   377,   379,   383,   390,   392,   396,   402,
     404,   408,   412,   419,   420,   424,   425,   426,   429,   431,
     435,   440,   447,   449,   453,   457,   458,   462,   467,   472,
     478,   483,   491,   496,   503,   513,   514,   515,   516,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,   530,   531,   532,   533,   534,   535,   536,   537,
     538,   539,   543,   544,   545,   546,   547,   551,   552,   556,
     557,   563,   567,   568,   569,   575,   576,   577,   578,   579,
     583,   588,   593,   598,   599,   603,   608,   616,   617,   621,
     622,   623,   637,   638,   639,   643,   644,   650,   658,   659,
     662,   664,   668,   669,   673,   674,   678,   682,   683,   687,
     688,   689,   690,   691,   697,   705,   719,   727,   731,   738,
     739,   746,   756,   762,   764,   768,   773,   777,   784,   786,
     790,   791,   797,   805,   806,   812,   818,   826,   827,   831,
     835,   839,   843,   853,   859,   860,   864,   868,   869,   873,
     877,   884,   890,   891,   892,   896,   897,   898,   899,   905,
     916,   917,   921,   932,   944,   955,   963,   964,   973
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
  "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", "TOK_BEGIN", 
  "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_COLLATION", "TOK_COMMIT", 
  "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", "TOK_DESC", 
  "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", 
  "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", 
  "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", "TOK_IN", 
  "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_ISOLATION", 
  "TOK_LEVEL", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", 
  "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_SUM", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", 
  "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", "')'", "'.'", 
  "$accept", "request", "statement", "multi_stmt_list", "multi_stmt", 
  "select_from", "select_items_list", "select_item", "opt_alias", 
  "select_expr", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "where_item", "expr_ident", "const_int", "const_float", 
  "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "show_stmt", "show_what", "set_value", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "update", "update_items_list", 
  "update_item", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "set_transaction", 
  "opt_scope", "isolation_level", "create_function", "udf_type", 
  "drop_function", "attach_index", "flush_rtindex", "select_sysvar", 
  "sysvar_name", "truncate", 0
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
     335,   336,   337,   338,   339,   340,   341,   342,   343,   124,
      38,    61,   344,    60,    62,   345,   346,    43,    45,    42,
      47,    37,   347,   348,    59,    44,    40,    41,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   109,   110,   110,   110,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   112,   112,   113,   113,   114,   115,   115,
     116,   116,   117,   117,   117,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   119,   119,   120,   120,   121,
     122,   122,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   124,   124,   124,   124,   124,   124,   125,   125,   126,
     126,   127,   127,   128,   128,   129,   130,   130,   131,   132,
     132,   133,   133,   134,   134,   135,   135,   135,   136,   136,
     137,   137,   138,   138,   139,   140,   140,   141,   141,   141,
     141,   141,   142,   142,   143,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   145,   145,   145,   145,   145,   146,   146,   147,
     147,   148,   149,   149,   149,   150,   150,   150,   150,   150,
     151,   151,   151,   151,   151,   152,   152,   153,   153,   154,
     154,   154,   155,   155,   155,   156,   156,   157,   158,   158,
     159,   159,   160,   160,   161,   161,   162,   163,   163,   164,
     164,   164,   164,   164,   165,   165,   166,   167,   167,   168,
     168,   169,   169,   170,   170,   171,   171,   172,   173,   173,
     174,   174,   175,   176,   176,   177,   178,   179,   179,   180,
     180,   180,   180,   181,   182,   182,   183,   184,   184,   185,
     186,   187,   188,   188,   188,   189,   189,   189,   189,   190,
     191,   191,   192,   193,   194,   195,   196,   196,   197
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,    10,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     4,     3,     3,     5,     1,     3,     0,     1,     2,
       1,     3,     4,     3,     3,     5,     6,     3,     4,     5,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     3,
       3,     1,     1,     4,     3,     3,     1,     1,     2,     1,
       2,     1,     3,     0,     1,     3,     0,     1,     5,     0,
       1,     3,     5,     1,     3,     1,     2,     2,     0,     1,
       2,     4,     0,     1,     2,     1,     3,     3,     3,     5,
       6,     3,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     4,     4,     3,     6,     6,     1,     3,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     4,     4,     3,     4,     7,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     6,     1,     1,
       0,     3,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     3,     2,     7,     9,     6,     1,     3,     1,
       3,     1,     3,     0,     2,     1,     3,     3,     0,     1,
       1,     1,     2,     1,     1,     2,     5,     1,     3,     3,
       3,     5,     4,     4,     0,     1,     2,     1,     3,     3,
       2,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     3,     6,     3,     3,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   175,     0,   172,     0,     0,   214,   213,     0,
       0,   178,   179,   173,     0,   232,   232,     0,     0,     0,
       0,     2,     3,    23,    25,    26,     7,     8,     9,   174,
       5,     0,     6,    10,    11,     0,    12,    13,    14,    15,
      20,    16,    17,    18,    19,    21,    22,     0,     0,     0,
       0,     0,     0,   115,   116,   118,   119,   120,   246,     0,
       0,     0,     0,   117,     0,     0,     0,     0,     0,     0,
      30,     0,     0,     0,    28,    32,    35,   141,    98,     0,
       0,   233,     0,   234,     0,   230,   233,   154,   153,   215,
     152,   151,     0,   176,     0,    45,     0,     1,     4,     0,
     212,     0,     0,     0,     0,   242,   244,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     121,   122,     0,     0,     0,    33,     0,    31,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   245,    99,     0,     0,
       0,     0,   155,   158,   159,   157,   156,   163,     0,   224,
     248,     0,     0,     0,     0,    24,   180,     0,    77,    79,
     191,     0,     0,   189,   190,   199,   203,   197,     0,     0,
     150,   144,   149,     0,   147,   247,     0,     0,     0,    43,
       0,     0,     0,     0,     0,    42,     0,     0,   140,    47,
      29,    34,   132,   133,   139,   138,   130,   129,   136,   137,
     127,   128,   135,   134,   123,   124,   125,   126,   131,   100,
     167,   168,   170,   162,   169,     0,   171,   161,   160,   164,
       0,     0,     0,     0,   223,   225,     0,     0,   217,    46,
       0,     0,     0,    78,    80,   201,   193,    81,     0,     0,
       0,     0,   241,   240,     0,     0,     0,   142,    36,     0,
      41,    40,   143,     0,    37,     0,    38,    39,     0,     0,
       0,    83,    48,     0,   166,     0,     0,     0,   226,   227,
       0,     0,   216,    71,    72,     0,     0,    76,     0,   182,
       0,     0,   243,     0,   192,     0,   200,   199,   198,   204,
     205,   196,     0,     0,     0,   148,    44,     0,     0,     0,
      49,    50,     0,     0,    86,    84,   101,     0,     0,     0,
     238,   231,     0,     0,     0,   219,   220,   218,     0,     0,
       0,     0,   181,     0,   177,   184,    82,   202,   209,     0,
       0,   239,     0,   194,   146,   145,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    89,
      87,   165,   236,   235,   237,   229,   228,   222,     0,     0,
      74,    75,   183,     0,     0,   187,     0,   210,   211,   207,
     208,   206,     0,     0,    51,     0,     0,    57,     0,    53,
      64,    54,    65,    61,    67,    60,    66,    62,    69,    63,
      70,     0,    85,     0,     0,    98,    90,   221,    73,     0,
     186,   185,   195,    52,     0,     0,     0,    58,     0,     0,
       0,   102,   188,    59,     0,    68,    55,     0,     0,     0,
      95,    91,    93,     0,    27,   103,    56,    88,     0,    96,
      97,     0,     0,   104,   105,    92,    94,     0,     0,   107,
     108,   111,     0,   106,     0,     0,     0,   112,     0,     0,
       0,   109,   110,   114,   113
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    20,    21,    22,    23,    24,    73,    74,   127,    75,
      96,   271,   272,   310,   311,   430,   247,   174,   248,   314,
     315,   359,   360,   405,   406,   431,   432,   146,   147,   434,
     435,   443,   444,   456,   457,    76,    77,   183,   184,    25,
      91,   157,    26,    27,   227,   228,    28,    29,    30,    31,
     241,   290,   334,   335,   374,   175,    32,    33,   176,   177,
     249,   251,   299,   300,   339,   379,    34,    35,    36,    37,
     237,   238,    38,   234,   235,   278,   279,    39,    40,    84,
     321,    41,   254,    42,    43,    44,    45,    78,    46
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -266
static const short yypact[] =
{
     727,    82,  -266,    55,  -266,   116,   124,  -266,  -266,   141,
     121,  -266,  -266,  -266,   217,   263,   725,   113,   142,   187,
     207,  -266,   107,  -266,  -266,  -266,  -266,  -266,  -266,  -266,
    -266,   174,  -266,  -266,  -266,   221,  -266,  -266,  -266,  -266,
    -266,  -266,  -266,  -266,  -266,  -266,  -266,   222,   136,   238,
     187,   261,   267,   148,  -266,  -266,  -266,  -266,   164,   169,
     170,   171,   175,  -266,   176,   181,   182,   185,   194,   280,
    -266,   280,   280,    48,  -266,    32,   555,  -266,   245,   214,
     218,   138,   382,  -266,   234,  -266,  -266,  -266,  -266,  -266,
    -266,  -266,   229,  -266,   310,  -266,   -50,  -266,    80,   311,
    -266,   243,    -2,   264,   -48,  -266,  -266,   102,   322,   280,
      70,   220,   280,   131,   280,   280,   280,   226,   228,   230,
    -266,  -266,   355,   187,   256,  -266,   335,  -266,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   336,  -266,  -266,    89,   382,
     257,   258,  -266,  -266,  -266,  -266,  -266,  -266,   304,   266,
    -266,   349,   357,   256,   -30,  -266,   255,   313,  -266,  -266,
    -266,   162,    14,  -266,  -266,  -266,   274,  -266,    -3,   341,
    -266,  -266,   555,    -4,  -266,  -266,   380,   386,   284,  -266,
     406,    54,   306,   327,   429,  -266,   280,   280,  -266,   -46,
    -266,  -266,  -266,  -266,   275,   580,   605,   630,   639,   639,
     117,   117,   117,   117,   135,   135,  -266,  -266,  -266,   287,
    -266,  -266,  -266,  -266,  -266,   405,  -266,  -266,  -266,  -266,
      53,   323,   365,   427,  -266,  -266,   340,   -45,  -266,  -266,
     299,   358,   433,  -266,  -266,  -266,  -266,  -266,    58,    68,
      -2,   331,  -266,  -266,   370,   -29,   131,  -266,  -266,   334,
    -266,  -266,  -266,   280,  -266,   280,  -266,  -266,   505,   530,
     190,   420,  -266,   456,  -266,    19,    96,   369,   376,  -266,
      25,   349,  -266,  -266,  -266,   359,   360,  -266,   377,  -266,
      72,   379,  -266,    19,  -266,   478,  -266,   126,  -266,   383,
    -266,  -266,   481,   384,    19,  -266,  -266,   455,   480,   385,
     404,  -266,   251,   446,   424,  -266,  -266,    75,    16,   452,
    -266,  -266,   504,   427,    15,  -266,  -266,  -266,   415,   408,
     425,   299,  -266,    20,   426,  -266,  -266,  -266,  -266,    51,
      20,  -266,    19,  -266,  -266,  -266,   527,   190,     4,     9,
       4,     4,     4,     4,     4,     4,   496,   299,   501,   482,
    -266,  -266,  -266,  -266,  -266,  -266,  -266,  -266,    79,   434,
    -266,  -266,  -266,    18,    90,  -266,   379,  -266,  -266,  -266,
     528,  -266,    91,   450,  -266,   472,   473,  -266,    19,  -266,
    -266,  -266,  -266,  -266,  -266,  -266,  -266,  -266,  -266,  -266,
    -266,    24,  -266,   506,   546,   245,  -266,  -266,  -266,    20,
    -266,  -266,  -266,  -266,    19,    30,    98,  -266,    19,   547,
     240,   510,  -266,  -266,   576,  -266,  -266,   105,   299,   479,
      21,   483,  -266,   583,  -266,  -266,  -266,   483,   484,  -266,
    -266,   299,   498,   485,  -266,  -266,  -266,     8,   583,   507,
    -266,  -266,   604,  -266,   603,   521,   140,  -266,   508,    19,
     604,  -266,  -266,  -266,  -266
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -266,  -266,  -266,  -266,   516,  -266,  -266,   492,  -266,  -266,
     -23,  -266,   395,  -266,   289,  -238,  -102,  -265,  -261,  -266,
    -266,  -266,  -266,  -266,  -266,   209,   197,   235,  -266,  -266,
    -266,  -266,   191,  -266,   200,   -64,  -266,   544,   407,  -266,
    -266,   512,  -266,  -266,   411,  -266,  -266,  -266,  -266,  -266,
    -266,  -266,  -266,   286,  -266,  -249,  -266,  -266,  -266,   414,
    -266,  -266,  -266,   325,  -266,  -266,  -266,  -266,  -266,  -266,
    -266,   401,  -266,  -266,  -266,  -266,   343,  -266,  -266,   651,
    -266,  -266,  -266,  -266,  -266,  -266,  -266,  -266,  -266
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -209
static const short yytable[] =
{
     173,   297,   289,   168,   169,   120,   170,   121,   122,   168,
     169,   449,   303,   450,   317,   326,   451,   161,   387,   168,
     168,    87,   245,   168,   168,   168,   169,   104,   170,   252,
     168,   169,   312,   417,   439,   125,   169,   179,   362,   270,
     270,   253,    88,   182,   126,   186,   226,   440,   190,   182,
     192,   193,   194,    90,   377,   162,   220,   162,    48,   162,
     281,   221,   304,   368,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   382,   123,   386,   375,   390,   392,   394,   396,   398,
     400,   380,   220,   372,   168,   363,   171,   221,   187,   378,
     199,   256,   171,   257,   172,    53,    54,    55,    56,   312,
     180,    57,   225,   225,   452,   388,   225,   225,   171,   402,
     222,   246,   367,   171,    47,   246,   373,   416,   424,  -208,
     418,   324,   268,   269,    53,    54,    55,    56,   338,   180,
      57,   150,    63,    64,   223,   163,   128,   151,   173,   164,
     425,    49,   118,   124,   119,   318,   319,   427,    50,   256,
     422,   262,   320,   293,   128,   294,   224,   243,   244,   188,
     129,    63,    64,   295,  -208,   296,    51,   331,   325,   332,
     293,   118,   361,   119,   293,    52,   407,   225,   129,    93,
      95,   336,   182,   283,   284,   409,   293,   410,   412,   307,
      69,   308,   343,   293,    71,   426,    94,    97,    72,   181,
     293,    98,   436,   285,   140,   141,   142,   143,   144,    99,
      53,    54,    55,    56,   100,   101,    57,    58,   286,    69,
     287,   173,    59,    71,   142,   143,   144,    72,   173,   309,
      60,   103,   102,   283,   284,   460,   385,   461,   389,   391,
     393,   395,   397,   399,   107,    61,    62,    63,    64,    53,
      54,    55,    56,   285,   105,    57,    79,    65,   348,    66,
     106,    59,   108,    80,   288,   109,   110,   111,   286,    60,
     287,   112,   113,    53,    54,    55,    56,   114,   115,    57,
      67,   116,   349,   145,    61,    62,    63,    64,   429,    81,
     117,    68,   283,   284,   128,   148,    65,   173,    66,   149,
     158,   159,   423,   160,   166,    69,    70,    82,   167,    71,
      63,    64,   285,    72,   288,   185,   178,   189,   129,    67,
     118,    83,   119,   195,   196,   128,   197,   286,   201,   287,
      68,   219,   350,   351,   352,   353,   354,   355,   230,   231,
     232,   233,   236,   356,    69,    70,   128,   463,    71,   129,
     239,   240,    72,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   242,    69,   250,
     129,   255,    71,   288,   128,   152,    72,   153,   154,   259,
     155,   260,   273,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   129,   128,
     243,   263,   276,   264,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   275,
     277,   280,   265,   129,   266,   128,   292,   156,   301,   291,
     302,   306,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   313,   128,   129,
     322,   316,   198,   323,   357,   328,   329,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   129,   330,   128,   333,   337,   258,   340,   341,
     342,   346,   347,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   129,   128,
     358,   364,   365,   261,   369,   370,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   376,   371,   129,   128,   383,   267,   401,   403,   404,
     338,   408,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   413,   129,   128,
     414,   415,   344,   419,   420,   428,   433,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   244,   129,   128,   438,   442,   345,   441,   447,
     448,   445,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   455,   129,   128,
     263,   458,   459,   454,   165,   462,   200,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   282,   129,   128,   265,   384,   437,   446,   453,
     421,   274,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   191,   129,   128,
     464,   229,   411,   305,   298,   381,   366,    92,   128,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   327,   129,     0,     0,     0,     0,     0,     0,
       0,     0,   129,     0,     0,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   136,   137,   138,   139,   140,   141,   142,   143,
     144,     1,     0,     2,     0,    85,     3,     0,     4,     0,
       0,     5,     6,     7,     8,     0,     0,     9,     0,     0,
      10,    86,     0,     0,     0,     0,     0,     0,     0,     0,
      11,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
      13,     0,    14,    83,    15,     0,    16,    88,    17,    89,
       0,     0,     0,     0,     0,    18,     0,    19,    90
};

static const short yycheck[] =
{
     102,   250,   240,     5,     6,    69,     8,    71,    72,     5,
       6,     3,    41,     5,   275,   280,     8,    67,     9,     5,
       5,    51,     8,     5,     5,     5,     6,    50,     8,    32,
       5,     6,   270,     9,    13,     3,     6,    85,    22,    85,
      85,    44,    72,   107,    12,   109,   148,    26,   112,   113,
     114,   115,   116,    83,     3,   105,     3,   105,     3,   105,
     105,     8,    91,   324,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   342,    34,   348,   333,   350,   351,   352,   353,   354,
     355,   340,     3,   331,     5,    79,    98,     8,    28,    48,
     123,   105,    98,   107,   106,     3,     4,     5,     6,   347,
       8,     9,    98,    98,   106,   106,    98,    98,    98,   357,
      31,   107,   107,    98,    42,   107,   106,   388,    98,     3,
     106,   106,   196,   197,     3,     4,     5,     6,    12,     8,
       9,     3,    40,    41,    55,    65,    29,     9,   250,    69,
     415,    35,    50,   105,    52,    59,    60,   418,    34,   105,
     409,   107,    66,   105,    29,   107,    77,     5,     6,    99,
      53,    40,    41,   105,    48,   107,    35,   105,   280,   107,
     105,    50,   107,    52,   105,    64,   107,    98,    53,    76,
       3,   293,   256,     3,     4,   105,   105,   107,   107,   263,
      98,   265,   304,   105,   102,   107,    64,     0,   106,   107,
     105,   104,   107,    23,    97,    98,    99,   100,   101,    45,
       3,     4,     5,     6,     3,     3,     9,    10,    38,    98,
      40,   333,    15,   102,    99,   100,   101,   106,   340,    49,
      23,     3,   106,     3,     4,   105,   348,   107,   350,   351,
     352,   353,   354,   355,   106,    38,    39,    40,    41,     3,
       4,     5,     6,    23,     3,     9,     3,    50,    17,    52,
       3,    15,   108,    10,    84,   106,   106,   106,    38,    23,
      40,   106,   106,     3,     4,     5,     6,   106,   106,     9,
      73,   106,    41,    48,    38,    39,    40,    41,    58,    36,
     106,    84,     3,     4,    29,    91,    50,   409,    52,    91,
      76,    82,   414,     3,     3,    98,    99,    54,    75,   102,
      40,    41,    23,   106,    84,     3,    62,   107,    53,    73,
      50,    68,    52,   107,   106,    29,   106,    38,     3,    40,
      84,     5,    91,    92,    93,    94,    95,    96,    91,    91,
      46,    85,     3,   102,    98,    99,    29,   459,   102,    53,
       3,   106,   106,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,    64,    98,   105,
      53,    40,   102,    84,    29,     3,   106,     5,     6,     3,
       8,   107,   105,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,    53,    29,
       5,   105,    47,   107,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   106,
       3,    91,   105,    53,   107,    29,     3,    55,   107,    81,
      70,   107,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,    37,    29,    53,
      91,     5,   107,    87,    18,   106,   106,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,    53,   106,    29,   106,     8,   107,   105,     8,
     106,   106,    88,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,    53,    29,
      86,    59,     8,   107,    99,   107,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   105,   107,    53,    29,     8,   107,    41,    37,    57,
      12,   107,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   107,    53,    29,
      88,    88,   107,    57,    18,    18,    56,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,     6,    53,    29,   106,     3,   107,   105,    91,
     105,   107,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,     3,    53,    29,
     105,     8,    91,   106,    98,   107,   124,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   237,    53,    29,   105,   347,   428,   441,   448,
     405,   230,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   113,    53,    29,
     460,   149,   376,   256,   250,   340,   323,    16,    29,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   281,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,    93,    94,    95,    96,    97,    98,    99,   100,
     101,    14,    -1,    16,    -1,    20,    19,    -1,    21,    -1,
      -1,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      33,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,
      63,    -1,    65,    68,    67,    -1,    69,    72,    71,    74,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    80,    83
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    16,    19,    21,    24,    25,    26,    27,    30,
      33,    43,    61,    63,    65,    67,    69,    71,    78,    80,
     110,   111,   112,   113,   114,   148,   151,   152,   155,   156,
     157,   158,   165,   166,   175,   176,   177,   178,   181,   186,
     187,   190,   192,   193,   194,   195,   197,    42,     3,    35,
      34,    35,    64,     3,     4,     5,     6,     9,    10,    15,
      23,    38,    39,    40,    41,    50,    52,    73,    84,    98,
      99,   102,   106,   115,   116,   118,   144,   145,   196,     3,
      10,    36,    54,    68,   188,    20,    36,    51,    72,    74,
      83,   149,   188,    76,    64,     3,   119,     0,   104,    45,
       3,     3,   106,     3,   119,     3,     3,   106,   108,   106,
     106,   106,   106,   106,   106,   106,   106,   106,    50,    52,
     144,   144,   144,    34,   105,     3,    12,   117,    29,    53,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,    48,   136,   137,    91,    91,
       3,     9,     3,     5,     6,     8,    55,   150,    76,    82,
       3,    67,   105,    65,    69,   113,     3,    75,     5,     6,
       8,    98,   106,   125,   126,   164,   167,   168,    62,    85,
       8,   107,   144,   146,   147,     3,   144,    28,    99,   107,
     144,   146,   144,   144,   144,   107,   106,   106,   107,   119,
     116,     3,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,     5,
       3,     8,    31,    55,    77,    98,   125,   153,   154,   150,
      91,    91,    46,    85,   182,   183,     3,   179,   180,     3,
     106,   159,    64,     5,     6,     8,   107,   125,   127,   169,
     105,   170,    32,    44,   191,    40,   105,   107,   107,     3,
     107,   107,   107,   105,   107,   105,   107,   107,   144,   144,
      85,   120,   121,   105,   153,   106,    47,     3,   184,   185,
      91,   105,   121,     3,     4,    23,    38,    40,    84,   124,
     160,    81,     3,   105,   107,   105,   107,   164,   168,   171,
     172,   107,    70,    41,    91,   147,   107,   144,   144,    49,
     122,   123,   124,    37,   128,   129,     5,   127,    59,    60,
      66,   189,    91,    87,   106,   125,   126,   180,   106,   106,
     106,   105,   107,   106,   161,   162,   125,     8,    12,   173,
     105,     8,   106,   125,   107,   107,   106,    88,    17,    41,
      91,    92,    93,    94,    95,    96,   102,    18,    86,   130,
     131,   107,    22,    79,    59,     8,   185,   107,   127,    99,
     107,   107,   124,   106,   163,   164,   105,     3,    48,   174,
     164,   172,   127,     8,   123,   125,   126,     9,   106,   125,
     126,   125,   126,   125,   126,   125,   126,   125,   126,   125,
     126,    41,   124,    37,    57,   132,   133,   107,   107,   105,
     107,   162,   107,   107,    88,    88,   127,     9,   106,    57,
      18,   136,   164,   125,    98,   126,   107,   127,    18,    58,
     124,   134,   135,    56,   138,   139,   107,   134,   106,    13,
      26,   105,     3,   140,   141,   107,   135,    91,   105,     3,
       5,     8,   106,   141,   106,     3,   142,   143,     8,    91,
     105,   107,   107,   125,   143
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

  case 23:

    { pParser->PushQuery(); ;}
    break;

  case 24:

    { pParser->PushQuery(); ;}
    break;

  case 27:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 30:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 33:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 34:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 36:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 37:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 38:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 39:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 40:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 41:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 42:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 43:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 44:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 46:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 52:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 53:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 54:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 55:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 56:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 57:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 58:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 59:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 60:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 67:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 73:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 74:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 76:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 77:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 78:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 79:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 80:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 81:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 82:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 85:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 88:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 91:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 92:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 94:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 96:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 97:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 100:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 101:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 107:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 108:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 109:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 110:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 111:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 112:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 113:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 114:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 116:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 117:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 121:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 122:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 123:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 142:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 143:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 144:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 145:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 146:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 152:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 153:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 154:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 160:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 161:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 162:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 163:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 164:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 165:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 166:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 169:

    { yyval.m_iValue = 1; ;}
    break;

  case 170:

    { yyval.m_iValue = 0; ;}
    break;

  case 171:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 177:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 178:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 179:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 182:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 183:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 186:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 187:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 188:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 189:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 190:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 191:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 192:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 193:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 194:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 195:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 197:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 198:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 200:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 201:

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

  case 202:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 205:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 207:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 211:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 212:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 215:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 216:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 219:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 220:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 221:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 222:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 223:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 230:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 231:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 239:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 240:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 241:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 242:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 243:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 244:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 245:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 247:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 248:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
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

