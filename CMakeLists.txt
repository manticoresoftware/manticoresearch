cmake_minimum_required ( VERSION 3.17 )

FOREACH ( policy CMP0012 CMP0013 CMP0014 CMP0048 CMP0074 CMP0077 CMP0091 CMP0110 )
	IF ( POLICY ${policy} )
		CMAKE_POLICY ( SET ${policy} NEW )
	ENDIF ()
ENDFOREACH ()

if (DEFINED ENV{CMAKE_INTERPROCEDURAL_OPTIMIZATION})
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION $ENV{CMAKE_INTERPROCEDURAL_OPTIMIZATION})
endif()

PROJECT ( MANTICORE )
SET ( CMAKE_CXX_STANDARD 14 )
SET ( MANTICORE_CMAKE_DIR "${MANTICORE_SOURCE_DIR}/cmake" )
SET ( CMAKE_MODULE_PATH "${MANTICORE_SOURCE_DIR}/cmake" )
SET ( MANTICORE_MODULE_PATH "${CMAKE_MODULE_PATH}")

include (init_cache_settings) # set libs_bundle, cacheb, diagnostic. etc.
include (helpers)

# Set a default build type for single-configuration CMake generators if no build type is set.
get_property (isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT isMultiConfig AND NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE RelWithDebInfo)
	message (STATUS "Automatically set build type to RelWithDebInfo since no other provided")
endif ()

if (WIN32 AND LIBS_BUNDLE)
	message (DEBUG "CMAKE_PREFIX_PATH on top was ${CMAKE_PREFIX_PATH}")
	LIST (APPEND CMAKE_PREFIX_PATH "${LIBS_BUNDLE}")
	set (CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
	message (DEBUG "CMAKE_PREFIX_PATH became ${CMAKE_PREFIX_PATH}")
endif ()

set (BUILD_TAG "" CACHE STRING "The tag which will be added to build")
set (PACKAGE_NAME "manticore")
set (COMPANY_NAME "Manticore Software LTD")

include (setup_distribution_build) # process PACK, DISTR_BUILD and ENV{DISTR}
include (FeatureSummary)

# cast away too old gcc compiler (Minspec is 4.7.2)
if (CXX_COMPILER_ID STREQUAL GNU AND CXX_COMPILER_VERSION VERSION_LESS 4.7.2)
	message (FATAL_ERROR "Gcc version error. Minspec is 4.7.2")
endif ()

# Collect string information which will be included into binaries
include(banner)
bannervar (DISTR_BUILD)

add_library (lextra INTERFACE) # main interface lib to collect all build-wide libs and options

# Checking for specific headers
include (CheckIncludeFile)

# that part is safe on windows, however these libs are absent there, so no reason to spend time on them
if (NOT WIN32)
	# Checking if mincore has unsigned 3-rd param
	include (mincore_test)
	check_include_file ("execinfo.h" HAVE_EXECINFO_H)
	check_include_file ("sys/eventfd.h" HAVE_SYS_EVENTFD_H)
	check_include_file ("sys/prctl.h" HAVE_SYS_PRCTL_H)
	add_lib_for (getaddrinfo_a anl lextra)
	add_lib_for (setsockopt socket lextra)
	add_lib_for (getaddrinfo "nsl;socket;resolv" lextra)
	add_lib_for (clock_gettime rt lextra)
	add_lib_for (logf m lextra)
	set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
	check_function_exists (poll HAVE_POLL)
	check_function_exists (pthread_mutex_timedlock HAVE_PTHREAD_MUTEX_TIMEDLOCK)
	check_function_exists (pthread_cond_timedwait HAVE_PTHREAD_COND_TIMEDWAIT)
	check_function_exists (pread HAVE_PREAD)
	check_function_exists (backtrace HAVE_BACKTRACE)
	check_function_exists (backtrace_symbols HAVE_BACKTRACE_SYMBOLS)
	check_function_exists (mremap HAVE_MREMAP)
	check_function_exists (nanosleep HAVE_NANOSLEEP)
	check_function_exists (eventfd HAVE_EVENTFD)
	check_function_exists (kqueue HAVE_KQUEUE)
	check_function_exists (pthread_getname_np HAVE_PTHREAD_GETNAME_NP)
	check_function_exists (getrlimit HAVE_GETRLIMIT)
	check_function_exists (setrlimit HAVE_SETRLIMIT)
	check_function_exists (epoll_ctl HAVE_EPOLL)

	# Checking for few other flags
	include (CheckSymbolExists)
	check_symbol_exists (F_SETLKW "fcntl.h" HAVE_F_SETLKW)
	check_symbol_exists (SO_REUSEPORT "sys/types.h;sys/socket.h" HAVE_SO_REUSEPORT)
	check_symbol_exists (malloc_trim "malloc.h" HAVE_MALLOC_TRIM)
	check_symbol_exists (malloc_stats "malloc.h" HAVE_MALLOC_STATS)

	# Checking for PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP flag
	include (check_rwlock_prefer_writer)

	# Check availabiliby of pthread_setname_np and whether it pick 1 or 2 params
	include (check_vitable_pthread_setname_np)

	set (STATIC_BINARY OFF CACHE BOOL "Produce statically linked ELF")
	add_feature_info (Static STATIC_BINARY "make pure static build undependent of any local libraries")
	if (STATIC_BINARY)
		# options for clang/gcc c and c++
		target_compile_options (lextra INTERFACE $<${GNUC_CXX}:-static>)
		set (HAVE_DLOPEN 0 CACHE BOOL "" FORCE)
		set (HAVE_DLERROR 0 CACHE BOOL "" FORCE)
		set (HAVE_GETADDRINFO_A 0 CACHE BOOL "" FORCE)
		set (WITH_ODBC 0 CACHE BOOL "" FORCE)
		set (WITH_MYSQL 0 CACHE BOOL "" FORCE)
		set (WITH_POSTGRESQL 0 CACHE BOOL "" FORCE)
		set (WITH_GALERA 0 CACHE BOOL "" FORCE)
		set (WITH_SSL 0 CACHE BOOL "" FORCE)
		SET (BUILD_TESTING 0 CACHE BOOL "" FORCE)
	elseif (NOT WIN32)
		if (CMAKE_DL_LIBS)
			target_link_libraries (lextra INTERFACE ${CMAKE_DL_LIBS})
		endif ()
		set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_DL_LIBS}")
		check_symbol_exists (dlopen "dlfcn.h" HAVE_DLOPEN)
		check_symbol_exists (dlerror "dlfcn.h" HAVE_DLERROR)
	endif ()
