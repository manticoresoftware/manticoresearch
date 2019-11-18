//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxstd.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "sphinxrt.h"
#include "killlist.h"
#include "secondaryindex.h"
#include <time.h>


void StripStdin ( const char * sIndexAttrs, const char * sRemoveElements )
{
	CSphString sError;
	CSphHTMLStripper tStripper ( true );
	if ( !tStripper.SetIndexedAttrs ( sIndexAttrs, sError )
		|| !tStripper.SetRemovedElements ( sRemoveElements, sError ) )
			sphDie ( "failed to configure stripper: %s", sError.cstr() );

	CSphVector<BYTE> dBuffer;
	while ( !feof(stdin) )
	{
		char sBuffer[1024];
		int iLen = fread ( sBuffer, 1, sizeof(sBuffer), stdin );
		if ( !iLen )
			break;

		dBuffer.Append ((BYTE*)sBuffer, iLen);
	}
	dBuffer.Add ( 0 );

	tStripper.Strip ( &dBuffer[0] );
	fprintf ( stdout, "dumping stripped results...\n%s\n", &dBuffer[0] );
}


void ApplyMorphology ( CSphIndex * pIndex )
{
	CSphVector<BYTE> dInBuffer, dOutBuffer;
	const int READ_BUFFER_SIZE = 1024;
	dInBuffer.Reserve ( READ_BUFFER_SIZE );
	char sBuffer[READ_BUFFER_SIZE];
	while ( !feof(stdin) )
	{
		int iLen = fread ( sBuffer, 1, sizeof(sBuffer), stdin );
		if ( !iLen )
			break;
		dInBuffer.Append ( sBuffer, iLen );
	}
	dInBuffer.Add(0);
	dOutBuffer.Reserve ( dInBuffer.GetLength() );

	TokenizerRefPtr_c pTokenizer { pIndex->GetTokenizer()->Clone ( SPH_CLONE_INDEX ) };
	DictRefPtr_c pDict { pIndex->GetDictionary() };
	SafeAddRef ( pDict );
	BYTE * sBufferToDump = &dInBuffer[0];
	if ( pTokenizer )
	{
		pTokenizer->SetBuffer ( &dInBuffer[0], dInBuffer.GetLength() );
		while ( BYTE * sToken = pTokenizer->GetToken() )
		{
			if ( pDict )
				pDict->ApplyStemmers ( sToken );

			int iLen = strlen ( (char *)sToken );
			sToken[iLen] = ' ';
			dOutBuffer.Append ( sToken, iLen+1 );
		}

		if ( dOutBuffer.GetLength() )
			dOutBuffer[dOutBuffer.GetLength()-1] = 0;
		else
			dOutBuffer.Add(0);

		sBufferToDump = &dOutBuffer[0];
	}

	fprintf ( stdout, "dumping stemmed results...\n%s\n", sBufferToDump );
}


void CharsetFold ( CSphIndex * pIndex, FILE * fp )
{
	CSphVector<BYTE> sBuf1 ( 16384 );
	CSphVector<BYTE> sBuf2 ( 16384 );

	CSphLowercaser tLC = pIndex->GetTokenizer()->GetLowercaser();

#if USE_WINDOWS
	setmode ( fileno(stdout), O_BINARY );
#endif

	int iBuf1 = 0; // how many leftover bytes from previous iteration
	while ( !feof(fp) )
	{
		int iGot = fread ( sBuf1.Begin()+iBuf1, 1, sBuf1.GetLength()-iBuf1, fp );
		if ( iGot<0 )
			sphDie ( "read error: %s", strerrorm(errno) );

		if ( iGot==0 )
			if ( feof(fp) )
				if ( iBuf1==0 )
					break;


		const BYTE * pIn = sBuf1.Begin();
		const BYTE * pInMax = pIn + iBuf1 + iGot;

		if ( pIn==pInMax && feof(fp) )
			break;

		// tricky bit
		// on full buffer, and not an eof, terminate a bit early
		// to avoid codepoint vs buffer boundary issue
		if ( ( iBuf1+iGot )==sBuf1.GetLength() && iGot!=0 )
			pInMax -= 16;

		// do folding
		BYTE * pOut = sBuf2.Begin();
		BYTE * pOutMax = pOut + sBuf2.GetLength() - 16;
		while ( pIn < pInMax )
		{
			int iCode = sphUTF8Decode ( pIn );
			if ( iCode==0 )
				pIn++; // decoder does not do that!
			assert ( iCode>=0 );

			if ( iCode!=0x09 && iCode!=0x0A && iCode!=0x0D )
			{
				iCode = tLC.ToLower ( iCode ) & 0xffffffUL;
				if ( !iCode )
					iCode = 0x20;
			}

			pOut += sphUTF8Encode ( pOut, iCode );
			if ( pOut>=pOutMax )
			{
				fwrite ( sBuf2.Begin(), 1, pOut-sBuf2.Begin(), stdout );
				pOut = sBuf2.Begin();
			}
		}
		fwrite ( sBuf2.Begin(), 1, pOut-sBuf2.Begin(), stdout );

		// now move around leftovers
		BYTE * pRealEnd = sBuf1.Begin() + iBuf1 + iGot;
		if ( pIn < pRealEnd )
		{
			iBuf1 = pRealEnd - pIn;
			memmove ( sBuf1.Begin(), pIn, iBuf1 );
		}
	}
}

#pragma pack(push,4)
struct IDFWord_t
{
	uint64_t	m_uWordID;
	DWORD		m_iDocs;
};
#pragma pack(pop)
STATIC_SIZE_ASSERT	( IDFWord_t, 12 );


