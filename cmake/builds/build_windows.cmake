# ---------- windows ----------
# Above line is mandatory!
# rules to build default zip archive for windows
message ( STATUS "Will create default ZIP" )

if (NOT SUGGEST_GENERATOR)
	set ( CMAKE_BUILD_TYPE "RelWithDebInfo" )
endif ()

set ( CPACK_GENERATOR "ZIP" )
set ( SPLIT_SYMBOLS 1 )
set ( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-x64" )
