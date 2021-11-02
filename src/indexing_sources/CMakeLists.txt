cmake_minimum_required (VERSION 3.17)

FOREACH (policy CMP0054 CMP0057 CMP0042 CMP0074 CMP0110)
	IF (POLICY ${policy})
		CMAKE_POLICY (SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

include(printers)
debugmsg ("Inside directory src/indexing sources...")

# generic source - used everywhere
add_library ( sourcedoc source_document.cpp source_document.h )
target_include_directories ( sourcedoc PUBLIC "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( sourcedoc PRIVATE stripper conversion PUBLIC lextra )

add_library ( indexer_sources INTERFACE )
target_link_libraries ( indexer_sources INTERFACE sourcedoc )

# tsv/csv pipe
add_library (source_svpipe source_svpipe.cpp source_svpipe.h)
target_link_libraries (source_svpipe PUBLIC lextra)
target_link_libraries (indexer_sources INTERFACE source_svpipe)

# generic sql
add_library (source_sql source_sql.cpp source_sql.h)
target_link_libraries (source_sql PRIVATE lextra)

if (WITH_ZLIB)
	target_link_libraries (source_sql PRIVATE ZLIB::ZLIB)
endif ()

# mssql, unixodbc
if (WITH_ODBC)
	add_library (source_odbc source_odbc.cpp source_odbc.h)
	target_link_libraries (source_odbc PRIVATE lextra source_sql)
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
	target_link_libraries (source_pgsql PRIVATE lextra source_sql)

	if (DL_POSTGRESQL)
		target_link_libraries (source_pgsql PRIVATE PostgreSQL::PostgreSQL_ld)
	else ()
		target_link_libraries (source_pgsql PRIVATE PostgreSQL::PostgreSQL)
	endif ()

	target_link_libraries (indexer_sources INTERFACE source_pgsql)
endif ()

#mysql
if (WITH_MYSQL)
	add_library (source_mysql source_mysql.cpp source_mysql.h )
	target_link_libraries (source_mysql PRIVATE lextra source_sql)
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
	target_link_libraries (source_xmlpipe2 PRIVATE lextra)
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
