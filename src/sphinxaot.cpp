//
// $Id$
//

//
// Copyright (c) 2011-2013, Andrew Aksyonoff
// Copyright (c) 2011-2013, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
// Based on AOT lemmatizer, http://aot.ru/
// Copyright (c) 2004-2013, Alexey Sokirko and others
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxutils.h"

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
		SafeDelete ( m_pNodes );
		SafeDelete ( m_pRelations );
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


/// lemmatizer
class CLemmatizer
{
protected:
	static const int			MAX_PREFIX_LEN = 12;
	static const bool			m_bMaximalPrediction = false;
	static const bool			m_bIsGerman = false;

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
	CSphVector<CFlexiaModel>	m_FlexiaModels;			///< flexia models

	bool						LemmatizeWord ( BYTE * pWord, DWORD * results ) const;
	bool						LoadPak ( CSphReader & rd );
};

//////////////////////////////////////////////////////////////////////////

DWORD CABCEncoder::DecodeFromAlphabet ( const BYTE * sPath, int iPath ) const
{
	int c = 1;
	int Result = 0;
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
	BYTE sPath[128];
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
	const bool bCap = false; // maybe when we manage to drag this all the way from tokenizer
	const bool bPredict = true;

	// uppercase (and maybe other translations), check, and compute length
	BYTE * p;
	for ( p = pWord; *p; p++ )
	{
		// uppercase
		*p = m_UC[*p];

		// russian chars are in 0xC0..0xDF range
		// avoid lemmatizing words with other chars in them
		if ( ( *p>>5 )!=6 )
		{
			*results = AOT_NOFORM;
			return false;
		}
	}
	int iLen = (int)( p-pWord );

	// do dictionary lookup
	m_FormAutomat.GetInnerMorphInfos ( pWord, results );
	if ( *results!=AOT_NOFORM )
		return true;
	if ( !bPredict )
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
	for ( DWORD * p=results; *p!=AOT_NOFORM; p++ )
		if ( m_NPSs[ AOT_MODEL_NO(*p) ]==AOT_POS_UNKNOWN )
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

	ARRAY_FOREACH ( j, res )
	{
		BYTE PartOfSpeechNo = res[j].m_PartOfSpeechNo;
		if	( !m_bMaximalPrediction && has_nps[PartOfSpeechNo]!=-1 )
		{
			int iOldFreq = m_ModelFreq [ AOT_MODEL_NO ( FindResults[has_nps[PartOfSpeechNo]] ) ];
			int iNewFreq = m_ModelFreq [ m_LemmaFlexiaModel [ res[j].m_LemmaInfoNo ] ];
			if ( iOldFreq < iNewFreq )
				FindResults [ has_nps [ PartOfSpeechNo ] ] = PredictPack ( res[j] );
			continue;
		}

		has_nps [ PartOfSpeechNo ] = (int)( pOut-FindResults );
		*pOut++ = PredictPack ( res[j] );
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

static CLemmatizer *	g_pLemmatizer = NULL;
static CSphNamedInt		g_tDictinfo;

void sphAotSetCacheSize ( int iCacheSize )
{
	g_iCacheSize = Max ( iCacheSize, 0 );
}

bool sphAotInitRu ( const CSphString & sDictFile, CSphString & sError )
{
	if ( g_pLemmatizer )
		return true;

	CSphAutofile rdFile;
	if ( rdFile.Open ( sDictFile, SPH_O_READ, sError )<0 )
		return false;

	g_pLemmatizer = new CLemmatizer ();

	CSphReader rd;
	rd.SetFile ( rdFile );
	if ( !g_pLemmatizer->LoadPak(rd) )
	{
		sError.SetSprintf ( "failed to load lemmatizer dictionary: %s", rd.GetErrorMessage().cstr() );
		SafeDelete ( g_pLemmatizer );
		return false;
	}

	// track dictionary crc
	DWORD uCrc;
	if ( !sphCalcFileCRC32 ( sDictFile.cstr(), uCrc ) )
	{
		sError.SetSprintf ( "failed to crc32 lemmatizer dictionary %s", sDictFile.cstr() );
		SafeDelete ( g_pLemmatizer );
		return false;
	}

	// extract basename
	const char * a = sDictFile.cstr();
	const char * b = a + strlen(a) - 1;
	while ( b>a && b[-1]!='/' && b[-1]!='\\' )
		b--;

	g_tDictinfo.m_sName = b;
	g_tDictinfo.m_iValue = (int)uCrc;
	return true;
}


static inline bool IsAlpha1251 ( BYTE c )
{
	return ( c>=0xC0 || c==0xA8 || c==0xB8 );
}


template < bool IS_UTF8 >
static inline BYTE * Emit ( BYTE * sOut, BYTE uChar );

template<>
inline BYTE * Emit<false> ( BYTE * sOut, BYTE uChar )
{
	if ( uChar=='-' )
		return sOut;
	*sOut++ = uChar | 0x20;
	return sOut;
}

template<>
inline BYTE * Emit<true> ( BYTE * sOut, BYTE uChar )
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

template < bool IS_UTF8 >
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
		( m_InputWordBase.substr ( LemmPrefix.length(), F.m_PrefixStr.length() ) == F.m_PrefixStr ) ) )
	{
		m_InputWordBase.erase(0, LemmPrefix.length()+ M.m_PrefixStr.length());
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
		iBaseLen -= iSuff;
		while ( iBaseLen-- )
			sOut = Emit<IS_UTF8> ( sOut, *sBase++ );

		int iLemmaSuff = M[0].m_FlexiaLen;
		const char * sFlexia = M[0].m_Flexia;
		while ( iLemmaSuff-- ) // OPTIMIZE? can remove len here
			sOut = Emit<IS_UTF8> ( sOut, *sFlexia++ );
	} else
	{
		// whoops, no suffix match, just copy and lowercase the current base
		while ( iBaseLen-- )
			sOut = Emit<IS_UTF8> ( sOut, *sBase++ );
	}
	*sOut = '\0';
}


