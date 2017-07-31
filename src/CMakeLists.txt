cmake_minimum_required ( VERSION 2.8 )

# main bunch of sources (for libsphinx)
set ( LIBSPHINX_SRCS sphinx.cpp sphinxexcerpt.cpp
		sphinxquery.cpp sphinxsoundex.cpp sphinxmetaphone.cpp
		sphinxstemen.cpp sphinxstemru.cpp sphinxstemcz.cpp
		sphinxstemar.cpp sphinxutils.cpp sphinxstd.cpp
		sphinxsort.cpp sphinxexpr.cpp sphinxfilter.cpp
		sphinxsearch.cpp sphinxrt.cpp sphinxjson.cpp
		sphinxaot.cpp sphinxplugin.cpp sphinxudf.c
		sphinxqcache.cpp sphinxrlp.cpp json/cJSON.c )
set ( INDEXER_SRCS indexer.cpp )
set ( INDEXTOOL_SRCS indextool.cpp )
set ( SEARCHD_SRCS searchd.cpp searchdha.cpp http/http_parser.c searchdhttp.cpp )
set ( SPELLDUMP_SRCS spelldump.cpp )
set ( TESTS_SRCS tests.cpp )
set ( WORDBREAKER_SRCS sphinxstd.cpp wordbreaker.cpp)
set ( LIBSPHINX_BISON sphinxexpr.y sphinxselect.y sphinxquery.y sphinxjson.y )
set ( LIBSPHINX_FLEX sphinxjson.l )
set ( SEARCHD_BISON sphinxql.y )
set ( SEARCHD_FLEX sphinxql.l )

# The headers is not neccessary for executable build, but nesessary for MSVC and other projects
# all the (non-generated) headers
file ( GLOB HEADERS "sphinx*.h" lz4.h lz4hc.h http/http_parser.h )
file ( GLOB SEARCHD_H "searchd*.h" )

file ( GLOB GHEADERS "yysphinx*.h" )
source_group ( "Generated Header Files" FILES ${GHEADERS} )
source_group ( "Grammar sources" FILES ${LIBSPHINX_BISON} ${SEARCHD_BISON} )
source_group ( "Lexer sources" FILES ${LIBSPHINX_FLEX} ${SEARCHD_FLEX} )

add_custom_target (
		vergen
		COMMAND
		${CMAKE_COMMAND}
			-D SOURCE_DIR=${CMAKE_SOURCE_DIR}
			-D BINARY_DIR=${CMAKE_BINARY_DIR}
			-D SPHINX_TAG=${SPHINX_TAG}
			-D CPACK_SUFFIX=${SPHINX_PACKAGE_SUFFIX}
			-D DISTR_SUFFIX=${DISTR_SUFFIX}
			-D PACKAGE_NAME=${PACKAGE_NAME}
			-P ${CMAKE_SOURCE_DIR}/sphinxrev.cmake
		COMMENT "Generate version from sources"
		VERBATIM
		SOURCES ${CMAKE_SOURCE_DIR}/sphinxrev.cmake
			${CMAKE_SOURCE_DIR}/src/sphinxversion.h.in
			${CMAKE_SOURCE_DIR}/CPackOptions.cmake.in
)
include_directories ( "${MANTICORE_BINARY_DIR}/config" )
add_definitions ( "-DBUILD_WITH_CMAKE" )

#message (STATUS "Extra libraries:${EXTRA_LIBRARIES}")

# add the extra targets in the case we want on-the-fly grammar compiler
if ( USE_BISON )

	set ( BISON_DIR "${MANTICORE_BINARY_DIR}/config" )

	BISON_TARGET ( ExprParser "${CMAKE_CURRENT_SOURCE_DIR}/sphinxexpr.y" ${BISON_DIR}/bissphinxexpr.c )
	BISON_TARGET ( SelectParser "${CMAKE_CURRENT_SOURCE_DIR}/sphinxselect.y" ${BISON_DIR}/bissphinxselect.c )
	BISON_TARGET ( QueryParser "${CMAKE_CURRENT_SOURCE_DIR}/sphinxquery.y" ${BISON_DIR}/bissphinxquery.c )
	BISON_TARGET ( SQLParser "${CMAKE_CURRENT_SOURCE_DIR}/sphinxql.y" ${BISON_DIR}/bissphinxql.c )
	BISON_TARGET ( JsonParser "${CMAKE_CURRENT_SOURCE_DIR}/sphinxjson.y" ${BISON_DIR}/bissphinxjson.c )

	set_property ( SOURCE sphinxexpr.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_ExprParser_OUTPUT_SOURCE} )
	set_property ( SOURCE sphinx.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_SelectParser_OUTPUT_SOURCE} )
	set_property ( SOURCE sphinxquery.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_QueryParser_OUTPUT_SOURCE} )
	set_property ( SOURCE searchd.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_SQLParser_OUTPUT_SOURCE} )
	set_property ( SOURCE sphinxjson.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${BISON_JsonParser_OUTPUT_SOURCE} )

