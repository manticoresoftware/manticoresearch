# ---------- windows ----------
# Above line is mandatory!
# rules to build default zip archive for windows
message ( STATUS "Will create default ZIP" )

set ( CPACK_GENERATOR "ZIP" )
set ( SPLIT_SYMBOLS 1 )
set ( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-x64" )
