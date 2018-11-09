#find if pthread_setname_np is exists and has correct flavour - accept 2 args, int and const char*
set ( _CHECK_PTHREAD_SETNAME_NP_PROG "
#define _GNU_SOURCE
#include <pthread.h>
#ifdef __CLASSIC_C__
int main() {
	int ac;
	char*av[];
#else
int main(int ac, char*av[]){
#endif
	pthread_t thread;
	pthread_setname_np (thread, \"test\");
	return 0;
}" )

include ( CheckCXXSourceCompiles )
set ( _oldflags "${CMAKE_REQUIRED_FLAGS}" )

set ( CMAKE_REQUIRED_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}" )

CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_PTHREAD_SETNAME_NP_PROG}" HAVE_PTHREAD_SETNAME_NP )

set ( CMAKE_REQUIRED_FLAGS "${_oldflags}" )
unset ( _oldflags )