endif ( USE_BISON )

# add the extra targets in the case we want on-the-fly lex compiler
if ( USE_FLEX )
	set ( FLEX_DIR "${MANTICORE_BINARY_DIR}/config" )

	FLEX_TARGET ( SQLlex "${CMAKE_CURRENT_SOURCE_DIR}/sphinxql.l" ${FLEX_DIR}/flexsphinxql.c COMPILE_FLAGS "-i" )
	FLEX_TARGET ( Jsonlex "${CMAKE_CURRENT_SOURCE_DIR}/sphinxjson.l" ${FLEX_DIR}/flexsphinxjson.c COMPILE_FLAGS "-i -Pyy2" )

	set_property ( SOURCE searchd.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${FLEX_SQLlex_OUTPUTS} )
	set_property ( SOURCE sphinxjson.cpp APPEND PROPERTY
			OBJECT_DEPENDS ${FLEX_Jsonlex_OUTPUTS} )

endif ( USE_FLEX )

# our mega-lib
add_library ( libsphinx STATIC ${LIBSPHINX_SRCS} ${HEADERS} ${GHEADERS} ${LIBSPHINX_BISON} ${LIBSPHINX_FLEX} )

add_dependencies ( libsphinx vergen )

# our executables
add_executable ( indexer ${INDEXER_SRCS} )
add_executable ( indextool ${INDEXTOOL_SRCS} )
add_executable ( searchd ${SEARCHD_SRCS} ${SEARCHD_H} ${SEARCHD_BISON} ${SEARCHD_FLEX} )
add_executable ( spelldump ${SPELLDUMP_SRCS} )
add_executable ( wordbreaker ${WORDBREAKER_SRCS} )
add_dependencies ( wordbreaker vergen )
target_link_libraries ( indexer libsphinx ${EXTRA_LIBRARIES} )
target_link_libraries ( indextool libsphinx ${EXTRA_LIBRARIES} )
target_link_libraries ( searchd libsphinx ${EXTRA_LIBRARIES} )
target_link_libraries ( spelldump libsphinx ${EXTRA_LIBRARIES} )
target_link_libraries ( wordbreaker ${EXTRA_LIBRARIES} )

if ( SPLIT_SYMBOLS )
	include ( helpers )
	install_dbg ( indexer )
	install_dbg ( indextool )
	install_dbg ( searchd )
	install_dbg ( spelldump )
	install_dbg ( wordbreaker )
	INSTALL ( PROGRAMS $<TARGET_FILE:indexer>
			$<TARGET_FILE:indextool>
			$<TARGET_FILE:searchd>
			$<TARGET_FILE:spelldump>
			$<TARGET_FILE:wordbreaker>
			DESTINATION bin COMPONENT APPLICATIONS )
else()
	INSTALL ( TARGETS indexer indextool searchd spelldump wordbreaker RUNTIME DESTINATION bin COMPONENT APPLICATIONS )
endif ( SPLIT_SYMBOLS )

if ( BUILD_TESTING )
	add_executable ( tests ${TESTS_SRCS} )
	target_link_libraries ( tests libsphinx ${EXTRA_LIBRARIES} )
	if ( SPLIT_SYMBOLS )
		install_dbg ( tests )
	endif()
    # testrt is win-only project
	if ( WIN32 )
		add_executable ( testrt testrt.cpp )
		target_link_libraries ( testrt libsphinx ${EXTRA_LIBRARIES} )
		add_test ( NAME "\"Internal rt src/tests\""
			#	    	CONFIGURATIONS Debug
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND "testrt" )
	endif ( WIN32 )
	add_test ( NAME "\"Internal src/tests\""
			#    	CONFIGURATIONS Debug
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			COMMAND "tests" )
endif()
