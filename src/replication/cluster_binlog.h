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

/////////////////////////////////////////////////////////////////////////////
/// cluster binlog

#include "sphinxstd.h"

struct ClusterBinlogData_c : public ClusterDesc_t
{
	Wsrep::GlobalTid_t		m_tGtid;
};

class ClusterBinlog_i : public ISphNonCopyMovable
{
public:
	virtual ~ClusterBinlog_i() = default;

	virtual void Init ( const CSphString & sConfigFile, bool bDisabled ) = 0;
	virtual void Close() = 0;

	virtual void OnClusterDelete ( const CSphString & sCluster, const StrVec_t & dIndexes ) = 0;
	virtual void OnClusterLoad ( ClusterBinlogData_c & tCluster ) = 0;

	virtual void ClusterTnx ( const ClusterBinlogData_c & tCluster ) = 0;
	virtual void OnClusterSynced ( const ClusterBinlogData_c & tCluster ) = 0;
};

ClusterBinlog_i * RplBinlog();