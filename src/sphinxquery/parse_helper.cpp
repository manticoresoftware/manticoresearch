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

#include "parse_helper.h"
#include "sphinxplugin.h"

#include "tokenizer/tokenizer.h"
#include "dict/dict_base.h"

namespace { // static

void TransformMorphOnlyFields ( XQNode_t * pNode, const CSphBitvec & tMorphDisabledFields )
{
	if ( !pNode )
		return;

	pNode->WithChildren([&tMorphDisabledFields] ( auto & dChildren ) {
		for ( auto& pChild : dChildren )
			TransformMorphOnlyFields ( pChild, tMorphDisabledFields );
	});
	if ( pNode->m_dSpec.IsEmpty () || pNode->dWords().IsEmpty () )
		return;

	const XQLimitSpec_t & tSpec = pNode->m_dSpec;
	if ( tSpec.m_bFieldSpec && !tSpec.m_dFieldMask.TestAll ( true ) )
	{
		auto iField=tMorphDisabledFields.Scan ( 0 );
		while ( iField<tMorphDisabledFields.GetSize() )
		{
			if ( pNode->m_dSpec.m_dFieldMask.Test ( iField ) )
			{
				pNode->WithWords ( [] ( auto& dWords ) {
					dWords.for_each ( [] ( XQKeyword_t & tKw )
					{
						if ( !tKw.m_sWord.IsEmpty() && !tKw.m_sWord.Begins( "=" ) && !tKw.m_sWord.Begins("*") && !tKw.m_sWord.Ends("*") )
							tKw.m_sWord.SetSprintf ( "=%s", tKw.m_sWord.cstr() );
					});
				});
			}

			if ( ( iField+1 )>=tMorphDisabledFields.GetSize() )
				break;
			iField=tMorphDisabledFields.Scan ( iField+1 );
		}
	}
}

void FixupMorphOnlyFields ( XQNode_t * pNode, const CSphBitvec * pMorphFields )
{
	if ( !pNode || !pMorphFields || pMorphFields->IsEmpty() )
		return;

	// set the only fields with the morphology_skip_fields option to use bitvec::scan
	CSphBitvec tMorphDisabledFields ( *pMorphFields );
	tMorphDisabledFields.Negate();
	TransformMorphOnlyFields ( pNode, tMorphDisabledFields );
}

int GetMaxAtomPos ( const XQNode_t * pNode )
{
	int iMaxAtomPos = -1;
	if ( !pNode )
		return iMaxAtomPos;

	for ( const auto & tWord : pNode->dWords() )
		iMaxAtomPos = Max ( iMaxAtomPos, tWord.m_iAtomPos );

	for ( const XQNode_t * pItem : pNode->dChildren() )
		iMaxAtomPos = GetMaxAtomPos ( pItem );

	return iMaxAtomPos;
}

void FixupDegenerates ( XQNode_t * pNode, CSphString & sWarning )
{
	if ( !pNode )
		return;

	if ( pNode->dWords().GetLength() == 1 &&
		(pNode->GetOp() == SPH_QUERY_PHRASE || pNode->GetOp() == SPH_QUERY_PROXIMITY || pNode->GetOp() == SPH_QUERY_QUORUM) )
	{
		if ( pNode->GetOp() == SPH_QUERY_QUORUM && !pNode->m_bPercentOp && pNode->m_iOpArg > 1 )
			sWarning.SetSprintf ( "quorum threshold too high (words=%d, thresh=%d); replacing quorum operator with AND operator", pNode->dWords().GetLength(), pNode->m_iOpArg );

		pNode->SetOp ( SPH_QUERY_AND );
		return;
	}

	pNode->WithChildren ( [&sWarning] ( auto & dChildren ) {
		dChildren.for_each ( [&sWarning] ( auto & dChild ) { FixupDegenerates ( dChild, sWarning ); } );
	} );
}

XQNode_t * TransformOnlyNot ( XQNode_t * pNode, CSphVector<XQNode_t *> & dSpawned )
{
	XQNode_t * pScan = new XQNode_t ( pNode->m_dSpec );
	pScan->SetOp ( SPH_QUERY_SCAN );
	// set the last atom pos for the spawned scan term
	pScan->m_iAtomPos = GetMaxAtomPos ( pNode );
	if ( pScan->m_iAtomPos != -1 )
		pScan->m_iAtomPos++;

	dSpawned.Add ( pScan );

	XQNode_t * pAnd = new XQNode_t ( pNode->m_dSpec );
	pAnd->SetOp ( SPH_QUERY_ANDNOT, pScan, pNode );
	dSpawned.Add ( pScan );

	return pAnd;
}

XQNode_t * FixupNot ( XQNode_t * pNode, CSphVector<XQNode_t *> & dSpawned )
{
	pNode->SetOp ( SPH_QUERY_AND );
	return TransformOnlyNot ( pNode, dSpawned );
}
} // namespace

