//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "source_svpipe.h"

#include "indexcheck.h"
#include "schema_configurator.h"
#include "attribute.h"
#include "sphinxint.h"
#include "conversion.h"
#include "tokenizer/tokenizer.h"

class CSphSource_BaseSV : public CSphSource, public CSphSchemaConfigurator<CSphSource_BaseSV>
{
	using BASE = CSphSource;

public:
	explicit		CSphSource_BaseSV ( const char * sName );
					~CSphSource_BaseSV () override;

	bool	Connect ( CSphString & sError ) override;			///< run the command and open the pipe
	void	Disconnect () override;								///< close the pipe
	const char *	DecorateMessage ( const char * sTemplate, ... ) const __attribute__ ( ( format ( printf, 2, 3 ) ) );

	bool	IterateStart ( CSphString & ) override;				///< Connect() starts getting documents automatically, so this one is empty
	BYTE **	NextDocument ( bool & bEOF, CSphString & ) override;				///< parse incoming chunk and emit some hits
	const int *	GetFieldLengths () const override { return m_dFieldLengths.Begin(); }

	bool	IterateMultivaluedStart ( int, CSphString & ) override	{ return false; }
	bool	IterateMultivaluedNext ( int64_t &, int64_t & ) override{ return false; }
	bool	IterateKillListStart ( CSphString & ) override			{ return false; }
	bool	IterateKillListNext ( DocID_t & ) override				{ return false; }

	void	Setup ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings ) override;
	bool	SetupPipe ( const CSphConfigSection & hSource, FILE * pPipe, bool bOwnPipe, CSphString & sError );

protected:
	enum ESphParseResult
	{
		PARSING_FAILED,
		GOT_DOCUMENT,
		DATA_OVER
	};

	CSphVector<BYTE>			m_dBuf;
	CSphFixedVector<char>		m_dError {1024};
	CSphFixedVector<int>		m_dColumnsLen {0};
	CSphVector<RemapXSV_t>		m_dRemap;

	// output
	CSphFixedVector<BYTE *>		m_dFields {0};
	CSphFixedVector<int>		m_dFieldLengths {0};

	FILE *						m_pFP = nullptr;
	bool						m_bOwnPipe = false;
	int							m_iDataStart = 0;	///< where the next line to parse starts in m_dBuf
	int							m_iDocStart = 0;	///< where the last parsed document stats in m_dBuf
	int							m_iBufUsed = 0;		///< bytes [0,m_iBufUsed) are actually currently used; the rest of m_dBuf is free
	int							m_iLine = 0;

	BYTE **					ReportDocumentError();
	virtual bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) = 0;
	virtual ESphParseResult	SplitColumns ( CSphString & ) = 0;
	bool				SetupSVpipeSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphSchema & tSchema, CSphString & sError ) const;

protected:
	bool	StoreAttribute ( int iAttr, int iOff );
};


class CSphSource_TSV : public CSphSource_BaseSV
{
	using CSphSource_BaseSV::CSphSource_BaseSV;

public:
	ESphParseResult	SplitColumns ( CSphString & sError ) final;					///< parse incoming chunk and emit some hits
	bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) final;
};


class CSphSource_CSV : public CSphSource_BaseSV
{
public:
	explicit		CSphSource_CSV ( const char * sName );

	ESphParseResult	SplitColumns ( CSphString & sError ) final;					///< parse incoming chunk and emit some hits
	bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) final;
	void			SetDelimiter ( const char * sDelimiter );

private:
	BYTE			m_iDelimiter;
};


bool CSphSource_BaseSV::SetupSVpipeSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphSchema & tSchema, CSphString & sError ) const
{
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_uint"), SPH_ATTR_INTEGER, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_timestamp"), SPH_ATTR_TIMESTAMP, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_bool"), SPH_ATTR_BOOL, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_float"), SPH_ATTR_FLOAT, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_bigint"), SPH_ATTR_BIGINT, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_multi"), SPH_ATTR_UINT32SET, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_multi_64"), SPH_ATTR_INT64SET, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_float_vector"), SPH_ATTR_FLOAT_VECTOR, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_string"), SPH_ATTR_STRING, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_json"), SPH_ATTR_JSON, tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_field_string"), SPH_ATTR_STRING, tSchema, sError );

	if ( !bOk )
		return false;

	CSphString sAttrOrder = hSource.GetStr ( "csvpipe_attr_order" );
	if ( !sAttrOrder.IsEmpty() )
	{
		StrVec_t dNames;
		sphSplit ( dNames, sAttrOrder.cstr(), "," );
		CSphVector<bool> dUsed ( tSchema.GetAttrsCount() );
		dUsed.ZeroVec();
		CSphSchema tOrdered ( tSchema.GetName() );
		for ( const CSphString & sName : dNames )
		{
			int iAttr = tSchema.GetAttrIndex ( sName.cstr() );
			if ( iAttr<0 || dUsed[iAttr] )
			{
				sError.SetSprintf ( "csvpipe_attr_order contains %s attribute '%s'", iAttr<0 ? "unknown" : "duplicate", sName.cstr() );
				return false;
			}
			tOrdered.AddAttr ( tSchema.GetAttr(iAttr), true );
			dUsed[iAttr] = true;
		}
		for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
			if ( !dUsed[i] )
				tOrdered.AddAttr ( tSchema.GetAttr(i), true );
		tSchema = std::move ( tOrdered );
	}

	bOk &= ConfigureFields ( hSource("csvpipe_field"), bWordDict, tSchema, sError );
	bOk &= ConfigureFields ( hSource("csvpipe_field_string"), bWordDict, tSchema, sError );
	return bOk;
}

constexpr int SOURCE_BINARY_BUFFER_SIZE = 64 * 1024;

class SourceBinaryReader_c final : ISphNoncopyable
{
public:
	SourceBinaryReader_c ()
	{
		m_dBuffer.Resize ( SOURCE_BINARY_BUFFER_SIZE );
	}

	void SetFile ( FILE * pFile )
	{
		m_pFile = pFile;
		m_iPos = 0;
		m_iUsed = 0;
		m_bError = false;
		m_bCleanEOF = false;
		m_sError = "";
		if ( m_pFile )
			clearerr ( m_pFile );
	}

	bool ReadBatch ( uint32_t & uRows, bool & bEOF )
	{
		bEOF = false;
		BYTE uFirst;
		if ( !ReadOptionalByte ( uFirst, bEOF ) )
			return false;
		if ( bEOF )
			return true;
		BYTE dValue[4] = { uFirst, 0, 0, 0 };
		if ( !ReadBytes ( dValue+1, 3 ) )
			return false;
		uRows = (uint32_t)dValue[0] | ((uint32_t)dValue[1]<<8) | ((uint32_t)dValue[2]<<16) | ((uint32_t)dValue[3]<<24);
		return true;
	}

