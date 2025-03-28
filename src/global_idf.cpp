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
#include "global_idf.h"
#include "sphinxint.h"
#include "fileutils.h"

#include <math.h>


#pragma pack(push, 4)
struct IDFWord_t
{
	uint64_t m_uWordID;
	DWORD m_iDocs;
};
#pragma pack(pop)
STATIC_SIZE_ASSERT            ( IDFWord_t, 12 );

static const int HASH_BITS = 16;

using namespace sph;

/// global IDF
class CSphGlobalIDF final : public IDFer_c
{
protected:
	~CSphGlobalIDF() final = default;
public:
	bool Touch ( const CSphString& sFilename );
	bool Preread ( const CSphString& sFilename, CSphString& sError );
	float GetIDF ( const CSphString& sWord, int64_t iDocsLocal, bool bPlainIDF ) const final;

private:
	DWORD GetDocs ( const CSphString& sWord ) const;

	int64_t m_iTotalDocuments = 0;
	int64_t m_iTotalWords = 0;
	SphOffset_t m_uMTime = 0;
	CSphLargeBuffer<IDFWord_t> m_pWords;
	CSphLargeBuffer<int64_t> m_pHash;
};

using CSphGlobalIDFRefPtr_c = CSphRefcountedPtr<CSphGlobalIDF>;

// check if backend file was modified
bool CSphGlobalIDF::Touch ( const CSphString& sFilename )
{
	// update m_uMTime, return true if modified
	struct_stat tStat = { 0 };
	if ( stat ( sFilename.cstr (), &tStat )<0 )
		tStat.st_mtime = 0;
	bool bModified = ( m_uMTime!=tStat.st_mtime );
	m_uMTime = tStat.st_mtime;
	return bModified;
}


bool CSphGlobalIDF::Preread ( const CSphString& sFilename, CSphString& sError )
{
	Touch ( sFilename );

	CSphAutofile tFile;
	if ( tFile.Open ( sFilename, SPH_O_READ, sError )<0 )
		return false;

	const SphOffset_t iSize = sphGetFileSize ( tFile.GetFD (), nullptr ) - sizeof ( SphOffset_t );
	sphReadThrottled ( tFile.GetFD (), &m_iTotalDocuments, sizeof ( SphOffset_t ));
	m_iTotalWords = iSize / sizeof ( IDFWord_t );

	// allocate words cache
	CSphString sWarning;
	if ( !m_pWords.Alloc ( m_iTotalWords, sError ))
		return false;

	// allocate lookup table if needed
	int iHashSize = ( int ) ( U64C( 1 ) << HASH_BITS );
	if ( m_iTotalWords>iHashSize * 8 )
	{
		if ( !m_pHash.Alloc ( iHashSize + 2, sError ))
			return false;
	}

	// read file into memory (may exceed 2GB)
	int64_t iRead = sphReadThrottled ( tFile.GetFD (), m_pWords.GetWritePtr (), iSize );
	if ( iRead!=iSize )
		return false;

	if ( sphInterrupted ())
		return false;

	// build lookup table
	if ( m_pHash.GetLengthBytes ())
	{
		int64_t* pHash = m_pHash.GetWritePtr ();

		uint64_t uFirst = m_pWords[0].m_uWordID;
		uint64_t uRange = m_pWords[m_iTotalWords - 1].m_uWordID - uFirst;

		DWORD iShift = 0;
		while ( uRange>=( U64C( 1 ) << HASH_BITS ))
		{
			iShift++;
			uRange >>= 1;
		}

		pHash[0] = iShift;
		pHash[1] = 0;
		DWORD uLastHash = 0;

		for ( int64_t i = 1; i<m_iTotalWords; i++ )
		{
			// check for interrupt (throttled for speed)
			if (( i & 0xffff )==0 && sphInterrupted ())
				return false;

			auto uHash = ( DWORD ) (( m_pWords[i].m_uWordID - uFirst ) >> iShift );

			if ( uHash==uLastHash )
				continue;

			while ( uLastHash<uHash )
				pHash[++uLastHash + 1] = i;

			uLastHash = uHash;
		}
		pHash[++uLastHash + 1] = m_iTotalWords;
	}
	return true;
}


DWORD CSphGlobalIDF::GetDocs ( const CSphString& sWord ) const
{
	const char* s = sWord.cstr ();

	// replace = to MAGIC_WORD_HEAD_NONSTEMMED for exact terms
	char sBuf[3 * SPH_MAX_WORD_LEN + 4];
	if ( s && *s=='=' )
	{
		strncpy ( sBuf, s, sizeof ( sBuf ) - 1 );
		sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
		s = sBuf;
	}

	uint64_t uWordID = sphFNV64 ( s );

	int64_t iStart = 0;
	int64_t iEnd = m_iTotalWords - 1;

	auto pWords = (const IDFWord_t*)m_pWords.GetReadPtr();

	if ( m_pHash.GetLengthBytes ())
	{
		uint64_t uFirst = pWords[0].m_uWordID;
		auto uHash = ( DWORD ) (( uWordID - uFirst ) >> m_pHash[0] );
		if ( uHash>( U64C( 1 ) << HASH_BITS ))
			return 0;

		iStart = m_pHash[uHash + 1];
		iEnd = m_pHash[uHash + 2] - 1;
	}

	const IDFWord_t* pWord = sphBinarySearch ( pWords + iStart, pWords + iEnd,
		bind ( &IDFWord_t::m_uWordID ), uWordID );
	return pWord ? pWord->m_iDocs : 0;
}


