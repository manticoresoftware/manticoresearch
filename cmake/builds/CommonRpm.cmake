# Common rpm-specific build variables
cmake_minimum_required ( VERSION 3.12 )

set ( CPACK_GENERATOR "RPM" )
SET(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
  
set ( CPACK_PACKAGING_INSTALL_PREFIX "/" )
set ( BINPREFIX "usr/" )

set ( CPACK_RPM_PACKAGE_RELEASE 1 )
set ( CPACK_RPM_PACKAGE_RELEASE_DIST ON )
set ( CPACK_RPM_PACKAGE_URL "https://manticoresearch.com/" )
set ( CPACK_RPM_PACKAGE_GROUP "Applications/Internet" )


set ( CPACK_RPM_META_PACKAGE_NAME "manticore" )
set ( CPACK_COMPONENT_META_DESCRIPTION "Meta Package for Manticore Search")


set ( CPACK_RPM_META_PACKAGE_REQUIRES "manticore-server, manticore-tools" )
set ( CPACK_RPM_META_PACKAGE_OBSOLETES "manticore , sphinx")
set ( CPACK_RPM_MAIN_COMPONENT meta)
set ( CPACK_COMPONENT_META_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering. This package is a group package that guarantee you have all is needed 
to run Manticore Search.")
set (CPACK_RPM_META_BUILD_SOURCE_DIRS_PREFIX  OFF)

set ( CPACK_RPM_BIN_PACKAGE_NAME "manticore-server" )
set ( CPACK_RPM_BIN_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_RPM_BIN_INSTALL_WITH_EXEC ON)

set ( CPACK_COMPONENT_BIN_SUMMARY "Manticore Search server files")
set ( CPACK_RPM_BIN_PACKAGE_OBSOLETES "sphinx, manticore <= 3.5.0_200722.1d34c49" )
set ( CPACK_RPM_BIN_PACKAGE_CONFLICTS "sphinx, manticore <= 3.5.0_200722.1d34c49" )
set ( CPACK_COMPONENT_BIN_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering.  This package contains the service daemon.")

set ( CPACK_RPM_TOOLS_PACKAGE_NAME "manticore-tools" )
set ( CPACK_RPM_TOOLS_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_COMPONENT_TOOLS_SUMMARY "Manticore Search utilities")
set ( CPACK_COMPONENT_TOOLS_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering.  This package contains auxiliary tools.")

set ( CPACK_RPM_DEVEL_PACKAGE_NAME "manticore-devel" )
set ( CPACK_RPM_DEVEL_FILE_NAME "RPM-DEFAULT" )



set  ( CPACK_COMPONENT_CONVERTER_DESCRIPTION "This package provides the index_converter tool 
for Manticore Search which converts indexes created with Manticore Search 2.x or Sphinx 2.x to 
Manticore Search 3.x format" )

set ( CPACK_RPM_BIN_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_TOOLS_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_CONVERTER_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_DEVEL_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_META_DEBUGINFO_PACKAGE OFF )

#set ( CPACK_BUILD_SOURCE_DIRS OFF )
string ( LENGTH "${CMAKE_SOURCE_DIR}" source_dir_len_ )
if ( source_dir_len_ LESS 75 )
	message ( STATUS "set src prefix to /tmp/m due to too long path")
	set ( CPACK_RPM_BUILD_SOURCE_DIRS_PREFIX "/tmp")
endif ()


#set ( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "/usr/include" )
 
set ( CPACK_RPM_BIN_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
		"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
   )

set ( CPACK_RPM_BIN_SPEC_MORE_DEFINE
		"%define _scripts ${MANTICORE_BINARY_DIR}
%define manticore_user manticore
%define manticore_group manticore" )

SET ( CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)" )

# add more component 'adm'
set ( CPACK_COMPONENT_ADM_GROUP "bin" )
set ( CPACK_COMPONENT_ADM_DISPLAY_NAME "Scripts and settings" )

# block below used to patch the minconf - add a slash at the end of 'binlog_path' section
file ( READ "manticore.conf.in" _MINCONF LIMIT 10240 )
string ( REPLACE "@CONFDIR@/log/searchd.pid" "@RUNDIR@/searchd.pid" _MINCONF "${_MINCONF}" )
string ( REPLACE "@CONFDIR@/log" "@LOGDIR@" _MINCONF "${_MINCONF}" )
file ( WRITE "${MANTICORE_BINARY_DIR}/manticore.conf.in" "${_MINCONF}" )
unset (_MINCONF)

# this values set for correct substitution in configure files below
set ( CONFDIR "${LOCALSTATEDIR}/lib/manticore" )
set ( RUNDIR "${LOCALSTATEDIR}/run/manticore" )
set ( LOGDIR "${LOCALSTATEDIR}/log/manticore" )

configure_file ( "${MANTICORE_BINARY_DIR}/manticore.conf.in" "${MANTICORE_BINARY_DIR}/manticore.conf.dist" @ONLY )

configure_file ( "dist/rpm/manticore.logrotate.in" "${MANTICORE_BINARY_DIR}/manticore.logrotate" @ONLY )

configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm/manticore_s.post.in"
		"${MANTICORE_BINARY_DIR}/manticore_s.post" @ONLY )
configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm/manticore.post.in"
		"${MANTICORE_BINARY_DIR}/manticore.post" @ONLY )

set ( SCR "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm" ) # a shortcut
if ( WITH_SYSTEMD )
	set ( CPACK_RPM_BIN_BUILDREQUIRES "systemd-units" )
	set ( CPACK_RPM_BIN_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore_s.post")
	set ( CPACK_RPM_BIN_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun" )
	set ( CPACK_RPM_BIN_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun" )

	configure_file ( "dist/rpm/manticore.tmpfiles.in" "${MANTICORE_BINARY_DIR}/manticore.conf" @ONLY )
	configure_file ( "dist/rpm/manticore.generator.in" "${MANTICORE_BINARY_DIR}/manticore-search-generator" @ONLY )
	install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION usr/lib/tmpfiles.d COMPONENT adm )
	install ( PROGRAMS ${MANTICORE_BINARY_DIR}/manticore-search-generator
			DESTINATION usr/lib/systemd/system-generators COMPONENT adm )

else ()
	set ( CPACK_RPM_BIN_PACKAGE_REQUIRES_POST chkconfig )
	set ( CPACK_RPM_BIN_PACKAGE_REQUIRES_PREUN "chkconfig, initscripts" )
	set ( CPACK_RPM_BIN_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore.post")
	set ( CPACK_RPM_BIN_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore.preun" )
	configure_file ( "dist/rpm/manticore.init.in" "${MANTICORE_BINARY_DIR}/manticore" @ONLY )
	install ( PROGRAMS ${MANTICORE_BINARY_DIR}/manticore
			DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/rc.d/init.d COMPONENT adm )

endif ()

install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf.dist
		DESTINATION usr/${CMAKE_INSTALL_DOCDIR} COMPONENT doc )

install ( FILES COPYING example.sql DESTINATION usr/${CMAKE_INSTALL_DOCDIR} COMPONENT doc )


install ( FILES doc/indexer.1 doc/spelldump.1
		DESTINATION usr/${CMAKE_INSTALL_MANDIR}/man1 COMPONENT tools )

install ( FILES doc/searchd.1 DESTINATION usr/${CMAKE_INSTALL_MANDIR}/man1 COMPONENT applications )


install ( DIRECTORY misc/stopwords DESTINATION usr/${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME} COMPONENT doc )
if (USE_ICU)
	install ( FILES ${ICU_DATA} DESTINATION usr/${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME}/icu COMPONENT doc)
endif()

if (NOT NOAPI)
install ( DIRECTORY api DESTINATION usr/${CMAKE_INSTALL_DATADIR}/manticore COMPONENT doc )
endif()


install ( FILES ${MANTICORE_BINARY_DIR}/manticore.logrotate
		DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT adm RENAME manticore)

install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf.dist
		DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT adm RENAME manticore.conf )

install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore COMPONENT adm )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/run/manticore COMPONENT adm )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT adm )

install ( FILES INSTALL   DESTINATION usr/${CMAKE_INSTALL_DATADIR}/manticore  COMPONENT meta )

set ( CONFFILEDIR "${SYSCONFDIR}/manticoresearch" )

