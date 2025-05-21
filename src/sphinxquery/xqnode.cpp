//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "sphinxquery.h"
#include "xqdebug.h"

// ctor
XQNode_t::XQNode_t ( const XQLimitSpec_t & dSpec )
	: m_dSpec ( dSpec )
{
#if XQ_DUMP_NODE_ADDR
	printf ( "node new %p\n", this );
#endif
}

/// dtor
XQNode_t::~XQNode_t ()
{
#if XQ_DUMP_NODE_ADDR
	printf ( "node deleted %d %p\n", m_eOp, this );
#endif
	ARRAY_FOREACH ( i, m_dChildren )
		SafeDelete ( m_dChildren[i] );
}

// manipulate with words, caring hash
void XQNode_t::AddDirtyWord ( XQKeyword_t dWord )
{
	assert (!m_iFuzzyHash && !m_iMagicHash);
	m_dWords.Add ( std::move ( dWord ) );
}


void XQNode_t::SetFieldSpec ( const FieldMask_t & uMask, int iMaxPos )
{
	// set it, if we do not yet have one
	if ( !m_dSpec.m_bFieldSpec )
		m_dSpec.SetFieldSpec ( uMask, iMaxPos );

	// some of the children might not yet have a spec, even if the node itself has
	// eg. 'hello @title world' (whole node has '@title' spec but 'hello' node does not have any!)
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetFieldSpec ( uMask, iMaxPos );
}



void XQNode_t::SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan )
{
	// set it, if we do not yet have one
	if ( !m_dSpec.m_dZones.GetLength() )
		m_dSpec.SetZoneSpec ( dZones, bZoneSpan );

	// some of the children might not yet have a spec, even if the node itself has
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetZoneSpec ( dZones, bZoneSpan );
}

void XQNode_t::CopySpecs ( const XQNode_t * pSpecs )
{
	if ( !pSpecs )
		return;

	if ( !m_dSpec.m_bFieldSpec )
		m_dSpec.SetFieldSpec ( pSpecs->m_dSpec.m_dFieldMask, pSpecs->m_dSpec.m_iFieldMaxPos );

	if ( !m_dSpec.m_dZones.GetLength() )
		m_dSpec.SetZoneSpec ( pSpecs->m_dSpec.m_dZones, pSpecs->m_dSpec.m_bZoneSpan );
}


void XQNode_t::ClearFieldMask ()
{
	m_dSpec.m_dFieldMask.SetAll();

	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->ClearFieldMask();
}


uint64_t XQNode_t::GetHash () const noexcept
{
	if ( m_iMagicHash )
		return m_iMagicHash;

	XQOperator_e dZeroOp[2];
	dZeroOp[0] = m_eOp;
	dZeroOp[1] = (XQOperator_e) 0;

	for ( const auto& dWord : dWords() )
		m_iMagicHash = 100 + ( m_iMagicHash ^ sphFNV64 ( dWord.m_sWord.cstr() ) ); // +100 to make it non-transitive
	for ( const auto* pChild : m_dChildren )
		m_iMagicHash = 100 + ( m_iMagicHash ^ pChild->GetHash() ); // +100 to make it non-transitive
	m_iMagicHash += 1000000; // to immerse difference between parents and children
	m_iMagicHash ^= sphFNV64 ( dZeroOp );

	return m_iMagicHash;
}


uint64_t XQNode_t::GetFuzzyHash () const noexcept
{
	if ( m_iFuzzyHash )
		return m_iFuzzyHash;

	XQOperator_e dZeroOp[2];
	dZeroOp[0] = ( m_eOp==SPH_QUERY_PHRASE ? SPH_QUERY_PROXIMITY : m_eOp );
	dZeroOp[1] = (XQOperator_e) 0;

	for ( const auto& dWord : dWords() )
		m_iFuzzyHash = 100 + ( m_iFuzzyHash ^ sphFNV64 ( dWord.m_sWord.cstr() ) ); // +100 to make it non-transitive
	for ( const auto* pChild : m_dChildren )
		m_iFuzzyHash = 100 + ( m_iFuzzyHash ^ pChild->GetFuzzyHash () ); // +100 to make it non-transitive
	m_iFuzzyHash += 1000000; // to immerse difference between parents and children
	m_iFuzzyHash ^= sphFNV64 ( dZeroOp );

	return m_iFuzzyHash;
}


void XQNode_t::SetOp ( XQOperator_e eOp, XQNode_t * pArg1, XQNode_t * pArg2 )
{
	SetOp ( eOp );
	m_dChildren.Reset();
	if ( pArg1 )
	{
		m_dChildren.Add ( pArg1 );
		pArg1->m_pParent = this;
	}
	if ( pArg2 )
	{
		m_dChildren.Add ( pArg2 );
		pArg2->m_pParent = this;
	}
}


int XQNode_t::FixupAtomPos() const noexcept
{
	assert ( m_eOp==SPH_QUERY_PROXIMITY && !dWords().IsEmpty() );

	ARRAY_FOREACH ( i, m_dWords )
	{
		if ( const int iSub = m_dWords[i].m_iSkippedBefore-1; iSub>0 )
		{
			for ( int j = i; j < m_dWords.GetLength(); j++ )
				m_dWords[j].m_iAtomPos -= iSub;
		}
	}

	return m_dWords.Last().m_iAtomPos+1;
}


XQNode_t * XQNode_t::Clone () const
{
	XQNode_t * pRet = new XQNode_t ( m_dSpec );
	pRet->SetOp ( m_eOp );
	pRet->m_dWords = m_dWords;
	pRet->m_iOpArg = m_iOpArg;
	pRet->m_iAtomPos = m_iAtomPos;
	pRet->m_iUser = m_iUser;
	pRet->m_bVirtuallyPlain = m_bVirtuallyPlain;
	pRet->m_bNotWeighted = m_bNotWeighted;
	pRet->m_bPercentOp = m_bPercentOp;

	if ( m_dChildren.IsEmpty() )
		return pRet;

	pRet->m_dChildren.Reserve ( m_dChildren.GetLength() );
	for ( const auto* pChild : m_dChildren )
		pRet->AddNewChild ( pChild->Clone() );

	return pRet;
}