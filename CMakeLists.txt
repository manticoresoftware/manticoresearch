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

PROJECT ( MANTICORE )
SET ( MANTICORE_CMAKE_DIR "${MANTICORE_SOURCE_DIR}/cmake" )
SET ( CMAKE_MODULE_PATH "${MANTICORE_SOURCE_DIR}/cmake" )
SET ( EXTRA_LIBRARIES )

option ( DISABLE_TESTING "Disable all testing (if you just build a distr)" OFF )

if ( NOT LIBS_BUNDLE )
	SET ( LIBS_BUNDLE "${MANTICORE_SOURCE_DIR}/../bundle" )
endif ()

# Make release build for the pointed distr
# That will override defaults and give possibility to build
# the distribution with minimal command line
if ( DISTR_BUILD )
	# set default options to be included into build
	set ( DISTR "${DISTR_BUILD}" CACHE STRING "Choose the distr." )
	set ( CMAKE_BUILD_TYPE "RelWithDebInfo" )
	set ( SPLIT_SYMBOLS ON CACHE BOOLEAN "Forced debuginfo" )
	set ( WITH_MYSQL ON CACHE BOOLEAN "Forced Mysql" )
	set ( WITH_EXPAT ON CACHE BOOLEAN "Forced Expat" )
	set ( WITH_PGSQL ON CACHE BOOLEAN "Forced Pgsql" )
	set ( WITH_RE2 ON CACHE BOOLEAN "Forced RE2" )
	set ( WITH_STEMMER ON CACHE BOOLEAN "Forced Stemmer" )
	SET ( DISABLE_TESTING 1 CACHE BOOLEAN "Forced no testing" FORCE )
	SET ( CMAKE_INSTALL_PREFIX "" CACHE FILEPATH "Forced install to /" FORCE )
	message ( STATUS "DISTR_BUILD applied.
Package will be set to ${DISTR_BUILD},
also option forced to ON values: WITH_MYSQL, WITH_EXPAT, WITH_PGSQL, WITH_RE2, WITH_STEMMER.
Also LIBS_BUNDLE is automatically set to folder 'bundle' placed one level above the sources" )
endif ()


# Collect build info which will be embedded into binaries
SET ( BANNER )
# append any text to build info
macro( cfginfo msgtext )
	list ( APPEND BANNER "${msgtext}" )
endmacro()

# append values of given var to build info
macro( memcfgvalues )
	FOREACH ( var ${ARGN} )
		if ( ${var} )
			list ( APPEND BANNER "${var}=${${var}}" )
		endif ( ${var} )
	endforeach ()
endmacro()


memcfgvalues ( CMAKE_BUILD_TYPE )
memcfgvalues ( DISTR_BUILD )

# ctest variables
set ( DEFAULT_CTEST_CONFIGURATION_TYPE "Debug" )

if ( NOT DISABLE_TESTING )
	include ( CTest )
endif ()

set ( SPHINX_TAG "" CACHE STRING "The tag which will be added to build" )
set ( PKGSUFFIXES ) # list of suffixes which will be append to package name, will be '-'-separated
set ( PACKAGE_NAME "manticore" )
set ( COMPANY_NAME "Manticore Software LTD" )

message ( STATUS "Checking for endianess" )
include ( SphEndianess )

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
			LIST ( APPEND PKGSUFFIXES "pgsql" )
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
	add_definitions ("-D_CRT_SECURE_NO_WARNINGS")
    add_definitions ( "-wd4996" "-wd4706" "-wd4244" "-wd4702" )
else ( WIN32 )

	include ( helpers )

	message ( STATUS "Checking for gcc version" )
	include ( check_veryold_gcc )

	if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
		set ( HAVE_GCC_LIKE ON )
	endif ()
	mark_as_advanced ( CMAKE_CXX_COMPILER CMAKE_C_COMPILER )

	if ( HAVE_GCC_LIKE )
		set ( CFLAGS "-fno-strict-aliasing -Wall -g" )
		add_definitions ( "-D_FILE_OFFSET_BITS=64" )
		if ( CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo )
			set ( CFLAGS "${CFLAGS} -O2" )
			add_definitions ( "-DNDEBUG" )
		elseif ( CMAKE_BULD_TYPE STREQUAL Release )
			set ( CFLAGS "-fno-strict-aliasing -Wall -O3" )
			set ( SPLIT_SYMBOLS OFF )
			add_definitions ( "-DNDEBUG" )
		elseif ( CMAKE_BUILD_TYPE STREQUAL Debug )
#			set ( CLANGFLAGS "-Wthread-safety" ) # made a different option, off by default
			add_definitions ( "-D_DEBUG" )
			set ( SPLIT_SYMBOLS OFF )
		else ()
			set ( CFLAGS "${CFLAGS} -O3" )
			add_definitions ( "-DNDEBUG" )
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

	include ( mincore_test )

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

	message ( STATUS "Checking for SO_REUSEPORT flag" )
	include ( check_so_reuseport_availability )

	message ( STATUS "Checking for PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP flag" )
	include ( check_rwlock_prefer_writer )

	message ( STATUS "Configuring the options..." )
	message ( STATUS "Option DISABLE_MEMROUTINES ${DISABLE_MEMROUTINES}" )
	option ( DISABLE_MEMROUTINES "If the build is broken because new/delete override, this option will disable the overriding in sphinx" )
	if ( DISABLE_MEMROUTINES )
		SET ( SPH_DONT_OVERRIDE_MEMROUTINES 1 )
		cfginfo ( "DISABLE_MEMROUTINES=ON" )
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
		GET_SONAME ( ${_DUMMY_LIB} UNIXODBC_LIB )
		message ( STATUS "ODBC will be loaded dynamically in runtime as ${UNIXODBC_LIB}" )
		memcfgvalues ( DL_UNIXODBC UNIXODBC_LIB )
	else ( DL_UNIXODBC )
		list ( APPEND EXTRA_LIBRARIES ${_DUMMY_LIB} )
		message ( STATUS "ODBC will be linked as ${_DUMMY_LIB}" )
	endif ( DL_UNIXODBC )

	message ( STATUS "Option WITH_ZLIB ${WITH_ZLIB}" )
	option_menu ( ZLIB "compile with zlib support" USE_ZLIB EXTRA_LIBRARIES )
	mark_as_advanced ( WITH_ZLIB )

	message ( STATUS "Option WITH_RLP ${WITH_RLP}" )
	option ( WITH_RLP "compile with RLP library support" OFF )
	if ( WITH_RLP )
		SET ( RLP_ROOT "${MANTICORE_SOURCE_DIR}/rlp" CACHE PATH "RLP root folder." )
		if ( RLP_PATH )
			if ( NOT RLP_ROOT )
				set ( RLP_ROOT "${RLP_PATH}" )
				message ( STATUS "Option RLP_PATH is deprecated. Use RLP_ROOT instead." )
			else ()
				message ( STATUS "Option RLP_PATH is deprecated. Will use value of RLP_ROOT." )
			endif ()
		endif ()

		if ( RLP_ROOT )
			if ( EXISTS "${RLP_ROOT}/rlp/include/bt_rlp_c.h" )
				set ( USE_RLP 1 )
			else ()
				message ( SEND_ERROR "missing RLP sources from ${RLP_ROOT}" )
			endif ()
		endif ()

		if ( USE_RLP )
			if ( NOT RLP_LIBPATH )
				file ( GLOB RLPLIBS "${RLP_ROOT}/lib/*gcc*" )
			endif ()

			include_directories ( "${RLP_ROOT}/rlp/include" "${RLP_ROOT}/utilities/include" )
			add_definitions ( "-D_REENTRANT" )
			find_library ( LIBRLPC btrlpc HINT "${RLPLIBS}" )
			find_library ( LIBRLPCORE btrlpcore HINT "${RLPLIBS}" )
			find_library ( LIBRLPUTILS btutils HINT "${RLPLIBS}" )
			list ( APPEND EXTRA_LIBRARIES ${LIBRLPC} ${LIBRLPCORE} ${LIBRLPUTILS} )
			message ( STATUS "RLP from ${RLP_ROOT}, libraries looked at ${RLPLIBS}" )
		else ()
			message ( STATUS "RLP is not active, check path of RLP_ROOT (${RLP_ROOT})" )
		endif ()

		mark_as_advanced ( USE_RLP RLP_PATH LIBRLPC LIBRLPCORE LIBRLPUTILS RLP_LIBPATH RLPLIBS )
	endif ( WITH_RLP )

	# test for EXPAT
	message ( STATUS "Option WITH_EXPAT ${WITH_EXPAT}" )
	unset ( _DUMMY_LIB )
	option_menu ( EXPAT "compile with libexpat support" USE_LIBEXPAT _DUMMY_LIB )
	CMAKE_DEPENDENT_OPTION ( DL_EXPAT "load expat library dynamically" ON "USE_LIBEXPAT;HAVE_DL" OFF )
	if ( DL_EXPAT )
		set ( DL_EXPAT 1 )
		GET_SONAME ( ${_DUMMY_LIB} EXPAT_LIB )
		message ( STATUS "EXPAT will be loaded dynamically in runtime as ${EXPAT_LIB}" )
		memcfgvalues ( DL_EXPAT EXPAT_LIB )
	else ( DL_EXPAT )
		list ( APPEND EXTRA_LIBRARIES ${_DUMMY_LIB} )
		message ( STATUS "EXPAT will be linked as ${_DUMMY_LIB}" )
	endif ()

	# test for ICONV
	message ( STATUS "Option WITH_ICONV ${WITH_ICONV}" )
	if ( WITH_EXPAT )
		option_menu ( ICONV "compile with iconv support" USE_LIBICONV EXTRA_LIBRARIES )
		mark_as_advanced ( WITH_ICONV )
		memcfgvalues ( USE_LIBICONV )
	endif ( WITH_EXPAT )

	# test for MYSQL
	message ( STATUS "Option WITH_MYSQL ${WITH_MYSQL}" )
	option ( WITH_MYSQL "compile with MySQL support" ON )
	set ( WITH_MYSQL_INCLUDES "" CACHE PATH "path to MySQL header files" )
	set ( WITH_MYSQL_LIBS "" CACHE PATH "path to MySQL library" )
	set ( WITH_MYSQL_ROOT "" CACHE PATH "path to the MySQL bundle (where both header and library lives)" )
	mark_as_advanced ( WITH_MYSQL_INCLUDES WITH_MYSQL_LIBS WITH_MYSQL_ROOT )


	if ( WITH_MYSQL )
		include ( GetMYSQL )
	endif ( WITH_MYSQL )

	# test for POSTGRESQL
	message ( STATUS "Option WITH_PGSQL ${WITH_PGSQL}" )
	option ( WITH_PGSQL "compile with PostgreSQL support" OFF )
	set ( WITH_PGSQL_INCLUDES "" CACHE PATH "path to PostgreSQL header files" )
	set ( WITH_PGSQL_LIBS "" CACHE PATH "path to PostgreSQL library" )
	mark_as_advanced ( WITH_PGSQL_LIBS WITH_PGSQL_INCLUDES )

	if ( WITH_PGSQL )
		include ( GetPGSQL )
	endif ( WITH_PGSQL )

endif ( WIN32 )

INCLUDE ( GNUInstallDirs )

if ( "${CMAKE_INSTALL_FULL_DATAROOTDIR}" STREQUAL "/share" )
	set ( CMAKE_INSTALL_FULL_DATAROOTDIR "/usr${CMAKE_INSTALL_FULL_DATAROOTDIR}" )
	set ( CMAKE_INSTALL_FULL_DATADIR "/usr${CMAKE_INSTALL_FULL_DATADIR}" )
	set ( CMAKE_INSTALL_FULL_MANDIR "/usr${CMAKE_INSTALL_FULL_MANDIR}" )
	set ( CMAKE_INSTALL_FULL_DOCDIR "/usr${CMAKE_INSTALL_FULL_DOCDIR}" )
endif()

if ( "${CMAKE_INSTALL_FULL_BINDIR}" STREQUAL "/bin" )
	set ( CMAKE_INSTALL_FULL_BINDIR "/usr${CMAKE_INSTALL_FULL_BINDIR}" )
endif()

SET ( LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR} )
SET ( LOCALSTATEDIR ${CMAKE_INSTALL_FULL_LOCALSTATEDIR} )
SET ( DATADIR "${LOCALSTATEDIR}/data" )
SET ( SHAREDIR "${CMAKE_INSTALL_FULL_DATADIR}" )
SET ( BINDIR ${CMAKE_INSTALL_FULL_BINDIR} )
SET ( MANDIR ${CMAKE_INSTALL_FULL_MANDIR} )
SET ( SYSCONFDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR} )
SET ( DOCDIR ${CMAKE_INSTALL_FULL_DOCDIR} )