bool BuildIDF ( const CSphString & sFilename, const StrVec_t & dFiles, CSphString & sError, bool bSkipUnique )
{
	// text dictionaries are ordered alphabetically - we can use that fact while reading
	// to merge duplicates, calculate total number of occurrences and process bSkipUnique
	// this method is about 3x faster and consumes ~2x less memory than a hash based one

	typedef char StringBuffer_t [ 3*SPH_MAX_WORD_LEN+16+128 ]; // { dict-keyowrd, 32bit number, 32bit number, 64bit number }

	int64_t iTotalDocuments = 0;
	int64_t iTotalWords = 0;
	int64_t iReadWords = 0;
	int64_t iMergedWords = 0;
	int64_t iSkippedWords = 0;
	int64_t iReadBytes = 0;
	int64_t iTotalBytes = 0;

	const int64_t tmStart = sphMicroTimer ();

	int iFiles = dFiles.GetLength ();

	CSphVector<CSphAutoreader> dReaders ( iFiles );

	ARRAY_FOREACH ( i, dFiles )
	{
		if ( !dReaders[i].Open ( dFiles[i], sError ) )
			return false;
		iTotalBytes += dReaders[i].GetFilesize ();
	}

	// internal state
	CSphFixedVector<StringBuffer_t> dWords ( iFiles );
	CSphVector<int> dDocs ( iFiles );
	CSphVector<bool> dFinished ( iFiles );
	dFinished.Fill ( false );
	bool bPreread = false;

	// current entry
	StringBuffer_t sWord = {0};
	DWORD iDocs = 0;

	// output vector, preallocate 10M
	CSphTightVector<IDFWord_t> dEntries;
	dEntries.Reserve ( 1024*1024*10 );

	for ( int i=0;; )
	{
		// read next input
		while (true)
		{
			int iLen;
			char * sBuffer = dWords[i];
			if ( ( iLen = dReaders[i].GetLine ( sBuffer, sizeof(StringBuffer_t) ) )>=0 )
			{
				iReadBytes += iLen;

				// find keyword pattern ( ^<keyword>,<docs>,... )
				char * p1 = strchr ( sBuffer, ',' );
				if ( p1 )
				{
					char * p2 = strchr ( p1+1, ',' );
					if ( p2 )
					{
						*p1 = *p2 = '\0';
						int iDocuments = atoi ( p1+1 );
						if ( iDocuments )
						{
							dDocs[i] = iDocuments;
							iReadWords++;
							break;
						}
					}
				} else
				{
					// keyword pattern not found (rather rare case), try to parse as a header, then
					char sSearch[] = "total-documents: ";
					if ( strstr ( sBuffer, sSearch )==sBuffer )
						iTotalDocuments += atoi ( sBuffer+strlen(sSearch) );
				}
			} else
			{
				dFinished[i] = true;
				break;
			}
		}

		bool bEnd = !dFinished.Contains ( false );

		i++;
		if ( !bPreread && i==iFiles )
			bPreread = true;

		if ( bPreread )
		{
			// find the next smallest input
			i = 0;
			for ( int j=0; j<iFiles; j++ )
				if ( !dFinished[j] && ( dFinished[i] || strcmp ( dWords[i], dWords[j] )>0 ) )
					i = j;

			// merge if we got the same word
			if ( !strcmp ( sWord, dWords[i] ) && !bEnd )
			{
				iDocs += dDocs[i];
				iMergedWords++;
			} else
			{
				if ( sWord[0]!='\0' )
				{
					if ( !bSkipUnique || iDocs>1 )
					{
						IDFWord_t & tEntry = dEntries.Add ();
						tEntry.m_uWordID = sphFNV64 ( sWord );
						tEntry.m_iDocs = iDocs;
						iTotalWords++;
					} else
						iSkippedWords++;
				}

				strncpy ( sWord, dWords[i], sizeof ( dWords[i] ) - 1 );
				iDocs = dDocs[i];
			}
		}

		if ( ( iReadWords & 0xffff )==0 || bEnd )
			fprintf ( stderr, "read %.1f of %.1f MB, %.1f%% done%c", ( bEnd ? float(iTotalBytes) : float(iReadBytes) )/1000000.0f,
			float(iTotalBytes)/1000000.0f, bEnd ? 100.0f : float(iReadBytes)*100.0f/float(iTotalBytes), bEnd ? '\n' : '\r' );

		if ( bEnd )
			break;
	}

	fprintf ( stdout, INT64_FMT" documents, " INT64_FMT " words (" INT64_FMT " read, " INT64_FMT " merged, " INT64_FMT " skipped)\n",
		iTotalDocuments, iTotalWords, iReadWords, iMergedWords, iSkippedWords );

	// write to disk
	fprintf ( stdout, "writing %s (%1.fM)...\n", sFilename.cstr(), float(iTotalWords*sizeof(IDFWord_t))/1000000.0f );

	dEntries.Sort ( bind ( &IDFWord_t::m_uWordID ) );

	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sFilename, sError ) )
		return false;

	// write file header
	tWriter.PutOffset ( iTotalDocuments );

	// write data
	tWriter.PutBytes ( dEntries.Begin(), dEntries.GetLength()*sizeof(IDFWord_t) );

	int tmWallMsec = (int)( ( sphMicroTimer() - tmStart )/1000 );
	fprintf ( stdout, "finished in %d.%d sec\n", tmWallMsec/1000, (tmWallMsec/100)%10 );

	return true;
}


bool MergeIDF ( const CSphString & sFilename, const StrVec_t & dFiles, CSphString & sError, bool bSkipUnique )
{
	// binary dictionaries are ordered by 64-bit word id, we can use that for merging.
	// read every file, check repeating word ids, merge if found, write to disk if not
	// memory requirements are about ~4KB per input file (used for buffered reading)

	int64_t iTotalDocuments = 0;
	int64_t iTotalWords = 0;
	int64_t iReadWords = 0;
	int64_t iMergedWords = 0;
	int64_t iSkippedWords = 0;
	int64_t iReadBytes = 0;
	int64_t iTotalBytes = 0;

	const int64_t tmStart = sphMicroTimer ();

	int iFiles = dFiles.GetLength ();

	// internal state
	CSphVector<CSphAutoreader> dReaders ( iFiles );
	CSphVector<IDFWord_t> dWords ( iFiles );
	CSphVector<int64_t> dRead ( iFiles );
	CSphVector<int64_t> dSize ( iFiles );
	CSphVector<BYTE*> dBuffers ( iFiles );
	CSphVector<bool> dFinished ( iFiles );
	dFinished.Fill ( false );
	bool bPreread = false;

	// current entry
	IDFWord_t tWord;
	tWord.m_uWordID = 0;
	tWord.m_iDocs = 0;

	// preread buffer
	const int iEntrySize = sizeof(int64_t)+sizeof(DWORD);
	const int iBufferSize = iEntrySize*256;

	// initialize vectors
	ARRAY_FOREACH ( i, dFiles )
	{
		if ( !dReaders[i].Open ( dFiles[i], sError ) )
			return false;
		iTotalDocuments += dReaders[i].GetOffset ();
		dRead[i] = 0;
		dSize[i] = dReaders[i].GetFilesize() - sizeof( SphOffset_t );
		dBuffers[i] = new BYTE [ iBufferSize ];
		iTotalBytes += dSize[i];
	}

	// open output file
	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sFilename, sError ) )
		return false;

	// write file header
	tWriter.PutOffset ( iTotalDocuments );

	for ( int i=0;; )
	{
		if ( dRead[i]<dSize[i] )
		{
			iReadBytes += iEntrySize;

			// This part basically does the following:
			// dWords[i].m_uWordID = dReaders[i].GetOffset ();
			// dWords[i].m_iDocs = dReaders[i].GetDword ();
			// but reading by 12 bytes seems quite slow (SetBuffers doesn't help)
			// the only way to speed it up is to buffer up a few entries manually

			int iOffset = (int)( dRead[i] % iBufferSize );
			if ( iOffset==0 )
				dReaders[i].GetBytes ( dBuffers[i], ( dSize[i]-dRead[i] )<iBufferSize ? (int)( dSize[i]-dRead[i] ) : iBufferSize );

			dWords[i].m_uWordID = *(uint64_t*)( dBuffers[i]+iOffset );
			dWords[i].m_iDocs = *(DWORD*)( dBuffers[i]+iOffset+sizeof(uint64_t) );

			dRead[i] += iEntrySize;
			iReadWords++;
		} else
			dFinished[i] = true;

		bool bEnd = !dFinished.Contains ( false );

		i++;
		if ( !bPreread && i==iFiles )
			bPreread = true;

		if ( bPreread )
		{
			// find the next smallest input
			i = 0;
			for ( int j=0; j<iFiles; j++ )
				if ( !dFinished[j] && ( dFinished[i] || dWords[i].m_uWordID>dWords[j].m_uWordID ) )
					i = j;

			// merge if we got the same word
			if ( tWord.m_uWordID==dWords[i].m_uWordID && !bEnd )
			{
				tWord.m_iDocs += dWords[i].m_iDocs;
				iMergedWords++;
			} else
			{
				if ( tWord.m_uWordID )
				{
					if ( !bSkipUnique || tWord.m_iDocs>1 )
					{
						tWriter.PutOffset ( tWord.m_uWordID );
						tWriter.PutDword ( tWord.m_iDocs );
						iTotalWords++;
					} else
						iSkippedWords++;
				}

				tWord = dWords[i];
			}
		}

		if ( ( iReadWords & 0xffff )==0 || bEnd )
			fprintf ( stderr, "read %.1f of %.1f MB, %.1f%% done%c", ( bEnd ? float(iTotalBytes) : float(iReadBytes) )/1000000.0f,
			float(iTotalBytes)/1000000.0f, bEnd ? 100.0f : float(iReadBytes)*100.0f/float(iTotalBytes), bEnd ? '\n' : '\r' );

		if ( bEnd )
			break;
	}

	ARRAY_FOREACH ( i, dFiles )
		SafeDeleteArray ( dBuffers[i] );

	fprintf ( stdout, INT64_FMT" documents, " INT64_FMT " words (" INT64_FMT " read, " INT64_FMT " merged, " INT64_FMT " skipped)\n",
		iTotalDocuments, iTotalWords, iReadWords, iMergedWords, iSkippedWords );

	int tmWallMsec = (int)( ( sphMicroTimer() - tmStart )/1000 );
	fprintf ( stdout, "finished in %d.%d sec\n", tmWallMsec/1000, (tmWallMsec/100)%10 );

	return true;
}


