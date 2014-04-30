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
     TOK_BAD_NUMERIC = 267,
     TOK_SUBKEY = 268,
     TOK_DOT_NUMBER = 269,
     TOK_ADD = 270,
     TOK_AGENT = 271,
     TOK_ALTER = 272,
     TOK_AS = 273,
     TOK_ASC = 274,
     TOK_ATTACH = 275,
     TOK_AVG = 276,
     TOK_BEGIN = 277,
     TOK_BETWEEN = 278,
     TOK_BIGINT = 279,
     TOK_BOOL = 280,
     TOK_BY = 281,
     TOK_CALL = 282,
     TOK_CHARACTER = 283,
     TOK_COLLATION = 284,
     TOK_COLUMN = 285,
     TOK_COMMIT = 286,
     TOK_COMMITTED = 287,
     TOK_COUNT = 288,
     TOK_CREATE = 289,
     TOK_DATABASES = 290,
     TOK_DELETE = 291,
     TOK_DESC = 292,
     TOK_DESCRIBE = 293,
     TOK_DISTINCT = 294,
     TOK_DIV = 295,
     TOK_DOUBLE = 296,
     TOK_DROP = 297,
     TOK_FACET = 298,
     TOK_FALSE = 299,
     TOK_FLOAT = 300,
     TOK_FLUSH = 301,
     TOK_FOR = 302,
     TOK_FROM = 303,
     TOK_FUNCTION = 304,
     TOK_GLOBAL = 305,
     TOK_GROUP = 306,
     TOK_GROUPBY = 307,
     TOK_GROUP_CONCAT = 308,
     TOK_HAVING = 309,
     TOK_ID = 310,
     TOK_IN = 311,
     TOK_INDEX = 312,
     TOK_INSERT = 313,
     TOK_INT = 314,
     TOK_INTEGER = 315,
     TOK_INTO = 316,
     TOK_IS = 317,
     TOK_ISOLATION = 318,
     TOK_JSON = 319,
     TOK_LEVEL = 320,
     TOK_LIKE = 321,
     TOK_LIMIT = 322,
     TOK_MATCH = 323,
     TOK_MAX = 324,
     TOK_META = 325,
     TOK_MIN = 326,
     TOK_MOD = 327,
     TOK_MULTI = 328,
     TOK_MULTI64 = 329,
     TOK_NAMES = 330,
     TOK_NULL = 331,
     TOK_OPTION = 332,
     TOK_ORDER = 333,
     TOK_OPTIMIZE = 334,
     TOK_PLAN = 335,
     TOK_PLUGIN = 336,
     TOK_PLUGINS = 337,
     TOK_PROFILE = 338,
     TOK_RAND = 339,
     TOK_RAMCHUNK = 340,
     TOK_READ = 341,
     TOK_REPEATABLE = 342,
     TOK_REPLACE = 343,
     TOK_REMAP = 344,
     TOK_RETURNS = 345,
     TOK_ROLLBACK = 346,
     TOK_RTINDEX = 347,
     TOK_SELECT = 348,
     TOK_SERIALIZABLE = 349,
     TOK_SET = 350,
     TOK_SESSION = 351,
     TOK_SHOW = 352,
     TOK_SONAME = 353,
     TOK_START = 354,
     TOK_STATUS = 355,
     TOK_STRING = 356,
     TOK_SUM = 357,
     TOK_TABLE = 358,
     TOK_TABLES = 359,
     TOK_THREADS = 360,
     TOK_TO = 361,
     TOK_TRANSACTION = 362,
     TOK_TRUE = 363,
     TOK_TRUNCATE = 364,
     TOK_TYPE = 365,
     TOK_UNCOMMITTED = 366,
     TOK_UPDATE = 367,
     TOK_VALUES = 368,
     TOK_VARIABLES = 369,
     TOK_WARNINGS = 370,
     TOK_WEIGHT = 371,
     TOK_WHERE = 372,
     TOK_WITHIN = 373,
     TOK_OR = 374,
     TOK_AND = 375,
     TOK_NE = 376,
     TOK_GTE = 377,
     TOK_LTE = 378,
     TOK_NOT = 379,
     TOK_NEG = 380
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
#define TOK_BAD_NUMERIC 267
#define TOK_SUBKEY 268
#define TOK_DOT_NUMBER 269
#define TOK_ADD 270
#define TOK_AGENT 271
#define TOK_ALTER 272
#define TOK_AS 273
#define TOK_ASC 274
#define TOK_ATTACH 275
#define TOK_AVG 276
#define TOK_BEGIN 277
#define TOK_BETWEEN 278
#define TOK_BIGINT 279
#define TOK_BOOL 280
#define TOK_BY 281
#define TOK_CALL 282
#define TOK_CHARACTER 283
#define TOK_COLLATION 284
#define TOK_COLUMN 285
#define TOK_COMMIT 286
#define TOK_COMMITTED 287
#define TOK_COUNT 288
#define TOK_CREATE 289
#define TOK_DATABASES 290
#define TOK_DELETE 291
#define TOK_DESC 292
#define TOK_DESCRIBE 293
#define TOK_DISTINCT 294
#define TOK_DIV 295
#define TOK_DOUBLE 296
#define TOK_DROP 297
#define TOK_FACET 298
#define TOK_FALSE 299
#define TOK_FLOAT 300
#define TOK_FLUSH 301
#define TOK_FOR 302
#define TOK_FROM 303
#define TOK_FUNCTION 304
#define TOK_GLOBAL 305
#define TOK_GROUP 306
#define TOK_GROUPBY 307
#define TOK_GROUP_CONCAT 308
#define TOK_HAVING 309
#define TOK_ID 310
#define TOK_IN 311
#define TOK_INDEX 312
#define TOK_INSERT 313
#define TOK_INT 314
#define TOK_INTEGER 315
#define TOK_INTO 316
#define TOK_IS 317
#define TOK_ISOLATION 318
#define TOK_JSON 319
#define TOK_LEVEL 320
#define TOK_LIKE 321
#define TOK_LIMIT 322
#define TOK_MATCH 323
#define TOK_MAX 324
#define TOK_META 325
#define TOK_MIN 326
#define TOK_MOD 327
#define TOK_MULTI 328
#define TOK_MULTI64 329
#define TOK_NAMES 330
#define TOK_NULL 331
#define TOK_OPTION 332
#define TOK_ORDER 333
#define TOK_OPTIMIZE 334
#define TOK_PLAN 335
#define TOK_PLUGIN 336
#define TOK_PLUGINS 337
#define TOK_PROFILE 338
#define TOK_RAND 339
#define TOK_RAMCHUNK 340
#define TOK_READ 341
#define TOK_REPEATABLE 342
#define TOK_REPLACE 343
#define TOK_REMAP 344
#define TOK_RETURNS 345
#define TOK_ROLLBACK 346
#define TOK_RTINDEX 347
#define TOK_SELECT 348
#define TOK_SERIALIZABLE 349
#define TOK_SET 350
#define TOK_SESSION 351
#define TOK_SHOW 352
#define TOK_SONAME 353
#define TOK_START 354
#define TOK_STATUS 355
#define TOK_STRING 356
#define TOK_SUM 357
#define TOK_TABLE 358
#define TOK_TABLES 359
#define TOK_THREADS 360
#define TOK_TO 361
#define TOK_TRANSACTION 362
#define TOK_TRUE 363
#define TOK_TRUNCATE 364
#define TOK_TYPE 365
#define TOK_UNCOMMITTED 366
#define TOK_UPDATE 367
#define TOK_VALUES 368
#define TOK_VARIABLES 369
#define TOK_WARNINGS 370
#define TOK_WEIGHT 371
#define TOK_WHERE 372
#define TOK_WITHIN 373
#define TOK_OR 374
#define TOK_AND 375
#define TOK_NE 376
#define TOK_GTE 377
#define TOK_LTE 378
#define TOK_NOT 379
#define TOK_NEG 380




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif



// some helpers
#include <float.h> // for FLT_MAX

