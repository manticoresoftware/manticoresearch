//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "word_forms.h"
#include "tokenizer/multiform_container.h"

/////////////////////////////////////////////////////////////////////////////
CSphWordforms::~CSphWordforms()
{
	if ( !m_pMultiWordforms )
		return;

	for ( auto& tForms : m_pMultiWordforms->m_Hash )
	{
		for ( auto* pForm : tForms.second->m_pForms )
			SafeDelete ( pForm );
		SafeDelete ( tForms.second );
	}
}


bool CSphWordforms::IsEqual ( const CSphVector<CSphSavedFile>& dFiles )
{
	if ( m_dFiles.GetLength() != dFiles.GetLength() )
		return false;

	// don't check file time AND check filename w/o path
	// that way same file in different dirs will be only loaded once
	ARRAY_FOREACH ( i, m_dFiles )
	{
		const CSphSavedFile& tF1 = m_dFiles[i];
		const CSphSavedFile& tF2 = dFiles[i];
		CSphString sFile1 = tF1.m_sFilename;
		CSphString sFile2 = tF2.m_sFilename;
		StripPath ( sFile1 );
		StripPath ( sFile2 );

		if ( sFile1 != sFile2 || tF1.m_uCRC32 != tF2.m_uCRC32 || tF1.m_uSize != tF2.m_uSize )
			return false;
	}

	return true;
}


bool CSphWordforms::ToNormalForm ( BYTE* pWord, bool bBefore, bool bOnlyCheck ) const
{
	int* pIndex = m_hHash ( (char*)pWord );
	if ( !pIndex )
		return false;

	if ( *pIndex < 0 || *pIndex >= m_dNormalForms.GetLength() )
		return false;

	if ( bBefore == m_dNormalForms[*pIndex].m_bAfterMorphology )
		return false;

	if ( m_dNormalForms[*pIndex].m_sWord.IsEmpty() )
		return false;

	if ( bOnlyCheck )
		return true;

	strcpy ( (char*)pWord, m_dNormalForms[*pIndex].m_sWord.cstr() ); // NOLINT
	return true;
}