endif()

include (TestBigEndian)
test_big_endian (USE_BIG_ENDIAN)

check_include_file ("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file ("stdint.h" HAVE_STDINT_H)
check_include_file ("inttypes.h" HAVE_INTTYPES_H)
check_include_file ("malloc.h" HAVE_MALLOC_H)

target_compile_options (lextra INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:-D_CRT_SECURE_NO_WARNINGS -wd4996 -wd4706 -wd4244 -wd4702>)

# Looking for threads library
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)
target_link_libraries (lextra INTERFACE Threads::Threads)

include (CheckFunctionExists)
check_function_exists (strnlen HAVE_STRNLEN)

# Checking for atomic function
include (check_atomic_intrinsics)

set (DISABLE_MEMROUTINES OFF CACHE BOOL "If the build is broken because new/delete override, this option will disable the overriding")
mark_as_advanced (DISABLE_MEMROUTINES)

# Option USE_SYSLOG
check_include_file ("syslog.h" HAVE_SYSLOG_H)
set (USE_SYSLOG ${HAVE_SYSLOG_H} CACHE BOOL "compile with possibility to use syslog for logging")
bannervar (USE_SYSLOG)

# options for clang/gcc c and c++
target_compile_options (lextra INTERFACE $<${GNUC_CXX}:-D_FILE_OFFSET_BITS=64 -fno-strict-aliasing -Wall>)

# disable rtti
target_compile_options (lextra INTERFACE $<${GNUCXX}:-fno-rtti>$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/GR->)


if (COVERAGE_TEST)
	target_compile_options (lextra INTERFACE $<${GNUC_CXX}:-fprofile-arcs -ftest-coverage>)
	target_link_options (lextra INTERFACE
			$<$<LINK_LANG_AND_ID:CXX,GNU>:-lgcov --coverage>
			$<$<LINK_LANG_AND_ID:C,GNU>:-lgcov --coverage>
			$<$<LINK_LANG_AND_ID:CXX,Clang,AppleClang>: --coverage>
			$<$<LINK_LANG_AND_ID:C,Clang,AppleClang>: --coverage>
			)
