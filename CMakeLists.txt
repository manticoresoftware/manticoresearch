cmake_minimum_required ( VERSION 3.17 )

FOREACH (policy CMP0012 CMP0013 CMP0014 CMP0048 CMP0074 CMP0077 CMP0091 CMP0110)
	IF (POLICY ${policy})
		CMAKE_POLICY ( SET ${policy} NEW )
	ENDIF ()
ENDFOREACH ()

if (DEFINED ENV{CMAKE_INTERPROCEDURAL_OPTIMIZATION})
	set ( CMAKE_INTERPROCEDURAL_OPTIMIZATION $ENV{CMAKE_INTERPROCEDURAL_OPTIMIZATION} )
endif ()

set ( CMAKE_ENABLE_EXPORTS ON )

set ( _CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} )

PROJECT ( MANTICORE )

# sometimes CMAKE_BUILD_TYPE became set after PROJECT statement, undo it.
if (NOT _CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE)
	unset ( CMAKE_BUILD_TYPE CACHE )
endif ()

SET ( CMAKE_CXX_STANDARD 17 )
SET ( MANTICORE_CMAKE_DIR "${MANTICORE_SOURCE_DIR}/cmake" )
SET ( CMAKE_MODULE_PATH "${MANTICORE_SOURCE_DIR}/cmake" )
SET ( MANTICORE_MODULE_PATH "${CMAKE_MODULE_PATH}" )

include ( init_cache_settings ) # set libs_bundle, cacheb, diagnostic. etc.
include ( helpers )

# Set a default build type for single-configuration CMake generators if no build type is set.
get_property ( isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )
if (NOT isMultiConfig AND NOT CMAKE_BUILD_TYPE)
	set ( CMAKE_BUILD_TYPE RelWithDebInfo )
	message ( STATUS "Automatically set build type to RelWithDebInfo since no other provided" )
endif ()

if (WIN32 AND WIN_BUNDLE)
	list ( APPEND CMAKE_PREFIX_PATH "${WIN_BUNDLE}" )
	set ( CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE )
endif ()

set ( BUILD_TAG "" CACHE STRING "The tag which will be added to build" )
set ( PACKAGE_NAME "manticore" )
set ( COMPANY_NAME "Manticore Software LTD" )
set ( CPACK_PACKAGE_NAME "${PACKAGE_NAME}" )

include ( setup_distribution_build ) # process PACK, DISTR_BUILD and ENV{DISTR}
include ( FeatureSummary )

# cast away too old gcc compiler (Minspec is 4.7.2)
if (CXX_COMPILER_ID STREQUAL GNU AND CXX_COMPILER_VERSION VERSION_LESS 4.7.2)
	message ( FATAL_ERROR "Gcc version error. Minspec is 4.7.2" )
endif ()

# Collect string information which will be included into binaries
include ( banner )
bannervar ( DISTR_BUILD )

add_library ( lextra INTERFACE ) # main interface lib to collect all build-wide libs and options

# Looking for threads library
set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )
target_link_libraries ( lextra INTERFACE Threads::Threads )

# Checking for specific headers
include ( CheckIncludeFile )

# that part is safe on windows, however these libs are absent there, so no reason to spend time on them
if (NOT WIN32)
	# Checking if mincore has unsigned 3-rd param
	include ( mincore_test )
	add_lib_for ( getaddrinfo_a anl lextra )
	add_lib_for ( setsockopt socket lextra )
	add_lib_for ( getaddrinfo "nsl;socket;resolv" lextra )
	add_lib_for ( clock_gettime rt lextra )
	add_lib_for ( logf m lextra )
	set ( CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} )
	check_function_exists ( poll HAVE_POLL )
	check_function_exists ( pthread_mutex_timedlock HAVE_PTHREAD_MUTEX_TIMEDLOCK )
	check_function_exists ( pthread_cond_timedwait HAVE_PTHREAD_COND_TIMEDWAIT )
	check_function_exists ( pread HAVE_PREAD )
	check_function_exists ( backtrace HAVE_BACKTRACE )
	check_function_exists ( backtrace_symbols HAVE_BACKTRACE_SYMBOLS )
	check_function_exists ( mremap HAVE_MREMAP )
	check_function_exists ( eventfd HAVE_EVENTFD )
	check_function_exists ( kqueue HAVE_KQUEUE )
	check_function_exists ( pthread_getname_np HAVE_PTHREAD_GETNAME_NP )
	check_function_exists ( getrlimit HAVE_GETRLIMIT )
	check_function_exists ( setrlimit HAVE_SETRLIMIT )
	check_function_exists ( epoll_ctl HAVE_EPOLL )

	# Checking for few other flags
	include ( CheckSymbolExists )
	check_symbol_exists ( F_SETLKW "fcntl.h" HAVE_F_SETLKW )
	check_symbol_exists ( SO_REUSEPORT "sys/types.h;sys/socket.h" HAVE_SO_REUSEPORT )
	check_symbol_exists ( malloc_trim "malloc.h" HAVE_MALLOC_TRIM )
	check_symbol_exists ( malloc_stats "malloc.h" HAVE_MALLOC_STATS )

	# Checking for PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP flag
	include ( check_rwlock_prefer_writer )

	# Check availabiliby of pthread_setname_np and whether it pick 1 or 2 params
	include ( check_vitable_pthread_setname_np )

	set ( STATIC_BINARY OFF CACHE BOOL "Produce statically linked ELF" )
	add_feature_info ( Static STATIC_BINARY "make pure static build undependent of any local libraries" )
