cmake_minimum_required ( VERSION 3.17 )

FOREACH (policy CMP0054 CMP0057 CMP0042 CMP0074 CMP0110)
	IF (POLICY ${policy})
		CMAKE_POLICY ( SET ${policy} NEW )
	ENDIF ()
ENDFOREACH ()

message ( STATUS "Inside directory src..." )

#set (CMAKE_DEBUG_TARGET_PROPERTIES
#		INCLUDE_DIRECTORIES
#		COMPILE_DEFINITIONS
#		LINK_LIBRARIES
#		POSITION_INDEPENDENT_CODE
#		CONTAINER_SIZE_REQUIRED
#		LIB_VERSION
#		)

# columnar and secondary modules
include ( GetColumnar )
target_link_libraries ( lextra INTERFACE columnar::columnar_api columnar::secondary_api columnar::knn_api )

# check if we have correct columnar api
set_property ( TARGET lextra PROPERTY INTERFACE_COLUMNAR_API_VERSION ${NEED_COLUMNAR_API} )
set_property ( TARGET lextra APPEND PROPERTY COMPATIBLE_INTERFACE_STRING COLUMNAR_API_VERSION )

# check if we have correct secondary api
set_property ( TARGET lextra PROPERTY INTERFACE_SECONDARY_API_VERSION ${NEED_SECONDARY_API} )
set_property ( TARGET lextra APPEND PROPERTY COMPATIBLE_INTERFACE_STRING SECONDARY_API_VERSION )

# check if we have correct knn api
set_property ( TARGET lextra PROPERTY INTERFACE_KNN_API_VERSION ${NEED_KNN_API} )
set_property ( TARGET lextra APPEND PROPERTY COMPATIBLE_INTERFACE_STRING KNN_API_VERSION )

target_include_directories ( lextra INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )
get_banner ( CONFIGURE_FLAGS )
infomsg ( "${CONFIGURE_FLAGS}" )

infomsg ( "Generating config files..." )
configure_file ( "${MANTICORE_SOURCE_DIR}/config/config_cmake.h.in"
		"${MANTICORE_BINARY_DIR}/config/config_.h" ESCAPE_QUOTES )
configure_file ( "${MANTICORE_SOURCE_DIR}/config/config_indexer_cmake.h.in"
		"${MANTICORE_BINARY_DIR}/config/config_indexer_.h" ESCAPE_QUOTES )

# avoid config rebuild if configuration produces the same file
execute_process ( COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MANTICORE_BINARY_DIR}/config/config_.h" "${MANTICORE_BINARY_DIR}/config/config.h" )
execute_process ( COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MANTICORE_BINARY_DIR}/config/config_indexer_.h" "${MANTICORE_BINARY_DIR}/config/config_indexer.h" )

add_library ( generated_config INTERFACE )
target_include_directories ( generated_config INTERFACE "${MANTICORE_BINARY_DIR}/config" )
target_link_libraries ( lextra INTERFACE generated_config )

# main bunch of sources (for libsphinx)
add_library ( lmanticore STATIC sphinx.cpp sphinxquery.cpp sphinxutils.cpp
		sphinxsort.cpp sortsetup.cpp sphinxexpr.cpp sphinxfilter.cpp sphinxsearch.cpp sphinxrt.cpp accumulator.cpp
		sphinxjson.cpp sphinxaot.cpp sphinxplugin.cpp sphinxudf.c sphinxqcache.cpp 
		attribute.cpp secondaryindex.cpp killlist.cpp searchnode.cpp json/cJSON.c sphinxpq.cpp
		global_idf.cpp docstore.cpp lz4/lz4.c lz4/lz4hc.c snippetfunctor.cpp snippetindex.cpp
		snippetstream.cpp snippetpassage.cpp threadutils.cpp sphinxversion.cpp indexcheck.cpp datareader.cpp
		indexformat.cpp indexsettings.cpp fileutils.cpp threads_detached.cpp hazard_pointer.cpp
		task_info.cpp mini_timer.cpp fileio.cpp memio.cpp queryprofile.cpp columnarfilter.cpp columnargrouper.cpp
		columnarlib.cpp collation.cpp histogram.cpp
		timeout_queue.cpp columnarrt.cpp columnarmisc.cpp exprtraits.cpp columnarexpr.cpp
		sphinx_alter.cpp columnarsort.cpp binlog.cpp chunksearchctx.cpp client_task_info.cpp
		indexfiles.cpp indexfilebase.cpp attrindex_builder.cpp queryfilter.cpp aggregate.cpp secondarylib.cpp costestimate.cpp
		docidlookup.cpp tracer.cpp attrindex_merge.cpp distinct.cpp hyperloglog.cpp pseudosharding.cpp geodist.cpp
		datetime.cpp grouper.cpp exprdatetime.cpp detail/indexlink.cpp knnmisc.cpp knnlib.cpp libutils.cpp
		aggrexpr.cpp joinsorter.cpp queuecreator.cpp exprgeodist.cpp exprremap.cpp exprdocstore.cpp schematransform.cpp
		sortergroup.cpp sortertraits.cpp sorterprecalc.cpp querycontext.cpp skip_cache.cpp jsonsi.cpp sorterscroll.cpp )

