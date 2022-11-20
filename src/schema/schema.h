//
// Copyright (c) 2017-2022, Manticore Software LTD (http://manticoresearch.com)
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

#include "helper.h"

class CSphSourceSettings;

/// plain good old schema
/// container that actually holds and owns all the fields, columns, etc
///
/// NOTE that while this one can be used everywhere where we need a schema
/// it might be huge (say 1000+ attributes) and expensive to copy, modify, etc!
/// so for most of the online query work, consider CSphRsetSchema
class CSphSchema final : public CSphSchemaHelper
{
	friend class CSphRsetSchema;

public:
	/// ctor
	explicit				CSphSchema ( CSphString sName="(nameless)" );
							CSphSchema ( const CSphSchema & rhs );
							CSphSchema ( CSphSchema && rhs ) noexcept;
	void					Swap ( CSphSchema & rhs ) noexcept;

	CSphSchema &			operator = ( const ISphSchema & rhs );
	CSphSchema &			operator = ( CSphSchema rhs );

	/// visitor-style uber-virtual assignment implementation
	void					AssignTo ( CSphRsetSchema & lhs ) const final;
	const char *			GetName() const final;

	void					AddAttr ( const CSphColumnInfo & tAttr, bool bDynamic ) final;

	void					AddField ( const char * sFieldName );
	void					AddField ( const CSphColumnInfo & tField );

	/// remove field
	void					RemoveField ( const char * szFieldName );
	void					RemoveField ( int iIdx );

	/// get field index by name
	/// returns -1 if not found
	int						GetFieldIndex ( const char * sName ) const final;

	/// get attribute index by name
	/// returns -1 if not found
	int						GetAttrIndex ( const char * sName ) const final;

	/// checks if two schemas fully match (ie. fields names, attr names, types and locators are the same)
	/// describe mismatch (if any) to sError
	bool					CompareTo ( const CSphSchema & rhs, CSphString & sError, bool bFullComparison = true, bool bIndexLoadCheck=false ) const;

	/// reset fields and attrs
	void					Reset ();

	/// get row size (static+dynamic combined)
	int						GetRowSize () const final		{ return GetStaticSize () + GetDynamicSize(); }

	/// get static row part size
	int						GetStaticSize () const final	{ return m_dStaticUsed.GetLength(); }

	int						GetAttrsCount () const final	{ return m_dAttrs.GetLength(); }
	int						GetFieldsCount() const final			{ return m_dFields.GetLength(); }

	const CSphColumnInfo &	GetField ( int iIndex ) const final { return m_dFields[iIndex]; }
	const CSphColumnInfo *	GetField ( const char * szName ) const final;
	const CSphVector<CSphColumnInfo> & GetFields () const final { return m_dFields; }

	// most of the time we only need to get the field name
	const char *			GetFieldName ( int iIndex ) const { return m_dFields[iIndex].m_sName.cstr(); }

	/// get attr by index
	const CSphColumnInfo &	GetAttr ( int iIndex ) const final	{ return m_dAttrs[iIndex]; }

	/// get attr by name
	const CSphColumnInfo *	GetAttr ( const char * sName ) const final;

	/// insert attr
	void					InsertAttr ( int iPos, const CSphColumnInfo & tAggr, bool bDynamic );

	/// remove attr
	void					RemoveAttr ( const char * szAttr, bool bDynamic );

	/// get the first one of field length attributes. return -1 if none exist
	int						GetAttrId_FirstFieldLen() const final;

	/// get the last one of field length attributes. return -1 if none exist
	int						GetAttrId_LastFieldLen() const final;


	static bool				IsReserved ( const char * szToken );

	/// full copy, both static & dynamic became pure dynamic
	void					CloneWholeMatch ( CSphMatch & tDst, const CSphMatch & rhs ) const;

	/// update wordpart settings for a field
	void					SetFieldWordpart ( int iField, ESphWordpart eWordpart );

	void					SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs ) final;

	ISphSchema * 			CloneMe () const final;

	bool					HasBlobAttrs() const;
	int						GetCachedRowSize() const;
	void					SetupFlags ( const CSphSourceSettings & tSettings, bool bPQ, StrVec_t * pWarnings );
	bool					HasStoredFields() const;
	bool					HasStoredAttrs() const;
	bool					HasColumnarAttrs() const;
	bool					HasNonColumnarAttrs() const;
	bool					IsFieldStored ( int iField ) const;
	bool					IsAttrStored ( int iAttr ) const;

private:
	static const int			HASH_THRESH		= 32;
	static const int			BUCKET_COUNT	= 256;

	WORD						m_dBuckets [ BUCKET_COUNT ];	///< uses indexes in m_dAttrs as ptrs; 0xffff is like NULL in this hash

	CSphString					m_sName;		///< my human-readable name

	int							m_iFirstFieldLenAttr = -1;///< position of the first field length attribute (cached on insert/delete)
	int							m_iLastFieldLenAttr = -1; ///< position of the last field length attribute (cached on insert/delete)

	int							m_iRowSize = 0;				///< cached row size

	CSphVector<CSphColumnInfo>	m_dFields;		///< my fulltext-searchable fields
	CSphVector<CSphColumnInfo>	m_dAttrs;		///< all my attributes
	CSphVector<int>				m_dStaticUsed;	///< static row part map (amount of used bits in each rowitem)


	/// returns 0xffff if bucket list is empty and position otherwise
	WORD &					GetBucketPos ( const char * sName );

	/// reset hash and re-add all attributes
	void					RebuildHash ();

	/// rebuild the attribute value array
	void					RebuildLocators();

	/// add iAddVal to all indexes strictly greater than iStartIdx in hash structures
	void					UpdateHash ( int iStartIdx, int iAddVal );

	void					SetupColumnarFlags ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings );
};