endif ()
if (STATIC_BINARY)
	# options for clang/gcc c and c++
	target_compile_options ( lextra INTERFACE $<${ONLYGNUCLANGC_CXX}:-static> )
	set ( HAVE_DLOPEN 0 CACHE BOOL "" FORCE )
	set ( HAVE_GETADDRINFO_A 0 CACHE BOOL "" FORCE )
	set ( WITH_ODBC 0 CACHE BOOL "" FORCE )
	set ( WITH_MYSQL 0 CACHE BOOL "" FORCE )
	set ( WITH_POSTGRESQL 0 CACHE BOOL "" FORCE )
	set ( WITH_GALERA 0 CACHE BOOL "" FORCE )
	set ( WITH_SSL 0 CACHE BOOL "" FORCE )
	set ( WITH_ZSTD 0 CACHE BOOL "" FORCE )
	set ( WITH_CURL 0 CACHE BOOL "" FORCE )
	SET ( BUILD_TESTING 0 CACHE BOOL "" FORCE )
elseif (NOT WIN32)
	if (CMAKE_DL_LIBS) # CMAKE internal, usually '-ldl' on most UNIX machines
		target_link_libraries ( lextra INTERFACE ${CMAKE_DL_LIBS} )
	endif ()
	set ( CMAKE_REQUIRED_LIBRARIES "${CMAKE_DL_LIBS}" )
	check_symbol_exists ( dlopen "dlfcn.h" HAVE_DLOPEN )
else ()
	set ( HAVE_DLOPEN 1 CACHE BOOL "Use emulated dlopen" FORCE )
endif ()

include ( TestBigEndian ) # fixme! That is deprecated in cmake 3.20 in favor to CMAKE_<LANG>_BYTE_ORDER var
test_big_endian ( USE_BIG_ENDIAN )

target_compile_options ( lextra INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:-D_CRT_SECURE_NO_WARNINGS -wd4996 -wd4706 -wd4244 -wd4702 -wd4577> )

include ( CheckFunctionExists )
check_function_exists ( strnlen HAVE_STRNLEN )

# Checking for atomic function
include ( check_atomic_intrinsics )

set ( DISABLE_MEMROUTINES OFF CACHE BOOL "If the build is broken because new/delete override, this option will disable the overriding" )
mark_as_advanced ( DISABLE_MEMROUTINES )

# Option USE_SYSLOG
check_include_file ( "syslog.h" HAVE_SYSLOG_H )
set ( USE_SYSLOG ${HAVE_SYSLOG_H} CACHE BOOL "compile with possibility to use syslog for logging" )
bannervar ( USE_SYSLOG )

# options for clang/gcc c and c++
target_compile_options ( lextra INTERFACE $<${ONLYGNUCLANGC_CXX}:-D_FILE_OFFSET_BITS=64 -Wall -fno-strict-aliasing> )
target_compile_options ( lextra INTERFACE $<${CLANGWIN}:-fno-strict-aliasing -Wno-format -Wno-uninitialized-const-reference -Wno-unneeded-internal-declaration
		-Wno-deprecated-declarations -Wno-implicit-const-int-float-conversion -Wno-missing-braces -Wno-unused-function> )

# disable rtti
target_compile_options ( lextra INTERFACE $<${ONLYGNUCLANGCXX}:-fno-rtti>$<${MSCXX}:/GR-> ) # no rtti

