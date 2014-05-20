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
     TOK_RECONFIGURE = 342,
     TOK_REPEATABLE = 343,
     TOK_REPLACE = 344,
     TOK_REMAP = 345,
     TOK_RETURNS = 346,
     TOK_ROLLBACK = 347,
     TOK_RTINDEX = 348,
     TOK_SELECT = 349,
     TOK_SERIALIZABLE = 350,
     TOK_SET = 351,
     TOK_SESSION = 352,
     TOK_SHOW = 353,
     TOK_SONAME = 354,
     TOK_START = 355,
     TOK_STATUS = 356,
     TOK_STRING = 357,
     TOK_SUM = 358,
     TOK_TABLE = 359,
     TOK_TABLES = 360,
     TOK_THREADS = 361,
     TOK_TO = 362,
     TOK_TRANSACTION = 363,
     TOK_TRUE = 364,
     TOK_TRUNCATE = 365,
     TOK_TYPE = 366,
     TOK_UNCOMMITTED = 367,
     TOK_UPDATE = 368,
     TOK_VALUES = 369,
     TOK_VARIABLES = 370,
     TOK_WARNINGS = 371,
     TOK_WEIGHT = 372,
     TOK_WHERE = 373,
     TOK_WITHIN = 374,
     TOK_OR = 375,
     TOK_AND = 376,
     TOK_NE = 377,
     TOK_GTE = 378,
     TOK_LTE = 379,
     TOK_NOT = 380,
     TOK_NEG = 381
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
#define TOK_RECONFIGURE 342
#define TOK_REPEATABLE 343
#define TOK_REPLACE 344
#define TOK_REMAP 345
#define TOK_RETURNS 346
#define TOK_ROLLBACK 347
#define TOK_RTINDEX 348
#define TOK_SELECT 349
#define TOK_SERIALIZABLE 350
#define TOK_SET 351
#define TOK_SESSION 352
#define TOK_SHOW 353
#define TOK_SONAME 354
#define TOK_START 355
#define TOK_STATUS 356
#define TOK_STRING 357
#define TOK_SUM 358
#define TOK_TABLE 359
#define TOK_TABLES 360
#define TOK_THREADS 361
#define TOK_TO 362
#define TOK_TRANSACTION 363
#define TOK_TRUE 364
#define TOK_TRUNCATE 365
#define TOK_TYPE 366
#define TOK_UNCOMMITTED 367
#define TOK_UPDATE 368
#define TOK_VALUES 369
#define TOK_VARIABLES 370
#define TOK_WARNINGS 371
#define TOK_WEIGHT 372
#define TOK_WHERE 373
#define TOK_WITHIN 374
#define TOK_OR 375
#define TOK_AND 376
#define TOK_NE 377
#define TOK_GTE 378
#define TOK_LTE 379
#define TOK_NOT 380
#define TOK_NEG 381




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
#define YYFINAL  188
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4449

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  146
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  132
/* YYNRULES -- Number of rules. */
#define YYNRULES  423
/* YYNRULES -- Number of states. */
#define YYNSTATES  774

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   381

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   134,   123,     2,
     138,   139,   132,   130,   140,   131,   143,   133,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   137,
     126,   124,   127,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   144,     2,   145,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   141,   122,   142,     2,     2,     2,     2,
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
     115,   116,   117,   118,   119,   120,   121,   125,   128,   129,
     135,   136
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
    1173,  1178,  1183,  1189,  1190,  1192,  1195,  1197,  1201,  1205,
    1208,  1212,  1219,  1220,  1222,  1224,  1227,  1230,  1233,  1235,
    1243,  1245,  1247,  1249,  1251,  1255,  1262,  1266,  1270,  1274,
    1276,  1280,  1283,  1287,  1291,  1299,  1305,  1308,  1310,  1313,
    1315,  1317,  1321,  1325,  1329,  1333,  1335,  1336,  1339,  1341,
    1344,  1346,  1348,  1352
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     147,     0,    -1,   148,    -1,   151,    -1,   151,   137,    -1,
     213,    -1,   225,    -1,   205,    -1,   206,    -1,   211,    -1,
     226,    -1,   235,    -1,   237,    -1,   238,    -1,   239,    -1,
     244,    -1,   249,    -1,   250,    -1,   254,    -1,   256,    -1,
     257,    -1,   258,    -1,   259,    -1,   251,    -1,   260,    -1,
     262,    -1,   263,    -1,   264,    -1,   243,    -1,   265,    -1,
     266,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    29,    -1,    33,    -1,    46,    -1,
      49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,
      63,    -1,    65,    -1,    66,    -1,    68,    -1,    69,    -1,
      70,    -1,    80,    -1,    81,    -1,    82,    -1,    83,    -1,
      85,    -1,    84,    -1,    86,    -1,    88,    -1,    91,    -1,
      92,    -1,    93,    -1,    95,    -1,    97,    -1,   100,    -1,
     101,    -1,   102,    -1,   103,    -1,   105,    -1,   106,    -1,
     110,    -1,   111,    -1,   112,    -1,   115,    -1,   116,    -1,
     117,    -1,   119,    -1,    64,    -1,   149,    -1,    75,    -1,
     108,    -1,   152,    -1,   151,   137,   152,    -1,   151,   277,
      -1,   153,    -1,   202,    -1,   154,    -1,    94,     3,   138,
     138,   154,   139,   155,   139,    -1,   161,    -1,    94,   162,
      48,   138,   158,   161,   139,   159,   160,    -1,    -1,   140,
     156,    -1,   157,    -1,   156,   140,   157,    -1,   150,    -1,
       5,    -1,    55,    -1,    -1,    78,    26,   185,    -1,    -1,
      67,     5,    -1,    67,     5,   140,     5,    -1,    94,   162,
      48,   166,   167,   177,   181,   180,   183,   187,   189,    -1,
     163,    -1,   162,   140,   163,    -1,   132,    -1,   165,   164,
      -1,    -1,   150,    -1,    18,   150,    -1,   195,    -1,    21,
     138,   195,   139,    -1,    69,   138,   195,   139,    -1,    71,
     138,   195,   139,    -1,   103,   138,   195,   139,    -1,    53,
     138,   195,   139,    -1,    33,   138,   132,   139,    -1,    52,
     138,   139,    -1,    33,   138,    39,   150,   139,    -1,   150,
      -1,   166,   140,   150,    -1,    -1,   168,    -1,   118,   169,
      -1,   170,    -1,   169,   121,   169,    -1,   138,   169,   139,
      -1,    68,   138,     8,   139,    -1,   171,    -1,   173,   124,
     174,    -1,   173,   125,   174,    -1,   173,    56,   138,   176,
     139,    -1,   173,   135,    56,   138,   176,   139,    -1,   173,
      56,     9,    -1,   173,   135,    56,     9,    -1,   173,    23,
     174,   121,   174,    -1,   173,   127,   174,    -1,   173,   126,
     174,    -1,   173,   128,   174,    -1,   173,   129,   174,    -1,
     173,   124,   175,    -1,   172,    -1,   173,    23,   175,   121,
     175,    -1,   173,    23,   174,   121,   175,    -1,   173,    23,
     175,   121,   174,    -1,   173,   127,   175,    -1,   173,   126,
     175,    -1,   173,   128,   175,    -1,   173,   129,   175,    -1,
     173,   124,     8,    -1,   173,   125,     8,    -1,   173,    62,
      76,    -1,   173,    62,   135,    76,    -1,   173,   125,   175,
      -1,   150,    -1,     4,    -1,    33,   138,   132,   139,    -1,
      52,   138,   139,    -1,   117,   138,   139,    -1,    55,    -1,
     267,    -1,    60,   138,   267,   139,    -1,    41,   138,   267,
     139,    -1,    24,   138,   267,   139,    -1,    43,   138,   139,
      -1,     5,    -1,   131,     5,    -1,     6,    -1,   131,     6,
      -1,    14,    -1,   174,    -1,   176,   140,   174,    -1,    -1,
      51,   178,    26,   179,    -1,    -1,     5,    -1,   173,    -1,
     179,   140,   173,    -1,    -1,    54,   171,    -1,    -1,   182,
      -1,   119,    51,    78,    26,   185,    -1,    -1,   184,    -1,
      78,    26,   185,    -1,    78,    26,    84,   138,   139,    -1,
     186,    -1,   185,   140,   186,    -1,   173,    -1,   173,    19,
      -1,   173,    37,    -1,    -1,   188,    -1,    67,     5,    -1,
      67,     5,   140,     5,    -1,    -1,   190,    -1,    77,   191,
      -1,   192,    -1,   191,   140,   192,    -1,   150,   124,   150,
      -1,   150,   124,     5,    -1,   150,   124,   138,   193,   139,
      -1,   150,   124,   150,   138,     8,   139,    -1,   150,   124,
       8,    -1,   194,    -1,   193,   140,   194,    -1,   150,   124,
     174,    -1,   150,    -1,     4,    -1,    55,    -1,     5,    -1,
       6,    -1,    14,    -1,     9,    -1,   131,   195,    -1,   135,
     195,    -1,   195,   130,   195,    -1,   195,   131,   195,    -1,
     195,   132,   195,    -1,   195,   133,   195,    -1,   195,   126,
     195,    -1,   195,   127,   195,    -1,   195,   123,   195,    -1,
     195,   122,   195,    -1,   195,   134,   195,    -1,   195,    40,
     195,    -1,   195,    72,   195,    -1,   195,   129,   195,    -1,
     195,   128,   195,    -1,   195,   124,   195,    -1,   195,   125,
     195,    -1,   195,   121,   195,    -1,   195,   120,   195,    -1,
     138,   195,   139,    -1,   141,   199,   142,    -1,   196,    -1,
     267,    -1,   270,    -1,   267,    62,    76,    -1,   267,    62,
     135,    76,    -1,     3,   138,   197,   139,    -1,    56,   138,
     197,   139,    -1,    60,   138,   197,   139,    -1,    24,   138,
     197,   139,    -1,    45,   138,   197,   139,    -1,    41,   138,
     197,   139,    -1,     3,   138,   139,    -1,    71,   138,   195,
     140,   195,   139,    -1,    69,   138,   195,   140,   195,   139,
      -1,   117,   138,   139,    -1,     3,   138,   195,    47,   150,
      56,   267,   139,    -1,    90,   138,   195,   140,   195,   140,
     138,   197,   139,   140,   138,   197,   139,   139,    -1,   198,
      -1,   197,   140,   198,    -1,   195,    -1,     8,    -1,   200,
     124,   201,    -1,   199,   140,   200,   124,   201,    -1,   150,
      -1,    56,    -1,   174,    -1,   150,    -1,    98,   204,    -1,
      -1,    66,     8,    -1,   116,    -1,   101,   203,    -1,    70,
     203,    -1,    16,   101,   203,    -1,    83,    -1,    80,    -1,
      82,    -1,   106,   189,    -1,    16,     8,   101,   203,    -1,
      16,   150,   101,   203,    -1,    57,   150,   101,    -1,    96,
     149,   124,   208,    -1,    96,   149,   124,   207,    -1,    96,
     149,   124,    76,    -1,    96,    75,   209,    -1,    96,    10,
     124,   209,    -1,    96,    28,    96,   209,    -1,    96,    50,
       9,   124,   138,   176,   139,    -1,    96,    50,   149,   124,
     207,    -1,    96,    50,   149,   124,     5,    -1,    96,    57,
     150,    50,     9,   124,   138,   176,   139,    -1,   150,    -1,
       8,    -1,   109,    -1,    44,    -1,   174,    -1,   210,    -1,
     209,   131,   210,    -1,   150,    -1,    76,    -1,     8,    -1,
       5,    -1,     6,    -1,    31,    -1,    92,    -1,   212,    -1,
      22,    -1,   100,   108,    -1,   214,    61,   150,   215,   114,
     218,   222,    -1,    58,    -1,    89,    -1,    -1,   138,   217,
     139,    -1,   150,    -1,    55,    -1,   216,    -1,   217,   140,
     216,    -1,   219,    -1,   218,   140,   219,    -1,   138,   220,
     139,    -1,   221,    -1,   220,   140,   221,    -1,   174,    -1,
     175,    -1,     8,    -1,   138,   176,   139,    -1,   138,   139,
      -1,    -1,    77,   223,    -1,   224,    -1,   223,   140,   224,
      -1,     3,   124,     8,    -1,    36,    48,   166,   168,    -1,
      27,   150,   138,   227,   230,   139,    -1,   228,    -1,   227,
     140,   228,    -1,   221,    -1,   138,   229,   139,    -1,     8,
      -1,   229,   140,     8,    -1,    -1,   140,   231,    -1,   232,
      -1,   231,   140,   232,    -1,   221,   233,   234,    -1,    -1,
      18,    -1,   150,    -1,    67,    -1,   236,   150,   203,    -1,
      38,    -1,    37,    -1,    98,   105,   203,    -1,    98,    35,
     203,    -1,   113,   166,    96,   240,   168,   189,    -1,   241,
      -1,   240,   140,   241,    -1,   150,   124,   174,    -1,   150,
     124,   175,    -1,   150,   124,   138,   176,   139,    -1,   150,
     124,   138,   139,    -1,   267,   124,   174,    -1,   267,   124,
     175,    -1,    60,    -1,    24,    -1,    45,    -1,    25,    -1,
      73,    -1,    74,    -1,    64,    -1,   102,    -1,    17,   104,
     150,    15,    30,   150,   242,    -1,    17,   104,   150,    42,
      30,   150,    -1,    17,    93,   150,    87,    -1,    98,   252,
     115,   245,    -1,    98,   252,   115,    66,     8,    -1,    -1,
     246,    -1,   118,   247,    -1,   248,    -1,   247,   120,   248,
      -1,   150,   124,     8,    -1,    98,    29,    -1,    98,    28,
      96,    -1,    96,   252,   108,    63,    65,   253,    -1,    -1,
      50,    -1,    97,    -1,    86,   112,    -1,    86,    32,    -1,
      88,    86,    -1,    95,    -1,    34,    49,   150,    91,   255,
      99,     8,    -1,    59,    -1,    24,    -1,    45,    -1,   102,
      -1,    42,    49,   150,    -1,    20,    57,   150,   107,    93,
     150,    -1,    46,    93,   150,    -1,    46,    85,   150,    -1,
      94,   261,   187,    -1,    10,    -1,    10,   143,   150,    -1,
      94,   195,    -1,   110,    93,   150,    -1,    79,    57,   150,
      -1,    34,    81,   150,   111,     8,    99,     8,    -1,    42,
      81,   150,   111,     8,    -1,   150,   268,    -1,   269,    -1,
     268,   269,    -1,    13,    -1,    14,    -1,   144,   195,   145,
      -1,   144,     8,   145,    -1,   195,   124,   271,    -1,   271,
     124,   195,    -1,     8,    -1,    -1,   273,   276,    -1,    26,
      -1,   275,   164,    -1,   195,    -1,   274,    -1,   276,   140,
     274,    -1,    43,   276,   272,   183,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   172,   172,   173,   174,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   214,   215,   215,   215,   215,   215,   216,   216,   216,
     216,   217,   217,   217,   217,   217,   218,   218,   218,   218,
     218,   219,   219,   219,   219,   219,   219,   219,   220,   220,
     220,   220,   221,   221,   221,   221,   221,   222,   222,   222,
     222,   222,   222,   223,   223,   223,   223,   224,   228,   228,
     228,   234,   235,   236,   240,   241,   245,   246,   254,   255,
     262,   264,   268,   272,   279,   280,   281,   285,   298,   305,
     307,   312,   321,   337,   338,   342,   343,   346,   348,   349,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   365,
     366,   369,   371,   375,   379,   380,   381,   385,   390,   394,
     401,   409,   417,   426,   431,   436,   441,   446,   451,   456,
     461,   466,   471,   476,   481,   486,   491,   496,   501,   506,
     511,   516,   521,   529,   533,   534,   539,   545,   551,   557,
     563,   564,   565,   566,   567,   571,   572,   583,   584,   585,
     589,   595,   601,   603,   606,   608,   615,   619,   625,   627,
     633,   635,   639,   650,   652,   656,   660,   667,   668,   672,
     673,   674,   677,   679,   683,   688,   695,   697,   701,   705,
     706,   710,   715,   720,   726,   731,   739,   744,   751,   761,
     762,   763,   764,   765,   766,   767,   768,   769,   771,   772,
     773,   774,   775,   776,   777,   778,   779,   780,   781,   782,
     783,   784,   785,   786,   787,   788,   789,   790,   791,   792,
     793,   794,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   813,   814,   818,   819,   823,   824,
     828,   829,   833,   834,   840,   843,   845,   849,   850,   851,
     852,   853,   854,   855,   856,   857,   862,   867,   877,   882,
     887,   892,   893,   894,   898,   903,   908,   913,   922,   923,
     927,   928,   929,   941,   942,   946,   947,   948,   949,   950,
     957,   958,   959,   963,   964,   970,   978,   979,   982,   984,
     988,   989,   993,   994,   998,   999,  1003,  1007,  1008,  1012,
    1013,  1014,  1015,  1016,  1019,  1020,  1024,  1025,  1029,  1035,
    1045,  1053,  1057,  1064,  1065,  1072,  1082,  1088,  1090,  1094,
    1099,  1103,  1110,  1112,  1116,  1117,  1123,  1131,  1132,  1138,
    1142,  1148,  1156,  1157,  1161,  1175,  1181,  1185,  1190,  1204,
    1215,  1216,  1217,  1218,  1219,  1220,  1221,  1222,  1226,  1234,
    1241,  1252,  1256,  1263,  1264,  1268,  1272,  1273,  1277,  1281,
    1288,  1295,  1301,  1302,  1303,  1307,  1308,  1309,  1310,  1316,
    1327,  1328,  1329,  1330,  1335,  1346,  1358,  1367,  1378,  1386,
    1387,  1391,  1401,  1412,  1423,  1434,  1445,  1448,  1449,  1453,
    1454,  1455,  1456,  1460,  1461,  1465,  1470,  1472,  1476,  1485,
    1489,  1497,  1498,  1502
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
  "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", "TOK_RECONFIGURE", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_REMAP", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", "TOK_TABLES", 
  "TOK_THREADS", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_TYPE", "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", 
  "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", 
  "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", 
  "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", 
  "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", "'}'", "'.'", 
  "'['", "']'", "$accept", "request", "statement", "ident_set", "ident", 
  "multi_stmt_list", "multi_stmt", "select", "select1", 
  "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg", 
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
     375,   376,   124,    38,    61,   377,    60,    62,   378,   379,
      43,    45,    42,    47,    37,   380,   381,    59,    40,    41,
      44,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   146,   147,   147,   147,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   150,   150,
     150,   151,   151,   151,   152,   152,   153,   153,   154,   154,
     155,   155,   156,   156,   157,   157,   157,   158,   159,   160,
     160,   160,   161,   162,   162,   163,   163,   164,   164,   164,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   166,
     166,   167,   167,   168,   169,   169,   169,   170,   170,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   172,   173,   173,   173,   173,   173,   173,
     173,   173,   173,   173,   173,   174,   174,   175,   175,   175,
     176,   176,   177,   177,   178,   178,   179,   179,   180,   180,
     181,   181,   182,   183,   183,   184,   184,   185,   185,   186,
     186,   186,   187,   187,   188,   188,   189,   189,   190,   191,
     191,   192,   192,   192,   192,   192,   193,   193,   194,   195,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   196,   196,   196,   196,   196,   196,   196,   196,
     196,   196,   196,   196,   197,   197,   198,   198,   199,   199,
     200,   200,   201,   201,   202,   203,   203,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   205,   205,
     205,   205,   205,   205,   206,   206,   206,   206,   207,   207,
     208,   208,   208,   209,   209,   210,   210,   210,   210,   210,
     211,   211,   211,   212,   212,   213,   214,   214,   215,   215,
     216,   216,   217,   217,   218,   218,   219,   220,   220,   221,
     221,   221,   221,   221,   222,   222,   223,   223,   224,   225,
     226,   227,   227,   228,   228,   229,   229,   230,   230,   231,
     231,   232,   233,   233,   234,   234,   235,   236,   236,   237,
     238,   239,   240,   240,   241,   241,   241,   241,   241,   241,
     242,   242,   242,   242,   242,   242,   242,   242,   243,   243,
     243,   244,   244,   245,   245,   246,   247,   247,   248,   249,
     250,   251,   252,   252,   252,   253,   253,   253,   253,   254,
     255,   255,   255,   255,   256,   257,   258,   259,   260,   261,
     261,   262,   263,   264,   265,   266,   267,   268,   268,   269,
     269,   269,   269,   270,   270,   271,   272,   272,   273,   274,
     275,   276,   276,   277
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
       4,     4,     5,     0,     1,     2,     1,     3,     3,     2,
       3,     6,     0,     1,     1,     2,     2,     2,     1,     7,
       1,     1,     1,     1,     3,     6,     3,     3,     3,     1,
       3,     2,     3,     3,     7,     5,     2,     1,     2,     1,
       1,     3,     3,     3,     3,     1,     0,     2,     1,     2,
       1,     1,     3,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   303,     0,   300,     0,     0,   348,   347,
       0,     0,   306,     0,   307,   301,     0,   382,   382,     0,
       0,     0,     0,     2,     3,    81,    84,    86,    88,    85,
       7,     8,     9,   302,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    77,    46,    47,
      48,    49,    50,    79,    51,    52,    53,    54,    56,    55,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    80,    70,    71,    72,    73,    74,    75,
      76,    78,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    31,   210,   212,   213,   415,   215,   399,   214,    34,
       0,    38,     0,     0,    43,    44,   211,     0,     0,    49,
       0,     0,    67,    75,     0,   105,     0,     0,     0,   209,
       0,   103,   107,   110,   237,   192,   238,   239,     0,     0,
       0,    41,     0,     0,    63,     0,     0,     0,     0,   379,
     265,   383,     0,   265,   272,   273,   271,   384,   265,   265,
     196,   267,   264,     0,   304,     0,   119,     0,     1,     0,
       4,    83,     0,   265,     0,     0,     0,     0,     0,     0,
       0,   394,     0,   397,   396,   403,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    31,    49,     0,   216,   217,     0,   261,   260,
       0,     0,   409,   410,     0,   406,   407,     0,     0,     0,
     108,   106,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     398,   193,     0,     0,     0,     0,     0,     0,     0,   298,
     299,   297,   296,   295,   281,   293,     0,     0,     0,   265,
       0,   380,     0,   350,     0,   269,   268,   349,     0,   274,
     197,   373,   402,     0,     0,   420,   421,   107,   416,     0,
       0,    82,   308,   346,   370,     0,     0,     0,   165,   167,
     321,   169,     0,     0,   319,   320,   333,   337,   331,     0,
       0,     0,   329,     0,   257,     0,   248,   256,     0,   254,
     400,     0,   256,     0,     0,     0,     0,     0,   117,     0,
       0,     0,     0,     0,     0,     0,   251,     0,     0,     0,
     235,     0,   236,     0,   415,     0,   408,    97,   121,   104,
     110,   109,   227,   228,   234,   233,   225,   224,   231,   413,
     232,   222,   223,   230,   229,   218,   219,   220,   221,   226,
     194,   240,     0,   414,   282,   283,     0,     0,     0,     0,
     289,   291,   280,   290,     0,   288,   292,   279,   278,     0,
     265,   270,   265,   266,   277,     0,   198,   199,     0,     0,
     371,   374,     0,     0,   352,     0,   120,   419,   418,     0,
     183,     0,     0,     0,     0,     0,     0,   166,   168,   335,
     323,   170,     0,     0,     0,     0,   391,   392,   390,   393,
       0,     0,   155,     0,    38,     0,     0,    43,   159,     0,
      48,    75,     0,   154,   123,   124,   128,   141,     0,   160,
     405,     0,     0,     0,   242,     0,   111,   245,     0,   116,
     247,   246,   115,   243,   244,   112,     0,   113,     0,     0,
     114,     0,     0,     0,   263,   262,   258,   412,   411,     0,
     172,   122,     0,   241,     0,   286,   285,     0,   294,     0,
     275,   276,     0,     0,   372,     0,   375,   376,     0,     0,
     196,     0,   422,     0,   423,   184,   417,   311,   310,   312,
       0,     0,     0,   369,   395,   322,     0,   334,     0,   342,
     332,   338,   339,   330,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    90,     0,   255,   118,
       0,     0,     0,     0,     0,     0,   174,   180,   195,     0,
       0,     0,     0,   388,   381,   202,   205,     0,   201,   200,
       0,     0,     0,   354,   355,   353,   351,   358,   359,     0,
     309,     0,     0,   324,   314,   361,   363,   362,   360,   366,
     364,   365,   367,   368,   171,   336,   343,     0,     0,   389,
     404,     0,     0,     0,     0,   164,   157,     0,     0,   158,
     126,   125,     0,     0,   133,     0,   151,     0,   149,   129,
     140,   150,   130,   153,   137,   146,   136,   145,   138,   147,
     139,   148,     0,     0,     0,     0,   250,   249,     0,   259,
       0,     0,   175,     0,     0,   178,   181,   284,     0,   386,
     385,   387,     0,     0,   206,     0,   378,   377,   357,     0,
      56,   189,   185,   187,   313,     0,     0,   317,     0,     0,
     305,   345,   344,   341,   342,   340,   163,   156,   162,   161,
     127,     0,     0,     0,   152,   134,     0,    95,    96,    94,
      91,    92,    87,     0,     0,     0,     0,    99,     0,     0,
       0,   183,     0,     0,   203,     0,     0,   356,     0,   190,
     191,     0,   316,     0,     0,   325,   326,   315,   135,   143,
     144,   142,   131,     0,     0,   252,     0,     0,     0,    89,
     176,   173,     0,   179,   192,   287,   208,   207,   204,   186,
     188,   318,     0,     0,   132,    93,     0,    98,   100,     0,
       0,   196,   328,   327,     0,     0,   177,   182,   102,     0,
     101,     0,     0,   253
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   111,   149,    24,    25,    26,    27,   644,
     700,   701,   489,   707,   739,    28,   150,   151,   241,   152,
     358,   490,   322,   454,   455,   456,   457,   458,   431,   315,
     432,   567,   653,   741,   711,   655,   656,   514,   515,   672,
     673,   260,   261,   289,   290,   406,   407,   663,   664,   332,
     154,   328,   329,   230,   231,   486,    29,   283,   182,    30,
      31,   397,   398,   274,   275,    32,    33,    34,    35,   423,
     519,   520,   593,   594,   676,   316,   680,   725,   726,    36,
      37,   317,   318,   433,   435,   531,   532,   607,   683,    38,
      39,    40,    41,    42,   413,   414,   603,    43,    44,   410,
     411,   506,   507,    45,    46,    47,   166,   574,    48,   440,
      49,    50,    51,    52,    53,   155,    54,    55,    56,    57,
      58,   156,   235,   236,   157,   158,   420,   421,   296,   297,
     298,   191
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -639
static const short yypact[] =
{
     538,    -9,    24,  -639,  3695,  -639,    36,    40,  -639,  -639,
      47,    81,  -639,    85,  -639,  -639,   718,  2850,  4045,     0,
      58,  3695,   153,  -639,   -13,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,   126,  -639,  -639,  -639,  3695,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  3695,
    3695,  3695,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,    55,  3695,  3695,  3695,  3695,  3695,  3695,  3695,
    3695,    78,  -639,  -639,  -639,  -639,  -639,    70,  -639,    93,
     100,   110,   112,   119,   138,   144,  -639,   148,   150,   154,
     158,   165,   172,   178,  1552,  -639,  1552,  1552,  3173,    13,
       3,  -639,  3277,   542,  -639,   135,   258,  -639,   202,   205,
     250,  3799,  3695,  2746,   240,   226,   243,  3383,   256,  -639,
     287,  -639,  3695,   287,  -639,  -639,  -639,  -639,   287,   287,
     277,  -639,  -639,   242,  -639,  3695,  -639,   -48,  -639,  1552,
      88,  -639,  3695,   287,   271,    65,   252,    23,   269,   251,
     -20,  -639,   253,  -639,  -639,  -639,   857,  3695,  1552,  1691,
      -7,  1691,  1691,   222,  1552,  1691,  1691,  1552,  1552,  1552,
    1552,   224,   227,   229,   230,  -639,  -639,  4042,  -639,  -639,
      75,   245,  -639,  -639,  1830,    13,  -639,  2172,   996,  3695,
    -639,  -639,  1552,  1552,  1552,  1552,  1552,  1552,  1552,  1552,
    1552,  1552,  1552,  1552,  1552,  1552,  1552,  1552,  1552,   366,
    -639,  -639,   -34,  1552,  2746,  2746,   248,   249,   324,  -639,
    -639,  -639,  -639,  -639,   244,  -639,  2287,   313,   276,     5,
     278,  -639,   372,  -639,   280,  -639,  -639,  -639,  3695,  -639,
    -639,    38,  -639,  3695,  3695,  4241,  -639,  3277,   -10,  1135,
      20,  -639,   246,  -639,  -639,   352,   356,   295,  -639,  -639,
    -639,  -639,   184,    16,  -639,  -639,  -639,   254,  -639,    30,
     382,  1953,  -639,   383,   268,  1274,  -639,  4220,   -46,  -639,
    -639,  4063,  4241,    97,  3695,   257,   104,   107,  -639,  4083,
     113,   115,   557,  3800,  3821,  4105,  -639,  1413,  1552,  1552,
    -639,  3173,  -639,  2404,   259,   211,  -639,  -639,   -20,  -639,
    4241,  -639,  -639,  -639,  4315,   338,  2761,    95,   417,  -639,
     417,   167,   167,   167,   167,    46,    46,  -639,  -639,  -639,
     262,  -639,   317,   417,   244,   244,   265,  2965,   388,  2746,
    -639,  -639,  -639,  -639,   401,  -639,  -639,  -639,  -639,   342,
     287,  -639,   287,  -639,  -639,   284,   272,  -639,   405,  3695,
    -639,  -639,    48,    51,  -639,   285,  -639,  -639,  -639,  1552,
     336,  1552,  3487,   302,  3695,  3695,  3695,  -639,  -639,  -639,
    -639,  -639,   123,   125,    23,   303,  -639,  -639,  -639,  -639,
     318,   346,  -639,   308,   309,   311,   312,   314,  -639,   315,
     316,   335,  1953,    13,   320,  -639,  -639,  -639,   142,  -639,
    -639,   996,   340,  3695,  -639,  1691,  -639,  -639,   349,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  1552,  -639,  1552,  1552,
    -639,  3863,  3896,   327,  -639,  -639,  -639,  -639,  -639,   361,
     424,  -639,   451,  -639,     7,  -639,  -639,   353,  -639,   146,
    -639,  -639,  2068,  3695,  -639,   367,   370,  -639,    39,  3695,
     277,    68,  -639,   466,  -639,  -639,   354,  -639,  -639,  -639,
     134,   355,   185,  -639,  -639,  -639,     7,  -639,   489,    61,
    -639,   360,  -639,  -639,   493,   494,  3695,   375,  3695,   365,
     369,  3695,   501,   371,    73,  1953,    68,     1,   -30,    44,
      64,    68,    68,    68,    68,   455,   373,   456,  -639,  -639,
    4178,  4200,  3926,  2404,   996,   376,   509,   397,  -639,   139,
     379,    11,   432,  -639,  -639,  -639,  -639,  3695,   381,  -639,
     512,  3695,     6,  -639,  -639,  -639,  -639,  -639,  -639,  2521,
    -639,  3487,    33,   -57,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  3591,    33,  -639,
    -639,    13,   384,   392,   394,  -639,  -639,   396,   399,  -639,
    -639,  -639,   400,   406,  -639,     7,  -639,   446,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,    14,  3069,   413,  3695,  -639,  -639,   398,  -639,
      43,   461,  -639,   527,   503,   502,  -639,  -639,     7,  -639,
    -639,  -639,   433,   141,  -639,   554,  -639,  -639,  -639,   155,
     425,   177,   426,  -639,  -639,    10,   166,  -639,   561,   355,
    -639,  -639,  -639,  -639,   549,  -639,  -639,  -639,  -639,  -639,
    -639,    68,    68,   169,  -639,  -639,     7,  -639,  -639,  -639,
     428,  -639,  -639,   431,  1691,  3695,   545,   511,  2631,   505,
    2631,   336,   173,     7,  -639,  3695,   440,  -639,   442,  -639,
    -639,  2631,  -639,    33,   462,   448,  -639,  -639,  -639,  -639,
    -639,  -639,  -639,   175,  3069,  -639,   179,  2631,   584,  -639,
    -639,   450,   565,  -639,   135,  -639,  -639,  -639,  -639,  -639,
    -639,  -639,   585,   561,  -639,  -639,   452,   426,   454,  2631,
    2631,   277,  -639,  -639,   457,   593,  -639,   426,  -639,  1691,
    -639,   182,   460,  -639
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -639,  -639,  -639,   -11,    -4,  -639,   411,  -639,   279,  -639,
    -639,  -129,  -639,  -639,  -639,   117,    45,   374,   310,  -639,
      19,  -639,  -282,  -418,  -639,  -102,  -639,  -575,   -68,  -486,
    -485,  -639,  -639,  -639,  -639,  -639,  -639,  -101,  -639,  -638,
    -110,  -131,  -639,  -505,  -639,  -639,   116,  -639,  -100,   181,
    -639,  -208,   151,  -639,   267,    57,  -639,  -160,  -639,  -639,
    -639,   234,  -639,    59,   233,  -639,  -639,  -639,  -639,  -639,
      34,  -639,  -639,   -53,  -639,  -432,  -639,  -639,  -125,  -639,
    -639,  -639,   197,  -639,  -639,  -639,    25,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,   128,  -639,  -639,  -639,  -639,
    -639,  -639,    54,  -639,  -639,  -639,   622,  -639,  -639,  -639,
    -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,  -639,
    -639,  -234,  -639,   408,  -639,   402,  -639,  -639,   225,  -639,
     228,  -639
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -416
static const short yytable[] =
{
     112,   333,   529,   336,   337,   586,   165,   340,   341,   569,
     624,   308,   308,   285,   671,   308,   418,   186,   286,   287,
     678,   308,   584,   695,   429,   588,   232,   233,   308,   309,
     189,   310,   334,   303,   544,   193,   167,   311,   308,   309,
     187,   310,   381,   659,   308,   309,   626,   311,   293,   308,
     309,   237,   628,   311,   436,   194,   195,   196,   311,   415,
     623,   232,   233,   630,   633,   635,   637,   639,   641,   308,
     309,   282,   631,   308,   309,   437,   491,   172,   311,   606,
     305,    61,   311,   679,    59,   113,   242,   459,   115,   438,
     173,   705,   294,   464,   465,    60,   116,   669,   321,   757,
     174,   382,   175,   176,   408,   627,   -65,   306,   184,   198,
     199,   186,   201,   202,   203,   204,   205,   114,   243,   401,
     294,   178,   767,   660,   190,   335,   180,   621,   117,   314,
     419,   510,   439,   740,   200,   242,   181,   394,   394,   625,
     693,   394,   120,   238,   229,   668,   671,   394,   240,   430,
     267,   185,   696,   188,   312,   430,   409,   234,   268,   273,
     677,   313,   671,   280,   312,   546,   118,   243,   284,   321,
     312,   675,   508,   712,   119,   312,   684,   582,   256,   257,
     258,   292,   299,   238,   766,   671,   300,   192,   302,   427,
     428,   509,   234,   197,   545,   312,   719,   153,   547,   312,
    -333,  -333,   259,   330,   548,   729,   731,   242,   396,   595,
     596,   733,   620,   207,   720,   351,   206,   352,   459,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     597,   208,   571,   186,   572,   361,   467,   465,   209,   243,
     500,   573,   501,   470,   465,   598,   471,   465,   210,   599,
     211,   242,   473,   465,   474,   465,   768,   212,   600,   601,
     273,   273,   525,   526,   527,   528,   549,   550,   551,   552,
     553,   554,   395,   590,   591,   415,   213,   555,   657,   526,
     714,   715,   214,   243,   405,   485,   215,   602,   216,   412,
     416,   751,   217,   240,   717,   526,   218,   254,   255,   256,
     257,   258,   612,   219,   614,   722,   723,   617,   732,   526,
     220,   459,   745,   526,   754,   526,   221,   453,   756,   465,
     262,   772,   465,   384,   385,   225,   263,   226,   227,   264,
     468,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   265,   229,  -384,   484,
     276,   277,   281,   282,   288,   459,   488,   291,   304,   307,
     319,   338,   320,   346,   323,   347,   314,   348,   349,   353,
     295,   380,   386,   387,   388,   389,   399,   400,   242,   402,
     403,   404,   424,   395,   422,   273,   425,   327,   426,   331,
     441,   460,  -415,   493,   434,   339,   469,   497,   342,   343,
     344,   345,   492,   494,   487,   505,   427,   499,   502,   511,
     243,   703,   503,   504,   513,   355,   521,   534,   518,   360,
     522,   523,   524,   362,   363,   364,   365,   366,   367,   368,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     583,   545,   533,   587,   383,   535,   536,   537,   453,   538,
     539,   563,   540,   541,   542,   564,   568,   242,   604,   557,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   543,   459,   566,   459,   570,   622,   556,
     360,   629,   632,   634,   636,   638,   640,   459,   559,   243,
     581,   580,   589,   592,   419,   485,   736,   605,   578,   405,
     608,   609,   610,   459,   615,   412,   227,   613,   616,   618,
     619,   642,   645,   643,   652,   651,   654,   658,   661,   665,
     666,   691,   694,   686,   314,   459,   459,   692,   327,   481,
     482,   687,   611,   688,   611,   689,   704,   611,   690,   706,
     314,   453,  -401,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   702,   708,   709,     1,   710,   713,     2,   484,
       3,   771,   716,   718,   724,     4,   721,   606,   734,     5,
     735,   737,     6,   662,     7,     8,     9,   505,   738,   748,
      10,   749,   242,   742,    11,   453,   752,   518,   753,   758,
     759,   760,   764,   762,   765,   769,    12,   242,   770,   773,
     295,   301,   295,   682,   462,   755,   565,   417,   743,   650,
     744,   750,   359,   761,   243,   747,   558,    13,   483,   579,
     649,   496,   498,   728,   730,   674,   727,    14,   763,   243,
      15,   530,    16,   685,    17,   667,    18,   585,    19,   699,
     183,   611,   360,   356,   512,   746,     0,     0,    20,   516,
     369,    21,     0,     0,     0,   314,     0,   560,     0,   561,
     562,     0,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,     0,     0,     0,     0,   475,   476,     0,     0,
       0,   186,     0,     0,   453,     0,   453,     0,     0,     0,
       0,   662,     0,     0,     0,     0,     0,   453,     0,     0,
       0,   121,   122,   123,   124,     0,   125,   126,   127,     0,
     699,     0,   128,   453,    63,     0,     0,     0,    64,   129,
      66,     0,   130,    67,     0,   360,     0,    68,     0,     0,
       0,   131,     0,     0,     0,   453,   453,     0,     0,   132,
       0,     0,     0,   133,    70,     0,     0,    71,    72,    73,
     134,   135,     0,   136,   137,     0,     0,     0,   138,     0,
       0,    76,    77,    78,    79,     0,    80,   139,    82,   140,
       0,     0,     0,    83,     0,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,     0,    91,     0,   141,    92,
      93,    94,     0,    95,     0,    96,     0,     0,    97,    98,
      99,   142,     0,   101,   102,     0,   103,     0,   104,   105,
     106,     0,     0,   107,   108,   143,     0,   110,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   144,
     145,     0,     0,   146,     0,     0,   147,     0,     0,   148,
     222,   122,   123,   124,     0,   324,   126,     0,     0,     0,
       0,   128,     0,    63,     0,     0,     0,    64,    65,    66,
       0,   130,    67,     0,     0,     0,    68,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,   132,     0,
       0,     0,   133,    70,     0,     0,    71,    72,    73,    74,
      75,     0,   136,   137,     0,     0,     0,   138,     0,     0,
      76,    77,    78,    79,     0,    80,   223,    82,   224,     0,
       0,     0,    83,     0,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,     0,    91,     0,   141,    92,    93,
      94,     0,    95,     0,    96,     0,     0,    97,    98,    99,
     100,     0,   101,   102,     0,   103,     0,   104,   105,   106,
       0,     0,   107,   108,   143,     0,   110,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   144,     0,
       0,     0,   146,     0,     0,   325,   326,     0,   148,   222,
     122,   123,   124,     0,   125,   126,     0,     0,     0,     0,
     128,     0,    63,     0,     0,     0,    64,   129,    66,     0,
     130,    67,     0,     0,     0,    68,     0,     0,     0,   131,
       0,     0,     0,     0,     0,     0,     0,   132,     0,     0,
       0,   133,    70,     0,     0,    71,    72,    73,   134,   135,
       0,   136,   137,     0,     0,     0,   138,     0,     0,    76,
      77,    78,    79,     0,    80,   139,    82,   140,     0,     0,
       0,    83,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,     0,    91,     0,   141,    92,    93,    94,
       0,    95,     0,    96,     0,     0,    97,    98,    99,   142,
       0,   101,   102,     0,   103,     0,   104,   105,   106,     0,
       0,   107,   108,   143,     0,   110,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   144,   145,     0,
       0,   146,     0,     0,   147,     0,     0,   148,   121,   122,
     123,   124,     0,   125,   126,     0,     0,     0,     0,   128,
       0,    63,     0,     0,     0,    64,   129,    66,     0,   130,
      67,     0,     0,     0,    68,     0,     0,     0,   131,     0,
       0,     0,     0,     0,     0,     0,   132,     0,     0,     0,
     133,    70,     0,     0,    71,    72,    73,   134,   135,     0,
     136,   137,     0,     0,     0,   138,     0,     0,    76,    77,
      78,    79,     0,    80,   139,    82,   140,     0,     0,     0,
      83,     0,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,     0,    91,     0,   141,    92,    93,    94,     0,
      95,     0,    96,     0,     0,    97,    98,    99,   142,     0,
     101,   102,     0,   103,     0,   104,   105,   106,     0,     0,
     107,   108,   143,     0,   110,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   144,   145,     0,     0,
     146,     0,     0,   147,     0,     0,   148,   222,   122,   123,
     124,     0,   125,   126,     0,     0,     0,     0,   128,     0,
      63,     0,     0,     0,    64,    65,    66,     0,   130,    67,
       0,     0,     0,    68,     0,     0,     0,    69,     0,     0,
       0,     0,     0,     0,     0,   132,     0,     0,     0,   133,
      70,     0,     0,    71,    72,    73,    74,    75,     0,   136,
     137,     0,     0,     0,   138,     0,     0,    76,    77,    78,
      79,     0,    80,   223,    82,   224,     0,     0,     0,    83,
       0,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,     0,    91,     0,   141,    92,    93,    94,   461,    95,
       0,    96,     0,     0,    97,    98,    99,   100,     0,   101,
     102,     0,   103,     0,   104,   105,   106,     0,     0,   107,
     108,   143,     0,   110,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   144,     0,     0,     0,   146,
       0,     0,   147,     0,     0,   148,   222,   122,   123,   124,
       0,   324,   126,     0,     0,     0,     0,   128,     0,    63,
       0,     0,     0,    64,    65,    66,     0,   130,    67,     0,
       0,     0,    68,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,   132,     0,     0,     0,   133,    70,
       0,     0,    71,    72,    73,    74,    75,     0,   136,   137,
       0,     0,     0,   138,     0,     0,    76,    77,    78,    79,
       0,    80,   223,    82,   224,     0,     0,     0,    83,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
       0,    91,     0,   141,    92,    93,    94,     0,    95,     0,
      96,     0,     0,    97,    98,    99,   100,     0,   101,   102,
       0,   103,     0,   104,   105,   106,     0,     0,   107,   108,
     143,     0,   110,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   144,     0,     0,     0,   146,     0,
       0,   147,   326,     0,   148,   222,   122,   123,   124,     0,
     125,   126,     0,     0,     0,     0,   128,     0,    63,     0,
       0,     0,    64,    65,    66,     0,   130,    67,     0,     0,
       0,    68,     0,     0,     0,    69,     0,     0,     0,     0,
       0,     0,     0,   132,     0,     0,     0,   133,    70,     0,
       0,    71,    72,    73,    74,    75,     0,   136,   137,     0,
       0,     0,   138,     0,     0,    76,    77,    78,    79,     0,
      80,   223,    82,   224,     0,     0,     0,    83,     0,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,     0,
      91,     0,   141,    92,    93,    94,     0,    95,     0,    96,
       0,     0,    97,    98,    99,   100,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,   143,
       0,   110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   144,     0,     0,     0,   146,     0,     0,
     147,     0,     0,   148,   222,   122,   123,   124,     0,   324,
     126,     0,     0,     0,     0,   128,     0,    63,     0,     0,
       0,    64,    65,    66,     0,   130,    67,     0,     0,     0,
      68,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,   133,    70,     0,     0,
      71,    72,    73,    74,    75,     0,   136,   137,     0,     0,
       0,   138,     0,     0,    76,    77,    78,    79,     0,    80,
     223,    82,   224,     0,     0,     0,    83,     0,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,     0,    91,
       0,   141,    92,    93,    94,     0,    95,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,     0,   103,
       0,   104,   105,   106,     0,     0,   107,   108,   143,     0,
     110,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   144,     0,     0,     0,   146,     0,     0,   147,
       0,     0,   148,   222,   122,   123,   124,     0,   354,   126,
       0,     0,     0,     0,   128,     0,    63,     0,     0,     0,
      64,    65,    66,     0,   130,    67,     0,     0,     0,    68,
       0,     0,     0,    69,     0,     0,     0,     0,     0,     0,
       0,   132,     0,     0,     0,   133,    70,     0,     0,    71,
      72,    73,    74,    75,     0,   136,   137,     0,     0,     0,
     138,     0,     0,    76,    77,    78,    79,     0,    80,   223,
      82,   224,     0,     0,     0,    83,     0,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,     0,    91,     0,
     141,    92,    93,    94,     0,    95,     0,    96,     0,     0,
      97,    98,    99,   100,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,   143,     0,   110,
       0,     0,     0,     0,     0,     0,    62,   442,     0,     0,
       0,   144,     0,     0,     0,   146,     0,     0,   147,    63,
       0,   148,     0,    64,    65,    66,     0,   443,    67,     0,
       0,     0,    68,     0,     0,     0,   444,     0,     0,     0,
       0,     0,     0,     0,   445,     0,   446,     0,     0,    70,
       0,     0,    71,    72,    73,   447,    75,     0,   448,     0,
       0,     0,     0,   449,     0,     0,    76,    77,    78,    79,
       0,   450,    81,    82,     0,     0,     0,     0,    83,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
       0,    91,     0,     0,    92,    93,    94,     0,    95,     0,
      96,     0,     0,    97,    98,    99,   100,     0,   101,   102,
       0,   103,     0,   104,   105,   106,     0,     0,   107,   108,
     451,    62,   110,   575,     0,     0,   576,     0,     0,     0,
       0,     0,     0,     0,    63,     0,     0,     0,    64,    65,
      66,   452,     0,    67,     0,     0,     0,    68,     0,     0,
       0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    70,     0,     0,    71,    72,    73,
      74,    75,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    76,    77,    78,    79,     0,    80,    81,    82,     0,
       0,     0,     0,    83,     0,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,     0,    91,     0,     0,    92,
      93,    94,     0,    95,     0,    96,     0,     0,    97,    98,
      99,   100,     0,   101,   102,    62,   103,     0,   104,   105,
     106,     0,     0,   107,   108,   109,     0,   110,    63,     0,
       0,     0,    64,    65,    66,     0,     0,    67,     0,     0,
       0,    68,     0,     0,     0,    69,   577,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    70,     0,
       0,    71,    72,    73,    74,    75,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    76,    77,    78,    79,     0,
      80,    81,    82,     0,     0,     0,     0,    83,     0,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,     0,
      91,     0,     0,    92,    93,    94,     0,    95,     0,    96,
       0,     0,    97,    98,    99,   100,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,   109,
      62,   110,   308,     0,     0,   390,     0,     0,     0,     0,
       0,     0,     0,    63,     0,     0,     0,    64,    65,    66,
     357,     0,    67,     0,     0,     0,    68,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   391,     0,    70,     0,     0,    71,    72,    73,    74,
      75,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      76,    77,    78,    79,     0,    80,    81,    82,     0,     0,
       0,     0,    83,   392,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,     0,    91,     0,     0,    92,    93,
      94,     0,    95,     0,    96,     0,     0,    97,    98,    99,
     100,     0,   101,   102,     0,   103,   393,   104,   105,   106,
       0,     0,   107,   108,   109,     0,   110,    62,     0,   308,
       0,     0,     0,     0,     0,     0,     0,     0,   394,     0,
      63,     0,     0,     0,    64,    65,    66,     0,     0,    67,
       0,     0,     0,    68,     0,     0,     0,    69,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      70,     0,     0,    71,    72,    73,    74,    75,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    76,    77,    78,
      79,     0,    80,    81,    82,     0,     0,     0,     0,    83,
       0,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,     0,    91,     0,     0,    92,    93,    94,     0,    95,
       0,    96,     0,     0,    97,    98,    99,   100,     0,   101,
     102,     0,   103,     0,   104,   105,   106,     0,     0,   107,
     108,   109,     0,   110,    62,   442,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   394,     0,    63,     0,     0,
       0,    64,    65,    66,     0,   443,    67,     0,     0,     0,
      68,     0,     0,     0,   444,     0,     0,     0,     0,     0,
       0,     0,   445,     0,   446,     0,     0,    70,     0,     0,
      71,    72,    73,   447,    75,     0,   448,     0,     0,     0,
       0,   449,     0,     0,    76,    77,    78,    79,     0,    80,
      81,    82,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    84,    85,    86,    87,   670,    89,    90,     0,    91,
       0,     0,    92,    93,    94,     0,    95,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,     0,   103,
       0,   104,   105,   106,    62,   442,   107,   108,   451,     0,
     110,     0,     0,     0,     0,     0,     0,    63,     0,     0,
       0,    64,    65,    66,     0,   443,    67,     0,     0,     0,
      68,     0,     0,     0,   444,     0,     0,     0,     0,     0,
       0,     0,   445,     0,   446,     0,     0,    70,     0,     0,
      71,    72,    73,   447,    75,     0,   448,     0,     0,     0,
       0,   449,     0,     0,    76,    77,    78,    79,     0,    80,
      81,    82,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,     0,    91,
       0,     0,    92,    93,    94,     0,    95,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,     0,   103,
       0,   104,   105,   106,     0,     0,   107,   108,   451,    62,
     110,   269,   270,     0,   271,     0,     0,     0,     0,     0,
       0,     0,    63,     0,     0,     0,    64,    65,    66,     0,
       0,    67,     0,     0,     0,    68,     0,     0,     0,    69,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    70,     0,     0,    71,    72,    73,    74,    75,
       0,   242,     0,     0,     0,     0,     0,     0,     0,    76,
      77,    78,    79,     0,    80,    81,    82,     0,     0,     0,
       0,    83,   272,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,   243,    91,     0,     0,    92,    93,    94,
       0,    95,     0,    96,     0,     0,    97,    98,    99,   100,
       0,   101,   102,    62,   103,     0,   104,   105,   106,     0,
     159,   107,   108,   109,     0,   110,    63,     0,     0,     0,
      64,    65,    66,     0,     0,    67,     0,     0,   160,    68,
       0,     0,     0,    69,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,    70,     0,     0,    71,
     161,    73,    74,    75,     0,     0,     0,   162,     0,     0,
       0,     0,     0,    76,    77,    78,    79,     0,    80,    81,
      82,     0,     0,     0,     0,   163,     0,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,     0,    91,     0,
       0,    92,    93,    94,     0,    95,     0,   164,     0,     0,
      97,    98,    99,   100,     0,   101,   102,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,   109,    62,   110,
     495,     0,     0,   390,     0,     0,     0,     0,     0,     0,
       0,    63,     0,     0,     0,    64,    65,    66,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,    69,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70,     0,     0,    71,    72,    73,    74,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    76,    77,
      78,    79,     0,    80,    81,    82,     0,     0,     0,     0,
      83,     0,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,     0,    91,     0,     0,    92,    93,    94,     0,
      95,     0,    96,     0,     0,    97,    98,    99,   100,     0,
     101,   102,    62,   103,   697,   104,   105,   106,     0,     0,
     107,   108,   109,     0,   110,    63,     0,     0,     0,    64,
      65,    66,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,     0,     0,    71,    72,
      73,    74,    75,     0,   698,     0,     0,     0,     0,     0,
       0,     0,    76,    77,    78,    79,     0,    80,    81,    82,
       0,     0,     0,     0,    83,     0,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,     0,    91,     0,     0,
      92,    93,    94,     0,    95,     0,    96,     0,     0,    97,
      98,    99,   100,     0,   101,   102,    62,   103,     0,   104,
     105,   106,     0,     0,   107,   108,   109,     0,   110,    63,
       0,     0,     0,    64,    65,    66,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
       0,     0,    71,    72,    73,    74,    75,     0,     0,   228,
       0,     0,     0,     0,     0,     0,    76,    77,    78,    79,
       0,    80,    81,    82,     0,     0,     0,     0,    83,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
       0,    91,     0,     0,    92,    93,    94,     0,    95,     0,
      96,     0,     0,    97,    98,    99,   100,     0,   101,   102,
      62,   103,     0,   104,   105,   106,     0,     0,   107,   108,
     109,     0,   110,    63,     0,   239,     0,    64,    65,    66,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,     0,     0,    71,    72,    73,    74,
      75,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      76,    77,    78,    79,     0,    80,    81,    82,     0,     0,
       0,     0,    83,     0,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,     0,    91,     0,     0,    92,    93,
      94,     0,    95,     0,    96,     0,     0,    97,    98,    99,
     100,     0,   101,   102,     0,   103,    62,   104,   105,   106,
       0,   278,   107,   108,   109,     0,   110,     0,     0,    63,
       0,     0,     0,    64,    65,    66,     0,     0,    67,     0,
       0,     0,    68,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
       0,     0,    71,    72,    73,    74,    75,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,    77,    78,    79,
       0,    80,    81,    82,     0,     0,     0,     0,    83,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
       0,    91,     0,     0,    92,    93,    94,     0,    95,     0,
      96,     0,     0,    97,   279,    99,   100,     0,   101,   102,
      62,   103,     0,   104,   105,   106,     0,     0,   107,   108,
     109,     0,   110,    63,     0,     0,     0,    64,    65,    66,
       0,     0,    67,     0,     0,     0,    68,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,     0,     0,    71,    72,    73,    74,
      75,     0,   517,     0,     0,     0,     0,     0,     0,     0,
      76,    77,    78,    79,     0,    80,    81,    82,     0,     0,
       0,     0,    83,     0,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,     0,    91,     0,     0,    92,    93,
      94,     0,    95,     0,    96,     0,     0,    97,    98,    99,
     100,     0,   101,   102,    62,   103,     0,   104,   105,   106,
       0,     0,   107,   108,   109,     0,   110,    63,     0,     0,
       0,    64,    65,    66,     0,     0,    67,     0,     0,     0,
      68,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    70,     0,     0,
      71,    72,    73,    74,    75,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    76,    77,    78,    79,   681,    80,
      81,    82,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,     0,    91,
       0,     0,    92,    93,    94,     0,    95,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,    62,   103,
       0,   104,   105,   106,     0,     0,   107,   108,   109,     0,
     110,    63,     0,     0,     0,    64,    65,    66,     0,     0,
      67,     0,     0,     0,    68,     0,     0,     0,    69,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70,     0,     0,    71,    72,    73,    74,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    76,    77,
      78,    79,     0,    80,    81,    82,     0,     0,     0,     0,
      83,     0,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,     0,    91,     0,     0,    92,    93,    94,     0,
      95,     0,    96,     0,     0,    97,    98,    99,   100,     0,
     101,   102,    62,   103,     0,   104,   105,   106,   266,     0,
     107,   108,   109,     0,   110,    63,     0,     0,     0,    64,
      65,    66,     0,     0,    67,     0,     0,     0,    68,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
     242,     0,     0,     0,     0,    70,     0,     0,    71,    72,
      73,    74,    75,     0,     0,     0,     0,     0,     0,     0,
       0,   242,    76,    77,    78,    79,     0,    80,    81,    82,
       0,     0,   243,     0,     0,     0,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,     0,    91,     0,     0,
      92,    93,    94,   243,    95,     0,    96,     0,     0,    97,
      98,    99,   100,   242,   101,   102,     0,  -383,     0,   104,
     105,   106,     0,     0,   107,   108,   109,     0,   110,     0,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   243,   242,     0,     0,   477,
     478,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,     0,     0,     0,     0,
       0,   479,     0,     0,     0,     0,   242,     0,   243,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   243,     0,
       0,     0,     0,   476,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,     0,     0,     0,     0,     0,   478,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   167,     0,     0,     0,     0,   648,     0,     0,     0,
       0,     0,     0,   168,   169,     0,     0,     0,     0,     0,
     170,     0,   242,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,     0,     0,     0,     0,
       0,     0,   172,   242,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   243,   173,     0,     0,     0,     0,
       0,     0,     0,   242,     0,   174,     0,   175,   176,     0,
       0,     0,     0,     0,     0,   243,     0,     0,     0,     0,
       0,     0,   177,     0,     0,   242,   178,     0,     0,     0,
     179,   180,     0,     0,     0,   243,     0,     0,     0,     0,
       0,   181,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   243,     0,     0,
       0,   350,     0,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,     0,     0,
       0,     0,   466,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   242,     0,
       0,     0,   472,     0,     0,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     242,     0,     0,     0,   480,     0,     0,     0,     0,     0,
     243,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     242,     0,     0,     0,     0,     0,     0,   463,     0,     0,
       0,     0,   243,     0,     0,     0,     0,     0,     0,     0,
       0,   242,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   243,     0,     0,     0,     0,     0,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   243,     0,     0,     0,   646,     0,     0,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,     0,     0,     0,     0,   647,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   242,     0,     0,     0,     0,
       0,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   243,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258
};

static const short yycheck[] =
{
       4,   209,   434,   211,   212,   510,    17,   215,   216,   494,
       9,     5,     5,   173,   589,     5,    26,    21,   178,   179,
      77,     5,   508,     9,     8,   511,    13,    14,     5,     6,
      43,     8,    39,   193,   452,    39,    16,    14,     5,     6,
      21,     8,    76,    32,     5,     6,    76,    14,    96,     5,
       6,    48,     8,    14,    24,    59,    60,    61,    14,   293,
     546,    13,    14,   549,   550,   551,   552,   553,   554,     5,
       6,    66,     8,     5,     6,    45,   358,    57,    14,    18,
      15,    57,    14,   140,    93,    49,    40,   321,    48,    59,
      70,    48,   140,   139,   140,   104,    49,   582,   118,   737,
      80,   135,    82,    83,    66,   135,   101,    42,   108,   113,
     114,   115,   116,   117,   118,   119,   120,    81,    72,   279,
     140,   101,   760,   112,   137,   132,   106,   545,    81,   197,
     140,   413,   102,   708,   115,    40,   116,   131,   131,   138,
     625,   131,    57,   140,   148,   139,   721,   131,   152,   139,
     161,    93,   138,     0,   131,   139,   118,   144,   162,   163,
     592,   138,   737,   167,   131,    23,    85,    72,   172,   118,
     131,   138,   124,   658,    93,   131,   608,   138,   132,   133,
     134,   185,    94,   140,   759,   760,    98,    61,   192,     5,
       6,   140,   144,   138,   121,   131,    19,    16,    56,   131,
     139,   140,    67,   207,    62,   691,   692,    40,   276,    24,
      25,   696,   139,   143,    37,   140,   138,   142,   452,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
      45,   138,    86,   237,    88,   239,   139,   140,   138,    72,
     400,    95,   402,   139,   140,    60,   139,   140,   138,    64,
     138,    40,   139,   140,   139,   140,   761,   138,    73,    74,
     264,   265,   139,   140,   139,   140,   124,   125,   126,   127,
     128,   129,   276,   139,   140,   509,   138,   135,   139,   140,
     139,   140,   138,    72,   288,   353,   138,   102,   138,   293,
     294,   723,   138,   297,   139,   140,   138,   130,   131,   132,
     133,   134,   536,   138,   538,   139,   140,   541,   139,   140,
     138,   545,   139,   140,   139,   140,   138,   321,   139,   140,
      62,   139,   140,   264,   265,   144,   124,   146,   147,   124,
     334,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,    96,   351,   108,   353,
     124,   108,    96,    66,    77,   589,   145,   115,    87,   107,
      91,   139,   111,   139,   111,   138,   434,   138,   138,   124,
     189,     5,   124,   124,    50,   131,    63,   101,    40,   101,
       8,   101,    30,   387,   138,   389,    30,   206,    93,   208,
       8,     8,   124,    76,   140,   214,   139,     9,   217,   218,
     219,   220,   140,   138,   145,   409,     5,    65,   124,   124,
      72,   645,   140,     8,    78,   234,   114,    99,   422,   238,
     424,   425,   426,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     508,   121,   139,   511,   263,    99,   138,   138,   452,   138,
     138,   124,   138,   138,   138,    94,     5,    40,   526,   463,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   138,   708,    51,   710,   124,   546,   139,
     299,   549,   550,   551,   552,   553,   554,   721,   139,    72,
     120,   124,    26,   138,   140,   563,   704,     8,   502,   503,
     140,     8,     8,   737,   139,   509,   325,   132,   139,     8,
     139,    56,    56,   140,     5,   139,   119,   138,    86,   138,
       8,   121,    76,   139,   592,   759,   760,   121,   347,   348,
     349,   139,   536,   139,   538,   139,   138,   541,   139,    78,
     608,   545,     0,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   139,    26,    51,    17,    54,   124,    20,   563,
      22,   769,     8,   138,     3,    27,   140,    18,   140,    31,
     139,    26,    34,   577,    36,    37,    38,   581,    67,   139,
      42,   139,    40,    78,    46,   589,   124,   591,   140,     5,
     140,    26,   140,     8,   140,   138,    58,    40,     5,   139,
     419,   190,   421,   607,   325,   734,   489,   297,   710,   564,
     711,   721,   238,   744,    72,   715,   465,    79,   351,   503,
     563,   387,   389,   691,   692,   591,   679,    89,   753,    72,
      92,   434,    94,   608,    96,   581,    98,   509,   100,   643,
      18,   645,   461,   235,   419,   713,    -1,    -1,   110,   421,
     248,   113,    -1,    -1,    -1,   723,    -1,   476,    -1,   478,
     479,    -1,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,    -1,    -1,    -1,    -1,   139,   140,    -1,    -1,
      -1,   705,    -1,    -1,   708,    -1,   710,    -1,    -1,    -1,
      -1,   715,    -1,    -1,    -1,    -1,    -1,   721,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,    -1,
     734,    -1,    14,   737,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    24,    25,    -1,   564,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,   759,   760,    -1,    -1,    41,
      -1,    -1,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    55,    56,    -1,    -1,    -1,    60,    -1,
      -1,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    -1,    88,    -1,    90,    91,
      92,    93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,
     102,   103,    -1,   105,   106,    -1,   108,    -1,   110,   111,
     112,    -1,    -1,   115,   116,   117,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,    -1,   135,    -1,    -1,   138,    -1,    -1,   141,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    -1,    45,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    55,    56,    -1,    -1,    -1,    60,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    -1,    90,    91,    92,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,   102,
     103,    -1,   105,   106,    -1,   108,    -1,   110,   111,   112,
      -1,    -1,   115,   116,   117,    -1,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,
      -1,    -1,   135,    -1,    -1,   138,   139,    -1,   141,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    52,    53,
      -1,    55,    56,    -1,    -1,    -1,    60,    -1,    -1,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    -1,
      -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,    -1,    90,    91,    92,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,   101,   102,   103,
      -1,   105,   106,    -1,   108,    -1,   110,   111,   112,    -1,
      -1,   115,   116,   117,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
      -1,   135,    -1,    -1,   138,    -1,    -1,   141,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    52,    53,    -1,
      55,    56,    -1,    -1,    -1,    60,    -1,    -1,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    -1,    90,    91,    92,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,   102,   103,    -1,
     105,   106,    -1,   108,    -1,   110,   111,   112,    -1,    -1,
     115,   116,   117,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,    -1,
     135,    -1,    -1,   138,    -1,    -1,   141,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,
      46,    -1,    -1,    49,    50,    51,    52,    53,    -1,    55,
      56,    -1,    -1,    -1,    60,    -1,    -1,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    85,
      86,    -1,    88,    -1,    90,    91,    92,    93,    94,    95,
      -1,    97,    -1,    -1,   100,   101,   102,   103,    -1,   105,
     106,    -1,   108,    -1,   110,   111,   112,    -1,    -1,   115,
     116,   117,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,    -1,   135,
      -1,    -1,   138,    -1,    -1,   141,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    55,    56,
      -1,    -1,    -1,    60,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    -1,    90,    91,    92,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,   102,   103,    -1,   105,   106,
      -1,   108,    -1,   110,   111,   112,    -1,    -1,   115,   116,
     117,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,    -1,    -1,    -1,   135,    -1,
      -1,   138,   139,    -1,   141,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    52,    53,    -1,    55,    56,    -1,
      -1,    -1,    60,    -1,    -1,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    85,    86,    -1,
      88,    -1,    90,    91,    92,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,   101,   102,   103,    -1,   105,   106,    -1,
     108,    -1,   110,   111,   112,    -1,    -1,   115,   116,   117,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,    -1,    -1,    -1,   135,    -1,    -1,
     138,    -1,    -1,   141,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    55,    56,    -1,    -1,
      -1,    60,    -1,    -1,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      -1,    90,    91,    92,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,   102,   103,    -1,   105,   106,    -1,   108,
      -1,   110,   111,   112,    -1,    -1,   115,   116,   117,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,    -1,    -1,    -1,   135,    -1,    -1,   138,
      -1,    -1,   141,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    -1,    -1,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    55,    56,    -1,    -1,    -1,
      60,    -1,    -1,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    -1,    88,    -1,
      90,    91,    92,    93,    -1,    95,    -1,    97,    -1,    -1,
     100,   101,   102,   103,    -1,   105,   106,    -1,   108,    -1,
     110,   111,   112,    -1,    -1,   115,   116,   117,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,    -1,
      -1,   131,    -1,    -1,    -1,   135,    -1,    -1,   138,    16,
      -1,   141,    -1,    20,    21,    22,    -1,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    43,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,   102,   103,    -1,   105,   106,
      -1,   108,    -1,   110,   111,   112,    -1,    -1,   115,   116,
     117,     3,   119,     5,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,   138,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    -1,    88,    -1,    -1,    91,
      92,    93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,
     102,   103,    -1,   105,   106,     3,   108,    -1,   110,   111,
     112,    -1,    -1,   115,   116,   117,    -1,   119,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,   138,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,
      -1,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    -1,
      68,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    85,    86,    -1,
      88,    -1,    -1,    91,    92,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,   101,   102,   103,    -1,   105,   106,    -1,
     108,    -1,   110,   111,   112,    -1,    -1,   115,   116,   117,
       3,   119,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
     138,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    -1,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    76,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    -1,    -1,    91,    92,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,   102,
     103,    -1,   105,   106,    -1,   108,   109,   110,   111,   112,
      -1,    -1,   115,   116,   117,    -1,   119,     3,    -1,     5,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    85,
      86,    -1,    88,    -1,    -1,    91,    92,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,   101,   102,   103,    -1,   105,
     106,    -1,   108,    -1,   110,   111,   112,    -1,    -1,   115,
     116,   117,    -1,   119,     3,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    43,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    63,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      -1,    -1,    91,    92,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,   102,   103,    -1,   105,   106,    -1,   108,
      -1,   110,   111,   112,     3,     4,   115,   116,   117,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    43,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    63,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      -1,    -1,    91,    92,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,   102,   103,    -1,   105,   106,    -1,   108,
      -1,   110,   111,   112,    -1,    -1,   115,   116,   117,     3,
     119,     5,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    49,    50,    51,    52,    53,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,
      -1,    75,    76,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    85,    86,    72,    88,    -1,    -1,    91,    92,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,   101,   102,   103,
      -1,   105,   106,     3,   108,    -1,   110,   111,   112,    -1,
      10,   115,   116,   117,    -1,   119,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    28,    29,
      -1,    -1,    -1,    33,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,    46,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    -1,    88,    -1,
      -1,    91,    92,    93,    -1,    95,    -1,    97,    -1,    -1,
     100,   101,   102,   103,    -1,   105,   106,    -1,    -1,    -1,
     110,   111,   112,    -1,    -1,   115,   116,   117,     3,   119,
       5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    -1,    -1,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    -1,    -1,    91,    92,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,   102,   103,    -1,
     105,   106,     3,   108,     5,   110,   111,   112,    -1,    -1,
     115,   116,   117,    -1,   119,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    -1,    -1,
      91,    92,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
     101,   102,   103,    -1,   105,   106,     3,   108,    -1,   110,
     111,   112,    -1,    -1,   115,   116,   117,    -1,   119,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,   102,   103,    -1,   105,   106,
       3,   108,    -1,   110,   111,   112,    -1,    -1,   115,   116,
     117,    -1,   119,    16,    -1,    18,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    -1,    -1,    91,    92,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,   102,
     103,    -1,   105,   106,    -1,   108,     3,   110,   111,   112,
      -1,     8,   115,   116,   117,    -1,   119,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      -1,    88,    -1,    -1,    91,    92,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,   102,   103,    -1,   105,   106,
       3,   108,    -1,   110,   111,   112,    -1,    -1,   115,   116,
     117,    -1,   119,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    -1,    88,    -1,    -1,    91,    92,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,   102,
     103,    -1,   105,   106,     3,   108,    -1,   110,   111,   112,
      -1,    -1,   115,   116,   117,    -1,   119,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    -1,    88,
      -1,    -1,    91,    92,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,   102,   103,    -1,   105,   106,     3,   108,
      -1,   110,   111,   112,    -1,    -1,   115,   116,   117,    -1,
     119,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    -1,    -1,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    -1,    88,    -1,    -1,    91,    92,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,   102,   103,    -1,
     105,   106,     3,   108,    -1,   110,   111,   112,     9,    -1,
     115,   116,   117,    -1,   119,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,
      51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    63,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    85,    86,    -1,    88,    -1,    -1,
      91,    92,    93,    72,    95,    -1,    97,    -1,    -1,   100,
     101,   102,   103,    40,   105,   106,    -1,   108,    -1,   110,
     111,   112,    -1,    -1,   115,   116,   117,    -1,   119,    -1,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,    72,    40,    -1,    -1,   139,
     140,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,
      -1,   140,    -1,    -1,    -1,    -1,    40,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,    72,    -1,
      -1,    -1,    -1,   140,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,    -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,    16,    -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
      35,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    80,    -1,    82,    83,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    97,    -1,    -1,    40,   101,    -1,    -1,    -1,
     105,   106,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,   116,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,    72,    -1,    -1,
      -1,   139,    -1,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,    -1,    -1,
      -1,    -1,   139,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,    40,    -1,
      -1,    -1,   139,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
      40,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,    -1,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,    72,    -1,    -1,    -1,   139,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,   139,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,    40,    -1,    -1,    -1,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    31,    34,    36,    37,    38,
      42,    46,    58,    79,    89,    92,    94,    96,    98,   100,
     110,   113,   147,   148,   151,   152,   153,   154,   161,   202,
     205,   206,   211,   212,   213,   214,   225,   226,   235,   236,
     237,   238,   239,   243,   244,   249,   250,   251,   254,   256,
     257,   258,   259,   260,   262,   263,   264,   265,   266,    93,
     104,    57,     3,    16,    20,    21,    22,    25,    29,    33,
      46,    49,    50,    51,    52,    53,    63,    64,    65,    66,
      68,    69,    70,    75,    80,    81,    82,    83,    84,    85,
      86,    88,    91,    92,    93,    95,    97,   100,   101,   102,
     103,   105,   106,   108,   110,   111,   112,   115,   116,   117,
     119,   149,   150,    49,    81,    48,    49,    81,    85,    93,
      57,     3,     4,     5,     6,     8,     9,    10,    14,    21,
      24,    33,    41,    45,    52,    53,    55,    56,    60,    69,
      71,    90,   103,   117,   131,   132,   135,   138,   141,   150,
     162,   163,   165,   195,   196,   261,   267,   270,   271,    10,
      28,    50,    57,    75,    97,   149,   252,    16,    28,    29,
      35,    50,    57,    70,    80,    82,    83,    97,   101,   105,
     106,   116,   204,   252,   108,    93,   150,   166,     0,    43,
     137,   277,    61,   150,   150,   150,   150,   138,   150,   150,
     166,   150,   150,   150,   150,   150,   138,   143,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,     3,    69,    71,   195,   195,   195,    56,   150,
     199,   200,    13,    14,   144,   268,   269,    48,   140,    18,
     150,   164,    40,    72,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,    67,
     187,   188,    62,   124,   124,    96,     9,   149,   150,     5,
       6,     8,    76,   150,   209,   210,   124,   108,     8,   101,
     150,    96,    66,   203,   150,   203,   203,   203,    77,   189,
     190,   115,   150,    96,   140,   195,   274,   275,   276,    94,
      98,   152,   150,   203,    87,    15,    42,   107,     5,     6,
       8,    14,   131,   138,   174,   175,   221,   227,   228,    91,
     111,   118,   168,   111,     8,   138,   139,   195,   197,   198,
     150,   195,   195,   197,    39,   132,   197,   197,   139,   195,
     197,   197,   195,   195,   195,   195,   139,   138,   138,   138,
     139,   140,   142,   124,     8,   195,   269,   138,   166,   163,
     195,   150,   195,   195,   195,   195,   195,   195,   195,   271,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
       5,    76,   135,   195,   209,   209,   124,   124,    50,   131,
       8,    44,    76,   109,   131,   150,   174,   207,   208,    63,
     101,   203,   101,     8,   101,   150,   191,   192,    66,   118,
     245,   246,   150,   240,   241,   267,   150,   164,    26,   140,
     272,   273,   138,   215,    30,    30,    93,     5,     6,     8,
     139,   174,   176,   229,   140,   230,    24,    45,    59,   102,
     255,     8,     4,    24,    33,    41,    43,    52,    55,    60,
      68,   117,   138,   150,   169,   170,   171,   172,   173,   267,
       8,    94,   154,    47,   139,   140,   139,   139,   150,   139,
     139,   139,   139,   139,   139,   139,   140,   139,   140,   140,
     139,   195,   195,   200,   150,   174,   201,   145,   145,   158,
     167,   168,   140,    76,   138,     5,   207,     9,   210,    65,
     203,   203,   124,   140,     8,   150,   247,   248,   124,   140,
     168,   124,   274,    78,   183,   184,   276,    55,   150,   216,
     217,   114,   150,   150,   150,   139,   140,   139,   140,   221,
     228,   231,   232,   139,    99,    99,   138,   138,   138,   138,
     138,   138,   138,   138,   169,   121,    23,    56,    62,   124,
     125,   126,   127,   128,   129,   135,   139,   150,   198,   139,
     195,   195,   195,   124,    94,   161,    51,   177,     5,   176,
     124,    86,    88,    95,   253,     5,     8,   138,   150,   192,
     124,   120,   138,   174,   175,   241,   189,   174,   175,    26,
     139,   140,   138,   218,   219,    24,    25,    45,    60,    64,
      73,    74,   102,   242,   174,     8,    18,   233,   140,     8,
       8,   150,   267,   132,   267,   139,   139,   267,     8,   139,
     139,   169,   174,   175,     9,   138,    76,   135,     8,   174,
     175,     8,   174,   175,   174,   175,   174,   175,   174,   175,
     174,   175,    56,   140,   155,    56,   139,   139,   140,   201,
     162,   139,     5,   178,   119,   181,   182,   139,   138,    32,
     112,    86,   150,   193,   194,   138,     8,   248,   139,   176,
      84,   173,   185,   186,   216,   138,   220,   221,    77,   140,
     222,    67,   150,   234,   221,   232,   139,   139,   139,   139,
     139,   121,   121,   176,    76,     9,   138,     5,    55,   150,
     156,   157,   139,   267,   138,    48,    78,   159,    26,    51,
      54,   180,   176,   124,   139,   140,     8,   139,   138,    19,
      37,   140,   139,   140,     3,   223,   224,   219,   174,   175,
     174,   175,   139,   176,   140,   139,   197,    26,    67,   160,
     173,   179,    78,   171,   183,   139,   174,   194,   139,   139,
     186,   221,   124,   140,   139,   157,   139,   185,     5,   140,
      26,   187,     8,   224,   140,   140,   173,   185,   189,   138,
       5,   197,   139,   139
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
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 371:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 372:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 379:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 380:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 381:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 389:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 390:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 391:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 392:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 393:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 394:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 395:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 396:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 397:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 398:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 401:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 402:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 403:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 404:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 405:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 406:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 408:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 409:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 410:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 411:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 412:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 413:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 414:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 418:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 420:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 423:

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

