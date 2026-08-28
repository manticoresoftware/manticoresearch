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

if (NOT DEFINED BUILD_TAG AND DEFINED ENV{BUILD_TAG})
	set ( BUILD_TAG $ENV{BUILD_TAG} CACHE STRING "The tag which will be added to build" )
elseif (NOT DEFINED BUILD_TAG)
	set ( BUILD_TAG "" CACHE STRING "The tag which will be added to build" )
endif()
set ( PACKAGE_NAME "manticore" )
set ( COMPANY_NAME "Manticore Software LTD" )
set ( CPACK_PACKAGE_NAME "${PACKAGE_NAME}" )

include ( setup_distribution_build ) # process PACK, DISTR_BUILD and ENV{DISTR}

set ( PACK_BUNDLE OFF CACHE BOOL "Build bundle package (manticore = daemon + all deps from deps.txt); requires PACK=1 and network at configure" )
if (PACK AND PACK_BUNDLE AND DISTR AND NOT WIN32)
	# Helper: on non-zero result, merge stdout/stderr and FATAL_ERROR with script name.
	macro ( _bundle_script_fail result out err name )
		if ( ${result} )
			set ( _msg "${${out}}" )
			if ( ${err} )
				set ( _msg "${_msg}\n${${err}}" )
			endif ()
			message ( FATAL_ERROR "${name} failed (exit ${${result}}).\n${_msg}" )
		endif ()
	endmacro ()
	set ( BUNDLE_DEPS_EXTRACT_DIR "${CMAKE_BINARY_DIR}/bundle_deps_extract" )
	# Skip download if bundle deps already present. Linux package extracts typically
	# have usr/, while macOS bundle deps are normalized to bin/ and share/.
	if (
		EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/usr"
		OR EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/bin"
		OR EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/share"
	)
		message ( STATUS "PACK_BUNDLE: using existing bundle deps at ${BUNDLE_DEPS_EXTRACT_DIR}" )
	else ()
		message ( STATUS "PACK_BUNDLE: downloading bundle deps into ${BUNDLE_DEPS_EXTRACT_DIR}" )
		# Use cmake -E env to pass env vars for portability (same on Windows/macOS/Linux; no system env required)
		execute_process (
				COMMAND ${CMAKE_COMMAND} -E env
					DISTR=${DISTR}
					arch=${CMAKE_SYSTEM_PROCESSOR}
					BUNDLE_REPO_TYPE=$ENV{BUNDLE_REPO_TYPE}
						MCL_PACKAGE_DIR=$ENV{MCL_PACKAGE_DIR}
						MCL_PACKAGE_SHA=$ENV{MCL_PACKAGE_SHA}
						MCL_REPO_TYPE=$ENV{MCL_REPO_TYPE}
					REPO_ROOT=${MANTICORE_SOURCE_DIR}
					--
					"${MANTICORE_SOURCE_DIR}/dist/download_bundle_deps.sh" "${BUNDLE_DEPS_EXTRACT_DIR}"
				WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}"
				RESULT_VARIABLE _bundledeps_result
				OUTPUT_VARIABLE _bundledeps_out
				ERROR_VARIABLE _bundledeps_err
			)
		_bundle_script_fail ( _bundledeps_result _bundledeps_out _bundledeps_err "download_bundle_deps.sh" )
	endif ()
	# Import external runtime requirements discovered from the downloaded bundle deps
	# so the resulting bundle package still declares system-level dependencies.
	if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_depends.deb" )
		file ( STRINGS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_depends.deb" _bundle_deb_dep_lines )
		if ( _bundle_deb_dep_lines )
			list ( JOIN _bundle_deb_dep_lines ", " _bundle_deb_depends )
			set ( CPACK_DEBIAN_BUNDLE_PACKAGE_DEPENDS "${_bundle_deb_depends}" )
			message ( STATUS "PACK_BUNDLE: DEB bundle depends: ${CPACK_DEBIAN_BUNDLE_PACKAGE_DEPENDS}" )
		endif ()
	endif ()
	set ( _bundle_deb_replaces
		"manticore-common"
		"manticore-server-core"
		"manticore-server"
		"manticore-tools"
		"manticore-dev"
		"manticore-converter"
		"manticore-extra"
		"manticore-icudata-65l"
		"manticore-language-packs"
	)
	if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_owned_packages.deb" )
		file ( STRINGS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_owned_packages.deb" _bundle_deb_owned_lines )
		if ( _bundle_deb_owned_lines )
			list ( APPEND _bundle_deb_replaces ${_bundle_deb_owned_lines} )
		endif ()
	endif ()
	list ( REMOVE_DUPLICATES _bundle_deb_replaces )
	list ( JOIN _bundle_deb_replaces ", " _bundle_deb_replaces_joined )
	set ( CPACK_DEBIAN_BUNDLE_PACKAGE_REPLACES "${_bundle_deb_replaces_joined}" )
	set ( CPACK_DEBIAN_BUNDLE_PACKAGE_BREAKS "${_bundle_deb_replaces_joined}" )
	message ( STATUS "PACK_BUNDLE: DEB bundle replaces/breaks: ${_bundle_deb_replaces_joined}" )
	if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_requires.rpm" )
		file ( STRINGS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_requires.rpm" _bundle_rpm_req_lines )
		if ( _bundle_rpm_req_lines )
			list ( JOIN _bundle_rpm_req_lines ", " _bundle_rpm_requires )
			set ( CPACK_RPM_BUNDLE_PACKAGE_REQUIRES "${_bundle_rpm_requires}" )
			message ( STATUS "PACK_BUNDLE: RPM bundle requires: ${CPACK_RPM_BUNDLE_PACKAGE_REQUIRES}" )
		endif ()
	endif ()
	set ( _bundle_rpm_obsoletes
		"manticore-common"
		"manticore-server-core"
		"manticore-server"
		"manticore-tools"
		"manticore-devel"
		"manticore-converter"
		"manticore-extra"
		"manticore-icudata"
		"manticore-language-packs"
	)
	if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_owned_packages.rpm" )
		file ( STRINGS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_owned_packages.rpm" _bundle_rpm_owned_lines )
		if ( _bundle_rpm_owned_lines )
			list ( APPEND _bundle_rpm_obsoletes ${_bundle_rpm_owned_lines} )
		endif ()
	endif ()
	list ( REMOVE_DUPLICATES _bundle_rpm_obsoletes )
	list ( JOIN _bundle_rpm_obsoletes ", " _bundle_rpm_obsoletes_joined )
	set ( CPACK_RPM_BUNDLE_PACKAGE_OBSOLETES "${_bundle_rpm_obsoletes_joined}" )
	message ( STATUS "PACK_BUNDLE: RPM bundle obsoletes: ${_bundle_rpm_obsoletes_joined}" )
	set ( _bundle_rpm_provides
		"manticore-common"
		"manticore-server-core"
		"manticore-server"
		"manticore-tools"
		"manticore-devel"
		"manticore-converter"
		"manticore-extra"
		"manticore-icudata"
		"manticore-language-packs"
	)
	if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_provides.rpm" )
		file ( STRINGS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_provides.rpm" _bundle_rpm_provide_lines )
		if ( _bundle_rpm_provide_lines )
			list ( APPEND _bundle_rpm_provides ${_bundle_rpm_provide_lines} )
		endif ()
	endif ()
	list ( REMOVE_DUPLICATES _bundle_rpm_provides )
	list ( JOIN _bundle_rpm_provides ", " _bundle_rpm_provides_joined )
	set ( CPACK_RPM_BUNDLE_PACKAGE_PROVIDES "${_bundle_rpm_provides_joined}" )
	message ( STATUS "PACK_BUNDLE: RPM bundle provides: ${_bundle_rpm_provides_joined}" )
	# Language packs (morphology de/en/ru + jieba)
	set ( BUNDLE_LANG_PACKS_EXTRACT_DIR "${CMAKE_BINARY_DIR}/bundle_lang_packs_extract" )
	message ( STATUS "PACK_BUNDLE: downloading language packs into ${BUNDLE_LANG_PACKS_EXTRACT_DIR}" )
	execute_process (
		COMMAND "${MANTICORE_SOURCE_DIR}/dist/download_bundle_language_packs.sh" "${BUNDLE_LANG_PACKS_EXTRACT_DIR}"
		WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}"
		RESULT_VARIABLE _langpacks_result
		OUTPUT_VARIABLE _langpacks_out
		ERROR_VARIABLE _langpacks_err
	)
	_bundle_script_fail ( _langpacks_result _langpacks_out _langpacks_err "download_bundle_language_packs.sh" )
endif ()

include ( FeatureSummary )

# Option to build Source RPMs (SRPMs) - provides instructions for proper SRPM generation
set ( BUILD_SRPMS OFF CACHE BOOL "Show instructions for building Source RPMs (SRPMs)" )
if (BUILD_SRPMS)
	set ( CPACK_RPM_PACKAGE_SOURCES ON )
	message ( STATUS "SRPM generation requested - will provide instructions for proper SRPM creation" )
endif ()
add_feature_info ( "SRPM generation" BUILD_SRPMS "Show instructions for building Source RPM packages" )

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

if (QFUZZER)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options ( lextra INTERFACE -g -fsanitize=address,fuzzer )
	target_compile_definitions ( lextra INTERFACE DISABLE_MEMROUTINES=1 )
	set ( DISABLE_MEMROUTINES ON CACHE BOOL "If the build is broken because new/delete override, this option will disable the overriding" FORCE )
else()
	message(FATAL_ERROR "QFUZZER requires Clang compiler")
endif()
endif()

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
target_compile_options ( lextra INTERFACE $<${ONLYGNUCLANGC_CXX}:-D_FILE_OFFSET_BITS=64 -Wall -Wstrict-aliasing> )
target_compile_options ( lextra INTERFACE $<${CLANGWIN}:-Wstrict-aliasing -Wno-format -Wno-uninitialized-const-reference -Wno-unneeded-internal-declaration
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

set ( MANTICORE_PREFER_HOMEBREW_CURL ON CACHE BOOL "Prefer Homebrew curl over Apple SDK curl on macOS" )
if (CMAKE_SYSTEM_NAME STREQUAL Darwin AND MANTICORE_PREFER_HOMEBREW_CURL)
	set ( _curl_should_prefer_homebrew TRUE )
	if (DEFINED CURL_LIBRARY_RELEASE AND NOT CURL_LIBRARY_RELEASE MATCHES "\\.tbd$")
		set ( _curl_should_prefer_homebrew FALSE )
	endif ()

	if (_curl_should_prefer_homebrew)
		foreach (_curl_prefix /opt/homebrew/opt/curl /usr/local/opt/curl)
			if (EXISTS "${_curl_prefix}/include/curl/curl.h" AND EXISTS "${_curl_prefix}/lib/libcurl.4.dylib")
				set ( CURL_INCLUDE_DIR "${_curl_prefix}/include" CACHE PATH "Path to curl headers" FORCE )
				set ( CURL_LIBRARY_RELEASE "${_curl_prefix}/lib/libcurl.4.dylib" CACHE FILEPATH "Path to curl release library" FORCE )
				set ( CURL_ROOT "${_curl_prefix}" CACHE PATH "Path to curl installation" FORCE )
				break ()
			endif ()
		endforeach ()
	endif ()
	unset ( _curl_should_prefer_homebrew )
endif ()

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

# Bundle component: base layout (config, stopwords, dirs) plus bundle-only files. Used from cmake/builds/CommonDeb.cmake, CommonRpm.cmake when PACK_BUNDLE (distro files do not use install_component_and_bundle for common/searchd, so bundle base must be installed here).
macro ( _manticore_bundle_base_install_rules )
	install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT bundle )
	install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT bundle )
	if ( NOT NOAPI )
		install ( DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT bundle )
	endif ()
	install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore COMPONENT bundle )
	install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_RUNSTATEDIR}/manticore COMPONENT bundle )
	install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT bundle )
	install ( DIRECTORY DESTINATION /usr/local/lib/manticore COMPONENT bundle )
	_manticore_bundle_extra_install_rules ()