	template <typename HANDLER>
	bool ReadValue ( HANDLER & tHandler )
	{
		uint32_t uKind;
		if ( !ReadU32 ( uKind ) )
			return false;
		switch ( (ESphAttr)uKind )
		{
		case SPH_ATTR_BIGINT:
			{
				BYTE uNegative;
				uint64_t uValue;
				return ReadByte ( uNegative ) && ReadU64 ( uValue ) && tHandler.OnInteger ( uValue, !!uNegative );
			}
		case SPH_ATTR_FLOAT:
			{
				uint32_t uFloat;
				return ReadU32 ( uFloat ) && tHandler.OnFloat ( sphDW2F ( (DWORD)uFloat ) );
			}
		case SPH_ATTR_STRING:
			{
				uint32_t uLength;
				if ( !ReadU32 ( uLength ) || !Ensure ( (int)uLength ) )
					return false;
				const char * pValue = (const char *)m_dBuffer.Begin()+m_iPos;
				const bool bOk = tHandler.OnString ( pValue, (int)uLength );
				m_iPos += (int)uLength;
				return bOk;
			}
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
		case SPH_ATTR_FLOAT_VECTOR:
			{
				uint32_t uCount;
				if ( !ReadU32 ( uCount ) )
					return false;
				if ( !tHandler.OnMvaBegin() )
					return false;
				const bool bFloat = (ESphAttr)uKind==SPH_ATTR_FLOAT_VECTOR;
				for ( uint32_t i=0; i<uCount; ++i )
				{
					if ( bFloat )
					{
						uint32_t uFloat;
						if ( !ReadU32 ( uFloat ) )
							return false;
						if ( !tHandler.OnMvaFloat ( sphDW2F ( (DWORD)uFloat ) ) )
							return false;
					} else
					{
						uint64_t uInteger;
						if ( !ReadU64 ( uInteger ) )
							return false;
						if ( !tHandler.OnMvaInteger ( (int64_t)uInteger ) )
							return false;
					}
				}
				return tHandler.OnMvaEnd();
			}
		}
		return false;
	}

	const CSphString & GetError () const { return m_sError; }

private:
	bool ReadOptionalByte ( BYTE & uValue, bool & bEOF )
	{
		bEOF = false;
		if ( Ensure ( 1, true ) )
		{
			uValue = m_dBuffer[m_iPos++];
			return true;
		}
		if ( !m_bError && m_bCleanEOF )
		{
			bEOF = true;
			return true;
		}
		return false;
	}

	bool ReadByte ( BYTE & uValue )
	{
		if ( !Ensure ( 1 ) )
			return false;
		uValue = m_dBuffer[m_iPos++];
		return true;
	}

	bool ReadU16 ( uint16_t & uValue )
	{
		BYTE dValue[2];
		if ( !ReadBytes ( dValue, sizeof ( dValue ) ) )
			return false;
		uValue = (uint16_t)dValue[0] | ((uint16_t)dValue[1]<<8);
		return true;
	}

	bool ReadU32 ( uint32_t & uValue )
	{
		BYTE dValue[4];
		if ( !ReadBytes ( dValue, sizeof ( dValue ) ) )
			return false;
		uValue = (uint32_t)dValue[0] | ((uint32_t)dValue[1]<<8) | ((uint32_t)dValue[2]<<16) | ((uint32_t)dValue[3]<<24);
		return true;
	}

	bool ReadU64 ( uint64_t & uValue )
	{
		BYTE dValue[8];
		if ( !ReadBytes ( dValue, sizeof ( dValue ) ) )
			return false;
		uValue = 0;
		for ( int i=0; i<8; ++i )
			uValue |= (uint64_t)dValue[i] << (i*8);
		return true;
	}

	bool ReadBytes ( void * pData, int iLength )
	{
		if ( !Ensure ( iLength ) )
			return false;
		memcpy ( pData, m_dBuffer.Begin()+m_iPos, iLength );
		m_iPos += iLength;
		return true;
	}

	bool Ensure ( int iNeed, bool bOptional=false )
	{
		if ( m_iUsed-m_iPos>=iNeed )
		{
			m_bCleanEOF = false;
			return true;
		}

		if ( m_iPos )
		{
			const int iAvailable = m_iUsed-m_iPos;
			if ( iAvailable )
				memmove ( m_dBuffer.Begin(), m_dBuffer.Begin()+m_iPos, iAvailable );
			m_iUsed = iAvailable;
			m_iPos = 0;
		}

		if ( m_dBuffer.GetLength()<iNeed )
			m_dBuffer.Resize ( iNeed );

		while ( m_iUsed<iNeed )
		{
			if ( !m_pFile )
				return SetError ( "binary bulk source has no input" );
			// Do not let a previously enlarged buffer increase normal read-ahead.
			const int iRead = Max ( iNeed, SOURCE_BINARY_BUFFER_SIZE ) - m_iUsed;
			const int iGot = (int)fread ( m_dBuffer.Begin()+m_iUsed, 1, iRead, m_pFile );
			m_iUsed += iGot;
			if ( iGot )
				continue;
			if ( feof ( m_pFile ) )
			{
				m_bCleanEOF = true;
				if ( bOptional && !m_iUsed )
					return false;
				return SetError ( "truncated binary bulk source frame" );
			}
			return SetError ( "binary bulk source read error: %s", strerrorm ( errno ) );
		}
		m_bCleanEOF = false;
		return true;
	}

	bool SetError ( const char * sFormat, ... )
	{
		va_list ap;
		va_start ( ap, sFormat );
		m_sError.SetSprintfVa ( sFormat, ap );
		va_end ( ap );
		m_bError = true;
		return false;
	}

	FILE * m_pFile = nullptr;
	CSphVector<BYTE> m_dBuffer;
	int m_iPos = 0;
	int m_iUsed = 0;
	bool m_bError = false;
	bool m_bCleanEOF = false;
	CSphString m_sError;
};

class CSphSource_Binary : public CSphSource_BaseSV
{
	static int64_t GetSignedValue ( uint64_t uValue, bool bNegative )
	{
		if ( bNegative )
		{
			if ( uValue>uint64_t ( LLONG_MAX ) )
				return LLONG_MIN;
			return -int64_t ( uValue );
		}
		return uValue>uint64_t ( LLONG_MAX ) ? LLONG_MAX : int64_t ( uValue );
	}

