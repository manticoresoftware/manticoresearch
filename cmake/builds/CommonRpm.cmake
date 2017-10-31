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

set ( CONFDIR "${LOCALSTATEDIR}" )
set ( RUNDIR "${LOCALSTATEDIR}/run/manticore" )
set ( LOGDIR "${LOCALSTATEDIR}/log/manticore" )

configure_file ( "${MANTICORE_BINARY_DIR}/sphinx-min.conf.in" "${MANTICORE_BINARY_DIR}/sphinx-min.conf.dist" @ONLY )
configure_file ( "sphinx.conf.in" "${MANTICORE_BINARY_DIR}/sphinx.conf.dist" @ONLY )

configure_file ( "dist/rpm/manticore.init.in" "${MANTICORE_BINARY_DIR}/manticore.init" @ONLY )
configure_file ( "dist/rpm/searchd.conf.in" "${MANTICORE_BINARY_DIR}/searchd.conf" @ONLY )
configure_file ( "dist/rpm/searchd.service.in" "${MANTICORE_BINARY_DIR}/searchd.service" @ONLY )

install ( FILES ${MANTICORE_BINARY_DIR}/sphinx-min.conf.dist
		${MANTICORE_BINARY_DIR}/sphinx.conf.dist
		DESTINATION ${DOCDIR} COMPONENT doc )
install ( FILES doc/indexer.1 doc/indextool.1 doc/searchd.1 doc/spelldump.1
		DESTINATION share/man/man1 COMPONENT doc )
install ( DIRECTORY api DESTINATION share/${PACKAGE_NAME} COMPONENT doc )
install ( FILES COPYING example.sql
		DESTINATION ${DOCDIR} COMPONENT doc )

# Add one more component group
set ( CPACK_COMPONENT_ADM_GROUP "bin" )
set ( CPACK_COMPONENT_ADM_DISPLAY_NAME "Helper scripts" )

install ( DIRECTORY DESTINATION ${LOCALSTATEDIR}/lib/manticore COMPONENT adm )
install ( DIRECTORY DESTINATION ${LOCALSTATEDIR}/run/manticore COMPONENT adm )
install ( DIRECTORY DESTINATION ${LOCALSTATEDIR}/log/manticore COMPONENT adm )

set ( CONFFILEDIR "${SYSCONFDIR}/sphinx" )