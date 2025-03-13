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

template<typename NUM>
int64_t PercentOf ( NUM tVal, NUM tBase, int iFloatDigits )
{
	NUM tMultiplier = 100;
	switch ( iFloatDigits )
	{
	case 6:
		tMultiplier *= 10;
	case 5:
		tMultiplier *= 10;
	case 4:
		tMultiplier *= 10;
	case 3:
		tMultiplier *= 10;
	case 2:
		tMultiplier *= 10;
	case 1:
		tMultiplier *= 10;
	default: break;
	}
	if ( tBase )
		return int64_t ( tVal * tMultiplier / tBase );
	else
		return int64_t ( tMultiplier );
}