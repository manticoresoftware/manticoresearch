# Common rpm-specific build variables
cmake_minimum_required ( VERSION 3.12 )

set ( CPACK_GENERATOR "RPM" )

set ( CPACK_PACKAGING_INSTALL_PREFIX "/" )
set ( BINPREFIX "usr/" )

set ( CPACK_RPM_MAIN_COMPONENT bin )
set ( CPACK_RPM_PACKAGE_RELEASE 2 )
set ( CPACK_RPM_PACKAGE_RELEASE_DIST ON )

set ( CPACK_RPM_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_DEBUGINFO_SINGLE_PACKAGE ON )
#set ( CPACK_BUILD_SOURCE_DIRS OFF )

string ( LENGTH "${CMAKE_SOURCE_DIR}" source_dir_len_ )
if ( source_dir_len_ LESS 75 )
	message ( STATUS "set src prefix to /tmp/m due to too long path")
	set ( CPACK_RPM_BUILD_SOURCE_DIRS_PREFIX "/tmp")
endif ()

set ( CPACK_RPM_PACKAGE_URL "https://manticoresearch.com/" )
set ( CPACK_RPM_PACKAGE_GROUP "Applications/Text" )
set ( CPACK_RPM_PACKAGE_OBSOLETES "sphinx" )
set ( CPACK_RPM_PACKAGE_DESCRIPTION "Manticore is a full-text search server that adds many advanced features
on top of plain old text searching. Data can be fetched directly from
a database, or streamed in XML format. MySQL, PostgreSQL, SQL Server,
Oracle, and other databases are supported. Client programs can query
Sphinx either using native SphinxAPI library, or using regular MySQL
client programs and libraries via SQL-like SphinxQL interface.")

#set ( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "/usr/include" )
set ( CPACK_RPM_BIN_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
		"%config(noreplace) %{_sysconfdir}/sphinx/sphinx.conf" )

set ( CPACK_RPM_SPEC_MORE_DEFINE
		"%define _scripts ${MANTICORE_BINARY_DIR}
%define manticore_user manticore
%define manticore_group manticore" )

SET ( CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)" )

# add more component 'adm'
set ( CPACK_COMPONENT_ADM_GROUP "bin" )
set ( CPACK_COMPONENT_ADM_DISPLAY_NAME "Scripts and settings" )

# block below used to patch the minconf - add a slash at the end of 'binlog_path' section
file ( READ "sphinx-min.conf.in" _MINCONF LIMIT 10240 )
string ( REGEX REPLACE "(binlog_path[ \t]+\\=[ \t]+\\@CONFDIR\\@/data)" "\\1/" _MINCONF "${_MINCONF}" )
string ( REGEX REPLACE "(log/searchd.pid)" "run/manticore/searchd.pid" _MINCONF "${_MINCONF}" )
string ( REGEX REPLACE "(@CONFDIR@/log/)" "@CONFDIR@/log/manticore/" _MINCONF "${_MINCONF}" )
string ( REGEX REPLACE "(@CONFDIR@/data/)" "@CONFDIR@/lib/manticore/" _MINCONF "${_MINCONF}" )
file ( WRITE "${MANTICORE_BINARY_DIR}/sphinx-min.conf.in" "${_MINCONF}")
unset (_MINCONF)

# this values set for correct substitution in configure files below
set ( CONFDIR "${LOCALSTATEDIR}" )
set ( RUNDIR "${LOCALSTATEDIR}/run/manticore" )
set ( LOGDIR "${LOCALSTATEDIR}/log/manticore" )

configure_file ( "${MANTICORE_BINARY_DIR}/sphinx-min.conf.in" "${MANTICORE_BINARY_DIR}/sphinx-min.conf.dist" @ONLY )
configure_file ( "sphinx.conf.in" "${MANTICORE_BINARY_DIR}/sphinx.conf.dist" @ONLY )

configure_file ( "dist/rpm/manticore.logrotate.in" "${MANTICORE_BINARY_DIR}/manticore.logrotate" @ONLY )

set ( SCR "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm" ) # a shortcut
if ( WITH_SYSTEMD )
	set ( CPACK_RPM_BUILDREQUIRES "systemd-units" )
	set ( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${SCR}/manticore_s.post" )
	set ( CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun" )
	set ( CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun" )

	configure_file ( "dist/rpm/manticore.tmpfiles.in" "${MANTICORE_BINARY_DIR}/searchd.conf" @ONLY )
	configure_file ( "dist/rpm/manticore.generator.in" "${MANTICORE_BINARY_DIR}/manticore-search-generator" @ONLY )
	install ( FILES ${MANTICORE_BINARY_DIR}/searchd.conf DESTINATION usr/lib/tmpfiles COMPONENT adm )
	install ( PROGRAMS ${MANTICORE_BINARY_DIR}/manticore-search-generator
			DESTINATION usr/lib/systemd/system-generators COMPONENT adm )

else ()
	set ( CPACK_RPM_PACKAGE_REQUIRES_POST chkconfig )
	set ( CPACK_RPM_PACKAGE_REQUIRES_PREUN "chkconfig, initscripts" )
	set ( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${SCR}/manticore.post" )
	set ( CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore.preun" )
	configure_file ( "dist/rpm/manticore.init.in" "${MANTICORE_BINARY_DIR}/searchd" @ONLY )
	install ( PROGRAMS ${MANTICORE_BINARY_DIR}/searchd
			DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/rc.d/init.d COMPONENT adm )

endif ()

install ( FILES ${MANTICORE_BINARY_DIR}/sphinx-min.conf.dist
		${MANTICORE_BINARY_DIR}/sphinx.conf.dist
		DESTINATION usr/${CMAKE_INSTALL_DOCDIR} COMPONENT doc )

install ( FILES COPYING example.sql DESTINATION usr/${CMAKE_INSTALL_DOCDIR} COMPONENT doc )


install ( FILES doc/indexer.1 doc/indextool.1 doc/searchd.1 doc/spelldump.1
		DESTINATION usr/${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc )

install ( DIRECTORY misc/stopwords DESTINATION usr/${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME} COMPONENT doc )

install ( DIRECTORY api DESTINATION usr/${CMAKE_INSTALL_DATADIR}/manticore COMPONENT doc )



install ( FILES ${MANTICORE_BINARY_DIR}/manticore.logrotate
		DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT adm RENAME manticore)

install ( FILES ${MANTICORE_BINARY_DIR}/sphinx-min.conf.dist
		DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/sphinx COMPONENT adm RENAME sphinx.conf )

install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore COMPONENT adm )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/run/manticore COMPONENT adm )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT adm )

set ( CONFFILEDIR "${CMAKE_INSTALL_SYSCONFDIR}/sphinx" )
