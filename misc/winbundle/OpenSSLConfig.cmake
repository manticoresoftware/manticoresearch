if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
	message (FATAL_ERROR "CMake >= 2.6.0 required")
endif ()
cmake_policy (PUSH)
cmake_policy (VERSION 2.6...3.18)

# Compute the installation prefix relative to this file.
get_filename_component (_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
message (STATUS "Using config for configuring openssl - ${_IMPORT_PREFIX}/OpenSSLConfig.cmake")
# url is https://slproweb.com/products/Win32OpenSSL.html
#message (NOTICE "Configure ssl installed from from https://slproweb.com/download/Win64OpenSSL-1_1_1k.msi")

set (MSRT MD)
if (OPENSSL_MSVC_STATIC_RT)
	set (MSRT MT)
endif()

set (newdistr ON)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	if (newdistr)
		message (NOTICE "Configure ssl installed from from https://slproweb.com/download/Win64OpenSSL-1_1_1k.msi")
		set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/OpenSSL-Win64")
		set (_ssl "libssl64")
		set (_cr "libcrypto64")
		set (_ssldll "libssl-1_1-x64")
		set (_crdll "libcrypto-1_1-x64")
	else()
		message (STATUS "Consider to use fresh ssl from https://slproweb.com/download/Win64OpenSSL-1_1_1k.msi")
		set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/openssl-x64")
		set (_ssl "ssleay32")
		set (_cr "libeay32")
		set (_ssldll "ssleay32")
		set (_crdll "libeay32")
	endif()
else()
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/openssl-x32")
	set (_ssl "libssl32")
	set (_cr "libcrypto32")
	set (_ssldll "libssl-1_1")
	set (_crdll "libcrypto-1_1")
endif ()

if (OPENSSL_USE_STATIC_LIBS)

	# Create imported target OpenSSL::SSL static
	add_library (OpenSSL::SSL STATIC IMPORTED)
	set_target_properties (OpenSSL::SSL PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
			INTERFACE_LINK_LIBRARIES "OpenSSL::Crypto"
			IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
			IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
			IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/VC/static/${_ssl}${MSRT}d.lib"
			IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/VC/static/${_ssl}${MSRT}.lib"
			MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
			MAP_IMPORTED_CONFIG_MINSIZEREL Release
			)

	# Create imported target OpenSSL::Crypto static
	add_library (OpenSSL::Crypto STATIC IMPORTED)
	set_target_properties (OpenSSL::Crypto PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
			IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
			IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
			IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/VC/static/${_cr}${MSRT}d.lib"
			IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/VC/static/${_cr}${MSRT}.lib"
			MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
			MAP_IMPORTED_CONFIG_MINSIZEREL Release
			)
else ()

	# Create imported target OpenSSL::SSL shared
	add_library (OpenSSL::SSL SHARED IMPORTED)
	set_target_properties (OpenSSL::SSL PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
			INTERFACE_LINK_LIBRARIES "OpenSSL::Crypto"
			IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
			IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
			IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/VC/${_ssl}${MSRT}d.lib"
			IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/VC/${_ssl}${MSRT}.lib"
			IMPORTED_LOCATION "${_IMPORT_PREFIX}/${_ssldll}.dll"
			MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
			MAP_IMPORTED_CONFIG_MINSIZEREL Release
			)

	# Create imported target OpenSSL::Crypto shared
	add_library (OpenSSL::Crypto SHARED IMPORTED)
	set_target_properties (OpenSSL::Crypto PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
			IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
			IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
			IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/VC/${_cr}${MSRT}d.lib"
			IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/VC/${_cr}${MSRT}.lib"
			IMPORTED_LOCATION "${_IMPORT_PREFIX}/${_crdll}.dll"
			MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
			MAP_IMPORTED_CONFIG_MINSIZEREL Release
			)
endif()

# Create imported target OpenSSL::applink
add_library (OpenSSL::applink INTERFACE IMPORTED)
set_target_properties (OpenSSL::applink PROPERTIES INTERFACE_SOURCES "${_IMPORT_PREFIX}/include/openssl/applink.c")

if (CMAKE_VERSION VERSION_LESS 3.1.0)
	message (FATAL_ERROR "This file relies on consumers using CMake 3.1.0 or greater.")
endif ()

# Cleanup temporary variables.
set (_IMPORT_PREFIX)
cmake_policy (POP)