	class BinaryValueSink_c
	{
	public:
		BinaryValueSink_c ( CSphSource_Binary & tSource, int iRemap, CSphString & sError )
			: m_tSource ( tSource )
			, m_iRemap ( iRemap )
			, m_sError ( sError )
		{}

		bool OnInteger ( uint64_t uValue, bool bNegative )
		{
			return m_tSource.StoreBinaryInteger ( m_iRemap, uValue, bNegative, m_sError );
		}

		bool OnFloat ( float fValue )
		{
			return m_tSource.StoreBinaryFloat ( m_iRemap, fValue, m_sError );
		}

		bool OnString ( const char * pData, int iLength )
		{
			return m_tSource.StoreBinaryString ( m_iRemap, pData, iLength, m_sError );
		}

		bool OnMvaBegin ()
		{
			const RemapXSV_t & tRemap = m_tSource.m_dRemap[m_iRemap];
			m_bTextMva = tRemap.m_iField!=-1;
			if ( tRemap.m_iAttr!=-1 )
			{
				const ESphAttr eType = m_tSource.m_tSchema.GetAttr ( tRemap.m_iAttr ).m_eAttrType;
				m_bTextMva |= eType!=SPH_ATTR_UINT32SET && eType!=SPH_ATTR_INT64SET && eType!=SPH_ATTR_FLOAT_VECTOR;
			}
			m_bFirstMva = true;
			m_iTextStart = m_bTextMva ? m_tSource.m_iBufUsed : -1;
			return true;
		}

		bool OnMvaInteger ( int64_t iValue )
		{
			if ( m_bTextMva )
			{
				if ( !AppendSeparator() )
					return false;
				CSphString sValue;
				sValue.SetSprintf ( INT64_FMT, iValue );
				return m_tSource.AppendBinary ( sValue.cstr(), sValue.Length(), false, m_sError );
			}
			return m_tSource.StoreBinaryMvaInteger ( m_iRemap, iValue, m_sError );
		}

		bool OnMvaFloat ( float fValue )
		{
			if ( m_bTextMva )
			{
				if ( !AppendSeparator() )
					return false;
				CSphString sValue;
				sValue.SetSprintf ( "%.9g", fValue );
				return m_tSource.AppendBinary ( sValue.cstr(), sValue.Length(), false, m_sError );
			}
			return m_tSource.StoreBinaryMvaFloat ( m_iRemap, fValue, m_sError );
		}

		bool OnMvaEnd ()
		{
			return !m_bTextMva || m_tSource.FinishBinaryTextValue ( m_iRemap, m_iTextStart, m_sError );
		}

	private:
		bool AppendSeparator ()
		{
			if ( m_bFirstMva )
			{
				m_bFirstMva = false;
				return true;
			}
			return m_tSource.AppendBinary ( " ", 1, false, m_sError );
		}

		CSphSource_Binary & m_tSource;
		int m_iRemap;
		CSphString & m_sError;
		int m_iTextStart = -1;
		bool m_bTextMva = false;
		bool m_bFirstMva = true;
	};

public:
	explicit CSphSource_Binary ( const char * sName )
		: CSphSource_BaseSV ( sName )
	{}

	ESphParseResult SplitColumns ( CSphString & ) final
	{
		return PARSING_FAILED;
	}

	bool IterateStart ( CSphString & ) override
	{
		m_tReader.SetFile ( m_pFP );
		m_iPlainFieldsLength = m_tSchema.GetFieldsCount();
		m_iBufUsed = 0;
		m_iLine = 0;
		m_iRowsRemaining = 0;
		return true;
	}

	BYTE ** NextDocument ( bool & bEOF, CSphString & sError ) override
	{
		bEOF = false;
		if ( !m_iRowsRemaining )
		{
			bool bInputEOF;
			if ( !m_tReader.ReadBatch ( m_iRowsRemaining, bInputEOF ) )
			{
				sError = m_tReader.GetError();
				return ReportDocumentError();
			}
			if ( bInputEOF )
			{
				bEOF = true;
				m_tDocInfo.m_tRowID = INVALID_ROWID;
				return nullptr;
			}
		}

		for ( auto & tField : m_dFields )
			tField = nullptr;
		for ( auto & iLength : m_dFieldLengths )
			iLength = 0;
		m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
		for ( auto & dMva : m_dMvas )
			dMva.Resize ( 0 );

		m_iBufUsed = 0;
		ARRAY_FOREACH ( i, m_dRemap )
		{
			BinaryValueSink_c tSink ( *this, i, sError );
			if ( !m_tReader.ReadValue ( tSink ) )
			{
				if ( sError.IsEmpty() )
					sError = m_tReader.GetError();
				if ( sError.IsEmpty() )
					sError.SetSprintf ( "source '%s': invalid binary value at column %d, row %d", m_tSchema.GetName(), i+1, m_iLine+1 );
				return ReportDocumentError();
			}
		}

		--m_iRowsRemaining;
		++m_iLine;
		return m_dFields.Begin();
	}

	bool SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) final
	{
		return SetupSVpipeSchema ( hSource, bWordDict, m_tSchema, sError );
	}

