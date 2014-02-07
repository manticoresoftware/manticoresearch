# We expect that all libs necessary on win32 to build sphinx are located in subfolders of one folder
# This folder must be set in the config, and then we try to figure exact libs from the path
# like having all in c:/work, we can investigate c:/work/mysql, c:/work/expat*, etc to find headers and libs,
# or c:/work/mysql-x64, c:/work/expat*-x64, etc to find headers and libs for x64 arch build

SET (SPHX_LIBX "${SPHX_LIBX}" CACHE PATH "Choose the path to the dir which contains all helper libs like expat, mysql, etc." FORCE )
if (SPHX_LIBX)
	set (is64bit FALSE)
	IF (CMAKE_EXE_LINKER_FLAGS MATCHES "x64")
		SET(is64bit TRUE)
	endif()

	# here is the list of the libs we expect to find
	foreach (req_lib expat iconv mysql)
		file(GLOB list_libs "${SPHX_LIBX}/*${req_lib}*")
		SET (flib FALSE)
		# select whether we need -x64 or simple lib for our current arch
		FOREACH (lib ${list_libs})
			if (is64bit)
				if (lib MATCHES "x64")
					SET(flib "${lib}")
				endif (lib MATCHES "x64")
			else (is64bit)
				if (NOT lib MATCHES "x64")
					SET (flib "${lib}")
				endif (NOT lib MATCHES "x64")
			endif(is64bit)
		endforeach()
		# now try to find all the necessary paths. They are different, and so, mixed right here.
		find_path(${req_lib}_include "${req_lib}.h" "${flib}/include" "${flib}/source/lib")
		find_path(${req_lib}_lib "lib${req_lib}.lib" "${flib}/lib/opt" "${flib}/libs" "${flib}/lib")
		find_path(${req_lib}_lib "${req_lib}.lib" "${flib}/lib/opt" "${flib}/libs" "${flib}/lib")
		if ( ${req_lib}_include )
			include_directories(${${req_lib}_include})
		endif()
		if ( ${req_lib}_lib )
			link_directories(${${req_lib}_lib})
		endif()
	endforeach()
endif()
