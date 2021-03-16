//
// Copyright (c) 2020-2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarfilter_
#define _columnarfilter_

#include "sphinxfilter.h"

#if USE_COLUMNAR

class ColumnarFilterTraits_c
{
public:
			ColumnarFilterTraits_c() = default;
			ColumnarFilterTraits_c ( ISphExpr * pExpr );

	void	SetColumnarCol ( int iColumnarCol );

protected:
	int		m_iColumnarCol = -1;
};

ISphFilter * TryToCreateColumnarFilter ( int iAttr, const ISphSchema & tSchema, const CSphFilterSettings & tSettings, const CommonFilterSettings_t & tFixedSettings,
	ESphCollation eCollation, CSphString & sError, CSphString & sWarning );

#endif // USE_COLUMNAR

#endif // _columnarfilter_