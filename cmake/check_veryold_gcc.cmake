#cast away too old compiler
if ( ${CMAKE_COMPILER_IS_GNUCXX} )
	set (_CHECK_OLDCOMPILER_SOURCE_CODE "
	#ifdef __GNUC__
	#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3)
	int main() {}
	#else
	syntax error
	#endif
	#endif
	")

	include (CheckCXXSourceCompiles)
	CHECK_CXX_SOURCE_COMPILES("${_CHECK_OLDCOMPILER_SOURCE_CODE}" _RESULT)
	if (NOT _RESULT)
		message(FATAL_ERROR "Gcc version error. Minspec is 3.4")
	endif (NOT _RESULT)
endif()
