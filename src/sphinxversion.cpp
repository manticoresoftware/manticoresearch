//
// Copyright (c) 2019, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifdef BUILD_WITH_CMAKE
	#include "gen_sphinxversion.h"
#else
	#include "sphinxversion.h"
#endif

#ifdef BUILD_WITH_CMAKE
	#include "gen_sphinxversion.h"
#else
	#include "sphinxversion.h"
#endif

#ifndef SPHINX_TAG
#define BANNER_TAG "dev"
#else
#define BANNER_TAG SPHINX_TAG
#endif

#ifndef GIT_TIMESTAMP_ID
#define GIT_TIMESTAMP_ID "000101"
#endif

// this line is deprecated and no more used. Leaved here for a while.
// numbers now to be defined via sphinxversion.h
#ifndef VERNUMBERS
	#define VERNUMBERS    "7.7.7"
#endif

#define PRODUCT_VERSION          VERNUMBERS " " SPH_GIT_COMMIT_ID "@" GIT_TIMESTAMP_ID " " BANNER_TAG
#define PRODUCT_NAME			"Manticore " PRODUCT_VERSION
#define PRODUCT_BANNER            PRODUCT_NAME "\nCopyright (c) 2001-2016, Andrew Aksyonoff\n" \
	"Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)\n" \
	"Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)\n\n"


const char * szMANTICORE_VERSION = PRODUCT_VERSION;
const char * szMANTICORE_NAME = PRODUCT_NAME;
const char * szMANTICORE_BANNER = PRODUCT_BANNER;
const char * szGIT_COMMIT_ID = SPH_GIT_COMMIT_ID;
const char * szGIT_BRANCH_ID = GIT_BRANCH_ID;
const char * szGDB_SOURCE_DIR = GDB_SOURCE_DIR;