static inline bool IsFreq2 ( BYTE * pWord )
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


static inline bool IsFreq3 ( BYTE * pWord )
{
	if ( pWord[3]!=0 )
		return false;
	int iCode = ( ( pWord[0]<<16 ) + ( pWord[1]<<8 ) + pWord[2] ) | 0x202020;
	return ( iCode==0xE8EBE8 || iCode==0xE4EBFF || iCode==0xEFF0E8 // ili, dlya, pri
		|| iCode==0xE3EEE4 || iCode==0xF7F2EE || iCode==0xE1E5E7 ); // god, chto, bez
}



void sphAotLemmatizeRu1251 ( BYTE * pWord )
{
	// i must be initialized
	assert ( g_pLemmatizer );

	// pass-through 1-char words, and non-Russian words
	if ( !IsAlpha1251(*pWord) || !pWord[1] )
		return;

	// handle a few most frequent 2-char, 3-char pass-through words
	if ( IsFreq2(pWord) || IsFreq3(pWord) )
		return;

	// do lemmatizing
	// input keyword moves into sForm; LemmatizeWord() will also case fold sForm
	// we will generate results using sForm into pWord; so we need this extra copy
	BYTE sForm [ SPH_MAX_WORD_LEN*3+4 ]; // aka MAX_KEYWORD_BYTES
	int iFormLen = 0;

	// faster than strlen and strcpy..
	for ( BYTE * p=pWord; *p; )
		sForm[iFormLen++] = *p++;
	sForm[iFormLen] = '\0';

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizer->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	// pick a single form
	// picks a noun, if possible, and otherwise prefers shorter forms
	bool bNoun = false;
	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizer->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		bool bNewNoun = ( F.m_POS==0 );
		if ( i==0 || ( !bNoun && bNewNoun ) )
		{
			CreateLemma<false> ( pWord, sForm, iFormLen, bFound, M, F );
			bNoun = bNewNoun;
		} else if ( bNoun==bNewNoun )
		{
			BYTE sBuf[256];
			CreateLemma<false> ( sBuf, sForm, iFormLen, bFound, M, F );
			if ( strcmp ( (char*)sBuf, (char*)pWord )<0 )
				strcpy ( (char*)pWord, (char*)sBuf );
		}
	}
}


static inline bool IsAlphaUtf8 ( const BYTE * pWord )
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
		register BYTE uWin = dTable [ ( pWord[1] & 0x7F ) + ( ( pWord[0] & 1 )<<6 ) ];
		pWord += 2;

		if ( !uWin )
			return 0;
		*pOut++ = uWin;
	}

	*pOut = '\0';
	return (int)( pOut-pStart );
}


