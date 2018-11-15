include ( "${BINARY_DIR}/config/CPackOptions.cmake" )

macro( packbis src )
	if ( USE_BISON )
		configure_file ( "${BINARY_DIR}/config/bis${src}.c"
				"${PACKDIR}/src/yy${src}.c" COPYONLY )
		configure_file ( "${BINARY_DIR}/config/bis${src}.h"
				"${PACKDIR}/src/yy${src}.h" COPYONLY )
	endif ()
endmacro()

macro( packflex src )
	if ( USE_FLEX )
		configure_file ( "${BINARY_DIR}/config/flex${src}.c"
				"${PACKDIR}/src/ll${src}.c" COPYONLY )
	endif ()
endmacro()

if ( WITH_STEMMER )
	set (SUFFIX -stemmer)
endif()

set ( FNAME "${TARBALL_FILE_NAME}${SUFFIX}" )
set ( PACKDIR "${BINARY_DIR}/sources/${FNAME}" )
set ( SOURCE_DIR_FILES "${BINARY_DIR}/sources/sources" )

file ( REMOVE_RECURSE "${BINARY_DIR}/sources" )


# direct copy of source_dir fails if source_dir is symlink (in the case the symlink get copied and then
# packed). So, let us first copy the directory with external command to ensure all symlinks are
# resolved.

execute_process ( COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SOURCE_DIR}" "${SOURCE_DIR_FILES}" )

file (COPY ${SOURCE_DIR_FILES}/ DESTINATION ${PACKDIR}
		PATTERN "cmake-*" EXCLUDE
		PATTERN "build-*" EXCLUDE
		PATTERN "build" EXCLUDE
		PATTERN ".*" EXCLUDE
		PATTERN "CMakeCache.txt" EXCLUDE
		)
file ( REMOVE_RECURSE ${SOURCE_DIR_FILES} ) # copied dir no more necessary.

configure_file ("${BINARY_DIR}/config/gen_sphinxversion.h"
		"${PACKDIR}/src/sphinxversion.h" COPYONLY )

if ( WITH_STEMMER AND STEMMER_BASEDIR )
	message ( STATUS "Embedd stemmer sources... ${STEMMER_BASEDIR}" )
	file ( COPY ${STEMMER_BASEDIR}/ DESTINATION ${PACKDIR}/libstemmer_c
			PATTERN "CMakeFiles" EXCLUDE
			PATTERN "CTestTestfile.cmake" EXCLUDE
			PATTERN "cmake_install.cmake" EXCLUDE
			PATTERN "CMakeLists.txt" EXCLUDE
			)
endif()

if ( WITH_RE2 AND RE2_BASEDIR )
	message ( STATUS "Embedd RE2 sources... ${RE2_BASEDIR}" )
	file ( COPY ${RE2_BASEDIR}/ DESTINATION ${PACKDIR}/libre2
			PATTERN "CMakeFiles" EXCLUDE
			PATTERN "CTestTestfile.cmake" EXCLUDE
			PATTERN "cmake_install.cmake" EXCLUDE
			PATTERN "CMakeLists.txt" EXCLUDE
			PATTERN "*.a" EXCLUDE
			)
endif ()


packbis ( sphinxexpr )
packbis ( sphinxjson )
packflex ( sphinxjson )
packbis ( sphinxql )
packflex ( sphinxql )
packbis ( sphinxquery )
packbis ( sphinxselect )

execute_process (
		COMMAND "${CMAKE_COMMAND}" -E tar cfz "${BINARY_DIR}/${FNAME}.tar.gz" "${FNAME}"
		WORKING_DIRECTORY "${BINARY_DIR}/sources" )
file ( MD5 "${BINARY_DIR}/${FNAME}.tar.gz" MD5TARBALL )
message ( "${MD5TARBALL}  ${FNAME}.tar.gz")
