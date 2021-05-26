#find if pthread_setname_np is exists and has correct flavour - accept 2 args, int and const char*
include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES ( "
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
}" HAVE_PTHREAD_SETNAME_NP)

CHECK_CXX_SOURCE_COMPILES ( "
#define _GNU_SOURCE
#include <pthread.h>
#ifdef __CLASSIC_C__
int main() {
	int ac;
	char*av[];
#else
int main(int ac, char*av[]){
#endif
	pthread_setname_np (\"test\");
	return 0;
}" HAVE_PTHREAD_SETNAME_NP_1ARG)

