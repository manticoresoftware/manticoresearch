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
     TOK_LIMIT = 305,
     TOK_MATCH = 306,
     TOK_MAX = 307,
     TOK_META = 308,
     TOK_MIN = 309,
     TOK_MOD = 310,
     TOK_NAMES = 311,
     TOK_NULL = 312,
     TOK_OPTION = 313,
     TOK_ORDER = 314,
     TOK_OPTIMIZE = 315,
     TOK_RAND = 316,
     TOK_READ = 317,
     TOK_REPEATABLE = 318,
     TOK_REPLACE = 319,
     TOK_RETURNS = 320,
     TOK_ROLLBACK = 321,
     TOK_RTINDEX = 322,
     TOK_SELECT = 323,
     TOK_SERIALIZABLE = 324,
     TOK_SET = 325,
     TOK_SESSION = 326,
     TOK_SHOW = 327,
     TOK_SONAME = 328,
     TOK_START = 329,
     TOK_STATUS = 330,
     TOK_SUM = 331,
     TOK_TABLES = 332,
     TOK_TO = 333,
     TOK_TRANSACTION = 334,
     TOK_TRUE = 335,
     TOK_TRUNCATE = 336,
     TOK_UNCOMMITTED = 337,
     TOK_UPDATE = 338,
     TOK_VALUES = 339,
     TOK_VARIABLES = 340,
     TOK_WARNINGS = 341,
     TOK_WEIGHT = 342,
     TOK_WHERE = 343,
     TOK_WITHIN = 344,
     TOK_OR = 345,
     TOK_AND = 346,
     TOK_NE = 347,
     TOK_GTE = 348,
     TOK_LTE = 349,
     TOK_NOT = 350,
     TOK_NEG = 351
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
#define TOK_LIMIT 305
#define TOK_MATCH 306
#define TOK_MAX 307
#define TOK_META 308
#define TOK_MIN 309
#define TOK_MOD 310
#define TOK_NAMES 311
#define TOK_NULL 312
#define TOK_OPTION 313
#define TOK_ORDER 314
#define TOK_OPTIMIZE 315
#define TOK_RAND 316
#define TOK_READ 317
#define TOK_REPEATABLE 318
#define TOK_REPLACE 319
#define TOK_RETURNS 320
#define TOK_ROLLBACK 321
#define TOK_RTINDEX 322
#define TOK_SELECT 323
#define TOK_SERIALIZABLE 324
#define TOK_SET 325
#define TOK_SESSION 326
#define TOK_SHOW 327
#define TOK_SONAME 328
#define TOK_START 329
#define TOK_STATUS 330
#define TOK_SUM 331
#define TOK_TABLES 332
#define TOK_TO 333
#define TOK_TRANSACTION 334
#define TOK_TRUE 335
#define TOK_TRUNCATE 336
#define TOK_UNCOMMITTED 337
#define TOK_UPDATE 338
#define TOK_VALUES 339
#define TOK_VARIABLES 340
#define TOK_WARNINGS 341
#define TOK_WEIGHT 342
#define TOK_WHERE 343
#define TOK_WITHIN 344
#define TOK_OR 345
#define TOK_AND 346
#define TOK_NE 347
#define TOK_GTE 348
#define TOK_LTE 349
#define TOK_NOT 350
#define TOK_NEG 351




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
#define YYLAST   875

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  114
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  92
/* YYNRULES -- Number of rules. */
#define YYNRULES  256
/* YYNRULES -- Number of states. */
#define YYNSTATES  485

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   104,    93,     2,
     109,   110,   102,   100,   108,   101,   113,   103,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   107,
      96,    94,    97,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   111,    92,   112,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    95,    98,    99,
     105,   106
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
     499,   501,   505,   507,   509,   513,   519,   522,   524,   526,
     528,   531,   533,   535,   537,   539,   541,   546,   551,   556,
     560,   565,   573,   579,   581,   583,   585,   587,   589,   591,
     593,   595,   597,   600,   607,   609,   611,   612,   616,   618,
     622,   624,   628,   632,   634,   638,   640,   642,   644,   648,
     651,   659,   669,   676,   678,   682,   684,   688,   690,   694,
     695,   698,   700,   704,   708,   709,   711,   713,   715,   718,
     720,   722,   725,   732,   734,   738,   742,   746,   752,   757,
     762,   763,   765,   768,   770,   774,   778,   781,   785,   792,
     793,   795,   797,   800,   803,   806,   808,   816,   818,   820,
     824,   831,   835,   839,   841,   845,   849
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     115,     0,    -1,   116,    -1,   117,    -1,   117,   107,    -1,
     163,    -1,   171,    -1,   157,    -1,   158,    -1,   161,    -1,
     172,    -1,   181,    -1,   183,    -1,   184,    -1,   187,    -1,
     192,    -1,   193,    -1,   197,    -1,   199,    -1,   200,    -1,
     201,    -1,   194,    -1,   202,    -1,   204,    -1,   205,    -1,
     118,    -1,   117,   107,   118,    -1,   119,    -1,   154,    -1,
      68,   120,    36,   124,   125,   133,   135,   137,   141,   143,
      -1,   121,    -1,   120,   108,   121,    -1,   102,    -1,   123,
     122,    -1,    -1,     3,    -1,    13,     3,    -1,   149,    -1,
      16,   109,   149,   110,    -1,    52,   109,   149,   110,    -1,
      54,   109,   149,   110,    -1,    76,   109,   149,   110,    -1,
      41,   109,   149,   110,    -1,    25,   109,   102,   110,    -1,
      87,   109,   110,    -1,    40,   109,   110,    -1,    25,   109,
      30,     3,   110,    -1,     3,    -1,   124,   108,     3,    -1,
      -1,   126,    -1,    88,   127,    -1,   128,    -1,   127,    91,
     128,    -1,    51,   109,     8,   110,    -1,   129,    94,   130,
      -1,   129,    95,   130,    -1,   129,    43,   109,   132,   110,
      -1,   129,   105,    43,   109,   132,   110,    -1,   129,    43,
       9,    -1,   129,   105,    43,     9,    -1,   129,    18,   130,
      91,   130,    -1,   129,    97,   130,    -1,   129,    96,   130,
      -1,   129,    98,   130,    -1,   129,    99,   130,    -1,   129,
      94,   131,    -1,   129,    95,   131,    -1,   129,    97,   131,
      -1,   129,    96,   131,    -1,   129,    18,   131,    91,   131,
      -1,   129,    98,   131,    -1,   129,    99,   131,    -1,     3,
      -1,     4,    -1,    25,   109,   102,   110,    -1,    40,   109,
     110,    -1,    87,   109,   110,    -1,    42,    -1,     5,    -1,
     101,     5,    -1,     6,    -1,   101,     6,    -1,   130,    -1,
     132,   108,   130,    -1,    -1,   134,    -1,    39,    19,   129,
      -1,    -1,   136,    -1,    89,    39,    59,    19,   139,    -1,
      -1,   138,    -1,    59,    19,   139,    -1,    59,    19,    61,
     109,   110,    -1,   140,    -1,   139,   108,   140,    -1,   129,
      -1,   129,    14,    -1,   129,    28,    -1,    -1,   142,    -1,
      50,     5,    -1,    50,     5,   108,     5,    -1,    -1,   144,
      -1,    58,   145,    -1,   146,    -1,   145,   108,   146,    -1,
       3,    94,     3,    -1,     3,    94,     5,    -1,     3,    94,
     109,   147,   110,    -1,     3,    94,     3,   109,     8,   110,
      -1,     3,    94,     8,    -1,   148,    -1,   147,   108,   148,
      -1,     3,    94,   130,    -1,     3,    -1,     4,    -1,    42,
      -1,     5,    -1,     6,    -1,     9,    -1,   101,   149,    -1,
     105,   149,    -1,   149,   100,   149,    -1,   149,   101,   149,
      -1,   149,   102,   149,    -1,   149,   103,   149,    -1,   149,
      96,   149,    -1,   149,    97,   149,    -1,   149,    93,   149,
      -1,   149,    92,   149,    -1,   149,   104,   149,    -1,   149,
      31,   149,    -1,   149,    55,   149,    -1,   149,    99,   149,
      -1,   149,    98,   149,    -1,   149,    94,   149,    -1,   149,
      95,   149,    -1,   149,    91,   149,    -1,   149,    90,   149,
      -1,   109,   149,   110,    -1,   111,   153,   112,    -1,   150,
      -1,     3,   109,   151,   110,    -1,    43,   109,   151,   110,
      -1,     3,   109,   110,    -1,    54,   109,   149,   108,   149,
     110,    -1,    52,   109,   149,   108,   149,   110,    -1,   152,
      -1,   151,   108,   152,    -1,   149,    -1,     8,    -1,     3,
      94,   130,    -1,   153,   108,     3,    94,   130,    -1,    72,
     155,    -1,    86,    -1,    75,    -1,    53,    -1,    12,    75,
      -1,     3,    -1,    57,    -1,     8,    -1,     5,    -1,     6,
      -1,    70,     3,    94,   160,    -1,    70,     3,    94,   159,
      -1,    70,     3,    94,    57,    -1,    70,    56,   156,    -1,
      70,    10,    94,   156,    -1,    70,    38,     9,    94,   109,
     132,   110,    -1,    70,    38,     3,    94,   159,    -1,     3,
      -1,     8,    -1,    80,    -1,    33,    -1,   130,    -1,    23,
      -1,    66,    -1,   162,    -1,    17,    -1,    74,    79,    -1,
     164,    47,     3,   165,    84,   167,    -1,    45,    -1,    64,
      -1,    -1,   109,   166,   110,    -1,   129,    -1,   166,   108,
     129,    -1,   168,    -1,   167,   108,   168,    -1,   109,   169,
     110,    -1,   170,    -1,   169,   108,   170,    -1,   130,    -1,
     131,    -1,     8,    -1,   109,   132,   110,    -1,   109,   110,
      -1,    27,    36,   124,    88,    42,    94,   130,    -1,    27,
      36,   124,    88,    42,    43,   109,   132,   110,    -1,    20,
       3,   109,   173,   176,   110,    -1,   174,    -1,   173,   108,
     174,    -1,   170,    -1,   109,   175,   110,    -1,     8,    -1,
     175,   108,     8,    -1,    -1,   108,   177,    -1,   178,    -1,
     177,   108,   178,    -1,   170,   179,   180,    -1,    -1,    13,
      -1,     3,    -1,    50,    -1,   182,     3,    -1,    29,    -1,
      28,    -1,    72,    77,    -1,    83,   124,    70,   185,   126,
     143,    -1,   186,    -1,   185,   108,   186,    -1,     3,    94,
     130,    -1,     3,    94,   131,    -1,     3,    94,   109,   132,
     110,    -1,     3,    94,   109,   110,    -1,    72,   195,    85,
     188,    -1,    -1,   189,    -1,    88,   190,    -1,   191,    -1,
     190,    90,   191,    -1,     3,    94,     8,    -1,    72,    22,
      -1,    72,    21,    70,    -1,    70,   195,    79,    48,    49,
     196,    -1,    -1,    38,    -1,    71,    -1,    62,    82,    -1,
      62,    24,    -1,    63,    62,    -1,    69,    -1,    26,    37,
       3,    65,   198,    73,     8,    -1,    46,    -1,    34,    -1,
      32,    37,     3,    -1,    15,    44,     3,    78,    67,     3,
      -1,    35,    67,     3,    -1,    68,   203,   141,    -1,    10,
      -1,    10,   113,     3,    -1,    81,    67,     3,    -1,    60,
      44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   125,   125,   126,   127,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   156,   157,   161,   162,   166,
     181,   182,   186,   187,   190,   192,   193,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   210,   211,   214,
     216,   220,   224,   225,   229,   234,   241,   249,   257,   266,
     271,   276,   281,   286,   291,   296,   301,   302,   303,   304,
     309,   314,   319,   327,   328,   333,   339,   345,   351,   360,
     361,   372,   373,   377,   383,   389,   391,   395,   402,   404,
     408,   414,   416,   420,   424,   431,   432,   436,   437,   438,
     441,   443,   447,   452,   459,   461,   465,   469,   470,   474,
     479,   484,   490,   495,   503,   508,   515,   525,   526,   527,
     528,   529,   530,   531,   532,   533,   534,   535,   536,   537,
     538,   539,   540,   541,   542,   543,   544,   545,   546,   547,
     548,   549,   550,   551,   552,   556,   557,   558,   559,   560,
     564,   565,   569,   570,   574,   575,   581,   585,   586,   587,
     588,   594,   595,   596,   597,   598,   602,   607,   612,   617,
     618,   622,   627,   635,   636,   640,   641,   642,   656,   657,
     658,   662,   663,   669,   677,   678,   681,   683,   687,   688,
     692,   693,   697,   701,   702,   706,   707,   708,   709,   710,
     716,   724,   738,   746,   750,   757,   758,   765,   775,   781,
     783,   787,   792,   796,   803,   805,   809,   810,   816,   824,
     825,   831,   837,   845,   846,   850,   854,   858,   862,   872,
     878,   879,   883,   887,   888,   892,   896,   903,   910,   916,
     917,   918,   922,   923,   924,   925,   931,   942,   943,   947,
     958,   970,   981,   989,   990,   999,  1010
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
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", 
  "TOK_ORDER", "TOK_OPTIMIZE", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", 
  "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_SUM", "TOK_TABLES", 
  "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", 
  "';'", "','", "'('", "')'", "'{'", "'}'", "'.'", "$accept", "request", 
  "statement", "multi_stmt_list", "multi_stmt", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "show_stmt", "show_what", "set_value", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "update", "update_items_list", 
  "update_item", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "show_character_set", 
  "set_transaction", "opt_scope", "isolation_level", "create_function", 
  "udf_type", "drop_function", "attach_index", "flush_rtindex", 
  "select_sysvar", "sysvar_name", "truncate", "optimize_index", 0
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
     345,   346,   124,    38,    61,   347,    60,    62,   348,   349,
      43,    45,    42,    47,    37,   350,   351,    59,    44,    40,
      41,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   114,   115,   115,   115,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   117,   117,   118,   118,   119,
     120,   120,   121,   121,   122,   122,   122,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   124,   124,   125,
     125,   126,   127,   127,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   129,   129,   129,   129,   129,   129,   130,
     130,   131,   131,   132,   132,   133,   133,   134,   135,   135,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   140,
     141,   141,   142,   142,   143,   143,   144,   145,   145,   146,
     146,   146,   146,   146,   147,   147,   148,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   150,   150,   150,   150,   150,
     151,   151,   152,   152,   153,   153,   154,   155,   155,   155,
     155,   156,   156,   156,   156,   156,   157,   157,   157,   157,
     157,   158,   158,   159,   159,   160,   160,   160,   161,   161,
     161,   162,   162,   163,   164,   164,   165,   165,   166,   166,
     167,   167,   168,   169,   169,   170,   170,   170,   170,   170,
     171,   171,   172,   173,   173,   174,   174,   175,   175,   176,
     176,   177,   177,   178,   179,   179,   180,   180,   181,   182,
     182,   183,   184,   185,   185,   186,   186,   186,   186,   187,
     188,   188,   189,   190,   190,   191,   192,   193,   194,   195,
     195,   195,   196,   196,   196,   196,   197,   198,   198,   199,
     200,   201,   202,   203,   203,   204,   205
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
       1,     3,     1,     1,     3,     5,     2,     1,     1,     1,
       2,     1,     1,     1,     1,     1,     4,     4,     4,     3,
       4,     7,     5,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     6,     1,     1,     0,     3,     1,     3,
       1,     3,     3,     1,     3,     1,     1,     1,     3,     2,
       7,     9,     6,     1,     3,     1,     3,     1,     3,     0,
       2,     1,     3,     3,     0,     1,     1,     1,     2,     1,
       1,     2,     6,     1,     3,     3,     3,     5,     4,     4,
       0,     1,     2,     1,     3,     3,     2,     3,     6,     0,
       1,     1,     2,     2,     2,     1,     7,     1,     1,     3,
       6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   181,     0,   178,     0,     0,   220,   219,     0,
       0,   184,     0,   185,   179,     0,   239,   239,     0,     0,
       0,     0,     2,     3,    25,    27,    28,     7,     8,     9,
     180,     5,     0,     6,    10,    11,     0,    12,    13,    14,
      15,    16,    21,    17,    18,    19,    20,    22,    23,    24,
       0,     0,     0,     0,     0,     0,     0,   117,   118,   120,
     121,   122,   253,     0,     0,     0,     0,   119,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,    30,
      34,    37,   144,   100,     0,     0,   240,     0,   241,     0,
       0,     0,   236,   240,   159,   158,   221,   157,   156,     0,
     182,     0,    47,     0,     1,     4,     0,   218,     0,     0,
       0,     0,   249,   251,   256,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   123,   124,
       0,     0,     0,     0,     0,    35,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,   101,     0,     0,
       0,     0,   161,   164,   165,   163,   162,   169,     0,   160,
     237,   230,   255,     0,     0,     0,     0,    26,   186,     0,
      79,    81,   197,     0,     0,   195,   196,   205,   209,   203,
       0,     0,   153,   147,   152,     0,   150,   254,     0,     0,
       0,    45,     0,     0,     0,     0,     0,    44,     0,     0,
     142,     0,     0,   143,    49,    31,    36,   134,   135,   141,
     140,   132,   131,   138,   139,   129,   130,   137,   136,   125,
     126,   127,   128,   133,   102,   173,   174,   176,   168,   175,
       0,   177,   167,   166,   170,     0,     0,     0,     0,   229,
     231,     0,     0,   223,    48,     0,     0,     0,    80,    82,
     207,   199,    83,     0,     0,     0,     0,   248,   247,     0,
       0,     0,   145,    38,     0,    43,    42,   146,     0,    39,
       0,    40,    41,     0,     0,   154,     0,     0,    85,    50,
       0,   172,     0,     0,     0,   232,   233,     0,     0,   104,
      73,    74,     0,     0,    78,     0,   188,     0,     0,   250,
       0,   198,     0,   206,   205,   204,   210,   211,   202,     0,
       0,     0,   151,    46,     0,     0,     0,     0,    51,    52,
       0,     0,    88,    86,   103,     0,     0,     0,   245,   238,
       0,     0,     0,   225,   226,   224,     0,   222,   105,     0,
       0,     0,     0,   187,     0,   183,   190,    84,   208,   215,
       0,     0,   246,     0,   200,   149,   148,   155,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    91,    89,   171,   243,   242,   244,   235,   234,   228,
       0,     0,   106,   107,     0,    76,    77,   189,     0,     0,
     193,     0,   216,   217,   213,   214,   212,     0,     0,    53,
       0,     0,    59,     0,    55,    66,    56,    67,    63,    69,
      62,    68,    64,    71,    65,    72,     0,    87,     0,     0,
     100,    92,   227,     0,     0,    75,     0,   192,   191,   201,
      54,     0,     0,     0,    60,     0,     0,     0,   104,   109,
     110,   113,     0,   108,   194,    61,     0,    70,    57,     0,
       0,     0,    97,    93,    95,    29,     0,     0,     0,   114,
      58,    90,     0,    98,    99,     0,     0,     0,     0,   111,
      94,    96,   112,   116,   115
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,    78,    79,   137,    80,
     103,   288,   289,   328,   329,   462,   262,   186,   263,   332,
     333,   381,   382,   430,   431,   463,   464,   156,   157,   347,
     348,   392,   393,   468,   469,    81,    82,   195,   196,   132,
      26,    98,   167,    27,    28,   242,   243,    29,    30,    31,
      32,   256,   307,   355,   356,   399,   187,    33,    34,   188,
     189,   264,   266,   316,   317,   360,   404,    35,    36,    37,
      38,   252,   253,    39,   249,   250,   295,   296,    40,    41,
      42,    89,   339,    43,   269,    44,    45,    46,    47,    83,
      48,    49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -282
