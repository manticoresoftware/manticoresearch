//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/ints.h"

// implement exponential sliding stat

class ExpMeter_c
{
	double m_fSum = 0.0;
	double m_fVal = 0.0;
	BYTE m_uSamples = 0;
	const BYTE m_uTimeConstant;

public:
	explicit ExpMeter_c ( BYTE uTimeConstant )
		: m_uTimeConstant { uTimeConstant }
	{}

	[[nodiscard]] double Value() const noexcept { return m_fVal; }

	void Reset()
	{
		m_fSum = 0.0;
		m_fVal = 0;
		m_uSamples = 0;
	}

	template<typename NUM>
	void Tick ( NUM tSample ) noexcept
	{
		m_fSum += tSample;
		if ( m_uSamples < m_uTimeConstant )
			++m_uSamples;
		else
			m_fSum -= m_fVal;

		m_fVal = m_fSum / m_uSamples;
	}

};