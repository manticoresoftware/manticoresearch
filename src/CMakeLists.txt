cmake_minimum_required (VERSION 2.8)

include(CheckFunctionExists)

if (NOT WIN32)
	include (check_veryold_gcc)
	include (SphEndianess)
endif()

# Fill system details for future
if (CMAKE_COMPILER_IS_GNUCXX)
	execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-dumpversion" OUTPUT_VARIABLE gcc_ver)
	remove_crlf(COMPILER ${gcc_ver})
endif (CMAKE_COMPILER_IS_GNUCXX)
if (UNIX AND NOT CYGWIN)
	execute_process(COMMAND "uname" "-a" OUTPUT_VARIABLE OS_UNAME)
	remove_crlf(OS_UNAME ${OS_UNAME})
	set(CONFIGURE_FLAGS "none, used cmake")
endif (UNIX AND NOT CYGWIN)

if (NOT WIN32)
	find_package(Threads REQUIRED)
	SET (EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
	message(STATUS "Checking for header files")

include(ac_header_stdc)
ac_check_headers ("execinfo.h")
include(FindEXPAT)
check_include_file("iconv.h" have_iconv_h) #fixme! Move to module?
include(FindZLIB)
check_include_file("sql.h" have_sql_h) #fixme! Move to module?
check_include_file("syslog.h" have_syslog_h) #fixme! Move to module?

message(STATUS "Checking for types")
message(STATUS "Checking for library functions")
ac_check_funcs("strnlen;pread;poll;backtrace;backtrace_symbols")

ac_search_libs("socket" "setsockopt" _DUMMY EXTRA_LIBRARIES)
ac_search_libs("nsl;socket;resolv" "gethostbyname" _DUMMY EXTRA_LIBRARIES)
ac_search_libs("m" "logf" HAVE_LOGF EXTRA_LIBRARIES)
ac_search_libs("z" "inflate" _DUMMY EXTRA_LIBRARIES)
#check_function_exists(


if ( CMAKE_COMPILER_IS_GNUCXX )
	set (SPHINX_INJECT_C_FLAGS "-D_FILE_OFFSET_BITS=64 -fno-strict-aliasing" )
	if ( NOT WITH_DEBUG_MODE )
		set (SPHINX_INJECT_C_FLAGS "${SPHINX_INJECT_C_FLAGS} -O3 -DNDEBUG")
	endif(NOT WITH_DEBUG_MODE)
	set (SPHINX_INJECT_CXX_FLAGS "-fno-rtti -Wall -g ${SPHINX_INJECT_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${SPHINX_INJECT_CXX_FLAGS}")
	set(CMAKE_C_FLAGS "${SPHINX_INJECT_C_FLAGS}")
endif ( CMAKE_COMPILER_IS_GNUCXX )

# make our configure file
configure_file("${SPHINXSEARCH_SOURCE_DIR}/config/config_cmake.h.in"
	"${SPHINXSEARCH_BINARY_DIR}/config/config.h")
add_definitions (-DHAVE_CONFIG_H)
include_directories("${SPHINXSEARCH_BINARY_DIR}/config")

endif (NOT WIN32)

# main bunch of sources
set (SRCS sphinx.cpp sphinxexcerpt.cpp
sphinxquery.cpp sphinxsoundex.cpp sphinxmetaphone.cpp sphinxstemen.cpp
sphinxstemru.cpp sphinxstemcz.cpp sphinxstemar.cpp sphinxutils.cpp
sphinxstd.cpp sphinxsort.cpp sphinxexpr.cpp sphinxfilter.cpp
sphinxsearch.cpp sphinxrt.cpp sphinxjson.cpp sphinxudf.c sphinxaot.cpp
sphinxplugin.cpp)

# all the (non-generated) headers
file(GLOB HEADERS "sphinx*.h")

file(GLOB GHEADERS "yysphinx*.h")
source_group ("Generated Header Files" FILES ${GHEADERS})

if (WIN32)
	add_custom_command(OUTPUT "sphinxversion.h" COMMAND ${SPHINXSEARCH_SOURCE_DIR}/src/svnxrev.cmd ${SPHINXSEARCH_SOURCE_DIR})
else (WIN32)
	add_custom_command(OUTPUT "sphinxversion.h" COMMAND /bin/sh ${SPHINXSEARCH_SOURCE_DIR}/src/svnxrev.sh ${SPHINXSEARCH_SOURCE_DIR})
endif(WIN32)

set (HEADERS ${HEADERS} sphinxversion.h)

# our mega-lib
add_library ( libsphinx STATIC ${SRCS} ${HEADERS} ${GHEADERS})

message (STATUS "Extra libraries:${EXTRA_LIBRARIES}")


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

INSTALL(TARGETS indexer indextool searchd spelldump RUNTIME DESTINATION usr/bin)

# testrt is win-only project
if (WIN32)
	add_executable (testrt testrt.cpp )
	target_link_libraries (testrt libsphinx ${EXTRA_LIBRARIES})
endif (WIN32)

