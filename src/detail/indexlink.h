//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "std/string.h"

// if sBase differs from sNewPath - write .link file with sNewPath. Otherwise unlink .link file, if any.
bool WriteLinkFile ( const CSphString& sBase, const CSphString& sNewPath, CSphString& sError );

// if .link file exists in sBase - read it and return it's content. Otherwise return sBase unchanged.
CSphString RedirectToRealPath ( CSphString sBase );