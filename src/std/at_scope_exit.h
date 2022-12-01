//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "generics.h"
#include <utility>

/// perform any (function-defined) action on exit from a scope.
template<typename ACTION>
class AtScopeExit_T
{
	ACTION m_dAction;

public:
	explicit AtScopeExit_T ( ACTION&& tAction )
		: m_dAction { std::forward<ACTION> ( tAction ) }
	{}

	AtScopeExit_T ( AtScopeExit_T&& rhs ) noexcept
		: m_dAction { std::move ( rhs.m_dAction ) }
	{}

	~AtScopeExit_T()
	{
		m_dAction();
	}
};

// create action to be performed on-exit-from-scope.
// usage example:
// someObject * pObj; // need to be freed going out of scope
// auto dObjDeleter = AtScopeExit ( [&pObj] { SafeDelete (pObj); } )
// ...
template<typename ACTION>
AtScopeExit_T<ACTION> AtScopeExit ( ACTION&& action )
{
	return AtScopeExit_T<ACTION> { std::forward<ACTION> ( action ) };
}

#define AT_SCOPE_EXIT( ... ) auto SPH_UID ( tAtExit ) = AtScopeExit ( __VA_ARGS__ )
#define AT_SCOPE_FN( action, exit_action ) action; AT_SCOPE_EXIT ( exit_action )
#define AT_SCOPE( action, exit_action ) AT_SCOPE_FN ( action, [&] { exit_action; } )
#define SCOPED_CHANGE( name, on_enter, on_exit ) AT_SCOPE_FN ( name on_enter, [&name] { name on_exit; } )