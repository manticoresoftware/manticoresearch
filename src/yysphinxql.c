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
#define YYFINAL  106
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   914

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  98
/* YYNRULES -- Number of rules. */
#define YYNRULES  274
/* YYNRULES -- Number of states. */
#define YYNSTATES  524

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
     110,   111,   104,   102,   112,   103,   115,   105,     2,     2,
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
      62,    74,    75,    76,    79,    84,    95,    97,   101,   103,
     106,   107,   109,   112,   114,   119,   124,   129,   134,   139,
     144,   148,   152,   158,   160,   164,   165,   167,   170,   172,
     176,   177,   181,   185,   190,   194,   198,   204,   211,   215,
     220,   226,   230,   234,   238,   242,   244,   250,   254,   258,
     262,   266,   270,   274,   276,   278,   283,   287,   291,   293,
     295,   298,   300,   303,   305,   309,   310,   312,   316,   317,
     319,   325,   326,   328,   332,   338,   340,   344,   346,   349,
     352,   353,   355,   358,   363,   364,   366,   369,   371,   375,
     379,   383,   389,   396,   400,   402,   406,   410,   412,   414,
     416,   418,   420,   422,   425,   428,   432,   436,   440,   444,
     448,   452,   456,   460,   464,   468,   472,   476,   480,   484,
     488,   492,   496,   500,   504,   506,   511,   516,   520,   527,
     534,   536,   540,   542,   544,   548,   554,   557,   558,   561,
     563,   566,   569,   573,   578,   583,   587,   589,   591,   593,
     595,   597,   599,   603,   608,   613,   618,   622,   627,   635,
     641,   643,   645,   647,   649,   651,   653,   655,   657,   659,
     662,   669,   671,   673,   674,   678,   680,   684,   686,   690,
     694,   696,   700,   702,   704,   706,   710,   713,   721,   731,
     738,   740,   744,   746,   750,   752,   756,   757,   760,   762,
     766,   770,   771,   773,   775,   777,   781,   783,   785,   789,
     796,   798,   802,   806,   810,   816,   821,   826,   827,   829,
     832,   834,   838,   842,   845,   849,   856,   857,   859,   861,
     864,   867,   870,   872,   880,   882,   884,   886,   890,   897,
     901,   905,   907,   911,   915
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     117,     0,    -1,   118,    -1,   119,    -1,   119,   109,    -1,
     171,    -1,   179,    -1,   165,    -1,   166,    -1,   169,    -1,
     180,    -1,   189,    -1,   191,    -1,   192,    -1,   195,    -1,
     200,    -1,   201,    -1,   205,    -1,   207,    -1,   208,    -1,
     209,    -1,   202,    -1,   210,    -1,   212,    -1,   213,    -1,
     120,    -1,   119,   109,   120,    -1,   121,    -1,   160,    -1,
     124,    -1,    69,   125,    36,   110,   122,   124,   111,    60,
      19,   145,   123,    -1,    -1,    -1,    51,     5,    -1,    51,
       5,   112,     5,    -1,    69,   125,    36,   129,   130,   139,
     141,   143,   147,   149,    -1,   126,    -1,   125,   112,   126,
      -1,   104,    -1,   128,   127,    -1,    -1,     3,    -1,    13,
       3,    -1,   155,    -1,    16,   110,   155,   111,    -1,    53,
     110,   155,   111,    -1,    55,   110,   155,   111,    -1,    78,
     110,   155,   111,    -1,    41,   110,   155,   111,    -1,    25,
     110,   104,   111,    -1,    89,   110,   111,    -1,    40,   110,
     111,    -1,    25,   110,    30,     3,   111,    -1,     3,    -1,
     129,   112,     3,    -1,    -1,   131,    -1,    90,   132,    -1,
     134,    -1,   132,    93,   134,    -1,    -1,   135,    96,   137,
      -1,   135,    97,   137,    -1,    52,   110,     8,   111,    -1,
     135,    96,   136,    -1,   135,    97,   136,    -1,   135,    43,
     110,   138,   111,    -1,   135,   107,    43,   110,   138,   111,
      -1,   135,    43,     9,    -1,   135,   107,    43,     9,    -1,
     135,    18,   136,    93,   136,    -1,   135,    99,   136,    -1,
     135,    98,   136,    -1,   135,   100,   136,    -1,   135,   101,
     136,    -1,   133,    -1,   135,    18,   137,    93,   137,    -1,
     135,    99,   137,    -1,   135,    98,   137,    -1,   135,   100,
     137,    -1,   135,   101,   137,    -1,   135,    96,     8,    -1,
     135,    97,     8,    -1,     3,    -1,     4,    -1,    25,   110,
     104,   111,    -1,    40,   110,   111,    -1,    89,   110,   111,
      -1,    42,    -1,     5,    -1,   103,     5,    -1,     6,    -1,
     103,     6,    -1,   136,    -1,   138,   112,   136,    -1,    -1,
     140,    -1,    39,    19,   135,    -1,    -1,   142,    -1,    91,
      39,    60,    19,   145,    -1,    -1,   144,    -1,    60,    19,
     145,    -1,    60,    19,    62,   110,   111,    -1,   146,    -1,
     145,   112,   146,    -1,   135,    -1,   135,    14,    -1,   135,
      28,    -1,    -1,   148,    -1,    51,     5,    -1,    51,     5,
     112,     5,    -1,    -1,   150,    -1,    59,   151,    -1,   152,
      -1,   151,   112,   152,    -1,     3,    96,     3,    -1,     3,
      96,     5,    -1,     3,    96,   110,   153,   111,    -1,     3,
      96,     3,   110,     8,   111,    -1,     3,    96,     8,    -1,
     154,    -1,   153,   112,   154,    -1,     3,    96,   136,    -1,
       3,    -1,     4,    -1,    42,    -1,     5,    -1,     6,    -1,
       9,    -1,   103,   155,    -1,   107,   155,    -1,   155,   102,
     155,    -1,   155,   103,   155,    -1,   155,   104,   155,    -1,
     155,   105,   155,    -1,   155,    98,   155,    -1,   155,    99,
     155,    -1,   155,    95,   155,    -1,   155,    94,   155,    -1,
     155,   106,   155,    -1,   155,    31,   155,    -1,   155,    56,
     155,    -1,   155,   101,   155,    -1,   155,   100,   155,    -1,
     155,    96,   155,    -1,   155,    97,   155,    -1,   155,    93,
     155,    -1,   155,    92,   155,    -1,   110,   155,   111,    -1,
     113,   159,   114,    -1,   156,    -1,     3,   110,   157,   111,
      -1,    43,   110,   157,   111,    -1,     3,   110,   111,    -1,
      55,   110,   155,   112,   155,   111,    -1,    53,   110,   155,
     112,   155,   111,    -1,   158,    -1,   157,   112,   158,    -1,
     155,    -1,     8,    -1,     3,    96,   136,    -1,   159,   112,
       3,    96,   136,    -1,    73,   162,    -1,    -1,    50,     8,
      -1,    88,    -1,    76,   161,    -1,    54,   161,    -1,    12,
      76,   161,    -1,    12,     8,    76,   161,    -1,    12,     3,
      76,   161,    -1,    44,     3,    76,    -1,     3,    -1,    58,
      -1,     8,    -1,     5,    -1,     6,    -1,   163,    -1,   164,
     103,   163,    -1,    71,     3,    96,   168,    -1,    71,     3,
      96,   167,    -1,    71,     3,    96,    58,    -1,    71,    57,
     164,    -1,    71,    10,    96,   164,    -1,    71,    38,     9,
      96,   110,   138,   111,    -1,    71,    38,     3,    96,   167,
      -1,     3,    -1,     8,    -1,    82,    -1,    33,    -1,   136,
      -1,    23,    -1,    67,    -1,   170,    -1,    17,    -1,    75,
      81,    -1,   172,    47,     3,   173,    86,   175,    -1,    45,
      -1,    65,    -1,    -1,   110,   174,   111,    -1,   135,    -1,
     174,   112,   135,    -1,   176,    -1,   175,   112,   176,    -1,
     110,   177,   111,    -1,   178,    -1,   177,   112,   178,    -1,
     136,    -1,   137,    -1,     8,    -1,   110,   138,   111,    -1,
     110,   111,    -1,    27,    36,   129,    90,    42,    96,   136,
      -1,    27,    36,   129,    90,    42,    43,   110,   138,   111,
      -1,    20,     3,   110,   181,   184,   111,    -1,   182,    -1,
     181,   112,   182,    -1,   178,    -1,   110,   183,   111,    -1,
       8,    -1,   183,   112,     8,    -1,    -1,   112,   185,    -1,
     186,    -1,   185,   112,   186,    -1,   178,   187,   188,    -1,
      -1,    13,    -1,     3,    -1,    51,    -1,   190,     3,   161,
      -1,    29,    -1,    28,    -1,    73,    79,   161,    -1,    85,
     129,    71,   193,   131,   149,    -1,   194,    -1,   193,   112,
     194,    -1,     3,    96,   136,    -1,     3,    96,   137,    -1,
       3,    96,   110,   138,   111,    -1,     3,    96,   110,   111,
      -1,    73,   203,    87,   196,    -1,    -1,   197,    -1,    90,
     198,    -1,   199,    -1,   198,    92,   199,    -1,     3,    96,
       8,    -1,    73,    22,    -1,    73,    21,    71,    -1,    71,
     203,    81,    48,    49,   204,    -1,    -1,    38,    -1,    72,
      -1,    63,    84,    -1,    63,    24,    -1,    64,    63,    -1,
      70,    -1,    26,    37,     3,    66,   206,    74,     8,    -1,
      46,    -1,    34,    -1,    77,    -1,    32,    37,     3,    -1,
      15,    44,     3,    80,    68,     3,    -1,    35,    68,     3,
      -1,    69,   211,   147,    -1,    10,    -1,    10,   115,     3,
      -1,    83,    68,     3,    -1,    61,    44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   127,   127,   128,   129,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   158,   159,   163,   164,   168,
     169,   179,   190,   191,   195,   203,   219,   220,   224,   225,
     228,   230,   231,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   248,   249,   252,   254,   258,   262,   263,
     266,   267,   268,   272,   277,   284,   292,   300,   309,   314,
     319,   324,   329,   334,   339,   344,   349,   354,   359,   364,
     369,   374,   379,   387,   388,   393,   399,   405,   411,   420,
     421,   432,   433,   437,   443,   449,   451,   455,   461,   463,
     467,   473,   475,   479,   483,   490,   491,   495,   496,   497,
     500,   502,   506,   511,   518,   520,   524,   528,   529,   533,
     538,   543,   549,   554,   562,   567,   574,   584,   585,   586,
     587,   588,   589,   590,   591,   592,   593,   594,   595,   596,
     597,   598,   599,   600,   601,   602,   603,   604,   605,   606,
     607,   608,   609,   610,   611,   615,   616,   617,   618,   619,
     623,   624,   628,   629,   633,   634,   640,   643,   645,   649,
     650,   651,   652,   653,   658,   663,   673,   674,   675,   676,
     677,   681,   682,   686,   691,   696,   701,   702,   706,   711,
     719,   720,   724,   725,   726,   740,   741,   742,   746,   747,
     753,   761,   762,   765,   767,   771,   772,   776,   777,   781,
     785,   786,   790,   791,   792,   793,   794,   800,   808,   822,
     830,   834,   841,   842,   849,   859,   865,   867,   871,   876,
     880,   887,   889,   893,   894,   900,   908,   909,   915,   921,
     929,   930,   934,   938,   942,   946,   956,   962,   963,   967,
     971,   972,   976,   980,   987,   994,  1000,  1001,  1002,  1006,
    1007,  1008,  1009,  1015,  1026,  1027,  1028,  1032,  1043,  1055,
    1066,  1074,  1075,  1084,  1095
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
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", 
  "'}'", "'.'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select", "subselect_start", "opt_outer_limit", 
  "select_from", "select_items_list", "select_item", "opt_alias", 
  "select_expr", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "expr_float_unhandled", "where_item", "expr_ident", 
  "const_int", "const_float", "const_list", "opt_group_clause", 
  "group_clause", "opt_group_order_clause", "group_order_clause", 
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
      40,    41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   116,   117,   117,   117,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   119,   119,   120,   120,   121,
     121,   122,   123,   123,   123,   124,   125,   125,   126,   126,
     127,   127,   127,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   129,   129,   130,   130,   131,   132,   132,
     133,   133,   133,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   135,   135,   135,   135,   135,   135,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   141,   141,
     142,   143,   143,   144,   144,   145,   145,   146,   146,   146,
     147,   147,   148,   148,   149,   149,   150,   151,   151,   152,
     152,   152,   152,   152,   153,   153,   154,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   156,   156,   156,   156,   156,
     157,   157,   158,   158,   159,   159,   160,   161,   161,   162,
     162,   162,   162,   162,   162,   162,   163,   163,   163,   163,
     163,   164,   164,   165,   165,   165,   165,   165,   166,   166,
     167,   167,   168,   168,   168,   169,   169,   169,   170,   170,
     171,   172,   172,   173,   173,   174,   174,   175,   175,   176,
     177,   177,   178,   178,   178,   178,   178,   179,   179,   180,
     181,   181,   182,   182,   183,   183,   184,   184,   185,   185,
     186,   187,   187,   188,   188,   189,   190,   190,   191,   192,
     193,   193,   194,   194,   194,   194,   195,   196,   196,   197,
     198,   198,   199,   200,   201,   202,   203,   203,   203,   204,
     204,   204,   204,   205,   206,   206,   206,   207,   208,   209,
     210,   211,   211,   212,   213
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
      11,     0,     0,     2,     4,    10,     1,     3,     1,     2,
       0,     1,     2,     1,     4,     4,     4,     4,     4,     4,
       3,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       0,     3,     3,     4,     3,     3,     5,     6,     3,     4,
       5,     3,     3,     3,     3,     1,     5,     3,     3,     3,
       3,     3,     3,     1,     1,     4,     3,     3,     1,     1,
       2,     1,     2,     1,     3,     0,     1,     3,     0,     1,
       5,     0,     1,     3,     5,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     6,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     4,     4,     3,     6,     6,
       1,     3,     1,     1,     3,     5,     2,     0,     2,     1,
       2,     2,     3,     4,     4,     3,     1,     1,     1,     1,
       1,     1,     3,     4,     4,     4,     3,     4,     7,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       6,     1,     1,     0,     3,     1,     3,     1,     3,     3,
       1,     3,     1,     1,     1,     3,     2,     7,     9,     6,
       1,     3,     1,     3,     1,     3,     0,     2,     1,     3,
       3,     0,     1,     1,     1,     3,     1,     1,     3,     6,
       1,     3,     3,     3,     5,     4,     4,     0,     1,     2,
       1,     3,     3,     2,     3,     6,     0,     1,     1,     2,
       2,     2,     1,     7,     1,     1,     1,     3,     6,     3,
       3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   198,     0,   195,     0,     0,   237,   236,     0,
       0,   201,     0,   202,   196,     0,   256,   256,     0,     0,
       0,     0,     2,     3,    25,    27,    29,    28,     7,     8,
       9,   197,     5,     0,     6,    10,    11,     0,    12,    13,
      14,    15,    16,    21,    17,    18,    19,    20,    22,    23,
      24,     0,     0,     0,     0,     0,     0,     0,   127,   128,
     130,   131,   132,   271,     0,     0,     0,     0,   129,     0,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
      36,    40,    43,   154,   110,     0,     0,   257,     0,   258,
       0,     0,     0,   253,   257,     0,   167,   167,   167,   169,
     166,     0,   199,     0,    53,     0,     1,     4,     0,   167,
       0,     0,     0,     0,   267,   269,   274,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     133,   134,     0,     0,     0,     0,     0,    41,     0,    39,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   111,
       0,     0,     0,     0,   176,   179,   180,   178,   177,   181,
     186,     0,     0,     0,   167,   254,     0,     0,   171,   170,
     238,   247,   273,     0,     0,     0,     0,    26,   203,   235,
       0,    89,    91,   214,     0,     0,   212,   213,   222,   226,
     220,     0,     0,   163,   157,   162,     0,   160,   272,     0,
       0,     0,    51,     0,     0,     0,     0,     0,    50,     0,
       0,   152,     0,     0,   153,    31,    55,    37,    42,   144,
     145,   151,   150,   142,   141,   148,   149,   139,   140,   147,
     146,   135,   136,   137,   138,   143,   112,   190,   191,   193,
     185,   192,     0,   194,   184,   183,   187,     0,     0,     0,
       0,   167,   167,   172,   175,   168,     0,   246,   248,     0,
       0,   240,    54,     0,     0,     0,    90,    92,   224,   216,
      93,     0,     0,     0,     0,   265,   264,   266,     0,     0,
     155,     0,    44,     0,    49,    48,   156,    45,     0,    46,
       0,    47,     0,     0,   164,     0,     0,    60,    95,    56,
       0,   189,     0,   182,     0,   174,   173,     0,   249,   250,
       0,     0,   114,    83,    84,     0,     0,    88,     0,   205,
       0,     0,   268,   215,     0,   223,     0,   222,   221,   227,
     228,   219,     0,     0,     0,   161,    52,     0,     0,     0,
       0,     0,     0,    57,    75,    58,     0,     0,    98,    96,
     113,     0,     0,     0,   262,   255,     0,     0,     0,   242,
     243,   241,     0,   239,   115,     0,     0,     0,   204,     0,
       0,   200,   207,    94,   225,   232,     0,     0,   263,     0,
     217,   159,   158,   165,     0,     0,     0,    60,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   101,
      99,   188,   260,   259,   261,   252,   251,   245,     0,     0,
     116,   117,     0,    86,    87,   206,     0,     0,   210,     0,
     233,   234,   230,   231,   229,     0,     0,     0,     0,    59,
       0,     0,    68,     0,    81,    64,    61,    82,    65,    62,
      72,    78,    71,    77,    73,    79,    74,    80,     0,    97,
       0,     0,   110,   102,   244,     0,     0,    85,   209,     0,
     208,   218,     0,    63,     0,     0,     0,    69,     0,     0,
       0,   114,   119,   120,   123,     0,   118,   211,   107,    32,
     105,    70,     0,    76,    66,     0,     0,     0,   103,    35,
       0,     0,     0,   124,   108,   109,     0,     0,    30,    67,
     100,     0,     0,     0,   121,     0,    33,   106,   104,   122,
     126,   125,     0,    34
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   306,   508,    26,    79,
      80,   139,    81,   226,   308,   309,   353,   354,   355,   488,
     280,   197,   281,   358,   359,   409,   410,   462,   463,   489,
     490,   158,   159,   373,   374,   420,   421,   502,   503,    82,
      83,   206,   207,   134,    27,   178,   100,   169,   170,    28,
      29,   254,   255,    30,    31,    32,    33,   274,   330,   381,
     382,   427,   198,    34,    35,   199,   200,   282,   284,   339,
     340,   386,   432,    36,    37,    38,    39,   270,   271,    40,
     267,   268,   318,   319,    41,    42,    43,    90,   365,    44,
     288,    45,    46,    47,    48,    84,    49,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -343