private:
	bool EnsureBinaryBuffer ( int iAdditional, CSphString & sError )
	{
		if ( iAdditional<0 || m_iBufUsed>INT_MAX-iAdditional )
		{
			sError.SetSprintf ( "source '%s': binary field buffer is too large", m_tSchema.GetName() );
			return false;
		}

		const int iRequired = m_iBufUsed + iAdditional;
		if ( iRequired<=m_dBuf.GetLength() )
			return true;

		const BYTE * pOld = m_dBuf.Begin();
		CSphVector<int> dFieldOffsets ( m_dFields.GetLength() );
		ARRAY_FOREACH ( i, m_dFields )
			dFieldOffsets[i] = m_dFields[i] ? (int)( m_dFields[i]-pOld ) : -1;

		int iNewLength = m_dBuf.GetLength();
		if ( !iNewLength )
			iNewLength = DEFAULT_READ_BUFFER;
		while ( iNewLength<iRequired )
		{
			if ( iNewLength>INT_MAX/2 )
			{
				iNewLength = iRequired;
				break;
			}
			iNewLength *= 2;
		}
		m_dBuf.Resize ( iNewLength );
		BYTE * pNew = m_dBuf.Begin();
		ARRAY_FOREACH ( i, m_dFields )
			if ( dFieldOffsets[i]>=0 )
				m_dFields[i] = pNew + dFieldOffsets[i];
		return true;
	}


	bool AppendBinary ( const void * pData, int iLength, bool bTerminate, CSphString & sError )
	{
		if ( iLength<0 || ( iLength && !pData ) || ( bTerminate && iLength==INT_MAX ) )
		{
			sError.SetSprintf ( "source '%s': invalid binary field value length", m_tSchema.GetName() );
			return false;
		}
		const int iExtra = bTerminate ? 1 : 0;
		if ( !EnsureBinaryBuffer ( iLength+iExtra, sError ) )
			return false;
		if ( iLength )
			memcpy ( m_dBuf.Begin()+m_iBufUsed, pData, iLength );
		m_iBufUsed += iLength;
		if ( bTerminate )
			m_dBuf[m_iBufUsed++] = '\0';
		return true;
	}


	bool StoreBinaryField ( int iRemap, const char * pData, int iLength, CSphString & sError )
	{
		const RemapXSV_t & tRemap = m_dRemap[iRemap];
		if ( tRemap.m_iField==-1 )
			return true;
		const int iStart = m_iBufUsed;
		if ( !AppendBinary ( pData, iLength, true, sError ) )
			return false;
		m_dFields[tRemap.m_iField] = m_dBuf.Begin()+iStart;
		m_dFieldLengths[tRemap.m_iField] = iLength;
		return true;
	}


	bool StoreBinaryTextValue ( int iRemap, const char * pData, int iLength, CSphString & sError )
	{
		const int iStart = m_iBufUsed;
		if ( !AppendBinary ( pData, iLength, true, sError ) )
			return false;
		if ( !StoreAttribute ( iRemap, iStart ) )
		{
			if ( sError.IsEmpty() && m_dError.Begin()[0] )
				sError = m_dError.Begin();
			return false;
		}
		return true;
	}


	bool FinishBinaryTextValue ( int iRemap, int iStart, CSphString & sError )
	{
		if ( !AppendBinary ( nullptr, 0, true, sError ) )
			return false;
		if ( !StoreAttribute ( iRemap, iStart ) )
		{
			if ( sError.IsEmpty() && m_dError.Begin()[0] )
				sError = m_dError.Begin();
			return false;
		}
		return true;
	}


	bool StoreBinaryString ( int iRemap, const char * pData, int iLength, CSphString & sError )
	{
		const RemapXSV_t & tRemap = m_dRemap[iRemap];
		if ( tRemap.m_iAttr==-1 )
			return StoreBinaryField ( iRemap, pData, iLength, sError );

		const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( tRemap.m_iAttr );
		if ( tAttr.m_eAttrType==SPH_ATTR_STRING || tAttr.m_eAttrType==SPH_ATTR_JSON )
		{
			if ( !StoreBinaryField ( iRemap, pData, iLength, sError ) )
				return false;
			m_dStrAttrs[tRemap.m_iAttr].SetBinary ( pData, iLength );
			return true;
		}

		return StoreBinaryTextValue ( iRemap, pData, iLength, sError );
	}


	bool StoreBinaryInteger ( int iRemap, uint64_t uValue, bool bNegative, CSphString & sError )
	{
		const RemapXSV_t & tRemap = m_dRemap[iRemap];
		const int64_t iSignedValue = GetSignedValue ( uValue, bNegative );
		const bool bDocid = tRemap.m_iAttr==0;
		const CSphColumnInfo * pAttr = tRemap.m_iAttr==-1 ? nullptr : &m_tSchema.GetAttr ( tRemap.m_iAttr );
		if ( tRemap.m_iField!=-1 || ( pAttr && ( pAttr->m_eAttrType==SPH_ATTR_STRING || pAttr->m_eAttrType==SPH_ATTR_JSON ) ) )
		{
			CSphString sValue;
			sValue.SetSprintf ( INT64_FMT, bDocid ? (int64_t)uValue : iSignedValue );
			if ( !StoreBinaryField ( iRemap, sValue.cstr(), sValue.Length(), sError ) )
				return false;
			if ( pAttr )
			{
				m_dStrAttrs[tRemap.m_iAttr].SetBinary ( sValue.cstr(), sValue.Length() );
				return true;
			}
		}

		if ( !pAttr )
			return true;

		SphAttr_t & tCurIntAttr = m_dAttrs[tRemap.m_iAttr];
		switch ( pAttr->m_eAttrType )
		{
		case SPH_ATTR_FLOAT:
			tCurIntAttr = sphF2DW ( (float)iSignedValue );
			if ( !pAttr->IsColumnar() )
				m_tDocInfo.SetAttrFloat ( pAttr->m_tLocator, (float)iSignedValue );
			break;
		case SPH_ATTR_BIGINT:
			tCurIntAttr = bDocid ? (int64_t)uValue : iSignedValue;
			if ( !pAttr->IsColumnar() )
				m_tDocInfo.SetAttr ( pAttr->m_tLocator, tCurIntAttr );
			break;
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
			m_dMvas[tRemap.m_iAttr].Add ( iSignedValue );
			break;
		case SPH_ATTR_FLOAT_VECTOR:
			m_dMvas[tRemap.m_iAttr].Add ( sphF2DW ( (float)iSignedValue ) );
			break;
		case SPH_ATTR_TOKENCOUNT:
			m_tDocInfo.SetAttr ( pAttr->m_tLocator, 0 );
			break;
		case SPH_ATTR_BOOL:
			tCurIntAttr = (DWORD)iSignedValue ? 1 : 0;
			if ( !pAttr->IsColumnar() )
				m_tDocInfo.SetAttr ( pAttr->m_tLocator, tCurIntAttr );
			break;
		default:
			tCurIntAttr = (DWORD)iSignedValue;
			if ( !pAttr->IsColumnar() )
				m_tDocInfo.SetAttr ( pAttr->m_tLocator, tCurIntAttr );
			break;
		}
		return true;
	}


	bool StoreBinaryFloat ( int iRemap, float fValue, CSphString & sError )
	{
		const RemapXSV_t & tRemap = m_dRemap[iRemap];
		const CSphColumnInfo * pAttr = tRemap.m_iAttr==-1 ? nullptr : &m_tSchema.GetAttr ( tRemap.m_iAttr );
		int iTextStart = -1;
		if ( tRemap.m_iField!=-1 || ( pAttr && ( pAttr->m_eAttrType==SPH_ATTR_STRING || pAttr->m_eAttrType==SPH_ATTR_JSON ) ) )
		{
			CSphString sValue;
			sValue.SetSprintf ( "%.9g", fValue );
			iTextStart = m_iBufUsed;
			if ( !StoreBinaryField ( iRemap, sValue.cstr(), sValue.Length(), sError ) )
				return false;
			if ( pAttr && ( pAttr->m_eAttrType==SPH_ATTR_STRING || pAttr->m_eAttrType==SPH_ATTR_JSON ) )
			{
				m_dStrAttrs[tRemap.m_iAttr].SetBinary ( sValue.cstr(), sValue.Length() );
				return true;
			}
		}

		if ( !pAttr )
			return true;
		SphAttr_t & tCurIntAttr = m_dAttrs[tRemap.m_iAttr];
		switch ( pAttr->m_eAttrType )
		{
		case SPH_ATTR_FLOAT:
			tCurIntAttr = sphF2DW ( fValue );
			if ( !pAttr->IsColumnar() )
				m_tDocInfo.SetAttrFloat ( pAttr->m_tLocator, fValue );
			break;
		case SPH_ATTR_FLOAT_VECTOR:
			m_dMvas[tRemap.m_iAttr].Add ( sphF2DW ( fValue ) );
			break;
		case SPH_ATTR_TOKENCOUNT:
			m_tDocInfo.SetAttr ( pAttr->m_tLocator, 0 );
			break;
		default:
			if ( iTextStart>=0 )
				return StoreAttribute ( iRemap, iTextStart );
			CSphString sValue;
			sValue.SetSprintf ( "%.9g", fValue );
			return StoreBinaryTextValue ( iRemap, sValue.cstr(), sValue.Length(), sError );
		}
		return true;
	}


	bool StoreBinaryMvaInteger ( int iRemap, int64_t iValue, CSphString & )
	{
		const RemapXSV_t & tRemap = m_dRemap[iRemap];
		if ( tRemap.m_iAttr==-1 )
			return true;
		const ESphAttr eType = m_tSchema.GetAttr ( tRemap.m_iAttr ).m_eAttrType;
		if ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET )
			m_dMvas[tRemap.m_iAttr].Add ( iValue );
		else if ( eType==SPH_ATTR_FLOAT_VECTOR )
			m_dMvas[tRemap.m_iAttr].Add ( sphF2DW ( (float)iValue ) );
		return true;
	}


	bool StoreBinaryMvaFloat ( int iRemap, float fValue, CSphString & sError )
	{
		const RemapXSV_t & tRemap = m_dRemap[iRemap];
		if ( tRemap.m_iAttr==-1 )
			return true;
		const ESphAttr eType = m_tSchema.GetAttr ( tRemap.m_iAttr ).m_eAttrType;
		if ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET )
			m_dMvas[tRemap.m_iAttr].Add ( (int64_t)fValue );
		else if ( eType==SPH_ATTR_FLOAT_VECTOR )
		{
			m_dMvas[tRemap.m_iAttr].Add ( sphF2DW ( fValue ) );
		}
		return true;
	}

	SourceBinaryReader_c m_tReader;
	uint32_t m_iRowsRemaining = 0;
};


