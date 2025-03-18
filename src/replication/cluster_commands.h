//
// Copyright (c) 2023-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "searchdaemon.h"

void ReceiveClusterDelete ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster );
void ReceiveClusterFileReserve ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster );
void ReceiveClusterGetNodes ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster );
void ReceiveClusterIndexAddLocal ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster );
void ReceiveClusterSynced ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster );
void ReceiveClusterUpdateNodes ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster );
void ReceiveClusterGetState ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster );
void ReceiveClusterGetVer ( bool bServerid, ISphOutputBuffer & tOut );

// sending doesn't include cluster name
void ReceiveClusterFileSend ( ISphOutputBuffer& tOutb, InputBuffer_c& tBuf );

void ReceiveDistIndex ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster );