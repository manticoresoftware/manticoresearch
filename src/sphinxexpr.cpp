//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxexpr.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////

#ifndef M_LOG2E
#define M_LOG2E    1.44269504088896340736
#endif

#ifndef M_LOG10E
#define M_LOG10E   0.434294481903251827651
#endif

//////////////////////////////////////////////////////////////////////////
// PARSER INTERNALS
//////////////////////////////////////////////////////////////////////////

/// known docinfo entries list
enum Docinfo_e
{
	DI_ID,
	DI_WEIGHT
};

#include "sphinxexpryy.hpp"

//////////////////////////////////////////////////////////////////////////

/// VM opcodes
enum Opcode_e
{
	OPCODE_STOP,
	OPCODE_PUSH_ATTR_INT,
	OPCODE_PUSH_ATTR_BITS,
	OPCODE_PUSH_ATTR_FLOAT,
	OPCODE_PUSH_CONST,
	OPCODE_PUSH_ID,
	OPCODE_PUSH_WEIGHT,

	OPCODE_ADD,
	OPCODE_SUB,
	OPCODE_MUL,
	OPCODE_DIV,
	OPCODE_LT,
	OPCODE_GT,
	OPCODE_LTE,
	OPCODE_GTE,
	OPCODE_EQ,
	OPCODE_NE,

	OPCODE_NEG,
	OPCODE_ABS,
	OPCODE_CEIL,
	OPCODE_FLOOR,
	OPCODE_SIN,
	OPCODE_COS,
	OPCODE_LN,
	OPCODE_LOG2,
	OPCODE_LOG10,
	OPCODE_EXP,
	OPCODE_SQRT,

	OPCODE_MIN,
	OPCODE_MAX,
	OPCODE_POW,

	OPCODE_IF,
};

/// known functions
struct FuncDesc_t
{
	const char *	m_sName;
	int				m_iArgs;
	Opcode_e		m_eOpcode;
};
static FuncDesc_t g_dFuncs[] =
{
	{ "abs",	1,	OPCODE_ABS },
	{ "ceil",	1,	OPCODE_CEIL },
	{ "floor",	1,	OPCODE_FLOOR },
	{ "sin",	1,	OPCODE_SIN },
	{ "cos",	1,	OPCODE_COS },
	{ "ln",		1,	OPCODE_LN },
	{ "log2",	1,	OPCODE_LOG2 },
	{ "log10",	1,	OPCODE_LOG10 },
	{ "exp",	1,	OPCODE_EXP },
	{ "sqrt",	1,	OPCODE_SQRT },

	{ "min",	2,	OPCODE_MIN },
	{ "max",	2,	OPCODE_MAX },
	{ "pow",	2,	OPCODE_POW },

	{ "if",		3,	OPCODE_IF },
};

//////////////////////////////////////////////////////////////////////////

/// expression tree node
struct ExprNode_t
{
	int				m_iToken;	///< token type, including operators
	union
	{

		int			m_iAttrLocator;	///< attribute locator, for TOK_ATTR type
		float		m_fConst;		///< constant value, for TOK_CONST type
		int			m_iFunc;		///< built-in function id, for TOK_FUNC type
		Docinfo_e	m_eDocinfo;		///< docinfo field id, for TOK_DOCINFO type
		int			m_iArgs;		///< args count, for arglist (token==',') type
	};
	int				m_iLeft;
	int				m_iRight;

	ExprNode_t () : m_iToken ( 0 ), m_iLeft ( -1 ), m_iRight ( -1 ) {}
};

/// expression parser
class ExprParser_t
{
	friend int				yylex ( YYSTYPE * lvalp, ExprParser_t * pParser );
	friend int				yyparse ( ExprParser_t * pParser );
	friend void				yyerror ( ExprParser_t * pParser, char * sMessage );

public:	
							ExprParser_t () {}
							~ExprParser_t () {}

