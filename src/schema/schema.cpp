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

#include "schema.h"

#include "attribute.h"
#include "rset.h"
#include "match.h"
#include "indexsettings.h"


//////////////////////////////////////////////////////////////////////////


CSphSchema::CSphSchema ( CSphString sName )
	: m_sName ( std::move ( sName ) )
{
	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ) );
}


CSphSchema::CSphSchema ( const CSphSchema& rhs )
{
	m_dDataPtrAttrs = rhs.m_dDataPtrAttrs;
	m_iFirstFieldLenAttr = rhs.m_iFirstFieldLenAttr;
	m_iLastFieldLenAttr = rhs.m_iLastFieldLenAttr;
	m_iRowSize = rhs.m_iRowSize;
	m_sName = rhs.m_sName;
	m_dFields = rhs.m_dFields;
	m_dAttrs = rhs.m_dAttrs;
	m_dStaticUsed = rhs.m_dStaticUsed;
	m_dDynamicUsed = rhs.m_dDynamicUsed;
	memcpy ( m_dBuckets, rhs.m_dBuckets, sizeof ( m_dBuckets ) );
}

CSphSchema::CSphSchema ( CSphSchema&& rhs ) noexcept
	: CSphSchema ( nullptr )
{
	Swap ( rhs );
}

void CSphSchema::Swap ( CSphSchema& rhs ) noexcept
{
	::Swap ( m_dDataPtrAttrs, rhs.m_dDataPtrAttrs );
	::Swap ( m_iFirstFieldLenAttr, rhs.m_iFirstFieldLenAttr );
	::Swap ( m_iLastFieldLenAttr, rhs.m_iLastFieldLenAttr );
	::Swap ( m_iRowSize, rhs.m_iRowSize );
	::Swap ( m_sName, rhs.m_sName );
	::Swap ( m_dFields, rhs.m_dFields );
	::Swap ( m_dAttrs, rhs.m_dAttrs );
	::Swap ( m_dStaticUsed, rhs.m_dStaticUsed );
	::Swap ( m_dDynamicUsed, rhs.m_dDynamicUsed );
	std::swap ( m_dBuckets, rhs.m_dBuckets );
}


CSphSchema& CSphSchema::operator= ( const ISphSchema& rhs )
{
	Reset();

	m_sName = rhs.GetName();

	for ( int i = 0; i < rhs.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo& tAttr = rhs.GetAttr ( i );

		if ( tAttr.m_tLocator.m_bDynamic )
			AddAttr ( tAttr, true );
		else // static attr, keep previous storage
			m_dAttrs.Add ( tAttr );
	}

	for ( int i = 0; i < rhs.GetFieldsCount(); ++i )
		AddField ( rhs.GetField ( i ) );

	RebuildHash();

	return *this;
}


CSphSchema& CSphSchema::operator= ( CSphSchema rhs )
{
	Swap ( rhs );
	return *this;
}

const char* CSphSchema::GetName() const
{
	return m_sName.cstr();
}


