#find if PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP flag is available
include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#include <pthread.h>
int main() {
	int tmp = PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP;
	return 0;
}" HAVE_RWLOCK_PREFER_WRITER )