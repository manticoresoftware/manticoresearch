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

#include "sphinxstd.h"
#include "indexsettings.h"
#include "sphinxdefs.h"
#include "source_stats.h"
#include "match.h"
#include "schema/schema.h"
#include "std/openhash.h"

class CSphHTMLStripper;
class CSphDict;
using DictRefPtr_c = CSphRefcountedPtr<CSphDict>;
class ISphFieldFilter;
class ISphTokenizer;
class CSphAutofile;
using TokenizerRefPtr_c = CSphRefcountedPtr<ISphTokenizer>;

/// how to handle IO errors in file fields
enum ESphOnFileFieldError
{
	FFE_IGNORE_FIELD,
	FFE_SKIP_DOCUMENT,
	FFE_FAIL_INDEX
};

struct IDPair_t
{
	DocID_t m_tDocID;
	RowID_t m_tRowID;
};

constexpr int MAX_SOURCE_HITS = 32768;


class AttrSource_i
{
public:
	virtual							~AttrSource_i () {}

	/// returns value of a given attribute
	virtual SphAttr_t 				GetAttr ( int iAttr ) = 0;

	/// returns mva values for a given attribute (mva must be stored in a field)
	virtual CSphVector<int64_t> *	GetFieldMVA ( int iAttr ) = 0;

	/// returns string attributes for a given attribute
	virtual const CSphString &		GetStrAttr ( int iAttr ) = 0;
};

/// generic document source
/// provides multi-field support and generic tokenizer
class CSphSource : public CSphSourceSettings, public AttrSource_i
{
public:
	CSphMatch				m_tDocInfo;		///< current document info

	/// ctor
	explicit				CSphSource ( const char * sName );

	/// dtor
							~CSphSource () override;

	/// set dictionary
	void					SetDict ( const DictRefPtr_c& dict );

	/// set HTML stripping mode
	///
	/// sExtractAttrs defines what attributes to store. format is "img=alt; a=alt,title".
	/// empty string means to strip all tags; NULL means to disable stripping.
	///
	/// sRemoveElements defines what elements to cleanup. format is "style, script"
	///
	/// on failure, returns false and fills sError
	bool					SetStripHTML ( const char * sExtractAttrs, const char * sRemoveElements, bool bDetectParagraphs, const char * sZones, CSphString & sError );

	/// set field filter
	virtual void			SetFieldFilter ( std::unique_ptr<ISphFieldFilter> pFilter );

	/// set tokenizer
	void					SetTokenizer ( TokenizerRefPtr_c pTokenizer );

	/// set rows dump file
	virtual void			SetDumpRows ( FILE * fpDumpRows ) { m_fpDumpRows = fpDumpRows; }

	/// get stats
	virtual const CSphSourceStats &		GetStats ();

	/// updates schema fields and attributes
	/// updates pInfo if it's empty; checks for match if it's not
	/// must be called after IterateStart(); will always fail otherwise
	virtual bool			UpdateSchema ( CSphSchema * pInfo, CSphString & sError );

	/// setup misc indexing settings (prefix/infix/exact-word indexing, position steps)
	virtual void			Setup ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings );

	bool					SetupMorphFields ( CSphString & sError );

	/// connect to the source (eg. to the database)
	/// connection settings are specific for each source type and as such
	/// are implemented in specific descendants
	virtual bool			Connect ( CSphString & sError ) = 0;

	/// disconnect from the source
	virtual void			Disconnect () = 0;

	/// check if there are any joined fields
	virtual bool			HasJoinedFields () { return m_iPlainFieldsLength!=m_tSchema.GetFieldsCount(); }

	/// begin indexing this source
	/// to be implemented by descendants
	virtual bool			IterateStart ( CSphString & sError ) = 0;

	/// get next document
	/// to be implemented by descendants
	/// returns false on error
	/// returns true and sets bEOF
	virtual bool			IterateDocument ( bool & bEOF, CSphString & sError );

	/// get next hits chunk for current document
	/// to be implemented by descendants
	/// returns NULL when there are no more hits
	/// returns pointer to hit vector (with at most MAX_SOURCE_HITS) on success
	/// fills out-string with error message on failure
	virtual ISphHits *		IterateHits ( CSphString & sError );

	/// get joined hits from joined fields (w/o attached docinfos)
	/// returns false and fills out-string with error message on failure
	/// returns true and sets m_tDocInfo.m_uDocID to 0 on eof
	/// returns true and sets m_tDocInfo.m_uDocID to non-0 on success
	virtual ISphHits *		IterateJoinedHits ( CSphReader & tReader, CSphString & sError );

	/// get fields and store them in a file
	/// dJoinedOffsets holds per-field offsets to data in that file
	virtual bool			FetchJoinedFields ( CSphAutofile & tFile, CSphVector<std::unique_ptr<OpenHash_T<uint64_t, uint64_t>>> & dJoinedOffsets, CSphString & sError ) { return false; }

	/// begin iterating values of out-of-document multi-valued attribute iAttr
	/// will fail if iAttr is out of range, or is not multi-valued
	/// can also fail if configured settings are invalid (eg. SQL query can not be executed)
	virtual bool			IterateMultivaluedStart ( int iAttr, CSphString & sError ) = 0;

	/// fetch next multi-valued attribute value
	virtual bool			IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue ) = 0;

	/// notification that a rowid was assigned to a specific docid (used by joined fields)
	virtual void			RowIDAssigned ( DocID_t tDocID, RowID_t tRowID );

	/// fetch fields for a current document
	virtual void			GetDocFields ( CSphVector<VecTraits_T<BYTE>> & dFields );

	/// begin iterating kill list
	virtual bool			IterateKillListStart ( CSphString & sError ) = 0;

	/// get next kill list doc id
	virtual bool			IterateKillListNext ( DocID_t & uDocId ) = 0;

	/// post-index callback
	/// gets called when the indexing is succesfully (!) over
	virtual void			PostIndex () {}

	// returns false when there are no more documents
	void					BuildHits ( CSphString & sError, bool bSkipEndMarker );

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument ( bool & bEOF, CSphString & sError ) = 0;
	virtual const int *		GetFieldLengths () const = 0;

	CSphVector<int64_t> *	GetFieldMVA ( int iAttr ) override;
	const CSphString &		GetStrAttr ( int iAttr ) override;
	SphAttr_t				GetAttr ( int iAttr ) override;