add_library ( lstem STATIC sphinxsoundex.cpp sphinxmetaphone.cpp sphinxstemen.cpp sphinxstemru.cpp sphinxstemru.inl
		sphinxstemcz.cpp sphinxstemar.cpp )
target_link_libraries ( lstem PUBLIC lextra )

add_library ( conversion conversion.cpp )
target_link_libraries ( conversion PUBLIC lextra )

target_link_libraries ( lmanticore PUBLIC conversion )

add_subdirectory ( std )
target_link_libraries ( lmanticore PUBLIC manticore_std )

add_subdirectory ( stripper )
target_link_libraries ( lmanticore PUBLIC stripper )

add_subdirectory ( tokenizer )
target_link_libraries ( lmanticore PUBLIC tokenizer )
target_link_libraries ( lmanticore PUBLIC lextra )

add_subdirectory ( schema )
target_link_libraries ( lmanticore PUBLIC schema )

add_subdirectory ( dict )
target_link_libraries ( lmanticore PUBLIC dict )

if (WITH_ICU OR WITH_JIEBA)
	add_library ( lcjk cjkpreprocessor.cpp )
	target_link_libraries ( lcjk PUBLIC lextra)
endif()

add_library ( lmanticore_icu icu.cpp )
if (WITH_ICU)
	target_link_libraries ( lmanticore_icu PRIVATE icu::icu )
endif ()
target_link_libraries ( lmanticore_icu PRIVATE lcjk )
target_link_libraries ( lmanticore PUBLIC lmanticore_icu )

if (WITH_JIEBA)
	add_library ( lmanticore_jieba jieba.cpp jieba.h )
	target_link_libraries ( lmanticore_jieba PRIVATE lcjk jieba::jieba )
	target_link_libraries ( lmanticore PUBLIC lmanticore_jieba )
endif ()

if (WITH_STEMMER)
	target_link_libraries ( lmanticore PRIVATE stemmer::stemmer )
endif ()
if (WITH_RE2)
	target_link_libraries ( lmanticore PRIVATE re2::re2 )
endif ()

add_subdirectory ( indexing_sources )
target_link_libraries ( lmanticore PUBLIC sourcedoc )

if (WIN32)
	if (NOT CMAKE_CROSSCOMPILING)
		target_compile_options ( lmanticore PRIVATE /bigobj )
	endif ()
endif ()

# Boost is required dependency now
set ( Boost_USE_MULTITHREADED ON )
set ( Boost_USE_STATIC_LIBS ON )
# todo: as alternative we can try to recompile boost with clang, but precompiled also quite ok
if (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL Windows)
	set ( Boost_COMPILER vc142 )
endif ()
# ver 1.64 for process; filesystem need for process; ver 1.71 for limit_handles
find_package ( Boost 1.71.0 REQUIRED COMPONENTS context filesystem  )
target_link_libraries ( lextra INTERFACE Boost::boost Boost::context )

