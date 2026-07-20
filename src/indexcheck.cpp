//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "indexcheck.h"

#include "fileutils.h"
#include "attribute.h"
#include "indexformat.h"
#include "docidlookup.h"
#include "docstore.h"
#include "conversion.h"
#include "columnarlib.h"
#include "indexsettings.h"
#include "indexfiles.h"
#include "roaring/roaring64map.hh"

#include "killlist.h"

constexpr int FAILS_THRESH = 100;

static inline bool IsLegacyHitlessLayoutForCheck ( DWORD uVersion )
{
	return uVersion<68;
}

static inline int GetDictDocsForCheck ( int iRawDocs, bool bLegacyLayout )
{
	return bLegacyLayout ? iRawDocs : ( iRawDocs & HITLESS_DOC_MASK );
}

class DebugCheckError_c final : public DebugCheckError_i
{
public:
	DebugCheckError_c ( FILE* pFile, const DocID_t* pExtract );

	bool Fail ( const char* szFmt, ... ) final;
	void Msg ( const char* szFmt, ... ) final;
	void Progress ( const char* szFmt, ... ) final;
	void Done() final;

	int64_t GetNumFails() const final;
	const DocID_t* GetExtractDocs() const final { return m_pExtract; };

	void CheckDocidDup ( DocID_t tDocid, DWORD uRowid ) final;
	void FinishDiskChunk ( int64_t iNumRows ) final;

private:
	FILE* m_pFile { nullptr };
	bool m_bProgress { false };
	int64_t m_tStartTime { 0 };
	int64_t m_nFails { 0 };
	int64_t m_nFailsPrinted { 0 };
	const DocID_t* m_pExtract;

	roaring::Roaring64Map m_tDocids;
};

DebugCheckError_c::DebugCheckError_c ( FILE * pFile, const DocID_t* pExtract )
	: m_pFile ( pFile )
	, m_pExtract { pExtract }
{
	assert ( pFile );
	m_bProgress = isatty ( fileno ( pFile ) )!=0;
	m_tStartTime = sphMicroTimer();
}


void DebugCheckError_c::Msg ( const char * szFmt, ... )
{
	assert ( m_pFile );
	va_list ap;
	va_start ( ap, szFmt );
	vfprintf ( m_pFile, szFmt, ap );
	fprintf ( m_pFile, "\n" );
	va_end ( ap );
}


bool DebugCheckError_c::Fail ( const char * szFmt, ... )
{
	assert ( m_pFile );
	if ( ++m_nFails>=FAILS_THRESH )
		return false;

	va_list ap;
	va_start ( ap, szFmt );
	fprintf ( m_pFile, "FAILED, " );
	vfprintf ( m_pFile, szFmt, ap );

	fprintf ( m_pFile, "\n" );
	va_end ( ap );

	m_nFailsPrinted++;
	if ( m_nFailsPrinted==FAILS_THRESH )
		fprintf ( m_pFile, "(threshold reached; suppressing further output)\n" );

	return false;
}


void DebugCheckError_c::Progress ( const char * szFmt, ... )
{
	if ( !m_bProgress )
		return;

	assert ( m_pFile );

	va_list ap;
	va_start ( ap, szFmt );
	vfprintf ( m_pFile, szFmt, ap );
	fprintf ( m_pFile, "\r" );
	va_end ( ap );

	fflush ( m_pFile );
}


void DebugCheckError_c::Done()
{
	assert ( m_pFile );

	// well, no known kinds of failures, maybe some unknown ones
	int64_t tmCheck = sphMicroTimer() - m_tStartTime;
	if ( !m_nFails )
		fprintf ( m_pFile, "check passed" );
	else if ( m_nFails!=m_nFailsPrinted )
		fprintf ( m_pFile, "check FAILED, " INT64_FMT " of " INT64_FMT " failures reported", m_nFailsPrinted, m_nFails );
	else
		fprintf ( m_pFile, "check FAILED, " INT64_FMT " failures reported", m_nFails );

	fprintf ( m_pFile, ", %d.%d sec elapsed\n", (int)(tmCheck/1000000), (int)((tmCheck/100000)%10) );
}


int64_t DebugCheckError_c::GetNumFails() const
{
	return m_nFails;
}

void DebugCheckError_c::CheckDocidDup ( DocID_t tDocid, DWORD uRowid )
{
	if ( !m_tDocids.addChecked ( (uint64_t)tDocid ) )
		Fail ( "duplicate of docid " INT64_FMT " found at row %u", tDocid, uRowid );
}

void DebugCheckError_c::FinishDiskChunk ( int64_t iNumRows )
{
	m_tDocids.runOptimize();
	m_tDocids.shrinkToFit();
}


DebugCheckError_i* MakeDebugCheckError ( FILE* fp, DocID_t* pExtract )
{
	return new DebugCheckError_c ( fp, pExtract );
}

//////////////////////////////////////////////////////////////////////////

class FileDebugCheckReader_c final : public DebugCheckReader_i
{
public:
	explicit FileDebugCheckReader_c ( CSphAutoreader * pReader )
		: m_pReader ( pReader )
	{}

	int64_t GetLengthBytes() final
	{
		return ( m_pReader ? m_pReader->GetFilesize() : 0 );
	}

	bool GetBytes ( void * pData, int iSize ) final
	{
		if ( !m_pReader )
			return false;

		m_pReader->GetBytes ( pData, iSize );
		return !m_pReader->GetErrorFlag();
	}

	bool SeekTo ( int64_t iOff, int iHint ) final
	{
		if ( !m_pReader )
			return false;

		m_pReader->SeekTo ( iOff, iHint );
		return !m_pReader->GetErrorFlag();
	}

private:
	CSphAutoreader * m_pReader = nullptr;
};


void DebugCheck_Attributes ( DebugCheckReader_i & tAttrs, DebugCheckReader_i & tBlobs, int64_t nRows, int64_t iMinMaxBytes, const CSphSchema & tSchema, DebugCheckError_i & tReporter )
{
	// empty?
	if ( !tAttrs.GetLengthBytes() )
		return;

	tReporter.Msg ( "checking rows..." );

	if ( !tSchema.GetAttrsCount() )
		tReporter.Fail ( "no attributes in schema; schema should at least have '%s' attr", sphGetDocidName() );

	if ( tSchema.GetAttr(0).m_sName!=sphGetDocidName() )
		tReporter.Fail ( "first attribute in schema should be '%s'", tSchema.GetAttr(0).m_sName.cstr() );

	if ( tSchema.GetAttr(0).m_eAttrType!=SPH_ATTR_BIGINT )
		tReporter.Fail ( "%s attribute should be BIGINT", sphGetDocidName() );

	const CSphColumnInfo * pBlobLocator = nullptr;
	[[maybe_unused]] int nBlobAttrs = 0;

	if ( tSchema.HasBlobAttrs() )
	{
		pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );

		if ( !pBlobLocator )
			tReporter.Fail ( "schema has blob attrs, but no blob locator '%s'", sphGetBlobLocatorName() );

		if ( tSchema.GetAttr(1).m_sName!=sphGetBlobLocatorName() )
			tReporter.Fail ( "second attribute in schema should be '%s'", sphGetBlobLocatorName() );

		if ( tSchema.GetAttr(1).m_eAttrType!=SPH_ATTR_BIGINT )
			tReporter.Fail ( "%s attribute should be BIGINT", sphGetBlobLocatorName() );

		if ( !tBlobs.GetLengthBytes() )
			tReporter.Fail ( "schema has blob attrs, but blob file is empty" );

		for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
			if ( sphIsBlobAttr ( tSchema.GetAttr(i) ) )
				nBlobAttrs++;
	} else
	{
		if ( tBlobs.GetLengthBytes() )
			tReporter.Fail ( "schema has no blob attrs but has blob rows" );
	}

	// sizes and counts
	DWORD uStride = tSchema.GetRowSize();

	int64_t iAttrElemCount = ( tAttrs.GetLengthBytes() - iMinMaxBytes ) / sizeof(CSphRowitem);
	int64_t iAttrExpected = nRows*uStride;
	if ( iAttrExpected > iAttrElemCount )
		tReporter.Fail ( "rowitems count mismatch (expected=" INT64_FMT ", loaded=" INT64_FMT ")", iAttrExpected, iAttrElemCount );

	CSphVector<CSphAttrLocator> dFloatItems;
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT && !tAttr.IsColumnar()  )
			dFloatItems.Add	( tAttr.m_tLocator );
	}

	CSphFixedVector<CSphRowitem> dRow ( tSchema.GetRowSize() );
	const CSphRowitem * pRow = dRow.Begin();
	tAttrs.SeekTo ( 0, (int) dRow.GetLengthBytes() );

	for ( int64_t iRow=0; iRow<nRows; iRow++ )
	{
		tAttrs.GetBytes ( dRow.Begin(), (int) dRow.GetLengthBytes() );
		DocID_t tDocID = sphGetDocID(pRow);

		///////////////////////////
		// check blobs
		///////////////////////////

		if ( pBlobLocator )
		{
			int64_t iBlobOffset = sphGetRowAttr ( pRow, pBlobLocator->m_tLocator );

			CSphString sError;
			if ( !sphCheckBlobRow ( iBlobOffset, tBlobs, tSchema, sError ) )
				tReporter.Fail ( "%s at offset " INT64_FMT ", docid=" INT64_FMT ", rowid=" INT64_FMT " of " INT64_FMT, sError.cstr(), iBlobOffset, tDocID, iRow, nRows );
		}

		///////////////////////////
		// check floats
		///////////////////////////

		ARRAY_FOREACH ( iItem, dFloatItems )
		{
			const DWORD uValue = (DWORD)sphGetRowAttr ( pRow, dFloatItems[ iItem ] );
			const DWORD uExp = ( uValue >> 23 ) & 0xff;
			const DWORD uMantissa = uValue & 0x003fffff;

			// check normalized
			if ( uExp==0 && uMantissa!=0 )
				tReporter.Fail ( "float attribute value is unnormalized (row=" INT64_FMT ", attr=%d, id=" INT64_FMT ", raw=0x%x, value=%f)", 	iRow, iItem, tDocID, uValue, sphDW2F ( uValue ) );

			// check +-inf
			if ( uExp==0xff && uMantissa==0 )
				tReporter.Fail ( "float attribute is infinity (row=" INT64_FMT ", attr=%d, id=" INT64_FMT ", raw=0x%x, value=%f)", iRow, iItem, tDocID, uValue, sphDW2F ( uValue ) );
		}
	}
}