//////////////////////////////////////////////////////////////////////////
void XQParseHelper_c::SetString ( const char * szString )
{
	assert ( m_pTokenizer );
	m_pTokenizer->SetBuffer ( (const BYTE *) szString, (int) strlen ( szString ) );
	m_iAtomPos = 0;
}


/// lookup field and add it into mask
bool XQParseHelper_c::AddField ( FieldMask_t & dFields, const char * szField, int iLen )
{
	if ( !szField || !iLen )
		return Error ( "empty field passed to AddField()" );

	CSphString sField;
	sField.SetBinary ( szField, iLen );

	int iField = m_pSchema->GetFieldIndex ( sField.cstr() );
	if ( iField < 0 )
	{
		if ( m_bStopOnInvalid )
			return Error ( "no field '%s' found in schema", sField.cstr() );
		Warning ( "no field '%s' found in schema", sField.cstr () );
	} else
	{
		if ( iField>=SPH_MAX_FIELDS )
			return Error ( " max %d fields allowed", SPH_MAX_FIELDS );

		dFields.Set ( iField );
	}

	return true;
}


/// parse fields block
bool XQParseHelper_c::ParseFields ( FieldMask_t & dFields, int & iMaxFieldPos, bool & bIgnore )
{
	dFields.UnsetAll();
	iMaxFieldPos = 0;
	bIgnore = false;

	const char * pPtr = m_pTokenizer->GetBufferPtr ();
	const char * pLastPtr = m_pTokenizer->GetBufferEnd ();

	if ( pPtr==pLastPtr )
		return true; // silently ignore trailing field operator

	bool bNegate = false;
	bool bBlock = false;

	// handle special modifiers
	if ( *pPtr=='!' )
	{
		// handle @! and @!(
		bNegate = true; ++pPtr;
		if ( *pPtr=='(' ) { bBlock = true; ++pPtr; }

	} else if ( *pPtr=='*' )
	{
		// handle @*
		dFields.SetAll();
		m_pTokenizer->SetBufferPtr ( pPtr+1 );
		return true;
	} else if ( HandleSpecialFields ( pPtr, dFields ) )
		return true;
	else
		bBlock = HandleFieldBlockStart ( pPtr );

	// handle invalid chars
	if ( !sphIsAlpha(*pPtr) )
	{
		bIgnore = true;
		m_pTokenizer->SetBufferPtr ( pPtr ); // ignore and re-parse (FIXME! maybe warn?)
		return true;
	}
	assert ( sphIsAlpha(*pPtr) ); // i think i'm paranoid

	// handle field specification
	if ( !bBlock )
	{
		// handle standalone field specification
		const char * pFieldStart = pPtr;
		while ( sphIsAlpha(*pPtr) && pPtr<pLastPtr )
			++pPtr;

		assert ( pPtr-pFieldStart>0 );
		if ( !AddField ( dFields, pFieldStart, int ( pPtr-pFieldStart ) ) )
			return false;

		m_pTokenizer->SetBufferPtr ( pPtr );
		if ( bNegate )
			dFields.Negate();

	} else
	{
		// handle fields block specification
		assert ( sphIsAlpha(*pPtr) && bBlock ); // and complicated

		bool bOK = false;
		const char * pFieldStart = nullptr;
		while ( pPtr<pLastPtr )
		{
			// accumulate field name, while we can
			if ( sphIsAlpha(*pPtr) || *pPtr=='.' )
			{
				if ( !pFieldStart )
					pFieldStart = pPtr;
				++pPtr;
				continue;
			}

			// separator found
			if ( !pFieldStart )
			{
				CSphString sContext;
				sContext.SetBinary ( pPtr, (int)( pLastPtr-pPtr ) );
				return Error ( "error parsing field list: invalid field block operator syntax near '%s'", sContext.cstr() );
			}
			if ( *pPtr==',' )
			{
				if ( !AddField ( dFields, pFieldStart, int ( pPtr-pFieldStart ) ) )
					return false;

				pFieldStart = nullptr;
				++pPtr;

			} else if ( *pPtr==')' )
			{
				if ( !AddField ( dFields, pFieldStart, int ( pPtr-pFieldStart ) ) )
					return false;

				m_pTokenizer->SetBufferPtr ( ++pPtr );
				if ( bNegate )
					dFields.Negate();

				bOK = true;
				break;

			} else
			{
				return Error ( "error parsing field list: invalid character '%c' in field block operator", *pPtr );
			}
		}

		if ( !bOK )
		{
			if ( NeedTrailingSeparator() )
				return Error ( "error parsing field list: missing closing ')' in field block operator" );

			if ( !AddField ( dFields, pFieldStart, int ( pPtr-pFieldStart ) ) )
				return false;

			if ( bNegate )
				dFields.Negate();

			return true;
		}
	}

	// handle optional position range modifier
	if ( pPtr[0]=='[' && isdigit ( pPtr[1] ) )
	{
		// skip '[' and digits
		const char * p = pPtr+1;
		while ( *p && isdigit(*p) ) p++;

		// check that the range ends with ']' (FIXME! maybe report an error if it does not?)
		if ( *p!=']' )
			return true;

		// fetch my value
		iMaxFieldPos = strtoul ( pPtr+1, NULL, 10 );
		m_pTokenizer->SetBufferPtr ( p+1 );
	}

	// well done
	return true;
}


