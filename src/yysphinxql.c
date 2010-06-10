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
     TOK_BETWEEN = 265,
     TOK_BY = 266,
     TOK_COUNT = 267,
     TOK_DESC = 268,
     TOK_DELETE = 269,
     TOK_DISTINCT = 270,
     TOK_FROM = 271,
     TOK_GROUP = 272,
     TOK_LIMIT = 273,
     TOK_IN = 274,
     TOK_INSERT = 275,
     TOK_INTO = 276,
     TOK_ID = 277,
     TOK_MATCH = 278,
     TOK_MAX = 279,
     TOK_META = 280,
     TOK_MIN = 281,
     TOK_OPTION = 282,
     TOK_ORDER = 283,
     TOK_REPLACE = 284,
     TOK_SELECT = 285,
     TOK_SHOW = 286,
     TOK_STATUS = 287,
     TOK_SUM = 288,
     TOK_VALUES = 289,
     TOK_WARNINGS = 290,
     TOK_WEIGHT = 291,
     TOK_WITHIN = 292,
     TOK_WHERE = 293,
     TOK_SET = 294,
     TOK_COMMIT = 295,
     TOK_ROLLBACK = 296,
     TOK_START = 297,
     TOK_TRANSACTION = 298,
     TOK_BEGIN = 299,
     TOK_TRUE = 300,
     TOK_FALSE = 301,
     TOK_OR = 302,
     TOK_AND = 303,
     TOK_NE = 304,
     TOK_GTE = 305,
     TOK_LTE = 306,
     TOK_NOT = 307,
     TOK_NEG = 308
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST_INT 259
#define TOK_CONST_FLOAT 260
#define TOK_QUOTED_STRING 261
#define TOK_AS 262
#define TOK_ASC 263
#define TOK_AVG 264
#define TOK_BETWEEN 265
#define TOK_BY 266
#define TOK_COUNT 267
#define TOK_DESC 268
#define TOK_DELETE 269
#define TOK_DISTINCT 270
#define TOK_FROM 271
#define TOK_GROUP 272
#define TOK_LIMIT 273
#define TOK_IN 274
#define TOK_INSERT 275
#define TOK_INTO 276
#define TOK_ID 277
#define TOK_MATCH 278
#define TOK_MAX 279
#define TOK_META 280
#define TOK_MIN 281
#define TOK_OPTION 282
#define TOK_ORDER 283
#define TOK_REPLACE 284
#define TOK_SELECT 285
#define TOK_SHOW 286
#define TOK_STATUS 287
#define TOK_SUM 288
#define TOK_VALUES 289
#define TOK_WARNINGS 290
#define TOK_WEIGHT 291
#define TOK_WITHIN 292
#define TOK_WHERE 293
#define TOK_SET 294
#define TOK_COMMIT 295
#define TOK_ROLLBACK 296
#define TOK_START 297
#define TOK_TRANSACTION 298
#define TOK_BEGIN 299
#define TOK_TRUE 300
#define TOK_FALSE 301
#define TOK_OR 302
#define TOK_AND 303
#define TOK_NE 304
#define TOK_GTE 305
#define TOK_LTE 306
#define TOK_NOT 307
#define TOK_NEG 308




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif


// some helpers
#include <float.h> // for FLT_MAX

static void AddFloatRangeFilter ( SqlParser_t * pParser, const CSphString & sAttr, float fMin, float fMax )
{
	CSphFilterSettings tFilter;
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_FLOATRANGE;
	tFilter.m_fMinValue = fMin;
	tFilter.m_fMaxValue = fMax;
	pParser->m_pQuery->m_dFilters.Add ( tFilter );
}

