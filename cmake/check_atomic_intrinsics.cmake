# find if intrinsic __sync_fetch_and_or is available
include ( CheckCXXSourceCompiles )
CHECK_CXX_SOURCE_COMPILES ( "
int main() {
	volatile int ia=0;
	__sync_fetch_and_or( &ia, 1 );
	return 0;
}" HAVE_SYNC_FETCH )
