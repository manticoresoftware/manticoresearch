cmake_minimum_required ( VERSION 2.8 )

FOREACH ( policy CMP0012 CMP0013 CMP0014 )
	IF ( POLICY ${policy} )
		CMAKE_POLICY ( SET ${policy} NEW )
	ENDIF ()
ENDFOREACH ()

# override default options which affects windows builds
# that is necessary to change /MD to /MT, and also to set libraries on win to implicitly defined
set ( CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake )
set ( CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake )

# set default build type to RelWithDebInfo (if nothing else provided from outside yet)
IF ( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
	SET ( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE )
	set_property ( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
			"MinSizeRel" "RelWithDebInfo" )
ENDIF ()

PROJECT ( SPHINXSEARCH )
SET ( SPHINXSEARCH_CMAKE_DIR "${SPHINXSEARCH_SOURCE_DIR}/cmake" )
SET ( CMAKE_MODULE_PATH "${SPHINXSEARCH_SOURCE_DIR}/cmake" )
SET ( EXTRA_LIBRARIES )
SET ( BANNER )
list ( APPEND BANNER "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" )

include ( CTest )

set ( SPHINX_TAG "" CACHE STRING "The tag which will be added to build" )
set ( SPHINX_PACKAGE_SUFFIX "" )

if ( WIN32 )
	# on Win we skip all the tests and just use pre-defined folder
	include ( select_library_dir )
	include ( CMakeDependentOption )
	#	expat iconv mysql pq
	if ( HAVE_expat )
		option ( WITH_EXPAT "compile with libexpat support" ON )
		if ( WITH_EXPAT )
			set ( USE_LIBEXPAT 1 )
			include_directories ( ${expat_INCLUDE} )
			CMAKE_DEPENDENT_OPTION ( DL_EXPAT "load expat library dynamically" OFF "USE_LIBEXPAT" ON )
			if ( DL_EXPAT )
				set ( DL_EXPAT 1 )
				set ( EXPAT_LIB libexpat.dll )
			else ( DL_EXPAT )
				list ( APPEND EXTRA_LIBRARIES ${expat_LIB} )
			endif ()
			install ( FILES ${expat_ROOT}/libs/libexpat.dll DESTINATION bin COMPONENT APPLICATIONS )
		endif ()
	endif ()

	if ( HAVE_iconv )
		option ( WITH_ICONV "compile with iconv support" ON )
		if ( WITH_ICONV )
			set ( USE_LIBICONV 1 )
			include_directories ( ${iconv_INCLUDE} )
			list ( APPEND EXTRA_LIBRARIES ${iconv_LIB} )
			install ( FILES ${iconv_ROOT}/bin/iconv.dll DESTINATION bin COMPONENT APPLICATIONS )
		endif ()
	endif ()

	if ( HAVE_mysql )
		option ( WITH_MYSQL "compile with mysql support" ON )
		if ( WITH_MYSQL )
			set ( USE_MYSQL 1 )
			include_directories ( ${mysql_INCLUDE} )

			CMAKE_DEPENDENT_OPTION ( DL_MYSQL "load mysql library dynamically" OFF "USE_MYSQL" ON )
			if ( DL_MYSQL )
				set ( DL_MYSQL 1 )
				set ( MYSQL_LIB libmysql.dll )
			else ( DL_MYSQL )
				list ( APPEND EXTRA_LIBRARIES ${mysql_LIB} )
			endif ()
			install ( FILES ${mysql_ROOT}/bin/libmysql.dll DESTINATION bin COMPONENT APPLICATIONS )
		endif ()
	endif ()

	if ( HAVE_pq )
		option ( WITH_PGSQL "compile with pq support" OFF )
		if ( WITH_PGSQL )
			set ( USE_PGSQL 1 )
			include_directories ( ${pq_INCLUDE} )

			CMAKE_DEPENDENT_OPTION ( DL_PGSQL "load pgsql library dynamically" OFF "USE_PGSQL" ON )
			if ( DL_PGSQL )
				set ( DL_PGSQL 1 )
				set ( PGSQL_LIB libpq.dll )
			else ( DL_PGSQL )
				list ( APPEND EXTRA_LIBRARIES ${pq_LIB} )
			endif ()
			set ( SPHINX_PACKAGE_SUFFIX "${SPHINX_PACKAGE_SUFFIX}-pgsql" )
			IF ( CMAKE_EXE_LINKER_FLAGS MATCHES "x64" )
				install ( FILES
						${pq_ROOT}/bin/libeay32.dll
						${pq_ROOT}/bin/libiconv-2.dll
						${pq_ROOT}/bin/libintl-8.dll
						${pq_ROOT}/bin/libpq.dll
						${pq_ROOT}/bin/ssleay32.dll
						DESTINATION bin COMPONENT APPLICATIONS )
			else ()
				install ( FILES
						${pq_ROOT}/bin/libeay32.dll
						${pq_ROOT}/bin/libiconv.dll
						${pq_ROOT}/bin/libintl.dll
						${pq_ROOT}/bin/libpq.dll
						${pq_ROOT}/bin/ssleay32.dll
						DESTINATION bin COMPONENT APPLICATIONS )
			endif ()
		endif ()
	endif ()
	mark_as_advanced ( CMAKE_CONFIGURATION_TYPES )
else ( WIN32 )

	include ( helpers )

	message ( STATUS "Checking for gcc version" )
	include ( check_veryold_gcc )
	message ( STATUS "Checking for endianess" )
	include ( SphEndianess )

	if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
		set ( HAVE_GCC_LIKE ON )
	endif ()

	if ( HAVE_GCC_LIKE )
		set ( CFLAGS "-fno-strict-aliasing -Wall -g" )
		add_definitions ( "-D_FILE_OFFSET_BITS=64" )
		if ( CMAKE_BUILD_TYPE MATCHES RelWithDebInfo )
			set ( CFLAGS "${CFLAGS} -O2 -g" )
		elseif ( CMAKE_BULD_TYPE MATCHES Release )
			set ( CFLAGS "-fno-strict-aliasing -Wall -O3" )
			set ( SPLIT_SYMBOLS OFF )
		elseif ( NOT CMAKE_BUILD_TYPE MATCHES Debug )
			set ( CFLAGS "${CFLAGS} -O3 -g" )
		else ()
			OPTION ( COVERAGE_TEST "Test coverage" OFF )
			if ( COVERAGE_TEST )
				set ( CFLAGS "${CFLAGS} -fprofile-arcs -ftest-coverage" )
			endif ( COVERAGE_TEST )
			set ( CLANGFLAGS "-Wthread-safety" )
			set ( SPLIT_SYMBOLS OFF )
		endif ()
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS}" )
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS} -fno-rtti -std=c++11" )

		# eliminate also some too noisy warnings from CLang
		if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
			set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register -Wno-missing-exception-spec -Wno-implicit-exception-spec-mismatch -Wno-invalid-offsetof ${CLANGFLAGS}" )
		endif ()
	endif ( HAVE_GCC_LIKE )

	message ( STATUS "Storing compiler version" )
	if ( HAVE_GCC_LIKE )
		execute_process ( COMMAND "${CMAKE_CXX_COMPILER}" "-dumpversion" OUTPUT_VARIABLE gcc_ver )
		remove_crlf ( COMPILER ${gcc_ver} )
		message ( STATUS "Compiler is ${COMPILER}" )
	endif ()

	message ( STATUS "Storing system name" )
	if ( UNIX AND NOT CYGWIN )
		execute_process ( COMMAND "uname" "-a" OUTPUT_VARIABLE OS_UNAME )
		remove_crlf ( OS_UNAME ${OS_UNAME} )
	endif ( UNIX AND NOT CYGWIN )

	message ( STATUS "Looking for threads library" )
	find_package ( Threads REQUIRED )
	list ( APPEND EXTRA_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} )
	ac_check_func_ex ( "pthread_mutex_timedlock" "" "${CMAKE_THREAD_LIBS_INIT}" )

	message ( STATUS "Checking for standard header files" )
	include ( ac_header_stdc )

	message ( STATUS "Checking for specific headers" )
	ac_check_headers ( "execinfo.h;syslog.h;sys/eventfd.h" )

	# mb use something better. The code below is copy-pasted from automake script
	message ( STATUS "Checking for library functions" )
	ac_search_libs ( "socket" "setsockopt" _DUMMY EXTRA_LIBRARIES )
	ac_search_libs ( "nsl;socket;resolv" "gethostbyname" _DUMMY EXTRA_LIBRARIES )
	ac_search_libs ( "m" "logf" HAVE_LOGF EXTRA_LIBRARIES )
	ac_search_libs ( "dl;dld" "dlopen" HAVE_DL EXTRA_LIBRARIES )
	if ( HAVE_DL )
		set ( HAVE_DLOPEN 1 )
		set ( HAVE_DLERROR 1 )
	endif ( HAVE_DL )


	include ( CheckFunctionExists )
	ac_check_funcs ( "strnlen;pread;poll" )
	ac_check_funcs ( "backtrace;backtrace_symbols" )
	ac_check_funcs ( "mremap" )
	ac_check_funcs ( "nanosleep" )
	ac_check_funcs ( "eventfd" )
	ac_check_funcs ( "kqueue" )
	check_function_exists ( epoll_ctl HAVE_EPOLL )
	ac_search_libs ( "rt" "clock_gettime" HAVE_CLOCK_GETTIME EXTRA_LIBRARIES )

	sphinx_check_define ( "F_SETLKW" "fcntl.h" )
	message ( STATUS "Checking for atomic function" )
	include ( check_atomic_intrinsics )

	message ( STATUS "Checking for unaligned RAM access" )
	include ( check_unaligned_ram_access )

	message ( STATUS "Configuring the options..." )
	message ( STATUS "Option DISABLE_MEMROUTINES ${DISABLE_MEMROUTINES}" )
	option ( DISABLE_MEMROUTINES "If the build is broken because new/delete override, this option will disable the overriding in sphinx" )
	if ( DISABLE_MEMROUTINES )
		SET ( SPH_DONT_OVERRIDE_MEMROUTINES 1 )
		list ( APPEND BANNER "DISABLE_MEMROUTINES=ON" )
	endif ( DISABLE_MEMROUTINES )
	mark_as_advanced ( DISABLE_MEMROUTINES )

	message ( STATUS "Option USE_SYSLOG ${USE_SYSLOG}" )
	CMAKE_DEPENDENT_OPTION ( USE_SYSLOG "compile with possibility to use syslog for logging" ON "HAVE_SYSLOG_H" OFF )
	IF ( USE_SYSLOG )
		set ( USE_SYSLOG 1 )
	else ( USE_SYSLOG )
		unset ( USE_SYSLOG )
	ENDIF ( USE_SYSLOG )

	message ( STATUS "Option WITH_ODBC ${WITH_ODBC}" )
	unset ( _DUMMY_LIB )
	option_menu ( ODBC "compile with UnixODBC support" USE_ODBC _DUMMY_LIB )

	message ( STATUS "ODBC found is ${ODBC_FOUND}" )

	CMAKE_DEPENDENT_OPTION ( DL_UNIXODBC "load UnixODBC library dynamically" ON "USE_ODBC;HAVE_DL" OFF )
	if ( DL_UNIXODBC )
		set ( DL_UNIXODBC 1 )
		GET_FILENAME_COMPONENT ( UNIXODBC_LIB ${_DUMMY_LIB} NAME )
		message ( STATUS "ODBC will be loaded dynamically in runtime as ${UNIXODBC_LIB}" )
		list ( APPEND BANNER "DL_UNIXODBC=ON" )
	else ( DL_UNIXODBC )
		list ( APPEND EXTRA_LIBRARIES ${_DUMMY_LIB} )
	endif ( DL_UNIXODBC )

	message ( STATUS "Option WITH_ZLIB ${WITH_ZLIB}" )
	option_menu ( ZLIB "compile with zlib support" USE_ZLIB EXTRA_LIBRARIES )
	mark_as_advanced ( WITH_ZLIB )

	message ( STATUS "Option WITH_RLP ${WITH_RLP}" )
	option ( WITH_RLP "compile with RLP library support" OFF )
	if ( WITH_RLP )
		if ( EXISTS "${SPHINXSEARCH_SOURCE_DIR}/rlp/rlp/include/bt_rlp_c.h" )
			set ( USE_RLP 1 )
		else ()
			message ( SEND_ERROR "missing RLP sources from librlp" )
			unset ( WITH_RLP CACHE )
		endif ()
	endif ( WITH_RLP )

	# test for EXPAT
	message ( STATUS "Option WITH_EXPAT ${WITH_EXPAT}" )
	unset ( _DUMMY_LIB )
	option_menu ( EXPAT "compile with libexpat support" USE_LIBEXPAT _DUMMY_LIB )
	CMAKE_DEPENDENT_OPTION ( DL_EXPAT "load expat library dynamically" ON "USE_LIBEXPAT;HAVE_DL" OFF )
	if ( DL_EXPAT )
		set ( DL_EXPAT 1 )
		GET_FILENAME_COMPONENT ( EXPAT_LIB ${_DUMMY_LIB} NAME )
		message ( STATUS "EXPAT will be loaded dynamically in runtime as ${EXPAT_LIB}" )
		list ( APPEND BANNER "DL_EXPAT=ON" )
	else ( DL_EXPAT )
		list ( APPEND EXTRA_LIBRARIES ${_DUMMY_LIB} )
	endif ()

	# test for ICONV
	message ( STATUS "Option WITH_ICONV ${WITH_ICONV}" )
	if ( WITH_EXPAT )
		option_menu ( ICONV "compile with iconv support" USE_LIBICONV EXTRA_LIBRARIES )
		mark_as_advanced ( WITH_ICONV )
	endif ( WITH_EXPAT )

	# test for MYSQL
	message ( STATUS "Option WITH_MYSQL ${WITH_MYSQL}" )
	option ( WITH_MYSQL "compile with MySQL support" ON )
	set ( WITH_MYSQL_INCLUDES "" CACHE PATH "path to MySQL header files" )
	set ( WITH_MYSQL_LIBS "" CACHE PATH "path to MySQL library" )
	set ( WITH_MYSQL_ROOT "" CACHE PATH "path to the MySQL bundle (where both header and library lives)" )
	mark_as_advanced ( WITH_MYSQL_INCLUDES )
	mark_as_advanced ( WITH_MYSQL_LIBS )
	mark_as_advanced ( WITH_MYSQL_ROOT )

	if ( WITH_MYSQL )
		find_package ( MYSQL )

		if ( MYSQL_FOUND )
			set ( USE_MYSQL 1 )
			include_directories ( ${MYSQL_INCLUDE_DIR} )

			# -DNDEBUG we set or reset on global level, so purge it from myqsl flags
			string ( REPLACE "-DNDEBUG" "" MYSQL_CXXFLAGS "${MYSQL_CXXFLAGS}" )
			# keep only defs, include paths and libs
			string ( REGEX MATCHALL "-[DLIl]([^ ]+)" MYSQL_CXXFLAGS "${MYSQL_CXXFLAGS}" )
			# convert list after MATCHALL back to plain string
			string ( REGEX REPLACE ";" " " MYSQL_CXXFLAGS "${MYSQL_CXXFLAGS}" )

			if ( MYSQL_CXXFLAGS )
				set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CXXFLAGS}" )
			endif ( MYSQL_CXXFLAGS )
			CMAKE_DEPENDENT_OPTION ( DL_MYSQL "load mysql library dynamically" ON "MYSQL_FOUND;HAVE_DL;NOT STATIC_MYSQL" OFF )
			CMAKE_DEPENDENT_OPTION ( STATIC_MYSQL "link to mysql library statically" OFF "MYSQL_FOUND;NOT DL_MYSQL" OFF )
			if ( STATIC_MYSQL )
				message ( STATUS "Mysql will be linked statically" )
				string ( REGEX REPLACE "mysqlclient" "libmysqlclient.a" MYSQL_LIBRARIES "${MYSQL_LIBRARIES}" )
			endif ( STATIC_MYSQL )
			if ( DL_MYSQL )
				message ( STATUS "Mysql will not be linked (will be loaded at runtime)" )
				set ( DL_MYSQL 1 )
				list ( APPEND BANNER "DL_MYSQL=ON" )
			else ( DL_MYSQL )
				list ( APPEND EXTRA_LIBRARIES ${MYSQL_LIBRARIES} )
			endif ()
			list ( APPEND BANNER "MYSQL_LIB=${MYSQL_LIB}" )
			list ( APPEND BANNER "MYSQL_CONFIG_EXECUTABLE=${MYSQL_CONFIG_EXECUTABLE}" )
			mark_as_advanced ( MYSQL_CONFIG_EXECUTABLE )
			mark_as_advanced ( MYSQL_LIB )
		else ( MYSQL_FOUND )
			message ( SEND_ERROR
					"********************************************************************************
					ERROR: cannot find MySQL libraries. If you want to compile with MySQL support,
					you must either specify file locations explicitly using
					-D WITH_MYSQL_INCLUDES=... and -D WITH_MYSQL_LIBS=... options, or make sure path to
					mysql_config is listed in your PATH environment variable. Or specify the path usint
					-D MYSQL_CONFIG_EXECUTABLE=/path/to/mysql_config. If you want to
					disable MySQL support, use -D WITH_MYSQL=OFF option.
					********************************************************************************" )
		endif ( MYSQL_FOUND )
	endif ( WITH_MYSQL )

	# test for POSTGRESQL
	message ( STATUS "Option WITH_PGSQL ${WITH_PGSQL}" )
	option ( WITH_PGSQL "compile with PostgreSQL support" OFF )
	set ( WITH_PGSQL_INCLUDES "" CACHE PATH "path to PostgreSQL header files" )
	set ( WITH_PGSQL_LIBS "" CACHE PATH "path to PostgreSQL library" )
	mark_as_advanced ( WITH_PGSQL_LIBS )
	mark_as_advanced ( WITH_PGSQL_INCLUDES )

	if ( WITH_PGSQL )
		if ( WITH_PGSQL_INCLUDES )
			set ( PostgreSQL_INCLUDE_DIR ${WITH_PGSQL_INCLUDES} )
		endif ( WITH_PGSQL_INCLUDES )

		if ( WITH_PGSQL_LIBS )
			set ( PostgreSQL_LIBRARIES ${WITH_PGSQL_LIBS}/libpq.so )
		endif ( WITH_PGSQL_LIBS )

		if ( NOT ( WITH_PGSQL_INCLUDES AND WITH_PGSQL_LIBS ))
			find_package ( PostgreSQL )
		endif ()

		if ( PostgreSQL_FOUND )
			set ( USE_PGSQL 1 )
			include_directories ( ${PostgreSQL_INCLUDE_DIRS} )

			CMAKE_DEPENDENT_OPTION ( DL_PGSQL "load pgsql library dynamically" ON "PostgreSQL_FOUND;HAVE_DL;NOT STATIC_PGSQL" OFF )
			CMAKE_DEPENDENT_OPTION ( STATIC_PGSQL "link to pgsql library statically" OFF "PostgreSQL_FOUND;NOT DL_PGSQL" OFF )
			if ( STATIC_PGSQL )
				message ( STATUS "PosgreSQL will be linked statically" )
				string ( REGEX REPLACE "pq" "libpq.a" PostgreSQL_LIBRARIES "${PostgreSQL_LIBRARIES}" )
			endif ( STATIC_PGSQL )

			if ( DL_PGSQL )
				message ( STATUS "PosgreSQL will not be linked (will be loaded at runtime)" )
				set ( DL_PGSQL 1 )
				GET_FILENAME_COMPONENT ( PGSQL_LIB ${PostgreSQL_LIBRARY} NAME )
			else ( DL_PGSQL )
				list ( APPEND EXTRA_LIBRARIES ${PostgreSQL_LIBRARIES} )
			endif ()
		else ( PostgreSQL_FOUND )
			message ( SEND_ERROR
					"********************************************************************************
					ERROR: cannot find PostgreSQL libraries. If you want to compile with PosgregSQL support,
					you must either specify file locations explicitly using
					-D WITH_PGSQL_INCLUDES=... and -D WITH_PGSQL_LIBS=... options, or make sure path to
					pg_config is listed in your PATH environment variable. If you want to
					disable PostgreSQL support, use -D WITH_PGSQL=OFF option.
					********************************************************************************" )
		endif ( PostgreSQL_FOUND )
	endif ( WITH_PGSQL )

endif ( WIN32 )

if ( NOT CMAKE_BUILD_TYPE MATCHES Debug )
	add_definitions ( "-DNDEBUG" )
endif ()

find_package ( BISON )
if ( BISON_FOUND )
	option ( USE_BISON "Will use bison generated grammars" ON )
endif ( BISON_FOUND )

find_package ( FLEX )
if ( FLEX_FOUND )
	option ( USE_FLEX "Use fresh flex generated lexers" ON )
endif ( FLEX_FOUND )

if ( USE_BISON )
	set ( CMAKE_GENERATED_GRAMMAR ON )
else ()
	unset ( CMAKE_GENERATED_GRAMMAR )
	include_directories ( "${CMAKE_CURRENT_SOURCE_DIR}/src" )
endif ()

if ( USE_FLEX )
	set ( CMAKE_GENERATED_LEXER ON )
else ()
	unset ( CMAKE_GENERATED_LEXER )
	include_directories ( "${CMAKE_CURRENT_SOURCE_DIR}/src" )
endif ()

message ( STATUS "Option ENABLE_ID64 ${ENABLE_ID64}" )
mark_as_advanced ( ENABLE_ID64 )
option ( ENABLE_ID64 "use 64-bit document and word IDs" ON )
if ( ENABLE_ID64 )
	SET ( USE_64BIT 1 )
else ()
	list ( APPEND BANNER "ENABLE_ID64=OFF" )
endif ( ENABLE_ID64 )

message ( STATUS "Option WITH_RE2 ${WITH_RE2}" )
option ( WITH_RE2 "compile with re2 library support" OFF )
set ( WITH_RE2_INCLUDES "" CACHE PATH "path to re2 header files" )
set ( WITH_RE2_LIBS "" CACHE PATH "path to re2 libraries" )
set ( WITH_RE2_ROOT "" CACHE PATH "path to the libre2 bundle (where both header and library lives)" )
mark_as_advanced ( WITH_RE2_INCLUDES )
mark_as_advanced ( WITH_RE2_LIBS )
mark_as_advanced ( WITH_RE2_ROOT )

if ( WITH_RE2 )
	find_package ( RE2 )
	if ( RE2_FOUND )
		set ( USE_RE2 1 )
		include_directories ( ${RE2_INCLUDE_DIRS} )
		# check whether we will build RE2 from sources ourselves
		if ( RE2_PATH )
			add_subdirectory ( ${RE2_PATH} )
			list ( APPEND EXTRA_LIBRARIES RE2 )
		else ( RE2_PATH )
			list ( APPEND EXTRA_LIBRARIES ${RE2_LIBRARIES} )
		endif ( RE2_PATH )
	else ( RE2_FOUND )
		set ( RE2_BASEDIR "${SPHINXSEARCH_BINARY_DIR}/libre2" )
		mark_as_advanced ( RE2_BASEDIR )
		if ( NOT EXISTS "${RE2_BASEDIR}/Makefile" )
			find_package ( Git QUIET )
			if ( GIT_EXECUTABLE )
				set ( RE2_REPO "https://github.com/sphinxsearch/re2.git" )
				message ( STATUS "Cloning RE2 from ${RE2_REPO}" )
				# here we have to clone remote libre2 repo
				# c-p from cmake external project: try to clone 3 times in case of errors.
				set ( error_code 1 )
				set ( number_of_tries 0 )
				while ( error_code AND number_of_tries LESS 3 )
					execute_process (
							COMMAND "${GIT_EXECUTABLE}" clone "${RE2_REPO}" "${RE2_BASEDIR}"
							WORKING_DIRECTORY "${SPHINXSEARCH_BINARY_DIR}"
							RESULT_VARIABLE error_code
					)
					math ( EXPR number_of_tries "${number_of_tries} + 1" )
				endwhile ()
				if ( error_code )
					message ( FATAL_ERROR "Failed to clone repository: ${RE2_REPO}" )
				endif ()
			else () # No git executable found
				# check whether we have local copy (to not disturb network)
				if ( EXISTS "${LIBS_BUNDLE}/master.zip" )
					message ( STATUS "Unpack RE2 from ${LIBS_BUNDLE}/master.zip" )
					execute_process (
							COMMAND "${CMAKE_COMMAND}" -E tar xfz "${LIBS_BUNDLE}/master.zip"
							WORKING_DIRECTORY "${SPHINXSEARCH_BINARY_DIR}" )
					# download from github as zip archive
				else ()
					set ( RE2_URL "https://github.com/sphinxsearch/re2/archive/master.zip" )
					if ( NOT EXISTS "${SPHINXSEARCH_BINARY_DIR}/master.zip" )
						message ( STATUS "Downloading RE2" )
						file ( DOWNLOAD ${RE2_URL} ${SPHINXSEARCH_BINARY_DIR}/master.zip SHOW_PROGRESS )
					endif ()
					message ( STATUS "Unpack RE2 from ${SPHINXSEARCH_BINARY_DIR}/master.zip" )
					execute_process (
							COMMAND "${CMAKE_COMMAND}" -E tar xfz "${SPHINXSEARCH_BINARY_DIR}/master.zip"
							WORKING_DIRECTORY "${SPHINXSEARCH_BINARY_DIR}" )
				endif ()
				file ( RENAME "${SPHINXSEARCH_BINARY_DIR}/re2-master" "${SPHINXSEARCH_BINARY_DIR}/libre2" )
			endif ()
			# for master we don't need to explicitly checkout

			# also backup original Makefile; it is important step!
			configure_file ( "${RE2_BASEDIR}/Makefile" "${RE2_BASEDIR}/MakefileOrig" @ONLY )
		endif ( NOT EXISTS "${RE2_BASEDIR}/Makefile" )

		# RE2 sources cloned. Now patch them, if necessary
		if ( NOT EXISTS "${RE2_BASEDIR}/is_patched.txt" )
			set ( PATCH_FILE "${CMAKE_SOURCE_DIR}/libre2/libre2.patch" )

			mark_as_advanced ( PATCH_FILE )
			message ( STATUS "Patching RE2" )
			find_package ( Git QUIET )
			if ( GIT_EXECUTABLE )
				execute_process ( COMMAND "${GIT_EXECUTABLE}" apply ${PATCH_FILE}
						WORKING_DIRECTORY "${RE2_BASEDIR}"
						)
				file ( WRITE "${RE2_BASEDIR}/is_patched.txt" "ok" )
				# no git
			else ()
				find_program ( PATCH_PROG patch )
				if ( PATCH_PROG )
					mark_as_advanced ( PATCH_PROG )
					execute_process (
							COMMAND "${PATCH_PROG}" -p1 --binary -i "${PATCH_FILE}"
							WORKING_DIRECTORY "${RE2_BASEDIR}"
					)
					file ( WRITE "${RE2_BASEDIR}/is_patched.txt" "ok" )
				endif ()
			endif ()
		endif ()
		if ( NOT EXISTS "${RE2_BASEDIR}/is_patched.txt" )
			message ( ERROR "Couldn't patch RE2 distro. No Git or Patch found" )
		endif ()

		# copy our CMakeLists there
		if ( NOT EXISTS "${RE2_BASEDIR}/CMakeLists.txt" )
			message ( STATUS "${CMAKE_SOURCE_DIR} - source dir" )
			configure_file ( "${CMAKE_SOURCE_DIR}/libre2/CMakeLists.txt" "${RE2_BASEDIR}/CMakeLists.txt" @ONLY )
		endif ()
		set ( USE_RE2 1 )
		set ( RE2_INCLUDE_DIRS "${RE2_BASEDIR}" )
		include_directories ( ${RE2_BASEDIR} )
		add_subdirectory ( ${RE2_BASEDIR} ${RE2_BASEDIR} EXCLUDE_FROM_ALL )
		list ( APPEND EXTRA_LIBRARIES RE2 )
	endif ( RE2_FOUND )
endif ( WITH_RE2 )

message ( STATUS "Option WITH_STEMMER ${WITH_STEMMER}" )
OPTION ( WITH_STEMMER "compile with stemmer support" OFF )
IF ( WITH_STEMMER )
	find_package ( stemmer )
	if ( STEMMER_FOUND )
		SET ( USE_LIBSTEMMER 1 )
		include_directories ( ${STEMMER_INCLUDE_DIR} )
		if ( STEMMER_INTERNAL )
			add_subdirectory ( ${CMAKE_SOURCE_DIR}/libstemmer_c )
			list ( APPEND EXTRA_LIBRARIES stemmer )
		else ()
			list ( APPEND EXTRA_LIBRARIES ${STEMMER_LIBRARY} )
		endif ( STEMMER_INTERNAL )
	else ( STEMMER_FOUND )
		if ( EXISTS "${LIBS_BUNDLE}/libstemmer_c.tgz" )
			message ( STATUS "Unpack Stemmer from ${LIBS_BUNDLE}/libstemmer_c.tgz" )
			execute_process (
					COMMAND "${CMAKE_COMMAND}" -E tar xfz "${LIBS_BUNDLE}/libstemmer_c.tgz"
					WORKING_DIRECTORY "${SPHINXSEARCH_BINARY_DIR}" )
			# download from github as zip archive
		else ( EXISTS "${LIBS_BUNDLE}/libstemmer_c.tgz" )
			set ( STEMMER_URL "http://snowball.tartarus.org/dist/libstemmer_c.tgz" )
			if ( NOT EXISTS "${SPHINXSEARCH_BINARY_DIR}/libstemmer_c.tgz" )
				message ( STATUS "Downloading Stemmer" )
				file ( DOWNLOAD ${STEMMER_URL} ${SPHINXSEARCH_BINARY_DIR}/libstemmer_c.tgz SHOW_PROGRESS )
			endif ()
			message ( STATUS "Unpack Stemmer from ${SPHINXSEARCH_BINARY_DIR}/libstemmer_c.tgz" )
			execute_process (
					COMMAND "${CMAKE_COMMAND}" -E tar xfz "${SPHINXSEARCH_BINARY_DIR}/libstemmer_c.tgz"
					WORKING_DIRECTORY "${SPHINXSEARCH_BINARY_DIR}" )
		endif ( EXISTS "${LIBS_BUNDLE}/libstemmer_c.tgz" )
		set ( STEMMER_BASEDIR "${SPHINXSEARCH_BINARY_DIR}/libstemmer_c" )
		# copy our CMakeLists there
		if ( NOT EXISTS "${STEMMER_BASEDIR}/CMakeLists.txt" )
			configure_file ( "${CMAKE_SOURCE_DIR}/libstemmer_c/CMakeLists.txt" "${STEMMER_BASEDIR}/CMakeLists.txt" @ONLY )
		endif ()
		set ( USE_LIBSTEMMER 1 )
		set ( STEMMER_INCLUDE_DIR "${STEMMER_BASEDIR}/include" )
		include_directories ( ${STEMMER_BASEDIR}/include )
		add_subdirectory ( ${STEMMER_BASEDIR} ${STEMMER_BASEDIR} EXCLUDE_FROM_ALL )
		list ( APPEND EXTRA_LIBRARIES stemmer )
		if ( NOT EXISTS "${STEMMER_BASEDIR}/CMakeLists.txt" )
			message ( SEND_ERROR "missing libstemmer sources from libstemmer_c.
Please download the C version of libstemmer library from
http://snowball.tartarus.org/ and extract its sources over libstemmer_c/
subdirectory in order to build Sphinx with libstemmer support. Or
install the package named like 'libstemmer-dev' using your favorite
package manager." )
			unset ( WITH_STEMMER CACHE )
		endif ()
	ENDIF ( STEMMER_FOUND )
	set ( SPHINX_PACKAGE_SUFFIX "${SPHINX_PACKAGE_SUFFIX}-stemmer" )
ENDIF ( WITH_STEMMER )

option ( SPLIT_SYMBOLS "whether to split debug symbols from binaries" ON )

LIST ( APPEND BANNER "SPLIT_SYMBOLS=${SPLIT_SYMBOLS}" )
LIST ( APPEND BANNER "USE_BISON=${USE_BISON}" )
LIST ( APPEND BANNER "USE_FLEX=${USE_FLEX}" )
LIST ( APPEND BANNER "USE_SYSLOG=${USE_SYSLOG}" )
LIST ( APPEND BANNER "WITH_EXPAT=${WITH_EXPAT}" )
LIST ( APPEND BANNER "WITH_ICONV=${WITH_ICONV}" )
LIST ( APPEND BANNER "WITH_MYSQL=${WITH_MYSQL}" )
LIST ( APPEND BANNER "WITH_MYSQL_INCLUDES=${WITH_MYSQL_INCLUDES}" )
LIST ( APPEND BANNER "WITH_MYSQL_LIBS=${WITH_MYSQL_LIBS}" )
LIST ( APPEND BANNER "WITH_MYSQL_ROOT=${WITH_MYSQL_ROOT}" )
LIST ( APPEND BANNER "WITH_ODBC=${WITH_ODBC}" )
LIST ( APPEND BANNER "WITH_PGSQL=${WITH_PGSQL}" )
LIST ( APPEND BANNER "WITH_PGSQL_INCLUDES=${WITH_PGSQL_INCLUDES}" )
LIST ( APPEND BANNER "WITH_PGSQL_LIBS=${WITH_PGSQL_LIBS}" )
LIST ( APPEND BANNER "WITH_RE2=${WITH_RE2}" )
LIST ( APPEND BANNER "WITH_RE2_INCLUDES=${WITH_RE2_INCLUDES}" )
LIST ( APPEND BANNER "WITH_RE2_LIBS=${WITH_RE2_LIBS}" )
LIST ( APPEND BANNER "WITH_RE2_ROOT=${WITH_RE2_ROOT}" )
LIST ( APPEND BANNER "WITH_RLP=${WITH_RLP}" )
LIST ( APPEND BANNER "WITH_STEMMER=${WITH_STEMMER}" )
LIST ( APPEND BANNER "WITH_ZLIB=${WITH_ZLIB}" )

set ( CONFIGURE_FLAGS "Configured with CMake with these definitions:" )
FOREACH ( OPTION ${BANNER} )
	set ( CONFIGURE_FLAGS "${CONFIGURE_FLAGS} -D${OPTION}" )
endforeach ()

message ( STATUS "Generating config.h file" )
configure_file ( "${SPHINXSEARCH_SOURCE_DIR}/config/config_cmake.h.in"
		"${SPHINXSEARCH_BINARY_DIR}/config/config.h" )
add_definitions ( -DHAVE_CONFIG_H )
include_directories ( "${SPHINXSEARCH_BINARY_DIR}/config" )

add_subdirectory ( src )

add_subdirectory ( test )

# a bit of installation stuff
install ( DIRECTORY api doc contrib DESTINATION share/doc COMPONENT doc )
install ( FILES example.sql DESTINATION share/doc COMPONENT doc )
if ( WIN32 )
	install ( FILES COPYING INSTALL sphinx.conf.in sphinx-min.conf.in
			DESTINATION . COMPONENT config )
	install ( DIRECTORY DESTINATION log COMPONENT config )
	install ( DIRECTORY DESTINATION data COMPONENT config )
else ()
	install ( FILES COPYING INSTALL sphinx.conf.in sphinx-min.conf.in
			DESTINATION share/doc COMPONENT config )
endif ()
configure_file ( "${SPHINXSEARCH_SOURCE_DIR}/COPYING"
		"${SPHINXSEARCH_BINARY_DIR}/COPYING.txt" )

set ( CPACK_SOURCE_IGNORE_FILES "/\\\\.idea/" "/\\\\.git/"
		"/\\\\.svn/" "/autom4te\\\\.cache/" "/bin/" "/junk/" "/win/"
		".swp$" ".orig$" ".log$" ".deps/" )
set ( CPACK_SOURCE_GENERATOR TGZ )

set ( CPACK_COMPONENT_APPLICATIONS_GROUP "bin" )
set ( CPACK_COMPONENT_GROUP_BIN_DISPLAY_NAME "Runtime" )
set (CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Manticore applications")
set ( CPACK_COMPONENT_CONFIG_GROUP "bin" )
set ( CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Configuration examples" )
set ( CPACK_COMPONENT_DOC_GROUP "bin" )
set ( CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation" )
set ( CPACK_COMPONENT_DBGSYMBOLS_GROUP "dbg" )
set ( CPACK_COMPONENT_GROUP_DBG_DISPLAY_NAME "Development" )
set ( CPACK_COMPONENT_DBGSYMBOLS_DISPLAY_NAME "Debug symbols" )
set ( CPACK_COMPONENT_DBGSYMBOLS_DEPENDS applications )
set ( CPACK_DEB_COMPONENT_INSTALL 1 )
set ( CPACK_RPM_COMPONENT_INSTALL 1 )
set ( CPACK_ARCHIVE_COMPONENT_INSTALL 1 )
set (CPACK_PACKAGE_NAME "manticore")
set (CPACK_PACKAGE_VENDOR "Manticore software LTD.")
set ( CPACK_RESOURCE_FILE_LICENSE "${SPHINXSEARCH_BINARY_DIR}/COPYING.txt" )

# fallback falues
SET ( CPACK_PACKAGE_VERSION_MAJOR 0 )
SET ( CPACK_PACKAGE_VERSION_MINOR 0 )
SET ( CPACK_PACKAGE_VERSION_PATCH 0 )

# real string
FILE ( STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/sphinx.h" _SPHINXH
		LIMIT_COUNT 500
		REGEX "^#define SPHINX_(TAG|VERSION_NUMBERS).*" )
FOREACH ( LINE ${_SPHINXH} )
	IF ( "${LINE}" MATCHES "SPHINX_TAG" )
		STRING ( REGEX REPLACE "#define[ \t]+SPHINX_TAG[ \t]+\"-?(.*)\"$" "\\1" _TAG "${LINE}" )
		set ( SPHINX_TAG "${_TAG}" )
		LIST ( APPEND BANNER "SPHINX_TAG=${SPHINX_TAG}" )
	ELSEIF ( "${LINE}" MATCHES "SPHINX_VERSION_NUMBERS" )
		STRING ( REGEX REPLACE ".*\"(.*)\"(.*)$" "\\1" _VER "${LINE}" )
		STRING ( REGEX REPLACE "\\." ";" _VER "${_VER}" )
		LIST ( APPEND _VER "0;0;0" )
		LIST ( GET _VER 0 CPACK_PACKAGE_VERSION_MAJOR )
		LIST ( GET _VER 1 CPACK_PACKAGE_VERSION_MINOR )
		LIST ( GET _VER 2 CPACK_PACKAGE_VERSION_PATCH )
	ENDIF ()
endforeach ()

IF ( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
	SET ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64 )
	SET ( CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64" )
ELSEIF ( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686" )
	SET ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386 )
	SET ( CPACK_RPM_PACKAGE_ARCHITECTURE "i386" )
ENDIF ( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )

set ( CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )
set (CPACK_PACKAGE_CONTACT "Manticore package builds <builds@manticoresearch.com>")
SET ( CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)" )

set ( FULL_VERSION "${CPACK_PACKAGE_VERSION}" )

if ( WIN32 )
	set ( SUGGEST_GENERATOR "ZIP" )
	set ( SPHINX_ARCH "win32" )
	IF ( CMAKE_EXE_LINKER_FLAGS MATCHES "x64" )
		set ( SPHINX_ARCH "x64" )
	endif ()
elseif ( APPLE )
	set ( SUGGEST_GENERATOR "TGZ" )
else ()
	set ( SUGGEST_GENERATOR "DEB" )
endif ()

if ( NOT CPACK_GENERATOR )
	set ( CPACK_GENERATOR "${SUGGEST_GENERATOR}" )
endif ()

if ( SPHINX_TAG )
	set ( FULL_VERSION "${FULL_VERSION}-${SPHINX_TAG}" )
endif ()

if ( SPHINX_PACKAGE_SUFFIX )
	set ( FULL_VERSION "${FULL_VERSION}${SPHINX_PACKAGE_SUFFIX}" )
endif ()

if ( SPHINX_ARCH )
	set ( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${FULL_VERSION}-${SPHINX_ARCH}" )
else ()
	set ( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${FULL_VERSION}" )
endif ()

# ctest variables
set ( DEFAULT_CTEST_CONFIGURATION_TYPE "Debug" )

include ( CPack )
