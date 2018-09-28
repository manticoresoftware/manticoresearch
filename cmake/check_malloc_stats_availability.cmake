#find if malloc_stats() is available
set ( _CHECK_MALLOC_STATS_PROG "
#include <malloc.h>
#ifdef __CLASSIC_C__
int main() {
	int ac;
	char*av[];
#else
int main(int ac, char*av[]){
#endif
	malloc_stats();
	return 0;
}" )

include ( CheckCXXSourceCompiles )
CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_MALLOC_STATS_PROG}" HAVE_MALLOC_STATS )
