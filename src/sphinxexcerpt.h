//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
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

/// a query to generate an excerpt
/// everything string is expected to be UTF-8
struct ExcerptQuery_t
{
	CSphString		m_sSource;			///< source text (or file name, see m_bLoadFiles)
	CSphString		m_sWords;			///< words themselves
	CSphString		m_sBeforeMatch;		///< string to insert before each match
	CSphString		m_sAfterMatch;		///< string to insert after each match
	CSphString		m_sChunkSeparator;	///< string to insert between matching chunks (in limited mode only)
	CSphString		m_sStripMode;		///< strip mode
	int				m_iLimit;			///< max chars in snippet (0 if unlimited)
	int				m_iLimitWords;		///< max words in snippet
	int				m_iLimitPassages;	///< max passages in snippet
	int				m_iAround;			///< how much words to highlight around each match
	int				m_iPassageId;		///< current %PASSAGE_ID% counter value (must start at 1)
	bool			m_bRemoveSpaces;	///< whether to collapse whitespace
	bool			m_bExactPhrase;		///< whether to highlight exact phrase matches only
	bool			m_bUseBoundaries;	///< whether to extract passages by phrase boundaries setup in tokenizer
	bool			m_bWeightOrder;		///< whether to order best passages in document (default) or weight order
	bool			m_bHighlightQuery;	///< whether try to highlight the whole query, or always word-by-word
	bool			m_bForceAllWords;	///< whether to ignore limit until all needed keywords are highlighted (#448)
	bool			m_bLoadFiles;		///< whether to interpret source as text or file name
	bool			m_bAllowEmpty;		///< whether to allow empty snippets (by default, return something from the start)

public:
	ExcerptQuery_t ()
		: m_sBeforeMatch ( "<b>" )
		, m_sAfterMatch ( "</b>" )
		, m_sChunkSeparator ( " ... " )
		, m_sStripMode ( "index" )
		, m_iLimit ( 256 )
		, m_iLimitWords ( 0 )
		, m_iLimitPassages ( 0 )
		, m_iAround ( 5 )
		, m_iPassageId ( 1 )
		, m_bRemoveSpaces ( false )
		, m_bExactPhrase ( false )
		, m_bUseBoundaries ( false )
		, m_bWeightOrder ( false )
		, m_bHighlightQuery ( false )
		, m_bForceAllWords ( false )
		, m_bLoadFiles ( false )
		, m_bAllowEmpty ( false )
	{
	}
};

/// an excerpt generator
/// returns a newly allocated string in encoding specified by tokenizer on success
/// returns NULL on failure
char * sphBuildExcerpt ( ExcerptQuery_t &, CSphDict *, ISphTokenizer *, const CSphSchema *, CSphIndex *, CSphString & sError );

#endif // _sphinxexcerpt_

//
// $Id$
//
