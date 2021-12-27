#.rst:
# FindZSTD
# --------------
#
# Find ZSTD library and headers
#
# The module defines the following variables:
#
# ::
#
#   ZSTD_FOUND			- true if libzstd was found
#
# If library is found, imported target ZSTD::ZSTD is produced to link with.
#
#=============================================================================
# Copyright 2021, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_package ( PkgConfig )
if (PKG_CONFIG_FOUND)
	pkg_check_modules ( ZSTD libzstd )
	include (helpers)
	implib_pkgconfig (ZSTD ZSTD::ZSTD)
endif()