void XQParseHelper_c::Setup ( const CSphSchema * pSchema, TokenizerRefPtr_c pTokenizer, DictRefPtr_c pDict, XQQuery_t * pXQQuery, const CSphIndexSettings & tSettings )
{
	m_pSchema = pSchema;
	m_pTokenizer = std::move ( pTokenizer );
	m_pDict = std::move (pDict);
	m_pParsed = pXQQuery;
	m_iAtomPos = 0;
	m_bEmptyStopword = ( tSettings.m_iStopwordStep==0 );
}


bool XQParseHelper_c::Error ( const char * sTemplate, ... )
{
	assert ( m_pParsed );
	char sBuf[256];

	const char * sPrefix = "query error: ";
	auto iPrefix = strlen(sPrefix);
	memcpy ( sBuf, sPrefix, iPrefix );

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf+iPrefix, sizeof(sBuf)-iPrefix, sTemplate, ap );
	va_end ( ap );

	m_bError = true;
	m_pParsed->m_sParseError = sBuf;
	return false;
}


void XQParseHelper_c::Warning ( const char * sTemplate, ... )
{
	assert ( m_pParsed );
	char sBuf[256];

	const char * sPrefix = "query warning: ";
	auto iPrefix = strlen(sPrefix);
	memcpy ( sBuf, sPrefix, iPrefix );

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf+iPrefix, sizeof(sBuf)-iPrefix, sTemplate, ap );
	va_end ( ap );

	m_pParsed->m_sParseWarning = sBuf;
}


void XQParseHelper_c::Cleanup()
{
	m_dSpawned.Uniq(sph::unstable); // FIXME! should eliminate this by testing

	for ( auto& pSpawned : m_dSpawned )
	{
		pSpawned->ResetChildren();
		SafeDelete ( pSpawned );
	}
	m_dSpawned.Reset ();
}