endmacro ()

# Bundle component only: BUNDLE_LICENSES, LICENSE, INSTALL, component-licenses, deps, lang packs (no base layout; that comes from install_component_and_bundle or _manticore_bundle_base_install_rules).
macro ( _manticore_bundle_extra_install_rules )
	configure_file ( "${MANTICORE_SOURCE_DIR}/dist/bundle/BUNDLE_LICENSES.in" "${CMAKE_BINARY_DIR}/BUNDLE_LICENSES" @ONLY )
	install ( FILES "${CMAKE_BINARY_DIR}/BUNDLE_LICENSES" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore RENAME BUNDLE_LICENSES COMPONENT bundle )
	install ( FILES "${MANTICORE_BINARY_DIR}/LICENSE" "${MANTICORE_SOURCE_DIR}/INSTALL" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore COMPONENT bundle )
	install ( DIRECTORY "${MANTICORE_SOURCE_DIR}/component-licenses" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore COMPONENT bundle )
	if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}" )
		# Merge the extracted dependency tree into the package prefix rather than the host root.
		install ( DIRECTORY ${BUNDLE_DEPS_EXTRACT_DIR}/ DESTINATION . COMPONENT bundle USE_SOURCE_PERMISSIONS )
		if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/usr/share/manticore/modules" )
			install ( DIRECTORY "${BUNDLE_DEPS_EXTRACT_DIR}/usr/share/manticore/modules/" DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules COMPONENT bundle )
		endif ()
	endif ()
	if ( EXISTS "${BUNDLE_LANG_PACKS_EXTRACT_DIR}" )
		install ( DIRECTORY ${BUNDLE_LANG_PACKS_EXTRACT_DIR}/ DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT bundle )
	endif ()