//////////////////////////////////////////////////////////////////////////
static const DWORD META_HEADER_MAGIC = 0x54525053;    ///< my magic 'SPRT' header
static const DWORD META_VERSION = 13;

const char * AttrType2Str ( ESphAttr eAttrType )
{
	switch ( eAttrType )
	{
	case SPH_ATTR_NONE:				return "SPH_ATTR_NONE";
	case SPH_ATTR_INTEGER:			return "SPH_ATTR_INTEGER";
	case SPH_ATTR_TIMESTAMP:		return "SPH_ATTR_TIMESTAMP";
	case SPH_ATTR_BOOL:				return "SPH_ATTR_BOOL";
	case SPH_ATTR_FLOAT:			return "SPH_ATTR_FLOAT";
	case SPH_ATTR_BIGINT:			return "SPH_ATTR_BIGINT";
	case SPH_ATTR_STRING:			return "SPH_ATTR_STRING";
	case SPH_ATTR_POLY2D:			return "SPH_ATTR_POLY2D";
	case SPH_ATTR_STRINGPTR:		return "SPH_ATTR_STRINGPTR";
	case SPH_ATTR_TOKENCOUNT:		return "SPH_ATTR_TOKENCOUNT";
	case SPH_ATTR_JSON:				return "SPH_ATTR_JSON";
	case SPH_ATTR_UINT32SET:		return "SPH_ATTR_UINT32SET";
	case SPH_ATTR_INT64SET:			return "SPH_ATTR_INT64SET";
	case SPH_ATTR_MAPARG:			return "SPH_ATTR_MAPARG";
	case SPH_ATTR_FACTORS:			return "SPH_ATTR_FACTORS";
	case SPH_ATTR_JSON_FIELD:		return "SPH_ATTR_JSON_FIELD";
	case SPH_ATTR_FACTORS_JSON:		return "SPH_ATTR_FACTORS_JSON";
	case SPH_ATTR_UINT32SET_PTR:	return "SPH_ATTR_UINT32SET_PTR";
	case SPH_ATTR_INT64SET_PTR:		return "SPH_ATTR_INT64SET_PTR";
	case SPH_ATTR_JSON_PTR:			return "SPH_ATTR_JSON_PTR";
	case SPH_ATTR_JSON_FIELD_PTR:	return "SPH_ATTR_JSON_FIELD_PTR";
	default: return "";
	}

	return "SPH_ATTR_NONE";
}

static void InfoMetaSchemaColumn ( CSphReader &rdInfo,  DWORD uVersion )
{
	CSphString sName = rdInfo.GetString ();
	fprintf ( stdout, "%s", sName.cstr());
	fprintf ( stdout, " %s", AttrType2Str ((ESphAttr)rdInfo.GetDword ()) );

	if ( uVersion>=5 ) // m_uVersion for searching
	{
		fprintf ( stdout, " (rowitem %d)", rdInfo.GetDword () );
		fprintf ( stdout, " %d/", rdInfo.GetDword () );
		fprintf ( stdout, "%d", rdInfo.GetDword () );
	}

	if ( uVersion>=16 ) // m_uVersion for searching
		fprintf ( stdout, " payload %d", rdInfo.GetByte () );
}

void InfoMetaSchema ( CSphReader &rdMeta, DWORD uVersion )
{
	fprintf ( stdout, "\n ======== SCHEMA ========" );
	int iNumFields = rdMeta.GetDword ();
	fprintf ( stdout, "\n  Fields: %d", iNumFields );

	for ( int i = 0; i<iNumFields; i++ )
	{
		fprintf ( stdout, "\n%02d. ", i + 1 );
		InfoMetaSchemaColumn ( rdMeta, uVersion );
	}

	int iNumAttrs = rdMeta.GetDword ();
	fprintf ( stdout, "\n  Attributes: %d", iNumAttrs );
	for ( int i = 0; i<iNumAttrs; i++ )
	{
		fprintf ( stdout, "\n%02d. ", i + 1 );
		InfoMetaSchemaColumn ( rdMeta, uVersion );
	}
}

