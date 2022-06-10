if (__cmake_printers_included)
	return ()
endif ()
set ( __cmake_printers_included YES )

# different print diagnostic stuff
#
# diag - print one or many variables
# diags - print message in DEBUG scope, if DIAGNOSTIC defined
# infomsg - make message STATUS if not CMAKE_REQUIRED_QUIET
# debugmsg - make message DEBUG
# trace - traces target properties
# diagst - promptly output if target found and where

include ( CMakePrintHelpers )
function ( DIAG )
	if (DIAGNOSTIC)
		cmake_print_variables ( ${ARGN} )
	endif ()
endfunction ()

function ( DIAGS MSG )
	if (DIAGNOSTIC)
		message ( DEBUG "${MSG}" )
	endif ()
endfunction ()

function ( infomsg MSG )
	if (NOT CMAKE_REQUIRED_QUIET)
		message ( STATUS "${MSG}" )
	endif ()
endfunction ()

function ( debugmsg MSG )
	message ( DEBUG "${MSG}" )
endfunction ()

function ( tracep LIB PROPERTY )
	get_property ( _prp TARGET ${LIB} PROPERTY ${PROPERTY} )
	if (_prp)
		diags ( "${PROPERTY} = '${_prp}'" )
	endif ()
endfunction ()

function ( trace LIB )
	if (NOT TARGET ${LIB})
		diags ( "=========== ${LIB} is not found" )
		return ()
	endif ()

	diags ( "=========== Trace properties for ${LIB} =============" )
	foreach (_prop
			TYPE
			INTERFACE_AUTOUIC_OPTIONS
			INTERFACE_COMPILE_DEFINITIONS
			INTERFACE_COMPILE_FEATURES
			INTERFACE_COMPILE_OPTIONS
			INTERFACE_INCLUDE_DIRECTORIES
			INTERFACE_LINK_LIBRARIES
			INTERFACE_LINK_DEPENDS
			INTERFACE_LINK_DIRECTORIES
			INTERFACE_LINK_OPTIONS
			INTERFACE_PRECOMPILE_HEADERS
			INTERFACE_POSITION_INDEPENDENT_CODE
			INTERFACE_SOURCES
			INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
			IMPORTED_CONFIGURATIONS
			IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO
			IMPORTED_LOCATION_DEBUG
			IMPORTED_LOCATION_RELEASE
			IMPORTED_LOCATION_RELWITHDEBINFO
			IMPORTED_LOCATION
			IMPORTED_IMPLIB_DEBUG
			IMPORTED_IMPLIB_RELEASE
			IMPORTED_IMPLIB_RELWITHDEBINFO
			IMPORTED_IMPLIB
			MAP_IMPORTED_CONFIG_DEBUG
			MAP_IMPORTED_CONFIG_RELEASE
			MAP_IMPORTED_CONFIG_RELWITHDEBINFO
			MAP_IMPORTED_CONFIG_MINSIZEREL
			DL_LIBRARY
			LOCATION
			)
		tracep ( ${LIB} ${_prop} )
	endforeach ()
	diags ( "=========== Trace properties for ${LIB} finished =============" )
endfunction ()

function ( trace_internal LIB )
	diags ( "=========== Trace properties for internal ${LIB} =============" )
	foreach (_prop
			TYPE
			INTERFACE_COMPILE_DEFINITIONS
			INTERFACE_COMPILE_FEATURES
			INTERFACE_COMPILE_OPTIONS
			INTERFACE_INCLUDE_DIRECTORIES
			INTERFACE_LINK_LIBRARIES
			INTERFACE_LINK_DEPENDS
			INTERFACE_LINK_DIRECTORIES
			INTERFACE_LINK_OPTIONS
			INTERFACE_PRECOMPILE_HEADERS
			INTERFACE_POSITION_INDEPENDENT_CODE
			INTERFACE_SOURCES
			INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
			)
		tracep ( ${LIB} ${_prop} )
	endforeach ()
	diags ( "=========== Trace properties for internal ${LIB} finished =============" )
endfunction ()

function ( diagst TRG LEGEND )
	if (TARGET ${TRG})
		get_target_property ( LOC ${TRG} LOCATION )
		diags ( "${TRG} ${LEGEND} at ${LOC}" )
	else ()
		diags ( "${TRG} is not found" )
	endif ()
endfunction ()
