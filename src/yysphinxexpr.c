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
     TOK_CONST_INT = 258,
     TOK_CONST_FLOAT = 259,
     TOK_CONST_STRING = 260,
     TOK_ATTR_INT = 261,
     TOK_ATTR_BITS = 262,
     TOK_ATTR_FLOAT = 263,
     TOK_ATTR_MVA32 = 264,
     TOK_ATTR_MVA64 = 265,
     TOK_ATTR_STRING = 266,
     TOK_ATTR_FACTORS = 267,
     TOK_FUNC = 268,
     TOK_FUNC_IN = 269,
     TOK_USERVAR = 270,
     TOK_UDF = 271,
     TOK_HOOK_IDENT = 272,
     TOK_HOOK_FUNC = 273,
     TOK_IDENT = 274,
     TOK_ATID = 275,
     TOK_ATWEIGHT = 276,
     TOK_ID = 277,
     TOK_GROUPBY = 278,
     TOK_WEIGHT = 279,
     TOK_COUNT = 280,
     TOK_DISTINCT = 281,
     TOK_CONST_LIST = 282,
     TOK_ATTR_SINT = 283,
     TOK_CONST_HASH = 284,
     TOK_OR = 285,
     TOK_AND = 286,
     TOK_NE = 287,
     TOK_EQ = 288,
     TOK_GTE = 289,
     TOK_LTE = 290,
     TOK_MOD = 291,
     TOK_DIV = 292,
     TOK_NOT = 293,
     TOK_NEG = 294
   };
#endif
#define TOK_CONST_INT 258
#define TOK_CONST_FLOAT 259
#define TOK_CONST_STRING 260
#define TOK_ATTR_INT 261
#define TOK_ATTR_BITS 262
#define TOK_ATTR_FLOAT 263
#define TOK_ATTR_MVA32 264
#define TOK_ATTR_MVA64 265
#define TOK_ATTR_STRING 266
#define TOK_ATTR_FACTORS 267
#define TOK_FUNC 268
#define TOK_FUNC_IN 269
#define TOK_USERVAR 270
#define TOK_UDF 271
#define TOK_HOOK_IDENT 272
#define TOK_HOOK_FUNC 273
#define TOK_IDENT 274
#define TOK_ATID 275
#define TOK_ATWEIGHT 276
#define TOK_ID 277
#define TOK_GROUPBY 278
#define TOK_WEIGHT 279
#define TOK_COUNT 280
#define TOK_DISTINCT 281
#define TOK_CONST_LIST 282
#define TOK_ATTR_SINT 283
#define TOK_CONST_HASH 284
#define TOK_OR 285
#define TOK_AND 286
#define TOK_NE 287
#define TOK_EQ 288
#define TOK_GTE 289
#define TOK_LTE 290
#define TOK_MOD 291
#define TOK_DIV 292
#define TOK_NOT 293
#define TOK_NEG 294




