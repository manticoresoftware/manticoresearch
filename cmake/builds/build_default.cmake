# ---------- default ----------
# Above line is mandatory!
# rules to build default zip archive
message ( STATUS "Will create default ZIP" )

set ( CPACK_GENERATOR "ZIP" )
set ( SPLIT_SYMBOLS 1 )

if (WIN32)
	set (CPACK_SUFFIX "x64")
endif ()