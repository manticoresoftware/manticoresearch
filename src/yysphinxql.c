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
     TOK_CONST_INT = 259,
     TOK_CONST_FLOAT = 260,
     TOK_QUOTED_STRING = 261,
     TOK_AS = 262,
     TOK_ASC = 263,
     TOK_AVG = 264,
     TOK_BEGIN = 265,
     TOK_BETWEEN = 266,
     TOK_BY = 267,
     TOK_CALL = 268,
     TOK_COMMIT = 269,
     TOK_COUNT = 270,
     TOK_DELETE = 271,
     TOK_DESC = 272,
     TOK_DESCRIBE = 273,
     TOK_DISTINCT = 274,
     TOK_FALSE = 275,
     TOK_FROM = 276,
     TOK_GROUP = 277,
     TOK_ID = 278,
     TOK_IN = 279,
     TOK_INSERT = 280,
     TOK_INTO = 281,
     TOK_LIMIT = 282,
     TOK_MATCH = 283,
     TOK_MAX = 284,
     TOK_META = 285,
     TOK_MIN = 286,
     TOK_OPTION = 287,
     TOK_ORDER = 288,
     TOK_REPLACE = 289,
     TOK_ROLLBACK = 290,
     TOK_SELECT = 291,
     TOK_SET = 292,
     TOK_SHOW = 293,
     TOK_START = 294,
     TOK_STATUS = 295,
     TOK_SUM = 296,
     TOK_TABLES = 297,
     TOK_TRANSACTION = 298,
     TOK_TRUE = 299,
     TOK_VALUES = 300,
     TOK_WARNINGS = 301,
     TOK_WEIGHT = 302,
     TOK_WHERE = 303,
     TOK_WITHIN = 304,
     TOK_OR = 305,
     TOK_AND = 306,
     TOK_NE = 307,
     TOK_GTE = 308,
     TOK_LTE = 309,
     TOK_NOT = 310,
     TOK_NEG = 311
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST_INT 259
#define TOK_CONST_FLOAT 260
#define TOK_QUOTED_STRING 261
#define TOK_AS 262
#define TOK_ASC 263
#define TOK_AVG 264
#define TOK_BEGIN 265
#define TOK_BETWEEN 266
#define TOK_BY 267
#define TOK_CALL 268
#define TOK_COMMIT 269
#define TOK_COUNT 270
#define TOK_DELETE 271
#define TOK_DESC 272
#define TOK_DESCRIBE 273
#define TOK_DISTINCT 274
#define TOK_FALSE 275
#define TOK_FROM 276
#define TOK_GROUP 277
#define TOK_ID 278
#define TOK_IN 279
#define TOK_INSERT 280
#define TOK_INTO 281
#define TOK_LIMIT 282
#define TOK_MATCH 283
#define TOK_MAX 284
#define TOK_META 285
#define TOK_MIN 286
#define TOK_OPTION 287
#define TOK_ORDER 288
#define TOK_REPLACE 289
#define TOK_ROLLBACK 290
#define TOK_SELECT 291
#define TOK_SET 292
#define TOK_SHOW 293
#define TOK_START 294
#define TOK_STATUS 295
#define TOK_SUM 296
#define TOK_TABLES 297
#define TOK_TRANSACTION 298
#define TOK_TRUE 299
#define TOK_VALUES 300
#define TOK_WARNINGS 301
#define TOK_WEIGHT 302
#define TOK_WHERE 303
#define TOK_WITHIN 304
#define TOK_OR 305
#define TOK_AND 306
#define TOK_NE 307
#define TOK_GTE 308
#define TOK_LTE 309
#define TOK_NOT 310
#define TOK_NEG 311




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif


// some helpers
#include <float.h> // for FLT_MAX

static void AddFloatRangeFilter ( SqlParser_c * pParser, const CSphString & sAttr, float fMin, float fMax )
{
	CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_FLOATRANGE;
	tFilter.m_fMinValue = fMin;
	tFilter.m_fMaxValue = fMax;
}

