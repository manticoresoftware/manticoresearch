# find if the unaligned RAM access is possible on the build system
include(CheckCXXSourceRuns)
CHECK_CXX_SOURCE_RUNS ( "
#include <stdlib.h>
#include <string.h>

int main()
{
int ichars[8];
char * sBuf = (char*)ichars;
for ( int i=0; i<8*sizeof(int); i++ )
	sBuf[i] = i;

// check for crashes (SPARC)
volatile int iRes = 0;
for ( int i=0; i<(int)sizeof(int); ++i )
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
}
" UNALIGNED_RAM_ACCESS)