static void AddUintRangeFilter ( SqlParser_t * pParser, const CSphString & sAttr, DWORD uMin, DWORD uMax )
{
	CSphFilterSettings tFilter;
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_RANGE;
	tFilter.m_uMinValue = uMin;
	tFilter.m_uMaxValue = uMax;
	pParser->m_pQuery->m_dFilters.Add ( tFilter );
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
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   390

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  43
/* YYNRULES -- Number of rules. */
#define YYNRULES  127
/* YYNRULES -- Number of states. */
#define YYNSTATES  256

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   308

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      62,    63,    57,    55,    61,    56,     2,    58,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      51,    49,    52,     2,     2,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    50,    53,    54,    59,    60
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    26,
      28,    32,    34,    38,    45,    52,    59,    66,    68,    74,
      76,    80,    81,    83,    86,    88,    92,    97,   101,   105,
     109,   115,   122,   128,   132,   136,   140,   144,   148,   152,
     156,   160,   166,   170,   174,   176,   180,   181,   183,   187,
     188,   190,   196,   197,   199,   203,   205,   209,   211,   214,
     217,   218,   220,   223,   228,   229,   231,   234,   236,   240,
     244,   248,   250,   252,   254,   257,   260,   264,   268,   272,
     276,   280,   284,   288,   292,   296,   300,   304,   308,   312,
     314,   319,   324,   328,   335,   342,   344,   348,   351,   353,
     355,   357,   362,   364,   366,   368,   370,   372,   374,   376,
     379,   381,   383,   390,   391,   395,   397,   399,   403,   407,
     409,   413,   417,   419,   423,   425,   427,   429
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      65,     0,    -1,    66,    -1,    92,    -1,    99,    -1,   106,
      -1,    94,    -1,    96,    -1,    30,    67,    16,    69,    70,
      75,    77,    79,    83,    85,    -1,    68,    -1,    67,    61,
      68,    -1,     3,    -1,    89,     7,     3,    -1,     9,    62,
      89,    63,     7,     3,    -1,    24,    62,    89,    63,     7,
       3,    -1,    26,    62,    89,    63,     7,     3,    -1,    33,
      62,    89,    63,     7,     3,    -1,    57,    -1,    12,    62,
      15,     3,    63,    -1,     3,    -1,    69,    61,     3,    -1,
      -1,    71,    -1,    38,    72,    -1,    73,    -1,    72,    48,
      73,    -1,    23,    62,     6,    63,    -1,    22,    49,     4,
      -1,     3,    49,     4,    -1,     3,    50,     4,    -1,     3,
      19,    62,    74,    63,    -1,     3,    59,    19,    62,    74,
      63,    -1,     3,    10,     4,    48,     4,    -1,     3,    52,
       4,    -1,     3,    51,     4,    -1,     3,    53,     4,    -1,
       3,    54,     4,    -1,     3,    49,     5,    -1,     3,    50,
       5,    -1,     3,    52,     5,    -1,     3,    51,     5,    -1,
       3,    10,     5,    48,     5,    -1,     3,    53,     5,    -1,
       3,    54,     5,    -1,     4,    -1,    74,    61,     4,    -1,
      -1,    76,    -1,    17,    11,     3,    -1,    -1,    78,    -1,
      37,    17,    28,    11,    81,    -1,    -1,    80,    -1,    28,
      11,    81,    -1,    82,    -1,    81,    61,    82,    -1,     3,
      -1,     3,     8,    -1,     3,    13,    -1,    -1,    84,    -1,
      18,     4,    -1,    18,     4,    61,     4,    -1,    -1,    86,
      -1,    27,    87,    -1,    88,    -1,    87,    61,    88,    -1,
       3,    49,     3,    -1,     3,    49,     4,    -1,     3,    -1,
       4,    -1,     5,    -1,    56,    89,    -1,    59,    89,    -1,
      89,    55,    89,    -1,    89,    56,    89,    -1,    89,    57,
      89,    -1,    89,    58,    89,    -1,    89,    51,    89,    -1,
      89,    52,    89,    -1,    89,    54,    89,    -1,    89,    53,
      89,    -1,    89,    49,    89,    -1,    89,    50,    89,    -1,
      89,    48,    89,    -1,    89,    47,    89,    -1,    62,    89,
      63,    -1,    90,    -1,     3,    62,    91,    63,    -1,    19,
      62,    91,    63,    -1,     3,    62,    63,    -1,    26,    62,
      89,    61,    89,    63,    -1,    24,    62,    89,    61,    89,
      63,    -1,    89,    -1,    91,    61,    89,    -1,    31,    93,
      -1,    35,    -1,    32,    -1,    25,    -1,    39,     3,    49,
      95,    -1,    45,    -1,    46,    -1,     4,    -1,    40,    -1,
      41,    -1,    97,    -1,    44,    -1,    42,    43,    -1,    20,
      -1,    29,    -1,    98,    21,     3,   100,    34,   102,    -1,
      -1,    62,   101,    63,    -1,     3,    -1,    22,    -1,   101,
      61,     3,    -1,   101,    61,    22,    -1,   103,    -1,   102,
      61,   103,    -1,    62,   104,    63,    -1,   105,    -1,   104,
      61,   105,    -1,     4,    -1,     5,    -1,     6,    -1,    14,
      16,     3,    38,    22,    49,     4,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    98,    98,    99,   100,   101,   102,   103,   109,   124,
     125,   129,   130,   131,   132,   133,   134,   135,   136,   151,
     152,   155,   157,   161,   165,   166,   170,   183,   191,   199,
     208,   216,   225,   229,   233,   237,   241,   245,   246,   247,
     248,   253,   257,   261,   268,   269,   272,   274,   278,   285,
     287,   291,   297,   299,   303,   310,   311,   315,   316,   317,
     320,   322,   326,   331,   338,   340,   344,   348,   349,   353,
     354,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     382,   383,   384,   385,   386,   390,   391,   397,   401,   402,
     403,   409,   418,   419,   420,   434,   435,   436,   440,   441,
     447,   448,   452,   459,   461,   465,   466,   467,   468,   472,
     473,   477,   481,   482,   486,   487,   488,   494
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_CONST_INT", 
  "TOK_CONST_FLOAT", "TOK_QUOTED_STRING", "TOK_AS", "TOK_ASC", "TOK_AVG", 
  "TOK_BETWEEN", "TOK_BY", "TOK_COUNT", "TOK_DESC", "TOK_DELETE", 
  "TOK_DISTINCT", "TOK_FROM", "TOK_GROUP", "TOK_LIMIT", "TOK_IN", 
  "TOK_INSERT", "TOK_INTO", "TOK_ID", "TOK_MATCH", "TOK_MAX", "TOK_META", 
  "TOK_MIN", "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", "TOK_SELECT", 
  "TOK_SHOW", "TOK_STATUS", "TOK_SUM", "TOK_VALUES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WITHIN", "TOK_WHERE", "TOK_SET", "TOK_COMMIT", 
  "TOK_ROLLBACK", "TOK_START", "TOK_TRANSACTION", "TOK_BEGIN", "TOK_TRUE", 
  "TOK_FALSE", "TOK_OR", "TOK_AND", "'='", "TOK_NE", "'<'", "'>'", 
  "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "TOK_NOT", "TOK_NEG", 
  "','", "'('", "')'", "$accept", "statement", "select_from", 
  "select_items_list", "select_item", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "expr", "function", "arglist", "show_clause", "show_variable", 
  "set_clause", "boolean_value", "transact_op", "start_transaction", 
  "insert_or_replace_tok", "insert_into", "opt_insert_cols_list", 
  "schema_list", "opt_insert_cols_set", "opt_insert_cols", 
  "insert_vals_list", "insert_val", "delete_from", 0
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,    61,
     304,    60,    62,   305,   306,    43,    45,    42,    47,   307,
     308,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    64,    65,    65,    65,    65,    65,    65,    66,    67,
      67,    68,    68,    68,    68,    68,    68,    68,    68,    69,
      69,    70,    70,    71,    72,    72,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    74,    74,    75,    75,    76,    77,
      77,    78,    79,    79,    80,    81,    81,    82,    82,    82,
      83,    83,    84,    84,    85,    85,    86,    87,    87,    88,
      88,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      90,    90,    90,    90,    90,    91,    91,    92,    93,    93,
      93,    94,    95,    95,    95,    96,    96,    96,    97,    97,
      98,    98,    99,   100,   100,   101,   101,   101,   101,   102,
     102,   103,   104,   104,   105,   105,   105,   106
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,    10,     1,
       3,     1,     3,     6,     6,     6,     6,     1,     5,     1,
       3,     0,     1,     2,     1,     3,     4,     3,     3,     3,
       5,     6,     5,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     3,     3,     1,     3,     0,     1,     3,     0,
       1,     5,     0,     1,     3,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       4,     4,     3,     6,     6,     1,     3,     2,     1,     1,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     1,     6,     0,     3,     1,     1,     3,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     7
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   110,   111,     0,     0,     0,   105,   106,     0,
     108,     0,     2,     3,     6,     7,   107,     0,     4,     5,
       0,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,    17,     0,     0,     0,     9,     0,    89,   100,    99,
      98,    97,     0,   109,     1,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    71,     0,     0,    74,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   113,     0,    92,    95,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    88,
      19,    21,    10,    12,    87,    86,    84,    85,    80,    81,
      83,    82,    76,    77,    78,    79,   104,   102,   103,   101,
       0,     0,     0,     0,    90,     0,     0,    91,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    46,    22,   115,
     116,     0,     0,     0,    96,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    20,     0,    49,
      47,     0,   114,     0,   112,   119,   127,    13,    94,    14,
      93,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,    50,   117,
     118,   124,   125,   126,     0,   122,     0,     0,     0,     0,
      28,    37,    29,    38,    34,    40,    33,    39,    35,    42,
      36,    43,     0,    27,     0,    25,    48,     0,     0,    60,
      53,     0,   121,   120,     0,     0,    44,     0,     0,    26,
       0,     0,     0,    64,    61,   123,    32,    41,     0,    30,
       0,     0,    57,    54,    55,    62,     0,     8,    65,    45,
      31,    51,    58,    59,     0,     0,     0,    66,    67,    56,
      63,     0,     0,    69,    70,    68
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    11,    12,    34,    35,    91,   127,   128,   145,   146,
     217,   149,   150,   177,   178,   209,   210,   233,   234,   223,
     224,   237,   238,   247,   248,    36,    37,    80,    13,    41,
      14,   109,    15,    16,    17,    18,   111,   131,   154,   155,
     184,   185,    19
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -43
static const short yypact[] =
{
     115,    -9,   -43,   -43,    47,    37,    24,   -43,   -43,   -32,
     -43,    30,   -43,   -43,   -43,   -43,   -43,    27,   -43,   -43,
      42,   -15,   -43,   -43,    -2,    16,    20,    65,    68,    85,
      72,   -43,    72,    72,   -12,   -43,    63,   -43,   -43,   -43,
     -43,   -43,    40,   -43,   -43,    98,   120,     5,    72,   108,
      72,    72,    72,    72,   146,   161,   162,   -43,   -43,   147,
     172,    47,   189,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,     8,   163,   129,   -43,   262,
       2,   164,   206,    18,   113,   130,   181,    72,    72,   -43,
     -43,   -33,   -43,   -43,   273,   283,   291,   291,    29,    29,
      29,    29,   -42,   -42,   -43,   -43,   -43,   -43,   -43,   -43,
      10,   192,   191,    72,   -43,   234,   179,   -43,    72,   236,
      72,   250,   251,   232,   247,     3,   256,   243,   -43,   -43,
     -43,    34,   212,   271,   262,   274,   -43,   198,   288,   215,
     289,   303,    89,   227,   245,   302,   -43,   -43,   340,   315,
     -43,    52,   -43,    88,   292,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,   121,   293,   128,   132,   145,   148,   168,
     185,   335,   352,   351,     3,   355,   342,   332,   -43,   -43,
     -43,   -43,   -43,   -43,    39,   -43,   212,   313,   314,   359,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   304,   -43,   301,   -43,   -43,   337,   356,   350,
     -43,    88,   -43,   -43,   365,   366,   -43,    44,   359,   -43,
     361,   367,   369,   347,   -43,   -43,   -43,   -43,   371,   -43,
      61,   367,     6,   316,   -43,   317,   373,   -43,   -43,   -43,
     -43,   316,   -43,   -43,   367,   375,   331,   320,   -43,   -43,
     -43,   203,   373,   -43,   -43,   -43
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -43,   -43,   -43,   -43,   321,   -43,   -43,   -43,   -43,   209,
     166,   -43,   -43,   -43,   -43,   -43,   -43,   154,   142,   -43,
     -43,   -43,   -43,   -43,   135,   -30,   -43,   338,   -43,   -43,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   204,
     -43,   178,   -43
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -12
static const short yytable[] =
{
      57,   -11,    58,    59,    60,   125,   142,    20,    54,    22,
      23,    43,   106,   129,   242,    73,    74,    79,    81,   243,
      79,    84,    85,    86,    26,   143,   144,    42,   126,    55,
      44,    56,   130,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,    46,   -11,    47,    45,    61,
      21,    22,    23,   107,   108,   179,    24,   123,   124,    25,
      48,    30,    38,   113,    32,   114,    26,    33,    78,    39,
      62,    27,    40,    28,   180,    54,    22,    23,    49,   113,
      29,   117,    50,   134,    71,    72,    73,    74,   137,    75,
     139,    26,   181,   182,   183,   151,    55,   152,    56,   163,
     211,    76,   212,    30,    31,   228,    32,   229,   164,    33,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,   228,    82,   240,   187,   188,    51,    30,     1,
      52,    32,   190,   191,    33,     2,   192,   193,   165,   166,
     167,   168,   169,   170,     3,     4,     5,    53,   171,   194,
     195,   112,   196,   197,     6,     7,     8,     9,    77,    10,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,   198,   199,   118,    90,   119,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,   200,
     201,   120,    93,   121,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,   253,   254,    47,   116,
      89,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    87,    88,   110,   132,   115,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
     133,   135,   136,   138,   122,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,   140,   141,   147,
     148,   158,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,   153,   156,   172,   157,   160,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,   159,   161,   118,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,   162,   173,   120,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    67,    68,    69,    70,    71,    72,    73,    74,
     174,   175,   176,   186,   202,   189,   203,   204,   206,   207,
     208,   214,   215,   216,   219,   220,   218,   221,   222,   226,
     232,   227,   231,   235,   236,   239,   246,   244,   245,   250,
     251,   252,    92,   205,   230,   241,   249,   255,    83,   225,
     213
};

static const unsigned char yycheck[] =
{
      30,    16,    32,    33,    16,    38,     3,    16,     3,     4,
       5,    43,     4,     3,     8,    57,    58,    47,    48,    13,
      50,    51,    52,    53,    19,    22,    23,     3,    61,    24,
       0,    26,    22,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,     3,    61,    62,    21,    61,
       3,     4,     5,    45,    46,     3,     9,    87,    88,    12,
      62,    56,    25,    61,    59,    63,    19,    62,    63,    32,
       7,    24,    35,    26,    22,     3,     4,     5,    62,    61,
      33,    63,    62,   113,    55,    56,    57,    58,   118,    49,
     120,    19,     4,     5,     6,    61,    24,    63,    26,    10,
      61,     3,    63,    56,    57,    61,    59,    63,    19,    62,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    61,    15,    63,     4,     5,    62,    56,    14,
      62,    59,     4,     5,    62,    20,     4,     5,    49,    50,
      51,    52,    53,    54,    29,    30,    31,    62,    59,     4,
       5,    22,     4,     5,    39,    40,    41,    42,    38,    44,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,     4,     5,    61,     3,    63,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,     4,
       5,    61,     3,    63,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,     3,     4,    62,     3,
      63,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    62,    62,    62,    34,    63,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      49,     7,    63,     7,    63,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,     7,     7,     3,
      17,    63,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    62,     4,    49,     3,    63,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     3,     3,    61,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,     3,    62,    61,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    51,    52,    53,    54,    55,    56,    57,    58,
      48,    11,    37,    61,    19,    62,     4,     6,     3,    17,
      28,    48,    48,     4,    63,    28,    62,    11,    18,     4,
       3,     5,    11,     4,    27,     4,     3,    61,    61,     4,
      49,    61,    61,   174,   218,   231,   244,   252,    50,   211,
     186
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    20,    29,    30,    31,    39,    40,    41,    42,
      44,    65,    66,    92,    94,    96,    97,    98,    99,   106,
      16,     3,     4,     5,     9,    12,    19,    24,    26,    33,
      56,    57,    59,    62,    67,    68,    89,    90,    25,    32,
      35,    93,     3,    43,     0,    21,     3,    62,    62,    62,
      62,    62,    62,    62,     3,    24,    26,    89,    89,    89,
      16,    61,     7,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    49,     3,    38,    63,    89,
      91,    89,    15,    91,    89,    89,    89,    62,    62,    63,
       3,    69,    68,     3,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,     4,    45,    46,    95,
      62,   100,    22,    61,    63,    63,     3,    63,    61,    63,
      61,    63,    63,    89,    89,    38,    61,    70,    71,     3,
      22,   101,    34,    49,    89,     7,    63,    89,     7,    89,
       7,     7,     3,    22,    23,    72,    73,     3,    17,    75,
      76,    61,    63,    62,   102,   103,     4,     3,    63,     3,
      63,     3,     3,    10,    19,    49,    50,    51,    52,    53,
      54,    59,    49,    62,    48,    11,    37,    77,    78,     3,
      22,     4,     5,     6,   104,   105,    61,     4,     5,    62,
       4,     5,     4,     5,     4,     5,     4,     5,     4,     5,
       4,     5,    19,     4,     6,    73,     3,    17,    28,    79,
      80,    61,    63,   103,    48,    48,     4,    74,    62,    63,
      28,    11,    18,    83,    84,   105,     4,     5,    61,    63,
      74,    11,     3,    81,    82,     4,    27,    85,    86,     4,
      63,    81,     8,    13,    61,    61,     3,    87,    88,    82,
       4,    49,    61,     3,     4,    88
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
int yyparse ( SqlParser_t * pParser );
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
yyparse ( SqlParser_t * pParser )
#else
int
yyparse (pParser)
     SqlParser_t * pParser ;
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
        case 8:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 11:

    { pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 12:

    { pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 13:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 14:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 15:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 16:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 17:

    { pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 18:

    {
			if ( !pParser->m_pQuery->m_sGroupDistinct.IsEmpty() )
			{
				yyerror ( pParser, "too many COUNT(DISTINCT) clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sGroupDistinct = yyvsp[-1].m_sValue;
			}
		;}
    break;

  case 20:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 26:

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

  case 27:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 28:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 29:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 30:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 31:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-2].m_dValues;
			tFilter.m_bExclude = true;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 32:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 33:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 34:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 35:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 36:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 40:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 41:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 42:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 43:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 44:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 45:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 48:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 51:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 54:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 56:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 58:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 59:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 62:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 63:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 69:

    { if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 70:

    { if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 74:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 75:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 76:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 77:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 78:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 79:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 80:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 81:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 82:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 83:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 84:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 85:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 86:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 87:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 88:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 90:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 91:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 92:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 93:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 94:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 98:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 99:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 100:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 101:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 102:

    { yyval.m_iValue = 1; ;}
    break;

  case 103:

    { yyval.m_iValue = 0; ;}
    break;

  case 104:

    {
			yyval = yyvsp[0];
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 105:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 106:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 107:

    { pParser->m_pStmt->m_eStmt = STMT_STARTTRANSACTION; ;}
    break;

  case 110:

    { yyval = yyvsp[0]; yyval.m_eStmt = STMT_INSERT; ;}
    break;

  case 111:

    { yyval = yyvsp[0]; yyval.m_eStmt = STMT_REPLACE; ;}
    break;

  case 112:

    {
			pParser->m_pStmt->m_eStmt = yyval.m_eStmt;
			pParser->m_pStmt->m_sInsertIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 115:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 116:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 117:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 118:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 121:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 122:

    { pParser->m_pStmt->m_dInsertValues.Add ( yyvsp[0].m_tInsval ); ;}
    break;

  case 123:

    { pParser->m_pStmt->m_dInsertValues.Add ( yyvsp[0].m_tInsval ); ;}
    break;

  case 124:

    { yyval.m_tInsval.m_iType = TOK_CONST_INT; yyval.m_tInsval.m_iVal = yyvsp[0].m_iValue; ;}
    break;

  case 125:

    { yyval.m_tInsval.m_iType = TOK_CONST_FLOAT; yyval.m_tInsval.m_fVal = yyvsp[0].m_fValue; ;}
    break;

  case 126:

    { yyval.m_tInsval.m_iType = TOK_QUOTED_STRING; yyval.m_tInsval.m_sVal = yyvsp[0].m_sValue; ;}
    break;

  case 127:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sDeleteIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iDeleteID = yyvsp[0].m_iValue;
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

	      if (yycount < 5)
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

