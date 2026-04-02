option ( WITH_UI "Build embedded web UI" OFF )

if ( WITH_UI )
	find_program ( NODE_EXECUTABLE node )
	find_program ( NPM_EXECUTABLE npm )
	find_program ( PYTHON_EXECUTABLE python3 )
	if ( NOT NODE_EXECUTABLE OR NOT NPM_EXECUTABLE )
		message ( WARNING "node/npm not found, disabling UI build" )
		set ( WITH_UI OFF )
	elseif ( NOT PYTHON_EXECUTABLE )
		message ( WARNING "python3 not found, disabling UI build" )
		set ( WITH_UI OFF )
	endif ()
endif ()

set ( UI_APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app" )
set ( UI_DIST_DIR "${UI_APP_DIR}/dist" )
set ( UI_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/ui_assets_generated.cpp" )

if ( WITH_UI )
	add_custom_command (
		OUTPUT "${UI_DIST_DIR}/index.html"
		COMMAND ${NPM_EXECUTABLE} install
		COMMAND ${NPM_EXECUTABLE} run build
		WORKING_DIRECTORY "${UI_APP_DIR}"
		COMMENT "Building Svelte UI"
	)

	add_custom_command (
		OUTPUT "${UI_GENERATED}"
		COMMAND ${PYTHON_EXECUTABLE}
			"${CMAKE_CURRENT_SOURCE_DIR}/embed_assets.py"
			"${UI_DIST_DIR}"
			"${UI_GENERATED}"
		DEPENDS "${UI_DIST_DIR}/index.html"
		COMMENT "Embedding UI assets into C++ source"
	)

	add_library ( lui OBJECT searchdui.cpp "${UI_GENERATED}" )
	target_compile_definitions ( lui PRIVATE WITH_UI=1 )
else ()
	add_library ( lui OBJECT searchdui.cpp )
	target_compile_definitions ( lui PRIVATE WITH_UI=0 )
endif ()

target_include_directories ( lui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" )
target_link_libraries ( lui PRIVATE lextra generated_config )
