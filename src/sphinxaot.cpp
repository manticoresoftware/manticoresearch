//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
// Based on AOT lemmatizer, http://aot.ru/
// Copyright (c) 2004-2014, Alexey Sokirko and others
//

#include "sphinxint.h"
#include "fileutils.h"
#include "dict/stem/sphinxstem.h"
#include "tokenizer/token_filter.h"
#include "dict/word_forms.h"
#include "lz4/lz4.h"
#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// LEMMATIZER
//////////////////////////////////////////////////////////////////////////

const BYTE	AOT_POS_UNKNOWN				= 0xff;
const int	AOT_MIN_PREDICTION_SUFFIX	= 3;
const BYTE	AOT_MORPH_ANNOT_CHAR		= '+';
const int	AOT_MAX_ALPHABET_SIZE		= 54;
const DWORD	AOT_NOFORM					= 0xffffffffUL;
const DWORD	AOT_ORIGFORM				= 0xfffffffeUL;

static int	g_iCacheSize				= 262144; // in bytes, so 256K
static constexpr int UK_MAX_ASSET_STRING = 1024;
static constexpr DWORD UK_LEMMATIZER_VERSION = 3;
static constexpr std::array<char,8> UK_LEMMATIZER_MAGIC { 'M', 'S', 'U', 'K', 'L', 'E', 'M', '1' };
static constexpr DWORD UK_SECTION_STRINGS = 1;
static constexpr DWORD UK_SECTION_EXACT = 2;
static constexpr DWORD UK_SECTION_PREDICTIONS = 3;
static constexpr DWORD UK_CODEC_NONE = 0;
static constexpr DWORD UK_CODEC_LZ4 = 1;
static constexpr int UK_MAX_ANALOGY_DEPTH = 4;
static constexpr const char * UK_CHAR_SUBSTITUTE_TO = "ґ";
static constexpr std::array<const char *,4> UK_PARTICLES_AFTER_HYPHEN {
	"-но",
	"-таки",
	"-бо",
	"-от"
};
static constexpr std::array<const char *,10> UK_KNOWN_PREFIXES {
	"інтернет-",
	"псевдо-",
	"псевдо",
	"мега-",
	"мега",
	"гіпер-",
	"гіпер",
	"супер-",
	"супер",
	"міні"
};

struct UkSectionDesc_t
{
	DWORD		m_uType = 0;
	DWORD		m_uCodec = 0;
	uint64_t	m_uOffset = 0;
	uint64_t	m_uPackedSize = 0;
	uint64_t	m_uUnpackedSize = 0;
	DWORD		m_uFlags = 0;
	DWORD		m_uReserved = 0;
};

class UkMemoryReader_c
{
public:
	explicit UkMemoryReader_c ( const VecTraits_T<BYTE> & dData )
		: m_pData ( dData.Begin() )
		, m_iLength ( dData.GetLength() )
	{}

	bool GetDword ( DWORD & uValue )
	{
		if ( m_iPos + (int)sizeof(uValue) > m_iLength )
			return false;

		memcpy ( &uValue, m_pData + m_iPos, sizeof(uValue) );
		m_iPos += sizeof(uValue);
		return true;
	}

	bool GetBytes ( void * pDst, int iLen )
	{
		if ( iLen<0 || m_iPos + iLen > m_iLength )
			return false;

		if ( iLen )
			memcpy ( pDst, m_pData + m_iPos, iLen );
		m_iPos += iLen;
		return true;
	}

	bool GetString ( CSphString & sValue )
	{
		DWORD uLen = 0;
		if ( !GetDword ( uLen ) || uLen>UK_MAX_ASSET_STRING )
			return false;

		sValue.Reserve ( (int)uLen + 1 );
		if ( uLen && !GetBytes ( (void*)sValue.cstr(), uLen ) )
			return false;

		((char*)sValue.cstr())[uLen] = '\0';
		return true;
	}

private:
	const BYTE *	m_pData = nullptr;
	int				m_iLength = 0;
	int				m_iPos = 0;
};


#define		AOT_MODEL_NO(_a)	((_a)>>18)
#define		AOT_ITEM_NO(_a)		(((_a)&0x3FFFF)>>9)
#define		AOT_PREFIX_NO(_a)	((_a)&0x1FF)


/// morpohological form info
struct CMorphForm
{
	BYTE		m_FlexiaLen;
	BYTE		m_PrefixLen;
	BYTE		m_POS;
	BYTE		m_Dummy;
	char		m_Prefix[4];
	char		m_Flexia[24];
};


/// alphabet descriptor
struct AlphabetDesc_t
{
	int			m_iSize;
	BYTE		m_dCode2Alpha [ AOT_MAX_ALPHABET_SIZE ];
	BYTE		m_dCode2AlphaWA [ AOT_MAX_ALPHABET_SIZE ];
};


/// alphabet codec
class CABCEncoder : public ISphNoncopyable
{
public:
	int			m_AlphabetSize;
	int			m_Alphabet2Code[256];
	int			m_Alphabet2CodeWithoutAnnotator[256];

	void		InitAlphabet ( const AlphabetDesc_t & tDesc );
	bool		CheckABCWithoutAnnotator ( const BYTE * pWord ) const;
	DWORD		DecodeFromAlphabet ( const BYTE * sPath, int iPath ) const;
};

/// morphology automaton node, 1:31
/// 1 bit for "final or not" flag
/// 31 bits for index to relations (pointer to the first child)
struct CMorphAutomNode
{
	DWORD		m_Data;
	DWORD		GetChildrenStart() const	{ return m_Data&(0x80000000-1); }
	bool		IsFinal() const				{ return (m_Data&0x80000000) > 0; }
};


/// morphology automaton relation, 8:24
/// 8 bits for relational char (aka next char in current form)
/// 24 bites for index to nodes (pointer to the next level node)
struct CMorphAutomRelation
{
	DWORD		m_Data;
	DWORD		GetChildNo() const			{ return m_Data & 0xffffff; }
	BYTE		GetRelationalChar() const	{ return (BYTE)(m_Data>>24); }
};


/// morphology automaton
class CMorphAutomat : public CABCEncoder
{
protected:
	CMorphAutomNode *		m_pNodes;
	int						m_NodesCount;
	CMorphAutomRelation *	m_pRelations;
	int						m_RelationsCount;

	int						m_iCacheSize;
	CSphTightVector<int>	m_ChildrenCache;

	void	BuildChildrenCache ( int iCacheSize );
	int		FindStringAndPassAnnotChar ( const BYTE * pText ) const;

public:
	CMorphAutomat ()
		: m_pNodes ( NULL )
		, m_NodesCount ( 0 )
		, m_pRelations ( NULL )
		, m_RelationsCount ( 0 )
		, m_iCacheSize ( 0 )
	{}

	~CMorphAutomat ()
	{
		SafeDeleteArray ( m_pNodes );
		SafeDeleteArray ( m_pRelations );
	}

	int								GetChildrenCount ( int i ) const	{ return m_pNodes[i+1].GetChildrenStart() - m_pNodes[i].GetChildrenStart(); }
	const CMorphAutomRelation *		GetChildren ( int i ) const			{ return m_pRelations + m_pNodes[i].GetChildrenStart(); }
	const CMorphAutomNode			GetNode ( int i ) const				{ return m_pNodes[i]; }

public:
	bool	LoadPak ( CSphReader & rd, int iCacheSize );
	void	GetInnerMorphInfos ( const BYTE * pText, DWORD * Infos ) const;
	int		NextNode ( int NodeNo, BYTE Child ) const;
};


/// prediction data tuple
struct CPredictTuple
{
	WORD				m_ItemNo;
	DWORD				m_LemmaInfoNo;
	BYTE				m_PartOfSpeechNo;
};


/// flexia model is basically a vector of morphology forms
/// (there is other meta suff like per-model comments but that is now stripped)
typedef CSphVector<CMorphForm> CFlexiaModel;

class NativeUkLemmatizer_c
{
public:
	bool Load ( CSphReader & rd, CSphString & sError );
	bool Lookup ( const BYTE * pWord, CSphVector<CSphString> & dLemmas ) const;

private:
	struct PredictionRule_t
	{
		CSphString m_sRequiredPrefix;
		CSphString m_sFixedSuffix;
		CSphString m_sCurrentPrefix;
		CSphString m_sCurrentSuffix;
		CSphString m_sNormalPrefix;
		CSphString m_sNormalSuffix;
		DWORD m_uCount = 0;
	};

	bool LoadV3 ( CSphReader & rd, CSphString & sError );
	bool LookupWord ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const;
	bool LookupExact ( const std::string & sWord, CSphVector<CSphString> & dLemmas ) const;
	bool LookupPrediction ( const std::string & sWord, CSphVector<CSphString> & dLemmas ) const;
	bool LookupHyphenParticle ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const;
	bool LookupHyphenatedWord ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const;
	bool LookupKnownPrefix ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const;
	static bool ReadSection ( CSphReader & rd, const UkSectionDesc_t & tSection, CSphVector<BYTE> & dOut, CSphString & sError );
	static const UkSectionDesc_t * FindSection ( const CSphVector<UkSectionDesc_t> & dSections, DWORD uType );
	static const CSphString * GetStringById ( const CSphVector<CSphString> & dStrings, DWORD uId, const char * sWhat, CSphString & sError );
	static void AddUniqueLemma ( std::vector<CSphString> & dLemmas, const CSphString & sLemma );
	static void AddUniqueLemma ( CSphVector<CSphString> & dLemmas, const char * sLemma );
	static void AddAffixedLemmas ( const CSphVector<CSphString> & dBaseLemmas, const char * sPrefix, const char * sSuffix, CSphVector<CSphString> & dLemmas );
	static void AddHyphenatedLemmas ( const char * sLeft, const CSphVector<CSphString> & dRightLemmas, CSphVector<CSphString> & dLemmas );
	static bool StartsWith ( const std::string & sValue, const char * sPrefix );
	static bool EndsWith ( const std::string & sValue, const char * sSuffix );
	static std::string ApplyUkCharSubstitute ( const std::string & sWord );
	static bool HasKnownDashPrefix ( const std::string & sWord );

	std::unordered_map<std::string,std::vector<CSphString>> m_mLemmas;
	std::unordered_map<std::string,std::vector<PredictionRule_t>> m_mPredictionRules;
	DWORD m_uPredictionMinWordLen = 4;
	DWORD m_uPredictionMaxSuffixLen = 5;
};


/// lemmatizer
class CLemmatizer
{
protected:
	static constexpr int		MAX_PREFIX_LEN = 12;
	static constexpr bool		m_bMaximalPrediction = false;
	bool						m_bIsGerman;

	BYTE						m_UC[256];

	CMorphAutomat				m_FormAutomat;
	CSphVector<WORD>			m_LemmaFlexiaModel;		///< lemma id to flexia model id mapping
	CSphVector<BYTE>			m_NPSs;
	int							m_PrefixLen [ MAX_PREFIX_LEN ];
	CSphVector<BYTE>			m_PrefixBlob;

	CMorphAutomat				m_SuffixAutomat;
	CSphVector<DWORD>			m_ModelFreq;

	bool						IsPrefix ( const BYTE * sPrefix, int iLen ) const;

	DWORD						PredictPack ( const CPredictTuple & t ) const		{ return ( m_LemmaFlexiaModel [ t.m_LemmaInfoNo ]<<18 ) + ( t.m_ItemNo<<9 ); }
	bool						PredictFind ( const BYTE * pWord, int iLen, CSphVector<CPredictTuple> & res ) const;
	void						PredictFindRecursive ( int r, BYTE * sPath, int iPath, CSphVector<CPredictTuple> & Infos ) const;
	void						PredictByDataBase ( const BYTE * pWord, int iLen, DWORD * results, bool is_cap ) const;

public:
	explicit CLemmatizer ( bool IsGerman = false )
		: m_bIsGerman ( IsGerman )
		, m_iLang ( 0 )
	{}
	CSphVector<CFlexiaModel>	m_FlexiaModels;			///< flexia models
	int							m_iLang;					///< my language

	bool						LemmatizeWord ( BYTE * pWord, DWORD * results ) const;
	bool						LoadPak ( CSphReader & rd );
};

//////////////////////////////////////////////////////////////////////////

DWORD CABCEncoder::DecodeFromAlphabet ( const BYTE * sPath, int iPath ) const
{
	DWORD c = 1;
	DWORD Result = 0;
	for ( const BYTE * sMax = sPath+iPath; sPath<sMax; sPath++ )
	{
		Result += m_Alphabet2CodeWithoutAnnotator[*sPath] * c;
		c *= m_AlphabetSize - 1;
	}
	return Result;
}


bool CABCEncoder::CheckABCWithoutAnnotator ( const BYTE * pWord ) const
{
	while ( *pWord )
		if ( m_Alphabet2CodeWithoutAnnotator [ *pWord++ ]==-1 )
			return false;
	return true;
}


void CABCEncoder::InitAlphabet ( const AlphabetDesc_t & tDesc )
{
	m_AlphabetSize = tDesc.m_iSize;
	for ( int i=0; i<256; i++ )
	{
		m_Alphabet2Code[i] = -1;
		m_Alphabet2CodeWithoutAnnotator[i] = -1;
	}
	for ( int i=0; i<m_AlphabetSize; i++ )
		m_Alphabet2Code [ tDesc.m_dCode2Alpha[i] ] = i;
	for ( int i=0; i<m_AlphabetSize-1; i++ )
		m_Alphabet2CodeWithoutAnnotator [ tDesc.m_dCode2AlphaWA[i] ] = i;
}

