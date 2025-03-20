//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "jsonsi.h"

#include "sphinxjson.h"
#include "sphinxint.h"
#include "secondarylib.h"

class JsonRowIterator_i
{
public:
	virtual					~JsonRowIterator_i() = default;

	virtual bool			Setup ( CSphString & sError ) = 0;
	virtual bool			Next() = 0;
	virtual const BYTE *	GetJSON( const CSphAttrLocator & tLocator ) const = 0;
	virtual RowID_t			GetRowID() const = 0;
};


class JsonRowIterator_c : public JsonRowIterator_i
{
public:
					JsonRowIterator_c ( const CSphRowitem * pPool, int64_t iNumRows, int iStride, const BYTE * pBlobPool );

	bool			Setup ( CSphString & sError ) override						{ return true; }
	bool			Next() override;
	const BYTE *	GetJSON ( const CSphAttrLocator & tLocator ) const override	{ return sphGetBlobAttr ( m_pPtr-m_iStride, tLocator, m_pBlobPool ).first; }
	RowID_t			GetRowID() const override									{ return m_tRowID-1; }

private:
	const CSphRowitem * m_pEnd = nullptr;
	const CSphRowitem * m_pPtr = nullptr;
	RowID_t				m_tRowID = 0;
	int					m_iStride = 0;
	const BYTE *		m_pBlobPool = nullptr;
};


JsonRowIterator_c::JsonRowIterator_c ( const CSphRowitem * pPool, int64_t iNumRows, int iStride, const BYTE * pBlobPool )
	: m_pEnd ( pPool + ( iNumRows*iStride ) )
	, m_pPtr ( pPool )
	, m_iStride ( iStride )
	, m_pBlobPool ( pBlobPool )
{}


bool JsonRowIterator_c::Next()
{
	if ( m_pPtr>=m_pEnd )
		return false;

	m_pPtr += m_iStride;
	m_tRowID++;
	return true;
}

//////////////////////////////////////////////////////////////////////

class JsonRowIteratorFile_c : public JsonRowIterator_i
{
public:
					JsonRowIteratorFile_c ( const CSphString & sOffsetFile, const CSphString & sSPB, int64_t iNumRows );

	bool			Setup ( CSphString & sError ) override;
	bool			Next() override;
	const BYTE *	GetJSON ( const CSphAttrLocator & tLocator ) const override	{ return sphGetBlobAttr ( m_dRow.Begin(), tLocator ).first; }
	RowID_t			GetRowID() const override									{ return m_tRowID-1; }

private:
	CSphAutoreader	m_tReaderOffset;
	CSphAutoreader	m_tReaderSPB;
	CSphString		m_sOffsetFile;
	CSphString		m_sSPBFile;
	int64_t			m_iNumRows = 0;
	SphOffset_t		m_tPrevOffset = 0;
	RowID_t			m_tRowID = 0;
	CSphVector<BYTE> m_dRow;
};


JsonRowIteratorFile_c::JsonRowIteratorFile_c ( const CSphString & sOffsetFile, const CSphString & sSPB, int64_t iNumRows )
	: m_sOffsetFile ( sOffsetFile )
	, m_sSPBFile ( sSPB )
	, m_iNumRows ( iNumRows )
{}


bool JsonRowIteratorFile_c::Setup ( CSphString & sError )
{
	if ( !m_tReaderOffset.Open ( m_sOffsetFile, sError ) )
		return false;

	if ( !m_tReaderSPB.Open ( m_sSPBFile, sError ) )
		return false;

	m_tReaderSPB.SeekTo ( sizeof(SphOffset_t), -1 );
	return true;
}

bool JsonRowIteratorFile_c::Next()
{
	if ( m_tRowID>=m_iNumRows )
		return false;

	bool bDelta = !!m_tReaderOffset.GetByte();
	SphOffset_t tOffset = m_tReaderOffset.UnzipOffset();
	if ( bDelta )
		tOffset += m_tPrevOffset;

	SphOffset_t tSize = m_tReaderOffset.UnzipOffset();
	m_dRow.Resize(tSize);

	m_tReaderSPB.SeekTo ( tOffset, -1 );
	m_tReaderSPB.Read ( m_dRow.Begin(), tSize );

	m_tRowID++;
	m_tPrevOffset = tOffset;

	return true;
}

//////////////////////////////////////////////////////////////////////

