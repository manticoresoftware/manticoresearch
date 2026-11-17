//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "termproducer.h"
#include "block/termproducer.h"
#include "qword/termproducer.h"


TermReadMode_e PlanTermProducer ( const TermProducerSetup_t & tSetup )
{
	if ( tSetup.m_pFilter )
		return tSetup.m_bWideFields ? TermReadMode_e::UNSUPPORTED : TermReadMode_e::BLOCK;

	return tSetup.m_bWideFields ? TermReadMode_e::QWORD_WIDE_FIELDS : TermReadMode_e::QWORD_BATCHED;
}


std::unique_ptr<PostingProducer_i> CreateTermProducer ( ISphQword & tWord, const TermProducerSetup_t & tSetup )
{
	assert ( tSetup.m_iCapacity>0 );
	switch ( PlanTermProducer(tSetup) )
	{
	case TermReadMode_e::QWORD_BATCHED:
	case TermReadMode_e::QWORD_WIDE_FIELDS:
		return CreateQwordTermProducer ( tWord, tSetup );

	case TermReadMode_e::BLOCK:
		return CreateBlockTermProducer ( tWord, tSetup );

	case TermReadMode_e::UNSUPPORTED:
		return nullptr;
	}

	assert(0);
	return nullptr;
}
