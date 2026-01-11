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

#include "match.h"
#include "searchdaemon.h"

#include <functional>


// in MatchIterator_c we need matches sorted assending by DocID.
// also we don't want to sort matches themselves; sorted vec of indexes quite enough
// also we wont to avoid allocating vec for the matches as it may be huge.
// There are several possible solutions to have vec of indexes:
// 1. Use matches tags, as they're not used in this part of code. With intensive working it is however not a good in
// terms of cache misses (i.e. 'min' match is match[N] where N is match[0].tag, then match[M] where M is match[1] tag.
// So each time we make about random jumps.
// 2. Use space between last match and end of the vector (assuming reserved space > used space). If it is enough space,
// we can use it either as vec or WORDS, or as vec or DWORDS, depending from N of matches. First case need at most 128K
// of RAM, second needs more, but that RAM is compact.
// So, let's try with tail space first, but if it is not available (no, or not enough space), use tags.

class MatchIterator_c
{
	int m_iRawIdx; // raw iteration index (internal)
	int m_iLimit;
	std::function<int  ( int )> m_fnOrder; // use to access matches by accending docid order
	bool m_bTailClean = false;

	bool MaybeUseWordOrder ( const CSphSwapVector<CSphMatch> & dMatches ) const noexcept;
	bool MaybeUseDwordOrder ( const CSphSwapVector<CSphMatch> & dMatches ) const noexcept;
	void UseTags ( VecTraits_T<CSphMatch> & dOrder ) noexcept;

public:
	OneResultset_t & m_tResult;
	DocID_t m_tDocID;
	int m_iIdx; // ordering index (each step gives matches in sorted by Docid order)


	explicit MatchIterator_c ( OneResultset_t & tResult );
	~MatchIterator_c ();

	bool Step () noexcept;
	static bool IsLess ( MatchIterator_c * a, MatchIterator_c * b ) noexcept
	{
		if ( a->m_tDocID != b->m_tDocID )
			return a->m_tDocID < b->m_tDocID;

		// that mean local matches always preffered over remote, but it seems that is not necessary
//		if ( !a->m_dResult.m_bTag && b->m_dResult.m_bTag )
//			return true;

		return a->m_tResult.m_iTag>b->m_tResult.m_iTag;
	}
};