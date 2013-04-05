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
#define YYFINAL  114
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1182

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  124
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  105
/* YYNRULES -- Number of rules. */
#define YYNRULES  294
/* YYNRULES -- Number of states. */
#define YYNSTATES  571

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
      62,    64,    73,    75,    85,    86,    89,    91,    95,    97,
      99,   101,   102,   106,   107,   110,   115,   126,   128,   132,
     134,   137,   138,   140,   143,   145,   150,   155,   160,   165,
     170,   175,   179,   185,   187,   191,   192,   194,   197,   199,
     203,   207,   212,   216,   220,   226,   233,   237,   242,   248,
     252,   256,   260,   264,   268,   270,   276,   280,   284,   288,
     292,   296,   300,   304,   306,   308,   313,   317,   321,   323,
     325,   328,   330,   333,   335,   339,   340,   344,   346,   350,
     351,   353,   359,   360,   362,   366,   372,   374,   378,   380,
     383,   386,   387,   389,   392,   397,   398,   400,   403,   405,
     409,   413,   417,   423,   430,   434,   436,   440,   444,   446,
     448,   450,   452,   454,   456,   459,   462,   466,   470,   474,
     478,   482,   486,   490,   494,   498,   502,   506,   510,   514,
     518,   522,   526,   530,   534,   538,   540,   545,   550,   555,
     560,   565,   569,   576,   583,   587,   589,   593,   595,   597,
     601,   607,   610,   611,   614,   616,   619,   622,   626,   628,
     633,   638,   642,   644,   646,   648,   650,   652,   654,   658,
     663,   668,   673,   677,   682,   690,   696,   698,   700,   702,
     704,   706,   708,   710,   712,   714,   717,   724,   726,   728,
     729,   733,   735,   739,   741,   745,   749,   751,   755,   757,
     759,   761,   765,   768,   776,   786,   793,   795,   799,   801,
     805,   807,   811,   812,   815,   817,   821,   825,   826,   828,
     830,   832,   836,   838,   840,   844,   851,   853,   857,   861,
     865,   871,   876,   878,   880,   882,   890,   895,   896,   898,
     901,   903,   907,   911,   914,   918,   925,   926,   928,   930,
     933,   936,   939,   941,   949,   951,   953,   955,   959,   966,
     970,   974,   976,   980,   984
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     125,     0,    -1,   126,    -1,   127,    -1,   127,   117,    -1,
     184,    -1,   192,    -1,   178,    -1,   179,    -1,   182,    -1,
     193,    -1,   202,    -1,   204,    -1,   205,    -1,   210,    -1,
     215,    -1,   216,    -1,   220,    -1,   222,    -1,   223,    -1,
     224,    -1,   217,    -1,   225,    -1,   227,    -1,   228,    -1,
     209,    -1,   128,    -1,   127,   117,   128,    -1,   129,    -1,
     173,    -1,   130,    -1,    76,     3,   118,   118,   130,   119,
     131,   119,    -1,   137,    -1,    76,   138,    41,   118,   134,
     137,   119,   135,   136,    -1,    -1,   120,   132,    -1,   133,
      -1,   132,   120,   133,    -1,     3,    -1,     5,    -1,    47,
      -1,    -1,    66,    23,   158,    -1,    -1,    57,     5,    -1,
      57,     5,   120,     5,    -1,    76,   138,    41,   142,   143,
     152,   154,   156,   160,   162,    -1,   139,    -1,   138,   120,
     139,    -1,   112,    -1,   141,   140,    -1,    -1,     3,    -1,
      16,     3,    -1,   168,    -1,    19,   118,   168,   119,    -1,
      59,   118,   168,   119,    -1,    61,   118,   168,   119,    -1,
      85,   118,   168,   119,    -1,    46,   118,   168,   119,    -1,
      30,   118,   112,   119,    -1,    45,   118,   119,    -1,    30,
     118,    35,     3,   119,    -1,     3,    -1,   142,   120,     3,
      -1,    -1,   144,    -1,    98,   145,    -1,   146,    -1,   145,
     101,   145,    -1,   118,   145,   119,    -1,    58,   118,     8,
     119,    -1,   148,   104,   149,    -1,   148,   105,   149,    -1,
     148,    48,   118,   151,   119,    -1,   148,   115,    48,   118,
     151,   119,    -1,   148,    48,     9,    -1,   148,   115,    48,
       9,    -1,   148,    21,   149,   101,   149,    -1,   148,   107,
     149,    -1,   148,   106,   149,    -1,   148,   108,   149,    -1,
     148,   109,   149,    -1,   148,   104,   150,    -1,   147,    -1,
     148,    21,   150,   101,   150,    -1,   148,   107,   150,    -1,
     148,   106,   150,    -1,   148,   108,   150,    -1,   148,   109,
     150,    -1,   148,   104,     8,    -1,   148,   105,     8,    -1,
     148,   105,   150,    -1,     3,    -1,     4,    -1,    30,   118,
     112,   119,    -1,    45,   118,   119,    -1,    97,   118,   119,
      -1,    47,    -1,     5,    -1,   111,     5,    -1,     6,    -1,
     111,     6,    -1,   149,    -1,   151,   120,   149,    -1,    -1,
      44,    23,   153,    -1,   148,    -1,   153,   120,   148,    -1,
      -1,   155,    -1,    99,    44,    66,    23,   158,    -1,    -1,
     157,    -1,    66,    23,   158,    -1,    66,    23,    69,   118,
     119,    -1,   159,    -1,   158,   120,   159,    -1,   148,    -1,
     148,    17,    -1,   148,    33,    -1,    -1,   161,    -1,    57,
       5,    -1,    57,     5,   120,     5,    -1,    -1,   163,    -1,
      65,   164,    -1,   165,    -1,   164,   120,   165,    -1,     3,
     104,     3,    -1,     3,   104,     5,    -1,     3,   104,   118,
     166,   119,    -1,     3,   104,     3,   118,     8,   119,    -1,
       3,   104,     8,    -1,   167,    -1,   166,   120,   167,    -1,
       3,   104,   149,    -1,     3,    -1,     4,    -1,    47,    -1,
       5,    -1,     6,    -1,     9,    -1,   111,   168,    -1,   115,
     168,    -1,   168,   110,   168,    -1,   168,   111,   168,    -1,
     168,   112,   168,    -1,   168,   113,   168,    -1,   168,   106,
     168,    -1,   168,   107,   168,    -1,   168,   103,   168,    -1,
     168,   102,   168,    -1,   168,   114,   168,    -1,   168,    36,
     168,    -1,   168,    62,   168,    -1,   168,   109,   168,    -1,
     168,   108,   168,    -1,   168,   104,   168,    -1,   168,   105,
     168,    -1,   168,   101,   168,    -1,   168,   100,   168,    -1,
     118,   168,   119,    -1,   121,   172,   122,    -1,   169,    -1,
       3,   118,   170,   119,    -1,    48,   118,   170,   119,    -1,
      52,   118,   170,   119,    -1,    22,   118,   170,   119,    -1,
      39,   118,   170,   119,    -1,     3,   118,   119,    -1,    61,
     118,   168,   120,   168,   119,    -1,    59,   118,   168,   120,
     168,   119,    -1,    97,   118,   119,    -1,   171,    -1,   170,
     120,   171,    -1,   168,    -1,     8,    -1,     3,   104,   149,
      -1,   172,   120,     3,   104,   149,    -1,    80,   175,    -1,
      -1,    56,     8,    -1,    96,    -1,    83,   174,    -1,    60,
     174,    -1,    14,    83,   174,    -1,    68,    -1,    14,     8,
      83,   174,    -1,    14,     3,    83,   174,    -1,    49,     3,
      83,    -1,     3,    -1,    64,    -1,     8,    -1,     5,    -1,
       6,    -1,   176,    -1,   177,   111,   176,    -1,    78,     3,
     104,   181,    -1,    78,     3,   104,   180,    -1,    78,     3,
     104,    64,    -1,    78,    63,   177,    -1,    78,    10,   104,
     177,    -1,    78,    43,     9,   104,   118,   151,   119,    -1,
      78,    43,     3,   104,   180,    -1,     3,    -1,     8,    -1,
      90,    -1,    38,    -1,   149,    -1,    28,    -1,    74,    -1,
     183,    -1,    20,    -1,    82,    89,    -1,   185,    53,     3,
     186,    94,   188,    -1,    50,    -1,    72,    -1,    -1,   118,
     187,   119,    -1,   148,    -1,   187,   120,   148,    -1,   189,
      -1,   188,   120,   189,    -1,   118,   190,   119,    -1,   191,
      -1,   190,   120,   191,    -1,   149,    -1,   150,    -1,     8,
      -1,   118,   151,   119,    -1,   118,   119,    -1,    32,    41,
     142,    98,    47,   104,   149,    -1,    32,    41,   142,    98,
      47,    48,   118,   151,   119,    -1,    24,     3,   118,   194,
     197,   119,    -1,   195,    -1,   194,   120,   195,    -1,   191,
      -1,   118,   196,   119,    -1,     8,    -1,   196,   120,     8,
      -1,    -1,   120,   198,    -1,   199,    -1,   198,   120,   199,
      -1,   191,   200,   201,    -1,    -1,    16,    -1,     3,    -1,
      57,    -1,   203,     3,   174,    -1,    34,    -1,    33,    -1,
      80,    87,   174,    -1,    93,   142,    78,   206,   144,   162,
      -1,   207,    -1,   206,   120,   207,    -1,     3,   104,   149,
      -1,     3,   104,   150,    -1,     3,   104,   118,   151,   119,
      -1,     3,   104,   118,   119,    -1,    52,    -1,    22,    -1,
      39,    -1,    15,    86,     3,    13,    27,     3,   208,    -1,
      80,   218,    95,   211,    -1,    -1,   212,    -1,    98,   213,
      -1,   214,    -1,   213,   100,   214,    -1,     3,   104,     8,
      -1,    80,    26,    -1,    80,    25,    78,    -1,    78,   218,
      89,    54,    55,   219,    -1,    -1,    43,    -1,    79,    -1,
      70,    92,    -1,    70,    29,    -1,    71,    70,    -1,    77,
      -1,    31,    42,     3,    73,   221,    81,     8,    -1,    51,
      -1,    39,    -1,    84,    -1,    37,    42,     3,    -1,    18,
      49,     3,    88,    75,     3,    -1,    40,    75,     3,    -1,
      76,   226,   160,    -1,    10,    -1,    10,   123,     3,    -1,
      91,    75,     3,    -1,    67,    49,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   135,   135,   136,   137,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   167,   168,   172,   173,
     177,   178,   186,   187,   194,   196,   200,   204,   211,   212,
     213,   217,   230,   238,   240,   245,   254,   270,   271,   275,
     276,   279,   281,   282,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   298,   299,   302,   304,   308,   312,   313,
     314,   318,   323,   330,   338,   346,   355,   360,   365,   370,
     375,   380,   385,   390,   395,   400,   405,   410,   415,   420,
     425,   430,   438,   442,   443,   448,   454,   460,   466,   475,
     476,   487,   488,   492,   498,   504,   506,   510,   514,   520,
     522,   526,   537,   539,   543,   547,   554,   555,   559,   560,
     561,   564,   566,   570,   575,   582,   584,   588,   592,   593,
     597,   602,   607,   613,   618,   626,   631,   638,   648,   649,
     650,   651,   652,   653,   654,   655,   656,   657,   658,   659,
     660,   661,   662,   663,   664,   665,   666,   667,   668,   669,
     670,   671,   672,   673,   674,   675,   679,   680,   681,   682,
     683,   684,   685,   686,   687,   691,   692,   696,   697,   701,
     702,   708,   711,   713,   717,   718,   719,   720,   721,   722,
     727,   732,   742,   743,   744,   745,   746,   750,   751,   755,
     760,   765,   770,   771,   775,   780,   788,   789,   793,   794,
     795,   809,   810,   811,   815,   816,   822,   830,   831,   834,
     836,   840,   841,   845,   846,   850,   854,   855,   859,   860,
     861,   862,   863,   869,   877,   891,   899,   903,   910,   911,
     918,   928,   934,   936,   940,   945,   949,   956,   958,   962,
     963,   969,   977,   978,   984,   990,   998,   999,  1003,  1007,
    1011,  1015,  1025,  1026,  1027,  1031,  1044,  1050,  1051,  1055,
    1059,  1060,  1064,  1068,  1075,  1082,  1088,  1089,  1090,  1094,
    1095,  1096,  1097,  1103,  1114,  1115,  1116,  1120,  1131,  1143,
    1154,  1162,  1163,  1172,  1183
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
  "select", "select1", "opt_tablefunc_args", "tablefunc_args_list", 
  "tablefunc_arg", "subselect_start", "opt_outer_order", 
  "opt_outer_limit", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "expr_float_unhandled", 
  "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_items_list", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "show_stmt", "like_filter", "show_what", 
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
     129,   129,   130,   130,   131,   131,   132,   132,   133,   133,
     133,   134,   135,   136,   136,   136,   137,   138,   138,   139,
     139,   140,   140,   140,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   142,   142,   143,   143,   144,   145,   145,
     145,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   147,   148,   148,   148,   148,   148,   148,   149,
     149,   150,   150,   151,   151,   152,   152,   153,   153,   154,
     154,   155,   156,   156,   157,   157,   158,   158,   159,   159,
     159,   160,   160,   161,   161,   162,   162,   163,   164,   164,
     165,   165,   165,   165,   165,   166,   166,   167,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   170,   170,   171,   171,   172,
     172,   173,   174,   174,   175,   175,   175,   175,   175,   175,
     175,   175,   176,   176,   176,   176,   176,   177,   177,   178,
     178,   178,   178,   178,   179,   179,   180,   180,   181,   181,
     181,   182,   182,   182,   183,   183,   184,   185,   185,   186,
     186,   187,   187,   188,   188,   189,   190,   190,   191,   191,
     191,   191,   191,   192,   192,   193,   194,   194,   195,   195,
     196,   196,   197,   197,   198,   198,   199,   200,   200,   201,
     201,   202,   203,   203,   204,   205,   206,   206,   207,   207,
     207,   207,   208,   208,   208,   209,   210,   211,   211,   212,
     213,   213,   214,   215,   216,   217,   218,   218,   218,   219,
     219,   219,   219,   220,   221,   221,   221,   222,   223,   224,
     225,   226,   226,   227,   228
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     8,     1,     9,     0,     2,     1,     3,     1,     1,
       1,     0,     3,     0,     2,     4,    10,     1,     3,     1,
       2,     0,     1,     2,     1,     4,     4,     4,     4,     4,
       4,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       3,     4,     3,     3,     5,     6,     3,     4,     5,     3,
       3,     3,     3,     3,     1,     5,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     4,     3,     3,     1,     1,
       2,     1,     2,     1,     3,     0,     3,     1,     3,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     6,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     4,     4,     4,     4,
       4,     3,     6,     6,     3,     1,     3,     1,     1,     3,
       5,     2,     0,     2,     1,     2,     2,     3,     1,     4,
       4,     3,     1,     1,     1,     1,     1,     1,     3,     4,
       4,     4,     3,     4,     7,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     6,     1,     1,     0,
       3,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     3,     2,     7,     9,     6,     1,     3,     1,     3,
       1,     3,     0,     2,     1,     3,     3,     0,     1,     1,
       1,     3,     1,     1,     3,     6,     1,     3,     3,     3,
       5,     4,     1,     1,     1,     7,     4,     0,     1,     2,
       1,     3,     3,     2,     3,     6,     0,     1,     1,     2,
       2,     2,     1,     7,     1,     1,     1,     3,     6,     3,
       3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   214,     0,   211,     0,     0,   253,   252,
       0,     0,   217,     0,   218,   212,     0,   276,   276,     0,
       0,     0,     0,     2,     3,    26,    28,    30,    32,    29,
       7,     8,     9,   213,     5,     0,     6,    10,    11,     0,
      12,    13,    25,    14,    15,    16,    21,    17,    18,    19,
      20,    22,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,   138,   139,   141,   142,   143,   291,     0,     0,
       0,     0,     0,     0,   140,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,    47,    51,    54,
     165,   121,     0,     0,   277,     0,   278,     0,     0,     0,
     273,   277,     0,   182,   188,   182,   182,   184,   181,     0,
     215,     0,    63,     0,     1,     4,     0,   182,     0,     0,
       0,     0,     0,   287,   289,   294,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     138,     0,     0,   144,   145,     0,     0,     0,     0,     0,
      52,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   290,   122,     0,     0,     0,     0,   192,   195,   196,
     194,   193,   197,   202,     0,     0,     0,   182,   274,     0,
       0,   186,   185,   254,   267,   293,     0,     0,     0,     0,
      27,   219,   251,     0,     0,    99,   101,   230,     0,     0,
     228,   229,   238,   242,   236,     0,     0,   178,     0,   171,
     177,     0,   175,   292,     0,     0,     0,     0,     0,    61,
       0,     0,     0,     0,     0,     0,   174,     0,     0,     0,
     163,     0,     0,   164,    41,    65,    48,    53,   155,   156,
     162,   161,   153,   152,   159,   160,   150,   151,   158,   157,
     146,   147,   148,   149,   154,   123,   206,   207,   209,   201,
     208,     0,   210,   200,   199,   203,     0,     0,     0,     0,
     182,   182,   187,   191,   183,     0,   266,   268,     0,     0,
     256,    64,     0,     0,     0,     0,   100,   102,   240,   232,
     103,     0,     0,     0,     0,   285,   284,   286,     0,     0,
       0,     0,   166,     0,    55,   169,     0,    60,   170,    59,
     167,   168,    56,     0,    57,     0,    58,     0,     0,   179,
       0,     0,     0,   105,    66,     0,   205,     0,   198,     0,
     190,   189,     0,   269,   270,     0,     0,   125,    93,    94,
       0,     0,    98,     0,   221,     0,     0,     0,   288,   231,
       0,   239,     0,   238,   237,   243,   244,   235,     0,     0,
       0,    34,   176,    62,     0,     0,     0,     0,     0,     0,
       0,    67,    68,    84,     0,     0,   109,   124,     0,     0,
       0,   282,   275,     0,     0,     0,   258,   259,   257,     0,
     255,   126,     0,     0,     0,   220,     0,     0,   216,   223,
     263,   264,   262,   265,   104,   241,   248,     0,     0,   283,
       0,   233,     0,     0,   173,   172,   180,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   112,   110,   204,   280,   279,   281,   272,
     271,   261,     0,     0,   127,   128,     0,    96,    97,   222,
       0,     0,   226,     0,   249,   250,   246,   247,   245,     0,
      38,    39,    40,    35,    36,    31,     0,     0,    43,     0,
      70,    69,     0,     0,    76,     0,    90,    72,    83,    91,
      73,    92,    80,    87,    79,    86,    81,    88,    82,    89,
       0,   107,   106,     0,     0,   121,   113,   260,     0,     0,
      95,   225,     0,   224,   234,     0,     0,     0,    33,    71,
       0,     0,     0,    77,     0,     0,     0,     0,   125,   130,
     131,   134,     0,   129,   227,    37,   118,    42,   116,    44,
      78,     0,    85,    74,     0,   108,     0,     0,   114,    46,
       0,     0,     0,   135,   119,   120,     0,     0,    75,   111,
       0,     0,     0,   132,     0,   117,    45,   115,   133,   137,
     136
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,    27,   423,   473,   474,
     331,   478,   518,    28,    86,    87,   152,    88,   245,   333,
     334,   381,   382,   383,   536,   300,   211,   301,   386,   502,
     443,   444,   505,   506,   537,   538,   171,   172,   400,   401,
     454,   455,   552,   553,   220,    90,   221,   222,   147,    29,
     191,   108,   182,   183,    30,    31,   273,   274,    32,    33,
      34,    35,   293,   355,   408,   409,   461,   212,    36,    37,
     213,   214,   302,   304,   365,   366,   417,   466,    38,    39,
      40,    41,   289,   290,   413,    42,    43,   286,   287,   343,
     344,    44,    45,    46,    97,   392,    47,   308,    48,    49,
      50,    51,    91,    52,    53
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -461
static const short yypact[] =
{
     957,    -9,    24,  -461,    82,  -461,    71,   144,  -461,  -461,
      78,    48,  -461,   157,  -461,  -461,   224,   105,   953,   115,
     167,   205,   210,  -461,   142,  -461,  -461,  -461,  -461,  -461,
    -461,  -461,  -461,  -461,  -461,   179,  -461,  -461,  -461,   246,
    -461,  -461,  -461,  -461,  -461,  -461,  -461,  -461,  -461,  -461,
    -461,  -461,  -461,  -461,   252,   262,   166,   283,   205,   294,
     298,   303,   190,  -461,  -461,  -461,  -461,   220,   195,   209,
     228,   230,   231,   232,  -461,   233,   234,   235,   236,   238,
     239,   627,  -461,   627,   627,   355,   -31,  -461,    67,   956,
    -461,   302,   256,   257,    79,    40,  -461,   273,    16,   285,
    -461,  -461,   362,   310,  -461,   310,   310,  -461,  -461,   272,
    -461,   365,  -461,   -18,  -461,   102,   366,   310,   363,   287,
      21,   305,   -59,  -461,  -461,  -461,   463,   376,   627,   547,
      -5,   547,   261,   627,   547,   547,   627,   627,   627,   263,
     265,   267,   269,  -461,  -461,   510,   277,   -17,     1,   325,
    -461,   385,  -461,   627,   627,   627,   627,   627,   627,   627,
     627,   627,   627,   627,   627,   627,   627,   627,   627,   627,
     384,  -461,  -461,   106,    40,   286,   289,  -461,  -461,  -461,
    -461,  -461,  -461,   280,   340,   312,   313,   310,  -461,   314,
     390,  -461,  -461,  -461,   301,  -461,   398,   399,   402,   177,
    -461,   291,  -461,   377,   337,  -461,  -461,  -461,   187,     7,
    -461,  -461,  -461,   293,  -461,   135,   368,  -461,   587,  -461,
     956,    93,  -461,  -461,   744,   103,   411,   297,   128,  -461,
     771,   138,   155,   609,   663,   798,  -461,   486,   627,   627,
    -461,    20,   414,  -461,  -461,   -48,  -461,  -461,  -461,  -461,
     972,   989,  1009,  1068,   367,   367,   182,   182,   182,   182,
     139,   139,  -461,  -461,  -461,   299,  -461,  -461,  -461,  -461,
    -461,   413,  -461,  -461,  -461,   280,   197,   307,    40,   371,
     310,   310,  -461,  -461,  -461,   417,  -461,  -461,   319,   -46,
    -461,  -461,   194,   333,   425,   427,  -461,  -461,  -461,  -461,
    -461,   170,   180,    21,   315,  -461,  -461,  -461,   350,   -28,
     325,   316,  -461,   547,  -461,  -461,   320,  -461,  -461,  -461,
    -461,  -461,  -461,   627,  -461,   627,  -461,   690,   717,  -461,
     329,   369,     4,   394,  -461,   439,  -461,    20,  -461,   132,
    -461,  -461,   347,   352,  -461,    58,   398,   388,  -461,  -461,
     338,   339,  -461,   341,  -461,   183,   342,   168,  -461,  -461,
      20,  -461,   447,    53,  -461,   344,  -461,  -461,   450,   364,
      20,   345,  -461,  -461,   825,   852,    20,   325,   343,   370,
       4,   382,  -461,  -461,   173,   461,   387,  -461,   185,   -13,
     400,  -461,  -461,   485,   417,    17,  -461,  -461,  -461,   493,
    -461,  -461,   386,   378,   381,  -461,   194,    27,   383,  -461,
    -461,  -461,  -461,  -461,  -461,  -461,  -461,    55,    27,  -461,
      20,  -461,   184,   393,  -461,  -461,  -461,   -20,   435,   496,
     -44,     4,    60,     9,    32,    36,    60,    60,    60,    60,
     457,   194,   462,   441,  -461,  -461,  -461,  -461,  -461,  -461,
    -461,  -461,   191,   405,   396,  -461,   407,  -461,  -461,  -461,
      18,   204,  -461,   342,  -461,  -461,  -461,   502,  -461,   206,
    -461,  -461,  -461,   401,  -461,  -461,   205,   504,   471,   410,
    -461,  -461,   418,   429,  -461,    20,  -461,  -461,  -461,  -461,
    -461,  -461,  -461,  -461,  -461,  -461,  -461,  -461,  -461,  -461,
      22,  -461,   412,   465,   512,   302,  -461,  -461,     6,   493,
    -461,  -461,    27,  -461,  -461,   184,   194,   531,  -461,  -461,
      20,    30,   213,  -461,    20,   194,   514,   169,   388,   421,
    -461,  -461,   537,  -461,  -461,  -461,    38,   422,  -461,   423,
    -461,   535,  -461,  -461,   218,  -461,   194,   426,   422,  -461,
     540,   445,   221,  -461,  -461,  -461,   194,   549,  -461,   422,
     438,   440,    20,  -461,   537,  -461,  -461,  -461,  -461,  -461,
    -461
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -461,  -461,  -461,  -461,   443,  -461,   346,  -461,  -461,    46,
    -461,  -461,  -461,   237,   186,   416,  -461,  -461,    33,  -461,
     278,  -333,  -461,  -461,  -264,  -120,  -342,  -336,  -461,  -461,
    -461,  -461,  -461,  -461,  -460,    10,    57,  -461,    42,  -461,
    -461,    62,  -461,    11,    -3,  -461,   133,   260,  -461,  -461,
    -100,  -461,   309,   403,  -461,  -461,   300,  -461,  -461,  -461,
    -461,  -461,  -461,  -461,  -461,   116,  -461,  -301,  -461,  -461,
    -461,   282,  -461,  -461,  -461,   162,  -461,  -461,  -461,  -461,
    -461,  -461,  -461,   242,  -461,  -461,  -461,  -461,  -461,  -461,
     208,  -461,  -461,  -461,   571,  -461,  -461,  -461,  -461,  -461,
    -461,  -461,  -461,  -461,  -461
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -248
static const short yytable[] =
{
     210,   388,   363,   397,   112,   192,   193,   348,   349,   529,
     148,   530,   205,    89,   531,   298,   446,   202,   484,   185,
     369,   476,   205,   205,   186,   205,   205,   206,   354,   207,
     226,   523,   205,   206,   350,   207,   206,   205,   206,   216,
     486,   205,   206,   177,   489,   178,   179,   430,   180,   351,
     332,   352,   332,   272,   113,   554,  -247,   431,   464,   452,
     196,   197,   379,   205,   206,   205,   206,   548,   384,   416,
     150,   555,   197,    55,   346,   480,   370,    54,   143,   447,
     144,   145,   175,   151,   469,    56,   559,   282,   176,   149,
     483,   122,   488,   491,   493,   495,   497,   499,   481,   187,
     149,   353,   197,   242,   181,   243,   462,   227,    92,   266,
    -247,   205,   465,    57,   267,    93,   384,   467,   271,   244,
      59,   329,   380,    60,   532,   224,   299,   485,   271,   271,
     230,   271,   208,   233,   234,   235,   451,   299,   208,   209,
     524,   541,   459,   208,   268,   460,    89,   208,    94,   522,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   384,    95,   208,
     269,   208,   348,   349,   305,   153,   395,   501,   198,   542,
     340,   341,   199,   210,    96,    58,   306,   470,   544,   471,
     410,    98,   296,   297,   432,    89,   270,   348,   349,   350,
     266,   154,   389,   390,   110,   267,    61,   411,   112,   391,
     114,   534,   312,   313,   351,   145,   352,   271,   153,   307,
     412,   433,   315,   313,   350,   396,   102,    62,    63,    64,
      65,   472,   116,    66,    67,   327,   328,   103,   547,   351,
     414,   352,   111,    68,   154,   104,    69,   318,   313,   117,
     421,   167,   168,   169,    70,   118,   426,   320,   313,   115,
     105,   545,   225,    71,   228,   119,   353,   231,   232,    72,
      73,    74,    75,   107,   321,   313,    76,   434,   435,   436,
     437,   438,   439,    77,   120,    78,   121,   210,   440,   359,
     360,   353,   165,   166,   167,   168,   169,   123,   210,   361,
     362,   124,   405,   406,   445,   360,   125,    89,   126,    79,
     507,   360,   482,   128,   487,   490,   492,   494,   496,   498,
     374,    80,   375,   511,   512,   514,   360,   129,   140,    63,
      64,    65,   543,   360,    66,    81,    82,   558,   360,    83,
     563,   564,    84,   127,    68,    85,   130,    69,   131,   132,
     133,   134,   135,   136,   137,    70,   138,   139,   146,   170,
     173,   174,   184,   188,    71,   189,   190,   194,   195,   201,
      72,    73,    74,    75,    89,   204,   203,    76,   215,   223,
     229,   241,   236,   237,    77,   238,    78,   239,   247,   265,
     276,   278,   210,   277,   279,   280,   281,   283,   284,   285,
     540,   288,   291,   153,   294,    62,    63,    64,    65,   292,
      79,    66,   295,   303,   316,   309,   317,   330,   296,   335,
     342,    68,    80,   345,    69,   337,   339,   356,   357,   154,
     358,   368,    70,   376,   367,   371,    81,    82,   385,   373,
      83,    71,   569,    84,   387,   377,    85,    72,    73,    74,
      75,   393,   394,   399,    76,   415,   402,   403,   419,   404,
     407,    77,   428,    78,   418,   422,   140,    63,    64,    65,
     448,   217,    66,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   420,   431,   441,    69,   442,    79,   429,   140,
      63,    64,    65,   449,   217,    66,   453,   457,   456,    80,
     458,   477,    71,   463,   479,   500,   503,   504,    69,   508,
      74,    75,   475,    81,    82,    76,   509,    83,   416,   520,
      84,   515,   141,    85,   142,    71,   510,   516,   517,   519,
     521,   526,   525,    74,    75,   527,   539,   546,    76,   550,
     551,   297,   556,   557,   560,   141,   153,   142,   561,   562,
     140,    63,    64,    65,   566,   217,    66,   567,   200,   568,
      80,   535,   528,   427,   311,   246,   565,   347,   378,    69,
     549,   533,   154,   372,    81,   570,   336,   275,    83,   513,
     468,   218,   219,    80,    85,   364,    71,   338,   398,   109,
     140,    63,    64,    65,    74,    75,    66,    81,     0,    76,
       0,    83,   450,     0,    84,   219,   141,    85,   142,    69,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,     0,    71,     0,     0,   240,
     140,    63,    64,    65,    74,    75,    66,     0,     0,    76,
       0,     0,     0,     0,    80,   153,   141,     0,   142,    69,
       0,     0,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,    83,   310,     0,    84,    71,     0,    85,     0,
       0,   154,     0,     0,    74,    75,     0,     0,     0,    76,
       0,     0,     0,     0,    80,     0,   141,     0,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    81,   153,
       0,     0,    83,     0,     0,    84,     0,     0,    85,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,    80,   154,   153,     0,   322,   323,
       0,     0,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,    83,     0,     0,    84,     0,     0,    85,     0,
       0,     0,   154,   153,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,     0,   154,
     153,     0,   324,   325,     0,     0,     0,     0,     0,     0,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,     0,   154,   153,     0,     0,
     323,     0,     0,     0,     0,     0,     0,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,     0,   154,   153,     0,     0,   325,     0,     0,
       0,     0,     0,     0,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,     0,
     154,   153,     0,   314,     0,     0,     0,     0,     0,     0,
       0,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,     0,   154,   153,     0,
     319,     0,     0,     0,     0,     0,     0,     0,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,     0,   154,     0,     0,   326,     0,     0,
       0,     0,     0,     0,     0,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
       0,     0,     0,     0,   424,     0,     0,     0,     0,     0,
       0,     0,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,    98,     0,     0,
       0,   425,     1,     0,     0,     2,     0,     3,    99,   100,
       0,     4,     0,     0,     0,     5,     0,     0,     6,     7,
       8,     9,   153,     0,    10,     0,   101,    11,     0,     0,
       0,     0,   102,     0,     0,     0,     0,    12,   153,     0,
       0,     0,     0,   103,     0,     0,     0,     0,   154,     0,
       0,   104,     0,     0,    13,   153,     0,     0,     0,    14,
       0,    15,    96,    16,   154,    17,   105,    18,     0,    19,
     106,     0,     0,     0,     0,   153,     0,     0,    20,   107,
      21,   154,     0,     0,     0,     0,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   154,     0,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,     0,     0,     0,
       0,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   153,     0,     0,     0,     0,     0,
       0,     0,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,     0,     0,     0,     0,     0,     0,
     154,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169
};

static const short yycheck[] =
{
     120,   337,   303,   345,     3,   105,   106,     3,     4,     3,
      41,     5,     5,    16,     8,     8,    29,   117,     9,     3,
      48,    41,     5,     5,     8,     5,     5,     6,   292,     8,
      35,     9,     5,     6,    30,     8,     6,     5,     6,    98,
       8,     5,     6,     3,     8,     5,     6,   380,     8,    45,
      98,    47,    98,   173,    21,    17,     3,   101,     3,   395,
      78,   120,    58,     5,     6,     5,     6,   527,   332,    16,
       3,    33,   120,    49,   120,   119,   104,    86,    81,    92,
      83,    84,     3,    16,   420,     3,   546,   187,     9,   120,
     432,    58,   434,   435,   436,   437,   438,   439,   431,    83,
     120,    97,   120,   120,    64,   122,   407,   112,     3,     3,
      57,     5,    57,    42,     8,    10,   380,   418,   111,   118,
      42,   241,   118,    75,   118,   128,   119,   118,   111,   111,
     133,   111,   111,   136,   137,   138,   119,   119,   111,   118,
     118,   111,   406,   111,    38,   118,   149,   111,    43,   485,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   431,    63,   111,
      64,   111,     3,     4,    39,    36,   118,   441,    76,   521,
     280,   281,    80,   303,    79,    41,    51,     3,   524,     5,
      22,    14,     5,     6,    21,   198,    90,     3,     4,    30,
       3,    62,    70,    71,    89,     8,    49,    39,     3,    77,
       0,   512,   119,   120,    45,   218,    47,   111,    36,    84,
      52,    48,   119,   120,    30,   345,    49,     3,     4,     5,
       6,    47,    53,     9,    10,   238,   239,    60,    69,    45,
     360,    47,    75,    19,    62,    68,    22,   119,   120,     3,
     370,   112,   113,   114,    30,     3,   376,   119,   120,   117,
      83,   525,   129,    39,   131,     3,    97,   134,   135,    45,
      46,    47,    48,    96,   119,   120,    52,   104,   105,   106,
     107,   108,   109,    59,   118,    61,     3,   407,   115,   119,
     120,    97,   110,   111,   112,   113,   114,     3,   418,   119,
     120,     3,   119,   120,   119,   120,     3,   310,   118,    85,
     119,   120,   432,   118,   434,   435,   436,   437,   438,   439,
     323,    97,   325,   119,   120,   119,   120,   118,     3,     4,
       5,     6,   119,   120,     9,   111,   112,   119,   120,   115,
     119,   120,   118,   123,    19,   121,   118,    22,   118,   118,
     118,   118,   118,   118,   118,    30,   118,   118,     3,    57,
     104,   104,    89,    78,    39,     3,    56,    95,     3,     3,
      45,    46,    47,    48,   377,    88,    13,    52,    73,     3,
     119,   104,   119,   118,    59,   118,    61,   118,     3,     5,
     104,   111,   512,   104,    54,    83,    83,    83,     8,    98,
     520,     3,     3,    36,    27,     3,     4,     5,     6,   118,
      85,     9,    75,   120,     3,    47,   119,     3,     5,   120,
       3,    19,    97,   104,    22,   118,    55,    94,     3,    62,
       3,    81,    30,   104,   119,   119,   111,   112,    44,   119,
     115,    39,   562,   118,     5,    76,   121,    45,    46,    47,
      48,   104,   100,    65,    52,     8,   118,   118,     8,   118,
     118,    59,   119,    61,   120,   120,     3,     4,     5,     6,
      70,     8,     9,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   118,   101,    23,    22,    99,    85,   118,     3,
       4,     5,     6,     8,     8,     9,     3,   119,   112,    97,
     119,    66,    39,   120,     8,    48,    44,    66,    22,   104,
      47,    48,   119,   111,   112,    52,   120,   115,    16,   101,
     118,   120,    59,   121,    61,    39,   119,    23,    57,   119,
     101,    66,   120,    47,    48,    23,     5,    23,    52,   118,
       3,     6,   120,   120,   118,    59,    36,    61,     8,   104,
       3,     4,     5,     6,     5,     8,     9,   119,   115,   119,
      97,   515,   505,   377,   218,   149,   556,   289,   331,    22,
     528,   509,    62,   313,   111,   564,   276,   174,   115,   463,
     418,   118,   119,    97,   121,   303,    39,   278,   346,    18,
       3,     4,     5,     6,    47,    48,     9,   111,    -1,    52,
      -1,   115,   394,    -1,   118,   119,    59,   121,    61,    22,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    -1,    39,    -1,    -1,   119,
       3,     4,     5,     6,    47,    48,     9,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    97,    36,    59,    -1,    61,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,   115,    76,    -1,   118,    39,    -1,   121,    -1,
      -1,    62,    -1,    -1,    47,    48,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    97,    -1,    59,    -1,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    36,
      -1,    -1,   115,    -1,    -1,   118,    -1,    -1,   121,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    97,    62,    36,    -1,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,   115,    -1,    -1,   118,    -1,    -1,   121,    -1,
      -1,    -1,    62,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    -1,    62,
      36,    -1,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    -1,    62,    36,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    -1,    62,    36,    -1,    -1,   120,    -1,    -1,
      -1,    -1,    -1,    -1,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    -1,
      62,    36,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    -1,    62,    36,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    -1,    62,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    14,    -1,    -1,
      -1,   119,    15,    -1,    -1,    18,    -1,    20,    25,    26,
      -1,    24,    -1,    -1,    -1,    28,    -1,    -1,    31,    32,
      33,    34,    36,    -1,    37,    -1,    43,    40,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    50,    36,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    68,    -1,    -1,    67,    36,    -1,    -1,    -1,    72,
      -1,    74,    79,    76,    62,    78,    83,    80,    -1,    82,
      87,    -1,    -1,    -1,    -1,    36,    -1,    -1,    91,    96,
      93,    62,    -1,    -1,    -1,    -1,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    62,    -1,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    -1,    -1,    -1,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    18,    20,    24,    28,    31,    32,    33,    34,
      37,    40,    50,    67,    72,    74,    76,    78,    80,    82,
      91,    93,   125,   126,   127,   128,   129,   130,   137,   173,
     178,   179,   182,   183,   184,   185,   192,   193,   202,   203,
     204,   205,   209,   210,   215,   216,   217,   220,   222,   223,
     224,   225,   227,   228,    86,    49,     3,    42,    41,    42,
      75,    49,     3,     4,     5,     6,     9,    10,    19,    22,
      30,    39,    45,    46,    47,    48,    52,    59,    61,    85,
      97,   111,   112,   115,   118,   121,   138,   139,   141,   168,
     169,   226,     3,    10,    43,    63,    79,   218,    14,    25,
      26,    43,    49,    60,    68,    83,    87,    96,   175,   218,
      89,    75,     3,   142,     0,   117,    53,     3,     3,     3,
     118,     3,   142,     3,     3,     3,   118,   123,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
       3,    59,    61,   168,   168,   168,     3,   172,    41,   120,
       3,    16,   140,    36,    62,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      57,   160,   161,   104,   104,     3,     9,     3,     5,     6,
       8,    64,   176,   177,    89,     3,     8,    83,    78,     3,
      56,   174,   174,   174,    95,     3,    78,   120,    76,    80,
     128,     3,   174,    13,    88,     5,     6,     8,   111,   118,
     149,   150,   191,   194,   195,    73,    98,     8,   118,   119,
     168,   170,   171,     3,   168,   170,    35,   112,   170,   119,
     168,   170,   170,   168,   168,   168,   119,   118,   118,   118,
     119,   104,   120,   122,   118,   142,   139,     3,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,     5,     3,     8,    38,    64,
      90,   111,   149,   180,   181,   177,   104,   104,   111,    54,
      83,    83,   174,    83,     8,    98,   211,   212,     3,   206,
     207,     3,   118,   186,    27,    75,     5,     6,     8,   119,
     149,   151,   196,   120,   197,    39,    51,    84,   221,    47,
      76,   130,   119,   120,   119,   119,     3,   119,   119,   119,
     119,   119,   119,   120,   119,   120,   119,   168,   168,   149,
       3,   134,    98,   143,   144,   120,   180,   118,   176,    55,
     174,   174,     3,   213,   214,   104,   120,   144,     3,     4,
      30,    45,    47,    97,   148,   187,    94,     3,     3,   119,
     120,   119,   120,   191,   195,   198,   199,   119,    81,    48,
     104,   119,   171,   119,   168,   168,   104,    76,   137,    58,
     118,   145,   146,   147,   148,    44,   152,     5,   151,    70,
      71,    77,   219,   104,   100,   118,   149,   150,   207,    65,
     162,   163,   118,   118,   118,   119,   120,   118,   188,   189,
      22,    39,    52,   208,   149,     8,    16,   200,   120,     8,
     118,   149,   120,   131,   119,   119,   149,   138,   119,   118,
     145,   101,    21,    48,   104,   105,   106,   107,   108,   109,
     115,    23,    99,   154,   155,   119,    29,    92,    70,     8,
     214,   119,   151,     3,   164,   165,   112,   119,   119,   148,
     118,   190,   191,   120,     3,    57,   201,   191,   199,   151,
       3,     5,    47,   132,   133,   119,    41,    66,   135,     8,
     119,   145,   149,   150,     9,   118,     8,   149,   150,     8,
     149,   150,   149,   150,   149,   150,   149,   150,   149,   150,
      48,   148,   153,    44,    66,   156,   157,   119,   104,   120,
     119,   119,   120,   189,   119,   120,    23,    57,   136,   119,
     101,   101,   151,     9,   118,   120,    66,    23,   160,     3,
       5,     8,   118,   165,   191,   133,   148,   158,   159,     5,
     149,   111,   150,   119,   151,   148,    23,    69,   158,   162,
     118,     3,   166,   167,    17,    33,   120,   120,   119,   158,
     118,     8,   104,   119,   120,   159,     5,   119,   119,   149,
     167
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
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->m_pStmt->m_sTableFunc = yyvsp[-6].m_sValue;
		;}
    break;

  case 33:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 36:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 37:

    {
			pParser->m_pStmt->m_dTableFuncArgs.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 39:

    { yyval.m_sValue.SetSprintf ( "%d", yyvsp[0].m_iValue ); ;}
    break;

  case 40:

    { yyval.m_sValue = "id"; ;}
    break;

  case 41:

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

  case 42:

    {
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 44:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 45:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 46:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 49:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 52:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 53:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 54:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 55:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 56:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 57:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 58:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 59:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 60:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 61:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 62:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 64:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 71:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 72:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 73:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 74:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 75:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 76:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 95:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 96:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 97:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 98:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 99:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 100:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 101:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 102:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 103:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 104:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 107:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 108:

    {
			pParser->AddGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 111:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 114:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 115:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 117:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 119:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 120:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 123:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 124:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 132:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 133:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 135:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 136:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 137:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 139:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 140:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 144:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 145:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 164:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 171:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 172:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 173:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 174:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 179:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 180:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 183:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 184:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 185:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 186:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 187:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 188:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 189:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 190:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 191:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 199:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 200:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 201:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 204:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 205:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 208:

    { yyval.m_iValue = 1; ;}
    break;

  case 209:

    { yyval.m_iValue = 0; ;}
    break;

  case 210:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 211:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 212:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 213:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 216:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 217:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 218:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 221:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 222:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 225:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 226:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 227:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 228:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 229:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 230:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 231:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 232:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 233:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 234:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 235:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 236:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 237:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 239:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 240:

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

  case 241:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 244:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 246:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 250:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 251:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 254:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 255:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 258:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 259:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 260:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 261:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 262:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 263:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 264:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 265:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER;
			tStmt.m_sIndex = yyvsp[-4].m_sValue;
			tStmt.m_sAlterAttr = yyvsp[-1].m_sValue;
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 266:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 273:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 274:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 275:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 283:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 284:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 285:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 286:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 287:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 288:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 289:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 290:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 292:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 293:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 294:

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

