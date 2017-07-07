# ---------- macos ----------
# Above line is mandatory!
# rules to build tgz archive for Mac OS X

message ( STATUS "Will create TGZ with build for Mac Os X" )

# configure specific stuff
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64" )
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64" )
list ( APPEND CMAKE_LIBRARY_PATH "/opt/local/lib" )
list ( APPEND CMAKE_INCLUDE_PATH "/opt/local/include" )

# generate config files
set ( CONFDIR "." )
configure_file ( "sphinx-min.conf.in" "${MANTICORE_BINARY_DIR}/sphinx-min.conf" @ONLY )
configure_file ( "sphinx.conf.in" "${MANTICORE_BINARY_DIR}/sphinx.conf" @ONLY )

# install specific stuff
install ( DIRECTORY api doc contrib DESTINATION . COMPONENT doc )

install ( FILES COPYING INSTALL example.sql
			"${MANTICORE_BINARY_DIR}/sphinx-min.conf"
			"${MANTICORE_BINARY_DIR}/sphinx.conf"
		DESTINATION . COMPONENT doc )

install ( DIRECTORY DESTINATION bin COMPONENT doc )
install ( DIRECTORY DESTINATION data COMPONENT doc )
install ( DIRECTORY DESTINATION log COMPONENT doc )

# package specific

find_program ( SWVERSPROG sw_vers )
if ( SWVERSPROG )
	# use dpkg to fix the package file name
	execute_process (
			COMMAND ${SWVERSPROG} -productVersion
			OUTPUT_VARIABLE MACOSVER
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	mark_as_advanced ( SWVERSPROG MACOSVER )
endif ( SWVERSPROG )

if ( NOT MACOSVER )
	set ( MACOSVER "10.12" )
endif ()

set ( CPACK_GENERATOR "TGZ" )
LIST ( APPEND PKGSUFFIXES "osx${MACOSVER}" "x86_64" )