void InfoMetaIndexSettings ( CSphReader &tReader, DWORD uVersion )
{
	fprintf ( stdout, "\n ======== INDEX SETTINGS ========" );
	if ( uVersion>=8 )
	{
		fprintf ( stdout, "\nMinPrefixLen: %d", tReader.GetDword () );
		fprintf ( stdout, "\nMinInfixLen: %d", tReader.GetDword () );
	} else if ( uVersion>=6 )
	{
		bool bPrefixesOnly = ( tReader.GetByte ()!=0 );
		fprintf ( stdout, "\nPrefixes only: %d", bPrefixesOnly );
		fprintf ( stdout, "\nMinInfix/PrefixLen: %d", tReader.GetDword () );
	}

	if ( uVersion>=38 )
		fprintf ( stdout, "\nMaxSubstringLen: %d", tReader.GetDword () );

	if ( uVersion>=9 )
	{
		fprintf ( stdout, "\nbHtmlStrip: %d", tReader.GetByte () );
		fprintf ( stdout, "\nsHtmlIndexAttrs: %s", tReader.GetString ().cstr() );
		fprintf ( stdout, "\nsHtmlRemoveElements: %s", tReader.GetString ().cstr() );
	}

	if ( uVersion>=12 )
		fprintf ( stdout, "\nbIndexExactWords: %d", tReader.GetByte () );

	if ( uVersion>=18 )
		fprintf ( stdout, "\neHitless: %d", tReader.GetDword () );

	if ( uVersion>=19 )
		fprintf ( stdout, "\neHitFormat: %d", tReader.GetDword () );


	if ( uVersion>=21 )
		fprintf ( stdout, "\nbIndexSP: %d", tReader.GetByte () );

	if ( uVersion>=22 )
		fprintf ( stdout, "\nsZones: %s", tReader.GetString ().cstr () );

	if ( uVersion>=23 )
	{
		fprintf ( stdout, "\niBoundaryStep: %d", tReader.GetDword () );
		fprintf ( stdout, "\niStopwordStep: %d", tReader.GetDword () );
	}

	if ( uVersion>=28 )
		fprintf ( stdout, "\niOvershortStep: %d", tReader.GetDword () );

	if ( uVersion>=30 )
		fprintf ( stdout, "\niEmbeddedLimit: %d", tReader.GetDword () );

	if ( uVersion>=32 )
	{
		fprintf ( stdout, "\neBigramIndex: %d", tReader.GetByte () );
		fprintf ( stdout, "\nsBigramWords: %s", tReader.GetString ().cstr () );
	}

	if ( uVersion>=35 )
		fprintf ( stdout, "\nbIndexFieldLens: %d", tReader.GetByte () );

	if ( uVersion>=39 )
	{
		fprintf ( stdout, "\nePreprocessor: %d", tReader.GetByte () );
		tReader.GetString();	// was: RLP context
	}

	if ( uVersion>=41 )
		fprintf ( stdout, "\nsIndexTokenFilter: %s", tReader.GetString ().cstr () );
}

static void InfoMetaFileInfo ( CSphReader &tReader )
{
	fprintf ( stdout, "\n  ======== FILE INFO ========" );
	fprintf ( stdout, "\nuSize: " INT64_FMT, tReader.GetOffset () );
	fprintf ( stdout, "\nuCTime: " INT64_FMT, tReader.GetOffset () );
	fprintf ( stdout, "\nuMTime: " INT64_FMT, tReader.GetOffset () );
	fprintf ( stdout, "\nuCRC32: %d", tReader.GetDword () );
}

bool InfoMetaTokenizerSettings ( CSphReader &tReader, DWORD uVersion)
{
	fprintf ( stdout, "\n ======== TOKENIZER SETTINGS ========" );

	if ( uVersion<9 )
		return true;

	int m_iType = tReader.GetByte ();
	fprintf ( stdout, "\niType: %d", m_iType );
	if ( m_iType!=TOKENIZER_UTF8 && m_iType!=TOKENIZER_NGRAM )
	{
		fprintf (stdout, "\ncan't load an old index with SBCS tokenizer" );
		return false;
	}

	fprintf ( stdout, "\nsCaseFolding: %s", tReader.GetString ().cstr () );
	fprintf ( stdout, "\niMinWordLen: %d", tReader.GetDword() );

	if ( uVersion>=30 )
	{
		bool bsyn = !!tReader.GetByte ();
		fprintf ( stdout, "\nbEmbeddedSynonyms: %d", bsyn );
		if ( bsyn )
		{
			int nSynonyms = ( int ) tReader.GetDword ();
			fprintf ( stdout, "\nnSynonyms: %d", nSynonyms );
			for ( int i=0; i<nSynonyms; ++i)
				fprintf ( stdout, "\nEmbedded Syn(%d): %s", i, tReader.GetString ().cstr () );
		}
	}

	fprintf ( stdout, "\nsSynonymsFile: %s", tReader.GetString ().cstr () );
	InfoMetaFileInfo ( tReader );
	fprintf ( stdout, "\nsBoundary: %s", tReader.GetString ().cstr () );
	fprintf ( stdout, "\nsIgnoreChars : %s", tReader.GetString ().cstr () );
	fprintf ( stdout, "\niNgramLen : %d", tReader.GetDword ());
	fprintf ( stdout, "\nsNgramChars : %s", tReader.GetString ().cstr () );
	if ( uVersion>=15 )
		fprintf ( stdout, "\nsBlendChars : %s", tReader.GetString ().cstr () );
	if ( uVersion>=24 )
		fprintf ( stdout, "\nsBlendMode : %s", tReader.GetString ().cstr () );

	return true;
}

void InfoMetaDictionarySettings ( CSphReader &tReader, DWORD uVersion )
{
	fprintf ( stdout, "\n ======== DICTIONARY SETTINGS ========" );

	if ( uVersion<9 )
		return;

	fprintf ( stdout, "\nsMorphology : %s", tReader.GetString ().cstr () );
	if ( uVersion>=43 )
		fprintf ( stdout, "\nsMorphFields : %s", tReader.GetString ().cstr () );


	if ( uVersion>=30 )
	{
		bool bEmbeddedStopwords = !!tReader.GetByte ();
		fprintf ( stdout, "\nbEmbeddedStopwords : %d", bEmbeddedStopwords );
		if ( bEmbeddedStopwords )
		{
			int nStopwords = ( int ) tReader.GetDword ();
			fprintf ( stdout, "\nnStopwords : %d", nStopwords );
			for ( int i = 0; i<nStopwords; ++i )
				fprintf ( stdout, "\nEmbedded Stp(%d): " INT64_FMT, i, tReader.UnzipOffset () );
		}
	}

	fprintf ( stdout, "\nsStopwords : %s", tReader.GetString ().cstr () );

	int nFiles = tReader.GetDword ();
	fprintf ( stdout, "\nnFiles : %d", nFiles );

	for ( int i = 0; i<nFiles; i++ )
	{
		fprintf ( stdout, "\nFile %d: %s", i+1, tReader.GetString ().cstr () );
		InfoMetaFileInfo ( tReader );
	}

	if ( uVersion>=30 )
	{
		bool bEmbeddedWordforms = !!tReader.GetByte ();
		fprintf ( stdout, "\nbEmbeddedWordforms : %d", bEmbeddedWordforms );

		if ( bEmbeddedWordforms )
		{
			int nWordforms = ( int ) tReader.GetDword ();
			fprintf ( stdout, "\nnWordforms : %d", nWordforms );
			for ( int i = 0; i<nWordforms; ++i )
				fprintf ( stdout, "\nEmbedded Wrd(%d): %s", i, tReader.GetString ().cstr() );
		}
	}

	int iWrdForms = 1;
	if ( uVersion>=29 )
	{
		iWrdForms = tReader.GetDword ();
		fprintf ( stdout, "\niWordForms : %d", iWrdForms );
	}

	for ( int i = 0; i<iWrdForms; i++ )
	{
		fprintf ( stdout, "\nFile %d: %s", i + 1, tReader.GetString ().cstr () );
		InfoMetaFileInfo ( tReader );
	}

	if ( uVersion>=13 )
		fprintf ( stdout, "\niMinStemmingLen : %d", tReader.GetDword () );

	if ( uVersion>=21 )
	{
		fprintf ( stdout, "\nbWordDict : %d", tReader.GetByte () );
	}

	if ( uVersion>=36 )
		fprintf ( stdout, "\nbStopwordsUnstemmed : %d", tReader.GetByte () );

	if ( uVersion>=37 )
		fprintf ( stdout, "\nsMorphFingerprint : %s", tReader.GetString ().cstr() );

}

