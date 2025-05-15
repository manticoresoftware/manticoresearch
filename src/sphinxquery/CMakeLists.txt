cmake_minimum_required ( VERSION 3.20 )

add_library ( sphinxquery OBJECT
		sphinxquery.cpp
		sphinxquery.h
)

include ( mybison )
MY_BISON ( QueryParser sphinxquery sphinxquery.cpp SPHINXQUERY )
source_group ( "Grammar sources" FILES ${SPHINXQUERY_BISON} )
target_include_directories ( sphinxquery PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( sphinxquery PRIVATE lextra )
