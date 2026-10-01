// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstdint>

class FieldNormReader_i
{
public:
	virtual ~FieldNormReader_i() = default;
	virtual uint32_t Rows() const = 0;
	virtual uint32_t Fields() const = 0;
	virtual uint64_t Sum ( uint32_t uField ) const = 0;
	virtual bool Get ( uint32_t uField, uint32_t uRow, uint32_t & uValue ) const = 0;

	virtual bool ReadRange ( uint32_t uField, uint32_t uFirst, uint32_t uCount, uint32_t * pOut ) const
	{
		if ( !pOut || uFirst>Rows() || uCount>Rows()-uFirst )
			return false;
		for ( uint32_t i=0; i<uCount; ++i )
			if ( !Get(uField,uFirst+i,pOut[i]) )
				return false;
		return true;
	}

	virtual bool Gather ( uint32_t uField, const uint32_t * pRows, uint32_t uCount, uint32_t * pOut ) const
	{
		if ( !pRows || !pOut )
			return false;
		if ( uCount && pRows[uCount-1]-pRows[0]==uCount-1 )
			return ReadRange ( uField, pRows[0], uCount, pOut );
		for ( uint32_t i=0; i<uCount; ++i )
			if ( !Get(uField,pRows[i],pOut[i]) )
				return false;
		return true;
	}

	virtual bool Gather128 ( uint32_t uField, const uint32_t * pRows, uint32_t * pOut ) const
	{
		return Gather ( uField, pRows, 128, pOut );
	}
	virtual bool GatherTotal ( const uint32_t * pRows, uint32_t uCount, uint32_t * pOut ) const
	{
		if ( !pRows || !pOut )
			return false;
		for ( uint32_t i=0; i<uCount; ++i )
		{
			uint32_t uTotal = 0;
			for ( uint32_t iField=0; iField<Fields(); ++iField )
			{
				uint32_t uValue = 0;
				if ( !Get(iField,pRows[i],uValue) )
					return false;
				uTotal += uValue;
			}
			pOut[i] = uTotal;
		}
		return true;
	}
};