bool CSphSchema::CompareTo ( const CSphSchema& rhs, CSphString& sError, bool bFullComparison, bool bIndexLoadCheck ) const
{
	// check attr count
	if ( GetAttrsCount() != rhs.GetAttrsCount() )
	{
		sError.SetSprintf ( "attribute count mismatch (me=%s, in=%s, myattrs=%d, inattrs=%d)", m_sName.cstr(), rhs.m_sName.cstr(), GetAttrsCount(), rhs.GetAttrsCount() );
		return false;
	}

	// check attrs
	ARRAY_FOREACH ( i, m_dAttrs )
	{
		const CSphColumnInfo& tAttr1 = rhs.m_dAttrs[i];
		const CSphColumnInfo& tAttr2 = m_dAttrs[i];

		bool bMismatch;
		if ( bFullComparison )
			bMismatch = !( tAttr1 == tAttr2 );
		else
		{
			ESphAttr eAttr1 = tAttr1.m_eAttrType;
			ESphAttr eAttr2 = tAttr2.m_eAttrType;

			bMismatch = tAttr1.m_sName != tAttr2.m_sName || eAttr1 != eAttr2 || tAttr1.m_eWordpart != tAttr2.m_eWordpart || tAttr1.m_tLocator.m_iBitCount != tAttr2.m_tLocator.m_iBitCount || tAttr1.m_tLocator.m_iBitOffset != tAttr2.m_tLocator.m_iBitOffset;
		}

		if ( bMismatch )
		{
			sError.SetSprintf ( "attribute mismatch (me=%s, in=%s, idx=%d, myattr=%s, inattr=%s)",
					m_sName.cstr(),
					rhs.m_sName.cstr(),
					i,
					sphDumpAttr ( m_dAttrs[i] ).cstr(),
					sphDumpAttr ( rhs.m_dAttrs[i] ).cstr() );
			return false;
		}
	}

	// check field count
	if ( rhs.m_dFields.GetLength() != m_dFields.GetLength() )
	{
		sError.SetSprintf ( "fulltext fields count mismatch (me=%s, in=%s, myfields=%d, infields=%d)",
				m_sName.cstr(),
				rhs.m_sName.cstr(),
				m_dFields.GetLength(),
				rhs.m_dFields.GetLength() );
		return false;
	}

	// check fulltext field names
	ARRAY_FOREACH ( i, rhs.m_dFields )
	{
		if ( rhs.m_dFields[i].m_sName != m_dFields[i].m_sName )
		{
			sError.SetSprintf ( "fulltext field mismatch (me=%s, myfield=%s, idx=%d, in=%s, infield=%s)",
					m_sName.cstr(),
					rhs.m_sName.cstr(),
					i,
					m_dFields[i].m_sName.cstr(),
					rhs.m_dFields[i].m_sName.cstr() );
			return false;
		}

		DWORD uFlags1 = rhs.m_dFields[i].m_uFieldFlags;
		DWORD uFlags2 = m_dFields[i].m_uFieldFlags;

		// stored fields became the default at some point in time
		// some disk chunks have them and some don't
		// this doesn't affect functionality but it does affect checks on index load
		if ( bIndexLoadCheck )
		{
			uFlags1 &= ~CSphColumnInfo::FIELD_STORED;
			uFlags2 &= ~CSphColumnInfo::FIELD_STORED;
		}

		if ( uFlags1!=uFlags2 )
		{
			sError.SetSprintf ( "fulltext field flag mismatch (me=%s, myfield=%s, idx=%d, in=%s, infield=%s)",
				m_sName.cstr(),
				rhs.m_sName.cstr(),
				i,
				m_dFields[i].m_sName.cstr(),
				rhs.m_dFields[i].m_sName.cstr() );
			return false;
		}
	}

	return true;
}


int CSphSchema::GetFieldIndex ( const char* sName ) const
{
	if ( !sName )
		return -1;
	ARRAY_FOREACH ( i, m_dFields )
		if ( strcasecmp ( m_dFields[i].m_sName.cstr(), sName ) == 0 )
			return i;
	return -1;
}


const CSphColumnInfo* CSphSchema::GetField ( const char* szName ) const
{
	int iField = GetFieldIndex ( szName );
	if ( iField == -1 )
		return nullptr;

	return &m_dFields[iField];
}


int CSphSchema::GetAttrIndex ( const char* sName ) const
{
	if ( !sName )
		return -1;

	if ( m_dAttrs.GetLength() >= HASH_THRESH )
	{
		DWORD uCrc = sphCRC32 ( sName );
		DWORD uPos = m_dBuckets[uCrc % BUCKET_COUNT];
		while ( uPos != 0xffff && m_dAttrs[uPos].m_sName != sName )
			uPos = m_dAttrs[uPos].m_uNext;

		return (short)uPos; // 0xffff == -1 is our "end of list" marker
	}

	ARRAY_FOREACH ( i, m_dAttrs )
		if ( m_dAttrs[i].m_sName == sName )
			return i;

	return -1;
}


const CSphColumnInfo* CSphSchema::GetAttr ( const char* sName ) const
{
	int iIndex = GetAttrIndex ( sName );
	if ( iIndex >= 0 )
		return &m_dAttrs[iIndex];
	return NULL;
}


void CSphSchema::Reset()
{
	CSphSchemaHelper::ResetSchemaHelper();

	m_dFields.Reset();
	m_dAttrs.Reset();
	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ) );
	m_dStaticUsed.Reset();
	m_iRowSize = 0;
}


