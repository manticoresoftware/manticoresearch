cmake_minimum_required(VERSION 3.17)

FOREACH (policy CMP0011 CMP0054 CMP0074)
	IF (POLICY ${policy})
		CMAKE_POLICY(SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

project(TestBoostContext)

set(Boost_DEBUG ON)
set(Boost_DETAILED_FAILURE_MSG ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(BOOSTROOT @BOOSTROOT@)

if (BOOSTROOT)
	set(Boost_NO_SYSTEM_PATHS ON)
endif()

find_package(Boost 1.61.0 COMPONENTS context REQUIRED)
