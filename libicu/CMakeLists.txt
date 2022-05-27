cmake_minimum_required(VERSION 3.17)
PROJECT(icu CXX)
SET(CMAKE_CXX_STANDARD 11)
add_library(icu
		source/common/appendable.cpp
		source/common/bmpset.cpp
		source/common/brkeng.cpp
		source/common/brkiter.cpp
		source/common/bytesinkutil.cpp
		source/common/bytestream.cpp
		source/common/bytestrie.cpp
		source/common/characterproperties.cpp
		source/common/chariter.cpp
		source/common/charstr.cpp
		source/common/cmemory.cpp
		source/common/cstring.cpp
		source/common/dictbe.cpp
		source/common/dictionarydata.cpp
		source/common/edits.cpp
		source/common/errorcode.cpp
		source/common/filteredbrk.cpp
		source/common/loadednormalizer2impl.cpp
		source/common/locavailable.cpp
		source/common/locbased.cpp
		source/common/locdispnames.cpp
		source/common/locid.cpp
		source/common/loclikely.cpp
		source/common/locmap.cpp
		source/common/locresdata.cpp
		source/common/locutil.cpp
		source/common/normalizer2.cpp
		source/common/normalizer2impl.cpp
		source/common/patternprops.cpp
		source/common/parsepos.cpp
		source/common/putil.cpp
		source/common/propname.cpp
		source/common/rbbi.cpp
		source/common/rbbi_cache.cpp
		source/common/rbbidata.cpp
		source/common/rbbinode.cpp
		source/common/rbbirb.cpp
		source/common/rbbiscan.cpp
		source/common/rbbisetb.cpp
		source/common/rbbistbl.cpp
		source/common/rbbitblb.cpp
		source/common/resbund.cpp
		source/common/resbund_cnv.cpp
		source/common/resource.cpp
		source/common/ruleiter.cpp
		source/common/schriter.cpp
		source/common/serv.cpp
		source/common/servlkf.cpp
		source/common/servlk.cpp
		source/common/servls.cpp
		source/common/servnotf.cpp
		source/common/servrbf.cpp
		source/common/servslkf.cpp
		source/common/stringpiece.cpp
		source/common/stringtriebuilder.cpp
		source/common/uarrsort.cpp
		source/common/ubidi_props.cpp
		source/common/ucase.cpp
		source/common/uchar.cpp
		source/common/ucharstrie.cpp
		source/common/ucharstriebuilder.cpp
		source/common/uchriter.cpp
		source/common/ucln_cmn.cpp
		source/common/ucmndata.cpp
		source/common/ucnv.cpp
		source/common/ucnv2022.cpp
		source/common/ucnv_bld.cpp
		source/common/ucnv_cb.cpp
		source/common/ucnv_cnv.cpp
		source/common/ucnv_ct.cpp
		source/common/ucnv_err.cpp
		source/common/ucnv_ext.cpp
		source/common/ucnv_io.cpp
		source/common/ucnv_lmb.cpp
		source/common/ucnv_set.cpp
		source/common/ucnv_u16.cpp
		source/common/ucnv_u32.cpp
		source/common/ucnv_u7.cpp
		source/common/ucnv_u8.cpp
		source/common/ucnvbocu.cpp
		source/common/ucnvdisp.cpp
		source/common/ucnvhz.cpp
		source/common/ucnvisci.cpp
		source/common/ucnvlat1.cpp
		source/common/ucnvmbcs.cpp
		source/common/ucnvscsu.cpp
		source/common/ucnvsel.cpp
		source/common/ucol_swp.cpp
		source/common/ucptrie.cpp
		source/common/udata.cpp
		source/common/udatamem.cpp
		source/common/udataswp.cpp
		source/common/uenum.cpp
		source/common/uhash.cpp
		source/common/uhash_us.cpp
		source/common/uinvchar.cpp
		source/common/uloc.cpp
		source/common/uloc_tag.cpp
		source/common/uloc_keytype.cpp
		source/common/umapfile.cpp
		source/common/umath.cpp
		source/common/umutablecptrie.cpp
		source/common/umutex.cpp
		source/common/unames.cpp
		source/common/unifilt.cpp
		source/common/unifunct.cpp
		source/common/uniset.cpp
		source/common/uniset_closure.cpp
		source/common/uniset_props.cpp
		source/common/unisetspan.cpp
		source/common/unistr.cpp
		source/common/unistr_case.cpp
		source/common/unistr_cnv.cpp
		source/common/uobject.cpp
		source/common/uprops.cpp
		source/common/uresbund.cpp
		source/common/uresdata.cpp
		source/common/uscript_props.cpp
		source/common/ustack.cpp
		source/common/ustr_cnv.cpp
		source/common/ustrcase.cpp
		source/common/ustrenum.cpp
		source/common/ustrfmt.cpp
		source/common/ustring.cpp
		source/common/ustrtrns.cpp
		source/common/utext.cpp
		source/common/utf_impl.cpp
		source/common/utrace.cpp
		source/common/utrie.cpp
		source/common/utrie_swap.cpp
		source/common/utrie2.cpp
		source/common/utrie2_builder.cpp
		source/common/uvectr32.cpp
		source/common/uvectr64.cpp
		source/common/uvector.cpp
		source/common/util.cpp
		source/common/utypes.cpp
		source/common/wintz.cpp
		source/stubdata/stubdata.cpp
		)

