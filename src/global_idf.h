//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
#ifndef MANTICORE_GLOBAL_IDF_H
#define MANTICORE_GLOBAL_IDF_H

#include "sphinxstd.h"

namespace sph {

	class IDFer_c : public ISphRefcountedMT
	{
	protected:
		~IDFer_c () override  = default;
	public:
		virtual float GetIDF ( const CSphString& sWord, int64_t iDocsLocal, bool bPlainIDF ) const = 0;
	};

	using IDFerRefPtr_c = CSphRefcountedPtr<IDFer_c>;

	IDFerRefPtr_c GetIDFer ( const CSphString& IDFPath );
	/// update/clear global IDF cache
	bool PrereadGlobalIDF ( const CSphString& sPath, CSphString& sError );
	void UpdateGlobalIDFs ( const StrVec_t& dFiles );
	void ShutdownGlobalIDFs ();

}

#endif //MANTICORE_GLOBAL_IDF_H