//////////////////////////////////////////////////////////////////////////

void CMorphAutomat::BuildChildrenCache ( int iCacheSize )
{
	iCacheSize /= AOT_MAX_ALPHABET_SIZE*4;
	iCacheSize = Max ( iCacheSize, 0 );
	m_iCacheSize = Min ( m_NodesCount, iCacheSize );

	m_ChildrenCache.Resize ( m_iCacheSize*AOT_MAX_ALPHABET_SIZE );
	m_ChildrenCache.Fill ( -1 );
	for ( int NodeNo=0; NodeNo<m_iCacheSize; NodeNo++ )
	{
		const CMorphAutomRelation * pStart = m_pRelations + m_pNodes [ NodeNo ].GetChildrenStart();
		const CMorphAutomRelation * pEnd = pStart + GetChildrenCount ( NodeNo );
		for ( ; pStart!=pEnd; pStart++ )
		{
			const CMorphAutomRelation & p = *pStart;
			m_ChildrenCache [ NodeNo*AOT_MAX_ALPHABET_SIZE + m_Alphabet2Code [ p.GetRelationalChar() ] ] = p.GetChildNo();
		}
	}
}


bool CMorphAutomat::LoadPak ( CSphReader & rd, int iCacheSize )
{
	rd.Tag ( "automaton-nodes" );
	m_NodesCount = rd.UnzipInt();
	m_pNodes = new CMorphAutomNode [ m_NodesCount+1 ];
	rd.GetBytes ( m_pNodes, m_NodesCount*sizeof(CMorphAutomNode) );

	rd.Tag ( "automaton-relations" );
	m_RelationsCount = rd.UnzipInt();
	m_pRelations = new CMorphAutomRelation [ m_RelationsCount ];
	rd.GetBytes ( m_pRelations, m_RelationsCount*sizeof(CMorphAutomRelation) );

	if ( rd.GetErrorFlag() )
		return false;

	m_pNodes [ m_NodesCount ].m_Data = m_RelationsCount;

#if !USE_LITTLE_ENDIAN
	for ( int i=0; i< m_NodesCount; ++i )
		FlipEndianness ( &m_pNodes[i].m_Data );
	for ( int i=0; i< m_RelationsCount; ++i )
		FlipEndianness ( &m_pRelations[i].m_Data );
#endif

	BuildChildrenCache ( iCacheSize );
	return true;
}


int CMorphAutomat::NextNode ( int NodeNo, BYTE RelationChar ) const
{
	if ( NodeNo<m_iCacheSize )
	{
		int z = m_Alphabet2Code [ RelationChar ];
		if ( z==-1 )
			return -1;
		return m_ChildrenCache [ NodeNo*AOT_MAX_ALPHABET_SIZE + z ];
	} else
	{
		const CMorphAutomRelation * pStart = m_pRelations + m_pNodes [ NodeNo ].GetChildrenStart();
		const CMorphAutomRelation * pEnd = pStart + GetChildrenCount ( NodeNo );
		for ( ; pStart!=pEnd; pStart++ )
		{
			const CMorphAutomRelation & p = *pStart;
			if ( RelationChar==p.GetRelationalChar() )
				return p.GetChildNo();
		}
		return -1;
	}
}


int	CMorphAutomat::FindStringAndPassAnnotChar ( const BYTE * pText ) const
{
	int r = 0;
	while ( *pText )
	{
		int nd = NextNode ( r, *pText++ );
		if ( nd==-1 )
			return -1;
		r = nd;
	}
	return NextNode ( r, AOT_MORPH_ANNOT_CHAR ); // passing annotation char
}


void CMorphAutomat::GetInnerMorphInfos ( const BYTE * pText, DWORD * Infos ) const
{
	*Infos = AOT_NOFORM;

	int r = FindStringAndPassAnnotChar ( pText );
	if ( r==-1 )
		return;

	// recursively get all interpretations
	const int MAX_DEPTH = 32;
	int iLevel = 0;
	BYTE sPath[MAX_DEPTH];
	int iChild[MAX_DEPTH];
	int iChildMax[MAX_DEPTH];

	iChild[0] = m_pNodes[r].GetChildrenStart();
	iChildMax[0] = m_pNodes[r+1].GetChildrenStart();

	while ( iLevel>=0 )
	{
		while ( iChild[iLevel]<iChildMax[iLevel] )
		{
			CMorphAutomRelation Rel = m_pRelations[iChild[iLevel]];
			int NodeNo = Rel.GetChildNo();
			sPath[iLevel] = Rel.GetRelationalChar();
			iChild[iLevel]++;
			if ( m_pNodes[NodeNo].IsFinal() )
			{
				*Infos++ = DecodeFromAlphabet ( sPath, iLevel+1 );
			} else
			{
				iLevel++;
				assert ( iLevel<MAX_DEPTH );
				iChild[iLevel] = m_pNodes[NodeNo].GetChildrenStart();
				iChildMax[iLevel] = m_pNodes[NodeNo+1].GetChildrenStart();
			}
		}
		iLevel--;
	}
	*Infos = AOT_NOFORM;
}

//////////////////////////////////////////////////////////////////////////

void CLemmatizer::PredictFindRecursive ( int NodeNo, BYTE * sPath, int iPath, CSphVector<CPredictTuple> & Infos ) const
{
	const CMorphAutomNode & N = m_SuffixAutomat.GetNode ( NodeNo );
	if ( N.IsFinal() )
	{
		int i = 0;
		while ( i<iPath && sPath[i]!=AOT_MORPH_ANNOT_CHAR )
			i++;

		int j = i+1;
		while ( j<iPath && sPath[j]!=AOT_MORPH_ANNOT_CHAR )
			j++;

		int k = j+1;
		while ( k<iPath && sPath[k]!=AOT_MORPH_ANNOT_CHAR )
			k++;

		CPredictTuple & A = Infos.Add();
		A.m_PartOfSpeechNo = (BYTE) m_SuffixAutomat.DecodeFromAlphabet ( sPath+i+1, j-i-1 );
		A.m_LemmaInfoNo = m_SuffixAutomat.DecodeFromAlphabet ( sPath+j+1, k-j-1 );
		A.m_ItemNo = (WORD) m_SuffixAutomat.DecodeFromAlphabet ( sPath+k+1, iPath-k-1 );
	}

	int Count = m_SuffixAutomat.GetChildrenCount ( NodeNo );
	for ( int i=0; i<Count; i++ )
	{
		const CMorphAutomRelation & p = m_SuffixAutomat.GetChildren ( NodeNo )[i];
		sPath[iPath] = p.GetRelationalChar();
		PredictFindRecursive ( p.GetChildNo(), sPath, iPath+1, Infos );
	}
}


bool CLemmatizer::PredictFind ( const BYTE * pWord, int iLen, CSphVector<CPredictTuple> & res ) const
{
	// FIXME? we might not want to predict words with annot char inside
	// was: if (ReversedWordForm.find(AnnotChar) != string::npos) return false;

	int r = 0;
	int i = 0;
	const BYTE * p = pWord + iLen;
	for ( ; i<iLen; i++ )
	{
		int nd = m_SuffixAutomat.NextNode ( r, *--p );
		if ( nd==-1 )
			break;
		r = nd;
	}

	// no prediction by suffix which is less than 3
	if ( i<AOT_MIN_PREDICTION_SUFFIX )
		return false;

	assert ( r!=-1 );
	BYTE sPath[128] = {0};
	PredictFindRecursive ( r, sPath, 0, res );
	return true;
}


bool CLemmatizer::IsPrefix ( const BYTE * sPrefix, int iLen ) const
{
	// empty prefix is a prefix
	if ( !iLen )
		return true;
	if ( iLen>=MAX_PREFIX_LEN || m_PrefixLen[iLen]<0 )
		return false;

	const BYTE * p = &m_PrefixBlob [ m_PrefixLen[iLen] ];
	while ( *p==iLen )
	{
		if ( !memcmp ( p+1, sPrefix, iLen ) )
			return true;
		p += 1+iLen;
	}
	return false;
}


/// returns true if matched in dictionary, false if predicted
bool CLemmatizer::LemmatizeWord ( BYTE * pWord, DWORD * results ) const
{
	constexpr bool bCap = false; // maybe when we manage to drag this all the way from tokenizer
	constexpr bool bPredict = true;

	// uppercase (and maybe other translations), check, and compute length
	BYTE * p;
	if ( m_iLang==AOT_RU )
	{
		for ( p = pWord; *p; p++ )
		{
			BYTE b = m_UC[*p];
			// russian chars are in 0xC0..0xDF range
			// avoid lemmatizing words with other chars in them
			if ( ( b>>5 )!=6 )
			{
				*results = AOT_NOFORM;
				return false;
			}
			// uppercase
			*p = b;
		}
	} else ///< use the alphabet to reduce another letters
	{
		for ( p = pWord; *p; p++ )
		{
			BYTE b = m_UC[*p];
			// english chars are in 0x61..0x7A range
			// avoid lemmatizing words with other chars in them
			if ( m_FormAutomat.m_Alphabet2CodeWithoutAnnotator[b]<0 )
			{
				*results = AOT_NOFORM;
				return false;
			}
			// uppercase
			*p = b;
		}
	}

	int iLen = (int)( p-pWord );

	// do dictionary lookup
	m_FormAutomat.GetInnerMorphInfos ( pWord, results );
	if ( *results!=AOT_NOFORM )
		return true;
	if_const ( !bPredict )
		return false;

	// attempt prediction by keyword suffix
	// find the longest suffix that finds dictionary results
	// require that suffix to be 4+ chars too
	int iSuffix;
	for ( iSuffix=1; iSuffix<=iLen-4; iSuffix++ )
	{
		m_FormAutomat.GetInnerMorphInfos ( pWord+iSuffix, results );
		if ( *results!=AOT_NOFORM )
			break;
	}

	// cancel suffix predictions with no hyphens, short enough
	// known postfixes, and unknown prefixes
	if ( pWord [ iSuffix-1 ]!='-'
		&& ( iLen-iSuffix )<6
		&& !IsPrefix ( pWord, iSuffix ) )
	{
		*results = AOT_NOFORM;
	}

	// cancel predictions by pronouns, eg [Sem'ykin'ym]
	for ( DWORD * pRes=results; *pRes!=AOT_NOFORM; pRes++ )
		if ( m_NPSs[ AOT_MODEL_NO ( *pRes ) ]==AOT_POS_UNKNOWN )
	{
		*results = AOT_NOFORM;
		break;
	}

	// what, still no results?
	if ( *results==AOT_NOFORM )
	{
		// attempt prediction by database
		PredictByDataBase ( pWord, iLen, results, bCap );

		// filter out too short flexias
		DWORD * s = results;
		DWORD * d = s;
		while ( *s!=AOT_NOFORM )
		{
			const CMorphForm & F = m_FlexiaModels [ AOT_MODEL_NO(*s) ][ AOT_ITEM_NO(*s) ];
			if ( F.m_FlexiaLen<iLen )
				*d++ = *s;
			s++;
		}
		*d = AOT_NOFORM;
	}

	return false;
}


void CLemmatizer::PredictByDataBase ( const BYTE * pWord, int iLen, DWORD * FindResults, bool is_cap ) const
{
	// FIXME? handle all-consonant abbreviations anyway?
	// was: if ( CheckAbbreviation ( InputWordStr, FindResults, is_cap ) ) return;

	assert ( *FindResults==AOT_NOFORM );
	DWORD * pOut = FindResults;
	CSphVector<CPredictTuple> res;

	// if the ABC is wrong this prediction yields too many variants
	if ( m_FormAutomat.CheckABCWithoutAnnotator ( pWord ) )
		PredictFind ( pWord, iLen, res );

	// assume not more than 32 different pos
	int has_nps[32];
	for ( int i=0; i<32; i++ )
		has_nps[i] = -1;

	for ( const auto& tPredict : res )
	{
		BYTE PartOfSpeechNo = tPredict.m_PartOfSpeechNo;
		if ( !m_bMaximalPrediction && has_nps[PartOfSpeechNo]!=-1 )
		{
			int iOldFreq = m_ModelFreq [ AOT_MODEL_NO ( FindResults[has_nps[PartOfSpeechNo]] ) ];
			int iNewFreq = m_ModelFreq [ m_LemmaFlexiaModel [tPredict.m_LemmaInfoNo ] ];
			if ( iOldFreq < iNewFreq )
				FindResults [ has_nps [ PartOfSpeechNo ] ] = PredictPack ( tPredict );
			continue;
		}

		has_nps [ PartOfSpeechNo ] = (int)( pOut-FindResults );
		*pOut++ = PredictPack ( tPredict );
		*pOut = AOT_NOFORM;
	}

	if	( has_nps[0]==-1 // no noun
		|| ( is_cap && !m_bIsGerman ) ) // or can be a proper noun (except German, where all nouns are written uppercase)
	{
		static BYTE CriticalNounLetterPack[4] = "+++";
		PredictFind ( CriticalNounLetterPack, AOT_MIN_PREDICTION_SUFFIX, res );
		*pOut++ = PredictPack ( res.Last() );
		*pOut = AOT_NOFORM;
	}
}


