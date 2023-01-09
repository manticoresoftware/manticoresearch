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

#include "attrindex_builder.h"

#include <cfloat>
#include <climits>

#include "conversion.h"
#include "sphinxint.h"

// dirty hack for some build systems which not has LLONG_MAX
#ifndef LLONG_MAX
#define LLONG_MAX (((unsigned long long)(-1))>>1)
#endif

#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#endif


AttrIndexBuilder_c::AttrIndexBuilder_c ( const CSphSchema & tSchema )
{
	Init ( tSchema );
}

void AttrIndexBuilder_c::Init ( const CSphSchema& tSchema )
{
	m_uStride = tSchema.GetRowSize();
	for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo& tCol = tSchema.GetAttr ( i );
		if ( tCol.IsColumnar() )
			continue;

		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_BOOL:
		case SPH_ATTR_BIGINT:
		case SPH_ATTR_TOKENCOUNT:
			m_dIntAttrs.Add ( tCol.m_tLocator );
			break;

		case SPH_ATTR_FLOAT:
			m_dFloatAttrs.Add ( tCol.m_tLocator );
			break;

		default:
			break;
		}
	}

	m_dIntMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntMax.Resize ( m_dIntAttrs.GetLength() );
	m_dFloatMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatMax.Resize ( m_dFloatAttrs.GetLength() );

	m_dIntIndexMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntIndexMax.Resize ( m_dIntAttrs.GetLength() );
	m_dFloatIndexMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatIndexMax.Resize ( m_dFloatAttrs.GetLength() );

	m_dIntIndexMin.Fill ( LLONG_MAX );
	m_dIntIndexMax.Fill ( LLONG_MIN );
	m_dFloatIndexMin.Fill ( FLT_MAX );
	m_dFloatIndexMax.Fill ( FLT_MIN );

	ResetLocal();
}


void AttrIndexBuilder_c::Collect ( const CSphRowitem * pRow )
{
	// check if it is time to flush already collected values
	if ( m_nLocalCollected>=DOCINFO_INDEX_FREQ )
		FlushComputed();

	m_nLocalCollected++;

	// ints
	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		SphAttr_t tVal = sphGetRowAttr ( pRow, m_dIntAttrs[i] );
		m_dIntMin[i] = Min ( m_dIntMin[i], tVal );
		m_dIntMax[i] = Max ( m_dIntMax[i], tVal );
	}

	// floats
	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pRow, m_dFloatAttrs[i] ) );
		m_dFloatMin[i] = Min ( m_dFloatMin[i], fVal );
		m_dFloatMax[i] = Max ( m_dFloatMax[i], fVal );
	}
}


void AttrIndexBuilder_c::FinishCollect()
{
	if ( m_nLocalCollected )
		FlushComputed();

	CSphRowitem * pMinAttrs = m_dMinMaxRows.AddN ( m_uStride*2 );
	CSphRowitem * pMaxAttrs = pMinAttrs+m_uStride;
	memset ( pMinAttrs, 0, sizeof(CSphRowitem) * m_uStride * 2 );

	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntIndexMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntIndexMax[i] );
	}

	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMin[i] ) );
		sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMax[i] ) );
	}
}


const CSphTightVector<CSphRowitem> & AttrIndexBuilder_c::GetCollected() const
{
	return m_dMinMaxRows;
}


void AttrIndexBuilder_c::ResetLocal()
{
	for ( auto & i : m_dIntMin )
		i = LLONG_MAX;

	for ( auto & i : m_dIntMax )
		i = LLONG_MIN;

	for ( auto & i : m_dFloatMin )
		i = FLT_MAX;

	for ( auto & i : m_dFloatMax )
		i = FLT_MIN;

	m_nLocalCollected = 0;
}


void AttrIndexBuilder_c::FlushComputed ()
{
	CSphRowitem * pMinAttrs = m_dMinMaxRows.AddN ( m_uStride*2 );
	CSphRowitem * pMaxAttrs = pMinAttrs+m_uStride;
	memset ( pMinAttrs, 0, sizeof ( CSphRowitem ) * m_uStride * 2 );

	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		m_dIntIndexMin[i] = Min ( m_dIntIndexMin[i], m_dIntMin[i] );
		m_dIntIndexMax[i] = Max ( m_dIntIndexMax[i], m_dIntMax[i] );
		sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntMax[i] );
	}

	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		m_dFloatIndexMin[i] = Min ( m_dFloatIndexMin[i], m_dFloatMin[i] );
		m_dFloatIndexMax[i] = Max ( m_dFloatIndexMax[i], m_dFloatMax[i] );
		sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMin[i] ) );
		sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMax[i] ) );
	}

	ResetLocal();
}
