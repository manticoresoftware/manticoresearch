cmake_minimum_required ( VERSION 3.20 )

# Provide valid perfetto library despite presence of the sources
if (EXISTS perfetto.cc AND NOT CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
	# Define a static library for Perfetto.
	add_library ( perfetto STATIC perfetto.cc )
	target_include_directories ( perfetto INTERFACE "${MANTICORE_CURRENT_SOURCE_DIR}" )
	target_compile_definitions ( perfetto PUBLIC PERFETTO )

	if (WIN32)
		target_compile_definitions ( perfetto PRIVATE NOMINMAX=1 WIN32_LEAN_AND_MEAN=1 PUBLIC _WIN32 )
		target_compile_options ( perfetto PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj> )
		target_link_libraries ( perfetto PRIVATE WS2_32 )
	endif ()
else()
	# Define an empty (stub) perfetto.
	add_library ( perfetto INTERFACE )
endif()