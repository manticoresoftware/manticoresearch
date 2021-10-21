//
// Copyright (c) 2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "columnarrt.h"
#include "fileio.h"
#include "memio.h"
#include "attribute.h"
#include "schema/schema.h"

template <typename T>
static std::pair<T,T> GetLengthOffset ( const CSphVector<T> & dLengths, RowID_t tRowID )
{
	T iLength = dLengths[tRowID];
	T iOffset = 0;
	if ( tRowID>0 )
	{
		iOffset = dLengths[tRowID-1];
		iLength -= iOffset;
	}

	return {iLength, iOffset};
}

template <typename T>
static T GetLength ( const CSphVector<T> & dLengths, RowID_t tRowID )
{
	T iLength = dLengths[tRowID];
	if ( tRowID>0 )
		iLength -= dLengths[tRowID-1];

	return iLength;
}

/////////////////////////////////////////////////////////////////////

class ColumnarAttrRT_i
{
public:
	virtual			~ColumnarAttrRT_i() = default;

	virtual void	AddDoc ( SphAttr_t tAttr ) = 0;
	virtual void	AddDoc ( const BYTE * pData, int iLength ) = 0;
	virtual void	AddDoc ( const int64_t * pData, int iLength ) = 0;

	virtual void	Kill ( const CSphVector<RowID_t> & dKilled ) = 0;
	virtual void	Save ( MemoryWriter_c & tWriter ) = 0;
	virtual void	Save ( CSphWriter & tWriter ) = 0;
	virtual void	Load ( MemoryReader_c & tReader ) = 0;
	virtual void	Load ( CSphReader & tReader ) = 0;
	virtual int64_t	AllocatedBytes() const = 0;

	virtual columnar::Iterator_i * CreateIterator()	const = 0;
};


class ColumnarAttrRT_c : public ColumnarAttrRT_i
{
public:
	void	AddDoc ( SphAttr_t tAttr ) override						{ assert ( 0 && "Unsupported type" ); }
	void	AddDoc ( const BYTE * pData, int iLength ) override		{ assert ( 0 && "Unsupported type" ); }
	void	AddDoc ( const int64_t * pData, int iLength	) override	{ assert ( 0 && "Unsupported type" ); }
};


class ColumnarIterator_RT_c : public columnar::Iterator_i
{
public:
	uint32_t	AdvanceTo ( uint32_t tRowID ) override { m_tRowID = tRowID; return tRowID; }

	int64_t		Get() override						{ assert ( 0 && "Unsupported function" ); return 0; }
	void		Fetch ( const columnar::Span_T<uint32_t> & dRowIDs, columnar::Span_T<int64_t> & dValues ) override { assert ( 0 && "Unsupported function" ); }

	int			Get ( const uint8_t * & pData ) override { assert ( 0 && "Unsupported function" ); return 0; }
	uint8_t *	GetPacked() override				{ assert ( 0 && "Unsupported function" ); return 0; }
	int			GetLength() override				{ assert ( 0 && "Unsupported function" ); return 0; }

	uint64_t	GetStringHash() override			{ assert ( 0 && "Unsupported function" ); return 0; }
	bool		HaveStringHashes() const override	{ assert ( 0 && "Unsupported function" ); return false; }

protected:
	RowID_t		m_tRowID = INVALID_ROWID;
};

/////////////////////////////////////////////////////////////////////

template<typename T>
class ColumnarIterator_Int_T : public ColumnarIterator_RT_c
{
public:
			ColumnarIterator_Int_T ( const CSphVector<T> & dValues ) : m_dValues ( dValues ) {}

	int64_t	Get() override { return m_dValues[m_tRowID]; }
	void	Fetch ( const columnar::Span_T<uint32_t> & dRowIDs, columnar::Span_T<int64_t> & dValues ) override;

private:
	const CSphVector<T> & m_dValues;
};

template<typename T>
void ColumnarIterator_Int_T<T>::Fetch ( const columnar::Span_T<uint32_t> & dRowIDs, columnar::Span_T<int64_t> & dValues )
{
	uint32_t * pRowID = dRowIDs.begin();
	uint32_t * pRowIDEnd = dRowIDs.end();
	int64_t * pValue = dValues.begin();
	while ( pRowID<pRowIDEnd )
		*pValue++ = m_dValues[*pRowID++];
}

