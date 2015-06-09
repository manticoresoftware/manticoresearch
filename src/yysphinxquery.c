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
     TOK_KEYWORD = 258,
     TOK_NEAR = 259,
     TOK_INT = 260,
     TOK_FLOAT = 261,
     TOK_FIELDLIMIT = 262,
     TOK_ZONE = 263,
     TOK_ZONESPAN = 264,
     TOK_BEFORE = 265,
     TOK_SENTENCE = 266,
     TOK_PARAGRAPH = 267,
     TOK_MAYBE = 268
   };
#endif
#define TOK_KEYWORD 258
#define TOK_NEAR 259
#define TOK_INT 260
#define TOK_FLOAT 261
#define TOK_FIELDLIMIT 262
#define TOK_ZONE 263
#define TOK_ZONESPAN 264
#define TOK_BEFORE 265
#define TOK_SENTENCE 266
#define TOK_PARAGRAPH 267
#define TOK_MAYBE 268




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#endif


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

typedef union YYSTYPE {
	XQNode_t *		pNode;			// tree node
	struct
	{
		int			iValue;
		int			iStrIndex;
		float		fValue;
	} tInt;
	struct							// field spec
	{
		FieldMask_t	dMask;			// acceptable fields mask
		int			iMaxPos;		// max allowed position within field
	} tFieldLimit;
	int				iZoneVec;
} YYSTYPE;
/* Line 186 of yacc.c.  */

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
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   159

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  23
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  14
/* YYNRULES -- Number of rules. */
#define YYNRULES  49
/* YYNRULES -- Number of states. */
#define YYNSTATES  79

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   268

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    15,    17,     2,     2,     2,     2,     2,
      20,    21,     2,     2,     2,    14,     2,    19,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    22,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    16,     2,    18,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    11,    13,    15,    17,
      19,    23,    27,    29,    33,    37,    40,    45,    50,    52,
      54,    56,    59,    64,    69,    74,    78,    84,    90,    96,
     100,   105,   107,   109,   111,   114,   118,   122,   126,   130,
     132,   136,   138,   141,   143,   145,   147,   149,   151,   153
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      24,     0,    -1,    25,    -1,    27,    -1,    25,    27,    -1,
      -1,     7,    -1,     8,    -1,     9,    -1,    28,    -1,    27,
      10,    28,    -1,    27,     4,    28,    -1,    29,    -1,    26,
      14,    29,    -1,    26,    15,    29,    -1,    26,    30,    -1,
      29,    16,    26,    30,    -1,    29,    13,    26,    30,    -1,
      31,    -1,    32,    -1,    33,    -1,    17,    17,    -1,    17,
      17,    18,     5,    -1,    17,    17,    19,     5,    -1,    17,
      17,    19,     6,    -1,    17,    35,    17,    -1,    17,    35,
      17,    18,     5,    -1,    17,    35,    17,    19,     5,    -1,
      17,    35,    17,    19,     6,    -1,    20,    25,    21,    -1,
      22,    17,    35,    17,    -1,     3,    -1,     5,    -1,     6,
      -1,    22,    31,    -1,    34,    11,    34,    -1,    32,    11,
      34,    -1,    34,    12,    34,    -1,    33,    12,    34,    -1,
      31,    -1,    17,    35,    17,    -1,    36,    -1,    35,    36,
      -1,    31,    -1,    20,    -1,    21,    -1,    14,    -1,    16,
      -1,    18,    -1,    19,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    56,    56,    60,    61,    64,    66,    67,    68,    72,
      73,    74,    78,    79,    80,    84,    85,    86,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   106,   107,   108,   109,   113,   114,   118,   119,   123,
     124,   128,   129,   133,   134,   135,   136,   137,   138,   139
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_KEYWORD", "TOK_NEAR", "TOK_INT", 
  "TOK_FLOAT", "TOK_FIELDLIMIT", "TOK_ZONE", "TOK_ZONESPAN", "TOK_BEFORE", 
  "TOK_SENTENCE", "TOK_PARAGRAPH", "TOK_MAYBE", "'-'", "'!'", "'|'", 
  "'\"'", "'~'", "'/'", "'('", "')'", "'='", "$accept", "query", "expr", 
  "tok_limiter", "beforelist", "orlistf", "orlist", "atom", "keyword", 
  "sentence", "paragraph", "sp_item", "phrase", "phrasetoken", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,    45,    33,   124,    34,   126,    47,
      40,    41,    61
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    23,    24,    25,    25,    26,    26,    26,    26,    27,
      27,    27,    28,    28,    28,    29,    29,    29,    30,    30,
      30,    30,    30,    30,    30,    30,    30,    30,    30,    30,
      30,    31,    31,    31,    31,    32,    32,    33,    33,    34,
      34,    35,    35,    36,    36,    36,    36,    36,    36,    36
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     0,     1,     1,     1,     1,
       3,     3,     1,     3,     3,     2,     4,     4,     1,     1,
       1,     2,     4,     4,     4,     3,     5,     5,     5,     3,
       4,     1,     1,     1,     2,     3,     3,     3,     3,     1,
       3,     1,     2,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       5,     6,     7,     8,     0,     5,     0,     3,     9,    12,
       1,     4,    31,    32,    33,     5,     5,     0,     5,     0,
      15,    18,    19,    20,     0,     5,     5,     5,     5,     0,
      13,    14,    46,    47,    21,    48,    49,    44,    45,     0,
      43,     0,    41,     5,     0,    34,     0,     0,     0,     0,
      11,    10,     0,     0,     0,     0,    25,    42,    29,     0,
       0,    39,    36,    38,    35,    37,    17,    16,    22,    23,
      24,     0,     0,    30,     0,    26,    27,    28,    40
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     4,     5,     6,     7,     8,     9,    20,    40,    22,
      23,    24,    41,    42
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -40
static const short yypact[] =
{
     109,   -40,   -40,   -40,    21,   150,     2,    26,   -40,    -7,
     -40,    26,   -40,   -40,   -40,   109,   109,    45,   109,    52,
     -40,    41,    34,    20,    61,   109,   109,   109,   109,     9,
      -7,    -7,   -40,   -40,    19,   -40,   -40,   -40,   -40,    22,
     -40,    65,   -40,    88,   125,   -40,    72,    72,    72,    72,
     -40,   -40,     9,     9,    44,    87,    -8,   -40,   -40,    85,
     125,   -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,   -40,
     -40,    49,   127,   -40,   105,   -40,   -40,   -40,   -40
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -40,   -40,    38,   121,    -4,   126,   119,   101,    -6,   -40,
     -40,    66,   -26,   -39
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -41
static const yysigned_char yytable[] =
{
      21,    11,    57,   -40,   -40,    12,    27,    13,    14,    28,
      71,    72,    12,    45,    13,    14,    15,    16,    59,    17,
      57,    10,    18,    21,    19,    12,    17,    13,    14,    18,
      25,    19,    47,    45,    74,    57,    26,    54,    55,    11,
      61,    61,    61,    61,    39,    46,    21,    21,    12,    68,
      13,    14,   -39,   -39,    75,    12,    43,    13,    14,    32,
       0,    33,    34,    35,    36,    37,    38,    39,    12,    44,
      13,    14,    48,    49,    39,    12,     0,    13,    14,    32,
       0,    33,    56,    35,    36,    37,    38,    39,    12,    60,
      13,    14,    69,    70,    39,     1,     2,     3,     0,    32,
       0,    33,    73,    35,    36,    37,    38,    39,    12,    58,
      13,    14,    62,    63,    64,    65,     1,     2,     3,    32,
       0,    33,    78,    35,    36,    37,    38,    39,    12,     0,
      13,    14,    76,    77,    30,    31,    29,    29,     0,    32,
       0,    33,     0,    35,    36,    37,    38,    39,    52,    53,
      -2,    50,    51,    66,    67,     0,     0,     1,     2,     3
};

static const yysigned_char yycheck[] =
{
       6,     5,    41,    11,    12,     3,    13,     5,     6,    16,
      18,    19,     3,    19,     5,     6,    14,    15,    44,    17,
      59,     0,    20,    29,    22,     3,    17,     5,     6,    20,
       4,    22,    12,    39,    60,    74,    10,    18,    19,    43,
      46,    47,    48,    49,    22,    11,    52,    53,     3,     5,
       5,     6,    11,    12,     5,     3,    18,     5,     6,    14,
      -1,    16,    17,    18,    19,    20,    21,    22,     3,    17,
       5,     6,    11,    12,    22,     3,    -1,     5,     6,    14,
      -1,    16,    17,    18,    19,    20,    21,    22,     3,    17,
       5,     6,     5,     6,    22,     7,     8,     9,    -1,    14,
      -1,    16,    17,    18,    19,    20,    21,    22,     3,    21,
       5,     6,    46,    47,    48,    49,     7,     8,     9,    14,
      -1,    16,    17,    18,    19,    20,    21,    22,     3,    -1,
       5,     6,     5,     6,    15,    16,    15,    16,    -1,    14,
      -1,    16,    -1,    18,    19,    20,    21,    22,    27,    28,
       0,    25,    26,    52,    53,    -1,    -1,     7,     8,     9
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     7,     8,     9,    24,    25,    26,    27,    28,    29,
       0,    27,     3,     5,     6,    14,    15,    17,    20,    22,
      30,    31,    32,    33,    34,     4,    10,    13,    16,    26,
      29,    29,    14,    16,    17,    18,    19,    20,    21,    22,
      31,    35,    36,    25,    17,    31,    11,    12,    11,    12,
      28,    28,    26,    26,    18,    19,    17,    36,    21,    35,
      17,    31,    34,    34,    34,    34,    30,    30,     5,     5,
       6,    18,    19,    17,    35,     5,     5,     6,    17
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
int yyparse ( XQParser_t * pParser );
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
yyparse ( XQParser_t * pParser )
#else
int
yyparse (pParser)
     XQParser_t * pParser ;
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

    { pParser->AddQuery ( yyvsp[0].pNode ); ;}
    break;

  case 3:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 4:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_AND, yyvsp[-1].pNode, yyvsp[0].pNode ); ;}
    break;

  case 6:

    { pParser->SetFieldSpec ( yyvsp[0].tFieldLimit.dMask, yyvsp[0].tFieldLimit.iMaxPos ); ;}
    break;

  case 7:

    { pParser->SetZoneVec ( yyvsp[0].iZoneVec ); ;}
    break;

  case 8:

    { pParser->SetZoneVec ( yyvsp[0].iZoneVec, true ); ;}
    break;

  case 9:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 10:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_BEFORE, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 11:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_NEAR, yyvsp[-2].pNode, yyvsp[0].pNode, yyvsp[-1].tInt.iValue ); ;}
    break;

  case 12:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 13:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_NOT, yyvsp[0].pNode, NULL ); ;}
    break;

  case 14:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_NOT, yyvsp[0].pNode, NULL ); ;}
    break;

  case 15:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 16:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_OR, yyvsp[-3].pNode, yyvsp[0].pNode ); ;}
    break;

  case 17:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_MAYBE, yyvsp[-3].pNode, yyvsp[0].pNode ); ;}
    break;

  case 18:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 19:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 20:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 21:

    { yyval.pNode = NULL; ;}
    break;

  case 22:

    { yyval.pNode = NULL; ;}
    break;

  case 23:

    { yyval.pNode = NULL; ;}
    break;

  case 24:

    { yyval.pNode = NULL; ;}
    break;

  case 25:

    { yyval.pNode = yyvsp[-1].pNode; pParser->SetPhrase ( yyval.pNode, false ); ;}
    break;

  case 26:

    { yyval.pNode = yyvsp[-3].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_PROXIMITY ); yyval.pNode->m_iOpArg = yyvsp[0].tInt.iValue; pParser->m_iAtomPos = yyval.pNode->FixupAtomPos(); } ;}
    break;

  case 27:

    { yyval.pNode = yyvsp[-3].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_QUORUM ); yyval.pNode->m_iOpArg = yyvsp[0].tInt.iValue; } ;}
    break;

  case 28:

    { yyval.pNode = yyvsp[-3].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_QUORUM ); yyval.pNode->m_iOpArg = yyvsp[0].tInt.fValue * 100; yyval.pNode->m_bPercentOp = true; } ;}
    break;

  case 29:

    { yyval.pNode = yyvsp[-1].pNode; ;}
    break;

  case 30:

    { yyval.pNode = yyvsp[-1].pNode; pParser->SetPhrase ( yyval.pNode, true ); ;}
    break;

  case 31:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 32:

    { yyval.pNode = pParser->AddKeyword ( ( yyvsp[0].tInt.iStrIndex>=0 ) ? pParser->m_dIntTokens[yyvsp[0].tInt.iStrIndex].cstr() : NULL ); ;}
    break;

  case 33:

    { yyval.pNode = pParser->AddKeyword ( ( yyvsp[0].tInt.iStrIndex>=0 ) ? pParser->m_dIntTokens[yyvsp[0].tInt.iStrIndex].cstr() : NULL ); ;}
    break;

  case 34:

    { yyval.pNode = yyvsp[0].pNode; assert ( yyval.pNode->m_dWords.GetLength()==1 ); if ( !(yyval.pNode->m_dWords[0].m_sWord.IsEmpty()) ) yyval.pNode->m_dWords[0].m_sWord.SetSprintf ( "=%s", yyval.pNode->m_dWords[0].m_sWord.cstr() ); ;}
    break;

  case 35:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_SENTENCE, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 36:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_SENTENCE, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 37:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_PARAGRAPH, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 38:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_PARAGRAPH, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 39:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 40:

    { yyval.pNode = yyvsp[-1].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_PHRASE); } ;}
    break;

  case 41:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 42:

    { yyval.pNode = pParser->AddKeyword ( yyvsp[-1].pNode, yyvsp[0].pNode ); ;}
    break;

  case 43:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 44:

    { yyval.pNode = NULL; ;}
    break;

  case 45:

    { yyval.pNode = NULL; ;}
    break;

  case 46:

    { yyval.pNode = NULL; ;}
    break;

  case 47:

    { yyval.pNode = NULL; ;}
    break;

  case 48:

    { yyval.pNode = NULL; ;}
    break;

  case 49:

    { yyval.pNode = NULL; ;}
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
	    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
//yyerrlab1:

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

