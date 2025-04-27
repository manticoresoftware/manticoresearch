cmake_minimum_required ( VERSION 3.20 )

add_library ( daemon OBJECT
	winservice.cpp
	winservice.h)
target_include_directories ( daemon PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( daemon PRIVATE lextra )