/////////////////////////////////////////////////////////////////////

template<typename T>
class ColumnarAttr_Int_T : public ColumnarAttrRT_c
{
public:
			ColumnarAttr_Int_T ( ESphAttr eType, int iBits );

	void	AddDoc ( SphAttr_t tAttr ) override			{ m_dValues.Add ( ( (T)tAttr ) & m_uMask ); }

	void	Kill ( const CSphVector<RowID_t> & dKilled ) override;
	void	Save ( MemoryWriter_c & tWriter ) override	{ SaveData(tWriter); }
	void	Save ( CSphWriter & tWriter ) override		{ SaveData(tWriter); }
	void	Load ( MemoryReader_c & tReader ) override	{ LoadData(tReader); }
	void	Load ( CSphReader & tReader ) override		{ LoadData(tReader); }
	int64_t	AllocatedBytes() const override				{ return m_dValues.GetLengthBytes64(); }

	columnar::Iterator_i * CreateIterator()	const override { return new ColumnarIterator_Int_T<T> ( m_dValues ); }

private:
	CSphVector<T>	m_dValues;
	ESphAttr		m_eType = SPH_ATTR_NONE;
	T				m_uMask = 0;

	template <typename WRITER>
	void	SaveData ( WRITER & tWriter );

	template <typename READER>
	void	LoadData ( READER & tReader );
};

template<typename T>
ColumnarAttr_Int_T<T>::ColumnarAttr_Int_T ( ESphAttr eType, int iBits )
	: m_eType ( eType )
	, m_uMask ( iBits==64 ? (T)0xFFFFFFFFFFFFFFFFULL : (T)( (1ULL<<iBits)-1 ) )
{}

template<typename T>
void ColumnarAttr_Int_T<T>::Kill ( const CSphVector<RowID_t> & dKilled )
{
	for ( int i = dKilled.GetLength()-1; i>=0; i-- )
		m_dValues.Remove(dKilled[i]);
}

template<typename T>
template <typename WRITER>
void ColumnarAttr_Int_T<T>::SaveData ( WRITER & tWriter )
{
	tWriter.PutDword(m_eType);
	tWriter.PutOffset(m_uMask);
	tWriter.PutDword ( m_dValues.GetLength() );
	tWriter.PutBytes ( m_dValues.Begin(), (int)m_dValues.GetLengthBytes64() );
}

template<typename T>
template <typename READER>
void ColumnarAttr_Int_T<T>::LoadData ( READER & tReader )
{
	m_uMask = (T)tReader.GetOffset();
	m_dValues.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dValues.Begin(), (int)m_dValues.GetLengthBytes64() );
}

/////////////////////////////////////////////////////////////////////

class ColumnarIterator_String_c : public ColumnarIterator_RT_c
{
public:
				ColumnarIterator_String_c ( const CSphVector<BYTE> & dData, const CSphVector<int64_t> & dLengths ) : m_dData ( dData ), m_dLengths ( dLengths )  {}

	int			Get ( const uint8_t * & pData ) override;
	uint8_t *	GetPacked() override;
	int			GetLength() override;

	uint64_t	GetStringHash() override			{ return 0; }
	bool		HaveStringHashes() const override	{ return false; }

private:
	const CSphVector<BYTE> &	m_dData;
	const CSphVector<int64_t> &	m_dLengths;
};


int ColumnarIterator_String_c::Get ( const uint8_t * & pData )
{
	int64_t iLength, iOffset;
	std::tie(iLength, iOffset) = GetLengthOffset ( m_dLengths, m_tRowID );
	pData = iLength>0 ? (const uint8_t*)&m_dData[iOffset] : nullptr;
	return (int)iLength;
}


uint8_t * ColumnarIterator_String_c::GetPacked()
{
	int64_t iLength, iOffset;
	std::tie(iLength, iOffset) = GetLengthOffset ( m_dLengths, m_tRowID );
	auto pStr = iLength>0 ? (const uint8_t*)&m_dData[iOffset] : nullptr;
	return sphPackPtrAttr ( { pStr, iLength } );
}


int ColumnarIterator_String_c::GetLength()
{
	return (int)::GetLength ( m_dLengths, m_tRowID );
}