void InfoMetaFieldFilterSettings ( CSphReader &tReader )
{
	fprintf ( stdout, "\n ======== FIELD FILTER SETTINGS ========" );

	int nRegexps = tReader.GetDword ();
	fprintf (stdout, "\n %d filters", nRegexps);
	if ( !nRegexps )
		return;

	for (int i=0; i<nRegexps; ++i)
		fprintf (stdout, "\n Filter(%d) = %s", i, tReader.GetString ().cstr());

	fprintf(stdout, "\n Deprecated utf-8 flag = %d", tReader.GetByte ());
}

void InfoMeta ( const CSphString &sMeta )
{
	fprintf ( stdout, "\nDescribing meta %s", sMeta.cstr());
	CSphString sError;
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, sError ) )
	{
		fprintf (stdout, "\n unable to open file: %s", sError.cstr());
		return;
	}

	DWORD dwFoo = rdMeta.GetDword();
	fprintf (stdout, "\nMagick: 0x%x (expected 0x%x)", dwFoo, META_HEADER_MAGIC);
	if ( dwFoo!=META_HEADER_MAGIC )
	{
		fprintf ( stdout, "\nwrong magick!");
		return;
	}
	DWORD uVersion = rdMeta.GetDword ();
	fprintf ( stdout, "\nVersion: %d (expected 1 to %d)", uVersion, META_VERSION );
	if ( uVersion==0 || uVersion>META_VERSION )
	{
		fprintf ( stdout, "%s is v.%d, binary is v.%d", sMeta.cstr (), uVersion, META_VERSION );
		return;
	}
	dwFoo = rdMeta.GetDword (); // disk chunks
	fprintf ( stdout, "\ndisk chunks: %d", dwFoo );
	if ( uVersion>=6 )
	{
		fprintf ( stdout, "\ndisk base: %d", rdMeta.GetDword () );
	} else
		fprintf ( stdout, "\ndisk base assumed 0 (not written in this ver of meta)" );
	fprintf ( stdout, "\nTotal documents: %d", rdMeta.GetDword());
	fprintf ( stdout, "\nTotal bytes: " INT64_FMT, rdMeta.GetOffset () );
	if ( uVersion>=2 )
		fprintf ( stdout, "\nTID: " INT64_FMT, rdMeta.GetOffset () );

	if ( uVersion>=4 )
	{
		DWORD uSettingsVer = rdMeta.GetDword ();
		fprintf (stdout, "\n Settings ver: %d", uSettingsVer );
		InfoMetaSchema(rdMeta, uSettingsVer);
		InfoMetaIndexSettings(rdMeta, uSettingsVer);
		InfoMetaTokenizerSettings (rdMeta, uSettingsVer);
		InfoMetaDictionarySettings(rdMeta,uSettingsVer);
	}

	if ( uVersion>=5 )
	{
		fprintf ( stdout, "\niWordsCheckpoint: %d", rdMeta.GetDword () );
	}

	if ( uVersion>=7 )
	{
		fprintf ( stdout, "\niMaxCodepointLength: %d", rdMeta.GetDword () );
		fprintf ( stdout, "\niBloomKeyLen: %d", rdMeta.GetByte () );
		fprintf ( stdout, "\niBloomHashesCount: %d", rdMeta.GetByte () );
	}

	if ( uVersion>=11 )
	{
		InfoMetaFieldFilterSettings ( rdMeta );
	}

	if ( uVersion>=12 )
	{
		CSphFixedVector<int> dChunkNames ( 0 );
		int iLen = ( int ) rdMeta.GetDword ();
		fprintf ( stdout, "\nNum of Chunknames: %d", iLen );
		dChunkNames.Reset ( iLen );
		rdMeta.GetBytes ( dChunkNames.Begin (), iLen * sizeof ( int ) );
		for ( int nm : dChunkNames)
			fprintf (stdout, "\n %d", nm);
	}

}


struct IndexInfo_t 
{
	bool							m_bEnabled {false};
	DWORD							m_nDocs {0};
	CSphString						m_sName;
	CSphString						m_sPath;
	CSphFixedVector<DocID_t>		m_dKilllist;
	CSphVector<KillListTarget_t>	m_dTargets;
	CSphMappedBuffer<BYTE>			m_tLookup;
	DeadRowMap_Disk_c 				m_tDeadRowMap;


	IndexInfo_t()
		: m_dKilllist ( 0 )
	{}
};


void ApplyKilllist ( IndexInfo_t & tTarget, const IndexInfo_t & tKiller, const KillListTarget_t & tSettings )
{
	if ( tSettings.m_uFlags & KillListTarget_t::USE_DOCIDS )
	{
		LookupReader_c tTargetReader ( tTarget.m_tLookup.GetWritePtr() );
		LookupReader_c tKillerReader ( tKiller.m_tLookup.GetWritePtr() );

		KillByLookup ( tTargetReader, tKillerReader, tTarget.m_tDeadRowMap );
	}

	if ( tSettings.m_uFlags & KillListTarget_t::USE_KLIST )
	{
		LookupReader_c tTargetReader ( tTarget.m_tLookup.GetWritePtr() );
		DocidListReader_c tKillerReader ( tKiller.m_dKilllist );

		KillByLookup ( tTargetReader, tKillerReader, tTarget.m_tDeadRowMap );
	}
}


