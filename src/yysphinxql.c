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
     TOK_ID = 293,
     TOK_IN = 294,
     TOK_INDEX = 295,
     TOK_INSERT = 296,
     TOK_INT = 297,
     TOK_INTO = 298,
     TOK_ISOLATION = 299,
     TOK_LEVEL = 300,
     TOK_LIMIT = 301,
     TOK_MATCH = 302,
     TOK_MAX = 303,
     TOK_META = 304,
     TOK_MIN = 305,
     TOK_MOD = 306,
     TOK_NAMES = 307,
     TOK_NULL = 308,
     TOK_OPTION = 309,
     TOK_ORDER = 310,
     TOK_RAND = 311,
     TOK_READ = 312,
     TOK_REPEATABLE = 313,
     TOK_REPLACE = 314,
     TOK_RETURNS = 315,
     TOK_ROLLBACK = 316,
     TOK_RTINDEX = 317,
     TOK_SELECT = 318,
     TOK_SERIALIZABLE = 319,
     TOK_SET = 320,
     TOK_SESSION = 321,
     TOK_SHOW = 322,
     TOK_SONAME = 323,
     TOK_START = 324,
     TOK_STATUS = 325,
     TOK_SUM = 326,
     TOK_TABLES = 327,
     TOK_TO = 328,
     TOK_TRANSACTION = 329,
     TOK_TRUE = 330,
     TOK_TRUNCATE = 331,
     TOK_UNCOMMITTED = 332,
     TOK_UPDATE = 333,
     TOK_VALUES = 334,
     TOK_VARIABLES = 335,
     TOK_WARNINGS = 336,
     TOK_WEIGHT = 337,
     TOK_WHERE = 338,
     TOK_WITHIN = 339,
     TOK_OR = 340,
     TOK_AND = 341,
     TOK_NE = 342,
     TOK_GTE = 343,
     TOK_LTE = 344,
     TOK_NOT = 345,
     TOK_NEG = 346
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
#define TOK_ID 293
#define TOK_IN 294
#define TOK_INDEX 295
#define TOK_INSERT 296
#define TOK_INT 297
#define TOK_INTO 298
#define TOK_ISOLATION 299
#define TOK_LEVEL 300
#define TOK_LIMIT 301
#define TOK_MATCH 302
#define TOK_MAX 303
#define TOK_META 304
#define TOK_MIN 305
#define TOK_MOD 306
#define TOK_NAMES 307
#define TOK_NULL 308
#define TOK_OPTION 309
#define TOK_ORDER 310
#define TOK_RAND 311
#define TOK_READ 312
#define TOK_REPEATABLE 313
#define TOK_REPLACE 314
#define TOK_RETURNS 315
#define TOK_ROLLBACK 316
#define TOK_RTINDEX 317
#define TOK_SELECT 318
#define TOK_SERIALIZABLE 319
#define TOK_SET 320
#define TOK_SESSION 321
#define TOK_SHOW 322
#define TOK_SONAME 323
#define TOK_START 324
#define TOK_STATUS 325
#define TOK_SUM 326
#define TOK_TABLES 327
#define TOK_TO 328
#define TOK_TRANSACTION 329
#define TOK_TRUE 330
#define TOK_TRUNCATE 331
#define TOK_UNCOMMITTED 332
#define TOK_UPDATE 333
#define TOK_VALUES 334
#define TOK_VARIABLES 335
#define TOK_WARNINGS 336
#define TOK_WEIGHT 337
#define TOK_WHERE 338
#define TOK_WITHIN 339
#define TOK_OR 340
#define TOK_AND 341
#define TOK_NE 342
#define TOK_GTE 343
#define TOK_LTE 344
#define TOK_NOT 345
#define TOK_NEG 346




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
#define YYFINAL  94
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   771

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  88
/* YYNRULES -- Number of rules. */
#define YYNRULES  242
/* YYNRULES -- Number of states. */
#define YYNSTATES  451

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   346

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    99,    88,     2,
     104,   105,    97,    95,   103,    96,     2,    98,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   102,
      91,    89,    92,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    87,     2,     2,     2,     2,     2,
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
      85,    86,    90,    93,    94,   100,   101
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
     106,   111,   115,   121,   123,   127,   128,   130,   133,   135,
     139,   144,   148,   152,   158,   165,   169,   174,   180,   184,
     188,   192,   196,   200,   204,   208,   212,   218,   222,   226,
     228,   230,   235,   239,   241,   243,   246,   248,   251,   253,
     257,   258,   260,   264,   265,   267,   273,   274,   276,   280,
     286,   288,   292,   294,   297,   300,   301,   303,   306,   311,
     312,   314,   317,   319,   323,   327,   331,   337,   344,   348,
     350,   354,   358,   360,   362,   364,   366,   368,   370,   373,
     376,   380,   384,   388,   392,   396,   400,   404,   408,   412,
     416,   420,   424,   428,   432,   436,   440,   444,   448,   450,
     455,   460,   464,   471,   478,   480,   484,   486,   488,   491,
     493,   495,   497,   499,   501,   503,   505,   507,   512,   517,
     522,   526,   531,   539,   545,   547,   549,   551,   553,   555,
     557,   559,   561,   563,   566,   573,   575,   577,   578,   582,
     584,   588,   590,   594,   598,   600,   604,   606,   608,   610,
     614,   622,   632,   639,   641,   645,   647,   651,   653,   657,
     658,   661,   663,   667,   671,   672,   674,   676,   678,   681,
     683,   685,   688,   694,   696,   700,   704,   708,   714,   719,
     724,   725,   727,   730,   732,   736,   740,   743,   750,   751,
     753,   755,   758,   761,   764,   766,   774,   776,   778,   782,
     789,   793,   797
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     107,     0,    -1,   108,    -1,   109,    -1,   109,   102,    -1,
     154,    -1,   162,    -1,   148,    -1,   149,    -1,   152,    -1,
     163,    -1,   172,    -1,   174,    -1,   175,    -1,   178,    -1,
     183,    -1,   187,    -1,   189,    -1,   190,    -1,   191,    -1,
     184,    -1,   192,    -1,   193,    -1,   110,    -1,   109,   102,
     110,    -1,   111,    -1,   145,    -1,    63,   112,    34,   116,
     117,   125,   127,   129,   133,   135,    -1,   113,    -1,   112,
     103,   113,    -1,    97,    -1,   115,   114,    -1,    -1,     3,
      -1,    12,     3,    -1,   141,    -1,    15,   104,   141,   105,
      -1,    48,   104,   141,   105,    -1,    50,   104,   141,   105,
      -1,    71,   104,   141,   105,    -1,    23,   104,    97,   105,
      -1,    82,   104,   105,    -1,    23,   104,    28,     3,   105,
      -1,     3,    -1,   116,   103,     3,    -1,    -1,   118,    -1,
      83,   119,    -1,   120,    -1,   119,    86,   120,    -1,    47,
     104,     8,   105,    -1,   121,    89,   122,    -1,   121,    90,
     122,    -1,   121,    39,   104,   124,   105,    -1,   121,   100,
      39,   104,   124,   105,    -1,   121,    39,     9,    -1,   121,
     100,    39,     9,    -1,   121,    17,   122,    86,   122,    -1,
     121,    92,   122,    -1,   121,    91,   122,    -1,   121,    93,
     122,    -1,   121,    94,   122,    -1,   121,    89,   123,    -1,
     121,    90,   123,    -1,   121,    92,   123,    -1,   121,    91,
     123,    -1,   121,    17,   123,    86,   123,    -1,   121,    93,
     123,    -1,   121,    94,   123,    -1,     3,    -1,     4,    -1,
      23,   104,    97,   105,    -1,    82,   104,   105,    -1,    38,
      -1,     5,    -1,    96,     5,    -1,     6,    -1,    96,     6,
      -1,   122,    -1,   124,   103,   122,    -1,    -1,   126,    -1,
      37,    18,   121,    -1,    -1,   128,    -1,    84,    37,    55,
      18,   131,    -1,    -1,   130,    -1,    55,    18,   131,    -1,
      55,    18,    56,   104,   105,    -1,   132,    -1,   131,   103,
     132,    -1,   121,    -1,   121,    13,    -1,   121,    26,    -1,
      -1,   134,    -1,    46,     5,    -1,    46,     5,   103,     5,
      -1,    -1,   136,    -1,    54,   137,    -1,   138,    -1,   137,
     103,   138,    -1,     3,    89,     3,    -1,     3,    89,     5,
      -1,     3,    89,   104,   139,   105,    -1,     3,    89,     3,
     104,     8,   105,    -1,     3,    89,     8,    -1,   140,    -1,
     139,   103,   140,    -1,     3,    89,   122,    -1,     3,    -1,
       4,    -1,    38,    -1,     5,    -1,     6,    -1,     9,    -1,
      96,   141,    -1,   100,   141,    -1,   141,    95,   141,    -1,
     141,    96,   141,    -1,   141,    97,   141,    -1,   141,    98,
     141,    -1,   141,    91,   141,    -1,   141,    92,   141,    -1,
     141,    88,   141,    -1,   141,    87,   141,    -1,   141,    99,
     141,    -1,   141,    29,   141,    -1,   141,    51,   141,    -1,
     141,    94,   141,    -1,   141,    93,   141,    -1,   141,    89,
     141,    -1,   141,    90,   141,    -1,   141,    86,   141,    -1,
     141,    85,   141,    -1,   104,   141,   105,    -1,   142,    -1,
       3,   104,   143,   105,    -1,    39,   104,   143,   105,    -1,
       3,   104,   105,    -1,    50,   104,   141,   103,   141,   105,
      -1,    48,   104,   141,   103,   141,   105,    -1,   144,    -1,
     143,   103,   144,    -1,   141,    -1,     8,    -1,    67,   146,
      -1,    81,    -1,    70,    -1,    49,    -1,     3,    -1,    53,
      -1,     8,    -1,     5,    -1,     6,    -1,    65,     3,    89,
     151,    -1,    65,     3,    89,   150,    -1,    65,     3,    89,
      53,    -1,    65,    52,   147,    -1,    65,    10,    89,   147,
      -1,    65,    36,     9,    89,   104,   124,   105,    -1,    65,
      36,     3,    89,   150,    -1,     3,    -1,     8,    -1,    75,
      -1,    31,    -1,   122,    -1,    21,    -1,    61,    -1,   153,
      -1,    16,    -1,    69,    74,    -1,   155,    43,     3,   156,
      79,   158,    -1,    41,    -1,    59,    -1,    -1,   104,   157,
     105,    -1,   121,    -1,   157,   103,   121,    -1,   159,    -1,
     158,   103,   159,    -1,   104,   160,   105,    -1,   161,    -1,
     160,   103,   161,    -1,   122,    -1,   123,    -1,     8,    -1,
     104,   124,   105,    -1,    25,    34,     3,    83,    38,    89,
     122,    -1,    25,    34,     3,    83,    38,    39,   104,   124,
     105,    -1,    19,     3,   104,   164,   167,   105,    -1,   165,
      -1,   164,   103,   165,    -1,   161,    -1,   104,   166,   105,
      -1,     8,    -1,   166,   103,     8,    -1,    -1,   103,   168,
      -1,   169,    -1,   168,   103,   169,    -1,   161,   170,   171,
      -1,    -1,    12,    -1,     3,    -1,    46,    -1,   173,     3,
      -1,    27,    -1,    26,    -1,    67,    72,    -1,    78,   116,
      65,   176,   118,    -1,   177,    -1,   176,   103,   177,    -1,
       3,    89,   122,    -1,     3,    89,   123,    -1,     3,    89,
     104,   124,   105,    -1,     3,    89,   104,   105,    -1,    67,
     185,    80,   179,    -1,    -1,   180,    -1,    83,   181,    -1,
     182,    -1,   181,    85,   182,    -1,     3,    89,     8,    -1,
      67,    20,    -1,    65,   185,    74,    44,    45,   186,    -1,
      -1,    36,    -1,    66,    -1,    57,    77,    -1,    57,    22,
      -1,    58,    57,    -1,    64,    -1,    24,    35,     3,    60,
     188,    68,     8,    -1,    42,    -1,    32,    -1,    30,    35,
       3,    -1,    14,    40,     3,    73,    62,     3,    -1,    33,
      62,     3,    -1,    63,    10,   133,    -1,    76,    62,     3,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   120,   120,   121,   122,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   149,   150,   154,   155,   159,   174,   175,
     179,   180,   183,   185,   186,   190,   191,   192,   193,   194,
     195,   196,   197,   201,   202,   205,   207,   211,   215,   216,
     220,   225,   232,   240,   248,   257,   262,   267,   272,   277,
     282,   287,   292,   293,   294,   295,   300,   305,   310,   318,
     319,   324,   330,   336,   345,   346,   350,   351,   355,   361,
     367,   369,   373,   380,   382,   386,   392,   394,   398,   402,
     409,   410,   414,   415,   416,   419,   421,   425,   430,   437,
     439,   443,   447,   448,   452,   457,   462,   468,   473,   481,
     486,   493,   503,   504,   505,   506,   507,   508,   509,   510,
     511,   512,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   533,
     534,   535,   536,   537,   541,   542,   546,   547,   553,   557,
     558,   559,   565,   566,   567,   568,   569,   573,   578,   583,
     588,   589,   593,   598,   606,   607,   611,   612,   613,   627,
     628,   629,   633,   634,   640,   648,   649,   652,   654,   658,
     659,   663,   664,   668,   672,   673,   677,   678,   679,   680,
     686,   692,   704,   712,   716,   723,   724,   731,   741,   747,
     749,   753,   758,   762,   769,   771,   775,   776,   782,   790,
     791,   797,   803,   811,   812,   816,   820,   824,   828,   837,
     843,   844,   848,   852,   853,   857,   861,   868,   874,   875,
     876,   880,   881,   882,   883,   889,   900,   901,   905,   916,
     928,   939,   948
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
  "TOK_GROUP", "TOK_ID", "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", 
  "TOK_INTO", "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", 
  "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_SUM", "TOK_TABLES", 
  "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", 
  "';'", "','", "'('", "')'", "$accept", "request", "statement", 
  "multi_stmt_list", "multi_stmt", "select_from", "select_items_list", 
  "select_item", "opt_alias", "select_expr", "ident_list", 
  "opt_where_clause", "where_clause", "where_expr", "where_item", 
  "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "show_stmt", "show_what", "set_value", "set_stmt", "set_global_stmt", 
  "set_string_value", "boolean_value", "transact_op", "start_transaction", 
  "insert_into", "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", "show_variables", 
  "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "set_transaction", "opt_scope", "isolation_level", 
  "create_function", "udf_type", "drop_function", "attach_index", 
  "flush_rtindex", "select_sysvar", "truncate", 0
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
     335,   336,   337,   338,   339,   340,   341,   124,    38,    61,
     342,    60,    62,   343,   344,    43,    45,    42,    47,    37,
     345,   346,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   106,   107,   107,   107,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   109,   109,   110,   110,   111,   112,   112,
     113,   113,   114,   114,   114,   115,   115,   115,   115,   115,
     115,   115,   115,   116,   116,   117,   117,   118,   119,   119,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   121,
     121,   121,   121,   121,   122,   122,   123,   123,   124,   124,
     125,   125,   126,   127,   127,   128,   129,   129,   130,   130,
     131,   131,   132,   132,   132,   133,   133,   134,   134,   135,
     135,   136,   137,   137,   138,   138,   138,   138,   138,   139,
     139,   140,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   142,
     142,   142,   142,   142,   143,   143,   144,   144,   145,   146,
     146,   146,   147,   147,   147,   147,   147,   148,   148,   148,
     148,   148,   149,   149,   150,   150,   151,   151,   151,   152,
     152,   152,   153,   153,   154,   155,   155,   156,   156,   157,
     157,   158,   158,   159,   160,   160,   161,   161,   161,   161,
     162,   162,   163,   164,   164,   165,   165,   166,   166,   167,
     167,   168,   168,   169,   170,   170,   171,   171,   172,   173,
     173,   174,   175,   176,   176,   177,   177,   177,   177,   178,
     179,   179,   180,   181,   181,   182,   183,   184,   185,   185,
     185,   186,   186,   186,   186,   187,   188,   188,   189,   190,
     191,   192,   193
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,    10,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       4,     3,     3,     5,     6,     3,     4,     5,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     3,     3,     1,
       1,     4,     3,     1,     1,     2,     1,     2,     1,     3,
       0,     1,     3,     0,     1,     5,     0,     1,     3,     5,
       1,     3,     1,     2,     2,     0,     1,     2,     4,     0,
       1,     2,     1,     3,     3,     3,     5,     6,     3,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     4,
       4,     3,     6,     6,     1,     3,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     4,     4,
       3,     4,     7,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     6,     1,     1,     0,     3,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     1,     3,
       7,     9,     6,     1,     3,     1,     3,     1,     3,     0,
       2,     1,     3,     3,     0,     1,     1,     1,     2,     1,
       1,     2,     5,     1,     3,     3,     3,     5,     4,     4,
       0,     1,     2,     1,     3,     3,     2,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     3,     6,
       3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   172,     0,   169,     0,     0,   210,   209,     0,
       0,   175,   176,   170,     0,   228,   228,     0,     0,     0,
       0,     2,     3,    23,    25,    26,     7,     8,     9,   171,
       5,     0,     6,    10,    11,     0,    12,    13,    14,    15,
      20,    16,    17,    18,    19,    21,    22,     0,     0,     0,
       0,     0,     0,   112,   113,   115,   116,   117,    95,     0,
       0,   114,     0,     0,     0,     0,     0,     0,    30,     0,
       0,     0,    28,    32,    35,   138,     0,     0,   229,     0,
     230,     0,   226,   229,   151,   150,   211,   149,   148,     0,
     173,     0,    43,     0,     1,     4,     0,   208,     0,     0,
       0,     0,   238,   240,     0,     0,   241,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   118,   119,     0,
       0,     0,    33,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   152,   155,   156,   154,
     153,   160,     0,   220,   242,     0,     0,     0,     0,    24,
     177,     0,    74,    76,   188,     0,     0,   186,   187,   195,
     199,   193,     0,     0,   147,   141,   146,     0,   144,    97,
       0,     0,     0,     0,     0,     0,     0,    41,     0,     0,
     137,    45,    29,    34,   129,   130,   136,   135,   127,   126,
     133,   134,   124,   125,   132,   131,   120,   121,   122,   123,
     128,   164,   165,   167,   159,   166,     0,   168,   158,   157,
     161,     0,     0,     0,     0,   219,   221,     0,     0,   213,
      44,     0,     0,     0,    75,    77,   197,    78,     0,     0,
       0,     0,   237,   236,     0,     0,     0,   139,     0,    36,
       0,    40,   140,     0,    37,     0,    38,    39,     0,     0,
       0,    80,    46,   163,     0,     0,     0,   222,   223,     0,
       0,   212,    69,    70,     0,    73,     0,   179,     0,     0,
     239,     0,   189,     0,   196,   195,   194,   200,   201,   192,
       0,     0,     0,   145,    98,    42,     0,     0,     0,    47,
      48,     0,     0,    83,    81,     0,     0,     0,   234,   227,
       0,     0,     0,   215,   216,   214,     0,     0,     0,   178,
       0,   174,   181,    79,   198,   205,     0,     0,   235,     0,
     190,   143,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    86,    84,   162,   232,
     231,   233,   225,   224,   218,     0,     0,    72,   180,     0,
       0,   184,     0,   206,   207,   203,   204,   202,     0,     0,
      49,     0,     0,    55,     0,    51,    62,    52,    63,    59,
      65,    58,    64,    60,    67,    61,    68,     0,    82,     0,
       0,    95,    87,   217,    71,     0,   183,   182,   191,    50,
       0,     0,     0,    56,     0,     0,     0,    99,   185,    57,
       0,    66,    53,     0,     0,     0,    92,    88,    90,     0,
      27,   100,    54,    85,     0,    93,    94,     0,     0,   101,
     102,    89,    91,     0,     0,   104,   105,   108,     0,   103,
       0,     0,     0,   109,     0,     0,     0,   106,   107,   111,
     110
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    20,    21,    22,    23,    24,    71,    72,   124,    73,
      93,   261,   262,   299,   300,   416,   237,   168,   238,   303,
     304,   346,   347,   391,   392,   417,   418,   106,   107,   420,
     421,   429,   430,   442,   443,    74,    75,   177,   178,    25,
      88,   151,    26,    27,   218,   219,    28,    29,    30,    31,
     232,   278,   321,   322,   360,   169,    32,    33,   170,   171,
     239,   241,   287,   288,   326,   365,    34,    35,    36,    37,
     228,   229,    38,   225,   226,   267,   268,    39,    40,    81,
     309,    41,   244,    42,    43,    44,    45,    46
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -255
static const short yypact[] =
{
     693,   -14,  -255,    53,  -255,    43,    82,  -255,  -255,    80,
      65,  -255,  -255,  -255,   207,   302,   672,    69,    77,   143,
     152,  -255,    52,  -255,  -255,  -255,  -255,  -255,  -255,  -255,
    -255,   140,  -255,  -255,  -255,   164,  -255,  -255,  -255,  -255,
    -255,  -255,  -255,  -255,  -255,  -255,  -255,   187,   110,   216,
     231,   232,   234,   147,  -255,  -255,  -255,  -255,   198,   154,
     157,  -255,   176,   177,   180,   196,   202,   170,  -255,   170,
     170,    16,  -255,    29,   509,  -255,   209,   213,    49,   145,
    -255,   235,  -255,  -255,  -255,  -255,  -255,  -255,  -255,   230,
    -255,   322,  -255,   -55,  -255,   -10,   324,  -255,   255,    13,
     270,   246,  -255,  -255,    92,   326,  -255,  -255,   170,    27,
     156,   170,   170,   170,   227,   229,   233,  -255,  -255,   336,
     143,   244,  -255,   331,  -255,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   260,   145,   250,   254,  -255,  -255,  -255,  -255,
    -255,  -255,   301,   264,  -255,   346,   347,   244,    96,  -255,
     247,   290,  -255,  -255,  -255,    34,     9,  -255,  -255,  -255,
     252,  -255,   149,   315,  -255,  -255,   509,    81,  -255,   263,
     362,   354,   262,   102,   285,   313,   385,  -255,   170,   170,
    -255,   -70,  -255,  -255,  -255,  -255,   532,   558,   581,   607,
     225,   225,   104,   104,   104,   104,   134,   134,  -255,  -255,
    -255,  -255,  -255,  -255,  -255,  -255,   380,  -255,  -255,  -255,
    -255,    46,   282,   344,   389,  -255,  -255,   304,   -52,  -255,
    -255,   133,   316,   391,  -255,  -255,  -255,  -255,   120,   121,
      13,   291,  -255,  -255,   329,   -12,   156,  -255,   410,  -255,
     312,  -255,  -255,   170,  -255,   170,  -255,  -255,   460,   483,
      87,   382,  -255,  -255,    15,   215,   348,   335,  -255,    24,
     346,  -255,  -255,  -255,   334,  -255,   338,  -255,   124,   339,
    -255,    15,  -255,   431,  -255,   132,  -255,   341,  -255,  -255,
     437,   342,    15,  -255,  -255,  -255,   411,   434,   360,   379,
    -255,   269,   448,   384,  -255,   159,    72,   412,  -255,  -255,
     478,   389,     7,  -255,  -255,  -255,   390,   383,   133,  -255,
      17,   388,  -255,  -255,  -255,  -255,   126,    17,  -255,    15,
    -255,  -255,  -255,   484,    87,    30,    -2,    30,    30,    30,
      30,    30,    30,   454,   133,   457,   440,  -255,  -255,  -255,
    -255,  -255,  -255,  -255,  -255,   166,   408,  -255,  -255,    15,
     172,  -255,   339,  -255,  -255,  -255,   502,  -255,   182,   413,
    -255,   429,   449,  -255,    15,  -255,  -255,  -255,  -255,  -255,
    -255,  -255,  -255,  -255,  -255,  -255,  -255,     0,  -255,   462,
     518,   198,  -255,  -255,  -255,    17,  -255,  -255,  -255,  -255,
      15,    10,   185,  -255,    15,   519,    76,   486,  -255,  -255,
     535,  -255,  -255,   190,   133,   438,    11,   441,  -255,   540,
    -255,  -255,  -255,   441,   459,  -255,  -255,   133,   473,   463,
    -255,  -255,  -255,     3,   540,   461,  -255,  -255,   564,  -255,
     576,   496,   194,  -255,   485,    15,   564,  -255,  -255,  -255,
    -255
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -255,  -255,  -255,  -255,   493,  -255,  -255,   468,  -255,  -255,
     471,  -255,   364,  -255,   259,  -226,   -99,  -254,  -236,  -255,
    -255,  -255,  -255,  -255,  -255,   197,   186,   221,  -255,  -255,
    -255,  -255,   181,  -255,   168,   -66,  -255,   506,   371,  -255,
    -255,   490,  -255,  -255,   414,  -255,  -255,  -255,  -255,  -255,
    -255,  -255,  -255,   272,  -255,  -238,  -255,  -255,  -255,   397,
    -255,  -255,  -255,   311,  -255,  -255,  -255,  -255,  -255,  -255,
    -255,   369,  -255,  -255,  -255,  -255,   330,  -255,  -255,   624,
    -255,  -255,  -255,  -255,  -255,  -255,  -255,  -255
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -205
static const short yytable[] =
{
     167,   117,   285,   118,   119,   277,   435,   373,   436,   403,
     155,   437,   162,   260,   162,   314,   163,   236,   162,   163,
     162,   164,   162,   163,   425,   164,    47,   291,   305,   162,
     163,   260,   122,   156,   301,   162,   163,   426,   176,   234,
     235,   123,   180,   217,   176,   184,   185,   186,   156,   211,
     120,   270,   144,   157,   212,   181,    48,   158,   145,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   355,   292,    49,   272,
     273,   372,   361,   376,   378,   380,   382,   384,   386,   366,
     272,   273,   358,   368,   349,    53,    54,    55,    56,   274,
     174,    57,   374,   216,   404,   216,   410,   438,   301,   165,
     274,   216,   354,   165,   275,    51,    50,   166,   388,   121,
     165,   359,   258,   259,   182,   275,   165,    52,   312,   363,
      61,    62,   415,   125,   298,  -204,   272,   273,   402,    91,
     115,   167,   116,    90,   325,    84,    92,   411,   146,   350,
     147,   148,    94,   149,    95,   126,   274,   408,   276,    53,
      54,    55,    56,   125,   174,    57,    85,    97,   413,   276,
     313,   275,   364,    53,    54,    55,    56,    87,  -204,    57,
     176,   242,   323,    96,   246,   126,   247,   296,    67,   297,
      98,   243,    69,   330,    61,    62,    70,   175,   150,   137,
     138,   139,   140,   141,   115,   246,   116,   252,    61,    62,
      53,    54,    55,    56,    99,   276,    57,    58,   115,   100,
     116,   167,    59,   281,   283,   282,   284,   318,   167,   319,
      60,   139,   140,   141,   101,   102,   371,   103,   375,   377,
     379,   381,   383,   385,   105,    61,    62,    53,    54,    55,
      56,   104,    67,    57,   125,    63,    69,    64,   108,    59,
      70,   109,   281,   211,   348,   162,    67,    60,   212,   281,
      69,   393,   306,   307,    70,   395,   126,   396,    65,   308,
     110,   111,    61,    62,   112,   281,   335,   398,   281,    66,
     412,   213,    63,   281,    64,   422,   167,   446,   142,   447,
     113,   409,   143,    67,    68,    76,   114,    69,   336,   152,
     153,    70,    77,   214,   125,    65,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   154,    66,   160,   161,   173,
     172,   179,   187,   188,   193,   215,   126,   189,    78,   221,
      67,    68,   125,   222,    69,   223,   449,   224,    70,   227,
     230,   231,   233,   245,    79,   240,   216,   250,   337,   338,
     339,   340,   341,   342,   126,   125,   248,   251,    80,   343,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   234,   264,   126,   253,   265,
     254,   125,   266,   269,   280,   279,   289,   290,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   126,   125,   294,   255,   295,   256,   302,
     311,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   126,   310,   316,   324,
     125,   190,   317,   320,   327,   328,   329,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   126,   125,   333,   334,   344,   249,   345,   351,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   126,   352,   356,   357,   125,
     257,   362,   369,   387,   389,   390,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   126,   125,   394,   325,   400,   331,   405,   399,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   126,   401,   406,   414,   125,   332,
     419,   235,   424,   428,   427,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     126,   125,   433,   253,   431,   440,   434,   441,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   126,   444,   445,   255,   125,   159,   192,
     448,   191,   271,   370,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   126,
     125,   423,   407,   432,   450,   439,   183,   293,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   126,   220,   397,   263,   125,   286,   367,   315,
      89,   353,     0,     0,     0,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   126,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,     1,    83,     2,
       0,     0,     3,     0,     4,     0,     0,     5,     6,     7,
       8,    84,     0,     9,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,    11,     0,     0,     0,    80,     0,
       0,     0,    85,     0,    86,     0,     0,     0,     0,     0,
       0,     0,    12,    87,    13,     0,    14,     0,    15,     0,
      16,     0,    17,     0,     0,     0,     0,     0,     0,    18,
       0,    19
};

static const short yycheck[] =
{
      99,    67,   240,    69,    70,   231,     3,     9,     5,     9,
      65,     8,     5,    83,     5,   269,     6,     8,     5,     6,
       5,     8,     5,     6,    13,     8,    40,    39,   264,     5,
       6,    83,     3,   103,   260,     5,     6,    26,   104,     5,
       6,    12,   108,   142,   110,   111,   112,   113,   103,     3,
      34,   103,     3,    63,     8,    28,     3,    67,     9,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   312,    89,    35,     3,
       4,   335,   320,   337,   338,   339,   340,   341,   342,   327,
       3,     4,   318,   329,    22,     3,     4,     5,     6,    23,
       8,     9,   104,    96,   104,    96,    96,   104,   334,    96,
      23,    96,   105,    96,    38,    35,    34,   104,   344,   103,
      96,   104,   188,   189,    97,    38,    96,    62,   104,     3,
      38,    39,    56,    29,    47,     3,     3,     4,   374,    62,
      48,   240,    50,    74,    12,    49,     3,   401,     3,    77,
       5,     6,     0,     8,   102,    51,    23,   395,    82,     3,
       4,     5,     6,    29,     8,     9,    70,     3,   404,    82,
     269,    38,    46,     3,     4,     5,     6,    81,    46,     9,
     246,    32,   281,    43,   103,    51,   105,   253,    96,   255,
       3,    42,   100,   292,    38,    39,   104,   105,    53,    95,
      96,    97,    98,    99,    48,   103,    50,   105,    38,    39,
       3,     4,     5,     6,   104,    82,     9,    10,    48,     3,
      50,   320,    15,   103,   103,   105,   105,   103,   327,   105,
      23,    97,    98,    99,     3,     3,   335,     3,   337,   338,
     339,   340,   341,   342,    46,    38,    39,     3,     4,     5,
       6,   104,    96,     9,    29,    48,   100,    50,   104,    15,
     104,   104,   103,     3,   105,     5,    96,    23,     8,   103,
     100,   105,    57,    58,   104,   103,    51,   105,    71,    64,
     104,   104,    38,    39,   104,   103,    17,   105,   103,    82,
     105,    31,    48,   103,    50,   105,   395,   103,    89,   105,
     104,   400,    89,    96,    97,     3,   104,   100,    39,    74,
      80,   104,    10,    53,    29,    71,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     3,    82,     3,    73,    83,
      60,     5,   105,   104,     3,    75,    51,   104,    36,    89,
      96,    97,    29,    89,   100,    44,   445,    83,   104,     3,
       3,   104,    62,    38,    52,   103,    96,     3,    89,    90,
      91,    92,    93,    94,    51,    29,   103,   105,    66,   100,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     5,   104,    51,   103,    45,
     105,    29,     3,    89,     3,    79,   105,    68,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,    51,    29,     5,   103,   105,   105,    37,
      85,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,    51,    89,   104,     8,
      29,   105,   104,   104,   103,     8,   104,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    51,    29,   104,    86,    18,   105,    84,    57,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,    51,     8,    97,   105,    29,
     105,   103,     8,    39,    37,    55,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,    51,    29,   105,    12,    86,   105,    55,   105,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,    51,    86,    18,    18,    29,   105,
      54,     6,   104,     3,   103,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      51,    29,    89,   103,   105,   104,   103,     3,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,    51,     8,    89,   103,    29,    95,   121,
     105,   120,   228,   334,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,    51,
      29,   414,   391,   427,   446,   434,   110,   246,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    51,   143,   362,   221,    29,   240,   327,   270,
      16,   311,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,    14,    36,    16,
      -1,    -1,    19,    -1,    21,    -1,    -1,    24,    25,    26,
      27,    49,    -1,    30,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    70,    -1,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    81,    61,    -1,    63,    -1,    65,    -1,
      67,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      -1,    78
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    16,    19,    21,    24,    25,    26,    27,    30,
      33,    41,    59,    61,    63,    65,    67,    69,    76,    78,
     107,   108,   109,   110,   111,   145,   148,   149,   152,   153,
     154,   155,   162,   163,   172,   173,   174,   175,   178,   183,
     184,   187,   189,   190,   191,   192,   193,    40,     3,    35,
      34,    35,    62,     3,     4,     5,     6,     9,    10,    15,
      23,    38,    39,    48,    50,    71,    82,    96,    97,   100,
     104,   112,   113,   115,   141,   142,     3,    10,    36,    52,
      66,   185,    20,    36,    49,    70,    72,    81,   146,   185,
      74,    62,     3,   116,     0,   102,    43,     3,     3,   104,
       3,     3,     3,     3,   104,    46,   133,   134,   104,   104,
     104,   104,   104,   104,   104,    48,    50,   141,   141,   141,
      34,   103,     3,    12,   114,    29,    51,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    89,    89,     3,     9,     3,     5,     6,     8,
      53,   147,    74,    80,     3,    65,   103,    63,    67,   110,
       3,    73,     5,     6,     8,    96,   104,   122,   123,   161,
     164,   165,    60,    83,     8,   105,   141,   143,   144,     5,
     141,    28,    97,   143,   141,   141,   141,   105,   104,   104,
     105,   116,   113,     3,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141,     3,     8,    31,    53,    75,    96,   122,   150,   151,
     147,    89,    89,    44,    83,   179,   180,     3,   176,   177,
       3,   104,   156,    62,     5,     6,     8,   122,   124,   166,
     103,   167,    32,    42,   188,    38,   103,   105,   103,   105,
       3,   105,   105,   103,   105,   103,   105,   105,   141,   141,
      83,   117,   118,   150,   104,    45,     3,   181,   182,    89,
     103,   118,     3,     4,    23,    38,    82,   121,   157,    79,
       3,   103,   105,   103,   105,   161,   165,   168,   169,   105,
      68,    39,    89,   144,     5,   105,   141,   141,    47,   119,
     120,   121,    37,   125,   126,   124,    57,    58,    64,   186,
      89,    85,   104,   122,   123,   177,   104,   104,   103,   105,
     104,   158,   159,   122,     8,    12,   170,   103,     8,   104,
     122,   105,   105,   104,    86,    17,    39,    89,    90,    91,
      92,    93,    94,   100,    18,    84,   127,   128,   105,    22,
      77,    57,     8,   182,   105,   124,    97,   105,   121,   104,
     160,   161,   103,     3,    46,   171,   161,   169,   124,     8,
     120,   122,   123,     9,   104,   122,   123,   122,   123,   122,
     123,   122,   123,   122,   123,   122,   123,    39,   121,    37,
      55,   129,   130,   105,   105,   103,   105,   159,   105,   105,
      86,    86,   124,     9,   104,    55,    18,   133,   161,   122,
      96,   123,   105,   124,    18,    56,   121,   131,   132,    54,
     135,   136,   105,   131,   104,    13,    26,   103,     3,   137,
     138,   105,   132,    89,   103,     3,     5,     8,   104,   138,
     104,     3,   139,   140,     8,    89,   103,   105,   105,   122,
     140
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

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 41:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 42:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 44:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 50:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 51:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 52:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 53:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 54:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 55:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 56:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 57:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 58:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 59:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 60:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 65:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 66:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 67:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 71:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 72:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 73:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 74:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 75:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 76:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 77:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 78:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 79:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 82:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 85:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 88:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 89:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 91:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 93:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 94:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 97:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 98:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 104:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 105:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 106:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 107:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 110:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 111:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 113:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 114:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 118:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 119:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 139:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 140:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 141:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 142:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 143:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 149:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 150:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 151:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 157:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 158:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 159:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 160:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 161:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 162:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 163:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 166:

    { yyval.m_iValue = 1; ;}
    break;

  case 167:

    { yyval.m_iValue = 0; ;}
    break;

  case 168:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 169:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 174:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 175:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 176:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 179:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 180:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 183:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 184:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 185:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 186:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 187:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 188:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 189:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_iValue = yyvsp[-1].m_pValues->GetLength(); yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 190:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 191:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 192:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 193:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 194:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 196:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 197:

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

  case 198:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 201:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 203:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 207:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 208:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 211:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 212:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 215:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 216:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 217:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 218:

    {
			pParser->UpdateAttr ( yyvsp[-3].m_sValue, NULL, SPH_ATTR_UINT32SET );
		;}
    break;

  case 219:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 226:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 227:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 235:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 236:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 237:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 238:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 239:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 240:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 241:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 242:

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

