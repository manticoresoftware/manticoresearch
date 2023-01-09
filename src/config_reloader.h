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

#include "searchdaemon.h"

class ConfigReloader_c : public ISphNoncopyable
{
	class Impl_c;
	Impl_c* m_pImpl;

public:
	ConfigReloader_c ( HashOfServed_c& hDeferred );
	~ConfigReloader_c();

	void LoadIndexFromConfig ( const CSphString& sIndex, IndexType_e eType, const CSphConfigSection& hIndex );
	void IssuePlainOldRotation();
};