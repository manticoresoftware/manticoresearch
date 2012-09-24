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
#define YYLAST   899

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  96
/* YYNRULES -- Number of rules. */
#define YYNRULES  266
/* YYNRULES -- Number of states. */
#define YYNSTATES  513

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
     176,   180,   184,   190,   197,   201,   206,   212,   216,   220,
     224,   228,   232,   236,   240,   244,   250,   254,   258,   260,
     262,   267,   271,   275,   277,   279,   282,   284,   287,   289,
     293,   294,   296,   300,   301,   303,   309,   310,   312,   316,
     322,   324,   328,   330,   333,   336,   337,   339,   342,   347,
     348,   350,   353,   355,   359,   363,   367,   373,   380,   384,
     386,   390,   394,   396,   398,   400,   402,   404,   406,   409,
     412,   416,   420,   424,   428,   432,   436,   440,   444,   448,
     452,   456,   460,   464,   468,   472,   476,   480,   484,   488,
     490,   495,   500,   504,   511,   518,   520,   524,   526,   528,
     532,   538,   541,   542,   545,   547,   550,   553,   557,   562,
     567,   569,   571,   573,   575,   577,   582,   587,   592,   596,
     601,   609,   615,   617,   619,   621,   623,   625,   627,   629,
     631,   633,   636,   643,   645,   647,   648,   652,   654,   658,
     660,   664,   668,   670,   674,   676,   678,   680,   684,   687,
     695,   705,   712,   714,   718,   720,   724,   726,   730,   731,
     734,   736,   740,   744,   745,   747,   749,   751,   755,   757,
     759,   763,   770,   772,   776,   780,   784,   790,   795,   800,
     801,   803,   806,   808,   812,   816,   819,   823,   830,   831,
     833,   835,   838,   841,   844,   846,   854,   856,   858,   860,
     864,   871,   875,   879,   881,   885,   889
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     117,     0,    -1,   118,    -1,   119,    -1,   119,   109,    -1,
     169,    -1,   177,    -1,   163,    -1,   164,    -1,   167,    -1,
     178,    -1,   187,    -1,   189,    -1,   190,    -1,   193,    -1,
     198,    -1,   199,    -1,   203,    -1,   205,    -1,   206,    -1,
     207,    -1,   200,    -1,   208,    -1,   210,    -1,   211,    -1,
     120,    -1,   119,   109,   120,    -1,   121,    -1,   159,    -1,
     124,    -1,    69,   125,    36,   110,   122,   124,   111,    60,
      19,   144,   123,    -1,    -1,    -1,    51,     5,    -1,    69,
     125,    36,   129,   130,   138,   140,   142,   146,   148,    -1,
     126,    -1,   125,   112,   126,    -1,   104,    -1,   128,   127,
      -1,    -1,     3,    -1,    13,     3,    -1,   154,    -1,    16,
     110,   154,   111,    -1,    53,   110,   154,   111,    -1,    55,
     110,   154,   111,    -1,    78,   110,   154,   111,    -1,    41,
     110,   154,   111,    -1,    25,   110,   104,   111,    -1,    89,
     110,   111,    -1,    40,   110,   111,    -1,    25,   110,    30,
       3,   111,    -1,     3,    -1,   129,   112,     3,    -1,    -1,
     131,    -1,    90,   132,    -1,   133,    -1,   132,    93,   133,
      -1,    52,   110,     8,   111,    -1,   134,    96,   135,    -1,
     134,    97,   135,    -1,   134,    43,   110,   137,   111,    -1,
     134,   107,    43,   110,   137,   111,    -1,   134,    43,     9,
      -1,   134,   107,    43,     9,    -1,   134,    18,   135,    93,
     135,    -1,   134,    99,   135,    -1,   134,    98,   135,    -1,
     134,   100,   135,    -1,   134,   101,   135,    -1,   134,    96,
     136,    -1,   134,    97,   136,    -1,   134,    99,   136,    -1,
     134,    98,   136,    -1,   134,    18,   136,    93,   136,    -1,
     134,   100,   136,    -1,   134,   101,   136,    -1,     3,    -1,
       4,    -1,    25,   110,   104,   111,    -1,    40,   110,   111,
      -1,    89,   110,   111,    -1,    42,    -1,     5,    -1,   103,
       5,    -1,     6,    -1,   103,     6,    -1,   135,    -1,   137,
     112,   135,    -1,    -1,   139,    -1,    39,    19,   134,    -1,
      -1,   141,    -1,    91,    39,    60,    19,   144,    -1,    -1,
     143,    -1,    60,    19,   144,    -1,    60,    19,    62,   110,
     111,    -1,   145,    -1,   144,   112,   145,    -1,   134,    -1,
     134,    14,    -1,   134,    28,    -1,    -1,   147,    -1,    51,
       5,    -1,    51,     5,   112,     5,    -1,    -1,   149,    -1,
      59,   150,    -1,   151,    -1,   150,   112,   151,    -1,     3,
      96,     3,    -1,     3,    96,     5,    -1,     3,    96,   110,
     152,   111,    -1,     3,    96,     3,   110,     8,   111,    -1,
       3,    96,     8,    -1,   153,    -1,   152,   112,   153,    -1,
       3,    96,   135,    -1,     3,    -1,     4,    -1,    42,    -1,
       5,    -1,     6,    -1,     9,    -1,   103,   154,    -1,   107,
     154,    -1,   154,   102,   154,    -1,   154,   103,   154,    -1,
     154,   104,   154,    -1,   154,   105,   154,    -1,   154,    98,
     154,    -1,   154,    99,   154,    -1,   154,    95,   154,    -1,
     154,    94,   154,    -1,   154,   106,   154,    -1,   154,    31,
     154,    -1,   154,    56,   154,    -1,   154,   101,   154,    -1,
     154,   100,   154,    -1,   154,    96,   154,    -1,   154,    97,
     154,    -1,   154,    93,   154,    -1,   154,    92,   154,    -1,
     110,   154,   111,    -1,   113,   158,   114,    -1,   155,    -1,
       3,   110,   156,   111,    -1,    43,   110,   156,   111,    -1,
       3,   110,   111,    -1,    55,   110,   154,   112,   154,   111,
      -1,    53,   110,   154,   112,   154,   111,    -1,   157,    -1,
     156,   112,   157,    -1,   154,    -1,     8,    -1,     3,    96,
     135,    -1,   158,   112,     3,    96,   135,    -1,    73,   161,
      -1,    -1,    50,     8,    -1,    88,    -1,    76,   160,    -1,
      54,   160,    -1,    12,    76,   160,    -1,    12,     8,    76,
     160,    -1,    12,     3,    76,   160,    -1,     3,    -1,    58,
      -1,     8,    -1,     5,    -1,     6,    -1,    71,     3,    96,
     166,    -1,    71,     3,    96,   165,    -1,    71,     3,    96,
      58,    -1,    71,    57,   162,    -1,    71,    10,    96,   162,
      -1,    71,    38,     9,    96,   110,   137,   111,    -1,    71,
      38,     3,    96,   165,    -1,     3,    -1,     8,    -1,    82,
      -1,    33,    -1,   135,    -1,    23,    -1,    67,    -1,   168,
      -1,    17,    -1,    75,    81,    -1,   170,    47,     3,   171,
      86,   173,    -1,    45,    -1,    65,    -1,    -1,   110,   172,
     111,    -1,   134,    -1,   172,   112,   134,    -1,   174,    -1,
     173,   112,   174,    -1,   110,   175,   111,    -1,   176,    -1,
     175,   112,   176,    -1,   135,    -1,   136,    -1,     8,    -1,
     110,   137,   111,    -1,   110,   111,    -1,    27,    36,   129,
      90,    42,    96,   135,    -1,    27,    36,   129,    90,    42,
      43,   110,   137,   111,    -1,    20,     3,   110,   179,   182,
     111,    -1,   180,    -1,   179,   112,   180,    -1,   176,    -1,
     110,   181,   111,    -1,     8,    -1,   181,   112,     8,    -1,
      -1,   112,   183,    -1,   184,    -1,   183,   112,   184,    -1,
     176,   185,   186,    -1,    -1,    13,    -1,     3,    -1,    51,
      -1,   188,     3,   160,    -1,    29,    -1,    28,    -1,    73,
      79,   160,    -1,    85,   129,    71,   191,   131,   148,    -1,
     192,    -1,   191,   112,   192,    -1,     3,    96,   135,    -1,
       3,    96,   136,    -1,     3,    96,   110,   137,   111,    -1,
       3,    96,   110,   111,    -1,    73,   201,    87,   194,    -1,
      -1,   195,    -1,    90,   196,    -1,   197,    -1,   196,    92,
     197,    -1,     3,    96,     8,    -1,    73,    22,    -1,    73,
      21,    71,    -1,    71,   201,    81,    48,    49,   202,    -1,
      -1,    38,    -1,    72,    -1,    63,    84,    -1,    63,    24,
      -1,    64,    63,    -1,    70,    -1,    26,    37,     3,    66,
     204,    74,     8,    -1,    46,    -1,    34,    -1,    77,    -1,
      32,    37,     3,    -1,    15,    44,     3,    80,    68,     3,
      -1,    35,    68,     3,    -1,    69,   209,   146,    -1,    10,
      -1,    10,   115,     3,    -1,    83,    68,     3,    -1,    61,
      44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   127,   127,   128,   129,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   158,   159,   163,   164,   168,
     169,   179,   190,   191,   198,   214,   215,   219,   220,   223,
     225,   226,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   243,   244,   247,   249,   253,   257,   258,   262,
     267,   274,   282,   290,   299,   304,   309,   314,   319,   324,
     329,   334,   335,   336,   337,   342,   347,   352,   360,   361,
     366,   372,   378,   384,   393,   394,   405,   406,   410,   416,
     422,   424,   428,   435,   437,   441,   447,   449,   453,   457,
     464,   465,   469,   470,   471,   474,   476,   480,   485,   492,
     494,   498,   502,   503,   507,   512,   517,   523,   528,   536,
     541,   548,   558,   559,   560,   561,   562,   563,   564,   565,
     566,   567,   568,   569,   570,   571,   572,   573,   574,   575,
     576,   577,   578,   579,   580,   581,   582,   583,   584,   585,
     589,   590,   591,   592,   593,   597,   598,   602,   603,   607,
     608,   614,   617,   619,   623,   624,   625,   626,   627,   632,
     642,   643,   644,   645,   646,   650,   655,   660,   665,   666,
     670,   675,   683,   684,   688,   689,   690,   704,   705,   706,
     710,   711,   717,   725,   726,   729,   731,   735,   736,   740,
     741,   745,   749,   750,   754,   755,   756,   757,   758,   764,
     772,   786,   794,   798,   805,   806,   813,   823,   829,   831,
     835,   840,   844,   851,   853,   857,   858,   864,   872,   873,
     879,   885,   893,   894,   898,   902,   906,   910,   920,   926,
     927,   931,   935,   936,   940,   944,   951,   958,   964,   965,
     966,   970,   971,   972,   973,   979,   990,   991,   992,   996,
    1007,  1019,  1030,  1038,  1039,  1048,  1059
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
  "where_expr", "where_item", "expr_ident", "const_int", "const_float", 
  "const_list", "opt_group_clause", "group_clause", 
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
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   134,   134,
     134,   134,   134,   134,   135,   135,   136,   136,   137,   137,
     138,   138,   139,   140,   140,   141,   142,   142,   143,   143,
     144,   144,   145,   145,   145,   146,   146,   147,   147,   148,
     148,   149,   150,   150,   151,   151,   151,   151,   151,   152,
     152,   153,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     155,   155,   155,   155,   155,   156,   156,   157,   157,   158,
     158,   159,   160,   160,   161,   161,   161,   161,   161,   161,
     162,   162,   162,   162,   162,   163,   163,   163,   163,   163,
     164,   164,   165,   165,   166,   166,   166,   167,   167,   167,
     168,   168,   169,   170,   170,   171,   171,   172,   172,   173,
     173,   174,   175,   175,   176,   176,   176,   176,   176,   177,
     177,   178,   179,   179,   180,   180,   181,   181,   182,   182,
     183,   183,   184,   185,   185,   186,   186,   187,   188,   188,
     189,   190,   191,   191,   192,   192,   192,   192,   193,   194,
     194,   195,   196,   196,   197,   198,   199,   200,   201,   201,
     201,   202,   202,   202,   202,   203,   204,   204,   204,   205,
     206,   207,   208,   209,   209,   210,   211
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
      11,     0,     0,     2,    10,     1,     3,     1,     2,     0,
       1,     2,     1,     4,     4,     4,     4,     4,     4,     3,
       3,     5,     1,     3,     0,     1,     2,     1,     3,     4,
       3,     3,     5,     6,     3,     4,     5,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     3,     3,     1,     1,
       4,     3,     3,     1,     1,     2,     1,     2,     1,     3,
       0,     1,     3,     0,     1,     5,     0,     1,     3,     5,
       1,     3,     1,     2,     2,     0,     1,     2,     4,     0,
       1,     2,     1,     3,     3,     3,     5,     6,     3,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       4,     4,     3,     6,     6,     1,     3,     1,     1,     3,
       5,     2,     0,     2,     1,     2,     2,     3,     4,     4,
       1,     1,     1,     1,     1,     4,     4,     4,     3,     4,
       7,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     6,     1,     1,     0,     3,     1,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     3,     2,     7,
       9,     6,     1,     3,     1,     3,     1,     3,     0,     2,
       1,     3,     3,     0,     1,     1,     1,     3,     1,     1,
       3,     6,     1,     3,     3,     3,     5,     4,     4,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     3,
       6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   190,     0,   187,     0,     0,   229,   228,     0,
       0,   193,     0,   194,   188,     0,   248,   248,     0,     0,
       0,     0,     2,     3,    25,    27,    29,    28,     7,     8,
       9,   189,     5,     0,     6,    10,    11,     0,    12,    13,
      14,    15,    16,    21,    17,    18,    19,    20,    22,    23,
      24,     0,     0,     0,     0,     0,     0,     0,   122,   123,
     125,   126,   127,   263,     0,     0,     0,     0,   124,     0,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
      35,    39,    42,   149,   105,     0,     0,   249,     0,   250,
       0,     0,     0,   245,   249,   162,   162,   162,   164,   161,
       0,   191,     0,    52,     0,     1,     4,     0,   162,     0,
       0,     0,     0,   259,   261,   266,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   128,
     129,     0,     0,     0,     0,     0,    40,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   262,   106,     0,
       0,     0,     0,   170,   173,   174,   172,   171,   178,     0,
       0,     0,   162,   246,     0,   166,   165,   230,   239,   265,
       0,     0,     0,     0,    26,   195,   227,     0,    84,    86,
     206,     0,     0,   204,   205,   214,   218,   212,     0,     0,
     158,   152,   157,     0,   155,   264,     0,     0,     0,    50,
       0,     0,     0,     0,     0,    49,     0,     0,   147,     0,
       0,   148,    31,    54,    36,    41,   139,   140,   146,   145,
     137,   136,   143,   144,   134,   135,   142,   141,   130,   131,
     132,   133,   138,   107,   182,   183,   185,   177,   184,     0,
     186,   176,   175,   179,     0,     0,     0,   162,   162,   167,
     163,     0,   238,   240,     0,     0,   232,    53,     0,     0,
       0,    85,    87,   216,   208,    88,     0,     0,     0,     0,
     257,   256,   258,     0,     0,   150,     0,    43,     0,    48,
      47,   151,    44,     0,    45,     0,    46,     0,     0,   159,
       0,     0,     0,    90,    55,     0,   181,     0,     0,   169,
     168,     0,   241,   242,     0,     0,   109,    78,    79,     0,
       0,    83,     0,   197,     0,     0,   260,   207,     0,   215,
       0,   214,   213,   219,   220,   211,     0,     0,     0,   156,
      51,     0,     0,     0,     0,     0,     0,    56,    57,     0,
       0,    93,    91,   108,     0,     0,     0,   254,   247,     0,
       0,     0,   234,   235,   233,     0,   231,   110,     0,     0,
       0,   196,     0,     0,   192,   199,    89,   217,   224,     0,
       0,   255,     0,   209,   154,   153,   160,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    94,   180,   252,   251,   253,   244,   243,
     237,     0,     0,   111,   112,     0,    81,    82,   198,     0,
       0,   202,     0,   225,   226,   222,   223,   221,     0,     0,
       0,     0,    58,     0,     0,    64,     0,    60,    71,    61,
      72,    68,    74,    67,    73,    69,    76,    70,    77,     0,
      92,     0,     0,   105,    97,   236,     0,     0,    80,   201,
       0,   200,   210,     0,    59,     0,     0,     0,    65,     0,
       0,     0,   109,   114,   115,   118,     0,   113,   203,   102,
      32,   100,    66,     0,    75,    62,     0,     0,     0,    98,
      34,     0,     0,     0,   119,   103,   104,     0,     0,    30,
      63,    95,     0,     0,     0,   116,     0,    33,   101,    99,
     117,   121,   120
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   301,   499,    26,    79,
      80,   138,    81,   223,   303,   304,   347,   348,   479,   275,
     194,   276,   351,   352,   402,   403,   453,   454,   480,   481,
     157,   158,   366,   367,   413,   414,   493,   494,    82,    83,
     203,   204,   133,    27,   175,    99,   168,    28,    29,   251,
     252,    30,    31,    32,    33,   269,   324,   374,   375,   420,
     195,    34,    35,   196,   197,   277,   279,   333,   334,   379,
     425,    36,    37,    38,    39,   265,   266,    40,   262,   263,
     312,   313,    41,    42,    43,    90,   358,    44,   283,    45,
      46,    47,    48,    84,    49,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -424
