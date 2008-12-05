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
	CSphString sDumpHeader, sDumpDocids;

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( argv[i][0]!='-' )
			break;

		// this is an option
		if ( (i+1)>=argc )		break;
		OPT ( "-c", "--config" )	sOptConfig = argv[++i];
		OPT1 ( "--dumpheader" )		sDumpHeader = argv[++i];
		OPT1 ( "--dumpdocids" )		sDumpDocids = argv[++i];
		else break; // unknown option
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

	if ( !sDumpHeader.IsEmpty() )
	{
		if ( hConf["index"](sDumpHeader) )
		{
			fprintf ( stdout, "dumping header for index '%s'...\n", sDumpHeader.cstr() );

			if ( !hConf["index"][sDumpHeader]("path") )
				sphDie ( "missing 'path' for index '%s'\n", sDumpHeader.cstr() );

			sDumpHeader.SetSprintf ( "%s.sph", hConf["index"][sDumpHeader]["path"].cstr() );
		}

		fprintf ( stdout, "dumping header '%s'...\n", sDumpHeader.cstr() );
		CSphIndex * pIndex = sphCreateIndexPhrase ( "" );
		pIndex->DebugDumpHeader ( stdout, sDumpHeader.cstr() );

	} else if ( !sDumpDocids.IsEmpty() )
	{
		if ( !hConf["index"](sDumpDocids) )
			sphDie ( "index '%s': no such index in config\n", sDumpDocids.cstr() );

		if ( !hConf["index"][sDumpDocids]("path") )
			sphDie ( "index '%s': missing 'path' in config'\n", sDumpDocids.cstr() );

		CSphIndex * pIndex = sphCreateIndexPhrase ( hConf["index"][sDumpDocids]["path"].cstr() );
		if ( !pIndex )
			sphDie ( "index '%s': failed to create", sDumpDocids.cstr() );

		CSphString sWarn;
		if ( !pIndex->Prealloc ( false, sWarn ) )
			sphDie ( "index '%s': prealloc failed: %s\n", sDumpDocids.cstr(), pIndex->GetLastError().cstr() );

		if ( !pIndex->Preread() )
			sphDie ( "index '%s': preread failed: %s\n", sDumpDocids.cstr(), pIndex->GetLastError().cstr() );

		fprintf ( stdout, "dumping docids for index '%s'...\n", sDumpDocids.cstr() );
		pIndex->DebugDumpDocids ( stdout );

	} else
	{
		sphDie ( "nothing to do; specify a command (run indextool w/o switches for help)" );
	}
}

//
// $Id$
//