bool XQParseHelper_c::CheckQuorumProximity ( const XQNode_t * pNode )
{
	if ( !pNode )
		return true;

	bool bQuorumPassed = ( pNode->GetOp()!=SPH_QUERY_QUORUM ||
		( pNode->m_iOpArg>0 && ( !pNode->m_bPercentOp || pNode->m_iOpArg<=100 ) ) );
	if ( !bQuorumPassed )
	{
		if ( pNode->m_bPercentOp )
			return Error ( "quorum threshold out of bounds 0.0 and 1.0f (%f)", 1.0f / 100.0f * pNode->m_iOpArg );
		return Error ( "quorum threshold too low (%d)", pNode->m_iOpArg );
	}

	if ( pNode->GetOp()==SPH_QUERY_PROXIMITY && pNode->m_iOpArg<1 )
		return Error ( "proximity threshold too low (%d)", pNode->m_iOpArg );

	return pNode->dChildren().all_of ( [&] ( XQNode_t * pChild ) { return CheckQuorumProximity(pChild); } );
}

XQNode_t * XQParseHelper_c::FixupTree ( XQNode_t * pRoot, const XQLimitSpec_t & tLimitSpec, const CSphBitvec * pMorphFields, bool bOnlyNotAllowed )
{
	constexpr bool bDump = false;
	if constexpr ( bDump ) Dump ( pRoot, "raw FixupTree" );
	FixupDestForms ();
	if constexpr ( bDump ) Dump ( pRoot, "FixupDestForms" );
	DeleteNodesWOFields ( pRoot );
	if constexpr ( bDump ) Dump ( pRoot, "DeleteNodesWOFields" );
	pRoot = SweepNulls ( pRoot, bOnlyNotAllowed );
	if constexpr ( bDump ) Dump ( pRoot, "SweepNulls" );
	FixupDegenerates ( pRoot, m_pParsed->m_sParseWarning );
	if constexpr ( bDump ) Dump ( pRoot, "FixupDegenerates" );
	FixupMorphOnlyFields ( pRoot, pMorphFields );
	if constexpr ( bDump ) Dump ( pRoot, "FixupMorphOnlyFields" );
	FixupNulls ( pRoot );
	if constexpr ( bDump ) Dump ( pRoot, "FixupNulls" );

	if ( !FixupNots ( pRoot, bOnlyNotAllowed, &pRoot ) )
	{
		Cleanup ();
		return nullptr;
	}
	if constexpr ( bDump ) Dump ( pRoot, "FixupNots" );

	if ( !CheckQuorumProximity ( pRoot ) )
	{
		Cleanup();
		return nullptr;
	}
	if constexpr ( bDump ) Dump ( pRoot, "CheckQuorumProximity" );

	if ( pRoot && pRoot->GetOp()==SPH_QUERY_NOT )
	{
		if ( bOnlyNotAllowed  )
		{
			pRoot = FixupNot ( pRoot, m_dSpawned );
			if constexpr ( bDump ) Dump ( pRoot, "FixupNot" );
		} else if ( !pRoot->m_iOpArg )
		{
			Cleanup();
			Error ( "query is non-computable (single NOT operator)" );
			return nullptr;
		}
	}

	// all ok; might want to create a dummy node to indicate that
	m_dSpawned.Reset();

	if ( pRoot && pRoot->GetOp()==SPH_QUERY_NULL )
		SafeDelete ( pRoot );

	return pRoot ? pRoot : new XQNode_t ( tLimitSpec );
}


