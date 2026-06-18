#pragma once

#include "searchdsql.h"
#include "searchdha.h"

struct ShardSpanRef_t
{
	int		m_iOff = -1;
	int		m_iLen = 0;
};

struct ShardMvaRef_t
{
	int		m_iOff = -1;
	int		m_iCount = 0;
};

struct ShardBufferedRow_t
{
	int				m_iAttrRowOff = -1;
	int				m_iColumnarOff = -1;
	ShardMvaRef_t	m_tMva;
	int				m_iFieldRefOff = -1;
	int				m_iStringRefOff = -1;
	bool			m_bReplace = false;
	int				m_iTxnOrdinal = -1;
};

struct ShardDocStorage_t
{
	CSphVector<BYTE>					m_dAttrRows;
	CSphVector<SphAttr_t>				m_dColumnar;
	CSphVector<int64_t>					m_dMvaPool;
	CSphVector<ShardSpanRef_t>			m_dFieldRefs;
	CSphVector<ShardSpanRef_t>			m_dStringRefs;
	CSphVector<BYTE>					m_dBlobPool;
	CSphVector<ShardBufferedRow_t>		m_dRows;

	void Cleanup();
};

struct ShardDeletePredicate_t
{
	CSphString							m_sQuery;
	CSphVector<CSphFilterSettings>		m_dFilters;
	CSphVector<FilterTreeItem_t>		m_dFilterTree;
	QueryType_e							m_eQueryType { QUERY_SQL };
};

enum class ShardTxnOp_e : BYTE
{
	ROW_WRITE = 0,
	DOC_DELETE,
	QUERY_DELETE
};

struct ShardBatchOp_t
{
	ShardTxnOp_e	m_eOp { ShardTxnOp_e::ROW_WRITE };
	int				m_iData = -1;
	int				m_iCount = 1;
};

struct ShardTargetBatch_t
{
	CSphString				m_sIndex;
	CSphString				m_sCluster;
	MultiAgentDescRefPtr_c	m_pAgent;
	AgentDesc_t				m_tPinnedAgent;
	int						m_iRouteId = -1;
	bool					m_bLocal = true;
	bool					m_bHasPinnedAgent = false;
	bool					m_bCommitted = false;
	CSphVector<int>			m_dRowIds;
	CSphVector<int64_t>		m_dDeleteDocids;
	CSphVector<ShardBatchOp_t> m_dOps;
};

struct ShardTxnOrderRef_t
{
	int				m_iTarget = -1;
	ShardTxnOp_e	m_eOp { ShardTxnOp_e::ROW_WRITE };
	int				m_iData = -1;
	int				m_iCount = 1;
};

struct ShardTxnState_t
{
	CSphString						m_sShard;
	const ShardIndex_c *			m_pShardInstance = nullptr;
	SqlStmt_e						m_eWriteStmt { STMT_DUMMY };
	ShardDocStorage_t				m_tStorage;
	CSphVector<ShardDeletePredicate_t> m_dDeletePredicates;
	CSphVector<ShardTargetBatch_t>	m_dTargets;
	CSphVector<ShardTxnOrderRef_t>	m_dOrder;

	bool HasPendingData() const;
	void Cleanup();
};

class ClientSession_c;

bool IsHttpStmt ( const SqlStmt_t & tStmt );
bool CreateAttrMaps ( CSphVector<int> & dAttrSchema, CSphVector<int> & dFieldSchema, CSphVector<bool> & dFieldAttrs, const CSphSchema & tSchema, const StrVec_t & dStmtInsertSchema, StmtErrorReporter_i & tOut );
bool ConfigureRTPercolate ( CSphSchema & tSchema, CSphIndexSettings & tSettings, const char * szIndexName, const CSphConfigSection & hIndex, bool bWordDict, bool bPercolate, StrVec_t * pWarnings, CSphString & sError );

struct StringPtrTraits_t
{
	CSphVector<BYTE> m_dPackedData;
	CSphFixedVector<int> m_dOff { 0 };
	CSphVector<BYTE> m_dParserBuf;

	// remap offsets to string pointers
	void SavePointersTo ( VecTraits_T<const char *> & dStrings, bool bSkipInvalid=true ) const;
	void Reset ();
	BYTE * ReserveBlob ( int iBlobSize, int iOffset );
};

class AttributeConverter_c : public InsertDocData_c
{
public:
				AttributeConverter_c ( const CSphSchema & tSchema, const CSphVector<bool> & dFieldAttrs, CSphString & sError, CSphString & sWarning );

	bool		SetAttrValue ( int iCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx, CSphString & sError );
	void		SetDefaultAttrValue ( int iCol );

	bool		SetFieldValue ( int iField, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx );
	void		SetDefaultFieldValue ( int iField );

	void		NewRow();
	void		Finalize();

private:
	const CSphSchema &		m_tSchema;
	const CSphColumnInfo *	m_pDocId = nullptr;

	const CSphVector<bool> & m_dFieldAttrs;

	StringPtrTraits_t 		m_tStrings;
	StrVec_t				m_dTmpFieldStorage;
	CSphVector<float>		m_dTmpFloats;
	CSphVector<int>			m_dColumnarRemap;

	CSphString &			m_sError;
	CSphString &			m_sWarning;

	bool		String2JsonPack ( char * pStr, CSphVector<BYTE> & dBuf );

	bool		CheckStrings ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iCol, int iRow );
	bool		CheckJson ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal );
	bool		CheckMVA ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iCol, int iRow );
	bool		CheckInsertTypes ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx );
};

ResultAndIndex_t AddShardIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bLoadMeta, CSphString & sError, StrVec_t * pWarnings=nullptr );

bool EnsureNoPendingShardTxn ( const ClientSession_c & tSession, CSphString & sError );
void RollbackShardTxn ( ShardTxnState_t & tShardTxn );
bool CommitShardTxn ( ClientSession_c & tSession, CSphString & sError, CSphVector<int64_t> * pCommittedDocIDs = nullptr, int * pDeletedCount = nullptr );
void HandleCommandShardWrite ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );
bool AddDocumentShard ( const SqlStmt_t & tStmt, const ShardIndex_c & tShard, StmtErrorReporter_i & tOut, int & iAffectedRows );
bool DeleteDocumentShard ( const SqlStmt_t & tStmt, const ShardIndex_c & tShard, StmtErrorReporter_i & tOut, int & iAffectedRows );
