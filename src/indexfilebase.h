//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "std/string.h"

#include <utility>

enum ESphExt : BYTE;

class IndexFileBase_c
{
	CSphString m_sFileBase; // prefix (i.e. folder + index name, excluding extensions)

public:
	IndexFileBase_c() = default;
	explicit IndexFileBase_c ( CSphString sFileBase )
		: m_sFileBase { std::move ( sFileBase ) }
	{}

	/// set new index base path
	void SetFilebase ( CSphString sNewBase ) { m_sFileBase = std::move ( sNewBase ); }

	/// get for the base file name
	const char* GetFilebase() const { return m_sFileBase.cstr(); }

	/// build filename
	CSphString GetFilename ( const char* szExt ) const;
	CSphString GetFilename ( int iSuffix ) const;
	CSphString GetFilename ( ESphExt eExt ) const;
	CSphString GetTmpFilename ( ESphExt eExt ) const;
};