add_definitions ( "-DDATADIR=\"${DATADIR}\"" )
memcfgvalues ( DATADIR )

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
	SET ( USE_64BIT 0 )
	cfginfo ( "ENABLE_ID64=OFF" )
endif ( ENABLE_ID64 )

# Check for RE2 build
message ( STATUS "Option WITH_RE2 ${WITH_RE2}" )
option ( WITH_RE2 "compile with re2 library support" OFF )
if ( WITH_RE2 )
	OPTION ( WITH_RE2_FORCE_STATIC "force to compile re2 from sources" OFF )
	include ( GetRE2 )
endif ( WITH_RE2 )

# Check for Libstemmer build
message ( STATUS "Option WITH_STEMMER ${WITH_STEMMER}" )
OPTION ( WITH_STEMMER "compile with stemmer support" OFF )
IF ( WITH_STEMMER )
	OPTION ( WITH_STEMMER_FORCE_STATIC "force to compile stemmer from sources" OFF )
	include ( GetStemmer )
	LIST ( APPEND PKGSUFFIXES "stemmer" )
ENDIF ( WITH_STEMMER )

# Move this flags here to avoid configuration problems
if ( HAVE_GCC_LIKE AND CMAKE_BUILD_TYPE STREQUAL Debug )
	OPTION ( COVERAGE_TEST "Test coverage" OFF )
	if ( COVERAGE_TEST )
		set ( POSTCFLAGS "-fprofile-arcs -ftest-coverage" )
	endif ( COVERAGE_TEST )

	if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
		OPTION ( THREAD_SAFETY "Clang thread-safety warnings" ON )
		if ( THREAD_SAFETY )
			set ( POSTFLAGS "-Wthread-safety" )
		endif ()
	endif ()

	SET ( SANITIZER None CACHE STRING "Choose a sanitizer checker" )
	set_property ( CACHE SANITIZER PROPERTY STRINGS "None" "Thread"
			"Address" "Memory" )

	if ( SANITIZER STREQUAL "Thread" )
		set ( POSTCFLAGS "${POSTCFLAGS} -O1 -fsanitize=thread -fno-omit-frame-pointer -fPIC" )
	elseif ( SANITIZER STREQUAL "Address" )
		set ( POSTCFLAGS "${POSTCFLAGS} -O1 -fsanitize=address -fno-omit-frame-pointer" )
	elseif ( SANITIZER STREQUAL "Memory" )
		set ( POSTCFLAGS "${POSTCFLAGS} -O1 -fsanitize=memory -fno-omit-frame-pointer" )
	endif () # "None" means no action

	set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${POSTCFLAGS}" )
	set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${POSTCFLAGS}" )
