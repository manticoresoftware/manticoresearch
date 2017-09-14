#cast away too old compiler
if ( ${CMAKE_COMPILER_IS_GNUCXX} )
	set ( _CHECK_OLDCOMPILER_SOURCE_CODE "
	#ifdef __GNUC__
	#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
	#if GCC_VERSION > 40701
	int main() {}
	#else
	syntax error
	#endif
	#endif
	" )

	include ( CheckCXXSourceCompiles )
	CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_OLDCOMPILER_SOURCE_CODE}" _RESULT )
	if ( NOT _RESULT )
		message ( FATAL_ERROR "Gcc version error. Minspec is 4.7.2" )
	endif ( NOT _RESULT )
endif ()