XQNode_t * XQParseHelper_c::SweepNulls ( XQNode_t * pNode, bool bOnlyNotAllowed )
{
	if ( !pNode )
		return nullptr;

	// sweep plain node
	if ( !pNode->dWords().IsEmpty() )
	{
		pNode->WithWords ( [] (auto& dWords) {
		ARRAY_FOREACH ( i, dWords )
			if ( !dWords[i].m_sWord.cstr() )
				dWords.Remove ( i-- );
		});

		if ( pNode->dWords().IsEmpty() )
		{
			DeleteSpawned ( pNode );
			return nullptr;
		}

		return pNode;
	}

	// sweep op node
	pNode->WithChildren ( [this,bOnlyNotAllowed,pNode] ( CSphVector<XQNode_t*>& dChildren ) {
	ARRAY_FOREACH ( i, dChildren )
	{
		dChildren[i] = SweepNulls ( dChildren[i], bOnlyNotAllowed );
		if ( !dChildren[i] )
		{
			dChildren.Remove ( i-- );
			// use non-null iOpArg as a flag indicating that the sweeping happened.
			++pNode->m_iOpArg;
		}
	}});

	if ( pNode->dChildren().IsEmpty() )
	{
		DeleteSpawned ( pNode ); // OPTIMIZE!
		return nullptr;
	}

	// remove redundancies if needed
	if ( pNode->GetOp() != SPH_QUERY_NOT && pNode->dChildren().GetLength() == 1 )
	{
		auto * pRet = pNode->dChildren()[0];
		pNode->ResetChildren();
		pRet->m_pParent = pNode->m_pParent;
		// expressions like 'la !word' (having min_word_len>len(la)) became a 'null' node.
		if ( pNode->m_iOpArg && pRet->GetOp()==SPH_QUERY_NOT && !bOnlyNotAllowed )
		{
			pRet->SetOp ( SPH_QUERY_NULL );
			pRet->WithChildren ( [this] ( auto& dChildren ) {
				dChildren.for_each ( [this] (auto& pChild) { DeleteSpawned ( pChild );} );
				dChildren.Reset();
			});
		}
		pRet->m_iOpArg = pNode->m_iOpArg;

		DeleteSpawned ( pNode ); // OPTIMIZE!
		return SweepNulls ( pRet, bOnlyNotAllowed );
	}

	// done
	return pNode;
}

void XQParseHelper_c::FixupNulls ( XQNode_t * pNode )
{
	if ( !pNode )
		return;

	pNode->WithChildren([this,pNode](auto& dChildren) {

	dChildren.for_each ( [this](auto& dChild) { FixupNulls ( dChild );} );

	// smth OR null == smth.
	if ( pNode->GetOp()==SPH_QUERY_OR )
	{

		CSphVector<XQNode_t*> dNotNulls;
		for ( XQNode_t* pChild : dChildren )
		{
			if ( pChild->GetOp()!=SPH_QUERY_NULL )
				dNotNulls.Add ( pChild );
			else
				DeleteSpawned ( pChild );
		}
		dChildren.SwapData ( dNotNulls );
		dNotNulls.Reset();
		// smth AND null = null.
	} else if ( pNode->GetOp()==SPH_QUERY_AND )
	{
		if ( dChildren.any_of (
			[] ( XQNode_t * pChild ) { return pChild->GetOp ()==SPH_QUERY_NULL; } ) )
		{
			pNode->SetOp ( SPH_QUERY_NULL );
			for ( auto &pChild : dChildren )
				DeleteSpawned ( pChild );
			dChildren.Reset ();
		}
	}} );
}