void CSphSchema::InsertAttr ( int iPos, const CSphColumnInfo& tCol, bool bDynamic )
{
	// it's redundant in case of AddAttr
	if ( iPos != m_dAttrs.GetLength() )
		UpdateHash ( iPos - 1, 1 );

	CSphSchemaHelper::InsertAttr ( m_dAttrs, bDynamic ? m_dDynamicUsed : m_dStaticUsed, iPos, tCol, bDynamic );

	// update field length locators
	if ( tCol.m_eAttrType == SPH_ATTR_TOKENCOUNT )
	{
		m_iFirstFieldLenAttr = m_iFirstFieldLenAttr == -1 ? iPos : Min ( m_iFirstFieldLenAttr, iPos );
		m_iLastFieldLenAttr = Max ( m_iLastFieldLenAttr, iPos );
	}

	// do hash add
	if ( m_dAttrs.GetLength() == HASH_THRESH )
		RebuildHash();
	else if ( m_dAttrs.GetLength() > HASH_THRESH )
	{
		WORD& uPos = GetBucketPos ( m_dAttrs[iPos].m_sName.cstr() );
		m_dAttrs[iPos].m_uNext = uPos;
		uPos = (WORD)iPos;
	}

	m_iRowSize = GetRowSize();
}


void CSphSchema::RemoveAttr ( const char* szAttr, bool bDynamic )
{
	int iIndex = GetAttrIndex ( szAttr );
	if ( iIndex < 0 )
		return;

	if ( bDynamic )
		m_dDynamicUsed.Reset();
	else
		m_dStaticUsed.Reset();

	CSphSchemaHelper::ResetSchemaHelper();
	m_iFirstFieldLenAttr = -1;
	m_iLastFieldLenAttr = -1;

	CSphVector<CSphColumnInfo> dBackup;
	dBackup.Reserve ( m_dAttrs.GetLength() );
	dBackup.SwapData ( m_dAttrs );
	ARRAY_FOREACH ( i, dBackup )
		if ( i != iIndex )
			AddAttr ( dBackup[i], bDynamic );
}


void CSphSchema::AddAttr ( const CSphColumnInfo& tCol, bool bDynamic )
{
	InsertAttr ( m_dAttrs.GetLength(), tCol, bDynamic );
}


void CSphSchema::AddField ( const char* szFieldName )
{
	CSphColumnInfo& tField = m_dFields.Add();
	tField.m_sName = szFieldName;
}


void CSphSchema::AddField ( const CSphColumnInfo& tField )
{
	m_dFields.Add ( tField );
}

void CSphSchema::RemoveField ( const char* szFieldName )
{
	auto iIdx = GetFieldIndex ( szFieldName );
	RemoveField ( iIdx );
}

void CSphSchema::RemoveField ( int iIdx )
{
	if ( iIdx >= 0 )
		m_dFields.Remove ( iIdx );
}

int CSphSchema::GetAttrId_FirstFieldLen() const
{
	return m_iFirstFieldLenAttr;
}


int CSphSchema::GetAttrId_LastFieldLen() const
{
	return m_iLastFieldLenAttr;
}


bool CSphSchema::IsReserved ( const char* szToken )
{
	static const char * dReserved[] =
	{
		"AND", "AS", "BY", "COLUMNARSCAN", "DISTINCT", "DIV", "DOCIDINDEX", "EXPLAIN",
		"FACET", "FALSE", "FORCE", "FROM", "IGNORE", "IN", "INDEXES", "IS", "LIMIT",
		"MOD", "NOT", "NO_COLUMNARSCAN", "NO_DOCIDINDEX", "NO_SECONDARYINDEX", "NULL",
		"OFFSET", "OR", "ORDER", "REGEX", "RELOAD", "SECONDARYINDEX", "SELECT", "SYSFILTERS",
		"TRUE", NULL
	};

	const char** p = dReserved;
	while ( *p )
		if ( strcasecmp ( szToken, *p++ ) == 0 )
			return true;
	return false;
}


WORD& CSphSchema::GetBucketPos ( const char* sName )
{
	DWORD uCrc = sphCRC32 ( sName );
	return m_dBuckets[uCrc % BUCKET_COUNT];
}


void CSphSchema::RebuildHash()
{
	if ( m_dAttrs.GetLength() < HASH_THRESH )
		return;

	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ) );

	ARRAY_FOREACH ( i, m_dAttrs )
	{
		WORD& uPos = GetBucketPos ( m_dAttrs[i].m_sName.cstr() );
		m_dAttrs[i].m_uNext = uPos;
		uPos = WORD ( i );
	}
}


