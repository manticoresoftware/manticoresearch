# We expect that all libs necessary on win32 to build sphinx are located in subfolders of one folder
# This folder must be set in the config, and then we try to figure exact libs from the path
# like having all in c:/work, we can investigate c:/work/mysql, c:/work/expat*, etc to find headers and libs,
# or c:/work/mysql-x64, c:/work/expat*-x64, etc to find headers and libs for x64 arch build

SET (LIBS_BUNDLE "${LIBS_BUNDLE}" CACHE PATH "Choose the path to the dir which contains all helper libs like expat, mysql, etc." FORCE )
if (LIBS_BUNDLE)
	set (is64bit FALSE)
	IF (CMAKE_EXE_LINKER_FLAGS MATCHES "x64")
		SET(is64bit TRUE)
	endif()

	# here is the list of the libs we expect to find
	foreach (req_lib expat iconv mysql pq)
		file(GLOB list_libs "${LIBS_BUNDLE}/*${req_lib}*" "${LIBS_BUNDLE}/*pgsql*")
		SET (flib FALSE)
		# select whether we need -x64 or simple lib for our current arch
		FOREACH (lib ${list_libs})
			if (is64bit)
				if (lib MATCHES "x64")
					SET(flib "${lib}")
					break()
				endif (lib MATCHES "x64")
			else (is64bit)
				if (NOT lib MATCHES "x64")
					SET (flib "${lib}")
					break()
				endif (NOT lib MATCHES "x64")
			endif(is64bit)
		endforeach()
		if (req_lib STREQUAL "pq")
			set (test_h "libpq-fe")
		else()
			set(test_h "${req_lib}")
		endif()
		# now try to find all the necessary paths. They are different, and so, mixed right here.
		find_path(${req_lib}_include
			"${test_h}.h"
			"${flib}/include"
			"${flib}/source/lib" NO_DEFAULT_PATH)
		message(STATUS "Looking for ${req_lib}")
		find_library(MY_${req_lib}
			NAMES ${req_lib}
				lib${req_lib}
			PATHS "${flib}/lib/opt"
				"${flib}/libs"
				"${flib}/lib"
			NO_DEFAULT_PATH)
		message(STATUS ${MY_${req_lib}})
		if (MY_${req_lib})
			set("HAVE_${req_lib}" 1)
			set(${req_lib}_LIB "${MY_${req_lib}}")
			set(${req_lib}_ROOT "${flib}")
			unset(MY_${req_lib} CACHE)
		endif()
		if ( ${req_lib}_include )
			set(${req_lib}_INCLUDE "${${req_lib}_include}")
		endif()
	endforeach()
endif()