float CSphGlobalIDF::GetIDF ( const CSphString& sWord, int64_t iDocsLocal, bool bPlainIDF ) const
{
	int64_t iDocs = Max ( iDocsLocal, ( int64_t ) GetDocs ( sWord ));
	int64_t iTotalClamped = Max ( m_iTotalDocuments, iDocs );

	if ( !iDocs )
		return 0.0f;

	if ( bPlainIDF )
		iTotalClamped += 1-iDocs;

	float fLogTotal = logf ( float ( 1 + iTotalClamped ));
	return logf ( float ( iTotalClamped ) / float ( iDocs )) / ( 2 * fLogTotal );
}

/// global idf definitions hash
class cGlobalIDF
{
	mutable RwLock_t m_tLock;
	SmallStringHash_T<CSphGlobalIDFRefPtr_c> m_hIDFs GUARDED_BY ( m_tLock );

public:
	bool LoadGlobalIDF ( const CSphString& sPath, CSphString& sError );
	bool ReloadGlobalIDF ( const CSphString& sPath, CSphString& sError );
	CSphGlobalIDFRefPtr_c* GetIDF ( const CSphString& sPath );
	StrVec_t Collect() const;
	void DeleteMany ( const StrVec_t& dFiles );
	void Clear ();
};

cGlobalIDF& GetGlobalIDF()
{
	static cGlobalIDF tIDF;
	return tIDF;
}

static CSphGlobalIDFRefPtr_c DoPrereadIDF ( const CSphString& sPath, CSphString& sError )
{
	CSphGlobalIDFRefPtr_c pNewIDF { new CSphGlobalIDF };
	if ( !pNewIDF->Preread ( sPath, sError ))
		pNewIDF = nullptr;
	return pNewIDF;
}

bool cGlobalIDF::LoadGlobalIDF ( const CSphString& sPath, CSphString& sError )
{
	sphLogDebug ( "Loading global IDF (%s)", sPath.cstr ());
	auto pGlobalIDF = DoPrereadIDF ( sPath, sError );
	if ( !pGlobalIDF )
		return false;

	ScWL_t wLock ( m_tLock );
	m_hIDFs.Add ( std::move (pGlobalIDF), sPath );
	return true;
}

bool cGlobalIDF::ReloadGlobalIDF ( const CSphString& sPath, CSphString& sError )
{
	sphLogDebug ( "Reloading global IDF (%s)", sPath.cstr ());
	auto pGlobalIDF = DoPrereadIDF ( sPath, sError );
	if ( !pGlobalIDF )
		return false;

	ScWL_t wLock ( m_tLock );
	auto* ppGlobalIDF = m_hIDFs ( sPath );
	if ( ppGlobalIDF )
		*ppGlobalIDF = std::exchange ( pGlobalIDF, nullptr );
	return true;
}

CSphGlobalIDFRefPtr_c* cGlobalIDF::GetIDF ( const CSphString& sPath )
{
	ScRL_t RLock ( m_tLock );
	return m_hIDFs ( sPath );
}

StrVec_t cGlobalIDF::Collect() const
{
	StrVec_t dCollection;
	ScRL_t rLock ( m_tLock );
	for ( auto& dIdf : m_hIDFs )
		dCollection.Add ( dIdf.first );
	return dCollection;
}

void cGlobalIDF::DeleteMany ( const StrVec_t& dFiles )
{
	ScWL_t wLock ( m_tLock );
	for ( const auto& sKey : dFiles )
	{
		sphLogDebug ( "Unloading global IDF (%s)", sKey.cstr() );
		m_hIDFs.Delete ( sKey );
	}
}

void cGlobalIDF::Clear()
{
	ScWL_t wLock ( m_tLock );
	m_hIDFs.Reset();
}

bool sph::PrereadGlobalIDF ( const CSphString& sPath, CSphString& sError )
{
	auto& tGlobalIDF = GetGlobalIDF();
	auto* ppGlobalIDF = tGlobalIDF.GetIDF(sPath);

	if ( !ppGlobalIDF )
		return tGlobalIDF.LoadGlobalIDF ( sPath, sError );

	auto& pGlobalIDF = *ppGlobalIDF;

	if ( pGlobalIDF && pGlobalIDF->Touch ( sPath ))
		return tGlobalIDF.ReloadGlobalIDF ( sPath, sError );

	return true;
}

static StrVec_t CollectUnlistedIn ( const StrVec_t& dFiles )
{
	StrVec_t dAllIDFs = GetGlobalIDF().Collect();
	StrVec_t dCollection;
	for ( const auto& sIdf : dAllIDFs )
		if ( !dFiles.Contains ( sIdf ) )
			dCollection.Add ( sIdf );
	return dCollection;
}

static void DeleteUnlistedIn ( const StrVec_t& dFiles )
{
	auto dUnlisted = CollectUnlistedIn ( dFiles );
	GetGlobalIDF().DeleteMany(dUnlisted);
}

void sph::UpdateGlobalIDFs ( const StrVec_t& dFiles )
{
	// delete unlisted entries
	DeleteUnlistedIn ( dFiles );

	// load/rotate remaining entries
	CSphString sError;
	ARRAY_FOREACH ( i, dFiles )
	{
		const auto& sPath = dFiles[i];
		if ( !PrereadGlobalIDF ( sPath, sError ))
			sphLogDebug ( "Could not load global IDF (%s): %s", sPath.cstr (), sError.cstr ());
	}
}

void sph::ShutdownGlobalIDFs ()
{
	StrVec_t dAllIDFs = GetGlobalIDF().Collect();
	GetGlobalIDF().DeleteMany ( dAllIDFs );
}

IDFerRefPtr_c sph::GetIDFer ( const CSphString& IDFPath )
{
	IDFerRefPtr_c pResult;
	auto* ppGlobalIDF = GetGlobalIDF().GetIDF ( IDFPath );
	if ( ppGlobalIDF )
		pResult = *ppGlobalIDF;
	return pResult;
}