class ColumnarAttr_String_c : public ColumnarAttrRT_c
{
public:
			ColumnarAttr_String_c ( ESphAttr eType ) : m_eType ( eType ) {}

	void	AddDoc ( const BYTE * pData, int iLength ) override;

	void	Kill ( const CSphVector<RowID_t> & dKilled ) override;
	void	Save ( MemoryWriter_c & tWriter ) override	{ SaveData(tWriter); }
	void	Save ( CSphWriter & tWriter ) override		{ SaveData(tWriter); }
	void	Load ( MemoryReader_c & tReader ) override	{ LoadData(tReader); }
	void	Load ( CSphReader & tReader ) override		{ LoadData(tReader); }
	int64_t	AllocatedBytes() const override				{ return m_dData.GetLengthBytes64() + m_dLengths.GetLengthBytes64(); }

	columnar::Iterator_i * CreateIterator()	const override { return new ColumnarIterator_String_c ( m_dData, m_dLengths ); }

private:
	CSphVector<BYTE>	m_dData;
	CSphVector<int64_t>	m_dLengths;
	int64_t				m_iTotalLength = 0;
	ESphAttr			m_eType = SPH_ATTR_NONE;

	template <typename WRITER>
	void	SaveData ( WRITER & tWriter );

	template <typename READER>
	void	LoadData ( READER & tReader );
};


void ColumnarAttr_String_c::AddDoc ( const BYTE * pData, int iLength )
{
	m_iTotalLength += iLength;
	m_dLengths.Add(m_iTotalLength);
	m_dData.Append ( pData, iLength );
}


void ColumnarAttr_String_c::Kill ( const CSphVector<RowID_t> & dKilled )
{
	for ( int i = dKilled.GetLength()-1; i>=0; i-- )
	{
		RowID_t tRowID = dKilled[i];

		int64_t iLength, iOffset;
		std::tie(iLength, iOffset) = GetLengthOffset ( m_dLengths, tRowID );
		m_dData.Remove ( (int)iOffset, (int)iLength );

		for ( RowID_t tNextRowID = tRowID+1; tNextRowID < (RowID_t)m_dLengths.GetLength(); tNextRowID++ )
			m_dLengths[tNextRowID] -= iLength;
	}
}

template <typename WRITER>
void ColumnarAttr_String_c::SaveData ( WRITER & tWriter )
{
	tWriter.PutDword(m_eType);
	tWriter.PutDword ( m_dLengths.GetLength() );
	tWriter.PutBytes ( m_dLengths.Begin(), (int)m_dLengths.GetLengthBytes64() );
	tWriter.PutDword ( m_dData.GetLength() );
	tWriter.PutBytes ( m_dData.Begin(), (int)m_dData.GetLengthBytes64() );
}

template <typename READER>
void ColumnarAttr_String_c::LoadData ( READER & tReader )
{
	m_dLengths.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dLengths.Begin(), (int)m_dLengths.GetLengthBytes64() );
	m_dData.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dData.Begin(), (int)m_dData.GetLengthBytes64() );
}

/////////////////////////////////////////////////////////////////////

template <typename T>
class ColumnarIterator_MVA_T : public ColumnarIterator_RT_c
{
public:
				ColumnarIterator_MVA_T ( const CSphVector<T> & dData, const CSphVector<int> & dLengths ) : m_dData ( dData ), m_dLengths ( dLengths )  {}

	int			Get ( const uint8_t * & pData ) override;
	uint8_t *	GetPacked() override;
	int			GetLength() override;

private:
	const CSphVector<T> &	m_dData;
	const CSphVector<int> &	m_dLengths;
};

template <typename T>
int ColumnarIterator_MVA_T<T>::Get ( const uint8_t * & pData )
{
	int iLength, iOffset;
	std::tie(iLength, iOffset) = GetLengthOffset ( m_dLengths, m_tRowID );

	iLength *= sizeof(T);
	pData = iLength > 0 ? (const uint8_t*)&m_dData[iOffset] : nullptr;

	return iLength;
}

