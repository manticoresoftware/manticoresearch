include ( "${BINARY_DIR}/config/CPackOptions.cmake" )

set ( FNAME "${TARBALL_FILE_NAME}" )
set ( PACKDIR "${BINARY_DIR}/sources/${FNAME}" )

file (COPY ${SOURCE_DIR}/ DESTINATION ${PACKDIR}
		PATTERN "cmake-*" EXCLUDE
		PATTERN ".*" EXCLUDE
		)
configure_file ("${BINARY_DIR}/config/gen_sphinxversion.h"
		"${PACKDIR}/src/sphinxversion.h" COPYONLY )

execute_process (
		COMMAND "${CMAKE_COMMAND}" -E tar cfz "${BINARY_DIR}/${FNAME}.tar.gz" "${FNAME}"
		WORKING_DIRECTORY "${BINARY_DIR}/sources" )
