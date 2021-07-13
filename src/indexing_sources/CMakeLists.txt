cmake_minimum_required (VERSION 3.17)

FOREACH (policy CMP0054 CMP0057 CMP0042 CMP0074 CMP0110)
	IF (POLICY ${policy})
		CMAKE_POLICY (SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

include(printers)
debugmsg ("Inside directory src/indexing sources...")

add_library (common_includes INTERFACE)
target_include_directories (common_includes INTERFACE "${MANTICORE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (common_includes INTERFACE lextra)

add_library (indexer_sources INTERFACE)

# tsv/csv pipe
add_library (source_svpipe source_svpipe.cpp source_svpipe.h)
target_link_libraries (source_svpipe PUBLIC common_includes)
target_link_libraries (indexer_sources INTERFACE source_svpipe)

# generic sql
add_library (source_sql source_sql.cpp source_sql.h)
target_link_libraries (source_sql PRIVATE common_includes)

if (WITH_ZLIB)
	target_link_libraries (source_sql PRIVATE ZLIB::ZLIB)
endif ()

# mssql, unixodbc
if (WITH_ODBC)
	add_library (source_odbc source_odbc.cpp source_odbc.h)
	target_link_libraries (source_odbc PRIVATE common_includes source_sql)
	if (DL_ODBC)
		target_link_libraries (source_odbc PRIVATE ODBC::ODBC_ld)
	else ()
		target_link_libraries (source_odbc PRIVATE ODBC::ODBC)
	endif ()

	target_link_libraries(indexer_sources INTERFACE source_odbc)
#	target_sources (indexer_sources $<TARGET_OBJECTS:source_odbc>)
endif ()

#postgresql
if (WITH_POSTGRESQL)
	add_library (source_pgsql source_pgsql.cpp source_pgsql.h)
	target_link_libraries (source_pgsql PRIVATE common_includes source_sql)

	if (DL_POSTGRESQL)
		target_link_libraries (source_pgsql PRIVATE PostgreSQL::PostgreSQL_ld)
	else ()
		target_link_libraries (source_pgsql PRIVATE PostgreSQL::PostgreSQL)
	endif ()

	target_link_libraries (indexer_sources INTERFACE source_pgsql)
endif ()

#mysql
if (WITH_MYSQL)
	add_library (source_mysql source_mysql.cpp source_mysql.h)
	target_link_libraries (source_mysql PRIVATE common_includes source_sql)
	if (DL_MYSQL)
		target_link_libraries (source_mysql PRIVATE Mysql::Mysql_ld)
	else ()
		target_link_libraries (source_mysql PRIVATE Mysql::Mysql)
	endif ()

	target_link_libraries (indexer_sources INTERFACE source_mysql)
endif ()

#xmlpipe (with or without iconv)
if (WITH_EXPAT)
	add_library (source_xmlpipe2 source_xmlpipe2.cpp source_xmlpipe2.h)
	target_link_libraries (source_xmlpipe2 PRIVATE common_includes)
	if (DL_EXPAT)
		target_link_libraries (source_xmlpipe2 PRIVATE EXPAT::EXPAT_ld)
	else ()
		target_link_libraries (source_xmlpipe2 PRIVATE EXPAT::EXPAT)
	endif ()
	if (WITH_ICONV)
		if (DL_ICONV)
			target_link_libraries (source_xmlpipe2 PRIVATE Iconv::Iconv_ld)
		else ()
			target_link_libraries (source_xmlpipe2 PRIVATE Iconv::Iconv)
		endif ()
	endif ()

	target_link_libraries (indexer_sources INTERFACE source_xmlpipe2)
endif ()
