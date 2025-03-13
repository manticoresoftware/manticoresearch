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
# Copyright 2021-2025, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================


find_path ( ZSTD_INCLUDE_DIRS NAMES "zstd.h" )
find_library ( ZSTD_LINK_LIBRARIES NAMES zstd libzstd NAMES_PER_DIR )

mark_as_advanced ( ZSTD_LINK_LIBRARIES ZSTD_INCLUDE_DIRS )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( ZSTD REQUIRED_VARS ZSTD_LINK_LIBRARIES )
set_package_properties ( ZSTD PROPERTIES TYPE RUNTIME URL "https://github.com/facebook/zstd" )

include ( helpers )
implib_config ( ZSTD ZSTD::ZSTD )