void CSphSchema::RebuildLocators()
{
	CSphVector<CSphColumnInfo> dBackup = m_dAttrs;

	m_dDynamicUsed.Reset();
	m_dStaticUsed.Reset();

	CSphSchemaHelper::ResetSchemaHelper();
	m_dAttrs.Reset();
	m_iFirstFieldLenAttr = -1;
	m_iLastFieldLenAttr = -1;

	for ( auto& i : dBackup )
		AddAttr ( i, i.m_tLocator.m_bDynamic );
}


void CSphSchema::UpdateHash ( int iStartIndex, int iAddVal )
{
	if ( m_dAttrs.GetLength() < HASH_THRESH )
		return;

	for ( auto& dAttr : m_dAttrs )
	{
		WORD& uPos = dAttr.m_uNext;
		if ( uPos != 0xffff && uPos > iStartIndex )
			uPos = (WORD)( uPos + iAddVal );
	}
	for ( WORD& uPos : m_dBuckets )
		if ( uPos != 0xffff && uPos > iStartIndex )
			uPos = (WORD)( uPos + iAddVal );
}


void CSphSchema::AssignTo ( CSphRsetSchema& lhs ) const
{
	lhs = *this;
}


void CSphSchema::CloneWholeMatch ( CSphMatch& tDst, const CSphMatch& rhs ) const
{
	FreeDataPtrs ( tDst );
	tDst.Combine ( rhs, GetRowSize() );
	CopyPtrs ( tDst, rhs );
}


void CSphSchema::SetFieldWordpart ( int iField, ESphWordpart eWordpart )
{
	m_dFields[iField].m_eWordpart = eWordpart;
}


void CSphSchema::SwapAttrs ( CSphVector<CSphColumnInfo>& dAttrs )
{
	m_dAttrs.SwapData ( dAttrs );
	RebuildHash();
	m_iRowSize = GetRowSize();
}


ISphSchema* CSphSchema::CloneMe() const
{
	return new CSphSchema ( *this );
}


bool CSphSchema::HasBlobAttrs() const
{
	return m_dAttrs.any_of ( [] ( const CSphColumnInfo& i ) { return sphIsBlobAttr ( i ); } );
}


int CSphSchema::GetCachedRowSize() const
{
	return m_iRowSize;
}


void CSphSchema::SetupColumnarFlags ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings )
{
	bool bAllColumnar = false;
	bool bAllRowwise = false;
	bool bAllNoHashes = false;

	SmallStringHash_T<int> hColumnar, hRowwise, hNoHashes;
	for ( const auto & i : tSettings.m_dColumnarAttrs )
	{
		bAllColumnar |= i=="*";	
		hColumnar.Add ( 0, i );
	}

	for ( const auto & i : tSettings.m_dRowwiseAttrs )
	{
		bAllRowwise |= i=="*";	
		hRowwise.Add ( 0, i );
	}

	for ( const auto & i : tSettings.m_dColumnarStringsNoHash )
	{
		bAllNoHashes |= i=="*";	
		hNoHashes.Add ( 0, i );
	}

	bool bHaveColumnar = false;
	for ( auto& tAttr : m_dAttrs )
	{
		if ( ( hColumnar.Exists ( tAttr.m_sName ) && hRowwise.Exists ( tAttr.m_sName ) ) || ( bAllColumnar && bAllRowwise ) ) 
		{
			if ( pWarnings )
			{
				CSphString sWarning;
				sWarning.SetSprintf ( "both columnar and rowwise storages specified for '%s'; using rowwise", tAttr.m_sName.cstr() );
				pWarnings->Add ( sWarning );
			}
		}

		if ( sphIsInternalAttr ( tAttr ) )
			tAttr.m_eEngine = AttrEngine_e::ROWWISE;
		else
		{
			tAttr.m_eEngine = AttrEngine_e::DEFAULT;
			if ( bAllColumnar || hColumnar.Exists ( tAttr.m_sName ) )
				tAttr.m_eEngine = AttrEngine_e::COLUMNAR;

			if ( bAllRowwise || hRowwise.Exists ( tAttr.m_sName ) )
				tAttr.m_eEngine = AttrEngine_e::ROWWISE;
		}
		
		AttrEngine_e eIndexEngine = CombineEngines ( GetDefaultAttrEngine(), tSettings.m_eEngine );
		AttrEngine_e eEngine = CombineEngines ( eIndexEngine, tAttr.m_eEngine );
		if ( eEngine != AttrEngine_e::COLUMNAR )
			continue;

		if ( tAttr.m_eAttrType==SPH_ATTR_JSON )
		{
			if ( pWarnings )
			{
				CSphString sWarning;
				sWarning.SetSprintf ( "columnar storage does not support json type ('%s' attribute specified as columnar)", tAttr.m_sName.cstr() );
				pWarnings->Add ( sWarning );
			}

			tAttr.m_uAttrFlags &= ~CSphColumnInfo::ATTR_COLUMNAR;
			continue;
		}

		tAttr.m_uAttrFlags |= CSphColumnInfo::ATTR_COLUMNAR;

		// set strings to have pre-generated hashes by default
		if ( tAttr.m_eAttrType==SPH_ATTR_STRING && !( bAllNoHashes || hNoHashes.Exists ( tAttr.m_sName ) ) )
			tAttr.m_uAttrFlags |= CSphColumnInfo::ATTR_COLUMNAR_HASHES;

		bHaveColumnar = true;
	}

	// if all blob attrs turned columnar, there's no point in keeping the blob locator
	bool bHaveBlobs = m_dAttrs.any_of ( [] ( const CSphColumnInfo& tAttr ) { return sphIsBlobAttr ( tAttr ); } );
	const CSphColumnInfo* pBlobLocator = GetAttr ( sphGetBlobLocatorName() );
	if ( !bHaveBlobs && pBlobLocator )
		RemoveAttr ( sphGetBlobLocatorName(), pBlobLocator->m_tLocator.m_bDynamic );

	if ( bHaveColumnar )
		RebuildLocators(); // remove columnar attrs from row storage
}


