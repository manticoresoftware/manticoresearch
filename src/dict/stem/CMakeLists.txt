cmake_minimum_required ( VERSION 3.20 )

add_library ( stem sphinxstem.h sphinxstemru.inl sphinxmetaphone.cpp sphinxsoundex.cpp sphinxstemar.cpp
		sphinxstemcz.cpp sphinxstemen.cpp sphinxstemru.cpp)
target_include_directories ( stem PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( stem PRIVATE lextra )