template <typename T>
uint8_t * ColumnarIterator_MVA_T<T>::GetPacked()
{
	int iLength, iOffset;
	std::tie(iLength, iOffset) = GetLengthOffset ( m_dLengths, m_tRowID );

	iLength *= sizeof(T);
	auto pMVA = iLength > 0 ? (const uint8_t*)&m_dData[iOffset] : nullptr;
	return sphPackPtrAttr ( { pMVA, iLength } );
}


template <typename T>
int ColumnarIterator_MVA_T<T>::GetLength()
{
	return ::GetLength ( m_dLengths, m_tRowID )*sizeof(T);
}


template <typename T>
class ColumnarAttr_MVA_T : public ColumnarAttrRT_c
{
public:
			ColumnarAttr_MVA_T ( ESphAttr eType ) : m_eType ( eType ) {}

	void	AddDoc ( const int64_t * pData, int iLength ) override;

	void	Kill ( const CSphVector<RowID_t> & dKilled ) override;
	void	Save ( MemoryWriter_c & tWriter ) override	{ SaveData(tWriter); }
	void	Save ( CSphWriter & tWriter ) override		{ SaveData(tWriter); }
	int64_t	AllocatedBytes() const override				{ return m_dData.GetLengthBytes64() + m_dLengths.GetLengthBytes64(); }
	void	Load ( MemoryReader_c & tReader ) override	{ LoadData(tReader); }
	void	Load ( CSphReader & tReader ) override		{ LoadData(tReader); }

	columnar::Iterator_i * CreateIterator()	const override { return new ColumnarIterator_MVA_T<T> ( m_dData, m_dLengths ); }

private:
	CSphVector<T>		m_dData;
	CSphVector<int>		m_dLengths;
	int64_t				m_iTotalLength = 0;
	ESphAttr			m_eType = SPH_ATTR_NONE;

	template <typename WRITER>
	void	SaveData ( WRITER & tWriter );

	template <typename READER>
	void	LoadData ( READER & tReader );
};

template <typename T>
void ColumnarAttr_MVA_T<T>::AddDoc ( const int64_t * pData, int iLength )
{
	m_iTotalLength += iLength;
	m_dLengths.Add ( (int)m_iTotalLength );
	for ( int i = 0; i < iLength; i++ )
		m_dData.Add ( (T)pData[i] );
}

template <typename T>
void ColumnarAttr_MVA_T<T>::Kill ( const CSphVector<RowID_t> & dKilled )
{
	for ( int i = dKilled.GetLength()-1; i>=0; i-- )
	{
		RowID_t tRowID = dKilled[i];

		int iLength, iOffset;
		std::tie(iLength, iOffset) = GetLengthOffset ( m_dLengths, tRowID );
		m_dData.Remove ( iOffset, iLength );

		for ( RowID_t tNextRowID = tRowID+1; tNextRowID < (RowID_t)m_dLengths.GetLength(); tNextRowID++ )
			m_dLengths[tNextRowID] -= iLength;
	}
}

template <typename T>
template <typename WRITER>
void ColumnarAttr_MVA_T<T>::SaveData ( WRITER & tWriter )
{
	tWriter.PutDword(m_eType);
	tWriter.PutDword ( m_dLengths.GetLength() );
	tWriter.PutBytes ( m_dLengths.Begin(), (int)m_dLengths.GetLengthBytes64() );
	tWriter.PutDword ( m_dData.GetLength() );
	tWriter.PutBytes ( m_dData.Begin(), (int)m_dData.GetLengthBytes64() );
}

template <typename T>
template <typename READER>
void ColumnarAttr_MVA_T<T>::LoadData ( READER & tReader )
{
	m_dLengths.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dLengths.Begin(), (int)m_dLengths.GetLengthBytes64() );
	m_dData.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dData.Begin(), (int)m_dData.GetLengthBytes64() );
}

////////////////////////////////////////////////////////////////////

static ColumnarAttrRT_i * CreateColumnarAttrRT ( ESphAttr eType, int iBits )
{
	switch ( eType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_FLOAT:
		return new ColumnarAttr_Int_T<DWORD> ( eType, iBits );

	case SPH_ATTR_BOOL:		return new ColumnarAttr_Int_T<BYTE> ( eType, 1 );
	case SPH_ATTR_BIGINT:	return new ColumnarAttr_Int_T<int64_t> ( eType, iBits );
	case SPH_ATTR_STRING:	return new ColumnarAttr_String_c(eType);
	case SPH_ATTR_UINT32SET:return new ColumnarAttr_MVA_T<DWORD>(eType);
	case SPH_ATTR_INT64SET:	return new ColumnarAttr_MVA_T<int64_t>(eType);

	default:
		assert ( 0 && "Unsupported type" );
		return nullptr;
	}
}

