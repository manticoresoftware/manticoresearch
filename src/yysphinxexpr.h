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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_CONST_INT = 258,
     TOK_CONST_FLOAT = 259,
     TOK_ATTR_INT = 260,
     TOK_ATTR_BITS = 261,
     TOK_ATTR_FLOAT = 262,
     TOK_ATTR_MVA = 263,
     TOK_ATTR_STRING = 264,
     TOK_FUNC = 265,
     TOK_FUNC_IN = 266,
     TOK_USERVAR = 267,
     TOK_UDF = 268,
     TOK_ATID = 269,
     TOK_ATWEIGHT = 270,
     TOK_ID = 271,
     TOK_WEIGHT = 272,
     TOK_COUNT = 273,
     TOK_DISTINCT = 274,
     TOK_CONST_LIST = 275,
     TOK_ATTR_SINT = 276,
     TOK_OR = 277,
     TOK_AND = 278,
     TOK_NE = 279,
     TOK_EQ = 280,
     TOK_GTE = 281,
     TOK_LTE = 282,
     TOK_MOD = 283,
     TOK_DIV = 284,
     TOK_NOT = 285,
     TOK_NEG = 286
   };
#endif
#define TOK_CONST_INT 258
#define TOK_CONST_FLOAT 259
#define TOK_ATTR_INT 260
#define TOK_ATTR_BITS 261
#define TOK_ATTR_FLOAT 262
#define TOK_ATTR_MVA 263
#define TOK_ATTR_STRING 264
#define TOK_FUNC 265
#define TOK_FUNC_IN 266
#define TOK_USERVAR 267
#define TOK_UDF 268
#define TOK_ATID 269
#define TOK_ATWEIGHT 270
#define TOK_ID 271
#define TOK_WEIGHT 272
#define TOK_COUNT 273
#define TOK_DISTINCT 274
#define TOK_CONST_LIST 275
#define TOK_ATTR_SINT 276
#define TOK_OR 277
#define TOK_AND 278
#define TOK_NE 279
#define TOK_EQ 280
#define TOK_GTE 281
#define TOK_LTE 282
#define TOK_MOD 283
#define TOK_DIV 284
#define TOK_NOT 285
#define TOK_NEG 286




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
	int64_t			iConst;			// constant value
	float			fConst;			// constant value
	uint64_t		iAttrLocator;	// attribute locator (rowitem for int/float; offset+size for bits)
	int				iFunc;			// function id
	int				iNode;			// node, or uservar, or udf index
} YYSTYPE;
/* Line 1204 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





