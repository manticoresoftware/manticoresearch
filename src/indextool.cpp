//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
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

	CSphScopedPtr<ISphTokenizer> pTokenizer ( pIndex->GetTokenizer()->Clone ( false ) );
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

//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#include <io.h> // for open()
#define sphSeek		_lseeki64
#else
#define sphSeek		lseek
#endif

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


bool BuildGlobalIDF ( const CSphString & sFilename, const CSphVector<CSphString> & dFiles, CSphString & sError, bool bSkipUnique, bool bTextFile )
{
	typedef CSphOrderedHash<int, uint64_t, IdentityHash_fn, 1024*1024*64> IDFHash_t;

	// preallocate 64M hash
	const int64_t tmStart = sphMicroTimer ();
	IDFHash_t * pHash = new IDFHash_t();
	IDFHash_t & hWords = *pHash;

	int64_t iTotalDocuments = 0;
	int64_t iTotalWords = 0;

	ARRAY_FOREACH ( i, dFiles )
	{
		CSphAutoreader tReader;
		if ( !tReader.Open ( dFiles[i], sError ) )
			return false;

		fprintf ( stdout, "reading %s...\n", dFiles[i].cstr() );

		const SphOffset_t iSize = tReader.GetFilesize();
		if ( iSize > INT_MAX )
		{
			fprintf ( stdout, "ERROR: dictionaries over 2048 MB not supported, skipping %s\n", dFiles[i].cstr() );
			continue;
		}

		char * pData = new char [ (int)iSize ];
		tReader.GetBytes ( pData, (int)iSize );
		char *pEnd = pData+iSize;

		if ( bTextFile )
		{
			char * p = pData;
			while ( p<pEnd )
			{
				// strchr ought to be faster than incremental search
				char * p0 = strchr ( p, '\n' );
				if ( !p0 )
					break;
				*p0 = '\0';

				// find keyword pattern ( ^<keyword>,<docs>,... )
				char * p1 = strchr ( p, ',' );
				if ( p1 )
				{
					char * p2 = strchr ( p1+1, ',' );
					if ( p2 )
					{
						*p1 = *p2 = '\0';

						int iDocs = atoi ( p1+1 );
						if ( iDocs )
						{
							// calculate keyword id
							uint64_t uWordID = sphFNV64 ( (BYTE*)p );

							int * pDocs = hWords ( uWordID );
							if ( pDocs )
								*pDocs += iDocs;
							else
								hWords.Add ( iDocs, uWordID );

							iTotalWords++;
						}
					}
				} else
				{
					// keyword pattern not found (rather rare case), try to parse as a header, then
					char sSearch[] = "total-documents: ";
					if ( strstr ( p, sSearch )==p )
						iTotalDocuments += atoi ( p+strlen(sSearch) ); // sizeof may vary
				}

				// advance to the next line
				p = p0+1;
			}
		} else
		{
			// parse binary file
			char * p = pData;

			iTotalDocuments += *(int64_t*)p;
			p += sizeof(int64_t);

			while ( p<pEnd )
			{
				uint64_t uWordID = *(uint64_t*)p;
				p += sizeof(uint64_t);

				int iDocs = int(*(DWORD*)p);
				p += sizeof(DWORD);

				int * pDocs = hWords ( uWordID );
				if ( pDocs )
					*pDocs += iDocs;
				else
					hWords.Add ( iDocs, uWordID );

				iTotalWords++;
			}
		}
		SafeDeleteArray ( pData );
	}

	// skip unique words and sort by id
	CSphVector<uint64_t> dWords;

	hWords.IterateStart ();
	while ( hWords.IterateNext () )
	{
		uint64_t uWordID = hWords.IterateGetKey ();
		int iDocs = hWords.IterateGet ();
		if ( !bSkipUnique || iDocs>1 )
			dWords.Add ( uWordID );
	}

	dWords.Sort ();

	fprintf ( stdout, INT64_FMT" documents, %d words ("INT64_FMT" read, "INT64_FMT" merged, %d skipped)\n", iTotalDocuments,
		dWords.GetLength(), iTotalWords, iTotalWords-hWords.GetLength(), hWords.GetLength()-dWords.GetLength() );

	// write to disk
	fprintf ( stdout, "writing %s...\n", sFilename.cstr() );

	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sFilename, sError ) )
		return false;

	tWriter.PutOffset ( iTotalDocuments );

	ARRAY_FOREACH ( i, dWords )
	{
		tWriter.PutBytes ( &dWords[i], sizeof(uint64_t) );
		tWriter.PutDword ( *hWords ( dWords[i] ) );
	}

	tWriter.CloseFile ();

	SafeDelete ( pHash );

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
	fprintf ( stdout, SPHINX_BANNER );
	if ( argc<=1 )
	{
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
			"--htmlstrip <INDEX>\tfilter stdin using index HTML stripper settings\n"
			"--optimize-rt-klists <INDEX>\n"
			"\t\t\toptimize kill list memory use in RT index disk chunks;\n"
			"\t\t\teither for a given index or --all\n"
			"--buildidf <index1.dict> [index2.dict ...] [--skip-uniq] --out <global.idf>\n"
			"\t\t\tjoin dictionary dumps with statistics (--stats) into an .idf file\n"
			"--mergeidf <node1.idf> [node2.idf ...] [--skip-uniq] --out <global.idf>\n"
			"\t\t\tmerge several .idf files into one file\n"
			"\n"
			"Options are:\n"
			"-c, --config <file>\tuse given config file instead of defaults\n"
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
	CSphString sDumpHeader, sIndex, sKeyword;
	bool bWordid = false;
	bool bStripPath = false;
	CSphVector<CSphString> dFiles;
	CSphString sOut;
	bool bStats = false;
	bool bSkipUnique = false;
	CSphString sDumpDict;

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
		CMD_CHECKCONFIG
	} eCommand = CMD_NOTHING;

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( argv[i][0]!='-' )
			break;

		// this is an option
		if ( (i+1)>=argc )			break;
		OPT ( "-c", "--config" )	sOptConfig = argv[++i];
		OPT1 ( "--dumpheader" )		{ eCommand = CMD_DUMPHEADER; sDumpHeader = argv[++i]; }
		OPT1 ( "--dumpconfig" )		{ eCommand = CMD_DUMPCONFIG; sDumpHeader = argv[++i]; }
		OPT1 ( "--dumpdocids" )		{ eCommand = CMD_DUMPDOCIDS; sIndex = argv[++i]; }
		OPT1 ( "--check" )			{ eCommand = CMD_CHECK; sIndex = argv[++i]; }
		OPT1 ( "--htmlstrip" )		{ eCommand = CMD_STRIP; sIndex = argv[++i]; }
		OPT1 ( "--build-infixes" )	{ eCommand = CMD_BUILDINFIXES; sIndex = argv[++i]; }
		OPT1 ( "--build-skips" )	{ eCommand = CMD_BUILDSKIPS; sIndex = argv[++i]; }
		OPT1 ( "--morph" )			{ eCommand = CMD_MORPH; sIndex = argv[++i]; }
		OPT1 ( "--strip-path" )		{ bStripPath = true; }
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
	if ( i!=argc )
	{
		fprintf ( stdout, "ERROR: malformed or unknown option near '%s'.\n", argv[i] );
		return 1;
	}

	//////////////////////
	// load proper config
	//////////////////////

	CSphConfigParser cp;
	CSphConfig & hConf = cp.m_tConf;
	for ( ;; )
	{
		if ( ( eCommand==CMD_DUMPHEADER || eCommand==CMD_DUMPCONFIG ) && sDumpHeader.Ends ( ".sph" ) )
			break;

		if ( eCommand==CMD_BUILDIDF || eCommand==CMD_MERGEIDF )
			break;

		if ( eCommand==CMD_DUMPDICT )
		{
			if ( sDumpDict.Ends ( ".spi" ) )
				break;
			sIndex = sDumpDict;
		}

		sphLoadConfig ( sOptConfig, false, cp );
		break;
	}

	///////////
	// action!
	///////////

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
			const CSphString * pPath = tIndex ( "path" );
			if ( !pPath )
				continue;

			const CSphString * pType = tIndex ( "type" );
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
		bool bDictKeywords = false;
		if ( hConf["index"][sIndex].Exists ( "dict" ) )
			bDictKeywords = ( hConf["index"][sIndex]["dict"]=="keywords" );

		if ( hConf["index"][sIndex]("type") && hConf["index"][sIndex]["type"]=="rt" )
		{
			CSphSchema tSchema;
			if ( sphRTSchemaConfigure ( hConf["index"][sIndex], &tSchema, &sError ) )
				pIndex = sphCreateIndexRT ( tSchema, sIndex.cstr(), 32*1024*1024, hConf["index"][sIndex]["path"].cstr(), bDictKeywords );
		} else
		{
			pIndex = sphCreateIndexPhrase ( sIndex.cstr(), hConf["index"][sIndex]["path"].cstr() );
		}

		if ( !pIndex )
			sphDie ( "index '%s': failed to create (%s)", sIndex.cstr(), sError.cstr() );

		// don't need any long load operations
		// but not for dict=keywords + infix
		pIndex->SetWordlistPreload ( bDictKeywords );

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

			const CSphString & sValue = hConf["index"][sIndex]["hitless_words"];
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

			CSphIndex * pIndex = sphCreateIndexPhrase ( sIndexName.cstr(), "" );
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

				pIndex->SetWordlistPreload ( true );

				CSphString sWarn;
				if ( !pIndex->Prealloc ( false, bStripPath, sWarn ) )
					sphDie ( "index '%s': prealloc failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

				if ( !pIndex->Preread() )
					sphDie ( "index '%s': preread failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );
			} else
				fprintf ( stdout, "dumping dictionary for index '%s'...\n", sIndex.cstr() );

			if ( bStats )
				fprintf ( stdout, "total-documents: %d\n", pIndex->GetStats().m_iTotalDocuments );
			pIndex->DebugDumpDict ( stdout );
			break;
		}

		case CMD_CHECK:
			fprintf ( stdout, "checking index '%s'...\n", sIndex.cstr() );
			return pIndex->DebugCheck ( stdout );

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
			if ( !BuildGlobalIDF ( sOut, dFiles, sError, bSkipUnique, true ) )
				fprintf ( stdout, "ERROR: %s\n", sError.cstr() );
			break;
		}

		case CMD_MERGEIDF:
		{
			CSphString sError;
			if ( !BuildGlobalIDF ( sOut, dFiles, sError, bSkipUnique, false ) )
				fprintf ( stdout, "ERROR: %s\n", sError.cstr() );
			break;
		}

		default:
			sphDie ( "INTERNAL ERROR: unhandled command (id=%d)", (int)eCommand );
	}

	return 0;
}

//
// $Id$
//
