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

#include "schema.h"


/// lightweight schema to be used in sorters, result sets, etc
/// avoids copying of static attributes part by keeping a pointer
/// manages the additional dynamic attributes on its own
///
/// NOTE that for that reason CSphRsetSchema needs the originating index to exist
/// (in case it keeps and uses a pointer to original schema in that index)
class CSphRsetSchema: public CSphSchemaHelper
{
public:
					CSphRsetSchema() = default;
					CSphRsetSchema ( const CSphRsetSchema& rhs );
					CSphRsetSchema ( CSphRsetSchema&& rhs ) noexcept { Swap (rhs); }


	CSphRsetSchema & operator = ( const ISphSchema& rhs );
	CSphRsetSchema & operator = ( const CSphSchema& rhs );

	CSphRsetSchema& operator= ( CSphRsetSchema rhs ) noexcept
	{
		Swap ( rhs );
		return *this;
	}

	void			AddAttr ( const CSphColumnInfo& tCol, bool bDynamic ) final;
	void			AssignTo ( CSphRsetSchema& lhs ) const final { lhs = *this; }
	const char *	GetName() const final;

	void			Swap ( CSphRsetSchema& rhs ) noexcept;

public:
	int		GetRowSize() const final;
	int		GetStaticSize() const final;
	int		GetAttrsCount() const final;
	int		GetFieldsCount() const final;
	int		GetAttrIndex ( const char* sName ) const final;
	int		GetAttrIndexOriginal ( const char * szName ) const final;
	int		GetFieldIndex ( const char* sName ) const final;
	const CSphColumnInfo &	GetField ( int iIndex ) const final;
	const CSphColumnInfo *	GetField ( const char * szName ) const final;
	const CSphVector<CSphColumnInfo> & GetFields() const final;
	const CSphColumnInfo &	GetAttr ( int iIndex ) const final;
	const CSphColumnInfo *	GetAttr ( const char * szName ) const final;

	int		GetAttrId_FirstFieldLen() const final;
	int		GetAttrId_LastFieldLen() const final;

public:
	void	RemoveStaticAttr ( int iAttr );
	void	ResetRsetSchema();

public:
	/// swap in a subset of current attributes, with not necessarily (!) unique names
	/// used to create a network result set (ie. rset to be sent and then discarded)
	/// WARNING, DO NOT USE THIS UNLESS ABSOLUTELY SURE!
	void	SwapAttrs ( CSphVector<CSphColumnInfo>& dAttrs ) final;
	ISphSchema * CloneMe() const final;

protected:
	const CSphSchema *			m_pIndexSchema = nullptr;	///< original index schema, for the static part
	CSphVector<CSphColumnInfo>	m_dExtraAttrs;				///< additional dynamic attributes, for the dynamic one
	CSphVector<int>				m_dRemoved;					///< original indexes that are suppressed from the index schema by RemoveStaticAttr()

private:
	int		ActualLen() const; ///< len of m_pIndexSchema accounting removed stuff
};