bool CLemmatizer::LoadPak ( CSphReader & rd )
{
	rd.Tag ( "sphinx-aot" );
	int iVer = rd.UnzipInt();
	if ( iVer!=1 )
		return false;

	rd.Tag ( "alphabet-desc" );
	AlphabetDesc_t tDesc;
	tDesc.m_iSize = rd.UnzipInt();
	rd.GetBytes ( tDesc.m_dCode2Alpha, tDesc.m_iSize );
	rd.GetBytes ( tDesc.m_dCode2AlphaWA, tDesc.m_iSize );

	m_FormAutomat.InitAlphabet ( tDesc );
	m_SuffixAutomat.InitAlphabet ( tDesc );

	rd.Tag ( "uc-table" );
	rd.GetBytes ( m_UC, 256 );

	// caching forms can help a lot (from 4% with 256K cache to 13% with 110M cache)
	rd.Tag ( "forms-automaton" );
	m_FormAutomat.LoadPak ( rd, g_iCacheSize );

	rd.Tag ( "flexia-models" );
	m_FlexiaModels.Resize ( rd.UnzipInt() );
	ARRAY_FOREACH ( i, m_FlexiaModels )
	{
		m_FlexiaModels[i].Resize ( rd.UnzipInt() );
		ARRAY_FOREACH ( j, m_FlexiaModels[i] )
		{
			CMorphForm & F = m_FlexiaModels[i][j];
			F.m_FlexiaLen = (BYTE) rd.GetByte();
			rd.GetBytes ( F.m_Flexia, F.m_FlexiaLen );
			F.m_PrefixLen = (BYTE) rd.GetByte();
			rd.GetBytes ( F.m_Prefix, F.m_PrefixLen );
			F.m_POS = (BYTE) rd.GetByte();

			assert ( F.m_FlexiaLen<sizeof(F.m_Flexia) );
			assert ( F.m_PrefixLen<sizeof(F.m_Prefix) );
			F.m_Flexia[F.m_FlexiaLen] = 0;
			F.m_Prefix[F.m_PrefixLen] = 0;
		}
	}

	rd.Tag ( "prefixes" );
	for ( int i=0; i<MAX_PREFIX_LEN; i++ )
		m_PrefixLen[i] = rd.UnzipInt();
	m_PrefixBlob.Resize ( rd.UnzipInt() );
	rd.GetBytes ( m_PrefixBlob.Begin(), m_PrefixBlob.GetLength() );

	rd.Tag ( "lemma-flexia-models" );
	m_LemmaFlexiaModel.Resize ( rd.UnzipInt() );
	ARRAY_FOREACH ( i, m_LemmaFlexiaModel )
		m_LemmaFlexiaModel[i] = (WORD) rd.UnzipInt();

	// build model freqs
	m_ModelFreq.Resize ( m_FlexiaModels.GetLength() );
	m_ModelFreq.Fill ( 0 );
	ARRAY_FOREACH ( i, m_LemmaFlexiaModel )
		m_ModelFreq [ m_LemmaFlexiaModel[i] ]++;

	rd.Tag ( "nps-vector" );
	m_NPSs.Resize ( rd.UnzipInt() );
	rd.GetBytes ( m_NPSs.Begin(), m_NPSs.GetLength() );

	// caching predictions does not measurably affect performance though
	rd.Tag ( "prediction-automaton" );
	m_SuffixAutomat.LoadPak ( rd, 0 );

	rd.Tag ( "eof" );
	return !rd.GetErrorFlag();
}

//////////////////////////////////////////////////////////////////////////
// SPHINX MORPHOLOGY INTERFACE
//////////////////////////////////////////////////////////////////////////

const char* AOT_LANGUAGES[AOT_LENGTH] = {"ru", "en", "de", "uk" };

static CLemmatizer *	g_pLemmatizers[AOT_LENGTH] = {0};
static CSphNamedInt		g_tDictinfos[AOT_LENGTH];
static std::unique_ptr<NativeUkLemmatizer_c> g_pUkLemmatizer;

void sphAotSetCacheSize ( int iCacheSize )
{
	g_iCacheSize = Max ( iCacheSize, 0 );
}

static bool LoadLemmatizerUk ( const CSphString & sDictFile, CSphString & sError );

bool AotInit ( const CSphString & sDictFile, CSphString & sError, int iLang )
{
	if ( g_pLemmatizers[iLang] )
		return true;

	if ( iLang==AOT_UK )
		return LoadLemmatizerUk ( sDictFile, sError );

	CSphAutofile rdFile;
	if ( rdFile.Open ( sDictFile, SPH_O_READ, sError )<0 )
		return false;

	g_pLemmatizers[iLang] = new CLemmatizer ( iLang==AOT_DE );
	g_pLemmatizers[iLang]->m_iLang = iLang;

	CSphReader rd;
	rd.SetFile ( rdFile );
	if ( !g_pLemmatizers[iLang]->LoadPak(rd) )
	{
		sError.SetSprintf ( "failed to load lemmatizer dictionary: %s", rd.GetErrorMessage().cstr() );
		SafeDelete ( g_pLemmatizers[iLang] );
		return false;
	}

	// track dictionary crc
	DWORD uCrc;
	if ( !sphCalcFileCRC32 ( sDictFile.cstr(), uCrc ) )
	{
		sError.SetSprintf ( "failed to crc32 lemmatizer dictionary %s", sDictFile.cstr() );
		SafeDelete ( g_pLemmatizers[iLang] );
		return false;
	}

	// extract basename
	const char * a = sDictFile.cstr();
	const char * b = a + strlen(a) - 1;
	while ( b>a && b[-1]!='/' && b[-1]!='\\' )
		b--;

	g_tDictinfos[iLang] = { b, (int) uCrc };
	return true;
}

bool sphAotInit ( const CSphString & sDictFile, CSphString & sError, int iLang )
{
	return AotInit ( sDictFile, sError, iLang );
}

static inline bool IsAlpha1251 ( BYTE c )
{
	return ( c>=0xC0 || c==0xA8 || c==0xB8 );
}

static inline bool IsGermanAlpha1252 ( BYTE c )
{
	if ( c==0xb5 || c==0xdf )
		return true;

	BYTE lc = c | 0x20;
	switch ( lc )
	{
	case 0xe2:
	case 0xe4:
	case 0xe7:
	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xf1:
	case 0xf4:
	case 0xf6:
	case 0xfb:
	case 0xfc:
		return true;
	default:
		return ( lc>0x60 && lc<0x7b );
	}
}

static inline bool IsAlphaAscii ( BYTE c )
{
	BYTE lc = c | 0x20;
	return ( lc>0x60 && lc<0x7b );
}

enum EMMITERS {EMIT_1BYTE, EMIT_UTF8RU, EMIT_UTF8};
template < EMMITERS >
inline BYTE * Emit ( BYTE * sOut, BYTE uChar )
{
	if ( uChar=='-' )
		return sOut;
	*sOut++ = uChar | 0x20;
	return sOut;
}

template<>
inline BYTE * Emit<EMIT_UTF8RU> ( BYTE * sOut, BYTE uChar )
{
	if ( uChar=='-' )
		return sOut;
	assert ( uChar!=0xA8 && uChar!=0xB8 ); // no country for yo
	uChar |= 0x20; // lowercase, E0..FF range now
	if ( uChar & 0x10 )
	{
		// F0..FF -> D1 80..D1 8F
		*sOut++ = 0xD1;
		*sOut++ = uChar - 0x70;
	} else
	{
		// E0..EF -> D0 B0..D0 BF
		*sOut++ = 0xD0;
		*sOut++ = uChar - 0x30;
	}
	return sOut;
}

template<>
inline BYTE * Emit<EMIT_UTF8> ( BYTE * sOut, BYTE uChar )
{
	if ( uChar=='-' )
		return sOut;

	if ( uChar!=0xDF ) // don't touch 'ss' umlaut
		uChar |= 0x20;

	if ( uChar & 0x80 )
	{
		*sOut++ = 0xC0 | (uChar>>6);
		*sOut++ = 0x80 | (uChar&0x3F); // NOLINT
	} else
		*sOut++ = uChar;
	return sOut;
}

template < EMMITERS IS_UTF8 >
inline void CreateLemma ( BYTE * sOut, const BYTE * sBase, int iBaseLen, bool bFound, const CFlexiaModel & M, const CMorphForm & F )
{
	// cut the form prefix
	int PrefixLen = F.m_PrefixLen;
	if	( bFound || strncmp ( (const char*)sBase, F.m_Prefix, PrefixLen )==0 )
	{
		sBase += PrefixLen;
		iBaseLen -= PrefixLen;
	}

	// FIXME! maybe handle these lemma wide prefixes too?
#if 0
	const string & LemmPrefix = m_pParent->m_Prefixes[m_InnerAnnot.m_PrefixNo];
	if ( m_bFound
		|| (
		( m_InputWordBase.substr ( 0, LemmPrefix.length() )==LemmPrefix ) &&
		( m_InputWordBase.substr ( LemmPrefix.length(), F.m_PrefixStr.length() )==F.m_PrefixStr ) ) )
	{
		m_InputWordBase.erase ( 0, LemmPrefix.length()+ M.m_PrefixStr.length() );
		m_bPrefixesWereCut = true;
	}
#endif

	// cut the form suffix and append the lemma suffix
	// UNLESS this was a predicted form, and form suffix does not fully match!
	// eg. word=GUBARIEVICHA, flexion=IEIVICHA, so this is not really a matching lemma
	int iSuff = F.m_FlexiaLen;
	if ( bFound || ( iBaseLen>=iSuff && strncmp ( (const char*)sBase+iBaseLen-iSuff, F.m_Flexia, iSuff )==0 ) )
	{
		// ok, found and/or suffix matches, the usual route
		int iCodePoints = 0;
		iBaseLen -= iSuff;
		while ( iBaseLen-- && iCodePoints<SPH_MAX_WORD_LEN )
		{
			sOut = Emit<IS_UTF8> ( sOut, *sBase++ );
			iCodePoints++;
		}

		int iLemmaSuff = M[0].m_FlexiaLen;
		const char * sFlexia = M[0].m_Flexia;
		while ( iLemmaSuff-- && iCodePoints<SPH_MAX_WORD_LEN ) // OPTIMIZE? can remove len here
		{
			sOut = Emit<IS_UTF8> ( sOut, *sFlexia++ );
			iCodePoints++;
		}
	} else
	{
		// whoops, no suffix match, just copy and lowercase the current base
		while ( iBaseLen-- )
			sOut = Emit<IS_UTF8> ( sOut, *sBase++ );
	}
	*sOut = '\0';
}

static inline bool IsRuFreq2 ( BYTE * pWord )
{
	if ( pWord[2]!=0 )
		return false;

	int iCode = ( ( pWord[0]<<8 ) + pWord[1] ) | 0x2020;
	switch ( iCode )
	{
		case 0xEDE0: // na
		case 0xEFEE: // po
		case 0xEDE5: // ne
		case 0xEEF2: // ot
		case 0xE7E0: // za
		case 0xEEE1: // ob
		case 0xE4EE: // do
		case 0xF1EE: // so
		case 0xE8E7: // iz
		case 0xE8F5: // ih
		case 0xF8F2: // sht
		case 0xF3EB: // ul
			return true;
	}
	return false;
}

static inline bool IsEnFreq2 ( BYTE * )
{
	// stub
	return false;
}

static inline bool IsDeFreq2 ( BYTE * )
{
	// stub
	return false;
}

static inline bool IsRuFreq3 ( BYTE * pWord )
{
	if ( pWord[3]!=0 )
		return false;
	int iCode = ( ( pWord[0]<<16 ) + ( pWord[1]<<8 ) + pWord[2] ) | 0x202020;
	return ( iCode==0xE8EBE8 || iCode==0xE4EBFF || iCode==0xEFF0E8 // ili, dlya, pri
		|| iCode==0xE3EEE4 || iCode==0xF7F2EE || iCode==0xE1E5E7 ); // god, chto, bez
}

static inline bool IsRuNeed2 ( BYTE * pWord )
{
	if ( pWord[2]!=0 )
		return false;
	int iCode = ( ( pWord[0]<<8 ) + ( pWord[1] ) ) | 0x2020;
	return ( iCode==0xECEC || iCode==0xF1EC || iCode==0xEAEC || iCode==0xEAE3 ); // mm, cm, km, kg
}


static inline bool IsEnFreq3 ( BYTE * )
{
	// stub
	return false;
}

static inline bool IsDeFreq3 ( BYTE * )
{
	// stub
	return false;
}

void sphAotLemmatizeRu1251 ( BYTE * pWord, int iLen )
{
	// i must be initialized
	assert ( g_pLemmatizers[AOT_RU] );

	// pass-through 1-char words, and non-Russian words
	if ( !IsAlpha1251(*pWord) || !pWord[1] )
		return;

	// handle a few most frequent 2-char, 3-char pass-through words
	if ( iLen==2 && IsRuFreq2 ( pWord ))
		return;

	if ( iLen==3 && IsRuFreq3 ( pWord ))
		return;

	// do lemmatizing
	// input keyword moves into sForm; LemmatizeWord() will also case fold sForm
	// we will generate results using sForm into pWord; so we need this extra copy
	BYTE sForm[MAX_KEYWORD_BYTES];
	int iFormLen = 0;

	// faster than strlen and strcpy..
	for ( BYTE * p=pWord; *p; )
		sForm[iFormLen++] = *p++;
	sForm[iFormLen] = '\0';

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizers[AOT_RU]->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	// pick a single form
	// picks a noun, if possible, and otherwise prefers shorter forms
	bool bNoun = false;
	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizers[AOT_RU]->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		bool bNewNoun = ( F.m_POS==0 );
		if ( i==0 || ( !bNoun && bNewNoun ) )
		{
			CreateLemma<EMIT_1BYTE> ( pWord, sForm, iFormLen, bFound, M, F );
			bNoun = bNewNoun;
		} else if ( bNoun==bNewNoun )
		{
			BYTE sBuf[256];
			CreateLemma<EMIT_1BYTE> ( sBuf, sForm, iFormLen, bFound, M, F );
			if ( strcmp ( (char*)sBuf, (char*)pWord )<0 )
				strcpy ( (char*)pWord, (char*)sBuf ); // NOLINT
		}
	}
}