endif ()

option ( SPLIT_SYMBOLS "whether to split debug symbols from binaries" ${SPLIT_SYMBOLS} )

memcfgvalues ( SPLIT_SYMBOLS USE_BISON USE_FLEX USE_SYSLOG WITH_EXPAT WITH_ICONV WITH_MYSQL
		WITH_MYSQL_INCLUDES WITH_MYSQL_LIBS WITH_MYSQL_ROOT WITH_ODBC WITH_PGSQL
		WITH_PGSQL_INCLUDES WITH_PGSQL_LIBS WITH_RE2 WITH_RE2_INCLUDES WITH_RE2_LIBS
		WITH_RE2_ROOT WITH_RLP WITH_STEMMER WITH_ZLIB )

if ( WIN32 )
	IF ( CMAKE_EXE_LINKER_FLAGS MATCHES "x64" )
		LIST ( APPEND PKGSUFFIXES "x64" )
	else ()
		LIST ( APPEND PKGSUFFIXES "win32" )
	endif ()
endif ()

# a bit of installation stuff
if ( WIN32 )
	install ( DIRECTORY api doc contrib DESTINATION ${CMAKE_INSTALL_DATADIR}/doc COMPONENT doc )
	install ( FILES example.sql DESTINATION ${CMAKE_INSTALL_DATADIR}/doc COMPONENT doc )
	install ( FILES COPYING INSTALL sphinx.conf.in sphinx-min.conf.in
			DESTINATION . COMPONENT config )
	install ( DIRECTORY DESTINATION log COMPONENT config )
	install ( DIRECTORY DESTINATION data COMPONENT config )