class JsonTypeDeductor_c
{
public:
	void SubmitRow ( const CSphString & sAttr, const BYTE * pJson )
	{
		bson::Bson_c tBson ( { (BYTE*)pJson, 1 } );
		ProcessJsonObj ( sAttr, tBson, [this]( const CSphString & sAttrName, const bson::NodeHandle_t & tNode )
			{
				StoredType_t * pStored = (*m_pTypes)(sAttrName);
				ESphJsonType eType = pStored ? pStored->m_eType : JSON_EOF;
				
				bool bTypeOk = DetermineNodeType ( eType, tNode );
				if ( pStored )
				{
					bool bOldTypeOk = pStored->m_bTypeOk;
					*pStored = { eType, 0, bOldTypeOk && bTypeOk };
				}
				else
					m_pTypes->Add ( { eType, 0, bTypeOk }, sAttrName );
			} );
	}

	common::Schema_t CreateSchema()
	{
		m_tSchema.resize(0);
		for ( auto & i : *m_pTypes )
		{
			if ( !i.second.m_bTypeOk )
				continue;

			i.second.m_iAttr = m_tSchema.size();

			common::StringHash_fn fnStringCalcHash = nullptr;
			common::AttrType_e eColumnarType = ToColumnarType ( i.second.m_eType );

			// fixme! make default collation configurable
			if ( eColumnarType==common::AttrType_e::STRING )
				fnStringCalcHash = LibcCIHash_fn::Hash;

			m_tSchema.push_back ( { i.first.cstr(), eColumnarType, fnStringCalcHash } );
		}

		return m_tSchema;
	}

	void SetBuilder ( SI::Builder_i * pBuilder ) { m_pBuilder = pBuilder; }

	void ConvertTypeAndStore ( const CSphString & sAttr, RowID_t tRowID, const BYTE * pJson )
	{
		assert(m_pBuilder);
		m_pBuilder->SetRowID(tRowID);

		bson::Bson_c tBson ( { (BYTE*)pJson, 1 } );
		ProcessJsonObj ( sAttr, tBson, [this]( const CSphString & sAttrName, const bson::NodeHandle_t & tNode )
		{
			StoredType_t * pStored = (*m_pTypes)(sAttrName);
			if ( pStored && pStored->m_bTypeOk )
				ConvertAndStore ( tNode, pStored->m_eType, pStored->m_iAttr );
		} );
	}

private:
	struct StoredType_t
	{
		ESphJsonType	m_eType = JSON_EOF;
		int				m_iAttr = 0;
		bool			m_bTypeOk = true;
	};

	using AttrHash_c = CSphOrderedHash<StoredType_t, CSphString, CSphStrHashFunc, 16384>;

	std::unique_ptr<AttrHash_c>	m_pTypes { std::make_unique<AttrHash_c>() };
	common::Schema_t			m_tSchema;
	SI::Builder_i *				m_pBuilder = nullptr;

	template <typename ACTION>
	void ProcessJsonObj ( const CSphString & sAttrPrefix, const bson::NodeHandle_t & tNode, ACTION && tAction )
	{
		if ( tNode.second==JSON_EOF )
			return;

		if ( tNode.second!=JSON_ROOT )
			tAction ( sAttrPrefix, tNode );

		if ( tNode.second!=JSON_ROOT && tNode.second!=JSON_OBJECT )
			return;

		bson::Bson_c tBson(tNode);
		tBson.ForEach ( [this,sAttrPrefix,tAction]( CSphString && sName, const bson::NodeHandle_t & tNode )
			{
				CSphString sAttrName;
				sAttrName.SetSprintf ( "%s['%s']", sAttrPrefix.cstr(), sName.cstr() );
				ProcessJsonObj ( sAttrName, tNode, tAction );
			} );
	}

	void ConvertAndStore ( int64_t iValue, ESphJsonType eType, int iAttr )
	{
		switch ( eType )
		{
		case JSON_INT32:
		case JSON_INT64:
		case JSON_OBJECT:
			m_pBuilder->SetAttr ( iAttr, iValue );
			break;

		case JSON_DOUBLE:
			m_pBuilder->SetAttr ( iAttr, sphF2DW(float(iValue)) );
			break;

		case JSON_STRING:
			{
				char szBuf[64];
				snprintf ( szBuf, 64, INT64_FMT, iValue );
				m_pBuilder->SetAttr ( iAttr, (const uint8_t*)szBuf, strlen(szBuf) );
			}
			break;

		default:
			assert ( 0 && "Internal error on json type conversion");
			break;
		}
	}