void sphAotLemmatize ( BYTE * pWord, int iLang )
{
	// i must be initialized
	assert ( g_pLemmatizers[iLang] );

	// pass-through 1-char words, and non-Russian words
	if ( !IsAlphaAscii(*pWord) || !pWord[1] )
		return;

	// handle a few most frequent 2-char, 3-char pass-through words
	if ( iLang==AOT_EN && ( IsEnFreq2(pWord) || IsEnFreq3(pWord) ) )
		return;

	if ( iLang==AOT_DE && ( IsDeFreq2(pWord) || IsDeFreq3(pWord) ) )
		return;

	// do lemmatizing
	// input keyword moves into sForm; LemmatizeWord() will also case fold sForm
	// we will generate results using sForm into pWord; so we need this extra copy
	BYTE sForm[MAX_KEYWORD_BYTES];
	int iFormLen = 0;

	// faster than strlen and strcpy..
	for ( BYTE * p=pWord; *p; )
		sForm[iFormLen++] = *p++;
	sForm[iFormLen] = '\0';

	// do nothing with one-char words
	if ( iFormLen<=1 )
		return;

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizers[iLang]->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	// pick a single form
	// picks a noun, if possible, and otherwise prefers shorter forms
	bool bNoun = false;
	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizers[iLang]->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		bool bNewNoun = ( F.m_POS==0 );
		if ( i==0 || ( !bNoun && bNewNoun ) )
		{
			CreateLemma<EMIT_1BYTE> ( pWord, sForm, iFormLen, bFound, M, F );
			bNoun = bNewNoun;
		} else if ( bNoun==bNewNoun )
		{
			BYTE sBuf[256];
			CreateLemma<EMIT_1BYTE> ( sBuf, sForm, iFormLen, bFound, M, F );
			if ( strcmp ( (char*)sBuf, (char*)pWord )<0 )
				strcpy ( (char*)pWord, (char*)sBuf ); // NOLINT
		}
	}
}

static inline bool IsRussianAlphaUtf8 ( const BYTE * pWord )
{
	// letters, windows-1251, utf-8
	// A..YA, C0..DF, D0 90..D0 AF
	// a..p, E0..EF, D0 B0..D0 BF
	// r..ya, F0..FF, D1 80..D1 8F
	// YO, A8, D0 81
	// yo, B8, D1 91
	if ( pWord[0]==0xD0 )
		if ( pWord[1]==0x81 || ( pWord[1]>=0x90 && pWord[1]<0xC0 ) )
			return true;
	if ( pWord[0]==0xD1 )
		if ( pWord[1]>=0x80 && pWord[1]<=0x91 && pWord[1]!=0x90 )
			return true;
	return false;
}

void sphAotLemmatizeDe1252 ( BYTE * pWord, int iLen )
{
	// i must be initialized
	assert ( g_pLemmatizers[AOT_DE] );

	// pass-through 1-char words, and non-German words
	if ( !IsGermanAlpha1252(*pWord) || !pWord[1] )
		return;

	// handle a few most frequent 2-char, 3-char pass-through words
	if ( iLen==2 && IsDeFreq2 ( pWord ))
		return;

	if ( iLen==3 && IsDeFreq3 ( pWord ))
		return;

	// do lemmatizing
	// input keyword moves into sForm; LemmatizeWord() will also case fold sForm
	// we will generate results using sForm into pWord; so we need this extra copy
	BYTE sForm[MAX_KEYWORD_BYTES];
	int iFormLen = 0;

	// faster than strlen and strcpy..
	for ( BYTE * p=pWord; *p; )
		sForm[iFormLen++] = *p++;
	sForm[iFormLen] = '\0';

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizers[AOT_DE]->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	// pick a single form
	// picks a noun, if possible, and otherwise prefers shorter forms
	bool bNoun = false;
	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizers[AOT_DE]->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		bool bNewNoun = ( F.m_POS==0 );
		if ( i==0 || ( !bNoun && bNewNoun ) )
		{
			CreateLemma<EMIT_1BYTE> ( pWord, sForm, iFormLen, bFound, M, F );
			bNoun = bNewNoun;
		} else if ( bNoun==bNewNoun )
		{
			BYTE sBuf[256];
			CreateLemma<EMIT_1BYTE> ( sBuf, sForm, iFormLen, bFound, M, F );
			if ( strcmp ( (char*)sBuf, (char*)pWord )<0 )
				strcpy ( (char*)pWord, (char*)sBuf ); // NOLINT
		}
	}
}

/// returns length in bytes (aka chars) if all letters were russian and converted
/// returns 0 and aborts early if non-russian letters are encountered
static inline int Utf8ToWin1251 ( BYTE * pOut, const BYTE * pWord )
{
	// YO, win A8, utf D0 81
	// A..YA, win C0..DF, utf D0 90..D0 AF
	// a..p, win E0..EF, utf D0 B0..D0 BF
	// r..ya, win F0..FF, utf D1 80..D1 8F
	// yo, win B8, utf D1 91
	static const BYTE dTable[128] =
	{
		0, 0xa8, 0, 0, 0, 0, 0, 0, // 00
		0, 0, 0, 0, 0, 0, 0, 0, // 08
		0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, // 10
		0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, // 18
		0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, // 20
		0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, // 28
		0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, // 30
		0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, // 38
		0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, // 40
		0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, // 48
		0, 0xb8, 0, 0, 0, 0, 0, 0, // 50
		0, 0, 0, 0, 0, 0, 0, 0, // 58
		0, 0, 0, 0, 0, 0, 0, 0, // 60
		0, 0, 0, 0, 0, 0, 0, 0, // 68
		0, 0, 0, 0, 0, 0, 0, 0, // 70
		0, 0, 0, 0, 0, 0, 0, 0 // 78
	};

	BYTE * pStart = pOut;
	while ( *pWord )
	{
		// russian utf-8 letters begin with either D0 or D1
		// and any valid 2nd utf-8 byte must be in 80..BF range
		if ( ( *pWord & 0xFE )!=0xD0 )
			return 0;
		assert ( pWord[1]>=0x80 && pWord[1]<0xC0 );

		// table index D0 80..BF to 0..3F, and D1 80..BF to 40..7F
		BYTE uWin = dTable [ ( pWord[1] & 0x7F ) + ( ( pWord[0] & 1 )<<6 ) ];
		pWord += 2;

		if ( !uWin )
			return 0;
		*pOut++ = uWin;
	}

	*pOut = '\0';
	return (int)( pOut-pStart );
}

/// returns length in bytes (aka chars) if all letters were converted
/// returns 0 and aborts early if non-western letters are encountered
static inline int Utf8ToWin1252 ( BYTE * pOut, const BYTE * pWord )
{
	BYTE * pStart = pOut;
	while ( *pWord )
	{
		if ( (*pWord)&0x80 )
		{
			if ( ((*pWord)&0xFC)==0xC0 )
			{
				*pOut++ = ( pWord[1] & 0x7F ) + ( ( pWord[0] & 3 )<<6 );
				pWord += 2;
			} else
				return 0;
		} else
			*pOut++ = *pWord++;
	}

	*pOut = '\0';
	return (int)( pOut-pStart );
}

static inline bool IsGermanAlphaUtf8 ( const BYTE * pWord )
{
	// letters, windows-1252, utf-8
	// A..Z, trivial
	if ( pWord[0]>0x40 && pWord[0]<0x5b )
		return true;

	// a..z, also trivial
	if ( pWord[0]>0x60 && pWord[0]<0x7b )
		return true;

	// mu, 0xb5
	if ( pWord[0]==0xC2 && pWord[1]==0xB5 )
		return true;

	// some upper
	if ( pWord[0]==0xC3 )
	{
		if ( pWord[1]==0X9F ) // ss umlaut
			return true;
		switch ( pWord[1] | 0x20 )
		{
			case 0xA2: // umlauts
			case 0xA4:
			case 0xA7:
			case 0xA8:
			case 0xA9:
			case 0xAa:
			case 0xB1:
			case 0xB4:
			case 0xB6:
			case 0xBb:
			case 0xBc:
				return true;
		}
	}
	return false;
}

static inline void Win1251ToLowercaseUtf8 ( BYTE * pOut, const BYTE * pWord )
{
	while ( *pWord )
	{
		// a..p, E0..EF maps to D0 B0..D0 BF
		// r..ya, F0..FF maps to D1 80..D1 8F
		// yo maps to D1 91
		if ( *pWord>=0xC0 )
		{
			BYTE iCh = ( *pWord | 0x20 ); // lowercase
			BYTE iF = ( iCh>>4 ) & 1; // 0xE? or 0xF? value
			*pOut++ = 0xD0 + iF;
			*pOut++ = iCh - 0x30 - ( iF<<6 );
		} else if ( *pWord==0xA8 || *pWord==0xB8 )
		{
			*pOut++ = 0xD1;
			*pOut++ = 0x91;
		} else
			assert ( false );
		pWord++;
	}
	*pOut++ = '\0';
}

static inline void Win1252ToLowercaseUtf8 ( BYTE * pOut, const BYTE * pWord )
{
	while ( *pWord )
	{
		if ( !((*pWord)&0x80) )
			*pOut++ = *pWord | 0x20;
		else
		{
			*pOut++ = 0xC0 | ((*pWord)>>6);
			*pOut++ = 0x80 | ((*pWord)&0x3F);
		}
		++pWord;
	}
	*pOut++ = '\0';
}

void sphAotLemmatizeRuUTF8 ( BYTE * pWord )
{
	// i must be initialized
	assert ( g_pLemmatizers[AOT_RU] );

	// only if the word is russian
	if ( !IsRussianAlphaUtf8(pWord) )
		return;

	// convert to Windows-1251
	// failure means we should not lemmatize this
	BYTE sBuf [ SPH_MAX_WORD_LEN+4 ];
	auto iFormLen = Utf8ToWin1251 ( sBuf, pWord );
	if ( !iFormLen )
		return;

	// lemmatize, convert back, done!
	sphAotLemmatizeRu1251 ( sBuf, iFormLen );
	Win1251ToLowercaseUtf8 ( pWord, sBuf );
}

void sphAotLemmatizeDeUTF8 ( BYTE * pWord )
{
	// i must be initialized
	assert ( g_pLemmatizers[AOT_DE] );

	// only if the word is german
	if ( !IsGermanAlphaUtf8(pWord) )
		return;

	// convert to Windows-1252
	// failure means we should not lemmatize this
	BYTE sBuf [ SPH_MAX_WORD_LEN+4 ];
	auto iFormLen = Utf8ToWin1252 ( sBuf, pWord );
	if ( !iFormLen )
		return;

	// lemmatize, convert back, done!
	sphAotLemmatizeDe1252 ( sBuf, iFormLen );
	Win1252ToLowercaseUtf8 ( pWord, sBuf );
}

void sphAotLemmatizeRu ( StrVec_t & dLemmas, const BYTE * pWord )
{
	assert ( g_pLemmatizers[AOT_RU] );
	if ( !IsRussianAlphaUtf8(pWord) )
		return;

	BYTE sForm [ SPH_MAX_WORD_LEN+4 ];
	int iFormLen = 0;
	iFormLen = Utf8ToWin1251 ( sForm, pWord );

	if ( iFormLen<2 || IsRuFreq2(sForm) )
		return;
	if ( ( iFormLen<3 || IsRuFreq3(sForm) ) && !IsRuNeed2(sForm) )
		return;

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizers[AOT_RU]->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizers[AOT_RU]->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		BYTE sRes [ 3*SPH_MAX_WORD_LEN+4 ];

		CreateLemma<EMIT_UTF8RU> ( sRes, sForm, iFormLen, bFound, M, F );
		dLemmas.Add ( (const char*)sRes );
	}

	// OPTIMIZE?
	dLemmas.Uniq();
}

void sphAotLemmatizeDe ( StrVec_t & dLemmas, const BYTE * pWord )
{
	assert ( g_pLemmatizers[AOT_DE] );
	if ( !IsGermanAlphaUtf8(pWord) )
		return;

	BYTE sForm [ SPH_MAX_WORD_LEN+4 ];
	int iFormLen = 0;
	iFormLen = Utf8ToWin1252 ( sForm, pWord );

	if ( iFormLen<=1 )
		return;

	if ( IsDeFreq2(sForm) || IsDeFreq3(sForm) )
		return;

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizers[AOT_DE]->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizers[AOT_DE]->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		BYTE sRes [ 3*SPH_MAX_WORD_LEN+4 ];

		CreateLemma<EMIT_UTF8> ( sRes, sForm, iFormLen, bFound, M, F );
		dLemmas.Add ( (const char*)sRes );
	}

	// OPTIMIZE?
	dLemmas.Uniq();
}

