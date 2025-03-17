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

#ifndef _grouper_
#define _grouper_

#include "sortsetup.h"

namespace columnar
{
	class Columnar_i;
}


class BlobPool_c
{
public:
	virtual			~BlobPool_c() = default;
	virtual void	SetBlobPool ( const BYTE * pBlobPool ) { m_pBlobPool = pBlobPool; }
	const BYTE *	GetBlobPool () const { return m_pBlobPool; }

protected:
	const BYTE *	m_pBlobPool {nullptr};
};

/// groupby key type
typedef int64_t SphGroupKey_t;

/// base grouper (class that computes groupby key)
class CSphGrouper : public BlobPool_c, public ISphRefcountedMT
{
public:
	virtual SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const = 0;
	virtual SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const = 0;
	virtual void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const = 0;
	virtual void			GetLocator ( CSphAttrLocator & tOut ) const = 0;
	virtual ESphAttr		GetResultType () const = 0;
	virtual CSphGrouper *	Clone() const = 0;
	virtual bool			IsMultiValue() const { return false; }
	virtual void			SetColumnar ( const columnar::Columnar_i * ) {}

protected:
							~CSphGrouper () override {} // =default causes bunch of errors building on wheezy
};

class DistinctFetcher_i : public ISphRefcountedMT
{
public:
	virtual SphAttr_t		GetKey ( const CSphMatch & tMatch ) const = 0;
	virtual void			GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const = 0;
	virtual void			SetBlobPool ( const BYTE * pBlobPool ) = 0;
	virtual void			SetColumnar ( const columnar::Columnar_i * pColumnar ) = 0;
	virtual void			FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) = 0;
	virtual bool			IsMultiValue() const = 0;
	virtual DistinctFetcher_i *	Clone() const = 0;
};

CSphGrouper * CreateGrouperDay ( const CSphAttrLocator & tLoc );
CSphGrouper * CreateGrouperWeek ( const CSphAttrLocator & tLoc );
CSphGrouper * CreateGrouperMonth ( const CSphAttrLocator & tLoc );
CSphGrouper * CreateGrouperYear ( const CSphAttrLocator & tLoc );
CSphGrouper * CreateGrouperJsonField ( const CSphAttrLocator & tLoc, ISphExpr * pExpr );
CSphGrouper * CreateGrouperMVA32 ( const CSphAttrLocator & tLoc );
CSphGrouper * CreateGrouperMVA64 ( const CSphAttrLocator & tLoc );
CSphGrouper * CreateGrouperAttr ( const CSphAttrLocator & tLoc );
CSphGrouper * CreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation );
CSphGrouper * CreateGrouperStringExpr ( ISphExpr * pExpr, ESphCollation eCollation );
CSphGrouper * CreateGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation );

DistinctFetcher_i * CreateDistinctFetcher ( const CSphString & sName, const CSphAttrLocator & tLocator, ESphAttr eType );

#endif // _grouper_
