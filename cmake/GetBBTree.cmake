# Opt-in only: ordinary builds do not need Cargo or fetch Rust dependencies.
option ( WITH_BBTREE_DOCID "Build the experimental bounded bbtree DocID cache" OFF )
if (NOT WITH_BBTREE_DOCID)
	return ()
endif ()

if (CMAKE_CROSSCOMPILING OR WIN32)
	message ( FATAL_ERROR "WITH_BBTREE_DOCID currently supports native Unix builds only" )
endif ()
find_program ( BBTREE_CARGO cargo REQUIRED )
set ( BBTREE_MANIFEST "${MANTICORE_SOURCE_DIR}/src/bbtree/Cargo.toml" )
set ( BBTREE_TARGET_DIR "${MANTICORE_BINARY_DIR}/bbtree-cargo" )
set ( BBTREE_LIBRARY "${BBTREE_TARGET_DIR}/release/libmanticore_bbtree.a" )
add_custom_command (
	OUTPUT "${BBTREE_LIBRARY}"
	COMMAND "${CMAKE_COMMAND}" -E env "CARGO_TARGET_DIR=${BBTREE_TARGET_DIR}"
		"${BBTREE_CARGO}" build --release --locked --manifest-path "${BBTREE_MANIFEST}"
	DEPENDS "${BBTREE_MANIFEST}" "${MANTICORE_SOURCE_DIR}/src/bbtree/Cargo.lock"
		"${MANTICORE_SOURCE_DIR}/src/bbtree/src/lib.rs"
	COMMENT "Building the pinned bbtree DocID cache"
	VERBATIM )
add_custom_target ( bbtree_docid_build DEPENDS "${BBTREE_LIBRARY}" )
add_library ( bbtree_docid STATIC IMPORTED GLOBAL )
set_target_properties ( bbtree_docid PROPERTIES IMPORTED_LOCATION "${BBTREE_LIBRARY}" )
add_dependencies ( bbtree_docid bbtree_docid_build )
find_package ( Threads REQUIRED )
target_link_libraries ( bbtree_docid INTERFACE Threads::Threads ${CMAKE_DL_LIBS} m )
target_link_libraries ( lextra INTERFACE bbtree_docid )
target_compile_definitions ( lextra INTERFACE WITH_BBTREE_DOCID=1 )
install_component_and_bundle ( common FILES "${MANTICORE_SOURCE_DIR}/src/bbtree/LICENSE-bbtree"
	DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore/licenses" )

if (BUILD_TESTING)
	add_test ( NAME bbtree_docid_rust
		COMMAND "${CMAKE_COMMAND}" -E env "CARGO_TARGET_DIR=${BBTREE_TARGET_DIR}"
			"${BBTREE_CARGO}" test --release --locked --manifest-path "${BBTREE_MANIFEST}" )
endif ()