void ApplyKilllists ( CSphConfig & hConf )
{
	int nIndexes = 0;
	hConf["index"].IterateStart();
	while ( hConf["index"].IterateNext() )
		nIndexes++;

	CSphFixedVector<IndexInfo_t> dIndexes ( nIndexes );

	int iIndex = 0;

	hConf["index"].IterateStart();
	while ( hConf["index"].IterateNext() )
	{
		CSphConfigSection & hIndex = hConf["index"].IterateGet();
		if ( !hIndex("path") )
			continue;

		const CSphVariant * pType = hIndex ( "type" );
		if ( pType && ( *pType=="rt" || *pType=="distributed" || *pType=="percolate" ) )
			continue;

		IndexInfo_t & tIndex = dIndexes[iIndex++];
		tIndex.m_sName = hConf["index"].IterateGetKey().cstr();
		tIndex.m_sPath = hIndex["path"].cstr();

		IndexFiles_c tIndexFiles ( tIndex.m_sPath, tIndex.m_sName.cstr () );

		if ( !tIndexFiles.CheckHeader() )
		{
			fprintf ( stdout, "WARNING: unable to index header for index %s\n", tIndex.m_sName.cstr() );
			continue;
		}

		// no lookups prior to v.54
		if ( tIndexFiles.GetVersion() < 54 )
		{
			fprintf ( stdout, "WARNING: index '%s' version: %u, min supported is 54\n", tIndex.m_sName.cstr(), tIndexFiles.GetVersion() );
			continue;
		}

		CSphString sError;
		{
			CSphScopedPtr<CSphIndex> pIndex ( sphCreateIndexPhrase ( nullptr, tIndex.m_sPath.cstr() ) );
			if ( !pIndex->LoadKillList ( &tIndex.m_dKilllist, tIndex.m_dTargets, sError ) )
			{
				fprintf ( stdout, "WARNING: unable to load kill-list for index %s: %s\n", tIndex.m_sName.cstr(), sError.cstr() );
				continue;
			}

			if ( !pIndex->Prealloc(false) )
			{
				fprintf ( stdout, "WARNING: unable to prealloc index %s: %s\n", tIndex.m_sName.cstr(), sError.cstr() );
				continue;
			}

			tIndex.m_nDocs = pIndex->GetStats().m_iTotalDocuments;
		}

		CSphString sLookup;
		sLookup.SetSprintf ( "%s.spt", tIndex.m_sPath.cstr() );
		if ( !tIndex.m_tLookup.Setup ( sLookup.cstr(), sError, false ) )
		{
			fprintf ( stdout, "WARNING: unable to load lookup for index %s: %s\n", tIndex.m_sName.cstr(), sError.cstr() );
			continue;
		}

		CSphString sDeadRowMap;
		sDeadRowMap.SetSprintf ( "%s.spm", tIndex.m_sPath.cstr() );		
		if ( !tIndex.m_tDeadRowMap.Prealloc ( tIndex.m_nDocs, sDeadRowMap, sError ) )
		{
			fprintf ( stdout, "WARNING: unable to load dead row map for index %s: %s\n", tIndex.m_sName.cstr(), sError.cstr() );
			continue;
		}

		tIndex.m_bEnabled = true;
	}

	for ( const auto & tIndex : dIndexes )
	{
		if ( !tIndex.m_bEnabled || !tIndex.m_dTargets.GetLength() )
			continue;

		fprintf ( stdout, "applying kill-list of index %s\n", tIndex.m_sName.cstr() );

		for ( const auto & tTarget : tIndex.m_dTargets )
		{
			if ( tTarget.m_sIndex==tIndex.m_sName )
			{
				fprintf ( stdout, "WARNING: index '%s': appying killlist to itself\n", tIndex.m_sName.cstr() );
				continue;
			}

			for ( auto & tTargetIndex : dIndexes )
			{
				if ( !tTargetIndex.m_bEnabled || tTarget.m_sIndex!=tTargetIndex.m_sName )
					continue;

				ApplyKilllist ( tTargetIndex, tIndex, tTarget );
			}
		}
	}

	for ( auto & tIndex : dIndexes )
	{
		if ( !tIndex.m_bEnabled )
			continue;

		// flush maps
		CSphString sError;
		if ( !tIndex.m_tDeadRowMap.Flush ( true, sError ) )
			fprintf ( stdout, "WARNING: index '%s': unable to flush dead row map: %s\n", tIndex.m_sName.cstr(), sError.cstr() );

		// delete killlists
		CSphString sKilllist;
		sKilllist.SetSprintf ( "%s.spk", tIndex.m_sPath.cstr() );

		if ( !sphIsReadable(sKilllist) )
			continue;

		::unlink ( sKilllist.cstr() );
	}
}


static void ShowVersion ()
{
	fprintf ( stdout, SPHINX_BANNER );
}

static void ShowHelp ()
{
	fprintf ( stdout,
		"Usage: indextool <COMMAND> [OPTIONS]\n"
		"\n"
		"Commands are:\n"
		"-h, --help\t\tdisplay this help message\n"
		"-v\t\t\tdisplay version information\n"
		"--check <INDEX>\t\tperform index consistency check\n"
		"--checkconfig\t\tperform config consistency check\n"
		"--dumpconfig <SPH-FILE>\tdump index header in config format by file name\n"
		"--dumpdocids <INDEX>\tdump docids by index name\n"
		"--dumpdict <SPI-FILE>\tdump dictionary by file name\n"
		"--dumpdict <INDEX>\tdump dictionary\n"
		"--dumpheader <SPH-FILE>|<META-FILE>\tdump index header, or rt index meta by file name\n"
		"--dumpheader <INDEX>\tdump index header by index name\n"
		"--dumphitlist <INDEX> <KEYWORD>\n"
		"--dumphitlist <INDEX> --wordid <ID>\n"
		"\t\t\tdump hits for a given keyword\n"
		"--fold <INDEX> [FILE]\tfold FILE or stdin using INDEX charset_table\n"
		"--htmlstrip <INDEX>\tfilter stdin using index HTML stripper settings\n"
		"--buildidf <INDEX1.dict> [INDEX2.dict ...] [--skip-uniq] --out <GLOBAL.idf>\n"
		"\t\t\tjoin --stats dictionary dumps into global.idf file\n"
		"--mergeidf <NODE1.idf> [NODE2.idf ...] [--skip-uniq] --out <GLOBAL.idf>\n"
		"\t\t\tmerge several .idf files into one file\n"
		"--apply-killlists\t\tapply index killlists\n"
		"\n"
		"Options are:\n"
		"-c, --config <file>\tuse given config file instead of defaults\n"
		"-q, --quiet\t\tbe quiet, skip banner etc (useful with --fold etc)\n"
		"--strip-path\t\tstrip path from filenames referenced by index\n"
		"\t\t\t(eg. stopwords, exceptions, etc)\n"
		"--stats\t\t\tshow total statistics in the dictionary dump\n"
		"--skip-uniq\t\tskip unique (df=1) words in the .idf files\n"
	);
}

enum class IndextoolCmd_e
{
	NOTHING,
	DUMPHEADER,
	DUMPCONFIG,
	DUMPDOCIDS,
	DUMPHITLIST,
	DUMPDICT,
	CHECK,
	STRIP,
	MORPH,
	BUILDIDF,
	MERGEIDF,
	CHECKCONFIG,
	FOLD,
	APPLYKLISTS
};

static IndextoolCmd_e g_eCommand = IndextoolCmd_e::NOTHING;
static const char * g_sCommands[] = {"", "dumpheader", "dumpconfig", "dumpdocids", "dumphitlist", "dumpdict",
	"check", "htmlstrip", "morph", "buildidf", "mergeidf", "checkconfig", "fold", "apply-killlists" };


static void SetCmd ( IndextoolCmd_e eCmd )
{
	if ( g_eCommand!=IndextoolCmd_e::NOTHING )
	{
		fprintf ( stdout, "ERROR: multiple commands not supported (%s, %s).\n", g_sCommands[(int)g_eCommand], g_sCommands[(int)eCmd] );
		exit ( 1 );
	}

	g_eCommand = eCmd;
}

