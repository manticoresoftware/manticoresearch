//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxexcerpt_
#define _sphinxexcerpt_

#include "sphinx.h"
#include "sphinxquery.h"

enum ESphSpz : DWORD
{
	SPH_SPZ_NONE		= 0,
	SPH_SPZ_SENTENCE	= 1UL<<0,
	SPH_SPZ_PARAGRAPH	= 1UL<<1,
	SPH_SPZ_ZONE		= 1UL<<2
};


class TextSource_i
{
public:
	virtual						~TextSource_i() {}

	virtual bool				PrepareText ( ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, CSphString & sError ) = 0;
	virtual VecTraits_T<BYTE>	GetText ( int iField ) const = 0;
	virtual int					GetNumFields() const = 0;
	virtual bool				TextFromIndex() const = 0;
};

/// a query to generate an excerpt
/// everything string is expected to be UTF-8
struct SnippetQuerySettings_t
{
	CSphString		m_sBeforeMatch {"<b>"};	///< string to insert before each match
	CSphString		m_sAfterMatch {"</b>"};	///< string to insert after each match
	CSphString		m_sChunkSeparator {" ... \0"};	///< string to insert between matching chunks (in limited mode only)
	CSphString		m_sFieldSeparator {" | "};///< string to insert between fields
	CSphString		m_sStripMode {"index"};	///< strip mode
	int				m_iLimit = 256;			///< max chars in snippet (0 if unlimited)
	int				m_iLimitWords = 0;		///< max words in snippet
	int				m_iLimitPassages = 0;	///< max passages in snippet
	int				m_iAround = 5;			///< how much words to highlight around each match
	int				m_iPassageId = 1;		///< current %PASSAGE_ID% counter value (must start at 1)
	bool			m_bRemoveSpaces = false;///< whether to collapse whitespace
	bool			m_bUseBoundaries = false;	///< whether to extract passages by phrase boundaries setup in tokenizer
	bool			m_bWeightOrder = false;	///< whether to order best passages in document (default) or weight order
	bool			m_bForceAllWords = false;///< whether to ignore limit until all needed keywords are highlighted (#448)
	BYTE			m_uFilesMode = 0;		///< sources are text(0), files(1), scattered files(2), only scattered files (3).
	bool			m_bAllowEmpty = false;	///< whether to allow empty snippets (by default, return something from the start)
	bool			m_bEmitZones = false;	///< whether to emit zone for passage
	bool			m_bForcePassages = false; ///< whether to force passages generation

	bool			m_bHasBeforePassageMacro = false;
	bool			m_bHasAfterPassageMacro = false;
	CSphString		m_sBeforeMatchPassage;
	CSphString		m_sAfterMatchPassage;

	ESphSpz			m_ePassageSPZ { SPH_SPZ_NONE };
	bool			m_bJsonQuery { false };
};


struct SnippetResult_t
{
	CSphVector<int>		m_dSeparators;
	CSphVector<BYTE>	m_dResult;
	CSphString			m_sError;
	CSphString			m_sWarning;
};


class SnippetBuilder_i
{
public:
	virtual			~SnippetBuilder_i() = default;

	virtual bool	Setup ( const CSphIndex * pIndex, const SnippetQuerySettings_t & tQuery, CSphString & sError ) = 0;
	virtual bool	SetQuery ( const CSphString & sQuery, bool bIgnoreFields, CSphString & sError ) = 0;
	virtual bool	Build ( TextSource_i * pSource, SnippetResult_t & tRes ) const = 0;
};


// helper whether filepath from sPath does not escape area of sPrefix
bool			TestEscaping( const CSphString& sPrefix, const CSphString& sPath );
ESphSpz			GetPassageBoundary ( const CSphString & sPassageBoundaryMode );
const char *	PassageBoundarySz ( ESphSpz eBoundary );
bool			sphCheckOptionsSPZ ( const SnippetQuerySettings_t & q, ESphSpz eMode, CSphString & sError );
bool			SnippetTransformPassageMacros ( CSphString & sSrc, CSphString & sPost );

TextSource_i *		CreateSnippetSource ( DWORD uFilesMode, const BYTE * pSource, int iLen );
TextSource_i *		CreateHighlightSource ( const CSphVector<VecTraits_T<BYTE>> & dAllFields );
SnippetBuilder_i *	CreateSnippetBuilder();

extern CSphString g_sSnippetsFilePrefix;

#endif // _sphinxexcerpt_
