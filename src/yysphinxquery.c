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
     TOK_FIELDLIMIT = 261,
     TOK_ZONE = 262,
     TOK_BEFORE = 263,
     TOK_SENTENCE = 264,
     TOK_PARAGRAPH = 265
   };
#endif
#define TOK_KEYWORD 258
#define TOK_NEAR 259
#define TOK_INT 260
#define TOK_FIELDLIMIT 261
#define TOK_ZONE 262
#define TOK_BEFORE 263
#define TOK_SENTENCE 264
#define TOK_PARAGRAPH 265




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
	} tInt;
	struct							// field spec
	{
		CSphSmallBitvec		dMask;			// acceptable fields mask
		int			iMaxPos;		// max allowed position within field
	} tFieldLimit;
	int				iZoneVec;
} YYSTYPE;
/* Line 191 of yacc.c.  */

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
#define YYFINAL  38
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   136

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  20
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  15
/* YYNRULES -- Number of rules. */
#define YYNRULES  46
/* YYNRULES -- Number of states. */
#define YYNSTATES  72

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   265

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    19,     2,    11,     2,     2,     2,
      13,    14,     2,     2,     2,    15,     2,    18,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    12,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    16,     2,    17,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    14,    17,    21,
      23,    25,    27,    29,    31,    33,    35,    37,    40,    42,
      46,    50,    54,    58,    62,    64,    66,    68,    71,    76,
      81,    85,    91,    97,   101,   103,   106,   109,   111,   115,
     117,   120,   124,   126,   130,   134,   136
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      21,     0,    -1,    34,    -1,     3,    -1,     5,    -1,    22,
      -1,    22,    11,    -1,    12,    22,    -1,    12,    22,    11,
      -1,    23,    -1,    13,    -1,    14,    -1,    15,    -1,    16,
      -1,    17,    -1,    18,    -1,    24,    -1,    25,    24,    -1,
      23,    -1,    19,    25,    19,    -1,    26,     9,    26,    -1,
      27,     9,    26,    -1,    26,    10,    26,    -1,    28,    10,
      26,    -1,    23,    -1,    27,    -1,    28,    -1,    19,    19,
      -1,    19,    19,    17,     5,    -1,    19,    19,    18,     5,
      -1,    19,    25,    19,    -1,    19,    25,    19,    17,     5,
      -1,    19,    25,    19,    18,     5,    -1,    13,    34,    14,
      -1,    29,    -1,     6,    29,    -1,     7,    29,    -1,    30,
      -1,    31,    16,    30,    -1,    31,    -1,    15,    31,    -1,
       6,    15,    31,    -1,    32,    -1,    33,     8,    32,    -1,
      33,     4,    32,    -1,    33,    -1,    34,    33,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    54,    54,    58,    59,    63,    64,    65,    66,    70,
      71,    72,    73,    74,    75,    76,    80,    81,    85,    86,
      90,    91,    95,    96,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   113,   114,   115,   119,   120,   124,
     125,   126,   130,   131,   132,   136,   137
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_KEYWORD", "TOK_NEAR", "TOK_INT", 
  "TOK_FIELDLIMIT", "TOK_ZONE", "TOK_BEFORE", "TOK_SENTENCE", 
  "TOK_PARAGRAPH", "'$'", "'^'", "'('", "')'", "'-'", "'|'", "'~'", "'/'", 
  "'\"'", "$accept", "query", "rawkeyword", "keyword", "phrasetoken", 
  "phrase", "sp_item", "sentence", "paragraph", "atom", "atomf", "orlist", 
  "orlistf", "beforelist", "expr", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,    36,    94,    40,    41,    45,   124,   126,    47,    34
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    20,    21,    22,    22,    23,    23,    23,    23,    24,
      24,    24,    24,    24,    24,    24,    25,    25,    26,    26,
      27,    27,    28,    28,    29,    29,    29,    29,    29,    29,
      29,    29,    29,    29,    30,    30,    30,    31,    31,    32,
      32,    32,    33,    33,    33,    34,    34
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     2,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     3,
       3,     3,     3,     3,     1,     1,     1,     2,     4,     4,
       3,     5,     5,     3,     1,     2,     2,     1,     3,     1,
       2,     3,     1,     3,     3,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     3,     4,     0,     0,     0,     0,     0,     0,     0,
       5,    24,     0,    25,    26,    34,    37,    39,    42,    45,
       2,     0,    35,    36,     7,     0,     0,    40,    10,    11,
      12,    13,    14,    15,    27,     9,    16,     0,     1,     6,
       0,     0,     0,     0,     0,     0,     0,    46,    41,     8,
      33,     0,     0,    30,    17,     0,    18,    20,    22,    21,
      23,    38,    44,    43,    28,    29,     0,     0,     0,    31,
      32,    19
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     9,    10,    11,    36,    37,    12,    13,    14,    15,
      16,    17,    18,    19,    20
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -33
static const yysigned_char yypact[] =
{
      81,   -33,   -33,   103,   107,    20,    81,    92,     1,    27,
      42,    36,    41,    22,    45,   -33,   -33,    54,   -33,    18,
      81,    92,   -33,   -33,    64,    66,   107,    54,   -33,   -33,
     -33,   -33,   -33,   -33,    74,   -33,   -33,    25,   -33,   -33,
      71,    71,    71,    71,    92,    81,    81,    18,    54,   -33,
     -33,    72,    77,    39,   -33,   118,   -33,   -33,   -33,   -33,
     -33,   -33,   -33,   -33,   -33,   -33,    84,    96,    49,   -33,
     -33,   -33
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -33,   -33,    97,    -8,   -32,    48,   -31,   -33,   -33,    -2,
      63,     0,    13,   -17,   108
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -20
static const yysigned_char yytable[] =
{
      35,    22,    23,    47,     1,    54,     2,    27,    47,    57,
      58,    59,    60,     5,    28,    29,    30,    31,    32,    33,
      34,    48,    45,     1,    22,     2,    46,    38,     1,    35,
       2,    42,    56,    56,    56,    56,    54,     5,    28,    29,
      30,    31,    32,    33,    53,   -18,   -18,    35,   -19,   -19,
      40,    41,     1,    39,     2,    43,    66,    67,    62,    63,
      35,     5,    28,    29,    30,    31,    32,    33,    71,     1,
      44,     2,     3,     4,     1,    49,     2,    64,     5,     6,
      50,     7,    65,     5,     1,     8,     2,     3,     4,    69,
      55,    51,    52,     5,     6,     1,     7,     2,    26,     4,
       8,    70,    24,    68,     5,     6,     1,    61,     2,     0,
       1,     8,     2,     0,    25,     5,     6,     0,    21,     5,
       6,     1,     8,     2,     0,     0,     8,     0,     0,     0,
       5,    28,    29,    30,    31,    32,    33
};

static const yysigned_char yycheck[] =
{
       8,     3,     4,    20,     3,    37,     5,     7,    25,    40,
      41,    42,    43,    12,    13,    14,    15,    16,    17,    18,
      19,    21,     4,     3,    26,     5,     8,     0,     3,    37,
       5,     9,    40,    41,    42,    43,    68,    12,    13,    14,
      15,    16,    17,    18,    19,     9,    10,    55,     9,    10,
       9,    10,     3,    11,     5,    10,    17,    18,    45,    46,
      68,    12,    13,    14,    15,    16,    17,    18,    19,     3,
      16,     5,     6,     7,     3,    11,     5,     5,    12,    13,
      14,    15,     5,    12,     3,    19,     5,     6,     7,     5,
      19,    17,    18,    12,    13,     3,    15,     5,     6,     7,
      19,     5,     5,    55,    12,    13,     3,    44,     5,    -1,
       3,    19,     5,    -1,     6,    12,    13,    -1,    15,    12,
      13,     3,    19,     5,    -1,    -1,    19,    -1,    -1,    -1,
      12,    13,    14,    15,    16,    17,    18
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     5,     6,     7,    12,    13,    15,    19,    21,
      22,    23,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    15,    29,    29,    22,    34,     6,    31,    13,    14,
      15,    16,    17,    18,    19,    23,    24,    25,     0,    11,
       9,    10,     9,    10,    16,     4,     8,    33,    31,    11,
      14,    17,    18,    19,    24,    19,    23,    26,    26,    26,
      26,    30,    32,    32,     5,     5,    17,    18,    25,     5,
       5,    19
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

    { yyval.pNode = pParser->AddKeyword ( ( yyvsp[0].tInt.iStrIndex>=0 ) ? pParser->m_dIntTokens[yyvsp[0].tInt.iStrIndex].cstr() : NULL ); ;}
    break;

  case 6:

    { yyval.pNode = yyvsp[-1].pNode; assert ( yyval.pNode->m_dWords.GetLength()==1 ); yyval.pNode->m_dWords[0].m_bFieldEnd = true; ;}
    break;

  case 7:

    { yyval.pNode = yyvsp[0].pNode; assert ( yyval.pNode->m_dWords.GetLength()==1 ); yyval.pNode->m_dWords[0].m_bFieldStart = true; ;}
    break;

  case 8:

    { yyval.pNode = yyvsp[-1].pNode; assert ( yyval.pNode->m_dWords.GetLength()==1 ); yyval.pNode->m_dWords[0].m_bFieldStart = true; yyval.pNode->m_dWords[0].m_bFieldEnd = true; ;}
    break;

  case 9:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 10:

    { yyval.pNode = NULL; ;}
    break;

  case 11:

    { yyval.pNode = NULL; ;}
    break;

  case 12:

    { yyval.pNode = NULL; ;}
    break;

  case 13:

    { yyval.pNode = NULL; ;}
    break;

  case 14:

    { yyval.pNode = NULL; ;}
    break;

  case 15:

    { yyval.pNode = NULL; ;}
    break;

  case 16:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 17:

    { yyval.pNode = pParser->AddKeyword ( yyvsp[-1].pNode, yyvsp[0].pNode ); ;}
    break;

  case 18:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 19:

    { yyval.pNode = yyvsp[-1].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_PHRASE); } ;}
    break;

  case 20:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_SENTENCE, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 21:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_SENTENCE, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 22:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_PARAGRAPH, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 23:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_PARAGRAPH, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 24:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 25:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 26:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 27:

    { yyval.pNode = NULL; ;}
    break;

  case 28:

    { yyval.pNode = NULL; ;}
    break;

  case 29:

    { yyval.pNode = NULL; ;}
    break;

  case 30:

    { yyval.pNode = yyvsp[-1].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_PHRASE); } ;}
    break;

  case 31:

    { yyval.pNode = yyvsp[-3].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_PROXIMITY ); yyval.pNode->m_iOpArg = yyvsp[0].tInt.iValue; } ;}
    break;

  case 32:

    { yyval.pNode = yyvsp[-3].pNode; if ( yyval.pNode ) { assert ( yyval.pNode->m_dWords.GetLength() ); yyval.pNode->SetOp ( SPH_QUERY_QUORUM ); yyval.pNode->m_iOpArg = yyvsp[0].tInt.iValue; } ;}
    break;

  case 33:

    { yyval.pNode = yyvsp[-1].pNode; if ( yyval.pNode ) yyval.pNode->m_bFieldSpec = false; ;}
    break;

  case 34:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 35:

    { yyval.pNode = yyvsp[0].pNode; if ( yyval.pNode ) yyval.pNode->SetFieldSpec ( yyvsp[-1].tFieldLimit.dMask, yyvsp[-1].tFieldLimit.iMaxPos ); ;}
    break;

  case 36:

    { yyval.pNode = yyvsp[0].pNode; if ( yyval.pNode ) yyval.pNode->SetZoneSpec ( pParser->GetZoneVec ( yyvsp[-1].iZoneVec ) ); ;}
    break;

  case 37:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 38:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_OR, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 39:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 40:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_NOT, yyvsp[0].pNode, NULL ); ;}
    break;

  case 41:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_NOT, yyvsp[0].pNode, NULL ); yyval.pNode->SetFieldSpec ( yyvsp[-2].tFieldLimit.dMask, yyvsp[-2].tFieldLimit.iMaxPos ); ;}
    break;

  case 43:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_BEFORE, yyvsp[-2].pNode, yyvsp[0].pNode ); ;}
    break;

  case 44:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_NEAR, yyvsp[-2].pNode, yyvsp[0].pNode, yyvsp[-1].tInt.iValue ); ;}
    break;

  case 45:

    { yyval.pNode = yyvsp[0].pNode; ;}
    break;

  case 46:

    { yyval.pNode = pParser->AddOp ( SPH_QUERY_AND, yyvsp[-1].pNode, yyvsp[0].pNode ); ;}
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

