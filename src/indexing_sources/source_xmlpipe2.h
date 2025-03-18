//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "source_document.h"
#include "sphinxutils.h"

/// XMLPipe2 source fabric
CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, const char * szSourceName, int iMaxFieldLen, CSphString & sError );
