//
// $Id$
//

//
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
#include "sphinxutils.h"
#include "sphinxint.h"
#include "sphinxrt.h"
#include <time.h>

#if USE_WINDOWS
#include <io.h> // for setmode(). open() on windows
#define sphSeek		_lseeki64
#else
#define sphSeek		lseek
#endif


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

		int iPos = dBuffer.GetLength();
		dBuffer.Resize ( iPos+iLen );
		memcpy ( &dBuffer[iPos], sBuffer, iLen );
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

		int iPos = dInBuffer.GetLength();
		dInBuffer.Resize ( iPos+iLen );
		memcpy ( &dInBuffer[iPos], sBuffer, iLen );
	}
	dInBuffer.Add(0);
	dOutBuffer.Reserve ( dInBuffer.GetLength() );

	CSphScopedPtr<ISphTokenizer> pTokenizer ( pIndex->GetTokenizer()->Clone ( SPH_CLONE_INDEX ) );
	CSphDict * pDict = pIndex->GetDictionary();
	BYTE * sBufferToDump = &dInBuffer[0];
	if ( pTokenizer.Ptr() )
	{
		pTokenizer->SetBuffer ( &dInBuffer[0], dInBuffer.GetLength() );
		while ( BYTE * sToken = pTokenizer->GetToken() )
		{
			if ( pDict )
				pDict->ApplyStemmers ( sToken );

			int iPos = dOutBuffer.GetLength();
			int iLen = strlen ( (char *)sToken );
			sToken[iLen] = ' ';
			dOutBuffer.Resize ( iPos+iLen+1 );
			memcpy ( &dOutBuffer[iPos], sToken, iLen+1 );
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
			sphDie ( "read error: %s", strerror(errno) );

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

//////////////////////////////////////////////////////////////////////////

bool FixupFiles ( const CSphVector<CSphString> & dFiles, CSphString & sError )
{
	ARRAY_FOREACH ( i, dFiles )
	{
		const CSphString & sPath = dFiles[i];
		CSphString sKlistOld, sKlistNew, sHeader;
		sKlistOld.SetSprintf ( "%s.spk", sPath.cstr() );
		sKlistNew.SetSprintf ( "%s.new.spk", sPath.cstr() );
		sHeader.SetSprintf ( "%s.sph", sPath.cstr() );

		DWORD iCount = 0;
		{
			CSphAutoreader rdHeader, rdKlistNew, rdKlistOld;
			if ( !rdHeader.Open ( sHeader, sError ) || !rdKlistNew.Open ( sKlistNew, sError ) || !rdKlistOld.Open ( sKlistOld, sError ) )
				return false;

			const SphOffset_t iSize = rdKlistNew.GetFilesize();
			iCount = (DWORD)( iSize / sizeof(SphAttr_t) );
		}

		if ( ::unlink ( sKlistOld.cstr() )!=0 )
		{
			sError.SetSprintf ( "file: '%s', error: '%s'", sKlistOld.cstr(), strerror(errno) );
			return false;
		}

		if ( ::rename ( sKlistNew.cstr(), sKlistOld.cstr() )!=0 )
		{
			sError.SetSprintf ( "files: '%s'->'%s', error: '%s'", sKlistNew.cstr(), sKlistOld.cstr(), strerror(errno) );
			return false;
		}

		int iFD = ::open ( sHeader.cstr(), SPH_O_BINARY | O_RDWR, 0644 );
		if ( iFD<0 )
		{
			sError.SetSprintf ( "file: '%s', error: '%s'", sHeader.cstr(), strerror(errno) );
			return false;
		}

		if ( sphSeek ( iFD, -4, SEEK_END )==-1L )
		{
			sError.SetSprintf ( "file: '%s', error: '%s'", sHeader.cstr(), strerror(errno) );
			SafeClose ( iFD );
			return false;
		}

		if ( ::write ( iFD, &iCount, 4 )==-1 )
		{
			sError.SetSprintf ( "file: '%s', error: '%s'", sHeader.cstr(), strerror(errno) );
			SafeClose ( iFD );
			return false;
		}

		SafeClose ( iFD );
	}

	return true;
}


bool DoKlistsOptimization ( int iRowSize, const char * sPath, int iChunkCount, CSphVector<CSphString> & dFiles )
{
	CSphTightVector<SphDocID_t> dLiveID;

	CSphString sError;

	for ( int iChunk=0; iChunk<iChunkCount; iChunk++ )
	{
		const int64_t tmStart = sphMicroTimer();

		fprintf ( stdout, "\nprocessing '%s.%d'...", sPath, iChunk );

		CSphString sKlist, sAttr, sNew;
		sKlist.SetSprintf ( "%s.%d.spk", sPath, iChunk );
		sAttr.SetSprintf ( "%s.%d.spa", sPath, iChunk );
		sNew.SetSprintf ( "%s.%d.new.spk", sPath, iChunk );

		CSphAutoreader rdKList, rdAttr;
		CSphWriter wrNew;
		if ( !rdKList.Open ( sKlist, sError ) || !rdAttr.Open ( sAttr, sError ) || !wrNew.OpenFile ( sNew, sError ) )
		{
			fprintf ( stdout, "\n%s\n", sError.cstr() );
			return false;
		}

		CSphTightVector<SphAttr_t> dKlist;

		if ( dLiveID.GetLength()>0 )
		{
			assert ( rdKList.GetFilesize()<INT_MAX );

			dKlist.Resize ( (int)( rdKList.GetFilesize()/sizeof(SphAttr_t) ) );
			rdKList.GetBytes ( dKlist.Begin(), (int)rdKList.GetFilesize() );

			// 1nd step kill all k-list ids not in live ids

			ARRAY_FOREACH ( i, dKlist )
			{
				SphDocID_t uid = (SphDocID_t)dKlist[i];
				SphDocID_t * pInLive = sphBinarySearch ( dLiveID.Begin(), &dLiveID.Last(), uid );
				if ( !pInLive )
					dKlist.RemoveFast ( i-- );
			}
			dKlist.Sort();

			// 2nd step kill all prev ids by this fresh k-list

			SphDocID_t * pFirstLive = dLiveID.Begin();
			SphDocID_t * pLastLive = &dLiveID.Last();

			ARRAY_FOREACH ( i, dKlist )
			{
				SphDocID_t uID = (SphDocID_t)dKlist[i];
				SphDocID_t * pKilled = sphBinarySearch ( pFirstLive, pLastLive, uID );

				assert ( pKilled );
				pFirstLive = pKilled+1;
				*pKilled = 0;
			}

#ifndef NDEBUG
			const int iWasLive = dLiveID.GetLength();
#endif

			if ( dKlist.GetLength()>0 )
				ARRAY_FOREACH ( i, dLiveID )
				if ( dLiveID[i]==0 )
					dLiveID.RemoveFast ( i-- );

			assert ( dLiveID.GetLength()+dKlist.GetLength()==iWasLive );

			dLiveID.Sort();
		}

		// 3d step write new k-list

		if ( dKlist.GetLength()>0 )
			wrNew.PutBytes ( dKlist.Begin(), dKlist.GetLength()*sizeof(SphAttr_t) );

		dKlist.Reset();
		wrNew.CloseFile();

		// 4th step merge ID from this segment into live ids
		if ( iChunk!=iChunkCount-1 )
		{
			const int iWasLive = Max ( dLiveID.GetLength()-1, 0 );
			const int iRowCount = (int)( rdAttr.GetFilesize() / ( (DOCINFO_IDSIZE+iRowSize)*4 ) );

			for ( int i=0; i<iRowCount; i++ )
			{
				SphDocID_t uID = 0;
				rdAttr.GetBytes ( &uID, DOCINFO_IDSIZE*4 );
				rdAttr.SkipBytes ( iRowSize*4 );

				if ( sphBinarySearch ( dLiveID.Begin(), dLiveID.Begin()+iWasLive, uID )==NULL )
					dLiveID.Add ( uID );
			}

			dLiveID.Sort();
		}

		CSphString & sFile = dFiles.Add();
		sFile.SetSprintf ( "%s.%d", sPath, iChunk );

		const int64_t tmEnd = sphMicroTimer();
		fprintf ( stdout, "\rprocessed '%s.%d' in %.3f sec", sPath, iChunk, float(tmEnd-tmStart )/1000000.0f );
	}

	return true;
}


#pragma pack(push,4)
struct IDFWord_t
{
	uint64_t	m_uWordID;
	DWORD		m_iDocs;
};
#pragma pack(pop)
STATIC_SIZE_ASSERT	( IDFWord_t, 12 );


bool BuildIDF ( const CSphString & sFilename, const CSphVector<CSphString> & dFiles, CSphString & sError, bool bSkipUnique )
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
		for ( ;; )
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

				strncpy ( sWord, dWords[i], sizeof ( dWords[i] ) );
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


bool MergeIDF ( const CSphString & sFilename, const CSphVector<CSphString> & dFiles, CSphString & sError, bool bSkipUnique )
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


void OptimizeRtKlists ( const CSphString & sIndex, const CSphConfig & hConf )
{
	const int64_t tmStart = sphMicroTimer();

	int iDone = 0;
	CSphVector<CSphString> dFiles;

	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext () )
	{
		CSphString sError;

		const CSphConfigSection & hIndex = hConf["index"].IterateGet ();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( !hIndex("type") || hIndex["type"]!="rt" )
			continue;

		if ( !sIndex.IsEmpty() && sIndex!=sIndexName )
			continue;

		if ( !hIndex.Exists ( "path" ) )
		{
			fprintf ( stdout, "key 'path' not found in index '%s' - skiped\n", sIndexName );
			continue;
		}

		const int64_t tmIndexStart = sphMicroTimer();

		CSphSchema tSchema ( sIndexName );
		CSphColumnInfo tCol;

		// fields
		for ( CSphVariant * v=hIndex("rt_field"); v; v=v->m_pNext )
		{
			tCol.m_sName = v->cstr();
			tSchema.m_dFields.Add ( tCol );
		}
		if ( !tSchema.m_dFields.GetLength() )
		{
			fprintf ( stdout, "index '%s': no fields configured (use rt_field directive) - skiped\n", sIndexName );
			continue;
		}

		// attrs
		const int iNumTypes = 5;
		const char * sTypes[iNumTypes] = { "rt_attr_uint", "rt_attr_bigint", "rt_attr_float", "rt_attr_timestamp", "rt_attr_string" };
		const ESphAttr iTypes[iNumTypes] = { SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, SPH_ATTR_FLOAT, SPH_ATTR_TIMESTAMP, SPH_ATTR_STRING };

		for ( int iType=0; iType<iNumTypes; iType++ )
		{
			for ( CSphVariant * v = hIndex ( sTypes[iType] ); v; v = v->m_pNext )
			{
				tCol.m_sName = v->cstr();
				tCol.m_eAttrType = iTypes[iType];
				tSchema.AddAttr ( tCol, false );
			}
		}

		const char * sPath = hIndex["path"].cstr();

		CSphString sMeta;
		sMeta.SetSprintf ( "%s.meta", sPath );
		CSphAutoreader rdMeta;
		if ( !rdMeta.Open ( sMeta.cstr(), sError ) )
		{
			fprintf ( stdout, "%s\n", sError.cstr() );
			continue;
		}

		rdMeta.SeekTo ( 8, 4 );
		const int iDiskCunkCount = rdMeta.GetDword();

		if ( !DoKlistsOptimization ( tSchema.GetRowSize(), sPath, iDiskCunkCount, dFiles ) )
			sphDie ( "can't cook k-list '%s'", sPath );

		const int64_t tmIndexDone = sphMicroTimer();
		fprintf ( stdout, "\nindex '%s' done in %.3f sec\n", sIndexName, float(tmIndexDone-tmIndexStart )/1000000.0f );
		iDone++;
	}

	const int64_t tmIndexesDone = sphMicroTimer();
	fprintf ( stdout, "\ntotal processed=%d in %.3f sec\n", iDone, float(tmIndexesDone-tmStart )/1000000.0f );

	CSphString sError("none");
	if ( !FixupFiles ( dFiles, sError ) )
		fprintf ( stdout, "error during files fixup: %s\n", sError.cstr() );

	const int64_t tmDone = sphMicroTimer();
	fprintf ( stdout, "\nfinished in %.3f sec\n", float(tmDone-tmStart )/1000000.0f );
}

//////////////////////////////////////////////////////////////////////////

extern void sphDictBuildInfixes ( const char * sPath );
extern void sphDictBuildSkiplists ( const char * sPath );


int main ( int argc, char ** argv )
{
	if ( argc<=1 )
	{
		fprintf ( stdout, SPHINX_BANNER );
		fprintf ( stdout,
			"Usage: indextool <COMMAND> [OPTIONS]\n"
			"\n"
			"Commands are:\n"
			"--build-infixes <INDEX>\tbuild infixes for an existing dict=keywords index\n"
			"\t\t\t(upgrades .sph, .spi in place)\n"
			"--build-skips <INDEX>\tbuild skiplists for an existing index (builds .spe and\n"
			"\t\t\tupgrades .sph, .spi in place)\n"
			"--check <INDEX>\t\tperform index consistency check\n"
			"--checkconfig\t\tperform config consistency check\n"
			"--dumpconfig <SPH-FILE>\tdump index header in config format by file name\n"
			"--dumpdocids <INDEX>\tdump docids by index name\n"
			"--dumpdict <SPI-FILE>\tdump dictionary by file name\n"
			"--dumpdict <INDEX>\tdump dictionary\n"
			"--dumpheader <SPH-FILE>\tdump index header by file name\n"
			"--dumpheader <INDEX>\tdump index header by index name\n"
			"--dumphitlist <INDEX> <KEYWORD>\n"
			"--dumphitlist <INDEX> --wordid <ID>\n"
			"\t\t\tdump hits for a given keyword\n"
			"--fold <INDEX> [FILE]\tfold FILE or stdin using INDEX charset_table\n"
			"--htmlstrip <INDEX>\tfilter stdin using index HTML stripper settings\n"
			"--optimize-rt-klists <INDEX>\n"
			"\t\t\toptimize kill list memory use in RT index disk chunks;\n"
			"\t\t\teither for a given index or --all\n"
			"--buildidf <INDEX1.dict> [INDEX2.dict ...] [--skip-uniq] --out <GLOBAL.idf>\n"
			"\t\t\tjoin --stats dictionary dumps into global.idf file\n"
			"--mergeidf <NODE1.idf> [NODE2.idf ...] [--skip-uniq] --out <GLOBAL.idf>\n"
			"\t\t\tmerge several .idf files into one file\n"
			"\n"
			"Options are:\n"
			"-c, --config <file>\tuse given config file instead of defaults\n"
			"-q, --quiet\t\tbe quiet, skip banner etc (useful with --fold etc)\n"
			"--strip-path\t\tstrip path from filenames referenced by index\n"
			"\t\t\t(eg. stopwords, exceptions, etc)\n"
			"--stats\t\t\tshow total statistics in the dictionary dump\n"
			"--skip-uniq\t\tskip unique (df=1) words in the .idf files\n"
		);
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
	CSphVector<CSphString> dFiles;
	CSphString sOut;
	bool bStats = false;
	bool bSkipUnique = false;
	CSphString sDumpDict;
	bool bQuiet = false;
	bool bRotate = false;

	enum
	{
		CMD_NOTHING,
		CMD_DUMPHEADER,
		CMD_DUMPCONFIG,
		CMD_DUMPDOCIDS,
		CMD_DUMPHITLIST,
		CMD_DUMPDICT,
		CMD_CHECK,
		CMD_STRIP,
		CMD_OPTIMIZEKLISTS,
		CMD_BUILDINFIXES,
		CMD_MORPH,
		CMD_BUILDSKIPS,
		CMD_BUILDIDF,
		CMD_MERGEIDF,
		CMD_CHECKCONFIG,
		CMD_FOLD
	} eCommand = CMD_NOTHING;

	int i;
	for ( i=1; i<argc; i++ )
	{
		// handle argless options
		if ( argv[i][0]!='-' ) break;
		OPT ( "-q", "--quiet" )		{ bQuiet = true; continue; }
		OPT1 ( "--strip-path" )		{ bStripPath = true; continue; }

		// handle options/commands with 1+ args
		if ( (i+1)>=argc )			break;
		OPT ( "-c", "--config" )	sOptConfig = argv[++i];
		OPT1 ( "--dumpheader" )		{ eCommand = CMD_DUMPHEADER; sDumpHeader = argv[++i]; }
		OPT1 ( "--dumpconfig" )		{ eCommand = CMD_DUMPCONFIG; sDumpHeader = argv[++i]; }
		OPT1 ( "--dumpdocids" )		{ eCommand = CMD_DUMPDOCIDS; sIndex = argv[++i]; }
		OPT1 ( "--check" )			{ eCommand = CMD_CHECK; sIndex = argv[++i]; }
		OPT1 ( "--rotate" )			{ bRotate = true; }
		OPT1 ( "--htmlstrip" )		{ eCommand = CMD_STRIP; sIndex = argv[++i]; }
		OPT1 ( "--build-infixes" )	{ eCommand = CMD_BUILDINFIXES; sIndex = argv[++i]; }
		OPT1 ( "--build-skips" )	{ eCommand = CMD_BUILDSKIPS; sIndex = argv[++i]; }
		OPT1 ( "--morph" )			{ eCommand = CMD_MORPH; sIndex = argv[++i]; }
		OPT1 ( "--checkconfig" )	{ eCommand = CMD_CHECKCONFIG; }
		OPT1 ( "--optimize-rt-klists" )
		{
			eCommand = CMD_OPTIMIZEKLISTS;
			sIndex = argv[++i];
			if ( sIndex=="--all" )
				sIndex = "";
		}
		OPT1 ( "--dumpdict" )
		{
			eCommand = CMD_DUMPDICT;
			sDumpDict = argv[++i];
			if ( (i+1)<argc && !strcmp ( argv[i+1], "--stats" ) )
			{
				bStats = true;
				i++;
			}
		}
		OPT1 ( "--fold" )
		{
			eCommand = CMD_FOLD;
			sIndex = argv[++i];
			if ( (i+1)<argc && argv[i+1][0]!='-' )
				sFoldFile = argv[++i];
		}

		// options with 2 args
		else if ( (i+2)>=argc ) // NOLINT
		{
			// not enough args
			break;

		} else if ( !strcmp ( argv[i], "--dumphitlist" ) )
		{
			eCommand = CMD_DUMPHITLIST;
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
			eCommand = !strcmp ( argv[i], "--buildidf" ) ? CMD_BUILDIDF : CMD_MERGEIDF;
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
	for ( ;; )
	{
		if ( eCommand==CMD_BUILDIDF || eCommand==CMD_MERGEIDF )
			break;

		if ( eCommand==CMD_DUMPDICT && !sDumpDict.Ends ( ".spi" ) )
				sIndex = sDumpDict;

		sphLoadConfig ( sOptConfig, bQuiet, cp );
		break;
	}

	///////////
	// action!
	///////////
	int iMvaDefault = 1048576;
	if ( hConf.Exists ( "searchd" ) && hConf["searchd"].Exists ( "searchd" ) )
	{
		const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];
		iMvaDefault = hSearchd.GetSize ( "mva_updates_pool", iMvaDefault );
	}
	const char * sArenaError = sphArenaInit ( iMvaDefault );
	if ( sArenaError )
		sphWarning ( "process shared mutex unsupported, persist MVA disabled ( %s )", sArenaError );


	if ( eCommand==CMD_CHECKCONFIG )
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
			if ( pType && ( *pType=="rt" || *pType=="distributed" ) )
				continue;

			// checking index presence by sph file available
			CSphString sHeader, sError;
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

	// configure common settings (as of time of this writing, AOT and RLP setup)
	sphConfigureCommon ( hConf );

	// common part for several commands, check and preload index
	CSphIndex * pIndex = NULL;
	while ( !sIndex.IsEmpty() && eCommand!=CMD_OPTIMIZEKLISTS )
	{
		// check config
		if ( !hConf["index"](sIndex) )
			sphDie ( "index '%s': no such index in config\n", sIndex.cstr() );

		// only need config-level settings for --htmlstrip
		if ( eCommand==CMD_STRIP )
			break;

		if ( !hConf["index"][sIndex]("path") )
			sphDie ( "index '%s': missing 'path' in config'\n", sIndex.cstr() );

		// only need path for --build-infixes, it will access the files directly
		if ( eCommand==CMD_BUILDINFIXES )
			break;

		// preload that index
		CSphString sError;
		bool bDictKeywords = true;
		if ( hConf["index"][sIndex].Exists ( "dict" ) )
			bDictKeywords = ( hConf["index"][sIndex]["dict"]!="crc" );

		if ( hConf["index"][sIndex]("type") && hConf["index"][sIndex]["type"]=="rt" )
		{
			CSphSchema tSchema;
			if ( sphRTSchemaConfigure ( hConf["index"][sIndex], &tSchema, &sError ) )
				pIndex = sphCreateIndexRT ( tSchema, sIndex.cstr(), 32*1024*1024, hConf["index"][sIndex]["path"].cstr(), bDictKeywords );
		} else
		{
			const char * sPath = hConf["index"][sIndex]["path"].cstr();
			CSphStringBuilder tPath;
			if ( bRotate )
			{
				tPath.Appendf ( "%s.tmp", sPath );
				sPath = tPath.cstr();
			}
			pIndex = sphCreateIndexPhrase ( sIndex.cstr(), sPath );
		}

		if ( !pIndex )
			sphDie ( "index '%s': failed to create (%s)", sIndex.cstr(), sError.cstr() );

		if ( eCommand==CMD_CHECK )
			pIndex->SetDebugCheck();

		CSphString sWarn;
		if ( !pIndex->Prealloc ( false, bStripPath, sWarn ) )
			sphDie ( "index '%s': prealloc failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

		if ( eCommand==CMD_MORPH )
			break;

		if ( !pIndex->Preread() )
			sphDie ( "index '%s': preread failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

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
	switch ( eCommand )
	{
		case CMD_NOTHING:
			sphDie ( "nothing to do; specify a command (run indextool w/o switches for help)" );

		case CMD_DUMPHEADER:
		case CMD_DUMPCONFIG:
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
			pIndex->DebugDumpHeader ( stdout, sDumpHeader.cstr(), eCommand==CMD_DUMPCONFIG );
			break;
		}

		case CMD_DUMPDOCIDS:
			fprintf ( stdout, "dumping docids for index '%s'...\n", sIndex.cstr() );
			pIndex->DebugDumpDocids ( stdout );
			break;

		case CMD_DUMPHITLIST:
			fprintf ( stdout, "dumping hitlist for index '%s' keyword '%s'...\n", sIndex.cstr(), sKeyword.cstr() );
			pIndex->DebugDumpHitlist ( stdout, sKeyword.cstr(), bWordid );
			break;

		case CMD_DUMPDICT:
		{
			if ( sDumpDict.Ends ( ".spi" ) )
			{
				fprintf ( stdout, "dumping dictionary file '%s'...\n", sDumpDict.cstr() );

				sIndex = sDumpDict.SubString ( 0, sDumpDict.Length()-4 );
				pIndex = sphCreateIndexPhrase ( sIndex.cstr(), sIndex.cstr() );

				CSphString sError;
				if ( !pIndex )
					sphDie ( "index '%s': failed to create (%s)", sIndex.cstr(), sError.cstr() );

				CSphString sWarn;
				if ( !pIndex->Prealloc ( false, bStripPath, sWarn ) )
					sphDie ( "index '%s': prealloc failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

				if ( !pIndex->Preread() )
					sphDie ( "index '%s': preread failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );
			} else
				fprintf ( stdout, "dumping dictionary for index '%s'...\n", sIndex.cstr() );

			if ( bStats )
				fprintf ( stdout, "total-documents: " INT64_FMT "\n", pIndex->GetStats().m_iTotalDocuments );
			pIndex->DebugDumpDict ( stdout );
			break;
		}

		case CMD_CHECK:
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

		case CMD_STRIP:
			{
				const CSphConfigSection & hIndex = hConf["index"][sIndex];
				if ( hIndex.GetInt ( "html_strip" )==0 )
					sphDie ( "HTML stripping is not enabled in index '%s'", sIndex.cstr() );
				StripStdin ( hIndex.GetStr ( "html_index_attrs" ), hIndex.GetStr ( "html_remove_elements" ) );
			}
			break;

		case CMD_OPTIMIZEKLISTS:
			OptimizeRtKlists ( sIndex, hConf );
			break;

		case CMD_BUILDINFIXES:
			{
				const CSphConfigSection & hIndex = hConf["index"][sIndex];
				if ( hIndex("type") && hIndex["type"]=="rt" )
					sphDie ( "build-infixes requires a disk index" );
				if ( !hIndex("dict") || hIndex["dict"]!="keywords" )
					sphDie ( "build-infixes requires dict=keywords" );

				fprintf ( stdout, "building infixes for index %s...\n", sIndex.cstr() );
				sphDictBuildInfixes ( hIndex["path"].cstr() );
			}
			break;

		case CMD_BUILDSKIPS:
			{
				const CSphConfigSection & hIndex = hConf["index"][sIndex];
				if ( hIndex("type") && hIndex["type"]=="rt" )
					sphDie ( "build-infixes requires a disk index" );

				fprintf ( stdout, "building skiplists for index %s...\n", sIndex.cstr() );
				sphDictBuildSkiplists ( hIndex["path"].cstr() );
			}
			break;

		case CMD_MORPH:
			ApplyMorphology ( pIndex );
			break;

		case CMD_BUILDIDF:
		{
			CSphString sError;
			if ( !BuildIDF ( sOut, dFiles, sError, bSkipUnique ) )
				sphDie ( "ERROR: %s\n", sError.cstr() );
			break;
		}

		case CMD_MERGEIDF:
		{
			CSphString sError;
			if ( !MergeIDF ( sOut, dFiles, sError, bSkipUnique ) )
				sphDie ( "ERROR: %s\n", sError.cstr() );
			break;
		}

		case CMD_FOLD:
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
			sphDie ( "INTERNAL ERROR: unhandled command (id=%d)", (int)eCommand );
	}

	return 0;
}

//
// $Id$
//
