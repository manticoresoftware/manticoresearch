cmake_minimum_required ( VERSION 3.20 )

add_library ( sqlchecks STATIC
		checks.h
		scan_parameters.cpp
		validate_vector.cpp
)

include ( mybison )
MY_FLEX ( SQLChecksLex scanparams scan_parameters.cpp SEARCHD )
MY_FLEX ( SQLValidateVecLex validatevec validate_vector.cpp SEARCHD )
set_source_files_properties ( validate_vector.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON )

target_include_directories ( sqlchecks PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( sqlchecks PRIVATE lextra )
