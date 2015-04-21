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
     TOK_SUBKEY = 261,
     TOK_DOT_NUMBER = 262,
     TOK_ATTR_INT = 263,
     TOK_ATTR_BITS = 264,
     TOK_ATTR_FLOAT = 265,
     TOK_ATTR_MVA32 = 266,
     TOK_ATTR_MVA64 = 267,
     TOK_ATTR_STRING = 268,
     TOK_ATTR_FACTORS = 269,
     TOK_FUNC = 270,
     TOK_FUNC_IN = 271,
     TOK_FUNC_RAND = 272,
     TOK_FUNC_REMAP = 273,
     TOK_FUNC_PF = 274,
     TOK_USERVAR = 275,
     TOK_UDF = 276,
     TOK_HOOK_IDENT = 277,
     TOK_HOOK_FUNC = 278,
     TOK_IDENT = 279,
     TOK_ATTR_JSON = 280,
     TOK_ATID = 281,
     TOK_ATWEIGHT = 282,
     TOK_ID = 283,
     TOK_GROUPBY = 284,
     TOK_WEIGHT = 285,
     TOK_COUNT = 286,
     TOK_DISTINCT = 287,
     TOK_CONST_LIST = 288,
     TOK_ATTR_SINT = 289,
     TOK_MAP_ARG = 290,
     TOK_FOR = 291,
     TOK_ITERATOR = 292,
     TOK_IS = 293,
     TOK_NULL = 294,
     TOK_IS_NULL = 295,
     TOK_IS_NOT_NULL = 296,
     TOK_OR = 297,
     TOK_AND = 298,
     TOK_NE = 299,
     TOK_EQ = 300,
     TOK_GTE = 301,
     TOK_LTE = 302,
     TOK_MOD = 303,
     TOK_DIV = 304,
     TOK_NOT = 305,
     TOK_NEG = 306
   };
