//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "token_filter.h"

#include "schema/schema.h"
#include "sphinxplugin.h"

class PluginFilterTokenizer_c final: public CSphTokenFilter
{
protected:
	PluginTokenFilterRefPtr_c m_pFilter;  ///< plugin descriptor
	CSphString m_sOptions;				  ///< options string for the plugin init()
	void* m_pUserdata = nullptr;		  ///< userdata returned from by the plugin init()
	bool m_bGotExtra = false;			  ///< are we looping through extra tokens?
	int m_iPosDelta = 0;				  ///< position delta for the current token, see comments in GetToken()
	bool m_bWasBlended = false;			  ///< whether the last raw token was blended

	~PluginFilterTokenizer_c() final
	{
		if ( m_pFilter->m_fnDeinit )
			m_pFilter->m_fnDeinit ( m_pUserdata );
	}

public:
	PluginFilterTokenizer_c ( TokenizerRefPtr_c pTok, PluginTokenFilterRefPtr_c pFilter, const char* sOptions )
		: CSphTokenFilter ( std::move (pTok) )
		, m_pFilter ( std::move ( pFilter ) )
		, m_sOptions ( sOptions )
	{
		assert ( m_pFilter );
		// FIXME!!! handle error in constructor \ move to setup?
		CSphString sError;
		SetFilterSchema ( CSphSchema(), sError );
	}

	TokenizerRefPtr_c Clone ( ESphTokenizerClone eMode ) const noexcept final
	{
		return TokenizerRefPtr_c { new PluginFilterTokenizer_c ( m_pTokenizer->Clone ( eMode ), m_pFilter, m_sOptions.cstr() ) };
	}

	bool SetFilterSchema ( const CSphSchema& s, CSphString& sError ) final
	{
		if ( m_pUserdata && m_pFilter->m_fnDeinit )
			m_pFilter->m_fnDeinit ( m_pUserdata );

		CSphVector<const char*> dFields;
		for ( int i = 0; i < s.GetFieldsCount(); i++ )
			dFields.Add ( s.GetFieldName ( i ) );

		char sErrBuf[SPH_UDF_ERROR_LEN + 1];
		if ( m_pFilter->m_fnInit ( &m_pUserdata, dFields.GetLength(), dFields.Begin(), m_sOptions.cstr(), sErrBuf ) == 0 )
			return true;
		sError = sErrBuf;
		return false;
	}

	bool SetFilterOptions ( const char* sOptions, CSphString& sError ) final
	{
		char sErrBuf[SPH_UDF_ERROR_LEN + 1];
		if ( m_pFilter->m_fnBeginDocument ( m_pUserdata, sOptions, sErrBuf ) == 0 )
			return true;
		sError = sErrBuf;
		return false;
	}

	void BeginField ( int iField ) final
	{
		if ( m_pFilter->m_fnBeginField )
			m_pFilter->m_fnBeginField ( m_pUserdata, iField );
	}

	BYTE* GetToken() final
	{
		// we have two principal states here
		// a) have pending extra tokens, keep looping and returning those
		// b) no extras, keep pushing until plugin returns anything
		//
		// we also have to handle position deltas, and that story is a little tricky
		// positions are not assigned in the tokenizer itself (we might wanna refactor that)
		// however, tokenizer has some (partial) control over the keyword positions, too
		// when it skips some too-short tokens, it returns a non-zero value via GetOvershortCount()
		// when it returns a blended token, it returns true via TokenIsBlended()
		// so while the default position delta is 1, overshorts can increase it by N,
		// and blended flag can decrease it by 1, and that's under tokenizer's control
		//
		// so for the plugins, we simplify (well i hope!) this complexity a little
		// we compute a proper position delta here, pass it, and let the plugin modify it
		// we report all tokens as regular, and return the delta via GetOvershortCount()

		// state (a), just loop the pending extras
		if ( m_bGotExtra )
		{
			m_iPosDelta = 1; // default delta is 1
			BYTE* pTok = (BYTE*)m_pFilter->m_fnGetExtraToken ( m_pUserdata, &m_iPosDelta );
			GetBlended();
			if ( pTok )
				return pTok;
			m_bGotExtra = false;
		}

		// state (b), push raw tokens, return results
		for ( ;; )
		{
			// get next raw token, handle field end
			BYTE* pRaw = CSphTokenFilter::GetToken();
			if ( !pRaw )
			{
				// no more hits? notify plugin of a field end,
				// and check if there are pending tokens
				m_bGotExtra = 0;
				if ( m_pFilter->m_fnEndField )
					if ( !m_pFilter->m_fnEndField ( m_pUserdata ) )
					{
						m_bBlended = false;
						m_bBlendedPart = false;
						return NULL;
					}

				// got them, start fetching
				m_bGotExtra = true;
				BYTE* pTok = (BYTE*)m_pFilter->m_fnGetExtraToken ( m_pUserdata, &m_iPosDelta );
				GetBlended();
				return pTok;
			}

			// compute proper position delta
			m_iPosDelta = ( m_bWasBlended ? 0 : 1 ) + CSphTokenFilter::GetOvershortCount();
			m_bWasBlended = CSphTokenFilter::TokenIsBlended();

			// push raw token to plugin, return a processed one, if any
			int iExtra = 0;
			BYTE* pTok = (BYTE*)m_pFilter->m_fnPushToken ( m_pUserdata, (char*)pRaw, &iExtra, &m_iPosDelta );
			m_bGotExtra = ( iExtra != 0 );
			GetBlended();
			if ( pTok )
				return pTok;
		}
	}

	int GetOvershortCount() const noexcept final
	{
		return m_iPosDelta - 1;
	}

private:
	void GetBlended()
	{
		if ( m_pFilter->m_fnTokenIsBlended )
			m_bBlended = ( !!m_pFilter->m_fnTokenIsBlended ( m_pUserdata ) );
		if ( m_pFilter->m_fnTokenIsBlendedPart )
			m_bBlendedPart = ( !!m_pFilter->m_fnTokenIsBlendedPart ( m_pUserdata ) );
	}
};


void Tokenizer::AddPluginFilterTo ( TokenizerRefPtr_c& pTokenizer, const CSphString& sSpec, CSphString& sError )
{
	StrVec_t dPlugin; // dll, filtername, options
	if ( !sphPluginParseSpec ( sSpec, dPlugin, sError ) )
		return;

	if ( dPlugin.IsEmpty() )
		return;

	PluginTokenFilterRefPtr_c p = PluginAcquire<PluginTokenFilter_c> ( dPlugin[0].cstr(), PLUGIN_INDEX_TOKEN_FILTER, dPlugin[1].cstr(), sError );
	if ( !p )
	{
		sError.SetSprintf ( "INTERNAL ERROR: plugin %s:%s loaded ok but lookup fails, error: %s", dPlugin[0].cstr(), dPlugin[1].cstr(), sError.cstr() );
		return;
	}
	pTokenizer = new PluginFilterTokenizer_c ( std::move ( pTokenizer ), std::move (p), dPlugin[2].cstr() );
}
