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
#define YYLAST   490

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  57
/* YYNRULES -- Number of rules. */
#define YYNRULES  154
/* YYNRULES -- Number of states. */
#define YYNSTATES  305

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
      19,    21,    32,    34,    39,    41,    45,    47,    51,    58,
      65,    72,    79,    81,    87,    89,    93,    94,    96,    99,
     101,   105,   110,   114,   118,   122,   128,   135,   141,   145,
     149,   153,   157,   161,   165,   169,   173,   179,   183,   187,
     189,   192,   194,   197,   199,   203,   204,   206,   210,   211,
     213,   219,   220,   222,   226,   228,   232,   234,   237,   240,
     242,   244,   245,   247,   250,   255,   256,   258,   261,   263,
     267,   271,   275,   281,   283,   287,   291,   293,   295,   297,
     300,   303,   307,   311,   315,   319,   323,   327,   331,   335,
     339,   343,   347,   351,   355,   359,   363,   365,   370,   375,
     379,   386,   393,   395,   399,   402,   404,   406,   408,   413,
     415,   417,   419,   421,   423,   425,   427,   430,   437,   439,
     441,   442,   446,   448,   452,   454,   458,   462,   464,   468,
     470,   472,   474,   482,   489,   490,   493,   495,   499,   503,
     505,   507,   510,   512,   514
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      71,     0,    -1,    73,    -1,   104,    -1,   110,    -1,   118,
      -1,   106,    -1,   108,    -1,   119,    -1,   124,    -1,   126,
      -1,    36,    74,    21,    76,    77,    84,    86,    88,    93,
      95,    -1,    72,    -1,    73,    66,    72,    66,    -1,    75,
      -1,    74,    67,    75,    -1,     3,    -1,   101,     7,     3,
      -1,     9,    68,   101,    69,     7,     3,    -1,    29,    68,
     101,    69,     7,     3,    -1,    31,    68,   101,    69,     7,
       3,    -1,    41,    68,   101,    69,     7,     3,    -1,    62,
      -1,    15,    68,    19,     3,    69,    -1,     3,    -1,    76,
      67,     3,    -1,    -1,    78,    -1,    48,    79,    -1,    80,
      -1,    79,    51,    80,    -1,    28,    68,     6,    69,    -1,
      23,    54,    81,    -1,     3,    54,    81,    -1,     3,    55,
      81,    -1,     3,    24,    68,    83,    69,    -1,     3,    64,
      24,    68,    83,    69,    -1,     3,    11,    81,    51,    81,
      -1,     3,    57,    81,    -1,     3,    56,    81,    -1,     3,
      58,    81,    -1,     3,    59,    81,    -1,     3,    54,    82,
      -1,     3,    55,    82,    -1,     3,    57,    82,    -1,     3,
      56,    82,    -1,     3,    11,    82,    51,    82,    -1,     3,
      58,    82,    -1,     3,    59,    82,    -1,     4,    -1,    61,
       4,    -1,     5,    -1,    61,     5,    -1,    81,    -1,    83,
      67,    81,    -1,    -1,    85,    -1,    22,    12,     3,    -1,
      -1,    87,    -1,    49,    22,    33,    12,    90,    -1,    -1,
      89,    -1,    33,    12,    90,    -1,    91,    -1,    90,    67,
      91,    -1,    92,    -1,    92,     8,    -1,    92,    17,    -1,
      23,    -1,     3,    -1,    -1,    94,    -1,    27,     4,    -1,
      27,     4,    67,     4,    -1,    -1,    96,    -1,    32,    97,
      -1,    98,    -1,    97,    67,    98,    -1,     3,    54,     3,
      -1,     3,    54,     4,    -1,     3,    54,    68,    99,    69,
      -1,   100,    -1,    99,    67,   100,    -1,     3,    54,    81,
      -1,     3,    -1,     4,    -1,     5,    -1,    61,   101,    -1,
      64,   101,    -1,   101,    60,   101,    -1,   101,    61,   101,
      -1,   101,    62,   101,    -1,   101,    63,   101,    -1,   101,
      56,   101,    -1,   101,    57,   101,    -1,   101,    53,   101,
      -1,   101,    52,   101,    -1,   101,    59,   101,    -1,   101,
      58,   101,    -1,   101,    54,   101,    -1,   101,    55,   101,
      -1,   101,    51,   101,    -1,   101,    50,   101,    -1,    68,
     101,    69,    -1,   102,    -1,     3,    68,   103,    69,    -1,
      24,    68,   103,    69,    -1,     3,    68,    69,    -1,    31,
      68,   101,    67,   101,    69,    -1,    29,    68,   101,    67,
     101,    69,    -1,   101,    -1,   103,    67,   101,    -1,    38,
     105,    -1,    46,    -1,    40,    -1,    30,    -1,    37,     3,
      54,   107,    -1,    44,    -1,    20,    -1,    81,    -1,    14,
      -1,    35,    -1,   109,    -1,    10,    -1,    39,    43,    -1,
     111,    26,     3,   112,    45,   114,    -1,    25,    -1,    34,
      -1,    -1,    68,   113,    69,    -1,    92,    -1,   113,    67,
      92,    -1,   115,    -1,   114,    67,   115,    -1,    68,   116,
      69,    -1,   117,    -1,   116,    67,   117,    -1,    81,    -1,
      82,    -1,     6,    -1,    16,    21,     3,    48,    23,    54,
      81,    -1,    13,     3,    68,   116,   120,    69,    -1,    -1,
      67,   121,    -1,   122,    -1,   121,    67,   122,    -1,   117,
       7,   123,    -1,     3,    -1,    27,    -1,   125,     3,    -1,
      18,    -1,    17,    -1,    38,    42,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   110,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   124,   139,   140,   144,   145,   149,   151,   153,   154,
     155,   156,   157,   158,   174,   175,   178,   180,   184,   188,
     189,   193,   206,   213,   220,   228,   236,   245,   249,   253,
     257,   261,   265,   266,   267,   268,   273,   277,   281,   288,
     289,   293,   294,   298,   299,   302,   304,   308,   315,   317,
     321,   327,   329,   333,   340,   341,   345,   346,   347,   351,
     352,   355,   357,   361,   366,   373,   375,   379,   383,   384,
     388,   393,   398,   407,   417,   429,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   463,   464,   465,
     466,   467,   471,   472,   478,   482,   483,   484,   490,   499,
     500,   501,   515,   516,   517,   521,   522,   528,   536,   537,
     540,   542,   546,   547,   551,   552,   556,   560,   561,   565,
     566,   567,   573,   584,   591,   593,   597,   602,   606,   614,
     615,   621,   629,   630,   636
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
      71,    72,    73,    73,    74,    74,    75,    75,    75,    75,
      75,    75,    75,    75,    76,    76,    77,    77,    78,    79,
      79,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    81,
      81,    82,    82,    83,    83,    84,    84,    85,    86,    86,
      87,    88,    88,    89,    90,    90,    91,    91,    91,    92,
      92,    93,    93,    94,    94,    95,    95,    96,    97,    97,
      98,    98,    98,    99,    99,   100,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   102,   102,   102,
     102,   102,   103,   103,   104,   105,   105,   105,   106,   107,
     107,   107,   108,   108,   108,   109,   109,   110,   111,   111,
     112,   112,   113,   113,   114,   114,   115,   116,   116,   117,
     117,   117,   118,   119,   120,   120,   121,   121,   122,   123,
     123,   124,   125,   125,   126
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,    10,     1,     4,     1,     3,     1,     3,     6,     6,
       6,     6,     1,     5,     1,     3,     0,     1,     2,     1,
       3,     4,     3,     3,     3,     5,     6,     5,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     3,     3,     1,
       2,     1,     2,     1,     3,     0,     1,     3,     0,     1,
       5,     0,     1,     3,     1,     3,     1,     2,     2,     1,
       1,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     1,     3,     3,     1,     1,     1,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     4,     4,     3,
       6,     6,     1,     3,     2,     1,     1,     1,     4,     1,
       1,     1,     1,     1,     1,     1,     2,     6,     1,     1,
       0,     3,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     7,     6,     0,     2,     1,     3,     3,     1,
       1,     2,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   125,     0,   122,     0,   153,   152,   128,   129,   123,
       0,     0,     0,     0,     0,    12,     2,     3,     6,     7,
     124,     4,     0,     5,     8,     9,     0,    10,     0,     0,
      86,    87,    88,     0,     0,     0,     0,     0,     0,     0,
      22,     0,     0,     0,    14,     0,   106,     0,   117,   116,
     154,   115,   114,   126,     1,     0,     0,   151,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    86,     0,     0,
      89,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   130,    49,    51,   141,     0,   139,   140,   144,
     137,     0,   109,   112,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,    24,    26,    15,    17,   104,   103,
      98,    97,   101,   102,    95,    96,   100,    99,    91,    92,
      93,    94,   120,   119,     0,   121,   118,    13,     0,     0,
      50,    52,     0,     0,     0,     0,   107,     0,     0,   108,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    55,
      27,    70,    69,   132,     0,     0,   138,   145,   146,   143,
       0,   113,     0,    23,     0,     0,     0,     0,     0,     0,
       0,     0,    28,    29,    25,     0,    58,    56,     0,   131,
       0,   127,   134,     0,     0,   142,    18,   111,    19,   110,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    61,    59,   133,     0,
       0,   149,   150,   148,     0,   147,     0,     0,     0,    33,
      42,    34,    43,    39,    45,    38,    44,    40,    47,    41,
      48,     0,    32,     0,    30,    57,     0,     0,    71,    62,
       0,   136,   135,     0,     0,    53,     0,     0,    31,     0,
       0,     0,    75,    72,   138,    37,     0,    46,     0,    35,
       0,     0,    63,    64,    66,    73,     0,    11,    76,    54,
      36,    60,     0,    67,    68,     0,     0,    77,    78,    65,
      74,     0,     0,    80,    81,     0,    79,     0,     0,    83,
       0,     0,    82,    85,    84
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    14,    15,    16,    43,    44,   115,   159,   160,   182,
     183,    97,    98,   256,   186,   187,   216,   217,   248,   249,
     272,   273,   274,   262,   263,   277,   278,   287,   288,   298,
     299,    45,    46,   104,    17,    52,    18,   136,    19,    20,
      21,    22,   139,   164,   191,   192,    99,   100,    23,    24,
     143,   167,   168,   223,    25,    26,    27
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -182
static const short yypact[] =
{
     215,  -182,    31,  -182,    22,  -182,  -182,  -182,  -182,  -182,
     125,    34,   106,    40,    48,  -182,    19,  -182,  -182,  -182,
    -182,  -182,    65,  -182,  -182,  -182,   100,  -182,    39,   105,
     -18,  -182,  -182,    58,    63,    64,    67,    71,    73,   140,
    -182,   140,   140,     9,  -182,   161,  -182,    79,  -182,  -182,
    -182,  -182,  -182,  -182,  -182,   111,   139,  -182,     7,   103,
      41,   140,   134,   140,   140,   140,   140,    87,    89,    90,
    -182,  -182,   245,   156,   125,   157,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
      27,    96,    93,  -182,  -182,  -182,    95,  -182,  -182,    98,
    -182,   147,  -182,   381,    15,   265,   169,    23,   205,   225,
     285,   140,   140,  -182,  -182,    26,  -182,  -182,   394,   406,
     417,   427,   185,   185,   -56,   -56,   -56,   -56,   -53,   -53,
    -182,  -182,  -182,  -182,   170,  -182,  -182,  -182,    78,   128,
    -182,  -182,     7,   107,   121,   140,  -182,   172,   108,  -182,
     140,   181,   140,   184,   187,   345,   363,    66,   189,   173,
    -182,  -182,  -182,  -182,    28,   129,   191,   132,  -182,  -182,
      16,   381,   193,  -182,   305,   197,   325,   199,   200,   126,
     151,   138,   158,  -182,  -182,   195,   177,  -182,    78,  -182,
       7,   160,  -182,     5,     7,  -182,  -182,  -182,  -182,  -182,
    -182,  -182,    14,   162,    14,    14,    14,    14,    14,    14,
     210,    16,   229,    66,   233,   216,   204,  -182,  -182,    29,
     129,  -182,  -182,  -182,   191,  -182,   188,   218,    16,  -182,
    -182,  -182,  -182,  -182,  -182,  -182,  -182,  -182,  -182,  -182,
    -182,   202,  -182,   220,  -182,  -182,   238,   261,   263,  -182,
       7,  -182,  -182,    16,    17,  -182,    37,    16,  -182,   279,
      78,   289,   277,  -182,  -182,  -182,   306,  -182,    16,  -182,
      44,    78,   243,  -182,    25,   246,   309,  -182,  -182,  -182,
    -182,   243,    78,  -182,  -182,   326,   275,   264,  -182,  -182,
    -182,    -2,   309,  -182,  -182,   329,  -182,   295,    55,  -182,
      16,   329,  -182,  -182,  -182
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -182,  -182,   278,  -182,  -182,   276,  -182,  -182,  -182,  -182,
     159,   -90,  -181,    94,  -182,  -182,  -182,  -182,  -182,  -182,
      81,    88,  -109,  -182,  -182,  -182,  -182,  -182,    61,  -182,
      68,   -25,  -182,   308,  -182,  -182,  -182,  -182,  -182,  -182,
    -182,  -182,  -182,  -182,  -182,   153,   201,  -127,  -182,  -182,
    -182,  -182,   196,  -182,  -182,  -182,  -182
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -17
static const short yytable[] =
{
     135,   293,   294,   -16,    86,    87,    88,    89,   221,    88,
      89,    93,    94,    95,    70,   166,    71,    72,    93,    94,
      93,   227,    94,   230,   232,   234,   236,   238,   240,   163,
      73,    93,   222,   283,    28,   103,   105,    47,   103,   108,
     109,   110,   284,    29,    67,    31,    32,   132,    54,   -16,
      60,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,    35,   295,   224,    96,   179,
      68,   133,    69,   267,   157,    96,    74,   134,   266,   218,
     195,   161,   145,    53,   146,    55,   155,   156,   134,   180,
     145,    56,   149,   158,   181,   188,   250,   189,   251,   140,
     141,   162,    39,    57,   268,    41,   269,    58,    59,    42,
     102,   268,   226,   280,   229,   231,   233,   235,   237,   239,
     171,   242,   301,   264,   302,   174,    61,   176,    30,    31,
      32,    62,    63,    90,    33,    64,    48,   202,   255,    65,
      34,    66,    92,    67,    31,    32,    49,    10,    50,    35,
     203,   101,    51,   106,    36,    60,    37,   111,   112,   114,
     117,   138,   137,   265,    35,   142,    38,   255,    75,    68,
     144,    69,   148,   165,   140,   170,   169,   173,   279,   172,
     204,   205,   206,   207,   208,   209,    39,    40,   175,    41,
     210,   177,   184,    42,   178,   185,   196,   190,   193,   194,
     198,    39,   200,   201,    41,   211,   212,   214,    42,   213,
     303,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,     1,   215,   220,     2,     3,
     228,     4,     5,     6,   241,   243,   245,   247,   246,   253,
       7,    82,    83,    84,    85,    86,    87,    88,    89,     8,
       9,    10,    11,    12,    13,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,   254,
     257,   259,   150,   260,   151,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,   258,
     261,   271,   152,   275,   153,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,   276,
     282,   141,   286,   285,   113,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,   291,
     290,   292,   297,    91,   147,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,   300,
     116,   270,   281,   296,   154,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,   304,
     289,   107,   244,   252,   197,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,     0,
     225,   219,     0,     0,   199,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,     0,
       0,     0,   150,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
     152,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89
};

static const short yycheck[] =
{
      90,     3,     4,    21,    60,    61,    62,    63,     3,    62,
      63,     4,     5,     6,    39,   142,    41,    42,     4,     5,
       4,   202,     5,   204,   205,   206,   207,   208,   209,   138,
      21,     4,    27,     8,     3,    60,    61,     3,    63,    64,
      65,    66,    17,    21,     3,     4,     5,    20,     0,    67,
      68,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    24,    68,   194,    61,     3,
      29,    44,    31,   254,    48,    61,    67,    61,    61,   188,
     170,     3,    67,    43,    69,    66,   111,   112,    61,    23,
      67,    26,    69,    67,    28,    67,    67,    69,    69,     4,
       5,    23,    61,     3,    67,    64,    69,    68,     3,    68,
      69,    67,   202,    69,   204,   205,   206,   207,   208,   209,
     145,   211,    67,   250,    69,   150,    68,   152,     3,     4,
       5,    68,    68,    54,     9,    68,    30,    11,   228,    68,
      15,    68,     3,     3,     4,     5,    40,    36,    42,    24,
      24,    48,    46,    19,    29,    68,    31,    68,    68,     3,
       3,    68,    66,   253,    24,    67,    41,   257,     7,    29,
      23,    31,     3,    45,     4,    54,    69,    69,   268,     7,
      54,    55,    56,    57,    58,    59,    61,    62,     7,    64,
      64,     7,     3,    68,     7,    22,     3,    68,     7,    67,
       3,    61,     3,     3,    64,    54,    68,    12,    68,    51,
     300,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    10,    49,    67,    13,    14,
      68,    16,    17,    18,    24,     6,     3,    33,    22,    51,
      25,    56,    57,    58,    59,    60,    61,    62,    63,    34,
      35,    36,    37,    38,    39,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    51,
      68,    33,    67,    12,    69,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    69,
      27,    12,    67,     4,    69,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    32,
      67,     5,     3,    67,    69,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    54,
       4,    67,     3,    55,    69,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    54,
      74,   257,   271,   292,    69,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,   301,
     282,    63,   213,   220,    69,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
     194,   190,    -1,    -1,    69,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      -1,    -1,    67,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    -1,    -1,
      67,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63
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
     101,   101,    20,    44,    61,    81,   107,    66,    68,   112,
       4,     5,    67,   120,    23,    67,    69,    69,     3,    69,
      67,    69,    67,    69,    69,   101,   101,    48,    67,    77,
      78,     3,    23,    92,   113,    45,   117,   121,   122,    69,
      54,   101,     7,    69,   101,     7,   101,     7,     7,     3,
      23,    28,    79,    80,     3,    22,    84,    85,    67,    69,
      68,   114,   115,     7,    67,    81,     3,    69,     3,    69,
       3,     3,    11,    24,    54,    55,    56,    57,    58,    59,
      64,    54,    68,    51,    12,    49,    86,    87,    92,   116,
      67,     3,    27,   123,   117,   122,    81,    82,    68,    81,
      82,    81,    82,    81,    82,    81,    82,    81,    82,    81,
      82,    24,    81,     6,    80,     3,    22,    33,    88,    89,
      67,    69,   115,    51,    51,    81,    83,    68,    69,    33,
      12,    27,    93,    94,   117,    81,    61,    82,    67,    69,
      83,    12,    90,    91,    92,     4,    32,    95,    96,    81,
      69,    90,    67,     8,    17,    67,     3,    97,    98,    91,
       4,    54,    67,     3,     4,    68,    98,     3,    99,   100,
      54,    67,    69,    81,   100
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

  case 16:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd );
											pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 17:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd );
									pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 18:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 19:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 20:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 21:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 22:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 23:

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

  case 25:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 31:

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

  case 32:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 33:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
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
			tFilter.m_bExclude = true;
		;}
    break;

  case 35:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 36:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 37:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 38:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 39:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 40:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 41:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 45:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 46:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 47:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 48:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 49:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 50:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 51:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 52:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 53:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 54:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 57:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 60:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 63:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 65:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 67:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 68:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 73:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 74:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
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
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 83:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 84:

    {
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 85:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 89:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 90:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 91:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 107:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 108:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 109:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 110:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 111:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 115:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 116:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 117:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 118:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 119:

    { yyval.m_iValue = 1; ;}
    break;

  case 120:

    { yyval.m_iValue = 0; ;}
    break;

  case 121:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 122:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 123:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 124:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 127:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 128:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 129:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 132:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 133:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 136:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 137:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 138:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 139:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 140:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 141:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 142:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iDeleteID = yyvsp[0].m_iValue;
		;}
    break;

  case 143:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 146:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 148:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 150:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 151:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 154:

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