# Fixme! That is temporary solution - as we just use some headers of Boost::fiber, and that is available by default.
# If we link with the lib, we should find_package for component fiber, and then link with it. *NOT* that is not necessary.
# But Windows implicitly links with boost::fibers even if we just include some of that headers. That is why BOOST_ALL_NO_LIB
# is poisoned into lextra interface - to avoid such behavior.
# (we don't link with fiber since our custom build dockers doesn't include it, and they need to be rebuilt before)
target_compile_definitions ( lextra INTERFACE BOOST_ALL_NO_LIB=1 )

add_library ( lmanticore_coro coroutine.cpp coro_stack.cpp task_dispatcher.cpp coroutine.h coro_stack.h coro_waker.h task_dispatcher.h)
target_link_libraries ( lmanticore_coro PUBLIC lextra )

target_link_libraries ( lmanticore PUBLIC lmanticore_coro )

# The headers is not neccessary for executable build, but nesessary for MSVC and other projects
# all the (non-generated) headers
# So if you add headers to the project and NOT see them in visual studio solution - just list them here!
set ( HEADERS sphinxfilter.h sphinxint.h sphinxpq.h sphinxrt.h
		sphinxsort.h sphinxstem.h sphinxutils.h sphinxexpr.h sphinx.h sphinxjson.h sphinxplugin.h sphinxqcache.h
		sphinxquery.h sphinxsearch.h sphinxstd.h sphinxudf.h lz4/lz4.h lz4/lz4hc.h http/http_parser.h secondaryindex.h
		searchnode.h killlist.h attribute.h accumulator.h global_idf.h event.h threadutils.h threadutils_impl.h
		hazard_pointer.h task_info.h mini_timer.h collation.h histogram.h sortsetup.h
		indexsettings.h columnarlib.h fileio.h memio.h memio_impl.h queryprofile.h columnarfilter.h columnargrouper.h fileutils.h
		libutils.h conversion.h columnarsort.h sortcomp.h binlog_defs.h binlog.h ${MANTICORE_BINARY_DIR}/config/config.h
		chunksearchctx.h indexfilebase.h indexfiles.h attrindex_builder.h queryfilter.h aggregate.h secondarylib.h
		costestimate.h docidlookup.h tracer.h attrindex_merge.h columnarmisc.h distinct.h hyperloglog.h pseudosharding.h datetime.h
		grouper.h exprdatetime.h geodist.h detail/indexlink.h detail/expmeter.h knnmisc.h knnlib.h match_impl.h std/string_impl.h
		aggrexpr.h joinsorter.h queuecreator.h exprgeodist.h exprremap.h exprdocstore.h schematransform.h sortergroup.h
		sortertraits.h sorterprecalc.h querycontext.h skip_cache.h jsonsi.h jieba.h cjkpreprocessor.h sorterscroll.h )

set ( SEARCHD_H searchdaemon.h searchdconfig.h searchdddl.h searchdexpr.h searchdha.h searchdreplication.h searchdsql.h
		searchdtask.h client_task_info.h taskflushattrs.h taskflushbinlog.h taskflushmutable.h taskglobalidf.h
		taskmalloctrim.h taskping.h taskpreread.h tasksavestate.h net_action_accept.h
		netreceive_api.h netreceive_http.h netreceive_ql.h networking_daemon.h query_status.h
		compressed_zlib_mysql.h sphinxql_debug.h stackmock.h searchdssl.h digest_sha1.h
		client_session.h compressed_zstd_mysql.h docs_collector.h index_rotator.h config_reloader.h searchdhttp.h timeout_queue.h
		netpoll.h pollable_event.h netfetch.h searchdbuddy.h sphinxql_second.h sphinxql_extra.h sphinxjsonquery.h
		frontendschema.h debug_cmds.h dynamic_idx.h sphinxexcerpt.h taskflushdisk.h)


# add the extra targets in the case we want on-the-fly grammar compiler
find_package ( BISON QUIET )
set ( BISON_DIR "${MANTICORE_BINARY_DIR}/config" )
set ( BIS_FLAGS "" )
if (BISON_VERSION VERSION_GREATER 3.0)
	set ( BIS_FLAGS "-Wno-deprecated" )
endif ()

