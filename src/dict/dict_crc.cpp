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

#include "dict_crc.h"

//////////////////////////////////////////////////////////////////////////

void DiskDictTraits_c::DictBegin ( CSphAutofile&, CSphAutofile& tDict, int iLimit )
{
	DiskDictTraits_c::SortedDictBegin ( tDict, iLimit, 0 );
}

void DiskDictTraits_c::SortedDictBegin ( CSphAutofile& tDict, int, int )
{
	m_wrDict.CloseFile();
	m_wrDict.SetFile ( tDict, nullptr, m_sWriterError );
	m_wrDict.PutByte ( 1 );
}

bool DiskDictTraits_c::DictEnd ( DictHeader_t* pHeader, int, CSphString& sError )
{
	// flush wordlist checkpoints
	pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos();
	pHeader->m_iDictCheckpoints = m_dCheckpoints.GetLength();
	ARRAY_FOREACH ( i, m_dCheckpoints )
	{
		assert ( m_dCheckpoints[i].m_iWordlistOffset );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_uWordID );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_iWordlistOffset );
	}

	// done
	m_wrDict.CloseFile();
	if ( m_wrDict.IsError() )
		sError = m_sWriterError;
	return !m_wrDict.IsError();
}

void DiskDictTraits_c::DictEntry ( const DictEntry_t& tEntry )
{
	assert ( m_iSkiplistBlockSize > 0 );

	// insert wordlist checkpoint
	if ( ( m_iEntries % SPH_WORDLIST_CHECKPOINT ) == 0 )
	{
		if ( m_iEntries ) // but not the 1st entry
		{
			assert ( tEntry.m_iDoclistOffset > m_iLastDoclistPos );
			m_wrDict.ZipInt ( 0 );												// indicate checkpoint
			m_wrDict.ZipOffset ( tEntry.m_iDoclistOffset - m_iLastDoclistPos ); // store last length
		}

		// restart delta coding, once per SPH_WORDLIST_CHECKPOINT entries
		m_iLastWordID = 0;
		m_iLastDoclistPos = 0;

		// begin new wordlist entry
		assert ( m_wrDict.GetPos() <= UINT_MAX );

		CSphWordlistCheckpoint& tCheckpoint = m_dCheckpoints.Add();
		tCheckpoint.m_uWordID = tEntry.m_uWordID;
		tCheckpoint.m_iWordlistOffset = m_wrDict.GetPos();
	}

	assert ( tEntry.m_iDoclistOffset > m_iLastDoclistPos );
	m_wrDict.ZipOffset ( tEntry.m_uWordID - m_iLastWordID ); // FIXME! slow with 32bit wordids
	m_wrDict.ZipOffset ( tEntry.m_iDoclistOffset - m_iLastDoclistPos );

	m_iLastWordID = tEntry.m_uWordID;
	m_iLastDoclistPos = tEntry.m_iDoclistOffset;

	assert ( tEntry.m_iDocs );
	assert ( tEntry.m_iHits );
	m_wrDict.ZipInt ( tEntry.m_iDocs );
	m_wrDict.ZipInt ( tEntry.m_iHits );

	// write skiplist location info, if any
	if ( tEntry.m_iDocs > m_iSkiplistBlockSize )
		m_wrDict.ZipOffset ( tEntry.m_iSkiplistOffset );

	++m_iEntries;
}

void DiskDictTraits_c::DictEndEntries ( SphOffset_t iDoclistOffset )
{
	assert ( iDoclistOffset >= m_iLastDoclistPos );
	m_wrDict.ZipInt ( 0 );									   // indicate checkpoint
	m_wrDict.ZipOffset ( iDoclistOffset - m_iLastDoclistPos ); // store last doclist length
}

//////////////////////////////////////////////////////////////////////////

DictRefPtr_c sphCreateDictionaryCRC ( const CSphDictSettings& tSettings, const CSphEmbeddedFiles* pFiles, const TokenizerRefPtr_c& pTokenizer, const char* szIndex, bool bStripFile, int iSkiplistBlockSize, FilenameBuilder_i* pFilenameBuilder, CSphString& sError )
{
	DictRefPtr_c pDict { new CSphDictCRC<CRCALGO::FNV64> };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, szIndex, bStripFile, pFilenameBuilder, sError );
	// might be empty due to wrong morphology setup
	if ( pDict )
		pDict->SetSkiplistBlockSize ( iSkiplistBlockSize );
	return pDict;
}