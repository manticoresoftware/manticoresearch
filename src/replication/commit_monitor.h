//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "accumulator.h"
#include "searchdaemon.h"

// interface to pass into static Replicate to issue commit for specific command
class CommitMonitor_c
{
public:
	explicit CommitMonitor_c ( RtAccum_t& tAcc )
		: m_tAcc ( tAcc )
	{}

	CommitMonitor_c ( RtAccum_t& tAcc, CSphString* pWarning, int* pUpdated )
		: m_tAcc ( tAcc )
		, m_pWarning ( pWarning )
		, m_pUpdated ( pUpdated )
	{}
	CommitMonitor_c ( RtAccum_t& tAcc, int* pDeletedCount, CSphString* pWarning, int* pUpdated )
		: m_tAcc ( tAcc )
		, m_pDeletedCount ( pDeletedCount )
		, m_pWarning ( pWarning )
		, m_pUpdated ( pUpdated )
	{}

	~CommitMonitor_c();

	// commit for common commands
	bool Commit ();

	// commit for Total Order Isolation commands
	bool CommitTOI ();

	// update with Total Order Isolation
	bool UpdateTOI ();

private:
	RtAccum_t & m_tAcc;
	int * m_pDeletedCount = nullptr;
	CSphString * m_pWarning = nullptr;
	int * m_pUpdated = nullptr;

	bool CommitNonEmptyCmds ( RtIndex_i* pIndex, const ReplicationCommand_t& tCmd, bool bOnlyTruncate ) const;
};