function ( MY_BISON ParserName ParserSrc Dependency TargetBison )
	if (NOT BISON_FOUND)
		if (EXISTS "${BISON_DIR}/bis${ParserSrc}.c" AND EXISTS "${BISON_DIR}/bis${ParserSrc}.h")
			infomsg ( "Will use pre-built ${BISON_DIR}/bis${ParserSrc}.c and ${BISON_DIR}/bis${ParserSrc}.h for grammar parser ${ParserName}" )
			return ()
		else ()
			message ( FATAL_ERROR "No pre-compiled grammar files bis${ParserSrc}.c and bis${ParserSrc}.h for ${ParserSrc}.y exists, and Bison not found. Can't continue" )
		endif ()
	endif ()

	LIST ( APPEND ${TargetBison}_BISON "${ParserSrc}.y" )
	set ( ${TargetBison}_BISON ${${TargetBison}_BISON} PARENT_SCOPE )
	BISON_TARGET ( ${ParserName} "${CMAKE_CURRENT_SOURCE_DIR}/${ParserSrc}.y" "${BISON_DIR}/bis${ParserSrc}.c" COMPILE_FLAGS ${BIS_FLAGS} )
	set_source_files_properties ( ${Dependency} PROPERTIES OBJECT_DEPENDS ${BISON_${ParserName}_OUTPUT_SOURCE} SKIP_UNITY_BUILD_INCLUSION ON )
endfunction ()

MY_BISON ( ExprParser sphinxexpr sphinxexpr.cpp LMANTICORE )
MY_BISON ( SelectParser sphinxselect sphinx.cpp LMANTICORE )
MY_BISON ( QueryParser sphinxquery sphinxquery.cpp LMANTICORE )
MY_BISON ( JsonParser sphinxjson sphinxjson.cpp LMANTICORE )
MY_BISON ( SQLParser sphinxql searchdsql.cpp SEARCHD )
MY_BISON ( DDLParser ddl searchdddl.cpp SEARCHD )
MY_BISON ( SQLDebugParser sphinxql_debug sphinxql_debug.cpp SEARCHD )
MY_BISON ( SQLSecondParser sphinxql_second sphinxql_second.cpp SEARCHD )
MY_BISON ( SQLExtraParser sphinxql_extra sphinxql_extra.cpp SEARCHD )
source_group ( "Grammar sources" FILES ${LMANTICORE_BISON} ${SEARCHD_BISON} )

# add the extra targets in the case we want on-the-fly lex compiler
find_package ( FLEX QUIET )
set ( FLEX_DIR "${MANTICORE_BINARY_DIR}/config" )
function ( MY_FLEX LexerName LexerSrc Dependency TargetFlex )
	if (NOT FLEX_FOUND)
		if (EXISTS "${FLEX_DIR}/flex${LexerSrc}.c")
			infomsg ( "Will use pre-built ${FLEX_DIR}/flex${LexerSrc}.c for lexer ${LexerName}" )
			return ()
		else ()
			message ( FATAL_ERROR "No pre-compiled lexer file flex${LexerSrc}.c for ${LexerSrc}.l exists, and Flex not found. Can't continue" )
		endif ()
	endif ()

	LIST ( APPEND ${TargetFlex}_FLEX "${LexerSrc}.l" )
	set ( ${TargetFlex}_FLEX ${${TargetFlex}_FLEX} PARENT_SCOPE )
	FLEX_TARGET ( ${LexerName} "${CMAKE_CURRENT_SOURCE_DIR}/${LexerSrc}.l" "${FLEX_DIR}/flex${LexerSrc}.c" )
	set_property ( SOURCE ${Dependency} APPEND PROPERTY OBJECT_DEPENDS ${FLEX_${LexerName}_OUTPUTS} )
endfunction ()

MY_FLEX ( Jsonlex sphinxjson sphinxjson.cpp LMANTICORE )
MY_FLEX ( Exprlex sphinxexpr sphinxexpr.cpp LMANTICORE )
MY_FLEX ( SQLlex sphinxql searchdsql.cpp SEARCHD )
MY_FLEX ( DDLlex ddl searchdddl.cpp SEARCHD )
MY_FLEX ( SQLDebuglex sphinxql_debug sphinxql_debug.cpp SEARCHD )
MY_FLEX ( SQLSecondlex sphinxql_second sphinxql_second.cpp SEARCHD )
MY_FLEX ( SQLExtralex sphinxql_extra sphinxql_extra.cpp SEARCHD )
source_group ( "Lexer sources" FILES ${LMANTICORE_FLEX} ${SEARCHD_FLEX} )