static inline void Win1251ToUtf8 ( BYTE * pOut, const BYTE * pWord )
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


void sphAotLemmatizeRuUTF8 ( BYTE * pWord )
{
	// i must be initialized
	assert ( g_pLemmatizer );

	// only if the word is russian
	if ( !IsAlphaUtf8(pWord) )
		return;

	// convert to Windows-1251
	// failure means we should not lemmatize this
	BYTE sBuf [ SPH_MAX_WORD_LEN+4 ];
	if ( !Utf8ToWin1251 ( sBuf, pWord ) )
		return;

	// lemmatize, convert back, done!
	sphAotLemmatizeRu1251 ( sBuf );
	Win1251ToUtf8 ( pWord, sBuf );
}


void sphAotLemmatizeRu ( CSphVector<CSphString> & dLemmas, const BYTE * pWord, bool bUtf8 )
{
	assert ( g_pLemmatizer );
	if ( bUtf8 )
	{
		if ( !IsAlphaUtf8(pWord) )
			return;
	} else
	{
		if ( !IsAlpha1251(*pWord) )
			return;
	}

	BYTE sForm [ SPH_MAX_WORD_LEN+4 ];
	int iFormLen = 0;
	if ( bUtf8 )
	{
		iFormLen = Utf8ToWin1251 ( sForm, pWord );
	} else
	{
		while ( *pWord )
			sForm [ iFormLen++ ] = *pWord++;
		sForm [ iFormLen ] = '\0';
	}
	if ( iFormLen<=1 )
		return;

	if ( IsFreq2(sForm) || IsFreq3(sForm) )
		return;

	DWORD FindResults[12]; // max results is like 6
	bool bFound = g_pLemmatizer->LemmatizeWord ( (BYTE*)sForm, FindResults );
	if ( FindResults[0]==AOT_NOFORM )
		return;

	for ( int i=0; FindResults[i]!=AOT_NOFORM; i++ )
	{
		const CFlexiaModel & M = g_pLemmatizer->m_FlexiaModels [ AOT_MODEL_NO ( FindResults[i] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( FindResults[i] ) ];

		BYTE sRes [ 3*SPH_MAX_WORD_LEN+4 ];
		if ( bUtf8 )
			CreateLemma<true> ( sRes, sForm, iFormLen, bFound, M, F );
		else
			CreateLemma<false> ( sRes, sForm, iFormLen, bFound, M, F );

		dLemmas.Add ( (const char*)sRes );
	}

	// OPTIMIZE?
	dLemmas.Uniq();
}


const CSphNamedInt & sphAotDictinfoRu()
{
	return g_tDictinfo;
}

//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#pragma warning(disable:4127) // conditional expr is const for MSVC
#endif

/// token filter for AOT morphology indexing
/// AOT may return multiple (!) morphological hypotheses for a single token
/// we return such additional hypotheses as blended tokens
template < bool IS_UTF8 >
class CSphAotTokenizer : public CSphTokenFilter
{
protected:
	BYTE		m_sForm [ SPH_MAX_WORD_LEN*3+4 ];	///< aka MAX_KEYWORD_BYTES
	int			m_iFormLen;							///< in bytes, but in windows-1251 that is characters, too
	bool		m_bFound;							///< found or predicted?
	DWORD		m_FindResults[12];					///< max results is like 6
	int			m_iCurrent;							///< index in m_FindResults that was just returned, -1 means no blending
	BYTE		m_sToken [ SPH_MAX_WORD_LEN*3+4 ];	///< to hold generated lemmas
	BYTE		m_sOrigToken [ SPH_MAX_WORD_LEN*3+4 ];	///< to hold original token
	bool		m_bIndexExact;

	const CSphWordforms *	m_pWordforms;

public:
	CSphAotTokenizer ( ISphTokenizer * pTok, CSphDict * pDict, bool bIndexExact )
		: CSphTokenFilter ( pTok )
	{
		assert ( pTok );
		assert ( g_pLemmatizer );
		m_iCurrent = -1;
		m_FindResults[0] = AOT_NOFORM;
		m_pWordforms = NULL;
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
	}

	ISphTokenizer * Clone ( ESphTokenizerClone eMode ) const
	{
		// this token filter must NOT be created as escaped
		// it must only be used during indexing time, NEVER in searching time
		assert ( eMode==SPH_CLONE_INDEX );
		CSphAotTokenizer<IS_UTF8> * pClone = new CSphAotTokenizer ( m_pTokenizer->Clone ( eMode ), NULL, m_bIndexExact );
		if ( m_pWordforms )
			pClone->m_pWordforms = m_pWordforms;
		return pClone;
	}

	void SetBuffer ( BYTE * sBuffer, int iLength )
	{
		m_pTokenizer->SetBuffer ( sBuffer, iLength );
	}

	bool TokenIsBlended() const
	{
		return m_iCurrent>=0 || m_pTokenizer->TokenIsBlended();
	}

	BYTE * GetToken()
	{
		m_eTokenMorph = SPH_TOKEN_MORPH_GUESS;

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
				m_iCurrent = -1;
				m_eTokenMorph = SPH_TOKEN_MORPH_ORIGINAL;
				return m_sToken;
			}

			// generate that lemma
			const CFlexiaModel & M = g_pLemmatizer->m_FlexiaModels [ AOT_MODEL_NO ( m_FindResults [ m_iCurrent ] ) ];
			const CMorphForm & F = M [ AOT_ITEM_NO ( m_FindResults [ m_iCurrent ] ) ];
			CreateLemma<IS_UTF8> ( m_sToken, m_sForm, m_iFormLen, m_bFound, M, F );

			// is this the last one? gotta tag it non-blended
			if ( m_FindResults [ m_iCurrent+1 ]==AOT_NOFORM )
				m_iCurrent = -1;

			if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
				m_pWordforms->ToNormalForm ( m_sToken, false );

			return m_sToken;			
		}

		// ok, time to work on a next word
		assert ( m_iCurrent<0 );
		BYTE * pToken = m_pTokenizer->GetToken();
		if ( !pToken )
			return NULL;

		// pass-through blended parts
		if ( m_pTokenizer->TokenIsBlended() )
			return pToken;

		// pass-through matched wordforms
		if ( m_pWordforms && m_pWordforms->ToNormalForm ( pToken, true ) )
			return pToken;

		// pass-through 1-char "words"
		if ( pToken [ 1+IS_UTF8 ]=='\0' )
			return pToken;

		// pass-through non-Russian words
		if ( IS_UTF8 )
		{
			if ( !IsAlphaUtf8 ( pToken ) )
				return pToken;
		} else
		{
			if ( !IsAlpha1251 ( pToken[0] ) )
				return pToken;
		}

		// convert or copy regular tokens
		if ( IS_UTF8 )
			m_iFormLen = Utf8ToWin1251 ( m_sForm, pToken );
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
		if ( IsFreq2 ( m_sForm ) || IsFreq3 ( m_sForm ) )
			return pToken;

		// lemmatize
		m_bFound = g_pLemmatizer->LemmatizeWord ( m_sForm, m_FindResults );
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
		}

		// in any event, prepare the first lemma for return
		const CFlexiaModel & M = g_pLemmatizer->m_FlexiaModels [ AOT_MODEL_NO ( m_FindResults[0] ) ];
		const CMorphForm & F = M [ AOT_ITEM_NO ( m_FindResults[0] ) ];
		CreateLemma<IS_UTF8> ( pToken, m_sForm, m_iFormLen, m_bFound, M, F );

		// schedule lemmas 2+ for return
		if ( m_FindResults[1]!=AOT_NOFORM )
			m_iCurrent = 0;

		// suddenly, post-morphology wordforms
		if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
			m_pWordforms->ToNormalForm ( pToken, false );

		return pToken;
	}
};

#if USE_WINDOWS
#pragma warning(default:4127) // conditional expr is const for MSVC
#endif

CSphTokenFilter * sphAotCreateFilter ( ISphTokenizer * pTokenizer, CSphDict * pDict, bool bIndexExact )
{
	if ( pTokenizer->IsUtf8() )
		return new CSphAotTokenizer<true> ( pTokenizer, pDict, bIndexExact );
	else
		return new CSphAotTokenizer<false> ( pTokenizer, pDict, bIndexExact );
}

//
// $Id$
//