static const short yypact[] =
{
     814,    22,  -424,    60,  -424,   124,   131,  -424,  -424,   149,
     127,  -424,   170,  -424,  -424,   122,   773,   649,   109,   129,
     213,   227,  -424,   114,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,   184,  -424,  -424,  -424,   214,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,   231,   128,   241,   213,   244,   247,   248,   144,  -424,
    -424,  -424,  -424,   143,   152,   172,   180,   187,  -424,   199,
     205,   207,   226,   228,   289,  -424,   289,   289,   342,   -21,
    -424,    94,   612,  -424,   273,   251,   255,   134,   202,  -424,
     267,    24,   286,  -424,  -424,   309,   309,   309,  -424,  -424,
     278,  -424,   366,  -424,   -53,  -424,   105,   371,   309,   298,
      32,   322,   -68,  -424,  -424,  -424,   146,   386,   289,   -17,
     279,   289,   263,   289,   289,   289,   280,   283,   285,  -424,
    -424,   426,   301,   -10,     1,   236,  -424,   395,  -424,   289,
     289,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   289,   289,   289,   289,   396,  -424,  -424,    90,
     202,   308,   310,  -424,  -424,  -424,  -424,  -424,  -424,   359,
     332,   333,   309,  -424,   402,  -424,  -424,  -424,   340,  -424,
     408,   409,   236,    13,  -424,   324,  -424,   363,  -424,  -424,
    -424,    25,    18,  -424,  -424,  -424,   323,  -424,   110,   410,
    -424,  -424,   612,    69,  -424,  -424,   449,   448,   347,  -424,
     477,   101,   321,   344,   503,  -424,   289,   289,  -424,    15,
     450,  -424,  -424,    79,  -424,  -424,  -424,  -424,   645,   663,
     696,   722,   281,   281,   224,   224,   224,   224,   217,   217,
    -424,  -424,  -424,   348,  -424,  -424,  -424,  -424,  -424,   454,
    -424,  -424,  -424,  -424,    31,   351,   413,   309,   309,  -424,
    -424,   460,  -424,  -424,   383,    86,  -424,  -424,   316,   397,
     478,  -424,  -424,  -424,  -424,  -424,   108,   125,    32,   374,
    -424,  -424,  -424,   412,     3,  -424,   263,  -424,   376,  -424,
    -424,  -424,  -424,   289,  -424,   289,  -424,   372,   398,  -424,
     392,   420,   154,   467,  -424,   502,  -424,    15,   139,  -424,
    -424,   415,   417,  -424,    56,   408,   453,  -424,  -424,   403,
     404,  -424,   405,  -424,   153,   406,  -424,  -424,    15,  -424,
     509,   179,  -424,   423,  -424,  -424,   528,   428,    15,  -424,
    -424,   530,   554,    15,   236,   429,   446,   464,  -424,   203,
     520,   471,  -424,  -424,   196,     4,   495,  -424,  -424,   555,
     460,    19,  -424,  -424,  -424,   561,  -424,  -424,   461,   455,
     456,  -424,   316,    50,   472,  -424,  -424,  -424,  -424,   133,
      50,  -424,    15,  -424,  -424,  -424,  -424,   -20,   508,   579,
     154,    36,    -2,    36,    36,    36,    36,    36,    36,   546,
     316,   551,   531,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,   223,   496,   481,  -424,   483,  -424,  -424,  -424,    30,
     242,  -424,   406,  -424,  -424,  -424,   598,  -424,   250,   213,
     593,   504,  -424,   523,   524,  -424,    15,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,    10,
    -424,   553,   599,   273,  -424,  -424,     9,   561,  -424,  -424,
      50,  -424,  -424,   316,  -424,    15,     2,   252,  -424,    15,
     600,   271,   453,   510,  -424,  -424,   618,  -424,  -424,    29,
     -22,  -424,  -424,   631,  -424,  -424,   256,   316,   529,   526,
    -424,   632,   548,   260,  -424,  -424,  -424,   637,   316,  -424,
    -424,   526,   534,   552,    15,  -424,   618,  -424,  -424,  -424,
    -424,  -424,  -424
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -424,  -424,  -424,  -424,   556,  -424,  -424,  -424,   365,   320,
     532,  -424,  -424,    40,  -424,   407,  -424,   284,  -266,  -110,
    -281,  -296,  -424,  -424,  -424,  -424,  -424,  -424,  -423,   171,
     220,  -424,   206,  -424,  -424,   218,  -424,   173,   -71,  -424,
     558,   391,  -424,  -424,   -87,  -424,   521,  -424,  -424,   430,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,   261,  -424,
    -277,  -424,  -424,  -424,   411,  -424,  -424,  -424,   302,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,   370,  -424,  -424,  -424,
    -424,   326,  -424,  -424,  -424,   671,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -224
static const short yytable[] =
{
     193,   331,   323,   129,   103,   130,   131,   435,   189,   176,
     177,   354,   473,   207,   474,   134,   429,   475,   180,   468,
     188,   186,   199,   188,   188,    91,   273,   170,   405,   497,
     271,   272,   171,   363,   244,   188,   349,   188,   189,   245,
     190,   188,   189,   495,   181,   202,   337,   206,   489,   250,
     210,   202,   212,   213,   214,   188,   189,   496,   190,   181,
     104,   188,   189,    52,   501,   411,    51,    95,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   259,   428,   208,   406,    96,
     498,   135,   135,   244,   112,   188,   421,   136,   245,   338,
     172,    98,   220,   426,   221,   483,   418,   137,   436,   299,
     434,   222,   438,   440,   442,   444,   446,   448,   249,   476,
     469,   249,   249,   246,   349,    58,    59,    60,    61,   274,
     410,    62,    63,   249,   450,   191,   423,   161,    64,   191,
     467,   274,   192,   162,   280,   297,   298,    65,   247,    58,
      59,    60,    61,   191,   200,    62,   281,   317,   318,   191,
     419,    53,    66,    67,    68,    69,   361,    54,   193,   302,
     309,   310,   248,   486,   182,    70,   302,    71,   183,   319,
     285,   286,  -223,   478,   424,   484,    55,   282,    68,    69,
     101,   181,   378,   249,   320,    56,   321,   102,   315,   127,
      72,   128,   355,   356,   362,   163,   346,   164,   165,   357,
     166,    73,   291,   286,    57,   202,   103,   108,   376,   327,
     328,   391,   341,   106,   342,    74,    75,   105,   383,    76,
    -223,   107,    77,   386,   109,    78,   329,   330,   110,    58,
      59,    60,    61,   322,   111,    62,   392,   113,   139,    74,
     114,   115,    64,    76,   116,   139,    77,   201,   117,    78,
     167,    65,   118,   193,   371,   372,    58,    59,    60,    61,
     193,   200,    62,   140,   317,   318,    66,    67,    68,    69,
     140,   433,   119,   437,   439,   441,   443,   445,   447,    70,
     120,    71,    58,    59,    60,    61,   319,   121,    62,   393,
     394,   395,   396,   397,   398,    68,    69,   404,   328,   122,
     399,   320,   139,   321,    72,   123,   127,   124,   128,   317,
     318,   153,   154,   155,   156,    73,   151,   152,   153,   154,
     155,    68,    69,   488,   455,   328,   125,   140,   126,    74,
      75,   319,   127,    76,   128,   132,    77,   159,   169,    78,
     193,   160,   139,   459,   460,   482,   320,   173,   321,   174,
     322,   462,   328,   485,   328,   178,    74,   500,   328,   179,
      76,   505,   506,    77,   185,   139,    78,   140,   187,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   198,   205,
     209,   215,    74,   216,   511,   217,    76,   219,   225,    77,
     140,   243,    78,   139,   254,   322,   255,   256,   257,   258,
     260,   264,   267,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   140,   139,
     261,   270,   292,   293,   268,   278,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   288,   284,   300,   140,   294,   295,   139,   289,   271,
     305,   307,   308,   311,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   314,
     139,   326,   140,   325,   293,   335,   336,   340,   343,   344,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   140,   350,   353,   139,   360,
     295,   359,   365,   368,   369,   370,   373,   377,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   140,   139,   380,   381,   218,   382,   400,
     388,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   389,   390,   407,   140,
     287,   139,   401,   408,   412,   415,   416,   417,   430,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   422,   139,   140,   431,   290,   449,
     451,   452,   456,   457,   458,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     140,   378,   463,   470,   296,   464,   465,   466,   471,   487,
     491,   492,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   272,   498,   502,
     503,   384,   507,   139,   504,   509,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,    91,   184,   510,   387,   385,   345,   224,   140,   508,
      92,    93,   316,   472,   432,   477,   139,   339,   490,   512,
     211,   253,   427,   461,   306,   364,   409,    94,   100,   332,
       0,     0,     0,     0,   139,     0,     0,     0,     0,     0,
       0,   140,     0,    95,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   140,
       0,    89,     0,     0,     0,    96,     0,   139,    97,     0,
       0,     0,     0,     0,     0,     0,     0,    98,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   140,   139,     0,     0,     0,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
       0,     0,     0,     0,     0,     0,    85,     0,   140,     0,
       0,     0,     0,    86,     0,     0,     0,     0,     0,     0,
       0,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,     0,     0,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,     0,     0,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,     1,
      88,     2,     0,     0,     3,     0,     0,     4,     0,     0,
       5,     6,     7,     8,     0,    89,     9,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,    13,
       0,    14,     0,    15,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,     0,     0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     110,   278,   268,    74,     3,    76,    77,     9,     6,    96,
      97,   307,     3,    30,     5,    36,    36,     8,    71,     9,
       5,   108,    90,     5,     5,    12,     8,     3,    24,    51,
       5,     6,     8,   314,     3,     5,   302,     5,     6,     8,
       8,     5,     6,    14,   112,   116,    43,   118,   471,   159,
     121,   122,   123,   124,   125,     5,     6,    28,     8,   112,
      20,     5,     6,     3,   487,   361,    44,    54,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   172,   382,   104,    84,    76,
     112,   112,   112,     3,    54,     5,   373,     3,     8,    96,
      76,    88,   112,   380,   114,   103,   372,    13,   110,   219,
     391,   110,   393,   394,   395,   396,   397,   398,   103,   110,
     110,   103,   103,    33,   390,     3,     4,     5,     6,   111,
     111,     9,    10,   103,   400,   103,     3,     3,    16,   103,
     436,   111,   110,     9,    34,   216,   217,    25,    58,     3,
       4,     5,     6,   103,     8,     9,    46,     3,     4,   103,
     110,    37,    40,    41,    42,    43,   110,    36,   278,    90,
     257,   258,    82,   469,    69,    53,    90,    55,    73,    25,
     111,   112,     3,   460,    51,   466,    37,    77,    42,    43,
      81,   112,    13,   103,    40,    68,    42,    68,   112,    53,
      78,    55,    63,    64,   314,     3,    52,     5,     6,    70,
       8,    89,   111,   112,    44,   286,     3,     3,   328,   111,
     112,    18,   293,   109,   295,   103,   104,     0,   338,   107,
      51,    47,   110,   343,     3,   113,   111,   112,   110,     3,
       4,     5,     6,    89,     3,     9,    43,     3,    31,   103,
       3,     3,    16,   107,   110,    31,   110,   111,   115,   113,
      58,    25,   110,   373,   111,   112,     3,     4,     5,     6,
     380,     8,     9,    56,     3,     4,    40,    41,    42,    43,
      56,   391,   110,   393,   394,   395,   396,   397,   398,    53,
     110,    55,     3,     4,     5,     6,    25,   110,     9,    96,
      97,    98,    99,   100,   101,    42,    43,   111,   112,   110,
     107,    40,    31,    42,    78,   110,    53,   110,    55,     3,
       4,   104,   105,   106,    51,    89,   102,   103,   104,   105,
     106,    42,    43,    62,   111,   112,   110,    56,   110,   103,
     104,    25,    53,   107,    55,     3,   110,    96,    81,   113,
     460,    96,    31,   111,   112,   465,    40,    71,    42,    50,
      89,   111,   112,   111,   112,    87,   103,   111,   112,     3,
     107,   111,   112,   110,     3,    31,   113,    56,    80,    98,
      99,   100,   101,   102,   103,   104,   105,   106,    66,     3,
     111,   111,   103,   110,   504,   110,   107,    96,     3,   110,
      56,     5,   113,    31,    96,    89,    96,    48,    76,    76,
       8,     3,     3,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,    56,    31,
      90,    68,   111,   112,   110,   112,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,     3,    42,     3,    56,   111,   112,    31,   111,     5,
     112,   110,    49,     3,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    96,
      31,     3,    56,    86,   112,   111,    74,   111,    96,    69,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    56,    39,     5,    31,    92,
     112,    96,    59,   110,   110,   110,   110,     8,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,    56,    31,   112,     8,   111,   110,    19,
     111,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   110,    93,    63,    56,
     111,    31,    91,     8,     3,   104,   111,   111,    60,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   112,    31,    56,     8,   111,    43,
      39,    60,    96,   112,   111,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      56,    13,    19,    60,   111,   111,    93,    93,    19,    19,
     110,     3,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,     6,   112,   110,
       8,   111,     5,    31,    96,   111,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    12,   106,   111,   344,   111,   301,   135,    56,   498,
      21,    22,   265,   453,   390,   457,    31,   286,   472,   506,
     122,   160,   380,   422,   254,   315,   360,    38,    17,   278,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    54,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    56,
      -1,    72,    -1,    -1,    -1,    76,    -1,    31,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,    56,    31,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    56,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    15,
      57,    17,    -1,    -1,    20,    -1,    -1,    23,    -1,    -1,
      26,    27,    28,    29,    -1,    72,    32,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,    65,
      -1,    67,    -1,    69,    -1,    71,    -1,    73,    -1,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    -1,    85
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    61,    65,    67,    69,    71,    73,    75,    83,
      85,   117,   118,   119,   120,   121,   124,   159,   163,   164,
     167,   168,   169,   170,   177,   178,   187,   188,   189,   190,
     193,   198,   199,   200,   203,   205,   206,   207,   208,   210,
     211,    44,     3,    37,    36,    37,    68,    44,     3,     4,
       5,     6,     9,    10,    16,    25,    40,    41,    42,    43,
      53,    55,    78,    89,   103,   104,   107,   110,   113,   125,
     126,   128,   154,   155,   209,     3,    10,    38,    57,    72,
     201,    12,    21,    22,    38,    54,    76,    79,    88,   161,
     201,    81,    68,     3,   129,     0,   109,    47,     3,     3,
     110,     3,   129,     3,     3,     3,   110,   115,   110,   110,
     110,   110,   110,   110,   110,   110,   110,    53,    55,   154,
     154,   154,     3,   158,    36,   112,     3,    13,   127,    31,
      56,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    51,   146,   147,    96,
      96,     3,     9,     3,     5,     6,     8,    58,   162,    81,
       3,     8,    76,    71,    50,   160,   160,   160,    87,     3,
      71,   112,    69,    73,   120,     3,   160,    80,     5,     6,
       8,   103,   110,   135,   136,   176,   179,   180,    66,    90,
       8,   111,   154,   156,   157,     3,   154,    30,   104,   111,
     154,   156,   154,   154,   154,   111,   110,   110,   111,    96,
     112,   114,   110,   129,   126,     3,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,     5,     3,     8,    33,    58,    82,   103,
     135,   165,   166,   162,    96,    96,    48,    76,    76,   160,
       8,    90,   194,   195,     3,   191,   192,     3,   110,   171,
      68,     5,     6,     8,   111,   135,   137,   181,   112,   182,
      34,    46,    77,   204,    42,   111,   112,   111,     3,   111,
     111,   111,   111,   112,   111,   112,   111,   154,   154,   135,
       3,   122,    90,   130,   131,   112,   165,   110,    49,   160,
     160,     3,   196,   197,    96,   112,   131,     3,     4,    25,
      40,    42,    89,   134,   172,    86,     3,   111,   112,   111,
     112,   176,   180,   183,   184,   111,    74,    43,    96,   157,
     111,   154,   154,    96,    69,   124,    52,   132,   133,   134,
      39,   138,   139,     5,   137,    63,    64,    70,   202,    96,
      92,   110,   135,   136,   192,    59,   148,   149,   110,   110,
     110,   111,   112,   110,   173,   174,   135,     8,    13,   185,
     112,     8,   110,   135,   111,   111,   135,   125,   111,   110,
      93,    18,    43,    96,    97,    98,    99,   100,   101,   107,
      19,    91,   140,   141,   111,    24,    84,    63,     8,   197,
     111,   137,     3,   150,   151,   104,   111,   111,   134,   110,
     175,   176,   112,     3,    51,   186,   176,   184,   137,    36,
      60,     8,   133,   135,   136,     9,   110,   135,   136,   135,
     136,   135,   136,   135,   136,   135,   136,   135,   136,    43,
     134,    39,    60,   142,   143,   111,    96,   112,   111,   111,
     112,   174,   111,    19,   111,    93,    93,   137,     9,   110,
      60,    19,   146,     3,     5,     8,   110,   151,   176,   134,
     144,   145,   135,   103,   136,   111,   137,    19,    62,   144,
     148,   110,     3,   152,   153,    14,    28,    51,   112,   123,
     111,   144,   110,     8,    96,   111,   112,     5,   145,   111,
     111,   135,   153
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

  case 59:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 60:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 61:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 62:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 63:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 64:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 65:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 66:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 67:

    {
			if ( !pParser->AddIntFilterGTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1 ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->AddIntFilterLTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddIntFilterGTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddIntFilterLTE ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 75:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 76:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 80:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 81:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 82:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 83:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 84:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 85:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 86:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 87:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 88:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 89:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 92:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 95:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 98:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 99:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 101:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 103:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 104:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 107:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 108:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 114:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 115:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 116:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 117:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 118:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 119:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 120:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 121:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 123:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 124:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 128:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 129:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 150:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 151:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 152:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 153:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 154:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 159:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 160:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 163:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 164:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 165:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 166:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS; ;}
    break;

  case 168:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 169:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 175:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 176:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 177:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 178:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 179:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 180:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 181:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 184:

    { yyval.m_iValue = 1; ;}
    break;

  case 185:

    { yyval.m_iValue = 0; ;}
    break;

  case 186:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 187:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 188:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 192:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 193:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 194:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 197:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 198:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 201:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 202:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 203:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 204:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 205:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 206:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 207:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 208:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 209:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 210:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 211:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 212:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 213:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 215:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 216:

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

  case 217:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 220:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 222:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 226:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 227:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 230:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 231:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 234:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 235:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 236:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 237:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 238:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 245:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 246:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 247:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 255:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 256:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 257:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 258:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 259:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 260:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 261:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 262:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 264:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 265:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 266:

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