	bool					Parse ( const char * sExpr, const CSphSchema & tSchema, CSphExpr & tOutExpr, CSphString & sError );

protected:
	int						m_iParsed;	///< filled by yyparse() at the very end
	CSphString				m_sLexerError;
	CSphString				m_sParserError;

protected:
	int						AddNodeNumber ( float fValue );
	int						AddNodeAttr ( int iTokenType, int iAttrLocator );
	int						AddNodeDocinfo ( Docinfo_e eDocinfo );
	int						AddNodeOp ( int iOp, int iLeft, int iRight );
	int						AddNodeFunc ( int iFunc, int iLeft );

private:
	const char *			m_sExpr;
	const char *			m_pCur;
	const char *			m_pLastTokenStart;
	const CSphSchema *		m_pSchema;
	CSphVector<ExprNode_t>	m_dNodes;

private:
	int						GetToken ( YYSTYPE * lvalp );
	void					FoldTree ( int iNode, CSphExpr & tOutExpr );
};

//////////////////////////////////////////////////////////////////////////

/// let's build our own theme park!
static inline int sphIsAttr ( int c )
{
	// different from sphIsAlpha() in that we don't allow minus
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_';
}

/// a lexer of my own
/// returns token id and fills lvalp on success
/// returns -1 and fills sError on failure
int ExprParser_t::GetToken ( YYSTYPE * lvalp )
{
	// skip whitespace, check eof
	while ( isspace(*m_pCur) ) m_pCur++;
	m_pLastTokenStart = m_pCur;
	if ( !*m_pCur ) return 0;

	// check for constant
	if ( isdigit(*m_pCur) )
	{
		char * pEnd = NULL;
		lvalp->fNumber = (float)strtod ( m_pCur, &pEnd );
		m_pCur = pEnd;
		return TOK_NUMBER;
	}

	// check for magic names
	if ( *m_pCur=='@' && sphIsAttr(m_pCur[1]) && !isdigit(m_pCur[1]) )
	{
		// get token
		const char * pStart = ++m_pCur;
		while ( sphIsAttr(*m_pCur) ) m_pCur++;

		CSphString sTok;
		sTok.SetBinary ( pStart, m_pCur-pStart );
		sTok.ToLower ();

		if ( sTok=="id" )		{ lvalp->eDocinfo = DI_ID; return TOK_DOCINFO; }
		if ( sTok=="weight" )	{ lvalp->eDocinfo = DI_WEIGHT; return TOK_DOCINFO; }

		m_sLexerError.SetSprintf ( "unknown magic name '@%s'", sTok.cstr() );
		return -1;
	}

	// check for field or function name
	if ( sphIsAttr(*m_pCur) )
	{
		// get token
		const char * pStart = m_pCur;
		assert ( !isdigit(*m_pCur) ); // can't start with a digit, right?

		while ( sphIsAttr(*m_pCur) ) m_pCur++;
		assert ( !sphIsAttr(*m_pCur) );

		CSphString sTok;
		sTok.SetBinary ( pStart, m_pCur-pStart );

		// check for attribute
		int iAttr = m_pSchema->GetAttrIndex ( sTok.cstr() );
		if ( iAttr>=0 )
		{
			// check attribute type and width
			const CSphColumnInfo & tCol = m_pSchema->GetAttr ( iAttr );
			if ( tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP || tCol.m_eAttrType==SPH_ATTR_BOOL )
			{
				if ( tCol.m_iRowitem>=0 )
				{
					lvalp->iAttrLocator = tCol.m_iRowitem;
					return TOK_ATTR_INT;
				} else
				{
					DWORD uRowitem = tCol.m_iBitOffset / ROWITEM_BITS;
					DWORD uItemOff = tCol.m_iBitOffset % ROWITEM_BITS;

					assert ( uRowitem<=65535 );
					assert ( uItemOff<=255 );
					assert ( tCol.m_iBitCount<=255 );
					lvalp->iAttrLocator = ( uRowitem<<16 ) + ( uItemOff<<8 ) + tCol.m_iBitCount;
					return TOK_ATTR_BITS;
				}
			} else if ( tCol.m_eAttrType==SPH_ATTR_FLOAT )
			{
				lvalp->iAttrLocator = tCol.m_iRowitem;
				return TOK_ATTR_FLOAT;
			} else
			{
				if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
					m_sLexerError.SetSprintf ( "attribute '%s' is MVA, can not be used in expressions", sTok.cstr() );
				else
					m_sLexerError.SetSprintf ( "attribute '%s' is of unsupported type (type=%d)", sTok.cstr(), tCol.m_eAttrType );
				return -1;
			}
		}

		// check for function
		sTok.ToLower();
		for ( int i=0; i<sizeof(g_dFuncs)/sizeof(g_dFuncs[0]); i++ )
			if ( sTok==g_dFuncs[i].m_sName )
		{
			lvalp->iFunc = i;
			return TOK_FUNC;
		}

		m_sLexerError.SetSprintf ( "unknown identifier '%s' (not an attribute, not a function)", sTok.cstr() );
		return -1;
	}

	// check for known operators, then
	switch ( *m_pCur )
	{
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
		case ')':
		case ',':
			return *m_pCur++;

		case '<':
			m_pCur++;
			if ( *m_pCur=='>' ) { m_pCur++; return TOK_NE; }
			if ( *m_pCur=='=' ) { m_pCur++; return TOK_LTE; }
			return '<';

		case '>':
			m_pCur++;
			if ( *m_pCur=='=' ) { m_pCur++; return TOK_GTE; }
			return '>';

		case '=':
			m_pCur++;
			if ( *m_pCur=='=' ) m_pCur++;
			return TOK_EQ;
	}

	m_sLexerError.SetSprintf ( "unknown operator '%c' near '%s'", *m_pCur, m_pCur );
	return -1;
}

/// fold nodes subtree into opcodes
void ExprParser_t::FoldTree ( int iNode, CSphExpr & tOutExpr )
{
	if ( iNode<0 )
		return;

	const ExprNode_t & tNode = m_dNodes[iNode];
	FoldTree ( tNode.m_iLeft, tOutExpr );
	FoldTree ( tNode.m_iRight, tOutExpr );

	switch ( tNode.m_iToken )
	{
		case TOK_ATTR_INT:
			tOutExpr.Add ( OPCODE_PUSH_ATTR_INT );
			tOutExpr.Add ( tNode.m_iAttrLocator );
			break;

		case TOK_ATTR_BITS:
			tOutExpr.Add ( OPCODE_PUSH_ATTR_BITS );
			tOutExpr.Add ( tNode.m_iAttrLocator );
			break;

		case TOK_ATTR_FLOAT:
			tOutExpr.Add ( OPCODE_PUSH_ATTR_FLOAT );
			tOutExpr.Add ( tNode.m_iAttrLocator );
			break;

		case TOK_NUMBER:
			tOutExpr.Add ( OPCODE_PUSH_CONST );
			tOutExpr.Add ( sphF2DW(tNode.m_fConst) );
			break;

		case '+':		tOutExpr.Add ( OPCODE_ADD ); break;
		case '-':		tOutExpr.Add ( OPCODE_SUB ); break;
		case '*':		tOutExpr.Add ( OPCODE_MUL ); break;
		case '/':		tOutExpr.Add ( OPCODE_DIV ); break;
		case '<':		tOutExpr.Add ( OPCODE_LT ); break;
		case '>':		tOutExpr.Add ( OPCODE_GT ); break;
		case TOK_LTE:	tOutExpr.Add ( OPCODE_LTE ); break;
		case TOK_GTE:	tOutExpr.Add ( OPCODE_GTE ); break;
		case TOK_EQ:	tOutExpr.Add ( OPCODE_EQ ); break;
		case TOK_NE:	tOutExpr.Add ( OPCODE_NE ); break;
		case ',':		break;
		case TOK_NEG:	tOutExpr.Add ( OPCODE_NEG ); break;

		case TOK_FUNC:
			assert ( tNode.m_iFunc>=0 && tNode.m_iFunc<sizeof(g_dFuncs)/sizeof(g_dFuncs[0]) );
			tOutExpr.Add ( g_dFuncs[tNode.m_iFunc].m_eOpcode );
			break;

		case TOK_DOCINFO:
			switch ( tNode.m_eDocinfo )
			{
				case DI_ID:		tOutExpr.Add ( OPCODE_PUSH_ID ); break;
				case DI_WEIGHT:	tOutExpr.Add ( OPCODE_PUSH_WEIGHT ); break;
				default:		assert ( 0 && "unhandled docinfo element id" ); break;
			}
			break;

		default:
			assert ( 0 && "unhandled token type" );
			break;
	}
}

