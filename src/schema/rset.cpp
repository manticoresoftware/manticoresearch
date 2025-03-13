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

#include "rset.h"

inline int CSphRsetSchema::ActualLen() const
{
	if ( m_pIndexSchema )
		return m_pIndexSchema->GetAttrsCount() - m_dRemoved.GetLength();
	return 0;
}

void CSphRsetSchema::ResetRsetSchema()
{
	CSphSchemaHelper::ResetSchemaHelper();

	m_pIndexSchema = nullptr;
	m_dExtraAttrs.Reset();
}


void CSphRsetSchema::AddAttr ( const CSphColumnInfo& tCol, bool bDynamic )
{
	assert ( bDynamic );
	InsertAttr ( m_dExtraAttrs, m_dDynamicUsed, m_dExtraAttrs.GetLength(), tCol, true );
}


const char* CSphRsetSchema::GetName() const
{
	return m_pIndexSchema ? m_pIndexSchema->GetName() : nullptr;
}


int CSphRsetSchema::GetRowSize() const
{
	// we copy over dynamic map in case index schema has dynamic attributes
	// (that happens in case of inline attributes, or RAM segments in RT indexes)
	// so there is no need to add GetDynamicSize() here
	return GetDynamicSize() + ( m_pIndexSchema ? m_pIndexSchema->GetStaticSize() : 0 );
}


int CSphRsetSchema::GetStaticSize() const
{
	// result set schemas additions are always dynamic
	return m_pIndexSchema ? m_pIndexSchema->GetStaticSize() : 0;
}


int CSphRsetSchema::GetAttrsCount() const
{
	return m_dExtraAttrs.GetLength() + ActualLen();
}


int CSphRsetSchema::GetFieldsCount() const
{
	return m_pIndexSchema ? m_pIndexSchema->GetFieldsCount() : 0;
}


int CSphRsetSchema::GetAttrIndex ( const char* sName ) const
{
	ARRAY_FOREACH ( i, m_dExtraAttrs )
		if ( m_dExtraAttrs[i].m_sName == sName )
			return i + ActualLen();

	if ( !m_pIndexSchema )
		return -1;

	int iRes = m_pIndexSchema->GetAttrIndex ( sName );
	if ( iRes >= 0 )
	{
		if ( m_dRemoved.BinarySearch ( iRes ) )
			return -1;
		int iSub = 0;
		ARRAY_FOREACH_COND ( i, m_dRemoved, iRes >= m_dRemoved[i] )
			iSub++;
		return iRes - iSub;
	}
	return -1;
}


int	CSphRsetSchema::GetAttrIndexOriginal ( const char * szName ) const
{
	return m_pIndexSchema->GetAttrIndex ( szName );
}


int CSphRsetSchema::GetFieldIndex ( const char * szName ) const
{
	if ( !m_pIndexSchema )
		return -1;

	return m_pIndexSchema->GetFieldIndex ( szName );
}


const CSphColumnInfo & CSphRsetSchema::GetField ( int iIndex ) const
{
	assert ( m_pIndexSchema );
	return m_pIndexSchema->GetField ( iIndex );
}


const CSphColumnInfo * CSphRsetSchema::GetField ( const char * szName ) const
{
	assert ( m_pIndexSchema );
	return m_pIndexSchema->GetField(szName);
}


const CSphVector<CSphColumnInfo>& CSphRsetSchema::GetFields() const
{
	assert ( m_pIndexSchema );
	return m_pIndexSchema->GetFields();
}


const CSphColumnInfo& CSphRsetSchema::GetAttr ( int iIndex ) const
{
	if ( !m_pIndexSchema )
		return m_dExtraAttrs[iIndex];

	if ( iIndex < ActualLen() )
	{
		ARRAY_FOREACH_COND ( i, m_dRemoved, iIndex >= m_dRemoved[i] )
			++iIndex;

		return m_pIndexSchema->GetAttr ( iIndex );
	}

	return m_dExtraAttrs[iIndex - ActualLen()];
}


const CSphColumnInfo* CSphRsetSchema::GetAttr ( const char* sName ) const
{
	for ( auto& tExtraAttr : m_dExtraAttrs )
		if ( tExtraAttr.m_sName == sName )
			return &tExtraAttr;

	if ( m_pIndexSchema )
		return m_pIndexSchema->GetAttr ( sName );

	return nullptr;
}


