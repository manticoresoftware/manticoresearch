cmake_minimum_required ( VERSION 3.20 )

add_library ( daemon OBJECT
		api_search.cpp
		api_search.h
		crash_logger.cpp
		crash_logger.h
		daemon_log.cpp
		failures_log.cpp
		failures_log.h
		logger.h
		match_iterator.cpp
		match_iterator.h
		minimize_aggr_result.cpp
		minimize_aggr_result.h
		http_log.cpp
		query_log.cpp
		search_handler.cpp
		search_handler.h
		winservice.cpp
		winservice.h
		api_commands.h
)
target_include_directories ( daemon PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( daemon PRIVATE lextra )