	void ConvertAndStore ( double fValue, ESphJsonType eType, int iAttr )
	{
		switch ( eType )
		{
		case JSON_DOUBLE:
			m_pBuilder->SetAttr ( iAttr, sphF2DW ( (float)fValue ) );
			break;

		case JSON_STRING:
			{
				char szBuf[64];
				snprintf ( szBuf, 64, "%f", (float)fValue );
				m_pBuilder->SetAttr ( iAttr, (const uint8_t*)szBuf, strlen(szBuf) );
			}
			break;

		default:
			assert ( 0 && "Internal error on json type conversion");
			break;
		}
	}

	void ConvertAndStore ( const CSphString & sValue, ESphJsonType eType, int iAttr )
	{
		switch ( eType )
		{
		case JSON_STRING:
			m_pBuilder->SetAttr ( iAttr, (const uint8_t*)sValue.cstr(), sValue.Length() );
			break;

		default:
			assert ( 0 && "Internal error on json type conversion");
			break;
		}
	}

	void ConvertAndStore ( const bson::NodeHandle_t & tNode, ESphJsonType eType, int iAttr )
	{
		switch ( tNode.second )
		{
		case JSON_INT32:
		case JSON_INT64:
			ConvertAndStore ( bson::Int(tNode), eType, iAttr );
			break;

		case JSON_DOUBLE:
			ConvertAndStore ( bson::Double(tNode), eType, iAttr );
			break;

		case JSON_STRING:
			ConvertAndStore ( bson::String(tNode), eType, iAttr );
			break;

		case JSON_STRING_VECTOR:
			bson::ForEach ( tNode, [this,eType,iAttr]( const CSphString & sName, const bson::NodeHandle_t & tLocator )
				{
					ConvertAndStore ( bson::String(tLocator), eType, iAttr );
				} );
			break;

		case JSON_INT32_VECTOR:
		case JSON_INT64_VECTOR:
			bson::ForEach ( tNode, [this,eType,iAttr]( const CSphString & sName, const bson::NodeHandle_t & tLocator )
				{
					ConvertAndStore ( bson::Int(tLocator), eType, iAttr );
				} );
			break;

		case JSON_DOUBLE_VECTOR:
			bson::ForEach ( tNode, [this,eType,iAttr]( const CSphString & sName, const bson::NodeHandle_t & tLocator )
				{
					ConvertAndStore ( bson::Double(tLocator), eType, iAttr );
				} );
			break;

		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
			ConvertAndStore ( bson::Int(tNode), eType, iAttr );
			break;

		case JSON_OBJECT:
			ConvertAndStore ( int64_t(1), eType, iAttr );
			break;

		case JSON_MIXED_VECTOR:
			break; // assume 0-length

		default:
			assert ( 0 && "Internal error: unsupported json type" );
			break;
		}
	}

	static common::AttrType_e ToColumnarType ( ESphJsonType eType )
	{
		switch ( eType )
		{
		case JSON_INT32:	return common::AttrType_e::UINT32;
		case JSON_INT64:	return common::AttrType_e::INT64;
		case JSON_DOUBLE:	return common::AttrType_e::FLOAT;
		case JSON_STRING:	return common::AttrType_e::STRING;
		default:
			assert ( 0 && "Internal error: unsupported json type" );
			return common::AttrType_e::NONE;
		}
	}

	static ESphJsonType ToWidestType ( ESphJsonType ePrevType, ESphJsonType eNodeType )
	{
		assert ( ePrevType<=JSON_STRING && eNodeType<=JSON_STRING );
		return Max ( ePrevType, eNodeType );
	}

	static bool DetermineNodeType ( ESphJsonType & eType, const bson::NodeHandle_t & tNode )
	{
		ESphJsonType eNodeType = tNode.second;
		switch ( eNodeType )
		{
		case JSON_INT32:
		case JSON_INT64:
		case JSON_DOUBLE:
		case JSON_STRING:
			eType = ToWidestType ( eType, eNodeType );
			break;

		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
			eType = ToWidestType ( eType, JSON_INT32 );
			break;

		case JSON_STRING_VECTOR:
			eType = ToWidestType ( eType, JSON_STRING );
			break;

		case JSON_INT32_VECTOR:
			eType = ToWidestType ( eType, JSON_INT32 );
			break;

		case JSON_INT64_VECTOR:
			eType = ToWidestType ( eType, JSON_INT64 );
			break;

		case JSON_DOUBLE_VECTOR:
			eType = ToWidestType ( eType, JSON_DOUBLE );
			break;

		case JSON_OBJECT:
			eType = ToWidestType ( eType, JSON_INT32 );
			break;

		case JSON_MIXED_VECTOR:
		{
			const BYTE * p = tNode.first;
			sphJsonUnpackInt(&p);	// total len
			if ( sphJsonUnpackInt(&p) )
				return false;

			// 0-length mixed vectors are supported
			eType = ToWidestType ( eType, JSON_INT32 );
		}
		break;

		default:
			break;
		}

		return true;
	}
};