int CSphRsetSchema::GetAttrId_FirstFieldLen() const
{
	// we assume that field_lens are in the base schema
	return m_pIndexSchema->GetAttrId_FirstFieldLen();
}


int CSphRsetSchema::GetAttrId_LastFieldLen() const
{
	// we assume that field_lens are in the base schema
	return m_pIndexSchema->GetAttrId_LastFieldLen();
}


CSphRsetSchema& CSphRsetSchema::operator= ( const ISphSchema& rhs )
{
	rhs.AssignTo ( *this );
	return *this;
}


CSphRsetSchema& CSphRsetSchema::operator= ( const CSphSchema& rhs )
{
	ResetRsetSchema();
	m_pIndexSchema = &rhs;

	// copy over dynamic rowitems map
	// so that the new attributes we might add would not overlap
	m_dDynamicUsed = rhs.m_dDynamicUsed;

	// copy data ptr map. we might want to add proper access via virtual funcs later
	m_dDataPtrAttrs = rhs.m_dDataPtrAttrs;

	return *this;
}

void CSphRsetSchema::Swap ( CSphRsetSchema& rhs ) noexcept
{
	CSphSchemaHelper::Swap ( rhs );
	::Swap ( m_pIndexSchema, rhs.m_pIndexSchema );
	::Swap ( m_dExtraAttrs, rhs.m_dExtraAttrs );
	::Swap ( m_dRemoved, rhs.m_dRemoved );
}

CSphRsetSchema::CSphRsetSchema ( const CSphRsetSchema& rhs )
	: CSphSchemaHelper ( rhs )
{
	m_pIndexSchema = rhs.m_pIndexSchema;
	m_dExtraAttrs = rhs.m_dExtraAttrs;
	m_dRemoved = rhs.m_dRemoved;
}

void CSphRsetSchema::RemoveStaticAttr ( int iAttr )
{
	assert ( m_pIndexSchema );
	assert ( iAttr >= 0 );
	assert ( iAttr < ActualLen() );

	// map from rset indexes (adjusted for removal) to index schema indexes (the original ones)
	ARRAY_FOREACH_COND ( i, m_dRemoved, iAttr >= m_dRemoved[i] )
		iAttr++;

	m_dRemoved.Add ( iAttr );
	m_dRemoved.Uniq();
}

bool CSphRsetSchema::IsRemovedAttr ( int iAttr ) const
{
	return ( m_dRemoved.BinarySearch ( iAttr ) );
}

void CSphRsetSchema::SwapAttrs ( CSphVector<CSphColumnInfo>& dAttrs )
{
#ifndef NDEBUG
	// ensure that every incoming column has a matching original column
	// only check locators and attribute types, because at this stage,
	// names that are used in dAttrs are already overwritten by the aliases
	// (example: SELECT col1 a, col2 b, count(*) c FROM test)
	//
	// FIXME? maybe also lockdown the schema from further swaps, adds etc from here?

	for ( auto& dAttr : dAttrs )
	{
		auto fComparer = [&dAttr] ( const CSphColumnInfo& dInfo ) {
			return dInfo.m_tLocator == dAttr.m_tLocator
				&& dInfo.m_eAttrType == dAttr.m_eAttrType;
		};
		bool bFound1 = m_pIndexSchema ? m_pIndexSchema->m_dAttrs.any_of ( fComparer ) : false;
		bool bFound2 = m_dExtraAttrs.any_of ( fComparer );
		assert ( bFound1 || bFound2 );
	}
#endif
	m_dExtraAttrs.SwapData ( dAttrs );
	m_pIndexSchema = nullptr;
}


ISphSchema* CSphRsetSchema::CloneMe() const
{
	auto pClone = new CSphRsetSchema;
	AssignTo ( *pClone );

	ARRAY_FOREACH ( i, m_dExtraAttrs )
		if ( m_dExtraAttrs[i].m_pExpr )
			pClone->m_dExtraAttrs[i].m_pExpr = SafeClone ( m_dExtraAttrs[i].m_pExpr );

	return pClone;
}
