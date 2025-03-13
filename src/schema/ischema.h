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

#pragma once

#include "columninfo.h"
class CSphRsetSchema;

/// barebones schema interface
/// everything that is needed from every implementation of a schema
class ISphSchema
{
public:
	/// dtor
	virtual ~ISphSchema () = default;

	/// assign current schema to rset schema (kind of a visitor operator)
	virtual void					AssignTo ( class CSphRsetSchema & lhs ) const = 0;

	/// get schema name
	virtual const char *			GetName() const = 0;

	/// add an attribute
	virtual void					AddAttr ( const CSphColumnInfo & tAttr, bool bDynamic ) = 0;

	/// get row size (static+dynamic combined)
	virtual int						GetRowSize() const = 0;

	/// get static row part size
	virtual int						GetStaticSize() const = 0;

	/// get dynamic row part size
	virtual int						GetDynamicSize() const = 0;

	virtual int						GetAttrsCount() const = 0;
	virtual int						GetFieldsCount() const = 0;

	virtual const CSphColumnInfo &	GetField ( int iIndex ) const = 0;
	virtual const CSphColumnInfo *	GetField ( const char * szName ) const  = 0;
	virtual const CSphVector<CSphColumnInfo> & GetFields () const = 0;

	/// get attribute index by name, returns -1 if not found
	virtual int						GetAttrIndex ( const char * sName ) const = 0;

	/// get field index by name, returns -1 if not found
	virtual int						GetFieldIndex ( const char * sName ) const = 0;

	/// get attr by index
	virtual const CSphColumnInfo &	GetAttr ( int iIndex ) const = 0;

	/// get attr by name
	virtual const CSphColumnInfo *	GetAttr ( const char * sName ) const = 0;

	/// get the first one of field length attributes. return -1 if none exist
	virtual int						GetAttrId_FirstFieldLen() const = 0;

	/// get the last one of field length attributes. return -1 if none exist
	virtual int						GetAttrId_LastFieldLen() const = 0;

	/// free all allocated attibures. Does NOT free m_pDynamic of match itself!
	virtual void					FreeDataPtrs ( CSphMatch & tMatch ) const = 0;

	/// return original attribute id (for compound schemas)
	virtual int						GetAttrIndexOriginal ( const char * szName ) const = 0;

	/// simple copy; clones either the entire dynamic part, or a part thereof
	virtual void					CloneMatch ( CSphMatch & tDst, const CSphMatch & rhs ) const = 0;

	virtual void					SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs ) = 0;

	virtual ISphSchema*				CloneMe() const = 0;
};
