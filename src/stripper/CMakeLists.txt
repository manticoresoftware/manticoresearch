cmake_minimum_required ( VERSION 3.20 )

FOREACH (policy CMP0054 CMP0057 CMP0042 CMP0074 CMP0110)
	IF (POLICY ${policy})
		CMAKE_POLICY ( SET ${policy} NEW )
	ENDIF ()
ENDFOREACH ()

message ( STATUS "Inside directory src/stripper..." )

add_library ( stripper html_stripper.cpp html_stripper.h html_stripper_tag.h )
target_include_directories ( stripper PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( stripper PRIVATE lextra )