# embed_assets.cmake
# Converts all files in DIST_DIR into C++ byte arrays for embedding.
# Usage: cmake -DDIST_DIR=... -DOUTPUT_FILE=... -P embed_assets.cmake

if(NOT DEFINED DIST_DIR OR NOT DEFINED OUTPUT_FILE)
	message(FATAL_ERROR "DIST_DIR and OUTPUT_FILE must be defined")
endif()

file(GLOB_RECURSE ASSET_FILES RELATIVE "${DIST_DIR}" "${DIST_DIR}/*")

# Map file extensions to MIME types
function(get_mime_type EXT RESULT_VAR)
	if("${EXT}" STREQUAL ".html")
		set(${RESULT_VAR} "text/html" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".js")
		set(${RESULT_VAR} "application/javascript" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".css")
		set(${RESULT_VAR} "text/css" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".svg")
		set(${RESULT_VAR} "image/svg+xml" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".ico")
		set(${RESULT_VAR} "image/x-icon" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".png")
		set(${RESULT_VAR} "image/png" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".json")
		set(${RESULT_VAR} "application/json" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".woff2")
		set(${RESULT_VAR} "font/woff2" PARENT_SCOPE)
	elseif("${EXT}" STREQUAL ".woff")
		set(${RESULT_VAR} "font/woff" PARENT_SCOPE)
	else()
		set(${RESULT_VAR} "application/octet-stream" PARENT_SCOPE)
	endif()
endfunction()

# Sanitize filename for use as C++ identifier
function(sanitize_name FILENAME RESULT_VAR)
	string(REGEX REPLACE "[^a-zA-Z0-9]" "_" SANITIZED "${FILENAME}")
	set(${RESULT_VAR} "${SANITIZED}" PARENT_SCOPE)
endfunction()

set(CPP_CONTENT "// Auto-generated file — do not edit\n")
string(APPEND CPP_CONTENT "#include \"searchdui.h\"\n\n")
string(APPEND CPP_CONTENT "#if WITH_UI\n\n")

set(ASSET_ENTRIES "")
set(ASSET_COUNT 0)

foreach(ASSET_FILE ${ASSET_FILES})
	file(READ "${DIST_DIR}/${ASSET_FILE}" FILE_HEX HEX)
	string(LENGTH "${FILE_HEX}" HEX_LEN)
	math(EXPR BYTE_COUNT "${HEX_LEN} / 2")

	sanitize_name("${ASSET_FILE}" SAFE_NAME)

	# Convert hex pairs to 0xHH format
	set(BYTE_ARRAY "")
	set(POS 0)
	set(LINE_COUNT 0)
	while(POS LESS HEX_LEN)
		string(SUBSTRING "${FILE_HEX}" ${POS} 2 HEX_BYTE)
		if(BYTE_ARRAY)
			string(APPEND BYTE_ARRAY ",")
		endif()
		if(LINE_COUNT EQUAL 20)
			string(APPEND BYTE_ARRAY "\n\t")
			set(LINE_COUNT 0)
		endif()
		string(APPEND BYTE_ARRAY "0x${HEX_BYTE}")
		math(EXPR POS "${POS} + 2")
		math(EXPR LINE_COUNT "${LINE_COUNT} + 1")
	endwhile()

	string(APPEND CPP_CONTENT "static const unsigned char kAsset_${SAFE_NAME}[] = {\n\t${BYTE_ARRAY}\n};\n\n")

	get_filename_component(FILE_EXT "${ASSET_FILE}" EXT)
	get_mime_type("${FILE_EXT}" MIME_TYPE)

	string(APPEND ASSET_ENTRIES "\t{ \"${ASSET_FILE}\", kAsset_${SAFE_NAME}, ${BYTE_COUNT}, \"${MIME_TYPE}\" },\n")
	math(EXPR ASSET_COUNT "${ASSET_COUNT} + 1")
endforeach()

string(APPEND CPP_CONTENT "const EmbeddedAsset_t g_dUIAssets[] = {\n${ASSET_ENTRIES}};\n\n")
string(APPEND CPP_CONTENT "const int g_iUIAssetCount = ${ASSET_COUNT};\n\n")
string(APPEND CPP_CONTENT "#endif // WITH_UI\n")

file(WRITE "${OUTPUT_FILE}" "${CPP_CONTENT}")
message(STATUS "Embedded ${ASSET_COUNT} UI assets into ${OUTPUT_FILE}")
