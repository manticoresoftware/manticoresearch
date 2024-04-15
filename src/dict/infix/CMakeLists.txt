cmake_minimum_required ( VERSION 3.20 )

add_library ( infix infix_builder.cpp infix_builder.h )
target_include_directories ( infix PRIVATE "${MANTICORE_SOURCE_DIR}/src" )

target_link_libraries ( infix PRIVATE lextra )