endmacro ()

# Bundle component: systemd units and etc/default files from dirserver/dirtools (both distros create these). Call from CommonDeb/CommonRpm when PACK_BUNDLE; Debian additionally installs dirserver/manticore (etc/default) before calling.
macro ( _manticore_bundle_server_tools_install_rules )
	install ( FILES "${dirserver}/manticore.service" DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system COMPONENT bundle )
	install ( FILES "${dirtools}/manticore-indexer_global" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/default COMPONENT bundle )
	install ( FILES "${dirtools}/manticore-indexer.service" "${dirtools}/manticore-indexer@.service" DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system COMPONENT bundle )
	if ( EXISTS "${MANTICORE_BINARY_DIR}/manticore.tmpfiles" )
		install ( FILES "${MANTICORE_BINARY_DIR}/manticore.tmpfiles" DESTINATION ${CMAKE_INSTALL_LIBDIR}/tmpfiles.d COMPONENT bundle RENAME searchd.conf )
	endif ()
endmacro ()

diag ( installed )
if (NOT installed) # GNUInstallDirs wasn't yet included...
	include ( GNUInstallDirs )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}" ) # will be used also in the app
	if (WIN32)
		set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc" )
		SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}" )
		SET ( CMAKE_INSTALL_FULL_LOCALLIBDIR "usr/local/lib" )
		install_component_and_bundle ( common DIRECTORY api doc contrib DESTINATION ${CMAKE_INSTALL_DOCDIR} )
		install_component_and_bundle ( tools FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} )
		install_component_and_bundle ( common DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR} )
		install_component_and_bundle ( common FILES LICENSE INSTALL manticore.conf.in DESTINATION . )
		install_component_and_bundle ( searchd DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log )
		install_component_and_bundle ( common DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR} )
		if (WITH_ICU AND WITH_ICU_FORCE_STATIC)
			install_icudata ( ${CMAKE_INSTALL_DATADIR}/icu )
		endif ()
	else ()
		set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
		configure_config ( lib/manticore )
		SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )
		SET ( CMAKE_INSTALL_FULL_LOCALLIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}" )
		# FHS layout: config under etc, data under var/lib/manticore, logs under var/log; when PACK_BUNDLE add bundle-only extras
		install_component_and_bundle ( common FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch )
		install_component_and_bundle ( common DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore )
		install_component_and_bundle ( common DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore )
		install_component_and_bundle ( searchd DIRECTORY DESTINATION ${CMAKE_INSTALL_RUNSTATEDIR}/manticore )
		install_component_and_bundle ( searchd DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore )
		if ( PACK_BUNDLE )
			_manticore_bundle_extra_install_rules ()
		endif ()
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

