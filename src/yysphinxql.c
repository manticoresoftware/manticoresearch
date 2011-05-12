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
     TOK_QUOTED_STRING = 262,
     TOK_USERVAR = 263,
     TOK_SYSVAR = 264,
     TOK_AS = 265,
     TOK_ASC = 266,
     TOK_AVG = 267,
     TOK_BEGIN = 268,
     TOK_BETWEEN = 269,
     TOK_BY = 270,
     TOK_CALL = 271,
     TOK_COLLATION = 272,
     TOK_COMMIT = 273,
     TOK_COUNT = 274,
     TOK_CREATE = 275,
     TOK_DELETE = 276,
     TOK_DESC = 277,
     TOK_DESCRIBE = 278,
     TOK_DISTINCT = 279,
     TOK_DIV = 280,
     TOK_DROP = 281,
     TOK_FALSE = 282,
     TOK_FLOAT = 283,
     TOK_FROM = 284,
     TOK_FUNCTION = 285,
     TOK_GLOBAL = 286,
     TOK_GROUP = 287,
     TOK_ID = 288,
     TOK_IN = 289,
     TOK_INSERT = 290,
     TOK_INT = 291,
     TOK_INTO = 292,
     TOK_LIMIT = 293,
     TOK_MATCH = 294,
     TOK_MATCH_WEIGHT = 295,
     TOK_MAX = 296,
     TOK_META = 297,
     TOK_MIN = 298,
     TOK_MOD = 299,
     TOK_NAMES = 300,
     TOK_NULL = 301,
     TOK_OPTION = 302,
     TOK_ORDER = 303,
     TOK_REPLACE = 304,
     TOK_RETURNS = 305,
     TOK_ROLLBACK = 306,
     TOK_SELECT = 307,
     TOK_SET = 308,
     TOK_SHOW = 309,
     TOK_SONAME = 310,
     TOK_START = 311,
     TOK_STATUS = 312,
     TOK_SUM = 313,
     TOK_TABLES = 314,
     TOK_TRANSACTION = 315,
     TOK_TRUE = 316,
     TOK_UPDATE = 317,
     TOK_VALUES = 318,
     TOK_VARIABLES = 319,
     TOK_WARNINGS = 320,
     TOK_WEIGHT = 321,
     TOK_WHERE = 322,
     TOK_WITHIN = 323,
     TOK_OR = 324,
     TOK_AND = 325,
     TOK_NE = 326,
     TOK_GTE = 327,
     TOK_LTE = 328,
     TOK_NOT = 329,
     TOK_NEG = 330
   };
