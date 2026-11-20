cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

if (DEFINED WITH_SSL AND NOT WITH_SSL)
	return()
endif()

if (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL Windows)
	message (STATUS "will NOT do cross-compile OpenSSL for windows")
	return()
endif()

include ( GetVCPKG )
if (NOT VCPKG)
	return()
endif()

message ( STATUS "executing ${VCPKG} install ${vcpkg_params} openssl" )
execute_process (
		COMMAND "${VCPKG}" install ${vcpkg_params}
				"--overlay-ports=${MANTICORE_SOURCE_DIR}/cmake/vcpkg-overlay-ports" openssl
		RESULT_VARIABLE OPENSSL_INSTALL_RESULT
)
if (NOT "${OPENSSL_INSTALL_RESULT}" STREQUAL "0")
	message ( FATAL_ERROR "Installing OpenSSL 3.6.5 failed (${OPENSSL_INSTALL_RESULT}); refusing to fall back to an older library." )
endif()
