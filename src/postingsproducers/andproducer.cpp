//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "andproducer.h"
#include "block/andproducer.h"
#include "qword/andproducer.h"

#include <algorithm>


AndReadMode_e PlanAndProducer ( const AndProducerSetup_t & tSetup )
{
	auto HasWideFields = [] ( const AndProducerTerm_t & tTerm ) { return tTerm.m_bWideFields; };
	if ( std::any_of ( tSetup.m_dTerms.begin(), tSetup.m_dTerms.end(), HasWideFields ) || std::any_of ( tSetup.m_dExcluded.begin(), tSetup.m_dExcluded.end(), HasWideFields ) )
		return tSetup.m_pFilter || !tSetup.m_dExcluded.empty() ? AndReadMode_e::UNSUPPORTED : AndReadMode_e::QWORD;

	return AndReadMode_e::BLOCK;
}


std::unique_ptr<PostingProducer_i> CreateAndProducer ( const AndProducerSetup_t & tSetup )
{
	assert ( tSetup.m_iCapacity>0 );
	if ( tSetup.m_dTerms.empty() || std::any_of ( tSetup.m_dTerms.begin(), tSetup.m_dTerms.end(), [] ( const AndProducerTerm_t & tTerm ) { return !tTerm.m_pQword->m_iDocs; } ) )
		return nullptr;

	AndReadMode_e eMode = PlanAndProducer(tSetup);
	if ( eMode==AndReadMode_e::UNSUPPORTED )
		return nullptr;

	std::unique_ptr<PostingProducer_i> pProducer;
	if ( eMode==AndReadMode_e::BLOCK )
		pProducer = CreateBlockAndProducer(tSetup);

	if ( !pProducer )
	{
		if ( tSetup.m_pFilter || !tSetup.m_dExcluded.empty() )
			return nullptr;

		pProducer = CreateQwordAndProducer ( tSetup, eMode==AndReadMode_e::QWORD );
	}

	pProducer->SetPayloadRequirements(tSetup.m_tPayloads);
	return pProducer;
}
