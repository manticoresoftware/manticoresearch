//
// Copyright (c) 2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

class XQParser_t;
#include "bissphinxquery.h"

class XQParser_t : public XQParseHelper_c
{
	friend void yyerror ( XQParser_t * pParser, const char * sMessage );

	friend int yyparse (XQParser_t * pParser);

public:
					XQParser_t();
					~XQParser_t() override;

public:
	bool			Parse ( XQQuery_t & tQuery, const char * sQuery, const CSphQuery * pQuery, const TokenizerRefPtr_c & pTokenizer, const CSphSchema * pSchema, const DictRefPtr_c & pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields );
	int				ParseZone ( const char * pZone );

	bool			IsSpecial ( char c );
	bool			GetNumber ( const char * p, const char * sRestart );
	int				GetToken ( YYSTYPE * lvalp );

	void			HandleModifiers ( XQKeyword_t & tKeyword ) const noexcept;

	void			AddQuery ( XQNode_t * pNode );
	XQNode_t *		AddKeyword ( const char * sKeyword, int iSkippedPosBeforeToken=0 );
	XQNode_t *		AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight );
	XQNode_t *		AddOp ( XQOperator_e eOp, XQNode_t * pLeft, XQNode_t * pRight, int iOpArg=0 );
	void			SetPhrase ( XQNode_t * pNode, bool bSetExact );
	void			PhraseShiftQpos ( XQNode_t * pNode );

	void	Cleanup () override;

	void SetFieldSpec ( const FieldMask_t& uMask, int iMaxPos )
	{
		FixRefSpec();
		m_dStateSpec.Last()->SetFieldSpec ( uMask, iMaxPos );
	}
	void SetZoneVec ( int iZoneVec, bool bZoneSpan = false )
	{
		FixRefSpec();
		m_dStateSpec.Last()->SetZoneSpec ( m_dZoneVecs[iZoneVec], bZoneSpan );
	}

	void FixRefSpec ()
	{
		bool bRef = ( m_dStateSpec.GetLength()>1 && ( m_dStateSpec[m_dStateSpec.GetLength()-1]==m_dStateSpec[m_dStateSpec.GetLength()-2] ) );
		if ( !bRef )
			return;

		XQLimitSpec_t * pSpec = m_dStateSpec.Pop();
		m_dSpecPool.Add ( new XQLimitSpec_t ( *pSpec ) );
		m_dStateSpec.Add ( m_dSpecPool.Last() );
	}

public:
	const CSphVector<int> & GetZoneVec ( int iZoneVec ) const
	{
		return m_dZoneVecs[iZoneVec];
	}

public:
	BYTE *					m_sQuery = nullptr;
	int						m_iQueryLen = 0;
	const char *			m_pErrorAt = nullptr;

	XQNode_t *				m_pRoot = nullptr;

	int						m_iPendingNulls = 0;
	int						m_iPendingType = 0;
	YYSTYPE					m_tPendingToken;
	bool					m_bWasKeyword = false;

	bool					m_bEmpty = false;
	bool					m_bQuoted = false;
	int						m_iOvershortStep = 0;

	int						m_iQuorumQuote = -1;
	int						m_iQuorumFSlash = -1;
	bool					m_bCheckNumber = false;

	StrVec_t				m_dIntTokens;

	CSphVector < CSphVector<int> >	m_dZoneVecs;
	CSphVector<XQLimitSpec_t *>		m_dStateSpec;
	CSphVector<XQLimitSpec_t *>		m_dSpecPool;
	IntVec_t						m_dPhraseStar;

protected:
	bool			HandleFieldBlockStart ( const char * & pPtr ) override;

private:
	XQNode_t *		ParseRegex ( const char * pStart );
};