/* Copy the first part of user declarations.  */


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
	int64_t			iConst;			// constant value
	float			fConst;			// constant value
	uint64_t		iAttrLocator;	// attribute locator (rowitem for int/float; offset+size for bits)
	int				iFunc;			// function id
	int				iNode;			// node, or uservar, or udf index
	const char *	sIdent;			// generic identifier (token does NOT own ident storage; ie values are managed by parser)
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
#define YYFINAL  32
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   276

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  11
/* YYNRULES -- Number of rules. */
#define YYNRULES  60
/* YYNRULES -- Number of states. */
#define YYNSTATES  110

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   294

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    44,    33,     2,
      49,    50,    42,    40,    51,    41,     2,    43,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      36,     2,    37,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,    32,    53,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    34,    35,    38,
      39,    45,    46,    47,    48
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    29,    33,    35,    38,    41,    45,
      49,    53,    57,    61,    65,    69,    73,    77,    81,    85,
      89,    93,    97,   101,   105,   109,   113,   117,   123,   125,
     129,   131,   133,   135,   137,   139,   141,   145,   147,   149,
     153,   157,   159,   161,   163,   165,   170,   174,   179,   183,
     190
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      55,     0,    -1,    57,    -1,     6,    -1,     7,    -1,     8,
      -1,    56,    -1,    64,    -1,     3,    -1,     4,    -1,    20,
      -1,    21,    -1,    22,    -1,    24,    49,    50,    -1,    23,
      49,    50,    -1,    17,    -1,    41,    57,    -1,    47,    57,
      -1,    57,    40,    57,    -1,    57,    41,    57,    -1,    57,
      42,    57,    -1,    57,    43,    57,    -1,    57,    36,    57,
      -1,    57,    37,    57,    -1,    57,    33,    57,    -1,    57,
      32,    57,    -1,    57,    44,    57,    -1,    57,    46,    57,
      -1,    57,    45,    57,    -1,    57,    39,    57,    -1,    57,
      38,    57,    -1,    57,    35,    57,    -1,    57,    34,    57,
      -1,    57,    31,    57,    -1,    57,    30,    57,    -1,    49,
      57,    50,    -1,    63,    35,     3,    -1,    58,    51,    63,
      35,     3,    -1,    57,    -1,    52,    58,    53,    -1,    11,
      -1,     9,    -1,    10,    -1,    12,    -1,     5,    -1,    59,
      -1,    60,    51,    59,    -1,     3,    -1,     4,    -1,    61,
      51,     3,    -1,    61,    51,     4,    -1,    61,    -1,    15,
      -1,     6,    -1,    19,    -1,    13,    49,    60,    50,    -1,
      13,    49,    50,    -1,    16,    49,    60,    50,    -1,    16,
      49,    50,    -1,    14,    49,    59,    51,    62,    50,    -1,
      18,    49,    60,    50,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    68,    68,    72,    73,    74,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   111,   115,   122,   123,
     124,   125,   126,   127,   128,   132,   133,   137,   138,   139,
     140,   144,   145,   149,   153,   157,   158,   159,   160,   161,
     165
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_CONST_INT", "TOK_CONST_FLOAT", 
  "TOK_CONST_STRING", "TOK_ATTR_INT", "TOK_ATTR_BITS", "TOK_ATTR_FLOAT", 
  "TOK_ATTR_MVA32", "TOK_ATTR_MVA64", "TOK_ATTR_STRING", 
  "TOK_ATTR_FACTORS", "TOK_FUNC", "TOK_FUNC_IN", "TOK_USERVAR", "TOK_UDF", 
  "TOK_HOOK_IDENT", "TOK_HOOK_FUNC", "TOK_IDENT", "TOK_ATID", 
  "TOK_ATWEIGHT", "TOK_ID", "TOK_GROUPBY", "TOK_WEIGHT", "TOK_COUNT", 
  "TOK_DISTINCT", "TOK_CONST_LIST", "TOK_ATTR_SINT", "TOK_CONST_HASH", 
  "TOK_OR", "TOK_AND", "'|'", "'&'", "TOK_NE", "TOK_EQ", "'<'", "'>'", 
  "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_MOD", 
  "TOK_DIV", "TOK_NOT", "TOK_NEG", "'('", "')'", "','", "'{'", "'}'", 
  "$accept", "exprline", "attr", "expr", "consthash", "arg", "arglist", 
  "constlist", "constlist_or_uservar", "ident", "function", 0
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
     285,   286,   124,    38,   287,   288,    60,    62,   289,   290,
      43,    45,    42,    47,    37,   291,   292,   293,   294,    40,
      41,    44,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    54,    55,    56,    56,    56,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    58,    58,    59,    59,
      59,    59,    59,    59,    59,    60,    60,    61,    61,    61,
      61,    62,    62,    63,    63,    64,    64,    64,    64,    64,
      64
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     1,     3,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     3,
       3,     1,     1,     1,     1,     4,     3,     4,     3,     6,
       4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     8,     9,     3,     4,     5,     0,     0,     0,    15,
       0,    10,    11,    12,     0,     0,     0,     0,     0,     0,
       6,     2,     7,     0,     0,     0,     0,     0,     0,    16,
      17,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      44,    41,    42,    40,    43,    56,     0,    38,    45,     0,
       0,    58,     0,     0,    14,    13,    35,    34,    33,    25,
      24,    32,    31,    22,    23,    30,    29,    18,    19,    20,
      21,    26,    28,    27,    53,    54,     0,     0,    55,     0,
       0,    57,    60,     0,    39,     0,    46,    47,    48,    52,
      51,     0,     0,    36,     0,    59,     0,    49,    50,    37
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    19,    20,    57,    86,    58,    59,   100,   101,    87,
      22
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -37
static const short yypact[] =
{
     143,   -37,   -37,   -37,   -37,   -37,   -36,   -32,   -14,   -37,
       0,   -37,   -37,   -37,    13,    15,   143,   143,   143,    66,
     -37,   184,   -37,    -2,    70,    20,    70,    18,    21,   -37,
     -37,   163,   -37,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     -37,   -37,   -37,   -37,   -37,   -37,    40,   184,   -37,    65,
      34,   -37,    73,    75,   -37,   -37,   -37,   137,   199,    64,
     212,   223,   223,   230,   230,   230,   230,    11,    11,   -37,
     -37,   -37,   -37,   -37,   -37,   -37,     7,    54,   -37,    70,
      48,   -37,   -37,    40,   -37,    92,   -37,   -37,   -37,   -37,
      67,    98,   118,   -37,   117,   -37,   149,   -37,   -37,   -37
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -37,   -37,   -37,    96,   -37,   -24,   102,   -37,   -37,    61,
     -37
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      60,     1,     2,    50,     3,     4,     5,    51,    52,    53,
      54,     6,     7,    23,     8,     9,    10,    24,    11,    12,
      13,    14,    15,     1,     2,    50,     3,     4,     5,    51,
      52,    53,    54,     6,     7,    25,     8,     9,    10,    16,
      11,    12,    13,    14,    15,    17,    84,    18,    55,    26,
      56,    97,    98,    45,    46,    47,    48,    49,    93,    85,
      94,    16,    27,    99,    28,    96,    32,    17,    64,    18,
      61,    65,    56,     1,     2,    50,     3,     4,     5,    51,
      52,    53,    54,     6,     7,    90,     8,     9,    10,    95,
      11,    12,    13,    14,    15,   103,    21,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    16,    29,    30,    31,    88,    89,    17,   104,    18,
     107,   108,    56,    91,    89,    92,    89,    62,    63,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     1,     2,   105,     3,
       4,     5,   109,   106,   102,     0,     6,     7,     0,     8,
       9,    10,     0,    11,    12,    13,    14,    15,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    16,     0,     0,     0,     0,     0,
      17,     0,    18,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,     0,     0,    66,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      43,    44,    45,    46,    47,    48,    49
};

