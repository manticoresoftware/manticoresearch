if ( __cmake_helpers_included )
	return ()
endif ()
set ( __cmake_helpers_included YES )

# check for list of headers, ;-separated. For every existing header.h
# the HAVE_HEADER_H became defined as 1
include ( CheckIncludeFile )
macro( ac_check_headers _HEADERS )
	foreach ( it ${_HEADERS} )
		string ( REGEX REPLACE "[/.]" "_" _it "${it}" )
		string ( TOUPPER "${_it}" _it )
		check_include_file ( "${it}" "HAVE_${_it}" )
	endforeach ( it )
endmacro( ac_check_headers )

# check for list of functions, ;-separated. For every existing
# function the HAVE_FUNCION became defined as 1
include ( CheckFunctionExists )
macro( ac_check_funcs _FUNCTIONS )
	foreach ( it ${_FUNCTIONS} )
		string ( TOUPPER "${it}" _it )
		check_function_exists ( "${it}" "HAVE_${_it}" )
	endforeach ( it )
endmacro( ac_check_funcs )

macro( ac_check_func_ex _FUNCTION _INCLUDES _LIBS )
	string ( TOUPPER "${_FUNCTION}" __FUNCTION )
	set ( saved_includes "${CMAKE_REQUIRED_INCLUDES}" )
	set ( saved_libs "${CMAKE_REQUIRED_LIBRARIES}" )
	set ( CMAKE_REQUIRED_INCLUDES "${_INCLUDES}" )
	set ( CMAKE_REQUIRED_LIBRARIES "${_LIBS}" )
	check_function_exists ( "${_FUNCTION}" "HAVE_${__FUNCTION}" )
	set ( CMAKE_REQUIRED_INCLUDES "${saved_includes}" )
	set ( CMAKE_REQUIRED_LIBRARIES "${saved_libs}" )
endmacro( ac_check_func_ex _FUNCTION _INCLUDES _LIBS )

# remove cr lf from input string
#macro (remove_crlf _SOURCE _DESTINATION)
#	string(REGEX REPLACE "\n" "" _DESTINATION "${_SOURCE}")
#endmacro(remove_crlf)

# AWI, downloaded from KDE repository since has not yet been transferred
# to cmake repository as of 2006-07-31.
# http://websvn.kde.org/trunk/KDE/kdelibs/cmake/modules/CheckPrototypeExists.cmake?rev=505849&view=markup
#
# - Check if the prototype for a function exists.
# CHECK_PROTOTYPE_EXISTS (FUNCTION HEADER VARIABLE)
#
# FUNCTION - the name of the function you are looking for
# HEADER - the header(s) where the prototype should be declared
# VARIABLE - variable to store the result
#

include ( CheckCXXSourceCompiles )
macro( check_prototype_exists _SYMBOL _HEADER _RESULT )
	set ( _INCLUDE_FILES )
	foreach ( it ${_HEADER} )
		set ( _INCLUDE_FILES "${_INCLUDE_FILES}#include <${it}>\n" )
	endforeach ( it )

	set ( _CHECK_PROTO_EXISTS_SOURCE_CODE "
${_INCLUDE_FILES}
void cmakeRequireSymbol(int dummy,...){(void)dummy;}
int main()
{
#ifndef ${_SYMBOL}
#ifndef _MSC_VER
cmakeRequireSymbol(0,&${_SYMBOL});
#else
char i = sizeof(&${_SYMBOL});
#endif
#endif
return 0;
}
" )
	CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_PROTO_EXISTS_SOURCE_CODE}" ${_RESULT} )
endmacro( check_prototype_exists _SYMBOL _HEADER _RESULT )


