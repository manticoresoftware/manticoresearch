//
// Copyright (c) 2023-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <gtest/gtest.h>
#include "replication/wsrep_cxx.h"

constexpr Wsrep::UUID_t dZeroUUID { 0 };

TEST ( Wsrep, default_uuid )
{
	Wsrep::UUID_t x {};
	EXPECT_EQ ( x, dZeroUUID );
}

TEST ( Wsrep, Uuid2Str_Str2Uuid )
{
	using namespace Wsrep;
	UUID_t y { { 1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 } };
	UUID_t Empty {};

	auto sUuid = Uuid2Str ( y );
	EXPECT_STREQ ( sUuid.cstr(), "01234567-89ab-cdef-fedc-ba9876543210");
	auto sEmpty = Uuid2Str ( Empty );
	EXPECT_STREQ ( sEmpty.cstr(), "00000000-0000-0000-0000-000000000000" );

	auto x1 = Str2Uuid ( "01234567-89ab-cdef-fedc-ba9876543210" );
	auto x2 = Str2Uuid ( "01234567-89Ab-CdEf-FeDc-Ba9876543210" );

	EXPECT_TRUE ( y == x1 );
	EXPECT_TRUE ( y == x2 );

	auto wrong1 = Str2Uuid ( "" );
	auto wrong2 = Str2Uuid ( "BlaBla" );

	EXPECT_TRUE ( Empty == wrong1 );
	EXPECT_TRUE ( Empty == wrong2 );
}

TEST ( Wsrep, default_globaltid )
{
	Wsrep::GlobalTid_t x;

	EXPECT_EQ ( x.m_tUuid, dZeroUUID );
	EXPECT_EQ ( x.m_iSeqNo, Wsrep::WRONG_SEQNO );
}

TEST ( Wsrep, Gtid2Str_Str2Gtid )
{
	using namespace Wsrep;
	GlobalTid_t y { { {1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10} }, 0xEE };
	GlobalTid_t Empty;

	auto sUuid = Gtid2Str ( y );
	EXPECT_STREQ ( sUuid.cstr(), "01234567-89ab-cdef-fedc-ba9876543210:238" );
	auto sEmpty = Gtid2Str ( Empty );
	EXPECT_STREQ ( sEmpty.cstr(), "00000000-0000-0000-0000-000000000000:-1" );

	EXPECT_TRUE ( y == Str2Gtid ( "01234567-89ab-cdef-fedc-ba9876543210:238" ) );
	EXPECT_TRUE ( y == Str2Gtid ( "01234567-89Ab-CdEf-FeDc-Ba9876543210:238" ) );

	EXPECT_TRUE ( Empty == Str2Gtid ( "" ) );
	EXPECT_TRUE ( Empty == Str2Gtid ( "BlaBla" ) );
	EXPECT_TRUE ( Empty == Str2Gtid ( "01234567-89Ab-CdEf-FeDc-Ba9876543210:aa" ) );
}