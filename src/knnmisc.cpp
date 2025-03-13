//
// Copyright (c) 2023-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <algorithm>
#include "knnmisc.h"
#include "knnlib.h"
#include "exprtraits.h"
#include "sphinxint.h"
#include "fileio.h"
#include "sphinxjson.h"


void NormalizeVec ( VecTraits_T<float> & dData )
{
	float fNorm = 0.0f;
	for ( auto i : dData )
		fNorm += i*i;

	fNorm = 1.0f / ( sqrtf(fNorm) + 1e-30f );
	for ( auto & i : dData )
		i *= fNorm;
}


class Expr_KNNDist_c : public ISphExpr
{
public:
				Expr_KNNDist_c ( const CSphVector<float> & dAnchor, const CSphColumnInfo & tAttr );

	float		Eval ( const CSphMatch & tMatch ) const override;
	int			IntEval ( const CSphMatch & tMatch ) const override		{ return (int)Eval(tMatch); }
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const override	{ return (int64_t)Eval(tMatch); }
	void		FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override { sphFixupLocator ( m_tAttr.m_tLocator, pOldSchema, pNewSchema ); }
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override;
	ISphExpr *	Clone() const override									{ return new Expr_KNNDist_c ( m_dAnchor, m_tAttr ); }
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;

	void		SetData ( const util::Span_T<const knn::DocDist_t> & dData );

private:
	std::unique_ptr<knn::Distance_i>	m_pDistCalc;
	CSphVector<float>					m_dAnchor;
	CSphColumnInfo						m_tAttr;
	const BYTE *						m_pBlobPool = nullptr;
	std::unique_ptr<columnar::Iterator_i> m_pIterator;

	util::Span_T<const  knn::DocDist_t>	m_dData;
	mutable const knn::DocDist_t *		m_pStart = nullptr;
};


Expr_KNNDist_c::Expr_KNNDist_c ( const CSphVector<float> & dAnchor, const CSphColumnInfo & tAttr )
	: m_dAnchor ( dAnchor )
	, m_tAttr ( tAttr )
{
	knn::IndexSettings_t tDistSettings = tAttr.m_tKNN;
	tDistSettings.m_eQuantization = knn::Quantization_e::NONE; // we operate on non-quantized data
	m_pDistCalc = CreateKNNDistanceCalc(tDistSettings);

	if ( tAttr.m_tKNN.m_eHNSWSimilarity==knn::HNSWSimilarity_e::COSINE )
		NormalizeVec(m_dAnchor);
}


float Expr_KNNDist_c::Eval ( const CSphMatch & tMatch ) const
{
	if ( m_pStart ) // use precalculated data
	{
		const knn::DocDist_t * pEnd = m_dData.end();
		const knn::DocDist_t * pPtr = std::lower_bound ( m_pStart, pEnd, tMatch.m_tRowID, []( auto & tEntry, RowID_t tValue ){ return tEntry.m_tRowID < tValue; } );
		assert ( pPtr!=pEnd && pPtr->m_tRowID==tMatch.m_tRowID );
		m_pStart = pPtr;
		return m_pStart->m_fDist;
	}
	else // calculate distance
	{
		// this code path is used when no iterator is available, i.e. in ram chunk
		ByteBlob_t tRes;
		if ( m_tAttr.IsColumnar() )
			tRes.second = m_pIterator->Get ( tMatch.m_tRowID, tRes.first );
		else
			tRes = tMatch.FetchAttrData ( m_tAttr.m_tLocator, m_pBlobPool );

		VecTraits_T<float> dData ( (float*)tRes.first, tRes.second / sizeof(float) );
		if ( dData.GetLength()!=m_tAttr.m_tKNN.m_iDims )
			return FLT_MAX;

		return m_pDistCalc->CalcDist ( { dData.Begin(), (size_t)dData.GetLength() }, { m_dAnchor.Begin(), (size_t)m_dAnchor.GetLength() } );
	}
}


void Expr_KNNDist_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	switch ( eCmd )
	{
	case SPH_EXPR_SET_COLUMNAR:
		if ( m_tAttr.IsColumnar() )
		{
			auto pColumnar = (const columnar::Columnar_i*)pArg;
			if ( pColumnar )
			{
				std::string sError; // FIXME! report errors
				m_pIterator = CreateColumnarIterator ( pColumnar, m_tAttr.m_sName.cstr(), sError );
			}
			else
				m_pIterator.reset();
		}
		break;

	case SPH_EXPR_SET_BLOB_POOL:
		m_pBlobPool = (const BYTE*)pArg;

		// reset our temporary data (e.g. between index chunks)
		m_pStart = nullptr;
		break;

	default:
		break;
	}
}