static const short yypact[] =
{
     829,    -4,  -343,    45,  -343,    84,    81,  -343,  -343,    88,
      72,  -343,    98,  -343,  -343,   140,    96,   645,    77,    99,
     168,   186,  -343,    87,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,   164,  -343,  -343,  -343,   211,  -343,  -343,
    -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,   214,   115,   224,   168,   229,   246,   248,   146,  -343,
    -343,  -343,  -343,    91,   150,   165,   185,   194,  -343,   198,
     200,   206,   209,   217,   317,  -343,   317,   317,   265,   -16,
    -343,    52,   608,  -343,   245,   255,   256,    57,   475,  -343,
     250,    54,   282,  -343,  -343,   351,   305,   305,   305,  -343,
    -343,   269,  -343,   359,  -343,   -15,  -343,    -6,   364,   305,
     286,    16,   308,    80,  -343,  -343,  -343,   258,   372,   317,
       6,   266,   317,   275,   317,   317,   317,   272,   270,   274,
    -343,  -343,   446,   280,   -12,     3,   199,  -343,   383,  -343,
     317,   317,   317,   317,   317,   317,   317,   317,   317,   317,
     317,   317,   317,   317,   317,   317,   317,   382,  -343,  -343,
     152,   475,   293,   294,  -343,  -343,  -343,  -343,  -343,  -343,
     288,   344,   319,   320,   305,  -343,   321,   385,  -343,  -343,
    -343,   309,  -343,   397,   398,   199,   253,  -343,   298,  -343,
     341,  -343,  -343,  -343,   173,    11,  -343,  -343,  -343,   299,
    -343,   143,   368,  -343,  -343,   608,   119,  -343,  -343,   472,
     409,   302,  -343,   498,   124,   342,   363,   524,  -343,   317,
     317,  -343,     9,   411,  -343,  -343,   101,  -343,  -343,  -343,
    -343,   659,   685,   711,   737,   720,   720,   301,   301,   301,
     301,   166,   166,  -343,  -343,  -343,   303,  -343,  -343,  -343,
    -343,  -343,   412,  -343,  -343,  -343,   288,   158,   306,   475,
     369,   305,   305,  -343,  -343,  -343,   418,  -343,  -343,   326,
     104,  -343,  -343,   339,   337,   423,  -343,  -343,  -343,  -343,
    -343,   134,   147,    16,   318,  -343,  -343,  -343,   357,   -10,
    -343,   275,  -343,   322,  -343,  -343,  -343,  -343,   317,  -343,
     317,  -343,   394,   420,  -343,   336,   380,   148,   413,  -343,
     465,  -343,     9,  -343,   235,  -343,  -343,   375,   381,  -343,
      25,   397,   425,  -343,  -343,   362,   391,  -343,   395,  -343,
     162,   399,  -343,  -343,     9,  -343,   471,   156,  -343,   370,
    -343,  -343,   477,   400,     9,  -343,  -343,   550,   576,     9,
     199,   393,   401,   414,  -343,  -343,   239,   489,   436,  -343,
    -343,   174,    34,   467,  -343,  -343,   523,   418,     5,  -343,
    -343,  -343,   531,  -343,  -343,   431,   426,   442,  -343,   339,
      21,   424,  -343,  -343,  -343,  -343,    56,    21,  -343,     9,
    -343,  -343,  -343,  -343,   -11,   496,   551,   148,    29,    19,
      33,    38,    29,    29,    29,    29,   515,   339,   521,   501,
    -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,   203,   466,
     451,  -343,   468,  -343,  -343,  -343,    12,   213,  -343,   399,
    -343,  -343,  -343,   569,  -343,   222,   168,   565,   474,  -343,
     493,   494,  -343,     9,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,    23,  -343,
     528,   570,   245,  -343,  -343,    10,   531,  -343,  -343,    21,
    -343,  -343,   339,  -343,     9,     1,   233,  -343,     9,   586,
     159,   425,   500,  -343,  -343,   605,  -343,  -343,   184,    -9,
    -343,  -343,   606,  -343,  -343,   236,   339,   503,   499,  -343,
     607,   518,   238,  -343,  -343,  -343,   626,   339,  -343,  -343,
     499,   522,   525,     9,  -343,   605,   526,  -343,  -343,  -343,
    -343,  -343,   629,  -343
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -343,  -343,  -343,  -343,   530,  -343,  -343,  -343,   334,   291,
     527,  -343,  -343,    41,  -343,   388,  -343,  -343,   262,  -270,
    -111,  -311,  -304,  -343,  -343,  -343,  -343,  -343,  -343,  -342,
     153,   223,  -343,   181,  -343,  -343,   218,  -343,   171,   -72,
    -343,   542,   402,  -343,  -343,   -86,  -343,   429,   533,  -343,
    -343,   434,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,
     263,  -343,  -282,  -343,  -343,  -343,   415,  -343,  -343,  -343,
     310,  -343,  -343,  -343,  -343,  -343,  -343,  -343,   374,  -343,
    -343,  -343,  -343,   329,  -343,  -343,  -343,   701,  -343,  -343,
    -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -232
static const short yytable[] =
{
     196,   337,   130,   329,   131,   132,   104,   192,   361,   370,
     191,   179,   180,   482,   191,   483,   191,   191,   484,   278,
     135,   191,   192,   189,   193,   436,   191,   192,   442,   193,
     191,   192,   477,   343,   191,   192,   210,   356,   191,   192,
      51,   444,   506,   191,   192,   205,   447,   209,    52,   253,
     213,   205,   215,   216,   217,   137,   183,   172,   412,   430,
     162,   105,   173,   185,   418,   138,   163,   186,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   435,   344,   441,   263,   446,
     449,   451,   453,   455,   457,   113,   136,   184,   428,    85,
     223,   136,   224,   507,   492,   433,    86,   431,   252,   425,
     211,   304,   252,   225,   252,   252,   417,    54,   413,   194,
     485,    53,   279,   279,   194,    55,   195,   356,   194,   443,
     174,   426,   194,   478,    87,   368,   194,   459,   498,   476,
      56,   194,    57,    58,    59,    60,    61,   302,   303,    62,
      63,   323,   324,    88,   510,   247,    64,   191,   102,  -231,
     248,   247,   323,   324,   493,    65,   248,   103,    89,   385,
     202,   104,   196,   325,   495,   315,   316,   285,   276,   277,
      66,    67,    68,    69,   325,   249,   106,   487,   326,   286,
     327,   307,   184,    70,   307,    71,   107,   140,   504,   326,
     352,   327,    58,    59,    60,    61,   118,  -231,    62,   369,
     250,   108,   505,   184,   109,    64,   321,   110,    72,   205,
     287,   497,   141,   383,    65,   111,   347,   112,   348,    73,
     290,   291,   114,   390,   251,   296,   291,   328,   393,    66,
      67,    68,    69,    74,    75,   333,   334,    76,   328,   115,
      77,   116,    70,    78,    71,   252,   117,   398,   335,   336,
     119,    58,    59,    60,    61,    91,   203,    62,   133,   196,
     154,   155,   156,   378,   379,   120,   196,    72,    58,    59,
      60,    61,   399,   203,    62,   411,   334,   440,    73,   445,
     448,   450,   452,   454,   456,   121,   157,    95,   362,   363,
      68,    69,    74,    75,   122,   364,    76,    96,   123,    77,
     124,   128,    78,   129,   464,   334,   125,    68,    69,   126,
      58,    59,    60,    61,   468,   469,    62,   127,   128,    97,
     129,   171,   140,   471,   334,   400,   401,   402,   403,   404,
     405,    99,   323,   324,   494,   334,   406,   509,   334,   514,
     515,   160,   161,   175,   176,   177,   181,   141,   196,    68,
      69,    74,   182,   491,   325,    76,   190,   188,    77,   204,
     128,    78,   129,   140,   201,   208,   222,   212,    74,   326,
     219,   327,    76,   218,   220,    77,   228,   246,    78,   257,
     258,   259,   260,   265,   140,   261,   262,   264,   141,   266,
     269,   272,   520,   152,   153,   154,   155,   156,   273,   275,
     289,   283,   293,   294,   305,   310,   312,   276,   314,   141,
      74,   317,   320,   331,    76,   140,   332,    77,   328,   341,
      78,   342,   349,   346,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   350,
     141,   140,   357,   297,   298,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     360,   366,   375,   367,   299,   300,   141,   140,   164,   384,
     165,   166,   387,   167,   372,   388,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   376,   141,   140,   395,   377,   298,   397,   407,   380,
     389,   396,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   408,   141,   140,
     414,   415,   300,   168,   419,   422,   429,   423,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   424,   141,   140,   437,   221,   458,   438,
     460,   461,   465,   466,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   467,
     141,   140,   385,   292,   472,   473,   474,   475,   479,   480,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   496,   141,   140,   501,   295,
     500,   507,   277,   511,   513,   512,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   516,   141,   518,   523,   301,   519,   187,   522,   140,
     351,   394,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,    91,   322,   439,
     517,   391,   499,   227,   141,   214,    92,    93,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    94,   486,   481,   521,   392,   313,    95,
     140,   311,   470,   345,   256,   371,   416,   434,   338,    96,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   141,   140,    89,   101,     0,
       0,    97,     0,     0,    98,     0,     0,     0,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,     0,     0,
       0,   141,   140,     0,     0,     0,     0,     0,     0,     0,
       0,   140,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,     0,   141,   140,     0,
       0,     0,     0,     0,     0,     0,   141,     0,     0,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,     0,   141,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   148,   149,
     150,   151,   152,   153,   154,   155,   156,     0,     0,     0,
       0,     0,     0,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,     1,     0,     2,     0,     0,     3,
       0,     0,     4,     0,     0,     5,     6,     7,     8,     0,
       0,     9,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,    13,     0,    14,     0,    15,     0,
      16,     0,    17,     0,    18,     0,     0,     0,     0,     0,
       0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     111,   283,    74,   273,    76,    77,     3,     6,   312,   320,
       5,    97,    98,     3,     5,     5,     5,     5,     8,     8,
      36,     5,     6,   109,     8,    36,     5,     6,     9,     8,
       5,     6,     9,    43,     5,     6,    30,   307,     5,     6,
      44,     8,    51,     5,     6,   117,     8,   119,     3,   160,
     122,   123,   124,   125,   126,     3,    71,     3,    24,     3,
       3,    20,     8,    69,   368,    13,     9,    73,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   389,    96,   398,   174,   400,
     401,   402,   403,   404,   405,    54,   112,   112,   380,     3,
     112,   112,   114,   112,   103,   387,    10,    51,   103,   379,
     104,   222,   103,   110,   103,   103,   111,    36,    84,   103,
     110,    37,   111,   111,   103,    37,   110,   397,   103,   110,
      76,   110,   103,   110,    38,   110,   103,   407,   480,   443,
      68,   103,    44,     3,     4,     5,     6,   219,   220,     9,
      10,     3,     4,    57,   496,     3,    16,     5,    81,     3,
       8,     3,     3,     4,   475,    25,     8,    68,    72,    13,
      90,     3,   283,    25,   478,   261,   262,    34,     5,     6,
      40,    41,    42,    43,    25,    33,     0,   469,    40,    46,
      42,    90,   112,    53,    90,    55,   109,    31,    14,    40,
      52,    42,     3,     4,     5,     6,   115,    51,     9,   320,
      58,    47,    28,   112,     3,    16,   112,     3,    78,   291,
      77,    62,    56,   334,    25,   110,   298,     3,   300,    89,
     111,   112,     3,   344,    82,   111,   112,    89,   349,    40,
      41,    42,    43,   103,   104,   111,   112,   107,    89,     3,
     110,     3,    53,   113,    55,   103,   110,    18,   111,   112,
     110,     3,     4,     5,     6,    12,     8,     9,     3,   380,
     104,   105,   106,   111,   112,   110,   387,    78,     3,     4,
       5,     6,    43,     8,     9,   111,   112,   398,    89,   400,
     401,   402,   403,   404,   405,   110,    51,    44,    63,    64,
      42,    43,   103,   104,   110,    70,   107,    54,   110,   110,
     110,    53,   113,    55,   111,   112,   110,    42,    43,   110,
       3,     4,     5,     6,   111,   112,     9,   110,    53,    76,
      55,    81,    31,   111,   112,    96,    97,    98,    99,   100,
     101,    88,     3,     4,   111,   112,   107,   111,   112,   111,
     112,    96,    96,    71,     3,    50,    87,    56,   469,    42,
      43,   103,     3,   474,    25,   107,    80,     3,   110,   111,
      53,   113,    55,    31,    66,     3,    96,   111,   103,    40,
     110,    42,   107,   111,   110,   110,     3,     5,   113,    96,
      96,   103,    48,     8,    31,    76,    76,    76,    56,    90,
       3,     3,   513,   102,   103,   104,   105,   106,   110,    68,
      42,   112,     3,   111,     3,   112,   110,     5,    49,    56,
     103,     3,    96,    86,   107,    31,     3,   110,    89,   111,
     113,    74,    96,   111,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    69,
      56,    31,    39,   111,   112,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
       5,    96,   110,    92,   111,   112,    56,    31,     3,     8,
       5,     6,   112,     8,    59,     8,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   110,    56,    31,   111,   110,   112,    93,    19,   110,
     110,   110,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    91,    56,    31,
      63,     8,   112,    58,     3,   104,   112,   111,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   111,    56,    31,    60,   111,    43,     8,
      39,    60,    96,   112,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   111,
      56,    31,    13,   111,    19,   111,    93,    93,    60,    19,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    19,    56,    31,     3,   111,
     110,   112,     6,   110,    96,     8,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,     5,    56,   111,     5,   111,   111,   107,   112,    31,
     306,   350,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    12,   270,   397,
     507,   111,   481,   136,    56,   123,    21,    22,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,    38,   466,   462,   515,   111,   259,    44,
      31,   257,   429,   291,   161,   321,   367,   387,   283,    54,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    56,    31,    72,    17,    -1,
      -1,    76,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    -1,    56,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    15,    -1,    17,    -1,    -1,    20,
      -1,    -1,    23,    -1,    -1,    26,    27,    28,    29,    -1,
      -1,    32,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    -1,    -1,    -1,    65,    -1,    67,    -1,    69,    -1,
      71,    -1,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    -1,    85
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    61,    65,    67,    69,    71,    73,    75,    83,
      85,   117,   118,   119,   120,   121,   124,   160,   165,   166,
     169,   170,   171,   172,   179,   180,   189,   190,   191,   192,
     195,   200,   201,   202,   205,   207,   208,   209,   210,   212,
     213,    44,     3,    37,    36,    37,    68,    44,     3,     4,
       5,     6,     9,    10,    16,    25,    40,    41,    42,    43,
      53,    55,    78,    89,   103,   104,   107,   110,   113,   125,
     126,   128,   155,   156,   211,     3,    10,    38,    57,    72,
     203,    12,    21,    22,    38,    44,    54,    76,    79,    88,
     162,   203,    81,    68,     3,   129,     0,   109,    47,     3,
       3,   110,     3,   129,     3,     3,     3,   110,   115,   110,
     110,   110,   110,   110,   110,   110,   110,   110,    53,    55,
     155,   155,   155,     3,   159,    36,   112,     3,    13,   127,
      31,    56,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    51,   147,   148,
      96,    96,     3,     9,     3,     5,     6,     8,    58,   163,
     164,    81,     3,     8,    76,    71,     3,    50,   161,   161,
     161,    87,     3,    71,   112,    69,    73,   120,     3,   161,
      80,     5,     6,     8,   103,   110,   136,   137,   178,   181,
     182,    66,    90,     8,   111,   155,   157,   158,     3,   155,
      30,   104,   111,   155,   157,   155,   155,   155,   111,   110,
     110,   111,    96,   112,   114,   110,   129,   126,     3,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,     5,     3,     8,    33,
      58,    82,   103,   136,   167,   168,   164,    96,    96,   103,
      48,    76,    76,   161,    76,     8,    90,   196,   197,     3,
     193,   194,     3,   110,   173,    68,     5,     6,     8,   111,
     136,   138,   183,   112,   184,    34,    46,    77,   206,    42,
     111,   112,   111,     3,   111,   111,   111,   111,   112,   111,
     112,   111,   155,   155,   136,     3,   122,    90,   130,   131,
     112,   167,   110,   163,    49,   161,   161,     3,   198,   199,
      96,   112,   131,     3,     4,    25,    40,    42,    89,   135,
     174,    86,     3,   111,   112,   111,   112,   178,   182,   185,
     186,   111,    74,    43,    96,   158,   111,   155,   155,    96,
      69,   124,    52,   132,   133,   134,   135,    39,   139,   140,
       5,   138,    63,    64,    70,   204,    96,    92,   110,   136,
     137,   194,    59,   149,   150,   110,   110,   110,   111,   112,
     110,   175,   176,   136,     8,    13,   187,   112,     8,   110,
     136,   111,   111,   136,   125,   111,   110,    93,    18,    43,
      96,    97,    98,    99,   100,   101,   107,    19,    91,   141,
     142,   111,    24,    84,    63,     8,   199,   111,   138,     3,
     151,   152,   104,   111,   111,   135,   110,   177,   178,   112,
       3,    51,   188,   178,   186,   138,    36,    60,     8,   134,
     136,   137,     9,   110,     8,   136,   137,     8,   136,   137,
     136,   137,   136,   137,   136,   137,   136,   137,    43,   135,
      39,    60,   143,   144,   111,    96,   112,   111,   111,   112,
     176,   111,    19,   111,    93,    93,   138,     9,   110,    60,
      19,   147,     3,     5,     8,   110,   152,   178,   135,   145,
     146,   136,   103,   137,   111,   138,    19,    62,   145,   149,
     110,     3,   153,   154,    14,    28,    51,   112,   123,   111,
     145,   110,     8,    96,   111,   112,     5,   146,   111,   111,
     136,   154,   112,     5
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

  case 30:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[-1].m_iStart,
				yyvsp[-1].m_iEnd-yyvsp[-1].m_iStart );
		;}
    break;

  case 31:

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
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 34:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 35:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 38:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 41:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 42:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 43:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 44:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 45:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 46:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 47:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 48:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 49:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 50:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 51:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 52:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 54:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 63:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 64:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 65:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 66:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 67:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 68:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 73:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyerror ( pParser, "only >=, <=,<,>, and BETWEEN floating-point filter types are supported in this version" );
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

  case 84:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 85:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 86:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 87:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 88:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 89:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 90:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 91:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 92:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 93:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 94:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 97:

    {
			pParser->SetGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 100:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 103:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 104:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 106:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 108:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 109:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 112:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 113:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 119:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 120:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 121:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 122:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 123:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 124:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 125:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 126:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 128:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 129:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 133:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 134:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 155:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 156:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 157:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 158:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 159:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 164:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 165:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 168:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 169:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 173:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 174:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 175:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 183:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 184:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 185:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 186:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 187:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 188:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 189:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 192:

    { yyval.m_iValue = 1; ;}
    break;

  case 193:

    { yyval.m_iValue = 0; ;}
    break;

  case 194:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 195:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 196:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 197:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 200:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 205:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 206:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 209:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 210:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 211:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 212:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 213:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 214:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 215:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 216:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 217:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 218:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 219:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 220:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 221:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 223:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 224:

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

  case 225:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 228:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 230:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 234:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 235:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 238:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 239:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 242:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 243:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 244:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 245:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 246:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 253:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 254:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 255:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 263:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 264:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 265:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 266:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 267:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 268:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 269:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 270:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 272:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 273:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 274:

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