void DebugCheck_DeadRowMap ( int64_t iSizeBytes, int64_t nRows, DebugCheckError_i & tReporter )
{
	tReporter.Msg ( "checking dead row map..." );

	int64_t nExpectedEntries = int(( nRows+31 ) / 32);
	int64_t iExpectedSize = nExpectedEntries*sizeof(DWORD);
	if ( iSizeBytes!=iExpectedSize )
		tReporter.Fail ( "unexpected dead row map: " INT64_FMT ", expected: " INT64_FMT " bytes", iSizeBytes, iExpectedSize );
}

//////////////////////////////////////////////////////////////////////////

class CheckError_c
{
	CSphString m_sWhat;

public:
	explicit CheckError_c ( const char* szWhat )
		: m_sWhat { SphSprintf ("%s", szWhat ) }
	{}

	CheckError_c ( const char* szWhat, const CSphString& sError )
		: m_sWhat { SphSprintf ( "%s: %s", szWhat, sError.scstr() ) }
	{}

	CheckError_c ( const char* szTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) )
	{
		va_list ap;
		va_start ( ap, szTemplate );
		m_sWhat.SetSprintfVa ( szTemplate, ap );
		va_end ( ap );
	}

	const char* sWhat() const noexcept { return m_sWhat.scstr(); }
};

//////////////////////////////////////////////////////////////////////////

struct Wordid_t
{
	bool m_bWordDict;
	union {
		SphWordID_t m_uWordid;
		const char* m_szWordid;
	};
};

static StringBuilder_c& operator<< ( StringBuilder_c& dOut, const Wordid_t& tWordID )
{
	ScopedComma_c _ (dOut, dEmptyBl);
	if ( tWordID.m_bWordDict )
		return dOut << tWordID.m_szWordid;
	return dOut << "(hash) " << tWordID.m_uWordid;
}

static JsonEscapedBuilder& operator<< ( JsonEscapedBuilder& dOut, const Wordid_t& tWordID )
{
	if ( tWordID.m_bWordDict )
		dOut.NamedString ( "token", tWordID.m_szWordid );
	else
		dOut.NamedVal ( "crc", tWordID.m_uWordid );
	return dOut;
}

using cbWordidFn = std::function<void ( RowID_t, Wordid_t, int iField, int iPos, bool bIsEnd )>;

static SphOffset_t UnzipSkiplistOffsetForCheck ( CSphAutoreader & tReader, DWORD uVersion )
{
	return uVersion<=57 ? (SphOffset_t)tReader.UnzipInt() : tReader.UnzipOffset();
}


struct DictEntryForCheck_t
{
	int64_t			m_iDictPos = 0;
	SphWordID_t		m_uWordid = 0;
	SphOffset_t		m_iDoclistOffset = 0;
	SphOffset_t		m_iSkiplistOffset = 0;
	int				m_iRawDocs = 0;
	int				m_iLayoutDocs = 0;
	int				m_iDocs = 0;
	int				m_iHits = 0;
	int				m_iHint = 0;
	bool			m_bHitless = false;
	const char *	m_szWord = "";
	int				m_iWordLen = 0;
};


enum class DictReadResult_e
{
	ENTRY,
	CHECKPOINT
};


static bool UnpackWordDictKeywordForCheck ( CSphAutoreader & tReader, DictFormat_e eDictFormat, BYTE uPack, CSphVector<BYTE> & dWord, int64_t iDictPos, DebugCheckError_i & tReporter, DictEntryForCheck_t & tEntry )
{
	int iMatch = 0;
	int iDelta = 0;

	if ( uPack & 0x80 )
	{
		iDelta = ( ( uPack>>4 ) & 7 ) + 1;
		iMatch = uPack & 15;
	} else if ( eDictFormat==DictFormat_e::KEYWORDS_V2 && uPack==0x7f )
	{
		iDelta = tReader.UnzipInt();
		iMatch = tReader.UnzipInt();
	} else
	{
		iDelta = eDictFormat==DictFormat_e::KEYWORDS_V2 ? uPack : ( uPack & 127 );
		iMatch = tReader.GetByte();
	}

	const int iLastWordLen = (int)strlen ( (const char*)dWord.Begin() );
	const int iMaxWordLen = GetKeywordMaxStoredBytes ( eDictFormat );
	if ( iDelta<=0 || iMatch<0 || iMatch>iLastWordLen || iMatch+iDelta>iMaxWordLen )
	{
		tReporter.Fail ( "wrong word-delta (pos=" INT64_FMT ", word=%s, len=%d, begin=%d, delta=%d)", iDictPos, (const char*)dWord.Begin(), iLastWordLen, iMatch, iDelta );
		if ( iDelta>0 )
			tReader.SkipBytes ( iDelta );
		tEntry.m_szWord = (const char*)dWord.Begin();
		tEntry.m_iWordLen = iLastWordLen;
		return false;
	}

	tReader.GetBytes ( dWord.Begin()+iMatch, iDelta );
	tEntry.m_iWordLen = iMatch+iDelta;
	dWord[tEntry.m_iWordLen] = '\0';
	tEntry.m_szWord = (const char*)dWord.Begin();
	return true;
}


static DictReadResult_e ReadDictEntryForCheck ( CSphAutoreader & tReader, DictFormat_e eDictFormat, const CSphIndexSettings & tIndexSettings, DWORD uVersion, bool bLegacySkiplistLayout, const CSphVector<SphWordID_t> & dHitlessWords, SphWordID_t & uWordid, SphOffset_t & iDoclistOffset, CSphVector<BYTE> & dWord, DictEntryForCheck_t & tEntry, DebugCheckError_i & tReporter )
{
	const bool bWordDict = eDictFormat!=DictFormat_e::CRC;
	tEntry = DictEntryForCheck_t();
	tEntry.m_iDictPos = tReader.GetPos();

	SphWordID_t iDeltaWord = 0;
	if ( bWordDict )
		iDeltaWord = tReader.GetByte();
	else
		iDeltaWord = tReader.UnzipWordid();

	if ( !iDeltaWord )
	{
		tReader.UnzipOffset();
		uWordid = 0;
		iDoclistOffset = 0;
		if ( bWordDict )
			dWord[0] = '\0';
		return DictReadResult_e::CHECKPOINT;
	}

	if ( bWordDict )
	{
		UnpackWordDictKeywordForCheck ( tReader, eDictFormat, (BYTE)iDeltaWord, dWord, tEntry.m_iDictPos, tReporter, tEntry );

		tEntry.m_iDoclistOffset = tReader.UnzipOffset();
		tEntry.m_iRawDocs = tReader.UnzipInt();
		tEntry.m_iLayoutDocs = GetDictDocsForCheck ( tEntry.m_iRawDocs, bLegacySkiplistLayout );
		tEntry.m_iDocs = tEntry.m_iRawDocs;
		tEntry.m_iHits = tReader.UnzipInt();
		if ( tEntry.m_iLayoutDocs>=DOCLIST_HINT_THRESH )
			tEntry.m_iHint = DoclistHintUnpack ( tEntry.m_iLayoutDocs, (BYTE)tReader.GetByte() );

		if ( tIndexSettings.m_eHitless==SPH_HITLESS_SOME && ( tEntry.m_iDocs & HITLESS_DOC_FLAG )!=0 )
		{
			tEntry.m_iDocs = ( tEntry.m_iDocs & HITLESS_DOC_MASK );
			tEntry.m_bHitless = true;
		}
	} else
	{
		tEntry.m_uWordid = uWordid + iDeltaWord;
		tEntry.m_iDoclistOffset = iDoclistOffset + tReader.UnzipOffset();
		tEntry.m_iRawDocs = tReader.UnzipInt();
		tEntry.m_iLayoutDocs = GetDictDocsForCheck ( tEntry.m_iRawDocs, bLegacySkiplistLayout );
		tEntry.m_iDocs = tEntry.m_iRawDocs;
		tEntry.m_iHits = tReader.UnzipInt();
		tEntry.m_bHitless = ( dHitlessWords.BinarySearch ( tEntry.m_uWordid )!=nullptr );
		if ( tEntry.m_bHitless )
			tEntry.m_iDocs = ( tEntry.m_iDocs & HITLESS_DOC_MASK );
	}

	assert ( tIndexSettings.m_iSkiplistBlockSize>0 );
	if ( tEntry.m_iLayoutDocs>tIndexSettings.m_iSkiplistBlockSize )
		tEntry.m_iSkiplistOffset = UnzipSkiplistOffsetForCheck ( tReader, uVersion );

	return DictReadResult_e::ENTRY;
}

class DiskIndexChecker_c::Impl_c
{
public:
			Impl_c ( CSphIndex & tIndex, DebugCheckError_i & tReporter );

	bool	OpenFiles ();
	void	Setup ( int64_t iNumRows, int64_t iDocinfoIndex, int64_t iMinMaxIndex, bool bCheckIdDups );
	CSphVector<SphWordID_t> & GetHitlessWords() { return m_dHitlessWords; }

	void	Check();
	void	ExtractDocs ();

private:
	CSphIndex &				m_tIndex;
	CSphAutoreader			m_tDictReader;
	DataReaderFactoryPtr_c	m_pDocsReader;
	DataReaderFactoryPtr_c	m_pHitsReader;
	CSphAutoreader			m_tSkipsReader;
	CSphAutoreader			m_tDeadRowReader;
	CSphAutoreader			m_tAttrReader;
	CSphAutoreader			m_tBlobReader;
	CSphAutoreader			m_tDocstoreReader;
	CSphVector<SphWordID_t> m_dHitlessWords;

	DebugCheckError_i &		m_tReporter;

	bool					m_bHasBlobs = false;
	bool					m_bHasDocstore = false;
	bool					m_bIsEmpty = false;
	DWORD					m_uVersion = 0;
	int64_t					m_iNumRows = 0;
	int64_t					m_iDocinfoIndex = 0;
	int64_t					m_iMinMaxIndex = 0;
	bool					m_bCheckIdDups = false;
	CSphSchema				m_tSchema;
	CWordlist				m_tWordlist;

	RowID_t GetRowidByDocid ( DocID_t iDocid ) const;
	RowID_t CheckIfKilled ( RowID_t iRowID ) const;

	void	CheckDictionary();
	void	CheckDocs ( cbWordidFn&& cbfndoc = nullptr );
	void	CheckAttributes();
	void	CheckKillList() const;
	void	CheckBlockIndex();
	void	CheckColumnar();
	void	CheckDocidLookup();
	void	CheckUuidDocidLookup ( CSphReader & tLookup, int64_t iDocs, SphOffset_t tEntriesOffset, SphOffset_t tNumericEnd );
	void	CheckDocids();
	void	CheckDocstore();
	void	CheckSchema();