if ( PACK_BUNDLE AND installed AND NOT WIN32 )
	if ( APPLE )
		# macOS does not go through the Linux distro install helpers, so bundle-only
		# common payload has to be registered here explicitly.
		install ( FILES "${MANTICORE_BINARY_DIR}/manticore.conf" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT bundle )
		install ( DIRECTORY "${MANTICORE_SOURCE_DIR}/misc/stopwords" DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT bundle )
		if ( NOT NOAPI )
			install ( DIRECTORY "${MANTICORE_SOURCE_DIR}/api" DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT bundle )
		endif ()
		_manticore_bundle_extra_install_rules ()
	endif ()
	# Server/tools etc/default and systemd units are installed from CommonDeb/CommonRpm (each distro adds only the files it creates).
	install ( PROGRAMS "${MANTICORE_SOURCE_DIR}/dist/deb/manticore_new_cluster" DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bundle )
	if ( NOT APPLE )
		install ( FILES "${MANTICORE_SOURCE_DIR}/dist/70-manticore.conf" DESTINATION /etc/sysctl.d COMPONENT bundle )
	endif ()
	install ( FILES "${MANTICORE_SOURCE_DIR}/doc/indexer.1" "${MANTICORE_SOURCE_DIR}/doc/indextool.1" DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT bundle )
	install ( FILES "${MANTICORE_SOURCE_DIR}/example.sql" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT bundle )
	install ( FILES "${MANTICORE_BINARY_DIR}/manticore.conf" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT bundle RENAME manticore.conf.dist )
	# searchd: logrotate, man page
	if ( EXISTS "${MANTICORE_BINARY_DIR}/manticore.logrotate" )
		install ( FILES "${MANTICORE_BINARY_DIR}/manticore.logrotate" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT bundle RENAME manticore )
	endif ()
	install ( FILES "${MANTICORE_SOURCE_DIR}/doc/searchd.1" DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT bundle )
	# ICU data (same content as manticore-icudata-65l)
	if ( WITH_ICU AND WITH_ICU_FORCE_STATIC AND TARGET icu::icudata )
		get_target_property ( _bundle_icu_data icu::icudata INTERFACE_SOURCES )
		if ( _bundle_icu_data )
			install ( FILES ${_bundle_icu_data} DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/icu COMPONENT bundle )
		endif ()
		unset ( _bundle_icu_data )
	endif ()
	# Common: README.Debian if present
	if ( EXISTS "${MANTICORE_BINARY_DIR}/README.Debian" )
		install ( FILES "${MANTICORE_BINARY_DIR}/README.Debian" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT bundle )
	endif ()
