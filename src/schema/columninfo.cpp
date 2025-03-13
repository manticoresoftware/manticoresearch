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

#include "columninfo.h"

#include "sphinxint.h"
#include "attribute.h"
#include "indexing_sources/source_stats.h"
#include "match.h"

CSphColumnInfo::CSphColumnInfo ( const char* sName, ESphAttr eType )
	: m_sName ( sName )
	, m_eAttrType ( eType )
{
	sphColumnToLowercase ( const_cast<char*> ( m_sName.cstr() ) );
}


bool CSphColumnInfo::operator== ( const CSphColumnInfo & rhs ) const
{
	return m_sName == rhs.m_sName
		&& m_eAttrType == rhs.m_eAttrType
		&& m_tLocator.m_iBitCount == rhs.m_tLocator.m_iBitCount
		&& m_tLocator.m_iBitOffset == rhs.m_tLocator.m_iBitOffset
		&& m_tLocator.m_bDynamic == rhs.m_tLocator.m_bDynamic;
}


bool CSphColumnInfo::IsDataPtr() const
{
	return sphIsDataPtrAttr ( m_eAttrType );
}


bool CSphColumnInfo::IsColumnar() const
{
	return m_uAttrFlags & ATTR_COLUMNAR;
}


bool CSphColumnInfo::HasStringHashes() const
{
	return m_uAttrFlags & ATTR_COLUMNAR_HASHES;
}


bool CSphColumnInfo::IsColumnarExpr() const
{
	return m_pExpr.Ptr() && m_pExpr->IsColumnar();
}


bool CSphColumnInfo::IsStoredExpr() const
{
	return m_pExpr.Ptr() && m_pExpr->IsStored();
}


bool CSphColumnInfo::IsIndexedKNN() const
{
	return m_uAttrFlags & ATTR_INDEXED_KNN;
}


bool CSphColumnInfo::IsJoined() const
{
	return m_uAttrFlags & ATTR_JOINED;
}


bool CSphColumnInfo::IsIndexedSI() const
{
	return m_uAttrFlags & ATTR_INDEXED_SI;
}


CSphString sphDumpAttr ( const CSphColumnInfo & tAttr )
{
	CSphString sRes;
	sRes.SetSprintf ( "%s %s:%d@%d", sphTypeName ( tAttr.m_eAttrType ), tAttr.m_sName.cstr(), tAttr.m_tLocator.m_iBitCount, tAttr.m_tLocator.m_iBitOffset );
	return sRes;
}
