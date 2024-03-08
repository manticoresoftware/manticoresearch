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

#include "searchdaemon.h"


class FrontendSchemaBuilder_c
{
public:
	FrontendSchemaBuilder_c ( const AggrResult_t & tRes, const CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, const CSphVector<CSphQueryItem> & dQueryItems,	const sph::StringSet & hExtraColumns, bool bQueryFromAPI, bool bHaveLocals );

	bool	Build ( bool bMaster, CSphString & sError );
	void	PopulateSchema ( CSphSchema & tSchema )  { tSchema.SwapAttrs(m_dFrontend); }

private:
	const AggrResult_t &				m_tRes;
	const CSphQuery &					m_tQuery;
	const CSphVector<CSphQueryItem> &	m_dItems;
	const CSphVector<CSphQueryItem> &	m_dQueryItems;
	const sph::StringSet &				m_hExtraColumns;
	bool								m_bQueryFromAPI;
	bool								m_bHaveLocals;
	bool								m_bAgent;

	CSphVector<CSphColumnInfo>			m_dFrontend;
	CSphVector<int>						m_dKnownAttrs;
	CSphVector<int>						m_dUnmappedAttrs;

	void	CollectKnownItems();
	void	AddAttrs();
	void	AddNullMask();

	bool	CheckUnmapped ( CSphString & sError ) const;
	void	Finalize();

	void	RemapGroupBy();
	void	RemapFacets();
};