void CSphSchema::SetupFlags ( const CSphSourceSettings & tSettings, bool bPQ, StrVec_t * pWarnings )
{
	bool bAllFieldsStored = false;
	for ( const auto & i : tSettings.m_dStoredFields )
		bAllFieldsStored |= i=="*";

	for ( auto & tSchemaField : m_dFields )
	{
		bool bFieldStored = bAllFieldsStored;
		for ( const auto & sStored : tSettings.m_dStoredFields )
			if ( !bFieldStored && tSchemaField.m_sName == sStored )
			{
				bFieldStored = true;
				break;
			}

		if ( bFieldStored )
			tSchemaField.m_uFieldFlags |= CSphColumnInfo::FIELD_STORED;

		for ( const auto & sStoredOnly : tSettings.m_dStoredOnlyFields )
			if ( tSchemaField.m_sName == sStoredOnly )
				tSchemaField.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;
	}

	if ( !bPQ )
		SetupColumnarFlags ( tSettings, pWarnings );

	bool bAllNonStored = false;
	SmallStringHash_T<int> hColumnarNonStored;
	for ( const auto & i : tSettings.m_dColumnarNonStoredAttrs )
	{
		bAllNonStored |= i=="*";
		hColumnarNonStored.Add ( 0, i );
	}

	int iNumColumnar = 0;
	for ( auto & i : m_dAttrs )
		if ( i.IsColumnar() && !( bAllNonStored || hColumnarNonStored.Exists ( i.m_sName ) ) )
			iNumColumnar++;

	if ( iNumColumnar>1 || ( iNumColumnar>0 && ( bAllNonStored || hColumnarNonStored.GetLength() ) ) )
	{
		for ( auto & i : m_dAttrs )
			if ( i.IsColumnar() && !hColumnarNonStored.Exists ( i.m_sName ) )
				i.m_uAttrFlags |= CSphColumnInfo::ATTR_STORED;
	}
}


bool CSphSchema::HasStoredFields() const
{
	return m_dFields.any_of ( [] ( const CSphColumnInfo& tField ) { return tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED; } );
}


bool CSphSchema::HasStoredAttrs() const
{
	return m_dAttrs.any_of ( []( const CSphColumnInfo & tAttr ){ return tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED; } );
}


bool CSphSchema::HasColumnarAttrs() const
{
	return m_dAttrs.any_of ( [] ( const CSphColumnInfo& tAttr ) { return tAttr.IsColumnar(); } );
}


bool CSphSchema::HasNonColumnarAttrs() const
{
	return m_dAttrs.any_of ( [] ( const CSphColumnInfo& tAttr ) { return !tAttr.IsColumnar(); } );
}


bool CSphSchema::IsFieldStored ( int iField ) const
{
	return !!( m_dFields[iField].m_uFieldFlags & CSphColumnInfo::FIELD_STORED );
}


bool CSphSchema::IsAttrStored ( int iAttr ) const
{
	return !!( m_dAttrs[iAttr].m_uAttrFlags & CSphColumnInfo::ATTR_STORED );
}
