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
#define YYFINAL  105
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   911

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  98
/* YYNRULES -- Number of rules. */
#define YYNRULES  270
/* YYNRULES -- Number of states. */
#define YYNSTATES  517

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
      62,    74,    75,    76,    79,    90,    92,    96,    98,   101,
     102,   104,   107,   109,   114,   119,   124,   129,   134,   139,
     143,   147,   153,   155,   159,   160,   162,   165,   167,   171,
     172,   176,   180,   185,   189,   193,   199,   206,   210,   215,
     221,   225,   229,   233,   237,   239,   245,   249,   253,   257,
     261,   263,   265,   270,   274,   278,   280,   282,   285,   287,
     290,   292,   296,   297,   299,   303,   304,   306,   312,   313,
     315,   319,   325,   327,   331,   333,   336,   339,   340,   342,
     345,   350,   351,   353,   356,   358,   362,   366,   370,   376,
     383,   387,   389,   393,   397,   399,   401,   403,   405,   407,
     409,   412,   415,   419,   423,   427,   431,   435,   439,   443,
     447,   451,   455,   459,   463,   467,   471,   475,   479,   483,
     487,   491,   493,   498,   503,   507,   514,   521,   523,   527,
     529,   531,   535,   541,   544,   545,   548,   550,   553,   556,
     560,   565,   570,   572,   574,   576,   578,   580,   582,   586,
     591,   596,   601,   605,   610,   618,   624,   626,   628,   630,
     632,   634,   636,   638,   640,   642,   645,   652,   654,   656,
     657,   661,   663,   667,   669,   673,   677,   679,   683,   685,
     687,   689,   693,   696,   704,   714,   721,   723,   727,   729,
     733,   735,   739,   740,   743,   745,   749,   753,   754,   756,
     758,   760,   764,   766,   768,   772,   779,   781,   785,   789,
     793,   799,   804,   809,   810,   812,   815,   817,   821,   825,
     828,   832,   839,   840,   842,   844,   847,   850,   853,   855,
     863,   865,   867,   869,   873,   880,   884,   888,   890,   894,
     898
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
      19,   145,   123,    -1,    -1,    -1,    51,     5,    -1,    69,
     125,    36,   129,   130,   139,   141,   143,   147,   149,    -1,
     126,    -1,   125,   112,   126,    -1,   104,    -1,   128,   127,
      -1,    -1,     3,    -1,    13,     3,    -1,   155,    -1,    16,
     110,   155,   111,    -1,    53,   110,   155,   111,    -1,    55,
     110,   155,   111,    -1,    78,   110,   155,   111,    -1,    41,
     110,   155,   111,    -1,    25,   110,   104,   111,    -1,    89,
     110,   111,    -1,    40,   110,   111,    -1,    25,   110,    30,
       3,   111,    -1,     3,    -1,   129,   112,     3,    -1,    -1,
     131,    -1,    90,   132,    -1,   134,    -1,   132,    93,   134,
      -1,    -1,   135,    96,   137,    -1,   135,    97,   137,    -1,
      52,   110,     8,   111,    -1,   135,    96,   136,    -1,   135,
      97,   136,    -1,   135,    43,   110,   138,   111,    -1,   135,
     107,    43,   110,   138,   111,    -1,   135,    43,     9,    -1,
     135,   107,    43,     9,    -1,   135,    18,   136,    93,   136,
      -1,   135,    99,   136,    -1,   135,    98,   136,    -1,   135,
     100,   136,    -1,   135,   101,   136,    -1,   133,    -1,   135,
      18,   137,    93,   137,    -1,   135,    99,   137,    -1,   135,
      98,   137,    -1,   135,   100,   137,    -1,   135,   101,   137,
      -1,     3,    -1,     4,    -1,    25,   110,   104,   111,    -1,
      40,   110,   111,    -1,    89,   110,   111,    -1,    42,    -1,
       5,    -1,   103,     5,    -1,     6,    -1,   103,     6,    -1,
     136,    -1,   138,   112,   136,    -1,    -1,   140,    -1,    39,
      19,   135,    -1,    -1,   142,    -1,    91,    39,    60,    19,
     145,    -1,    -1,   144,    -1,    60,    19,   145,    -1,    60,
      19,    62,   110,   111,    -1,   146,    -1,   145,   112,   146,
      -1,   135,    -1,   135,    14,    -1,   135,    28,    -1,    -1,
     148,    -1,    51,     5,    -1,    51,     5,   112,     5,    -1,
      -1,   150,    -1,    59,   151,    -1,   152,    -1,   151,   112,
     152,    -1,     3,    96,     3,    -1,     3,    96,     5,    -1,
       3,    96,   110,   153,   111,    -1,     3,    96,     3,   110,
       8,   111,    -1,     3,    96,     8,    -1,   154,    -1,   153,
     112,   154,    -1,     3,    96,   136,    -1,     3,    -1,     4,
      -1,    42,    -1,     5,    -1,     6,    -1,     9,    -1,   103,
     155,    -1,   107,   155,    -1,   155,   102,   155,    -1,   155,
     103,   155,    -1,   155,   104,   155,    -1,   155,   105,   155,
      -1,   155,    98,   155,    -1,   155,    99,   155,    -1,   155,
      95,   155,    -1,   155,    94,   155,    -1,   155,   106,   155,
      -1,   155,    31,   155,    -1,   155,    56,   155,    -1,   155,
     101,   155,    -1,   155,   100,   155,    -1,   155,    96,   155,
      -1,   155,    97,   155,    -1,   155,    93,   155,    -1,   155,
      92,   155,    -1,   110,   155,   111,    -1,   113,   159,   114,
      -1,   156,    -1,     3,   110,   157,   111,    -1,    43,   110,
     157,   111,    -1,     3,   110,   111,    -1,    55,   110,   155,
     112,   155,   111,    -1,    53,   110,   155,   112,   155,   111,
      -1,   158,    -1,   157,   112,   158,    -1,   155,    -1,     8,
      -1,     3,    96,   136,    -1,   159,   112,     3,    96,   136,
      -1,    73,   162,    -1,    -1,    50,     8,    -1,    88,    -1,
      76,   161,    -1,    54,   161,    -1,    12,    76,   161,    -1,
      12,     8,    76,   161,    -1,    12,     3,    76,   161,    -1,
       3,    -1,    58,    -1,     8,    -1,     5,    -1,     6,    -1,
     163,    -1,   164,   103,   163,    -1,    71,     3,    96,   168,
      -1,    71,     3,    96,   167,    -1,    71,     3,    96,    58,
      -1,    71,    57,   164,    -1,    71,    10,    96,   164,    -1,
      71,    38,     9,    96,   110,   138,   111,    -1,    71,    38,
       3,    96,   167,    -1,     3,    -1,     8,    -1,    82,    -1,
      33,    -1,   136,    -1,    23,    -1,    67,    -1,   170,    -1,
      17,    -1,    75,    81,    -1,   172,    47,     3,   173,    86,
     175,    -1,    45,    -1,    65,    -1,    -1,   110,   174,   111,
      -1,   135,    -1,   174,   112,   135,    -1,   176,    -1,   175,
     112,   176,    -1,   110,   177,   111,    -1,   178,    -1,   177,
     112,   178,    -1,   136,    -1,   137,    -1,     8,    -1,   110,
     138,   111,    -1,   110,   111,    -1,    27,    36,   129,    90,
      42,    96,   136,    -1,    27,    36,   129,    90,    42,    43,
     110,   138,   111,    -1,    20,     3,   110,   181,   184,   111,
      -1,   182,    -1,   181,   112,   182,    -1,   178,    -1,   110,
     183,   111,    -1,     8,    -1,   183,   112,     8,    -1,    -1,
     112,   185,    -1,   186,    -1,   185,   112,   186,    -1,   178,
     187,   188,    -1,    -1,    13,    -1,     3,    -1,    51,    -1,
     190,     3,   161,    -1,    29,    -1,    28,    -1,    73,    79,
     161,    -1,    85,   129,    71,   193,   131,   149,    -1,   194,
      -1,   193,   112,   194,    -1,     3,    96,   136,    -1,     3,
      96,   137,    -1,     3,    96,   110,   138,   111,    -1,     3,
      96,   110,   111,    -1,    73,   203,    87,   196,    -1,    -1,
     197,    -1,    90,   198,    -1,   199,    -1,   198,    92,   199,
      -1,     3,    96,     8,    -1,    73,    22,    -1,    73,    21,
      71,    -1,    71,   203,    81,    48,    49,   204,    -1,    -1,
      38,    -1,    72,    -1,    63,    84,    -1,    63,    24,    -1,
      64,    63,    -1,    70,    -1,    26,    37,     3,    66,   206,
      74,     8,    -1,    46,    -1,    34,    -1,    77,    -1,    32,
      37,     3,    -1,    15,    44,     3,    80,    68,     3,    -1,
      35,    68,     3,    -1,    69,   211,   147,    -1,    10,    -1,
      10,   115,     3,    -1,    83,    68,     3,    -1,    61,    44,
       3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   127,   127,   128,   129,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   158,   159,   163,   164,   168,
     169,   179,   190,   191,   198,   214,   215,   219,   220,   223,
     225,   226,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   243,   244,   247,   249,   253,   257,   258,   261,
     262,   263,   267,   272,   279,   287,   295,   304,   309,   314,
     319,   324,   329,   334,   339,   344,   349,   354,   359,   364,
     372,   373,   378,   384,   390,   396,   405,   406,   417,   418,
     422,   428,   434,   436,   440,   446,   448,   452,   458,   460,
     464,   468,   475,   476,   480,   481,   482,   485,   487,   491,
     496,   503,   505,   509,   513,   514,   518,   523,   528,   534,
     539,   547,   552,   559,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   586,   587,   588,   589,   590,   591,   592,   593,   594,
     595,   596,   600,   601,   602,   603,   604,   608,   609,   613,
     614,   618,   619,   625,   628,   630,   634,   635,   636,   637,
     638,   643,   653,   654,   655,   656,   657,   661,   662,   666,
     671,   676,   681,   682,   686,   691,   699,   700,   704,   705,
     706,   720,   721,   722,   726,   727,   733,   741,   742,   745,
     747,   751,   752,   756,   757,   761,   765,   766,   770,   771,
     772,   773,   774,   780,   788,   802,   810,   814,   821,   822,
     829,   839,   845,   847,   851,   856,   860,   867,   869,   873,
     874,   880,   888,   889,   895,   901,   909,   910,   914,   918,
     922,   926,   936,   942,   943,   947,   951,   952,   956,   960,
     967,   974,   980,   981,   982,   986,   987,   988,   989,   995,
    1006,  1007,  1008,  1012,  1023,  1035,  1046,  1054,  1055,  1064,
    1075
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
     121,   122,   123,   123,   124,   125,   125,   126,   126,   127,
     127,   127,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   129,   129,   130,   130,   131,   132,   132,   133,
     133,   133,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     135,   135,   135,   135,   135,   135,   136,   136,   137,   137,
     138,   138,   139,   139,   140,   141,   141,   142,   143,   143,
     144,   144,   145,   145,   146,   146,   146,   147,   147,   148,
     148,   149,   149,   150,   151,   151,   152,   152,   152,   152,
     152,   153,   153,   154,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   156,   156,   156,   156,   156,   157,   157,   158,
     158,   159,   159,   160,   161,   161,   162,   162,   162,   162,
     162,   162,   163,   163,   163,   163,   163,   164,   164,   165,
     165,   165,   165,   165,   166,   166,   167,   167,   168,   168,
     168,   169,   169,   169,   170,   170,   171,   172,   172,   173,
     173,   174,   174,   175,   175,   176,   177,   177,   178,   178,
     178,   178,   178,   179,   179,   180,   181,   181,   182,   182,
     183,   183,   184,   184,   185,   185,   186,   187,   187,   188,
     188,   189,   190,   190,   191,   192,   193,   193,   194,   194,
     194,   194,   195,   196,   196,   197,   198,   198,   199,   200,
     201,   202,   203,   203,   203,   204,   204,   204,   204,   205,
     206,   206,   206,   207,   208,   209,   210,   211,   211,   212,
     213
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
      11,     0,     0,     2,    10,     1,     3,     1,     2,     0,
       1,     2,     1,     4,     4,     4,     4,     4,     4,     3,
       3,     5,     1,     3,     0,     1,     2,     1,     3,     0,
       3,     3,     4,     3,     3,     5,     6,     3,     4,     5,
       3,     3,     3,     3,     1,     5,     3,     3,     3,     3,
       1,     1,     4,     3,     3,     1,     1,     2,     1,     2,
       1,     3,     0,     1,     3,     0,     1,     5,     0,     1,
       3,     5,     1,     3,     1,     2,     2,     0,     1,     2,
       4,     0,     1,     2,     1,     3,     3,     3,     5,     6,
       3,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     4,     4,     3,     6,     6,     1,     3,     1,
       1,     3,     5,     2,     0,     2,     1,     2,     2,     3,
       4,     4,     1,     1,     1,     1,     1,     1,     3,     4,
       4,     4,     3,     4,     7,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     6,     1,     1,     0,
       3,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     3,     2,     7,     9,     6,     1,     3,     1,     3,
       1,     3,     0,     2,     1,     3,     3,     0,     1,     1,
       1,     3,     1,     1,     3,     6,     1,     3,     3,     3,
       5,     4,     4,     0,     1,     2,     1,     3,     3,     2,
       3,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     1,     3,     6,     3,     3,     1,     3,     3,
       3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   194,     0,   191,     0,     0,   233,   232,     0,
       0,   197,     0,   198,   192,     0,   252,   252,     0,     0,
       0,     0,     2,     3,    25,    27,    29,    28,     7,     8,
       9,   193,     5,     0,     6,    10,    11,     0,    12,    13,
      14,    15,    16,    21,    17,    18,    19,    20,    22,    23,
      24,     0,     0,     0,     0,     0,     0,     0,   124,   125,
     127,   128,   129,   267,     0,     0,     0,     0,   126,     0,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
      35,    39,    42,   151,   107,     0,     0,   253,     0,   254,
       0,     0,     0,   249,   253,   164,   164,   164,   166,   163,
       0,   195,     0,    52,     0,     1,     4,     0,   164,     0,
       0,     0,     0,   263,   265,   270,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   130,
     131,     0,     0,     0,     0,     0,    40,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   266,   108,     0,
       0,     0,     0,   172,   175,   176,   174,   173,   177,   182,
       0,     0,     0,   164,   250,     0,   168,   167,   234,   243,
     269,     0,     0,     0,     0,    26,   199,   231,     0,    86,
      88,   210,     0,     0,   208,   209,   218,   222,   216,     0,
       0,   160,   154,   159,     0,   157,   268,     0,     0,     0,
      50,     0,     0,     0,     0,     0,    49,     0,     0,   149,
       0,     0,   150,    31,    54,    36,    41,   141,   142,   148,
     147,   139,   138,   145,   146,   136,   137,   144,   143,   132,
     133,   134,   135,   140,   109,   186,   187,   189,   181,   188,
       0,   190,   180,   179,   183,     0,     0,     0,     0,   164,
     164,   169,   165,     0,   242,   244,     0,     0,   236,    53,
       0,     0,     0,    87,    89,   220,   212,    90,     0,     0,
       0,     0,   261,   260,   262,     0,     0,   152,     0,    43,
       0,    48,    47,   153,    44,     0,    45,     0,    46,     0,
       0,   161,     0,     0,    59,    92,    55,     0,   185,     0,
     178,     0,   171,   170,     0,   245,   246,     0,     0,   111,
      80,    81,     0,     0,    85,     0,   201,     0,     0,   264,
     211,     0,   219,     0,   218,   217,   223,   224,   215,     0,
       0,     0,   158,    51,     0,     0,     0,     0,     0,     0,
      56,    74,    57,     0,     0,    95,    93,   110,     0,     0,
       0,   258,   251,     0,     0,     0,   238,   239,   237,     0,
     235,   112,     0,     0,     0,   200,     0,     0,   196,   203,
      91,   221,   228,     0,     0,   259,     0,   213,   156,   155,
     162,     0,     0,     0,    59,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    98,    96,   184,   256,
     255,   257,   248,   247,   241,     0,     0,   113,   114,     0,
      83,    84,   202,     0,     0,   206,     0,   229,   230,   226,
     227,   225,     0,     0,     0,     0,    58,     0,     0,    67,
       0,    63,    60,    64,    61,    71,    77,    70,    76,    72,
      78,    73,    79,     0,    94,     0,     0,   107,    99,   240,
       0,     0,    82,   205,     0,   204,   214,     0,    62,     0,
       0,     0,    68,     0,     0,     0,   111,   116,   117,   120,
       0,   115,   207,   104,    32,   102,    69,     0,    75,    65,
       0,     0,     0,   100,    34,     0,     0,     0,   121,   105,
     106,     0,     0,    30,    66,    97,     0,     0,     0,   118,
       0,    33,   103,   101,   119,   123,   122
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   303,   503,    26,    79,
      80,   138,    81,   224,   305,   306,   350,   351,   352,   483,
     277,   195,   278,   355,   356,   406,   407,   457,   458,   484,
     485,   157,   158,   370,   371,   417,   418,   497,   498,    82,
      83,   204,   205,   133,    27,   176,    99,   168,   169,    28,
      29,   252,   253,    30,    31,    32,    33,   271,   327,   378,
     379,   424,   196,    34,    35,   197,   198,   279,   281,   336,
     337,   383,   429,    36,    37,    38,    39,   267,   268,    40,
     264,   265,   315,   316,    41,    42,    43,    90,   362,    44,
     285,    45,    46,    47,    48,    84,    49,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -430
