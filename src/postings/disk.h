// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "stream.h"
#include "fileio.h"

namespace postings
{

constexpr int FILE_HEADER_SIZE = 9;
void WriteFileHeader ( CSphWriter & tWriter, const char * sMagic );
bool CheckFileHeader ( const CSphString & sFile, const char * sMagic, CSphString & sError );

class DiskOutput_c final : public Output_i
{
public:
	explicit DiskOutput_c ( CSphWriter & tWriter ) :	m_tWriter(tWriter) {}

	uint64_t	GetPos() const final	{ return m_tWriter.GetPos(); }
	bool		Write ( Bytes_t tData, std::string & sError ) final;
	bool		Patch ( uint64_t uOffset, Bytes_t tData, std::string & sError ) final;

private:
	CSphWriter &	m_tWriter;
};

class DiskWriter_c
{
public:
	explicit DiskWriter_c ( CSphWriter & tWriter );

	bool	Begin ( unsigned uFields, bool bPositions, CSphString & sError );
	bool	Add ( const Posting_t & tPosting, CSphString & sError );
	bool	Finish ( CSphWriter & tSkips, CSphString & sError );

private:
	DiskOutput_c	m_tOutput;
	Writer_c		m_tWriter;
	std::string		m_sError;
};

}
