#include "searchdui.h"
#include "searchdhttp.h"

bool g_bUI = false;

#if WITH_UI

static const EmbeddedAsset_t * FindAsset ( const char * szPath )
{
	for ( int i = 0; i < g_iUIAssetCount; ++i )
	{
		if ( strcmp ( g_dUIAssets[i].m_szPath, szPath ) == 0 )
			return &g_dUIAssets[i];
	}
	return nullptr;
}

static const EmbeddedAsset_t * FindIndexHtml()
{
	return FindAsset ( "index.html" );
}

static void BuildAssetReply ( const EmbeddedAsset_t * pAsset, CSphVector<BYTE> & dResult, bool bHeadReply, bool bCacheable )
{
	const char * sCacheControl = bCacheable
		? "public, max-age=31536000, immutable"
		: "no-cache";

	CSphString sHttp;
	sHttp.SetSprintf (
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: %s; charset=UTF-8\r\n"
		"Content-Length: %d\r\n"
		"Cache-Control: %s\r\n"
		"\r\n",
		pAsset->m_szContentType,
		pAsset->m_iSize,
		sCacheControl );

	int iHeaderLen = sHttp.Length();
	int iBufLen = iHeaderLen;
	if ( !bHeadReply )
		iBufLen += pAsset->m_iSize;
	dResult.Resize ( iBufLen );
	memcpy ( dResult.Begin(), sHttp.cstr(), iHeaderLen );
	if ( !bHeadReply )
		memcpy ( dResult.Begin() + iHeaderLen, pAsset->m_pData, pAsset->m_iSize );
}

bool HttpServeUI ( const CSphString & sEndpoint, CSphVector<BYTE> & dResult, bool bHeadReply )
{
	if ( !g_bUI )
		return false;

	// Must start with "ui" prefix
	if ( !sEndpoint.Begins ( "ui" ) )
		return false;

	// Extract sub-path after "ui/" or just "ui"
	const char * szSubPath = sEndpoint.cstr() + 2;
	if ( *szSubPath == '/' )
		++szSubPath;

	// Empty sub-path or "index.html" -> serve SPA entry point
	if ( *szSubPath == '\0' || strcmp ( szSubPath, "index.html" ) == 0 )
	{
		const auto * pIndex = FindIndexHtml();
		if ( pIndex )
		{
			BuildAssetReply ( pIndex, dResult, bHeadReply, false );
			return true;
		}
		return false;
	}

	// Try to find the exact asset
	const auto * pAsset = FindAsset ( szSubPath );
	if ( pAsset )
	{
		// Assets with hashed names are immutable-cacheable
		bool bCacheable = ( strstr ( szSubPath, "assets/" ) == szSubPath );
		BuildAssetReply ( pAsset, dResult, bHeadReply, bCacheable );
		return true;
	}

	// SPA fallback: serve index.html for any unrecognized path under /ui/
	const auto * pIndex = FindIndexHtml();
	if ( pIndex )
	{
		BuildAssetReply ( pIndex, dResult, bHeadReply, false );
		return true;
	}

	return false;
}

#else // !WITH_UI

bool HttpServeUI ( const CSphString &, CSphVector<BYTE> &, bool )
{
	return false;
}

#endif // WITH_UI