bool XQParseHelper_c::FixupNots ( XQNode_t * pNode, bool bOnlyNotAllowed, XQNode_t ** ppRoot )
{
	// no processing for plain nodes
	if ( !pNode || !pNode->dWords().IsEmpty() )
		return true;

	// query was already transformed
	if ( pNode->GetOp()==SPH_QUERY_ANDNOT )
		return true;

	// process 'em children
	CSphVector<XQNode_t *> dNots;
	bool bSuccess = true;
	pNode->WithChildren ( [bOnlyNotAllowed,ppRoot,this,&bSuccess,&dNots](auto& dChildren) {
		for ( auto & dNode: dChildren )
		{
			if ( FixupNots ( dNode, bOnlyNotAllowed, ppRoot ) )
				continue;

			bSuccess = false;
			return;
		}

		// extract NOT subnodes
		ARRAY_FOREACH ( i, dChildren )
			if ( dChildren[i]->GetOp() == SPH_QUERY_NOT )
			{
				dNots.Add ( dChildren[i] );
				dChildren.RemoveFast ( i-- );
			}
	} );
	if ( !bSuccess )
		return false;

	// no NOTs? we're square
	if ( !dNots.GetLength() )
		return true;

	// nothing but NOTs? we can't compute that
	if ( pNode->dChildren().IsEmpty() && !bOnlyNotAllowed )
		return Error ( "query is non-computable (node consists of NOT operators only)" );

	// NOT within OR or MAYBE? we can't compute that
	switch ( pNode->GetOp() )
	{
	case SPH_QUERY_OR: return Error ("query is non-computable (NOT is not allowed within OR)");
	case SPH_QUERY_MAYBE: return Error ("query is non-computable (NOT is not allowed within MAYBE)");
	case SPH_QUERY_NEAR: return Error ("query is non-computable (NOT is not allowed within NEAR)");
	case SPH_QUERY_NOTNEAR: return Error ("query is non-computable (NOT is not allowed within NOTNEAR)");
	default: break;
	}

	// NOT used in before operator
	if ( pNode->GetOp()==SPH_QUERY_BEFORE )
		return Error ( "query is non-computable (NOT cannot be used as before operand)" );

	// must be some NOTs within AND at this point, convert this node to ANDNOT
	assert ( ( ( pNode->GetOp()==SPH_QUERY_AND && pNode->dChildren().GetLength() )
		|| ( pNode->GetOp()==SPH_QUERY_AND && pNode->dChildren().IsEmpty() && bOnlyNotAllowed ) )
		&& dNots.GetLength() );

	if ( pNode->GetOp()==SPH_QUERY_AND && pNode->dChildren().IsEmpty() )
	{
		if ( !bOnlyNotAllowed )
			return Error ( "query is non-computable (node consists of NOT operators only)" );

		if ( !pNode->m_pParent )
		{
			pNode->ResetChildren (true);
			pNode->SetOp ( SPH_QUERY_OR, dNots );
			*ppRoot = TransformOnlyNot ( pNode, m_dSpawned );
			return true;
		}

		if ( /*pNode->m_pParent && */pNode->m_pParent->GetOp()==SPH_QUERY_AND && pNode->m_pParent->dChildren().GetLength()==2 )
		{
			pNode->m_pParent->SetOp ( SPH_QUERY_ANDNOT );
			pNode->ResetChildren (true);
			pNode->SetOp ( SPH_QUERY_OR, dNots );
			return true;
		}

		return Error ( "query is non-computable (node consists of NOT operators only)" );
	}

	XQNode_t * pAnd = SpawnNode ( pNode->m_dSpec );
	pNode->WithChildren ( [pAnd](auto& dChildren) { pAnd->SetOp ( SPH_QUERY_AND, dChildren ); } );

	XQNode_t * pNot = nullptr;
	if ( dNots.GetLength()==1 )
	{
		pNot = dNots[0];
	} else
	{
		pNot = SpawnNode ( pNode->m_dSpec );
		pNot->SetOp ( SPH_QUERY_OR, dNots );
	}

	pNode->ResetChildren();
	pNode->SetOp ( SPH_QUERY_ANDNOT, pAnd, pNot );
	return true;
}

