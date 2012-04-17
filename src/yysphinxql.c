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
     TOK_OPTIMIZE = 313,
     TOK_RAND = 314,
     TOK_READ = 315,
     TOK_REPEATABLE = 316,
     TOK_REPLACE = 317,
     TOK_RETURNS = 318,
     TOK_ROLLBACK = 319,
     TOK_RTINDEX = 320,
     TOK_SELECT = 321,
     TOK_SERIALIZABLE = 322,
     TOK_SET = 323,
     TOK_SESSION = 324,
     TOK_SHOW = 325,
     TOK_SONAME = 326,
     TOK_START = 327,
     TOK_STATUS = 328,
     TOK_SUM = 329,
     TOK_TABLES = 330,
     TOK_TO = 331,
     TOK_TRANSACTION = 332,
     TOK_TRUE = 333,
     TOK_TRUNCATE = 334,
     TOK_UNCOMMITTED = 335,
     TOK_UPDATE = 336,
     TOK_VALUES = 337,
     TOK_VARIABLES = 338,
     TOK_WARNINGS = 339,
     TOK_WEIGHT = 340,
     TOK_WHERE = 341,
     TOK_WITHIN = 342,
     TOK_OR = 343,
     TOK_AND = 344,
     TOK_NE = 345,
     TOK_GTE = 346,
     TOK_LTE = 347,
     TOK_NOT = 348,
     TOK_NEG = 349
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
#define TOK_OPTIMIZE 313
#define TOK_RAND 314
#define TOK_READ 315
#define TOK_REPEATABLE 316
#define TOK_REPLACE 317
#define TOK_RETURNS 318
#define TOK_ROLLBACK 319
#define TOK_RTINDEX 320
#define TOK_SELECT 321
#define TOK_SERIALIZABLE 322
#define TOK_SET 323
#define TOK_SESSION 324
#define TOK_SHOW 325
#define TOK_SONAME 326
#define TOK_START 327
#define TOK_STATUS 328
#define TOK_SUM 329
#define TOK_TABLES 330
#define TOK_TO 331
#define TOK_TRANSACTION 332
#define TOK_TRUE 333
#define TOK_TRUNCATE 334
#define TOK_UNCOMMITTED 335
#define TOK_UPDATE 336
#define TOK_VALUES 337
#define TOK_VARIABLES 338
#define TOK_WARNINGS 339
#define TOK_WEIGHT 340
#define TOK_WHERE 341
#define TOK_WITHIN 342
#define TOK_OR 343
#define TOK_AND 344
#define TOK_NE 345
#define TOK_GTE 346
#define TOK_LTE 347
#define TOK_NOT 348
#define TOK_NEG 349




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
#define YYFINAL  100
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   818

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  110
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  90
/* YYNRULES -- Number of rules. */
#define YYNRULES  250
/* YYNRULES -- Number of states. */
#define YYNSTATES  469

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   349

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   102,    91,     2,
     107,   108,   100,    98,   106,    99,   109,   101,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   105,
      94,    92,    95,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    90,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    93,    96,    97,   103,   104
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    54,    56,    58,    69,
      71,    75,    77,    80,    81,    83,    86,    88,    93,    98,
     103,   108,   113,   118,   122,   126,   132,   134,   138,   139,
     141,   144,   146,   150,   155,   159,   163,   169,   176,   180,
     185,   191,   195,   199,   203,   207,   211,   215,   219,   223,
     229,   233,   237,   239,   241,   246,   250,   254,   256,   258,
     261,   263,   266,   268,   272,   273,   275,   279,   280,   282,
     288,   289,   291,   295,   301,   303,   307,   309,   312,   315,
     316,   318,   321,   326,   327,   329,   332,   334,   338,   342,
     346,   352,   359,   363,   365,   369,   373,   375,   377,   379,
     381,   383,   385,   388,   391,   395,   399,   403,   407,   411,
     415,   419,   423,   427,   431,   435,   439,   443,   447,   451,
     455,   459,   463,   465,   470,   475,   479,   486,   493,   495,
     499,   501,   503,   506,   508,   510,   512,   514,   516,   518,
     520,   522,   527,   532,   537,   541,   546,   554,   560,   562,
     564,   566,   568,   570,   572,   574,   576,   578,   581,   588,
     590,   592,   593,   597,   599,   603,   605,   609,   613,   615,
     619,   621,   623,   625,   629,   632,   640,   650,   657,   659,
     663,   665,   669,   671,   675,   676,   679,   681,   685,   689,
     690,   692,   694,   696,   699,   701,   703,   706,   712,   714,
     718,   722,   726,   732,   737,   742,   743,   745,   748,   750,
     754,   758,   761,   768,   769,   771,   773,   776,   779,   782,
     784,   792,   794,   796,   800,   807,   811,   815,   817,   821,
     825
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     111,     0,    -1,   112,    -1,   113,    -1,   113,   105,    -1,
     158,    -1,   166,    -1,   152,    -1,   153,    -1,   156,    -1,
     167,    -1,   176,    -1,   178,    -1,   179,    -1,   182,    -1,
     187,    -1,   191,    -1,   193,    -1,   194,    -1,   195,    -1,
     188,    -1,   196,    -1,   198,    -1,   199,    -1,   114,    -1,
     113,   105,   114,    -1,   115,    -1,   149,    -1,    66,   116,
      34,   120,   121,   129,   131,   133,   137,   139,    -1,   117,
      -1,   116,   106,   117,    -1,   100,    -1,   119,   118,    -1,
      -1,     3,    -1,    12,     3,    -1,   145,    -1,    15,   107,
     145,   108,    -1,    50,   107,   145,   108,    -1,    52,   107,
     145,   108,    -1,    74,   107,   145,   108,    -1,    39,   107,
     145,   108,    -1,    23,   107,   100,   108,    -1,    85,   107,
     108,    -1,    38,   107,   108,    -1,    23,   107,    28,     3,
     108,    -1,     3,    -1,   120,   106,     3,    -1,    -1,   122,
      -1,    86,   123,    -1,   124,    -1,   123,    89,   124,    -1,
      49,   107,     8,   108,    -1,   125,    92,   126,    -1,   125,
      93,   126,    -1,   125,    41,   107,   128,   108,    -1,   125,
     103,    41,   107,   128,   108,    -1,   125,    41,     9,    -1,
     125,   103,    41,     9,    -1,   125,    17,   126,    89,   126,
      -1,   125,    95,   126,    -1,   125,    94,   126,    -1,   125,
      96,   126,    -1,   125,    97,   126,    -1,   125,    92,   127,
      -1,   125,    93,   127,    -1,   125,    95,   127,    -1,   125,
      94,   127,    -1,   125,    17,   127,    89,   127,    -1,   125,
      96,   127,    -1,   125,    97,   127,    -1,     3,    -1,     4,
      -1,    23,   107,   100,   108,    -1,    38,   107,   108,    -1,
      85,   107,   108,    -1,    40,    -1,     5,    -1,    99,     5,
      -1,     6,    -1,    99,     6,    -1,   126,    -1,   128,   106,
     126,    -1,    -1,   130,    -1,    37,    18,   125,    -1,    -1,
     132,    -1,    87,    37,    57,    18,   135,    -1,    -1,   134,
      -1,    57,    18,   135,    -1,    57,    18,    59,   107,   108,
      -1,   136,    -1,   135,   106,   136,    -1,   125,    -1,   125,
      13,    -1,   125,    26,    -1,    -1,   138,    -1,    48,     5,
      -1,    48,     5,   106,     5,    -1,    -1,   140,    -1,    56,
     141,    -1,   142,    -1,   141,   106,   142,    -1,     3,    92,
       3,    -1,     3,    92,     5,    -1,     3,    92,   107,   143,
     108,    -1,     3,    92,     3,   107,     8,   108,    -1,     3,
      92,     8,    -1,   144,    -1,   143,   106,   144,    -1,     3,
      92,   126,    -1,     3,    -1,     4,    -1,    40,    -1,     5,
      -1,     6,    -1,     9,    -1,    99,   145,    -1,   103,   145,
      -1,   145,    98,   145,    -1,   145,    99,   145,    -1,   145,
     100,   145,    -1,   145,   101,   145,    -1,   145,    94,   145,
      -1,   145,    95,   145,    -1,   145,    91,   145,    -1,   145,
      90,   145,    -1,   145,   102,   145,    -1,   145,    29,   145,
      -1,   145,    53,   145,    -1,   145,    97,   145,    -1,   145,
      96,   145,    -1,   145,    92,   145,    -1,   145,    93,   145,
      -1,   145,    89,   145,    -1,   145,    88,   145,    -1,   107,
     145,   108,    -1,   146,    -1,     3,   107,   147,   108,    -1,
      41,   107,   147,   108,    -1,     3,   107,   108,    -1,    52,
     107,   145,   106,   145,   108,    -1,    50,   107,   145,   106,
     145,   108,    -1,   148,    -1,   147,   106,   148,    -1,   145,
      -1,     8,    -1,    70,   150,    -1,    84,    -1,    73,    -1,
      51,    -1,     3,    -1,    55,    -1,     8,    -1,     5,    -1,
       6,    -1,    68,     3,    92,   155,    -1,    68,     3,    92,
     154,    -1,    68,     3,    92,    55,    -1,    68,    54,   151,
      -1,    68,    10,    92,   151,    -1,    68,    36,     9,    92,
     107,   128,   108,    -1,    68,    36,     3,    92,   154,    -1,
       3,    -1,     8,    -1,    78,    -1,    31,    -1,   126,    -1,
      21,    -1,    64,    -1,   157,    -1,    16,    -1,    72,    77,
      -1,   159,    45,     3,   160,    82,   162,    -1,    43,    -1,
      62,    -1,    -1,   107,   161,   108,    -1,   125,    -1,   161,
     106,   125,    -1,   163,    -1,   162,   106,   163,    -1,   107,
     164,   108,    -1,   165,    -1,   164,   106,   165,    -1,   126,
      -1,   127,    -1,     8,    -1,   107,   128,   108,    -1,   107,
     108,    -1,    25,    34,   120,    86,    40,    92,   126,    -1,
      25,    34,   120,    86,    40,    41,   107,   128,   108,    -1,
      19,     3,   107,   168,   171,   108,    -1,   169,    -1,   168,
     106,   169,    -1,   165,    -1,   107,   170,   108,    -1,     8,
      -1,   170,   106,     8,    -1,    -1,   106,   172,    -1,   173,
      -1,   172,   106,   173,    -1,   165,   174,   175,    -1,    -1,
      12,    -1,     3,    -1,    48,    -1,   177,     3,    -1,    27,
      -1,    26,    -1,    70,    75,    -1,    81,   120,    68,   180,
     122,    -1,   181,    -1,   180,   106,   181,    -1,     3,    92,
     126,    -1,     3,    92,   127,    -1,     3,    92,   107,   128,
     108,    -1,     3,    92,   107,   108,    -1,    70,   189,    83,
     183,    -1,    -1,   184,    -1,    86,   185,    -1,   186,    -1,
     185,    88,   186,    -1,     3,    92,     8,    -1,    70,    20,
      -1,    68,   189,    77,    46,    47,   190,    -1,    -1,    36,
      -1,    69,    -1,    60,    80,    -1,    60,    22,    -1,    61,
      60,    -1,    67,    -1,    24,    35,     3,    63,   192,    71,
       8,    -1,    44,    -1,    32,    -1,    30,    35,     3,    -1,
      14,    42,     3,    76,    65,     3,    -1,    33,    65,     3,
      -1,    66,   197,   137,    -1,    10,    -1,    10,   109,     3,
      -1,    79,    65,     3,    -1,    58,    42,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   123,   123,   124,   125,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   153,   154,   158,   159,   163,   178,
     179,   183,   184,   187,   189,   190,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   207,   208,   211,   213,
     217,   221,   222,   226,   231,   238,   246,   254,   263,   268,
     273,   278,   283,   288,   293,   298,   299,   300,   301,   306,
     311,   316,   324,   325,   330,   336,   342,   348,   357,   358,
     362,   363,   367,   373,   379,   381,   385,   392,   394,   398,
     404,   406,   410,   414,   421,   422,   426,   427,   428,   431,
     433,   437,   442,   449,   451,   455,   459,   460,   464,   469,
     474,   480,   485,   493,   498,   505,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   535,   536,   537,   538,
     539,   540,   541,   545,   546,   547,   548,   549,   553,   554,
     558,   559,   565,   569,   570,   571,   577,   578,   579,   580,
     581,   585,   590,   595,   600,   601,   605,   610,   618,   619,
     623,   624,   625,   639,   640,   641,   645,   646,   652,   660,
     661,   664,   666,   670,   671,   675,   676,   680,   684,   685,
     689,   690,   691,   692,   693,   699,   707,   721,   729,   733,
     740,   741,   748,   758,   764,   766,   770,   775,   779,   786,
     788,   792,   793,   799,   807,   808,   814,   820,   828,   829,
     833,   837,   841,   845,   855,   861,   862,   866,   870,   871,
     875,   879,   886,   892,   893,   894,   898,   899,   900,   901,
     907,   918,   919,   923,   934,   946,   957,   965,   966,   975,
     986
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
  "TOK_OPTIMIZE", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", 
  "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", 
  "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
  "TOK_START", "TOK_STATUS", "TOK_SUM", "TOK_TABLES", "TOK_TO", 
  "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", 
  "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", 
  "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", 
  "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", 
  "')'", "'.'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "expr_ident", "const_int", 
  "const_float", "const_list", "opt_group_clause", "group_clause", 
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
  "sysvar_name", "truncate", "optimize_index", 0
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
     124,    38,    61,   345,    60,    62,   346,   347,    43,    45,
      42,    47,    37,   348,   349,    59,    44,    40,    41,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   110,   111,   111,   111,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   113,   113,   114,   114,   115,   116,
     116,   117,   117,   118,   118,   118,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   120,   120,   121,   121,
     122,   123,   123,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   125,   126,   126,
     127,   127,   128,   128,   129,   129,   130,   131,   131,   132,
     133,   133,   134,   134,   135,   135,   136,   136,   136,   137,
     137,   138,   138,   139,   139,   140,   141,   141,   142,   142,
     142,   142,   142,   143,   143,   144,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   146,   146,   146,   146,   146,   147,   147,
     148,   148,   149,   150,   150,   150,   151,   151,   151,   151,
     151,   152,   152,   152,   152,   152,   153,   153,   154,   154,
     155,   155,   155,   156,   156,   156,   157,   157,   158,   159,
     159,   160,   160,   161,   161,   162,   162,   163,   164,   164,
     165,   165,   165,   165,   165,   166,   166,   167,   168,   168,
     169,   169,   170,   170,   171,   171,   172,   172,   173,   174,
     174,   175,   175,   176,   177,   177,   178,   179,   180,   180,
     181,   181,   181,   181,   182,   183,   183,   184,   185,   185,
     186,   187,   188,   189,   189,   189,   190,   190,   190,   190,
     191,   192,   192,   193,   194,   195,   196,   197,   197,   198,
     199
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,    10,     1,
       3,     1,     2,     0,     1,     2,     1,     4,     4,     4,
       4,     4,     4,     3,     3,     5,     1,     3,     0,     1,
       2,     1,     3,     4,     3,     3,     5,     6,     3,     4,
       5,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       3,     3,     1,     1,     4,     3,     3,     1,     1,     2,
       1,     2,     1,     3,     0,     1,     3,     0,     1,     5,
       0,     1,     3,     5,     1,     3,     1,     2,     2,     0,
       1,     2,     4,     0,     1,     2,     1,     3,     3,     3,
       5,     6,     3,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     4,     4,     3,     6,     6,     1,     3,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     4,     4,     3,     4,     7,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     6,     1,
       1,     0,     3,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     3,     2,     7,     9,     6,     1,     3,
       1,     3,     1,     3,     0,     2,     1,     3,     3,     0,
       1,     1,     1,     2,     1,     1,     2,     5,     1,     3,
       3,     3,     5,     4,     4,     0,     1,     2,     1,     3,
       3,     2,     6,     0,     1,     1,     2,     2,     2,     1,
       7,     1,     1,     3,     6,     3,     3,     1,     3,     3,
       3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   176,     0,   173,     0,     0,   215,   214,     0,
       0,   179,     0,   180,   174,     0,   233,   233,     0,     0,
       0,     0,     2,     3,    24,    26,    27,     7,     8,     9,
     175,     5,     0,     6,    10,    11,     0,    12,    13,    14,
      15,    20,    16,    17,    18,    19,    21,    22,    23,     0,
       0,     0,     0,     0,     0,     0,   116,   117,   119,   120,
     121,   247,     0,     0,     0,     0,   118,     0,     0,     0,
       0,     0,     0,    31,     0,     0,     0,    29,    33,    36,
     142,    99,     0,     0,   234,     0,   235,     0,   231,   234,
     155,   154,   216,   153,   152,     0,   177,     0,    46,     0,
       1,     4,     0,   213,     0,     0,     0,     0,   243,   245,
     250,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   122,   123,     0,     0,     0,    34,
       0,    32,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     246,   100,     0,     0,     0,     0,   156,   159,   160,   158,
     157,   164,     0,   225,   249,     0,     0,     0,     0,    25,
     181,     0,    78,    80,   192,     0,     0,   190,   191,   200,
     204,   198,     0,     0,   151,   145,   150,     0,   148,   248,
       0,     0,     0,    44,     0,     0,     0,     0,     0,    43,
       0,     0,   141,    48,    30,    35,   133,   134,   140,   139,
     131,   130,   137,   138,   128,   129,   136,   135,   124,   125,
     126,   127,   132,   101,   168,   169,   171,   163,   170,     0,
     172,   162,   161,   165,     0,     0,     0,     0,   224,   226,
       0,     0,   218,    47,     0,     0,     0,    79,    81,   202,
     194,    82,     0,     0,     0,     0,   242,   241,     0,     0,
       0,   143,    37,     0,    42,    41,   144,     0,    38,     0,
      39,    40,     0,     0,     0,    84,    49,     0,   167,     0,
       0,     0,   227,   228,     0,     0,   217,    72,    73,     0,
       0,    77,     0,   183,     0,     0,   244,     0,   193,     0,
     201,   200,   199,   205,   206,   197,     0,     0,     0,   149,
      45,     0,     0,     0,    50,    51,     0,     0,    87,    85,
     102,     0,     0,     0,   239,   232,     0,     0,     0,   220,
     221,   219,     0,     0,     0,     0,   182,     0,   178,   185,
      83,   203,   210,     0,     0,   240,     0,   195,   147,   146,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    90,    88,   166,   237,   236,   238,   230,
     229,   223,     0,     0,    75,    76,   184,     0,     0,   188,
       0,   211,   212,   208,   209,   207,     0,     0,    52,     0,
       0,    58,     0,    54,    65,    55,    66,    62,    68,    61,
      67,    63,    70,    64,    71,     0,    86,     0,     0,    99,
      91,   222,    74,     0,   187,   186,   196,    53,     0,     0,
       0,    59,     0,     0,     0,   103,   189,    60,     0,    69,
      56,     0,     0,     0,    96,    92,    94,     0,    28,   104,
      57,    89,     0,    97,    98,     0,     0,   105,   106,    93,
      95,     0,     0,   108,   109,   112,     0,   107,     0,     0,
       0,   113,     0,     0,     0,   110,   111,   115,   114
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,    76,    77,   131,    78,
      99,   275,   276,   314,   315,   434,   251,   178,   252,   318,
     319,   363,   364,   409,   410,   435,   436,   150,   151,   438,
     439,   447,   448,   460,   461,    79,    80,   187,   188,    26,
      94,   161,    27,    28,   231,   232,    29,    30,    31,    32,
     245,   294,   338,   339,   378,   179,    33,    34,   180,   181,
     253,   255,   303,   304,   343,   383,    35,    36,    37,    38,
     241,   242,    39,   238,   239,   282,   283,    40,    41,    87,
     325,    42,   258,    43,    44,    45,    46,    81,    47,    48
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -265
static const short yypact[] =
{
     737,   -25,  -265,    59,  -265,    48,   113,  -265,  -265,   126,
     111,  -265,   139,  -265,  -265,   114,   227,   718,   109,   130,
     195,   204,  -265,   103,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,   175,  -265,  -265,  -265,   223,  -265,  -265,  -265,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,   242,
     148,   258,   195,   267,   279,   280,   177,  -265,  -265,  -265,
    -265,   189,   192,   196,   198,   203,  -265,   208,   209,   210,
     218,   220,   271,  -265,   271,   271,    25,  -265,   171,   548,
    -265,   254,   238,   240,    17,   164,  -265,   256,  -265,  -265,
    -265,  -265,  -265,  -265,  -265,   251,  -265,   333,  -265,   -66,
    -265,   105,   335,  -265,   263,    52,   278,   -58,  -265,  -265,
    -265,    98,   339,   271,     9,   235,   271,   137,   271,   271,
     271,   236,   239,   243,  -265,  -265,   346,   195,   219,  -265,
     344,  -265,   271,   271,   271,   271,   271,   271,   271,   271,
     271,   271,   271,   271,   271,   271,   271,   271,   271,   347,
    -265,  -265,   207,   164,   257,   259,  -265,  -265,  -265,  -265,
    -265,  -265,   307,   270,  -265,   352,   354,   219,   -28,  -265,
     252,   296,  -265,  -265,  -265,   273,    13,  -265,  -265,  -265,
     260,  -265,    -8,   320,  -265,  -265,   548,   -81,  -265,  -265,
     373,   359,   255,  -265,   396,   -65,   295,   316,   423,  -265,
     271,   271,  -265,   -52,  -265,  -265,  -265,  -265,   573,   598,
     623,   648,   632,   632,   555,   555,   555,   555,   -15,   -15,
    -265,  -265,  -265,   261,  -265,  -265,  -265,  -265,  -265,   360,
    -265,  -265,  -265,  -265,   165,   264,   317,   365,  -265,  -265,
     281,   -51,  -265,  -265,   297,   290,   374,  -265,  -265,  -265,
    -265,  -265,    24,    88,    52,   272,  -265,  -265,   308,    22,
     137,  -265,  -265,   292,  -265,  -265,  -265,   271,  -265,   271,
    -265,  -265,   498,   523,   291,   361,  -265,   376,  -265,    -1,
     124,   327,   332,  -265,    83,   352,  -265,  -265,  -265,   314,
     321,  -265,   322,  -265,   101,   323,  -265,    -1,  -265,   415,
    -265,   132,  -265,   325,  -265,  -265,   419,   326,    -1,  -265,
    -265,   448,   473,   343,   362,  -265,   194,   414,   366,  -265,
    -265,   110,   -19,   395,  -265,  -265,   449,   365,    14,  -265,
    -265,  -265,   356,   350,   351,   297,  -265,   157,   372,  -265,
    -265,  -265,  -265,   108,   157,  -265,    -1,  -265,  -265,  -265,
     452,   291,    26,     1,    26,    26,    26,    26,    26,    26,
     438,   297,   443,   425,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,   121,   375,  -265,  -265,  -265,    28,   125,  -265,
     323,  -265,  -265,  -265,   487,  -265,   135,   392,  -265,   416,
     417,  -265,    -1,  -265,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,  -265,  -265,  -265,     3,  -265,   446,   489,   254,
    -265,  -265,  -265,   157,  -265,  -265,  -265,  -265,    -1,     0,
     140,  -265,    -1,   490,   269,   453,  -265,  -265,   504,  -265,
    -265,   159,   297,   421,    16,   424,  -265,   526,  -265,  -265,
    -265,   424,   426,  -265,  -265,   297,   440,   427,  -265,  -265,
    -265,     8,   526,   428,  -265,  -265,   550,  -265,   546,   463,
     160,  -265,   450,    -1,   550,  -265,  -265,  -265,  -265
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -265,  -265,  -265,  -265,   456,  -265,  -265,   431,  -265,  -265,
     -22,  -265,   319,  -265,   228,  -235,  -105,  -262,  -264,  -265,
    -265,  -265,  -265,  -265,  -265,   146,   138,   173,  -265,  -265,
    -265,  -265,   128,  -265,   141,   -67,  -265,   468,   349,  -265,
    -265,   454,  -265,  -265,   369,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,  -265,   226,  -265,  -253,  -265,  -265,  -265,   377,
    -265,  -265,  -265,   266,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,   345,  -265,  -265,  -265,  -265,   301,  -265,  -265,   615,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -210
static const short yytable[] =
{
     177,   301,   165,   366,   172,   124,   173,   125,   126,   293,
     391,   453,   421,   454,   132,   321,   455,    49,   172,   172,
     154,   249,   330,    90,   256,   260,   155,   261,   183,   443,
     107,   172,   173,   172,   274,   274,   257,   191,   133,   316,
     166,   260,   444,   266,   186,    91,   190,   230,   166,   194,
     186,   196,   197,   198,   166,   285,    93,   172,   173,   127,
     174,   367,    50,   307,   372,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   386,    51,   379,   146,   147,   148,   172,   173,
     390,   384,   394,   396,   398,   400,   402,   404,   229,   428,
     376,    56,    57,    58,    59,   203,   184,    60,   392,   192,
     422,   381,   229,   229,   308,   456,   316,    56,    57,    58,
      59,   250,   371,    60,    61,   175,   406,   229,   420,    62,
     297,   128,   298,   272,   273,  -209,   250,    63,    66,    67,
      56,    57,    58,    59,   342,   184,    60,    52,   122,   177,
     123,   175,    64,    65,    66,    67,   382,   429,   431,   176,
     426,    53,   172,   173,    68,   174,    69,   156,   224,   157,
     158,   167,   159,   225,   129,   168,    54,    66,    67,   329,
    -209,    55,   175,   130,   322,   323,    96,   122,    70,   123,
     328,   324,   340,   186,   299,    97,   300,    72,    98,    71,
     311,    74,   312,   347,   100,    75,   185,   335,   101,   336,
     224,   352,   172,    72,    73,   225,   297,    74,   365,   160,
     102,    75,    56,    57,    58,    59,   103,   297,    60,   411,
      82,   413,   177,   414,    62,   353,    72,    83,   226,   177,
      74,   297,    63,   416,    75,   104,   297,   389,   430,   393,
     395,   397,   399,   401,   403,   105,   175,    64,    65,    66,
      67,   106,   227,    84,   377,   297,   464,   440,   465,    68,
     108,    69,   287,   288,    56,    57,    58,    59,   247,   248,
      60,    85,   109,   110,   111,   228,   354,   355,   356,   357,
     358,   359,   289,    70,   287,   288,    86,   360,   112,   113,
     287,   288,   149,   114,    71,   115,   229,   290,   177,   291,
     116,    66,    67,   427,   289,   117,   118,   119,    72,    73,
     289,   122,    74,   123,   132,   120,    75,   121,   433,   290,
     152,   291,   153,   162,   163,   290,   164,   291,   170,   171,
     313,   182,   189,   193,   199,   132,   200,   205,   133,   234,
     201,   235,   223,   236,   292,   240,   237,   243,   467,   244,
     259,   246,   263,   264,   280,   247,   254,   277,   281,   133,
      72,   279,   295,   284,    74,   132,   292,   296,    75,   306,
     305,   320,   292,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   317,   133,
     310,   267,   132,   268,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   326,
     327,   332,   269,   341,   270,   132,   133,   345,   333,   334,
     337,   344,   361,   346,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   133,
     350,   351,   132,   362,   202,   368,   373,   369,   374,   375,
     387,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   133,   132,   380,   405,
     407,   262,   408,   412,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   342,
     417,   133,   132,   423,   265,   418,   419,   424,   432,   437,
     248,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   133,   132,   442,   446,
     445,   271,   451,   452,   449,   458,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   133,   132,   459,   462,   463,   348,   169,   466,   204,
     286,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   133,   132,   441,   388,
     457,   349,   425,   450,   132,   195,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   133,   132,   278,   267,   468,   415,   233,   133,   309,
     385,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   133,   132,   370,   269,
     331,   302,    95,     0,     0,     0,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   133,   132,   144,   145,   146,   147,   148,     0,     0,
       0,   132,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   133,   132,     0,     0,
       0,     0,     0,     0,     0,   133,     0,     0,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   133,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   140,   141,   142,   143,
     144,   145,   146,   147,   148,     0,     0,     0,    88,     0,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,     1,     0,     2,    89,     0,     3,     0,     4,     0,
       0,     5,     6,     7,     8,     0,     0,     9,     0,    90,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,     0,     0,     0,     0,     0,     0,    86,     0,     0,
       0,    91,     0,    92,     0,    12,     0,     0,     0,    13,
       0,    14,    93,    15,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,     0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     105,   254,    68,    22,     5,    72,     6,    74,    75,   244,
       9,     3,     9,     5,    29,   279,     8,    42,     5,     5,
       3,     8,   284,    51,    32,   106,     9,   108,    86,    13,
      52,     5,     6,     5,    86,    86,    44,    28,    53,   274,
     106,   106,    26,   108,   111,    73,   113,   152,   106,   116,
     117,   118,   119,   120,   106,   106,    84,     5,     6,    34,
       8,    80,     3,    41,   328,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   346,    35,   337,   100,   101,   102,     5,     6,
     352,   344,   354,   355,   356,   357,   358,   359,    99,    99,
     335,     3,     4,     5,     6,   127,     8,     9,   107,   100,
     107,     3,    99,    99,    92,   107,   351,     3,     4,     5,
       6,   108,   108,     9,    10,    99,   361,    99,   392,    15,
     106,   106,   108,   200,   201,     3,   108,    23,    40,    41,
       3,     4,     5,     6,    12,     8,     9,    34,    50,   254,
      52,    99,    38,    39,    40,    41,    48,   419,   422,   107,
     413,    35,     5,     6,    50,     8,    52,     3,     3,     5,
       6,    66,     8,     8,     3,    70,    65,    40,    41,   284,
      48,    42,    99,    12,    60,    61,    77,    50,    74,    52,
     107,    67,   297,   260,   106,    65,   108,    99,     3,    85,
     267,   103,   269,   308,     0,   107,   108,   106,   105,   108,
       3,    17,     5,    99,   100,     8,   106,   103,   108,    55,
      45,   107,     3,     4,     5,     6,     3,   106,     9,   108,
       3,   106,   337,   108,    15,    41,    99,    10,    31,   344,
     103,   106,    23,   108,   107,     3,   106,   352,   108,   354,
     355,   356,   357,   358,   359,   107,    99,    38,    39,    40,
      41,     3,    55,    36,   107,   106,   106,   108,   108,    50,
       3,    52,     3,     4,     3,     4,     5,     6,     5,     6,
       9,    54,     3,     3,   107,    78,    92,    93,    94,    95,
      96,    97,    23,    74,     3,     4,    69,   103,   109,   107,
       3,     4,    48,   107,    85,   107,    99,    38,   413,    40,
     107,    40,    41,   418,    23,   107,   107,   107,    99,   100,
      23,    50,   103,    52,    29,   107,   107,   107,    59,    38,
      92,    40,    92,    77,    83,    38,     3,    40,     3,    76,
      49,    63,     3,   108,   108,    29,   107,     3,    53,    92,
     107,    92,     5,    46,    85,     3,    86,     3,   463,   107,
      40,    65,     3,   108,    47,     5,   106,   106,     3,    53,
      99,   107,    82,    92,   103,    29,    85,     3,   107,    71,
     108,     5,    85,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,    37,    53,
     108,   106,    29,   108,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,    92,
      88,   107,   106,     8,   108,    29,    53,     8,   107,   107,
     107,   106,    18,   107,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,    53,
     107,    89,    29,    87,   108,    60,   100,     8,   108,   108,
       8,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,    53,    29,   106,    41,
      37,   108,    57,   108,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,    12,
     108,    53,    29,    57,   108,    89,    89,    18,    18,    56,
       6,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,    53,    29,   107,     3,
     106,   108,    92,   106,   108,   107,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,    53,    29,     3,     8,    92,   108,   101,   108,   128,
     241,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,    53,    29,   432,   351,
     452,   108,   409,   445,    29,   117,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,    53,    29,   234,   106,   464,   380,   153,    53,   260,
     344,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,    53,    29,   327,   106,
     285,   254,    17,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,    53,    29,    98,    99,   100,   101,   102,    -1,    -1,
      -1,    29,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,    53,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,    94,    95,    96,    97,
      98,    99,   100,   101,   102,    -1,    -1,    -1,    20,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,    14,    -1,    16,    36,    -1,    19,    -1,    21,    -1,
      -1,    24,    25,    26,    27,    -1,    -1,    30,    -1,    51,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    73,    -1,    75,    -1,    58,    -1,    -1,    -1,    62,
      -1,    64,    84,    66,    -1,    68,    -1,    70,    -1,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,    81
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    16,    19,    21,    24,    25,    26,    27,    30,
      33,    43,    58,    62,    64,    66,    68,    70,    72,    79,
      81,   111,   112,   113,   114,   115,   149,   152,   153,   156,
     157,   158,   159,   166,   167,   176,   177,   178,   179,   182,
     187,   188,   191,   193,   194,   195,   196,   198,   199,    42,
       3,    35,    34,    35,    65,    42,     3,     4,     5,     6,
       9,    10,    15,    23,    38,    39,    40,    41,    50,    52,
      74,    85,    99,   100,   103,   107,   116,   117,   119,   145,
     146,   197,     3,    10,    36,    54,    69,   189,    20,    36,
      51,    73,    75,    84,   150,   189,    77,    65,     3,   120,
       0,   105,    45,     3,     3,   107,     3,   120,     3,     3,
       3,   107,   109,   107,   107,   107,   107,   107,   107,   107,
     107,   107,    50,    52,   145,   145,   145,    34,   106,     3,
      12,   118,    29,    53,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,    48,
     137,   138,    92,    92,     3,     9,     3,     5,     6,     8,
      55,   151,    77,    83,     3,    68,   106,    66,    70,   114,
       3,    76,     5,     6,     8,    99,   107,   126,   127,   165,
     168,   169,    63,    86,     8,   108,   145,   147,   148,     3,
     145,    28,   100,   108,   145,   147,   145,   145,   145,   108,
     107,   107,   108,   120,   117,     3,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,     5,     3,     8,    31,    55,    78,    99,
     126,   154,   155,   151,    92,    92,    46,    86,   183,   184,
       3,   180,   181,     3,   107,   160,    65,     5,     6,     8,
     108,   126,   128,   170,   106,   171,    32,    44,   192,    40,
     106,   108,   108,     3,   108,   108,   108,   106,   108,   106,
     108,   108,   145,   145,    86,   121,   122,   106,   154,   107,
      47,     3,   185,   186,    92,   106,   122,     3,     4,    23,
      38,    40,    85,   125,   161,    82,     3,   106,   108,   106,
     108,   165,   169,   172,   173,   108,    71,    41,    92,   148,
     108,   145,   145,    49,   123,   124,   125,    37,   129,   130,
       5,   128,    60,    61,    67,   190,    92,    88,   107,   126,
     127,   181,   107,   107,   107,   106,   108,   107,   162,   163,
     126,     8,    12,   174,   106,     8,   107,   126,   108,   108,
     107,    89,    17,    41,    92,    93,    94,    95,    96,    97,
     103,    18,    87,   131,   132,   108,    22,    80,    60,     8,
     186,   108,   128,   100,   108,   108,   125,   107,   164,   165,
     106,     3,    48,   175,   165,   173,   128,     8,   124,   126,
     127,     9,   107,   126,   127,   126,   127,   126,   127,   126,
     127,   126,   127,   126,   127,    41,   125,    37,    57,   133,
     134,   108,   108,   106,   108,   163,   108,   108,    89,    89,
     128,     9,   107,    57,    18,   137,   165,   126,    99,   127,
     108,   128,    18,    59,   125,   135,   136,    56,   139,   140,
     108,   135,   107,    13,    26,   106,     3,   141,   142,   108,
     136,    92,   106,     3,     5,     8,   107,   142,   107,     3,
     143,   144,     8,    92,   106,   108,   108,   126,   144
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

  case 24:

    { pParser->PushQuery(); ;}
    break;

  case 25:

    { pParser->PushQuery(); ;}
    break;

  case 28:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 31:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 34:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 36:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 37:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 38:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 39:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 40:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 41:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 42:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 43:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 44:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 45:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 47:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 53:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 54:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 55:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 56:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 57:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 58:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 59:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 60:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 64:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 73:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 74:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 76:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 77:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 78:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 79:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 80:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 81:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 82:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 83:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 86:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 89:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 92:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 93:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 95:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 97:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 98:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 101:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 102:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 110:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 111:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 112:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 113:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 114:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 115:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 117:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 118:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 122:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 123:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 141:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 143:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 144:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 145:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 146:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 147:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 153:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 154:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 155:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 161:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 162:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 163:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 164:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 165:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 166:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 167:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 170:

    { yyval.m_iValue = 1; ;}
    break;

  case 171:

    { yyval.m_iValue = 0; ;}
    break;

  case 172:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 175:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 178:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 179:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 180:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 183:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 184:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 187:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 188:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 189:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 190:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 191:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 192:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 193:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 194:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 195:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 197:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 198:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 199:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 201:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 202:

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

  case 203:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 206:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 208:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 212:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 213:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 216:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 217:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 220:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 221:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 222:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 223:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 224:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 231:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 232:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 240:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 241:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 242:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 243:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 244:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 245:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 246:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 248:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 249:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 250:

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