CSphSource * sphCreateSourceTSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName )
{
	CSphString sError;
	auto * pTSV = new CSphSource_TSV(sSourceName);
	if ( !pTSV->SetupPipe ( *pSource, pPipe, true, sError ) )
	{
		SafeDelete ( pTSV );
		fprintf ( stdout, "ERROR: tsvpipe: %s", sError.cstr() );
	}

	return pTSV;
}


CSphSource * sphCreateSourceCSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName, bool bOwnPipe )
{
	CSphString sError;
	auto sDelimiter = pSource->GetStr ( "csvpipe_delimiter" );
	auto * pCSV = new CSphSource_CSV(sSourceName);
	pCSV->SetDelimiter ( sDelimiter.cstr() );
	if ( !pCSV->SetupPipe ( *pSource, pPipe, bOwnPipe, sError ) )
	{
		SafeDelete ( pCSV );
		fprintf ( stdout, "ERROR: csvpipe: %s", sError.cstr() );
	}

	return pCSV;
}


CSphSource * sphCreateSourceBinarypipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName, bool bOwnPipe )
{
	CSphString sError;
	auto * pBinary = new CSphSource_Binary ( sSourceName );
	if ( !pBinary->SetupPipe ( *pSource, pPipe, bOwnPipe, sError ) )
	{
		SafeDelete ( pBinary );
		fprintf ( stdout, "ERROR: binarypipe: %s", sError.cstr() );
	}

	return pBinary;
}


CSphSource_BaseSV::CSphSource_BaseSV ( const char * sName )
	: CSphSource ( sName )
	{}



CSphSource_BaseSV::~CSphSource_BaseSV ()
{
	Disconnect();
}


bool CSphSource_BaseSV::SetupPipe ( const CSphConfigSection & hSource, FILE * pPipe, bool bOwnPipe, CSphString & sError )
{
	m_pFP = pPipe;
	m_bOwnPipe = bOwnPipe;
	m_tSchema.Reset ();
	bool bWordDict = ( m_pDict && m_pDict->GetSettings().IsWordDict() );

	if ( !SetupSchema ( hSource, bWordDict, sError ) )
		return false;

	if ( !DebugCheckSchema ( m_tSchema, sError ) )
		return false;

	if ( !AddAutoAttrs ( sError ) )
		return false;

	int nFields = m_tSchema.GetFieldsCount();
	m_dFields.Reset ( nFields );
	m_dFieldLengths.Reset ( nFields );

	// build hash from schema names
	SmallStringHash_T<RemapXSV_t> hSchema;

	for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		RemapXSV_t tField;
		tField.m_iField = i;
		hSchema.Add ( tField, m_tSchema.GetFieldName(i) );
	}

	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphString & sAttrName = m_tSchema.GetAttr(i).m_sName;
		RemapXSV_t * pRemap = hSchema ( sAttrName );
		if ( pRemap )
			pRemap->m_iAttr = i;
		else
		{
			RemapXSV_t tAttr;
			tAttr.m_iAttr = i;
			hSchema.Add ( tAttr, sAttrName );
		}
	}

	// restore order for declared columns
	CSphString sColumn;
	for ( const auto& tVal : hSource )
	{
		if ( tVal.first=="csvpipe_attr_order" )
			continue;

		const CSphVariant * pVal = &tVal.second;
		while ( pVal )
		{
			sColumn = pVal->strval();
			// uint attribute might have bit count that should by cut off from name
			const char * pColon = strchr ( sColumn.cstr(), ':' );
			if ( pColon )
			{
				int iColon = int ( pColon-sColumn.cstr() );
				CSphString sTmp;
				sTmp.SetBinary ( sColumn.cstr(), iColon );
				sColumn.Swap ( sTmp );
			}

			// let's handle different char cases
			sColumn.ToLower();

			RemapXSV_t * pColumn = hSchema ( sColumn );
			assert ( !pColumn || pColumn->m_iAttr>=0 || pColumn->m_iField>=0 );
			assert ( !pColumn || pColumn->m_iTag==-1 );
			if ( pColumn )
				pColumn->m_iTag = pVal->m_iTag;

			pVal = pVal->m_pNext;
		}
	}

	RemapXSV_t * pIdCol = hSchema ( sphGetDocidName() );
	assert ( pIdCol && pIdCol->m_iTag==-1 && pIdCol->m_iAttr==0 );
	pIdCol->m_iTag = 0;

	for ( const auto& tCol : hSchema )
		if ( tCol.second.m_iTag>=0 )
			m_dRemap.Add ( tCol.second );

	m_dColumnsLen.Reset ( m_dRemap.GetLength() );

	sphSort ( m_dRemap.Begin(), m_dRemap.GetLength(), bind ( &RemapXSV_t::m_iTag ) );

	return true;
}