int main ( int argc, char ** argv )
{
	if ( argc<=1 )
	{
		ShowVersion();
		ShowHelp();
		exit ( 0 );
	}

	//////////////////////
	// parse command line
	//////////////////////

	#define OPT(_a1,_a2)	else if ( !strcmp(argv[i],_a1) || !strcmp(argv[i],_a2) )
	#define OPT1(_a1)		else if ( !strcmp(argv[i],_a1) )

	const char * sOptConfig = NULL;
	CSphString sDumpHeader, sIndex, sKeyword, sFoldFile;
	bool bWordid = false;
	bool bStripPath = false;
	StrVec_t dFiles;
	CSphString sOut;
	bool bStats = false;
	bool bSkipUnique = false;
	CSphString sDumpDict;
	bool bQuiet = false;
	bool bRotate = false;
	bool bCheckIdDups = false;

	int i;
	for ( i=1; i<argc; i++ )
	{
		// handle argless options
		if ( argv[i][0]!='-' ) break;
		OPT ( "-q", "--quiet" )		{ bQuiet = true; continue; }
		OPT1 ( "--strip-path" )		{ bStripPath = true; continue; }
		OPT1 ( "--checkconfig" )	{ SetCmd ( IndextoolCmd_e::CHECKCONFIG ); continue; }
		OPT1 ( "--rotate" )			{ bRotate = true; continue; }
		OPT1 ( "-v" )				{ ShowVersion(); exit(0); }
		OPT ( "-h", "--help" )		{ ShowVersion(); ShowHelp(); exit(0); }
		OPT1 ( "--apply-killlists" ){ SetCmd ( IndextoolCmd_e::APPLYKLISTS ); continue; }
		OPT1 ( "--check-id-dups" )	{ bCheckIdDups = true; continue; }

		// handle options/commands with 1+ args
		if ( (i+1)>=argc )			break;
		OPT ( "-c", "--config" )	sOptConfig = argv[++i];
		OPT1 ( "--dumpheader" )		{ SetCmd ( IndextoolCmd_e::DUMPHEADER ); sDumpHeader = argv[++i]; }
		OPT1 ( "--dumpconfig" )		{ SetCmd ( IndextoolCmd_e::DUMPCONFIG ); sDumpHeader = argv[++i]; }
		OPT1 ( "--dumpdocids" )		{ SetCmd ( IndextoolCmd_e::DUMPDOCIDS ); sIndex = argv[++i]; }
		OPT1 ( "--check" )			{ SetCmd ( IndextoolCmd_e::CHECK ); sIndex = argv[++i]; }
		OPT1 ( "--htmlstrip" )		{ SetCmd ( IndextoolCmd_e::STRIP ); sIndex = argv[++i]; }
		OPT1 ( "--morph" )			{ SetCmd ( IndextoolCmd_e::MORPH ); sIndex = argv[++i]; }
		OPT1 ( "--dumpdict" )
		{
			SetCmd ( IndextoolCmd_e::DUMPDICT );
			sDumpDict = argv[++i];
			if ( (i+1)<argc && !strcmp ( argv[i+1], "--stats" ) )
			{
				bStats = true;
				i++;
			}
		}
		OPT1 ( "--fold" )
		{
			SetCmd ( IndextoolCmd_e::FOLD );
			sIndex = argv[++i];
			if ( (i+1)<argc && argv[i+1][0]!='-' )
				sFoldFile = argv[++i];
		}

		// options with 2 args
		else if ( (i+2)>=argc ) // NOLINT
		{
			// not enough args
			break;

		}
		OPT1 ("--dumphitlist" )
		{
			SetCmd ( IndextoolCmd_e::DUMPHITLIST );
			sIndex = argv[++i];

			if ( !strcmp ( argv[i+1], "--wordid" ) )
			{
				if ( (i+3)<argc )
					break; // not enough args
				bWordid = true;
				i++;
			}

			sKeyword = argv[++i];

		} else if ( !strcmp ( argv[i], "--buildidf" ) || !strcmp ( argv[i], "--mergeidf" ) )
		{
			SetCmd ( !strcmp ( argv[i], "--buildidf" ) ? IndextoolCmd_e::BUILDIDF : IndextoolCmd_e::MERGEIDF );
			while ( ++i<argc )
			{
				if ( !strcmp ( argv[i], "--out" ) )
				{
					if ( (i+1)>=argc )
						break; // too few args
					sOut = argv[++i];

				} else if ( !strcmp ( argv[i], "--skip-uniq" ) )
				{
					bSkipUnique = true;

				} else if ( argv[i][0]=='-' )
				{
					break; // unknown switch

				} else
				{
					dFiles.Add ( argv[i] ); // handle everything else as a file name
				}
			}
			break;

		} else
		{
			// unknown option
			break;
		}
	}

	if ( !bQuiet )
		fprintf ( stdout, SPHINX_BANNER );

	if ( i!=argc )
	{
		fprintf ( stdout, "ERROR: malformed or unknown option near '%s'.\n", argv[i] );
		return 1;
	}

	//////////////////////
	// load proper config
	//////////////////////

	CSphString sError;
	if ( !sphInitCharsetAliasTable ( sError ) )
		sphDie ( "failed to init charset alias table: %s", sError.cstr() );

	CSphConfigParser cp;
	CSphConfig & hConf = cp.m_tConf;
	sphLoadConfig ( sOptConfig, bQuiet, cp );

	while (true)
	{
		if ( g_eCommand==IndextoolCmd_e::DUMPHEADER && sDumpHeader.Ends ( ".meta" ) )
		{
			InfoMeta ( sDumpHeader );
			return 0;
		}

		if ( g_eCommand==IndextoolCmd_e::DUMPDICT && !sDumpDict.Ends ( ".spi" ) )
			sIndex = sDumpDict;

		break;
	}

	///////////
	// action!
	///////////

	if ( g_eCommand==IndextoolCmd_e::CHECKCONFIG )
	{
		fprintf ( stdout, "config valid\nchecking index(es) ... " );

		bool bError = false;
		// config parser made sure that index(es) present
		const CSphConfigType & hIndexes = hConf ["index"];

		hIndexes.IterateStart();
		while ( hIndexes.IterateNext() )
		{
			const CSphConfigSection & tIndex = hIndexes.IterateGet();
			const CSphVariant * pPath = tIndex ( "path" );
			if ( !pPath )
				continue;

			const CSphVariant * pType = tIndex ( "type" );
			if ( pType && ( *pType=="rt" || *pType=="distributed" || *pType=="percolate" ) )
				continue;

			// checking index presence by sph file available
			CSphString sHeader;
			sHeader.SetSprintf ( "%s.sph", pPath->cstr() );
			CSphAutoreader rdHeader;
			if ( !rdHeader.Open ( sHeader, sError ) )
			{
				// nice looking output
				if ( !bError )
					fprintf ( stdout, "\nmissed index(es): '%s'", hIndexes.IterateGetKey().cstr() );
				else
					fprintf ( stdout, ", '%s'", hIndexes.IterateGetKey().cstr() );

				bError = true;
			}
		}
		if ( !bError )
		{
			fprintf ( stdout, "ok\n" );
			exit ( 0 );
		} else
		{
			fprintf ( stdout, "\n" );
			exit ( 1 );
		}
	}

	if ( g_eCommand==IndextoolCmd_e::APPLYKLISTS )
	{
		ApplyKilllists ( hConf );
		exit (0);
	}

	// configure common settings (as of time of this writing, AOT and ICU setup)
	sphConfigureCommon ( hConf );

	// common part for several commands, check and preload index
	CSphIndex * pIndex = nullptr;
	while ( !sIndex.IsEmpty() )
	{
		// check config
		if ( !hConf["index"].Exists(sIndex) )
			sphDie ( "index '%s': no such index in config\n", sIndex.cstr() );

		// only need config-level settings for --htmlstrip
		if ( g_eCommand==IndextoolCmd_e::STRIP )
			break;

		CSphVariant * pType = hConf["index"][sIndex]("type");
		if ( pType && ( *pType=="distributed" || *pType=="percolate" ) )
			sphDie ( "index '%s': check of '%s' type is not supported'\n", sIndex.cstr(), pType->cstr() );

		if ( !hConf["index"][sIndex]("path") )
			sphDie ( "index '%s': missing 'path' in config'\n", sIndex.cstr() );

		// preload that index
		bool bDictKeywords = true;
		if ( hConf["index"][sIndex].Exists ( "dict" ) )
			bDictKeywords = ( hConf["index"][sIndex]["dict"]!="crc" );

		if ( hConf["index"][sIndex]("type") && hConf["index"][sIndex]["type"]=="rt" )
		{
			CSphSchema tSchema;
			if ( sphRTSchemaConfigure ( hConf["index"][sIndex], tSchema, sError, false ) )
				pIndex = sphCreateIndexRT ( tSchema, sIndex.cstr(), 32*1024*1024, hConf["index"][sIndex]["path"].cstr(), bDictKeywords );
		} else
		{
			StringBuilder_c tPath;
			tPath << hConf["index"][sIndex]["path"] << ( bRotate ? ".tmp" : nullptr );
			pIndex = sphCreateIndexPhrase ( sIndex.cstr(), tPath.cstr() );
		}

		if ( !pIndex )
			sphDie ( "index '%s': failed to create (%s)", sIndex.cstr(), sError.cstr() );

		if ( g_eCommand==IndextoolCmd_e::CHECK )
			pIndex->SetDebugCheck ( bCheckIdDups );

		CSphString sWarn;
		if ( !pIndex->Prealloc ( bStripPath ) )
			sphDie ( "index '%s': prealloc failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

		if ( g_eCommand==IndextoolCmd_e::MORPH )
			break;

		if ( g_eCommand!=IndextoolCmd_e::CHECK )
			pIndex->Preread();

		if ( hConf["index"][sIndex]("hitless_words") )
		{
			CSphIndexSettings tSettings = pIndex->GetSettings();

			const CSphString & sValue = hConf["index"][sIndex]["hitless_words"].strval();
			if ( sValue=="all" )
			{
				tSettings.m_eHitless = SPH_HITLESS_ALL;
			} else
			{
				tSettings.m_eHitless = SPH_HITLESS_SOME;
				tSettings.m_sHitlessFiles = sValue;
			}

			pIndex->Setup ( tSettings );
		}

		break;
	}

	int iCheckErrno = 0;
	CSphString sNewIndex;

	// do the dew
	switch ( g_eCommand )
	{
		case IndextoolCmd_e::NOTHING:
			sphDie ( "nothing to do; specify a command (run indextool w/o switches for help)" );

		case IndextoolCmd_e::DUMPHEADER:
		case IndextoolCmd_e::DUMPCONFIG:
		{
			CSphString sIndexName = "(none)";
			if ( hConf("index") && hConf["index"](sDumpHeader) )
			{
				fprintf ( stdout, "dumping header for index '%s'...\n", sDumpHeader.cstr() );

				if ( !hConf["index"][sDumpHeader]("path") )
					sphDie ( "missing 'path' for index '%s'\n", sDumpHeader.cstr() );

				sIndexName = sDumpHeader;
				sDumpHeader.SetSprintf ( "%s.sph", hConf["index"][sDumpHeader]["path"].cstr() );
			} else
				fprintf ( stdout, "dumping header file '%s'...\n", sDumpHeader.cstr() );

			pIndex = sphCreateIndexPhrase ( sIndexName.cstr(), "" );
			pIndex->DebugDumpHeader ( stdout, sDumpHeader.cstr(), g_eCommand==IndextoolCmd_e::DUMPCONFIG );
			break;
		}

		case IndextoolCmd_e::DUMPDOCIDS:
			fprintf ( stdout, "dumping docids for index '%s'...\n", sIndex.cstr() );
			pIndex->DebugDumpDocids ( stdout );
			break;

		case IndextoolCmd_e::DUMPHITLIST:
			fprintf ( stdout, "dumping hitlist for index '%s' keyword '%s'...\n", sIndex.cstr(), sKeyword.cstr() );
			pIndex->DebugDumpHitlist ( stdout, sKeyword.cstr(), bWordid );
			break;

		case IndextoolCmd_e::DUMPDICT:
		{
			if ( sDumpDict.Ends ( ".spi" ) )
			{
				fprintf ( stdout, "dumping dictionary file '%s'...\n", sDumpDict.cstr() );

				sIndex = sDumpDict.SubString ( 0, sDumpDict.Length()-4 );
				pIndex = sphCreateIndexPhrase ( sIndex.cstr(), sIndex.cstr() );

				if ( !pIndex )
					sphDie ( "index '%s': failed to create (%s)", sIndex.cstr(), sError.cstr() );

				CSphString sWarn;
				if ( !pIndex->Prealloc ( bStripPath ) )
					sphDie ( "index '%s': prealloc failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

				pIndex->Preread();
			} else
				fprintf ( stdout, "dumping dictionary for index '%s'...\n", sIndex.cstr() );

			if ( bStats )
				fprintf ( stdout, "total-documents: " INT64_FMT "\n", pIndex->GetStats().m_iTotalDocuments );
			pIndex->DebugDumpDict ( stdout );
			break;
		}

		case IndextoolCmd_e::CHECK:
			fprintf ( stdout, "checking index '%s'...\n", sIndex.cstr() );
			iCheckErrno = pIndex->DebugCheck ( stdout );
			if ( iCheckErrno )
				return iCheckErrno;
			if ( bRotate )
			{
				pIndex->Dealloc();
				sNewIndex.SetSprintf ( "%s.new", hConf["index"][sIndex]["path"].cstr() );
				if ( !pIndex->Rename ( sNewIndex.cstr() ) )
					sphDie ( "index '%s': rotate failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );
			}
			return 0;

		case IndextoolCmd_e::STRIP:
			{
				const CSphConfigSection & hIndex = hConf["index"][sIndex];
				if ( hIndex.GetInt ( "html_strip" )==0 )
					sphDie ( "HTML stripping is not enabled in index '%s'", sIndex.cstr() );
				StripStdin ( hIndex.GetStr ( "html_index_attrs" ), hIndex.GetStr ( "html_remove_elements" ) );
			}
			break;

		case IndextoolCmd_e::MORPH:
			ApplyMorphology ( pIndex );
			break;

		case IndextoolCmd_e::BUILDIDF:
			if ( !BuildIDF ( sOut, dFiles, sError, bSkipUnique ) )
				sphDie ( "ERROR: %s\n", sError.cstr() );
			break;

		case IndextoolCmd_e::MERGEIDF:
			if ( !MergeIDF ( sOut, dFiles, sError, bSkipUnique ) )
				sphDie ( "ERROR: %s\n", sError.cstr() );
			break;

		case IndextoolCmd_e::FOLD:
			{
				FILE * fp = stdin;
				if ( !sFoldFile.IsEmpty() )
				{
					fp = fopen ( sFoldFile.cstr(), "rb" );
					if ( !fp )
						sphDie ( "failed to topen %s\n", sFoldFile.cstr() );
				}
				CharsetFold ( pIndex, fp );
				if ( fp!=stdin )
					fclose ( fp );
			}
			break;

		default:
			sphDie ( "INTERNAL ERROR: unhandled command (id=%d)", (int)g_eCommand );
	}

	return 0;
}