void Expr_KNNDist_c::SetData ( const util::Span_T<const knn::DocDist_t> & dData )
{
	m_dData = dData;
	m_pStart = m_dData.data();
}


uint64_t Expr_KNNDist_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_KNNDist_c");
	return CALC_DEP_HASHES();
}

/////////////////////////////////////////////////////////////////////

const char * GetKnnDistAttrName()
{
	static const char * szName = "@knn_dist";
	return szName;
}


ISphExpr * CreateExpr_KNNDist ( const CSphVector<float> & dAnchor, const CSphColumnInfo & tAttr )
{
	return new Expr_KNNDist_c ( dAnchor, tAttr );
}


static const char * HNSWSimilarity2Str ( knn::HNSWSimilarity_e eSim )
{
	switch ( eSim )
	{
	case knn::HNSWSimilarity_e::L2:		return "L2";
	case knn::HNSWSimilarity_e::IP:		return "IP";
	case knn::HNSWSimilarity_e::COSINE:	return "COSINE";
	default:							return nullptr;
	}
}


static const char * Quantization2Str ( knn::Quantization_e eQuant )
{
	switch ( eQuant )
	{
	case knn::Quantization_e::BIT1: return "1BIT";
	case knn::Quantization_e::BIT4: return "4BIT";
	case knn::Quantization_e::BIT8: return "8BIT";
	default: return nullptr;
	}
}


bool Str2HNSWSimilarity ( const CSphString & sSimilarity, knn::HNSWSimilarity_e & eSimilarity, CSphString * pError )
{
	CSphString sSim = sSimilarity;
	sSim.ToUpper();

	if ( sSim=="L2" )
	{
		eSimilarity = knn::HNSWSimilarity_e::L2;
		return true;
	}

	if ( sSim=="IP" )
	{
		eSimilarity = knn::HNSWSimilarity_e::IP;
		return true;
	}

	if ( sSim=="COSINE" )
	{
		eSimilarity = knn::HNSWSimilarity_e::COSINE;
		return true;
	}

	if ( pError )
		pError->SetSprintf ( "Unknown knn similarity '%s'", sSimilarity.cstr() );

	return false;
}


bool Str2Quantization ( const CSphString & sQuantization, knn::Quantization_e & eQuantization, CSphString * pError )
{
	CSphString sQuant = sQuantization;
	sQuant.ToUpper();

	if ( sQuant=="NONE" )
	{
		eQuantization = knn::Quantization_e::NONE;
		return true;
	}

	if ( sQuant=="1BIT" )
	{
		eQuantization = knn::Quantization_e::BIT1;
		return true;
	}

	if ( sQuant=="4BIT" )
	{
		eQuantization = knn::Quantization_e::BIT4;
		return true;
	}

	if ( sQuant=="8BIT" )
	{
		eQuantization = knn::Quantization_e::BIT8;
		return true;
	}

	if ( pError )
		pError->SetSprintf ( "Unknown quantization '%s'", sQuantization.cstr() );

	return false;
}


void AddKNNSettings ( StringBuilder_c & sRes, const CSphColumnInfo & tAttr )
{
	if ( !tAttr.IsIndexedKNN() )
		return;

	const auto & tKNN = tAttr.m_tKNN;

	sRes << " knn_type='hnsw'";
	sRes << " knn_dims='" << tKNN.m_iDims << "'";
	sRes << " hnsw_similarity='" << HNSWSimilarity2Str ( tKNN.m_eHNSWSimilarity ) << "'";

	knn::IndexSettings_t tDefault;
	if ( tKNN.m_iHNSWM!=tDefault.m_iHNSWM )
		sRes << " hnsw_m='" << tKNN.m_iHNSWM << "'";

	if ( tKNN.m_iHNSWEFConstruction!=tDefault.m_iHNSWEFConstruction )
		sRes << " hnsw_ef_construction='" << tKNN.m_iHNSWEFConstruction << "'";

	if ( tKNN.m_eQuantization!=tDefault.m_eQuantization )
		sRes << " quantization='" << Quantization2Str ( tKNN.m_eQuantization ) << "'";
}


