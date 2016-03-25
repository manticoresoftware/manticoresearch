#find if the sync functions are available
if ( ${CMAKE_COMPILER_IS_GNUCXX} )
	set (_CHECK_INTERLOCKED_PROG "
int interlocked_routine ( )
{
	volatile int ia=0;
	__sync_fetch_and_add( &ia, 1 );
	__sync_fetch_and_sub( &ia, 1 );
}
#ifdef __CLASSIC_C__
int main() {
	int ac;
	char*av[];
#else
int main(int ac, char*av[]){
#endif
	int m = interlocked_routine();
	if(ac>1000)
	{
		return *av[0];
	}
	return 0;
}")

include(CheckCXXSourceCompiles)
	CHECK_CXX_SOURCE_COMPILES("${_CHECK_INTERLOCKED_PROG}" _RESULT)
endif()

if (_RESULT)
	set (HAVE_SYNC_FETCH 1)
endif (_RESULT)
