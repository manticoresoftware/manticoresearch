//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _snippetstream_
#define _snippetstream_

#include "snippetfunctor.h"


class CacheStreamer_i
{
public:
	virtual			~CacheStreamer_i() {}

	virtual void	StoreToken ( const TokenInfo_t & tTok, int iTermIndex ) = 0;
	virtual void	StoreOverlap ( int iStart, int iLen, int iBoundary ) = 0;
	virtual void	StoreSkipHtml ( int iStart, int iLen ) = 0;
	virtual void	StoreSPZ ( BYTE iSPZ, DWORD uPosition, const char *, int iZone ) = 0;
	virtual void	StoreTail ( int iStart, int iLen, int iBoundary ) = 0;

	virtual void	SetZoneInfo ( const FunctorZoneInfo_t & tZoneInfo ) = 0;
	virtual void	Tokenize ( TokenFunctor_i & tFunctor ) = 0;
	virtual bool	IsEmpty() const = 0;
};


CacheStreamer_i *	CreateCacheStreamer ( int iDocLen );
void				TokenizeDocument ( HitCollector_i & tFunctor, const CSphHTMLStripper * pStripper, DWORD iSPZ );


#endif // _snippetstream_