// generic lemmatize for other languages
void sphAotLemmatize ( StrVec_t & dLemmas, const BYTE * pWord, int iLang )
{
	assert ( iLang!=AOT_RU ); // must be processed by the specialized function
	assert ( g_pLemmatizers[iLang] );

	if ( !IsAlphaAscii(*pWord) )
		return;

	BYTE sForm [ SPH_MAX_WORD_LEN+4 ];
	int iFormLen = 0;

	while ( *pWord )
		sForm [ iFormLen++ ] = *pWord++;
	sForm [ iFormLen ] = '\0';

	if ( iFormLen<=1 )
		return;

	if ( iLang==AOT_EN && ( IsEnFreq2(sForm) || IsEnFreq3(sForm) ) )
		return;

	if ( iLang==AOT_DE && ( IsDeFreq2(sForm) || IsDeFreq3(sForm) ) )
		return;

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizers[iLang]->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizers[iLang]->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		BYTE sRes [ 3*SPH_MAX_WORD_LEN+4 ];
		CreateLemma<EMIT_1BYTE> ( sRes, sForm, iFormLen, bFound, M, F );

		dLemmas.Add ( (const char*)sRes );
	}

	// OPTIMIZE?
	dLemmas.Uniq();
}


const CSphNamedInt & sphAotDictinfo ( int iLang )
{
	return g_tDictinfos[iLang];
}

//////////////////////////////////////////////////////////////////////////

#define MAX_EXTRA_TOKENS 12

/// token filter for AOT morphology indexing
/// AOT may return multiple (!) morphological hypotheses for a single token
/// we return such additional hypotheses as blended tokens
class CSphAotTokenizerTmpl : public CSphTokenFilter
{
protected:
	using Base = CSphTokenFilter;
	BYTE		m_sForm[MAX_KEYWORD_BYTES];
	int			m_iFormLen = 0;						///< in bytes, but in windows-1251 that is characters, too
	bool		m_bFound = false;					///< found or predicted?
	DWORD		m_FindResults[MAX_EXTRA_TOKENS];					///< max results is like 6
	int			m_iCurrent = -1;					///< index in m_FindResults that was just returned, -1 means no blending
	BYTE		m_sToken[MAX_KEYWORD_BYTES];		///< to hold generated lemmas
	BYTE		m_sOrigToken[MAX_KEYWORD_BYTES];	///< to hold original token
	bool		m_bIndexExact;

	const CSphWordforms *	m_pWordforms = nullptr;

public:
	CSphAotTokenizerTmpl ( TokenizerRefPtr_c pTok, const DictRefPtr_c& pDict, bool bIndexExact, int DEBUGARG(iLang) )
		: CSphTokenFilter ( std::move (pTok) )
	{
		assert ( g_pLemmatizers[iLang] );
		m_FindResults[0] = AOT_NOFORM;
		if ( pDict )
		{
			// tricky bit
			// one does not simply take over the wordforms from the dict
			// that would break saving of the (embedded) wordforms data
			// but as this filter replaces wordforms
			m_pWordforms = pDict->GetWordforms();
			pDict->DisableWordforms();
		}
		m_bIndexExact = bIndexExact;
		*(DWORD*)m_sForm = 0; // fix valgrind on test 463: Conditional jump or move depends on uninitialised value
	}


	bool TokenIsBlended() const noexcept final
	{
		return m_iCurrent>=0 || m_pTokenizer->TokenIsBlended();
	}

	uint64_t GetSettingsFNV () const noexcept final
	{
		uint64_t uHash = CSphTokenFilter::GetSettingsFNV();
		uHash ^= (uint64_t)m_pWordforms;
		DWORD uFlags = m_bIndexExact ? 1 : 0;
		uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );
		return uHash;
	}
};

class CSphAotTokenizerRu : public CSphAotTokenizerTmpl
{
public:
	CSphAotTokenizerRu ( TokenizerRefPtr_c pTok, const DictRefPtr_c& pDict, bool bIndexExact )
		: CSphAotTokenizerTmpl ( std::move (pTok), pDict, bIndexExact, AOT_RU )
	{}

	TokenizerRefPtr_c Clone ( ESphTokenizerClone eMode ) const noexcept final
	{
		// this token filter must NOT be created as escaped
		// it must only be used during indexing time, NEVER in searching time
		assert ( eMode==SPH_CLONE_INDEX );
		auto * pClone = new CSphAotTokenizerRu ( m_pTokenizer->Clone ( eMode ), nullptr, m_bIndexExact );
		if ( m_pWordforms )
			pClone->m_pWordforms = m_pWordforms;
		return TokenizerRefPtr_c { pClone };
	}

	BYTE * GetToken() final
	{
		m_eTokenMorph = SPH_TOKEN_MORPH_RAW;

		// any pending lemmas left?
		if ( m_iCurrent>=0 )
		{
			++m_iCurrent;
			assert ( m_FindResults[m_iCurrent]!=AOT_NOFORM );

			// return original token
			if ( m_FindResults[m_iCurrent]==AOT_ORIGFORM )
			{
				assert ( m_FindResults[m_iCurrent+1]==AOT_NOFORM );
				strncpy ( (char*)m_sToken, (char*)m_sOrigToken, sizeof(m_sToken)-1 );
				m_iCurrent = -1;
				m_eTokenMorph = SPH_TOKEN_MORPH_ORIGINAL;
				return m_sToken;
			}

			// generate that lemma
			const CFlexiaModel & M = g_pLemmatizers[AOT_RU]->m_FlexiaModels [ AOT_MODEL_NO ( m_FindResults [ m_iCurrent ] ) ];
			const CMorphForm & F = M [ AOT_ITEM_NO ( m_FindResults [ m_iCurrent ] ) ];
			CreateLemma<EMIT_UTF8RU> ( m_sToken, m_sForm, m_iFormLen, m_bFound, M, F );

			// is this the last one? gotta tag it non-blended
			if ( m_FindResults [ m_iCurrent+1 ]==AOT_NOFORM )
				m_iCurrent = -1;

			if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
				m_pWordforms->ToNormalForm ( m_sToken, false, false );

			m_eTokenMorph = SPH_TOKEN_MORPH_GUESS;
			return m_sToken;
		}

		// ok, time to work on a next word
		assert ( m_iCurrent<0 );
		BYTE * pToken = Base::GetToken();
		m_eTokenMorph = m_pTokenizer->GetTokenMorph();
		if ( !pToken )
			return nullptr;

		// pass-through blended parts
		if ( m_pTokenizer->TokenIsBlended() )
			return pToken;

		// pass-through matched wordforms
		if ( m_pWordforms && m_pWordforms->ToNormalForm ( pToken, true, false ) )
			return pToken;

		// pass-through 1-char "words"
		if ( pToken[1]=='\0' )
			return pToken;

		// pass-through non-Russian words
		if ( !IsRussianAlphaUtf8 ( pToken ) )
			return pToken;

		// convert or copy regular tokens
		m_iFormLen = Utf8ToWin1251 ( m_sForm, pToken );

		// do nothing with one-char words
		if ( m_iFormLen<=1 )
			return pToken;

		// handle a few most frequent 2-char, 3-char pass-through words
		// OPTIMIZE? move up?
		if ( IsRuFreq2 ( m_sForm ) || IsRuFreq3 ( m_sForm ) )
			return pToken;

		// lemmatize
		m_bFound = g_pLemmatizers[AOT_RU]->LemmatizeWord ( m_sForm, m_FindResults );
		if ( m_FindResults[0]==AOT_NOFORM )
		{
			assert ( m_iCurrent<0 );
			return pToken;
		}

		// schedule original form for return, if needed
		if ( m_bIndexExact )
		{
			int i = 1;
			while ( m_FindResults[i]!=AOT_NOFORM )
				i++;
			m_FindResults[i] = AOT_ORIGFORM;
			m_FindResults[i+1] = AOT_NOFORM;
			strncpy ( (char*)m_sOrigToken, (char*)pToken, sizeof(m_sOrigToken) );
			m_sOrigToken[sizeof(m_sOrigToken)-1] = '\0';
		}

		// in any event, prepare the first lemma for return
		const CFlexiaModel & M = g_pLemmatizers[AOT_RU]->m_FlexiaModels [ AOT_MODEL_NO ( m_FindResults[0] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( m_FindResults[0] ) ];
		CreateLemma<EMIT_UTF8RU> ( pToken, m_sForm, m_iFormLen, m_bFound, M, F );

		// schedule lemmas 2+ for return
		if ( m_FindResults[1]!=AOT_NOFORM )
			m_iCurrent = 0;

		// suddenly, post-morphology wordforms
		if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
			m_pWordforms->ToNormalForm ( pToken, false, false );

		m_eTokenMorph = SPH_TOKEN_MORPH_GUESS;
		return pToken;
	}
};

class CSphAotTokenizer : public CSphAotTokenizerTmpl
{
	AOT_LANGS		m_iLang;

public:
	CSphAotTokenizer ( TokenizerRefPtr_c pTok, const DictRefPtr_c& pDict, bool bIndexExact, int iLang )
		: CSphAotTokenizerTmpl ( std::move (pTok), pDict, bIndexExact, iLang )
		, m_iLang ( AOT_LANGS(iLang) )
	{}

	TokenizerRefPtr_c Clone ( ESphTokenizerClone eMode ) const noexcept final
	{
		// this token filter must NOT be created as escaped
		// it must only be used during indexing time, NEVER in searching time
		assert ( eMode==SPH_CLONE_INDEX );
		auto * pClone = new CSphAotTokenizer ( m_pTokenizer->Clone ( eMode ), nullptr, m_bIndexExact, m_iLang );
		if ( m_pWordforms )
			pClone->m_pWordforms = m_pWordforms;
		return TokenizerRefPtr_c { pClone };
	}

	BYTE * GetToken() final
	{
		m_eTokenMorph = SPH_TOKEN_MORPH_RAW;

		// any pending lemmas left?
		if ( m_iCurrent>=0 )
		{
			++m_iCurrent;
			assert ( m_FindResults[m_iCurrent]!=AOT_NOFORM );

			// return original token
			if ( m_FindResults[m_iCurrent]==AOT_ORIGFORM )
			{
				assert ( m_FindResults[m_iCurrent+1]==AOT_NOFORM );
				strncpy ( (char*)m_sToken, (char*)m_sOrigToken, sizeof(m_sToken) );
				m_sToken[sizeof ( m_sToken ) - 1] = '\0';
				m_iCurrent = -1;
				m_eTokenMorph = SPH_TOKEN_MORPH_ORIGINAL;
				return m_sToken;
			}

			// generate that lemma
			const CFlexiaModel & M = g_pLemmatizers[m_iLang]->m_FlexiaModels [ AOT_MODEL_NO ( m_FindResults [ m_iCurrent ] ) ];
			const CMorphForm & F = M [ AOT_ITEM_NO ( m_FindResults [ m_iCurrent ] ) ];
			CreateLemma<EMIT_UTF8> ( m_sToken, m_sForm, m_iFormLen, m_bFound, M, F );

			// is this the last one? gotta tag it non-blended
			if ( m_FindResults [ m_iCurrent+1 ]==AOT_NOFORM )
				m_iCurrent = -1;

			if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
				m_pWordforms->ToNormalForm ( m_sToken, false, false );

			m_eTokenMorph = SPH_TOKEN_MORPH_GUESS;
			return m_sToken;
		}

		// ok, time to work on a next word
		assert ( m_iCurrent<0 );
		BYTE * pToken = Base::GetToken();
		m_eTokenMorph = m_pTokenizer->GetTokenMorph();
		if ( !pToken )
			return nullptr;

		// pass-through blended parts
		if ( m_pTokenizer->TokenIsBlended() )
			return pToken;

		// pass-through matched wordforms
		if ( m_pWordforms && m_pWordforms->ToNormalForm ( pToken, true, false ) )
			return pToken;

		// pass-through 1-char "words"
		if ( pToken[1]=='\0' )
			return pToken;

		// pass-through non-Russian words
		if ( m_iLang==AOT_DE )
		{
			if ( !IsGermanAlphaUtf8 ( pToken ) )
				return pToken;
		} else
		{
			if ( !IsGermanAlpha1252 ( pToken[0] ) )
				return pToken;
		}

		// convert or copy regular tokens
		if ( m_iLang==AOT_DE )
			m_iFormLen = Utf8ToWin1252 ( m_sForm, pToken );
		else
		{
			// manual strlen and memcpy; faster this way
			BYTE * p = pToken;
			m_iFormLen = 0;
			while ( *p )
				m_sForm [ m_iFormLen++ ] = *p++;
			m_sForm [ m_iFormLen ] = '\0';
		}

		// do nothing with one-char words
		if ( m_iFormLen<=1 )
			return pToken;

		// handle a few most frequent 2-char, 3-char pass-through words
		// OPTIMIZE? move up?
		if ( ( m_iLang==AOT_DE && ( IsDeFreq2 ( m_sForm ) || IsDeFreq3 ( m_sForm ) ) )
			|| ( m_iLang==AOT_EN && ( IsEnFreq2 ( m_sForm ) || IsEnFreq3 ( m_sForm ) ) ) )
			return pToken;

		// lemmatize
		m_bFound = g_pLemmatizers[m_iLang]->LemmatizeWord ( m_sForm, m_FindResults );
		if ( m_FindResults[0]==AOT_NOFORM )
		{
			assert ( m_iCurrent<0 );
			return pToken;
		}

		// schedule original form for return, if needed
		if ( m_bIndexExact )
		{
			int i = 1;
			while ( m_FindResults[i]!=AOT_NOFORM )
				i++;
			m_FindResults[i] = AOT_ORIGFORM;
			m_FindResults[i+1] = AOT_NOFORM;
			strncpy ( (char*)m_sOrigToken, (char*)pToken, sizeof(m_sOrigToken) );
			m_sOrigToken[sizeof ( m_sOrigToken ) - 1] = '\0';
		}

		// in any event, prepare the first lemma for return
		const CFlexiaModel & M = g_pLemmatizers[m_iLang]->m_FlexiaModels [ AOT_MODEL_NO ( m_FindResults[0] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( m_FindResults[0] ) ];
		CreateLemma<EMIT_UTF8> ( pToken, m_sForm, m_iFormLen, m_bFound, M, F );

		// schedule lemmas 2+ for return
		if ( m_FindResults[1]!=AOT_NOFORM )
			m_iCurrent = 0;

		// suddenly, post-morphology wordforms
		if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
			m_pWordforms->ToNormalForm ( pToken, false, false );

		m_eTokenMorph = SPH_TOKEN_MORPH_GUESS;
		return pToken;
	}
};

class LemmatizerUk_c : public LemmatizerTrait_i
{
	CSphVector<CSphString> m_dLemmas;
	int m_iCurrent = 0;
	BYTE m_sResultToken [ MAX_KEYWORD_BYTES ] {};

public:
	LemmatizerUk_c();
	virtual ~LemmatizerUk_c() override;