static void AddInsval ( SqlParser_c * pParser, CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iType;
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	if ( tIns.m_iType==TOK_QUOTED_STRING )
		pParser->ToStringUnescape ( tIns.m_sVal, tNode );
	tIns.m_pVals = tNode.m_pValues;
}

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	_res.m_iEnd = _right.m_iEnd; \
	_res.m_iType = 0;



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
#define YYFINAL  187
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4374

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  145
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  132
/* YYNRULES -- Number of rules. */
#define YYNRULES  422
/* YYNRULES -- Number of states. */
#define YYNSTATES  771

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   380

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   133,   122,     2,
     137,   138,   131,   129,   139,   130,   142,   132,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   136,
     125,   123,   126,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   143,     2,   144,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   140,   121,   141,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   124,   127,   128,   134,
     135
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    56,    58,
      60,    62,    64,    66,    68,    70,    72,    74,    76,    78,
      80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
     100,   102,   104,   106,   108,   110,   112,   114,   116,   118,
     120,   122,   124,   126,   128,   130,   132,   134,   136,   138,
     140,   142,   144,   146,   148,   150,   152,   154,   156,   158,
     160,   162,   164,   168,   171,   173,   175,   177,   186,   188,
     198,   199,   202,   204,   208,   210,   212,   214,   215,   219,
     220,   223,   228,   240,   242,   246,   248,   251,   252,   254,
     257,   259,   264,   269,   274,   279,   284,   289,   293,   299,
     301,   305,   306,   308,   311,   313,   317,   321,   326,   328,
     332,   336,   342,   349,   353,   358,   364,   368,   372,   376,
     380,   384,   386,   392,   398,   404,   408,   412,   416,   420,
     424,   428,   432,   437,   441,   443,   445,   450,   454,   458,
     460,   462,   467,   472,   477,   481,   483,   486,   488,   491,
     493,   495,   499,   500,   505,   506,   508,   510,   514,   515,
     518,   519,   521,   527,   528,   530,   534,   540,   542,   546,
     548,   551,   554,   555,   557,   560,   565,   566,   568,   571,
     573,   577,   581,   585,   591,   598,   602,   604,   608,   612,
     614,   616,   618,   620,   622,   624,   626,   629,   632,   636,
     640,   644,   648,   652,   656,   660,   664,   668,   672,   676,
     680,   684,   688,   692,   696,   700,   704,   708,   710,   712,
     714,   718,   723,   728,   733,   738,   743,   748,   753,   757,
     764,   771,   775,   784,   799,   801,   805,   807,   809,   813,
     819,   821,   823,   825,   827,   830,   831,   834,   836,   839,
     842,   846,   848,   850,   852,   855,   860,   865,   869,   874,
     879,   884,   888,   893,   898,   906,   912,   918,   928,   930,
     932,   934,   936,   938,   940,   944,   946,   948,   950,   952,
     954,   956,   958,   960,   962,   965,   973,   975,   977,   978,
     982,   984,   986,   988,   992,   994,   998,  1002,  1004,  1008,
    1010,  1012,  1014,  1018,  1021,  1022,  1025,  1027,  1031,  1035,
    1040,  1047,  1049,  1053,  1055,  1059,  1061,  1065,  1066,  1069,
    1071,  1075,  1079,  1080,  1082,  1084,  1086,  1090,  1092,  1094,
    1098,  1102,  1109,  1111,  1115,  1119,  1123,  1129,  1134,  1138,
    1142,  1144,  1146,  1148,  1150,  1152,  1154,  1156,  1158,  1166,
    1173,  1178,  1184,  1185,  1187,  1190,  1192,  1196,  1200,  1203,
    1207,  1214,  1215,  1217,  1219,  1222,  1225,  1228,  1230,  1238,
    1240,  1242,  1244,  1246,  1250,  1257,  1261,  1265,  1269,  1271,
    1275,  1278,  1282,  1286,  1294,  1300,  1303,  1305,  1308,  1310,
    1312,  1316,  1320,  1324,  1328,  1330,  1331,  1334,  1336,  1339,
    1341,  1343,  1347
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     146,     0,    -1,   147,    -1,   150,    -1,   150,   136,    -1,
     212,    -1,   224,    -1,   204,    -1,   205,    -1,   210,    -1,
     225,    -1,   234,    -1,   236,    -1,   237,    -1,   238,    -1,
     243,    -1,   248,    -1,   249,    -1,   253,    -1,   255,    -1,
     256,    -1,   257,    -1,   258,    -1,   250,    -1,   259,    -1,
     261,    -1,   262,    -1,   263,    -1,   242,    -1,   264,    -1,
     265,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    29,    -1,    33,    -1,    46,    -1,
      49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,
      63,    -1,    65,    -1,    66,    -1,    68,    -1,    69,    -1,
      70,    -1,    80,    -1,    81,    -1,    82,    -1,    83,    -1,
      85,    -1,    84,    -1,    86,    -1,    87,    -1,    90,    -1,
      91,    -1,    92,    -1,    94,    -1,    96,    -1,    99,    -1,
     100,    -1,   101,    -1,   102,    -1,   104,    -1,   105,    -1,
     109,    -1,   110,    -1,   111,    -1,   114,    -1,   115,    -1,
     116,    -1,   118,    -1,    64,    -1,   148,    -1,    75,    -1,
     107,    -1,   151,    -1,   150,   136,   151,    -1,   150,   276,
      -1,   152,    -1,   201,    -1,   153,    -1,    93,     3,   137,
     137,   153,   138,   154,   138,    -1,   160,    -1,    93,   161,
      48,   137,   157,   160,   138,   158,   159,    -1,    -1,   139,
     155,    -1,   156,    -1,   155,   139,   156,    -1,   149,    -1,
       5,    -1,    55,    -1,    -1,    78,    26,   184,    -1,    -1,
      67,     5,    -1,    67,     5,   139,     5,    -1,    93,   161,
      48,   165,   166,   176,   180,   179,   182,   186,   188,    -1,
     162,    -1,   161,   139,   162,    -1,   131,    -1,   164,   163,
      -1,    -1,   149,    -1,    18,   149,    -1,   194,    -1,    21,
     137,   194,   138,    -1,    69,   137,   194,   138,    -1,    71,
     137,   194,   138,    -1,   102,   137,   194,   138,    -1,    53,
     137,   194,   138,    -1,    33,   137,   131,   138,    -1,    52,
     137,   138,    -1,    33,   137,    39,   149,   138,    -1,   149,
      -1,   165,   139,   149,    -1,    -1,   167,    -1,   117,   168,
      -1,   169,    -1,   168,   120,   168,    -1,   137,   168,   138,
      -1,    68,   137,     8,   138,    -1,   170,    -1,   172,   123,
     173,    -1,   172,   124,   173,    -1,   172,    56,   137,   175,
     138,    -1,   172,   134,    56,   137,   175,   138,    -1,   172,
      56,     9,    -1,   172,   134,    56,     9,    -1,   172,    23,
     173,   120,   173,    -1,   172,   126,   173,    -1,   172,   125,
     173,    -1,   172,   127,   173,    -1,   172,   128,   173,    -1,
     172,   123,   174,    -1,   171,    -1,   172,    23,   174,   120,
     174,    -1,   172,    23,   173,   120,   174,    -1,   172,    23,
     174,   120,   173,    -1,   172,   126,   174,    -1,   172,   125,
     174,    -1,   172,   127,   174,    -1,   172,   128,   174,    -1,
     172,   123,     8,    -1,   172,   124,     8,    -1,   172,    62,
      76,    -1,   172,    62,   134,    76,    -1,   172,   124,   174,
      -1,   149,    -1,     4,    -1,    33,   137,   131,   138,    -1,
      52,   137,   138,    -1,   116,   137,   138,    -1,    55,    -1,
     266,    -1,    60,   137,   266,   138,    -1,    41,   137,   266,
     138,    -1,    24,   137,   266,   138,    -1,    43,   137,   138,
      -1,     5,    -1,   130,     5,    -1,     6,    -1,   130,     6,
      -1,    14,    -1,   173,    -1,   175,   139,   173,    -1,    -1,
      51,   177,    26,   178,    -1,    -1,     5,    -1,   172,    -1,
     178,   139,   172,    -1,    -1,    54,   170,    -1,    -1,   181,
      -1,   118,    51,    78,    26,   184,    -1,    -1,   183,    -1,
      78,    26,   184,    -1,    78,    26,    84,   137,   138,    -1,
     185,    -1,   184,   139,   185,    -1,   172,    -1,   172,    19,
      -1,   172,    37,    -1,    -1,   187,    -1,    67,     5,    -1,
      67,     5,   139,     5,    -1,    -1,   189,    -1,    77,   190,
      -1,   191,    -1,   190,   139,   191,    -1,   149,   123,   149,
      -1,   149,   123,     5,    -1,   149,   123,   137,   192,   138,
      -1,   149,   123,   149,   137,     8,   138,    -1,   149,   123,
       8,    -1,   193,    -1,   192,   139,   193,    -1,   149,   123,
     173,    -1,   149,    -1,     4,    -1,    55,    -1,     5,    -1,
       6,    -1,    14,    -1,     9,    -1,   130,   194,    -1,   134,
     194,    -1,   194,   129,   194,    -1,   194,   130,   194,    -1,
     194,   131,   194,    -1,   194,   132,   194,    -1,   194,   125,
     194,    -1,   194,   126,   194,    -1,   194,   122,   194,    -1,
     194,   121,   194,    -1,   194,   133,   194,    -1,   194,    40,
     194,    -1,   194,    72,   194,    -1,   194,   128,   194,    -1,
     194,   127,   194,    -1,   194,   123,   194,    -1,   194,   124,
     194,    -1,   194,   120,   194,    -1,   194,   119,   194,    -1,
     137,   194,   138,    -1,   140,   198,   141,    -1,   195,    -1,
     266,    -1,   269,    -1,   266,    62,    76,    -1,   266,    62,
     134,    76,    -1,     3,   137,   196,   138,    -1,    56,   137,
     196,   138,    -1,    60,   137,   196,   138,    -1,    24,   137,
     196,   138,    -1,    45,   137,   196,   138,    -1,    41,   137,
     196,   138,    -1,     3,   137,   138,    -1,    71,   137,   194,
     139,   194,   138,    -1,    69,   137,   194,   139,   194,   138,
      -1,   116,   137,   138,    -1,     3,   137,   194,    47,   149,
      56,   266,   138,    -1,    89,   137,   194,   139,   194,   139,
     137,   196,   138,   139,   137,   196,   138,   138,    -1,   197,
      -1,   196,   139,   197,    -1,   194,    -1,     8,    -1,   199,
     123,   200,    -1,   198,   139,   199,   123,   200,    -1,   149,
      -1,    56,    -1,   173,    -1,   149,    -1,    97,   203,    -1,
      -1,    66,     8,    -1,   115,    -1,   100,   202,    -1,    70,
     202,    -1,    16,   100,   202,    -1,    83,    -1,    80,    -1,
      82,    -1,   105,   188,    -1,    16,     8,   100,   202,    -1,
      16,   149,   100,   202,    -1,    57,   149,   100,    -1,    95,
     148,   123,   207,    -1,    95,   148,   123,   206,    -1,    95,
     148,   123,    76,    -1,    95,    75,   208,    -1,    95,    10,
     123,   208,    -1,    95,    28,    95,   208,    -1,    95,    50,
       9,   123,   137,   175,   138,    -1,    95,    50,   148,   123,
     206,    -1,    95,    50,   148,   123,     5,    -1,    95,    57,
     149,    50,     9,   123,   137,   175,   138,    -1,   149,    -1,
       8,    -1,   108,    -1,    44,    -1,   173,    -1,   209,    -1,
     208,   130,   209,    -1,   149,    -1,    76,    -1,     8,    -1,
       5,    -1,     6,    -1,    31,    -1,    91,    -1,   211,    -1,
      22,    -1,    99,   107,    -1,   213,    61,   149,   214,   113,
     217,   221,    -1,    58,    -1,    88,    -1,    -1,   137,   216,
     138,    -1,   149,    -1,    55,    -1,   215,    -1,   216,   139,
     215,    -1,   218,    -1,   217,   139,   218,    -1,   137,   219,
     138,    -1,   220,    -1,   219,   139,   220,    -1,   173,    -1,
     174,    -1,     8,    -1,   137,   175,   138,    -1,   137,   138,
      -1,    -1,    77,   222,    -1,   223,    -1,   222,   139,   223,
      -1,     3,   123,     8,    -1,    36,    48,   165,   167,    -1,
      27,   149,   137,   226,   229,   138,    -1,   227,    -1,   226,
     139,   227,    -1,   220,    -1,   137,   228,   138,    -1,     8,
      -1,   228,   139,     8,    -1,    -1,   139,   230,    -1,   231,
      -1,   230,   139,   231,    -1,   220,   232,   233,    -1,    -1,
      18,    -1,   149,    -1,    67,    -1,   235,   149,   202,    -1,
      38,    -1,    37,    -1,    97,   104,   202,    -1,    97,    35,
     202,    -1,   112,   165,    95,   239,   167,   188,    -1,   240,
      -1,   239,   139,   240,    -1,   149,   123,   173,    -1,   149,
     123,   174,    -1,   149,   123,   137,   175,   138,    -1,   149,
     123,   137,   138,    -1,   266,   123,   173,    -1,   266,   123,
     174,    -1,    60,    -1,    24,    -1,    45,    -1,    25,    -1,
      73,    -1,    74,    -1,    64,    -1,   101,    -1,    17,   103,
     149,    15,    30,   149,   241,    -1,    17,   103,   149,    42,
      30,   149,    -1,    97,   251,   114,   244,    -1,    97,   251,
     114,    66,     8,    -1,    -1,   245,    -1,   117,   246,    -1,
     247,    -1,   246,   119,   247,    -1,   149,   123,     8,    -1,
      97,    29,    -1,    97,    28,    95,    -1,    95,   251,   107,
      63,    65,   252,    -1,    -1,    50,    -1,    96,    -1,    86,
     111,    -1,    86,    32,    -1,    87,    86,    -1,    94,    -1,
      34,    49,   149,    90,   254,    98,     8,    -1,    59,    -1,
      24,    -1,    45,    -1,   101,    -1,    42,    49,   149,    -1,
      20,    57,   149,   106,    92,   149,    -1,    46,    92,   149,
      -1,    46,    85,   149,    -1,    93,   260,   186,    -1,    10,
      -1,    10,   142,   149,    -1,    93,   194,    -1,   109,    92,
     149,    -1,    79,    57,   149,    -1,    34,    81,   149,   110,
       8,    98,     8,    -1,    42,    81,   149,   110,     8,    -1,
     149,   267,    -1,   268,    -1,   267,   268,    -1,    13,    -1,
      14,    -1,   143,   194,   144,    -1,   143,     8,   144,    -1,
     194,   123,   270,    -1,   270,   123,   194,    -1,     8,    -1,
      -1,   272,   275,    -1,    26,    -1,   274,   163,    -1,   194,
      -1,   273,    -1,   275,   139,   273,    -1,    43,   275,   271,
     182,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   171,   171,   172,   173,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   213,   214,   214,   214,   214,   214,   215,   215,   215,
     215,   216,   216,   216,   216,   216,   217,   217,   217,   217,
     217,   218,   218,   218,   218,   218,   218,   218,   219,   219,
     219,   219,   220,   220,   220,   220,   220,   221,   221,   221,
     221,   221,   221,   222,   222,   222,   222,   223,   227,   227,
     227,   233,   234,   235,   239,   240,   244,   245,   253,   254,
     261,   263,   267,   271,   278,   279,   280,   284,   297,   304,
     306,   311,   320,   336,   337,   341,   342,   345,   347,   348,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   364,
     365,   368,   370,   374,   378,   379,   380,   384,   389,   393,
     400,   408,   416,   425,   430,   435,   440,   445,   450,   455,
     460,   465,   470,   475,   480,   485,   490,   495,   500,   505,
     510,   515,   520,   528,   532,   533,   538,   544,   550,   556,
     562,   563,   564,   565,   566,   570,   571,   582,   583,   584,
     588,   594,   600,   602,   605,   607,   614,   618,   624,   626,
     632,   634,   638,   649,   651,   655,   659,   666,   667,   671,
     672,   673,   676,   678,   682,   687,   694,   696,   700,   704,
     705,   709,   714,   719,   725,   730,   738,   743,   750,   760,
     761,   762,   763,   764,   765,   766,   767,   768,   770,   771,
     772,   773,   774,   775,   776,   777,   778,   779,   780,   781,
     782,   783,   784,   785,   786,   787,   788,   789,   790,   791,
     792,   793,   797,   798,   799,   800,   801,   802,   803,   804,
     805,   806,   807,   808,   812,   813,   817,   818,   822,   823,
     827,   828,   832,   833,   839,   842,   844,   848,   849,   850,
     851,   852,   853,   854,   855,   856,   861,   866,   876,   881,
     886,   891,   892,   893,   897,   902,   907,   912,   921,   922,
     926,   927,   928,   940,   941,   945,   946,   947,   948,   949,
     956,   957,   958,   962,   963,   969,   977,   978,   981,   983,
     987,   988,   992,   993,   997,   998,  1002,  1006,  1007,  1011,
    1012,  1013,  1014,  1015,  1018,  1019,  1023,  1024,  1028,  1034,
    1044,  1052,  1056,  1063,  1064,  1071,  1081,  1087,  1089,  1093,
    1098,  1102,  1109,  1111,  1115,  1116,  1122,  1130,  1131,  1137,
    1141,  1147,  1155,  1156,  1160,  1174,  1180,  1184,  1189,  1203,
    1214,  1215,  1216,  1217,  1218,  1219,  1220,  1221,  1225,  1233,
    1245,  1249,  1256,  1257,  1261,  1265,  1266,  1270,  1274,  1281,
    1288,  1294,  1295,  1296,  1300,  1301,  1302,  1303,  1309,  1320,
    1321,  1322,  1323,  1328,  1339,  1351,  1360,  1371,  1379,  1380,
    1384,  1394,  1405,  1416,  1427,  1438,  1441,  1442,  1446,  1447,
    1448,  1449,  1453,  1454,  1458,  1463,  1465,  1469,  1478,  1482,
    1490,  1491,  1495
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
  "TOK_BAD_NUMERIC", "TOK_SUBKEY", "TOK_DOT_NUMBER", "TOK_ADD", 
  "TOK_AGENT", "TOK_ALTER", "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", 
  "TOK_BEGIN", "TOK_BETWEEN", "TOK_BIGINT", "TOK_BOOL", "TOK_BY", 
  "TOK_CALL", "TOK_CHARACTER", "TOK_COLLATION", "TOK_COLUMN", 
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", 
  "TOK_DATABASES", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_DOUBLE", "TOK_DROP", "TOK_FACET", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FOR", "TOK_FROM", 
  "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", 
  "TOK_GROUP_CONCAT", "TOK_HAVING", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTEGER", "TOK_INTO", "TOK_IS", 
  "TOK_ISOLATION", "TOK_JSON", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", 
  "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_MULTI", 
  "TOK_MULTI64", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PLUGIN", "TOK_PLUGINS", "TOK_PROFILE", 
  "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", 
  "TOK_REMAP", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", 
  "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
  "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", 
  "TOK_TABLES", "TOK_THREADS", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", 
  "TOK_TRUNCATE", "TOK_TYPE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", 
  "'}'", "'.'", "'['", "']'", "$accept", "request", "statement", 
  "ident_set", "ident", "multi_stmt_list", "multi_stmt", "select", 
  "select1", "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg", 
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "filter_item", "expr_float_unhandled", "expr_ident", 
  "const_int", "const_float", "const_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_having_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "hash_key", "hash_val", "show_stmt", "like_filter", 
  "show_what", "set_stmt", "set_global_stmt", "set_string_value", 
  "boolean_value", "set_value", "simple_set_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_ident", "column_list", "insert_rows_list", 
  "insert_row", "insert_vals_list", "insert_val", "opt_insert_options", 
  "insert_options_list", "insert_option", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "show_databases", "update", 
  "update_items_list", "update_item", "alter_col_type", "alter", 
  "show_variables", "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "flush_ramchunk", "select_sysvar", 
  "sysvar_name", "select_dual", "truncate", "optimize_index", 
  "create_plugin", "drop_plugin", "json_field", "subscript", "subkey", 
  "streq", "strval", "opt_facet_by_items_list", "facet_by", "facet_item", 
  "facet_expr", "facet_items_list", "facet_stmt", 0
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
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   124,    38,    61,   376,    60,    62,   377,   378,    43,
      45,    42,    47,    37,   379,   380,    59,    40,    41,    44,
     123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   145,   146,   146,   146,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   149,   149,
     149,   150,   150,   150,   151,   151,   152,   152,   153,   153,
     154,   154,   155,   155,   156,   156,   156,   157,   158,   159,
     159,   159,   160,   161,   161,   162,   162,   163,   163,   163,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   165,
     165,   166,   166,   167,   168,   168,   168,   169,   169,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   171,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   173,   173,   174,   174,   174,
     175,   175,   176,   176,   177,   177,   178,   178,   179,   179,
     180,   180,   181,   182,   182,   183,   183,   184,   184,   185,
     185,   185,   186,   186,   187,   187,   188,   188,   189,   190,
     190,   191,   191,   191,   191,   191,   192,   192,   193,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   195,   195,   196,   196,   197,   197,   198,   198,
     199,   199,   200,   200,   201,   202,   202,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   204,   204,
     204,   204,   204,   204,   205,   205,   205,   205,   206,   206,
     207,   207,   207,   208,   208,   209,   209,   209,   209,   209,
     210,   210,   210,   211,   211,   212,   213,   213,   214,   214,
     215,   215,   216,   216,   217,   217,   218,   219,   219,   220,
     220,   220,   220,   220,   221,   221,   222,   222,   223,   224,
     225,   226,   226,   227,   227,   228,   228,   229,   229,   230,
     230,   231,   232,   232,   233,   233,   234,   235,   235,   236,
     237,   238,   239,   239,   240,   240,   240,   240,   240,   240,
     241,   241,   241,   241,   241,   241,   241,   241,   242,   242,
     243,   243,   244,   244,   245,   246,   246,   247,   248,   249,
     250,   251,   251,   251,   252,   252,   252,   252,   253,   254,
     254,   254,   254,   255,   256,   257,   258,   259,   260,   260,
     261,   262,   263,   264,   265,   266,   267,   267,   268,   268,
     268,   268,   269,   269,   270,   271,   271,   272,   273,   274,
     275,   275,   276
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     8,     1,     9,
       0,     2,     1,     3,     1,     1,     1,     0,     3,     0,
       2,     4,    11,     1,     3,     1,     2,     0,     1,     2,
       1,     4,     4,     4,     4,     4,     4,     3,     5,     1,
       3,     0,     1,     2,     1,     3,     3,     4,     1,     3,
       3,     5,     6,     3,     4,     5,     3,     3,     3,     3,
       3,     1,     5,     5,     5,     3,     3,     3,     3,     3,
       3,     3,     4,     3,     1,     1,     4,     3,     3,     1,
       1,     4,     4,     4,     3,     1,     2,     1,     2,     1,
       1,     3,     0,     4,     0,     1,     1,     3,     0,     2,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       3,     4,     4,     4,     4,     4,     4,     4,     3,     6,
       6,     3,     8,    14,     1,     3,     1,     1,     3,     5,
       1,     1,     1,     1,     2,     0,     2,     1,     2,     2,
       3,     1,     1,     1,     2,     4,     4,     3,     4,     4,
       4,     3,     4,     4,     7,     5,     5,     9,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     7,     1,     1,     0,     3,
       1,     1,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     3,     2,     0,     2,     1,     3,     3,     4,
       6,     1,     3,     1,     3,     1,     3,     0,     2,     1,
       3,     3,     0,     1,     1,     1,     3,     1,     1,     3,
       3,     6,     1,     3,     3,     3,     5,     4,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     7,     6,
       4,     5,     0,     1,     2,     1,     3,     3,     2,     3,
       6,     0,     1,     1,     2,     2,     2,     1,     7,     1,
       1,     1,     1,     3,     6,     3,     3,     3,     1,     3,
       2,     3,     3,     7,     5,     2,     1,     2,     1,     1,
       3,     3,     3,     3,     1,     0,     2,     1,     2,     1,
       1,     3,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   303,     0,   300,     0,     0,   348,   347,
       0,     0,   306,     0,   307,   301,     0,   381,   381,     0,
       0,     0,     0,     2,     3,    81,    84,    86,    88,    85,
       7,     8,     9,   302,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,    29,    30,     0,
       0,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    77,    46,    47,    48,
      49,    50,    79,    51,    52,    53,    54,    56,    55,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    80,    70,    71,    72,    73,    74,    75,    76,
      78,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      31,   210,   212,   213,   414,   215,   398,   214,    34,     0,
      38,     0,     0,    43,    44,   211,     0,     0,    49,     0,
       0,    67,    75,     0,   105,     0,     0,     0,   209,     0,
     103,   107,   110,   237,   192,   238,   239,     0,     0,     0,
      41,     0,     0,    63,     0,     0,     0,     0,   378,   265,
     382,     0,   265,   272,   273,   271,   383,   265,   265,   196,
     267,   264,     0,   304,     0,   119,     0,     1,     0,     4,
      83,     0,   265,     0,     0,     0,     0,     0,     0,   393,
       0,   396,   395,   402,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      31,    49,     0,   216,   217,     0,   261,   260,     0,     0,
     408,   409,     0,   405,   406,     0,     0,     0,   108,   106,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   397,   193,
       0,     0,     0,     0,     0,     0,     0,   298,   299,   297,
     296,   295,   281,   293,     0,     0,     0,   265,     0,   379,
       0,   350,     0,   269,   268,   349,     0,   274,   197,   372,
     401,     0,     0,   419,   420,   107,   415,     0,     0,    82,
     308,   346,     0,     0,     0,   165,   167,   321,   169,     0,
       0,   319,   320,   333,   337,   331,     0,     0,     0,   329,
       0,   257,     0,   248,   256,     0,   254,   399,     0,   256,
       0,     0,     0,     0,     0,   117,     0,     0,     0,     0,
       0,     0,     0,   251,     0,     0,     0,   235,     0,   236,
       0,   414,     0,   407,    97,   121,   104,   110,   109,   227,
     228,   234,   233,   225,   224,   231,   412,   232,   222,   223,
     230,   229,   218,   219,   220,   221,   226,   194,   240,     0,
     413,   282,   283,     0,     0,     0,     0,   289,   291,   280,
     290,     0,   288,   292,   279,   278,     0,   265,   270,   265,
     266,   277,     0,   198,   199,     0,     0,   370,   373,     0,
       0,   352,     0,   120,   418,   417,     0,   183,     0,     0,
       0,     0,     0,     0,   166,   168,   335,   323,   170,     0,
       0,     0,     0,   390,   391,   389,   392,     0,     0,   155,
       0,    38,     0,     0,    43,   159,     0,    48,    75,     0,
     154,   123,   124,   128,   141,     0,   160,   404,     0,     0,
       0,   242,     0,   111,   245,     0,   116,   247,   246,   115,
     243,   244,   112,     0,   113,     0,     0,   114,     0,     0,
       0,   263,   262,   258,   411,   410,     0,   172,   122,     0,
     241,     0,   286,   285,     0,   294,     0,   275,   276,     0,
       0,   371,     0,   374,   375,     0,     0,   196,     0,   421,
       0,   422,   184,   416,   311,   310,   312,     0,     0,     0,
     369,   394,   322,     0,   334,     0,   342,   332,   338,   339,
     330,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    90,     0,   255,   118,     0,     0,     0,
       0,     0,     0,   174,   180,   195,     0,     0,     0,     0,
     387,   380,   202,   205,     0,   201,   200,     0,     0,     0,
     354,   355,   353,   351,   358,   359,     0,   309,     0,     0,
     324,   314,   361,   363,   362,   360,   366,   364,   365,   367,
     368,   171,   336,   343,     0,     0,   388,   403,     0,     0,
       0,     0,   164,   157,     0,     0,   158,   126,   125,     0,
       0,   133,     0,   151,     0,   149,   129,   140,   150,   130,
     153,   137,   146,   136,   145,   138,   147,   139,   148,     0,
       0,     0,     0,   250,   249,     0,   259,     0,     0,   175,
       0,     0,   178,   181,   284,     0,   385,   384,   386,     0,
       0,   206,     0,   377,   376,   357,     0,    56,   189,   185,
     187,   313,     0,     0,   317,     0,     0,   305,   345,   344,
     341,   342,   340,   163,   156,   162,   161,   127,     0,     0,
       0,   152,   134,     0,    95,    96,    94,    91,    92,    87,
       0,     0,     0,     0,    99,     0,     0,     0,   183,     0,
       0,   203,     0,     0,   356,     0,   190,   191,     0,   316,
       0,     0,   325,   326,   315,   135,   143,   144,   142,   131,
       0,     0,   252,     0,     0,     0,    89,   176,   173,     0,
     179,   192,   287,   208,   207,   204,   186,   188,   318,     0,
       0,   132,    93,     0,    98,   100,     0,     0,   196,   328,
     327,     0,     0,   177,   182,   102,     0,   101,     0,     0,
     253
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   110,   148,    24,    25,    26,    27,   641,
     697,   698,   486,   704,   736,    28,   149,   150,   239,   151,
     355,   487,   319,   451,   452,   453,   454,   455,   428,   312,
     429,   564,   650,   738,   708,   652,   653,   511,   512,   669,
     670,   258,   259,   287,   288,   403,   404,   660,   661,   329,
     153,   325,   326,   228,   229,   483,    29,   281,   181,    30,
      31,   394,   395,   272,   273,    32,    33,    34,    35,   420,
     516,   517,   590,   591,   673,   313,   677,   722,   723,    36,
      37,   314,   315,   430,   432,   528,   529,   604,   680,    38,
      39,    40,    41,    42,   410,   411,   600,    43,    44,   407,
     408,   503,   504,    45,    46,    47,   165,   571,    48,   437,
      49,    50,    51,    52,    53,   154,    54,    55,    56,    57,
      58,   155,   233,   234,   156,   157,   417,   418,   294,   295,
     296,   190
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -636
static const short yypact[] =
{
    4224,   -46,    30,  -636,  3667,  -636,    19,    29,  -636,  -636,
      36,    -2,  -636,    37,  -636,  -636,   715,  2830,  3954,   -16,
      38,  3667,   103,  -636,   -20,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,    71,  -636,  -636,  -636,  3667,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  3667,
    3667,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,   -17,  3667,  3667,  3667,  3667,  3667,  3667,  3667,  3667,
      55,  -636,  -636,  -636,  -636,  -636,    82,  -636,    90,    98,
     124,   139,   141,   144,   146,  -636,   152,   155,   166,   169,
     171,   182,   183,  1543,  -636,  1543,  1543,  3150,    13,   -12,
    -636,  3253,    88,  -636,    99,   260,  -636,   201,   207,    75,
    3770,  3667,  2727,   224,   213,   230,  3358,   243,  -636,   274,
    -636,  3667,   274,  -636,  -636,  -636,  -636,   274,   274,   264,
    -636,  -636,   228,  -636,  3667,  -636,   -41,  -636,  1543,   -15,
    -636,  3667,   274,    64,   237,    24,   255,   238,   -21,  -636,
     240,  -636,  -636,  -636,   853,  3667,  1543,  1681,    42,  1681,
    1681,   209,  1543,  1681,  1681,  1543,  1543,  1543,  1543,   214,
     217,   218,   219,  -636,  -636,  3951,  -636,  -636,    95,   251,
    -636,  -636,  1819,    13,  -636,  2158,   991,  3667,  -636,  -636,
    1543,  1543,  1543,  1543,  1543,  1543,  1543,  1543,  1543,  1543,
    1543,  1543,  1543,  1543,  1543,  1543,  1543,   346,  -636,  -636,
     -30,  1543,  2727,  2727,   252,   253,   327,  -636,  -636,  -636,
    -636,  -636,   249,  -636,  2272,   318,   285,    -5,   303,  -636,
     396,  -636,   306,  -636,  -636,  -636,  3667,  -636,  -636,    23,
    -636,  3667,  3667,  4241,  -636,  3253,    -8,  1129,   389,  -636,
     270,  -636,   378,   379,   319,  -636,  -636,  -636,  -636,   189,
       7,  -636,  -636,  -636,   271,  -636,    62,   404,  1941,  -636,
     405,   293,  1267,  -636,  4225,    59,  -636,  -636,  3972,  4241,
     106,  3667,   282,   108,   111,  -636,  3992,   116,   118,   486,
     523,   561,  4013,  -636,  1405,  1543,  1543,  -636,  3150,  -636,
    2388,   278,   392,  -636,  -636,   -21,  -636,  4241,  -636,  -636,
    -636,   267,   418,  3270,  3869,   710,  -636,   710,    53,    53,
      53,    53,    57,    57,  -636,  -636,  -636,   284,  -636,   354,
     710,   249,   249,   294,  2944,   424,  2727,  -636,  -636,  -636,
    -636,   429,  -636,  -636,  -636,  -636,   370,   274,  -636,   274,
    -636,  -636,   313,   298,  -636,   432,  3667,  -636,  -636,    56,
      33,  -636,   320,  -636,  -636,  -636,  1543,   363,  1543,  3461,
     329,  3667,  3667,  3667,  -636,  -636,  -636,  -636,  -636,   126,
     128,    24,   310,  -636,  -636,  -636,  -636,   352,   353,  -636,
     307,   315,   316,   317,   328,  -636,   331,   333,   336,  1941,
      13,   335,  -636,  -636,  -636,   170,  -636,  -636,   991,   337,
    3667,  -636,  1681,  -636,  -636,   339,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  1543,  -636,  1543,  1543,  -636,  3771,  3801,
     334,  -636,  -636,  -636,  -636,  -636,   373,   423,  -636,   473,
    -636,    11,  -636,  -636,   356,  -636,   143,  -636,  -636,  2055,
    3667,  -636,   357,   362,  -636,    39,  3667,   264,    70,  -636,
     456,  -636,  -636,   345,  -636,  -636,  -636,   130,   348,   178,
    -636,  -636,  -636,    11,  -636,   478,    10,  -636,   349,  -636,
    -636,   479,   483,  3667,   361,  3667,   355,   365,  3667,   492,
     367,   102,  1941,    70,    16,    47,    44,    66,    70,    70,
      70,    70,   445,   368,   450,  -636,  -636,  4086,  4107,  3836,
    2388,   991,   371,   503,   409,  -636,   133,   391,   -10,   444,
    -636,  -636,  -636,  -636,  3667,   395,  -636,   502,  3667,     6,
    -636,  -636,  -636,  -636,  -636,  -636,  2504,  -636,  3461,    34,
     -34,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  3564,    34,  -636,  -636,    13,   399,
     414,   415,  -636,  -636,   416,   417,  -636,  -636,  -636,   437,
     439,  -636,    11,  -636,   484,  -636,  -636,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,    28,
    3047,   426,  3667,  -636,  -636,   428,  -636,     3,   490,  -636,
     535,   518,   517,  -636,  -636,    11,  -636,  -636,  -636,   449,
     135,  -636,   565,  -636,  -636,  -636,   147,   440,   206,   441,
    -636,  -636,     8,   161,  -636,   573,   348,  -636,  -636,  -636,
    -636,   560,  -636,  -636,  -636,  -636,  -636,  -636,    70,    70,
     163,  -636,  -636,    11,  -636,  -636,  -636,   442,  -636,  -636,
     447,  1681,  3667,   553,   516,  2613,   508,  2613,   363,   172,
      11,  -636,  3667,   451,  -636,   453,  -636,  -636,  2613,  -636,
      34,   464,   455,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
     174,  3047,  -636,   177,  2613,   591,  -636,  -636,   458,   576,
    -636,    99,  -636,  -636,  -636,  -636,  -636,  -636,  -636,   595,
     573,  -636,  -636,   460,   441,   465,  2613,  2613,   264,  -636,
    -636,   485,   615,  -636,   441,  -636,  1681,  -636,   179,   488,
    -636
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -636,  -636,  -636,    -9,    -4,  -636,   434,  -636,   299,  -636,
    -636,  -104,  -636,  -636,  -636,   142,    68,   394,   340,  -636,
      20,  -636,  -308,  -418,  -636,   -76,  -636,  -579,  -122,  -484,
    -487,  -636,  -636,  -636,  -636,  -636,  -636,   -74,  -636,  -635,
     -86,  -102,  -636,  -505,  -636,  -636,   137,  -636,   -72,   117,
    -636,  -204,   195,  -636,   311,    81,  -636,  -158,  -636,  -636,
    -636,   276,  -636,    63,   272,  -636,  -636,  -636,  -636,  -636,
      76,  -636,  -636,   -13,  -636,  -430,  -636,  -636,   -85,  -636,
    -636,  -636,   235,  -636,  -636,  -636,    65,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,   162,  -636,  -636,  -636,  -636,
    -636,  -636,    89,  -636,  -636,  -636,   651,  -636,  -636,  -636,
    -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,  -636,
    -636,  -258,  -636,   438,  -636,   427,  -636,  -636,   256,  -636,
     257,  -636
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -415
static const short yytable[] =
{
     111,   526,   583,   330,   566,   333,   334,   668,   164,   337,
     338,   305,   305,   305,   283,   426,   305,   185,   415,   284,
     285,   581,   656,   188,   585,   621,   230,   231,   603,   305,
     306,   541,   307,   412,   301,   192,   235,   692,   308,   305,
     306,   186,   307,   675,   305,   306,   378,   488,   308,   305,
     306,   702,   625,   308,   291,   193,   194,    59,   308,   620,
     456,   280,   627,   630,   632,   634,   636,   638,   112,   230,
     231,   305,   306,   311,   628,   305,   306,   114,   297,   302,
     308,   331,   298,   117,   308,   115,   433,    60,  -400,   405,
     118,   183,   666,   240,   119,   -65,   318,   240,   292,   754,
     113,   657,   507,   187,   379,   676,   303,   434,   196,   197,
     185,   199,   200,   201,   202,   203,   189,   116,   292,   398,
     195,   435,   764,   623,   618,   241,   737,   236,   240,   241,
     184,   416,   191,   152,   198,   690,   391,   391,   391,   668,
     406,   391,   236,   227,   665,   427,   427,   238,  -333,  -333,
     318,   265,   393,   622,   309,   668,   232,   266,   271,   674,
     241,   310,   278,   436,   309,   693,   257,   282,   709,   309,
     263,   672,   506,   332,   309,   681,   579,   763,   668,   505,
     290,   624,   252,   253,   254,   255,   256,   300,   254,   255,
     256,   456,   204,   543,   424,   425,   309,   461,   462,   232,
     309,   327,   592,   593,   726,   728,   730,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   542,   594,   205,   716,   544,   206,   482,   568,
     569,   185,   545,   358,   348,   207,   349,   570,   595,   497,
     617,   498,   596,   717,   464,   462,   467,   462,   412,   468,
     462,   597,   598,   765,   470,   462,   471,   462,   271,   271,
     223,   208,   224,   225,   522,   523,   524,   525,   587,   588,
     392,   654,   523,   711,   712,   609,   209,   611,   210,   599,
     614,   211,   402,   212,   456,   714,   523,   409,   413,   213,
     748,   238,   214,   546,   547,   548,   549,   550,   551,   719,
     720,   729,   523,   215,   552,   293,   216,   240,   217,   311,
     742,   523,   751,   523,   450,   753,   462,   769,   462,   218,
     219,   324,   260,   328,   261,   381,   382,   465,   456,   336,
     262,  -383,   339,   340,   341,   342,   274,   275,   279,   241,
     280,   286,   289,   304,   227,   316,   481,   335,   317,   352,
     320,   377,   343,   357,   344,   345,   346,   359,   360,   361,
     362,   363,   364,   365,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   350,   383,   384,   385,   380,   386,
     392,   396,   271,   580,   700,   397,   584,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   601,   502,   399,   400,   166,   401,   419,   421,   422,
     431,   423,   438,   457,   357,   515,  -414,   519,   520,   521,
     466,   619,   484,   489,   626,   629,   631,   633,   635,   637,
     490,   491,   240,   494,   424,   496,   499,   500,   482,   225,
     501,   510,   518,   508,   533,   450,   171,   456,   530,   456,
     531,   532,   534,   535,   536,   542,   554,   560,   240,   172,
     456,   324,   478,   479,   241,   537,   561,   311,   538,   173,
     539,   174,   175,   540,   563,   553,   456,   556,   565,   567,
     577,   578,   586,   311,   416,   589,   602,   606,   605,   177,
     241,   607,   610,   612,   179,   575,   402,   733,   456,   456,
     615,   639,   409,   613,   180,   616,   642,   640,   649,   648,
     663,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   240,   651,   655,   608,
     658,   608,   662,   293,   608,   293,   485,   683,   450,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   684,   685,   686,   687,   481,   688,   241,   689,
     691,   705,   768,   240,   699,   701,   725,   727,   703,   706,
     659,   707,   710,   713,   502,   357,   721,   715,   603,   734,
     718,   731,   450,   735,   515,   732,   739,   749,   743,   745,
     557,   746,   558,   559,   750,   241,   755,   756,   311,   761,
     679,   240,   757,   759,   762,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     767,   459,   766,   299,   472,   473,   770,   752,   562,   647,
     356,   740,   747,   241,   741,   414,   696,   576,   608,   758,
     744,   646,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   555,   495,   480,
     493,   474,   475,   724,   671,   760,   527,   664,   582,   182,
     682,   353,   509,   366,     0,   513,     0,     0,   357,     0,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,     0,     0,     0,   185,     0,
     476,   450,     0,   450,     0,     0,     0,     0,   659,     0,
       0,     0,     0,     0,   450,     0,     0,     0,   120,   121,
     122,   123,     0,   124,   125,   126,     0,   696,     0,   127,
     450,    62,     0,     0,     0,    63,   128,    65,     0,   129,
      66,     0,     0,     0,    67,     0,     0,     0,   130,     0,
     240,     0,   450,   450,     0,     0,   131,     0,     0,     0,
     132,    69,     0,     0,    70,    71,    72,   133,   134,     0,
     135,   136,     0,     0,     0,   137,     0,     0,    75,    76,
      77,    78,   241,    79,   138,    81,   139,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,   140,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,   141,     0,   100,
     101,     0,   102,     0,   103,   104,   105,     0,     0,   106,
     107,   142,     0,   109,     0,   248,   249,   250,   251,   252,
     253,   254,   255,   256,     0,   143,   144,     0,     0,   145,
       0,     0,   146,     0,     0,   147,   220,   121,   122,   123,
       0,   321,   125,     0,     0,     0,     0,   127,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   129,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,   131,     0,     0,     0,   132,    69,
       0,     0,    70,    71,    72,    73,    74,     0,   135,   136,
       0,     0,     0,   137,     0,     0,    75,    76,    77,    78,
       0,    79,   221,    81,   222,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,   140,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,     0,
     102,     0,   103,   104,   105,     0,     0,   106,   107,   142,
       0,   109,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,     0,     0,     0,   145,     0,     0,
     322,   323,     0,   147,   220,   121,   122,   123,     0,   124,
     125,     0,     0,     0,     0,   127,     0,    62,     0,     0,
       0,    63,   128,    65,     0,   129,    66,     0,     0,     0,
      67,     0,     0,     0,   130,     0,     0,     0,     0,     0,
       0,     0,   131,     0,     0,     0,   132,    69,     0,     0,
      70,    71,    72,   133,   134,     0,   135,   136,     0,     0,
       0,   137,     0,     0,    75,    76,    77,    78,     0,    79,
     138,    81,   139,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
     140,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,   141,     0,   100,   101,     0,   102,     0,
     103,   104,   105,     0,     0,   106,   107,   142,     0,   109,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,     0,     0,   145,     0,     0,   146,     0,
       0,   147,   120,   121,   122,   123,     0,   124,   125,     0,
       0,     0,     0,   127,     0,    62,     0,     0,     0,    63,
     128,    65,     0,   129,    66,     0,     0,     0,    67,     0,
       0,     0,   130,     0,     0,     0,     0,     0,     0,     0,
     131,     0,     0,     0,   132,    69,     0,     0,    70,    71,
      72,   133,   134,     0,   135,   136,     0,     0,     0,   137,
       0,     0,    75,    76,    77,    78,     0,    79,   138,    81,
     139,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,   140,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,   141,     0,   100,   101,     0,   102,     0,   103,   104,
     105,     0,     0,   106,   107,   142,     0,   109,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   143,
     144,     0,     0,   145,     0,     0,   146,     0,     0,   147,
     220,   121,   122,   123,     0,   124,   125,     0,     0,     0,
       0,   127,     0,    62,     0,     0,     0,    63,    64,    65,
       0,   129,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,   131,     0,
       0,     0,   132,    69,     0,     0,    70,    71,    72,    73,
      74,     0,   135,   136,     0,     0,     0,   137,     0,     0,
      75,    76,    77,    78,     0,    79,   221,    81,   222,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,   140,    91,    92,    93,
     458,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,   101,     0,   102,     0,   103,   104,   105,     0,
       0,   106,   107,   142,     0,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,     0,     0,
       0,   145,     0,     0,   146,     0,     0,   147,   220,   121,
     122,   123,     0,   321,   125,     0,     0,     0,     0,   127,
       0,    62,     0,     0,     0,    63,    64,    65,     0,   129,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,   131,     0,     0,     0,
     132,    69,     0,     0,    70,    71,    72,    73,    74,     0,
     135,   136,     0,     0,     0,   137,     0,     0,    75,    76,
      77,    78,     0,    79,   221,    81,   222,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,   140,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
     101,     0,   102,     0,   103,   104,   105,     0,     0,   106,
     107,   142,     0,   109,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   143,     0,     0,     0,   145,
       0,     0,   146,   323,     0,   147,   220,   121,   122,   123,
       0,   124,   125,     0,     0,     0,     0,   127,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   129,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,   131,     0,     0,     0,   132,    69,
       0,     0,    70,    71,    72,    73,    74,     0,   135,   136,
       0,     0,     0,   137,     0,     0,    75,    76,    77,    78,
       0,    79,   221,    81,   222,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,   140,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,     0,
     102,     0,   103,   104,   105,     0,     0,   106,   107,   142,
       0,   109,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,     0,     0,     0,   145,     0,     0,
     146,     0,     0,   147,   220,   121,   122,   123,     0,   321,
     125,     0,     0,     0,     0,   127,     0,    62,     0,     0,
       0,    63,    64,    65,     0,   129,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,   131,     0,     0,     0,   132,    69,     0,     0,
      70,    71,    72,    73,    74,     0,   135,   136,     0,     0,
       0,   137,     0,     0,    75,    76,    77,    78,     0,    79,
     221,    81,   222,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
     140,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,   101,     0,   102,     0,
     103,   104,   105,     0,     0,   106,   107,   142,     0,   109,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,     0,     0,     0,   145,     0,     0,   146,     0,
       0,   147,   220,   121,   122,   123,     0,   351,   125,     0,
       0,     0,     0,   127,     0,    62,     0,     0,     0,    63,
      64,    65,     0,   129,    66,     0,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
     131,     0,     0,     0,   132,    69,     0,     0,    70,    71,
      72,    73,    74,     0,   135,   136,     0,     0,     0,   137,
       0,     0,    75,    76,    77,    78,     0,    79,   221,    81,
     222,     0,     0,     0,    82,     0,     0,     0,     0,    83,
      84,    85,    86,    87,    88,    89,    90,     0,   140,    91,
      92,    93,     0,    94,     0,    95,     0,     0,    96,    97,
      98,    99,     0,   100,   101,     0,   102,     0,   103,   104,
     105,     0,     0,   106,   107,   142,     0,   109,     0,     0,
       0,     0,     0,     0,    61,   439,     0,     0,     0,   143,
       0,     0,     0,   145,     0,     0,   146,    62,     0,   147,
       0,    63,    64,    65,     0,   440,    66,     0,     0,     0,
      67,     0,     0,     0,   441,     0,     0,     0,     0,     0,
       0,     0,   442,     0,   443,     0,     0,    69,     0,     0,
      70,    71,    72,   444,    74,     0,   445,     0,     0,     0,
       0,   446,     0,     0,    75,    76,    77,    78,     0,   447,
      80,    81,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,   101,     0,   102,     0,
     103,   104,   105,     0,     0,   106,   107,   448,    61,   109,
     572,     0,     0,   573,     0,     0,     0,     0,     0,     0,
       0,    62,     0,     0,     0,    63,    64,    65,   449,     0,
      66,     0,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,     0,     0,
      82,     0,     0,     0,     0,    83,    84,    85,    86,    87,
      88,    89,    90,     0,     0,    91,    92,    93,     0,    94,
       0,    95,     0,     0,    96,    97,    98,    99,     0,   100,
     101,    61,   102,     0,   103,   104,   105,     0,     0,   106,
     107,   108,     0,   109,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,   574,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,    97,    98,
      99,     0,   100,   101,     0,   102,     0,   103,   104,   105,
       0,     0,   106,   107,   108,    61,   109,   305,     0,     0,
     387,     0,     0,     0,     0,     0,     0,     0,    62,     0,
       0,     0,    63,    64,    65,   354,     0,    66,     0,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   388,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,     0,     0,    82,   389,     0,
       0,     0,    83,    84,    85,    86,    87,    88,    89,    90,
       0,     0,    91,    92,    93,     0,    94,     0,    95,     0,
       0,    96,    97,    98,    99,     0,   100,   101,     0,   102,
     390,   103,   104,   105,     0,     0,   106,   107,   108,     0,
     109,    61,     0,   305,     0,     0,     0,     0,     0,     0,
       0,     0,   391,     0,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,    97,    98,
      99,     0,   100,   101,     0,   102,     0,   103,   104,   105,
       0,     0,   106,   107,   108,     0,   109,    61,   439,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   391,     0,
      62,     0,     0,     0,    63,    64,    65,     0,   440,    66,
       0,     0,     0,    67,     0,     0,     0,   441,     0,     0,
       0,     0,     0,     0,     0,   442,     0,   443,     0,     0,
      69,     0,     0,    70,    71,    72,   444,    74,     0,   445,
       0,     0,     0,     0,   446,     0,     0,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,   667,    88,
      89,    90,     0,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,    99,     0,   100,   101,
       0,   102,     0,   103,   104,   105,    61,   439,   106,   107,
     448,     0,   109,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,    63,    64,    65,     0,   440,    66,     0,
       0,     0,    67,     0,     0,     0,   441,     0,     0,     0,
       0,     0,     0,     0,   442,     0,   443,     0,     0,    69,
       0,     0,    70,    71,    72,   444,    74,     0,   445,     0,
       0,     0,     0,   446,     0,     0,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,     0,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,     0,
     102,     0,   103,   104,   105,     0,     0,   106,   107,   448,
      61,   109,   267,   268,     0,   269,     0,     0,     0,     0,
       0,     0,     0,    62,     0,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,   270,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,   101,    61,   102,     0,   103,   104,   105,     0,
     158,   106,   107,   108,     0,   109,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,   159,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
     160,    72,    73,    74,     0,     0,     0,   161,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,   162,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,     0,
      91,    92,    93,     0,    94,     0,   163,     0,     0,    96,
      97,    98,    99,     0,   100,   101,     0,     0,     0,   103,
     104,   105,     0,     0,   106,   107,   108,    61,   109,   492,
       0,     0,   387,     0,     0,     0,     0,     0,     0,     0,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,    99,     0,   100,   101,
      61,   102,   694,   103,   104,   105,     0,     0,   106,   107,
     108,     0,   109,    62,     0,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,   695,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,   101,    61,   102,     0,   103,   104,   105,     0,
       0,   106,   107,   108,     0,   109,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,   226,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,     0,     0,    82,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,     0,
      91,    92,    93,     0,    94,     0,    95,     0,     0,    96,
      97,    98,    99,     0,   100,   101,    61,   102,     0,   103,
     104,   105,     0,     0,   106,   107,   108,     0,   109,    62,
       0,   237,     0,    63,    64,    65,     0,     0,    66,     0,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,     0,     0,
     240,     0,     0,     0,     0,     0,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,    82,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,     0,   241,    91,    92,    93,     0,    94,     0,    95,
       0,     0,    96,    97,    98,    99,     0,   100,   101,     0,
     102,    61,   103,   104,   105,     0,   276,   106,   107,   108,
       0,   109,     0,     0,    62,     0,     0,     0,    63,    64,
      65,     0,     0,    66,     0,     0,     0,    67,     0,     0,
       0,    68,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,    69,     0,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    83,    84,
      85,    86,    87,    88,    89,    90,     0,     0,    91,    92,
      93,     0,    94,     0,    95,     0,     0,    96,   277,    98,
      99,     0,   100,   101,    61,   102,     0,   103,   104,   105,
       0,     0,   106,   107,   108,     0,   109,    62,     0,     0,
       0,    63,    64,    65,     0,     0,    66,     0,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,   514,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    83,    84,    85,    86,    87,    88,    89,    90,     0,
       0,    91,    92,    93,     0,    94,     0,    95,     0,     0,
      96,    97,    98,    99,     0,   100,   101,    61,   102,     0,
     103,   104,   105,     0,     0,   106,   107,   108,     0,   109,
      62,     0,     0,     0,    63,    64,    65,     0,     0,    66,
       0,     0,     0,    67,     0,     0,     0,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,   678,    79,    80,    81,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,     0,     0,    91,    92,    93,     0,    94,     0,
      95,     0,     0,    96,    97,    98,    99,     0,   100,   101,
      61,   102,     0,   103,   104,   105,     0,     0,   106,   107,
     108,     0,   109,    62,     0,     0,     0,    63,    64,    65,
       0,     0,    66,     0,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    83,    84,    85,
      86,    87,    88,    89,    90,     0,     0,    91,    92,    93,
       0,    94,     0,    95,     0,     0,    96,    97,    98,    99,
       0,   100,   101,    61,   102,     0,   103,   104,   105,   264,
       0,   106,   107,   108,     0,   109,    62,     0,     0,     0,
      63,    64,    65,     0,     0,    66,     0,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
       0,   240,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,     0,    79,    80,
      81,   240,     0,   241,     0,     0,     0,     0,     0,     0,
      83,    84,    85,    86,    87,    88,    89,    90,     0,     0,
      91,    92,    93,     0,    94,     0,    95,     0,     0,    96,
      97,    98,    99,   241,   100,   101,   240,  -382,     0,   103,
     104,   105,     0,     0,   106,   107,   108,     0,   109,     0,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,     0,     0,     0,   241,   240,
     473,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,     0,     0,     0,     0,     0,
     475,   241,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     166,     0,     0,     0,     0,   645,     0,     0,     0,     0,
       0,     0,   167,   168,     0,     0,     0,     0,     0,   169,
       0,   240,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,     0,   170,     0,     0,     0,     0,     0,
       0,   171,   240,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   241,   172,     0,     0,     0,     0,     0,
       0,     0,   240,     0,   173,     0,   174,   175,     0,     0,
       0,     0,     0,     0,   241,     0,     0,     0,     0,     0,
     176,     0,     0,   240,   177,     0,     0,     0,   178,   179,
       0,     0,     0,     0,   241,     0,     0,     0,     0,   180,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   241,     0,     0,     0,   347,
       0,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,     0,     0,     0,     0,
     463,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   240,     0,     0,     0,
     469,     0,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   240,     0,     0,
       0,   477,     0,     0,     0,     0,     0,     0,   241,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   241,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
       0,     0,     0,     0,   643,     0,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,     1,     0,     0,     2,   644,     3,     0,     0,     0,
       0,     4,     0,     0,     0,     5,     0,     0,     6,     0,
       7,     8,     9,     0,     0,   240,    10,     0,     0,     0,
      11,     0,   460,     0,     0,     0,     0,     0,     0,     0,
       0,   240,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   241,     0,     0,
       0,     0,     0,    13,     0,     0,     0,     0,     0,     0,
       0,     0,    14,   241,     0,    15,     0,    16,     0,    17,
       0,    18,     0,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    20,     0,     0,    21,     0,     0,     0,
       0,     0,     0,     0,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,     0,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256
};

static const short yycheck[] =
{
       4,   431,   507,   207,   491,   209,   210,   586,    17,   213,
     214,     5,     5,     5,   172,     8,     5,    21,    26,   177,
     178,   505,    32,    43,   508,     9,    13,    14,    18,     5,
       6,   449,     8,   291,   192,    39,    48,     9,    14,     5,
       6,    21,     8,    77,     5,     6,    76,   355,    14,     5,
       6,    48,     8,    14,    95,    59,    60,   103,    14,   543,
     318,    66,   546,   547,   548,   549,   550,   551,    49,    13,
      14,     5,     6,   195,     8,     5,     6,    48,    93,    15,
      14,    39,    97,    85,    14,    49,    24,    57,     0,    66,
      92,   107,   579,    40,    57,   100,   117,    40,   139,   734,
      81,   111,   410,     0,   134,   139,    42,    45,   112,   113,
     114,   115,   116,   117,   118,   119,   136,    81,   139,   277,
     137,    59,   757,    76,   542,    72,   705,   139,    40,    72,
      92,   139,    61,    16,   114,   622,   130,   130,   130,   718,
     117,   130,   139,   147,   138,   138,   138,   151,   138,   139,
     117,   160,   274,   137,   130,   734,   143,   161,   162,   589,
      72,   137,   166,   101,   130,   137,    67,   171,   655,   130,
      95,   137,   139,   131,   130,   605,   137,   756,   757,   123,
     184,   134,   129,   130,   131,   132,   133,   191,   131,   132,
     133,   449,   137,    23,     5,     6,   130,   138,   139,   143,
     130,   205,    24,    25,   688,   689,   693,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   120,    45,   142,    19,    56,   137,   350,    86,
      87,   235,    62,   237,   139,   137,   141,    94,    60,   397,
     138,   399,    64,    37,   138,   139,   138,   139,   506,   138,
     139,    73,    74,   758,   138,   139,   138,   139,   262,   263,
     143,   137,   145,   146,   138,   139,   138,   139,   138,   139,
     274,   138,   139,   138,   139,   533,   137,   535,   137,   101,
     538,   137,   286,   137,   542,   138,   139,   291,   292,   137,
     720,   295,   137,   123,   124,   125,   126,   127,   128,   138,
     139,   138,   139,   137,   134,   188,   137,    40,   137,   431,
     138,   139,   138,   139,   318,   138,   139,   138,   139,   137,
     137,   204,    62,   206,   123,   262,   263,   331,   586,   212,
     123,   107,   215,   216,   217,   218,   123,   107,    95,    72,
      66,    77,   114,   106,   348,    90,   350,   138,   110,   232,
     110,     5,   138,   236,   137,   137,   137,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   123,   123,   123,    50,   261,   130,
     384,    63,   386,   505,   642,   100,   508,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   523,   406,   100,     8,    16,   100,   137,    30,    30,
     139,    92,     8,     8,   297,   419,   123,   421,   422,   423,
     138,   543,   144,   139,   546,   547,   548,   549,   550,   551,
      76,   137,    40,     9,     5,    65,   123,   139,   560,   322,
       8,    78,   113,   123,   137,   449,    57,   705,   138,   707,
      98,    98,   137,   137,   137,   120,   460,   123,    40,    70,
     718,   344,   345,   346,    72,   137,    93,   589,   137,    80,
     137,    82,    83,   137,    51,   138,   734,   138,     5,   123,
     123,   119,    26,   605,   139,   137,     8,     8,   139,   100,
      72,     8,   131,   138,   105,   499,   500,   701,   756,   757,
       8,    56,   506,   138,   115,   138,    56,   139,     5,   138,
       8,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,    40,   118,   137,   533,
      86,   535,   137,   416,   538,   418,   144,   138,   542,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   138,   138,   138,   138,   560,   120,    72,   120,
      76,    26,   766,    40,   138,   137,   688,   689,    78,    51,
     574,    54,   123,     8,   578,   458,     3,   137,    18,    26,
     139,   139,   586,    67,   588,   138,    78,   123,   710,   138,
     473,   138,   475,   476,   139,    72,     5,   139,   720,   139,
     604,    40,    26,     8,   139,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       5,   322,   137,   189,   138,   139,   138,   731,   486,   561,
     236,   707,   718,    72,   708,   295,   640,   500,   642,   741,
     712,   560,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   462,   386,   348,
     384,   138,   139,   676,   588,   750,   431,   578,   506,    18,
     605,   233,   416,   246,    -1,   418,    -1,    -1,   561,    -1,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,   702,    -1,
     139,   705,    -1,   707,    -1,    -1,    -1,    -1,   712,    -1,
      -1,    -1,    -1,    -1,   718,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    10,    -1,   731,    -1,    14,
     734,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      40,    -1,   756,   757,    -1,    -1,    41,    -1,    -1,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    52,    53,    -1,
      55,    56,    -1,    -1,    -1,    60,    -1,    -1,    63,    64,
      65,    66,    72,    68,    69,    70,    71,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    87,    -1,    89,    90,    91,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,   100,   101,   102,    -1,   104,
     105,    -1,   107,    -1,   109,   110,   111,    -1,    -1,   114,
     115,   116,    -1,   118,    -1,   125,   126,   127,   128,   129,
     130,   131,   132,   133,    -1,   130,   131,    -1,    -1,   134,
      -1,    -1,   137,    -1,    -1,   140,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    55,    56,
      -1,    -1,    -1,    60,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      87,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    -1,    99,   100,   101,   102,    -1,   104,   105,    -1,
     107,    -1,   109,   110,   111,    -1,    -1,   114,   115,   116,
      -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    -1,    -1,   134,    -1,    -1,
     137,   138,    -1,   140,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    55,    56,    -1,    -1,
      -1,    60,    -1,    -1,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      89,    90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,   100,   101,   102,    -1,   104,   105,    -1,   107,    -1,
     109,   110,   111,    -1,    -1,   114,   115,   116,    -1,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,   131,    -1,    -1,   134,    -1,    -1,   137,    -1,
      -1,   140,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    52,    53,    -1,    55,    56,    -1,    -1,    -1,    60,
      -1,    -1,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    85,    86,    87,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,
     101,   102,    -1,   104,   105,    -1,   107,    -1,   109,   110,
     111,    -1,    -1,   114,   115,   116,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    -1,   134,    -1,    -1,   137,    -1,    -1,   140,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    -1,    45,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    55,    56,    -1,    -1,    -1,    60,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    87,    -1,    89,    90,    91,    92,
      93,    94,    -1,    96,    -1,    -1,    99,   100,   101,   102,
      -1,   104,   105,    -1,   107,    -1,   109,   110,   111,    -1,
      -1,   114,   115,   116,    -1,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,
      -1,   134,    -1,    -1,   137,    -1,    -1,   140,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    52,    53,    -1,
      55,    56,    -1,    -1,    -1,    60,    -1,    -1,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    87,    -1,    89,    90,    91,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,   100,   101,   102,    -1,   104,
     105,    -1,   107,    -1,   109,   110,   111,    -1,    -1,   114,
     115,   116,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,   134,
      -1,    -1,   137,   138,    -1,   140,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    55,    56,
      -1,    -1,    -1,    60,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      87,    -1,    89,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    -1,    99,   100,   101,   102,    -1,   104,   105,    -1,
     107,    -1,   109,   110,   111,    -1,    -1,   114,   115,   116,
      -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    -1,    -1,   134,    -1,    -1,
     137,    -1,    -1,   140,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    55,    56,    -1,    -1,
      -1,    60,    -1,    -1,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      89,    90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,   100,   101,   102,    -1,   104,   105,    -1,   107,    -1,
     109,   110,   111,    -1,    -1,   114,   115,   116,    -1,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,    -1,    -1,    -1,   134,    -1,    -1,   137,    -1,
      -1,   140,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    52,    53,    -1,    55,    56,    -1,    -1,    -1,    60,
      -1,    -1,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    85,    86,    87,    -1,    89,    90,
      91,    92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,
     101,   102,    -1,   104,   105,    -1,   107,    -1,   109,   110,
     111,    -1,    -1,   114,   115,   116,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,    -1,    -1,   130,
      -1,    -1,    -1,   134,    -1,    -1,   137,    16,    -1,   140,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    43,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    63,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      -1,    90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,   100,   101,   102,    -1,   104,   105,    -1,   107,    -1,
     109,   110,   111,    -1,    -1,   114,   115,   116,     3,   118,
       5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,   137,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    -1,    -1,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    87,    -1,    -1,    90,    91,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,   100,   101,   102,    -1,   104,
     105,     3,   107,    -1,   109,   110,   111,    -1,    -1,   114,
     115,   116,    -1,   118,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,   101,
     102,    -1,   104,   105,    -1,   107,    -1,   109,   110,   111,
      -1,    -1,   114,   115,   116,     3,   118,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,   137,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    46,    -1,
      -1,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    -1,
      68,    69,    70,    -1,    -1,    -1,    -1,    75,    76,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    85,    86,    87,
      -1,    -1,    90,    91,    92,    -1,    94,    -1,    96,    -1,
      -1,    99,   100,   101,   102,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,    -1,    -1,   114,   115,   116,    -1,
     118,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,   101,
     102,    -1,   104,   105,    -1,   107,    -1,   109,   110,   111,
      -1,    -1,   114,   115,   116,    -1,   118,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    -1,    43,    -1,    -1,
      46,    -1,    -1,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    60,    -1,    -1,    63,    64,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    85,
      86,    87,    -1,    -1,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    -1,    99,   100,   101,   102,    -1,   104,   105,
      -1,   107,    -1,   109,   110,   111,     3,     4,   114,   115,
     116,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    43,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      87,    -1,    -1,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    -1,    99,   100,   101,   102,    -1,   104,   105,    -1,
     107,    -1,   109,   110,   111,    -1,    -1,   114,   115,   116,
       3,   118,     5,     6,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    76,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    87,    -1,    -1,    90,    91,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,   101,   102,
      -1,   104,   105,     3,   107,    -1,   109,   110,   111,    -1,
      10,   114,   115,   116,    -1,   118,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    28,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,   101,   102,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,    -1,   114,   115,   116,     3,   118,     5,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    85,
      86,    87,    -1,    -1,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    -1,    99,   100,   101,   102,    -1,   104,   105,
       3,   107,     5,   109,   110,   111,    -1,    -1,   114,   115,
     116,    -1,   118,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    87,    -1,    -1,    90,    91,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,   101,   102,
      -1,   104,   105,     3,   107,    -1,   109,   110,   111,    -1,
      -1,   114,   115,   116,    -1,   118,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,   101,   102,    -1,   104,   105,     3,   107,    -1,   109,
     110,   111,    -1,    -1,   114,   115,   116,    -1,   118,    16,
      -1,    18,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      87,    -1,    72,    90,    91,    92,    -1,    94,    -1,    96,
      -1,    -1,    99,   100,   101,   102,    -1,   104,   105,    -1,
     107,     3,   109,   110,   111,    -1,     8,   114,   115,   116,
      -1,   118,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,    46,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    90,    91,
      92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,   101,
     102,    -1,   104,   105,     3,   107,    -1,   109,   110,   111,
      -1,    -1,   114,   115,   116,    -1,   118,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      -1,    90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,   100,   101,   102,    -1,   104,   105,     3,   107,    -1,
     109,   110,   111,    -1,    -1,   114,   115,   116,    -1,   118,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    85,
      86,    87,    -1,    -1,    90,    91,    92,    -1,    94,    -1,
      96,    -1,    -1,    99,   100,   101,   102,    -1,   104,   105,
       3,   107,    -1,   109,   110,   111,    -1,    -1,   114,   115,
     116,    -1,   118,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    87,    -1,    -1,    90,    91,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,   101,   102,
      -1,   104,   105,     3,   107,    -1,   109,   110,   111,     9,
      -1,   114,   115,   116,    -1,   118,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    46,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    -1,    68,    69,
      70,    40,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,   101,   102,    72,   104,   105,    40,   107,    -1,   109,
     110,   111,    -1,    -1,   114,   115,   116,    -1,   118,    -1,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,    72,    40,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
      16,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    35,
      -1,    40,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    80,    -1,    82,    83,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,
      96,    -1,    -1,    40,   100,    -1,    -1,    -1,   104,   105,
      -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,   115,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,    72,    -1,    -1,    -1,   138,
      -1,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,
     138,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,    40,    -1,    -1,    -1,
     138,    -1,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,    40,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
      -1,    -1,    -1,    -1,   138,    -1,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,    17,    -1,    -1,    20,   138,    22,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    31,    -1,    -1,    34,    -1,
      36,    37,    38,    -1,    -1,    40,    42,    -1,    -1,    -1,
      46,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    72,    -1,    91,    -1,    93,    -1,    95,
      -1,    97,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   109,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,    -1,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    46,    58,    79,    88,    91,    93,    95,    97,    99,
     109,   112,   146,   147,   150,   151,   152,   153,   160,   201,
     204,   205,   210,   211,   212,   213,   224,   225,   234,   235,
     236,   237,   238,   242,   243,   248,   249,   250,   253,   255,
     256,   257,   258,   259,   261,   262,   263,   264,   265,   103,
      57,     3,    16,    20,    21,    22,    25,    29,    33,    46,
      49,    50,    51,    52,    53,    63,    64,    65,    66,    68,
      69,    70,    75,    80,    81,    82,    83,    84,    85,    86,
      87,    90,    91,    92,    94,    96,    99,   100,   101,   102,
     104,   105,   107,   109,   110,   111,   114,   115,   116,   118,
     148,   149,    49,    81,    48,    49,    81,    85,    92,    57,
       3,     4,     5,     6,     8,     9,    10,    14,    21,    24,
      33,    41,    45,    52,    53,    55,    56,    60,    69,    71,
      89,   102,   116,   130,   131,   134,   137,   140,   149,   161,
     162,   164,   194,   195,   260,   266,   269,   270,    10,    28,
      50,    57,    75,    96,   148,   251,    16,    28,    29,    35,
      50,    57,    70,    80,    82,    83,    96,   100,   104,   105,
     115,   203,   251,   107,    92,   149,   165,     0,    43,   136,
     276,    61,   149,   149,   149,   137,   149,   149,   165,   149,
     149,   149,   149,   149,   137,   142,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
       3,    69,    71,   194,   194,   194,    56,   149,   198,   199,
      13,    14,   143,   267,   268,    48,   139,    18,   149,   163,
      40,    72,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,    67,   186,   187,
      62,   123,   123,    95,     9,   148,   149,     5,     6,     8,
      76,   149,   208,   209,   123,   107,     8,   100,   149,    95,
      66,   202,   149,   202,   202,   202,    77,   188,   189,   114,
     149,    95,   139,   194,   273,   274,   275,    93,    97,   151,
     149,   202,    15,    42,   106,     5,     6,     8,    14,   130,
     137,   173,   174,   220,   226,   227,    90,   110,   117,   167,
     110,     8,   137,   138,   194,   196,   197,   149,   194,   194,
     196,    39,   131,   196,   196,   138,   194,   196,   196,   194,
     194,   194,   194,   138,   137,   137,   137,   138,   139,   141,
     123,     8,   194,   268,   137,   165,   162,   194,   149,   194,
     194,   194,   194,   194,   194,   194,   270,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,     5,    76,   134,
     194,   208,   208,   123,   123,    50,   130,     8,    44,    76,
     108,   130,   149,   173,   206,   207,    63,   100,   202,   100,
       8,   100,   149,   190,   191,    66,   117,   244,   245,   149,
     239,   240,   266,   149,   163,    26,   139,   271,   272,   137,
     214,    30,    30,    92,     5,     6,     8,   138,   173,   175,
     228,   139,   229,    24,    45,    59,   101,   254,     8,     4,
      24,    33,    41,    43,    52,    55,    60,    68,   116,   137,
     149,   168,   169,   170,   171,   172,   266,     8,    93,   153,
      47,   138,   139,   138,   138,   149,   138,   138,   138,   138,
     138,   138,   138,   139,   138,   139,   139,   138,   194,   194,
     199,   149,   173,   200,   144,   144,   157,   166,   167,   139,
      76,   137,     5,   206,     9,   209,    65,   202,   202,   123,
     139,     8,   149,   246,   247,   123,   139,   167,   123,   273,
      78,   182,   183,   275,    55,   149,   215,   216,   113,   149,
     149,   149,   138,   139,   138,   139,   220,   227,   230,   231,
     138,    98,    98,   137,   137,   137,   137,   137,   137,   137,
     137,   168,   120,    23,    56,    62,   123,   124,   125,   126,
     127,   128,   134,   138,   149,   197,   138,   194,   194,   194,
     123,    93,   160,    51,   176,     5,   175,   123,    86,    87,
      94,   252,     5,     8,   137,   149,   191,   123,   119,   137,
     173,   174,   240,   188,   173,   174,    26,   138,   139,   137,
     217,   218,    24,    25,    45,    60,    64,    73,    74,   101,
     241,   173,     8,    18,   232,   139,     8,     8,   149,   266,
     131,   266,   138,   138,   266,     8,   138,   138,   168,   173,
     174,     9,   137,    76,   134,     8,   173,   174,     8,   173,
     174,   173,   174,   173,   174,   173,   174,   173,   174,    56,
     139,   154,    56,   138,   138,   139,   200,   161,   138,     5,
     177,   118,   180,   181,   138,   137,    32,   111,    86,   149,
     192,   193,   137,     8,   247,   138,   175,    84,   172,   184,
     185,   215,   137,   219,   220,    77,   139,   221,    67,   149,
     233,   220,   231,   138,   138,   138,   138,   138,   120,   120,
     175,    76,     9,   137,     5,    55,   149,   155,   156,   138,
     266,   137,    48,    78,   158,    26,    51,    54,   179,   175,
     123,   138,   139,     8,   138,   137,    19,    37,   139,   138,
     139,     3,   222,   223,   218,   173,   174,   173,   174,   138,
     175,   139,   138,   196,    26,    67,   159,   172,   178,    78,
     170,   182,   138,   173,   193,   138,   138,   185,   220,   123,
     139,   138,   156,   138,   184,     5,   139,    26,   186,     8,
     223,   139,   139,   172,   184,   188,   137,     5,   196,   138,
     138
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

  case 81:

    { pParser->PushQuery(); ;}
    break;

  case 82:

    { pParser->PushQuery(); ;}
    break;

  case 83:

    { pParser->PushQuery(); ;}
    break;

  case 87:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 89:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 92:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 93:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 97:

    {
		CSphVector<CSphQueryItem> & dItems = pParser->m_pQuery->m_dItems;
		if ( dItems.GetLength()!=1 || dItems[0].m_sExpr!="*" )
		{
			yyerror ( pParser, "outer select list must be a single star" );
			YYERROR;
		}
		dItems.Reset();
		pParser->ResetSelect();
	;}
    break;

  case 98:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 100:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 101:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 102:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 105:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 108:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 109:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 110:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 111:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 112:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 113:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 114:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 115:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 116:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 117:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 118:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 120:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 127:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 129:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 130:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 131:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 132:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 133:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 135:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 136:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 155:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 156:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 157:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 158:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 159:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 165:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 166:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 167:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 168:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 169:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 170:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 171:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 175:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 176:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 177:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 179:

    {
			pParser->AddHaving();
		;}
    break;

  case 182:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 185:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 186:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 188:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 190:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 191:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 194:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 195:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 201:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 202:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 203:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 204:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 205:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 206:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 207:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 208:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 210:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 211:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 216:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 217:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 218:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 219:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 220:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 221:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 222:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 223:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 224:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 225:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 226:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 227:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 228:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 229:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 230:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 231:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 232:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 233:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 234:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 235:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 236:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 240:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 241:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 242:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 243:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 244:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 245:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 247:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 248:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 249:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 250:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 251:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 252:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 258:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 259:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 266:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 267:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 268:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 269:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 270:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 271:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 272:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 273:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 275:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 276:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 277:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 278:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 279:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 280:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 281:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 282:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 283:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 284:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 285:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 286:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 287:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 290:

    { yyval.m_iValue = 1; ;}
    break;

  case 291:

    { yyval.m_iValue = 0; ;}
    break;

  case 292:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 300:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 301:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 302:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 305:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 306:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 307:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 312:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 313:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 316:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 317:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 318:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 319:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 320:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 321:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 322:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 323:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 328:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 329:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 330:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 331:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 332:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 334:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 335:

    {
			// FIXME? for now, one such array per CALL statement, tops
			if ( pParser->m_pStmt->m_dCallStrings.GetLength() )
			{
				yyerror ( pParser, "unexpected constant string list" );
				YYERROR;
			}
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 336:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 339:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 341:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 346:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 349:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 350:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 351:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 354:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)yyvsp[0].m_iValue );
			DWORD uHi = (DWORD)( yyvsp[0].m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_INTEGER );
			}
		;}
    break;

  case 355:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 356:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 357:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 358:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)yyvsp[0].m_iValue );
			DWORD uHi = (DWORD)( yyvsp[0].m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_INTEGER );
			}
		;}
    break;

  case 359:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 360:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 361:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 362:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 363:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 364:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 365:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 366:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 367:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 368:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 369:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 370:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 371:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 378:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 379:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 380:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 388:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 389:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 390:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 391:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 392:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 393:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 394:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 395:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 396:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 397:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 400:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 401:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 402:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 403:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 404:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 405:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 407:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 408:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 409:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 410:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 411:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 412:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 413:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 417:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 419:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 422:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
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
	    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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