static const short yypact[] =
{
     792,     0,  -282,    65,  -282,    50,    64,  -282,  -282,   114,
      70,  -282,   116,  -282,  -282,   105,   770,   653,    87,   103,
     171,   176,  -282,    92,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,   150,  -282,  -282,  -282,   201,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
     210,   106,   215,   171,   221,   222,   231,   130,  -282,  -282,
    -282,  -282,   134,   147,   153,   161,   163,  -282,   164,   168,
     169,   170,   172,   297,  -282,   297,   297,   277,    -1,  -282,
      30,   586,  -282,   233,   191,   195,    17,    55,  -282,   216,
     219,   226,  -282,  -282,  -282,  -282,  -282,  -282,  -282,   213,
    -282,   302,  -282,   -51,  -282,    66,   308,  -282,   234,    22,
     248,   -63,  -282,  -282,  -282,   232,   311,   297,    20,   205,
     297,   245,   297,   297,   297,   211,   214,   220,  -282,  -282,
     384,   230,   -46,   171,   217,  -282,   317,  -282,   297,   297,
     297,   297,   297,   297,   297,   297,   297,   297,   297,   297,
     297,   297,   297,   297,   297,   320,  -282,  -282,   175,    55,
     237,   250,  -282,  -282,  -282,  -282,  -282,  -282,   300,  -282,
    -282,   282,  -282,   349,   364,   217,    89,  -282,   262,   306,
    -282,  -282,  -282,   149,     3,  -282,  -282,  -282,   276,  -282,
      98,   343,  -282,  -282,   586,    42,  -282,  -282,   409,   385,
     280,  -282,   435,    57,   334,   355,   460,  -282,   297,   297,
    -282,     5,   388,  -282,   -52,  -282,  -282,  -282,  -282,   616,
     666,   691,   702,   279,   279,   127,   127,   127,   127,   188,
     188,  -282,  -282,  -282,   285,  -282,  -282,  -282,  -282,  -282,
     382,  -282,  -282,  -282,  -282,    39,   286,   345,   393,  -282,
    -282,   303,   -49,  -282,  -282,   332,   315,   398,  -282,  -282,
    -282,  -282,  -282,    63,    69,    22,   293,  -282,  -282,   331,
     -30,   245,  -282,  -282,   295,  -282,  -282,  -282,   297,  -282,
     297,  -282,  -282,   535,   560,  -282,   318,   313,   368,  -282,
     404,  -282,     5,   100,   319,   321,  -282,    16,   349,   356,
    -282,  -282,   307,   309,  -282,   312,  -282,    76,   314,  -282,
       5,  -282,   412,  -282,   140,  -282,   333,  -282,  -282,   414,
     351,     5,  -282,  -282,   485,   510,     5,   352,   326,  -282,
     264,   424,   373,  -282,  -282,    77,   -15,   405,  -282,  -282,
     461,   393,    18,  -282,  -282,  -282,   465,  -282,  -282,   369,
     360,   362,   332,  -282,    32,   365,  -282,  -282,  -282,  -282,
      38,    32,  -282,     5,  -282,  -282,  -282,  -282,   481,   313,
      26,    -6,    26,    26,    26,    26,    26,    26,   449,   332,
     454,   436,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
      81,   402,   389,  -282,   407,  -282,  -282,  -282,    19,    85,
    -282,   314,  -282,  -282,  -282,   501,  -282,    86,   408,  -282,
     429,   430,  -282,     5,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,    -4,  -282,   439,   503,
     233,  -282,  -282,     9,   465,  -282,    32,  -282,  -282,  -282,
    -282,     5,    23,    90,  -282,     5,   504,   305,   356,   415,
    -282,  -282,   539,  -282,  -282,  -282,   537,  -282,  -282,    95,
     332,   437,   121,   440,  -282,  -282,   536,   453,   136,  -282,
    -282,   440,   457,  -282,  -282,   332,   458,     5,   539,  -282,
    -282,  -282,  -282,  -282,  -282
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -282,  -282,  -282,  -282,   444,  -282,  -282,   438,  -282,  -282,
     -35,  -282,   322,  -282,   200,  -253,  -109,  -281,  -277,  -282,
    -282,  -282,  -282,  -282,  -282,   111,   117,   143,  -282,   145,
    -282,  -282,   160,  -282,   118,   -69,  -282,   476,   327,  -282,
    -282,  -282,   459,  -282,  -282,   354,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,   218,  -282,  -264,  -282,  -282,  -282,
     357,  -282,  -282,  -282,   255,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,   323,  -282,  -282,  -282,  -282,   283,  -282,  -282,
    -282,   606,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -215
static const short yytable[] =
{
     185,   314,   306,   412,   128,   444,   129,   130,   180,   384,
     180,   260,   449,   320,   450,   335,   344,   451,   111,   173,
     160,   180,   181,   180,   180,   191,   161,   180,   181,   181,
     182,   180,   181,   135,   330,   133,   287,   180,   181,   287,
     182,   402,   235,   136,    50,   174,   194,   236,   198,   241,
     199,   202,   194,   204,   205,   206,   174,   174,   162,   298,
     163,   164,   212,   165,   321,   390,   213,   385,    51,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   407,    52,   403,   411,
     400,   415,   417,   419,   421,   423,   425,   405,   214,   397,
      53,    90,   285,   413,   240,   445,   240,   134,    57,    58,
      59,    60,   166,   261,    61,    62,   330,   183,   452,   240,
     240,    63,   200,   183,   456,   342,   427,   183,   389,   261,
      64,   184,   267,   183,   175,   473,   443,    55,   176,   283,
     284,   398,    94,  -214,   268,    65,    66,    67,    68,   474,
     271,    54,   272,   359,   258,   259,   185,    69,   138,    70,
      56,   457,   336,   337,    95,   271,   100,   277,   459,   338,
     101,   310,   454,   311,   102,    97,   104,   312,   235,   313,
     180,    71,   139,   236,   352,   310,   353,   383,   343,   310,
    -214,   432,    72,   436,   310,   437,   439,   106,   310,   105,
     458,   357,   194,   310,   107,   470,    73,    74,   237,   324,
      75,   325,   364,   108,    76,   109,    77,   367,   110,   138,
      57,    58,    59,    60,   112,   113,    61,   150,   151,   152,
     153,   154,   238,    63,   114,    57,    58,    59,    60,   115,
     192,    61,    64,   139,   478,   185,   479,   116,    57,    58,
      59,    60,   185,   192,    61,   239,   117,    65,    66,    67,
      68,   410,   118,   414,   416,   418,   420,   422,   424,    69,
     119,    70,   120,   121,    67,    68,   240,   122,   123,   124,
     131,   125,   370,   155,   126,   158,   127,    67,    68,   159,
     152,   153,   154,    71,   169,   168,   170,   126,   171,   127,
      57,    58,    59,    60,    72,   172,    61,   371,   300,   301,
     138,   178,   179,   190,   197,   201,   300,   301,    73,    74,
     216,   207,    75,   208,   211,   234,    76,   185,    77,   209,
     302,   245,   455,    73,   139,   300,   301,    75,   302,    67,
      68,    76,   193,    77,   246,   303,    73,   304,   247,   126,
      75,   127,   251,   303,    76,   304,    77,   302,   372,   373,
     374,   375,   376,   377,   327,   138,   461,   254,   483,   378,
     248,   255,   303,   257,   304,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   265,   270,   138,   258,   274,   139,
     275,   286,   305,   290,   293,   292,   294,   297,    73,   308,
     305,   309,    75,   318,   319,   323,    76,   331,    77,   334,
     139,   341,   326,   340,   346,   138,   349,   369,   350,   305,
     358,   351,   362,   354,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   139,
     138,   361,   278,   379,   279,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     363,   368,   380,   280,   139,   281,   138,   386,   391,   387,
     395,   394,   396,   401,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   408,
     139,   138,   426,   428,   210,   429,   433,   434,   446,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   359,   139,   138,   435,   440,   273,
     441,   442,   447,   460,   466,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     139,   138,   467,   259,   476,   276,   472,   477,   475,   177,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   139,   138,   480,   482,   409,
     282,   471,   215,   448,   299,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     139,   138,   481,   465,   453,   365,   484,   203,   322,   291,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   139,   406,   138,   244,   438,
     366,   345,   315,    99,   388,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   139,     0,   278,     0,     0,     0,   138,     0,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,    90,     0,     0,   280,     0,
       0,   139,     0,     0,    91,    92,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    93,     0,     0,     0,     0,     0,   138,     0,     0,
       0,     0,     0,     0,     0,     0,    94,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   139,   138,     0,    88,     0,     0,     0,    95,     0,
      96,     0,     0,   138,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,     0,     0,   139,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   139,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,     0,    84,     0,     0,     0,     0,     0,     0,
      85,     0,     0,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     1,    86,     2,
       0,     0,     3,     0,     0,     4,     0,     0,     5,     6,
       7,     8,     0,     0,     9,     0,    87,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,     0,     0,
       0,    88,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,    13,     0,    14,     0,
      15,     0,    16,     0,    17,     0,    18,     0,     0,     0,
       0,     0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     109,   265,   255,     9,    73,     9,    75,    76,     5,    24,
       5,     8,     3,    43,     5,   292,   297,     8,    53,    70,
       3,     5,     6,     5,     5,    88,     9,     5,     6,     6,
       8,     5,     6,     3,   287,    36,    88,     5,     6,    88,
       8,     3,     3,    13,    44,   108,   115,     8,   117,   158,
      30,   120,   121,   122,   123,   124,   108,   108,     3,   108,
       5,     6,   108,     8,    94,   342,   112,    82,     3,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   363,    37,    50,   370,
     354,   372,   373,   374,   375,   376,   377,   361,   133,   352,
      36,    12,   211,   109,   101,   109,   101,   108,     3,     4,
       5,     6,    57,   110,     9,    10,   369,   101,   109,   101,
     101,    16,   102,   101,   101,   109,   379,   101,   110,   110,
      25,   109,    34,   101,    68,    14,   413,    67,    72,   208,
     209,   109,    53,     3,    46,    40,    41,    42,    43,    28,
     108,    37,   110,    13,     5,     6,   265,    52,    31,    54,
      44,   442,    62,    63,    75,   108,    79,   110,   445,    69,
      67,   108,   436,   110,     3,    86,     0,   108,     3,   110,
       5,    76,    55,     8,   108,   108,   110,   110,   297,   108,
      50,   110,    87,   108,   108,   110,   110,    47,   108,   107,
     110,   310,   271,   108,     3,   110,   101,   102,    33,   278,
     105,   280,   321,     3,   109,   109,   111,   326,     3,    31,
       3,     4,     5,     6,     3,     3,     9,   100,   101,   102,
     103,   104,    57,    16,     3,     3,     4,     5,     6,   109,
       8,     9,    25,    55,   108,   354,   110,   113,     3,     4,
       5,     6,   361,     8,     9,    80,   109,    40,    41,    42,
      43,   370,   109,   372,   373,   374,   375,   376,   377,    52,
     109,    54,   109,   109,    42,    43,   101,   109,   109,   109,
       3,   109,    18,    50,    52,    94,    54,    42,    43,    94,
     102,   103,   104,    76,    75,    79,    70,    52,    85,    54,
       3,     4,     5,     6,    87,     3,     9,    43,     3,     4,
      31,     3,    78,    65,     3,   110,     3,     4,   101,   102,
       3,   110,   105,   109,    94,     5,   109,   436,   111,   109,
      25,    94,   441,   101,    55,     3,     4,   105,    25,    42,
      43,   109,   110,   111,    94,    40,   101,    42,    48,    52,
     105,    54,     3,    40,   109,    42,   111,    25,    94,    95,
      96,    97,    98,    99,    51,    31,    61,     3,   477,   105,
      88,   109,    40,    67,    42,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   108,    42,    31,     5,     3,    55,
     110,     3,    87,   108,    49,   109,     3,    94,   101,    84,
      87,     3,   105,   110,    73,   110,   109,    39,   111,     5,
      55,    90,    94,    94,    58,    31,   109,    91,   109,    87,
       8,   109,     8,   109,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    55,
      31,   108,   108,    19,   110,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     109,   109,    89,   108,    55,   110,    31,    62,     3,     8,
     110,   102,   110,   108,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,     8,
      55,    31,    43,    39,   110,    59,    94,   108,    59,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    13,    55,    31,   110,   110,   110,
      91,    91,    19,    19,   109,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
      55,    31,     3,     6,     8,   110,   109,    94,   108,   105,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    55,    31,   110,   110,   369,
     110,   460,   134,   430,   252,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
      55,    31,   475,   448,   434,   110,   478,   121,   271,   245,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    55,   361,    31,   159,   401,
     110,   298,   265,    17,   341,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
      -1,    55,    -1,   108,    -1,    -1,    -1,    31,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    12,    -1,    -1,   108,    -1,
      -1,    55,    -1,    -1,    21,    22,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    38,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    55,    31,    -1,    71,    -1,    -1,    -1,    75,    -1,
      77,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    -1,    -1,     3,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,    15,    38,    17,
      -1,    -1,    20,    -1,    -1,    23,    -1,    -1,    26,    27,
      28,    29,    -1,    -1,    32,    -1,    56,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    64,    -1,    66,    -1,
      68,    -1,    70,    -1,    72,    -1,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    83
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    60,    64,    66,    68,    70,    72,    74,    81,
      83,   115,   116,   117,   118,   119,   154,   157,   158,   161,
     162,   163,   164,   171,   172,   181,   182,   183,   184,   187,
     192,   193,   194,   197,   199,   200,   201,   202,   204,   205,
      44,     3,    37,    36,    37,    67,    44,     3,     4,     5,
       6,     9,    10,    16,    25,    40,    41,    42,    43,    52,
      54,    76,    87,   101,   102,   105,   109,   111,   120,   121,
     123,   149,   150,   203,     3,    10,    38,    56,    71,   195,
      12,    21,    22,    38,    53,    75,    77,    86,   155,   195,
      79,    67,     3,   124,     0,   107,    47,     3,     3,   109,
       3,   124,     3,     3,     3,   109,   113,   109,   109,   109,
     109,   109,   109,   109,   109,   109,    52,    54,   149,   149,
     149,     3,   153,    36,   108,     3,    13,   122,    31,    55,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    50,   141,   142,    94,    94,
       3,     9,     3,     5,     6,     8,    57,   156,    79,    75,
      70,    85,     3,    70,   108,    68,    72,   118,     3,    78,
       5,     6,     8,   101,   109,   130,   131,   170,   173,   174,
      65,    88,     8,   110,   149,   151,   152,     3,   149,    30,
     102,   110,   149,   151,   149,   149,   149,   110,   109,   109,
     110,    94,   108,   112,   124,   121,     3,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,     5,     3,     8,    33,    57,    80,
     101,   130,   159,   160,   156,    94,    94,    48,    88,   188,
     189,     3,   185,   186,     3,   109,   165,    67,     5,     6,
       8,   110,   130,   132,   175,   108,   176,    34,    46,   198,
      42,   108,   110,   110,     3,   110,   110,   110,   108,   110,
     108,   110,   110,   149,   149,   130,     3,    88,   125,   126,
     108,   159,   109,    49,     3,   190,   191,    94,   108,   126,
       3,     4,    25,    40,    42,    87,   129,   166,    84,     3,
     108,   110,   108,   110,   170,   174,   177,   178,   110,    73,
      43,    94,   152,   110,   149,   149,    94,    51,   127,   128,
     129,    39,   133,   134,     5,   132,    62,    63,    69,   196,
      94,    90,   109,   130,   131,   186,    58,   143,   144,   109,
     109,   109,   108,   110,   109,   167,   168,   130,     8,    13,
     179,   108,     8,   109,   130,   110,   110,   130,   109,    91,
      18,    43,    94,    95,    96,    97,    98,    99,   105,    19,
      89,   135,   136,   110,    24,    82,    62,     8,   191,   110,
     132,     3,   145,   146,   102,   110,   110,   129,   109,   169,
     170,   108,     3,    50,   180,   170,   178,   132,     8,   128,
     130,   131,     9,   109,   130,   131,   130,   131,   130,   131,
     130,   131,   130,   131,   130,   131,    43,   129,    39,    59,
     137,   138,   110,    94,   108,   110,   108,   110,   168,   110,
     110,    91,    91,   132,     9,   109,    59,    19,   141,     3,
       5,     8,   109,   146,   170,   130,   101,   131,   110,   132,
      19,    61,   129,   139,   140,   143,   109,     3,   147,   148,
     110,   139,   109,    14,    28,   108,     8,    94,   108,   110,
     110,   140,   110,   130,   148
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
			if ( (uint64_t)yyvsp[0].m_iValue > -LLONG_MIN )
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

  case 157:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 158:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 159:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 160:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS; ;}
    break;

  case 166:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 167:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 168:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 169:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 171:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 172:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 175:

    { yyval.m_iValue = 1; ;}
    break;

  case 176:

    { yyval.m_iValue = 0; ;}
    break;

  case 177:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 178:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 179:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 180:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 183:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 184:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 185:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 188:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 189:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 192:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 193:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 194:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 195:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 196:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 197:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 198:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 199:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 200:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 201:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 202:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 203:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 204:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 206:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 207:

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

  case 208:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 211:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 213:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 217:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 218:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 221:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 222:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 225:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 226:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 227:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 228:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 229:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 236:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 237:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 238:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 246:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 247:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 248:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 249:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 250:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 251:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 252:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 254:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 255:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 256:

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