	BYTE * GetToken ( const BYTE * pWord, int & iExtra ) override;
	BYTE * GetExtraToken() override;
};

class TokenizerUk_c : public CSphAotTokenizerTmpl
{
	LemmatizerUk_c m_tLemmatizer;

public:
	TokenizerUk_c ( TokenizerRefPtr_c pTok, const DictRefPtr_c& pDict, bool bIndexExact );
	TokenizerRefPtr_c Clone ( ESphTokenizerClone eMode ) const noexcept final;
	BYTE * GetToken() final;
};

void sphAotTransformFilter ( TokenizerRefPtr_c& pTokenizer, const DictRefPtr_c& pDict, bool bIndexExact, DWORD uLangMask )
{
	assert ( uLangMask!=0 );
	for ( int i=AOT_BEGIN; i<AOT_LENGTH; ++i )
	{
		if ( uLangMask & (1UL<<i) )
		{
			switch ( i )
			{
			case AOT_RU:
				pTokenizer = new CSphAotTokenizerRu ( pTokenizer, pDict, bIndexExact );
				break;
			case AOT_UK:
				pTokenizer = new TokenizerUk_c ( pTokenizer, pDict, bIndexExact );
				break;
			default:
				pTokenizer = new CSphAotTokenizer ( pTokenizer, pDict, bIndexExact, i );
			}
		}
	}
}


void sphAotShutdown ()
{
	for ( auto& pLemmantizer : g_pLemmatizers )
		SafeDelete ( pLemmantizer );
	g_pUkLemmatizer.reset();
}

bool NativeUkLemmatizer_c::ReadSection ( CSphReader & rd, const UkSectionDesc_t & tSection, CSphVector<BYTE> & dOut, CSphString & sError )
{
	if ( !tSection.m_uPackedSize || tSection.m_uPackedSize>INT_MAX || tSection.m_uUnpackedSize>INT_MAX )
	{
		sError = "invalid Ukrainian lemmatizer section size";
		return false;
	}

	CSphVector<BYTE> dPacked;
	dPacked.Resize ( (int)tSection.m_uPackedSize );
	rd.SeekTo ( tSection.m_uOffset, (int)tSection.m_uPackedSize );
	rd.GetBytes ( dPacked.Begin(), tSection.m_uPackedSize );
	if ( rd.GetErrorFlag() )
	{
		sError = "failed to read Ukrainian lemmatizer section";
		return false;
	}

	switch ( tSection.m_uCodec )
	{
	case UK_CODEC_NONE:
		if ( tSection.m_uPackedSize!=tSection.m_uUnpackedSize )
		{
			sError = "invalid uncompressed Ukrainian lemmatizer section size";
			return false;
		}
		dOut.SwapData ( dPacked );
		return true;

	case UK_CODEC_LZ4:
		dOut.Resize ( (int)tSection.m_uUnpackedSize );
		if ( LZ4_decompress_safe ( (const char*)dPacked.Begin(), (char*)dOut.Begin(), (int)tSection.m_uPackedSize, (int)tSection.m_uUnpackedSize )!= (int)tSection.m_uUnpackedSize )
		{
			sError = "failed to decompress Ukrainian lemmatizer section";
			return false;
		}
		return true;

	default:
		sError.SetSprintf ( "unsupported Ukrainian lemmatizer section codec %u", tSection.m_uCodec );
		return false;
	}
}

const UkSectionDesc_t * NativeUkLemmatizer_c::FindSection ( const CSphVector<UkSectionDesc_t> & dSections, DWORD uType )
{
	for ( const auto & tSection : dSections )
		if ( tSection.m_uType==uType )
			return &tSection;

	return nullptr;
}

const CSphString * NativeUkLemmatizer_c::GetStringById ( const CSphVector<CSphString> & dStrings, DWORD uId, const char * sWhat, CSphString & sError )
{
	if ( uId>= (DWORD)dStrings.GetLength() )
	{
		sError.SetSprintf ( "invalid Ukrainian lemmatizer %s id %u", sWhat, uId );
		return nullptr;
	}

	return &dStrings[uId];
}

void NativeUkLemmatizer_c::AddUniqueLemma ( std::vector<CSphString> & dLemmas, const CSphString & sLemma )
{
	if ( sLemma.IsEmpty() )
		return;

	for ( const auto & sExisting : dLemmas )
		if ( sExisting==sLemma )
			return;

	dLemmas.emplace_back ( sLemma );
}

bool NativeUkLemmatizer_c::LoadV3 ( CSphReader & rd, CSphString & sError )
{
	DWORD uSectionCount = rd.GetDword();
	m_uPredictionMinWordLen = rd.GetDword();
	m_uPredictionMaxSuffixLen = rd.GetDword();
	DWORD uReserved = rd.GetDword();
	if ( rd.GetErrorFlag() || !uSectionCount || uReserved )
	{
		sError = "failed to read Ukrainian lemmatizer header";
		return false;
	}

	CSphVector<UkSectionDesc_t> dSections;
	dSections.Resize ( uSectionCount );
	for ( DWORD i=0; i<uSectionCount; ++i )
	{
		auto & tSection = dSections[i];
		tSection.m_uType = rd.GetDword();
		tSection.m_uCodec = rd.GetDword();
		rd.Read ( tSection.m_uOffset );
		rd.Read ( tSection.m_uPackedSize );
		rd.Read ( tSection.m_uUnpackedSize );
		tSection.m_uFlags = rd.GetDword();
		tSection.m_uReserved = rd.GetDword();
		if ( rd.GetErrorFlag() || !tSection.m_uOffset || !tSection.m_uPackedSize || !tSection.m_uUnpackedSize || tSection.m_uReserved )
		{
			sError.SetSprintf ( "failed to read Ukrainian lemmatizer section %u", i );
			return false;
		}
	}

	const auto * pStrings = FindSection ( dSections, UK_SECTION_STRINGS );
	const auto * pExact = FindSection ( dSections, UK_SECTION_EXACT );
	const auto * pPredictions = FindSection ( dSections, UK_SECTION_PREDICTIONS );
	if ( !pStrings || !pExact || !pPredictions )
	{
		sError = "incomplete Ukrainian lemmatizer sections";
		return false;
	}

	CSphVector<BYTE> dStringsSection;
	CSphVector<BYTE> dExactSection;
	CSphVector<BYTE> dPredictionSection;
	if ( !ReadSection ( rd, *pStrings, dStringsSection, sError )
		|| !ReadSection ( rd, *pExact, dExactSection, sError )
		|| !ReadSection ( rd, *pPredictions, dPredictionSection, sError ) )
		return false;

	UkMemoryReader_c tStringsReader ( dStringsSection );
	DWORD uStringCount = 0;
	if ( !tStringsReader.GetDword ( uStringCount ) )
	{
		sError = "failed to read Ukrainian lemmatizer string table size";
		return false;
	}

	CSphVector<CSphString> dStrings;
	dStrings.Resize ( uStringCount );
	for ( DWORD i=0; i<uStringCount; ++i )
		if ( !tStringsReader.GetString ( dStrings[i] ) )
		{
			sError.SetSprintf ( "failed to read Ukrainian lemmatizer string %u", i );
			return false;
		}

	UkMemoryReader_c tExactReader ( dExactSection );
	DWORD uEntries = 0;
	if ( !tExactReader.GetDword ( uEntries ) )
	{
		sError = "failed to read Ukrainian lemmatizer exact entry count";
		return false;
	}

	for ( DWORD i=0; i<uEntries; ++i )
	{
		DWORD uWordId = 0;
		DWORD uLemmaCount = 0;
		if ( !tExactReader.GetDword ( uWordId ) || !tExactReader.GetDword ( uLemmaCount ) || !uLemmaCount )
		{
			sError.SetSprintf ( "failed to read Ukrainian lemmatizer exact entry %u", i );
			return false;
		}

		const auto * pWord = GetStringById ( dStrings, uWordId, "word", sError );
		if ( !pWord || pWord->IsEmpty() )
		{
			sError.SetSprintf ( "invalid Ukrainian lemmatizer word at entry %u", i );
			return false;
		}

		auto & dLemmas = m_mLemmas [ pWord->cstr() ];
		dLemmas.clear();
		for ( DWORD j=0; j<uLemmaCount; ++j )
		{
			DWORD uLemmaId = 0;
			if ( !tExactReader.GetDword ( uLemmaId ) )
			{
				sError.SetSprintf ( "failed to read Ukrainian lemmatizer lemma id at entry %u", i );
				return false;
			}

			const auto * pLemma = GetStringById ( dStrings, uLemmaId, "lemma", sError );
			if ( !pLemma || pLemma->IsEmpty() )
			{
				sError.SetSprintf ( "invalid Ukrainian lemmatizer lemma at entry %u", i );
				return false;
			}

			AddUniqueLemma ( dLemmas, *pLemma );
		}
	}

	UkMemoryReader_c tPredictionReader ( dPredictionSection );
	DWORD uPredictionCount = 0;
	if ( !tPredictionReader.GetDword ( uPredictionCount ) )
	{
		sError = "failed to read Ukrainian lemmatizer prediction count";
		return false;
	}

	for ( DWORD i=0; i<uPredictionCount; ++i )
	{
		DWORD uEndingId = 0;
		if ( !tPredictionReader.GetDword ( uEndingId ) )
		{
			sError.SetSprintf ( "failed to read Ukrainian prediction suffix id at rule %u", i );
			return false;
		}

		const auto * pEnding = GetStringById ( dStrings, uEndingId, "prediction suffix", sError );
		if ( !pEnding )
			return false;

		PredictionRule_t tRule;
		DWORD uRequiredPrefixId = 0;
		DWORD uFixedSuffixId = 0;
		DWORD uCurrentPrefixId = 0;
		DWORD uCurrentSuffixId = 0;
		DWORD uNormalPrefixId = 0;
		DWORD uNormalSuffixId = 0;
		if ( !tPredictionReader.GetDword ( uRequiredPrefixId )
			|| !tPredictionReader.GetDword ( uFixedSuffixId )
			|| !tPredictionReader.GetDword ( uCurrentPrefixId )
			|| !tPredictionReader.GetDword ( uCurrentSuffixId )
			|| !tPredictionReader.GetDword ( uNormalPrefixId )
			|| !tPredictionReader.GetDword ( uNormalSuffixId )
			|| !tPredictionReader.GetDword ( tRule.m_uCount ) )
		{
			sError.SetSprintf ( "failed to read Ukrainian prediction payload at rule %u", i );
			return false;
		}

		const auto * pRequiredPrefix = GetStringById ( dStrings, uRequiredPrefixId, "required prefix", sError );
		const auto * pFixedSuffix = GetStringById ( dStrings, uFixedSuffixId, "fixed suffix", sError );
		const auto * pCurrentPrefix = GetStringById ( dStrings, uCurrentPrefixId, "current prefix", sError );
		const auto * pCurrentSuffix = GetStringById ( dStrings, uCurrentSuffixId, "current suffix", sError );
		const auto * pNormalPrefix = GetStringById ( dStrings, uNormalPrefixId, "normal prefix", sError );
		const auto * pNormalSuffix = GetStringById ( dStrings, uNormalSuffixId, "normal suffix", sError );
		if ( !pRequiredPrefix || !pFixedSuffix || !pCurrentPrefix || !pCurrentSuffix || !pNormalPrefix || !pNormalSuffix )
			return false;

		tRule.m_sRequiredPrefix = *pRequiredPrefix;
		tRule.m_sFixedSuffix = *pFixedSuffix;
		tRule.m_sCurrentPrefix = *pCurrentPrefix;
		tRule.m_sCurrentSuffix = *pCurrentSuffix;
		tRule.m_sNormalPrefix = *pNormalPrefix;
		tRule.m_sNormalSuffix = *pNormalSuffix;
		m_mPredictionRules [ pEnding->cstr() ].emplace_back ( std::move ( tRule ) );
	}

	return true;
}

bool NativeUkLemmatizer_c::Load ( CSphReader & rd, CSphString & sError )
{

	std::array<char,UK_LEMMATIZER_MAGIC.size()> dMagic {};
	rd.GetBytes ( dMagic.data(), dMagic.size() );
	if ( rd.GetErrorFlag() )
	{
		sError = "failed to read Ukrainian lemmatizer asset header";
		return false;
	}

	if ( dMagic!=UK_LEMMATIZER_MAGIC )
	{
		sError = "invalid Ukrainian lemmatizer asset magic";
		return false;
	}

	auto uVersion = rd.GetDword();
	if ( rd.GetErrorFlag() || uVersion!=UK_LEMMATIZER_VERSION )
	{
		sError.SetSprintf ( "unsupported Ukrainian lemmatizer asset version %u", uVersion );
		return false;
	}

	m_uPredictionMinWordLen = 4;
	m_uPredictionMaxSuffixLen = 5;
	m_mLemmas.clear();
	m_mPredictionRules.clear();

	return LoadV3 ( rd, sError );
}

void NativeUkLemmatizer_c::AddUniqueLemma ( CSphVector<CSphString> & dLemmas, const char * sLemma )
{
	if ( !sLemma || !*sLemma )
		return;

	for ( const auto & sExisting : dLemmas )
		if ( sExisting==sLemma )
			return;

	dLemmas.Add ( sLemma );
}

void NativeUkLemmatizer_c::AddAffixedLemmas ( const CSphVector<CSphString> & dBaseLemmas, const char * sPrefix, const char * sSuffix, CSphVector<CSphString> & dLemmas )
{
	for ( const auto & sLemma : dBaseLemmas )
	{
		std::string sFullLemma = sPrefix ? sPrefix : "";
		sFullLemma += sLemma.cstr();
		if ( sSuffix )
			sFullLemma += sSuffix;
		AddUniqueLemma ( dLemmas, sFullLemma.c_str() );
	}
}

void NativeUkLemmatizer_c::AddHyphenatedLemmas ( const char * sLeft, const CSphVector<CSphString> & dRightLemmas, CSphVector<CSphString> & dLemmas )
{
	for ( const auto & sRightLemma : dRightLemmas )
	{
		std::string sCombined = sLeft;
		sCombined += "-";
		sCombined += sRightLemma.cstr();
		AddUniqueLemma ( dLemmas, sCombined.c_str() );
	}
}

bool NativeUkLemmatizer_c::StartsWith ( const std::string & sValue, const char * sPrefix )
{
	return sValue.rfind ( sPrefix, 0 )==0;
}

bool NativeUkLemmatizer_c::EndsWith ( const std::string & sValue, const char * sSuffix )
{
	const auto iSuffixLen = strlen ( sSuffix );
	return sValue.length()>=iSuffixLen && sValue.compare ( sValue.length()-iSuffixLen, iSuffixLen, sSuffix )==0;
}

std::string NativeUkLemmatizer_c::ApplyUkCharSubstitute ( const std::string & sWord )
{
	std::string sResult;
	bool bChanged = false;
	const char * pCur = sWord.c_str();
	while ( *pCur )
	{
		const BYTE * pCharStart = (const BYTE *)pCur;
		const BYTE * pNext = pCharStart;
		int iCode = sphUTF8Decode ( pNext );
		if ( iCode==0x433 )
		{
			sResult += UK_CHAR_SUBSTITUTE_TO;
			pCur = (const char *)pNext;
			bChanged = true;
			continue;
		}

		sResult.append ( pCur, (const char *)pNext - pCur );
		pCur = (const char *)pNext;
	}

	return bChanged ? sResult : sWord;
}

bool NativeUkLemmatizer_c::HasKnownDashPrefix ( const std::string & sWord )
{
	for ( const auto * sPrefix : UK_KNOWN_PREFIXES )
		if ( strchr ( sPrefix, '-' ) && StartsWith ( sWord, sPrefix ) )
			return true;

	return false;
}

bool NativeUkLemmatizer_c::LookupExact ( const std::string & sWord, CSphVector<CSphString> & dLemmas ) const
{
	auto tAddEntry = [this, &dLemmas] ( const std::string & sLookupWord )
	{
		auto it = m_mLemmas.find ( sLookupWord );
		if ( it==m_mLemmas.end() )
			return false;

		for ( const auto & sLemma : it->second )
			AddUniqueLemma ( dLemmas, sLemma.cstr() );
		return true;
	};

	bool bFound = tAddEntry ( sWord );

	auto sSubstitute = ApplyUkCharSubstitute ( sWord );
	if ( sSubstitute!=sWord )
		bFound = tAddEntry ( sSubstitute ) || bFound;

	return bFound;
}

bool NativeUkLemmatizer_c::LookupPrediction ( const std::string & sWord, CSphVector<CSphString> & dLemmas ) const
{
	if ( m_mPredictionRules.empty() )
		return false;

	const int iWordCodepoints = sphUTF8Len ( sWord.c_str() );
	if ( iWordCodepoints < (int)m_uPredictionMinWordLen )
		return false;

	struct ScoredLemma_t
	{
		DWORD		m_uCount = 0;
		CSphString	m_sLemma;
	};

	auto tCollectPredictions = [this, &dLemmas, iWordCodepoints] ( const std::string & sLookupWord ) -> bool
	{
		std::vector<int> dSuffixStarts;
		dSuffixStarts.reserve ( Min ( iWordCodepoints, (int)m_uPredictionMaxSuffixLen ) );
		const BYTE * pCur = (const BYTE*)sLookupWord.c_str();
		const BYTE * pStart = pCur;
		while ( *pCur )
		{
			const BYTE * pCharStart = pCur;
			sphUTF8Decode ( pCur );
			dSuffixStarts.push_back ( int ( pCharStart - pStart ) );
		}

		const int iMaxSuffixChars = Min ( iWordCodepoints, (int)m_uPredictionMaxSuffixLen );
		for ( int iSuffixChars=iMaxSuffixChars; iSuffixChars>=1; --iSuffixChars )
		{
			const int iSuffixStart = dSuffixStarts [ iWordCodepoints - iSuffixChars ];
			auto sEnding = sLookupWord.substr ( iSuffixStart );
			auto itRules = m_mPredictionRules.find ( sEnding );
			if ( itRules==m_mPredictionRules.end() )
				continue;

			std::vector<ScoredLemma_t> dCandidates;
			for ( const auto & tRule : itRules->second )
			{
				if ( tRule.m_sRequiredPrefix.Length()>0 && sLookupWord.rfind ( tRule.m_sRequiredPrefix.cstr(), 0 )!=0 )
					continue;

				std::string sFixedWord = sLookupWord.substr ( 0, iSuffixStart );
				sFixedWord += tRule.m_sFixedSuffix.cstr();

				const auto iCurrentPrefixLen = tRule.m_sCurrentPrefix.Length();
				const auto iCurrentSuffixLen = tRule.m_sCurrentSuffix.Length();
				if ( iCurrentPrefixLen>0 && sFixedWord.rfind ( tRule.m_sCurrentPrefix.cstr(), 0 )!=0 )
					continue;
				if ( iCurrentSuffixLen>0 )
				{
					if ( (int)sFixedWord.length()<iCurrentSuffixLen )
						continue;
					if ( sFixedWord.compare ( sFixedWord.length()-iCurrentSuffixLen, iCurrentSuffixLen, tRule.m_sCurrentSuffix.cstr() )!=0 )
						continue;
				}
				if ( (int)sFixedWord.length() < iCurrentPrefixLen + iCurrentSuffixLen )
					continue;

				std::string sStem = sFixedWord.substr ( iCurrentPrefixLen, sFixedWord.length() - iCurrentPrefixLen - iCurrentSuffixLen );
				ScoredLemma_t tCandidate;
				tCandidate.m_uCount = tRule.m_uCount;
				std::string sLemma = tRule.m_sNormalPrefix.cstr();
				sLemma += sStem;
				sLemma += tRule.m_sNormalSuffix.cstr();
				tCandidate.m_sLemma = sLemma.c_str();
				dCandidates.emplace_back ( std::move ( tCandidate ) );
			}

			std::stable_sort ( dCandidates.begin(), dCandidates.end(), [] ( const ScoredLemma_t & a, const ScoredLemma_t & b )
			{
				return a.m_uCount > b.m_uCount;
			} );

			for ( const auto & tCandidate : dCandidates )
				AddUniqueLemma ( dLemmas, tCandidate.m_sLemma.cstr() );

			if ( dLemmas.GetLength()>0 )
				return true;
		}

		return false;
	};

	bool bFound = tCollectPredictions ( sWord );
	if ( bFound )
		return true;

	auto sSubstitute = ApplyUkCharSubstitute ( sWord );
	if ( sSubstitute!=sWord )
		return tCollectPredictions ( sSubstitute );

	return false;
}

bool NativeUkLemmatizer_c::LookupHyphenParticle ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const
{
	for ( const auto * sParticle : UK_PARTICLES_AFTER_HYPHEN )
	{
		if ( !EndsWith ( sWord, sParticle ) )
			continue;

		std::string sBase = sWord.substr ( 0, sWord.length() - strlen ( sParticle ) );
		if ( sBase.empty() )
			return false;

		CSphVector<CSphString> dBaseLemmas;
		if ( !LookupWord ( sBase, dBaseLemmas, iDepth+1 ) )
			return false;

		AddAffixedLemmas ( dBaseLemmas, "", sParticle, dLemmas );
		return dLemmas.GetLength()>0;
	}

	return false;
}

bool NativeUkLemmatizer_c::LookupHyphenatedWord ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const
{
	const auto iHyphen = sWord.find ( '-' );
	if ( iHyphen==std::string::npos || iHyphen==0 || iHyphen+1>=sWord.length() )
		return false;

	if ( sWord.find ( '-', iHyphen+1 )!=std::string::npos )
		return false;

	if ( HasKnownDashPrefix ( sWord ) )
		return false;

	std::string sLeft = sWord.substr ( 0, iHyphen );
	std::string sRight = sWord.substr ( iHyphen+1 );
	CSphVector<CSphString> dLeftLemmas;
	CSphVector<CSphString> dRightLemmas;

	const bool bLeftFound = LookupWord ( sLeft, dLeftLemmas, iDepth+1 );
	const bool bRightFound = LookupWord ( sRight, dRightLemmas, iDepth+1 );
	if ( !bRightFound )
		return false;

	if ( bLeftFound )
		for ( const auto & sLeftLemma : dLeftLemmas )
			AddHyphenatedLemmas ( sLeftLemma.cstr(), dRightLemmas, dLemmas );

	AddHyphenatedLemmas ( sLeft.c_str(), dRightLemmas, dLemmas );

	return dLemmas.GetLength()>0;
}

bool NativeUkLemmatizer_c::LookupKnownPrefix ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const
{
	for ( const auto * sPrefix : UK_KNOWN_PREFIXES )
	{
		if ( !StartsWith ( sWord, sPrefix ) )
			continue;

		std::string sRemainder = sWord.substr ( strlen ( sPrefix ) );
		if ( sphUTF8Len ( sRemainder.c_str() )<3 )
			continue;

		CSphVector<CSphString> dBaseLemmas;
		if ( !LookupWord ( sRemainder, dBaseLemmas, iDepth+1 ) )
			continue;

		AddAffixedLemmas ( dBaseLemmas, sPrefix, "", dLemmas );

		if ( dLemmas.GetLength()>0 )
			return true;
	}

	return false;
}

bool NativeUkLemmatizer_c::LookupWord ( const std::string & sWord, CSphVector<CSphString> & dLemmas, int iDepth ) const
{
	if ( iDepth>UK_MAX_ANALOGY_DEPTH || sWord.empty() )
		return false;

	if ( LookupExact ( sWord, dLemmas ) )
		return true;

	if ( LookupHyphenParticle ( sWord, dLemmas, iDepth ) )
		return true;

	if ( LookupHyphenatedWord ( sWord, dLemmas, iDepth ) )
		return true;

	if ( LookupKnownPrefix ( sWord, dLemmas, iDepth ) )
		return true;

	return LookupPrediction ( sWord, dLemmas );
}

bool NativeUkLemmatizer_c::Lookup ( const BYTE * pWord, CSphVector<CSphString> & dLemmas ) const
{
	if ( !pWord || !*pWord )
		return false;

	return LookupWord ( (const char*)pWord, dLemmas, 0 );
}

bool LoadLemmatizerUk ( const CSphString & sDictFile, CSphString & sError )
{
	if ( g_pUkLemmatizer )
		return true;

	CSphAutofile rdFile;
	if ( rdFile.Open ( sDictFile, SPH_O_READ, sError )<0 )
		return false;

	auto tLemmatizer = std::make_unique<NativeUkLemmatizer_c>();
	CSphReader rd;
	rd.SetFile ( rdFile );
	if ( !tLemmatizer->Load ( rd, sError ) )
		return false;

	DWORD uCrc = 0;
	if ( !sphCalcFileCRC32 ( sDictFile.cstr(), uCrc ) )
	{
		sError.SetSprintf ( "failed to crc32 Ukrainian lemmatizer asset %s", sDictFile.cstr() );
		return false;
	}

	const char * a = sDictFile.cstr();
	const char * b = a + strlen(a) - 1;
	while ( b>a && b[-1]!='/' && b[-1]!='\\' )
		b--;

	g_tDictinfos[AOT_UK] = { b, (int)uCrc };
	g_pUkLemmatizer = std::move ( tLemmatizer );
	return true;
}

LemmatizerUk_c::LemmatizerUk_c() = default;
LemmatizerUk_c::~LemmatizerUk_c() = default;

BYTE * LemmatizerUk_c::GetToken ( const BYTE * pWord, int & iExtra )
{
	m_dLemmas.Resize ( 0 );
	m_iCurrent = 0;
	iExtra = 0;

	if ( !g_pUkLemmatizer || !g_pUkLemmatizer->Lookup ( pWord, m_dLemmas ) )
		return (BYTE *)pWord;

	if ( m_dLemmas.GetLength()>1 )
		iExtra = m_dLemmas.GetLength() - 1;

	strncpy ( (char*)m_sResultToken, m_dLemmas[0].cstr(), sizeof(m_sResultToken)-1 );
	m_sResultToken[sizeof(m_sResultToken)-1] = '\0';
	return m_sResultToken;
}

BYTE * LemmatizerUk_c::GetExtraToken()
{
	if ( m_iCurrent + 1 >= m_dLemmas.GetLength() )
		return nullptr;

	++m_iCurrent;
	strncpy ( (char*)m_sResultToken, m_dLemmas[m_iCurrent].cstr(), sizeof(m_sResultToken)-1 );
	m_sResultToken[sizeof(m_sResultToken)-1] = '\0';
	return m_sResultToken;
}

static bool SkipNonUkToken ( const BYTE * pWord )
{
	// pass-through 1-char "words"
	if ( pWord[1]=='\0' )
		return true;

	int iCodepoints = 0;
	int iCode = 0;

	while ( ( iCode = sphUTF8Decode ( pWord ) )>0 )
	{
		iCodepoints++;
		if ( iCode>=0x400 && iCode<=0x4ff )
			continue;

		if ( iCode=='-' )
			continue;

		// allow non_cjk uk mapping too
		if ( iCode==0x69 || iCode==0x73 || iCode==0xE6 )
			continue;

		// pass-through words with non ukrainian chars
		return true;
	}

	// pass-through 1-char "words"
	return ( iCodepoints<2 );
}

void sphAotLemmatizeUk ( BYTE * pWord, LemmatizerTrait_i * pLemmatizer )
{
	if ( !pLemmatizer )
		return;

	if ( SkipNonUkToken ( pWord ) )
		return;

	int iExtraCount = 0;
	const BYTE * pDst = pLemmatizer->GetToken ( pWord, iExtraCount );
	strcpy ( (char*)pWord, (char*)pDst ); // NOLINT
}

void sphAotLemmatizeUk ( StrVec_t & dLemmas, const BYTE * pWord, LemmatizerTrait_i * pLemmatizer )
{
	if ( !pLemmatizer )
		return;

	if ( SkipNonUkToken ( pWord ) )
		return;

	int iExtraCount = 0;
	dLemmas.Add ( (const char *)pLemmatizer->GetToken ( pWord, iExtraCount ) );

	iExtraCount = Min ( iExtraCount, MAX_EXTRA_TOKENS );
	for ( int i=0; i<iExtraCount; i++ )
		dLemmas.Add (  (const char *)pLemmatizer->GetExtraToken () );

	for ( int i=0; i<dLemmas.GetLength(); ++i )
		for ( int j=i+1; j<dLemmas.GetLength(); )
			if ( dLemmas[i]==dLemmas[j] )
				dLemmas.Remove ( j );
			else
				++j;
}

std::unique_ptr<LemmatizerTrait_i> CreateLemmatizer ( int iLang )
{
	if ( iLang!=AOT_UK )
		return nullptr;

	if ( !g_pUkLemmatizer )
		return nullptr;

	return std::make_unique<LemmatizerUk_c>();
}

TokenizerUk_c::TokenizerUk_c ( TokenizerRefPtr_c pTok, const DictRefPtr_c& pDict, bool bIndexExact )
	: CSphAotTokenizerTmpl ( std::move (pTok), pDict, bIndexExact, AOT_UK )
{
}

TokenizerRefPtr_c TokenizerUk_c::Clone ( ESphTokenizerClone eMode ) const noexcept
{
	// this token filter must NOT be created as escaped
	// it must only be used during indexing time, NEVER in searching time
	assert ( eMode==SPH_CLONE_INDEX );
	auto * pClone = new TokenizerUk_c ( m_pTokenizer->Clone ( eMode ), nullptr, m_bIndexExact );
	if ( m_pWordforms )
		pClone->m_pWordforms = m_pWordforms;
	return TokenizerRefPtr_c { pClone };
}

BYTE * TokenizerUk_c::GetToken()
{
		m_eTokenMorph = SPH_TOKEN_MORPH_RAW;

		// any pending lemmas left?
		if ( m_iCurrent>=0 )
		{
			++m_iCurrent;
			assert ( m_FindResults[m_iCurrent]!=AOT_NOFORM );

			// return original token
			if ( m_FindResults[m_iCurrent]==AOT_ORIGFORM )
			{
				assert ( m_FindResults[m_iCurrent+1]==AOT_NOFORM );
				strncpy ( (char*)m_sToken, (char*)m_sOrigToken, sizeof(m_sToken) );
				m_sToken[sizeof ( m_sToken ) - 1] = '\0';
				m_iCurrent = -1;
				m_eTokenMorph = SPH_TOKEN_MORPH_ORIGINAL;
				return m_sToken;
			}

			// generate that extra lemma
			BYTE * pToken = m_tLemmatizer.GetExtraToken();

			// is this the last one? gotta tag it non-blended
			if ( m_FindResults [ m_iCurrent+1 ]==AOT_NOFORM )
				m_iCurrent = -1;

			if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
				m_pWordforms->ToNormalForm ( pToken, false, false );

			m_eTokenMorph = SPH_TOKEN_MORPH_GUESS;
			return pToken;
		}

		// ok, time to work on a next word
		assert ( m_iCurrent<0 );
		BYTE * pToken = Base::GetToken();
		m_eTokenMorph = m_pTokenizer->GetTokenMorph();
		if ( !pToken )
			return nullptr;

		// pass-through blended parts
		if ( m_pTokenizer->TokenIsBlended() )
			return pToken;

		// pass-through matched wordforms
		if ( m_pWordforms && m_pWordforms->ToNormalForm ( pToken, true, false ) )
			return pToken;

		if ( SkipNonUkToken ( pToken ) )
			return pToken;

		// lemmatize
		int iExtra = 0;
		pToken = m_tLemmatizer.GetToken ( pToken, iExtra );
		
		// FIXME!!! implement token pass throu
		m_FindResults[0] = 0;
		int iLastEmpty = 1;
		int iTokensEnd = Min ( iLastEmpty+iExtra, MAX_EXTRA_TOKENS-1 );
		for ( ; iLastEmpty<iTokensEnd; iLastEmpty++ )
			m_FindResults[iLastEmpty] = 0;
		m_FindResults[iLastEmpty] = AOT_NOFORM;

		// schedule original form for return, if needed, will be last token
		if ( m_bIndexExact )
		{
			iLastEmpty = Min ( iLastEmpty, MAX_EXTRA_TOKENS-2 );
			m_FindResults[iLastEmpty] = AOT_ORIGFORM;
			m_FindResults[iLastEmpty+1] = AOT_NOFORM;
			strncpy ( (char*)m_sOrigToken, (char*)pToken, sizeof(m_sOrigToken) );
			m_sOrigToken[sizeof ( m_sOrigToken ) - 1] = '\0';
		}

		// schedule lemmas 2+ for return
		if ( m_FindResults[1]!=AOT_NOFORM )
			m_iCurrent = 0;

		// suddenly, post-morphology wordforms
		if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
			m_pWordforms->ToNormalForm ( pToken, false, false );

		m_eTokenMorph = SPH_TOKEN_MORPH_GUESS;
		return pToken;
}


namespace {

/// create a node from a set of lemmas
/// WARNING, tKeyword might or might not be pointing to pNode->m_dWords[0]
/// Called from the daemon side (searchd) in time of query
void TransformAotFilterKeyword ( XQNode_t * pNode, LemmatizerTrait_i * pLemmatizer, const XQKeyword_t & tKeyword, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	assert ( pNode->dWords().GetLength()<=1 );
	assert ( pNode->dChildren().IsEmpty() );

	XQNode_t * pExact = nullptr;
	if ( pWordforms )
	{
		// do a copy, because patching in place is not an option
		// short => longlonglong wordform mapping would crash
		// OPTIMIZE? forms that are not found will (?) get looked up again in the dict
		char sBuf [ MAX_KEYWORD_BYTES ];
		strncpy ( sBuf, tKeyword.m_sWord.cstr(), sizeof(sBuf)-1 );
		if ( pWordforms->ToNormalForm ( (BYTE*)sBuf, true, false ) )
		{
			if ( pNode->dWords().IsEmpty() )
				pNode->AddDirtyWord ( tKeyword );
			pNode->WithWord ( 0,[&sBuf] (auto& dWord) {
				dWord.m_sWord = sBuf;
				dWord.m_bMorphed = true;
			});
			return;
		}
	}

	StrVec_t dLemmas;
	DWORD uLangMask = tSettings.m_uAotFilterMask;
	for ( int i=AOT_BEGIN; i<AOT_LENGTH; ++i )
	{
		if ( uLangMask & (1UL<<i) )
		{
			if ( i==AOT_RU )
				sphAotLemmatizeRu ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr() );
			else if ( i==AOT_DE )
				sphAotLemmatizeDe ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr() );
			else if ( i==AOT_UK )
				sphAotLemmatizeUk ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr(), pLemmatizer );
			else
				sphAotLemmatize ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr(), i );
		}
	}

	// post-morph wordforms
	if ( pWordforms && pWordforms->m_bHavePostMorphNF )
	{
		char sBuf [ MAX_KEYWORD_BYTES ];
		ARRAY_FOREACH ( i, dLemmas )
		{
			strncpy ( sBuf, dLemmas[i].cstr(), sizeof(sBuf)-1 );
			if ( pWordforms->ToNormalForm ( (BYTE*)sBuf, false, false ) )
				dLemmas[i] = sBuf;
		}
	}

	if ( dLemmas.GetLength() && tSettings.m_bIndexExactWords )
	{
		pExact = CloneKeyword ( pNode );
		if ( pExact->dWords().IsEmpty() )
			pExact->AddDirtyWord ( tKeyword );

		pExact->WithWord(0,[&tKeyword](auto& dWord) {dWord.m_sWord.SetSprintf ( "=%s", tKeyword.m_sWord.cstr() ); });
	}

	if ( !pExact && dLemmas.GetLength()<=1 )
	{
		// zero or one lemmas, update node in-place
		if ( pNode->dWords().IsEmpty() )
			pNode->AddDirtyWord ( tKeyword );
		if ( dLemmas.GetLength() )
			pNode->WithWord ( 0, [&dLemmas] ( auto & dWord ) {
				dWord.m_sWord = dLemmas[0];
				dWord.m_bMorphed = true;
			} );
	} else
	{
		// multiple lemmas, create an OR node
		pNode->SetOp ( SPH_QUERY_OR );
		ARRAY_FOREACH ( i, dLemmas )
		{
			XQKeyword_t tLemma;
			tLemma.m_sWord = dLemmas[i];
			tLemma.m_iAtomPos = tKeyword.m_iAtomPos;
			tLemma.m_bFieldStart = tKeyword.m_bFieldStart;
			tLemma.m_bFieldEnd = tKeyword.m_bFieldEnd;
			tLemma.m_bMorphed = true;
			pNode->AddNewChild ( new XQNode_t ( pNode->m_dSpec ) );
			pNode->dChildren().Last()->AddDirtyWord ( tLemma );
		}
		pNode->ResetWords();
		if ( pExact )
			pNode->AddNewChild ( pExact );
	}
}
}// namespace

