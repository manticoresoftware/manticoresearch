//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "dict_base.h"

#include "fileutils.h"
#include "sphinx.h"
#include "tokenizer/tokenizer.h"

void CSphDict::DictBegin ( CSphAutofile&, CSphAutofile&, int ) {}
void CSphDict::DictEntry ( const DictEntry_t& ) {}
void CSphDict::DictEndEntries ( SphOffset_t ) {}

bool CSphDict::DictEnd ( DictHeader_t*, int, CSphString& )
{
	return true;
}

bool CSphDict::DictIsError() const
{
	return true;
}

DictRefPtr_c GetStatelessDict ( const DictRefPtr_c& pDict )
{
	if ( !pDict )
		return nullptr;

	if ( pDict->HasState() )
		return pDict->Clone();

	return pDict;
}

///////////////////////////////////////////////////////////////////////

void SetupDictionary ( DictRefPtr_c& pDict, const CSphDictSettings& tSettings, const CSphEmbeddedFiles* pFiles, const TokenizerRefPtr_c& pTokenizer, const char* szIndex, bool bStripFile, FilenameBuilder_i* pFilenameBuilder, CSphString& sError )
{
	assert ( pTokenizer );

	pDict->Setup ( tSettings );
	if ( pDict->SetMorphology ( tSettings.m_sMorphology.cstr(), sError ) == CSphDict::ST_ERROR )
	{
		pDict = nullptr;
		return;
	}

	if ( pFiles && pFiles->m_bEmbeddedStopwords )
		pDict->LoadStopwords ( pFiles->m_dStopwords );
	else
	{
		CSphString sStopwordFile = tSettings.m_sStopwords;
		if ( !sStopwordFile.IsEmpty() )
		{
			if ( pFilenameBuilder )
				sStopwordFile = pFilenameBuilder->GetFullPath ( sStopwordFile );

			pDict->LoadStopwords ( sStopwordFile.cstr(), pTokenizer, bStripFile );
		}
	}

	StrVec_t dWordformFiles;
	if ( pFilenameBuilder )
	{
		dWordformFiles.Resize ( tSettings.m_dWordforms.GetLength() );
		ARRAY_FOREACH ( i, tSettings.m_dWordforms )
			dWordformFiles[i] = pFilenameBuilder->GetFullPath ( tSettings.m_dWordforms[i] );
	}

	pDict->LoadWordforms ( pFilenameBuilder ? dWordformFiles : tSettings.m_dWordforms, pFiles && pFiles->m_bEmbeddedWordforms ? pFiles : nullptr, pTokenizer, szIndex );
}