knn::IndexSettings_t ReadKNNJson ( bson::Bson_c tRoot )
{
	knn::IndexSettings_t tRes;
	tRes.m_iDims = (int) bson::Int ( tRoot.ChildByName ( "knn_dims" ) );
	Str2HNSWSimilarity ( bson::String ( tRoot.ChildByName ( "hnsw_similarity" ) ), tRes.m_eHNSWSimilarity );
	tRes.m_iHNSWM = (int) bson::Int ( tRoot.ChildByName ( "hnsw_m" ), tRes.m_iHNSWM );
	tRes.m_iHNSWEFConstruction = (int) bson::Int ( tRoot.ChildByName ( "hnsw_ef_construction" ), tRes.m_iHNSWEFConstruction );
	Str2Quantization ( bson::String ( tRoot.ChildByName ( "quantization" ) ), tRes.m_eQuantization );

	return tRes;
}


void operator << ( JsonEscapedBuilder & tOut, const knn::IndexSettings_t & tSettings )
{
	auto _ = tOut.Object();

	knn::IndexSettings_t tDefault;

	tOut.NamedString ( "knn_type", "hnsw" );
	tOut.NamedVal ( "knn_dims", tSettings.m_iDims );
	tOut.NamedString ( "hnsw_similarity", HNSWSimilarity2Str ( tSettings.m_eHNSWSimilarity ) );
	tOut.NamedValNonDefault ( "hnsw_m", tSettings.m_iHNSWM, tDefault.m_iHNSWM );
	tOut.NamedValNonDefault ( "hnsw_ef_construction", tSettings.m_iHNSWEFConstruction, tDefault.m_iHNSWEFConstruction );
	tOut.NamedString ( "quantization", Quantization2Str ( tSettings.m_eQuantization ) );
}


CSphString FormatKNNConfigStr ( const CSphVector<NamedKNNSettings_t> & dAttrs )
{
	JsonObj_c tRoot;
	JsonObj_c tArray(true);

	for ( auto & i : dAttrs )
	{
		JsonObj_c tObj;
		tObj.AddStr ( "name", i.m_sName );
		tObj.AddStr ( "type", "hnsw" );
		tObj.AddInt ( "dims", i.m_iDims );
		tObj.AddStr ( "hnsw_similarity", HNSWSimilarity2Str ( i.m_eHNSWSimilarity ) );
		tObj.AddInt ( "hnsw_m", i.m_iHNSWM );
		tObj.AddInt ( "hnsw_ef_construction", i.m_iHNSWEFConstruction );
		tObj.AddStr ( "quantization", Quantization2Str ( i.m_eQuantization ) );
		tArray.AddItem(tObj);
	}

	tRoot.AddItem ( "attrs", tArray );
	return tRoot.AsString(false);
}


bool ParseKNNConfigStr ( const CSphString & sStr, CSphVector<NamedKNNSettings_t> & dParsed, CSphString & sError )
{
	JsonObj_c tRoot ( sStr.cstr() );
	const int MAX_ERROR_LEN = 256;
	char szError[MAX_ERROR_LEN];
	if ( tRoot.GetError ( szError, MAX_ERROR_LEN, sError ) )
		return false;

	JsonObj_c tAttrs = tRoot.GetArrayItem ( "attrs", sError );
	if ( !tAttrs )
		return false;

	for ( const auto & i : tAttrs )
	{
		auto & tParsed = dParsed.Add();

		if ( !i.FetchStrItem ( tParsed.m_sName, "name", sError ) )
			return false;

		CSphString sType;
		if ( !i.FetchStrItem ( sType, "type", sError ) )
			return false;

		sType.ToUpper();
		if ( sType!="HNSW" )
		{
			sError.SetSprintf ( "Unknown knn type '%s'", sType.cstr() );
			return false;
		}

		CSphString sSimilarity;
		if ( !i.FetchIntItem ( tParsed.m_iDims, "dims", sError ) ) return false;
		if ( !i.FetchIntItem ( tParsed.m_iHNSWM, "hnsw_m", sError, true ) ) return false;
		if ( !i.FetchIntItem ( tParsed.m_iHNSWEFConstruction, "hnsw_ef_construction", sError, true ) ) return false;
		if ( !i.FetchStrItem ( sSimilarity, "hnsw_similarity", sError) ) return false;

		if ( !Str2HNSWSimilarity ( sSimilarity.cstr(), tParsed.m_eHNSWSimilarity, &sError ) )
			return false;

		JsonObj_c tQuantization = i.GetStrItem ( "quantization", sError, true );
		if ( !sError.IsEmpty() )
			return false;

		if ( tQuantization )
		{
			CSphString sQuantization = tQuantization.StrVal();
			if ( !Str2Quantization ( sQuantization.cstr(), tParsed.m_eQuantization, &sError ) )
				return false;
		}
	}

	return true;
}


