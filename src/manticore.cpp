// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)

#include "manticorecli.h"

#include <cstdio>

int main ( int argc, char ** argv )
{
	auto tResult = manticorecli::ParseArgs ( argc, argv );
	if ( !tResult.m_bOk )
	{
		fprintf ( stderr, "manticore: %s\nRun 'manticore --help' for usage.\n", tResult.m_sError.c_str() );
		return 2;
	}
	return manticorecli::Run ( tResult.m_tOptions, argv[0] );
}