if (WIN32)
	target_compile_definitions ( lextra INTERFACE
			_CRT_SECURE_NO_DEPRECATE=1
			_CRT_NONSTDC_NO_DEPRECATE=1
			)

	# that is necessary only for IDE correct highlighting. Note, that is only global (not target-located) definitions, global-wide
#	add_compile_definitions ( _WIN32=1 )
endif()


if (COVERAGE_TEST)
	target_compile_options ( lextra INTERFACE $<${ONLYGNUCLANGC_CXX}:-fprofile-arcs -ftest-coverage> )
	target_link_options ( lextra INTERFACE
			$<$<LINK_LANG_AND_ID:CXX,GNU>:-lgcov --coverage>
			$<$<LINK_LANG_AND_ID:C,GNU>:-lgcov --coverage>
			$<$<LINK_LANG_AND_ID:CXX,Clang,AppleClang>: --coverage>
			$<$<LINK_LANG_AND_ID:C,Clang,AppleClang>: --coverage>
			)
endif (COVERAGE_TEST)

# options for clang in C++
target_compile_options ( lextra INTERFACE $<${CLANGCXX}:
		-Wno-deprecated-register -Wno-missing-exception-spec -Wno-implicit-exception-spec-mismatch -Wno-invalid-offsetof -Wc++11-narrowing> )

# Checking for unaligned RAM access
if (CMAKE_CROSSCOMPILING)
	string ( TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _PROC )
	if (_PROC MATCHES "^(i.86|x86(_64)?)$"
			OR _PROC STREQUAL amd64
			OR _PROC STREQUAL arm64
			OR _PROC STREQUAL aarch64
			OR _PROC STREQUAL s390x)
		set ( UNALIGNED_RAM_ACCESS_EXITCODE "0" CACHE STRING "Result from TRY_RUN" )
		set ( UNALIGNED_RAM_ACCESS_EXITCODE__TRYRUN_OUTPUT "" CACHE STRING "Output from TRY_RUN" )
	endif ()
endif ()
include ( check_unaligned_ram_access )

option ( USE_SMALLALLOC "Use 'small allocator' derived from loki lib. Bench! Glibc allocator now is fast enough!" )
bannervar ( WITH_SMALLALLOC )
mark_as_advanced ( WITH_SMALLALLOC )

#check galera, as it is not available on win, to fail early
if (WIN32)
	if (WITH_GALERA)
		message ( FATAL_ERROR "Galera is not supported on win; reconfigure with WITH_GALERA=FALSE" )
	endif ()
else ()
	include ( GetGALERA )
	if (WITH_GALERA)
		cache_galera_module_name () # write GALERA_SONAME as var to cache
	endif ()
	diag ( WITH_GALERA )
	bannervar ( WITH_GALERA )
endif ()

# Check for RE2 build
set ( WITH_RE2_FORCE_STATIC 1 CACHE BOOL "force to compile re2 from sources" )
with_get ( re2 "RE2" "a regular expression library" )

# Check for Libstemmer build
set ( WITH_STEMMER_FORCE_STATIC 1 CACHE BOOL "force to compile stemmer from sources" )
with_get ( stemmer "Stemmer" "stemming library (Snowball)" )

get_dep ( nlohmann_json NLJSON "JSON for Modern C++ library" )
get_dep ( uni-algo UniAlgo "unicode algorithm implementation for C/C++" )

# Check for ICU build
set ( WITH_ICU_FORCE_STATIC 1 CACHE BOOL "force to compile ICU from sources" )
if (STATIC_BINARY)
	set ( WITH_ICU_FORCE_STATIC ON CACHE BOOL "force to compile ICU from sources" FORCE )
	set ( WITH_ICU_FORCE_BUILD ON )
endif ()
with_get ( icu "ICU" "International Components for Unicode" )

include ( GetCCTZ )
target_link_libraries ( lextra INTERFACE cctz::cctz )

include ( GetxxHash )
target_link_libraries ( lextra INTERFACE xxHash::xxhash )

with_get ( jieba "Jieba" "Chinese text segmentation tool" )

# Support for OpenSSL
set ( OPENSSL_USE_STATIC_LIBS ON )
include ( GetSSL )
with_menu_comp ( OpenSSL SSL "OpenSSL" "for encrypted networking" )
win_install_c ( OpenSSL SSL daemon )

trace ( OpenSSL::Crypto )

# Support for ZLIB
with_menu ( ZLIB "zlib" "for compressed data and networking" )
win_install ( ZLIB common )

