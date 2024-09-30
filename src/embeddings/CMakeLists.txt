cmake_minimum_required ( VERSION 3.20 )

add_library ( embeddings embeddings.cpp embeddings.h )
target_include_directories ( embeddings PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( embeddings PRIVATE lextra )