diag ( LMANTICORE_BISON SEARCHD_BISON LMANTICORE_FLEX SEARCHD_FLEX )

# our mega-lib
target_sources ( lmanticore PUBLIC ${LMANTICORE_BISON} ${LMANTICORE_FLEX} ${HEADERS} ${CHARSET_FILES} ${CHARSET_TEMPLATE} "../misc/manticore.natvis" )
target_link_libraries ( lmanticore PUBLIC uni-algo::uni-algo )

add_library ( lsearchd OBJECT searchdha.cpp http/http_parser.c searchdhttp.cpp
		searchdtask.cpp taskping.cpp taskmalloctrim.cpp taskglobalidf.cpp tasksavestate.cpp
		taskflushbinlog.cpp taskflushattrs.cpp taskflushmutable.cpp taskpreread.cpp
		searchdaemon.cpp searchdfields.cpp searchdconfig.cpp
		searchdsql.cpp searchdddl.cpp networking_daemon.cpp
		net_action_accept.cpp netreceive_api.cpp
		netreceive_http.cpp netreceive_ql.cpp query_status.cpp
		sphinxql_debug.cpp sphinxql_second.cpp stackmock.cpp docs_collector.cpp index_rotator.cpp config_reloader.cpp netpoll.cpp
		pollable_event.cpp netfetch.cpp searchdbuddy.cpp searchdhttpcompat.cpp sphinxql_extra.cpp searchdreplication.cpp sphinxjsonquery.cpp
		frontendschema.cpp compressed_http.cpp debug_cmds.cpp jsonqueryfilter.cpp dynamic_idx.cpp sphinxexcerpt.cpp searchdexpr.cpp taskflushdisk.cpp)
target_sources ( lsearchd PUBLIC ${SEARCHD_SRCS_TESTABLE} ${SEARCHD_H} ${SEARCHD_BISON} ${SEARCHD_FLEX} )
add_library ( digest_sha1 digest_sha1.cpp )
target_link_libraries ( digest_sha1 PRIVATE lextra )
target_link_libraries ( lsearchd PUBLIC digest_sha1 lextra nlohmann_json::nlohmann_json )
target_link_libraries ( lsearchd INTERFACE Boost::filesystem )

function (stackmock processors compiler versions config values)
	if (NOT CMAKE_SYSTEM_PROCESSOR IN_LIST processors)
		return()
	endif()

	if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "${compiler}")
		return()
	endif()

	if (NOT CMAKE_CXX_COMPILER_VERSION IN_LIST versions)
		return()
	endif()

	string ( TOUPPER ${config} CONFIGUPPER )
	diags ( "Set ${values} for ${CMAKE_SYSTEM_PROCESSOR}, ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} config ${config}")
	set_source_files_properties ( stackmock.cpp PROPERTIES COMPILE_DEFINITIONS_${CONFIGUPPER} "${values}" )
endfunction()

