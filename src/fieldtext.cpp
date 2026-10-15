//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "fieldtext.h"

#include "sphinx.h"
#include "indexsettings.h"
#include "stripper/html_stripper.h"


std::unique_ptr<CSphHTMLStripper> CreateIndexHtmlStripper ( const CSphIndexSettings & tSettings, CSphString & sError )
{
	auto pStripper = std::make_unique<CSphHTMLStripper> ( true, true );
	if ( !pStripper->SetIndexedAttrs ( tSettings.m_sHtmlIndexAttrs.cstr(), sError ) || !pStripper->SetRemovedElements ( tSettings.m_sHtmlRemoveElements.cstr(), sError ) )
	{
		sError.SetSprintf ( "HTML stripper config error: %s", sError.cstr() );
		return nullptr;
	}

	return pStripper;
}


bool FilterAndStripText ( const VecTraits_T<BYTE> & dSourceText, CSphVector<BYTE> & dDestText, ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, bool & bUseOriginal )
{
	bool bFilterApplied = false;
	if ( pFilter )
	{
		int iGot;
		if ( bUseOriginal )
			iGot = pFilter->Apply ( dSourceText, dDestText, false );
		else
		{
			CSphVector<BYTE> dTmp;
			dTmp.Append ( dSourceText );
			iGot = pFilter->Apply ( dTmp, dDestText, false );
		}

		if ( iGot )
		{
			dDestText.Resize(iGot);
			bFilterApplied = true;
			bUseOriginal = false;
		}
	}

	if ( pStripper )
	{
		if ( bUseOriginal )
		{
			dDestText.Resize(0);
			dDestText.Append ( dSourceText );
			bUseOriginal = false;
		}

		if ( !( dDestText.IsEmpty () || dDestText.Last ()=='\0' ) )
			dDestText.Add('\0');

		if ( !dDestText.IsEmpty() )
		{
			pStripper->Strip ( dDestText.Begin() );
			dDestText.Resize ( (int) strlen ( (const char*)dDestText.Begin() ) );
		}
	}

	return bFilterApplied;
}


void PrepareFieldText ( const VecTraits_T<const BYTE> & dSource, CSphVector<BYTE> & dPrepared, ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper )
{
	dPrepared.Resize(0);
	bool bUseOriginal = true;
	VecTraits_T<BYTE> dSourceText ( const_cast<BYTE*> ( dSource.Begin() ), dSource.GetLength() ); // read only, the filter takes a non-const view
	FilterAndStripText ( dSourceText, dPrepared, pFilter, pStripper, bUseOriginal );
	if ( bUseOriginal )
	{
		dPrepared.Resize(0);
		dPrepared.Append ( dSource );
	}
}
