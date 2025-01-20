if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
	message (FATAL_ERROR "CMake >= 2.6.0 required")
endif ()
cmake_policy (PUSH)
cmake_policy (VERSION 2.6...3.18)

# Compute the installation prefix relative to this file.
get_filename_component (_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
message (STATUS "Using config for configuring openssl - ${_IMPORT_PREFIX}/OpenSSLConfig.cmake")
# url is https://slproweb.com/products/Win32OpenSSL.html
#message (NOTICE "Configure ssl installed from from https://slproweb.com/download/Win64OpenSSL-3_4_0.msi")

set (MSRT MD)
if (OPENSSL_MSVC_STATIC_RT)
	set (MSRT MT)
endif()

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message ( FATAL_ERROR "32-bit build is not available" )
endif ()

message (NOTICE "Configure ssl installed from from https://slproweb.com/download/Win64OpenSSL-3_4_0.msi")
set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/OpenSSL-Win64")
set (_ssl "libssl")
set (_cr "libcrypto")
set (_ssldll "libssl-3-x64.dll")
set (_crdll "libcrypto-3-x64.dll")

if (OPENSSL_USE_STATIC_LIBS)
	# Create imported target OpenSSL::SSL static
	add_library (OpenSSL::SSL STATIC IMPORTED)
	set_target_properties (OpenSSL::SSL PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
			INTERFACE_LINK_LIBRARIES "OpenSSL::Crypto;ws2_32;crypt32"
			IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
			IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
			IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}d/${_ssl}_static.lib"
			IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}/${_ssl}_static.lib"
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
			IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}d/${_cr}_static.lib"
			IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}/${_cr}_static.lib"
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
			IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}d/${_ssl}.lib"
			IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}/${_ssl}.lib"
			IMPORTED_LOCATION "${_IMPORT_PREFIX}/${_ssldll}"
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
			IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}d/${_cr}.lib"
			IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/VC/x64/${MSRT}/${_cr}.lib"
			IMPORTED_LOCATION "${_IMPORT_PREFIX}/${_crdll}"
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

