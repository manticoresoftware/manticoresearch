cmake_minimum_required ( VERSION 3.20 )

add_library ( sphinxquery OBJECT
		sphinxquery.cpp
		sphinxquery.h
		parse_helper.cpp
		parse_helper.h
		xqparser.cpp
		xqparser.h
		xqnode.cpp
		xqdebug.h
		common_subtree.cpp
		transformation.cpp
		transformation.h
		transform_commonnot.cpp
		transform_commonkeywords.cpp
		transform_commoncompoundnot.cpp
		transform_commonsubterm.cpp
		transform_commonandnotfactor.cpp
		transform_commonornot.cpp
		transform_hungoperand.cpp
		transform_excessandnot.cpp
		transform_andnotnotnot.cpp
)

include ( mybison )
MY_BISON_ALLOWING_UNITY ( QueryParser sphinxquery xqparser.cpp SPHINXQUERY )
source_group ( "Grammar sources" FILES ${SPHINXQUERY_BISON} )
target_include_directories ( sphinxquery PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( sphinxquery PRIVATE lextra )