/////////////////////////////////////////////////////////////////////

class ColumnarBuilderRT_c : public ColumnarBuilderRT_i
{
public:
			ColumnarBuilderRT_c() = default;
			ColumnarBuilderRT_c ( const CSphSchema & tSchema );
			~ColumnarBuilderRT_c() override { m_dAttrs.for_each ( []( auto i ){ delete i; } ); }

	void	SetAttr ( int iAttr, int64_t tAttr ) override						{ m_dAttrs[iAttr]->AddDoc(tAttr); }
	void	SetAttr ( int iAttr, const uint8_t * pData, int iLength ) override	{ m_dAttrs[iAttr]->AddDoc ( pData, iLength ); }
	void	SetAttr ( int iAttr, const int64_t * pData, int iLength ) override	{ m_dAttrs[iAttr]->AddDoc ( pData, iLength ); }
	bool	Done ( std::string & sError ) override { return true; }

	void	Kill ( const CSphVector<RowID_t> & dKilled ) override;
	void	Save ( MemoryWriter_c & tWriter ) override;
	CSphVector<ColumnarAttrRT_i*> & GetAttrs() override { return m_dAttrs; }

	void	Load ( MemoryReader_c & tReader );

private:
	CSphVector<ColumnarAttrRT_i *> m_dAttrs;
};


ColumnarBuilderRT_c::ColumnarBuilderRT_c ( const CSphSchema & tSchema )
{
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const auto & tAttr = tSchema.GetAttr(i);
		if ( tAttr.IsColumnar() )
		{
			m_dAttrs.Add ( CreateColumnarAttrRT ( tAttr.m_eAttrType, tAttr.m_tLocator.m_iBitCount ) );
			assert ( m_dAttrs.Last() );
		}
	}
}


void ColumnarBuilderRT_c::Kill ( const CSphVector<RowID_t> & dKilled )
{
	for ( auto & i : m_dAttrs )
		i->Kill(dKilled);
}


void ColumnarBuilderRT_c::Save ( MemoryWriter_c & tWriter )
{
	tWriter.PutDword ( m_dAttrs.GetLength() );
	m_dAttrs.for_each ( [&tWriter]( auto i ){ i->Save(tWriter); } );
}


void ColumnarBuilderRT_c::Load ( MemoryReader_c & tReader )
{
	m_dAttrs.Resize ( tReader.GetDword() );
	m_dAttrs.for_each ( [&tReader]( auto & pAttr ){ pAttr = CreateColumnarAttrRT ( (ESphAttr)tReader.GetDword(), 0 ); pAttr->Load(tReader); } );
}

/////////////////////////////////////////////////////////////////////

class ColumnarRT_c : public ColumnarRT_i
{
public:
					ColumnarRT_c() = default;
					ColumnarRT_c ( const CSphSchema & tSchema, ColumnarBuilderRT_i * pBuilder, bool bTakeOwnership );

	columnar::Iterator_i *						CreateIterator ( const std::string & sName, const columnar::IteratorHints_t & tHints, std::string & sError ) const override;
	std::vector<columnar::BlockIterator_i *>	CreateAnalyzerOrPrefilter ( const std::vector<columnar::Filter_t> & dFilters, std::vector<int> & dDeletedFilters, const columnar::BlockTester_i & tBlockTester ) const override { return {}; }

	int				GetAttributeId ( const std::string & sName ) const override;
	bool			EarlyReject ( const std::vector<columnar::Filter_t> & dFilters, const columnar::BlockTester_i & tBlockTester ) const override { return false; }
	bool			IsFilterDegenerate ( const columnar::Filter_t & tFilter ) const override { return false; }

	void			Save ( CSphWriter & tWriter ) override;
	int64_t			AllocatedBytes() const override;
	
