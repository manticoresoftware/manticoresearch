#find if malloc_trim() is available
set ( _CHECK_MALLOC_TRIM_PROG "
#include <malloc.h>
#ifdef __CLASSIC_C__
int main() {
	int ac;
	char*av[];
#else
int main(int ac, char*av[]){
#endif
	malloc_trim(0);
	return 0;
}" )

include ( CheckCXXSourceCompiles )
CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_MALLOC_TRIM_PROG}" HAVE_MALLOC_TRIM )
