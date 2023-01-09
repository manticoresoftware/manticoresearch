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

#ifndef _sphinx_alter_
#define _sphinx_alter_

#include "sphinxexpr.h"
#include "columnarlib.h"
#include "indexsettings.h"

class WriteWrapper_c
{
public:
	virtual ~WriteWrapper_c() = default;

	virtual void		PutBytes ( const BYTE * pData, int iSize ) = 0;
	virtual SphOffset_t	GetPos() const = 0;
	virtual bool		IsError() const = 0;
};

class CSphWriter;
std::unique_ptr<WriteWrapper_c> CreateWriteWrapperDisk ( CSphWriter & tWriter );
std::unique_ptr<WriteWrapper_c> CreateWriteWrapperMem ( CSphTightVector<CSphRowitem> & dSPA );
std::unique_ptr<WriteWrapper_c> CreateWriteWrapperMem ( CSphTightVector<BYTE> & dSPB );

class CSphSchema;
class Docstore_i;
class DocstoreBuilder_i;

struct AttrAddRemoveCtx_t
{
	CSphString		m_sName;
	ESphAttr		m_eType;
	int				m_iBits;
	DWORD			m_uFlags;
	AttrEngine_e	m_eEngine;
};

// common add/remove attribute/field code for both RT and plain indexes
class IndexAlterHelper_c
{
public:
	virtual			~IndexAlterHelper_c() = default;
	virtual bool	Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const { return false; }

protected:
	bool			Alter_AddRemoveRowwiseAttr ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo, DWORD uNumRows, const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter, WriteWrapper_c & tSPBWriter, bool bAddAttr, const CSphString & sAttrName );
	bool 			Alter_AddRemoveFromSchema ( CSphSchema & tSchema, const AttrAddRemoveCtx_t & tCtx, bool bAdd, CSphString & sError ) const;
	bool			Alter_AddRemoveColumnar ( bool bAdd, const ISphSchema & tOldSchema, const ISphSchema & tNewSchema, columnar::Columnar_i * pColumnar, columnar::Builder_i * pBuilder, DWORD uRows, const CSphString & sIndex, CSphString & sError );
	bool 			Alter_AddRemoveFieldFromSchema ( bool bAdd, CSphSchema & tSchema, const CSphString & sFieldName, DWORD uFieldFlags, CSphString & sError );

	void			Alter_AddRemoveFromDocstore ( DocstoreBuilder_i & tBuilder, const Docstore_i * pDocstore, DWORD uNumDocs, const CSphSchema & tNewSchema );
};


#endif // _sphinx_alter_

