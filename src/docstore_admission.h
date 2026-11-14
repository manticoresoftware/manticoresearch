// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.
#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

// Admit on the second visit to a fingerprint. Collisions only affect admission;
// the shared cache still compares the complete block key.
class DocstoreAdmission_c
{
public:
	DocstoreAdmission_c() : m_dHistory ( new std::atomic<uint32_t>[65536] {} ) {}
	bool Admit ( uint32_t uHash )
	{
		const uint32_t uTag = uHash ? uHash : 1;
		return m_dHistory[uHash & 65535].exchange ( uTag, std::memory_order_relaxed )==uTag;
	}

private:
	std::unique_ptr<std::atomic<uint32_t>[]> m_dHistory;
};
