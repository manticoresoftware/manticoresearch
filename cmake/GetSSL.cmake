cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

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
		COMMAND "${VCPKG}" install ${vcpkg_params} openssl
		RESULT_VARIABLE OPENSSL_INSTALL_RESULT
)
message ( STATUS "exitcode is ${OPENSSL_INSTALL_RESULT}" )