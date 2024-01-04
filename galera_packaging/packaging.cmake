if (__galera_cmake_packaging_included)
	return ()
endif ()
set ( __galera_cmake_packaging_included YES )

if ( NOT PACK )
	return ( )
endif()

set ( DISTR "$ENV{DISTR}" )

# common metadata
include ( "${GALERA_CMAKE_DIR}/metainfo.cmake" )

set ( DISTR_SRC "${GALERA_CMAKE_DIR}/build_${DISTR}.cmake" )
if (EXISTS "${DISTR_SRC}")
	include ( "${DISTR_SRC}" )
else ()
	message ( FATAL_ERROR "Can't find ${DISTR_SRC}, can't continue." )
endif ()

include ( CPack )