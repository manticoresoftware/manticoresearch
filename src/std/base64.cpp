//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//


#include "base64.h"

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>


bool DecodeBase64 ( const CSphString & sValue, CSphString & sResult )
{
	std::string sVal = sValue.cstr();

	using namespace boost::archive::iterators;
	try
	{
		using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
		sResult = boost::algorithm::trim_right_copy_if(std::string(It(std::begin(sVal)), It(std::end(sVal))), [](char c) { return c == '\0'; }).c_str();
	}
	catch (...)
	{
		return false;
	}

	return true;
}


CSphString EncodeBase64 ( const CSphString & sValue )
{
	std::string sVal = sValue.cstr();

	using namespace boost::archive::iterators;
	using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
	auto sTmp = std::string(It(std::begin(sVal)), It(std::end(sVal)));
	return sTmp.append((3 - sVal.size() % 3) % 3, '=').c_str();
}