diag ( CMAKE_SYSTEM_PROCESSOR CMAKE_CXX_COMPILER_ID CMAKE_CXX_COMPILER_VERSION )
stackmock ( "x86_64;amd64" Clang "12;13;13.0.1" relwithdebinfo "KNOWN_CREATE_SIZE=768;KNOWN_EXPR_SIZE=32;KNOWN_FILTER_SIZE=224" )
stackmock ( "x86_64;amd64" Clang "12;13;13.0.1" debug "KNOWN_CREATE_SIZE=4208;KNOWN_EXPR_SIZE=48;KNOWN_FILTER_SIZE=400" )
stackmock ( "x86_64;amd64" Clang "15.0.2" relwithdebinfo "KNOWN_CREATE_SIZE=208;KNOWN_EXPR_SIZE=32;KNOWN_FILTER_SIZE=128" )
stackmock ( "x86_64;amd64" Clang "15.0.4;15.0.6" relwithdebinfo "KNOWN_CREATE_SIZE=208;KNOWN_EXPR_SIZE=96;KNOWN_FILTER_SIZE=304;KNOWN_MATCH_SIZE=336;START_KNOWN_MATCH_SIZE=14456" )
stackmock ( "x86_64;amd64" Clang "15.0.7" relwithdebinfo "KNOWN_CREATE_SIZE=16;KNOWN_EXPR_SIZE=32;KNOWN_FILTER_SIZE=224;KNOWN_MATCH_SIZE=336;START_KNOWN_MATCH_SIZE=14456" )
stackmock ( "x86_64;amd64" Clang "15.0.2;15.0.4;15.0.6" debug "KNOWN_CREATE_SIZE=1936;KNOWN_EXPR_SIZE=48;KNOWN_FILTER_SIZE=368" )
stackmock ( "x86_64;amd64" GNU "9.3;9.4" relwithdebinfo "KNOWN_CREATE_SIZE=16;KNOWN_EXPR_SIZE=48;KNOWN_FILTER_SIZE=240" )
stackmock ( "x86_64;amd64" GNU "9.3;9.4" debug "KNOWN_CREATE_SIZE=256;KNOWN_EXPR_SIZE=48;KNOWN_FILTER_SIZE=272" )

if (WITH_ZLIB OR WITH_ZSTD)
	target_sources ( lsearchd PRIVATE compressed_mysql_layer.cpp compressed_mysql_layer.h )
	if (WITH_ZLIB)
		target_sources ( lsearchd PRIVATE compressed_zlib_mysql.cpp )
		target_link_libraries ( lsearchd PRIVATE ZLIB::ZLIB )
	endif ()

	if (WITH_ZSTD)
		target_sources ( lsearchd PRIVATE compressed_zstd_mysql.cpp )
		if (DL_ZSTD)
			target_link_libraries ( lsearchd PRIVATE ZSTD::ZSTD_ld )
		else ()
			target_link_libraries ( lsearchd PRIVATE ZSTD::ZSTD )
		endif ()
	endif ()
endif ()

if (WITH_CURL)
	if (DL_CURL)
		target_link_libraries ( lsearchd PRIVATE CURL::libcurl_ld )
	else ()
		target_link_libraries ( lsearchd PRIVATE CURL::libcurl )
	endif ()
endif ()

add_subdirectory ( perfetto )
if (TARGET perfetto)
	target_link_libraries ( lextra INTERFACE perfetto )
endif()

# stub library for all except daemon - to avoid link with openssl
add_library ( stub_ssl searchdssl_stub.cpp )
target_link_libraries ( stub_ssl PRIVATE lextra )

# support for ssl. Actual only for physical daemon
add_library ( searchd_ssl searchdssl.cpp )
target_link_libraries ( searchd_ssl PUBLIC lextra )
if (WITH_SSL)
	target_link_libraries ( searchd_ssl PRIVATE OpenSSL::SSL )
	target_link_libraries ( searchd_ssl PRIVATE OpenSSL::Crypto )
	target_link_libraries ( digest_sha1 PRIVATE OpenSSL::SSL )
	target_link_libraries ( digest_sha1 PRIVATE OpenSSL::Crypto )
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
		target_link_options ( searchd_ssl INTERFACE $<${ONLYGNUCLANGC_CXX}:-Wl,--exclude-libs,libssl.a,--exclude-libs,libcrypto.a> )
		target_link_options ( digest_sha1 INTERFACE $<${ONLYGNUCLANGC_CXX}:-Wl,--exclude-libs,libssl.a,--exclude-libs,libcrypto.a> )
	endif()
	target_compile_options ( searchd_ssl PRIVATE "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-deprecated-declarations>" )
	include ( CheckFunctionExists )
	set ( CMAKE_REQUIRED_LIBRARIES "OpenSSL::SSL" )
	check_function_exists ( TLS_server_method HAVE_TLS_SERVER_METHOD )
	if (HAVE_TLS_SERVER_METHOD)
		target_compile_definitions ( searchd_ssl PRIVATE HAVE_TLS_SERVER_METHOD=${HAVE_TLS_SERVER_METHOD} )
	endif ()
	check_function_exists ( TLSv1_2_method HAVE_TLSV1_2_METHOD )
	if (HAVE_TLSV1_2_METHOD)
		target_compile_definitions ( searchd_ssl PRIVATE HAVE_TLSV1_2_METHOD=${HAVE_TLSV1_2_METHOD} )
	endif ()
	check_function_exists ( TLSv1_1_server_method HAVE_TLSV1_1_SERVER_METHOD )
	if (HAVE_TLSV1_1_SERVER_METHOD)
		target_compile_definitions ( searchd_ssl PRIVATE HAVE_TLSV1_1_SERVER_METHOD=${HAVE_TLSV1_1_SERVER_METHOD} )
	endif ()
	if (WIN32 AND TARGET OpenSSL::applink)
		target_link_libraries ( searchd_ssl PRIVATE OpenSSL::applink )
		target_compile_options ( OpenSSL::applink INTERFACE $<${CLANGWIN}:-Wno-pedantic -Wno-missing-prototypes> )
	endif ()