void XQParseHelper_c::DeleteNodesWOFields ( XQNode_t * pNode )
{
	if ( !pNode )
		return;

//	Dump ( pNode, "DeleteNodesWOFields" );

	pNode->WithChildren([this](auto& dChildren) {
	for ( int i = 0; i < dChildren.GetLength (); ) // no ++i
	{
		XQNode_t * pChild = dChildren[i];
		if ( pChild->m_dSpec.m_dFieldMask.TestAll ( false ) )
		{
			const auto& dGrandChildren = pChild->dChildren();
#ifndef NDEBUG
			bool bAllEmpty = dGrandChildren.all_of ( [] ( XQNode_t * pChildNode ) { return pChildNode->m_dSpec.m_dFieldMask.TestAll ( false ); } );
			assert ( dGrandChildren.IsEmpty() || ( bAllEmpty && pChild->m_dSpec.m_dFieldMask.TestAll ( false )) );
#endif
			for ( auto& pGrand : dGrandChildren )
				m_dSpawned.RemoveValue ( pGrand );
			DeleteSpawned ( pChild );
			dChildren.RemoveFast ( i );
		} else
		{
			DeleteNodesWOFields ( dChildren[i] );
			++i;
		}
	} });
}


void XQParseHelper_c::FixupDestForms ()
{
	if ( !m_dMultiforms.GetLength() )
		return;

	CSphVector<XQNode_t *> dForms;

	for ( const MultiformNode_t & tDesc : m_dMultiforms )
	{
		XQNode_t * pMultiParent = tDesc.m_pNode;
		assert ( pMultiParent->dWords().GetLength()==1 && pMultiParent->dChildren().GetLength()==0 );

		XQKeyword_t tKeyword;
		pMultiParent->WithWords ( [&tKeyword] ( auto& dWords ) {
			Swap ( dWords[0], tKeyword );
			dWords.Reset();
		});

		// FIXME: what about whildcard?
		bool bExact = ( tKeyword.m_sWord.Length()>1 && tKeyword.m_sWord.cstr()[0]=='=' );
		bool bFieldStart = tKeyword.m_bFieldStart;
		bool bFieldEnd = tKeyword.m_bFieldEnd;
		tKeyword.m_bFieldStart = false;
		tKeyword.m_bFieldEnd = false;

		XQNode_t * pMultiHead = SpawnNode ( pMultiParent->m_dSpec );
		pMultiHead->AddDirtyWord ( tKeyword );
		dForms.Add ( pMultiHead );

		for ( int iForm=0; iForm<tDesc.m_iDestCount; ++iForm )
		{
			++tKeyword.m_iAtomPos;
			const auto& sWord =  m_dDestForms [ tDesc.m_iDestStart + iForm ];
			// propagate exact word flag to all destination forms
			if ( bExact )
				tKeyword.m_sWord.SetSprintf ( "=%s", sWord.cstr() );
			else
				tKeyword.m_sWord = sWord;

			XQNode_t * pMulti = SpawnNode ( pMultiParent->m_dSpec );
			pMulti->AddDirtyWord ( tKeyword );
			dForms.Add ( pMulti );
		}

		// fix-up field start\end modifier
		dForms[0]->dWord(0).m_bFieldStart = bFieldStart;
		dForms.Last()->dWord(0).m_bFieldEnd = bFieldEnd;

		assert ( pMultiParent->dChildren().IsEmpty() );
		pMultiParent->SetOp ( SPH_QUERY_AND, dForms );
		dForms.Resize ( 0 );
	}
}

void XQParseHelper_c::SetZone ( const StrVec_t & dZones ) const noexcept
{
	assert ( m_pParsed );
	m_pParsed->m_dZones = dZones;
}

const StrVec_t & XQParseHelper_c::GetZone() const noexcept
{
	assert ( m_pParsed );
	return m_pParsed->m_dZones;
}

XQNode_t * XQParseHelper_c::SpawnNode ( const XQLimitSpec_t & dSpec ) noexcept
{
	XQNode_t * pNode = new XQNode_t ( dSpec );
	m_dSpawned.Add ( pNode );
	return pNode;
}

void XQParseHelper_c::DeleteSpawned ( XQNode_t * pNode ) noexcept
{
	m_dSpawned.RemoveValue ( pNode ); // OPTIMIZE!
	pNode->WithChildren([this] ( auto & dChildren ) {
		for ( auto* pChild : dChildren )
			DeleteSpawned ( pChild );
	});
	pNode->ResetChildren();
	SafeDelete ( pNode );
}