void CSphSource_BaseSV::Setup ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings )
{
	// detect a case when blob row locator was removed (because all blobs became columnar)
	bool bHadLocator = !!m_tSchema.GetAttr ( sphGetBlobLocatorName() );
	BASE::Setup ( tSettings, pWarnings );
	bool bHaveLocator = !!m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	if ( bHadLocator && !bHaveLocator )
	{
		const int iBlobLocatorId = 1;
		for ( auto & i : m_dRemap )
			if ( i.m_iAttr>iBlobLocatorId )
				i.m_iAttr--;
	}
}


bool CSphSource_BaseSV::Connect ( CSphString & sError )
{
	// source settings have been updated after ::Setup
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		ESphWordpart eWordpart = GetWordpart ( m_tSchema.GetFieldName(i), m_pDict && m_pDict->GetSettings().IsWordDict() );
		m_tSchema.SetFieldWordpart ( i, eWordpart );
	}

	if ( !AddAutoAttrs ( sError ) )
		return false;

	AllocDocinfo();

	m_tHits.Reserve ( m_iMaxHits );
	m_dBuf.Resize ( DEFAULT_READ_BUFFER );

	return true;
}


void CSphSource_BaseSV::Disconnect()
{
	if ( m_pFP )
	{
		if ( m_bOwnPipe )
			pclose ( m_pFP );
		m_pFP = nullptr;
		m_bOwnPipe = false;
	}

	m_tHits.Reset();
}


const char * CSphSource_BaseSV::DecorateMessage ( const char * sTemplate, ... ) const
{
	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( m_dError.Begin (), m_dError.GetLength (), sTemplate, ap );
	va_end ( ap );
	return m_dError.Begin();
}

static const BYTE g_dBOM[] = { 0xEF, 0xBB, 0xBF };

bool CSphSource_BaseSV::IterateStart ( CSphString & sError )
{

	m_iLine = 0;
	m_iDataStart = 0;

	// initial buffer update
	m_iBufUsed = (int) fread ( m_dBuf.Begin(), 1, m_dBuf.GetLength(), m_pFP );
	if ( !m_iBufUsed )
	{
		sError.SetSprintf ( "source '%s': read error '%s'", m_tSchema.GetName(), strerrorm(errno) );
		return false;
	}
	m_iPlainFieldsLength = m_tSchema.GetFieldsCount();

	// space out BOM like xml-pipe does
	if ( m_iBufUsed>(int)sizeof(g_dBOM) && memcmp ( m_dBuf.Begin(), g_dBOM, sizeof ( g_dBOM ) )==0 )
		memset ( m_dBuf.Begin(), ' ', sizeof(g_dBOM) );
	return true;
}

BYTE ** CSphSource_BaseSV::ReportDocumentError ()
{
	m_tDocInfo.m_tRowID = 0; // INVALID_ROWID means legal eof
	m_iDataStart = 0;
	m_iBufUsed = 0;
	return nullptr;
}


bool CSphSource_BaseSV::StoreAttribute ( int iAttr, int iOff )
{
	// if+if for field-string attribute case
	const RemapXSV_t & tRemap = m_dRemap[iAttr];

	// field column
	if ( tRemap.m_iField!=-1 )
	{
		m_dFields[tRemap.m_iField] = m_dBuf.Begin() + iOff;
		m_dFieldLengths[tRemap.m_iField] = (int) strlen ( (char *)m_dFields[tRemap.m_iField] );
	}

	// attribute column
	if ( tRemap.m_iAttr==-1 )
		return true;

	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( tRemap.m_iAttr );
	const char * sVal = (const char *)m_dBuf.Begin() + iOff;

	CSphString & sCurStrAttr = m_dStrAttrs[tRemap.m_iAttr];
	SphAttr_t & tCurIntAttr = m_dAttrs[tRemap.m_iAttr];

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_JSON:
		sCurStrAttr = sVal;
		break;

	case SPH_ATTR_FLOAT:
		{
			float fValue = sphToFloat(sVal);
			tCurIntAttr = sphF2DW(fValue);

			if ( !tAttr.IsColumnar() )
				m_tDocInfo.SetAttrFloat ( tAttr.m_tLocator, fValue );
		}
		break;

	case SPH_ATTR_BIGINT:
		{
			CSphString sWarn;
			if ( tRemap.m_iAttr )
			{
				tCurIntAttr = sphToInt64 ( sVal, &sWarn );
				if ( !sWarn.IsEmpty() )
					sphWarn ( "%s", sWarn.cstr() );
			}
			else
			{
				tCurIntAttr = (int64_t)StrToDocID ( sVal, sWarn );
				if ( !sWarn.IsEmpty() )
				{
					sphWarn ( "%s", sWarn.cstr() );
					return false;
				}
			}

			if ( !tAttr.IsColumnar() )
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, tCurIntAttr );
		}
		break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		ParseFieldMVA ( tRemap.m_iAttr, sVal );
		break;

	case SPH_ATTR_FLOAT_VECTOR:
		if ( !ParseFieldFloatVector ( tRemap.m_iAttr, sVal ) )
		{
			DecorateMessage ( "invalid float vector value '%s'", sVal );
			return false;
		}
		break;

	case SPH_ATTR_TOKENCOUNT:
		m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
		break;

	case SPH_ATTR_BOOL:
		tCurIntAttr = sphToDword(sVal) ? 1 : 0;
		if ( !tAttr.IsColumnar() )
			m_tDocInfo.SetAttr ( tAttr.m_tLocator, tCurIntAttr );
		break;

	default:
		tCurIntAttr = sphToDword(sVal);
		if ( !tAttr.IsColumnar() )
			m_tDocInfo.SetAttr ( tAttr.m_tLocator, tCurIntAttr );
		break;
	}

	return true;
}


