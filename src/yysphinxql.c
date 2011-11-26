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
#define YYLAST   780

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  84
/* YYNRULES -- Number of rules. */
#define YYNRULES  236
/* YYNRULES -- Number of states. */
#define YYNSTATES  441

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
     723,   726,   733,   734,   736,   738,   741,   744,   747,   749,
     757,   759,   761,   765,   772,   776,   780
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     107,     0,    -1,   108,    -1,   109,    -1,   109,   102,    -1,
     154,    -1,   162,    -1,   148,    -1,   149,    -1,   152,    -1,
     163,    -1,   172,    -1,   174,    -1,   175,    -1,   178,    -1,
     179,    -1,   183,    -1,   185,    -1,   186,    -1,   187,    -1,
     180,    -1,   188,    -1,   189,    -1,   110,    -1,   109,   102,
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
     181,    80,    -1,    67,    20,    -1,    65,   181,    74,    44,
      45,   182,    -1,    -1,    36,    -1,    66,    -1,    57,    77,
      -1,    57,    22,    -1,    58,    57,    -1,    64,    -1,    24,
      35,     3,    60,   184,    68,     8,    -1,    42,    -1,    32,
      -1,    30,    35,     3,    -1,    14,    40,     3,    73,    62,
       3,    -1,    33,    62,     3,    -1,    63,    10,   133,    -1,
      76,    62,     3,    -1
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
     844,   851,   857,   858,   859,   863,   864,   865,   866,   872,
     883,   884,   888,   899,   911,   922,   931
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
  "show_stmt", "show_variable", "set_value", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "update", "update_items_list", 
  "update_item", "show_variables", "show_collation", "set_transaction", 
  "opt_scope", "isolation_level", "create_function", "udf_type", 
  "drop_function", "attach_index", "flush_rtindex", "select_sysvar", 
  "truncate", 0
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
     179,   180,   181,   181,   181,   182,   182,   182,   182,   183,
     184,   184,   185,   186,   187,   188,   189
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
       1,     2,     5,     1,     3,     3,     3,     5,     4,     3,
       2,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     3,     6,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   172,     0,   169,     0,     0,   210,   209,     0,
       0,   175,   176,   170,     0,   222,   222,     0,     0,     0,
       0,     2,     3,    23,    25,    26,     7,     8,     9,   171,
       5,     0,     6,    10,    11,     0,    12,    13,    14,    15,
      20,    16,    17,    18,    19,    21,    22,     0,     0,     0,
       0,     0,     0,   112,   113,   115,   116,   117,    95,     0,
       0,   114,     0,     0,     0,     0,     0,     0,    30,     0,
       0,     0,    28,    32,    35,   138,     0,     0,   223,     0,
     224,     0,   220,   223,   151,   150,   211,   149,   148,     0,
     173,     0,    43,     0,     1,     4,     0,   208,     0,     0,
       0,     0,   232,   234,     0,     0,   235,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   118,   119,     0,
       0,     0,    33,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   152,   155,   156,   154,
     153,   160,     0,   219,   236,     0,     0,     0,     0,    24,
     177,     0,    74,    76,   188,     0,     0,   186,   187,   195,
     199,   193,     0,     0,   147,   141,   146,     0,   144,    97,
       0,     0,     0,     0,     0,     0,     0,    41,     0,     0,
     137,    45,    29,    34,   129,   130,   136,   135,   127,   126,
     133,   134,   124,   125,   132,   131,   120,   121,   122,   123,
     128,   164,   165,   167,   159,   166,     0,   168,   158,   157,
     161,     0,     0,     0,     0,     0,   213,    44,     0,     0,
       0,    75,    77,   197,    78,     0,     0,     0,     0,   231,
     230,     0,     0,     0,   139,     0,    36,     0,    40,   140,
       0,    37,     0,    38,    39,     0,     0,     0,    80,    46,
     163,     0,     0,     0,     0,   212,    69,    70,     0,    73,
       0,   179,     0,     0,   233,     0,   189,     0,   196,   195,
     194,   200,   201,   192,     0,     0,     0,   145,    98,    42,
       0,     0,     0,    47,    48,     0,     0,    83,    81,     0,
       0,     0,   228,   221,     0,   215,   216,   214,     0,     0,
       0,   178,     0,   174,   181,    79,   198,   205,     0,     0,
     229,     0,   190,   143,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,    84,
     162,   226,   225,   227,   218,     0,     0,    72,   180,     0,
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
      93,   258,   259,   293,   294,   406,   234,   168,   235,   297,
     298,   338,   339,   381,   382,   407,   408,   106,   107,   410,
     411,   419,   420,   432,   433,    74,    75,   177,   178,    25,
      88,   151,    26,    27,   218,   219,    28,    29,    30,    31,
     229,   272,   313,   314,   350,   169,    32,    33,   170,   171,
     236,   238,   281,   282,   318,   355,    34,    35,    36,    37,
     225,   226,    38,    39,    40,    81,   303,    41,   241,    42,
      43,    44,    45,    46
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -245
static const short yypact[] =
{
     702,    -3,  -245,    51,  -245,    22,    47,  -245,  -245,    98,
      79,  -245,  -245,  -245,   199,   149,   670,    70,   101,   157,
     174,  -245,    73,  -245,  -245,  -245,  -245,  -245,  -245,  -245,
    -245,   126,  -245,  -245,  -245,   175,  -245,  -245,  -245,  -245,
    -245,  -245,  -245,  -245,  -245,  -245,  -245,   176,    95,   179,
     197,   203,   204,   106,  -245,  -245,  -245,  -245,   165,   117,
     121,  -245,   125,   132,   140,   141,   147,   123,  -245,   123,
     123,   -21,  -245,   136,   492,  -245,   137,   191,   134,   299,
    -245,   194,  -245,  -245,  -245,  -245,  -245,  -245,  -245,   213,
    -245,   280,  -245,    -9,  -245,    54,   281,  -245,   221,     9,
     246,   227,  -245,  -245,    92,   309,  -245,  -245,   123,    -6,
     250,   123,   123,   123,   211,   214,   216,  -245,  -245,   322,
     157,   237,  -245,   314,  -245,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   186,   299,   235,   239,  -245,  -245,  -245,  -245,
    -245,  -245,   285,  -245,  -245,   327,   328,   237,    86,  -245,
     231,   270,  -245,  -245,  -245,   148,    19,  -245,  -245,  -245,
     236,  -245,     8,   300,  -245,  -245,   492,    46,  -245,   240,
     343,   337,   242,    52,   272,   293,   371,  -245,   123,   123,
    -245,   -52,  -245,  -245,  -245,  -245,   518,   541,   567,   590,
     616,   616,   573,   573,   573,   573,   228,   228,  -245,  -245,
    -245,  -245,  -245,  -245,  -245,  -245,   340,  -245,  -245,  -245,
    -245,    50,   238,   303,   260,   -51,  -245,  -245,    76,   276,
     350,  -245,  -245,  -245,  -245,    67,    78,     9,   269,  -245,
    -245,   308,   -11,   250,  -245,   388,  -245,   290,  -245,  -245,
     123,  -245,   123,  -245,  -245,   443,   469,   274,   360,  -245,
    -245,    13,   -23,    20,   327,  -245,  -245,  -245,   295,  -245,
     297,  -245,    90,   298,  -245,    13,  -245,   395,  -245,   122,
    -245,   301,  -245,  -245,   397,   302,    13,  -245,  -245,  -245,
     394,   420,   320,   339,  -245,   173,   408,   359,  -245,   113,
      33,   387,  -245,  -245,     7,  -245,  -245,  -245,   349,   342,
      76,  -245,    15,   347,  -245,  -245,  -245,  -245,     2,    15,
    -245,    13,  -245,  -245,  -245,   444,   274,    24,    -2,    24,
      24,    24,    24,    24,    24,   412,    76,   416,   399,  -245,
    -245,  -245,  -245,  -245,  -245,   145,   368,  -245,  -245,    13,
     166,  -245,   298,  -245,  -245,  -245,   462,  -245,   169,   370,
    -245,   369,   391,  -245,    13,  -245,  -245,  -245,  -245,  -245,
    -245,  -245,  -245,  -245,  -245,  -245,  -245,     0,  -245,   423,
     477,   165,  -245,  -245,  -245,    15,  -245,  -245,  -245,  -245,
      13,    10,   187,  -245,    13,   478,   142,   446,  -245,  -245,
     491,  -245,  -245,   206,    76,   398,    23,   400,  -245,   498,
    -245,  -245,  -245,   400,   417,  -245,  -245,    76,   415,   421,
    -245,  -245,  -245,     3,   498,   419,  -245,  -245,   523,  -245,
     519,   455,   210,  -245,   440,    13,   523,  -245,  -245,  -245,
    -245
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -245,  -245,  -245,  -245,   453,  -245,  -245,   428,  -245,  -245,
     430,  -245,   326,  -245,   226,  -218,   -99,  -244,  -228,  -245,
    -245,  -245,  -245,  -245,  -245,   167,   156,   172,  -245,  -245,
    -245,  -245,   150,  -245,   139,   -66,  -245,   466,   351,  -245,
    -245,   450,  -245,  -245,   374,  -245,  -245,  -245,  -245,  -245,
    -245,  -245,  -245,   245,  -245,  -235,  -245,  -245,  -245,   361,
    -245,  -245,  -245,   282,  -245,  -245,  -245,  -245,  -245,  -245,
    -245,   335,  -245,  -245,  -245,   584,  -245,  -245,  -245,  -245,
    -245,  -245,  -245,  -245
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -205
static const short yytable[] =
{
     167,   117,   279,   118,   119,   353,   425,   363,   426,   393,
     271,   427,   162,   120,   162,   163,   163,   164,   162,   306,
     162,   163,   181,   164,   162,   162,   163,   233,   285,   162,
     163,   257,   257,   299,   300,   301,   415,    47,   176,   295,
     239,   302,   180,   217,   176,   184,   185,   186,   354,   416,
     240,   156,   264,   211,    48,   341,   155,    49,   212,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   345,   351,   286,   266,
     267,    50,   121,   362,   356,   366,   368,   370,   372,   374,
     376,   182,   348,   358,   156,    53,    54,    55,    56,   268,
     174,    57,   364,   216,   394,   165,   400,   428,   295,   216,
     342,   165,   344,   166,   269,   216,   165,   157,   378,   349,
     165,   158,   255,   256,   304,  -204,    53,    54,    55,    56,
      61,    62,    57,    51,   317,    84,   392,   144,   167,   122,
     115,    52,   116,   145,    90,   266,   267,   401,   123,   243,
     398,   244,    76,   231,   232,   243,    85,   249,   270,    77,
      92,    61,    62,    91,   305,   268,   403,    87,  -204,    96,
     275,   115,   276,   116,    94,    95,   315,   176,    97,    98,
     269,   277,   100,   278,   290,    78,   291,   322,    67,   211,
     327,   162,    69,   310,   212,   311,    70,   175,   405,    99,
     101,    79,    53,    54,    55,    56,   102,   103,    57,    58,
     104,   105,   328,   167,    59,    80,   275,   213,   340,    67,
     167,   108,    60,    69,   270,   109,   142,    70,   361,   110,
     365,   367,   369,   371,   373,   375,   111,    61,    62,   214,
      53,    54,    55,    56,   112,   113,    57,    63,   275,    64,
     383,   114,    59,    53,    54,    55,    56,   125,   174,    57,
      60,   215,   329,   330,   331,   332,   333,   334,   152,   385,
      65,   386,   275,   335,   388,    61,    62,   266,   267,   126,
     143,    66,   216,   154,   160,    63,   167,    64,    61,    62,
     275,   399,   402,   153,   161,    67,    68,   268,   115,    69,
     116,   125,   146,    70,   147,   148,   172,   149,    65,   275,
     173,   412,   269,   436,   179,   437,   187,   193,   188,    66,
     189,   292,   125,   126,   221,   139,   140,   141,   222,   223,
     224,   227,   230,    67,    68,   228,   439,    69,   242,   237,
     247,    70,   261,   245,   126,   231,    67,   248,   262,   263,
      69,   125,   150,   274,    70,   273,   270,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   125,   126,   283,   250,   284,   251,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   288,   126,   289,   252,   296,   253,   308,
     125,   309,   312,   316,   319,   320,   321,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   126,   125,   325,   326,   336,   190,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   337,   343,   126,   346,   347,   246,   125,
     352,   377,   359,   379,   380,   390,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   126,   125,   384,   317,   389,   254,   391,   395,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   126,   396,   404,   232,   125,   323,
     409,   418,   414,   417,   423,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     126,   125,   421,   430,   424,   324,   431,   434,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   126,   435,   438,   250,   125,   159,   192,
     191,   265,   360,   397,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   126,
     125,   413,   252,   422,   429,   440,   183,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   126,   220,   287,   260,   125,   387,   280,   307,
      89,   357,   125,     0,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   126,   125,
       0,     0,     0,     0,   126,     0,     0,     0,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   126,     0,     0,     0,   125,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   126,   137,   138,
     139,   140,   141,     0,     0,     0,     0,     0,     0,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,   133,   134,   135,
     136,   137,   138,   139,   140,   141,     1,     0,     2,    84,
       0,     3,     0,     4,     0,     0,     5,     6,     7,     8,
       0,     0,     9,     0,     0,    10,    80,     0,     0,     0,
      85,     0,    86,    11,     0,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,    13,     0,    14,     0,    15,     0,    16,
       0,    17,     0,     0,     0,     0,     0,     0,    18,     0,
      19
};

static const short yycheck[] =
{
      99,    67,   237,    69,    70,     3,     3,     9,     5,     9,
     228,     8,     5,    34,     5,     6,     6,     8,     5,   263,
       5,     6,    28,     8,     5,     5,     6,     8,    39,     5,
       6,    83,    83,   261,    57,    58,    13,    40,   104,   257,
      32,    64,   108,   142,   110,   111,   112,   113,    46,    26,
      42,   103,   103,     3,     3,    22,    65,    35,     8,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   304,   312,    89,     3,
       4,    34,   103,   327,   319,   329,   330,   331,   332,   333,
     334,    97,   310,   321,   103,     3,     4,     5,     6,    23,
       8,     9,   104,    96,   104,    96,    96,   104,   326,    96,
      77,    96,   105,   104,    38,    96,    96,    63,   336,   104,
      96,    67,   188,   189,   104,     3,     3,     4,     5,     6,
      38,    39,     9,    35,    12,    49,   364,     3,   237,     3,
      48,    62,    50,     9,    74,     3,     4,   391,    12,   103,
     385,   105,     3,     5,     6,   103,    70,   105,    82,    10,
       3,    38,    39,    62,   263,    23,   394,    81,    46,    43,
     103,    48,   105,    50,     0,   102,   275,   243,     3,     3,
      38,   103,     3,   105,   250,    36,   252,   286,    96,     3,
      17,     5,   100,   103,     8,   105,   104,   105,    56,   104,
       3,    52,     3,     4,     5,     6,     3,     3,     9,    10,
     104,    46,    39,   312,    15,    66,   103,    31,   105,    96,
     319,   104,    23,   100,    82,   104,    89,   104,   327,   104,
     329,   330,   331,   332,   333,   334,   104,    38,    39,    53,
       3,     4,     5,     6,   104,   104,     9,    48,   103,    50,
     105,   104,    15,     3,     4,     5,     6,    29,     8,     9,
      23,    75,    89,    90,    91,    92,    93,    94,    74,   103,
      71,   105,   103,   100,   105,    38,    39,     3,     4,    51,
      89,    82,    96,     3,     3,    48,   385,    50,    38,    39,
     103,   390,   105,    80,    73,    96,    97,    23,    48,   100,
      50,    29,     3,   104,     5,     6,    60,     8,    71,   103,
      83,   105,    38,   103,     5,   105,   105,     3,   104,    82,
     104,    47,    29,    51,    89,    97,    98,    99,    89,    44,
       3,     3,    62,    96,    97,   104,   435,   100,    38,   103,
       3,   104,   104,   103,    51,     5,    96,   105,    45,    89,
     100,    29,    53,     3,   104,    79,    82,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    29,    51,   105,   103,    68,   105,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,     5,    51,   105,   103,    37,   105,   104,
      29,   104,   104,     8,   103,     8,   104,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    51,    29,   104,    86,    18,   105,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,    84,    57,    51,    97,   105,   105,    29,
     103,    39,     8,    37,    55,    86,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,    51,    29,   105,    12,   105,   105,    86,    55,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,    51,    18,    18,     6,    29,   105,
      54,     3,   104,   103,    89,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      51,    29,   105,   104,   103,   105,     3,     8,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,    51,    89,   105,   103,    29,    95,   121,
     120,   225,   326,   381,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,    51,
      29,   404,   103,   417,   424,   436,   110,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    51,   143,   243,   221,    29,   352,   237,   264,
      16,   319,    29,    -1,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,    51,    29,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,    51,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,    51,    95,    96,
      97,    98,    99,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    91,    92,    93,
      94,    95,    96,    97,    98,    99,    14,    -1,    16,    49,
      -1,    19,    -1,    21,    -1,    -1,    24,    25,    26,    27,
      -1,    -1,    30,    -1,    -1,    33,    66,    -1,    -1,    -1,
      70,    -1,    72,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    -1,    61,    -1,    63,    -1,    65,    -1,    67,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    -1,
      78
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    16,    19,    21,    24,    25,    26,    27,    30,
      33,    41,    59,    61,    63,    65,    67,    69,    76,    78,
     107,   108,   109,   110,   111,   145,   148,   149,   152,   153,
     154,   155,   162,   163,   172,   173,   174,   175,   178,   179,
     180,   183,   185,   186,   187,   188,   189,    40,     3,    35,
      34,    35,    62,     3,     4,     5,     6,     9,    10,    15,
      23,    38,    39,    48,    50,    71,    82,    96,    97,   100,
     104,   112,   113,   115,   141,   142,     3,    10,    36,    52,
      66,   181,    20,    36,    49,    70,    72,    81,   146,   181,
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
     147,    89,    89,    44,     3,   176,   177,     3,   104,   156,
      62,     5,     6,     8,   122,   124,   166,   103,   167,    32,
      42,   184,    38,   103,   105,   103,   105,     3,   105,   105,
     103,   105,   103,   105,   105,   141,   141,    83,   117,   118,
     150,   104,    45,    89,   103,   118,     3,     4,    23,    38,
      82,   121,   157,    79,     3,   103,   105,   103,   105,   161,
     165,   168,   169,   105,    68,    39,    89,   144,     5,   105,
     141,   141,    47,   119,   120,   121,    37,   125,   126,   124,
      57,    58,    64,   182,   104,   122,   123,   177,   104,   104,
     103,   105,   104,   158,   159,   122,     8,    12,   170,   103,
       8,   104,   122,   105,   105,   104,    86,    17,    39,    89,
      90,    91,    92,    93,    94,   100,    18,    84,   127,   128,
     105,    22,    77,    57,   105,   124,    97,   105,   121,   104,
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

  case 220:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 221:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 229:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 230:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 231:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 232:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 233:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 234:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 235:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 236:

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