protected:
	void					ParseFieldMVA ( int iAttr, const char * szValue );
	bool					CheckFileField ( const BYTE * sField );
	int						LoadFileField ( BYTE ** ppField, CSphString & sError );

	bool					BuildZoneHits ( RowID_t tRowID, BYTE uCode );
	void					BuildSubstringHits ( RowID_t tRowID, bool bPayload, ESphWordpart eWordpart, int & iBlendedHitsStart );
	void					BuildRegularHits ( RowID_t tRowID, bool bPayload, int & iBlendedHitsStart );

	/// register autocomputed attributes such as field lengths (see index_field_lengths)
	bool					AddAutoAttrs ( CSphString & sError, StrVec_t * pDefaults = nullptr );

	/// allocate m_tDocInfo storage, do post-alloc magic (compute pointer to field lengths, etc)
	void					AllocDocinfo ();

protected:
	ISphHits				m_tHits;				///< my hitvector

protected:
	char *					m_pReadFileBuffer = nullptr;
	int						m_iReadFileBufferSize = 256*1024;	///< size of read buffer for the 'sql_file_field' fields
	int						m_iMaxFileBufferSize = 2*1024*1024;	///< max size of read buffer for the 'sql_file_field' fields
	ESphOnFileFieldError	m_eOnFileFieldError = FFE_IGNORE_FIELD;
	FILE *					m_fpDumpRows = nullptr;
	int						m_iPlainFieldsLength = 0;
	DWORD *					m_pFieldLengthAttrs = nullptr;	///< pointer into the part of m_tDocInfo where field lengths are stored

	CSphVector<IDPair_t>	m_dAllIds;					///< used for joined fields FIXME! unlimited RAM use
	bool					m_bIdsSorted = false;			///< we sort array to use binary search

protected:
	struct CSphBuildHitsState_t
	{
		bool m_bProcessingHits;
		bool m_bDocumentDone;

		BYTE ** m_dFields;
		CSphVector<int> m_dFieldLengths;

		CSphVector<BYTE*> m_dTmpFieldStorage;
		CSphVector<BYTE*> m_dTmpFieldPtrs;
		CSphVector<BYTE> m_dFiltered;

		int m_iStartPos;
		Hitpos_t m_iHitPos;
		int m_iField;
		int m_iStartField;
		int m_iEndField;

		int m_iBuildLastStep;

		CSphBuildHitsState_t ();
		~CSphBuildHitsState_t ();

		void Reset ();
	};

	CSphBuildHitsState_t	m_tState;
	int						m_iMaxHits;
	CSphVector<CSphVector<BYTE>> m_dDocFields;

protected:
	StrVec_t							m_dStrAttrs;	///< current document string attrs
	CSphVector<CSphVector<int64_t>>		m_dMvas;		///< per-attribute MVA storage
	CSphVector<SphAttr_t>				m_dAttrs;

	TokenizerRefPtr_c				m_pTokenizer;	///< my tokenizer
	DictRefPtr_c					m_pDict;		///< my dict
	std::unique_ptr<ISphFieldFilter> m_pFieldFilter;	///< my field filter

	CSphSourceStats					m_tStats;		///< my stats
	CSphSchema 						m_tSchema;		///< my schema

	CSphHTMLStripper *				m_pStripper = nullptr;	///< my HTML stripper
	CSphBitvec						m_tMorphFields;
};