BYTE **	CSphSource_BaseSV::NextDocument ( bool & bEOF, CSphString & sError )
{
	bEOF = false;

	bool bSkipDoc = false;
	do
	{
		ESphParseResult eRes = SplitColumns ( sError );
		if ( eRes==PARSING_FAILED )
			return ReportDocumentError();
		else if ( eRes==DATA_OVER )
		{
			bEOF = true;
			return nullptr;
		}

		assert ( eRes==GOT_DOCUMENT );

		m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
		for ( auto & i : m_dMvas )
			i.Resize(0);

		int iOff = m_iDocStart;

		bSkipDoc = false;
		ARRAY_FOREACH ( i, m_dRemap )
		{
			if ( !StoreAttribute ( i, iOff ) )
			{
				bSkipDoc = true;
				break;
			}

			iOff += m_dColumnsLen[i] + 1; // length of value plus null-terminator
		}

		m_iLine++;
	}
	while ( bSkipDoc );

	return m_dFields.Begin();
}


CSphSource_BaseSV::ESphParseResult CSphSource_TSV::SplitColumns ( CSphString & sError )
{
	int iColumns = m_dRemap.GetLength();
	int iCol = 0;
	int iColumnStart = m_iDataStart;
	BYTE * pData = m_dBuf.Begin() + m_iDataStart;
	const BYTE * pEnd = m_dBuf.Begin() + m_iBufUsed;
	m_iDocStart = m_iDataStart;

	while (true)
	{
		if ( iCol>=iColumns )
		{
			sError.SetSprintf ( "source '%s': too many columns found (found=%d, declared=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
			return CSphSource_BaseSV::PARSING_FAILED;
		}

		// move to next control symbol
		while ( pData<pEnd && *pData && *pData!='\t' && *pData!='\r' && *pData!='\n' )
			pData++;

		if ( pData<pEnd )
		{
			assert ( *pData=='\t' || !*pData || *pData=='\r' || *pData=='\n' );
			bool bNull = !*pData;
			bool bEOL = ( *pData=='\r' || *pData=='\n' );

			int iLen = int ( pData - m_dBuf.Begin() ) - iColumnStart;
			assert ( iLen>=0 );
			m_dColumnsLen[iCol] = iLen;
			*pData++ = '\0';
			iCol++;

			if ( bNull )
			{
				// null terminated string found
				m_iDataStart = m_iBufUsed = 0;
				break;
			} else if ( bEOL )
			{
				// end of document found
				// skip all EOL characters
				while ( pData<pEnd && *pData && ( *pData=='\r' || *pData=='\n' ) )
					pData++;
				break;
			}

			// column separator found
			iColumnStart = int ( pData - m_dBuf.Begin() );
			continue;
		}

		int iOff = int ( pData - m_dBuf.Begin() );

		// if there is space at the start, move data around
		// if not, resize the buffer
		if ( m_iDataStart>0 )
		{
			memmove ( m_dBuf.Begin(), m_dBuf.Begin() + m_iDataStart, m_iBufUsed - m_iDataStart );
			m_iBufUsed -= m_iDataStart;
			iOff -= m_iDataStart;
			iColumnStart -= m_iDataStart;
			m_iDataStart = 0;
			m_iDocStart = 0;
		} else if ( m_iBufUsed==m_dBuf.GetLength() )
		{
			m_dBuf.Resize ( m_dBuf.GetLength()*2 );
		}

		// do read
		auto iGot = (int) fread ( m_dBuf.Begin() + m_iBufUsed, 1, m_dBuf.GetLength() - m_iBufUsed, m_pFP );
		if ( !iGot )
		{
			if ( !iCol )
			{
				// normal file termination - no pending columns and documents
				m_iDataStart = m_iBufUsed = 0;
				m_tDocInfo.m_tRowID = INVALID_ROWID;
				return CSphSource_BaseSV::DATA_OVER;
			}

			// error in case no data left in middle of data stream
			sError.SetSprintf ( "source '%s': read error '%s' (line=%d)", m_tSchema.GetName(), strerror(errno), m_iLine );
			return CSphSource_BaseSV::PARSING_FAILED;
		}
		m_iBufUsed += iGot;

		// restored pointers after buffer resize
		pData = m_dBuf.Begin() + iOff;
		pEnd = m_dBuf.Begin() + m_iBufUsed;
	}

	// all columns presence check
	if ( iCol!=iColumns )
	{
		sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
		return CSphSource_BaseSV::PARSING_FAILED;
	}

	// tail data
	assert ( pData<=pEnd );
	m_iDataStart = int ( pData - m_dBuf.Begin() );
	return CSphSource_BaseSV::GOT_DOCUMENT;
}


bool CSphSource_TSV::SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError )
{
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_uint"),		SPH_ATTR_INTEGER,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_timestamp"),	SPH_ATTR_TIMESTAMP,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_bool"),		SPH_ATTR_BOOL,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_float"),		SPH_ATTR_FLOAT,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_bigint"),		SPH_ATTR_BIGINT,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_multi"),		SPH_ATTR_UINT32SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_multi_64"),	SPH_ATTR_INT64SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_float_vector"), SPH_ATTR_FLOAT_VECTOR, m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_string"),		SPH_ATTR_STRING,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_json"),		SPH_ATTR_JSON,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_field_string"),	SPH_ATTR_STRING,	m_tSchema, sError );

	if ( !bOk )
		return false;

	bOk &= ConfigureFields ( hSource("tsvpipe_field"), bWordDict, m_tSchema, sError );
	bOk &= ConfigureFields ( hSource("tsvpipe_field_string"), bWordDict, m_tSchema, sError );

	return bOk;
}


CSphSource_CSV::CSphSource_CSV ( const char * sName )
	: CSphSource_BaseSV ( sName )
{
	m_iDelimiter = BYTE ( ',' );
}