# - Check if the symbol is defined in thad header file
# CHECK_SYMBOL_DEFINED (SYMBOL HEADER VARIABLE)
#
# SYMBOL - the name of the symbol you are looking for
# HEADER - the header(s) where the symbol should be defined
# VARIABLE - variable to store the result
#
macro( check_symbol_defined _SYMBOL _HEADER _RESULT )
	set ( _INCLUDE_FILES )
	foreach ( it ${_HEADER} )
		set ( _INCLUDE_FILES "${_INCLUDE_FILES}#include <${it}>\n" )
	endforeach ( it )

	set ( _CHECK_PROTO_EXISTS_SOURCE_CODE "
${_INCLUDE_FILES}
#ifdef ${_SYMBOL}
int main() { return 0; }
#else
#error NO_SYMBOLS
#endif
" )
	CHECK_CXX_SOURCE_COMPILES ( "${_CHECK_PROTO_EXISTS_SOURCE_CODE}" ${_RESULT} )
endmacro( check_symbol_defined _SYMBOL _HEADER _RESULT )

macro( SPHINX_CHECK_DEFINE _SYMBOL _HEADER )
	string ( TOUPPER "${_SYMBOL}" __SYMBOL )
	check_symbol_defined ( "${_SYMBOL}" "${_HEADER}" "HAVE_${__SYMBOL}" )
endmacro( SPHINX_CHECK_DEFINE _SYMBOL _HEADER )
# - Check if the DIR symbol exists like in AC_HEADER_DIRENT.
# CHECK_DIRSYMBOL_EXISTS(FILES VARIABLE)
#
# FILES - include files to check
# VARIABLE - variable to return result
#
# This module is a small but important variation on CheckSymbolExists.cmake.
# The symbol always searched for is DIR, and the test programme follows
# the AC_HEADER_DIRENT test programme rather than the CheckSymbolExists.cmake
# test programme which always fails since DIR tends to be typedef'd
# rather than #define'd.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
# CMAKE_REQUIRED_FLAGS = string of compile command line flags
# CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
# CMAKE_REQUIRED_INCLUDES = list of include directories
# CMAKE_REQUIRED_LIBRARIES = list of libraries to link

MACRO( CHECK_DIRSYMBOL_EXISTS FILES VARIABLE )
	IF ( NOT DEFINED ${VARIABLE} )
		SET ( CMAKE_CONFIGURABLE_FILE_CONTENT "/* */\n" )
		SET ( MACRO_CHECK_DIRSYMBOL_EXISTS_FLAGS ${CMAKE_REQUIRED_FLAGS} )
		IF ( CMAKE_REQUIRED_LIBRARIES )
			SET ( CHECK_DIRSYMBOL_EXISTS_LIBS
					"-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}" )
		ELSE ( CMAKE_REQUIRED_LIBRARIES )
			SET ( CHECK_DIRSYMBOL_EXISTS_LIBS )
		ENDIF ( CMAKE_REQUIRED_LIBRARIES )
		IF ( CMAKE_REQUIRED_INCLUDES )
			SET ( CMAKE_DIRSYMBOL_EXISTS_INCLUDES
					"-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}" )
		ELSE ( CMAKE_REQUIRED_INCLUDES )
			SET ( CMAKE_DIRSYMBOL_EXISTS_INCLUDES )
		ENDIF ( CMAKE_REQUIRED_INCLUDES )
		FOREACH ( FILE ${FILES} )
			SET ( CMAKE_CONFIGURABLE_FILE_CONTENT
					"${CMAKE_CONFIGURABLE_FILE_CONTENT}#include <${FILE}>\n" )
		ENDFOREACH ( FILE )
		SET ( CMAKE_CONFIGURABLE_FILE_CONTENT
				"${CMAKE_CONFIGURABLE_FILE_CONTENT}\nint main()\n{if ((DIR *) 0) return 0;}\n" )

		CONFIGURE_FILE ( "${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
				"${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c" @ONLY )

		MESSAGE ( STATUS "Looking for DIR in ${FILES}" )
		TRY_COMPILE ( ${VARIABLE}
				${CMAKE_BINARY_DIR}
				${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c
				COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
				CMAKE_FLAGS
				-DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_DIRSYMBOL_EXISTS_FLAGS}
				"${CHECK_DIRSYMBOL_EXISTS_LIBS}"
				"${CMAKE_DIRSYMBOL_EXISTS_INCLUDES}"
				OUTPUT_VARIABLE OUTPUT )
		IF ( ${VARIABLE} )
			MESSAGE ( STATUS "Looking for DIR in ${FILES} - found" )
			SET ( ${VARIABLE} 1 CACHE INTERNAL "Have symbol DIR" )
			FILE ( APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log
					"Determining if the DIR symbol is defined as in AC_HEADER_DIRENT "
					"passed with the following output:\n"
					"${OUTPUT}\nFile ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c:\n"
					"${CMAKE_CONFIGURABLE_FILE_CONTENT}\n" )
		ELSE ( ${VARIABLE} )
			MESSAGE ( STATUS "Looking for DIR in ${FILES} - not found." )
			SET ( ${VARIABLE} "" CACHE INTERNAL "Have symbol DIR" )
			FILE ( APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
					"Determining if the DIR symbol is defined as in AC_HEADER_DIRENT "
					"failed with the following output:\n"
					"${OUTPUT}\nFile ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c:\n"
					"${CMAKE_CONFIGURABLE_FILE_CONTENT}\n" )
		ENDIF ( ${VARIABLE} )
	ENDIF ( NOT DEFINED ${VARIABLE} )
ENDMACRO( CHECK_DIRSYMBOL_EXISTS )


include ( CheckLibraryExists )
macro( AC_SEARCH_LIBS LIB_REQUIRED FUNCTION_NAME TARGET_VAR LIB_DIR )
	# check if we can use FUNCTION_NAME first.
	# if possible without extra libs - ok. If no - try to use LIB_REQUIRED list.
	# finally define TARGET_VAR as 1 if found, and also append found (if necessary) library path to LIB_DIR
	#if(${LIB_REQUIRED})
	string ( TOUPPER "${FUNCTION_NAME}" _upcase_name )
	check_function_exists ( "${FUNCTION_NAME}" "HAVE_${_upcase_name}" )
	if ( HAVE_${_upcase_name} )
		set ( ${TARGET_VAR} 1 )
	else ( HAVE_${_upcase_name} )
		foreach ( LIB ${LIB_REQUIRED} )
			UNSET ( _LIB_PATH CACHE )
			find_library ( _LIB_PATH ${LIB} )
			UNSET ( _LIB_FUNCTION_FOUND CACHE )
			check_library_exists ( ${LIB} ${FUNCTION_NAME} ${_LIB_PATH} _LIB_FUNCTION_FOUND )
			if ( ${_LIB_FUNCTION_FOUND} )
				set ( ${TARGET_VAR} 1 )
				set ( ${LIB_DIR} "${${LIB_DIR}};${_LIB_PATH}" )
				break ()
			endif ()
		endforeach ( LIB )
		UNSET ( _LIB_PATH CACHE )
	endif ()
	#endif(${LIB_REQUIRED})
endmacro()


macro( REMOVE_CRLF RETVAL INSTR )
	if ( NOT INSTR EQUAL "" )
		string ( REGEX REPLACE "\n" "" ${RETVAL} ${INSTR} )
	endif ()
	#	set(${RETVAL} "${RETVAL}" PARENT_SCOPE)
endmacro()

# check if PACKAGE is available.
# if so, add the option to use or don't use the package.
# if the option set, set OUTVAR to 1, append the lib path to LIB_DIR, include directories
include ( CMakeDependentOption )
macro( OPTION_MENU PACKAGE PROMPT OUTVAR LIB_DIR )
	find_package ( ${PACKAGE} QUIET )
	CMAKE_DEPENDENT_OPTION ( WITH_${PACKAGE} "${PROMPT}" ON "${PACKAGE}_FOUND" OFF )
	IF ( WITH_${PACKAGE} )
		set ( ${OUTVAR} 1 )
		if ( ${PACKAGE}_LIBRARIES )
			list ( APPEND ${LIB_DIR} ${${PACKAGE}_LIBRARIES} )
		endif ( ${PACKAGE}_LIBRARIES )
		include_directories ( ${${PACKAGE}_INCLUDE_DIRS} )
	ENDIF ( WITH_${PACKAGE} )
endmacro()

# add debug symbols to the target
function( INSTALL_DBG BINARYNAME )
	if ( MSVC )
		# windows case. The pdbs are located in bin/config/*.pdb
		set ( PDB_PATH "${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}" )
		INSTALL ( FILES ${PDB_PATH}/${BINARYNAME}.pdb DESTINATION debug COMPONENT DBGSYMBOLS )
	elseif ( APPLE )
		# Mac OS case. We have to explicitly extract dSYM and then strip the binary
		if ( NOT DEFINED CMAKE_DSYMUTIL )
			find_program ( CMAKE_DSYMUTIL dsymutil )
		endif ()
		if ( NOT DEFINED CMAKE_DSYMUTIL )
			message ( SEND_ERROR "Missed objcopy prog. Can't split symbols!" )
			unset ( SPLIT_SYMBOLS CACHE )
		endif ()
		mark_as_advanced ( CMAKE_DSYMUTIL )

		ADD_CUSTOM_COMMAND ( TARGET ${BINARYNAME} POST_BUILD
				COMMAND ${CMAKE_DSYMUTIL} -f $<TARGET_FILE:${BINARYNAME}> -o $<TARGET_FILE:${BINARYNAME}>.dSYM
				COMMAND strip -S $<TARGET_FILE:${BINARYNAME}>
				)
		INSTALL ( FILES ${MANTICORE_BINARY_DIR}/src/${BINARYNAME}.dSYM
				DESTINATION lib/debug/usr/bin COMPONENT DBGSYMBOLS )
	else ()
		# non-windows case. For linux - use objcopy to make 'clean' and 'debug' binaries
		if ( NOT DEFINED CMAKE_OBJCOPY )
			find_package ( BinUtils QUIET )
		endif ()
		if ( NOT DEFINED CMAKE_OBJCOPY )
			find_program ( CMAKE_OBJCOPY objcopy )
		endif ()
		if ( NOT DEFINED CMAKE_OBJCOPY )
			message ( SEND_ERROR "Missed objcopy prog. Can't split symbols!" )
			unset ( SPLIT_SYMBOLS CACHE )
		endif ( NOT DEFINED CMAKE_OBJCOPY )
		mark_as_advanced ( CMAKE_OBJCOPY )
		mark_as_advanced ( BinUtils_DIR )

		ADD_CUSTOM_COMMAND ( TARGET ${BINARYNAME} POST_BUILD
				COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:${BINARYNAME}> $<TARGET_FILE:${BINARYNAME}>.dbg
				COMMAND ${CMAKE_OBJCOPY} --strip-debug $<TARGET_FILE:${BINARYNAME}>
				COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE:${BINARYNAME}>.dbg $<TARGET_FILE:${BINARYNAME}>
				)
		INSTALL ( FILES ${MANTICORE_BINARY_DIR}/src/${BINARYNAME}.dbg
				DESTINATION lib/debug/usr/bin COMPONENT DBGSYMBOLS RENAME ${BINARYNAME} )
	endif ()
endfunction()
