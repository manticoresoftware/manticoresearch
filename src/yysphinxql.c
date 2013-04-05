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
#define YYLAST   1182

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  124
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  104
/* YYNRULES -- Number of rules. */
#define YYNRULES  293
/* YYNRULES -- Number of states. */
#define YYNSTATES  569

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
      62,    64,    74,    83,    84,    87,    89,    93,    95,    97,
      99,   100,   104,   105,   108,   113,   124,   126,   130,   132,
     135,   136,   138,   141,   143,   148,   153,   158,   163,   168,
     173,   177,   183,   185,   189,   190,   192,   195,   197,   201,
     205,   210,   214,   218,   224,   231,   235,   240,   246,   250,
     254,   258,   262,   266,   268,   274,   278,   282,   286,   290,
     294,   298,   302,   304,   306,   311,   315,   319,   321,   323,
     326,   328,   331,   333,   337,   338,   342,   344,   348,   349,
     351,   357,   358,   360,   364,   370,   372,   376,   378,   381,
     384,   385,   387,   390,   395,   396,   398,   401,   403,   407,
     411,   415,   421,   428,   432,   434,   438,   442,   444,   446,
     448,   450,   452,   454,   457,   460,   464,   468,   472,   476,
     480,   484,   488,   492,   496,   500,   504,   508,   512,   516,
     520,   524,   528,   532,   536,   538,   543,   548,   553,   558,
     563,   567,   574,   581,   585,   587,   591,   593,   595,   599,
     605,   608,   609,   612,   614,   617,   620,   624,   626,   631,
     636,   640,   642,   644,   646,   648,   650,   652,   656,   661,
     666,   671,   675,   680,   688,   694,   696,   698,   700,   702,
     704,   706,   708,   710,   712,   715,   722,   724,   726,   727,
     731,   733,   737,   739,   743,   747,   749,   753,   755,   757,
     759,   763,   766,   774,   784,   791,   793,   797,   799,   803,
     805,   809,   810,   813,   815,   819,   823,   824,   826,   828,
     830,   834,   836,   838,   842,   849,   851,   855,   859,   863,
     869,   874,   876,   878,   880,   888,   893,   894,   896,   899,
     901,   905,   909,   912,   916,   923,   924,   926,   928,   931,
     934,   937,   939,   947,   949,   951,   953,   957,   964,   968,
     972,   974,   978,   982
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     125,     0,    -1,   126,    -1,   127,    -1,   127,   117,    -1,
     183,    -1,   191,    -1,   177,    -1,   178,    -1,   181,    -1,
     192,    -1,   201,    -1,   203,    -1,   204,    -1,   209,    -1,
     214,    -1,   215,    -1,   219,    -1,   221,    -1,   222,    -1,
     223,    -1,   216,    -1,   224,    -1,   226,    -1,   227,    -1,
     208,    -1,   128,    -1,   127,   117,   128,    -1,   129,    -1,
     172,    -1,   136,    -1,    76,   137,    41,   118,   133,   136,
     119,   134,   135,    -1,    76,     3,   118,   118,   136,   119,
     130,   119,    -1,    -1,   120,   131,    -1,   132,    -1,   131,
     120,   132,    -1,     3,    -1,     5,    -1,    47,    -1,    -1,
      66,    23,   157,    -1,    -1,    57,     5,    -1,    57,     5,
     120,     5,    -1,    76,   137,    41,   141,   142,   151,   153,
     155,   159,   161,    -1,   138,    -1,   137,   120,   138,    -1,
     112,    -1,   140,   139,    -1,    -1,     3,    -1,    16,     3,
      -1,   167,    -1,    19,   118,   167,   119,    -1,    59,   118,
     167,   119,    -1,    61,   118,   167,   119,    -1,    85,   118,
     167,   119,    -1,    46,   118,   167,   119,    -1,    30,   118,
     112,   119,    -1,    45,   118,   119,    -1,    30,   118,    35,
       3,   119,    -1,     3,    -1,   141,   120,     3,    -1,    -1,
     143,    -1,    98,   144,    -1,   145,    -1,   144,   101,   144,
      -1,   118,   144,   119,    -1,    58,   118,     8,   119,    -1,
     147,   104,   148,    -1,   147,   105,   148,    -1,   147,    48,
     118,   150,   119,    -1,   147,   115,    48,   118,   150,   119,
      -1,   147,    48,     9,    -1,   147,   115,    48,     9,    -1,
     147,    21,   148,   101,   148,    -1,   147,   107,   148,    -1,
     147,   106,   148,    -1,   147,   108,   148,    -1,   147,   109,
     148,    -1,   147,   104,   149,    -1,   146,    -1,   147,    21,
     149,   101,   149,    -1,   147,   107,   149,    -1,   147,   106,
     149,    -1,   147,   108,   149,    -1,   147,   109,   149,    -1,
     147,   104,     8,    -1,   147,   105,     8,    -1,   147,   105,
     149,    -1,     3,    -1,     4,    -1,    30,   118,   112,   119,
      -1,    45,   118,   119,    -1,    97,   118,   119,    -1,    47,
      -1,     5,    -1,   111,     5,    -1,     6,    -1,   111,     6,
      -1,   148,    -1,   150,   120,   148,    -1,    -1,    44,    23,
     152,    -1,   147,    -1,   152,   120,   147,    -1,    -1,   154,
      -1,    99,    44,    66,    23,   157,    -1,    -1,   156,    -1,
      66,    23,   157,    -1,    66,    23,    69,   118,   119,    -1,
     158,    -1,   157,   120,   158,    -1,   147,    -1,   147,    17,
      -1,   147,    33,    -1,    -1,   160,    -1,    57,     5,    -1,
      57,     5,   120,     5,    -1,    -1,   162,    -1,    65,   163,
      -1,   164,    -1,   163,   120,   164,    -1,     3,   104,     3,
      -1,     3,   104,     5,    -1,     3,   104,   118,   165,   119,
      -1,     3,   104,     3,   118,     8,   119,    -1,     3,   104,
       8,    -1,   166,    -1,   165,   120,   166,    -1,     3,   104,
     148,    -1,     3,    -1,     4,    -1,    47,    -1,     5,    -1,
       6,    -1,     9,    -1,   111,   167,    -1,   115,   167,    -1,
     167,   110,   167,    -1,   167,   111,   167,    -1,   167,   112,
     167,    -1,   167,   113,   167,    -1,   167,   106,   167,    -1,
     167,   107,   167,    -1,   167,   103,   167,    -1,   167,   102,
     167,    -1,   167,   114,   167,    -1,   167,    36,   167,    -1,
     167,    62,   167,    -1,   167,   109,   167,    -1,   167,   108,
     167,    -1,   167,   104,   167,    -1,   167,   105,   167,    -1,
     167,   101,   167,    -1,   167,   100,   167,    -1,   118,   167,
     119,    -1,   121,   171,   122,    -1,   168,    -1,     3,   118,
     169,   119,    -1,    48,   118,   169,   119,    -1,    52,   118,
     169,   119,    -1,    22,   118,   169,   119,    -1,    39,   118,
     169,   119,    -1,     3,   118,   119,    -1,    61,   118,   167,
     120,   167,   119,    -1,    59,   118,   167,   120,   167,   119,
      -1,    97,   118,   119,    -1,   170,    -1,   169,   120,   170,
      -1,   167,    -1,     8,    -1,     3,   104,   148,    -1,   171,
     120,     3,   104,   148,    -1,    80,   174,    -1,    -1,    56,
       8,    -1,    96,    -1,    83,   173,    -1,    60,   173,    -1,
      14,    83,   173,    -1,    68,    -1,    14,     8,    83,   173,
      -1,    14,     3,    83,   173,    -1,    49,     3,    83,    -1,
       3,    -1,    64,    -1,     8,    -1,     5,    -1,     6,    -1,
     175,    -1,   176,   111,   175,    -1,    78,     3,   104,   180,
      -1,    78,     3,   104,   179,    -1,    78,     3,   104,    64,
      -1,    78,    63,   176,    -1,    78,    10,   104,   176,    -1,
      78,    43,     9,   104,   118,   150,   119,    -1,    78,    43,
       3,   104,   179,    -1,     3,    -1,     8,    -1,    90,    -1,
      38,    -1,   148,    -1,    28,    -1,    74,    -1,   182,    -1,
      20,    -1,    82,    89,    -1,   184,    53,     3,   185,    94,
     187,    -1,    50,    -1,    72,    -1,    -1,   118,   186,   119,
      -1,   147,    -1,   186,   120,   147,    -1,   188,    -1,   187,
     120,   188,    -1,   118,   189,   119,    -1,   190,    -1,   189,
     120,   190,    -1,   148,    -1,   149,    -1,     8,    -1,   118,
     150,   119,    -1,   118,   119,    -1,    32,    41,   141,    98,
      47,   104,   148,    -1,    32,    41,   141,    98,    47,    48,
     118,   150,   119,    -1,    24,     3,   118,   193,   196,   119,
      -1,   194,    -1,   193,   120,   194,    -1,   190,    -1,   118,
     195,   119,    -1,     8,    -1,   195,   120,     8,    -1,    -1,
     120,   197,    -1,   198,    -1,   197,   120,   198,    -1,   190,
     199,   200,    -1,    -1,    16,    -1,     3,    -1,    57,    -1,
     202,     3,   173,    -1,    34,    -1,    33,    -1,    80,    87,
     173,    -1,    93,   141,    78,   205,   143,   161,    -1,   206,
      -1,   205,   120,   206,    -1,     3,   104,   148,    -1,     3,
     104,   149,    -1,     3,   104,   118,   150,   119,    -1,     3,
     104,   118,   119,    -1,    52,    -1,    22,    -1,    39,    -1,
      15,    86,     3,    13,    27,     3,   207,    -1,    80,   217,
      95,   210,    -1,    -1,   211,    -1,    98,   212,    -1,   213,
      -1,   212,   100,   213,    -1,     3,   104,     8,    -1,    80,
      26,    -1,    80,    25,    78,    -1,    78,   217,    89,    54,
      55,   218,    -1,    -1,    43,    -1,    79,    -1,    70,    92,
      -1,    70,    29,    -1,    71,    70,    -1,    77,    -1,    31,
      42,     3,    73,   220,    81,     8,    -1,    51,    -1,    39,
      -1,    84,    -1,    37,    42,     3,    -1,    18,    49,     3,
      88,    75,     3,    -1,    40,    75,     3,    -1,    76,   225,
     159,    -1,    10,    -1,    10,   123,     3,    -1,    91,    75,
       3,    -1,    67,    49,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   135,   135,   136,   137,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   167,   168,   172,   173,
     177,   178,   183,   190,   192,   196,   200,   207,   208,   209,
     213,   226,   234,   236,   241,   250,   266,   267,   271,   272,
     275,   277,   278,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   294,   295,   298,   300,   304,   308,   309,   310,
     314,   319,   326,   334,   342,   351,   356,   361,   366,   371,
     376,   381,   386,   391,   396,   401,   406,   411,   416,   421,
     426,   434,   438,   439,   444,   450,   456,   462,   471,   472,
     483,   484,   488,   494,   500,   502,   506,   510,   516,   518,
     522,   533,   535,   539,   543,   550,   551,   555,   556,   557,
     560,   562,   566,   571,   578,   580,   584,   588,   589,   593,
     598,   603,   609,   614,   622,   627,   634,   644,   645,   646,
     647,   648,   649,   650,   651,   652,   653,   654,   655,   656,
     657,   658,   659,   660,   661,   662,   663,   664,   665,   666,
     667,   668,   669,   670,   671,   675,   676,   677,   678,   679,
     680,   681,   682,   683,   687,   688,   692,   693,   697,   698,
     704,   707,   709,   713,   714,   715,   716,   717,   718,   723,
     728,   738,   739,   740,   741,   742,   746,   747,   751,   756,
     761,   766,   767,   771,   776,   784,   785,   789,   790,   791,
     805,   806,   807,   811,   812,   818,   826,   827,   830,   832,
     836,   837,   841,   842,   846,   850,   851,   855,   856,   857,
     858,   859,   865,   873,   887,   895,   899,   906,   907,   914,
     924,   930,   932,   936,   941,   945,   952,   954,   958,   959,
     965,   973,   974,   980,   986,   994,   995,   999,  1003,  1007,
    1011,  1021,  1022,  1023,  1027,  1040,  1046,  1047,  1051,  1055,
    1056,  1060,  1064,  1071,  1078,  1084,  1085,  1086,  1090,  1091,
    1092,  1093,  1099,  1110,  1111,  1112,  1116,  1127,  1139,  1150,
    1158,  1159,  1168,  1179
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
  "select", "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg", 
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
     129,   129,   129,   130,   130,   131,   131,   132,   132,   132,
     133,   134,   135,   135,   135,   136,   137,   137,   138,   138,
     139,   139,   139,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   141,   141,   142,   142,   143,   144,   144,   144,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   146,   147,   147,   147,   147,   147,   147,   148,   148,
     149,   149,   150,   150,   151,   151,   152,   152,   153,   153,
     154,   155,   155,   156,   156,   157,   157,   158,   158,   158,
     159,   159,   160,   160,   161,   161,   162,   163,   163,   164,
     164,   164,   164,   164,   165,   165,   166,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   169,   169,   170,   170,   171,   171,
     172,   173,   173,   174,   174,   174,   174,   174,   174,   174,
     174,   175,   175,   175,   175,   175,   176,   176,   177,   177,
     177,   177,   177,   178,   178,   179,   179,   180,   180,   180,
     181,   181,   181,   182,   182,   183,   184,   184,   185,   185,
     186,   186,   187,   187,   188,   189,   189,   190,   190,   190,
     190,   190,   191,   191,   192,   193,   193,   194,   194,   195,
     195,   196,   196,   197,   197,   198,   199,   199,   200,   200,
     201,   202,   202,   203,   204,   205,   205,   206,   206,   206,
     206,   207,   207,   207,   208,   209,   210,   210,   211,   212,
     212,   213,   214,   215,   216,   217,   217,   217,   218,   218,
     218,   218,   219,   220,   220,   220,   221,   222,   223,   224,
     225,   225,   226,   227
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     9,     8,     0,     2,     1,     3,     1,     1,     1,
       0,     3,     0,     2,     4,    10,     1,     3,     1,     2,
       0,     1,     2,     1,     4,     4,     4,     4,     4,     4,
       3,     5,     1,     3,     0,     1,     2,     1,     3,     3,
       4,     3,     3,     5,     6,     3,     4,     5,     3,     3,
       3,     3,     3,     1,     5,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     4,     3,     3,     1,     1,     2,
       1,     2,     1,     3,     0,     3,     1,     3,     0,     1,
       5,     0,     1,     3,     5,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     6,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     4,     4,     4,     4,     4,
       3,     6,     6,     3,     1,     3,     1,     1,     3,     5,
       2,     0,     2,     1,     2,     2,     3,     1,     4,     4,
       3,     1,     1,     1,     1,     1,     1,     3,     4,     4,
       4,     3,     4,     7,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     6,     1,     1,     0,     3,
       1,     3,     1,     3,     3,     1,     3,     1,     1,     1,
       3,     2,     7,     9,     6,     1,     3,     1,     3,     1,
       3,     0,     2,     1,     3,     3,     0,     1,     1,     1,
       3,     1,     1,     3,     6,     1,     3,     3,     3,     5,
       4,     1,     1,     1,     7,     4,     0,     1,     2,     1,
       3,     3,     2,     3,     6,     0,     1,     1,     2,     2,
       2,     1,     7,     1,     1,     1,     3,     6,     3,     3,
       1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   213,     0,   210,     0,     0,   252,   251,
       0,     0,   216,     0,   217,   211,     0,   275,   275,     0,
       0,     0,     0,     2,     3,    26,    28,    30,    29,     7,
       8,     9,   212,     5,     0,     6,    10,    11,     0,    12,
      13,    25,    14,    15,    16,    21,    17,    18,    19,    20,
      22,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,   137,   138,   140,   141,   142,   290,     0,     0,     0,
       0,     0,     0,   139,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,    46,    50,    53,   164,
     120,     0,     0,   276,     0,   277,     0,     0,     0,   272,
     276,     0,   181,   187,   181,   181,   183,   180,     0,   214,
       0,    62,     0,     1,     4,     0,   181,     0,     0,     0,
       0,     0,   286,   288,   293,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   137,
       0,     0,   143,   144,     0,     0,     0,     0,     0,    51,
       0,    49,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     289,   121,     0,     0,     0,     0,   191,   194,   195,   193,
     192,   196,   201,     0,     0,     0,   181,   273,     0,     0,
     185,   184,   253,   266,   292,     0,     0,     0,     0,    27,
     218,   250,     0,     0,    98,   100,   229,     0,     0,   227,
     228,   237,   241,   235,     0,     0,   177,     0,   170,   176,
       0,   174,   291,     0,     0,     0,     0,     0,    60,     0,
       0,     0,     0,     0,     0,   173,     0,     0,     0,   162,
       0,     0,   163,    40,    64,    47,    52,   154,   155,   161,
     160,   152,   151,   158,   159,   149,   150,   157,   156,   145,
     146,   147,   148,   153,   122,   205,   206,   208,   200,   207,
       0,   209,   199,   198,   202,     0,     0,     0,     0,   181,
     181,   186,   190,   182,     0,   265,   267,     0,     0,   255,
      63,     0,     0,     0,     0,    99,   101,   239,   231,   102,
       0,     0,     0,     0,   284,   283,   285,     0,     0,     0,
       0,   165,     0,    54,   168,     0,    59,   169,    58,   166,
     167,    55,     0,    56,     0,    57,     0,     0,   178,     0,
       0,     0,   104,    65,     0,   204,     0,   197,     0,   189,
     188,     0,   268,   269,     0,     0,   124,    92,    93,     0,
       0,    97,     0,   220,     0,     0,     0,   287,   230,     0,
     238,     0,   237,   236,   242,   243,   234,     0,     0,     0,
       0,    33,   175,    61,     0,     0,     0,     0,     0,     0,
      66,    67,    83,     0,     0,   108,   123,     0,     0,     0,
     281,   274,     0,     0,     0,   257,   258,   256,     0,   254,
     125,     0,     0,     0,   219,     0,     0,   215,   222,   262,
     263,   261,   264,   103,   240,   247,     0,     0,   282,     0,
     232,     0,     0,     0,   172,   171,   179,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   111,   109,   203,   279,   278,   280,   271,   270,
     260,     0,     0,   126,   127,     0,    95,    96,   221,     0,
       0,   225,     0,   248,   249,   245,   246,   244,     0,    37,
      38,    39,    34,    35,    32,     0,    42,     0,    69,    68,
       0,     0,    75,     0,    89,    71,    82,    90,    72,    91,
      79,    86,    78,    85,    80,    87,    81,    88,     0,   106,
     105,     0,     0,   120,   112,   259,     0,     0,    94,   224,
       0,   223,   233,     0,     0,     0,    31,    70,     0,     0,
       0,    76,     0,     0,     0,     0,   124,   129,   130,   133,
       0,   128,   226,    36,   117,    41,   115,    43,    77,     0,
      84,    73,     0,   107,     0,     0,   113,    45,     0,     0,
       0,   134,   118,   119,     0,     0,    74,   110,     0,     0,
       0,   131,     0,   116,    44,   114,   132,   136,   135
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,   423,   472,   473,   330,
     476,   516,    27,    85,    86,   151,    87,   244,   332,   333,
     380,   381,   382,   534,   299,   210,   300,   385,   500,   442,
     443,   503,   504,   535,   536,   170,   171,   399,   400,   453,
     454,   550,   551,   219,    89,   220,   221,   146,    28,   190,
     107,   181,   182,    29,    30,   272,   273,    31,    32,    33,
      34,   292,   354,   407,   408,   460,   211,    35,    36,   212,
     213,   301,   303,   364,   365,   416,   465,    37,    38,    39,
      40,   288,   289,   412,    41,    42,   285,   286,   342,   343,
      43,    44,    45,    96,   391,    46,   307,    47,    48,    49,
      50,    90,    51,    52
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -468
static const short yypact[] =
{
     948,   -39,    24,  -468,    85,  -468,    63,    67,  -468,  -468,
     133,    54,  -468,   137,  -468,  -468,   224,    40,   351,    93,
     117,   200,   209,  -468,    96,  -468,  -468,  -468,  -468,  -468,
    -468,  -468,  -468,  -468,   167,  -468,  -468,  -468,   215,  -468,
    -468,  -468,  -468,  -468,  -468,  -468,  -468,  -468,  -468,  -468,
    -468,  -468,  -468,   220,   228,   119,   246,   200,   248,   250,
     255,   157,  -468,  -468,  -468,  -468,   161,   168,   174,   175,
     181,   188,   205,  -468,   211,   226,   232,   240,   241,   243,
     623,  -468,   623,   623,   305,   -31,  -468,    88,   942,  -468,
     306,   258,   260,    84,    49,  -468,   281,    66,   293,  -468,
    -468,   369,   318,  -468,   318,   318,  -468,  -468,   280,  -468,
     375,  -468,   -48,  -468,   113,   376,   318,   368,   295,    21,
     311,   -53,  -468,  -468,  -468,   459,   382,   623,   543,   -10,
     543,   267,   623,   543,   543,   623,   623,   623,   268,   270,
     271,   272,  -468,  -468,   506,   288,   -74,     6,   321,  -468,
     390,  -468,   623,   623,   623,   623,   623,   623,   623,   623,
     623,   623,   623,   623,   623,   623,   623,   623,   623,   391,
    -468,  -468,   106,    49,   291,   294,  -468,  -468,  -468,  -468,
    -468,  -468,   286,   354,   322,   326,   318,  -468,   327,   404,
    -468,  -468,  -468,   315,  -468,   411,   412,   398,   245,  -468,
     298,  -468,   394,   347,  -468,  -468,  -468,   206,    12,  -468,
    -468,  -468,   303,  -468,    31,   377,  -468,   583,  -468,   942,
     136,  -468,  -468,   740,   142,   422,   307,   145,  -468,   767,
     148,   154,   605,   659,   794,  -468,   482,   623,   623,  -468,
       7,   424,  -468,  -468,    86,  -468,  -468,  -468,  -468,   956,
     969,  1048,  1059,  1068,  1068,   190,   190,   190,   190,   183,
     183,  -468,  -468,  -468,   309,  -468,  -468,  -468,  -468,  -468,
     426,  -468,  -468,  -468,   286,   197,   317,    49,   385,   318,
     318,  -468,  -468,  -468,   445,  -468,  -468,   345,    87,  -468,
    -468,   194,   357,   449,   450,  -468,  -468,  -468,  -468,  -468,
     170,   191,    21,   335,  -468,  -468,  -468,   374,   -29,   321,
     337,  -468,   543,  -468,  -468,   339,  -468,  -468,  -468,  -468,
    -468,  -468,   623,  -468,   623,  -468,   686,   713,  -468,   356,
     393,     4,   417,  -468,   461,  -468,     7,  -468,   131,  -468,
    -468,   366,   371,  -468,    58,   411,   407,  -468,  -468,   355,
     358,  -468,   359,  -468,   212,   360,   152,  -468,  -468,     7,
    -468,   466,   165,  -468,   362,  -468,  -468,   467,   361,     7,
     -20,   364,  -468,  -468,   821,   848,     7,   370,   378,     4,
     379,  -468,  -468,   173,   469,   395,  -468,   214,    -6,   423,
    -468,  -468,   489,   445,     1,  -468,  -468,  -468,   496,  -468,
    -468,   388,   383,   384,  -468,   194,    27,   381,  -468,  -468,
    -468,  -468,  -468,  -468,  -468,  -468,    53,    27,  -468,     7,
    -468,   200,   185,   386,  -468,  -468,  -468,   442,   504,   -40,
       4,    60,     9,    32,    36,    60,    60,    60,    60,   474,
     194,   470,   451,  -468,  -468,  -468,  -468,  -468,  -468,  -468,
    -468,   218,   419,   405,  -468,   396,  -468,  -468,  -468,    17,
     227,  -468,   360,  -468,  -468,  -468,   508,  -468,   229,  -468,
    -468,  -468,   406,  -468,  -468,   505,   475,   408,  -468,  -468,
     430,   432,  -468,     7,  -468,  -468,  -468,  -468,  -468,  -468,
    -468,  -468,  -468,  -468,  -468,  -468,  -468,  -468,    22,  -468,
     415,   471,   513,   306,  -468,  -468,     8,   496,  -468,  -468,
      27,  -468,  -468,   185,   194,   533,  -468,  -468,     7,    30,
     233,  -468,     7,   194,   516,   169,   407,   427,  -468,  -468,
     537,  -468,  -468,  -468,    43,   433,  -468,   434,  -468,   538,
    -468,  -468,   235,  -468,   194,   437,   433,  -468,   542,   453,
     237,  -468,  -468,  -468,   194,   553,  -468,   433,   440,   441,
       7,  -468,   537,  -468,  -468,  -468,  -468,  -468,  -468
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -468,  -468,  -468,  -468,   447,  -468,  -468,  -468,    51,  -468,
    -468,  -468,  -193,   253,   418,  -468,  -468,    50,  -468,   275,
    -340,  -468,  -468,  -263,  -119,  -339,  -335,  -468,  -468,  -468,
    -468,  -468,  -468,  -467,    13,    68,  -468,    46,  -468,  -468,
      62,  -468,    11,    -2,  -468,   114,   263,  -468,  -468,  -101,
    -468,   299,   410,  -468,  -468,   310,  -468,  -468,  -468,  -468,
    -468,  -468,  -468,  -468,   122,  -468,  -300,  -468,  -468,  -468,
     279,  -468,  -468,  -468,   177,  -468,  -468,  -468,  -468,  -468,
    -468,  -468,   251,  -468,  -468,  -468,  -468,  -468,  -468,   230,
    -468,  -468,  -468,   580,  -468,  -468,  -468,  -468,  -468,  -468,
    -468,  -468,  -468,  -468
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -247
static const short yytable[] =
{
     209,   387,   362,   191,   192,   396,   204,   347,   348,   111,
     147,   527,   204,   528,    88,   201,   529,   204,   482,   368,
     297,   421,   204,   445,   310,   225,   204,   205,   353,   206,
     195,   521,   204,   205,   349,   206,   205,   204,   205,   429,
     484,   204,   205,    91,   487,   215,   241,    53,   242,   350,
      92,   351,   176,   271,   177,   178,   463,   179,   546,   451,
     552,   430,   378,   204,   205,   204,   205,   196,   383,   184,
     304,   112,   196,    54,   185,   369,   553,   557,   142,   478,
     143,   144,   305,    93,   468,   281,   446,   174,    55,   148,
     479,   149,   481,   175,   486,   489,   491,   493,   495,   497,
     148,   352,   226,    94,   150,    56,   461,   121,    57,   265,
     464,   204,   270,   180,   266,   306,   383,   466,   270,    95,
     450,   328,   379,   270,   243,   223,   530,   483,   270,    59,
     229,   298,   207,   232,   233,   234,   298,   377,   207,   208,
     522,   539,   458,   207,   267,   459,    88,   207,   520,   186,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   383,  -246,   207,
     268,   207,   347,   348,   409,    58,   394,   499,   339,   340,
     540,   415,   109,   209,   331,   331,    60,   542,   469,   197,
     470,   410,   110,   198,   431,    88,   269,   347,   348,   349,
     265,   388,   389,   111,   411,   266,   196,   345,   390,   113,
     532,   295,   296,   114,   350,   144,   351,   270,   116,   152,
     115,   432,  -246,   117,   349,   395,   152,    61,    62,    63,
      64,   118,   471,    65,    66,   326,   327,   119,   545,   350,
     413,   351,   224,    67,   227,   153,    68,   230,   231,   120,
     420,   122,   153,   123,    69,   311,   312,   426,   124,    97,
     543,   314,   312,    70,   317,   312,   352,   319,   312,    71,
      72,    73,    74,   320,   312,   125,    75,   433,   434,   435,
     436,   437,   438,    76,   126,    77,   127,   209,   439,   358,
     359,   352,   128,   129,   101,   166,   167,   168,   209,   130,
     164,   165,   166,   167,   168,   102,   131,    88,   145,    78,
     360,   361,   480,   103,   485,   488,   490,   492,   494,   496,
     374,    79,   375,   132,   139,    62,    63,    64,   104,   133,
      65,   404,   405,   444,   359,    80,    81,   505,   359,    82,
      67,   106,    83,    68,   134,    84,   509,   510,   512,   359,
     135,    69,   541,   359,   556,   359,   561,   562,   136,   137,
      70,   138,   172,   169,   173,    97,    71,    72,    73,    74,
     183,   187,   188,    75,   189,   193,    98,    99,   194,   200,
      76,   202,    77,   203,   214,   222,   228,   235,   236,   237,
     238,   209,   240,   246,   100,   275,   264,   277,   276,   538,
     101,    61,    62,    63,    64,   279,    78,    65,   278,   280,
     282,   102,   283,   284,   287,   290,   291,    67,    79,   103,
      68,   293,   294,   302,   308,   315,   316,   329,    69,   334,
      95,   295,    80,    81,   104,   336,    82,    70,   105,    83,
     338,   567,    84,    71,    72,    73,    74,   106,   341,   344,
      75,   355,   356,   357,   366,   367,   371,    76,   373,    77,
     376,   384,   139,    62,    63,    64,   386,   216,    65,   309,
     392,   393,   398,   401,   414,   418,   402,   403,   406,   419,
     430,    68,   417,    78,   422,   139,    62,    63,    64,   427,
     216,    65,   440,   447,   441,    79,   428,   448,    70,   452,
     455,   462,   456,   457,    68,   474,    73,    74,   475,    80,
      81,    75,   477,    82,   501,   508,    83,   502,   140,    84,
     141,    70,   498,   506,   415,   507,   513,   517,   514,    73,
      74,   518,   515,   519,    75,   523,   525,   524,   537,   544,
     549,   140,   152,   141,   296,   548,   139,    62,    63,    64,
     559,   216,    65,   554,   555,   558,    79,   560,   564,   565,
     566,   199,   370,   346,   533,    68,   245,   563,   153,   531,
      80,   526,   547,   568,    82,   372,   337,   217,   218,    79,
      84,   363,    70,   274,   511,   335,   139,    62,    63,    64,
      73,    74,    65,    80,   467,    75,   397,    82,   108,     0,
      83,   218,   140,    84,   141,    68,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,     0,    70,   449,     0,   239,   139,    62,    63,    64,
      73,    74,    65,     0,     0,    75,     0,     0,     0,     0,
      79,   152,   140,     0,   141,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,    82,   309,
       0,    83,    70,     0,    84,     0,     0,   153,     0,     0,
      73,    74,     0,     0,     0,    75,     0,     0,     0,     0,
      79,     0,   140,     0,   141,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    80,   152,     0,     0,    82,     0,
       0,    83,     0,     0,    84,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
      79,   153,   152,     0,   321,   322,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,    82,     0,
       0,    83,     0,     0,    84,     0,     0,     0,   153,   152,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,     0,   153,   152,     0,   323,   324,
       0,     0,     0,     0,     0,     0,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,     0,   153,   152,     0,     0,   322,     0,     0,     0,
       0,     0,     0,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,     0,   153,
     152,     0,     0,   324,     0,     0,     0,     0,     0,     0,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,     0,   153,   152,     0,   313,
       0,     0,     0,     0,     0,     0,     0,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,     0,   153,   152,     0,   318,     0,     0,     0,
       0,     0,     0,     0,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,     0,
     153,     0,     0,   325,     0,     0,     0,     0,     0,     0,
       0,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,     0,     0,     0,     0,
     424,     0,     0,     0,     0,     0,     0,     0,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,     1,     0,     0,     2,   425,     3,     0,
       0,     0,     4,     0,     0,     0,     5,     0,   152,     6,
       7,     8,     9,     0,     0,    10,     0,     0,    11,     0,
       0,     0,   152,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,   153,   152,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,     0,     0,   153,     0,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
      19,   153,     0,     0,     0,     0,     0,     0,     0,    20,
       0,    21,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   152,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,     0,     0,     0,     0,
       0,     0,     0,     0,   152,     0,     0,     0,     0,     0,
     153,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   153,     0,     0,     0,     0,     0,     0,     0,     0,
     153,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   160,   161,   162,   163,   164,   165,
     166,   167,   168
};

static const short yycheck[] =
{
     119,   336,   302,   104,   105,   344,     5,     3,     4,     3,
      41,     3,     5,     5,    16,   116,     8,     5,     9,    48,
       8,    41,     5,    29,   217,    35,     5,     6,   291,     8,
      78,     9,     5,     6,    30,     8,     6,     5,     6,   379,
       8,     5,     6,     3,     8,    98,   120,    86,   122,    45,
      10,    47,     3,   172,     5,     6,     3,     8,   525,   394,
      17,   101,    58,     5,     6,     5,     6,   120,   331,     3,
      39,    21,   120,    49,     8,   104,    33,   544,    80,   119,
      82,    83,    51,    43,   419,   186,    92,     3,     3,   120,
     430,     3,   431,     9,   433,   434,   435,   436,   437,   438,
     120,    97,   112,    63,    16,    42,   406,    57,    41,     3,
      57,     5,   111,    64,     8,    84,   379,   417,   111,    79,
     119,   240,   118,   111,   118,   127,   118,   118,   111,    75,
     132,   119,   111,   135,   136,   137,   119,   330,   111,   118,
     118,   111,   405,   111,    38,   118,   148,   111,   483,    83,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   430,     3,   111,
      64,   111,     3,     4,    22,    42,   118,   440,   279,   280,
     519,    16,    89,   302,    98,    98,    49,   522,     3,    76,
       5,    39,    75,    80,    21,   197,    90,     3,     4,    30,
       3,    70,    71,     3,    52,     8,   120,   120,    77,     0,
     510,     5,     6,   117,    45,   217,    47,   111,     3,    36,
      53,    48,    57,     3,    30,   344,    36,     3,     4,     5,
       6,     3,    47,     9,    10,   237,   238,   118,    69,    45,
     359,    47,   128,    19,   130,    62,    22,   133,   134,     3,
     369,     3,    62,     3,    30,   119,   120,   376,     3,    14,
     523,   119,   120,    39,   119,   120,    97,   119,   120,    45,
      46,    47,    48,   119,   120,   118,    52,   104,   105,   106,
     107,   108,   109,    59,   123,    61,   118,   406,   115,   119,
     120,    97,   118,   118,    49,   112,   113,   114,   417,   118,
     110,   111,   112,   113,   114,    60,   118,   309,     3,    85,
     119,   120,   431,    68,   433,   434,   435,   436,   437,   438,
     322,    97,   324,   118,     3,     4,     5,     6,    83,   118,
       9,   119,   120,   119,   120,   111,   112,   119,   120,   115,
      19,    96,   118,    22,   118,   121,   119,   120,   119,   120,
     118,    30,   119,   120,   119,   120,   119,   120,   118,   118,
      39,   118,   104,    57,   104,    14,    45,    46,    47,    48,
      89,    78,     3,    52,    56,    95,    25,    26,     3,     3,
      59,    13,    61,    88,    73,     3,   119,   119,   118,   118,
     118,   510,   104,     3,    43,   104,     5,   111,   104,   518,
      49,     3,     4,     5,     6,    83,    85,     9,    54,    83,
      83,    60,     8,    98,     3,     3,   118,    19,    97,    68,
      22,    27,    75,   120,    47,     3,   119,     3,    30,   120,
      79,     5,   111,   112,    83,   118,   115,    39,    87,   118,
      55,   560,   121,    45,    46,    47,    48,    96,     3,   104,
      52,    94,     3,     3,   119,    81,   119,    59,   119,    61,
     104,    44,     3,     4,     5,     6,     5,     8,     9,    76,
     104,   100,    65,   118,     8,     8,   118,   118,   118,   118,
     101,    22,   120,    85,   120,     3,     4,     5,     6,   119,
       8,     9,    23,    70,    99,    97,   118,     8,    39,     3,
     112,   120,   119,   119,    22,   119,    47,    48,    66,   111,
     112,    52,     8,   115,    44,   119,   118,    66,    59,   121,
      61,    39,    48,   104,    16,   120,   120,   119,    23,    47,
      48,   101,    57,   101,    52,   120,    23,    66,     5,    23,
       3,    59,    36,    61,     6,   118,     3,     4,     5,     6,
       8,     8,     9,   120,   120,   118,    97,   104,     5,   119,
     119,   114,   309,   288,   513,    22,   148,   554,    62,   507,
     111,   503,   526,   562,   115,   312,   277,   118,   119,    97,
     121,   302,    39,   173,   462,   275,     3,     4,     5,     6,
      47,    48,     9,   111,   417,    52,   345,   115,    18,    -1,
     118,   119,    59,   121,    61,    22,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    -1,    39,   393,    -1,   119,     3,     4,     5,     6,
      47,    48,     9,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      97,    36,    59,    -1,    61,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,   115,    76,
      -1,   118,    39,    -1,   121,    -1,    -1,    62,    -1,    -1,
      47,    48,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      97,    -1,    59,    -1,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    36,    -1,    -1,   115,    -1,
      -1,   118,    -1,    -1,   121,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      97,    62,    36,    -1,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,   115,    -1,
      -1,   118,    -1,    -1,   121,    -1,    -1,    -1,    62,    36,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    -1,    62,    36,    -1,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    -1,    62,    36,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    -1,    62,
      36,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    -1,    62,    36,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    -1,    62,    36,    -1,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    -1,
      62,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    15,    -1,    -1,    18,   119,    20,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    28,    -1,    36,    31,
      32,    33,    34,    -1,    -1,    37,    -1,    -1,    40,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    62,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    62,    -1,
      72,    -1,    74,    -1,    76,    -1,    78,    -1,    80,    -1,
      82,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   106,   107,   108,   109,   110,   111,
     112,   113,   114
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    24,    28,    31,    32,    33,    34,
      37,    40,    50,    67,    72,    74,    76,    78,    80,    82,
      91,    93,   125,   126,   127,   128,   129,   136,   172,   177,
     178,   181,   182,   183,   184,   191,   192,   201,   202,   203,
     204,   208,   209,   214,   215,   216,   219,   221,   222,   223,
     224,   226,   227,    86,    49,     3,    42,    41,    42,    75,
      49,     3,     4,     5,     6,     9,    10,    19,    22,    30,
      39,    45,    46,    47,    48,    52,    59,    61,    85,    97,
     111,   112,   115,   118,   121,   137,   138,   140,   167,   168,
     225,     3,    10,    43,    63,    79,   217,    14,    25,    26,
      43,    49,    60,    68,    83,    87,    96,   174,   217,    89,
      75,     3,   141,     0,   117,    53,     3,     3,     3,   118,
       3,   141,     3,     3,     3,   118,   123,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,     3,
      59,    61,   167,   167,   167,     3,   171,    41,   120,     3,
      16,   139,    36,    62,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    57,
     159,   160,   104,   104,     3,     9,     3,     5,     6,     8,
      64,   175,   176,    89,     3,     8,    83,    78,     3,    56,
     173,   173,   173,    95,     3,    78,   120,    76,    80,   128,
       3,   173,    13,    88,     5,     6,     8,   111,   118,   148,
     149,   190,   193,   194,    73,    98,     8,   118,   119,   167,
     169,   170,     3,   167,   169,    35,   112,   169,   119,   167,
     169,   169,   167,   167,   167,   119,   118,   118,   118,   119,
     104,   120,   122,   118,   141,   138,     3,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,     5,     3,     8,    38,    64,    90,
     111,   148,   179,   180,   176,   104,   104,   111,    54,    83,
      83,   173,    83,     8,    98,   210,   211,     3,   205,   206,
       3,   118,   185,    27,    75,     5,     6,     8,   119,   148,
     150,   195,   120,   196,    39,    51,    84,   220,    47,    76,
     136,   119,   120,   119,   119,     3,   119,   119,   119,   119,
     119,   119,   120,   119,   120,   119,   167,   167,   148,     3,
     133,    98,   142,   143,   120,   179,   118,   175,    55,   173,
     173,     3,   212,   213,   104,   120,   143,     3,     4,    30,
      45,    47,    97,   147,   186,    94,     3,     3,   119,   120,
     119,   120,   190,   194,   197,   198,   119,    81,    48,   104,
     137,   119,   170,   119,   167,   167,   104,   136,    58,   118,
     144,   145,   146,   147,    44,   151,     5,   150,    70,    71,
      77,   218,   104,   100,   118,   148,   149,   206,    65,   161,
     162,   118,   118,   118,   119,   120,   118,   187,   188,    22,
      39,    52,   207,   148,     8,    16,   199,   120,     8,   118,
     148,    41,   120,   130,   119,   119,   148,   119,   118,   144,
     101,    21,    48,   104,   105,   106,   107,   108,   109,   115,
      23,    99,   153,   154,   119,    29,    92,    70,     8,   213,
     119,   150,     3,   163,   164,   112,   119,   119,   147,   118,
     189,   190,   120,     3,    57,   200,   190,   198,   150,     3,
       5,    47,   131,   132,   119,    66,   134,     8,   119,   144,
     148,   149,     9,   118,     8,   148,   149,     8,   148,   149,
     148,   149,   148,   149,   148,   149,   148,   149,    48,   147,
     152,    44,    66,   155,   156,   119,   104,   120,   119,   119,
     120,   188,   119,   120,    23,    57,   135,   119,   101,   101,
     150,     9,   118,   120,    66,    23,   159,     3,     5,     8,
     118,   164,   190,   132,   147,   157,   158,     5,   148,   111,
     149,   119,   150,   147,    23,    69,   157,   161,   118,     3,
     165,   166,    17,    33,   120,   120,   119,   157,   118,     8,
     104,   119,   120,   158,     5,   119,   119,   148,   166
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
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->m_pStmt->m_sTableFunc = yyvsp[-6].m_sValue;
		;}
    break;

  case 35:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 36:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 38:

    { yyval.m_sValue.SetSprintf ( "%d", yyvsp[0].m_iValue ); ;}
    break;

  case 39:

    { yyval.m_sValue = "id"; ;}
    break;

  case 40:

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

  case 41:

    {
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 43:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 44:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 45:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 48:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 51:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 52:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 53:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 54:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 55:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 56:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 59:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 60:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 61:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 63:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 70:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 71:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 72:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 73:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 74:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 75:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 76:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 94:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 95:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 96:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 97:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 98:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 99:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 100:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 101:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 102:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 103:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 106:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 107:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 110:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 113:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 114:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 116:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 118:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 119:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 122:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 123:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 129:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 130:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 131:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 132:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 135:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 136:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 138:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 139:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 143:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 144:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 165:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 166:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 167:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 168:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 169:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 170:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 171:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 172:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 173:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 178:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 179:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 182:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 183:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 184:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 185:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 186:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 187:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 188:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 189:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 190:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 198:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 199:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 200:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 203:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 204:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 207:

    { yyval.m_iValue = 1; ;}
    break;

  case 208:

    { yyval.m_iValue = 0; ;}
    break;

  case 209:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 210:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 211:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 212:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 215:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 216:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 217:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 220:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 221:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 224:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 225:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 226:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 227:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 228:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 229:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 230:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 231:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 232:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 233:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 234:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 235:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 236:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 238:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 239:

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

  case 240:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 243:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 245:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 249:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 250:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 253:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 254:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 257:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 258:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 259:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 260:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 261:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 262:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 263:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 264:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			tStmt.m_sIndex = yyvsp[-4].m_sValue;
			tStmt.m_sAlterAttr = yyvsp[-1].m_sValue;
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 265:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 272:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 273:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 274:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 282:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 283:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 284:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 285:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 286:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 287:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 288:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 289:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 291:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 292:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 293:

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

