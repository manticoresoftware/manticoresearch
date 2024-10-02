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

#ifndef _icu_
#define _icu_

#include "sphinxstd.h"
#include "sphinxutils.h"
#include "indexsettings.h"

bool sphCheckConfigICU ( CSphIndexSettings & tSettings, CSphString & sError );
bool sphSpawnFilterICU ( std::unique_ptr<ISphFieldFilter> & pFieldFilter, const CSphIndexSettings & m_tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, CSphString & sError );

#endif // _icu_