endif (COVERAGE_TEST)

# options for clang in C++
target_compile_options (lextra INTERFACE $<${CLANGXX}:
		-Wno-deprecated-register -Wno-missing-exception-spec -Wno-implicit-exception-spec-mismatch -Wno-invalid-offsetof>)

# Checking for unaligned RAM access
include (check_unaligned_ram_access)

option (USE_SMALLALLOC "Use 'small allocator' derived from loki lib. Bench! Glibc allocator now is fast enough!")
bannervar (WITH_SMALLALLOC)
mark_as_advanced (WITH_SMALLALLOC)

#check galera, as it is not available on win, to fail early
if (WIN32)
	if (WITH_GALERA)
		message (FATAL_ERROR "Galera is not supported on win; reconfigure with WITH_GALERA=FALSE")
	endif ()
else ()
	include (GetGALERA)
	cache_galera_module_name() # write GALERA_SONAME as var to cache
	diag(WITH_GALERA)
	bannervar (WITH_GALERA)
endif ()

# Check for RE2 build
set (WITH_RE2_FORCE_STATIC 1 CACHE BOOL "force to compile re2 from sources")
with_get (re2 "RE2" "a regular expression library")

# Check for Libstemmer build
set (WITH_STEMMER_FORCE_STATIC 1 CACHE BOOL "force to compile stemmer from sources")
with_get (stemmer "Stemmer" "stemming library (Snowball)")

# Check for ICU build
set (WITH_ICU_FORCE_STATIC 1 CACHE BOOL "force to compile ICU from sources")
if (STATIC_BINARY)
	set (WITH_ICU_FORCE_STATIC ON CACHE BOOL "force to compile ICU from sources" FORCE)
	set (WITH_ICU_FORCE_BUILD ON)
endif ()
with_get (icu "ICU" "International Components for Unicode")

# Support for OpenSSL
with_menu_comp (OpenSSL SSL "OpenSSL" "for encrypted networking")
win_install_c(OpenSSL SSL)

trace (OpenSSL::Crypto)

# Support for Jemalloc
with_menu (Jemalloc "Jemalloc" "alternative memory allocator implementation")

# Support for ZLIB
with_menu (ZLIB "zlib" "for compressed data and networking")

# ODBC and it's dynamic linking
with_menu (ODBC "ODBC/UnixODBC/iODBC" "for indexing MSSQL (windows) and generic ODBC sources with indexer")
dl_package (ODBC "ODBC/UnixODBC/iODBC")

# EXPAT and it's dynamic linking
with_menu (EXPAT "expat" "for indexing xmlpipe sources with indexer")
dl_package (EXPAT "expat")
win_install (EXPAT)

# test for ICONV
if (WITH_EXPAT)
	with_menu (Iconv "iconv" "for support different encodings when indexing xmlpipe sources with indexer")
	if (Iconv_IS_BUILT_IN)
		set_target_properties(Iconv::Iconv PROPERTIES INTERFACE_COMPILE_DEFINITIONS LIBICONV_PLUG)
	else()
		dl_package (Iconv "iconv")
		win_install (Iconv)
	endif()
	include (check_const_iconv)
	diag(Iconv_IS_BUILT_IN)
endif ()

# MYSQL and it's dynamic linking
set (WITH_STATIC_MYSQL OFF CACHE BOOL "link to mysql library statically")
if (WITH_STATIC_MYSQL)
	set (MYSQL_USE_STATIC_LIBS ON)
endif()
with_menu (Mysql "mysql" "for indexing mysql sources with indexer")
dl_package (Mysql "mysql")
win_install (Mysql)

# PostgreSQL and it's dynamic linking
with_menu (PostgreSQL "pgsql" "for indexing postgresql sources with indexer")
dl_package (PostgreSQL "pgsql")
win_install (PostgreSQL)

