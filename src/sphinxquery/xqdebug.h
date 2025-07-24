//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#define XDEBUG_DEFAULT 0
#define XQDEBUG XDEBUG_DEFAULT
#define XQDEBUG_STAT XDEBUG_DEFAULT
#define XQ_DUMP_TRANSFORMED_TREE XDEBUG_DEFAULT
#define XQ_DUMP_NODE_ADDR XDEBUG_DEFAULT

struct XQNode_t;
void Dump ( const XQNode_t * pNode, const char * sHeader = "", bool bWithDot = false );

#if XQ_DUMP_TRANSFORMED_TREE
void xqDump ( const XQNode_t * pNode, int iIndent = 0 );
void DotDump ( const XQNode_t * pNode );
#endif

