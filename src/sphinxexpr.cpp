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

//////////////////////////////////////////////////////////////////////////
// PARSER INTERNALS
//////////////////////////////////////////////////////////////////////////

/// known docinfo entries list
enum Docinfo_e
{
	DI_ID,
	DI_WEIGHT
};

/// expression functions list
enum Func_e
{
	FUNC_ABS,
	FUNC_MIN,
	FUNC_IF,
};

#include "sphinxexpryy.hpp"

//////////////////////////////////////////////////////////////////////////

/// expression tree node
struct ExprNode_t
{
	int				m_iToken;	///< token type, including operators
	union
	{
		int			m_iRowitem;	///< attribute rowitem, for TOK_ATTR type
		float		m_fConst;	///< constant value, for TOK_CONST type
		Func_e		m_eFunc;	///< built-in function id, for TOK_FUNC type
		Docinfo_e	m_eDocinfo;	///< docinfo field id, for TOK_DOCINFO type
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
	int						AddNodeAttr ( int iRowitem );
	int						AddNodeDocinfo ( Docinfo_e eDocinfo );
	int						AddNodeOp ( int iOp, int iLeft, int iRight );
	int						AddNodeFunc ( Func_e eFunc, int iLeft );

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

/// VM opcodes
enum Opcode_e
{
	OPCODE_STOP,
	OPCODE_PUSH_ATTR,
	OPCODE_PUSH_CONST,
	OPCODE_PUSH_ID,
	OPCODE_PUSH_WEIGHT,
	OPCODE_ADD,
	OPCODE_SUB,
	OPCODE_MUL,
	OPCODE_DIV,
	OPCODE_LT,
	OPCODE_GT,
	OPCODE_NEG,
	OPCODE_ABS
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
	if ( *m_pCur=='@' )
	{
		m_pCur++;
		if ( strncasecmp ( m_pCur, "id", 2 )==0		)	{ m_pCur += 2; lvalp->eDocinfo = DI_ID; return TOK_DOCINFO; }
		if ( strncasecmp ( m_pCur, "weight", 6 )==0 )	{ m_pCur += 6; lvalp->eDocinfo = DI_WEIGHT; return TOK_DOCINFO; }
		m_pCur--; // let the check for bad operator fail
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
			if ( tCol.m_eAttrType!=SPH_ATTR_INTEGER )
			{
				m_sLexerError.SetSprintf ( "attribute '%s' is not integer, not supported yet", sTok.cstr() );
				return -1;
			}
			if ( tCol.m_iRowitem<0 )
			{
				m_sLexerError.SetSprintf ( "attribute '%s' is a bitfield, not supported yet", sTok.cstr() );
				return -1;
			}

			// we're done
			lvalp->iRowitem = tCol.m_iRowitem;
			return TOK_ATTR;
		}

		// check for function
		sTok.ToLower();
		if ( sTok=="abs" )		{ lvalp->eFunc = FUNC_ABS; return TOK_FUNC; }
		if ( sTok=="min" )		{ lvalp->eFunc = FUNC_MIN; return TOK_FUNC; }
		if ( sTok=="if" )		{ lvalp->eFunc = FUNC_IF; return TOK_FUNC; }

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
		case '<':
		case '>':
		case ',':
			return *m_pCur++;
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
		case TOK_ATTR:
			tOutExpr.Add ( OPCODE_PUSH_ATTR );
			tOutExpr.Add ( tNode.m_iRowitem );
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
		case OP_UMINUS:	tOutExpr.Add ( OPCODE_NEG ); break;

		case TOK_FUNC:
			switch ( tNode.m_eFunc )
			{
				case FUNC_ABS:	tOutExpr.Add ( OPCODE_ABS ); break;
				default:		assert ( 0 && "unhandled function" ); break;
			}
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

int ExprParser_t::AddNodeAttr ( int iRowitem )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_ATTR;
	tNode.m_iRowitem = iRowitem;
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
	tNode.m_iLeft = iLeft;
	tNode.m_iRight = iRight;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeFunc ( Func_e eFunc, int iArgs )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_eFunc = eFunc;
	tNode.m_iLeft = iArgs;
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
	if ( m_iParsed<0 )
	{
		sError = !m_sLexerError.IsEmpty() ? m_sLexerError : m_sParserError;
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
		case OPCODE_ADD:		assert ( iTop>=1 ); dStack[iTop-1] += dStack[iTop]; iTop--; break;
		case OPCODE_SUB:		assert ( iTop>=1 ); dStack[iTop-1] -= dStack[iTop]; iTop--; break;
		case OPCODE_MUL:		assert ( iTop>=1 ); dStack[iTop-1] *= dStack[iTop]; iTop--; break;
		case OPCODE_DIV:		assert ( iTop>=1 ); dStack[iTop-1] /= dStack[iTop]; iTop--; break;
		case OPCODE_STOP:		assert ( iTop==0 ); return dStack[0];
		case OPCODE_PUSH_ATTR:	assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = (float)tMatch.m_pRowitems[*pOpcodes++]; break;
		case OPCODE_PUSH_CONST:	assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = sphDW2F(*pOpcodes++); break;
		case OPCODE_LT:			assert ( iTop>=1 ); dStack[iTop-1] = ( dStack[iTop-1] < dStack[iTop] ) ? 1.0f : 0.0f; iTop--; break;
		case OPCODE_GT:			assert ( iTop>=1 ); dStack[iTop-1] = ( dStack[iTop-1] > dStack[iTop] ) ? 1.0f : 0.0f; iTop--; break;
		case OPCODE_NEG:		assert ( iTop>=0 ); dStack[iTop] = -dStack[iTop]; break;
		case OPCODE_ABS:		assert ( iTop>=0 ); if ( dStack[iTop]<0.0f ) dStack[iTop] = -dStack[iTop]; break;
		case OPCODE_PUSH_ID:	assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = (float)tMatch.m_iDocID; break;
		case OPCODE_PUSH_WEIGHT:assert ( iTop<STACK_SIZE-1 ); dStack[++iTop] = (float)tMatch.m_iWeight; break;
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
		case OPCODE_PUSH_ATTR:
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
			if ( iTop<0 ) { sError = "internal error: no args to unary operation"; return false; }
			break;

		// binary ops
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_MUL:
		case OPCODE_DIV:
		case OPCODE_LT:
		case OPCODE_GT:
			if ( iTop<1 ) { sError = "internal error: no args to binary operation"; return false; }
			iTop--;
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