static const yysigned_char yycheck[] =
{
      24,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    49,    16,    17,    18,    49,    20,    21,
      22,    23,    24,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    49,    16,    17,    18,    41,
      20,    21,    22,    23,    24,    47,     6,    49,    50,    49,
      52,     3,     4,    42,    43,    44,    45,    46,    51,    19,
      53,    41,    49,    15,    49,    89,     0,    47,    50,    49,
      50,    50,    52,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    51,    16,    17,    18,    35,
      20,    21,    22,    23,    24,     3,     0,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    41,    16,    17,    18,    50,    51,    47,    51,    49,
       3,     4,    52,    50,    51,    50,    51,    25,    26,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     3,     4,    50,     6,
       7,     8,     3,    35,    93,    -1,    13,    14,    -1,    16,
      17,    18,    -1,    20,    21,    22,    23,    24,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    41,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    49,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      -1,    -1,    -1,    50,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      40,    41,    42,    43,    44,    45,    46
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     6,     7,     8,    13,    14,    16,    17,
      18,    20,    21,    22,    23,    24,    41,    47,    49,    55,
      56,    57,    64,    49,    49,    49,    49,    49,    49,    57,
      57,    57,     0,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
       5,     9,    10,    11,    12,    50,    52,    57,    59,    60,
      59,    50,    60,    60,    50,    50,    50,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,     6,    19,    58,    63,    50,    51,
      51,    50,    50,    51,    53,    35,    59,     3,     4,    15,
      61,    62,    63,     3,    51,    50,    35,     3,     4,     3
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
int yyparse ( ExprParser_t * pParser );
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
yyparse ( ExprParser_t * pParser )
#else
int
yyparse (pParser)
     ExprParser_t * pParser ;
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

    { pParser->m_iParsed = yyvsp[0].iNode; ;}
    break;

  case 3:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_INT, yyvsp[0].iAttrLocator ); ;}
    break;

  case 4:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_BITS, yyvsp[0].iAttrLocator ); ;}
    break;

  case 5:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_FLOAT, yyvsp[0].iAttrLocator ); ;}
    break;

  case 8:

    { yyval.iNode = pParser->AddNodeInt ( yyvsp[0].iConst ); ;}
    break;

  case 9:

    { yyval.iNode = pParser->AddNodeFloat ( yyvsp[0].fConst ); ;}
    break;

  case 10:

    { yyval.iNode = pParser->AddNodeID(); ;}
    break;

  case 11:

    { yyval.iNode = pParser->AddNodeWeight(); ;}
    break;

  case 12:

    { yyval.iNode = pParser->AddNodeID(); ;}
    break;

  case 13:

    { yyval.iNode = pParser->AddNodeWeight(); ;}
    break;

  case 14:

    { yyval.iNode = pParser->AddNodeGroupby(); ;}
    break;

  case 15:

    { yyval.iNode = pParser->AddNodeHookIdent ( yyvsp[0].iNode ); ;}
    break;

  case 16:

    { yyval.iNode = pParser->AddNodeOp ( TOK_NEG, yyvsp[0].iNode, -1 ); ;}
    break;

  case 17:

    { yyval.iNode = pParser->AddNodeOp ( TOK_NOT, yyvsp[0].iNode, -1 ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 18:

    { yyval.iNode = pParser->AddNodeOp ( '+', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 19:

    { yyval.iNode = pParser->AddNodeOp ( '-', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 20:

    { yyval.iNode = pParser->AddNodeOp ( '*', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 21:

    { yyval.iNode = pParser->AddNodeOp ( '/', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 22:

    { yyval.iNode = pParser->AddNodeOp ( '<', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 23:

    { yyval.iNode = pParser->AddNodeOp ( '>', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 24:

    { yyval.iNode = pParser->AddNodeOp ( '&', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 25:

    { yyval.iNode = pParser->AddNodeOp ( '|', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 26:

    { yyval.iNode = pParser->AddNodeOp ( '%', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 27:

    { yyval.iNode = pParser->AddNodeFunc ( FUNC_IDIV, pParser->AddNodeOp ( ',', yyvsp[-2].iNode, yyvsp[0].iNode ) ); ;}
    break;

  case 28:

    { yyval.iNode = pParser->AddNodeOp ( '%', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 29:

    { yyval.iNode = pParser->AddNodeOp ( TOK_LTE, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 30:

    { yyval.iNode = pParser->AddNodeOp ( TOK_GTE, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 31:

    { yyval.iNode = pParser->AddNodeOp ( TOK_EQ, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 32:

    { yyval.iNode = pParser->AddNodeOp ( TOK_NE, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 33:

    { yyval.iNode = pParser->AddNodeOp ( TOK_AND, yyvsp[-2].iNode, yyvsp[0].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 34:

    { yyval.iNode = pParser->AddNodeOp ( TOK_OR, yyvsp[-2].iNode, yyvsp[0].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 35:

    { yyval.iNode = yyvsp[-1].iNode; ;}
    break;

  case 36:

    {
			yyval.iNode = pParser->AddNodeConsthash ( yyvsp[-2].sIdent, yyvsp[0].iConst );
		;}
    break;

  case 37:

    {
			pParser->AppendToConsthash ( yyval.iNode, yyvsp[-2].sIdent, yyvsp[0].iConst );
		;}
    break;

  case 39:

    { yyval.iNode = yyvsp[-1].iNode; ;}
    break;

  case 40:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_STRING, yyvsp[0].iAttrLocator ); ;}
    break;

  case 41:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_MVA32, yyvsp[0].iAttrLocator ); ;}
    break;

  case 42:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_MVA64, yyvsp[0].iAttrLocator ); ;}
    break;

  case 43:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_FACTORS, yyvsp[0].iAttrLocator ); ;}
    break;

  case 44:

    { yyval.iNode = pParser->AddNodeString ( yyvsp[0].iConst ); ;}
    break;

  case 45:

    { yyval.iNode = yyvsp[0].iNode; ;}
    break;

  case 46:

    { yyval.iNode = pParser->AddNodeOp ( ',', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 47:

    { yyval.iNode = pParser->AddNodeConstlist ( yyvsp[0].iConst ); ;}
    break;

  case 48:

    { yyval.iNode = pParser->AddNodeConstlist ( yyvsp[0].fConst ); ;}
    break;

  case 49:

    { pParser->AppendToConstlist ( yyval.iNode, yyvsp[0].iConst ); ;}
    break;

  case 50:

    { pParser->AppendToConstlist ( yyval.iNode, yyvsp[0].fConst ); ;}
    break;

  case 52:

    { yyval.iNode = pParser->AddNodeUservar ( yyvsp[0].iNode ); ;}
    break;

  case 53:

    {
			yyval.sIdent = pParser->Attr2Ident ( yyvsp[0].iAttrLocator );
		;}
    break;

  case 55:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-3].iFunc, yyvsp[-1].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 56:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-2].iFunc, -1 ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 57:

    { yyval.iNode = pParser->AddNodeUdf ( yyvsp[-3].iNode, yyvsp[-1].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 58:

    { yyval.iNode = pParser->AddNodeUdf ( yyvsp[-2].iNode, -1 ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 59:

    {
			yyval.iNode = pParser->AddNodeFunc ( yyvsp[-5].iFunc, yyvsp[-3].iNode, yyvsp[-1].iNode );
		;}
    break;

  case 60:

    {
			yyval.iNode = pParser->AddNodeHookFunc ( yyvsp[-3].iNode, yyvsp[-1].iNode );
			if ( yyval.iNode<0 )
				YYERROR;
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





