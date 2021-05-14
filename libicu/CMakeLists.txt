cmake_minimum_required(VERSION 3.0)
PROJECT(icu CXX)
SET(CMAKE_CXX_STANDARD 11)
set(ICU_SRCS
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

add_library(icu STATIC ${ICU_SRCS})

target_compile_options(icu PRIVATE "-DU_IMPORT=")
target_compile_options(icu PRIVATE "-DU_EXPORT=")
target_compile_options(icu PRIVATE "-DU_EXPORT2=")
target_compile_options(icu PRIVATE "-DU_COMMON_IMPLEMENTATION")
target_compile_options(icu PRIVATE "-DU_STATIC_IMPLEMENTATION")
target_compile_options(icu PRIVATE "-DU_CHARSET_IS_UTF8=1")
target_compile_options(icu PRIVATE "-DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1")
target_compile_options(icu PRIVATE "-D_CRT_SECURE_NO_WARNINGS")
target_compile_options(icu PRIVATE "-DUCONFIG_USE_WINDOWS_LCID_MAPPING_API=0")

if (WIN32)
	target_compile_definitions(icu PRIVATE "-wd4541 -wd4800 -wd4996")

	# on win we have default /GR- flag (no rtti), but ICU requires it, so remove the flag
	set(CompilerFlags
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL
			CMAKE_CXX_FLAGS_RELWITHDEBINFO
			)
	foreach (CompilerFlag ${CompilerFlags})
		STRING(REPLACE "/GR-" " " ${CompilerFlag} "${${CompilerFlag}}")
	endforeach ()
else ()
	target_compile_options(icu PRIVATE "-fPIC")
endif ()

target_include_directories(icu BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source/common)

set_target_properties(icu PROPERTIES
		MAP_IMPORTED_CONFIG_MINSIZEREL RelWithDebInfo
		MAP_IMPORTED_CONFIG_RELEASE RelWithDebInfo
		)
set_target_properties(icu PROPERTIES EXPORT_PROPERTIES "MAP_IMPORTED_CONFIG_MINSIZEREL;MAP_IMPORTED_CONFIG_RELEASE")

# separate target for icu data
file(GLOB ICU_DATA "${CMAKE_CURRENT_SOURCE_DIR}/source/data/in/icu*.dat")

add_library(icudata INTERFACE)
target_sources(icudata INTERFACE "${ICU_DATA}")

export(TARGETS icu icudata FILE "icu-targets.cmake" NAMESPACE icu::)