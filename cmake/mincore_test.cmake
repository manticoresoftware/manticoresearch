#.rst:
# Mincore char test
# --------------
#
# Test whether function 'mincore' accept 3-rd param as char* or unsigned char*
#
# This piece of code is runaround and exists untill mincore is in use.
# It is much more better to refactor 'crashdump' func in the code and move
# all the stuff about dumping current query to the very end, after
# all another significant info is collected. This way it will be
# not so important whether we dump the query or not at all, and so,
# no mincore check will be necessary.
#
# For now the problem is that prototype is different on linux/bsd,
# and so, this special testing is necessary to successfully compile.
#
#=============================================================================
# Copyright 2017-2023, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

set ( _MINCORE_TEST "#include <unistd.h>
#include <sys/mman.h>

int main()
{
size_t length = 0;
void* pAddr = 0;
unsigned char *vec = 0;
return mincore ( pAddr, length, vec );
}" )

include ( CheckCXXSourceCompiles )
CHECK_CXX_SOURCE_COMPILES ( "${_MINCORE_TEST}" HAVE_UNSIGNED_MINCORE )
unset ( _MINCORE_TEST )
mark_as_advanced ( HAVE_UNSIGNED_MINCORE )