	bool			Load ( CSphReader & tReader, const CSphSchema & tSchema, CSphString & sError );

private:
	CSphVector<ColumnarAttrRT_i*>			m_dAttrs;
	SmallStringHash_T<std::pair<ColumnarAttrRT_i*,int>> m_hAttrs;

	void			PopulateHashFromSchema ( const CSphSchema & tSchema );
};


ColumnarRT_c::ColumnarRT_c ( const CSphSchema & tSchema, ColumnarBuilderRT_i * pBuilder, bool bTakeOwnership )
{
	if ( bTakeOwnership )
		m_dAttrs = std::move ( pBuilder->GetAttrs() );
	else
		m_dAttrs = pBuilder->GetAttrs();

	PopulateHashFromSchema(tSchema);
}


columnar::Iterator_i * ColumnarRT_c::CreateIterator ( const std::string & sName, const columnar::IteratorHints_t & tHints, std::string & sError ) const
{
	auto * pFound = m_hAttrs ( sName.c_str() );
	if ( !pFound )
		return nullptr;

	return pFound->first->CreateIterator();
}


int ColumnarRT_c::GetAttributeId ( const std::string & sName ) const
{
	auto * pFound = m_hAttrs ( sName.c_str() );
	return pFound ? pFound->second : -1;
}


void ColumnarRT_c::Save ( CSphWriter & tWriter )
{
	tWriter.PutDword ( m_dAttrs.GetLength() );
	m_dAttrs.for_each ( [&tWriter]( auto pAttr ){ pAttr->Save(tWriter); } );
}


int64_t ColumnarRT_c::AllocatedBytes() const
{
	int64_t iTotal = 0;
	for ( const auto & i : m_dAttrs )
		iTotal += i->AllocatedBytes();

	return iTotal;
}


void ColumnarRT_c::PopulateHashFromSchema ( const CSphSchema & tSchema )
{
	int iColumnar = 0;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const auto & tAttr = tSchema.GetAttr(i);
		if ( !tAttr.IsColumnar() )
			continue;

		m_hAttrs.Add ( { m_dAttrs[iColumnar], iColumnar }, tAttr.m_sName );
		iColumnar++;
	}

	assert ( m_hAttrs.GetLength() == m_dAttrs.GetLength() );
}


bool ColumnarRT_c::Load ( CSphReader & tReader, const CSphSchema & tSchema, CSphString & sError )
{
	m_dAttrs.Resize ( tReader.GetDword() );
	m_dAttrs.for_each ( [&tReader]( auto & pAttr ){ pAttr = CreateColumnarAttrRT ( (ESphAttr)tReader.GetDword(), 0 ); pAttr->Load(tReader); } );
	PopulateHashFromSchema(tSchema);

	bool bOk = !tReader.GetErrorFlag();
	if ( !bOk )
		sError.SetSprintf ( "error loading columnar attribute storage: %s", tReader.GetErrorMessage().cstr() );

	return !tReader.GetErrorFlag();
}

/////////////////////////////////////////////////////////////////////

ColumnarBuilderRT_i * CreateColumnarBuilderRT ( MemoryReader_c & tReader )
{
	CSphScopedPtr<ColumnarBuilderRT_c> pBuilder ( new ColumnarBuilderRT_c );
	pBuilder->Load(tReader);
	return pBuilder.LeakPtr();
}


ColumnarBuilderRT_i * CreateColumnarBuilderRT ( const CSphSchema & tSchema )
{
	if ( !tSchema.HasColumnarAttrs() )
		return nullptr;

	return new ColumnarBuilderRT_c(tSchema);
}


ColumnarRT_i * CreateColumnarRT ( const CSphSchema & tSchema, ColumnarBuilderRT_i * pBuilder, bool bTakeOwnership )
{
	if ( !pBuilder )
		return nullptr;

	return new ColumnarRT_c ( tSchema, pBuilder, bTakeOwnership );
}


ColumnarRT_i * CreateColumnarRT ( const CSphSchema & tSchema, CSphReader & tReader, CSphString & sError )
{
	CSphScopedPtr<ColumnarRT_c> pColumnar ( new ColumnarRT_c );
	if ( !pColumnar->Load ( tReader, tSchema, sError ) )
		return nullptr;

	return pColumnar.LeakPtr();
}