/// AOT morph guesses transform
/// replaces tokens with their respective morph guesses subtrees
/// used in lemmatize_ru_all morphology processing mode that can generate multiple guesses
/// in other modes, there is always exactly one morph guess, and the dictionary handles it
/// Called from the daemon side (searchd)
void TransformAotFilter ( XQNode_t * pNode, LemmatizerTrait_i * pLemmatizer, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	if ( !pNode )
		return;
	// case one, regular operator (and empty nodes)
	for ( XQNode_t* pChild : pNode->dChildren() )
		TransformAotFilter ( pChild, pLemmatizer, pWordforms, tSettings );
	if ( pNode->dChildren().GetLength() || pNode->dWords().IsEmpty() )
		return;

	// case two, operator on a bag of words
	// FIXME? check phrase vs expand_keywords vs lemmatize_ru_all?
	if ( pNode->dWords().GetLength()
		&& ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM ) )
	{
		assert ( pNode->dWords().GetLength() );

		for ( const XQKeyword_t& tWord : pNode->dWords() )
		{
			auto * pNew = new XQNode_t ( pNode->m_dSpec );
			pNew->m_iAtomPos = tWord.m_iAtomPos;
			pNode->AddNewChild ( pNew );
			TransformAotFilterKeyword ( pNew, pLemmatizer, tWord, pWordforms, tSettings );
		}

		pNode->ResetWords();
		pNode->m_bVirtuallyPlain = true;
		return;
	}

	// case three, plain old single keyword
	assert ( pNode->dWords().GetLength()==1 );
	TransformAotFilterKeyword ( pNode, pLemmatizer, pNode->dWord(0), pWordforms, tSettings );
}

void TransformAotFilter ( XQNode_t * pNode, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	if ( !tSettings.m_uAotFilterMask )
		return;

	int iAotLang = ( tSettings.m_uAotFilterMask & ( 1<<AOT_UK ) ) ? AOT_UK : AOT_LENGTH;
	std::unique_ptr<LemmatizerTrait_i> tLemmatizer = CreateLemmatizer ( iAotLang );
	TransformAotFilter ( pNode, tLemmatizer.get(), pWordforms, tSettings );
}
