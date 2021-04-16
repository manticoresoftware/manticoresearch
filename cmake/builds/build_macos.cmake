# ---------- macos ----------
# Above line is mandatory!
# rules to build tgz archive for Mac OS X

set(CPACK_GENERATOR "TGZ")

message ( STATUS "Will create TGZ with build for Mac Os X" )
set ( SPLIT_SYMBOLS 1 )

# configure specific stuff
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64" )
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64" )

# generate config files
set ( CONFDIR "." )
configure_file ( "manticore.conf.in" "${MANTICORE_BINARY_DIR}/manticore.conf" @ONLY )

# install specific stuff
install ( DIRECTORY api doc manual contrib DESTINATION . COMPONENT applications )

install ( FILES COPYING INSTALL example.sql
			"${MANTICORE_BINARY_DIR}/manticore.conf"
		DESTINATION . COMPONENT doc )

install ( DIRECTORY DESTINATION bin COMPONENT applications )
install ( DIRECTORY DESTINATION data COMPONENT applications )
install ( DIRECTORY DESTINATION log COMPONENT applications )
install ( DIRECTORY misc/stopwords DESTINATION . COMPONENT applications )
if (USE_ICU)
	install ( FILES ${ICU_DATA} DESTINATION icu COMPONENT applications)
endif()

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

LIST ( APPEND PKGSUFFIXES "osx${MACOSVER}" "x86_64" )

mark_as_advanced ( CMAKE_OSX_ARCHITECTURES CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_SYSROOT )