# Storing compiler version
set (COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
diag (COMPILER)

# Storing system name
SET (OS_UNAME "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
diag (OS_UNAME)

diag(SPLIT)

# Common all-packages info
configure_file ("COPYING" "${MANTICORE_BINARY_DIR}/COPYING.txt" COPYONLY)
include (CommonInfo)

# select distribution build and install rules for most of the files
include (SetBuildType)

diag (installed)
if (NOT installed) # GNUInstallDirs wasn't yet included...
	include (GNUInstallDirs)
	set (CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore")
	GNUInstallDirs_get_absolute_install_dir (CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR)
endif ()

cmake_print_variables (CMAKE_INSTALL_PREFIX)
foreach (var BINDIR SBINDIR LIBEXECDIR SYSCONFDIR SHAREDSTATEDIR LOCALSTATEDIR RUNSTATEDIR LIBDIR INCLUDEDIR OLDINCLUDEDIR DATAROOTDIR DATADIR INFODIR LOCALEDIR MANDIR DOCDIR)
	cmake_print_variables (CMAKE_INSTALL_${var} CMAKE_INSTALL_FULL_${var})
endforeach ()

# a bit of default installation stuff, is in game if no distr build asked
if (NOT installed) # that is set in cmake/builds/build_*
	SET (LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/data") # will be used also in the app
	if (WIN32)
		set (CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc")
		GNUInstallDirs_get_absolute_install_dir (CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR)

		SET (FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}")
		install (DIRECTORY api doc contrib DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT applications)
		install (FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT applications)
		install (DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR} COMPONENT applications)
		install (FILES COPYING INSTALL manticore.conf.in DESTINATION . COMPONENT applications)
		install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log COMPONENT applications)
		install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/data COMPONENT applications)
		if (WITH_ICU)
			install_icudata (${CMAKE_INSTALL_DATADIR}/icu)
		endif ()
	else ()
		configure_config (lib/manticore)
		SET (FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore")
		install (FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT applications)
		install (DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT applications)
		install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore/data COMPONENT applications)
		install (DIRECTORY DESTINATION ${CMAKE_INSTALL_RUNSTATEDIR}/manticore COMPONENT applications)
		install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT applications)
		if (WITH_ICU)
			install_icudata (${CMAKE_INSTALL_DATADIR}/manticore/icu)
		endif ()
	endif ()
endif ()

set ( CPACK_SOURCE_IGNORE_FILES "/\\\\.idea/" "/\\\\.git/"
		"/\\\\.svn/" "/autom4te\\\\.cache/" "/bin/" "/junk/" "/win/" "/gmock/" "/gtest/" "/libtest"   "/pkgconfig/"
		".swp$" ".orig$" ".log$" ".deps/" )

set ( CPACK_SOURCE_GENERATOR TGZ )
set ( CPACK_RPM_COMPONENT_INSTALL 1 )
set ( CPACK_DEB_COMPONENT_INSTALL 1 )
set ( CPACK_ARCHIVE_COMPONENT_INSTALL 1 )

if ( SPLIT )
	set (CPACK_COMPONENTS_GROUPING IGNORE )
endif()

bannervar (LOCALDATADIR)
bannervar (FULL_SHARE_DIR)

set ( CPACK_PROJECT_CONFIG_FILE "${MANTICORE_BINARY_DIR}/config/CPackOptions.cmake" )

set (DEFAULT_CTEST_CONFIGURATION_TYPE "Debug")
include (CTest)

add_subdirectory ( src )

if ( BUILD_TESTING )
	add_subdirectory ( test )
	if (NOT TEST_SPECIAL_EXTERNAL)
		add_subdirectory ( api/libsphinxclient )
	endif()
endif ()

if (NOT CPack_CMake_INCLUDED)
	include ( CPack )
endif()
cpack_add_component (applications GROUP main)
cpack_add_component (runtime GROUP main)
cpack_add_component (tools GROUP main)
cpack_add_component (meta GROUP main)
cpack_add_component (devel GROUP main)
cpack_add_component (converter GROUP converter)
cpack_add_component_group(main EXTENDED)
cpack_add_component_group(converter)

#feature_summary (
#		INCLUDE_QUIET_PACKAGES
#		DESCRIPTION "Enabled Features1:"
#		VAR enabledFeaturesText
#		WHAT ALL)
#message (STATUS "${enabledFeaturesText}")

feature_summary (WHAT ENABLED_FEATURES INCLUDE_QUIET_PACKAGES DESCRIPTION "Enabled features compiled in:")
feature_summary (WHAT RUNTIME_PACKAGES_FOUND INCLUDE_QUIET_PACKAGES DESCRIPTION "Available runtime features:")

#feature_summary (WHAT ALL DESCRIPTION "Enabled ALL features4:")

