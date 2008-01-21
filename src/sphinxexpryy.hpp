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
     TOK_NUMBER = 258,
     TOK_ATTR_INT = 259,
     TOK_ATTR_BITS = 260,
     TOK_ATTR_FLOAT = 261,
     TOK_FUNC = 262,
     TOK_DOCINFO = 263,
     TOK_GTE = 264,
     TOK_LTE = 265,
     TOK_NEG = 266
   };
#endif
#define TOK_NUMBER 258
#define TOK_ATTR_INT 259
#define TOK_ATTR_BITS 260
#define TOK_ATTR_FLOAT 261
#define TOK_FUNC 262
#define TOK_DOCINFO 263
#define TOK_GTE 264
#define TOK_LTE 265
#define TOK_NEG 266




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
	float			fNumber;		// constant value
	int				iAttrLocator;	// attribute locator (rowitem for int/float; offset+size for bits)
	int				iFunc;			// function id
	Docinfo_e		eDocinfo;		// docinfo entry id
	int				iNode;			// node index
} YYSTYPE;
/* Line 1248 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





