//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _docstore_
#define _docstore_

#include "sphinxstd.h"
#include "sphinxdefs.h"
#include "sphinx.h"

class DocstoreAddField_i
{
public:
	virtual		~DocstoreAddField_i() {}

	virtual int	AddField ( const CSphString & sName, DocstoreDataType_e eType ) = 0;
	virtual void RemoveField ( const CSphString & sName, DocstoreDataType_e eType ) = 0;
};

class DocstoreGetField_i
{
public:
	virtual		~DocstoreGetField_i() {}

	virtual int	GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const = 0;
};

class DocstoreFields_i : public DocstoreAddField_i, public DocstoreGetField_i
{
public:
	virtual int GetNumFields() const = 0;
};


// an interface for plain index (disk chunk) document storage
class Docstore_i : public DocstoreGetField_i
{
public:
	virtual void				CreateReader ( int64_t iSessionId ) const = 0;
	virtual DocstoreDoc_t		GetDoc ( RowID_t tRowID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const = 0;
	virtual DocstoreSettings_t	GetDocstoreSettings() const = 0;
};

class DocstoreBuilder_i : public DocstoreAddField_i, public DocstoreGetField_i
{
public:
	struct Doc_t
	{
		CSphVector<VecTraits_T<BYTE>> m_dFields;

		Doc_t();
		Doc_t ( const DocstoreDoc_t & tDoc );
	};


	virtual void	AddDoc ( RowID_t tRowID, const Doc_t & tDoc ) = 0;
	virtual void	Finalize() = 0;
};

class CSphReader;
class CSphWriter;
class MemoryReader_c;
class MemoryWriter_c;

// an interface for RT index document storage
class DocstoreRT_i : public Docstore_i, public DocstoreBuilder_i
{
public:
	virtual bool	Load ( CSphReader & tReader ) = 0;
	virtual void	Save ( Writer_i & tWriter ) = 0;
	virtual void	Load ( MemoryReader_c & tReader ) = 0;
	virtual void	Save ( MemoryWriter_c & tWriter ) = 0;

	virtual void	AddPackedDoc ( RowID_t tRowID, const DocstoreRT_i * pSrcDocstore, RowID_t tSrcRowID ) = 0;

	virtual int64_t	AllocatedBytes() const = 0;

	virtual bool	CheckFieldsLoaded ( CSphString & sError ) const = 0;
	virtual void	SwapRows ( RowID_t tDstID, RowID_t tSrcID ) = 0;
	virtual void	DropTail ( RowID_t tTailID ) = 0;
};


class DocstoreSession_c
{
public:
	struct InfoDocID_t
	{
		const DocstoreReader_i *	m_pDocstore = nullptr;
		int64_t						m_iSessionId = 0;
	};

	struct InfoRowID_t
	{
		const Docstore_i *			m_pDocstore = nullptr;
		int64_t						m_iSessionId = 0;
	};



				DocstoreSession_c();
				~DocstoreSession_c();

	int64_t		GetUID() const { return m_iUID; }

private:
	static std::atomic<int64_t> m_tUIDGenerator;

	int64_t		m_iUID = 0;
};


std::unique_ptr<Docstore_i>			CreateDocstore ( int64_t iIndexId, const CSphString & sFilename, CSphString & sError );
std::unique_ptr<DocstoreBuilder_i>	CreateDocstoreBuilder ( const CSphString & sFilename, const DocstoreSettings_t & tSettings, int iBufferSize, CSphString & sError );
std::unique_ptr<DocstoreRT_i>		CreateDocstoreRT();
std::unique_ptr<DocstoreFields_i>	CreateDocstoreFields();

void				InitDocstore ( int64_t iCacheSize );
void				ShutdownDocstore();

class DebugCheckError_i;
class CSphAutoreader;
bool				CheckDocstore ( CSphAutoreader & tReader, DebugCheckError_i & tReporter, int64_t iRowsCount );

#endif

