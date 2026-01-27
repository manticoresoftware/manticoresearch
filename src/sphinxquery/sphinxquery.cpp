//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxquery.h"
#include "sphinxsearch.h"

#include "xqparser.h"
#include "xqdebug.h"

NodeEstimate_t & NodeEstimate_t::operator+= ( const NodeEstimate_t & tRhs )
{
	m_fCost += tRhs.m_fCost;
	m_iDocs += tRhs.m_iDocs;
	m_iTerms += tRhs.m_iTerms;
	return *this;
}

void XQLimitSpec_t::SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan )
{
	m_dZones = dZones;
	m_bZoneSpan = bZoneSpan;
}

void XQLimitSpec_t::SetFieldSpec ( const FieldMask_t & uMask, int iMaxPos )
{
	m_bFieldSpec = true;
	m_dFieldMask = uMask;
	m_iFieldMaxPos = iMaxPos;
}

uint64_t XQLimitSpec_t::Hash () const noexcept
{
	uint64_t uHash = sphFNV64 ( &m_dFieldMask, sizeof ( m_dFieldMask ) );
	uHash = sphFNV64 ( &m_iFieldMaxPos, sizeof ( m_iFieldMaxPos ), uHash );
	if ( m_bZoneSpan )
		++uHash;
	if ( !m_dZones.IsEmpty() )
		uHash = sphFNV64 ( m_dZones.begin(), m_dZones.GetLengthBytes(), uHash );
	return uHash;
}

XQQuery_t * CloneXQQuery ( const XQQuery_t & tQuery )
{
	auto * pQuery = new XQQuery_t;
	pQuery->m_dZones = tQuery.m_dZones;
	pQuery->m_bNeedSZlist = tQuery.m_bNeedSZlist;
	pQuery->m_bSingleWord = tQuery.m_bSingleWord;
	pQuery->m_bEmpty = tQuery.m_bEmpty;
	pQuery->m_pRoot = tQuery.m_pRoot ? tQuery.m_pRoot->Clone() : nullptr;
	return pQuery;
}

XQNode_t * CloneKeyword ( const XQNode_t * pNode )
{
	assert ( pNode );

	auto * pClone = new XQNode_t ( pNode->m_dSpec );
	pClone->WithWords ( [pNode] ( auto & dWords ) { dWords = pNode->dWords(); } );
	return pClone;
}

//////////////////////////////////////////////////////////////////////////

#if XQDEBUG
namespace {

void xqIndent ( int iIndent )
{
	iIndent *= 2;
	while ( iIndent-- )
		printf ( "|-" );
}


void xqDumpNode ( const XQNode_t * pNode )
{
	const bool bHasWords = !pNode->dWords().IsEmpty();
	if ( bHasWords && pNode->GetOp() == SPH_QUERY_AND )
		printf ( "MATCH(%d,%d):", pNode->m_dSpec.m_dFieldMask.GetMask32(), pNode->m_iOpArg );
	else
		printf ( "%s", XQOperatorNameSz ( pNode->GetOp()));

	printf ( " %p ", (void*)pNode->m_iFuzzyHash );

	for ( const XQKeyword_t & tWord : pNode->dWords() )
	{
		const char * sLocTag = "";
		if ( tWord.m_bFieldStart ) sLocTag = ", start";
		if ( tWord.m_bFieldEnd ) sLocTag = ", end";

		printf ( " %s (qpos %d%s)", tWord.m_sWord.cstr(), tWord.m_iAtomPos, sLocTag );
	}
}

} // namespace

void xqDump ( const XQNode_t * pNode, int iIndent )
{
#if XQ_DUMP_NODE_ADDR
	printf ( "%p ", pNode );
#endif
	xqIndent ( iIndent );
	xqDumpNode ( pNode );
	if ( pNode->dChildren().IsEmpty() )
	{
		printf ( "\n" );
		return;
	}

	if ( ( pNode->dChildren().GetLength()==1 )
		&& ( pNode->GetOp()==SPH_QUERY_AND)
		&& ( !pNode->dChild(0)->dWords().IsEmpty()))
	{
		xqDumpNode ( pNode->dChild(0) );
		printf ( "\n" );
		return;
	}

	printf ( " (%d)\n", pNode->dChildren().GetLength() );
	for ( const auto* tChild : pNode->dChildren() )
	{
		assert ( tChild->m_pParent==pNode );
		xqDump ( tChild, iIndent + 1 );
	}
}
#endif


#if XQDEBUG
void Dump ( const XQNode_t * pNode, const char * sHeader, bool bWithDot )
{
	if ( !pNode )
		return;

	printf ( "%s\n", sHeader );
	printf ( "%s\n", sphReconstructNode ( pNode ).cstr() );
#if XQ_DUMP_TRANSFORMED_TREE
	xqDump ( pNode, 0 );
	if( bWithDot )
		DotDump ( pNode );
	fflush ( stdout );
#endif
}
#else
void Dump (const XQNode_t *, const char *, bool)
{
}
#endif


