//
// $Id$
//

//
// Copyright (c) 2001-2011, Andrew Aksyonoff
// Copyright (c) 2008-2011, Sphinx Technologies Inc
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

void DoOptimization ( const CSphString & sIndex, const CSphConfig & hConfig );

int main ( int argc, char ** argv )
{
	fprintf ( stdout, SPHINX_BANNER );
	if ( argc<=1 )
	{
		fprintf ( stdout,
			"Usage: indextool <COMMAND> [OPTIONS]\n"
			"\n"
			"Commands are:\n"
			"--dumpheader <FILENAME.sph>\tdump index header by file name\n"
			"--dumpconfig <FILENAME.sph>\tdump index header in config format by file name\n"
			"--dumpheader <INDEXNAME>\tdump index header by index name\n"
			"--dumpdocids <INDEXNAME>\tdump docids by index name\n"
			"--dumphitlist <INDEXNAME> <KEYWORD>\n"
			"--dumphitlist <INDEXNAME> --wordid <ID>\n"
			"\t\t\t\tdump hits for given keyword\n"
			"--check <INDEXNAME>\t\tperform index consistency check\n"
			"--htmlstrip <INDEXNAME>\t\tfilter stdin usng HTML stripper settings\n"
			"\t\t\t\tfor a given index (taken from sphinx.conf)\n"
			"--optimize-rt-klists <INDEXNAME>\n"
			"\t\t\t\tperform kill list opimization in rt's disk chunks\n"
			"\t\t\t\tfor a given index (taken from sphinx.conf) or --all\n"
			"\n"
			"Options are:\n"
			"-c, --config <file>\t\tuse given config file instead of defaults\n"
			"--strip-path\t\t\tstrip path from filenames referenced by index\n"
			"\t\t\t\t(eg. stopwords, exceptions, etc)\n"
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

	enum
	{
		CMD_NOTHING,
		CMD_DUMPHEADER,
		CMD_DUMPCONFIG,
		CMD_DUMPDOCIDS,
		CMD_DUMPHITLIST,
		CMD_CHECK,
		CMD_STRIP,
		CMD_OPTIMIZE
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
		OPT1 ( "--strip-path" )		{ bStripPath = true; }
		OPT1 ( "--optimize-rt-klists" )
		{
			eCommand = CMD_OPTIMIZE;
			sIndex = argv[++i];
			if ( sIndex=="--all" )
				sIndex = "";
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

		sphLoadConfig ( sOptConfig, false, cp );
		break;
	}

	///////////
	// action!
	///////////

	// common part for several commands, check and preload index
	CSphIndex * pIndex = NULL;
	while ( !sIndex.IsEmpty() && eCommand!=CMD_OPTIMIZE )
	{
		// check config
		if ( !hConf["index"](sIndex) )
			sphDie ( "index '%s': no such index in config\n", sIndex.cstr() );

		if ( eCommand==CMD_STRIP )
			break;

		if ( !hConf["index"][sIndex]("path") )
			sphDie ( "index '%s': missing 'path' in config'\n", sIndex.cstr() );

		// preload that index
		CSphString sError;
		if ( hConf["index"][sIndex]("type") && hConf["index"][sIndex]["type"]=="rt" )
		{
			CSphSchema tSchema;
			if ( sphRTSchemaConfigure ( hConf["index"][sIndex], &tSchema, &sError ) )
				pIndex = sphCreateIndexRT ( tSchema, sIndex.cstr(), 32*1024*1024, hConf["index"][sIndex]["path"].cstr() );
		} else
		{
			pIndex = sphCreateIndexPhrase ( sIndex.cstr(), hConf["index"][sIndex]["path"].cstr() );
		}

		if ( !pIndex )
			sphDie ( "index '%s': failed to create (%s)", sIndex.cstr(), sError.cstr() );

		// don't need any long load operations
		pIndex->SetWordlistPreload ( false );

		CSphString sWarn;
		if ( !pIndex->Prealloc ( false, bStripPath, sWarn ) )
			sphDie ( "index '%s': prealloc failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

		if ( !pIndex->Preread() )
			sphDie ( "index '%s': preread failed: %s\n", sIndex.cstr(), pIndex->GetLastError().cstr() );

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
			if ( hConf("index") && hConf["index"](sDumpHeader) )
			{
				fprintf ( stdout, "dumping header for index '%s'...\n", sDumpHeader.cstr() );

				if ( !hConf["index"][sDumpHeader]("path") )
					sphDie ( "missing 'path' for index '%s'\n", sDumpHeader.cstr() );

				sDumpHeader.SetSprintf ( "%s.sph", hConf["index"][sDumpHeader]["path"].cstr() );
			}

			fprintf ( stdout, "dumping header file '%s'...\n", sDumpHeader.cstr() );
			CSphIndex * pIndex = sphCreateIndexPhrase ( NULL, "" );
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

		case CMD_OPTIMIZE:
			DoOptimization ( sIndex, hConf );
			break;

		default:
			sphDie ( "INTERNAL ERROR: unhandled command (id=%d)", (int)eCommand );
	}

	return 0;
}

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

void DoOptimization ( const CSphString & sIndex, const CSphConfig & hConf )
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

//
// $Id$
//