endif ()

cmake_print_variables ( CMAKE_INSTALL_PREFIX )
foreach (var BINDIR SBINDIR LIBEXECDIR SYSCONFDIR SHAREDSTATEDIR LOCALSTATEDIR RUNSTATEDIR LIBDIR INCLUDEDIR OLDINCLUDEDIR DATAROOTDIR DATADIR INFODIR LOCALEDIR MANDIR DOCDIR LOCALLIBDIR)
	cmake_print_variables ( CMAKE_INSTALL_${var} CMAKE_INSTALL_FULL_${var} )
endforeach ()

# Configure CPack source packaging for SRPM generation
set ( CPACK_SOURCE_GENERATOR "RPM" )
set ( CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}" )

# For source RPMs: Override the make command to use cmake --build instead
# This automatically handles any generator (Unix Makefiles, Ninja, etc.)
if (CPACK_SOURCE_GENERATOR STREQUAL "RPM" OR CPACK_RPM_PACKAGE_SOURCES)
    set ( CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}
%global __make cmake --build . --parallel")
endif()

# Configure source directories for debuginfo packages
# This is required when debuginfo packaging is enabled
set ( CPACK_BUILD_SOURCE_DIRS "${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR}" )

# Bundle package name: when PACK_BUNDLE=1 the bundle package is named "manticore" (not "manticore-bundle").
# All components are still built (server, tools, common, etc.); CommonDeb/CommonRpm append "bundle" instead of "meta" so the "manticore" package is the bundle.
if (PACK_BUNDLE AND NOT WIN32)
	set ( _bundle_deb_provides
		"manticore-common (= ${CPACK_PACKAGE_VERSION})"
		"manticore-server-core (= ${CPACK_PACKAGE_VERSION})"
		"manticore-server (= ${CPACK_PACKAGE_VERSION})"
		"manticore-tools (= ${CPACK_PACKAGE_VERSION})"
		"manticore-dev (= ${CPACK_PACKAGE_VERSION})"
		"manticore-converter (= ${CPACK_PACKAGE_VERSION})"
		"manticore-extra (= ${CPACK_PACKAGE_VERSION})"
		"manticore-icudata-65l (= ${CPACK_PACKAGE_VERSION})"
		"manticore-language-packs"
	)
	if ( EXISTS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_provides.deb" )
		file ( STRINGS "${BUNDLE_DEPS_EXTRACT_DIR}/.bundle_provides.deb" _bundle_deb_provide_lines )
		if ( _bundle_deb_provide_lines )
			list ( APPEND _bundle_deb_provides ${_bundle_deb_provide_lines} )
		endif ()
	endif ()
	list ( REMOVE_DUPLICATES _bundle_deb_provides )
	list ( JOIN _bundle_deb_provides ", " _bundle_deb_provides_joined )
	set ( CPACK_DEBIAN_BUNDLE_PACKAGE_PROVIDES "${_bundle_deb_provides_joined}" )
	message ( STATUS "PACK_BUNDLE: DEB bundle provides: ${_bundle_deb_provides_joined}" )
	set ( CPACK_DEBIAN_BUNDLE_PACKAGE_NAME "manticore" )
	set ( CPACK_RPM_BUNDLE_PACKAGE_NAME "manticore" )
	if ( APPLE )
		set ( CPACK_ARCHIVE_BUNDLE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}" )
	else ()
		set ( CPACK_ARCHIVE_BUNDLE_FILE_NAME "manticore" )
	endif ()
endif ()

if (NOT CPack_CMake_INCLUDED)
	include ( CPack )
endif ()

cpack_add_component ( server GROUP main )
cpack_add_component ( searchd GROUP main )
cpack_add_component ( common GROUP main )
cpack_add_component ( tools GROUP main )
if (NOT PACK_BUNDLE)
	cpack_add_component ( meta )
endif ()
cpack_add_component ( devel GROUP main )
cpack_add_component ( converter GROUP helpers )
cpack_add_component_group ( main EXPANDED )
cpack_add_component_group ( helpers EXPANDED )
# Bundle component only on Linux/macOS; Windows does not get a populated bundle package.
# When PACK_BUNDLE, CommonDeb/CommonRpm omit "meta" from CPACK_COMPONENTS_ALL so all component packages plus bundle (named "manticore") are built.
if (PACK_BUNDLE AND NOT WIN32)
	cpack_add_component ( bundle )
	set ( CPACK_COMPONENT_BUNDLE_DISPLAY_NAME "Manticore Search" )
	set ( CPACK_COMPONENT_BUNDLE_SUMMARY "Manticore Search (server and related components)" )
	set ( CPACK_COMPONENT_BUNDLE_DESCRIPTION "Manticore Search server and related components. This package aggregates multiple components with different licenses; see /usr/share/doc/manticore/BUNDLE_LICENSES." )
endif ()

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
