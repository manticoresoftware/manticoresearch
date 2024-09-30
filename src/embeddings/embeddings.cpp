//
// Copyright (c) 2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "embeddings.h"
#include "sphinxutils.h"

/// content of manticore_text_embeddings.h

using TextModel = const void *;

using LoadModelFn = TextModel( * ) ( const char *, uintptr_t );

using DeleteModelFn = void ( * ) ( TextModel );

struct FloatVec
{
	float * ptr;
	uintptr_t len;
	uintptr_t cap;
};

using MakeVectEmbeddingsFn = FloatVec( * ) ( TextModel, const char *, uintptr_t );

using DeleteVecFn = void ( * ) ( FloatVec );

using GetLenFn = uintptr_t( * ) ( TextModel );

struct EmbeddLib
{
	uintptr_t size;
	LoadModelFn load_model;
	DeleteModelFn delete_model;
	MakeVectEmbeddingsFn make_vect_embeddings;
	DeleteVecFn delete_vec;
	GetLenFn get_hidden_size;
	GetLenFn get_max_input_size;
};

extern "C" {

EmbeddLib GetLibFuncs ();

} // extern "C"

/// finished content of manticore_text_embeddings.h

constexpr uintptr_t iExpectedSize = 0x38;
static EmbeddLib g_tLib;

std::unique_ptr<CSphDynamicLibrary> g_pEmbeddingsLib { nullptr };
CSphString g_sLoadedLib;

inline static bool LibLoaded()
{
	return !!g_pEmbeddingsLib;
}


class TextModel_c
{
	TextModel m_pModel = nullptr;

public:
	~TextModel_c()
	{
		FreeModel();
	}

	void SetModel ( const CSphString & sName )
	{
		if ( !LibLoaded () )
			return;

		if ( sName.IsEmpty() )
		{
			FreeModel();
			return;
		}

		m_pModel = g_tLib.load_model ( sName.cstr(), sName.Length() );
	}

	void FreeModel ()
	{
		if ( !m_pModel )
			return;

		assert ( LibLoaded() );
		g_tLib.delete_model ( m_pModel );
		m_pModel = nullptr;
	}

	int GetHiddenSize() const noexcept
	{
		if ( !m_pModel )
			return -1;

		assert ( LibLoaded () );
		return g_tLib.get_hidden_size ( m_pModel );
	}


	int GetMaxInputSize () const noexcept
	{
		if ( !m_pModel )
			return -1;

		assert ( LibLoaded () );
		return g_tLib.get_max_input_size ( m_pModel );
	}

	bool ModelLoaded () const noexcept
	{
		return m_pModel;
	}

	FloatVec MakeEmbeddings ( const CSphString & sText ) const
	{
		FloatVec tRes { nullptr,0,0 };
		assert ( ModelLoaded() );
		if ( !ModelLoaded() )
			return tRes;

		tRes = g_tLib.make_vect_embeddings ( m_pModel, sText.cstr(), sText.Length() );
		return tRes;
	}
};

static TextModel_c g_model;

class FloatVec_t
{
	FloatVec m_tVec;
public:
	FloatVec_t () : m_tVec { nullptr, 0, 0 }
	{
	}

	explicit FloatVec_t ( FloatVec&& tRhs )
		: m_tVec { std::move ( tRhs ) }
	{
	}

	~FloatVec_t()
	{
		if ( !m_tVec.ptr )
			return;
		assert ( LibLoaded () );
		g_tLib.delete_vec ( m_tVec );
	}


	CSphVector<float> CopyAsVec() const noexcept
	{
		CSphVector<float> dResult;
		if ( !m_tVec.ptr )
			return dResult;

		dResult.Reserve ( m_tVec.len );
		dResult.Append ( m_tVec.ptr, m_tVec.len );
		return dResult;
	}

	VecTraits_T<float> AsSlice() const noexcept
	{
		return { m_tVec.ptr, (int64_t) m_tVec.len };
	}
};

bool Embeddings::LoadLib ( const CSphString & sPath )
{
	if ( LibLoaded() )
		return true;

	if ( sPath.IsEmpty() || sPath==g_sLoadedLib )
		return true;

	decltype ( &GetLibFuncs ) get_lib_funcs = nullptr;
	const char * sFuncs[] = { "GetLibFuncs", };
	void ** pFuncs[] = { (void **) &get_lib_funcs, };

	auto pLib = std::make_unique<CSphDynamicLibrary> ( sPath.cstr() );
	if ( !pLib )
	{
		sphWarning ("Failed to load lib '%s'", sPath.cstr() );
		return false;
	}

	auto bReady = pLib->LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs )/sizeof ( void ** ) );
	if ( !bReady )
		return false;

	assert ( get_lib_funcs );

	g_tLib = get_lib_funcs();
	assert ( g_tLib.size==iExpectedSize );
	if ( g_tLib.size!=iExpectedSize )
	{
		g_pEmbeddingsLib = nullptr;
		return false;
	}

	g_sLoadedLib = sPath;
	g_pEmbeddingsLib = std::move ( pLib );
	return true;
}


std::pair<int, int> Embeddings::LoadModel ( const CSphString & sName )
{
	if ( !LibLoaded () )
		sphWarning ("Embedding library is not loaded");

	g_model.SetModel (sName);
	return { g_model.GetHiddenSize(), g_model.GetMaxInputSize() };
}


CSphVector<float> Embeddings::MakeEmbeddings ( const CSphString & sText )
{
	CSphVector<float> dResult;
	if ( !LibLoaded () )
	{
		sphWarning ( "Embedding library is not loaded" );
		return dResult;
	}

	if ( !g_model.ModelLoaded() )
	{
		sphWarning ( "Model is not loaded" );
		return dResult;
	}

	FloatVec_t tmpResult { g_model.MakeEmbeddings ( sText ) };
	dResult = tmpResult.CopyAsVec();
	return dResult;
}


void Embeddings::ProcessEmbeddings ( const CSphString & sText, fnProcessor fnProcess )
{
	if ( !LibLoaded () )
	{
		sphWarning ( "Embedding library is not loaded" );
		return;
	}

	if ( !g_model.ModelLoaded () )
	{
		sphWarning ( "Model is not loaded" );
		return;
	}

	FloatVec_t tmpResult { g_model.MakeEmbeddings ( sText ) };
	fnProcess ( tmpResult.AsSlice() );
}