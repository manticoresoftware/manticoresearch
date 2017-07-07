cmake_minimum_required ( VERSION 2.8 )

PROJECT ( RE2 )
#Parses Makefile and creates the own project based on it
#(actually we only use OFILES var from there)

FILE ( READ "${CMAKE_CURRENT_SOURCE_DIR}/MakefileOrig" _CONTENT )
# replace '\ ' into one big line
STRING ( REGEX REPLACE "\\\\\n" " " _CONTENT "${_CONTENT}" )
# escape ';' (if any)
STRING ( REGEX REPLACE ";" "\\\\;" _CONTENT "${_CONTENT}" )
# now replace lf into ';' (it makes list from the line)
STRING ( REGEX REPLACE "\n" ";" _CONTENT "${_CONTENT}" )
FOREACH ( LINE ${_CONTENT} )
	# skip comments (beginning with #)
	IF ( NOT "${LINE}" MATCHES "^#.*" )
		# parse 'name=value1 value2..." - extract the 'name' part
		STRING ( REGEX REPLACE "=.*$" "" _NAME "${LINE}" )
		# extract the list of values part
		STRING ( REGEX REPLACE "^.*=" "" _LIST "${LINE}" )
		# replace (multi)spaces into ';' (it makes list from the line)
		separate_arguments ( _LIST UNIX_COMMAND "${_LIST}" )
		# finally get our two variables
		if ( _NAME STREQUAL "OFILES" )
			SET ( _OBJ "${_LIST}" )
		endif ()
		# INSTALL_HFILES and HFILES are not necessary for build,
		# however it is good if we put them into solution (just for a human that will open them)
		if ( _NAME STREQUAL "INSTALL_HFILES" )
			SET ( _IHF "${_LIST}" )
			source_group ( "Install headers" FILES ${_IHF} )
		endif ()
		if ( _NAME STREQUAL "HFILES" )
			SET ( _HF "${_LIST}" )
			source_group ( "Headers" FILES ${_HF} )
		endif ()
	endif ()
endforeach ()

if ( _OBJ )
	set ( SOURCES "" )
	foreach ( OBJ ${_OBJ} )
		get_filename_component ( NAME ${OBJ} NAME_WE )
		get_filename_component ( DIR ${OBJ} PATH )
		# exclude leading 'obj/' from all the paths.
		STRING ( REGEX REPLACE "^obj/" "" DIR "${DIR}" )
		LIST ( APPEND SOURCES "${DIR}/${NAME}.cc" )
	endforeach ()
else ( _OBJ )
	MESSAGE ( ERROR "No OFILE definition found in Makefile" )
endif ( _OBJ )


include_directories ( "." )
IF ( CMAKE_COMPILER_IS_GNUCXX )
	SET ( CMAKE_CXX_FLAGS "-Wall -O3 -g -pthread -Wsign-compare -c " )
ELSEIF ( MSVC )
	set ( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oi /GL" )
	add_definitions ( "/D NOMINMAX" )
ENDIF ()
add_definitions ( "-DNDEBUG" )
add_library ( RE2 STATIC ${SOURCES} ${_HF} ${_IHF} )

