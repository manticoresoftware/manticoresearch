//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
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
	CSphString		m_sSource;			///< source text
	CSphString		m_sWords;			///< words themselves
	CSphString		m_sBeforeMatch;		///< string to insert before each match
	CSphString		m_sAfterMatch;		///< string to insert after each match
	CSphString		m_sChunkSeparator;	///< string to insert between matching chunks (in limited mode only)
	int				m_iLimit;			///< max chars in excerpt (0 if unlimited)
	int				m_bRemoveSpaces;	///< whether to collapse whitespace
	int				m_iAround;			///< how much words to highlight around each match

public:
	ExcerptQuery_t ()
		: m_sBeforeMatch ( "<b>" )
		, m_sAfterMatch ( "</b>" )
		, m_sChunkSeparator ( " ... " )
		, m_iLimit ( 256 )
		, m_bRemoveSpaces ( false )
		, m_iAround ( 5 )
	{
	}
};

/// an excerpt generator
/// returns a newly allocated UTF-8 string
char *				sphBuildExcerpt ( const ExcerptQuery_t & q );

#endif // _sphinxexcerpt_

//
// $Id$
//
