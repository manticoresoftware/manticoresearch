cmake_minimum_required (VERSION 2.8)

# main bunch of sources (for libsphinx)
set (LIBSPHINX_SRCS sphinx.cpp sphinxexcerpt.cpp
		sphinxquery.cpp sphinxsoundex.cpp sphinxmetaphone.cpp
		sphinxstemen.cpp sphinxstemru.cpp sphinxstemcz.cpp
		sphinxstemar.cpp sphinxutils.cpp sphinxstd.cpp
		sphinxsort.cpp sphinxexpr.cpp sphinxfilter.cpp
		sphinxsearch.cpp sphinxrt.cpp sphinxjson.cpp
		sphinxaot.cpp sphinxplugin.cpp
		sphinxudf.c)
set (INDEXER_SRCS indexer.cpp)
set (INDEXTOOL_SRCS indextool.cpp)
set (SEARCHD_SRCS searchd.cpp)
set (SPELLDUMP_SRCS spelldump.cpp)
set (TESTS_SRCS tests.cpp)

# The headers is not neccessary for executable build, but nesessary for MSVC and other projects
# all the (non-generated) headers
file (GLOB HEADERS "sphinx*.h" lz4.h lz4hc.h http/http_parser.h)

file (GLOB GHEADERS "yysphinx*.h")
source_group ("Generated Header Files" FILES ${GHEADERS})

add_custom_target (
		generate_sphinxversion
		COMMAND
		${CMAKE_COMMAND}
		-D SOURCE_DIR=${CMAKE_SOURCE_DIR}
		-D BINARY_DIR=${CMAKE_BINARY_DIR}
		-D SPHINX_TAG="${SPHINX_TAG}"
		-P ${CMAKE_SOURCE_DIR}/sphinxrev.cmake
)
include_directories ("${SPHINXSEARCH_BINARY_DIR}/config")
add_definitions ("-DBUILD_WITH_CMAKE")

# our mega-lib
add_library (libsphinx STATIC ${LIBSPHINX_SRCS} ${HEADERS} ${GHEADERS})

add_dependencies (libsphinx generate_sphinxversion)

#message (STATUS "Extra libraries:${EXTRA_LIBRARIES}")

# our executables
add_executable (indexer ${INDEXER_SRCS})
add_executable (indextool ${INDEXTOOL_SRCS})
add_executable (searchd ${SEARCHD_SRCS})
add_executable (spelldump ${SPELLDUMP_SRCS})
add_executable (tests ${TESTS_SRCS})
target_link_libraries (indexer libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (indextool libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (searchd libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (spelldump libsphinx ${EXTRA_LIBRARIES})
target_link_libraries (tests libsphinx ${EXTRA_LIBRARIES})

# add the extra targets in the case we want on-the-fly grammar compiler
if (USE_BISON)

	set (BISON_DIR "${SPHINXSEARCH_BINARY_DIR}/config")

	BISON_TARGET (ExprParser sphinxexpr.y ${BISON_DIR}/bissphinxexpr.c)
	BISON_TARGET (SelectParser sphinxselect.y ${BISON_DIR}/bissphinxselect.c)
	BISON_TARGET (QueryParser sphinxquery.y ${BISON_DIR}/bissphinxquery.c)
	BISON_TARGET (SQLParser sphinxql.y ${BISON_DIR}/bissphinxql.c)
	BISON_TARGET (JsonParser sphinxjson.y ${BISON_DIR}/bissphinxjson.c)

	set_property (SOURCE sphinxexpr.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_ExprParser_OUTPUT_SOURCE})
	set_property (SOURCE sphinx.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_SelectParser_OUTPUT_SOURCE})
	set_property (SOURCE sphinxquery.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_QueryParser_OUTPUT_SOURCE})
	set_property (SOURCE searchd.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_SQLParser_OUTPUT_SOURCE})
	set_property (SOURCE sphinxjson.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_JsonParser_OUTPUT_SOURCE})

endif (USE_BISON)

# add the extra targets in the case we want on-the-fly lex compiler
if (USE_FLEX)
	set (FLEX_DIR "${SPHINXSEARCH_BINARY_DIR}/config")

	FLEX_TARGET (SQLlex sphinxql.l ${FLEX_DIR}/flexsphinxql.c)
	FLEX_TARGET (Jsonlex sphinxjson.l ${FLEX_DIR}/flexsphinxjson.c COMPILE_FLAGS "-Pyy2")

	set_property (SOURCE searchd.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${FLEX_SQLlex_OUTPUTS})
	set_property (SOURCE sphinxjson.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${FLEX_Jsonlex_OUTPUTS})

endif (USE_FLEX)


if (SPLIT_SYMBOLS)
	include(helpers)
	install_dbg (indexer)
	install_dbg (indextool)
	install_dbg (searchd)
	install_dbg (spelldump)
	install_dbg (tests)
endif (SPLIT_SYMBOLS)

INSTALL (TARGETS indexer indextool searchd spelldump RUNTIME DESTINATION bin COMPONENT APPLICATIONS)

# testrt is win-only project
if (WIN32)
	add_executable (testrt testrt.cpp)
	target_link_libraries (testrt libsphinx ${EXTRA_LIBRARIES})
endif (WIN32)