	bool	ReadLegacyHeader ( CSphString& sError );
	bool	ReadHeader ( CSphString& sError );
	CSphString	GetFilename ( ESphExt eExt ) const;
};


DiskIndexChecker_c::Impl_c::Impl_c ( CSphIndex & tIndex, DebugCheckError_i & tReporter )
	: m_tIndex ( tIndex )
	, m_tReporter ( tReporter )
{}


bool DiskIndexChecker_c::Impl_c::ReadLegacyHeader ( CSphString& sError )
{
	CSphAutoreader tHeaderReader;
	if ( !tHeaderReader.Open ( GetFilename ( SPH_EXT_SPH ), sError ) )
		return false;

	const char * szHeader = tHeaderReader.GetFilename().cstr();

	// magic header
	const char * szFmt = CheckFmtMagic ( tHeaderReader.GetDword() );
	if ( szFmt )
	{
		sError.SetSprintf ( szFmt, szHeader );
		return false;
	}

	// version
	m_uVersion = tHeaderReader.GetDword();
	if ( m_uVersion<=1 || m_uVersion>INDEX_FORMAT_VERSION )
	{
		sError.SetSprintf ( "%s is v.%u, binary is v.%u", szHeader, m_uVersion, INDEX_FORMAT_VERSION );
		return false;
	}

	// we don't support anything prior to v54
	DWORD uMinFormatVer = 54;
	if ( m_uVersion<uMinFormatVer )
	{
		sError.SetSprintf ( "tables prior to v.%u are no longer supported (use index_converter tool); %s is v.%u", uMinFormatVer, szHeader, m_uVersion );
		return false;
	}

	// schema
	ReadSchema ( tHeaderReader, m_tSchema, m_uVersion );

	// dictionary header (wordlist checkpoints, infix blocks, etc)
	m_tWordlist.m_iDictCheckpointsOffset = tHeaderReader.GetOffset();
	m_tWordlist.m_iDictCheckpoints = tHeaderReader.GetDword();
	m_tWordlist.m_iInfixCodepointBytes = tHeaderReader.GetByte();
	m_tWordlist.m_iInfixBlocksOffset = tHeaderReader.GetDword();
	m_tWordlist.m_iInfixBlocksWordsSize = tHeaderReader.GetDword();

	m_tWordlist.m_dCheckpoints.Reset ( m_tWordlist.m_iDictCheckpoints );

	auto eDictFormat = m_tIndex.GetDictionary()->GetSettings().GetDictFormat();
	if ( eDictFormat==DictFormat_e::KEYWORDS_V2 )
	{
		sError = "indexcheck for dict=keywords_32k is not implemented yet";
		return false;
	}

	return m_tWordlist.Preread ( GetFilename(SPH_EXT_SPI), eDictFormat, m_tIndex.GetSettings().m_iSkiplistBlockSize, sError );
	// FIXME! add more header checks

}

bool DiskIndexChecker_c::Impl_c::ReadHeader ( CSphString& sError )
{
	bool bHeaderIsJson;
	{
		BYTE dBuffer[8];
		CSphAutoreader tHeaderReader ( dBuffer, sizeof ( dBuffer ) );
		if ( !tHeaderReader.Open ( GetFilename ( SPH_EXT_SPH ), sError ) )
			return false;

		tHeaderReader.GetDword();
		bHeaderIsJson = dBuffer[0] == '{';
	}

	if ( !bHeaderIsJson ) // that is old style binary header
		return ReadLegacyHeader ( sError );


	auto sHeader = GetFilename ( SPH_EXT_SPH );
	const char* szHeader = sHeader.scstr();
	using namespace bson;

	CSphVector<BYTE> dData;
	if ( !sphJsonParse ( dData, GetFilename ( SPH_EXT_SPH ), sError ) )
		return false;

	Bson_c tBson ( dData );
	if ( tBson.IsEmpty() || !tBson.IsAssoc() )
	{
		sError = "Something wrong read from json header - it is either empty, either not root object.";
		return false;
	}

	// version
	m_uVersion = (DWORD)Int ( tBson.ChildByName ( "index_format_version" ) );
	if ( m_uVersion <= 1 || m_uVersion > INDEX_FORMAT_VERSION )
	{
		sError.SetSprintf ( "%s is v.%u, binary is v.%u", szHeader, m_uVersion, INDEX_FORMAT_VERSION );
		return false;
	}

	// we don't support anything prior to v64 with json format
	DWORD uMinFormatVer = 64;
	if ( m_uVersion < uMinFormatVer )
	{
		sError.SetSprintf ( "tables prior to v.%u are no longer supported (use index_converter tool); %s is v.%u", uMinFormatVer, szHeader, m_uVersion );
		return false;
	}

	// schema
	ReadSchemaJson ( tBson.ChildByName ( "schema" ), m_tSchema );

	// dictionary header (wordlist checkpoints, infix blocks, etc)
	m_tWordlist.m_iDictCheckpointsOffset = Int ( tBson.ChildByName ( "dict_checkpoints_offset" ) );
	m_tWordlist.m_iDictCheckpoints = Int ( tBson.ChildByName ( "dict_checkpoints" ) );
	m_tWordlist.m_iInfixCodepointBytes = (int)Int ( tBson.ChildByName ( "infix_codepoint_bytes" ) );
	m_tWordlist.m_iInfixBlocksOffset = Int ( tBson.ChildByName ( "infix_blocks_offset" ) );
	m_tWordlist.m_iInfixBlocksWordsSize = Int ( tBson.ChildByName ( "infix_block_words_size" ) );

	m_tWordlist.m_dCheckpoints.Reset ( m_tWordlist.m_iDictCheckpoints );

	auto eDictFormat = m_tIndex.GetDictionary()->GetSettings().GetDictFormat();
	return m_tWordlist.Preread ( GetFilename ( SPH_EXT_SPI ), eDictFormat, m_tIndex.GetSettings().m_iSkiplistBlockSize, sError );
	// FIXME! add more header checks
}


