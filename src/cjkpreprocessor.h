//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinx.h"
#include "tokenizer/remap_range.h"

class CJKPreprocessor_c
{
public:
	virtual					~CJKPreprocessor_c() = default;

	virtual bool			Init ( CSphString & sError ) = 0;
	virtual CJKPreprocessor_c * Clone ( const FieldFilterOptions_t * pOptions ) = 0;

	bool					Process ( const BYTE * pBuffer, int iLength, CSphVector<BYTE> & dOut, bool bQuery );
	bool					SetBlendChars ( const char * szBlendChars, CSphString & sError );

protected:
	virtual void			ProcessBuffer ( const BYTE * pBuffer, int iLength ) = 0;
	virtual const BYTE *	GetNextToken ( int & iTokenLen ) = 0;

private:
	CSphVector<CSphRemapRange> m_dBlendChars;

	void					AddTextChunk ( const BYTE * pStart, int iLen, CSphVector<BYTE> & dOut, bool bChinese, bool bQuery );

	FORCE_INLINE bool NeedAddSpace ( const BYTE * pToken, const CSphVector<BYTE> & dOut, bool bQuery ) const
	{
		int iResLen = dOut.GetLength();
		if ( !iResLen )
			return false;

		if ( bQuery && ( IsSpecialQueryCode ( dOut[iResLen - 1] ) || IsSpecialQueryCode ( *pToken ) ) )
			return false;

		if ( IsBlendChar( dOut[iResLen - 1] ) || IsBlendChar ( *pToken ) )
			return false;

		return !sphIsSpace ( dOut[iResLen-1] ) && !sphIsSpace ( *pToken );
	}

	FORCE_INLINE bool IsSpecialQueryCode ( int iCode ) const
	{
		return iCode=='!' || iCode=='^' || iCode=='$' || iCode=='*' || iCode=='=';
	}

	FORCE_INLINE bool IsBlendChar ( int iCode ) const
	{
		ARRAY_FOREACH ( i, m_dBlendChars )
			if ( iCode>=m_dBlendChars[i].m_iStart && iCode<=m_dBlendChars[i].m_iEnd )
				return true;

		return false;
	}
};


class FieldFilterCJK_c : public ISphFieldFilter 
{
public:
			FieldFilterCJK_c ( std::unique_ptr<CJKPreprocessor_c> pPreprocessor );

	bool	Init ( CSphString & sError );
	int		Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery ) final;
	void	GetSettings ( CSphFieldFilterSettings & tSettings ) const final;
	std::unique_ptr<ISphFieldFilter> Clone ( const FieldFilterOptions_t * pOptions ) const final;

	bool	SetBlendChars ( const char * szBlendChars, CSphString & sError );
	void	Setup ( std::unique_ptr<ISphFieldFilter> pParent );

private:
	std::unique_ptr<CJKPreprocessor_c>	m_pPreprocessor;
	std::unique_ptr<ISphFieldFilter>	m_pParent;
	CSphString							m_sBlendChars;
};

std::unique_ptr<ISphFieldFilter> CreateFilterCJK ( std::unique_ptr<ISphFieldFilter> pParent, std::unique_ptr<CJKPreprocessor_c> pPreprocessor, const char * szBlendChars, CSphString & sError );
bool CheckTokenizerCJK ( CSphIndexSettings & tSettings, const CSphTokenizerSettings & tTokSettings, CSphString & sError );
