cmake_minimum_required (VERSION 2.8)

cmake_policy(SET CMP0003 NEW)

#include(TestBigEndian)
include(CheckFunctionExists)

if (NOT WIN32)
#TEST_BIG_ENDIAN (BIG_ENDIAN)
#if(BIG_ENDIAN)
#	add_definitions(-DUSE_LITTLE_ENDIAN=no)
#else(BIG_ENDIAN)
	add_definitions(-DUSE_LITTLE_ENDIAN=yes)
#endif(BIG_ENDIAN)
else()
	add_definitions(-D_WIN32)
endif()

# Fill system details for future
execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-dumpversion" OUTPUT_VARIABLE gcc_ver)
string(REGEX REPLACE "\n" "" COMPILER "${CMAKE_CXX_COMPILER} ${gcc_ver}")
execute_process(COMMAND "uname" "-a" OUTPUT_VARIABLE OS_UNAME)
string(REGEX REPLACE "\n" "" OS_UNAME "${OS_UNAME}")
set(CONFIGURE_FLAGS "none, used cmake")

if (NOT WIN32)
message(STATUS "Checking for header files")

ac_header_stdc()
ac_check_headers ("fcntl.h;limits.h;netdb.h;netinet/in.h;stdlib.h;string.h;sys/file.h;sys/socket.h;sys/time.h;unistd.h;pthread.h;execinfo.h;sys/epoll.h")
include(FindEXPAT)
check_include_file("iconv.h" have_iconv_h) #fixme! Move to module?
include(FindZLIB)
check_include_file("sql.h" have_sql_h) #fixme! Move to module?
check_include_file("syslog.h" have_syslog_h) #fixme! Move to module?

message(STATUS "Checking for types")
message(STATUS "Checking for library functions")
ac_check_funcs("dup2;gethostbyname;gettimeofday;memmove;memset;select;socket;strcasecmp;strchr;strerror;strncasecmp;strnlen;strstr;strtol;logf;pread;poll;backtrace;backtrace_symbols")

ac_search_libs("m" "logf" HAVE_LOGF)
ac_search_libs("z" "inflate" HAVE_GETHOSTBYNAME)
#check_function_exists(

set (EXTRA_LIBRARIES
	stdc++
	pthread
	)


if ( CMAKE_COMPILER_IS_GNUCXX )
	set (SPHINX_INJECT_C_FLAGS "-D_FILE_OFFSET_BITS=64 -fno-strict-aliasing" )
	if ( NOT WITH_DEBUG_MODE )
		set (SPHINX_INJECT_C_FLAGS "${SPHINX_INJECT_C_FLAGS} -O3 -DNDEBUG")
	endif(NOT WITH_DEBUG_MODE)
	set (SPHINX_INJECT_CXX_FLAGS "-fno-rtti -Wall -g ${SPHINX_INJECT_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${SPHINX_INJECT_CXX_FLAGS}")
	set(CMAKE_C_FLAGS "${SPHINX_INJECT_C_FLAGS}")
endif ( CMAKE_COMPILER_IS_GNUCXX )
configure_file("${SPHINXSEARCH_SOURCE_DIR}/config/config_cmake.h.in"
	"${SPHINXSEARCH_BINARY_DIR}/config/config.h")
add_definitions (-DHAVE_CONFIG_H)
include_directories("${SPHINXSEARCH_BINARY_DIR}/config")
endif (NOT WIN32)

# our mega-lib
add_library ( libsphinx STATIC sphinx.cpp sphinxexcerpt.cpp
sphinxquery.cpp sphinxsoundex.cpp sphinxmetaphone.cpp sphinxstemen.cpp
sphinxstemru.cpp sphinxstemcz.cpp sphinxstemar.cpp sphinxutils.cpp
sphinxstd.cpp sphinxsort.cpp sphinxexpr.cpp sphinxfilter.cpp
sphinxsearch.cpp sphinxrt.cpp sphinxjson.cpp sphinxudf.c sphinxaot.cpp
sphinxplugin.cpp )

# our executables
add_executable (indexer indexer.cpp )
add_executable (indextool indextool.cpp )
add_executable (searchd searchd.cpp )
add_executable (spelldump spelldump.cpp )
add_executable (tests tests.cpp )
target_link_libraries (indexer libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (indextool libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (searchd libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (spelldump libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (tests libsphinx ${EXTRA_LIBRARIES})

# testrt is win-only project
if (WIN32)
	add_executable (testrt testrt.cpp )
	target_link_libraries (testrt libsphinx ${EXTRA_LIBRARIES})
endif (WIN32)