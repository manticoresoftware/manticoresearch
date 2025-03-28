//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "grastate.h"

#include "sphinxutils.h"
#include "fileutils.h"
#include "fileio.h"

#include <optional>

const char* dGaleraFiles[] = { "grastate.dat", "galera.cache" };
const char* szGrastatePath = "%s/grastate.dat";
const char* szGrastatePathNew = "%s/grastate.dat.new";
const char* szGrastatePathOld = "%s/grastate.dat.old";

const auto sSafePattern = FROMS ( "safe_to_bootstrap" );
const auto sSafeMsg = FROMS ( "safe_to_bootstrap: 1" );


static bool FilterGrastate ( const CSphString& sPath, std::function<bool ( Str_t )> fnGraFilter )
{
	auto sClusterState = SphSprintf ( szGrastatePath, sPath.cstr() );

	// cluster starts well without grastate.dat file
	if ( !sphIsReadable ( sClusterState ) )
		return true;

	TLS_MSG_STRING ( sError );
	CSphAutoreader tReader;
	if ( !tReader.Open ( sClusterState, sError ) )
		return false;

	CSphFixedVector<char> dBuf { 2048 };
	SphOffset_t iStateSize = tReader.GetFilesize();
	while ( tReader.GetPos() < iStateSize )
	{
		auto iLineLen = tReader.GetLine ( dBuf.Begin(), (int)dBuf.GetLengthBytes() );
		if ( !fnGraFilter ( { dBuf.Begin(), iLineLen } ) )
			break;
	}
	return true;
}

bool CheckClusterNew ( const CSphString& sPath )
{
	std::optional<int> iVal;
	FilterGrastate ( sPath, [&iVal] ( Str_t sLine ) {
		auto dKeyVal = sphSplit ( sLine.first, sLine.second, ":" );
		if ( dKeyVal.GetLength() < 2 )
			return true;

		dKeyVal.Apply ( [] ( CSphString& sVal ) { sVal.Trim(); } );
		if ( dKeyVal[0] != sSafePattern )
			return true;

		assert ( dKeyVal.GetLength() > 2 );
		iVal = atoi ( dKeyVal[1].cstr() );

		return false;
	} );

	return !iVal.has_value() // can start cluster without any safe_to_bootstrap flag
		|| iVal == 1
		|| TlsMsg::Err ( "can not start cluster without 'safe_to_bootstrap: 1' at the '%s/grastate.dat', got '%d', use '--new-cluster-force' to bootstrap this node and bypassing cluster restart protection", sPath.cstr(), iVal.value_or(0) );
}


// set safe_to_bootstrap: 1 at cluster/grastate.dat for Galera to start properly
bool NewClusterForce ( const CSphString& sPath )
{
	auto sClusterState = SphSprintf ( szGrastatePath, sPath.cstr() );
	auto sNewState = SphSprintf ( szGrastatePathNew, sPath.cstr() );
	auto sOldState = SphSprintf ( szGrastatePathOld, sPath.cstr() );

	TLS_MSG_STRING ( sError );

	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sNewState, sError ) )
		return false;

	FilterGrastate ( sPath, [&tWriter] ( Str_t sLine ) {
		AT_SCOPE_EXIT ( [&]() { tWriter.PutBytes ( sLine.first, sLine.second ); tWriter.PutByte ( '\n' ); } );
		auto dKeyVal = sphSplit ( sLine.first, sLine.second, ":" );
		if ( dKeyVal.GetLength() < 2 )
			return true;

		dKeyVal.Apply ( [] ( CSphString& sVal ) { sVal.Trim(); } );
		if ( dKeyVal[0] == sSafePattern )
			sLine = sSafeMsg;

		return true;
	} );

	if ( TlsMsg::HasErr() || tWriter.IsError() )
		return false;

	tWriter.CloseFile();

	if ( sph::rename ( sClusterState.cstr(), sOldState.cstr() ) != 0 )
		return TlsMsg::Err ( "failed to rename %s to %s", sClusterState.cstr(), sOldState.cstr() );

	if ( sph::rename ( sNewState.cstr(), sClusterState.cstr() ) != 0 )
		return TlsMsg::Err ( "failed to rename %s to %s", sNewState.cstr(), sClusterState.cstr() );

	::unlink ( sOldState.cstr() );
	return true;
}


// clean up Galera files at cluster path to start new and fresh cluster again
void CleanClusterFiles ( const CSphString& sPath )
{
	for ( const char* szFile : dGaleraFiles )
		::unlink ( SphSprintf ( "%s/%s", sPath.cstr(), szFile ).cstr() );
}