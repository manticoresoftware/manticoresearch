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
     TOK_STRING = 331,
     TOK_SUM = 332,
     TOK_TABLES = 333,
     TOK_TO = 334,
     TOK_TRANSACTION = 335,
     TOK_TRUE = 336,
     TOK_TRUNCATE = 337,
     TOK_UNCOMMITTED = 338,
     TOK_UPDATE = 339,
     TOK_VALUES = 340,
     TOK_VARIABLES = 341,
     TOK_WARNINGS = 342,
     TOK_WEIGHT = 343,
     TOK_WHERE = 344,
     TOK_WITHIN = 345,
     TOK_OR = 346,
     TOK_AND = 347,
     TOK_NE = 348,
     TOK_GTE = 349,
     TOK_LTE = 350,
     TOK_NOT = 351,
     TOK_NEG = 352
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
#define TOK_STRING 331
#define TOK_SUM 332
#define TOK_TABLES 333
#define TOK_TO 334
#define TOK_TRANSACTION 335
#define TOK_TRUE 336
#define TOK_TRUNCATE 337
#define TOK_UNCOMMITTED 338
#define TOK_UPDATE 339
#define TOK_VALUES 340
#define TOK_VARIABLES 341
#define TOK_WARNINGS 342
#define TOK_WEIGHT 343
#define TOK_WHERE 344
#define TOK_WITHIN 345
#define TOK_OR 346
#define TOK_AND 347
#define TOK_NE 348
#define TOK_GTE 349
#define TOK_LTE 350
#define TOK_NOT 351
#define TOK_NEG 352




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
#define YYLAST   878

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  115
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  92
/* YYNRULES -- Number of rules. */
#define YYNRULES  257
/* YYNRULES -- Number of states. */
#define YYNSTATES  486

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   352

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   105,    94,     2,
     110,   111,   103,   101,   109,   102,   114,   104,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   108,
      97,    95,    98,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   112,    93,   113,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    96,    99,
     100,   106,   107
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
     822,   826,   833,   837,   841,   843,   847,   851
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     116,     0,    -1,   117,    -1,   118,    -1,   118,   108,    -1,
     164,    -1,   172,    -1,   158,    -1,   159,    -1,   162,    -1,
     173,    -1,   182,    -1,   184,    -1,   185,    -1,   188,    -1,
     193,    -1,   194,    -1,   198,    -1,   200,    -1,   201,    -1,
     202,    -1,   195,    -1,   203,    -1,   205,    -1,   206,    -1,
     119,    -1,   118,   108,   119,    -1,   120,    -1,   155,    -1,
      68,   121,    36,   125,   126,   134,   136,   138,   142,   144,
      -1,   122,    -1,   121,   109,   122,    -1,   103,    -1,   124,
     123,    -1,    -1,     3,    -1,    13,     3,    -1,   150,    -1,
      16,   110,   150,   111,    -1,    52,   110,   150,   111,    -1,
      54,   110,   150,   111,    -1,    77,   110,   150,   111,    -1,
      41,   110,   150,   111,    -1,    25,   110,   103,   111,    -1,
      88,   110,   111,    -1,    40,   110,   111,    -1,    25,   110,
      30,     3,   111,    -1,     3,    -1,   125,   109,     3,    -1,
      -1,   127,    -1,    89,   128,    -1,   129,    -1,   128,    92,
     129,    -1,    51,   110,     8,   111,    -1,   130,    95,   131,
      -1,   130,    96,   131,    -1,   130,    43,   110,   133,   111,
      -1,   130,   106,    43,   110,   133,   111,    -1,   130,    43,
       9,    -1,   130,   106,    43,     9,    -1,   130,    18,   131,
      92,   131,    -1,   130,    98,   131,    -1,   130,    97,   131,
      -1,   130,    99,   131,    -1,   130,   100,   131,    -1,   130,
      95,   132,    -1,   130,    96,   132,    -1,   130,    98,   132,
      -1,   130,    97,   132,    -1,   130,    18,   132,    92,   132,
      -1,   130,    99,   132,    -1,   130,   100,   132,    -1,     3,
      -1,     4,    -1,    25,   110,   103,   111,    -1,    40,   110,
     111,    -1,    88,   110,   111,    -1,    42,    -1,     5,    -1,
     102,     5,    -1,     6,    -1,   102,     6,    -1,   131,    -1,
     133,   109,   131,    -1,    -1,   135,    -1,    39,    19,   130,
      -1,    -1,   137,    -1,    90,    39,    59,    19,   140,    -1,
      -1,   139,    -1,    59,    19,   140,    -1,    59,    19,    61,
     110,   111,    -1,   141,    -1,   140,   109,   141,    -1,   130,
      -1,   130,    14,    -1,   130,    28,    -1,    -1,   143,    -1,
      50,     5,    -1,    50,     5,   109,     5,    -1,    -1,   145,
      -1,    58,   146,    -1,   147,    -1,   146,   109,   147,    -1,
       3,    95,     3,    -1,     3,    95,     5,    -1,     3,    95,
     110,   148,   111,    -1,     3,    95,     3,   110,     8,   111,
      -1,     3,    95,     8,    -1,   149,    -1,   148,   109,   149,
      -1,     3,    95,   131,    -1,     3,    -1,     4,    -1,    42,
      -1,     5,    -1,     6,    -1,     9,    -1,   102,   150,    -1,
     106,   150,    -1,   150,   101,   150,    -1,   150,   102,   150,
      -1,   150,   103,   150,    -1,   150,   104,   150,    -1,   150,
      97,   150,    -1,   150,    98,   150,    -1,   150,    94,   150,
      -1,   150,    93,   150,    -1,   150,   105,   150,    -1,   150,
      31,   150,    -1,   150,    55,   150,    -1,   150,   100,   150,
      -1,   150,    99,   150,    -1,   150,    95,   150,    -1,   150,
      96,   150,    -1,   150,    92,   150,    -1,   150,    91,   150,
      -1,   110,   150,   111,    -1,   112,   154,   113,    -1,   151,
      -1,     3,   110,   152,   111,    -1,    43,   110,   152,   111,
      -1,     3,   110,   111,    -1,    54,   110,   150,   109,   150,
     111,    -1,    52,   110,   150,   109,   150,   111,    -1,   153,
      -1,   152,   109,   153,    -1,   150,    -1,     8,    -1,     3,
      95,   131,    -1,   154,   109,     3,    95,   131,    -1,    72,
     156,    -1,    87,    -1,    75,    -1,    53,    -1,    12,    75,
      -1,     3,    -1,    57,    -1,     8,    -1,     5,    -1,     6,
      -1,    70,     3,    95,   161,    -1,    70,     3,    95,   160,
      -1,    70,     3,    95,    57,    -1,    70,    56,   157,    -1,
      70,    10,    95,   157,    -1,    70,    38,     9,    95,   110,
     133,   111,    -1,    70,    38,     3,    95,   160,    -1,     3,
      -1,     8,    -1,    81,    -1,    33,    -1,   131,    -1,    23,
      -1,    66,    -1,   163,    -1,    17,    -1,    74,    80,    -1,
     165,    47,     3,   166,    85,   168,    -1,    45,    -1,    64,
      -1,    -1,   110,   167,   111,    -1,   130,    -1,   167,   109,
     130,    -1,   169,    -1,   168,   109,   169,    -1,   110,   170,
     111,    -1,   171,    -1,   170,   109,   171,    -1,   131,    -1,
     132,    -1,     8,    -1,   110,   133,   111,    -1,   110,   111,
      -1,    27,    36,   125,    89,    42,    95,   131,    -1,    27,
      36,   125,    89,    42,    43,   110,   133,   111,    -1,    20,
       3,   110,   174,   177,   111,    -1,   175,    -1,   174,   109,
     175,    -1,   171,    -1,   110,   176,   111,    -1,     8,    -1,
     176,   109,     8,    -1,    -1,   109,   178,    -1,   179,    -1,
     178,   109,   179,    -1,   171,   180,   181,    -1,    -1,    13,
      -1,     3,    -1,    50,    -1,   183,     3,    -1,    29,    -1,
      28,    -1,    72,    78,    -1,    84,   125,    70,   186,   127,
     144,    -1,   187,    -1,   186,   109,   187,    -1,     3,    95,
     131,    -1,     3,    95,   132,    -1,     3,    95,   110,   133,
     111,    -1,     3,    95,   110,   111,    -1,    72,   196,    86,
     189,    -1,    -1,   190,    -1,    89,   191,    -1,   192,    -1,
     191,    91,   192,    -1,     3,    95,     8,    -1,    72,    22,
      -1,    72,    21,    70,    -1,    70,   196,    80,    48,    49,
     197,    -1,    -1,    38,    -1,    71,    -1,    62,    83,    -1,
      62,    24,    -1,    63,    62,    -1,    69,    -1,    26,    37,
       3,    65,   199,    73,     8,    -1,    46,    -1,    34,    -1,
      76,    -1,    32,    37,     3,    -1,    15,    44,     3,    79,
      67,     3,    -1,    35,    67,     3,    -1,    68,   204,   142,
      -1,    10,    -1,    10,   114,     3,    -1,    82,    67,     3,
      -1,    60,    44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   126,   126,   127,   128,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   157,   158,   162,   163,   167,
     182,   183,   187,   188,   191,   193,   194,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   211,   212,   215,
     217,   221,   225,   226,   230,   235,   242,   250,   258,   267,
     272,   277,   282,   287,   292,   297,   302,   303,   304,   305,
     310,   315,   320,   328,   329,   334,   340,   346,   352,   361,
     362,   373,   374,   378,   384,   390,   392,   396,   403,   405,
     409,   415,   417,   421,   425,   432,   433,   437,   438,   439,
     442,   444,   448,   453,   460,   462,   466,   470,   471,   475,
     480,   485,   491,   496,   504,   509,   516,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   535,   536,   537,   538,
     539,   540,   541,   542,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   557,   558,   559,   560,   561,
     565,   566,   570,   571,   575,   576,   582,   586,   587,   588,
     589,   595,   596,   597,   598,   599,   603,   608,   613,   618,
     619,   623,   628,   636,   637,   641,   642,   643,   657,   658,
     659,   663,   664,   670,   678,   679,   682,   684,   688,   689,
     693,   694,   698,   702,   703,   707,   708,   709,   710,   711,
     717,   725,   739,   747,   751,   758,   759,   766,   776,   782,
     784,   788,   793,   797,   804,   806,   810,   811,   817,   825,
     826,   832,   838,   846,   847,   851,   855,   859,   863,   873,
     879,   880,   884,   888,   889,   893,   897,   904,   911,   917,
     918,   919,   923,   924,   925,   926,   932,   943,   944,   945,
     949,   960,   972,   983,   991,   992,  1001,  1012
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
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", 
  "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
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
     345,   346,   347,   124,    38,    61,   348,    60,    62,   349,
     350,    43,    45,    42,    47,    37,   351,   352,    59,    44,
      40,    41,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   115,   116,   116,   116,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   118,   118,   119,   119,   120,
     121,   121,   122,   122,   123,   123,   123,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   125,   125,   126,
     126,   127,   128,   128,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   130,   130,   130,   130,   130,   130,   131,
     131,   132,   132,   133,   133,   134,   134,   135,   136,   136,
     137,   138,   138,   139,   139,   140,   140,   141,   141,   141,
     142,   142,   143,   143,   144,   144,   145,   146,   146,   147,
     147,   147,   147,   147,   148,   148,   149,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   151,   151,   151,   151,   151,
     152,   152,   153,   153,   154,   154,   155,   156,   156,   156,
     156,   157,   157,   157,   157,   157,   158,   158,   158,   158,
     158,   159,   159,   160,   160,   161,   161,   161,   162,   162,
     162,   163,   163,   164,   165,   165,   166,   166,   167,   167,
     168,   168,   169,   170,   170,   171,   171,   171,   171,   171,
     172,   172,   173,   174,   174,   175,   175,   176,   176,   177,
     177,   178,   178,   179,   180,   180,   181,   181,   182,   183,
     183,   184,   185,   186,   186,   187,   187,   187,   187,   188,
     189,   189,   190,   191,   191,   192,   193,   194,   195,   196,
     196,   196,   197,   197,   197,   197,   198,   199,   199,   199,
     200,   201,   202,   203,   204,   204,   205,   206
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
       1,     1,     2,     2,     2,     1,     7,     1,     1,     1,
       3,     6,     3,     3,     1,     3,     3,     3
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
     121,   122,   254,     0,     0,     0,     0,   119,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,    30,
      34,    37,   144,   100,     0,     0,   240,     0,   241,     0,
       0,     0,   236,   240,   159,   158,   221,   157,   156,     0,
     182,     0,    47,     0,     1,     4,     0,   218,     0,     0,
       0,     0,   250,   252,   257,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   123,   124,
       0,     0,     0,     0,     0,    35,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   253,   101,     0,     0,
       0,     0,   161,   164,   165,   163,   162,   169,     0,   160,
     237,   230,   256,     0,     0,     0,     0,    26,   186,     0,
      79,    81,   197,     0,     0,   195,   196,   205,   209,   203,
       0,     0,   153,   147,   152,     0,   150,   255,     0,     0,
       0,    45,     0,     0,     0,     0,     0,    44,     0,     0,
     142,     0,     0,   143,    49,    31,    36,   134,   135,   141,
     140,   132,   131,   138,   139,   129,   130,   137,   136,   125,
     126,   127,   128,   133,   102,   173,   174,   176,   168,   175,
       0,   177,   167,   166,   170,     0,     0,     0,     0,   229,
     231,     0,     0,   223,    48,     0,     0,     0,    80,    82,
     207,   199,    83,     0,     0,     0,     0,   248,   247,   249,
       0,     0,     0,   145,    38,     0,    43,    42,   146,     0,
      39,     0,    40,    41,     0,     0,   154,     0,     0,    85,
      50,     0,   172,     0,     0,     0,   232,   233,     0,     0,
     104,    73,    74,     0,     0,    78,     0,   188,     0,     0,
     251,     0,   198,     0,   206,   205,   204,   210,   211,   202,
       0,     0,     0,   151,    46,     0,     0,     0,     0,    51,
      52,     0,     0,    88,    86,   103,     0,     0,     0,   245,
     238,     0,     0,     0,   225,   226,   224,     0,   222,   105,
       0,     0,     0,     0,   187,     0,   183,   190,    84,   208,
     215,     0,     0,   246,     0,   200,   149,   148,   155,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    89,   171,   243,   242,   244,   235,   234,
     228,     0,     0,   106,   107,     0,    76,    77,   189,     0,
       0,   193,     0,   216,   217,   213,   214,   212,     0,     0,
      53,     0,     0,    59,     0,    55,    66,    56,    67,    63,
      69,    62,    68,    64,    71,    65,    72,     0,    87,     0,
       0,   100,    92,   227,     0,     0,    75,     0,   192,   191,
     201,    54,     0,     0,     0,    60,     0,     0,     0,   104,
     109,   110,   113,     0,   108,   194,    61,     0,    70,    57,
       0,     0,     0,    97,    93,    95,    29,     0,     0,     0,
     114,    58,    90,     0,    98,    99,     0,     0,     0,     0,
     111,    94,    96,   112,   116,   115
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,    78,    79,   137,    80,
     103,   289,   290,   329,   330,   463,   262,   186,   263,   333,
     334,   382,   383,   431,   432,   464,   465,   156,   157,   348,
     349,   393,   394,   469,   470,    81,    82,   195,   196,   132,
      26,    98,   167,    27,    28,   242,   243,    29,    30,    31,
      32,   256,   308,   356,   357,   400,   187,    33,    34,   188,
     189,   264,   266,   317,   318,   361,   405,    35,    36,    37,
      38,   252,   253,    39,   249,   250,   296,   297,    40,    41,
      42,    89,   340,    43,   270,    44,    45,    46,    47,    83,
      48,    49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -282
