#=============================================================================
# Copyright 2017-2021, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
FOREACH (policy CMP0074)
	IF (POLICY ${policy})
		CMAKE_POLICY(SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

include(update_bundle)

set(ICU_GITHUB "https://github.com/unicode-org/icu/releases/download/release-65-1/icu4c-65_1-src.tgz")
set(ICU_BUNDLEZIP "icu4c-65_1-src.tgz")

# cb to check if provided icu dir contains icu src
function(CHECK_ICU_SRC RESULT HINT)
	if(EXISTS "${HINT}/as_is")
		set(${RESULT} TRUE PARENT_SCOPE)
	endif()
endfunction()

# cb to finalize icu sources (add cmake)
function(PREPARE_ICU ICU_SRC)
	# check if it is already patched before
	if (NOT EXISTS "${ICU_SRC}/CMakeLists.txt")
		configure_file("${CMAKE_SOURCE_DIR}/libicu/CMakeLists.txt" "${ICU_SRC}/CMakeLists.txt" COPYONLY)
	endif ()
endfunction()

set (__icu_namespace "icu::")
provide(ICU "${ICU_GITHUB}" "${ICU_BUNDLEZIP}")
if (ICU_FROMSOURCES)
	add_subdirectory(${ICU_SRC} ${ICU_BUILD} EXCLUDE_FROM_ALL)
	set(ICU_LIBRARIES icu)
	set(__icu_namespace)
elseif (NOT ICU_FOUND)
	unset(WITH_ICU CACHE)
	return()
endif ()

get_target_property(ICU_DATA ${__icu_namespace}icudata INTERFACE_SOURCES)

list(APPEND EXTRA_LIBRARIES ${ICU_LIBRARIES})
set(USE_ICU 1)
memcfgvalues(USE_ICU)

if (WIN32)
	# FIXME! need a proper way to detect if CMAKE_INSTALL_DATADIR is not default
	if (NOT CMAKE_INSTALL_DATADIR STREQUAL "share")
		add_compile_definitions(ICU_DATA_DIR="${CMAKE_INSTALL_DATADIR}/icu")
	else ()
		add_compile_definitions(ICU_DATA_DIR="../share/icu")
	endif ()
else ()
	add_compile_definitions(ICU_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PACKAGE_NAME}/icu")
endif ()

message(STATUS "library: ${ICU_LIBRARIES}, icu-data: ${ICU_DATA}")
diag(ICU_FOUND)
diag(ICU_INCLUDE_DIRS)
diag(ICU_LIBRARIES)
diag(ICU_SRC)
diag(ICU_BUILD)
diag(ICU_FROMSOURCES)
diag(ICU_DATA)
