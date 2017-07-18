# Helper script - scans cmake/builds for concrete distr rules.
# Rules for each distr must be placed in separate .cmake file.
# Each file with rule must be named asd build_*.cmake (that is used to find them).
# Name of the distr must be in comment in the first line. The name
# must not be too long, since could be reffered not only from GUI menu,
# but also from command line, like -DDISTR=rhel7, etc. and so,
# defs like -DDISTR="my super-puper os" is not so good in the case.

# It gives possibility to add more choices without touching the rest of files.

SET ( MENUDISTR )
SET ( FILESDISTR )
file ( GLOB _BUILDS "cmake/builds/build_*.cmake" )
# list among all build_...cmake, take first line and extract name from it
foreach ( CHOICE ${_BUILDS} )
	file ( READ ${CHOICE} CHOICE_NAME LIMIT 1024 )
	string ( REGEX REPLACE "\n.*" "" CHOICE_NAME "${CHOICE_NAME}" )
	string ( REGEX REPLACE "^# -* " "" CHOICE_NAME "${CHOICE_NAME}" )
	string ( REGEX REPLACE " -*$" "" CHOICE_NAME "${CHOICE_NAME}" )
	list ( APPEND MENUDISTR "${CHOICE_NAME}" )
	list ( APPEND FILESDISTR "${CHOICE}" )
endforeach ( CHOICE ${_BUILDS} )

IF ( NOT DISTR )
	SET ( DISTR default CACHE STRING "Choose the distr." )
endif ()

IF ( NOT DISTR )
	message (STATUS "Provide distr with -DDISTR=<distr>, one of: ${MENUDISTR}")

	if ( WIN32 )
		set ( SUGGEST_GENERATOR "ZIP" )
	elseif ( APPLE )
		set ( SUGGEST_GENERATOR "TGZ" )
	else ()
		set ( SUGGEST_GENERATOR "DEB" )
	endif ()

	if ( NOT CPACK_GENERATOR )
		set ( CPACK_GENERATOR "${SUGGEST_GENERATOR}" )
	endif ()

ENDIF ()

set_property ( CACHE DISTR PROPERTY STRINGS ${MENUDISTR} )

# find back the file by name of the distr and include it
list ( FIND MENUDISTR "${DISTR}" _idistr )
if ( _idistr GREATER -1 )
	list ( GET FILESDISTR ${_idistr} _RULES )
	include (${_RULES})
	message ( STATUS "Building for ${DISTR}" )
else()
	message ( STATUS "no distr selected " )
endif()