endif ()

configure_file ( "COPYING" "${MANTICORE_BINARY_DIR}/COPYING.txt" COPYONLY )

set ( CPACK_SOURCE_IGNORE_FILES "/\\\\.idea/" "/\\\\.git/"
		"/\\\\.svn/" "/autom4te\\\\.cache/" "/bin/" "/junk/" "/win/"
		".swp$" ".orig$" ".log$" ".deps/" )
set ( CPACK_SOURCE_GENERATOR TGZ )

set ( CPACK_PACKAGE_CONTACT "${PACKAGE_NAME} package builds <build@manticoresearch.com>" )
set ( CPACK_PACKAGE_NAME "${PACKAGE_NAME}" )
set ( CPACK_PACKAGE_VENDOR "${COMPANY_NAME}" )
set ( CPACK_RESOURCE_FILE_LICENSE "${MANTICORE_BINARY_DIR}/COPYING.txt" )
set ( CPACK_COMPONENT_APPLICATIONS_GROUP "bin" )
set ( CPACK_COMPONENT_GROUP_BIN_DISPLAY_NAME "Runtime" )
set ( CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${PACKAGE_NAME} applications" )
set ( CPACK_COMPONENT_CONFIG_GROUP "bin" )
set ( CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Configuration examples" )
set ( CPACK_COMPONENT_DOC_GROUP "bin" )
set ( CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation" )
set ( CPACK_COMPONENT_DBGSYMBOLS_GROUP "dbg" )
set ( CPACK_COMPONENT_GROUP_DBG_DISPLAY_NAME "Development" )
set ( CPACK_COMPONENT_DBGSYMBOLS_DISPLAY_NAME "Debug symbols" )
set ( CPACK_COMPONENT_DBGSYMBOLS_DEPENDS applications )

set ( CPACK_RPM_COMPONENT_INSTALL 1 )
set ( CPACK_DEB_COMPONENT_INSTALL 1 )
set ( CPACK_ARCHIVE_COMPONENT_INSTALL 1 )

include ( SetBuildType )

if ( CONFFILEDIR )
	add_definitions ( "-DSYSCONFDIR=\"${CONFFILEDIR}\"" )
	cfginfo ( "SYSCONFDIR=${CONFFILEDIR}" )
endif()

set ( CONFIGURE_FLAGS "Configured by CMake with these definitions:" )
FOREACH ( OPTION ${BANNER} )
	set ( CONFIGURE_FLAGS "${CONFIGURE_FLAGS} -D${OPTION}" )
endforeach ()

message ( STATUS "Generating config.h file" )
configure_file ( "${MANTICORE_SOURCE_DIR}/config/config_cmake.h.in"
		"${MANTICORE_BINARY_DIR}/config/config.h" ESCAPE_QUOTES )
add_definitions ( -DHAVE_CONFIG_H )
include_directories ( "${MANTICORE_BINARY_DIR}/config" )
message ( STATUS "${CONFIGURE_FLAGS}" )

set ( SPHINX_PACKAGE_SUFFIX "" )
FOREACH ( SUFFIX ${PKGSUFFIXES} )
	set ( SPHINX_PACKAGE_SUFFIX "${SPHINX_PACKAGE_SUFFIX}-${SUFFIX}" )
endforeach ()

set ( CPACK_PROJECT_CONFIG_FILE "${MANTICORE_BINARY_DIR}/config/CPackOptions.cmake" )

add_subdirectory ( src )
add_subdirectory ( test )


include ( CPack )