std::unique_ptr<knn::Builder_i> BuildCreateKNN ( const ISphSchema & tSchema, int64_t iNumElements, CSphVector<PlainOrColumnar_t> & dAttrs, CSphString & sError )
{
	std::unique_ptr<knn::Builder_i> pBuilder = CreateKNNBuilder ( tSchema, iNumElements, sError );
	if ( !pBuilder )
		return pBuilder;
	
	int iColumnar = 0;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.IsIndexedKNN() )
			dAttrs.Add ( PlainOrColumnar_t ( tAttr, iColumnar ) );

		if ( tAttr.IsColumnar() )
			iColumnar++;
	}

	return pBuilder;
}


template <typename ACTION>
static bool BuildProcessKNN ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, ACTION && fnAction )
{
	int iKNNAttrIndex = 0;
	for ( auto & i : dAttrs )
	{
		assert ( i.m_eType==SPH_ATTR_FLOAT_VECTOR );
		const BYTE * pSrc = nullptr;
		int iBytes = i.Get ( tRowID, pRow, pPool, dIterators, pSrc );
		int iValues = iBytes / sizeof(float);

		if ( !fnAction ( iKNNAttrIndex, { (float*)pSrc, (size_t)iValues } ) )
			return false;

		iKNNAttrIndex++;
	}

	return true;
}


void BuildTrainKNN ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, knn::Builder_i & tBuilder )
{
	BuildProcessKNN ( tRowID, pRow, pPool, dIterators, dAttrs, [&tBuilder]( int iAttr, const util::Span_T<float> & tValues ) { tBuilder.Train ( iAttr, tValues ); return true; } );
}


bool BuildStoreKNN ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, knn::Builder_i & tBuilder )
{
	return BuildProcessKNN ( tRowID, pRow, pPool, dIterators, dAttrs, [&tBuilder]( int iAttr, const util::Span_T<float> & tValues ) { return tBuilder.SetAttr ( iAttr, tValues ); } );
}


std::pair<RowidIterator_i *, bool> CreateKNNIterator ( knn::KNN_i * pKNN, const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, CSphString & sError )
{
	if ( tQuery.m_sKNNAttr.IsEmpty() )
		return { nullptr, false };

	auto pKNNAttr = tIndexSchema.GetAttr ( tQuery.m_sKNNAttr.cstr() );
	if ( !pKNNAttr )
	{
		sError.SetSprintf ( "KNN search attribute '%s' not found", tQuery.m_sKNNAttr.cstr() );
		return { nullptr, true };
	}

	if ( !pKNNAttr->IsIndexedKNN() )
	{
		sError.SetSprintf ( "KNN search attribute '%s' does not have KNN index", tQuery.m_sKNNAttr.cstr() );
		return { nullptr, true };
	}

	if ( !pKNN )
	{
		sError = "KNN index not loaded" ;
		return { nullptr, true };
	}

	const auto pAttr = tSorterSchema.GetAttr ( GetKnnDistAttrName() );
	assert(pAttr);

	ISphExpr * pExpr = pAttr->m_pExpr;
	assert(pExpr);

	auto pKnnDist = (Expr_KNNDist_c*)pExpr;

	CSphVector<float> dPoint ( tQuery.m_dKNNVec );
	if ( pKNNAttr->m_tKNN.m_eHNSWSimilarity == knn::HNSWSimilarity_e::COSINE )
		NormalizeVec(dPoint);

	std::string sErrorSTL;
	knn::Iterator_i * pIterator = pKNN->CreateIterator ( pKNNAttr->m_sName.cstr(), { dPoint.Begin(), (size_t)dPoint.GetLength() }, tQuery.m_iKNNK, tQuery.m_iKnnEf, sErrorSTL );
	if ( !pIterator )
	{
		sError = sErrorSTL.c_str();
		return { nullptr, true };
	}

	pKnnDist->SetData ( pIterator->GetData() );
	
	return { CreateIteratorWrapper ( pIterator, nullptr ), false };
}


RowIteratorsWithEstimates_t	CreateKNNIterators ( knn::KNN_i * pKNN, const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, bool & bError, CSphString & sError )
{
	RowIteratorsWithEstimates_t dIterators;

	auto tRes = CreateKNNIterator ( pKNN, tQuery, tIndexSchema, tSorterSchema, sError );
	if ( tRes.second )
	{
		bError = true;
		return dIterators;
	}

	if ( !tRes.first )
		return dIterators;

	dIterators.Add ( { tRes.first, tQuery.m_iKNNK } );
	return dIterators;
}