static void AddUintRangeFilter ( SqlParser_c * pParser, const CSphString & sAttr, DWORD uMin, DWORD uMax )
{
	CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_RANGE;
	tFilter.m_uMinValue = uMin;
	tFilter.m_uMaxValue = uMax;
}

static void AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iInstype;
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	tIns.m_sVal = tNode.m_sValue;
}


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
#define YYFINAL  54
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   481

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  57
/* YYNRULES -- Number of rules. */
#define YYNRULES  155
/* YYNRULES -- Number of states. */
#define YYNSTATES  304

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   311

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    53,     2,
      68,    69,    62,    60,    67,    61,     2,    63,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    66,
      56,    54,    57,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    52,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    55,    58,    59,
      64,    65
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    32,    34,    38,    41,    43,    47,    49,    53,
      60,    67,    74,    81,    83,    89,    91,    95,    96,    98,
     101,   103,   107,   112,   116,   120,   124,   130,   137,   143,
     147,   151,   155,   159,   163,   167,   171,   175,   181,   185,
     189,   191,   194,   196,   199,   201,   205,   206,   208,   212,
     213,   215,   221,   222,   224,   228,   230,   234,   236,   239,
     242,   244,   246,   247,   249,   252,   257,   258,   260,   263,
     265,   269,   273,   277,   283,   285,   289,   293,   295,   297,
     299,   302,   305,   309,   313,   317,   321,   325,   329,   333,
     337,   341,   345,   349,   353,   357,   361,   365,   367,   372,
     377,   381,   388,   395,   397,   401,   404,   406,   408,   410,
     415,   417,   419,   421,   423,   425,   427,   429,   432,   439,
     441,   443,   444,   448,   450,   454,   456,   460,   464,   466,
     470,   472,   474,   476,   484,   491,   492,   495,   497,   501,
     505,   507,   509,   512,   514,   516
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      71,     0,    -1,    73,    -1,   104,    -1,   110,    -1,   118,
      -1,   106,    -1,   108,    -1,   119,    -1,   124,    -1,   126,
      -1,    36,    74,    21,    76,    77,    84,    86,    88,    93,
      95,    -1,    72,    -1,    73,    66,    72,    -1,    73,    66,
      -1,    75,    -1,    74,    67,    75,    -1,     3,    -1,   101,
       7,     3,    -1,     9,    68,   101,    69,     7,     3,    -1,
      29,    68,   101,    69,     7,     3,    -1,    31,    68,   101,
      69,     7,     3,    -1,    41,    68,   101,    69,     7,     3,
      -1,    62,    -1,    15,    68,    19,     3,    69,    -1,     3,
      -1,    76,    67,     3,    -1,    -1,    78,    -1,    48,    79,
      -1,    80,    -1,    79,    51,    80,    -1,    28,    68,     6,
      69,    -1,    23,    54,    81,    -1,     3,    54,    81,    -1,
       3,    55,    81,    -1,     3,    24,    68,    83,    69,    -1,
       3,    64,    24,    68,    83,    69,    -1,     3,    11,    81,
      51,    81,    -1,     3,    57,    81,    -1,     3,    56,    81,
      -1,     3,    58,    81,    -1,     3,    59,    81,    -1,     3,
      54,    82,    -1,     3,    55,    82,    -1,     3,    57,    82,
      -1,     3,    56,    82,    -1,     3,    11,    82,    51,    82,
      -1,     3,    58,    82,    -1,     3,    59,    82,    -1,     4,
      -1,    61,     4,    -1,     5,    -1,    61,     5,    -1,    81,
      -1,    83,    67,    81,    -1,    -1,    85,    -1,    22,    12,
       3,    -1,    -1,    87,    -1,    49,    22,    33,    12,    90,
      -1,    -1,    89,    -1,    33,    12,    90,    -1,    91,    -1,
      90,    67,    91,    -1,    92,    -1,    92,     8,    -1,    92,
      17,    -1,    23,    -1,     3,    -1,    -1,    94,    -1,    27,
       4,    -1,    27,     4,    67,     4,    -1,    -1,    96,    -1,
      32,    97,    -1,    98,    -1,    97,    67,    98,    -1,     3,
      54,     3,    -1,     3,    54,     4,    -1,     3,    54,    68,
      99,    69,    -1,   100,    -1,    99,    67,   100,    -1,     3,
      54,    81,    -1,     3,    -1,     4,    -1,     5,    -1,    61,
     101,    -1,    64,   101,    -1,   101,    60,   101,    -1,   101,
      61,   101,    -1,   101,    62,   101,    -1,   101,    63,   101,
      -1,   101,    56,   101,    -1,   101,    57,   101,    -1,   101,
      53,   101,    -1,   101,    52,   101,    -1,   101,    59,   101,
      -1,   101,    58,   101,    -1,   101,    54,   101,    -1,   101,
      55,   101,    -1,   101,    51,   101,    -1,   101,    50,   101,
      -1,    68,   101,    69,    -1,   102,    -1,     3,    68,   103,
      69,    -1,    24,    68,   103,    69,    -1,     3,    68,    69,
      -1,    31,    68,   101,    67,   101,    69,    -1,    29,    68,
     101,    67,   101,    69,    -1,   101,    -1,   103,    67,   101,
      -1,    38,   105,    -1,    46,    -1,    40,    -1,    30,    -1,
      37,     3,    54,   107,    -1,    44,    -1,    20,    -1,    81,
      -1,    14,    -1,    35,    -1,   109,    -1,    10,    -1,    39,
      43,    -1,   111,    26,     3,   112,    45,   114,    -1,    25,
      -1,    34,    -1,    -1,    68,   113,    69,    -1,    92,    -1,
     113,    67,    92,    -1,   115,    -1,   114,    67,   115,    -1,
      68,   116,    69,    -1,   117,    -1,   116,    67,   117,    -1,
      81,    -1,    82,    -1,     6,    -1,    16,    21,     3,    48,
      23,    54,    81,    -1,    13,     3,    68,   116,   120,    69,
      -1,    -1,    67,   121,    -1,   122,    -1,   121,    67,   122,
      -1,   117,     7,   123,    -1,     3,    -1,    27,    -1,   125,
       3,    -1,    18,    -1,    17,    -1,    38,    42,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   110,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   124,   139,   140,   141,   145,   146,   150,   152,   154,
     155,   156,   157,   158,   159,   175,   176,   179,   181,   185,
     189,   190,   194,   207,   214,   221,   229,   237,   246,   250,
     254,   258,   262,   266,   267,   268,   269,   274,   278,   282,
     289,   290,   294,   295,   299,   300,   303,   305,   309,   316,
     318,   322,   328,   330,   334,   341,   342,   346,   347,   348,
     352,   353,   356,   358,   362,   367,   374,   376,   380,   384,
     385,   389,   394,   399,   408,   418,   430,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   464,   465,
     466,   467,   468,   472,   473,   479,   483,   484,   485,   491,
     500,   501,   502,   516,   517,   518,   522,   523,   529,   537,
     538,   541,   543,   547,   548,   552,   553,   557,   561,   562,
     566,   567,   568,   574,   585,   592,   594,   598,   603,   607,
     615,   616,   622,   630,   631,   637
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_CONST_INT", 
  "TOK_CONST_FLOAT", "TOK_QUOTED_STRING", "TOK_AS", "TOK_ASC", "TOK_AVG", 
  "TOK_BEGIN", "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_COMMIT", 
  "TOK_COUNT", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", 
  "TOK_FALSE", "TOK_FROM", "TOK_GROUP", "TOK_ID", "TOK_IN", "TOK_INSERT", 
  "TOK_INTO", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", 
  "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", "TOK_ROLLBACK", "TOK_SELECT", 
  "TOK_SET", "TOK_SHOW", "TOK_START", "TOK_STATUS", "TOK_SUM", 
  "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", "TOK_VALUES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "TOK_NOT", "TOK_NEG", "';'", 
  "','", "'('", "')'", "$accept", "statement", "select_from", 
  "select_from_list", "select_items_list", "select_item", "ident_list", 
  "opt_where_clause", "where_clause", "where_expr", "where_item", 
  "const_int", "const_float", "const_list", "opt_group_clause", 
  "group_clause", "opt_group_order_clause", "group_order_clause", 
  "opt_order_clause", "order_clause", "order_items_list", "order_item", 
  "ident_or_id", "opt_limit_clause", "limit_clause", "opt_option_clause", 
  "option_clause", "option_list", "option_item", "named_const_list", 
  "named_const", "expr", "function", "arglist", "show_clause", 
  "show_variable", "set_clause", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "opt_call_opts_list", "call_opts_list", "call_opt", "call_opt_name", 
  "describe", "describe_tok", "show_tables", 0
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
     305,   306,   124,    38,    61,   307,    60,    62,   308,   309,
      43,    45,    42,    47,   310,   311,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    70,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    72,    73,    73,    73,    74,    74,    75,    75,    75,
      75,    75,    75,    75,    75,    76,    76,    77,    77,    78,
      79,    79,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      81,    81,    82,    82,    83,    83,    84,    84,    85,    86,
      86,    87,    88,    88,    89,    90,    90,    91,    91,    91,
      92,    92,    93,    93,    94,    94,    95,    95,    96,    97,
      97,    98,    98,    98,    99,    99,   100,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   102,   102,
     102,   102,   102,   103,   103,   104,   105,   105,   105,   106,
     107,   107,   107,   108,   108,   108,   109,   109,   110,   111,
     111,   112,   112,   113,   113,   114,   114,   115,   116,   116,
     117,   117,   117,   118,   119,   120,   120,   121,   121,   122,
     123,   123,   124,   125,   125,   126
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,    10,     1,     3,     2,     1,     3,     1,     3,     6,
       6,     6,     6,     1,     5,     1,     3,     0,     1,     2,
       1,     3,     4,     3,     3,     3,     5,     6,     5,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     3,     3,
       1,     2,     1,     2,     1,     3,     0,     1,     3,     0,
       1,     5,     0,     1,     3,     1,     3,     1,     2,     2,
       1,     1,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     1,     3,     3,     1,     1,     1,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     4,     4,
       3,     6,     6,     1,     3,     2,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     1,     2,     6,     1,
       1,     0,     3,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     7,     6,     0,     2,     1,     3,     3,
       1,     1,     2,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   126,     0,   123,     0,   154,   153,   129,   130,   124,
       0,     0,     0,     0,     0,    12,     2,     3,     6,     7,
     125,     4,     0,     5,     8,     9,     0,    10,     0,     0,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
      23,     0,     0,     0,    15,     0,   107,     0,   118,   117,
     155,   116,   115,   127,     1,    14,     0,   152,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    87,     0,     0,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,   131,    50,    52,   142,     0,   140,   141,   145,
     138,     0,   110,   113,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,    25,    27,    16,    18,   105,   104,
      99,    98,   102,   103,    96,    97,   101,   100,    92,    93,
      94,    95,   121,   120,     0,   122,   119,     0,     0,    51,
      53,     0,     0,     0,     0,   108,     0,     0,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    56,    28,
      71,    70,   133,     0,     0,   139,   146,   147,   144,     0,
     114,     0,    24,     0,     0,     0,     0,     0,     0,     0,
       0,    29,    30,    26,     0,    59,    57,     0,   132,     0,
     128,   135,     0,     0,   143,    19,   112,    20,   111,    21,
      22,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    62,    60,   134,     0,     0,
     150,   151,   149,     0,   148,     0,     0,     0,    34,    43,
      35,    44,    40,    46,    39,    45,    41,    48,    42,    49,
       0,    33,     0,    31,    58,     0,     0,    72,    63,     0,
     137,   136,     0,     0,    54,     0,     0,    32,     0,     0,
       0,    76,    73,   139,    38,     0,    47,     0,    36,     0,
       0,    64,    65,    67,    74,     0,    11,    77,    55,    37,
      61,     0,    68,    69,     0,     0,    78,    79,    66,    75,
       0,     0,    81,    82,     0,    80,     0,     0,    84,     0,
       0,    83,    86,    85
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    14,    15,    16,    43,    44,   115,   158,   159,   181,
     182,    97,    98,   255,   185,   186,   215,   216,   247,   248,
     271,   272,   273,   261,   262,   276,   277,   286,   287,   297,
     298,    45,    46,   104,    17,    52,    18,   136,    19,    20,
      21,    22,   138,   163,   190,   191,    99,   100,    23,    24,
     142,   166,   167,   222,    25,    26,    27
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -178
static const short yypact[] =
{
     200,  -178,    -2,  -178,   -17,  -178,  -178,  -178,  -178,  -178,
     126,     5,    62,   -31,    15,  -178,   -29,  -178,  -178,  -178,
    -178,  -178,    65,  -178,  -178,  -178,    97,  -178,    41,   109,
     -18,  -178,  -178,    54,    55,    70,   100,   101,   108,    42,
    -178,    42,    42,    23,  -178,   145,  -178,    85,  -178,  -178,
    -178,  -178,  -178,  -178,  -178,   128,   175,  -178,    14,   131,
      64,    42,   162,    42,    42,    42,    42,   114,   115,   117,
    -178,  -178,   230,   181,   126,   183,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      28,  -178,   121,  -178,  -178,  -178,    80,  -178,  -178,   124,
    -178,   169,  -178,   366,   -60,   250,   208,    29,   190,   210,
     270,    42,    42,  -178,  -178,    34,  -178,  -178,   379,   391,
     402,   412,    86,    86,    98,    98,    98,    98,   -20,   -20,
    -178,  -178,  -178,  -178,   189,  -178,  -178,    74,   167,  -178,
    -178,    14,   146,   165,    42,  -178,   213,   152,  -178,    42,
     215,    42,   216,   217,   330,   348,    71,   223,   205,  -178,
    -178,  -178,  -178,    38,   160,   222,   163,  -178,  -178,    19,
     366,   228,  -178,   290,   229,   310,   251,   252,   116,   179,
     188,   207,  -178,  -178,   262,   226,  -178,    74,  -178,    14,
     209,  -178,     7,    14,  -178,  -178,  -178,  -178,  -178,  -178,
    -178,    17,   227,    17,    17,    17,    17,    17,    17,   254,
      19,   288,    71,   293,   275,   265,  -178,  -178,    67,   160,
    -178,  -178,  -178,   222,  -178,   263,   264,    19,  -178,  -178,
    -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,
     248,  -178,   249,  -178,  -178,   284,   322,   308,  -178,    14,
    -178,  -178,    19,    20,  -178,    84,    19,  -178,   324,    74,
     333,   306,  -178,  -178,  -178,   349,  -178,    19,  -178,    87,
      74,   289,  -178,    -6,   291,   352,  -178,  -178,  -178,  -178,
     289,    74,  -178,  -178,   353,   320,   309,  -178,  -178,  -178,
       2,   352,  -178,  -178,   372,  -178,   323,    96,  -178,    19,
     372,  -178,  -178,  -178
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -178,  -178,   339,  -178,  -178,   304,  -178,  -178,  -178,  -178,
     184,   -90,  -177,   139,  -178,  -178,  -178,  -178,  -178,  -178,
     142,   132,  -104,  -178,  -178,  -178,  -178,  -178,   123,  -178,
     176,   -25,  -178,   414,  -178,  -178,  -178,  -178,  -178,  -178,
    -178,  -178,  -178,  -178,  -178,   259,   292,  -128,  -178,  -178,
    -178,  -178,   286,  -178,  -178,  -178,  -178
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -18
static const short yytable[] =
{
     135,    28,   282,   -17,    29,   292,   293,   144,    47,   145,
     220,   283,    53,   165,    70,    54,    71,    72,    93,    94,
      95,    93,    94,    93,   226,    94,   229,   231,   233,   235,
     237,   239,    93,   162,   221,   103,   105,    55,   103,   108,
     109,   110,    88,    89,    73,    67,    31,    32,   132,   -17,
      60,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   223,    35,    67,    31,    32,
     294,    68,   133,    69,   178,    96,   266,   160,    96,   194,
     134,   265,   156,   217,   139,   140,   154,   155,    35,   134,
      74,    56,    48,    68,   179,    69,   144,   161,   148,   180,
      57,   157,    49,    39,    50,   187,    41,   188,    51,    58,
      42,   225,    59,   228,   230,   232,   234,   236,   238,   170,
     241,   263,    61,    62,   173,    39,   175,   201,    41,    30,
      31,    32,    42,   102,   249,    33,   250,   254,    63,    90,
     202,    34,    82,    83,    84,    85,    86,    87,    88,    89,
      35,   267,    75,   268,   267,    36,   279,    37,    86,    87,
      88,    89,   264,   300,    10,   301,   254,    38,    64,    65,
     203,   204,   205,   206,   207,   208,    66,   278,    92,   101,
     209,   106,    60,   111,   114,   112,   117,    39,    40,   137,
      41,   141,   143,   139,    42,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,   302,
       1,   147,   164,     2,     3,   168,     4,     5,     6,   169,
     171,   172,   174,   176,   177,     7,   183,   184,   189,   192,
     193,   195,   197,   210,     8,     9,    10,    11,    12,    13,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   199,   200,   211,   149,   212,   150,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   213,   214,   219,   151,   240,   152,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   242,   227,   244,   245,   246,   113,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   252,   253,   256,   258,   257,   146,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   259,   260,   270,   274,   275,   153,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   140,   285,   281,   289,   284,   196,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   290,   296,   291,   299,   116,   198,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    91,   269,   243,   149,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,   280,   288,   295,   151,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,   303,   107,   251,   224,
       0,   218
};

static const short yycheck[] =
{
      90,     3,     8,    21,    21,     3,     4,    67,     3,    69,
       3,    17,    43,   141,    39,     0,    41,    42,     4,     5,
       6,     4,     5,     4,   201,     5,   203,   204,   205,   206,
     207,   208,     4,   137,    27,    60,    61,    66,    63,    64,
      65,    66,    62,    63,    21,     3,     4,     5,    20,    67,
      68,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,   193,    24,     3,     4,     5,
      68,    29,    44,    31,     3,    61,   253,     3,    61,   169,
      61,    61,    48,   187,     4,     5,   111,   112,    24,    61,
      67,    26,    30,    29,    23,    31,    67,    23,    69,    28,
       3,    67,    40,    61,    42,    67,    64,    69,    46,    68,
      68,   201,     3,   203,   204,   205,   206,   207,   208,   144,
     210,   249,    68,    68,   149,    61,   151,    11,    64,     3,
       4,     5,    68,    69,    67,     9,    69,   227,    68,    54,
      24,    15,    56,    57,    58,    59,    60,    61,    62,    63,
      24,    67,     7,    69,    67,    29,    69,    31,    60,    61,
      62,    63,   252,    67,    36,    69,   256,    41,    68,    68,
      54,    55,    56,    57,    58,    59,    68,   267,     3,    48,
      64,    19,    68,    68,     3,    68,     3,    61,    62,    68,
      64,    67,    23,     4,    68,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,   299,
      10,     3,    45,    13,    14,    69,    16,    17,    18,    54,
       7,    69,     7,     7,     7,    25,     3,    22,    68,     7,
      67,     3,     3,    54,    34,    35,    36,    37,    38,    39,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,     3,     3,    68,    67,    51,    69,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    12,    49,    67,    67,    24,    69,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,     6,    68,     3,    22,    33,    69,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    51,    51,    68,    33,    69,    69,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    12,    27,    12,     4,    32,    69,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,     5,     3,    67,     4,    67,    69,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    54,     3,    67,    54,    74,    69,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    55,   256,   212,    67,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,   270,   281,   291,    67,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,   300,    63,   219,   193,
      -1,   189
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    10,    13,    14,    16,    17,    18,    25,    34,    35,
      36,    37,    38,    39,    71,    72,    73,   104,   106,   108,
     109,   110,   111,   118,   119,   124,   125,   126,     3,    21,
       3,     4,     5,     9,    15,    24,    29,    31,    41,    61,
      62,    64,    68,    74,    75,   101,   102,     3,    30,    40,
      42,    46,   105,    43,     0,    66,    26,     3,    68,     3,
      68,    68,    68,    68,    68,    68,    68,     3,    29,    31,
     101,   101,   101,    21,    67,     7,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      54,    72,     3,     4,     5,     6,    61,    81,    82,   116,
     117,    48,    69,   101,   103,   101,    19,   103,   101,   101,
     101,    68,    68,    69,     3,    76,    75,     3,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,    20,    44,    61,    81,   107,    68,   112,     4,
       5,    67,   120,    23,    67,    69,    69,     3,    69,    67,
      69,    67,    69,    69,   101,   101,    48,    67,    77,    78,
       3,    23,    92,   113,    45,   117,   121,   122,    69,    54,
     101,     7,    69,   101,     7,   101,     7,     7,     3,    23,
      28,    79,    80,     3,    22,    84,    85,    67,    69,    68,
     114,   115,     7,    67,    81,     3,    69,     3,    69,     3,
       3,    11,    24,    54,    55,    56,    57,    58,    59,    64,
      54,    68,    51,    12,    49,    86,    87,    92,   116,    67,
       3,    27,   123,   117,   122,    81,    82,    68,    81,    82,
      81,    82,    81,    82,    81,    82,    81,    82,    81,    82,
      24,    81,     6,    80,     3,    22,    33,    88,    89,    67,
      69,   115,    51,    51,    81,    83,    68,    69,    33,    12,
      27,    93,    94,   117,    81,    61,    82,    67,    69,    83,
      12,    90,    91,    92,     4,    32,    95,    96,    81,    69,
      90,    67,     8,    17,    67,     3,    97,    98,    91,     4,
      54,    67,     3,     4,    68,    98,     3,    99,   100,    54,
      67,    69,    81,   100
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
        case 11:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 12:

    { pParser->PushQuery(); ;}
    break;

  case 13:

    { pParser->PushQuery(); ;}
    break;

  case 17:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd );
											pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 18:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd );
									pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 19:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 20:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 21:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 22:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 23:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 24:

    {
			if ( !pParser->m_pQuery->m_sGroupDistinct.IsEmpty() )
			{
				yyerror ( pParser, "too many COUNT(DISTINCT) clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sGroupDistinct = yyvsp[-1].m_sValue;
				pParser->SetSelect ( yyvsp[-1].m_iStart, yyvsp[-1].m_iEnd );
			}
		;}
    break;

  case 26:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 32:

    {
			if ( pParser->m_bGotQuery )
			{
				yyerror ( pParser, "too many MATCH() clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sQuery = yyvsp[-1].m_sValue;
				pParser->m_bGotQuery = true;
			}
		;}
    break;

  case 33:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 34:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 35:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
		;}
    break;

  case 36:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 37:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 38:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 39:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 40:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 41:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 42:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 46:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 47:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 48:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 49:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 50:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 51:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 52:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 53:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 54:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 55:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 58:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 61:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 64:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 66:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 68:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 69:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 74:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 75:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 84:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 85:

    {
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 86:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 90:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 91:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 92:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 93:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 94:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 95:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 96:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 97:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 98:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 99:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 100:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 101:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 102:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 103:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 104:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 105:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 106:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 108:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 109:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 110:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 111:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 112:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 116:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 117:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 118:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 119:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 120:

    { yyval.m_iValue = 1; ;}
    break;

  case 121:

    { yyval.m_iValue = 0; ;}
    break;

  case 122:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 123:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 124:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 125:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 128:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 129:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 130:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 133:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 134:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 137:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 138:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 139:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 140:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 141:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 142:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 143:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iDeleteID = yyvsp[0].m_iValue;
		;}
    break;

  case 144:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 147:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 149:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 151:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 152:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 155:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
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

