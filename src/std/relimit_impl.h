//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <cassert>

//////////////////////////////////////////////////////////////////////////
/// Resize backends for vector
/// Each backend provides Relimit

namespace sph
{

/// Default relimit: grow 2x
int64_t DefaultRelimit::Relimit ( int64_t iLimit, int64_t iNewLimit )
{
	if ( !iLimit )
		iLimit = MAGIC_INITIAL_LIMIT;
	while ( iLimit < iNewLimit )
	{
		iLimit *= 2;
		assert ( iLimit > 0 );
	}
	return iLimit;
}


/// tight-vector policy
/// grow only 1.2x on resize (not 2x) starting from a certain threshold
int64_t TightRelimit::Relimit ( int64_t iLimit, int64_t iNewLimit )
{
	if ( !iLimit )
		iLimit = MAGIC_INITIAL_LIMIT;
	while ( iLimit < iNewLimit && iLimit < SLOW_GROW_TRESHOLD )
	{
		iLimit *= 2;
		assert ( iLimit > 0 );
	}
	while ( iLimit < iNewLimit )
	{
		iLimit = (int)( (float)iLimit * GROW );
		assert ( iLimit > 0 );
	}
	return iLimit;
}

} // namespace sph