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

#include "frontendschema.h"

#include "queuecreator.h"

/// returns internal magic names for expressions like COUNT(*) that have a corresponding one
/// returns expression itself otherwise
static const char * GetMagicSchemaName ( const CSphString & s )
{
	if ( s=="count(*)" )
		return "@count";
	if ( s=="weight()" )
		return "@weight";
	if ( s=="groupby()" )
		return "@groupby";
	return s.cstr();
}

/// a functor to sort columns by (is_aggregate ASC, column_index ASC)
struct AggregateColumnSort_fn
{
	bool IsAggr ( const CSphColumnInfo & c ) const
	{
		return c.m_eAggrFunc!=SPH_AGGR_NONE
			|| c.m_sName=="@groupby"
			|| c.m_sName=="@count"
			|| c.m_sName=="@distinct"
			|| IsSortJsonInternal ( c.m_sName );
	}

	bool IsLess ( const CSphColumnInfo & a, const CSphColumnInfo & b ) const
	{
		bool aa = IsAggr(a);
		bool bb = IsAggr(b);
		if ( aa!=bb )
			return aa < bb;
		return a.m_iIndex < b.m_iIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////

FrontendSchemaBuilder_c::FrontendSchemaBuilder_c ( const AggrResult_t & tRes, const CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, const CSphVector<CSphQueryItem> & dQueryItems, const sph::StringSet & hExtraColumns, bool bQueryFromAPI, bool bHaveLocals )
	: m_tRes ( tRes )
	, m_tQuery ( tQuery )
	, m_dItems ( dItems )
	, m_dQueryItems ( dQueryItems )
	, m_hExtraColumns ( hExtraColumns )
	, m_bQueryFromAPI ( bQueryFromAPI )
	, m_bHaveLocals ( bHaveLocals )
	, m_bAgent ( tQuery.m_bAgent )
{
	m_dFrontend.Resize(dItems.GetLength());
}


bool FrontendSchemaBuilder_c::Build ( bool bMaster, CSphString & sError )
{
	CollectKnownItems();
	AddAttrs();
	AddNullMask();
	if ( !CheckUnmapped(sError) )
		return false;

	Finalize();

	RemapGroupBy();
	// agent should provide raw attributes into master without any remapping
	if ( bMaster )
		RemapFacets();

	return true;
}


void FrontendSchemaBuilder_c::CollectKnownItems()
{
	ARRAY_CONSTFOREACH ( i, m_dItems )
	{
		const CSphQueryItem & tItem = m_dItems[i];

		int iCol = -1;
		if ( !m_bQueryFromAPI && tItem.m_sAlias.IsEmpty() )
			iCol = m_tRes.m_tSchema.GetAttrIndex ( tItem.m_sExpr.cstr() );

		if ( iCol>=0 )
		{
			m_dKnownAttrs.Add(i);
			m_dFrontend[i].m_sName = tItem.m_sExpr;
			m_dFrontend[i].m_iIndex = iCol;
		}
		else
			m_dUnmappedAttrs.Add(i);
	}
}


void FrontendSchemaBuilder_c::AddAttrs()
{
	bool bUsualApi = !m_bAgent && m_bQueryFromAPI;

	for ( int iCol=0; iCol<m_tRes.m_tSchema.GetAttrsCount(); ++iCol )
	{
		const CSphColumnInfo & tCol = m_tRes.m_tSchema.GetAttr(iCol);

		assert ( !tCol.m_sName.IsEmpty() );
		bool bMagic = IsGroupbyMagic ( tCol.m_sName ) || IsSortStringInternal ( tCol.m_sName );

		if ( !bMagic && tCol.m_pExpr )
		{
			ARRAY_FOREACH ( j, m_dUnmappedAttrs )
				if ( m_dItems[ m_dUnmappedAttrs[j] ].m_sAlias==tCol.m_sName )
				{
					int k = m_dUnmappedAttrs[j];
					m_dFrontend[k].m_iIndex = iCol;
					m_dFrontend[k].m_sName = m_dItems[k].m_sAlias;
					m_dKnownAttrs.Add(k);
					m_dUnmappedAttrs.Remove ( j-- ); // do not skip an element next to removed one!
				}

			// FIXME?
			// really not sure if this is the right thing to do
			// but it fixes a couple queries in test_163 in compaitbility mode
			if ( m_bAgent && !m_dFrontend.Contains ( bind ( &CSphColumnInfo::m_sName ), tCol.m_sName ) )
			{
				CSphColumnInfo & t = m_dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		} else if ( bMagic && ( tCol.m_pExpr || bUsualApi ) )
		{
			ARRAY_FOREACH ( j, m_dUnmappedAttrs )
				if ( tCol.m_sName==GetMagicSchemaName ( m_dItems[ m_dUnmappedAttrs[j] ].m_sExpr ) )
				{
					int k = m_dUnmappedAttrs[j];
					m_dFrontend[k].m_iIndex = iCol;
					m_dFrontend[k].m_sName = m_dItems[k].m_sAlias;
					m_dKnownAttrs.Add(k);
					m_dUnmappedAttrs.Remove ( j-- ); // do not skip an element next to removed one!
				}

			if ( !m_dFrontend.Contains ( bind ( &CSphColumnInfo::m_sName ), tCol.m_sName ) )
			{
				CSphColumnInfo & t = m_dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		} else
		{
			bool bAdded = false;
			ARRAY_FOREACH ( j, m_dUnmappedAttrs )
			{
				int k = m_dUnmappedAttrs[j];
				const CSphQueryItem & t = m_dItems[k];
				if ( ( tCol.m_sName==GetMagicSchemaName ( t.m_sExpr ) && t.m_eAggrFunc==SPH_AGGR_NONE )
					|| ( t.m_sAlias==tCol.m_sName &&
						( m_tRes.m_tSchema.GetAttrIndex ( GetMagicSchemaName ( t.m_sExpr ) )==-1 || t.m_eAggrFunc!=SPH_AGGR_NONE ) ) )
				{
					// tricky bit about naming
					//
					// in master mode, we can just use the alias or expression or whatever
					// the data will be fetched using the locator anyway, column name does not matter anymore
					//
					// in agent mode, however, we need to keep the original column names in our response
					// otherwise, queries like SELECT col1 c, count(*) c FROM dist will fail on master
					// because it won't be able to identify the count(*) aggregate by its name
					m_dFrontend[k].m_iIndex = iCol;
					m_dFrontend[k].m_sName = m_bAgent
						? tCol.m_sName
						: ( m_dItems[k].m_sAlias.IsEmpty()
							? m_dItems[k].m_sExpr
							: m_dItems[k].m_sAlias );
					m_dKnownAttrs.Add(k);
					bAdded = true;
					m_dUnmappedAttrs.Remove ( j-- ); // do not skip an element next to removed one!
				}
			}

			// column was not found in the select list directly
			// however we might need it anyway because of a non-NULL extra-schema
			// (extra-schema is additional set of columns came from right side of query
			// when you perform 'select a from index order by b', the 'b' is not displayed, but need for sorting,
			// so extra-schema in the case will contain 'b').
			// bMagic condition added for @groupbystr in the agent mode
			if ( !bAdded && m_bAgent && ( m_hExtraColumns[tCol.m_sName] || !m_bHaveLocals || bMagic ) )
			{
				CSphColumnInfo & t = m_dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		}
	}

	m_dKnownAttrs.Sort();
}


void FrontendSchemaBuilder_c::AddNullMask()
{
	int iNullMask = m_tRes.m_tSchema.GetAttrIndex ( GetNullMaskAttrName() );
	if ( iNullMask < 0 )
		return;

	for ( auto & i : m_dFrontend )
		if ( i.m_sName==GetNullMaskAttrName() )
			return;

	CSphColumnInfo & tAttr = m_dFrontend.Add();
	tAttr.m_iIndex = iNullMask;
	tAttr.m_sName = GetNullMaskAttrName();
}


bool FrontendSchemaBuilder_c::CheckUnmapped ( CSphString & sError ) const
{
	// sanity check
	// verify that we actually have all the queried select items
	assert ( m_dUnmappedAttrs.IsEmpty() || ( m_dUnmappedAttrs.GetLength()==1 && m_dItems [ m_dUnmappedAttrs[0] ].m_sExpr=="id" ) );
	ARRAY_CONSTFOREACH ( i, m_dItems )
	{
		const CSphQueryItem & tItem = m_dItems[i];
		if ( !m_dKnownAttrs.BinarySearch(i) && tItem.m_sExpr!="id" )
		{
			sError.SetSprintf ( "internal error: column '%s/%s' not found in result set schema", tItem.m_sExpr.cstr(), tItem.m_sAlias.cstr() );
			return false;
		}
	}

	return true;
}


void FrontendSchemaBuilder_c::Finalize()
{
	// finalize the frontend schema columns
	// we kept indexes into internal schema there, now use them to lookup and copy column data
	ARRAY_CONSTFOREACH ( i, m_dFrontend )
	{
		CSphColumnInfo & tFrontend = m_dFrontend[i];
		const CSphColumnInfo & s = m_tRes.m_tSchema.GetAttr ( tFrontend.m_iIndex );
		tFrontend.m_tLocator = s.m_tLocator;
		tFrontend.m_eAttrType = s.m_eAttrType;
		tFrontend.m_eAggrFunc = s.m_eAggrFunc; // for a sort loop just below
		tFrontend.m_iIndex = i; // to make the aggr sort loop just below stable

		tFrontend.m_uFieldFlags = s.m_uFieldFlags;
	}

	// tricky bit
	// in agents only, push aggregated columns, if any, to the end
	// for that, sort the schema by (is_aggregate ASC, column_index ASC)
	if ( m_bAgent )
		m_dFrontend.Sort ( AggregateColumnSort_fn() );
}


void FrontendSchemaBuilder_c::RemapGroupBy()
{
	// remap groupby() and aliased groupby() to @groupbystr or string attribute
	const CSphColumnInfo * p = nullptr;
	CSphString sJsonGroupBy;
	if ( sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr() ) )
	{
		sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
		p = m_tRes.m_tSchema.GetAttr ( sJsonGroupBy.cstr() );
	}

	if ( !p )
	{
		// try string attribute (multiple group-by still displays hashes)
		if ( !m_tQuery.m_sGroupBy.IsEmpty() )
		{
			p = m_tRes.m_tSchema.GetAttr ( m_tQuery.m_sGroupBy.cstr() );
			if ( p )
			{
				if ( p->m_eAttrType==SPH_ATTR_JSON_PTR )
				{
					sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
					p = m_tRes.m_tSchema.GetAttr ( sJsonGroupBy.cstr() );
				} else if ( p->m_eAttrType!=SPH_ATTR_STRINGPTR )
				{
					p = nullptr;
				}
			}
		}

		if ( !p )
			return;
	}

	for ( auto & tFrontend : m_dFrontend )
		if ( tFrontend.m_sName=="groupby()" )
		{
			tFrontend.m_tLocator = p->m_tLocator;
			tFrontend.m_eAttrType = p->m_eAttrType;
			tFrontend.m_eAggrFunc = p->m_eAggrFunc;
		}

	// check aliases too
	for ( const auto & tQueryItem : m_dQueryItems )
	{
		if ( tQueryItem.m_sExpr!="groupby()" )
			continue;

		for ( auto & tFrontend : m_dFrontend )
			if ( tFrontend.m_sName==tQueryItem.m_sAlias )
			{
				tFrontend.m_tLocator = p->m_tLocator;
				tFrontend.m_eAttrType = p->m_eAttrType;
				tFrontend.m_eAggrFunc = p->m_eAggrFunc;
			}
	}
}


void FrontendSchemaBuilder_c::RemapFacets()
{
	// facets
	if ( !m_tQuery.m_bFacet && !m_tQuery.m_bFacetHead )
		return;

	// remap MVA/JSON column to @groupby/@groupbystr in facet queries
	const CSphColumnInfo * pGroupByCol = nullptr;
	CSphString sJsonGroupBy;
	if ( sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr() ) )
	{
		sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
		pGroupByCol = m_tRes.m_tSchema.GetAttr ( sJsonGroupBy.cstr() );
	}

	if ( !pGroupByCol )
	{
		pGroupByCol = m_tRes.m_tSchema.GetAttr ( "@groupby" );
		if ( !pGroupByCol )
			return;
	}

	if ( m_tQuery.m_sGroupBy.IsEmpty() )
		return;

	for ( auto & tFrontend : m_dFrontend )
	{
		ESphAttr eAttr = tFrontend.m_eAttrType;
		// checking _PTR attrs only because we should not have and non-ptr attr at this point
		if ( m_tQuery.m_sGroupBy==tFrontend.m_sName && ( eAttr==SPH_ATTR_UINT32SET_PTR || eAttr==SPH_ATTR_INT64SET_PTR || eAttr==SPH_ATTR_FLOAT_VECTOR_PTR || eAttr==SPH_ATTR_JSON_FIELD_PTR ) )
		{
			tFrontend.m_tLocator = pGroupByCol->m_tLocator;
			tFrontend.m_eAttrType = pGroupByCol->m_eAttrType;
			tFrontend.m_eAggrFunc = pGroupByCol->m_eAggrFunc;
		}
	}
}
