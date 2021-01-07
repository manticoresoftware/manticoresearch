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

set ( CPACK_RPM_MAIN_PACKAGE_NAME "manticore")

set ( CPACK_RPM_META_PACKAGE_NAME "manticore-all" )
set ( CPACK_COMPONENT_META_DESCRIPTION "Meta Package for Manticore Search")
set ( CPACK_RPM_META_PACKAGE_REQUIRES "manticore-server, manticore-tools" )
set ( CPACK_RPM_META_PACKAGE_CONFLICTS "manticore, sphinx")

set ( CPACK_COMPONENT_META_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering. This package is a group package that guarantee you have all is needed 
to run Manticore Search.")
set (CPACK_RPM_META_BUILD_SOURCE_DIRS_PREFIX  OFF)

set ( CPACK_RPM_APPLICATIONS_PACKAGE_NAME "manticore-server" )
set ( CPACK_RPM_APPLICATIONS_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_RPM_APPLICATIONS_INSTALL_WITH_EXEC ON)
set ( CPACK_RPM_APPLICATIONS_PACKAGE_OBSOLETES "sphinx" )
set ( CPACK_RPM_APPLICATIONS_PACKAGE_CONFLICTS "sphinx" )
if ( "${DISTR_BUILD}" STREQUAL "rhel8" )
    set ( CPACK_RPM_APPLICATIONS_PACKAGE_SUGGESTS "manticore-icudata" )
endif()
set ( CPACK_COMPONENT_APPLICATIONS_SUMMARY "Manticore Search server files")
set ( CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering.  This package contains the service daemon.")


set ( CPACK_RPM_ICUDATA_PACKAGE_NAME "manticore-icudata" )
set ( CPACK_RPM_ICUDATA_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_COMPONENT_ICUDATA_SUMMARY "Manticore Search ICU files")
set ( CPACK_COMPONENT_ICUDATA_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering.  This package contains the ICU DAT file. You need this package only if you use the ICU morphology.")

set ( CPACK_RPM_TOOLS_PACKAGE_NAME "manticore-tools" )
set ( CPACK_RPM_TOOLS_PACKAGE_CONFLICTS "sphinx, manticore <= 3.5.0_200722.1d34c49" )
if ( "${DISTR_BUILD}" STREQUAL "rhel8" )
 set ( CPACK_RPM_TOOLS_PACKAGE_SUGGESTS "manticore-icudata" )
endif()
set ( CPACK_RPM_TOOLS_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_COMPONENT_TOOLS_SUMMARY "Manticore Search utilities")
set ( CPACK_COMPONENT_TOOLS_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering.  This package contains auxiliary tools.")

set ( CPACK_RPM_DEVEL_PACKAGE_NAME "manticore-devel" )
set ( CPACK_RPM_DEVEL_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_COMPONENT_TOOLS_SUMMARY "Manticore Search development headers")
set ( CPACK_COMPONENT_TOOLS_DESCRIPTION "Manticore Search is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering. This package contains the headers for writing custom functions.")

set ( CPACK_COMPONENT_CONVERTER_SUMMARY "Manticore Search index converter")
set  ( CPACK_COMPONENT_CONVERTER_DESCRIPTION "This package provides the index_converter tool 
for Manticore Search which converts indexes created with Manticore Search 2.x or Sphinx 2.x to 
Manticore Search 3.x format." )
set ( CPACK_RPM_MAIN_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_APPLICATIONS_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_TOOLS_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_CONVERTER_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_ICUDATA_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_DEVEL_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_META_DEBUGINFO_PACKAGE OFF )

#set ( CPACK_BUILD_SOURCE_DIRS OFF )
string ( LENGTH "${CMAKE_SOURCE_DIR}" source_dir_len_ )
if ( source_dir_len_ LESS 75 )
	message ( STATUS "set src prefix to /tmp/m due to too long path")
	set ( CPACK_RPM_BUILD_SOURCE_DIRS_PREFIX "/tmp")
endif ()


#set ( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "/usr/include" )
if (SPLIT)
set ( CPACK_RPM_APPLICATIONS_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
		"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
   )
else(SPLIT)
set ( CPACK_RPM_MAIN_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
		"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
   )
endif(SPLIT)
set ( CPACK_RPM_SPEC_MORE_DEFINE
		"%define _scripts ${MANTICORE_BINARY_DIR}
%define manticore_user manticore
%define manticore_group manticore" )

SET ( CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)" )

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
    if (SPLIT)
        set ( CPACK_RPM_APPLICATIONS_BUILDREQUIRES "systemd-units" )
        set ( CPACK_RPM_APPLICATIONS_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore_s.post")
        set ( CPACK_RPM_APPLICATIONS_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun" )
        set ( CPACK_RPM_APPLICATIONS_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun" )
    else(SPLIT)
        set ( CPACK_RPM_BUILDREQUIRES "systemd-units" )
        set ( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore_s.post")
        set ( CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun" )
        set ( CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun" )
    endif(SPLIT)

	configure_file ( "dist/rpm/manticore.tmpfiles.in" "${MANTICORE_BINARY_DIR}/manticore.conf" @ONLY )
	configure_file ( "dist/rpm/manticore.generator.in" "${MANTICORE_BINARY_DIR}/manticore-search-generator" @ONLY )
	install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION usr/lib/tmpfiles.d COMPONENT applications )
	install ( PROGRAMS ${MANTICORE_BINARY_DIR}/manticore-search-generator
			DESTINATION usr/lib/systemd/system-generators COMPONENT applications )

else (WITH_SYSTEMD)
    if (SPLIT)
        set ( CPACK_RPM_APPLICATIONS_PACKAGE_REQUIRES_POST chkconfig )
        set ( CPACK_RPM_APPLICATIONS_PACKAGE_REQUIRES_PREUN "chkconfig, initscripts" )
        set ( CPACK_RPM_APPLICATIONS_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore.post")
        set ( CPACK_RPM_APPLICATIONS_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore.preun" )
    else (SPLIT)
        set ( CPACK_RPM_PACKAGE_REQUIRES_POST chkconfig )
        set ( CPACK_RPM_PACKAGE_REQUIRES_PREUN "chkconfig, initscripts" )
        set ( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore.post")
        set ( CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore.preun" )
    endif (SPLIT)
	configure_file ( "dist/rpm/manticore.init.in" "${MANTICORE_BINARY_DIR}/manticore" @ONLY )
	install ( PROGRAMS ${MANTICORE_BINARY_DIR}/manticore
			DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/rc.d/init.d COMPONENT applications )

endif ()

install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf.dist
		DESTINATION usr/${CMAKE_INSTALL_DOCDIR} COMPONENT applications )

install ( FILES COPYING example.sql DESTINATION usr/${CMAKE_INSTALL_DOCDIR} COMPONENT applications )


install ( FILES doc/indexer.1 doc/spelldump.1
		DESTINATION usr/${CMAKE_INSTALL_MANDIR}/man1 COMPONENT tools )

install ( FILES doc/searchd.1 DESTINATION usr/${CMAKE_INSTALL_MANDIR}/man1 COMPONENT applications )


install ( DIRECTORY misc/stopwords DESTINATION usr/${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME} COMPONENT applications )
if (USE_ICU)
	install ( FILES ${ICU_DATA} DESTINATION usr/${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME}/icu COMPONENT icudata)
endif()

if (NOT NOAPI)
install ( DIRECTORY api DESTINATION usr/${CMAKE_INSTALL_DATADIR}/manticore COMPONENT applications )
endif()


install ( FILES ${MANTICORE_BINARY_DIR}/manticore.logrotate
		DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT applications RENAME manticore)

install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf.dist
		DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT applications RENAME manticore.conf )

install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore COMPONENT applications )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/run/manticore COMPONENT applications )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT applications )

install ( FILES INSTALL   DESTINATION usr/${CMAKE_INSTALL_DATADIR}/manticore  COMPONENT meta )

set ( CONFFILEDIR "${SYSCONFDIR}/manticoresearch" )