endif (WITH_SSL)

# support for replication
add_subdirectory ( replication )
target_link_libraries ( lsearchd PUBLIC repli_stuff)

trace ( lextra )
trace ( lsearchd )

# our executables
add_executable ( searchd searchd.cpp )
target_link_libraries ( searchd searchd_ssl lmanticore lsearchd )

if (NOT STATIC_BINARY)
	add_executable ( indexer indexer.cpp )
	add_executable ( indextool indextool.cpp )
	add_executable ( spelldump spelldump.cpp )
	add_executable ( wordbreaker wordbreaker.cpp )
	add_executable ( index_converter index_converter.cpp )
	target_link_libraries ( indexer indexer_sources lmanticore )
	target_link_libraries ( indextool lmanticore )
	target_link_libraries ( spelldump lmanticore )
	target_link_libraries ( wordbreaker lmanticore )
	target_link_libraries ( index_converter lmanticore )
	add_library ( udfexample MODULE udfexample.c )
	target_compile_options ( udfexample PRIVATE "$<${CLANGWIN}:-Wno-deprecated-declarations>" )
endif ()

if (SPLIT_SYMBOLS)
	include ( split )
	split_dbg ( indexer )
	split_dbg ( indextool )
	split_dbg ( searchd )
	split_dbg ( spelldump )
	split_dbg ( wordbreaker )
	split_dbg ( index_converter )
endif ()

INSTALL ( TARGETS searchd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT searchd )
if (NOT STATIC_BINARY)
	INSTALL ( TARGETS indexer indextool spelldump wordbreaker RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT tools )
	INSTALL ( TARGETS index_converter RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT converter )
	INSTALL ( FILES sphinxudf.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/manticore COMPONENT devel )
endif ()

# install windows runtime libraries - for final targets, and also for immediate testing
if (WIN32)
	set ( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE )
	include ( InstallRequiredSystemLibraries )
	addruntime ( "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}" common )
	getruntime ( libs common )
	install ( PROGRAMS ${libs} DESTINATION bin COMPONENT common )
	unset ( libs )
	getruntime ( libs daemon )
	install ( PROGRAMS ${libs} DESTINATION bin COMPONENT searchd )
	unset ( libs )
	getruntime ( libs indexer )
	install ( PROGRAMS ${libs} DESTINATION bin COMPONENT tools )
	if (BUILD_TESTING)
#		getruntime ( libs indexer ) # is just set above
		getruntime ( libs common )
		getruntime ( libs daemon )
		add_custom_command ( TARGET searchd POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${libs} "$<TARGET_FILE_DIR:searchd>" VERBATIM )
	endif ()
endif ()

# ---------------------------- benching stuff ----------------------------
set ( WITH_BENCHES OFF CACHE BOOL "Build development benches" )
if (WITH_BENCHES)
	add_subdirectory ( gbenches )
	add_feature_info ( benches WITH_BENCHES "development benchmarks" )
endif ()

if (NOT BUILD_TESTING)
	return ()
endif ()

