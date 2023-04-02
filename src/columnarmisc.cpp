//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "columnarmisc.h"
#include "schema/schema.h"
#include "attribute.h"

CSphVector<ScopedTypedIterator_t> CreateAllColumnarIterators ( const columnar::Columnar_i * pColumnar, const ISphSchema & tSchema )
{
	CSphVector<ScopedTypedIterator_t> dIterators;
	if ( !pColumnar )
		return dIterators;

	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.IsColumnar() )
		{
			std::string sError;
			dIterators.Add ( { CreateColumnarIterator ( pColumnar, tAttr.m_sName.cstr(), sError ), tAttr.m_eAttrType } );
			assert ( dIterators.Last().first );
		}
	}

	return dIterators;
}


SphAttr_t SetColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder, std::unique_ptr<columnar::Iterator_i> & pIterator, RowID_t tRowID, CSphVector<int64_t> & dTmp )
{
	switch ( eType )
	{
	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
	{
		const BYTE * pResult = nullptr;
		int iBytes = pIterator->Get ( tRowID, pResult );
		int iValues = iBytes / ( eType==SPH_ATTR_UINT32SET ? sizeof(DWORD) : sizeof(int64_t) );
		if ( eType==SPH_ATTR_UINT32SET )
		{
			// need a 64-bit array as input. so we need to convert our 32-bit array to 64-bit entries
			dTmp.Resize(iValues);
			ARRAY_FOREACH ( i, dTmp )
				dTmp[i] = ((DWORD*)pResult)[i];

			pBuilder->SetAttr ( iAttr, dTmp.Begin(), iValues );
		}
		else
			pBuilder->SetAttr ( iAttr, (const int64_t*)pResult, iValues );
	}
	break;

	case SPH_ATTR_STRING:
	{
		const BYTE * pResult = nullptr;
		int iBytes = pIterator->Get ( tRowID, pResult);
		pBuilder->SetAttr ( iAttr, (const uint8_t*)pResult, iBytes );
	}
	break;

	default:
		{
			int64_t iValue = pIterator->Get(tRowID);
			pBuilder->SetAttr ( iAttr, iValue );
			return iValue;
		}
	}

	return 0;
}


void SetDefaultColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder )
{
	switch ( eType )
	{
	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		pBuilder->SetAttr ( iAttr, (const int64_t *)0, 0 );
		break;

	case SPH_ATTR_STRING:
		pBuilder->SetAttr ( iAttr, (const uint8_t *)0, 0 );
		break;

	default:
		pBuilder->SetAttr ( iAttr, 0 );
		break;
	}
}


SphAttr_t PlainOrColumnar_t::Get ( RowID_t tRowID, const CSphRowitem * pRow, CSphVector<ScopedTypedIterator_t> & dIterators ) const
{
	if ( m_iColumnarId>=0 )
		return dIterators[m_iColumnarId].first->Get(tRowID);

	return sphGetRowAttr ( pRow, m_tLocator );
}

int PlainOrColumnar_t::Get ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const uint8_t * & pData ) const
{
	if ( m_iColumnarId>=0 )
		return dIterators[m_iColumnarId].first->Get ( tRowID, pData );

	int iLen = 0;
	pData = sphGetBlobAttr ( pRow, m_tLocator, pPool, iLen );
	return iLen;
}