static const short yypact[] =
{
     826,   -25,  -430,    45,  -430,    49,     2,  -430,  -430,    65,
     -11,  -430,    19,  -430,  -430,   112,   283,   635,    95,    41,
     157,   189,  -430,    74,  -430,  -430,  -430,  -430,  -430,  -430,
    -430,  -430,  -430,   156,  -430,  -430,  -430,   190,  -430,  -430,
    -430,  -430,  -430,  -430,  -430,  -430,  -430,  -430,  -430,  -430,
    -430,   206,   100,   209,   157,   215,   234,   244,   131,  -430,
    -430,  -430,  -430,   138,   141,   151,   155,   158,  -430,   163,
     201,   203,   207,   214,   139,  -430,   139,   139,   256,    28,
    -430,   175,   598,  -430,   265,   230,   245,    52,   359,  -430,
     249,    36,   263,  -430,  -430,   293,   293,   293,  -430,  -430,
     266,  -430,   342,  -430,   -47,  -430,   106,   357,   293,   286,
      10,   310,   -53,  -430,  -430,  -430,   153,   374,   139,    -7,
     267,   139,   272,   139,   139,   139,   269,   271,   273,  -430,
    -430,   412,   291,   115,     1,   229,  -430,   385,  -430,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   391,  -430,  -430,   197,
     359,   301,   320,  -430,  -430,  -430,  -430,  -430,  -430,   317,
     373,   361,   362,   293,  -430,   431,  -430,  -430,  -430,   354,
    -430,   442,   443,   229,   174,  -430,   337,  -430,   380,  -430,
    -430,  -430,   193,    20,  -430,  -430,  -430,   353,  -430,   134,
     407,  -430,  -430,   598,   128,  -430,  -430,   435,   464,   360,
    -430,   463,   132,   307,   330,   489,  -430,   139,   139,  -430,
      26,   466,  -430,  -430,    61,  -430,  -430,  -430,  -430,   631,
     649,   682,   708,   734,   734,   288,   288,   288,   288,   252,
     252,  -430,  -430,  -430,   363,  -430,  -430,  -430,  -430,  -430,
     467,  -430,  -430,  -430,   317,    98,   364,   359,   424,   293,
     293,  -430,  -430,   490,  -430,  -430,   396,    79,  -430,  -430,
      18,   409,   494,  -430,  -430,  -430,  -430,  -430,   146,   183,
      10,   387,  -430,  -430,  -430,   425,    -8,  -430,   272,  -430,
     389,  -430,  -430,  -430,  -430,   139,  -430,   139,  -430,   358,
     384,  -430,   405,   433,   295,   482,  -430,   498,  -430,    26,
    -430,   150,  -430,  -430,   426,   432,  -430,    94,   442,   483,
    -430,  -430,   415,   416,  -430,   434,  -430,   185,   438,  -430,
    -430,    26,  -430,   535,   136,  -430,   437,  -430,  -430,   542,
     441,    26,  -430,  -430,   516,   540,    26,   229,   459,   444,
     460,  -430,  -430,   205,   533,   484,  -430,  -430,   211,   -17,
     510,  -430,  -430,   568,   490,    21,  -430,  -430,  -430,   574,
    -430,  -430,   474,   468,   469,  -430,    18,    24,   485,  -430,
    -430,  -430,  -430,    53,    24,  -430,    26,  -430,  -430,  -430,
    -430,    54,   538,   591,   295,    35,     4,    35,    35,    35,
      35,    35,    35,   558,    18,   563,   543,  -430,  -430,  -430,
    -430,  -430,  -430,  -430,  -430,   217,   508,   493,  -430,   495,
    -430,  -430,  -430,    22,   238,  -430,   438,  -430,  -430,  -430,
     594,  -430,   240,   157,   604,   513,  -430,   532,   537,  -430,
      26,  -430,  -430,  -430,  -430,  -430,  -430,  -430,  -430,  -430,
    -430,  -430,  -430,    25,  -430,   566,   609,   265,  -430,  -430,
       9,   574,  -430,  -430,    24,  -430,  -430,    18,  -430,    26,
      27,   258,  -430,    26,   612,   306,   483,   539,  -430,  -430,
     645,  -430,  -430,   149,    -9,  -430,  -430,   644,  -430,  -430,
     260,    18,   545,   541,  -430,   650,   556,   262,  -430,  -430,
    -430,   654,    18,  -430,  -430,   541,   549,   550,    26,  -430,
     645,  -430,  -430,  -430,  -430,  -430,  -430
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -430,  -430,  -430,  -430,   557,  -430,  -430,  -430,   365,   318,
     529,  -430,  -430,   130,  -430,   399,  -430,  -430,   275,  -268,
    -110,  -306,  -299,  -430,  -430,  -430,  -430,  -430,  -430,  -429,
     165,   213,  -430,   195,  -430,  -430,   216,  -430,   162,   -71,
    -430,   552,   388,  -430,  -430,   -88,  -430,   418,   518,  -430,
    -430,   427,  -430,  -430,  -430,  -430,  -430,  -430,  -430,  -430,
     253,  -430,  -279,  -430,  -430,  -430,   401,  -430,  -430,  -430,
     299,  -430,  -430,  -430,  -430,  -430,  -430,  -430,   366,  -430,
    -430,  -430,  -430,   321,  -430,  -430,  -430,   669,  -430,  -430,
    -430,  -430,  -430,  -430,  -430,  -430,  -430,  -430
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -228
static const short yytable[] =
{
     194,   334,   326,   129,   103,   130,   131,   409,   177,   178,
     358,   367,   477,   439,   478,   189,   190,   479,   191,    51,
     187,   320,   321,   208,   181,   189,   189,   189,   275,   189,
     190,   189,   191,   190,   472,   340,   353,   200,    54,   171,
     189,   190,   501,   322,   172,   203,   493,   207,    52,   251,
     211,   203,   213,   214,   215,   161,   427,    56,   323,   182,
     324,   162,   505,    57,   134,   182,   415,   410,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   261,    53,   432,   341,   438,
     433,   442,   444,   446,   448,   450,   452,   209,   425,   189,
     190,   245,    55,   502,   428,   430,   246,   325,   422,   102,
     301,   223,   173,   192,   440,    58,    59,    60,    61,   480,
     193,    62,    63,   250,   250,   250,   353,   192,    64,   250,
     487,   276,   414,   276,   423,   473,   454,    65,   192,  -227,
     135,   471,    58,    59,    60,    61,   299,   300,    62,   382,
     104,   304,    66,    67,    68,    69,    58,    59,    60,    61,
     103,   201,    62,   499,   488,    70,   135,    71,   282,   304,
     194,   312,   313,   182,   490,   183,   101,   500,   136,   184,
     283,    68,    69,   106,   112,   482,    91,  -227,   137,   105,
      72,   318,   127,   108,   128,    68,    69,   192,   273,   274,
     245,    73,   189,   107,   365,   246,   127,   366,   128,   109,
     110,   284,   111,   359,   360,    74,    75,   203,   113,    76,
     361,   380,    77,   395,   344,    78,   345,   221,    95,   222,
     247,   387,    58,    59,    60,    61,   390,   114,    62,   287,
     288,   116,    74,   293,   288,    64,    76,   115,   396,    77,
      96,   118,    78,   117,    65,   248,    74,   330,   331,   132,
      76,   119,    98,    77,   202,   120,    78,   194,   121,    66,
      67,    68,    69,   122,   194,    58,    59,    60,    61,   249,
     201,    62,    70,   139,    71,   437,    85,   441,   443,   445,
     447,   449,   451,    86,   332,   333,   375,   376,   320,   321,
     250,   397,   398,   399,   400,   401,   402,    72,   140,   320,
     321,   123,   403,   124,    68,    69,   156,   125,    73,   139,
     322,    87,   408,   331,   126,   127,   159,   128,   459,   331,
     170,   322,    74,    75,   174,   323,    76,   324,   139,    77,
      88,   160,    78,   175,   140,   180,   323,   349,   324,   463,
     464,   466,   331,   179,   194,    89,   153,   154,   155,   486,
     186,   139,   163,   140,   164,   165,   188,   166,   492,   489,
     331,   504,   331,   509,   510,    74,   199,   206,   210,    76,
     216,   217,    77,   218,   325,    78,   140,   220,   226,   139,
     151,   152,   153,   154,   155,   325,   244,   255,   515,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   140,   139,   256,   167,   294,   295,
     257,   258,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   259,   260,   262,
     140,   296,   297,   139,   263,   266,   269,   270,   272,   286,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   280,   139,   290,   140,   302,
     295,   291,   273,   311,   309,   307,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   140,   317,   314,   139,   328,   297,   329,   338,   339,
     343,   346,   347,   357,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   140,
     139,   354,   363,   219,   364,   372,   373,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   369,   381,   374,   140,   289,   139,   377,   384,
     385,   386,   404,   394,   393,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     392,   139,   140,   411,   292,   405,   412,   416,   419,   420,
     421,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   140,   426,   434,   435,
     298,   453,   455,   456,   460,   461,   462,   382,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   467,   468,   469,   474,   388,   475,   139,
     470,   491,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,    91,   496,   495,
     274,   389,   508,   502,   140,   506,    92,    93,   507,   511,
     513,   514,   139,   185,   225,   391,   319,   512,   348,   436,
     476,   494,   516,    94,   212,   310,   342,   481,   254,   465,
     139,   335,   308,   431,   368,   413,   100,   140,     0,    95,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   140,     0,    89,     0,     0,
       0,    96,     0,   139,    97,     0,     0,     0,     0,     0,
       0,     0,     0,    98,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   140,   139,
       0,     0,     0,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,     0,     0,     0,     0,
       0,     0,     0,     0,   140,   139,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,     0,
     140,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   147,   148,   149,   150,   151,   152,   153,   154,
     155,     1,     0,     2,     0,     0,     3,     0,     0,     4,
       0,     0,     5,     6,     7,     8,     0,     0,     9,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,    13,     0,    14,     0,    15,     0,    16,     0,    17,
       0,    18,     0,     0,     0,     0,     0,     0,     0,    19,
       0,    20
};

static const short yycheck[] =
{
     110,   280,   270,    74,     3,    76,    77,    24,    96,    97,
     309,   317,     3,     9,     5,     5,     6,     8,     8,    44,
     108,     3,     4,    30,    71,     5,     5,     5,     8,     5,
       6,     5,     8,     6,     9,    43,   304,    90,    36,     3,
       5,     6,    51,    25,     8,   116,   475,   118,     3,   159,
     121,   122,   123,   124,   125,     3,     3,    68,    40,   112,
      42,     9,   491,    44,    36,   112,   365,    84,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   173,    37,   386,    96,   395,
      36,   397,   398,   399,   400,   401,   402,   104,   377,     5,
       6,     3,    37,   112,    51,   384,     8,    89,   376,    68,
     220,   110,    76,   103,   110,     3,     4,     5,     6,   110,
     110,     9,    10,   103,   103,   103,   394,   103,    16,   103,
     103,   111,   111,   111,   110,   110,   404,    25,   103,     3,
     112,   440,     3,     4,     5,     6,   217,   218,     9,    13,
      20,    90,    40,    41,    42,    43,     3,     4,     5,     6,
       3,     8,     9,    14,   470,    53,   112,    55,    34,    90,
     280,   259,   260,   112,   473,    69,    81,    28,     3,    73,
      46,    42,    43,   109,    54,   464,    12,    51,    13,     0,
      78,   112,    53,     3,    55,    42,    43,   103,     5,     6,
       3,    89,     5,    47,   110,     8,    53,   317,    55,     3,
     110,    77,     3,    63,    64,   103,   104,   288,     3,   107,
      70,   331,   110,    18,   295,   113,   297,   112,    54,   114,
      33,   341,     3,     4,     5,     6,   346,     3,     9,   111,
     112,   110,   103,   111,   112,    16,   107,     3,    43,   110,
      76,   110,   113,   115,    25,    58,   103,   111,   112,     3,
     107,   110,    88,   110,   111,   110,   113,   377,   110,    40,
      41,    42,    43,   110,   384,     3,     4,     5,     6,    82,
       8,     9,    53,    31,    55,   395,     3,   397,   398,   399,
     400,   401,   402,    10,   111,   112,   111,   112,     3,     4,
     103,    96,    97,    98,    99,   100,   101,    78,    56,     3,
       4,   110,   107,   110,    42,    43,    51,   110,    89,    31,
      25,    38,   111,   112,   110,    53,    96,    55,   111,   112,
      81,    25,   103,   104,    71,    40,   107,    42,    31,   110,
      57,    96,   113,    50,    56,     3,    40,    52,    42,   111,
     112,   111,   112,    87,   464,    72,   104,   105,   106,   469,
       3,    31,     3,    56,     5,     6,    80,     8,    62,   111,
     112,   111,   112,   111,   112,   103,    66,     3,   111,   107,
     111,   110,   110,   110,    89,   113,    56,    96,     3,    31,
     102,   103,   104,   105,   106,    89,     5,    96,   508,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    56,    31,    96,    58,   111,   112,
     103,    48,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    76,    76,     8,
      56,   111,   112,    31,    90,     3,     3,   110,    68,    42,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   112,    31,     3,    56,     3,
     112,   111,     5,    49,   110,   112,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    56,    96,     3,    31,    86,   112,     3,   111,    74,
     111,    96,    69,     5,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    56,
      31,    39,    96,   111,    92,   110,   110,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,    59,     8,   110,    56,   111,    31,   110,   112,
       8,   110,    19,    93,   110,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     111,    31,    56,    63,   111,    91,     8,     3,   104,   111,
     111,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    56,   112,    60,     8,
     111,    43,    39,    60,    96,   112,   111,    13,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,    19,   111,    93,    60,   111,    19,    31,
      93,    19,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    12,     3,   110,
       6,   111,    96,   112,    56,   110,    21,    22,     8,     5,
     111,   111,    31,   106,   135,   347,   267,   502,   303,   394,
     457,   476,   510,    38,   122,   257,   288,   461,   160,   426,
      31,   280,   255,   384,   318,   364,    17,    56,    -1,    54,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    56,    -1,    72,    -1,    -1,
      -1,    76,    -1,    31,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,    56,    31,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    15,    -1,    17,    -1,    -1,    20,    -1,    -1,    23,
      -1,    -1,    26,    27,    28,    29,    -1,    -1,    32,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,    -1,
      -1,    65,    -1,    67,    -1,    69,    -1,    71,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      -1,    85
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
     203,    12,    21,    22,    38,    54,    76,    79,    88,   162,
     203,    81,    68,     3,   129,     0,   109,    47,     3,     3,
     110,     3,   129,     3,     3,     3,   110,   115,   110,   110,
     110,   110,   110,   110,   110,   110,   110,    53,    55,   155,
     155,   155,     3,   159,    36,   112,     3,    13,   127,    31,
      56,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    51,   147,   148,    96,
      96,     3,     9,     3,     5,     6,     8,    58,   163,   164,
      81,     3,     8,    76,    71,    50,   161,   161,   161,    87,
       3,    71,   112,    69,    73,   120,     3,   161,    80,     5,
       6,     8,   103,   110,   136,   137,   178,   181,   182,    66,
      90,     8,   111,   155,   157,   158,     3,   155,    30,   104,
     111,   155,   157,   155,   155,   155,   111,   110,   110,   111,
      96,   112,   114,   110,   129,   126,     3,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,     5,     3,     8,    33,    58,    82,
     103,   136,   167,   168,   164,    96,    96,   103,    48,    76,
      76,   161,     8,    90,   196,   197,     3,   193,   194,     3,
     110,   173,    68,     5,     6,     8,   111,   136,   138,   183,
     112,   184,    34,    46,    77,   206,    42,   111,   112,   111,
       3,   111,   111,   111,   111,   112,   111,   112,   111,   155,
     155,   136,     3,   122,    90,   130,   131,   112,   167,   110,
     163,    49,   161,   161,     3,   198,   199,    96,   112,   131,
       3,     4,    25,    40,    42,    89,   135,   174,    86,     3,
     111,   112,   111,   112,   178,   182,   185,   186,   111,    74,
      43,    96,   158,   111,   155,   155,    96,    69,   124,    52,
     132,   133,   134,   135,    39,   139,   140,     5,   138,    63,
      64,    70,   204,    96,    92,   110,   136,   137,   194,    59,
     149,   150,   110,   110,   110,   111,   112,   110,   175,   176,
     136,     8,    13,   187,   112,     8,   110,   136,   111,   111,
     136,   125,   111,   110,    93,    18,    43,    96,    97,    98,
      99,   100,   101,   107,    19,    91,   141,   142,   111,    24,
      84,    63,     8,   199,   111,   138,     3,   151,   152,   104,
     111,   111,   135,   110,   177,   178,   112,     3,    51,   188,
     178,   186,   138,    36,    60,     8,   134,   136,   137,     9,
     110,   136,   137,   136,   137,   136,   137,   136,   137,   136,
     137,   136,   137,    43,   135,    39,    60,   143,   144,   111,
      96,   112,   111,   111,   112,   176,   111,    19,   111,    93,
      93,   138,     9,   110,    60,    19,   147,     3,     5,     8,
     110,   152,   178,   135,   145,   146,   136,   103,   137,   111,
     138,    19,    62,   145,   149,   110,     3,   153,   154,    14,
      28,    51,   112,   123,   111,   145,   110,     8,    96,   111,
     112,     5,   146,   111,   111,   136,   154
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
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 37:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 40:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 41:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 42:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 43:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 44:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 45:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 46:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 47:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 48:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 49:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 50:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 51:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 53:

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
			pFilter->m_dValues.Sort();
		;}
    break;

  case 66:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
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
			yyerror ( pParser, "only >=, <=,<,>, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 75:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 76:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 82:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 83:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 84:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 85:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 86:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 87:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 88:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 89:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 90:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 91:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 94:

    {
			pParser->SetGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 97:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 100:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 101:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 103:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 105:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 106:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 109:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 110:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 116:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 117:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 118:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 119:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 122:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 123:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 125:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 126:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 130:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 131:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 152:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 153:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 154:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 155:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 156:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 161:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 162:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 165:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 166:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 168:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 169:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS; ;}
    break;

  case 170:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 171:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 179:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 180:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 181:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 182:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 183:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 184:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 185:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 188:

    { yyval.m_iValue = 1; ;}
    break;

  case 189:

    { yyval.m_iValue = 0; ;}
    break;

  case 190:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 191:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 192:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 193:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 196:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 197:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 198:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 201:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 202:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 205:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 206:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 207:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 208:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 209:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 210:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 211:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 212:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 213:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 214:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 215:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 216:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 217:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 219:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 220:

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

  case 221:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 224:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 226:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 230:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 231:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 234:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 235:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 238:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 239:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 240:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 241:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 242:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 249:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 250:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 251:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 259:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 260:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 261:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 262:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 263:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 264:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 265:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 266:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 268:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 269:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 270:

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

