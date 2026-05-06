#pragma once

#include "sphinxstd.h"
#include "searchdaemon.h"

struct EmbeddedAsset_t
{
	const char *	m_szPath;
	const unsigned char * m_pData;
	int				m_iSize;
	const char *	m_szContentType;
};

#if WITH_UI
extern const EmbeddedAsset_t g_dUIAssets[];
extern const int g_iUIAssetCount;
#endif

// Try to serve an embedded UI asset for the given endpoint path.
// Returns true if the request was handled (dResult populated), false otherwise.
bool HttpServeUI ( const CSphString & sEndpoint, CSphVector<BYTE> & dResult, bool bHeadReply );

extern bool g_bUI;