//////////////////////////////////////////////////////////////////////////

int yylex ( YYSTYPE * lvalp, ExprParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

void yyerror ( ExprParser_t * pParser, char * sMessage )
{
	pParser->m_sParserError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart );
}

#if USE_WINDOWS
#pragma warning(push,1)
#endif

#include "sphinxexpryy.cpp"

#if USE_WINDOWS
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////

int ExprParser_t::AddNodeNumber ( float fValue )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_NUMBER;
	tNode.m_fConst = fValue;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeAttr ( int iTokenType, int iAttrLocator )
{
	assert ( iTokenType==TOK_ATTR_INT || iTokenType==TOK_ATTR_BITS || iTokenType==TOK_ATTR_FLOAT );
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iTokenType;
	tNode.m_iAttrLocator = iAttrLocator;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeDocinfo ( Docinfo_e eDocinfo )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_DOCINFO;
	tNode.m_eDocinfo  = eDocinfo;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeOp ( int iOp, int iLeft, int iRight )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iOp;
	tNode.m_iArgs = 0;
	if ( iOp==',' )
	{
		if ( iLeft>=0 )		tNode.m_iArgs += ( m_dNodes[iLeft].m_iToken==',' ) ? m_dNodes[iLeft].m_iArgs : 1;
		if ( iRight>=0 )	tNode.m_iArgs += ( m_dNodes[iRight].m_iToken==',' ) ? m_dNodes[iRight].m_iArgs : 1;
	}
	tNode.m_iLeft = iLeft;
	tNode.m_iRight = iRight;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeFunc ( int iFunc, int iArgsNode )
{
	// check args count
	assert ( iFunc>=0 && iFunc<sizeof(g_dFuncs)/sizeof(g_dFuncs[0]) );
	int iExpectedArgc = g_dFuncs[iFunc].m_iArgs;
	int iArgc = ( m_dNodes[iArgsNode].m_iToken==',' ) ? m_dNodes[iArgsNode].m_iArgs : 1;
	if ( iArgc!=iExpectedArgc )
	{
		m_sParserError.SetSprintf ( "%s() called with %d args, %d args expected", g_dFuncs[iFunc].m_sName, iArgc, iExpectedArgc );
		return -1;
	}

	// do add
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = iArgsNode;
	return m_dNodes.GetLength()-1;
}

bool ExprParser_t::Parse ( const char * sExpr, const CSphSchema & tSchema, CSphExpr & tOutExpr, CSphString & sError )
{
	// setup lexer
	m_sExpr = sExpr;
	m_pCur = sExpr;
	m_pSchema = &tSchema;

	// build tree
	m_iParsed = -1;
	yyparse ( this );

	// handle errors
	if ( m_iParsed<0 || !m_sLexerError.IsEmpty() || !m_sParserError.IsEmpty() )
	{
		sError = !m_sLexerError.IsEmpty() ? m_sLexerError : m_sParserError;
		if ( sError.IsEmpty() ) sError = "general parsing error";
		return false;
	}

	// fold the tree to opcodes
	tOutExpr.Reset ();
	tOutExpr.Reserve ( 128 );

	FoldTree ( m_iParsed, tOutExpr );
	if ( !tOutExpr.GetLength() )
	{
		sError.SetSprintf ( "empty expression" );
		return false;
	}

	tOutExpr.Add ( OPCODE_STOP );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC STUFF
//////////////////////////////////////////////////////////////////////////

/// max evaluation stack size
static const int STACK_SIZE = 64;

/// expression evaluation
float CSphExpr::Eval ( const CSphMatch & tMatch ) const
{
	// safety check
	if ( !m_iLength )
		return 0.0f;

	// do evaluation
	float dStack[STACK_SIZE];
	int iTop = -1; // used==top+1

	const DWORD * pOpcodes = m_pData;
	for ( ;; ) switch ( *pOpcodes++ )
	{
		case OPCODE_STOP:			assert ( iTop==0 ); return dStack[0];
		case OPCODE_PUSH_ATTR_INT:	assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = (float)tMatch.m_pRowitems[*pOpcodes++]; break;
		case OPCODE_PUSH_ATTR_BITS:
			{
				assert ( iTop<STACK_SIZE-1 );
				DWORD uLoc = *pOpcodes++;
				dStack[++iTop] = (float)( ( tMatch.m_pRowitems[uLoc>>16] >> ((uLoc>>8)&255) ) // val = rowitems[item] >> shift
					& ( (1UL<<(uLoc&255)) - 1 ) ); // mask = (1<<bitcount)-1
				break;
			}
		case OPCODE_PUSH_ATTR_FLOAT:assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = sphDW2F(tMatch.m_pRowitems[*pOpcodes++]); break;
		case OPCODE_PUSH_CONST:		assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = sphDW2F(*pOpcodes++); break;
		case OPCODE_PUSH_ID:		assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = (float)tMatch.m_iDocID; break;
		case OPCODE_PUSH_WEIGHT:	assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = (float)tMatch.m_iWeight; break;

		case OPCODE_ADD:		assert ( iTop>=1 ); dStack[iTop-1] += dStack[iTop]; iTop--; break;
		case OPCODE_SUB:		assert ( iTop>=1 ); dStack[iTop-1] -= dStack[iTop]; iTop--; break;
		case OPCODE_MUL:		assert ( iTop>=1 ); dStack[iTop-1] *= dStack[iTop]; iTop--; break;
		case OPCODE_DIV:		assert ( iTop>=1 ); dStack[iTop-1] /= dStack[iTop]; iTop--; break;
		case OPCODE_LT:			assert ( iTop>=1 ); dStack[iTop-1] = ( dStack[iTop-1] < dStack[iTop] ) ? 1.0f : 0.0f; iTop--; break;
		case OPCODE_GT:			assert ( iTop>=1 ); dStack[iTop-1] = ( dStack[iTop-1] > dStack[iTop] ) ? 1.0f : 0.0f; iTop--; break;
		case OPCODE_LTE:		assert ( iTop>=1 ); dStack[iTop-1] = ( dStack[iTop-1] <= dStack[iTop] ) ? 1.0f : 0.0f; iTop--; break;
		case OPCODE_GTE:		assert ( iTop>=1 ); dStack[iTop-1] = ( dStack[iTop-1] >= dStack[iTop] ) ? 1.0f : 0.0f; iTop--; break;
		case OPCODE_EQ:			assert ( iTop>=1 ); dStack[iTop-1] = fabs ( dStack[iTop-1]-dStack[iTop] ) <= 1e-6 ? 1.0f : 0.0f; iTop--; break;
		case OPCODE_NE:			assert ( iTop>=1 ); dStack[iTop-1] = fabs ( dStack[iTop-1]-dStack[iTop] ) > 1e-6 ? 1.0f : 0.0f; iTop--; break;

		case OPCODE_NEG:		assert ( iTop>=0 ); dStack[iTop] = -dStack[iTop]; break;
		case OPCODE_ABS:		assert ( iTop>=0 ); if ( dStack[iTop]<0.0f ) dStack[iTop] = -dStack[iTop]; break;
		case OPCODE_CEIL:		assert ( iTop>=0 ); dStack[iTop] = (float)ceil ( dStack[iTop] ); break;
		case OPCODE_FLOOR:		assert ( iTop>=0 ); dStack[iTop] = (float)floor ( dStack[iTop] ); break;
		case OPCODE_SIN:		assert ( iTop>=0 ); dStack[iTop] = (float)sin ( dStack[iTop] ); break;
		case OPCODE_COS:		assert ( iTop>=0 ); dStack[iTop] = (float)cos ( dStack[iTop] ); break;
		case OPCODE_LN:			assert ( iTop>=0 ); dStack[iTop] = (float)log ( dStack[iTop] ); break;
		case OPCODE_LOG2:		assert ( iTop>=0 ); dStack[iTop] = (float)( log ( dStack[iTop] ) * M_LOG2E ); break;
		case OPCODE_LOG10:		assert ( iTop>=0 ); dStack[iTop] = (float)( log ( dStack[iTop] ) * M_LOG10E ); break;
		case OPCODE_EXP:		assert ( iTop>=0 ); dStack[iTop] = (float)exp ( dStack[iTop] ); break;
		case OPCODE_SQRT:		assert ( iTop>=0 ); dStack[iTop] = (float)sqrt ( dStack[iTop] ); break;

		case OPCODE_MIN:		assert ( iTop>=1 ); dStack[iTop-1] = Min ( dStack[iTop-1], dStack[iTop] ); iTop--; break;
		case OPCODE_MAX:		assert ( iTop>=1 ); dStack[iTop-1] = Max ( dStack[iTop-1], dStack[iTop] ); iTop--; break;
		case OPCODE_POW:		assert ( iTop>=1 ); dStack[iTop-1] = (float)pow ( dStack[iTop-1], dStack[iTop] ); iTop--; break;

		case OPCODE_IF:			assert ( iTop>=2 ); dStack[iTop-2] = ( dStack[iTop-2]!=0.0f ) ? dStack[iTop-1] : dStack[iTop]; iTop-=2; break;

		default:				assert ( 0 && "unhandled opcode" );
	}
}

/// parser entry point
bool sphExprParse ( const char * sExpr, const CSphSchema & tSchema, CSphExpr & tOutExpr, CSphString & sError )
{
	// parse into opcodes
	ExprParser_t tParser;
	if ( !tParser.Parse ( sExpr, tSchema, tOutExpr, sError ) )
		return false;

	// expression sanity checks
	if ( !tOutExpr.GetLength() )		{ sError.SetSprintf ( "empty expression" ); return false; }
	if ( tOutExpr.Last()!=OPCODE_STOP )	{ sError.SetSprintf ( "internal error: unterminated opcodes list" ); return false; }

	// run simulation, check stack usage
	int iTop = -1;
	const DWORD * pOpcodes = &tOutExpr[0];
	for ( ;; ) switch ( *pOpcodes++ )
	{
		// push ops
		case OPCODE_PUSH_ATTR_INT:
		case OPCODE_PUSH_ATTR_BITS:
		case OPCODE_PUSH_ATTR_FLOAT:
		case OPCODE_PUSH_CONST:
			if ( iTop>=STACK_SIZE-1 ) { sError = "internal error: opcodes stack overflow"; return false; }
			pOpcodes++;
			iTop++;
			break;

		case OPCODE_PUSH_ID:
		case OPCODE_PUSH_WEIGHT:
			if ( iTop>=STACK_SIZE-1 ) { sError = "internal error: opcodes stack overflow"; return false; }
			iTop++;
			break;

		// unary ops
		case OPCODE_NEG:
		case OPCODE_ABS:
		case OPCODE_CEIL:
		case OPCODE_FLOOR:
		case OPCODE_SIN:
		case OPCODE_COS:
		case OPCODE_LN:
		case OPCODE_LOG2:
		case OPCODE_LOG10:
		case OPCODE_EXP:
		case OPCODE_SQRT:
			if ( iTop<0 ) { sError = "internal error: no args to unary operation"; return false; }
			break;

		// binary ops
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_MUL:
		case OPCODE_DIV:
		case OPCODE_LT:
		case OPCODE_GT:
		case OPCODE_LTE:
		case OPCODE_GTE:
		case OPCODE_EQ:
		case OPCODE_NE:
		case OPCODE_MIN:
		case OPCODE_MAX:
		case OPCODE_POW:
			if ( iTop<1 ) { sError = "internal error: no args to binary operation"; return false; }
			iTop--;
			break;

		// ternary ops
		case OPCODE_IF:
			if ( iTop<2 ) { sError = "internal error: no args to 3-arg function"; return false; }
			iTop-=2;
			break;

		// stop
		case OPCODE_STOP:
			if ( iTop!=0 ) { sError = "internal error: extra opcodes in stack on STOP"; return false; }
			return true; // clean stop, all ok

		// unhandled
		default:
			sError.SetSprintf ( "internal error: unhandled opcode (code=%d)", pOpcodes[-1] );
			return false;
	}
}

//
// $Id$
//