# ---------------------------- Below is pure testing stuff ----------------------------

if (CMAKE_VERSION VERSION_GREATER 3.1.0 AND NOT DISABLE_GTESTS AND NOT TEST_SPECIAL_EXTERNAL)
	add_subdirectory ( gtests )
endif ()

add_executable ( tests EXCLUDE_FROM_ALL tests.cpp )
target_link_libraries ( tests PUBLIC lmanticore )

if (WITH_MYSQL)
	# compile rtestconfig.h
	hsnippet ( rtestconfig testrt.conf )

	set ( DATAFLD "${MANTICORE_BINARY_DIR}/test/data" )
	file ( MAKE_DIRECTORY ${DATAFLD} )
	source_group ( "Testing config" FILES testrt.conf.in )
	add_executable ( testrt testrt.cpp ${MANTICORE_BINARY_DIR}/config/rtestconfig.h )
	target_compile_definitions ( testrt PRIVATE "DATAFLD=\"${DATAFLD}/\"" )
	target_link_libraries ( testrt PRIVATE source_mysql PUBLIC lmanticore $<$<PLATFORM_ID:Windows>:psapi.lib> )
	if (HAVE_RTESTCONFIG_H)
		target_compile_definitions ( testrt PRIVATE "-DHAVE_RTESTCONFIG_H" )
	endif ()

	# testrt is alive, however it needs database 'lj' accessible
	# in order to work. So, avoid it's calling till the base is persist
	# in out CI env
	if (0)

		add_test ( NAME "\"Remove dump index\""
				COMMAND ${CMAKE_COMMAND} -E remove "${DATAFLD}/dump.*" )
		SET_TESTS_PROPERTIES ( "\"Remove dump index\"" PROPERTIES FIXTURES_SETUP TESTRT )

		add_test ( NAME "\"Internal rt src/testrt\""
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND testrt )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt\"" PROPERTIES FIXTURES_REQUIRED TESTRT )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt\"" PROPERTIES RESOURCE_LOCK DbAccess )

		add_test ( NAME "\"Internal rt src/testrt step 1000\""
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND testrt 1000 )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 1000\"" PROPERTIES FIXTURES_REQUIRED TESTRT )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 1000\"" PROPERTIES RESOURCE_LOCK DbAccess )

		add_test ( NAME "\"Internal rt src/testrt step 100\""
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND testrt 100 )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 100\"" PROPERTIES FIXTURES_REQUIRED TESTRT )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 1000\"" PROPERTIES RESOURCE_LOCK DbAccess )

		add_test ( NAME "\"Internal rt src/testrt step 10\""
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND testrt 10 )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 10\"" PROPERTIES FIXTURES_REQUIRED TESTRT )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 1000\"" PROPERTIES RESOURCE_LOCK DbAccess )

		add_test ( NAME "\"Internal rt src/testrt step 1\""
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				COMMAND testrt 1 )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 1\"" PROPERTIES FIXTURES_REQUIRED TESTRT )
		SET_TESTS_PROPERTIES ( "\"Internal rt src/testrt step 1000\"" PROPERTIES RESOURCE_LOCK DbAccess )

	endif (0)
endif (WITH_MYSQL)

if (NOT TEST_SPECIAL_EXTERNAL)

	# since cmake 3.18 names need fixup
	include ( fixup_test_name )

	# first check syntax
	if (NOT WIN32)
		find_package ( PythonInterp QUIET )
		if (PYTHONINTERP_FOUND AND NOT DISABLE_GTESTS)
			fixup_test_name ( RESERVED_TEST "SphinxQL reserved keywords consistency" )
			message ( STATUS "python binary is ${PYTHON_EXECUTABLE}" ) # !COMMIT
			add_test ( NAME ${RESERVED_TEST}
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
					COMMAND ${PYTHON_EXECUTABLE} reserved.py )
			SET_TESTS_PROPERTIES ( ${RESERVED_TEST} PROPERTIES LABELS LINTER )
		endif ()
	endif ()

#	fixup_test_name ( tst "Internal src/tests" )
#	add_test ( NAME ${tst} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMMAND "tests" )
endif ()
