//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "sphinxquery.h"

class PluginQueryTokenFilter_c;
using PluginQueryTokenRefPtr_c = CSphRefcountedPtr<PluginQueryTokenFilter_c>;

class XQParseHelper_c
{
public:
	virtual ~XQParseHelper_c () = default;

	void SetString ( const char * szString );

	bool			AddField ( FieldMask_t & dFields, const char * szField, int iLen );
	bool			ParseFields ( FieldMask_t & dFields, int & iMaxFieldPos, bool & bIgnore );

	void			Setup ( const CSphSchema * pSchema, TokenizerRefPtr_c pTokenizer, DictRefPtr_c pDict, XQQuery_t * pXQQuery, const CSphIndexSettings & tSettings );
	bool			Error ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	void			Warning ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	XQNode_t *		FixupTree ( XQNode_t * pRoot, const XQLimitSpec_t & tLimitSpec, const CSphBitvec * pMorphFields, bool bOnlyNotAllowed );

	const CSphSchema * GetSchema() const { return m_pSchema; }
	DictRefPtr_c&	GetDict() { return m_pDict; }

	bool			IsError() { return m_bError; }
	virtual void	Cleanup();
	void			SetZone ( const StrVec_t & dZones ) const noexcept;
	const StrVec_t & GetZone() const noexcept;
	XQNode_t *		SpawnNode ( const XQLimitSpec_t & dSpec ) noexcept;
	void			DeleteSpawned ( XQNode_t * pNode ) noexcept;

protected:
	struct MultiformNode_t
	{
		XQNode_t *	m_pNode;
		int			m_iDestStart;
		int			m_iDestCount;
	};

	static const int MAX_TOKEN_BYTES = 3*SPH_MAX_WORD_LEN + 16;

	const CSphSchema *		m_pSchema {nullptr};
	TokenizerRefPtr_c		m_pTokenizer;
	DictRefPtr_c			m_pDict;
	bool					m_bStopOnInvalid {true};
	XQQuery_t *				m_pParsed {nullptr};
	bool					m_bError {false};

	PluginQueryTokenRefPtr_c m_pPlugin;
	void *					m_pPluginData {nullptr};

	int						m_iAtomPos {0};
	bool					m_bEmptyStopword {false};
	bool					m_bWasBlended {false};
	bool					m_bExpandBlended { false };

	CSphVector<XQNode_t*>		m_dSpawned;
	StrVec_t					m_dDestForms;
	CSphVector<MultiformNode_t>	m_dMultiforms;

	virtual bool	HandleFieldBlockStart ( const char * & pPtr ) = 0;
	virtual bool	HandleSpecialFields ( const char * & /*pPtr*/, FieldMask_t & /*dFields*/ ) { return false; }
	virtual bool	NeedTrailingSeparator() { return true; }

private:
	XQNode_t *		SweepNulls ( XQNode_t * pNode, bool bOnlyNotAllowed );
	bool			FixupNots ( XQNode_t * pNode, bool bOnlyNotAllowed, XQNode_t ** ppRoot );
	void			FixupNulls ( XQNode_t * pNode );
	void			DeleteNodesWOFields ( XQNode_t * pNode );
	void			FixupDestForms();
	bool			CheckQuorumProximity ( const XQNode_t * pNode );
	bool			CheckNear ( const XQNode_t * pNode );
	void			FixupBlend ( XQNode_t * pNode );
};