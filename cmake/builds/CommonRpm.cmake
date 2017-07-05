# Common rpm-specific build variables
set ( CPACK_GENERATOR "RPM" )
set ( CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm/manticore.spec.in" )
# doesn't work anyway
#set ( CPACK_RPM_bin_USER_BINARY_SPECFILE "" )

IF ( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
	SET ( CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64" )
ELSEIF ( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686" )
	SET ( CPACK_RPM_PACKAGE_ARCHITECTURE "i386" )
ENDIF ( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )

SET ( CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)" )


# block below used to patch the minconf - add a slash at the end of 'binlog_path' section
file ( READ "sphinx-min.conf.in" _MINCONF LIMIT 10240 )
string ( REGEX REPLACE "(binlog_path[ \t]+\\=[ \t]+\\@CONFDIR\\@/data)" "\\1/" _MINCONF "${_MINCONF}" )
file ( WRITE "${MANTICORE_BINARY_DIR}/sphinx-min.conf.in" "${_MINCONF}")
unset (_MINCONF)

set ( CONFDIR "/var" )
configure_file ( "${MANTICORE_BINARY_DIR}/sphinx-min.conf.in" "${MANTICORE_BINARY_DIR}/sphinx-min.conf.dist" @ONLY )
configure_file ( "sphinx.conf.in" "${MANTICORE_BINARY_DIR}/sphinx.conf.dist" @ONLY )

install ( FILES ${MANTICORE_BINARY_DIR}/sphinx-min.conf.dist
		${MANTICORE_BINARY_DIR}/sphinx.conf.dist
		DESTINATION share/doc/${PACKAGE_NAME} COMPONENT doc )
install ( FILES doc/indexer.1 doc/indextool.1 doc/searchd.1 doc/spelldump.1
		DESTINATION share/man/man1 COMPONENT doc )
install ( DIRECTORY api DESTINATION share/${PACKAGE_NAME} COMPONENT doc )
install ( FILES COPYING doc/sphinx.html doc/sphinx.txt example.sql
		DESTINATION share/doc/${PACKAGE_NAME} COMPONENT doc )