CSphString sphReconstructNode ( const XQNode_t * pNode, const CSphSchema * pSchema, StrVec_t * pZones )
{
	StringBuilder_c sRes ( " " );

	if ( !pNode )
		return CSphString{sRes};

	if ( pNode->dWords().GetLength() )
	{
		// say just words to me
		const CSphVector<XQKeyword_t> & dWords = pNode->dWords();
		for ( const auto& i: dWords )
			sRes << i.m_sWord.cstr();
		CSphString sTrim { sRes };
		sTrim.Trim();
		sRes.Clear ();

		switch ( pNode->GetOp() )
		{
		case SPH_QUERY_AND:			sRes << sTrim; break;
		case SPH_QUERY_PHRASE:		sRes << '"' << sTrim << '"'; break;
		case SPH_QUERY_PROXIMITY:	sRes << '"' << sTrim << "\"~" <<  pNode->m_iOpArg; break;
		case SPH_QUERY_QUORUM:		sRes << '"' << sTrim << "\"/" <<  pNode->m_iOpArg; break;
		case SPH_QUERY_NEAR:		sRes << '"' << sTrim << "\"NEAR/" <<  pNode->m_iOpArg; break;
		case SPH_QUERY_NOTNEAR:		sRes << '"' << sTrim << "\"NOTNEAR/" <<  pNode->m_iOpArg; break;
		default:					assert ( 0 && "unexpected op in ReconstructNode()" ); break;
		}

		sRes.MoveTo ( sTrim );
		if ( !pNode->m_dSpec.m_dFieldMask.TestAll(true) )
		{
			CSphString sFields ( "" );
			if ( pSchema )
			{
				for ( int i=0; i<pSchema->GetFieldsCount(); ++i )
					if ( pNode->m_dSpec.m_dFieldMask.Test(i) )
						sFields.SetSprintf ( "%s,%s", sFields.cstr(), pSchema->GetFieldName(i) );
			}
			else if (!pNode->m_dSpec.m_dFieldMask.TestAll(false))
				sFields.SetSprintf ( "%s,%u", sFields.cstr(), pNode->m_dSpec.m_dFieldMask.GetMask32() );

			const int iFieldLimit = pNode->m_dSpec.m_iFieldMaxPos;
			if ( iFieldLimit )
			{
				if ( sFields.IsEmpty() )
					sRes.Sprintf ( "( @missed[%d]: %s )", sTrim.cstr(), iFieldLimit );
				else
					sRes.Sprintf ( "( @%s[%d]: %s )", sFields.cstr() + 1, iFieldLimit, sTrim.cstr() );
			} else
			{
				if ( sFields.IsEmpty() )
					sRes.Sprintf ( "( @missed: %s )", sTrim.cstr() );
				else
					sRes.Sprintf ( "( @%s: %s )", sFields.cstr() + 1, sTrim.cstr() );
			}
		} else
		{
			if ( pNode->GetOp()==SPH_QUERY_AND && dWords.GetLength()>1 )
				sRes.Sprintf ( "( %s )", sTrim.cstr() ); // wrap bag of words
			else
				sRes << sTrim;
		}

		if ( pZones || !pNode->m_dSpec.m_dZones.IsEmpty () )
		{
			sRes.MoveTo ( sTrim );
			{
				ScopedComma_c sZone ( sRes, ",", (pNode->m_dSpec.m_bZoneSpan?"ZONESPAN:(":"ZONE:("), ") " );
				for ( const auto& iZone: pNode->m_dSpec.m_dZones )
					sRes << (*pZones)[iZone];
			}
			sRes << sTrim;
		}

	} else
	{
		ARRAY_FOREACH ( i, pNode->dChildren() )
		{
			if ( !i )
			{
				auto sFoo = sphReconstructNode ( pNode->dChild(i), pSchema, pZones );
				sRes.Clear();
				sRes << sFoo;
			} else
			{
				const char * sOp = XQOperatorNameSz ( pNode->GetOp() );
				// fixup some ops to shortened versions suitable for reconstruction
				switch ( pNode->GetOp() )
				{
				case SPH_QUERY_AND:		sOp = " "; break;
				case SPH_QUERY_OR:		sOp = "|"; break;
				case SPH_QUERY_ANDNOT:	sOp = "AND NOT"; break;
				case SPH_QUERY_PHRASE:	sOp = ""; break;
				default: break;
				}
				CSphString sTrim { sRes };
				sRes.Clear();

				if ( pNode->GetOp()==SPH_QUERY_PHRASE )
					sRes.Sprintf ( "\"%s %s\"", sTrim.cstr(), sphReconstructNode ( pNode->dChild(i), pSchema, pZones ).cstr() );
				else
					sRes.Sprintf ( "%s %s %s", sTrim.cstr(), sOp, sphReconstructNode ( pNode->dChild(i), pSchema, pZones ).cstr() );
			}
		}

		if ( pNode->dChildren().GetLength()>1 )
		{
			CSphString sFoo;
			sRes.MoveTo ( sFoo );
			sRes.Clear();
			sRes.Sprintf ( "( %s )", sFoo.cstr() );
		}
	}

	return CSphString{sRes};
}

alignas ( 128 ) static constexpr BYTE g_UrlEncodeTable[] = { // 0, if you need escape, 1 if not
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, // -.
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 0123456789
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ABCDEFGHIJKLMNO
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // PQRSTUVWXYZ_
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // abcdefghijklmno
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, // pqrstuvwxyz~
};

static CSphString UrlEncode ( const CSphString& sSource )
{
	StringBuilder_c sRes;
	for ( const auto* pC = sSource.cstr (); *pC; ++pC )
	{
		auto c = (BYTE)*pC;
		if ( ( c & 0x80 ) || !g_UrlEncodeTable[c] )
			sRes.Sprintf("%%%02x",c);
		else
			sRes.RawC((char)c);
	}

	CSphString sResult;
	sRes.MoveTo (sResult);
	return sResult;
}

void DotDump (const XQNode_t * pNode)
{
	auto dPlan = sphExplainQuery ( pNode );
	StringBuilder_c sRes;
	sph::RenderBsonPlan ( sRes, bson::MakeHandle ( dPlan ), true );
	printf ( "\nhttps://dreampuf.github.io/GraphvizOnline/#%s\n", UrlEncode ( CSphString{sRes} ).cstr() );
}
