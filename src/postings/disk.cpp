// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "disk.h"

namespace postings
{

void WriteFileHeader ( CSphWriter & tWriter, const char * sMagic )
{
	tWriter.PutByte(1);
	tWriter.PutBytes ( sMagic, 4 );
	tWriter.PutDword(COMPONENT_VERSION);
}


bool CheckFileHeader ( const CSphString & sFile, const char * sMagic, CSphString & sError )
{
	CSphAutoreader tReader;
	if ( !tReader.Open ( sFile, sError ) )
		return false;

	BYTE dHeader[FILE_HEADER_SIZE] {};
	tReader.GetBytes ( dHeader, sizeof(dHeader) );
	if ( tReader.GetErrorFlag() || dHeader[0]!=1 || memcmp ( dHeader+1, sMagic, 4 ) || dHeader[5]!=COMPONENT_VERSION || dHeader[6] || dHeader[7] || dHeader[8] )
	{
		sError.SetSprintf ( "invalid block postings file header: %s", sFile.cstr() );
		return false;
	}

	return true;
}


bool DiskOutput_c::Write ( Bytes_t tData, std::string & sError )
{
	if ( tData.m_iSize )
		m_tWriter.PutBytes ( tData.m_pData, tData.m_iSize );

	if ( !m_tWriter.IsError() )
		return true;

	sError = "posting file write failed";
	return false;
}


bool DiskOutput_c::Patch ( uint64_t uOffset, Bytes_t tData, std::string & sError )
{
	SphOffset_t iEnd = m_tWriter.GetPos();
	// SeekTo inside the live write buffer discards its tail. Flush before patching.
	m_tWriter.Flush();
	m_tWriter.SeekTo(uOffset);
	bool bOK = Write ( tData, sError );
	m_tWriter.Flush();
	m_tWriter.SeekTo(iEnd);
	return bOK && !m_tWriter.IsError();
}


DiskWriter_c::DiskWriter_c ( CSphWriter & tWriter ) : m_tOutput(tWriter), m_tWriter(m_tOutput) {}


bool DiskWriter_c::Begin ( unsigned uFields, bool bPositions, CSphString & sError )
{
	TermDesc_t tDesc;
	tDesc.m_uFields = uint16_t(uFields);
	tDesc.m_bPositions = bPositions;
	tDesc.m_bFieldTF = bPositions;
	if ( m_tWriter.Begin ( tDesc, m_sError ) )
		return true;

	sError = m_sError.c_str();
	return false;
}


bool DiskWriter_c::Add ( const Posting_t & tPosting, CSphString & sError )
{
	if ( m_tWriter.Add ( tPosting, m_sError ) )
		return true;

	sError = m_sError.c_str();
	return false;
}


bool DiskWriter_c::Finish ( CSphWriter & tSkips, CSphString & sError )
{
	if ( !m_tWriter.Finish(m_sError) )
	{
		sError = m_sError.c_str();
		return false;
	}

	const auto & dLeaves = m_tWriter.GetLeaves();
	tSkips.PutBytes ( "SKP1", 4 );
	tSkips.PutDword ( DWORD(dLeaves.size()) );
	for ( const auto & tLeaf : dLeaves )
	{
		tSkips.PutDword ( tLeaf.m_uFirst );
		tSkips.PutDword ( tLeaf.m_uLast );
		tSkips.PutOffset ( tLeaf.m_uOffset );
		tSkips.PutDword ( tLeaf.m_uMaxTF );
	}

	return !tSkips.IsError();
}

}