static const short yypact[] =
{
     794,   -24,  -282,    47,  -282,    30,    32,  -282,  -282,    62,
      67,  -282,    44,  -282,  -282,   105,   160,   639,    96,    76,
     168,   187,  -282,    83,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,   145,  -282,  -282,  -282,   201,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
     206,   115,   211,   168,   226,   229,   236,   130,  -282,  -282,
    -282,  -282,   133,   161,   170,   181,   182,  -282,   184,   185,
     186,   189,   197,   273,  -282,   273,   273,   257,    -2,  -282,
      29,   571,  -282,   220,   215,   217,   123,    55,  -282,   233,
     239,   247,  -282,  -282,  -282,  -282,  -282,  -282,  -282,   237,
    -282,   319,  -282,   -45,  -282,   -25,   321,  -282,   250,    21,
     265,   -76,  -282,  -282,  -282,   191,   328,   273,    15,   221,
     273,   246,   273,   273,   273,   228,   227,   240,  -282,  -282,
     367,   251,    56,   168,   232,  -282,   346,  -282,   273,   273,
     273,   273,   273,   273,   273,   273,   273,   273,   273,   273,
     273,   273,   273,   273,   273,   348,  -282,  -282,   303,    55,
     256,   259,  -282,  -282,  -282,  -282,  -282,  -282,   311,  -282,
    -282,   272,  -282,   359,   360,   232,    91,  -282,   254,   298,
    -282,  -282,  -282,    25,    11,  -282,  -282,  -282,   258,  -282,
     104,   330,  -282,  -282,   571,   -22,  -282,  -282,   392,   370,
     263,  -282,   418,    63,   316,   337,   443,  -282,   273,   273,
    -282,     3,   373,  -282,   -71,  -282,  -282,  -282,  -282,   601,
     652,   677,   688,   703,   703,   349,   349,   349,   349,   290,
     290,  -282,  -282,  -282,   261,  -282,  -282,  -282,  -282,  -282,
     372,  -282,  -282,  -282,  -282,   152,   268,   332,   379,  -282,
    -282,   291,   -65,  -282,  -282,   301,   302,   385,  -282,  -282,
    -282,  -282,  -282,    66,    70,    21,   279,  -282,  -282,  -282,
     318,   -33,   246,  -282,  -282,   285,  -282,  -282,  -282,   273,
    -282,   273,  -282,  -282,   520,   545,  -282,   304,   315,   358,
    -282,   395,  -282,     3,    89,   306,   333,  -282,    51,   359,
     344,  -282,  -282,   296,   334,  -282,   335,  -282,    74,   345,
    -282,     3,  -282,   435,  -282,   151,  -282,   317,  -282,  -282,
     448,   347,     3,  -282,  -282,   469,   494,     3,   365,   384,
    -282,   124,   458,   389,  -282,  -282,    75,    42,   419,  -282,
    -282,   472,   379,    17,  -282,  -282,  -282,   479,  -282,  -282,
     396,   390,   391,   301,  -282,    31,   397,  -282,  -282,  -282,
    -282,     9,    31,  -282,     3,  -282,  -282,  -282,  -282,   496,
     315,    35,    -6,    35,    35,    35,    35,    35,    35,   462,
     301,   468,   449,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,    79,   431,   421,  -282,   416,  -282,  -282,  -282,    18,
     117,  -282,   345,  -282,  -282,  -282,   515,  -282,   147,   420,
    -282,   440,   441,  -282,     3,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,  -282,    -4,  -282,   491,
     533,   220,  -282,  -282,     6,   479,  -282,    31,  -282,  -282,
    -282,  -282,     3,    22,   150,  -282,     3,   534,   202,   344,
     445,  -282,  -282,   553,  -282,  -282,  -282,   551,  -282,  -282,
     172,   301,   467,   121,   450,  -282,  -282,   550,   483,   176,
    -282,  -282,   450,   470,  -282,  -282,   301,   471,     3,   553,
    -282,  -282,  -282,  -282,  -282,  -282
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -282,  -282,  -282,  -282,   474,  -282,  -282,   473,  -282,  -282,
     -32,  -282,   331,  -282,   214,  -253,  -109,  -281,  -278,  -282,
    -282,  -282,  -282,  -282,  -282,   140,   127,   173,  -282,   157,
    -282,  -282,   174,  -282,   129,   -69,  -282,   489,   355,  -282,
    -282,  -282,   475,  -282,  -282,   383,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,  -282,   231,  -282,  -264,  -282,  -282,  -282,
     366,  -282,  -282,  -282,   293,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,   336,  -282,  -282,  -282,  -282,   288,  -282,  -282,
    -282,   635,  -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,
    -282,  -282
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -215
static const short yytable[] =
{
     185,   315,   307,   413,   128,   445,   129,   130,   180,   450,
     321,   451,   403,   191,   452,   336,   180,   345,   288,   260,
      50,   111,   180,   180,   288,   173,   180,   181,   181,   182,
     258,   259,   135,   174,   133,   331,   180,   181,   174,   182,
     180,   181,   136,   175,   299,   199,   194,   176,   198,   241,
      51,   202,   194,   204,   205,   206,   180,   181,   162,   404,
     163,   164,   322,   165,   174,   391,   385,    52,    53,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   408,   272,    56,   273,
     412,   401,   416,   418,   420,   422,   424,   426,   406,    54,
     398,   214,   286,    90,   414,   240,   446,   134,    57,    58,
      59,    60,   166,   240,    61,    62,   453,   331,   200,   240,
     240,    63,   261,   183,   457,   386,   160,   428,   390,   261,
      64,   184,   161,   183,    55,   474,   444,   183,   267,   284,
     285,   399,   371,   101,    94,    65,    66,    67,    68,   475,
     268,   337,   338,   183,  -214,   235,   185,    69,   339,    70,
     236,   343,   458,    84,   360,   212,    95,   372,   460,   213,
      85,   102,   272,   455,   278,   311,   100,   312,    97,   313,
     269,   314,    71,   353,   311,   354,   384,   104,   311,   344,
     433,   105,   106,    72,    57,    58,    59,    60,    86,   192,
      61,  -214,   358,   194,   107,   301,   302,    73,    74,   108,
     325,    75,   326,   365,   110,    76,    87,    77,   368,   373,
     374,   375,   376,   377,   378,   109,   437,   303,   438,   112,
     379,    88,   113,    67,    68,    57,    58,    59,    60,   114,
     115,    61,   304,   126,   305,   127,   185,   116,    63,    57,
      58,    59,    60,   185,   192,    61,   311,    64,   440,   311,
     131,   459,   411,   462,   415,   417,   419,   421,   423,   425,
     155,   117,    65,    66,    67,    68,    57,    58,    59,    60,
     118,   311,    61,   471,    69,   479,    70,   480,    67,    68,
     306,   119,   120,    73,   121,   122,   123,    75,   126,   124,
     127,    76,   193,    77,   301,   302,   235,   125,   180,    71,
     158,   236,   159,   168,   169,    67,    68,   170,   301,   302,
      72,   138,   172,   171,   178,   126,   303,   127,   185,   179,
     190,   197,   201,   456,    73,    74,   237,   208,    75,   207,
     303,   304,    76,   305,    77,   139,   211,   138,    73,   216,
     209,   245,    75,   234,   246,   304,    76,   305,    77,   247,
     238,   248,   251,   254,   255,   257,   328,   265,   138,   484,
     291,   139,   271,   275,   276,    73,   287,   258,   293,    75,
     138,   294,   295,    76,   239,    77,   298,   309,   310,   306,
     319,   320,   139,   152,   153,   154,   324,   332,   138,   327,
     335,   341,   347,   306,   139,   240,   350,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   139,   138,   342,   279,   362,   280,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   359,   351,   352,   281,   139,   282,   138,
     150,   151,   152,   153,   154,   355,   363,   364,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   139,   138,   369,   370,   380,   210,   381,
     388,   387,   392,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   139,   395,
     138,   396,   397,   274,   409,   427,   402,   429,   430,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   139,   138,   434,   436,   360,   277,
     435,   441,   442,   443,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   139,
     447,   138,   448,   461,   283,   467,   468,   259,   477,   476,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   139,   138,   473,   478,   177,
     366,   481,   483,   300,   410,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     139,   472,   138,   482,   449,   367,   466,   215,   485,   454,
     203,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   139,   323,   292,   279,
     389,   316,   138,   439,   244,   346,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    90,    99,     0,   281,   407,   139,     0,     0,     0,
      91,    92,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,    93,     0,     0,
       0,     0,     0,   138,     0,     0,     0,     0,     0,     0,
       0,     0,    94,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   139,   138,     0,
      88,     0,     0,     0,    95,     0,     0,    96,     0,   138,
       0,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,     0,   139,     0,   138,     0,     0,     0,     0,     0,
       0,     0,     0,   139,     0,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   139,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,     0,     0,     0,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     1,
       0,     2,     0,     0,     3,     0,     0,     4,     0,     0,
       5,     6,     7,     8,     0,     0,     9,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,     0,    13,     0,
      14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
       0,     0,     0,     0,     0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     109,   265,   255,     9,    73,     9,    75,    76,     5,     3,
      43,     5,     3,    89,     8,   293,     5,   298,    89,     8,
      44,    53,     5,     5,    89,    70,     5,     6,     6,     8,
       5,     6,     3,   109,    36,   288,     5,     6,   109,     8,
       5,     6,    13,    68,   109,    30,   115,    72,   117,   158,
       3,   120,   121,   122,   123,   124,     5,     6,     3,    50,
       5,     6,    95,     8,   109,   343,    24,    37,    36,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   364,   109,    44,   111,
     371,   355,   373,   374,   375,   376,   377,   378,   362,    37,
     353,   133,   211,    12,   110,   102,   110,   109,     3,     4,
       5,     6,    57,   102,     9,    10,   110,   370,   103,   102,
     102,    16,   111,   102,   102,    83,     3,   380,   111,   111,
      25,   110,     9,   102,    67,    14,   414,   102,    34,   208,
     209,   110,    18,    67,    53,    40,    41,    42,    43,    28,
      46,    62,    63,   102,     3,     3,   265,    52,    69,    54,
       8,   110,   443,     3,    13,   109,    75,    43,   446,   113,
      10,     3,   109,   437,   111,   109,    80,   111,    87,   109,
      76,   111,    77,   109,   109,   111,   111,     0,   109,   298,
     111,   108,    47,    88,     3,     4,     5,     6,    38,     8,
       9,    50,   311,   272,     3,     3,     4,   102,   103,     3,
     279,   106,   281,   322,     3,   110,    56,   112,   327,    95,
      96,    97,    98,    99,   100,   110,   109,    25,   111,     3,
     106,    71,     3,    42,    43,     3,     4,     5,     6,     3,
     110,     9,    40,    52,    42,    54,   355,   114,    16,     3,
       4,     5,     6,   362,     8,     9,   109,    25,   111,   109,
       3,   111,   371,    61,   373,   374,   375,   376,   377,   378,
      50,   110,    40,    41,    42,    43,     3,     4,     5,     6,
     110,   109,     9,   111,    52,   109,    54,   111,    42,    43,
      88,   110,   110,   102,   110,   110,   110,   106,    52,   110,
      54,   110,   111,   112,     3,     4,     3,   110,     5,    77,
      95,     8,    95,    80,    75,    42,    43,    70,     3,     4,
      88,    31,     3,    86,     3,    52,    25,    54,   437,    79,
      65,     3,   111,   442,   102,   103,    33,   110,   106,   111,
      25,    40,   110,    42,   112,    55,    95,    31,   102,     3,
     110,    95,   106,     5,    95,    40,   110,    42,   112,    48,
      57,    89,     3,     3,   110,    67,    51,   109,    31,   478,
     109,    55,    42,     3,   111,   102,     3,     5,   110,   106,
      31,    49,     3,   110,    81,   112,    95,    85,     3,    88,
     111,    73,    55,   103,   104,   105,   111,    39,    31,    95,
       5,    95,    58,    88,    55,   102,   110,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,    55,    31,    91,   109,   109,   111,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,     8,   110,   110,   109,    55,   111,    31,
     101,   102,   103,   104,   105,   110,     8,   110,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,    55,    31,   110,    92,    19,   111,    90,
       8,    62,     3,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,    55,   103,
      31,   111,   111,   111,     8,    43,   109,    39,    59,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,    55,    31,    95,   111,    13,   111,
     109,   111,    92,    92,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,    55,
      59,    31,    19,    19,   111,   110,     3,     6,     8,   109,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,    55,    31,   110,    95,   105,
     111,   111,   111,   252,   370,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
      55,   461,    31,   476,   431,   111,   449,   134,   479,   435,
     121,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    55,   272,   245,   109,
     342,   265,    31,   402,   159,   299,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,    12,    17,    -1,   109,   362,    55,    -1,    -1,    -1,
      21,    22,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,    38,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,    55,    31,    -1,
      71,    -1,    -1,    -1,    75,    -1,    -1,    78,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,    15,
      -1,    17,    -1,    -1,    20,    -1,    -1,    23,    -1,    -1,
      26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    64,    -1,
      66,    -1,    68,    -1,    70,    -1,    72,    -1,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    -1,    84
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    60,    64,    66,    68,    70,    72,    74,    82,
      84,   116,   117,   118,   119,   120,   155,   158,   159,   162,
     163,   164,   165,   172,   173,   182,   183,   184,   185,   188,
     193,   194,   195,   198,   200,   201,   202,   203,   205,   206,
      44,     3,    37,    36,    37,    67,    44,     3,     4,     5,
       6,     9,    10,    16,    25,    40,    41,    42,    43,    52,
      54,    77,    88,   102,   103,   106,   110,   112,   121,   122,
     124,   150,   151,   204,     3,    10,    38,    56,    71,   196,
      12,    21,    22,    38,    53,    75,    78,    87,   156,   196,
      80,    67,     3,   125,     0,   108,    47,     3,     3,   110,
       3,   125,     3,     3,     3,   110,   114,   110,   110,   110,
     110,   110,   110,   110,   110,   110,    52,    54,   150,   150,
     150,     3,   154,    36,   109,     3,    13,   123,    31,    55,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,    50,   142,   143,    95,    95,
       3,     9,     3,     5,     6,     8,    57,   157,    80,    75,
      70,    86,     3,    70,   109,    68,    72,   119,     3,    79,
       5,     6,     8,   102,   110,   131,   132,   171,   174,   175,
      65,    89,     8,   111,   150,   152,   153,     3,   150,    30,
     103,   111,   150,   152,   150,   150,   150,   111,   110,   110,
     111,    95,   109,   113,   125,   122,     3,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,     5,     3,     8,    33,    57,    81,
     102,   131,   160,   161,   157,    95,    95,    48,    89,   189,
     190,     3,   186,   187,     3,   110,   166,    67,     5,     6,
       8,   111,   131,   133,   176,   109,   177,    34,    46,    76,
     199,    42,   109,   111,   111,     3,   111,   111,   111,   109,
     111,   109,   111,   111,   150,   150,   131,     3,    89,   126,
     127,   109,   160,   110,    49,     3,   191,   192,    95,   109,
     127,     3,     4,    25,    40,    42,    88,   130,   167,    85,
       3,   109,   111,   109,   111,   171,   175,   178,   179,   111,
      73,    43,    95,   153,   111,   150,   150,    95,    51,   128,
     129,   130,    39,   134,   135,     5,   133,    62,    63,    69,
     197,    95,    91,   110,   131,   132,   187,    58,   144,   145,
     110,   110,   110,   109,   111,   110,   168,   169,   131,     8,
      13,   180,   109,     8,   110,   131,   111,   111,   131,   110,
      92,    18,    43,    95,    96,    97,    98,    99,   100,   106,
      19,    90,   136,   137,   111,    24,    83,    62,     8,   192,
     111,   133,     3,   146,   147,   103,   111,   111,   130,   110,
     170,   171,   109,     3,    50,   181,   171,   179,   133,     8,
     129,   131,   132,     9,   110,   131,   132,   131,   132,   131,
     132,   131,   132,   131,   132,   131,   132,    43,   130,    39,
      59,   138,   139,   111,    95,   109,   111,   109,   111,   169,
     111,   111,    92,    92,   133,     9,   110,    59,    19,   142,
       3,     5,     8,   110,   147,   171,   131,   102,   132,   111,
     133,    19,    61,   130,   140,   141,   144,   110,     3,   148,
     149,   111,   140,   110,    14,    28,   109,     8,    95,   109,
     111,   111,   141,   111,   131,   149
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

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 250:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 251:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 252:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 253:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 255:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 256:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 257:

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

