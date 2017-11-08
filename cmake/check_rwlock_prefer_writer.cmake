#find if PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP flag is available
set ( _CHECK_RWLOCK_PREFER_WRITER_PROG "
#include <pthread.h>
#ifdef __CLASSIC_C__
int main() {
	int ac;
	char*av[];
#else
int main(int ac, char*av[]){
#endif
	pthread_rwlockattr_t tAttr;
	pthread_rwlockattr_setkind_np ( &tAttr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP );
	return 0;
}" )

include ( CheckCXXSourceCompiles )
set (_oldflags "${CMAKE_REQUIRED_FLAGS}")

set ( CMAKE_REQUIRED_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_RWLOCK_PREFER_WRITER_PROG}" HAVE_RWLOCK_PREFER_WRITER )

set ( CMAKE_REQUIRED_FLAGS "${_oldflags}" )
unset ( _oldflags )