#endif
#define TOK_IDENT 258
#define TOK_ATIDENT 259
#define TOK_CONST_INT 260
#define TOK_CONST_FLOAT 261
#define TOK_QUOTED_STRING 262
#define TOK_USERVAR 263
#define TOK_SYSVAR 264
#define TOK_AS 265
#define TOK_ASC 266
#define TOK_AVG 267
#define TOK_BEGIN 268
#define TOK_BETWEEN 269
#define TOK_BY 270
#define TOK_CALL 271
#define TOK_COLLATION 272
#define TOK_COMMIT 273
#define TOK_COUNT 274
#define TOK_CREATE 275
#define TOK_DELETE 276
#define TOK_DESC 277
#define TOK_DESCRIBE 278
#define TOK_DISTINCT 279
#define TOK_DIV 280
#define TOK_DROP 281
#define TOK_FALSE 282
#define TOK_FLOAT 283
#define TOK_FROM 284
#define TOK_FUNCTION 285
#define TOK_GLOBAL 286
#define TOK_GROUP 287
#define TOK_ID 288
#define TOK_IN 289
#define TOK_INSERT 290
#define TOK_INT 291
#define TOK_INTO 292
#define TOK_LIMIT 293
#define TOK_MATCH 294
#define TOK_MATCH_WEIGHT 295
#define TOK_MAX 296
#define TOK_META 297
#define TOK_MIN 298
#define TOK_MOD 299
#define TOK_NAMES 300
#define TOK_NULL 301
#define TOK_OPTION 302
#define TOK_ORDER 303
#define TOK_REPLACE 304
#define TOK_RETURNS 305
#define TOK_ROLLBACK 306
#define TOK_SELECT 307
#define TOK_SET 308
#define TOK_SHOW 309
#define TOK_SONAME 310
#define TOK_START 311
#define TOK_STATUS 312
#define TOK_SUM 313
#define TOK_TABLES 314
#define TOK_TRANSACTION 315
#define TOK_TRUE 316
#define TOK_UPDATE 317
#define TOK_VALUES 318
#define TOK_VARIABLES 319
#define TOK_WARNINGS 320
#define TOK_WEIGHT 321
#define TOK_WHERE 322
#define TOK_WITHIN 323
#define TOK_OR 324
#define TOK_AND 325
#define TOK_NE 326
#define TOK_GTE 327
#define TOK_LTE 328
#define TOK_NOT 329
#define TOK_NEG 330




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
#define YYFINAL  78
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   671

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  90
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  72
/* YYNRULES -- Number of rules. */
#define YYNRULES  206
/* YYNRULES -- Number of states. */
#define YYNSTATES  407

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   330

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    83,    72,     2,
      88,    89,    81,    79,    87,    80,     2,    82,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    86,
      75,    73,    76,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    71,     2,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    74,    77,    78,    84,
      85
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      42,    44,    46,    57,    59,    63,    65,    67,    71,    78,
      85,    92,    99,   101,   108,   114,   120,   128,   129,   131,
     133,   137,   138,   140,   143,   145,   149,   154,   158,   162,
     168,   175,   179,   184,   190,   194,   198,   202,   206,   210,
     214,   218,   222,   228,   232,   236,   238,   240,   245,   249,
     253,   255,   257,   260,   262,   265,   267,   271,   272,   274,
     278,   279,   281,   287,   288,   290,   294,   296,   300,   302,
     305,   308,   309,   311,   314,   319,   320,   322,   325,   327,
     331,   335,   339,   345,   347,   351,   355,   357,   359,   361,
     363,   365,   367,   370,   373,   377,   381,   385,   389,   393,
     397,   401,   405,   409,   413,   417,   421,   425,   429,   433,
     437,   441,   445,   447,   452,   457,   461,   468,   475,   477,
     481,   483,   485,   488,   490,   492,   494,   496,   498,   500,
     502,   504,   509,   514,   519,   523,   528,   536,   542,   544,
     546,   548,   550,   552,   554,   556,   558,   560,   563,   570,
     572,   574,   575,   579,   581,   585,   587,   591,   595,   597,
     601,   603,   605,   607,   615,   625,   632,   633,   636,   638,
     642,   646,   648,   650,   653,   655,   657,   660,   669,   671,
     675,   679,   682,   685,   693,   695,   697
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      91,     0,    -1,    92,    -1,    93,    -1,    93,    86,    -1,
     137,    -1,   145,    -1,   131,    -1,   132,    -1,   135,    -1,
     146,    -1,   151,    -1,   153,    -1,   154,    -1,   157,    -1,
     158,    -1,   159,    -1,   161,    -1,    94,    -1,    93,    86,
      94,    -1,    95,    -1,   128,    -1,    52,    96,    29,    99,
     100,   108,   110,   112,   116,   118,    -1,    97,    -1,    96,
      87,    97,    -1,     3,    -1,    33,    -1,   124,    98,     3,
      -1,    12,    88,   124,    89,    98,     3,    -1,    41,    88,
     124,    89,    98,     3,    -1,    43,    88,   124,    89,    98,
       3,    -1,    58,    88,   124,    89,    98,     3,    -1,    81,
      -1,    19,    88,    81,    89,    98,     3,    -1,    66,    88,
      89,    98,     3,    -1,    40,    88,    89,    98,     3,    -1,
      19,    88,    24,     3,    89,    98,     3,    -1,    -1,    10,
      -1,     3,    -1,    99,    87,     3,    -1,    -1,   101,    -1,
      67,   102,    -1,   103,    -1,   102,    70,   103,    -1,    39,
      88,     7,    89,    -1,   104,    73,   105,    -1,   104,    74,
     105,    -1,   104,    34,    88,   107,    89,    -1,   104,    84,
      34,    88,   107,    89,    -1,   104,    34,     8,    -1,   104,
      84,    34,     8,    -1,   104,    14,   105,    70,   105,    -1,
     104,    76,   105,    -1,   104,    75,   105,    -1,   104,    77,
     105,    -1,   104,    78,   105,    -1,   104,    73,   106,    -1,
     104,    74,   106,    -1,   104,    76,   106,    -1,   104,    75,
     106,    -1,   104,    14,   106,    70,   106,    -1,   104,    77,
     106,    -1,   104,    78,   106,    -1,     3,    -1,     4,    -1,
      19,    88,    81,    89,    -1,    66,    88,    89,    -1,    40,
      88,    89,    -1,    33,    -1,     5,    -1,    80,     5,    -1,
       6,    -1,    80,     6,    -1,   105,    -1,   107,    87,   105,
      -1,    -1,   109,    -1,    32,    15,   104,    -1,    -1,   111,
      -1,    68,    32,    48,    15,   114,    -1,    -1,   113,    -1,
      48,    15,   114,    -1,   115,    -1,   114,    87,   115,    -1,
     104,    -1,   104,    11,    -1,   104,    22,    -1,    -1,   117,
      -1,    38,     5,    -1,    38,     5,    87,     5,    -1,    -1,
     119,    -1,    47,   120,    -1,   121,    -1,   120,    87,   121,
      -1,     3,    73,     3,    -1,     3,    73,     5,    -1,     3,
      73,    88,   122,    89,    -1,   123,    -1,   122,    87,   123,
      -1,     3,    73,   105,    -1,     3,    -1,     4,    -1,    33,
      -1,     5,    -1,     6,    -1,     8,    -1,    80,   124,    -1,
      84,   124,    -1,   124,    79,   124,    -1,   124,    80,   124,
      -1,   124,    81,   124,    -1,   124,    82,   124,    -1,   124,
      75,   124,    -1,   124,    76,   124,    -1,   124,    72,   124,
      -1,   124,    71,   124,    -1,   124,    83,   124,    -1,   124,
      25,   124,    -1,   124,    44,   124,    -1,   124,    78,   124,
      -1,   124,    77,   124,    -1,   124,    73,   124,    -1,   124,
      74,   124,    -1,   124,    70,   124,    -1,   124,    69,   124,
      -1,    88,   124,    89,    -1,   125,    -1,     3,    88,   126,
      89,    -1,    34,    88,   126,    89,    -1,     3,    88,    89,
      -1,    43,    88,   124,    87,   124,    89,    -1,    41,    88,
     124,    87,   124,    89,    -1,   127,    -1,   126,    87,   127,
      -1,   124,    -1,     7,    -1,    54,   129,    -1,    65,    -1,
      57,    -1,    42,    -1,     3,    -1,    46,    -1,     7,    -1,
       5,    -1,     6,    -1,    53,     3,    73,   134,    -1,    53,
       3,    73,   133,    -1,    53,     3,    73,    46,    -1,    53,
      45,   130,    -1,    53,     9,    73,   130,    -1,    53,    31,
       8,    73,    88,   107,    89,    -1,    53,    31,     3,    73,
     133,    -1,     3,    -1,     7,    -1,    61,    -1,    27,    -1,
     105,    -1,    18,    -1,    51,    -1,   136,    -1,    13,    -1,
      56,    60,    -1,   138,    37,     3,   139,    63,   141,    -1,
      35,    -1,    49,    -1,    -1,    88,   140,    89,    -1,   104,
      -1,   140,    87,   104,    -1,   142,    -1,   141,    87,   142,
      -1,    88,   143,    89,    -1,   144,    -1,   143,    87,   144,
      -1,   105,    -1,   106,    -1,     7,    -1,    21,    29,     3,
      67,    33,    73,   105,    -1,    21,    29,     3,    67,    33,
      34,    88,   107,    89,    -1,    16,     3,    88,   143,   147,
      89,    -1,    -1,    87,   148,    -1,   149,    -1,   148,    87,
     149,    -1,   144,    98,   150,    -1,     3,    -1,    38,    -1,
     152,     3,    -1,    23,    -1,    22,    -1,    54,    59,    -1,
      62,     3,    53,   155,    67,    33,    73,   105,    -1,   156,
      -1,   155,    87,   156,    -1,     3,    73,   105,    -1,    54,
      64,    -1,    54,    17,    -1,    20,    30,     3,    50,   160,
      55,     7,    -1,    36,    -1,    28,    -1,    26,    30,     3,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   104,   104,   105,   106,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   128,   129,
     133,   134,   138,   153,   154,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   173,   175,   179,
     180,   183,   185,   189,   193,   194,   198,   203,   210,   218,
     226,   235,   240,   245,   250,   255,   260,   265,   270,   271,
     272,   273,   278,   283,   288,   296,   297,   302,   308,   314,
     320,   329,   330,   334,   335,   339,   345,   351,   353,   357,
     364,   366,   370,   376,   378,   382,   389,   390,   394,   395,
     396,   399,   401,   405,   410,   417,   419,   423,   427,   428,
     432,   437,   442,   451,   456,   463,   473,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   503,   504,   505,   506,   507,   511,   512,
     516,   517,   523,   527,   528,   529,   535,   536,   537,   538,
     539,   543,   548,   553,   558,   559,   563,   568,   576,   577,
     581,   582,   583,   597,   598,   599,   603,   604,   610,   618,
     619,   622,   624,   628,   629,   633,   634,   638,   642,   643,
     647,   648,   649,   655,   661,   673,   680,   682,   686,   691,
     695,   703,   704,   710,   718,   719,   725,   731,   742,   743,
     747,   761,   765,   771,   782,   783,   787
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_ATIDENT", 
  "TOK_CONST_INT", "TOK_CONST_FLOAT", "TOK_QUOTED_STRING", "TOK_USERVAR", 
  "TOK_SYSVAR", "TOK_AS", "TOK_ASC", "TOK_AVG", "TOK_BEGIN", 
  "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_COLLATION", "TOK_COMMIT", 
  "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", 
  "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_ID", 
  "TOK_IN", "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MATCH_WEIGHT", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", 
  "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", 
  "TOK_RETURNS", "TOK_ROLLBACK", "TOK_SELECT", "TOK_SET", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_SUM", "TOK_TABLES", 
  "TOK_TRANSACTION", "TOK_TRUE", "TOK_UPDATE", "TOK_VALUES", 
  "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", 
  "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", 
  "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", 
  "TOK_NOT", "TOK_NEG", "';'", "','", "'('", "')'", "$accept", "request", 
  "statement", "multi_stmt_list", "multi_stmt", "select_from", 
  "select_items_list", "select_item", "opt_as", "ident_list", 
  "opt_where_clause", "where_clause", "where_expr", "where_item", 
  "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "show_clause", "show_variable", "set_value", "set_clause", 
  "set_global_clause", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "opt_call_opts_list", "call_opts_list", "call_opt", "call_opt_name", 
  "describe", "describe_tok", "show_tables", "update", 
  "update_items_list", "update_item", "show_variables", "show_collation", 
  "create_function", "udf_type", "drop_function", 0
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
     325,   124,    38,    61,   326,    60,    62,   327,   328,    43,
      45,    42,    47,    37,   329,   330,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    90,    91,    91,    91,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    93,    93,
      94,    94,    95,    96,    96,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    98,    98,    99,
      99,   100,   100,   101,   102,   102,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   104,   104,   104,   104,   104,
     104,   105,   105,   106,   106,   107,   107,   108,   108,   109,
     110,   110,   111,   112,   112,   113,   114,   114,   115,   115,
     115,   116,   116,   117,   117,   118,   118,   119,   120,   120,
     121,   121,   121,   122,   122,   123,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   125,   125,   125,   125,   125,   126,   126,
     127,   127,   128,   129,   129,   129,   130,   130,   130,   130,
     130,   131,   131,   131,   131,   131,   132,   132,   133,   133,
     134,   134,   134,   135,   135,   135,   136,   136,   137,   138,
     138,   139,   139,   140,   140,   141,   141,   142,   143,   143,
     144,   144,   144,   145,   145,   146,   147,   147,   148,   148,
     149,   150,   150,   151,   152,   152,   153,   154,   155,   155,
     156,   157,   158,   159,   160,   160,   161
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,    10,     1,     3,     1,     1,     3,     6,     6,
       6,     6,     1,     6,     5,     5,     7,     0,     1,     1,
       3,     0,     1,     2,     1,     3,     4,     3,     3,     5,
       6,     3,     4,     5,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     3,     3,     1,     1,     4,     3,     3,
       1,     1,     2,     1,     2,     1,     3,     0,     1,     3,
       0,     1,     5,     0,     1,     3,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     4,     4,     3,     6,     6,     1,     3,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     4,     4,     3,     4,     7,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     6,     1,
       1,     0,     3,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     7,     9,     6,     0,     2,     1,     3,
       3,     1,     1,     2,     1,     1,     2,     8,     1,     3,
       3,     2,     2,     7,     1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   166,     0,   163,     0,     0,   195,   194,     0,   169,
     170,   164,     0,     0,     0,     0,     0,     0,     2,     3,
      18,    20,    21,     7,     8,     9,   165,     5,     0,     6,
      10,    11,     0,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,   106,   107,   109,   110,   111,     0,     0,
     108,     0,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,    23,    37,   132,     0,     0,     0,     0,   202,
     145,   144,   196,   201,   143,   142,   167,     0,     1,     4,
       0,   193,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   106,   108,     0,     0,   112,
     113,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   146,   149,   150,
     148,   147,   154,     0,     0,    19,   171,    71,    73,   182,
       0,   180,   181,   186,   178,     0,     0,   141,   135,   140,
       0,   138,     0,     0,     0,     0,    37,     0,     0,     0,
      37,     0,     0,   131,    39,    41,    24,   123,   124,   130,
     129,   121,   120,   127,   128,   118,   119,   126,   125,   114,
     115,   116,   117,   122,    27,   158,   159,   161,   153,   160,
       0,   162,   152,   151,   155,     0,     0,     0,     0,   198,
       0,     0,    72,    74,     0,     0,   205,   204,     0,     0,
       0,   133,    37,     0,    37,   134,     0,     0,    37,     0,
      37,    37,     0,     0,     0,     0,     0,    77,    42,   157,
       0,     0,     0,     0,    65,    66,     0,    70,     0,     0,
     173,     0,     0,    37,   187,   188,   185,     0,     0,     0,
     139,     0,    37,     0,    35,     0,     0,     0,     0,     0,
      34,     0,    43,    44,     0,    40,     0,    80,    78,    75,
       0,   200,     0,   199,     0,     0,     0,     0,   172,     0,
     168,   175,     0,     0,   203,     0,   183,    28,     0,    33,
     137,    29,   136,    30,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,    81,
       0,   156,     0,     0,    69,    68,   174,     0,     0,   191,
     192,   190,    37,   189,     0,    36,     0,    45,     0,     0,
      51,     0,    47,    58,    48,    59,    55,    61,    54,    60,
      56,    63,    57,    64,     0,    79,     0,     0,    91,    84,
      76,   197,    67,     0,   177,   176,   184,    46,     0,     0,
       0,    52,     0,     0,     0,     0,    95,    92,   179,    53,
       0,    62,    49,     0,     0,    88,    85,    86,    93,     0,
      22,    96,    50,    82,    89,    90,     0,     0,     0,    97,
      98,    87,    94,     0,     0,   100,   101,     0,    99,     0,
       0,   103,     0,     0,   102,   105,   104
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    61,    62,   282,   165,
     227,   228,   262,   263,   375,   141,   142,   270,   267,   268,
     308,   309,   348,   349,   376,   377,   366,   367,   380,   381,
     389,   390,   400,   401,   149,    64,   150,   151,    22,    75,
     132,    23,    24,   192,   193,    25,    26,    27,    28,   201,
     241,   280,   281,   143,   144,    29,    30,   205,   244,   245,
     321,    31,    32,    33,    34,   198,   199,    35,    36,    37,
     208,    38
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -277
static const short yypact[] =
{
     609,  -277,    46,  -277,    -3,    23,  -277,  -277,    36,  -277,
    -277,  -277,   188,   625,   547,     5,    72,    89,  -277,    42,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,    87,  -277,
    -277,  -277,   154,  -277,  -277,  -277,  -277,  -277,  -277,    73,
     157,   181,   185,    33,  -277,  -277,  -277,  -277,    85,   121,
      16,   128,   129,   132,   136,   139,   145,    26,  -277,    26,
      26,    35,  -277,   213,  -277,   133,   164,    55,    67,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,   189,  -277,    95,
     245,  -277,    32,   206,   199,  -277,     7,    26,    11,    13,
     178,    26,    26,    26,   182,   187,  -277,   192,   193,  -277,
    -277,   293,   267,   188,  -277,    26,    26,    26,    26,    26,
      26,    26,    26,    26,    26,    26,    26,    26,    26,    26,
      26,    26,   274,   198,    67,   225,   241,  -277,  -277,  -277,
    -277,  -277,  -277,   312,   106,  -277,   236,  -277,  -277,  -277,
      18,  -277,  -277,   235,  -277,   118,   292,  -277,  -277,   438,
     -11,  -277,   314,   323,   238,    64,   318,   230,   272,   335,
     318,    26,    26,  -277,  -277,   -16,  -277,  -277,  -277,   458,
     478,   498,   518,   538,   538,   170,   170,   170,   170,   502,
     502,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
     324,  -277,  -277,  -277,  -277,   116,   242,   258,   -10,  -277,
     207,   269,  -277,  -277,    32,   244,  -277,  -277,   279,   -12,
      13,  -277,   318,   246,   318,  -277,   333,    26,   318,    26,
     318,   318,   337,   398,   418,   179,   353,   306,  -277,  -277,
      -1,    -1,   344,   312,  -277,  -277,   290,  -277,   310,   311,
    -277,    83,   313,    15,   270,  -277,  -277,   373,   331,    -1,
    -277,   417,   318,   419,  -277,   356,   437,   377,   441,   461,
    -277,   396,   371,  -277,    91,  -277,   446,   397,  -277,  -277,
     110,  -277,   413,  -277,   423,   435,   436,   207,  -277,    32,
     439,  -277,    30,    32,  -277,    -1,  -277,  -277,   503,  -277,
    -277,  -277,  -277,  -277,  -277,   537,   179,    38,    -6,    38,
      38,    38,    38,    38,    38,   511,   207,   515,   500,  -277,
      -1,  -277,    -1,   476,  -277,  -277,  -277,   115,   313,  -277,
    -277,  -277,   318,  -277,   126,  -277,   477,  -277,   497,   499,
    -277,    -1,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,     0,  -277,   520,   571,   549,  -277,
    -277,  -277,  -277,    32,  -277,  -277,  -277,  -277,    -1,    20,
     143,  -277,    -1,   573,   207,   585,   555,  -277,  -277,  -277,
     597,  -277,  -277,   147,   207,    31,   521,  -277,   522,   602,
    -277,  -277,  -277,   521,  -277,  -277,   207,   605,   534,   536,
    -277,  -277,  -277,     2,   602,  -277,  -277,   621,  -277,   553,
     234,  -277,    -1,   621,  -277,  -277,  -277
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -277,  -277,  -277,  -277,   554,  -277,  -277,   533,   -62,  -277,
    -277,  -277,  -277,   341,  -197,  -123,   -39,  -276,  -277,  -277,
    -277,  -277,  -277,  -277,   264,   253,  -277,  -277,  -277,  -277,
    -277,   247,  -277,   237,    24,  -277,   556,   432,  -277,  -277,
     519,  -277,  -277,   451,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,   329,   369,  -198,  -277,  -277,  -277,  -277,   366,
    -277,  -277,  -277,  -277,  -277,  -277,   420,  -277,  -277,  -277,
    -277,  -277
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -180
static const short yytable[] =
{
     191,   122,   330,   240,   137,   395,   243,   396,   361,   324,
      95,    44,    45,    46,   147,    47,    95,    44,    45,    46,
     147,    47,   248,   202,   203,   104,   138,    40,   264,    95,
      44,    45,    46,   319,    47,   153,    63,   137,   138,   139,
      96,    51,   384,   137,   138,   -26,    96,    51,    97,    39,
      98,   225,    41,   385,    97,   360,    98,   232,   125,    96,
      51,   249,   -25,   126,   102,    76,    42,    97,   320,    98,
     127,   226,   128,   129,   130,    77,   210,   233,   211,   190,
     316,    99,   331,   100,   101,   322,   373,    57,   362,    78,
     397,    59,   154,    57,   216,    60,   148,    59,   222,   264,
     370,    60,  -179,   -26,  -179,   297,    57,   269,   271,   345,
      59,   152,   140,   131,    60,   157,   158,   159,   140,   185,
     -25,    86,   103,   186,    80,   298,   286,    63,    79,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   206,    12,    70,   134,
     251,   210,   253,   215,   207,   368,   256,    81,   258,   259,
      83,    82,   269,    71,   299,   300,   301,   302,   303,   304,
     277,    74,   278,    87,   328,   305,   332,   334,   336,   338,
     340,   342,   234,   235,    84,   223,   224,   350,    85,   351,
     288,    43,    44,    45,    46,   105,    47,   310,   236,   311,
      48,   185,   353,   137,   354,   186,   123,    49,   269,    88,
     234,   235,   237,   310,   106,   356,    89,    90,   261,   238,
      91,    50,    51,   104,    92,   187,   236,    93,    52,    53,
     310,    54,   372,    94,   310,   369,   382,   124,   105,   269,
     237,   255,   133,   257,   188,   239,    55,   238,   136,   117,
     118,   119,   120,   121,    56,   105,   145,   106,   329,   189,
     333,   335,   337,   339,   341,   343,   146,   156,    57,    58,
     164,   160,    59,   239,   106,    86,    60,   184,   190,   405,
     161,   162,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   105,   195,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   196,   197,   106,   217,   105,   218,
     371,   403,   204,   404,   200,   209,   213,   214,   104,   202,
     230,   231,   242,   246,   247,   252,   254,   106,   266,   105,
     260,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   265,   283,   106,   219,
     105,   220,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   272,   274,   106,
     284,   105,   163,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   275,   276,
     106,   279,   105,   212,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   285,
     287,   106,   289,   105,   221,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     291,   296,   106,   105,   293,   290,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   306,   106,   105,   294,   307,   292,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   106,   105,   295,   217,   312,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   106,   105,   313,   219,   325,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   106,   105,   314,   315,   318,   105,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   106,   105,   326,   344,   106,   346,   347,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   106,   105,    69,   352,   357,   358,   363,   359,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   106,   119,   120,   121,   364,   365,   374,    70,
     378,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   379,   203,    71,   388,    72,   393,   386,   387,
     392,    73,    74,   113,   114,   115,   116,   117,   118,   119,
     120,   121,     1,   394,   399,     2,   402,     3,    65,     4,
       5,     6,     7,   135,    66,     8,   166,   327,   383,   391,
     406,   398,   250,   194,     9,   155,   229,   355,   317,   323,
       0,     0,     0,   273,     0,     0,    67,     0,    10,     0,
      11,    12,    13,    14,     0,    15,     0,     0,     0,     0,
      68,    16
};

static const short yycheck[] =
{
     123,    63,     8,   200,     5,     3,   204,     5,     8,   285,
       3,     4,     5,     6,     7,     8,     3,     4,     5,     6,
       7,     8,    34,     5,     6,    10,     6,    30,   225,     3,
       4,     5,     6,     3,     8,    24,    12,     5,     6,     7,
      33,    34,    11,     5,     6,    29,    33,    34,    41,     3,
      43,    67,    29,    22,    41,   331,    43,    67,     3,    33,
      34,    73,    29,     8,    29,    60,    30,    41,    38,    43,
       3,    87,     5,     6,     7,     3,    87,    87,    89,    80,
     277,    57,    88,    59,    60,   283,   362,    80,    88,     0,
      88,    84,    81,    80,   156,    88,    89,    84,   160,   296,
      80,    88,    87,    87,    89,    14,    80,   230,   231,   306,
      84,    87,    80,    46,    88,    91,    92,    93,    80,     3,
      87,    88,    87,     7,    37,    34,   249,   103,    86,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    28,    52,    42,    54,
     212,    87,   214,    89,    36,   353,   218,     3,   220,   221,
       3,    88,   285,    57,    73,    74,    75,    76,    77,    78,
      87,    65,    89,    88,   297,    84,   299,   300,   301,   302,
     303,   304,     3,     4,     3,   161,   162,   310,     3,   312,
     252,     3,     4,     5,     6,    25,     8,    87,    19,    89,
      12,     3,    87,     5,    89,     7,    73,    19,   331,    88,
       3,     4,    33,    87,    44,    89,    88,    88,    39,    40,
      88,    33,    34,    10,    88,    27,    19,    88,    40,    41,
      87,    43,    89,    88,    87,   358,    89,    73,    25,   362,
      33,   217,    53,   219,    46,    66,    58,    40,     3,    79,
      80,    81,    82,    83,    66,    25,    50,    44,   297,    61,
     299,   300,   301,   302,   303,   304,    67,    89,    80,    81,
       3,    89,    84,    66,    44,    88,    88,     3,    80,   402,
      88,    88,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    25,    73,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    73,     3,    44,    87,    25,    89,
     359,    87,    87,    89,    88,    33,     3,    89,    10,     5,
      88,    73,    63,    89,    55,    89,     3,    44,    32,    25,
       3,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     3,    87,    44,    87,
      25,    89,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    33,    88,    44,
       7,    25,    89,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    88,    88,
      44,    88,    25,    89,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    88,
       3,    44,     3,    25,    89,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       3,    70,    44,    25,     3,    89,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    15,    44,    25,     3,    68,    89,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    44,    25,    88,    87,    73,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    44,    25,    81,    87,     3,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    44,    25,    89,    89,    87,    25,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    44,    25,     7,    34,    44,    32,    48,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    44,    25,    17,    89,    89,    70,    48,    70,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    44,    81,    82,    83,    15,    38,    15,    42,
       5,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    47,     6,    57,     3,    59,    73,    87,    87,
       5,    64,    65,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    13,    87,     3,    16,    73,    18,     3,    20,
      21,    22,    23,    79,     9,    26,   103,   296,   374,   386,
     403,   394,   210,   124,    35,    89,   195,   318,   279,   283,
      -1,    -1,    -1,   233,    -1,    -1,    31,    -1,    49,    -1,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      45,    62
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    13,    16,    18,    20,    21,    22,    23,    26,    35,
      49,    51,    52,    53,    54,    56,    62,    91,    92,    93,
      94,    95,   128,   131,   132,   135,   136,   137,   138,   145,
     146,   151,   152,   153,   154,   157,   158,   159,   161,     3,
      30,    29,    30,     3,     4,     5,     6,     8,    12,    19,
      33,    34,    40,    41,    43,    58,    66,    80,    81,    84,
      88,    96,    97,   124,   125,     3,     9,    31,    45,    17,
      42,    57,    59,    64,    65,   129,    60,     3,     0,    86,
      37,     3,    88,     3,     3,     3,    88,    88,    88,    88,
      88,    88,    88,    88,    88,     3,    33,    41,    43,   124,
     124,   124,    29,    87,    10,    25,    44,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    98,    73,    73,     3,     8,     3,     5,     6,
       7,    46,   130,    53,    54,    94,     3,     5,     6,     7,
      80,   105,   106,   143,   144,    50,    67,     7,    89,   124,
     126,   127,   124,    24,    81,   126,    89,   124,   124,   124,
      89,    88,    88,    89,     3,    99,    97,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,     3,     3,     7,    27,    46,    61,
      80,   105,   133,   134,   130,    73,    73,     3,   155,   156,
      88,   139,     5,     6,    87,   147,    28,    36,   160,    33,
      87,    89,    89,     3,    89,    89,    98,    87,    89,    87,
      89,    89,    98,   124,   124,    67,    87,   100,   101,   133,
      88,    73,    67,    87,     3,     4,    19,    33,    40,    66,
     104,   140,    63,   144,   148,   149,    89,    55,    34,    73,
     127,    98,    89,    98,     3,   124,    98,   124,    98,    98,
       3,    39,   102,   103,   104,     3,    32,   108,   109,   105,
     107,   105,    33,   156,    88,    88,    88,    87,    89,    88,
     141,   142,    98,    87,     7,    88,   105,     3,    98,     3,
      89,     3,    89,     3,     3,    88,    70,    14,    34,    73,
      74,    75,    76,    77,    78,    84,    15,    68,   110,   111,
      87,    89,    73,    81,    89,    89,   104,   143,    87,     3,
      38,   150,   144,   149,   107,     3,     7,   103,   105,   106,
       8,    88,   105,   106,   105,   106,   105,   106,   105,   106,
     105,   106,   105,   106,    34,   104,    32,    48,   112,   113,
     105,   105,    89,    87,    89,   142,    89,    89,    70,    70,
     107,     8,    88,    48,    15,    38,   116,   117,   144,   105,
      80,   106,    89,   107,    15,   104,   114,   115,     5,    47,
     118,   119,    89,   114,    11,    22,    87,    87,     3,   120,
     121,   115,     5,    73,    87,     3,     5,    88,   121,     3,
     122,   123,    73,    87,    89,   105,   123
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

  case 18:

    { pParser->PushQuery(); ;}
    break;

  case 19:

    { pParser->PushQuery(); ;}
    break;

  case 22:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 25:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 26:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( "id", &yyvsp[0], NULL ); pParser->SetNewSyntax(); ;}
    break;

  case 27:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 28:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 29:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 30:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 31:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 32:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 33:

    { pParser->SetSelect ( yyvsp[-5].m_iStart, yyvsp[0].m_iEnd ); if ( !pParser->AddItem ( "count(*)", &yyvsp[0], true ) ) YYERROR; ;}
    break;

  case 34:

    { pParser->SetSelect ( yyvsp[-4].m_iStart, yyvsp[0].m_iEnd ); if ( !pParser->AddItem ( "weight()", &yyvsp[0], true ) ) YYERROR; ;}
    break;

  case 35:

    { pParser->SetSelect ( yyvsp[-4].m_iStart, yyvsp[0].m_iEnd ); if ( !pParser->AddItem ( "weight()", &yyvsp[0], true ) ) YYERROR; ;}
    break;

  case 36:

    { pParser->SetSelect ( yyvsp[-6].m_iStart, yyvsp[0].m_iEnd ); if ( !pParser->AddDistinct ( &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 40:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 46:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 47:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 48:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 49:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 50:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 51:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 52:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 53:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 54:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 55:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 56:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 57:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 64:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 66:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 67:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 68:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 69:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 70:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 71:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 72:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 73:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 74:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 75:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 76:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 79:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 82:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 85:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 87:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 89:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 90:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 93:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 94:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 100:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 101:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 102:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 103:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 104:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 105:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 107:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 108:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 112:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 113:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 114:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 115:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 116:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 117:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 118:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 119:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 120:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 121:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 122:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 133:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 134:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 135:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 136:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 137:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 143:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 144:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 145:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 151:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 152:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 153:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 154:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 155:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 156:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 157:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 160:

    { yyval.m_iValue = 1; ;}
    break;

  case 161:

    { yyval.m_iValue = 0; ;}
    break;

  case 162:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 163:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 164:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 165:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 168:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 169:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 173:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 174:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 177:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 178:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 179:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 180:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 181:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 182:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 183:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 184:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 185:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 188:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 190:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 192:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 193:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 196:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 197:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_UPDATE;
			tStmt.m_sIndex = yyvsp[-6].m_sValue;	
			tStmt.m_tUpdate.m_dDocids.Add ( (SphDocID_t) yyvsp[0].m_iValue );
			tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );
		;}
    break;

  case 200:

    {
			CSphAttrUpdate & tUpd = pParser->m_pStmt->m_tUpdate;
			CSphColumnInfo & tAttr = tUpd.m_dAttrs.Add();
			tAttr.m_sName = yyvsp[-2].m_sValue;
			tAttr.m_sName.ToLower();
			tAttr.m_eAttrType = SPH_ATTR_INTEGER; // sorry, ints only for now, riding on legacy shit!
			tUpd.m_dPool.Add ( (DWORD) yyvsp[0].m_iValue );
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
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 204:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 205:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 206:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
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

