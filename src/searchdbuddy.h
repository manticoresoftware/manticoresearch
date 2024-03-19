//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "searchdhttp.h"

void BuddyStart ( const CSphString & sConfigPath, const CSphString & sPluginDir, bool bHasBuddyPath, const VecTraits_T<ListenerDesc_t> & dListeners, bool bTelemetry, int iThreads, const CSphString & sConfigFilePath );
void BuddyShutdown ();

bool HasBuddy();

bool ProcessHttpQueryBuddy ( HttpProcessResult_t & tRes, Str_t sSrcQuery, OptionsHash_t& hOptions, CSphVector<BYTE>& dResult, bool bNeedHttpResponse, http_method eRequestType );
void ProcessSqlQueryBuddy ( Str_t sSrcQuery, Str_t sError, std::pair<int, BYTE> tSavedPos, BYTE& uPackedID, GenericOutputBuffer_c& tOut );