# Set a default build type for single-configuration CMake generators if no build type is set.
get_property ( isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )
if (NOT isMultiConfig AND NOT CMAKE_BUILD_TYPE)
	set ( CMAKE_BUILD_TYPE RelWithDebInfo )
	message ( STATUS "Automatically set build type to RelWithDebInfo since no other provided" )
endif ()

OPTION (STATIC_BUILD "build without dlopen, pure static" OFF )

target_compile_definitions(icu PRIVATE U_IMPORT= U_EXPORT= U_EXPORT2= U_COMMON_IMPLEMENTATION)
target_compile_definitions(icu PRIVATE $<$<CXX_COMPILER_ID:MSVC>:UCONFIG_USE_WINDOWS_LCID_MAPPING_API=0>)
target_compile_options(icu PRIVATE $<$<CXX_COMPILER_ID:MSVC>:-D_CRT_SECURE_NO_WARNINGS -wd4541 -wd4800 -wd4996>)
target_compile_definitions(icu PUBLIC U_STATIC_IMPLEMENTATION U_CHARSET_IS_UTF8=1 U_NO_DEFAULT_INCLUDE_UTF_HEADERS=1)

if (STATIC_BUILD)
	target_compile_definitions(icu PRIVATE HAVE_DLOPEN=0)
endif()

target_include_directories(icu PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source/common>$<INSTALL_INTERFACE:include/icu>)
set_target_properties (icu PROPERTIES
		DEBUG_POSTFIX d
		PDB_NAME icu
		PDB_NAME_DEBUG icud
		COMPILE_PDB_NAME icu
		COMPILE_PDB_NAME_DEBUG icud
		)

# separate target for icu data
file ( GLOB ICU_DATA "${CMAKE_CURRENT_SOURCE_DIR}/source/data/in/icu*.dat" )
get_filename_component ( ICU_DATA ${ICU_DATA} NAME )

add_library ( icudata INTERFACE )
target_sources ( icudata INTERFACE $<INSTALL_INTERFACE:data/${ICU_DATA}> )

# installation stuff
set ( CMAKE_CMAKE_DIR "lib/cmake/icu" )

# ensure ALL externally required headers and are exported here
install ( DIRECTORY source/common/unicode DESTINATION include/icu FILES_MATCHING PATTERN "*.h")
install ( FILES "source/data/in/${ICU_DATA}" DESTINATION data )
install ( TARGETS icu icudata EXPORT icuExport RUNTIME DESTINATION lib LIBRARY DESTINATION lib  )
install (EXPORT icuExport FILE icu-targets.cmake DESTINATION "${CMAKE_CMAKE_DIR}" NAMESPACE icu::)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	install (FILES "$<TARGET_FILE_DIR:icu>/icu$<$<CONFIG:Debug>:d>.pdb" DESTINATION lib OPTIONAL)
endif ()

include ( CMakePackageConfigHelpers )

file ( WRITE icu-config.cmake.in
		"\@PACKAGE_INIT@

if(NOT TARGET icu::icu)
    include(\"\${CMAKE_CURRENT_LIST_DIR}/icu-targets.cmake\")
    get_target_property ( configs icu::icu IMPORTED_CONFIGURATIONS )
    foreach (config DEBUG RELEASE MINSIZEREL)
        if (NOT \${config} IN_LIST configs)
            set_property ( TARGET icu::icu PROPERTY MAP_IMPORTED_CONFIG_\${config} RelWithDebInfo )
        endif ()
    endforeach ()
endif()
" )

configure_package_config_file ( icu-config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/icu-config.cmake"
		INSTALL_DESTINATION "${CMAKE_CMAKE_DIR}" )
install ( FILES "${CMAKE_CURRENT_BINARY_DIR}/icu-config.cmake" DESTINATION "${CMAKE_CMAKE_DIR}" )