#endif
#define TOK_CONST_INT 258
#define TOK_CONST_FLOAT 259
#define TOK_CONST_STRING 260
#define TOK_SUBKEY 261
#define TOK_DOT_NUMBER 262
#define TOK_ATTR_INT 263
#define TOK_ATTR_BITS 264
#define TOK_ATTR_FLOAT 265
#define TOK_ATTR_MVA32 266
#define TOK_ATTR_MVA64 267
#define TOK_ATTR_STRING 268
#define TOK_ATTR_FACTORS 269
#define TOK_FUNC 270
#define TOK_FUNC_IN 271
#define TOK_FUNC_RAND 272
#define TOK_FUNC_REMAP 273
#define TOK_FUNC_PF 274
#define TOK_USERVAR 275
#define TOK_UDF 276
#define TOK_HOOK_IDENT 277
#define TOK_HOOK_FUNC 278
#define TOK_IDENT 279
#define TOK_ATTR_JSON 280
#define TOK_ATID 281
#define TOK_ATWEIGHT 282
#define TOK_ID 283
#define TOK_GROUPBY 284
#define TOK_WEIGHT 285
#define TOK_COUNT 286
#define TOK_DISTINCT 287
#define TOK_CONST_LIST 288
#define TOK_ATTR_SINT 289
#define TOK_MAP_ARG 290
#define TOK_FOR 291
#define TOK_ITERATOR 292
#define TOK_IS 293
#define TOK_NULL 294
#define TOK_IS_NULL 295
#define TOK_IS_NOT_NULL 296
#define TOK_OR 297
#define TOK_AND 298
#define TOK_NE 299
#define TOK_EQ 300
#define TOK_GTE 301
#define TOK_LTE 302
#define TOK_MOD 303
#define TOK_DIV 304
#define TOK_NOT 305
#define TOK_NEG 306




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
#define YYFINAL  52
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   563

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  68
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  21
/* YYNRULES -- Number of rules. */
#define YYNRULES  103
/* YYNRULES -- Number of states. */
#define YYNSTATES  187

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   306

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    56,    45,     2,
      61,    62,    54,    52,    63,    53,     2,    55,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      48,     2,    49,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,    67,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,    44,    65,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    46,
      47,    50,    51,    57,    58,    59,    60
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    29,    33,    35,    38,    41,
      45,    49,    53,    57,    61,    65,    69,    73,    77,    81,
      85,    89,    93,    97,   101,   105,   109,   113,   115,   117,
     119,   123,   128,   132,   136,   142,   148,   150,   152,   154,
     156,   158,   162,   164,   166,   168,   170,   172,   174,   178,
     180,   183,   185,   188,   192,   197,   201,   206,   208,   212,
     214,   216,   218,   220,   222,   227,   231,   236,   240,   247,
     252,   258,   273,   277,   282,   286,   291,   293,   295,   298,
     300,   303,   307,   309,   311,   315,   319,   324,   326,   329,
     333,   337,   341,   343
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      69,     0,    -1,    71,    -1,     8,    -1,     9,    -1,    10,
      -1,    25,    -1,    70,    -1,    80,    -1,     3,    -1,     4,
      -1,     7,    -1,    26,    -1,    27,    -1,    28,    -1,    30,
      61,    62,    -1,    22,    -1,    53,    71,    -1,    59,    71,
      -1,    71,    52,    71,    -1,    71,    53,    71,    -1,    71,
      54,    71,    -1,    71,    55,    71,    -1,    71,    48,    71,
      -1,    71,    49,    71,    -1,    71,    45,    71,    -1,    71,
      44,    71,    -1,    71,    56,    71,    -1,    71,    58,    71,
      -1,    71,    57,    71,    -1,    71,    51,    71,    -1,    71,
      50,    71,    -1,    71,    47,    71,    -1,    71,    46,    71,
      -1,    71,    43,    71,    -1,    71,    42,    71,    -1,    61,
      71,    62,    -1,    82,    -1,    86,    -1,    87,    -1,    81,
      38,    39,    -1,    81,    38,    59,    39,    -1,    73,    47,
       3,    -1,    73,    47,    24,    -1,    72,    63,    73,    47,
       3,    -1,    72,    63,    73,    47,    24,    -1,    79,    -1,
      13,    -1,    16,    -1,    17,    -1,    71,    -1,    64,    72,
      65,    -1,    13,    -1,    11,    -1,    12,    -1,    14,    -1,
       5,    -1,    74,    -1,    75,    63,    74,    -1,     3,    -1,
      53,     3,    -1,     4,    -1,    53,     4,    -1,    76,    63,
       3,    -1,    76,    63,    53,     3,    -1,    76,    63,     4,
      -1,    76,    63,    53,     4,    -1,     5,    -1,    77,    63,
       5,    -1,    76,    -1,    77,    -1,    20,    -1,     8,    -1,
      24,    -1,    15,    61,    75,    62,    -1,    15,    61,    62,
      -1,    21,    61,    75,    62,    -1,    21,    61,    62,    -1,
      16,    61,    74,    63,    78,    62,    -1,    23,    61,    75,
      62,    -1,    15,    61,    71,    85,    62,    -1,    18,    61,
      71,    63,    71,    63,    61,    76,    62,    63,    61,    76,
      62,    62,    -1,    19,    61,    62,    -1,    19,    61,    74,
      62,    -1,    17,    61,    62,    -1,    17,    61,    75,    62,
      -1,    82,    -1,    70,    -1,    25,    83,    -1,    84,    -1,
      83,    84,    -1,    66,    71,    67,    -1,     6,    -1,     7,
      -1,    66,     5,    67,    -1,    66,    13,    67,    -1,    36,
      24,    16,    81,    -1,    24,    -1,    24,    83,    -1,    71,
      47,    88,    -1,    88,    47,    71,    -1,    88,    47,    88,
      -1,     5,    -1,    13,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    94,    94,    98,    99,   100,   101,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   143,   144,   145,   146,   150,   151,   152,   153,
     157,   158,   159,   160,   161,   162,   163,   167,   168,   172,
     173,   174,   175,   176,   177,   178,   179,   183,   184,   188,
     189,   190,   194,   195,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   214,   215,   219,   222,
     223,   227,   228,   229,   230,   231,   235,   239,   240,   244,
     245,   246,   250,   251
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_CONST_INT", "TOK_CONST_FLOAT", 
  "TOK_CONST_STRING", "TOK_SUBKEY", "TOK_DOT_NUMBER", "TOK_ATTR_INT", 
  "TOK_ATTR_BITS", "TOK_ATTR_FLOAT", "TOK_ATTR_MVA32", "TOK_ATTR_MVA64", 
  "TOK_ATTR_STRING", "TOK_ATTR_FACTORS", "TOK_FUNC", "TOK_FUNC_IN", 
  "TOK_FUNC_RAND", "TOK_FUNC_REMAP", "TOK_FUNC_PF", "TOK_USERVAR", 
  "TOK_UDF", "TOK_HOOK_IDENT", "TOK_HOOK_FUNC", "TOK_IDENT", 
  "TOK_ATTR_JSON", "TOK_ATID", "TOK_ATWEIGHT", "TOK_ID", "TOK_GROUPBY", 
  "TOK_WEIGHT", "TOK_COUNT", "TOK_DISTINCT", "TOK_CONST_LIST", 
  "TOK_ATTR_SINT", "TOK_MAP_ARG", "TOK_FOR", "TOK_ITERATOR", "TOK_IS", 
  "TOK_NULL", "TOK_IS_NULL", "TOK_IS_NOT_NULL", "TOK_OR", "TOK_AND", 
  "'|'", "'&'", "TOK_NE", "TOK_EQ", "'<'", "'>'", "TOK_GTE", "TOK_LTE", 
  "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_MOD", "TOK_DIV", "TOK_NOT", 
  "TOK_NEG", "'('", "')'", "','", "'{'", "'}'", "'['", "']'", "$accept", 
  "exprline", "attr", "expr", "maparg", "map_key", "arg", "arglist", 
  "constlist", "stringlist", "constlist_or_uservar", "ident", "function", 
  "json_field", "json_expr", "subscript", "subkey", "for_loop", 
  "iterator", "streq", "strval", 0
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
     295,   296,   297,   298,   124,    38,   299,   300,    60,    62,
     301,   302,    43,    45,    42,    47,    37,   303,   304,   305,
     306,    40,    41,    44,   123,   125,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    68,    69,    70,    70,    70,    70,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    72,    72,    72,    72,    73,    73,    73,    73,
      74,    74,    74,    74,    74,    74,    74,    75,    75,    76,
      76,    76,    76,    76,    76,    76,    76,    77,    77,    78,
      78,    78,    79,    79,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    81,    81,    82,    83,
      83,    84,    84,    84,    84,    84,    85,    86,    86,    87,
      87,    87,    88,    88
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       3,     4,     3,     3,     5,     5,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     3,     1,
       2,     1,     2,     3,     4,     3,     4,     1,     3,     1,
       1,     1,     1,     1,     4,     3,     4,     3,     6,     4,
       5,    14,     3,     4,     3,     4,     1,     1,     2,     1,
       2,     3,     1,     1,     3,     3,     4,     1,     2,     3,
       3,     3,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     9,    10,   102,    11,     3,     4,     5,   103,     0,
       0,     0,     0,     0,     0,    16,     0,    97,     6,    12,
      13,    14,     0,     0,     0,     0,     0,     7,     2,     8,
       0,    37,    38,    39,     0,     0,     0,     0,     0,     0,
       0,     0,    92,    93,     0,    98,    89,    88,     0,    17,
      18,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    56,    53,    54,    52,    55,    75,     0,    50,
      57,     0,    50,     0,    84,     0,     0,    82,     0,    77,
       0,     0,   102,   103,     0,    90,    15,    36,    35,    34,
      26,    25,    33,    32,    99,    23,    24,    31,    30,    19,
      20,    21,    22,    27,    29,    28,    40,     0,   100,   101,
      72,    47,    48,    49,    73,     0,     0,    46,     0,     0,
      74,     0,     0,    85,     0,    83,    76,    79,    94,    95,
      91,    41,     0,    51,     0,     0,    80,    58,    59,    61,
      67,    71,     0,    69,    70,     0,     0,     0,    42,    43,
       0,    60,    62,     0,     0,    78,     0,     0,    87,    96,
      86,    63,    65,     0,    68,     0,    44,    45,    64,    66,
       0,     0,     0,     0,     0,     0,    81
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    26,    27,    82,   125,   126,    80,    81,   153,   154,
     155,   127,    29,    30,    31,    45,    46,   129,    32,    33,
      34
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -164
static const short yypact[] =
{
     386,  -164,  -164,  -164,  -164,  -164,  -164,  -164,  -164,   -46,
     -19,    44,    72,    75,    78,  -164,    83,     4,     4,  -164,
    -164,  -164,    97,   386,   386,   386,    36,   152,   375,  -164,
     156,   157,  -164,  -164,   149,    76,   324,   138,   386,   200,
     262,   324,  -164,  -164,   433,     4,  -164,     4,   136,  -164,
    -164,   313,  -164,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   386,   386,   386,   386,   386,   386,   386,   386,
     -25,   386,   154,  -164,  -164,   159,  -164,  -164,   454,   131,
    -164,   -22,   375,   185,  -164,    10,   189,  -164,   158,  -164,
      12,    15,   162,   182,    65,  -164,  -164,  -164,   452,   467,
     481,   494,   505,   505,  -164,    -7,    -7,    -7,    -7,    70,
      70,  -164,  -164,  -164,  -164,  -164,  -164,   211,   505,  -164,
    -164,  -164,  -164,  -164,  -164,   -44,   204,  -164,   230,   193,
    -164,   324,    23,  -164,   386,  -164,  -164,  -164,  -164,  -164,
    -164,  -164,   454,  -164,     5,   240,  -164,  -164,  -164,  -164,
    -164,  -164,   127,   194,   195,   198,   251,   216,  -164,  -164,
      -3,  -164,  -164,    -1,   263,  -164,   221,     6,  -164,  -164,
    -164,  -164,  -164,   166,  -164,    29,  -164,  -164,  -164,  -164,
     109,   228,   249,    29,   130,   250,  -164
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -164,  -164,   151,     0,  -164,   171,   -35,   -24,  -163,  -164,
    -164,  -164,  -164,   160,   165,   298,    -8,  -164,  -164,  -164,
     -40
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -104
static const short yytable[] =
{
      28,    83,   171,   172,    88,     5,     6,     7,   158,   176,
      42,    43,   180,    85,   116,    35,    90,    91,   104,   142,
     184,   143,    18,    49,    50,    51,   148,   149,   150,   159,
     177,   119,   148,   149,   117,    79,    52,    95,    86,    95,
     130,   131,    36,   151,    94,    63,    64,    65,    66,    67,
      68,    69,   173,    98,    99,   100,   101,   102,   103,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
      44,   118,   133,   131,   136,   131,   152,   137,   131,     1,
       2,    72,   152,     4,     5,     6,     7,    73,    74,    75,
      76,     9,    10,    11,    12,    13,   147,    14,    15,    16,
      17,    18,    19,    20,    21,    37,    22,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    65,    66,    67,    68,    69,    23,
     161,   162,   140,    38,   156,    24,    39,    25,    77,    40,
      78,     1,     2,    72,    41,     4,     5,     6,     7,    73,
      74,    75,    76,     9,    10,    11,    12,    13,    48,    14,
      15,    16,    17,    18,    19,    20,    21,   128,    22,   178,
     179,   181,   163,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
     -87,    23,   185,   163,    70,   -86,    71,    24,    96,    25,
      84,  -102,    78,     1,     2,    72,  -103,     4,     5,     6,
       7,    73,    74,    75,    76,     9,    10,    11,    12,    13,
     135,    14,    15,    16,    17,    18,    19,    20,    21,   138,
      22,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,   132,   139,
     141,   144,   134,    23,   145,   146,   160,   163,   164,    24,
     165,    25,    87,   167,    78,     1,     2,    72,   174,     4,
       5,     6,     7,    73,    74,    75,    76,     9,    10,    11,
      12,    13,   175,    14,    15,    16,    17,    18,    19,    20,
      21,   182,    22,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
     183,   168,   186,   157,   166,    23,    47,     0,     0,     0,
     169,    24,     0,    25,    89,   170,    78,     1,     2,    72,
       0,     4,     5,     6,     7,    73,    74,    75,    76,     9,
      10,    11,    12,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,    97,     0,    23,     0,     0,
       0,     0,     0,    24,     0,    25,     0,     0,    78,     1,
       2,     3,     0,     4,     5,     6,     7,     0,     0,     8,
       0,     9,    10,    11,    12,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,     0,    22,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,     0,     0,     1,     2,    92,    23,
       4,     5,     6,     7,     0,    24,    93,    25,     9,    10,
      11,    12,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,   120,    22,     0,     0,     0,   121,     0,     0,
     122,   123,     0,     0,     0,     0,     0,     0,   124,     0,
       0,     0,     0,     0,     0,     0,    23,     0,     0,     0,
       0,     0,    24,     0,    25,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69
};

static const short yycheck[] =
{
       0,    36,     3,     4,    39,     8,     9,    10,     3,     3,
       6,     7,   175,    37,    39,    61,    40,    41,    58,    63,
     183,    65,    25,    23,    24,    25,     3,     4,     5,    24,
      24,    71,     3,     4,    59,    35,     0,    45,    38,    47,
      62,    63,    61,    20,    44,    52,    53,    54,    55,    56,
      57,    58,    53,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      66,    71,    62,    63,    62,    63,    53,    62,    63,     3,
       4,     5,    53,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,   131,    21,    22,    23,
      24,    25,    26,    27,    28,    61,    30,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    54,    55,    56,    57,    58,    53,
       3,     4,    67,    61,   134,    59,    61,    61,    62,    61,
      64,     3,     4,     5,    61,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    61,    21,
      22,    23,    24,    25,    26,    27,    28,    36,    30,     3,
       4,    62,    63,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      38,    53,    62,    63,    38,    38,    47,    59,    62,    61,
      62,    47,    64,     3,     4,     5,    47,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      62,    21,    22,    23,    24,    25,    26,    27,    28,    67,
      30,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    63,    67,
      39,    47,    63,    53,    24,    62,    16,    63,    63,    59,
      62,    61,    62,    47,    64,     3,     4,     5,     5,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    61,    21,    22,    23,    24,    25,    26,    27,
      28,    63,    30,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      61,   160,    62,   142,    63,    53,    18,    -1,    -1,    -1,
     160,    59,    -1,    61,    62,   160,    64,     3,     4,     5,
      -1,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    -1,    -1,    -1,    62,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    59,    -1,    61,    -1,    -1,    64,     3,
       4,     5,    -1,     7,     8,     9,    10,    -1,    -1,    13,
      -1,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    -1,     3,     4,     5,    53,
       7,     8,     9,    10,    -1,    59,    13,    61,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,     8,    30,    -1,    -1,    -1,    13,    -1,    -1,
      16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    59,    -1,    61,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     7,     8,     9,    10,    13,    15,
      16,    17,    18,    19,    21,    22,    23,    24,    25,    26,
      27,    28,    30,    53,    59,    61,    69,    70,    71,    80,
      81,    82,    86,    87,    88,    61,    61,    61,    61,    61,
      61,    61,     6,     7,    66,    83,    84,    83,    61,    71,
      71,    71,     0,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      38,    47,     5,    11,    12,    13,    14,    62,    64,    71,
      74,    75,    71,    74,    62,    75,    71,    62,    74,    62,
      75,    75,     5,    13,    71,    84,    62,    62,    71,    71,
      71,    71,    71,    71,    88,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    39,    59,    71,    88,
       8,    13,    16,    17,    24,    72,    73,    79,    36,    85,
      62,    63,    63,    62,    63,    62,    62,    62,    67,    67,
      67,    39,    63,    65,    47,    24,    62,    74,     3,     4,
       5,    20,    53,    76,    77,    78,    71,    73,     3,    24,
      16,     3,     4,    63,    63,    62,    63,    47,    70,    81,
      82,     3,     4,    53,     5,    61,     3,    24,     3,     4,
      76,    62,    63,    61,    76,    62,    62
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

  case 6:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_JSON, yyvsp[0].iAttrLocator ); ;}
    break;

  case 9:

    { yyval.iNode = pParser->AddNodeInt ( yyvsp[0].iConst ); ;}
    break;

  case 10:

    { yyval.iNode = pParser->AddNodeFloat ( yyvsp[0].fConst ); ;}
    break;

  case 11:

    { yyval.iNode = pParser->AddNodeDotNumber ( yyvsp[0].iConst ); ;}
    break;

  case 12:

    { yyval.iNode = pParser->AddNodeID(); ;}
    break;

  case 13:

    { yyval.iNode = pParser->AddNodeWeight(); ;}
    break;

  case 14:

    { yyval.iNode = pParser->AddNodeID(); ;}
    break;

  case 15:

    { yyval.iNode = pParser->AddNodeWeight(); ;}
    break;

  case 16:

    { yyval.iNode = pParser->AddNodeHookIdent ( yyvsp[0].iNode ); ;}
    break;

  case 17:

    { yyval.iNode = pParser->AddNodeOp ( TOK_NEG, yyvsp[0].iNode, -1 ); ;}
    break;

  case 18:

    { yyval.iNode = pParser->AddNodeOp ( TOK_NOT, yyvsp[0].iNode, -1 ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 19:

    { yyval.iNode = pParser->AddNodeOp ( '+', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 20:

    { yyval.iNode = pParser->AddNodeOp ( '-', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 21:

    { yyval.iNode = pParser->AddNodeOp ( '*', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 22:

    { yyval.iNode = pParser->AddNodeOp ( '/', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 23:

    { yyval.iNode = pParser->AddNodeOp ( '<', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 24:

    { yyval.iNode = pParser->AddNodeOp ( '>', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 25:

    { yyval.iNode = pParser->AddNodeOp ( '&', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 26:

    { yyval.iNode = pParser->AddNodeOp ( '|', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 27:

    { yyval.iNode = pParser->AddNodeOp ( '%', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 28:

    { yyval.iNode = pParser->AddNodeFunc ( FUNC_IDIV, pParser->AddNodeOp ( ',', yyvsp[-2].iNode, yyvsp[0].iNode ) ); ;}
    break;

  case 29:

    { yyval.iNode = pParser->AddNodeOp ( '%', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 30:

    { yyval.iNode = pParser->AddNodeOp ( TOK_LTE, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 31:

    { yyval.iNode = pParser->AddNodeOp ( TOK_GTE, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 32:

    { yyval.iNode = pParser->AddNodeOp ( TOK_EQ, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 33:

    { yyval.iNode = pParser->AddNodeOp ( TOK_NE, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 34:

    { yyval.iNode = pParser->AddNodeOp ( TOK_AND, yyvsp[-2].iNode, yyvsp[0].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 35:

    { yyval.iNode = pParser->AddNodeOp ( TOK_OR, yyvsp[-2].iNode, yyvsp[0].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 36:

    { yyval.iNode = yyvsp[-1].iNode; ;}
    break;

  case 40:

    { yyval.iNode = pParser->AddNodeOp ( TOK_IS_NULL, yyvsp[-2].iNode, -1); ;}
    break;

  case 41:

    { yyval.iNode = pParser->AddNodeOp ( TOK_IS_NOT_NULL, yyvsp[-3].iNode, -1); ;}
    break;

  case 42:

    { yyval.iNode = pParser->AddNodeMapArg ( yyvsp[-2].sIdent, NULL, yyvsp[0].iConst ); ;}
    break;

  case 43:

    { yyval.iNode = pParser->AddNodeMapArg ( yyvsp[-2].sIdent, yyvsp[0].sIdent, 0 ); ;}
    break;

  case 44:

    { pParser->AppendToMapArg ( yyval.iNode, yyvsp[-2].sIdent, NULL, yyvsp[0].iConst ); ;}
    break;

  case 45:

    { pParser->AppendToMapArg ( yyval.iNode, yyvsp[-2].sIdent, yyvsp[0].sIdent, 0 ); ;}
    break;

  case 46:

    { yyval.sIdent = yyvsp[0].sIdent; ;}
    break;

  case 47:

    { yyval.sIdent = pParser->Attr2Ident(yyvsp[0].iAttrLocator); ;}
    break;

  case 48:

    { yyval.sIdent = strdup("in"); ;}
    break;

  case 49:

    { yyval.sIdent = strdup("rand"); ;}
    break;

  case 51:

    { yyval.iNode = yyvsp[-1].iNode; ;}
    break;

  case 52:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_STRING, yyvsp[0].iAttrLocator ); ;}
    break;

  case 53:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_MVA32, yyvsp[0].iAttrLocator ); ;}
    break;

  case 54:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_MVA64, yyvsp[0].iAttrLocator ); ;}
    break;

  case 55:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_FACTORS, yyvsp[0].iAttrLocator ); ;}
    break;

  case 56:

    { yyval.iNode = pParser->AddNodeString ( yyvsp[0].iConst ); ;}
    break;

  case 57:

    { yyval.iNode = yyvsp[0].iNode; ;}
    break;

  case 58:

    { yyval.iNode = pParser->AddNodeOp ( ',', yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 59:

    { yyval.iNode = pParser->AddNodeConstlist ( yyvsp[0].iConst ); ;}
    break;

  case 60:

    { yyval.iNode = pParser->AddNodeConstlist ( -yyvsp[0].iConst );;}
    break;

  case 61:

    { yyval.iNode = pParser->AddNodeConstlist ( yyvsp[0].fConst ); ;}
    break;

  case 62:

    { yyval.iNode = pParser->AddNodeConstlist ( -yyvsp[0].fConst );;}
    break;

  case 63:

    { pParser->AppendToConstlist ( yyval.iNode, yyvsp[0].iConst ); ;}
    break;

  case 64:

    { pParser->AppendToConstlist ( yyval.iNode, -yyvsp[0].iConst );;}
    break;

  case 65:

    { pParser->AppendToConstlist ( yyval.iNode, yyvsp[0].fConst ); ;}
    break;

  case 66:

    { pParser->AppendToConstlist ( yyval.iNode, -yyvsp[0].fConst );;}
    break;

  case 67:

    { yyval.iNode = pParser->AddNodeConstlist ( yyvsp[0].iConst ); ;}
    break;

  case 68:

    { pParser->AppendToConstlist ( yyval.iNode, yyvsp[0].iConst ); ;}
    break;

  case 71:

    { yyval.iNode = pParser->AddNodeUservar ( yyvsp[0].iNode ); ;}
    break;

  case 72:

    { yyval.sIdent = pParser->Attr2Ident ( yyvsp[0].iAttrLocator ); ;}
    break;

  case 74:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-3].iFunc, yyvsp[-1].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 75:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-2].iFunc, -1 ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 76:

    { yyval.iNode = pParser->AddNodeUdf ( yyvsp[-3].iNode, yyvsp[-1].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 77:

    { yyval.iNode = pParser->AddNodeUdf ( yyvsp[-2].iNode, -1 ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 78:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-5].iFunc, yyvsp[-3].iNode, yyvsp[-1].iNode ); ;}
    break;

  case 79:

    { yyval.iNode = pParser->AddNodeHookFunc ( yyvsp[-3].iNode, yyvsp[-1].iNode ); if ( yyval.iNode<0 ) YYERROR; ;}
    break;

  case 80:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-4].iFunc, yyvsp[-2].iNode, yyvsp[-1].iNode ); ;}
    break;

  case 81:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-13].iFunc, yyvsp[-11].iNode, yyvsp[-9].iNode, yyvsp[-6].iNode, yyvsp[-2].iNode ); ;}
    break;

  case 82:

    { yyval.iNode = pParser->AddNodePF ( yyvsp[-2].iNode, -1 ); ;}
    break;

  case 83:

    { yyval.iNode = pParser->AddNodePF ( yyvsp[-3].iNode, yyvsp[-1].iNode ); ;}
    break;

  case 84:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-2].iFunc, -1 ); ;}
    break;

  case 85:

    { yyval.iNode = pParser->AddNodeFunc ( yyvsp[-3].iFunc, yyvsp[-1].iNode ); ;}
    break;

  case 88:

    { yyval.iNode = pParser->AddNodeJsonField ( yyvsp[-1].iAttrLocator, yyvsp[0].iNode ); ;}
    break;

  case 90:

    { yyval.iNode = pParser->AddNodeOp ( ',', yyvsp[-1].iNode, yyvsp[0].iNode ); ;}
    break;

  case 91:

    { yyval.iNode = yyvsp[-1].iNode; ;}
    break;

  case 92:

    { yyval.iNode = pParser->AddNodeJsonSubkey ( yyvsp[0].iConst ); ;}
    break;

  case 93:

    { yyval.iNode = pParser->AddNodeJsonSubkey ( yyvsp[0].iConst ); ;}
    break;

  case 94:

    { yyval.iNode = pParser->AddNodeString ( yyvsp[-1].iConst ); ;}
    break;

  case 95:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_STRING, yyvsp[-1].iAttrLocator ); ;}
    break;

  case 96:

    { yyval.iNode = pParser->AddNodeIdent ( yyvsp[-2].sIdent, yyvsp[0].iNode ); ;}
    break;

  case 97:

    { yyval.iNode = pParser->AddNodeIdent ( yyvsp[0].sIdent, -1 ); ;}
    break;

  case 98:

    { yyval.iNode = pParser->AddNodeIdent ( yyvsp[-1].sIdent, yyvsp[0].iNode ); ;}
    break;

  case 99:

    { yyval.iNode = pParser->AddNodeOp ( TOK_EQ, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 100:

    { yyval.iNode = pParser->AddNodeOp ( TOK_EQ, yyvsp[0].iNode, yyvsp[-2].iNode ); ;}
    break;

  case 101:

    { yyval.iNode = pParser->AddNodeOp ( TOK_EQ, yyvsp[-2].iNode, yyvsp[0].iNode ); ;}
    break;

  case 102:

    { yyval.iNode = pParser->AddNodeString ( yyvsp[0].iConst ); ;}
    break;

  case 103:

    { yyval.iNode = pParser->AddNodeAttr ( TOK_ATTR_STRING, yyvsp[0].iAttrLocator ); ;}
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





