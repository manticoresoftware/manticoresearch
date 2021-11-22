cmake_minimum_required ( VERSION 3.20 )

add_library ( schema columninfo.cpp helper.cpp schema.cpp locator.h locator_impl.h columninfo.h ischema.h helper.h
		schema.h rset.cpp rset.h )
target_include_directories ( schema PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( schema PRIVATE lextra )
