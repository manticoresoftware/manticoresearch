cmake_minimum_required ( VERSION 3.20 )

add_library ( daemon OBJECT
		crash_logger.cpp
		crash_logger.h
		daemon_log.cpp
		failures_log.cpp
		failures_log.h
		logger.h
		http_log.cpp
		winservice.cpp
		winservice.h
)
target_include_directories ( daemon PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( daemon PRIVATE lextra )
