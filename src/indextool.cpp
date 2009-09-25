//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"
#include <time.h>


void StripStdin ( const char * sIndexAttrs, const char * sRemoveElements )
{
	CSphString sError;
	CSphHTMLStripper tStripper;
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
			"--dumpheader <INDEXNAME>\tdump index header by index name\n"
			"--dumpdocids <INDEXNAME>\tdump docids by index name\n"
			"--dumphitlist <INDEXNAME> <KEYWORD>\n"
			"--dumphitlist <INDEXNAME> --wordid <ID>\n"
			"\t\t\t\tdump hits for given keyword\n"
			"--check <INDEXNAME>\t\tperform index consistency check\n"
			"--htmlstrip <INDEXNAME>\t\tfilter stdin usng HTML stripper settings\n"
			"\t\t\t\tfor a given index (taken from sphinx.conf)\n"
			"\n"
			"Options are:\n"
			"-c, --config <file>\t\tuse given config file instead of defaults\n"
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

	enum
	{
		CMD_NOTHING,
		CMD_DUMPHEADER,
		CMD_DUMPDOCIDS,
		CMD_DUMPHITLIST,
		CMD_CHECK,
		CMD_STRIP
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
		OPT1 ( "--dumpdocids" )		{ eCommand = CMD_DUMPDOCIDS; sIndex = argv[++i]; }
		OPT1 ( "--check" )			{ eCommand = CMD_CHECK; sIndex = argv[++i]; }
		OPT1 ( "--htmlstrip" )		{ eCommand = CMD_STRIP; sIndex = argv[++i]; }

		// options with 2 args
		else if ( (i+2)>=argc )
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
	sphLoadConfig ( sOptConfig, false, cp );

	///////////
	// action!
	///////////

	// common part for several commands, check and preload index
	CSphIndex * pIndex = NULL;
	while ( !sIndex.IsEmpty() )
	{
		// check config
		if ( !hConf["index"](sIndex) )
			sphDie ( "index '%s': no such index in config\n", sIndex.cstr() );

		if ( eCommand==CMD_STRIP )
			break;

		if ( !hConf["index"][sIndex]("path") )
			sphDie ( "index '%s': missing 'path' in config'\n", sIndex.cstr() );

		// preload that index
		pIndex = sphCreateIndexPhrase ( hConf["index"][sIndex]["path"].cstr() );
		if ( !pIndex )
			sphDie ( "index '%s': failed to create", sIndex.cstr() );

		CSphString sWarn;
		if ( !pIndex->Prealloc ( false, sWarn ) )
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
		{
			if ( hConf["index"](sDumpHeader) )
			{
				fprintf ( stdout, "dumping header for index '%s'...\n", sDumpHeader.cstr() );

				if ( !hConf["index"][sDumpHeader]("path") )
					sphDie ( "missing 'path' for index '%s'\n", sDumpHeader.cstr() );

				sDumpHeader.SetSprintf ( "%s.sph", hConf["index"][sDumpHeader]["path"].cstr() );
			}

			fprintf ( stdout, "dumping header file '%s'...\n", sDumpHeader.cstr() );
			CSphIndex * pIndex = sphCreateIndexPhrase ( "" );
			pIndex->DebugDumpHeader ( stdout, sDumpHeader.cstr() );
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

		default:
			sphDie ( "INTERNAL ERROR: unhandled command (id=%d)", (int)eCommand );
	}

	return 0;
}

//
// $Id$
//