bool DiskIndexChecker_c::Impl_c::OpenFiles ()
{
	CSphString sError;
	if ( !ReadHeader ( sError ) )
		return m_tReporter.Fail ( "error reading table header: %s", sError.cstr() );

	if ( !m_tDictReader.Open ( GetFilename ( SPH_EXT_SPI ), sError ) )
		return m_tReporter.Fail ( "unable to open dictionary: %s", sError.cstr() );

	// use file reader during debug check to lower memory pressure
	m_pDocsReader = NewProxyReader ( GetFilename(SPH_EXT_SPD), sError, DataReaderFactory_c::DOCS, m_tIndex.GetMutableSettings().m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE );
	if ( !m_pDocsReader )
		return m_tReporter.Fail ( "unable to open doclist: %s", sError.cstr() );

	// use file reader during debug check to lower memory pressure
	m_pHitsReader = NewProxyReader ( GetFilename(SPH_EXT_SPP), sError, DataReaderFactory_c::HITS, m_tIndex.GetMutableSettings().m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE );
	if ( !m_pHitsReader )
		return m_tReporter.Fail ( "unable to open hitlist: %s", sError.cstr() );

	if ( !m_tSkipsReader.Open ( GetFilename(SPH_EXT_SPE), sError ) )
		return m_tReporter.Fail ( "unable to open skiplist: %s", sError.cstr () );

	if ( !m_tDeadRowReader.Open ( GetFilename(SPH_EXT_SPM).cstr(), sError ) )
		return m_tReporter.Fail ( "unable to open dead-row map: %s", sError.cstr() );

	if ( m_tSchema.HasNonColumnarAttrs() && !m_tAttrReader.Open ( GetFilename(SPH_EXT_SPA).cstr(), sError ) )
		return m_tReporter.Fail ( "unable to open attributes: %s", sError.cstr() );

	if ( m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
	{
		if ( !m_tBlobReader.Open ( GetFilename(SPH_EXT_SPB), sError ) )
			return m_tReporter.Fail ( "unable to open blobs: %s", sError.cstr() );

		m_bHasBlobs = true;
	}

	if ( m_uVersion>=57 && ( m_tSchema.HasStoredFields() || m_tSchema.HasStoredAttrs() ) )
	{
		if ( !m_tDocstoreReader.Open ( GetFilename(SPH_EXT_SPDS).cstr(), sError ) )
			return m_tReporter.Fail ( "unable to open docstore: %s", sError.cstr() );

		m_bHasDocstore = true;
	}

	m_bIsEmpty = m_iNumRows==0;

	return true;
}


void DiskIndexChecker_c::Impl_c::Setup ( int64_t iNumRows, int64_t iDocinfoIndex, int64_t iMinMaxIndex, bool bCheckIdDups )
{
	m_iNumRows = iNumRows;
	m_iDocinfoIndex = iDocinfoIndex;
	m_iMinMaxIndex = iMinMaxIndex;
	m_bCheckIdDups = bCheckIdDups;
}

struct WordVariantHit_t
{
	CSphString m_sWord;
	Wordid_t m_tWord;
	bool m_bIsLast;
};

struct WordHit_t
{
	CSphVector<WordVariantHit_t> m_dHits;
	int 	m_iPos = -1;

	void AddWord ( Wordid_t tWord, bool bIsLast )
	{
		auto& dHit = m_dHits.Add();
		dHit.m_bIsLast = bIsLast;
		dHit.m_tWord = tWord;
		if ( tWord.m_bWordDict )
		{
			dHit.m_sWord = tWord.m_szWordid;
			dHit.m_tWord.m_szWordid = dHit.m_sWord.cstr();
		}
	}
	void Print ( StringBuilder_c& sOut, bool bLast )
	{
		if ( m_iPos < 0 )
		{
			sOut << "..";
			return;
		}

		auto fnPrintHit = [&sOut,bLast] (const WordVariantHit_t& dHit) {
			ScopedComma_c _ ( sOut, dEmptyBl );
			sOut << dHit.m_tWord;
			if ( !bLast && dHit.m_bIsLast )
				sOut << "<EOF>";
			if ( bLast && !dHit.m_bIsLast )
				sOut << "...";
		};

		if ( m_dHits.GetLength()==1 )
			fnPrintHit(m_dHits[0]);
		else
		{
			ScopedComma_c sDivider ( sOut, StrBlock_t { FROMS ( "|" ), FROMS ( "[" ), FROMS ( "]" ) } );
			for ( const auto& dHit : m_dHits )
				fnPrintHit(dHit);
		}
	}
};

struct DocField_t {
	int m_iField = -1;
	CSphVector<WordHit_t> m_dHits;
};

inline static void FormatWordHit ( JsonEscapedBuilder& dOut, const Wordid_t& tWord, int iField, int iPos, bool bIsLast )
{
	auto tObj = dOut.Object();
	dOut << tWord;
	dOut.NamedVal ( "field", iField );
	dOut.NamedVal ( "pos", iPos );
	dOut.NamedValNonDefault ( "is_last", bIsLast, false );
}

void DiskIndexChecker_c::Impl_c::ExtractDocs ()
{
	assert ( m_tReporter.GetExtractDocs() );
	auto uDocID = *m_tReporter.GetExtractDocs();
	auto iRowID = GetRowidByDocid ( uDocID );
	bool bIsKilled = ( INVALID_ROWID == CheckIfKilled ( iRowID ) );

	CSphVector<DocField_t> dFields;

	if ( iRowID!=INVALID_ROWID )
	{
		m_tReporter.Msg ( "\n# Restored document\n## Cloud of tokens\n\n```json\n[" );
		CheckDocs ( [&dFields, iRowID, bIsNotFirst=false, this] ( RowID_t tRow, Wordid_t tWord, int iField, int iPos, bool bIsLast ) mutable {

			if ( iRowID!=tRow )
				return;

			if ( dFields.GetLength() < iField + 1 )
				dFields.Resize ( iField + 1 );
			auto& dField = dFields[iField];
			dField.m_iField = iField;

			if ( dField.m_dHits.GetLength() < iPos )
				dField.m_dHits.Resize ( iPos );
			auto& dHit = dField.m_dHits[iPos - 1];

			dHit.AddWord ( tWord, bIsLast );
			dHit.m_iPos = iPos;

			JsonEscapedBuilder sReport;
			if ( std::exchange ( bIsNotFirst, true ) )
				sReport << ',';
			FormatWordHit ( sReport, tWord, iField, iPos, bIsLast );
			m_tReporter.Msg ( "%s", sReport.cstr() );
		});
		m_tReporter.Msg ( "]\n```\n" );
	}


	StringBuilder_c sReport;
	if ( iRowID == INVALID_ROWID )
		sReport << "* Document " << uDocID << " is not found";
	else {
		sReport << "## Document " << uDocID << "\n* RowID " << iRowID << ( bIsKilled ? " (killed)\n" : "\n" );
		ARRAY_FOREACH ( i, dFields )
		{
			const DocField_t& dField = dFields[i];
			const CSphColumnInfo& tCol = m_tSchema.GetField ( i );
			if ( dField.m_iField < 0 )
				sReport << "\n### Field '" << tCol.m_sName << "' is empty.\n";
			else {
				sReport << "\n### Field '" << tCol.m_sName << "'\n";
				ScopedComma_c tSpacer ( sReport, StrBlock_t { FROMS ( " " ), FROMS ( "" ), FROMS ( "\n" ) } );
				ARRAY_FOREACH ( k, dField.m_dHits )
					dField.m_dHits[k].Print ( sReport, k==dField.m_dHits.GetLength()-1 );
			}
		}
	}
	sReport << "\n--- <End of restored document> ---";
	m_tReporter.Msg ( "%s", sReport.cstr() );

}

void DiskIndexChecker_c::Impl_c::Check()
{
	if ( m_tReporter.GetExtractDocs() )
		return ExtractDocs();
	CheckSchema();
	CheckDictionary();
	CheckDocs();
	CheckAttributes();
	CheckBlockIndex();
	CheckColumnar();
	CheckKillList();
	CheckDocstore();

	DebugCheck_DeadRowMap ( m_tDeadRowReader.GetFilesize(), m_iNumRows, m_tReporter );
	CheckDocidLookup();

	if ( m_bCheckIdDups )
		CheckDocids();
}


void DiskIndexChecker_c::Impl_c::CheckDictionary()
{
	m_tReporter.Msg ( "checking dictionary..." );

	const CSphIndexSettings & tIndexSettings = m_tIndex.GetSettings();
	const auto & tDictSettings = m_tIndex.GetDictionary()->GetSettings();
	const DictFormat_e eDictFormat = tDictSettings.GetDictFormat();

	SphWordID_t uWordid = 0;
	SphOffset_t iDoclistOffset = 0;
	int iWordsTotal = 0;

	CSphVector<BYTE> dWord;
	dWord.Resize ( GetKeywordBufSize ( eDictFormat ) );
	dWord[0] = '\0';

	CSphVector<BYTE> dLastWord;
	dLastWord.Resize ( GetKeywordBufSize ( eDictFormat ) );
	dLastWord[0] = '\0';
	int iLastWordLen = 0;

	const int iWordPerCP = SPH_WORDLIST_CHECKPOINT;
	const bool bWordDict = tDictSettings.IsWordDict();
	const bool bLegacySkiplistLayout = IsLegacyHitlessLayoutForCheck ( m_uVersion );

	CSphVector<CSphWordlistCheckpoint> dCheckpoints;
	dCheckpoints.Reserve ( m_tWordlist.m_iDictCheckpoints );
	CSphVector<char> dCheckpointWords;

	CSphAutoreader & tDictReader = m_tDictReader;

	tDictReader.GetByte();
	SphOffset_t iLastSkipsOffset = 0;
	SphOffset_t iWordsEnd = m_tWordlist.GetWordsEnd();

	while ( tDictReader.GetPos()!=iWordsEnd && !m_bIsEmpty )
	{
		// sanity checks
		if ( tDictReader.GetPos()>=iWordsEnd )
		{
			m_tReporter.Fail ( "reading past checkpoints" );
			break;
		}

		// store current entry pos (for checkpointing later), read next delta
		const int64_t iDictPos = tDictReader.GetPos();
		DictEntryForCheck_t tEntry;
		DictReadResult_e eReadResult = ReadDictEntryForCheck ( tDictReader, eDictFormat, tIndexSettings, m_uVersion, bLegacySkiplistLayout, m_dHitlessWords, uWordid, iDoclistOffset, dWord, tEntry, m_tReporter );

		// checkpoint encountered, handle it
		if ( eReadResult==DictReadResult_e::CHECKPOINT )
		{
			if ( ( iWordsTotal%iWordPerCP )!=0 && tDictReader.GetPos()!=iWordsEnd )
				m_tReporter.Fail ( "unexpected checkpoint (pos=" INT64_FMT ", word=%d, words=%d, expected=%d)", iDictPos, iWordsTotal, ( iWordsTotal%iWordPerCP ), iWordPerCP );
			continue;
		}

		if ( bWordDict )
		{
			if ( tEntry.m_iWordLen==0 )
				m_tReporter.Fail ( "empty word in dictionary (pos=" INT64_FMT ")", iDictPos );

			if ( iLastWordLen && tEntry.m_iWordLen )
				if ( sphDictCmpStrictly ( tEntry.m_szWord, tEntry.m_iWordLen, (const char*)dLastWord.Begin(), iLastWordLen )<=0 )
					m_tReporter.Fail ( "word order decreased (pos=" INT64_FMT ", word=%s, prev=%s)", iDictPos, (const char*)dLastWord.Begin(), tEntry.m_szWord );

			if ( tEntry.m_iHint<0 )
				m_tReporter.Fail ( "invalid word hint (pos=" INT64_FMT ", word=%s, hint=%d)", iDictPos, tEntry.m_szWord, tEntry.m_iHint );

			if ( tEntry.m_iDocs<=0 || tEntry.m_iHits<=0 || tEntry.m_iHits<tEntry.m_iDocs )
				m_tReporter.Fail ( "invalid docs/hits (pos=" INT64_FMT ", word=%s, docs=" INT64_FMT ", hits=" INT64_FMT ")", (int64_t)iDictPos, tEntry.m_szWord, (int64_t)tEntry.m_iDocs, (int64_t)tEntry.m_iHits );

			memcpy ( dLastWord.Begin(), dWord.Begin(), tEntry.m_iWordLen+1 );
			iLastWordLen = tEntry.m_iWordLen;
		} else
		{
			if ( tEntry.m_uWordid<=uWordid )
				m_tReporter.Fail ( "wordid decreased (pos=" INT64_FMT ", wordid=" UINT64_FMT ", previd=" UINT64_FMT ")", (int64_t)iDictPos, (uint64_t)tEntry.m_uWordid, (uint64_t)uWordid );

			if ( tEntry.m_iDoclistOffset<=iDoclistOffset )
				m_tReporter.Fail ( "doclist offset decreased (pos=" INT64_FMT ", wordid=" UINT64_FMT ")", (int64_t)iDictPos, (uint64_t)tEntry.m_uWordid );

			if ( tEntry.m_iDocs<=0 || tEntry.m_iHits<=0 || tEntry.m_iHits<tEntry.m_iDocs )
				m_tReporter.Fail ( "invalid docs/hits (pos=" INT64_FMT ", wordid=" UINT64_FMT ", docs=" INT64_FMT ", hits=" INT64_FMT ", hitless=%s)",
					(int64_t)iDictPos, (uint64_t)tEntry.m_uWordid, (int64_t)tEntry.m_iDocs, (int64_t)tEntry.m_iHits, ( tEntry.m_bHitless?"true":"false" ) );
		}

		// Stream consumption depends on docs count in dictionary layout.
		// Logical hitless masking must not affect optional field parsing.
		if ( tEntry.m_iLayoutDocs>tIndexSettings.m_iSkiplistBlockSize )
		{
			if ( !bWordDict && tEntry.m_iSkiplistOffset<iLastSkipsOffset )
				m_tReporter.Fail ( "descending skiplist pos (last=" INT64_FMT ", cur=" INT64_FMT ", wordid=" UINT64_FMT ")", (int64_t)iLastSkipsOffset, (int64_t)tEntry.m_iSkiplistOffset, UINT64 ( tEntry.m_uWordid ) );

			iLastSkipsOffset = tEntry.m_iSkiplistOffset;
		}

		// update stats, add checkpoint
		if ( ( iWordsTotal%iWordPerCP )==0 )
		{
			CSphWordlistCheckpoint & tCP = dCheckpoints.Add();
			tCP.m_iWordlistOffset = iDictPos;

			if ( bWordDict )
			{
				auto iLen = tEntry.m_iWordLen;
				char * sArenaWord = dCheckpointWords.AddN ( iLen + 1 );
				memcpy ( sArenaWord, tEntry.m_szWord, iLen );
				sArenaWord[iLen] = '\0';
				tCP.m_uWordID = sArenaWord - dCheckpointWords.Begin();
			} else
				tCP.m_uWordID = tEntry.m_uWordid;
		}

		// TODO add back infix checking

		uWordid = tEntry.m_uWordid;
		iDoclistOffset = tEntry.m_iDoclistOffset;
		iWordsTotal++;
	}

	// check the checkpoints
	if ( dCheckpoints.GetLength()!=m_tWordlist.m_iDictCheckpoints )
		m_tReporter.Fail ( "checkpoint count mismatch (read=" INT64_FMT ", calc=" INT64_FMT ")", m_tWordlist.m_iDictCheckpoints, dCheckpoints.GetLength64() );

	m_tWordlist.DebugPopulateCheckpoints();
	for ( int64_t i=0; i < Min ( dCheckpoints.GetLength64(), m_tWordlist.m_iDictCheckpoints ); i++ )
	{
		CSphWordlistCheckpoint tRefCP = dCheckpoints[i];
		const CSphWordlistCheckpoint & tCP = m_tWordlist.m_dCheckpoints[i];
		const int iLen = bWordDict ? (int) strlen ( tCP.m_szWord ) : 0;
		if ( bWordDict )
			tRefCP.m_szWord = dCheckpointWords.Begin() + tRefCP.m_uWordID;
		if ( bWordDict && ( tRefCP.m_szWord[0]=='\0' || tCP.m_szWord[0]=='\0' ) )
		{
			m_tReporter.Fail ( "empty checkpoint %d (read_word=%s, read_len=%u, readpos=" INT64_FMT ", calc_word=%s, calc_len=%u, calcpos=" INT64_FMT ")",
				i, tCP.m_szWord, (DWORD)strlen ( tCP.m_szWord ), (int64_t)tCP.m_iWordlistOffset,
				tRefCP.m_szWord, (DWORD)strlen ( tRefCP.m_szWord ), (int64_t)tRefCP.m_iWordlistOffset );

		} else if ( sphCheckpointCmpStrictly ( tCP.m_szWord, iLen, tCP.m_uWordID, bWordDict, tRefCP ) || tRefCP.m_iWordlistOffset!=tCP.m_iWordlistOffset )
		{
			if ( bWordDict )
			{
				m_tReporter.Fail ( "checkpoint %d differs (read_word=%s, readpos=" INT64_FMT ", calc_word=%s, calcpos=" INT64_FMT ")",
					i,
					tCP.m_szWord,
					(int64_t)tCP.m_iWordlistOffset,
					tRefCP.m_szWord,
					(int64_t)tRefCP.m_iWordlistOffset );
			} else
			{
				m_tReporter.Fail ( "checkpoint %d differs (readid=" UINT64_FMT ", readpos=" INT64_FMT ", calcid=" UINT64_FMT ", calcpos=" INT64_FMT ")",
					i,
					(uint64_t)tCP.m_uWordID,
					(int64_t)tCP.m_iWordlistOffset,
					(uint64_t)tRefCP.m_uWordID,
					(int64_t)tRefCP.m_iWordlistOffset );
			}
		}
	}

	dCheckpoints.Reset();
	dCheckpointWords.Reset();
}


void DiskIndexChecker_c::Impl_c::CheckDocs( cbWordidFn&& fnCbWordid )
{
	const CSphIndexSettings & tIndexSettings = m_tIndex.GetSettings();
	const bool bLegacySkiplistLayout = IsLegacyHitlessLayoutForCheck ( m_uVersion );
	const auto & tDictSettings = m_tIndex.GetDictionary()->GetSettings();
	const DictFormat_e eDictFormat = tDictSettings.GetDictFormat();

	if ( !fnCbWordid )
		m_tReporter.Msg ( "checking data..." );

	int64_t iDocsSize = m_pDocsReader->GetFilesize();
	int64_t iSkiplistLen = m_tSkipsReader.GetFilesize();

	m_tDictReader.SeekTo ( 1, READ_NO_SIZE_HINT );
	m_pDocsReader->SeekTo ( 1 );
	m_pHitsReader->SeekTo ( 1 );

	SphWordID_t uWordid = 0;
	SphOffset_t iDoclistOffset = 0;

	const bool bWordDict = tDictSettings.IsWordDict();

	Wordid_t tCbWordid;
	tCbWordid.m_bWordDict = bWordDict;

	CSphVector<BYTE> dWord;
	dWord.Resize ( GetKeywordBufSize ( eDictFormat ) );
	dWord[0] = '\0';

	int iWordsChecked = 0;
	int iWordsTotal = 0;

	SphOffset_t iWordsEnd = m_tWordlist.GetWordsEnd();
	while ( m_tDictReader.GetPos()<iWordsEnd )
	{
		DictEntryForCheck_t tEntry;
		if ( ReadDictEntryForCheck ( m_tDictReader, eDictFormat, tIndexSettings, m_uVersion, bLegacySkiplistLayout, m_dHitlessWords, uWordid, iDoclistOffset, dWord, tEntry, m_tReporter )==DictReadResult_e::CHECKPOINT )
			continue;

		if ( bWordDict )
		{
			tCbWordid.m_szWordid = tEntry.m_szWord;
		} else
		{
			tCbWordid.m_uWordid = tEntry.m_uWordid;
		}

		uWordid = tEntry.m_uWordid;
		iDoclistOffset = tEntry.m_iDoclistOffset;
		bool bHitless = tEntry.m_bHitless;

		// check whether the offset is as expected
		if ( iDoclistOffset!=m_pDocsReader->GetPos() )
		{
			if ( !bWordDict )
				m_tReporter.Fail ( "unexpected doclist offset (wordid=" UINT64_FMT "(%s)(%d), dictpos=" INT64_FMT ", doclistpos=" INT64_FMT ")",
					(uint64_t)uWordid, tEntry.m_szWord, iWordsChecked, iDoclistOffset, (int64_t) m_pDocsReader->GetPos() );

			if ( iDoclistOffset>=iDocsSize || iDoclistOffset<0 )
			{
				m_tReporter.Fail ( "unexpected doclist offset, off the file (wordid=" UINT64_FMT "(%s)(%d), dictpos=" INT64_FMT ", doclistsize=" INT64_FMT ")",
					(uint64_t)uWordid, tEntry.m_szWord, iWordsChecked, iDoclistOffset, iDocsSize );
				iWordsChecked++;
				continue;
			} else
				m_pDocsReader->SeekTo ( iDoclistOffset );
		}

		// create and manually setup doclist reader
		DiskIndexQwordTraits_c * pQword = sphCreateDiskIndexQword ( tIndexSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE );

		pQword->m_tDoc.Reset ( m_tSchema.GetDynamicSize() );
		pQword->m_tDoc.m_tRowID = INVALID_ROWID;
		pQword->m_iDocs = 0;
		pQword->m_iHits = 0;
		pQword->SetDocReader ( m_pDocsReader );
//		pQword->m_rdDoclist.SeekTo ( tDocsReader.GetPos(), READ_NO_SIZE_HINT );
		pQword->SetHitReader ( m_pHitsReader );
//		pQword->m_rdHitlist.SeekTo ( tHitsReader.GetPos(), READ_NO_SIZE_HINT );

		// loop the doclist
		int iDoclistDocs = 0;
		int iDoclistHits = 0;
		int iHitlistHits = 0;

		bHitless |= ( tIndexSettings.m_eHitless==SPH_HITLESS_ALL ||
			( tIndexSettings.m_eHitless==SPH_HITLESS_SOME && m_dHitlessWords.BinarySearch ( uWordid ) ) );
		pQword->m_bHasHitlist = !bHitless;

		CSphVector<SkiplistEntry_t> dDoclistSkips;
		while (true)
		{
			// skiplist state is saved just *before* decoding those boundary entries
			if ( ( iDoclistDocs & ( tIndexSettings.m_iSkiplistBlockSize-1 ) )==0 )
			{
				SkiplistEntry_t & tBlock = dDoclistSkips.Add();
				tBlock.m_tBaseRowIDPlus1 = pQword->m_tDoc.m_tRowID+1;
				tBlock.m_iOffset = pQword->m_rdDoclist->GetPos();
				tBlock.m_iBaseHitlistPos = pQword->m_uHitPosition;
			}

			// FIXME? this can fail on a broken entry (eg fieldid over 256)
			const CSphMatch & tDoc = pQword->GetNextDoc();
			if ( tDoc.m_tRowID==INVALID_ROWID )
				break;

			// checks!
			if ( tDoc.m_tRowID>m_iNumRows )
				m_tReporter.Fail ( "rowid out of bounds (wordid=" UINT64_FMT "(%s), rowid=%u)",	uint64_t(uWordid), tEntry.m_szWord, tDoc.m_tRowID );

			++iDoclistDocs;
			iDoclistHits += pQword->m_uMatchHits;

			// check position in case of regular (not-inline) hit
			if (!( pQword->m_iHitlistPos>>63 ))
			{
				if ( !bWordDict && pQword->m_iHitlistPos!=pQword->m_rdHitlist->GetPos() )
					m_tReporter.Fail ( "unexpected hitlist offset (wordid=" UINT64_FMT "(%s), rowid=%u, expected=" INT64_FMT ", actual=" INT64_FMT ")",
						(uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, (int64_t)pQword->m_iHitlistPos, (int64_t)pQword->m_rdHitlist->GetPos() );
			}

			// aim
			pQword->SeekHitlist ( pQword->m_iHitlistPos );

			// loop the hitlist
			int iDocHits = 0;
			FieldMask_t dFieldMask;
			dFieldMask.UnsetAll();
			Hitpos_t uLastHit = EMPTY_HIT;

			while ( !bHitless )
			{
				Hitpos_t uHit = pQword->GetNextHit();
				if ( uHit==EMPTY_HIT )
					break;

				if ( !( uLastHit<uHit ) )
					m_tReporter.Fail ( "hit entries sorting order decreased (wordid=" UINT64_FMT "(%s), rowid=%u, hit=%u, last=%u)", (uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, uHit, uLastHit );

				if ( HITMAN::GetField ( uLastHit )==HITMAN::GetField ( uHit ) )
				{
					if ( !( HITMAN::GetPos ( uLastHit )<HITMAN::GetPos ( uHit ) ) )
						m_tReporter.Fail ( "hit decreased (wordid=" UINT64_FMT "(%s), rowid=%u, hit=%u, last=%u)",	(uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, HITMAN::GetPos ( uHit ), HITMAN::GetPos ( uLastHit ) );

					if ( HITMAN::IsEnd ( uLastHit ) )
						m_tReporter.Msg ( "WARNING, multiple tail hits (wordid=" UINT64_FMT "(%s), rowid=%u, hit=0x%x, last=0x%x)", (uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, uHit, uLastHit );
				} else
				{
					if ( !( HITMAN::GetField ( uLastHit )<HITMAN::GetField ( uHit ) ) )
						m_tReporter.Fail ( "hit field decreased (wordid=" UINT64_FMT "(%s), rowid=%u, hit field=%u, last field=%u)", (uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, HITMAN::GetField ( uHit ), HITMAN::GetField ( uLastHit ) );
				}

				if ( fnCbWordid )
					fnCbWordid ( tDoc.m_tRowID, tCbWordid, HITMAN::GetField ( uHit ), HITMAN::GetPos ( uHit ), HITMAN::IsEnd ( uHit ) );

				uLastHit = uHit;

				int iField = HITMAN::GetField ( uHit );
				if ( iField<0 || iField>=SPH_MAX_FIELDS )
					m_tReporter.Fail ( "hit field out of bounds (wordid=" UINT64_FMT "(%s), rowid=%u, field=%d)", (uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, iField );
				else if ( iField>=m_tSchema.GetFieldsCount() )
					m_tReporter.Fail ( "hit field out of schema (wordid=" UINT64_FMT "(%s), rowid=%u, field=%d)", (uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, iField );
				else
					dFieldMask.Set(iField);

				++iDocHits; // to check doclist entry
				++iHitlistHits; // to check dictionary entry
			}

			// check hit count
			if ( iDocHits!=(int)pQword->m_uMatchHits && !bHitless )
				m_tReporter.Fail ( "doc hit count mismatch (wordid=" UINT64_FMT "(%s), rowid=%u, doclist=%d, hitlist=%d)", (uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID, pQword->m_uMatchHits, iDocHits );

			if ( m_tSchema.GetFieldsCount()>32 )
				pQword->CollectHitMask();

			// check the mask
			if ( memcmp ( dFieldMask.m_dMask, pQword->m_dQwordFields.m_dMask, sizeof(dFieldMask.m_dMask) ) && !bHitless )
				m_tReporter.Fail ( "field mask mismatch (wordid=" UINT64_FMT "(%s), rowid=%u)", (uint64_t)uWordid, tEntry.m_szWord, pQword->m_tDoc.m_tRowID );

			// update my hitlist reader
			m_pHitsReader->SeekTo ( pQword->m_rdHitlist->GetPos() );
		}

		// do checks
		if ( tEntry.m_iDocs!=iDoclistDocs )
			m_tReporter.Fail ( "doc count mismatch (wordid=" UINT64_FMT "(%s), dict=%d, doclist=%d, hitless=%s)", uint64_t(uWordid), tEntry.m_szWord, tEntry.m_iDocs, iDoclistDocs, ( bHitless?"true":"false" ) );

		if ( ( tEntry.m_iHits!=iDoclistHits || tEntry.m_iHits!=iHitlistHits ) && !bHitless )
			m_tReporter.Fail ( "hit count mismatch (wordid=" UINT64_FMT "(%s), dict=%d, doclist=%d, hitlist=%d)", uint64_t(uWordid), tEntry.m_szWord, tEntry.m_iHits, iDoclistHits, iHitlistHits );

		while ( iDoclistDocs>tIndexSettings.m_iSkiplistBlockSize && !bHitless )
		{
			if ( tEntry.m_iSkiplistOffset<=0 || tEntry.m_iSkiplistOffset>iSkiplistLen )
			{
				m_tReporter.Fail ( "invalid skiplist offset (wordid=" UINT64_FMT "(%s), off=" INT64_FMT ", max=" INT64_FMT ")", UINT64 ( uWordid ), tEntry.m_szWord, (int64_t)tEntry.m_iSkiplistOffset, iSkiplistLen );
				break;
			}

			// boundary adjustment
			if ( ( iDoclistDocs & ( tIndexSettings.m_iSkiplistBlockSize-1 ) )==0 )
				dDoclistSkips.Pop();

			SkiplistEntry_t t;
			t.m_tBaseRowIDPlus1 = 0;
			t.m_iOffset = iDoclistOffset;
			t.m_iBaseHitlistPos = 0;

			// hint is: dDoclistSkips * ZIPPED( sizeof(int64_t) * 3 ) == dDoclistSkips * 8
			m_tSkipsReader.SeekTo ( tEntry.m_iSkiplistOffset, dDoclistSkips.GetLength ()*8 );
			int i = 0;
			while ( ++i<dDoclistSkips.GetLength() )
			{
				const SkiplistEntry_t & r = dDoclistSkips[i];

				RowID_t tRowIDDelta = m_tSkipsReader.UnzipRowid();
				uint64_t uOff = m_tSkipsReader.UnzipOffset();
				uint64_t uPosDelta = m_tSkipsReader.UnzipOffset();

				if ( m_tSkipsReader.GetErrorFlag () )
				{
					m_tReporter.Fail ( "skiplist reading error (wordid=" UINT64_FMT "(%s), exp=%d, got=%d, error='%s')", UINT64 ( uWordid ), tEntry.m_szWord, i, dDoclistSkips.GetLength (), m_tSkipsReader.GetErrorMessage ().cstr () );
					m_tSkipsReader.ResetError();
					break;
				}

				t.m_tBaseRowIDPlus1 += tIndexSettings.m_iSkiplistBlockSize + tRowIDDelta;
				t.m_iOffset += 4*tIndexSettings.m_iSkiplistBlockSize + uOff;
				t.m_iBaseHitlistPos += uPosDelta;
				if ( t.m_tBaseRowIDPlus1!=r.m_tBaseRowIDPlus1 || t.m_iOffset!=r.m_iOffset || t.m_iBaseHitlistPos!=r.m_iBaseHitlistPos )
				{
					m_tReporter.Fail ( "skiplist entry %d mismatch (wordid=" UINT64_FMT "(%s), exp={%u, " UINT64_FMT ", " UINT64_FMT "}, got={%u, " UINT64_FMT ", " UINT64_FMT "})",
						i, UINT64 ( uWordid ), tEntry.m_szWord,
						r.m_tBaseRowIDPlus1, UINT64 ( r.m_iOffset ), UINT64 ( r.m_iBaseHitlistPos ),
						t.m_tBaseRowIDPlus1, UINT64 ( t.m_iOffset ), UINT64 ( t.m_iBaseHitlistPos ) );
					break;
				}
			}
			break;
		}

		// move my reader instance forward too
		m_pDocsReader->SeekTo ( pQword->m_rdDoclist->GetPos() );

		// cleanup
		SafeDelete ( pQword );

		// progress bar
		iWordsTotal++;
		if ( (++iWordsChecked)%1000==0 )
			m_tReporter.Progress ( "%d/%d", iWordsChecked, iWordsTotal );
	}
}


void DiskIndexChecker_c::Impl_c::CheckAttributes()
{
	if ( !m_tSchema.HasNonColumnarAttrs() )
		return;

	const int64_t iMinMaxStart = sizeof(DWORD) * m_iMinMaxIndex;
	const int64_t iMinMaxEnd = sizeof(DWORD) * m_iMinMaxIndex + sizeof(DWORD) * ( m_iDocinfoIndex+1 ) * m_tSchema.GetRowSize() * 2;
	const int64_t iMinMaxBytes = iMinMaxEnd - iMinMaxStart;

	FileDebugCheckReader_c tAttrReader ( &m_tAttrReader );
	FileDebugCheckReader_c tBlobReader ( m_bHasBlobs ? &m_tBlobReader : nullptr );

	// common code with RT index
	DebugCheck_Attributes ( tAttrReader, tBlobReader, m_iNumRows, iMinMaxBytes, m_tSchema, m_tReporter );
}


void DiskIndexChecker_c::Impl_c::CheckKillList() const
{
	m_tReporter.Msg ( "checking kill-list..." );

	CSphString sSPK = GetFilename(SPH_EXT_SPK);
	if ( !sphIsReadable ( sSPK.cstr() ) )
		return;

	CSphString sError;
	CSphAutoreader tReader;
	if ( !tReader.Open ( sSPK.cstr(), sError ) )
	{
		m_tReporter.Fail ( "unable to open kill-list: %s", sError.cstr() );
		return;
	}

	DWORD nIndexes = tReader.GetDword();
	for ( int i = 0; i < (int)nIndexes; i++ )
	{
		CSphString sIndex = tReader.GetString();
		if ( tReader.GetErrorFlag() )
		{
			m_tReporter.Fail ( "error reading table name from kill-list: %s", tReader.GetErrorMessage().cstr() );
			return;
		}

		DWORD uFlags = tReader.GetDword();
		DWORD uMask = KillListTarget_t::USE_KLIST | KillListTarget_t::USE_DOCIDS;
		if ( uFlags & (~uMask) )
		{
			m_tReporter.Fail ( "unknown table flags in kill-list: %u", uMask );
			return;
		}
	}

	DWORD nKills = tReader.GetDword();
	if ( tReader.GetErrorFlag() )
	{
		m_tReporter.Fail ( "error reading kill-list" );
		return;
	}

	for ( DWORD i = 0; i<nKills; i++ )
	{
		DocID_t tDelta = tReader.UnzipOffset();
		if ( tDelta<=0 )
		{
			m_tReporter.Fail ( "descending docids found in kill-list" );
			return;
		}

		if ( tReader.GetErrorFlag() )
		{
			m_tReporter.Fail ( "error docids from kill-list" );
			return;
		}
	}
}


void DiskIndexChecker_c::Impl_c::CheckBlockIndex()
{
	if ( !m_tSchema.HasNonColumnarAttrs() )
		return;

	m_tReporter.Msg ( "checking attribute blocks index..." );

	int64_t iAllRowsTotal = m_iNumRows + (m_iDocinfoIndex+1)*2;
	DWORD uStride = m_tSchema.GetRowSize();
	int64_t iLoadedRowItems = m_tAttrReader.GetFilesize() / sizeof(CSphRowitem);
	if ( iAllRowsTotal*uStride>iLoadedRowItems && m_iNumRows )
		m_tReporter.Fail ( "rowitems count mismatch (expected=" INT64_FMT ", loaded=" INT64_FMT ")", iAllRowsTotal*uStride, iLoadedRowItems );

	// check size
	const int64_t iTempDocinfoIndex = ( m_iNumRows+DOCINFO_INDEX_FREQ-1 ) / DOCINFO_INDEX_FREQ;
	if ( iTempDocinfoIndex!=m_iDocinfoIndex )
		m_tReporter.Fail ( "block count differs (expected=" INT64_FMT ", got=" INT64_FMT ")", iTempDocinfoIndex, m_iDocinfoIndex );

	CSphFixedVector<CSphRowitem> dRow ( m_tSchema.GetRowSize() );
	const CSphRowitem * pRow = dRow.Begin();
	m_tAttrReader.SeekTo ( 0, (int) dRow.GetLengthBytes() );

	const int64_t iMinMaxEnd = sizeof(DWORD) * m_iMinMaxIndex + sizeof(DWORD) * ( m_iDocinfoIndex+1 ) * uStride * 2;
	CSphFixedVector<DWORD> dMinMax ( uStride*2 );
	const DWORD * pMinEntry = dMinMax.Begin();
	const DWORD * pMinAttrs = pMinEntry;
	const DWORD * pMaxAttrs = pMinAttrs + uStride;

	for ( int64_t iIndexEntry=0; iIndexEntry<m_iNumRows; iIndexEntry++ )
	{
		const int64_t iBlock = iIndexEntry / DOCINFO_INDEX_FREQ;

		// we have to do some checks in border cases, for example: when move from 1st to 2nd block
		const int64_t iPrevEntryBlock = ( iIndexEntry-1 )/DOCINFO_INDEX_FREQ;
		const bool bIsBordersCheckTime = ( iPrevEntryBlock!=iBlock );
		if ( bIsBordersCheckTime || iIndexEntry==0 )
		{
			int64_t iPos = m_tAttrReader.GetPos();

			int64_t iBlockPos = sizeof(DWORD) * m_iMinMaxIndex + sizeof(DWORD) * iBlock * uStride * 2;
			// check docid vs global range
			if ( int64_t( iBlockPos + sizeof(DWORD) * uStride) > iMinMaxEnd )
				m_tReporter.Fail ( "unexpected block index end (row=" INT64_FMT ", block=" INT64_FMT ")", iIndexEntry, iBlock );

			m_tAttrReader.SeekTo ( iBlockPos, (int) dMinMax.GetLengthBytes() );
			m_tAttrReader.GetBytes ( dMinMax.Begin(), (int) dMinMax.GetLengthBytes() );
			if ( m_tAttrReader.GetErrorFlag() )
				m_tReporter.Fail ( "unexpected block index (row=" INT64_FMT ", block=" INT64_FMT ")", iIndexEntry, iBlock );

			m_tAttrReader.SeekTo ( iPos, (int) dRow.GetLengthBytes() );
		}

		m_tAttrReader.GetBytes ( dRow.Begin(), (int) dRow.GetLengthBytes() );
		const DocID_t tDocID = sphGetDocID(pRow);

		// check values vs blocks range
		for ( int iItem=0; iItem < m_tSchema.GetAttrsCount(); iItem++ )
		{
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(iItem);
			if ( tCol.m_sName==sphGetBlobLocatorName() || tCol.IsColumnar() )
				continue;

			switch ( tCol.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_BIGINT:
			{
				const SphAttr_t uVal = sphGetRowAttr ( pRow, tCol.m_tLocator );
				const SphAttr_t uMin = sphGetRowAttr ( pMinAttrs, tCol.m_tLocator );
				const SphAttr_t uMax = sphGetRowAttr ( pMaxAttrs, tCol.m_tLocator );

				// checks is attribute min max range valid
				if ( uMin > uMax && bIsBordersCheckTime )
					m_tReporter.Fail ( "invalid attribute range (row=" INT64_FMT ", block=" INT64_FMT ", min=" INT64_FMT ", max=" INT64_FMT ")", iIndexEntry, iBlock, uMin, uMax );

				if ( uVal < uMin || uVal > uMax )
					m_tReporter.Fail ( "unexpected attribute value (row=" INT64_FMT ", attr=%u, docid=" INT64_FMT ", block=" INT64_FMT ", value=0x" UINT64_FMT ", min=0x" UINT64_FMT ", max=0x" UINT64_FMT ")",
						iIndexEntry, iItem, tDocID, iBlock, uint64_t(uVal), uint64_t(uMin), uint64_t(uMax) );
			}
			break;

			case SPH_ATTR_FLOAT:
			{
				const float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pRow, tCol.m_tLocator ) );
				const float fMin = sphDW2F ( (DWORD)sphGetRowAttr ( pMinAttrs, tCol.m_tLocator ) );
				const float fMax = sphDW2F ( (DWORD)sphGetRowAttr ( pMaxAttrs, tCol.m_tLocator ) );

				// checks is attribute min max range valid
				if ( fMin > fMax && bIsBordersCheckTime )
					m_tReporter.Fail ( "invalid attribute range (row=" INT64_FMT ", block=" INT64_FMT ", min=%f, max=%f)", iIndexEntry, iBlock, fMin, fMax );

				if ( fVal < fMin || fVal > fMax )
					m_tReporter.Fail ( "unexpected attribute value (row=" INT64_FMT ", attr=%u, docid=" INT64_FMT ", block=" INT64_FMT ", value=%f, min=%f, max=%f)", iIndexEntry, iItem, tDocID, iBlock, fVal, fMin, fMax );
			}
			break;

			default:
				break;
			}
		}

		// progress bar
		if ( iIndexEntry%1000==0 )
			m_tReporter.Progress ( INT64_FMT"/" INT64_FMT, iIndexEntry, m_iNumRows );
	}
}


void DiskIndexChecker_c::Impl_c::CheckColumnar()
{
	if ( !m_tSchema.HasColumnarAttrs() )
		return;

	m_tReporter.Msg ( "checking columnar storage..." );

	CheckColumnarStorage ( GetFilename(SPH_EXT_SPC), (DWORD)m_iNumRows,
		[this]( const char * szError ){ m_tReporter.Fail ( "\n%s", szError ); },
		[this]( const char * szProgress ){ m_tReporter.Progress ( "%s", szProgress ); } );
}


void DiskIndexChecker_c::Impl_c::CheckDocidLookup()
{
	CSphString sError;
	m_tReporter.Msg ( "checking doc-id lookup..." );

	CSphAutoreader tLookup;
	if ( !tLookup.Open ( GetFilename(SPH_EXT_SPT), sError ) )
	{
		// only if index not empty
		if ( m_iNumRows )
			m_tReporter.Fail ( "unable to lookup file: %s", sError.cstr() );

		return;
	}
	int64_t iLookupEnd = tLookup.GetFilesize();

	const CSphColumnInfo * pId = m_tSchema.GetAttr("id");
	assert(pId);

	CSphFixedVector<CSphRowitem> dRow ( m_tSchema.GetRowSize() );
	m_tAttrReader.SeekTo ( 0, (int) dRow.GetLengthBytes() );
	CSphBitvec dRowids ( m_iNumRows );

	int64_t iDocs = tLookup.GetDword();
	int64_t iDocsPerCheckpoint = tLookup.GetDword();
	tLookup.GetOffset(); // max docid
	SphOffset_t tUuidEntriesOffset = m_uVersion>=71 ? tLookup.GetOffset() : 0;
	int64_t iLookupBase = tLookup.GetPos();

	int iCheckpoints = ( iDocs + iDocsPerCheckpoint - 1 ) / iDocsPerCheckpoint;

	DocidLookupCheckpoint_t tCp;
	DocID_t tLastDocID = 0;
	int iCp = 0;
	while ( tLookup.GetPos()<iLookupEnd && iCp<iCheckpoints )
	{
		tLookup.SeekTo ( sizeof(DocidLookupCheckpoint_t) * iCp + iLookupBase, sizeof(DocidLookupCheckpoint_t) );

		DocidLookupCheckpoint_t tPrevCp = tCp;
		tCp.m_tBaseDocID = tLookup.GetOffset();
		tCp.m_tOffset = tLookup.GetOffset();
		tLastDocID = tCp.m_tBaseDocID;

		if ( (uint64_t)tPrevCp.m_tBaseDocID>=(uint64_t)tCp.m_tBaseDocID )
			m_tReporter.Fail ( "descending docid at checkpoint %d, previous docid " UINT64_FMT " docid " UINT64_FMT, iCp, tPrevCp.m_tBaseDocID, tCp.m_tBaseDocID );

		tLookup.SeekTo ( tCp.m_tOffset, sizeof(DWORD) * 3 * iDocsPerCheckpoint );

		int iCpDocs = iDocsPerCheckpoint;
		// last checkpoint might have fewer docs
		if ( iCp==iCheckpoints-1 )
		{
			int iLefover = ( iDocs % iDocsPerCheckpoint );
			iCpDocs = ( iLefover ? iLefover : iDocsPerCheckpoint );
		}

		for ( int i=0; i<iCpDocs; ++i )
		{
			uint64_t tDelta = 0;
			DocID_t tDocID = 0;
			RowID_t tRowID = INVALID_ROWID;

			if ( !( i % iCpDocs ) )
			{
				tDocID = tLastDocID;
				tRowID = tLookup.GetDword();
			} else
			{
				tDelta = tLookup.UnzipOffset();
				tRowID = tLookup.GetDword();
				if ( tDelta==0 )
					m_tReporter.Fail ( "invalid docid delta " UINT64_FMT " at row %u, checkpoint %d, doc %d, last docid " UINT64_FMT, tDocID, tRowID, iCp, i, tLastDocID );
				else
					tDocID = tLastDocID + tDelta;

			}

			if ( tRowID>=m_iNumRows )
				m_tReporter.Fail ( "rowid %u out of bounds " INT64_FMT, tRowID, m_iNumRows );
			else if ( !pId->IsColumnar() )
			{
				// read only docid
				m_tAttrReader.SeekTo ( dRow.GetLengthBytes() * tRowID, sizeof(DocID_t) );
				m_tAttrReader.GetBytes ( dRow.Begin(), sizeof(DocID_t) );

				if ( dRowids.BitGet ( tRowID ) )
					m_tReporter.Fail ( "row %u already mapped, current docid" INT64_FMT " checkpoint %d, doc %d", tRowID, INT64_FMT, iCp, i );

				dRowids.BitSet ( tRowID );

				if ( tDocID!=sphGetDocID ( dRow.Begin() ) )
					m_tReporter.Fail ( "invalid docid " UINT64_FMT "(" UINT64_FMT ") at row %u, checkpoint %d, doc %d, last docid " UINT64_FMT,
						tDocID, sphGetDocID ( dRow.Begin() ), tRowID, iCp, i, tLastDocID );
			}

			tLastDocID = tDocID;
		}

		++iCp;
	}

	if ( !pId->IsColumnar() )
	{
		for ( int64_t i=0; i<m_iNumRows; ++i )
		{
			if ( dRowids.BitGet ( i ) )
				continue;

			m_tAttrReader.SeekTo ( dRow.GetLengthBytes() * i, sizeof(DocID_t) );
			m_tAttrReader.GetBytes ( dRow.Begin(), sizeof(DocID_t) );

			DocID_t tDocID = sphGetDocID ( dRow.Begin() );
		
			m_tReporter.Fail ( "row %u(" INT64_FMT ") not mapped at lookup, docid " UINT64_FMT, i, m_iNumRows, tDocID );
		}
	}

	SphOffset_t tNumericEnd = tLookup.GetPos();
	if ( pId->IsUuidLinkedDocid() )
		CheckUuidDocidLookup ( tLookup, iDocs, tUuidEntriesOffset, tNumericEnd );
	else if ( tUuidEntriesOffset )
		m_tReporter.Fail ( "unexpected uuid lookup offset in .SPT" );
}


void DiskIndexChecker_c::Impl_c::CheckUuidDocidLookup ( CSphReader & tLookup, int64_t iDocs, SphOffset_t tEntriesOffset, SphOffset_t tNumericEnd )
{
	if ( m_uVersion<71 )
	{
		m_tReporter.Fail ( "uuid lookup requires index format 71 or newer" );
		return;
	}

	if ( iDocs!=m_iNumRows )
		m_tReporter.Fail ( "uuid lookup entry count " INT64_FMT " differs from row count " INT64_FMT, iDocs, m_iNumRows );

	uint64_t uEntriesBytes = (uint64_t)iDocs*sizeof(UuidDocidLookupPair_t);
	if ( tEntriesOffset!=tNumericEnd || tEntriesOffset<=0 || (uint64_t)tEntriesOffset>(uint64_t)tLookup.GetFilesize() || uEntriesBytes!=(uint64_t)tLookup.GetFilesize()-(uint64_t)tEntriesOffset )
	{
		m_tReporter.Fail ( "invalid uuid lookup bounds in .SPT" );
		return;
	}

	tLookup.SeekTo ( tEntriesOffset, sizeof(UuidDocidLookupPair_t) );
	UuidDocidLookupPair_t tPrev;
	for ( int64_t i=0; i<iDocs; ++i )
	{
		UuidDocidLookupPair_t tPair;
		tPair.m_tKey.m_uHi = (uint64_t)tLookup.GetOffset();
		tPair.m_tKey.m_uLo = (uint64_t)tLookup.GetOffset();
		tPair.m_tDocID = tLookup.GetOffset();
		if ( tLookup.GetErrorFlag() )
		{
			m_tReporter.Fail ( "error reading uuid lookup entry " INT64_FMT ": %s", i, tLookup.GetErrorMessage().cstr() );
			return;
		}

		if ( !tPair.m_tDocID )
			m_tReporter.Fail ( "uuid lookup has zero docid at entry " INT64_FMT, i );
		if ( i && !CmpUuidDocidLookup_fn::IsLess ( tPrev, tPair ) )
			m_tReporter.Fail ( "uuid lookup keys are not strictly increasing at entry " INT64_FMT, i );

		tPrev = tPair;
	}
}

RowID_t DiskIndexChecker_c::Impl_c::GetRowidByDocid ( DocID_t iDocID ) const
{
	CSphMappedBuffer<BYTE> tDocidLookup;
	CSphString sLastError;

	if ( !tDocidLookup.Setup ( GetFilename ( SPH_EXT_SPT ), sLastError, false ) )
		return INVALID_ROWID;

	LookupReader_c tLookupReader;
	tLookupReader.SetData ( tDocidLookup.GetReadPtr(), m_uVersion );

	return tLookupReader.Find(iDocID);
}

RowID_t DiskIndexChecker_c::Impl_c::CheckIfKilled ( RowID_t iRowID ) const
{
	DeadRowMap_Disk_c tDeadRowMap;
	CSphString sError;
	tDeadRowMap.Prealloc ( (DWORD)m_iNumRows, GetFilename ( SPH_EXT_SPM ), sError );

	if ( tDeadRowMap.IsSet ( iRowID ) )
		iRowID = INVALID_ROWID;
	return iRowID;
}


struct DocRow_fn
{
	inline static bool IsLess ( const DocidRowidPair_t & tA, DocidRowidPair_t & tB )
	{
		if ( tA.m_tDocID==tB.m_tDocID && tA.m_tRowID<tB.m_tRowID )
			return true;

		return ( tA.m_tDocID<tB.m_tDocID );
	}
};

void DiskIndexChecker_c::Impl_c::CheckDocids()
{
	m_tReporter.Msg ( "checking docid duplicates ..." );

	CSphString sError;
	DeadRowMap_Disk_c tDeadRowMap;
	tDeadRowMap.Prealloc ( (DWORD)m_iNumRows, GetFilename ( SPH_EXT_SPM ), sError );
	tDeadRowMap.Preread ( GetFilename ( SPH_EXT_SPM ).cstr(), "checking docid duplicates", false );

	CSphFixedVector<CSphRowitem> dRow ( m_tSchema.GetRowSize() );
	m_tAttrReader.SeekTo ( 0, (int) dRow.GetLengthBytes() );

	for ( int iRowID=0; iRowID<m_iNumRows; iRowID++ )
	{
		if ( tDeadRowMap.IsSet ( iRowID ) )
			continue;

		m_tAttrReader.SeekTo ( dRow.GetLengthBytes() * iRowID, sizeof(DocID_t) );
		m_tAttrReader.GetBytes ( dRow.Begin(), sizeof(DocID_t) );

		m_tReporter.CheckDocidDup ( sphGetDocID ( dRow.Begin() ), iRowID );
	}

	m_tReporter.FinishDiskChunk ( m_iNumRows );
}


void DiskIndexChecker_c::Impl_c::CheckDocstore()
{
	if ( !m_bHasDocstore )
		return;

	m_tReporter.Msg ( "checking docstore..." );

	::CheckDocstore ( m_tDocstoreReader, m_tReporter, m_iNumRows );
}


CSphString DiskIndexChecker_c::Impl_c::GetFilename ( ESphExt eExt ) const
{
	return m_tIndex.GetFilename ( eExt );
}

/// public interface
DiskIndexChecker_c::DiskIndexChecker_c ( CSphIndex& tIndex, DebugCheckError_i& tReporter )
	: m_pImpl { std::make_unique<Impl_c> ( tIndex, tReporter ) }
{}

DiskIndexChecker_c::~DiskIndexChecker_c() = default;

bool DiskIndexChecker_c::OpenFiles ()
{
	return m_pImpl->OpenFiles();
}

void DiskIndexChecker_c::Setup ( int64_t iNumRows, int64_t iDocinfoIndex, int64_t iMinMaxIndex, bool bCheckIdDups )
{
	m_pImpl->Setup (iNumRows, iDocinfoIndex, iMinMaxIndex, bCheckIdDups );
}

CSphVector<SphWordID_t> & DiskIndexChecker_c::GetHitlessWords()
{
	return m_pImpl->GetHitlessWords();
}

void DiskIndexChecker_c::Check()
{
	m_pImpl->Check();
}

struct ColumnNameCmp_fn
{
	inline bool IsLess ( const CSphColumnInfo & tColA, const CSphColumnInfo & tColB ) const
	{
		return ( strcasecmp ( tColA.m_sName.cstr(), tColB.m_sName.cstr() )<0 );
	}
};

static CSphString DumpAttr ( const CSphColumnInfo & tCol )
{
	CSphString sRes;
	if ( tCol.m_tLocator.IsBlobAttr() )
		sRes.SetSprintf ( "%s at blob@%d", sphTypeName ( tCol.m_eAttrType ), tCol.m_tLocator.m_iBlobAttrId );
	else
		sRes.SetSprintf ( "%s at %d@%d", sphTypeName ( tCol.m_eAttrType ), tCol.m_tLocator.m_iBitCount, tCol.m_tLocator.m_iBitOffset );

	return sRes;
}

template <typename T>
void DebugCheckSchema_T ( const ISphSchema & tSchema, T & tReporter )
{
	// check duplicated names
	CSphVector<CSphColumnInfo> dAttrs;
	dAttrs.Reserve ( tSchema.GetAttrsCount() );
	for ( int iAttr=0; iAttr<tSchema.GetAttrsCount(); iAttr++ )
		dAttrs.Add ( tSchema.GetAttr ( iAttr ) );

	dAttrs.Sort ( ColumnNameCmp_fn() );
	
	for ( int iAttr=1; iAttr<dAttrs.GetLength(); iAttr++ )
	{
		const CSphColumnInfo & tPrev = dAttrs[iAttr-1];
		const CSphColumnInfo & tCur = dAttrs[iAttr];
		if ( strcasecmp ( tPrev.m_sName.cstr(), tCur.m_sName.cstr() )==0 )
			tReporter.Fail ( "duplicate attributes name %s for columns: %s, %s", tCur.m_sName.cstr(), DumpAttr ( tPrev ).cstr(), DumpAttr ( tCur ).cstr() );
	}
}

void DiskIndexChecker_c::Impl_c::CheckSchema()
{
	m_tReporter.Msg ( "checking schema..." );
	DebugCheckSchema_T ( m_tSchema, m_tReporter );
}

struct StringReporter_t
{
	StringBuilder_c m_sErrors;

	void Fail ( const char * szFmt, ... )
	{
		va_list ap;
		va_start ( ap, szFmt );
		m_sErrors.vAppendf ( szFmt, ap );
		va_end ( ap );
	}
};

bool DebugCheckSchema ( const ISphSchema & tSchema, CSphString & sError )
{
	StringReporter_t tRes;
	DebugCheckSchema_T ( tSchema, tRes );
	
	if ( !tRes.m_sErrors.IsEmpty() )
	{
		sError = tRes.m_sErrors.cstr();
		return false;
	} else
	{
		return true;
	}
}

void DebugCheckSchema ( const ISphSchema & tSchema, DebugCheckError_i & tReporter )
{
	DebugCheckSchema_T ( tSchema, tReporter );
}


bool SchemaConfigureCheckAttribute ( const CSphSchema & tSchema, const CSphColumnInfo & tCol, CSphString & sError )
{
	if ( tCol.m_sName.IsEmpty() )
	{
		sError.SetSprintf ( "column number %d has no name", tCol.m_iIndex );
		return false;
	}

	if ( tSchema.GetAttr ( tCol.m_sName.cstr() ) )
	{
		sError.SetSprintf ( "can not add multiple attributes with same name '%s'", tCol.m_sName.cstr () );
		return false;
	}

	if ( sphIsInternalAttr ( tCol.m_sName ) && tCol.m_sName!=sphGetUuidDocidName() )
	{
		sError.SetSprintf ( "%s is not a valid attribute name", tCol.m_sName.cstr() );
		return false;
	}

	return true;
}
