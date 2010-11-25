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
     TOK_UPDATE = 300,
     TOK_VALUES = 301,
     TOK_WARNINGS = 302,
     TOK_WEIGHT = 303,
     TOK_WHERE = 304,
     TOK_WITHIN = 305,
     TOK_OR = 306,
     TOK_AND = 307,
     TOK_NE = 308,
     TOK_GTE = 309,
     TOK_LTE = 310,
     TOK_NOT = 311,
     TOK_NEG = 312
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
#define TOK_UPDATE 300
#define TOK_VALUES 301
#define TOK_WARNINGS 302
#define TOK_WEIGHT 303
#define TOK_WHERE 304
#define TOK_WITHIN 305
#define TOK_OR 306
#define TOK_AND 307
#define TOK_NE 308
#define TOK_GTE 309
#define TOK_LTE 310
#define TOK_NOT 311
#define TOK_NEG 312




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
#define YYFINAL  59
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   501

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  71
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  62
/* YYNRULES -- Number of rules. */
#define YYNRULES  162
/* YYNRULES -- Number of states. */
#define YYNSTATES  322

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    54,     2,
      69,    70,    63,    61,    68,    62,     2,    64,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    67,
      57,    55,    58,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    53,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    56,    59,
      60,    65,    66
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    32,    34,    36,    47,    49,
      53,    55,    59,    66,    73,    80,    87,    89,    95,    97,
     101,   102,   104,   107,   109,   113,   118,   122,   126,   130,
     136,   143,   149,   153,   157,   161,   165,   169,   173,   177,
     181,   187,   191,   195,   197,   200,   202,   205,   207,   211,
     212,   214,   218,   219,   221,   227,   228,   230,   234,   236,
     240,   242,   245,   248,   250,   252,   253,   255,   258,   263,
     264,   266,   269,   271,   275,   279,   283,   289,   291,   295,
     299,   301,   303,   305,   308,   311,   315,   319,   323,   327,
     331,   335,   339,   343,   347,   351,   355,   359,   363,   367,
     371,   373,   378,   383,   387,   394,   401,   403,   407,   410,
     412,   414,   416,   421,   423,   425,   427,   429,   431,   433,
     435,   438,   445,   447,   449,   450,   454,   456,   460,   462,
     466,   470,   472,   476,   478,   480,   482,   490,   497,   498,
     501,   503,   507,   511,   513,   515,   518,   520,   522,   525,
     534,   536,   540
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      72,     0,    -1,    73,    -1,    74,    -1,    74,    67,    -1,
     113,    -1,   121,    -1,   109,    -1,   111,    -1,   122,    -1,
     127,    -1,   129,    -1,   130,    -1,    75,    -1,    74,    67,
      75,    -1,    76,    -1,   107,    -1,    36,    77,    21,    79,
      80,    87,    89,    91,    96,    98,    -1,    78,    -1,    77,
      68,    78,    -1,     3,    -1,   104,     7,     3,    -1,     9,
      69,   104,    70,     7,     3,    -1,    29,    69,   104,    70,
       7,     3,    -1,    31,    69,   104,    70,     7,     3,    -1,
      41,    69,   104,    70,     7,     3,    -1,    63,    -1,    15,
      69,    19,     3,    70,    -1,     3,    -1,    79,    68,     3,
      -1,    -1,    81,    -1,    49,    82,    -1,    83,    -1,    82,
      52,    83,    -1,    28,    69,     6,    70,    -1,    23,    55,
      84,    -1,     3,    55,    84,    -1,     3,    56,    84,    -1,
       3,    24,    69,    86,    70,    -1,     3,    65,    24,    69,
      86,    70,    -1,     3,    11,    84,    52,    84,    -1,     3,
      58,    84,    -1,     3,    57,    84,    -1,     3,    59,    84,
      -1,     3,    60,    84,    -1,     3,    55,    85,    -1,     3,
      56,    85,    -1,     3,    58,    85,    -1,     3,    57,    85,
      -1,     3,    11,    85,    52,    85,    -1,     3,    59,    85,
      -1,     3,    60,    85,    -1,     4,    -1,    62,     4,    -1,
       5,    -1,    62,     5,    -1,    84,    -1,    86,    68,    84,
      -1,    -1,    88,    -1,    22,    12,     3,    -1,    -1,    90,
      -1,    50,    22,    33,    12,    93,    -1,    -1,    92,    -1,
      33,    12,    93,    -1,    94,    -1,    93,    68,    94,    -1,
      95,    -1,    95,     8,    -1,    95,    17,    -1,    23,    -1,
       3,    -1,    -1,    97,    -1,    27,     4,    -1,    27,     4,
      68,     4,    -1,    -1,    99,    -1,    32,   100,    -1,   101,
      -1,   100,    68,   101,    -1,     3,    55,     3,    -1,     3,
      55,     4,    -1,     3,    55,    69,   102,    70,    -1,   103,
      -1,   102,    68,   103,    -1,     3,    55,    84,    -1,     3,
      -1,     4,    -1,     5,    -1,    62,   104,    -1,    65,   104,
      -1,   104,    61,   104,    -1,   104,    62,   104,    -1,   104,
      63,   104,    -1,   104,    64,   104,    -1,   104,    57,   104,
      -1,   104,    58,   104,    -1,   104,    54,   104,    -1,   104,
      53,   104,    -1,   104,    60,   104,    -1,   104,    59,   104,
      -1,   104,    55,   104,    -1,   104,    56,   104,    -1,   104,
      52,   104,    -1,   104,    51,   104,    -1,    69,   104,    70,
      -1,   105,    -1,     3,    69,   106,    70,    -1,    24,    69,
     106,    70,    -1,     3,    69,    70,    -1,    31,    69,   104,
      68,   104,    70,    -1,    29,    69,   104,    68,   104,    70,
      -1,   104,    -1,   106,    68,   104,    -1,    38,   108,    -1,
      47,    -1,    40,    -1,    30,    -1,    37,     3,    55,   110,
      -1,    44,    -1,    20,    -1,    84,    -1,    14,    -1,    35,
      -1,   112,    -1,    10,    -1,    39,    43,    -1,   114,    26,
       3,   115,    46,   117,    -1,    25,    -1,    34,    -1,    -1,
      69,   116,    70,    -1,    95,    -1,   116,    68,    95,    -1,
     118,    -1,   117,    68,   118,    -1,    69,   119,    70,    -1,
     120,    -1,   119,    68,   120,    -1,    84,    -1,    85,    -1,
       6,    -1,    16,    21,     3,    49,    23,    55,    84,    -1,
      13,     3,    69,   119,   123,    70,    -1,    -1,    68,   124,
      -1,   125,    -1,   124,    68,   125,    -1,   120,     7,   126,
      -1,     3,    -1,    27,    -1,   128,     3,    -1,    18,    -1,
      17,    -1,    38,    42,    -1,    45,     3,    37,   131,    49,
      23,    55,    84,    -1,   132,    -1,   131,    68,   132,    -1,
       3,    55,    84,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   111,   111,   112,   113,   117,   118,   119,   120,   121,
     122,   123,   124,   130,   131,   135,   136,   140,   155,   156,
     160,   162,   164,   165,   166,   167,   168,   169,   185,   186,
     189,   191,   195,   199,   200,   204,   218,   225,   232,   240,
     248,   257,   261,   265,   269,   273,   277,   278,   279,   280,
     285,   289,   293,   300,   301,   305,   306,   310,   311,   314,
     316,   320,   327,   329,   333,   339,   341,   345,   352,   353,
     357,   358,   359,   363,   364,   367,   369,   373,   378,   385,
     387,   391,   395,   396,   400,   405,   410,   419,   429,   441,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   475,   476,   477,   478,   479,   483,   484,   490,   494,
     495,   496,   502,   511,   512,   513,   527,   528,   529,   533,
     534,   540,   548,   549,   552,   554,   558,   559,   563,   564,
     568,   572,   573,   577,   578,   579,   585,   596,   603,   605,
     609,   614,   618,   626,   627,   633,   641,   642,   648,   654,
     665,   666,   670
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
  "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", "TOK_UPDATE", "TOK_VALUES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "TOK_NOT", "TOK_NEG", "';'", 
  "','", "'('", "')'", "$accept", "request", "statement", 
  "multi_stmt_list", "multi_stmt", "select_from", "select_items_list", 
  "select_item", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "where_item", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "ident_or_id", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "show_clause", "show_variable", "set_clause", 
  "boolean_value", "transact_op", "start_transaction", "insert_into", 
  "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "opt_call_opts_list", "call_opts_list", 
  "call_opt", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", 0
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
     305,   306,   307,   124,    38,    61,   308,    60,    62,   309,
     310,    43,    45,    42,    47,   311,   312,    59,    44,    40,
      41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    71,    72,    72,    72,    73,    73,    73,    73,    73,
      73,    73,    73,    74,    74,    75,    75,    76,    77,    77,
      78,    78,    78,    78,    78,    78,    78,    78,    79,    79,
      80,    80,    81,    82,    82,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    84,    84,    85,    85,    86,    86,    87,
      87,    88,    89,    89,    90,    91,    91,    92,    93,    93,
      94,    94,    94,    95,    95,    96,    96,    97,    97,    98,
      98,    99,   100,   100,   101,   101,   101,   102,   102,   103,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   105,   105,   105,   105,   105,   106,   106,   107,   108,
     108,   108,   109,   110,   110,   110,   111,   111,   111,   112,
     112,   113,   114,   114,   115,   115,   116,   116,   117,   117,
     118,   119,   119,   120,   120,   120,   121,   122,   123,   123,
     124,   124,   125,   126,   126,   127,   128,   128,   129,   130,
     131,   131,   132
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,    10,     1,     3,
       1,     3,     6,     6,     6,     6,     1,     5,     1,     3,
       0,     1,     2,     1,     3,     4,     3,     3,     3,     5,
       6,     5,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     3,     3,     1,     2,     1,     2,     1,     3,     0,
       1,     3,     0,     1,     5,     0,     1,     3,     1,     3,
       1,     2,     2,     1,     1,     0,     1,     2,     4,     0,
       1,     2,     1,     3,     3,     3,     5,     1,     3,     3,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     4,     4,     3,     6,     6,     1,     3,     2,     1,
       1,     1,     4,     1,     1,     1,     1,     1,     1,     1,
       2,     6,     1,     1,     0,     3,     1,     3,     1,     3,
       3,     1,     3,     1,     1,     1,     7,     6,     0,     2,
       1,     3,     3,     1,     1,     2,     1,     1,     2,     8,
       1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   129,     0,   126,     0,   157,   156,   132,   133,   127,
       0,     0,     0,     0,     0,     0,     2,     3,    13,    15,
      16,     7,     8,   128,     5,     0,     6,     9,    10,     0,
      11,    12,     0,     0,    90,    91,    92,     0,     0,     0,
       0,     0,     0,     0,    26,     0,     0,     0,    18,     0,
     110,     0,   121,   120,   158,   119,   118,   130,     0,     1,
       4,     0,   155,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    90,     0,     0,    93,    94,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,   134,
      53,    55,   145,     0,   143,   144,   148,   141,     0,   113,
     116,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     109,    28,    30,    19,    21,   108,   107,   102,   101,   105,
     106,    99,   100,   104,   103,    95,    96,    97,    98,   124,
     123,     0,   125,   122,     0,     0,   160,     0,     0,    54,
      56,     0,     0,     0,     0,   111,     0,     0,   112,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    31,
       0,     0,     0,    74,    73,   136,     0,     0,   142,   149,
     150,   147,     0,   117,     0,    27,     0,     0,     0,     0,
       0,     0,     0,     0,    32,    33,    29,     0,    62,    60,
     162,     0,   161,     0,   135,     0,   131,   138,     0,     0,
     146,    22,   115,    23,   114,    24,    25,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    63,     0,   137,     0,     0,   153,   154,   152,
       0,   151,     0,     0,     0,    37,    46,    38,    47,    43,
      49,    42,    48,    44,    51,    45,    52,     0,    36,     0,
      34,    61,     0,     0,    75,    66,   159,     0,   140,   139,
       0,     0,    57,     0,     0,    35,     0,     0,     0,    79,
      76,   142,    41,     0,    50,     0,    39,     0,     0,    67,
      68,    70,    77,     0,    17,    80,    58,    40,    64,     0,
      71,    72,     0,     0,    81,    82,    69,    78,     0,     0,
      84,    85,     0,    83,     0,     0,    87,     0,     0,    86,
      89,    88
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    15,    16,    17,    18,    19,    47,    48,   122,   168,
     169,   194,   195,   104,   105,   273,   198,   199,   231,   232,
     264,   265,   289,   290,   291,   279,   280,   294,   295,   304,
     305,   315,   316,    49,    50,   111,    20,    56,    21,   143,
      22,    23,    24,    25,   148,   176,   206,   207,   106,   107,
      26,    27,   152,   179,   180,   239,    28,    29,    30,    31,
     145,   146
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -150
static const short yypact[] =
{
     207,  -150,    22,  -150,    -7,  -150,  -150,  -150,  -150,  -150,
      79,    33,    49,    35,    55,    81,  -150,    26,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,    59,  -150,  -150,  -150,    95,
    -150,  -150,    52,   127,    -5,  -150,  -150,    77,    78,    88,
     104,   105,   107,   129,  -150,   129,   129,    -2,  -150,   148,
    -150,    96,  -150,  -150,  -150,  -150,  -150,  -150,   141,  -150,
     -16,   189,  -150,     3,   144,    30,   129,   177,   129,   129,
     129,   129,   126,   128,   145,  -150,  -150,   242,   212,    79,
     213,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,    11,   215,    50,  -150,   150,
    -150,  -150,  -150,    97,  -150,  -150,   158,  -150,   204,  -150,
     378,    41,   262,   225,    45,   202,   222,   282,   129,   129,
    -150,  -150,     4,  -150,  -150,   391,   403,   414,   125,   176,
     176,    43,    43,    43,    43,    76,    76,  -150,  -150,  -150,
    -150,   226,  -150,  -150,   174,    18,  -150,    15,   185,  -150,
    -150,     3,   178,   192,   129,  -150,   243,   179,  -150,   129,
     244,   129,   260,   261,   342,   360,    34,   266,   249,  -150,
       8,   264,   215,  -150,  -150,  -150,    46,   219,   284,   221,
    -150,  -150,     8,   378,   304,  -150,   302,   305,   322,   306,
     307,   112,   256,   258,   276,  -150,  -150,   317,   280,  -150,
    -150,   292,  -150,    15,  -150,     3,   263,  -150,    29,     3,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,     6,   279,     6,
       6,     6,     6,     6,     6,   325,     8,   344,    34,   348,
     345,   335,  -150,     8,  -150,    67,   219,  -150,  -150,  -150,
     284,  -150,   318,   319,     8,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,   300,  -150,   320,
    -150,  -150,   354,   376,   362,  -150,  -150,     3,  -150,  -150,
       8,    12,  -150,    75,     8,  -150,   379,    15,   404,   375,
    -150,  -150,  -150,   420,  -150,     8,  -150,    82,    15,   341,
    -150,    13,   358,   424,  -150,  -150,  -150,  -150,   341,    15,
    -150,  -150,   475,   425,   413,  -150,  -150,  -150,     2,   424,
    -150,  -150,   479,  -150,   428,    86,  -150,     8,   479,  -150,
    -150,  -150
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -150,  -150,  -150,  -150,   426,  -150,  -150,   405,  -150,  -150,
    -150,  -150,   257,   -95,   -58,   214,  -150,  -150,  -150,  -150,
    -150,  -150,   199,   190,  -134,  -150,  -150,  -150,  -150,  -150,
     181,  -150,   173,   -42,  -150,   427,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,   265,   287,  -149,
    -150,  -150,  -150,  -150,   285,  -150,  -150,  -150,  -150,  -150,
    -150,   321
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -21
static const short yytable[] =
{
     142,    75,   178,    76,    77,   310,   311,   100,   101,   102,
     100,   101,   100,   175,    33,   100,   -20,   101,   173,    78,
      10,   300,    97,   110,   112,    32,   110,   115,   116,   117,
     301,   139,   237,    72,    35,    36,    51,   191,   174,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   166,    39,   140,   238,   192,    58,    73,
     240,    74,   193,   -20,    65,   103,    79,   171,   103,   234,
     141,   312,   167,   141,   283,   200,   164,   165,    57,    52,
      52,    59,    34,    35,    36,    61,   172,   210,    37,    53,
      53,    54,    43,    60,    38,    45,    55,    55,    62,    46,
     109,   149,   150,    39,    91,    92,    93,    94,    40,   154,
      41,   155,   183,   154,   203,   158,   204,   186,   281,   188,
      42,    63,   242,   217,   245,   247,   249,   251,   253,   255,
      64,   258,    72,    35,    36,   267,   218,   268,   266,    93,
      94,    43,    44,   285,    45,   286,    66,    67,    46,   272,
     285,    95,   297,    39,   318,    80,   319,    68,    73,   243,
      74,   246,   248,   250,   252,   254,   256,   219,   220,   221,
     222,   223,   224,    69,    70,   282,    71,   225,    96,   272,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
     296,    43,    99,   108,    45,    65,   113,   118,    46,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,   284,   119,   121,   124,     1,   144,   147,
       2,     3,   320,     4,     5,     6,   151,   153,   157,   170,
     149,   177,     7,    87,    88,    89,    90,    91,    92,    93,
      94,     8,     9,    10,    11,    12,    13,   182,   181,   185,
     184,   187,    14,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   189,   190,   196,
     159,   197,   160,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   201,   205,   209,
     161,   208,   162,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   211,   213,   215,
     216,   226,   120,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   227,   228,   229,
     230,   236,   156,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   233,   244,   257,
     259,   261,   163,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   262,   263,   274,
     270,   271,   212,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   276,   277,   278,
     275,   288,   214,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   293,   292,   299,
     159,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,   150,   302,   303,   161,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,   307,
     308,   309,   314,   317,   123,   260,    98,   298,   287,   306,
     313,   321,   235,   202,   241,   114,     0,     0,     0,     0,
       0,   269
};

static const short yycheck[] =
{
      95,    43,   151,    45,    46,     3,     4,     4,     5,     6,
       4,     5,     4,   147,    21,     4,    21,     5,     3,    21,
      36,     8,    38,    65,    66,     3,    68,    69,    70,    71,
      17,    20,     3,     3,     4,     5,     3,     3,    23,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    49,    24,    44,    27,    23,     3,    29,
     209,    31,    28,    68,    69,    62,    68,    49,    62,   203,
      62,    69,    68,    62,    62,   170,   118,   119,    43,    30,
      30,     0,     3,     4,     5,    26,    68,   182,     9,    40,
      40,    42,    62,    67,    15,    65,    47,    47,     3,    69,
      70,     4,     5,    24,    61,    62,    63,    64,    29,    68,
      31,    70,   154,    68,    68,    70,    70,   159,   267,   161,
      41,    69,   217,    11,   219,   220,   221,   222,   223,   224,
       3,   226,     3,     4,     5,    68,    24,    70,   233,    63,
      64,    62,    63,    68,    65,    70,    69,    69,    69,   244,
      68,    55,    70,    24,    68,     7,    70,    69,    29,   217,
      31,   219,   220,   221,   222,   223,   224,    55,    56,    57,
      58,    59,    60,    69,    69,   270,    69,    65,    37,   274,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
     285,    62,     3,    49,    65,    69,    19,    69,    69,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,   271,    69,     3,     3,    10,     3,    69,
      13,    14,   317,    16,    17,    18,    68,    23,     3,    55,
       4,    46,    25,    57,    58,    59,    60,    61,    62,    63,
      64,    34,    35,    36,    37,    38,    39,    55,    70,    70,
       7,     7,    45,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     7,     7,     3,
      68,    22,    70,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    23,    69,    68,
      68,     7,    70,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     3,     3,     3,
       3,    55,    70,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    69,    52,    12,
      50,    68,    70,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    55,    69,    24,
       6,     3,    70,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    22,    33,    69,
      52,    52,    70,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    33,    12,    27,
      70,    12,    70,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    32,     4,    68,
      68,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,     5,    68,     3,    68,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     4,
      55,    68,     3,    55,    79,   228,    60,   288,   274,   299,
     309,   318,   205,   172,   209,    68,    -1,    -1,    -1,    -1,
      -1,   236
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    10,    13,    14,    16,    17,    18,    25,    34,    35,
      36,    37,    38,    39,    45,    72,    73,    74,    75,    76,
     107,   109,   111,   112,   113,   114,   121,   122,   127,   128,
     129,   130,     3,    21,     3,     4,     5,     9,    15,    24,
      29,    31,    41,    62,    63,    65,    69,    77,    78,   104,
     105,     3,    30,    40,    42,    47,   108,    43,     3,     0,
      67,    26,     3,    69,     3,    69,    69,    69,    69,    69,
      69,    69,     3,    29,    31,   104,   104,   104,    21,    68,
       7,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    55,    37,    38,    75,     3,
       4,     5,     6,    62,    84,    85,   119,   120,    49,    70,
     104,   106,   104,    19,   106,   104,   104,   104,    69,    69,
      70,     3,    79,    78,     3,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,    20,
      44,    62,    84,   110,     3,   131,   132,    69,   115,     4,
       5,    68,   123,    23,    68,    70,    70,     3,    70,    68,
      70,    68,    70,    70,   104,   104,    49,    68,    80,    81,
      55,    49,    68,     3,    23,    95,   116,    46,   120,   124,
     125,    70,    55,   104,     7,    70,   104,     7,   104,     7,
       7,     3,    23,    28,    82,    83,     3,    22,    87,    88,
      84,    23,   132,    68,    70,    69,   117,   118,     7,    68,
      84,     3,    70,     3,    70,     3,     3,    11,    24,    55,
      56,    57,    58,    59,    60,    65,    55,    69,    52,    12,
      50,    89,    90,    55,    95,   119,    68,     3,    27,   126,
     120,   125,    84,    85,    69,    84,    85,    84,    85,    84,
      85,    84,    85,    84,    85,    84,    85,    24,    84,     6,
      83,     3,    22,    33,    91,    92,    84,    68,    70,   118,
      52,    52,    84,    86,    69,    70,    33,    12,    27,    96,
      97,   120,    84,    62,    85,    68,    70,    86,    12,    93,
      94,    95,     4,    32,    98,    99,    84,    70,    93,    68,
       8,    17,    68,     3,   100,   101,    94,     4,    55,    68,
       3,     4,    69,   101,     3,   102,   103,    55,    68,    70,
      84,   103
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

  case 13:

    { pParser->PushQuery(); ;}
    break;

  case 14:

    { pParser->PushQuery(); ;}
    break;

  case 17:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 20:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd );
											pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 21:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd );
									pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 22:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 23:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 24:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 25:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 26:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 27:

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

  case 29:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 35:

    {
			if ( pParser->m_bGotQuery )
			{
				yyerror ( pParser, "too many MATCH() clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sQuery = yyvsp[-1].m_sValue;
				pParser->m_pQuery->m_sRawQuery = yyvsp[-1].m_sValue;
				pParser->m_bGotQuery = true;
			}
		;}
    break;

  case 36:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 37:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 38:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
		;}
    break;

  case 39:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 40:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 41:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 42:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 43:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 44:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 45:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 49:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 50:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 51:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 52:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 53:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 54:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 55:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 56:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 57:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 58:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 61:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 64:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 67:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 69:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 71:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 72:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 77:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 78:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 85:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 87:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 88:

    {
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 89:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 93:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 94:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 107:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 108:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 109:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 111:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 112:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 113:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 114:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 115:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 119:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 120:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 121:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 122:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 123:

    { yyval.m_iValue = 1; ;}
    break;

  case 124:

    { yyval.m_iValue = 0; ;}
    break;

  case 125:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 126:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 127:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 128:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 131:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 132:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 133:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 136:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 137:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 140:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 141:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 142:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 143:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 144:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 145:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 146:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iDeleteID = yyvsp[0].m_iValue;
		;}
    break;

  case 147:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 150:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 152:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 154:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 155:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 158:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 159:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_UPDATE;
			tStmt.m_sIndex = yyvsp[-6].m_sValue;	
			tStmt.m_tUpdate.m_dDocids.Add ( (SphDocID_t) yyvsp[0].m_iValue );
			tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );
		;}
    break;

  case 162:

    {
			CSphAttrUpdate & tUpd = pParser->m_pStmt->m_tUpdate;
			CSphColumnInfo & tAttr = tUpd.m_dAttrs.Add();
			tAttr.m_sName = yyvsp[-2].m_sValue;
			tAttr.m_sName.ToLower();
			tAttr.m_eAttrType = SPH_ATTR_INTEGER; // sorry, ints only for now, riding on legacy shit!
			tUpd.m_dPool.Add ( (DWORD) yyvsp[0].m_iValue );
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