CSphSource_BaseSV::ESphParseResult CSphSource_CSV::SplitColumns ( CSphString & sError )
{
	int iColumns = m_dRemap.GetLength();
	int iCol = 0;
	int iColumnStart = m_iDataStart;
	int iQuotPrev = -1;
	int	iEscapeStart = -1;
	const BYTE * s = m_dBuf.Begin() + m_iDataStart; // parse this line
	BYTE * d = m_dBuf.Begin() + m_iDataStart; // do parsing in place
	const BYTE * pEnd = m_dBuf.Begin() + m_iBufUsed; // until we reach the end of current buffer
	m_iDocStart = m_iDataStart;
	bool bOnlySpace = true;
	bool bQuoted = false;
	bool bHasQuot = false;

	while (true)
	{
		assert ( d<=s );

		// move to next control symbol
		while ( s<pEnd && *s && *s!=m_iDelimiter && *s!='"' && *s!='\\' && *s!='\r' && *s!='\n' )
		{
			bOnlySpace &= sphIsSpace ( *s );
			*d++ = *s++;
		}

		if ( s<pEnd )
		{
			assert ( !*s || *s==m_iDelimiter || *s=='"' || *s=='\\' || *s=='\r' || *s=='\n' );
			bool bNull = !*s;
			bool bEOL = ( *s=='\r' || *s=='\n' );
			bool bDelimiter = ( *s==m_iDelimiter );
			bool bQuot = ( *s=='"' );
			bool bEscape = ( *s=='\\' );
			int iOff = int ( s - m_dBuf.Begin() );
			bool bEscaped = ( iEscapeStart>=0 && iEscapeStart+1==iOff );

			// escape symbol outside double quotation
			if ( !bQuoted && !bDelimiter && ( bEscape || bEscaped ) )
			{
				if ( bEscaped ) // next to escape symbol proceed as regular
				{
					*d++ = *s++;
				} else // escape just started
				{
					iEscapeStart = iOff;
					s++;
				}
				continue;
			}

			// double quote processing
			// [ " ... " ]
			// [ " ... "" ... " ]
			// [ " ... """ ]
			// [ " ... """" ... " ]
			// any symbol inside double quote proceed as regular
			// but quoted quote proceed as regular symbol
			if ( bQuot )
			{
				if ( bOnlySpace && iQuotPrev==-1 )
				{
					// enable double quote
					bQuoted = true;
					bHasQuot = true;
				} else if ( bQuoted )
				{
					// close double quote on 2st quote symbol
					bQuoted = false;
				} else if ( bHasQuot && iQuotPrev!=-1 && iQuotPrev+1==iOff )
				{
					// escaped quote found, re-enable double quote and copy symbol itself
					bQuoted = true;
					*d++ = '"';
				} else
				{
					*d++ = *s;
				}

				s++;
				iQuotPrev = iOff;
				continue;
			}

			if ( bQuoted )
			{
				*d++ = *s++;
				continue;
			}

			int iLen = int ( d - m_dBuf.Begin() - iColumnStart );
			assert ( iLen>=0 );
			if ( iCol<m_dColumnsLen.GetLength() )
				m_dColumnsLen[iCol] = iLen;
			*d++ = '\0';
			s++;
			iCol++;

			if ( bNull ) // null terminated string found
			{
				m_iDataStart = m_iBufUsed = 0;
				break;
			} else if ( bEOL ) // end of document found
			{
				// skip all EOL characters
				while ( s<pEnd && *s && ( *s=='\r' || *s=='\n' ) )
					s++;
				break;
			}

			assert ( bDelimiter );
			// column separator found
			iColumnStart = int ( d - m_dBuf.Begin() );
			bOnlySpace = true;
			bQuoted = false;
			bHasQuot = false;
			iQuotPrev = -1;
			continue;
		}

		/////////////////////
		// read in more data
		/////////////////////

		int iDstOff = int ( s - m_dBuf.Begin() );
		int iSrcOff = int ( d - m_dBuf.Begin() );

		// if there is space at the start, move data around
		// if not, resize the buffer
		if ( m_iDataStart>0 )
		{
			memmove ( m_dBuf.Begin(), m_dBuf.Begin() + m_iDataStart, m_iBufUsed - m_iDataStart );
			m_iBufUsed -= m_iDataStart;
			iDstOff -= m_iDataStart;
			iSrcOff -= m_iDataStart;
			iColumnStart -= m_iDataStart;
			if ( iQuotPrev!=-1 )
				iQuotPrev -= m_iDataStart;
			iEscapeStart -= m_iDataStart;
			m_iDataStart = 0;
			m_iDocStart = 0;
		} else if ( m_iBufUsed==m_dBuf.GetLength() )
		{
			m_dBuf.Resize ( m_dBuf.GetLength()*2 );
		}

		// do read
		auto iGot = (int) fread ( m_dBuf.Begin() + m_iBufUsed, 1, m_dBuf.GetLength() - m_iBufUsed, m_pFP );
		if ( !iGot )
		{
			if ( !iCol )
			{
				// normal file termination - no pending columns and documents
				m_iDataStart = m_iBufUsed = 0;
				m_tDocInfo.m_tRowID = INVALID_ROWID;
				return CSphSource_BaseSV::DATA_OVER;
			}

			if ( iCol!=iColumns )
				sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d, error='%s')",	m_tSchema.GetName(), iCol, iColumns, m_iLine, strerror(errno) );
			else
			{
				// error in case no data left in middle of data stream
				sError.SetSprintf ( "source '%s': read error '%s' (line=%d)", m_tSchema.GetName(), strerror(errno), m_iLine );
			}
			return CSphSource_BaseSV::PARSING_FAILED;
		}
		m_iBufUsed += iGot;

		// restore pointers because of the resize
		s = m_dBuf.Begin() + iDstOff;
		d = m_dBuf.Begin() + iSrcOff;
		pEnd = m_dBuf.Begin() + m_iBufUsed;

		// skip all EOL characters left from previous row
		if ( !iCol )
		{
			while ( s<pEnd && *s && ( *s=='\r' || *s=='\n' ) )
				s++;
		}
	}

	// all columns presence check
	if ( iCol!=iColumns )
	{
		sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
		return CSphSource_BaseSV::PARSING_FAILED;
	}

	// tail data
	assert ( s<=pEnd );
	m_iDataStart = int ( s - m_dBuf.Begin() );
	return CSphSource_BaseSV::GOT_DOCUMENT;
}


bool CSphSource_CSV::SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError )
{
	return SetupSVpipeSchema ( hSource, bWordDict, m_tSchema, sError );
}


void CSphSource_CSV::SetDelimiter ( const char * sDelimiter )
{
	if ( sDelimiter && *sDelimiter )
		m_iDelimiter = *sDelimiter;
}
