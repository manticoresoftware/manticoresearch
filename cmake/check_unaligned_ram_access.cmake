#find if the unaligned RAM access is possible on the build system
if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
MESSAGE (STATUS "Compiler is ${CMAKE_CXX_COMPILER_ID}")
#if ( ${CMAKE_COMPILER_IS_GNUCXX} )
	set (_CHECK_UNALIGNED_PROG "
#include <stdlib.h>
#include <string.h>

int main()
{
char * sBuf = new char [ 8*sizeof(int) ];
for ( int i=0; i<8*sizeof(int); i++ )
	sBuf[i] = i;

// check for crashes (SPARC)
volatile int iRes = 0;
for ( int i=0; i<(int)sizeof(int); i++ )
{
	int * pPtr = (int*)( sBuf+i );
	iRes += *pPtr;
}

// check for correct values (ARM)
iRes = *(int*)( sBuf+1 );
if (!( iRes==0x01020304 || iRes==0x04030201 ))
	return 1;
// all seems ok
return 0;
}")

	include(CheckCXXSourceRuns)
	CHECK_CXX_SOURCE_RUNS("${_CHECK_UNALIGNED_PROG}" _UNALIGNED_RAM)
endif()

if (_UNALIGNED_RAM)
	set (UNALIGNED_RAM_ACCESS 1)
endif (_UNALIGNED_RAM)