with_menu ( ZSTD "libzstd" "for compressed networking" )
dl_package ( ZSTD "zstd" )
win_install ( ZSTD daemon )

with_menu_libname ( CURL libcurl "libCURL" "for enhanced network communication" )
dl_package_comp ( CURL libcurl "curl" )
win_install_lib ( CURL libcurl daemon )

# ODBC and it's dynamic linking
with_menu ( ODBC "ODBC/UnixODBC/iODBC" "for indexing MSSQL (windows) and generic ODBC sources with indexer" )
dl_package ( ODBC "ODBC/UnixODBC/iODBC" )

# EXPAT and it's dynamic linking
with_menu ( EXPAT "expat" "for indexing xmlpipe sources with indexer" )
dl_package ( EXPAT "expat" )
win_install ( EXPAT common )

# test for ICONV
if (WITH_EXPAT)
	with_menu ( Iconv "iconv" "for support different encodings when indexing xmlpipe sources with indexer" )
	if (Iconv_IS_BUILT_IN)
		set_target_properties ( Iconv::Iconv PROPERTIES INTERFACE_COMPILE_DEFINITIONS LIBICONV_PLUG )
	else ()
		dl_package ( Iconv "iconv" )
		win_install ( Iconv common )
	endif ()
	include ( check_const_iconv )
	diag ( Iconv_IS_BUILT_IN )
endif ()

# MYSQL and it's dynamic linking
set ( WITH_STATIC_MYSQL OFF CACHE BOOL "link to mysql library statically" )
if (WITH_STATIC_MYSQL)
	set ( MYSQL_USE_STATIC_LIBS ON )
endif ()
with_menu ( Mysql "mysql" "for indexing mysql sources with indexer" )
dl_package ( Mysql "mysql" )

# roll-back to 'unversioned' mysql dylib on macos (#2997)
if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
	if (TARGET Mysql::Mysql_ld)
		if ( DL_MYSQL )
			GET_FILENAME_COMPONENT ( __mysql_21_dylib_name ${MYSQL_LIB} NAME  )
			get_target_property ( _mysql_dylib Mysql::Mysql LOCATION )
			GET_FILENAME_COMPONENT ( __mysql_dylib_name ${_mysql_dylib} NAME )
			if ( NOT __mysql_21_dylib_name STREQUAL ${__mysql_dylib_name} )
				GET_FILENAME_COMPONENT ( __mysql_dir ${MYSQL_LIB} DIRECTORY )
				set ( MYSQL_LIB "${__mysql_dir}/${__mysql_dylib_name}" CACHE FILEPATH "Library file of Mysql" FORCE )
				infomsg ( "Mysql lib name fixed from ${__mysql_21_dylib_name} to ${__mysql_dylib_name} (${MYSQL_LIB})" )
			endif()
		endif ()
	endif ()
endif ()

win_install ( Mysql indexer )

# PostgreSQL and it's dynamic linking
with_menu ( PostgreSQL "pgsql" "for indexing postgresql sources with indexer" )
dl_package ( PostgreSQL "pgsql" )
win_install ( PostgreSQL indexer )

# Storing compiler version
set ( COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}" )
diag ( COMPILER )

# Storing system name
set ( HOST_SYSTEM "${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}" )
set ( TARGET_SYSTEM "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}" )
set ( HOST_DISTRO "${HOST_SYSTEM}" )
set ( TARGET_DISTRO "${TARGET_SYSTEM}" )
if (DISTR)
	set ( HOST_DISTRO "${HOST_DISTRO} (${DISTR})" )
	set ( TARGET_DISTRO "${TARGET_DISTRO} (${DISTR})" )
endif ()
if (CMAKE_CROSSCOMPILING)
	if (HOST_SYSTEM STREQUAL "${TARGET_SYSTEM}")
		SET ( OS_UNAME "${HOST_DISTRO} (cross-compiled)" )
	else ()
		SET ( OS_UNAME "${HOST_SYSTEM} for ${TARGET_DISTRO}" )
	endif ()
else ()
	SET ( OS_UNAME "${HOST_DISTRO}" )
endif ()
diag ( OS_UNAME )

diag ( SPLIT )

configure_file ( "LICENSE" "${MANTICORE_BINARY_DIR}/LICENSE" COPYONLY )

# Common all-packages info
include ( ${MANTICORE_SOURCE_DIR}/sphinxrev.cmake )
include ( CommonInfo )