////////////////////////////////////////////////////////////////////

template <typename CREATE_ITERATOR>
static bool BuildJsonSI ( const StrVec_t & dAttributes, const ISphSchema & tSchema, const CSphString & sFile, const CSphString & sTmpFile, CREATE_ITERATOR && fnCreateIterator, CSphString & sError )
{
	CSphVector<CSphAttrLocator> dLocators;
	for ( auto & i : dAttributes )
	{
		const CSphColumnInfo * pAttr = tSchema.GetAttr ( i.cstr() );
		assert(pAttr);
		dLocators.Add ( pAttr->m_tLocator );
	}

	JsonTypeDeductor_c tDeductor;
	{
		std::unique_ptr<JsonRowIterator_i> pIterator = std::unique_ptr<JsonRowIterator_i> ( fnCreateIterator(sError) );
		while ( pIterator->Next() )
			ARRAY_FOREACH( i, dAttributes )
				tDeductor.SubmitRow ( dAttributes[i], pIterator->GetJSON ( dLocators[i] ) );
	}

	common::Schema_t tSISchema = tDeductor.CreateSchema();
	if ( tSISchema.empty() )
		return true;

	// FIXME!!! pass these settings to the function, don't use default
	BuildBufferSettings_t tSettings; // use default buffer settings
 	std::unique_ptr<SI::Builder_i> pBuilder = CreateSecondaryIndexBuilder ( tSISchema, tSettings.m_iSIMemLimit, sTmpFile, tSettings.m_iBufferStorage, sError );
	if ( !pBuilder )
		return false;

	tDeductor.SetBuilder ( pBuilder.get() );

	{
		std::unique_ptr<JsonRowIterator_i> pIterator = std::unique_ptr<JsonRowIterator_i> ( fnCreateIterator(sError) );
		while ( pIterator->Next() )
			ARRAY_FOREACH( i, dAttributes )
				tDeductor.ConvertTypeAndStore ( dAttributes[i], pIterator->GetRowID(), pIterator->GetJSON ( dLocators[i] ) );
	}

	std::string sErrorSTL;
	if ( !pBuilder->Done(sErrorSTL) )
	{
		sError = sErrorSTL.c_str();
		return false;
	}

	StrVec_t dSrc, dDst;
	dSrc.Add(sTmpFile);
	dDst.Add(sFile);

	if ( !RenameWithRollback ( dSrc, dDst, sError ) )
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////

class JsonSIBuilder_c : public JsonSIBuilder_i
{
public:
			JsonSIBuilder_c ( const ISphSchema & tSchema, const CSphString & sSPB, const CSphString & sSIFile );

	bool	Setup ( CSphString & sError )			{ return m_tWriter.OpenFile ( m_sTmpOffsetFile, sError ); }
	void	AddRowOffsetSize ( std::pair<SphOffset_t,SphOffset_t> tOffsetSize ) override;
	bool	Done ( CSphString & sError ) override;

private:
	const ISphSchema &	m_tSchema;
	CSphString			m_sSPB;
	CSphString			m_sSIFile;
	CSphString			m_sTmpOffsetFile;
	CSphWriter			m_tWriter;
	SphOffset_t			m_tPrevOffset = 0;
	int64_t				m_iNumRows = 0;
};


JsonSIBuilder_c::JsonSIBuilder_c ( const ISphSchema & tSchema, const CSphString & sSPB, const CSphString & sSIFile )
	: m_tSchema ( tSchema )
	, m_sSPB ( sSPB )
	, m_sSIFile ( sSIFile )
{
	m_sTmpOffsetFile.SetSprintf ( "%s.offset.tmp", m_sSIFile.cstr() );
}


void JsonSIBuilder_c::AddRowOffsetSize ( std::pair<SphOffset_t,SphOffset_t> tOffsetSize )
{
	if ( tOffsetSize.first>=m_tPrevOffset )
	{
		m_tWriter.PutByte(1);
		m_tWriter.ZipOffset ( tOffsetSize.first - m_tPrevOffset );
	}
	else
	{
		m_tWriter.PutByte(0);
		m_tWriter.ZipOffset ( tOffsetSize.first );
	}

	m_tWriter.ZipOffset ( tOffsetSize.second );

	m_tPrevOffset = tOffsetSize.first;
	m_iNumRows++;
}


bool JsonSIBuilder_c::Done ( CSphString & sError )
{
	m_tWriter.CloseFile();

	StrVec_t dAttributes;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		if ( m_tSchema.GetAttr(i).IsIndexedSI() )
			dAttributes.Add ( m_tSchema.GetAttr(i).m_sName );

	CSphString sTmpSIFile;
	sTmpSIFile.SetSprintf ( "%s.jsonsi.tmp", m_sSIFile.cstr() );

	bool bOk = BuildJsonSI ( dAttributes, m_tSchema, m_sSIFile, sTmpSIFile, [this] ( CSphString & sError ) -> std::unique_ptr<JsonRowIterator_i>
		{
			std::unique_ptr<JsonRowIterator_i> pIterator = std::make_unique<JsonRowIteratorFile_c> ( m_sTmpOffsetFile, m_sSPB, m_iNumRows );
			if ( !pIterator->Setup(sError) )
				pIterator.reset();

			return pIterator;
		}, sError );

	::unlink ( m_sTmpOffsetFile.cstr() );

	return bOk;
}

////////////////////////////////////////////////////////////////////

bool BuildJsonSI ( const CSphString & sAttribute, const CSphRowitem * pPool, int64_t iNumRows, const ISphSchema & tSchema, const BYTE * pBlobPool, const CSphString & sFile, const CSphString & sTmpFile, CSphString & sError )
{
	if ( sphFileExists(sFile) )
	{
		sError.SetSprintf ( "file '%s' already exists", sFile.cstr() );
		return false;
	}

	if ( sAttribute==GetFixedJsonSIAttrName() )
	{
		sError.SetSprintf ( "secondary index attribute name '%s' not allowed", sAttribute.cstr() );
		return false;
	}

	StrVec_t dAttributes;
	dAttributes.Add(sAttribute);
	int iStride = tSchema.GetRowSize();
	return BuildJsonSI ( dAttributes, tSchema, sFile, sTmpFile, [pPool, iNumRows, iStride, pBlobPool] ( CSphString & sError ) -> std::unique_ptr<JsonRowIterator_i>
	{
		return std::make_unique<JsonRowIterator_c> ( pPool, iNumRows, iStride, pBlobPool );
	}, sError );
}


std::unique_ptr<JsonSIBuilder_i> CreateJsonSIBuilder ( const ISphSchema & tSchema, const CSphString & sSPB, const CSphString & sSIFile, CSphString & sError )
{
	auto pBuilder = std::make_unique<JsonSIBuilder_c> ( tSchema, sSPB, sSIFile );
	if ( !pBuilder->Setup(sError) )
		pBuilder.reset();

	return pBuilder;
}


CSphString UnifyJsonFieldName ( const CSphString & sName )
{
	enum class State_e
	{
		NONE,
		KEY_NAME,
		QUOTES
	};

	State_e eState = State_e::KEY_NAME;
	const char * pStart = sName.cstr();
	const char * pMax = pStart + sName.Length() + 1;
	const char * p = pStart;
	CSphString sRes;

	while ( p < pMax )
	{
		switch ( eState )
		{
		case State_e::NONE:
			switch ( *p )
			{
			case '[':
				if ( *(p+1)=='\'' )
				{
					eState = State_e::QUOTES;
					p++;
					pStart = p+1;
				}
				break;

			case '.':
				eState = State_e::KEY_NAME;
				pStart = p+1;
				break;

			default: break;
			}
			break;

		case State_e::QUOTES:
			if ( *p=='\'' && *(p+1)==']' )
			{
				eState = State_e::NONE;
				CSphString sKey;
				sKey.SetBinary( pStart, p-pStart );
				sRes.SetSprintf ( "%s['%s']", sRes.cstr(), sKey.cstr() );
				p++;
			}
			break;

		case State_e::KEY_NAME:
			if ( *p=='.' || *p=='[' || *p=='\0' )
			{
				eState = State_e::NONE;
				CSphString sKey;
				sKey.SetBinary( pStart, p-pStart );

				if ( sRes.IsEmpty() )
					sRes = sKey;
				else
					sRes.SetSprintf ( "%s['%s']", sRes.cstr(), sKey.cstr() );

				p--;
			}
			break;

		default:
			break;
		}

		p++;
	}

	return sRes;
}


CSphString GetFixedJsonSIAttrName()
{
	return "_extra_";
}
