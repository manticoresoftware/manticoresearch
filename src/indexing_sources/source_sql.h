//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "source_document.h"
#include "sphinxstd.h"
#include "tokenizer/tokenizer.h"
#include "sphinxdefs.h"


/// column unpack format
enum ESphUnpackFormat
{
	SPH_UNPACK_NONE				= 0,
	SPH_UNPACK_ZLIB				= 1,
	SPH_UNPACK_MYSQL_COMPRESS	= 2
};

using SqlEscapedBuilder_c = EscapedStringBuilder_T<BaseQuotation_T<SqlQuotator_t>>;

struct CSphUnpackInfo
{
	ESphUnpackFormat	m_eFormat;
	CSphString			m_sName;
};

struct CSphJoinedField
{
	CSphString			m_sName;
	CSphString			m_sQuery;
	CSphString			m_sRanged;
	bool				m_bPayload;
	bool				m_bRangedMain;
};

/// generic SQL source params
struct CSphSourceParams_SQL
{
	// query params
	CSphString						m_sQuery;
	CSphString						m_sQueryRange;
	CSphString						m_sQueryKilllist;
	int64_t							m_iRangeStep = 1024;
	int64_t							m_iRefRangeStep = 1024;
	bool							m_bPrintQueries = false;
	bool							m_bPrintRTQueries = false;
	CSphString						m_sDumpRTIndex;

	StrVec_t						m_dQueryPre;
	StrVec_t						m_dQueryPreAll;
	StrVec_t						m_dQueryPost;
	StrVec_t						m_dQueryPostIndex;
	CSphVector<CSphColumnInfo>		m_dAttrs;
	StrVec_t						m_dFileFields;

	int								m_iRangedThrottleMs = 0;
	int								m_iMaxFileBufferSize = 0;
	ESphOnFileFieldError			m_eOnFileFieldError {FFE_IGNORE_FIELD};

	CSphVector<CSphUnpackInfo>		m_dUnpack;
	DWORD							m_uUnpackMemoryLimit = 0;

	CSphVector<CSphJoinedField>		m_dJoinedFields;

	// connection params
	CSphString						m_sHost;
	CSphString						m_sUser;
	CSphString						m_sPass;
	CSphString						m_sDB;
	unsigned int					m_uPort = 0;

	// hooks
	CSphString						m_sHookConnect;
	CSphString						m_sHookQueryRange;
	CSphString						m_sHookPostIndex;
};

/// generic SQL source
/// multi-field plain-text documents fetched from given query
struct CSphSource_SQL : CSphSource
{
	explicit			CSphSource_SQL ( const char * sName );
						~CSphSource_SQL () override = default;

	bool				SetupSQL ( const CSphSourceParams_SQL & pParams );
	bool				Connect ( CSphString & sError ) override;
	void				Disconnect () override;
	bool				IterateStart ( CSphString & sError ) override;
	BYTE **				NextDocument ( bool & bEOF, CSphString & sError ) override;
	const int *			GetFieldLengths () const override;
	void				PostIndex () override;

	ISphHits *			IterateJoinedHits ( CSphReader & tReader, CSphString & sError ) override;
	bool				FetchJoinedFields ( CSphAutofile & tFile, CSphVector<std::unique_ptr<OpenHashTable_T<uint64_t, uint64_t>>> & dJoinedOffsets, CSphString & sError ) override;

	bool				IterateMultivaluedStart ( int iAttr, CSphString & sError ) override;
	bool				IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue ) override;

	bool				IterateKillListStart ( CSphString & sError ) override;
	bool				IterateKillListNext ( DocID_t & tDocId ) override;


protected:
	static const int			MACRO_COUNT = 2;
	static const char * const	MACRO_VALUES [ MACRO_COUNT ];

	/// by what reason the internal SetupRanges called
	enum ERangesReason
	{
		SRE_DOCS,
		SRE_MVA,
		SRE_JOINEDHITS
	};

	CSphString			m_sSqlDSN;

	BYTE *				m_dFields [ SPH_MAX_FIELDS ] { nullptr };
	int					m_dFieldLengths [ SPH_MAX_FIELDS ];
	ESphUnpackFormat	m_dUnpack [ SPH_MAX_FIELDS ] { SPH_UNPACK_NONE };

	DocID_t				m_tMinID = 0;			///< grand min ID
	DocID_t				m_tMaxID = 0;			///< grand max ID
	DocID_t				m_tCurrentID = 0;		///< current min ID
	DocID_t				m_tMaxFetchedID = 0;	///< max actually fetched ID
	int					m_iMultiAttr = -1;		///< multi-valued attr being currently fetched
	int					m_iSqlFields = 0;		///< field count (for row dumper)

	CSphSourceParams_SQL m_tParams;

	bool				m_bCanUnpack = false;
	bool				m_bUnpackFailed = false;
	bool				m_bUnpackOverflow = false;
	CSphVector<char>	m_dUnpackBuffers [ SPH_MAX_FIELDS ];

	int					m_iJoinedHitField = -1;	///< currently pulling joined hits from this field (index into schema; -1 if not pulling)
	DocID_t				m_iJoinedHitID = 0;		///< last document id
	int					m_iJoinedHitPos = 0;	///< last hit position
	CSphVector<BYTE>	m_dJoinedField;
	BYTE *				m_pJoinedFields = nullptr;
	SphOffset_t			m_iJoinedFileSize = 0;

	using TinyCol_t = std::pair<int,bool>; // int idx in sql resultset; bool whether it is string
	CSphVector<TinyCol_t>	m_dDumpMap;
	SqlEscapedBuilder_c		m_sCollectDump;
	int 					m_iCutoutDumpSize = 100*1024;

protected:
	bool					SetupRanges ( const char * sRangeQuery, const char * sQuery, const char * sPrefix, CSphString & sError, ERangesReason iReason );
	bool					RunQueryStep ( const char * sQuery, CSphString & sError );

	virtual void			SqlDismissResult () = 0;
	virtual bool			SqlQuery ( const char * sQuery ) = 0;
	virtual bool			SqlIsError () = 0;
	virtual const char *	SqlError () = 0;
	virtual bool			SqlConnect () = 0;
	virtual void			SqlDisconnect () = 0;
	virtual int				SqlNumFields() = 0;
	virtual bool			SqlFetchRow() = 0;
	virtual DWORD			SqlColumnLength ( int iIndex ) = 0;
	virtual const char *	SqlColumn ( int iIndex ) = 0;
	virtual const char *	SqlFieldName ( int iIndex ) = 0;

	virtual Str_t			SqlCompressedColumnStream ( int iFieldIndex );
	virtual void			SqlCompressedColumnReleaseStream ( Str_t tStream );

	Str_t					SqlColumnStream ( int iFieldIndex );
	Str_t					SqlUnpackColumn ( int iFieldIndex, ESphUnpackFormat eFormat );
	void					ReportUnpackError ( int iIndex, int iError );

	void					DumpRowsHeader();
	void					DumpRowsHeaderSphinxql();

	void					DumpDocument();
	void					DumpDocumentSphinxql();

	bool 					QueryPreAll ( CSphString& sError) ;

private:
	bool					m_bSqlConnected = false;	///< am i connected?

	bool					StoreAttribute ( int iAttr );
};
