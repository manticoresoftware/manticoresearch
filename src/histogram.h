//
// Copyright (c) 2018-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _histogram_
#define _histogram_

#include "columnarmisc.h"
#include "sphinx.h"

class CSphReader;
class CSphWriter;

enum HistogramType_e
{
	HISTOGRAM_STREAMED_UINT32,
	HISTOGRAM_STREAMED_INT64,
	HISTOGRAM_STREAMED_FLOAT
};

struct HistogramRset_t
{
	int64_t m_iTotal { 0 };
	DWORD m_iCount { 0 };
	float m_fRangeSize { 0.0f };
};

class Histogram_i
{
public:
	virtual			~Histogram_i() {}

	virtual void	Insert ( SphAttr_t tAttrVal ) = 0;		// index time insert element when values and counters could be updated
	virtual void	UpdateCounter ( SphAttr_t tAttr ) = 0;	// run-time update counters only element values are same
	virtual void	Delete ( SphAttr_t tAttrVal ) = 0;
	virtual bool	EstimateRsetSize ( const CSphFilterSettings & tFilter, HistogramRset_t & tEstimate ) const = 0;
	virtual DWORD	GetNumValues() const = 0;
	virtual bool	IsOutdated() const = 0;
	virtual int		GetSize() const = 0;

	virtual HistogramType_e		GetType() const = 0;
	virtual const CSphString &	GetAttrName() const = 0;

	virtual void	Finalize() {}
	virtual bool	Save ( CSphWriter & tWriter ) const = 0;
	virtual bool	Load ( CSphReader & tReader, CSphString & sError ) = 0;

	virtual void	Dump ( StringBuilder_c & tOut ) const = 0;
};


class HistogramContainer_c
{
public:
					~HistogramContainer_c();

	bool			Save ( const CSphString & sFile, CSphString & sError );
	bool			Load ( const CSphString & sFile, CSphString & sError );
	bool			Add ( std::unique_ptr<Histogram_i> pHistogram );
	void			Remove ( const CSphString & sAttr );
	Histogram_i *	Get ( const CSphString & sAttr ) const;

	void			Insert ( int iHistogram, SphAttr_t tAttr ) { m_dHistograms[iHistogram]->Insert(tAttr); }

private:
	CSphVector<Histogram_i*>		m_dHistograms;
	SmallStringHash_T<Histogram_i*>	m_dHistogramHash;

	void			Reset();
};


std::unique_ptr<Histogram_i>	CreateHistogram ( const CSphString & sAttr, ESphAttr eAttrType, int iSize=0 );
void			CreateHistograms ( HistogramContainer_c & tHistograms, CSphVector<PlainOrColumnar_t> & dAttrsForHistogram, const ISphSchema & tSchema );

struct HistogramSource_t
{
	Histogram_i * m_pHist { nullptr };
	int m_iAttr { -1 };
	ESphAttr m_eAttrType { SPH_ATTR_NONE };
};

#endif // _histogram_