include( builds/VersionDeps )
message("EXECUTOR image: manticoresearch/manticore-executor:${EXECUTOR_VERNUM}")

# set final paths for build
set ( only_set_paths ON )
include ( SetBuildType )

diag ( installed )
if (NOT installed) # GNUInstallDirs wasn't yet included...
	include ( GNUInstallDirs )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}" ) # will be used also in the app
	if (WIN32)
		set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc" )
		SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}" )
		SET ( CMAKE_INSTALL_FULL_LOCALLIBDIR "usr/local/lib" )
		install ( DIRECTORY api doc contrib DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
		install ( FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT tools )
		install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR} COMPONENT common )
		install ( FILES LICENSE INSTALL manticore.conf.in DESTINATION . COMPONENT common )
		install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log COMPONENT searchd )
		install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR} COMPONENT common )
		if (WITH_ICU AND WITH_ICU_FORCE_STATIC)
			install_icudata ( ${CMAKE_INSTALL_DATADIR}/icu )
		endif ()
	else ()
		set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
		configure_config ( lib/manticore )
		SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )
		SET ( CMAKE_INSTALL_FULL_LOCALLIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}" )
		install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT common )
		install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT common )
		install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore COMPONENT common )
		install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_RUNSTATEDIR}/manticore COMPONENT searchd )
		install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT searchd )
		if (WITH_ICU AND WITH_ICU_FORCE_STATIC)
			install_icudata ( ${CMAKE_INSTALL_DATADIR}/manticore/icu )
		endif ()
	endif ()
endif ()

set ( CPACK_RPM_COMPONENT_INSTALL 1 )
set ( CPACK_DEB_COMPONENT_INSTALL 1 )
set ( CPACK_ARCHIVE_COMPONENT_INSTALL 1 )

bannervar ( LOCALDATADIR )
bannervar ( FULL_SHARE_DIR )

set ( CPACK_PROJECT_CONFIG_FILE "${MANTICORE_BINARY_DIR}/config/CPackOptions.cmake" )

set ( DEFAULT_CTEST_CONFIGURATION_TYPE "Debug" )
include ( CTest )

add_subdirectory ( src )

if (BUILD_TESTING)
	add_subdirectory ( test )
	if (NOT TEST_SPECIAL_EXTERNAL)
		add_subdirectory ( api/libsphinxclient )
	endif ()
endif ()

# run setBuildType again, full pass. Select distribution build and install rules for most of the files
if (installed)
	set ( only_set_paths OFF )
	include ( SetBuildType )
endif ()

cmake_print_variables ( CMAKE_INSTALL_PREFIX )
foreach (var BINDIR SBINDIR LIBEXECDIR SYSCONFDIR SHAREDSTATEDIR LOCALSTATEDIR RUNSTATEDIR LIBDIR INCLUDEDIR OLDINCLUDEDIR DATAROOTDIR DATADIR INFODIR LOCALEDIR MANDIR DOCDIR LOCALLIBDIR)
	cmake_print_variables ( CMAKE_INSTALL_${var} CMAKE_INSTALL_FULL_${var} )
endforeach ()

if (NOT CPack_CMake_INCLUDED)
	include ( CPack )
endif ()

cpack_add_component ( server GROUP main )
cpack_add_component ( searchd GROUP main )
cpack_add_component ( common GROUP main )
cpack_add_component ( tools GROUP main )
cpack_add_component ( meta )
cpack_add_component ( devel GROUP main )
cpack_add_component ( converter GROUP helpers )
cpack_add_component_group ( main EXPANDED )
cpack_add_component_group ( helpers EXPANDED )

if (WITH_ICU)
	cpack_add_component ( icudata )
endif ()

#if (WITH_JIEBA)
#	cpack_add_component ( jiebadicts )
#endif ()

if (WIN32)
	cpack_add_component ( runtime GROUP main )
endif ()

#feature_summary (
#		INCLUDE_QUIET_PACKAGES
#		DESCRIPTION "Enabled Features1:"
#		VAR enabledFeaturesText
#		WHAT ALL)
#message (STATUS "${enabledFeaturesText}")

feature_summary ( WHAT ENABLED_FEATURES INCLUDE_QUIET_PACKAGES DESCRIPTION "Enabled features compiled in:" )
feature_summary ( WHAT RUNTIME_PACKAGES_FOUND INCLUDE_QUIET_PACKAGES DESCRIPTION "Available runtime features:" )

#feature_summary (WHAT ALL DESCRIPTION "Enabled ALL features4:")

