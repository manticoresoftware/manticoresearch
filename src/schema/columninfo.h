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

#pragma once

#include "sphinxstd.h"

#include "sphinxdefs.h"
#include "locator.h"
#include "sphinxexpr.h"

/// source column info
struct CSphColumnInfo
{
	enum
	{
		FIELD_NONE			= 0,
		FIELD_STORED		= 1 << 0,
		FIELD_INDEXED		= 1 << 1,
		FIELD_IS_ATTRIBUTE	= 1 << 2, // internal flag used in 'alter'
	};

	enum
	{
		ATTR_NONE				= 0,
		ATTR_COLUMNAR			= 1 << 0,
		ATTR_COLUMNAR_HASHES	= 1 << 1,
		ATTR_STORED				= 1 << 2
	};

	CSphString		m_sName;							///< column name
	ESphAttr		m_eAttrType;						///< attribute type

	ESphWordpart	m_eWordpart { SPH_WORDPART_WHOLE };	///< wordpart processing type
	bool			m_bIndexed = false;					///< whether to index this column as fulltext field too

	int				m_iIndex = -1;						///< index into source result set (-1 for joined fields)
	CSphAttrLocator m_tLocator;							///< attribute locator in the row

	ESphAttrSrc		m_eSrc { SPH_ATTRSRC_NONE };		///< attr source (for multi-valued attrs only)
	CSphString		m_sQuery;							///< query to retrieve values (for multi-valued attrs only)
	CSphString		m_sQueryRange;						///< query to retrieve range (for multi-valued attrs only)

	ISphExprRefPtr_c m_pExpr;							///< evaluator for expression items
	ESphAggrFunc	m_eAggrFunc { SPH_AGGR_NONE };		///< aggregate function on top of expression (for GROUP BY)
	ESphEvalStage	m_eStage { SPH_EVAL_STATIC };		///< column evaluation stage (who and how computes this column)
	bool			m_bPayload = false;
	bool			m_bFilename = false;				///< column is a file name
	bool			m_bWeight = false;					///< is a weight column
	DWORD			m_uFieldFlags = FIELD_INDEXED;		///< stored/indexed/highlighted etc
	DWORD			m_uAttrFlags = ATTR_NONE;			///< attribute storage spec
	AttrEngine_e	m_eEngine = AttrEngine_e::DEFAULT;	///< used together with per-table engine specs to determine attribute storage

	WORD			m_uNext = 0xFFFF;					///< next in linked list for hash in CSphSchema

	/// handy ctor
	explicit CSphColumnInfo ( const char * sName = nullptr, ESphAttr eType = SPH_ATTR_NONE );

	/// equality comparison checks name, type, and locator
	bool operator== ( const CSphColumnInfo & rhs ) const;

	/// returns true if this column stores a pointer to data
	bool IsDataPtr() const;

	bool IsColumnar() const;
	bool HasStringHashes() const;
	bool IsColumnarExpr() const;
	bool IsStoredExpr() const;
};


CSphString sphDumpAttr ( const CSphColumnInfo & tAttr );
