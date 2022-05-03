# only cmake since 3.17 supports packaging of debuginfo
cmake_minimum_required ( VERSION 3.17 )

if (NOT installed)
	# start with short route - set all paths
	set ( CPACK_PACKAGING_INSTALL_PREFIX "/" )
	set ( CMAKE_INSTALL_PREFIX "${CPACK_PACKAGING_INSTALL_PREFIX}" CACHE PATH "prefix from distr build" FORCE )
	include ( GNUInstallDirs )
	SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/manticore/data" ) # will be used also in the app
	set ( installed ON )
	diag ( CPACK_PACKAGING_INSTALL_PREFIX CMAKE_INSTALL_PREFIX )
endif ()

if (only_set_paths)
	return ()
endif ()

# Common rpm-specific build variables
set ( CPACK_GENERATOR "RPM" )

set ( CPACK_RPM_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_RPM_PACKAGE_RELEASE 1 )
set ( CPACK_RPM_PACKAGE_RELEASE_DIST ON )
set ( CPACK_RPM_PACKAGE_GROUP "Applications/Internet" )

set ( CPACK_RPM_MAIN_PACKAGE_NAME "manticore" )

if (NOT CPACK_RPM_META_PACKAGE_NAME)
	set ( CPACK_RPM_META_PACKAGE_NAME "manticore-all" )
endif ()
set ( CPACK_RPM_META_PACKAGE_REQUIRES "manticore-systemd, manticore-tools" )
set ( CPACK_RPM_META_PACKAGE_CONFLICTS "manticore, sphinx" )
set ( CPACK_RPM_META_PACKAGE_ARCHITECTURE noarch )
set ( CPACK_RPM_META_BUILD_SOURCE_DIRS_PREFIX OFF )

set ( CPACK_RPM_SERVER_PACKAGE_NAME "manticore-server" )
set ( CPACK_RPM_SERVER_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_RPM_SERVER_INSTALL_WITH_EXEC ON )
set ( CPACK_RPM_SERVER_PACKAGE_OBSOLETES "sphinx" )
set ( CPACK_RPM_SERVER_PACKAGE_CONFLICTS "sphinx" )

set ( CPACK_RPM_SYSTEMD_PACKAGE_NAME "manticore-systemd" )
set ( CPACK_RPM_SYSTEMD_PACKAGE_REQUIRES "manticore-server" )
set ( CPACK_RPM_SYSTEMD_PACKAGE_ARCHITECTURE noarch )
set ( CPACK_RPM_SYSTEMD_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_RPM_SYSTEMD_PACKAGE_DEBUG "OFF" )

set (CPACK_RPM_ICUDATA_PACKAGE_NAME "manticore-icudata")
set (CPACK_RPM_ICUDATA_FILE_NAME "RPM-DEFAULT")
set (CPACK_RPM_ICUDATA_PACKAGE_ARCHITECTURE noarch)
# other vars defined in CommonInfo

set ( CPACK_RPM_TOOLS_PACKAGE_NAME "manticore-tools" )
set ( CPACK_RPM_TOOLS_PACKAGE_CONFLICTS "sphinx, manticore <= 3.5.0_200722.1d34c49" )
set ( CPACK_RPM_TOOLS_FILE_NAME "RPM-DEFAULT" )

set ( CPACK_RPM_DEVEL_PACKAGE_NAME "manticore-devel" )
set ( CPACK_RPM_DEVEL_FILE_NAME "RPM-DEFAULT" )
set ( CPACK_RPM_DEVEL_PACKAGE_ARCHITECTURE noarch )

set ( CPACK_RPM_MAIN_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_SERVER_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_TOOLS_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_CONVERTER_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_ICUDATA_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_DEVEL_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_META_DEBUGINFO_PACKAGE OFF )

#set ( CPACK_BUILD_SOURCE_DIRS OFF )
string ( LENGTH "${CMAKE_SOURCE_DIR}" source_dir_len_ )
if (source_dir_len_ LESS 75)
	message ( STATUS "set src prefix to /tmp/m due to too long path" )
	set ( CPACK_RPM_BUILD_SOURCE_DIRS_PREFIX "/tmp" )
endif ()

#set ( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "/usr/include" )
set ( CPACK_RPM_SERVER_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
		)
set ( CPACK_RPM_SYSTEMD_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
		)
set ( CPACK_RPM_MAIN_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
		"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
		)

set ( CPACK_RPM_SPEC_MORE_DEFINE
		"%define _scripts ${MANTICORE_BINARY_DIR}
%define manticore_user manticore
%define manticore_group manticore" )

SET ( CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)" )

set ( SCR "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm" ) # a shortcut

# systemd - on components install
set ( CPACK_RPM_SYSTEMD_BUILDREQUIRES "systemd-units" )
set ( CPACK_RPM_SYSTEMD_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/config/systemd/manticore_s.post" )
set ( CPACK_RPM_SYSTEMD_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun" )
set ( CPACK_RPM_SYSTEMD_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun" )

# server - on components install
set ( CPACK_RPM_SERVER_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/config/server/manticore_s.post" )

# main - on common install (group)
set ( CPACK_RPM_MAIN_BUILDREQUIRES "systemd-units" )
set ( CPACK_RPM_MAIN_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore_s.post" )
set ( CPACK_RPM_MAIN_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun" )
set ( CPACK_RPM_MAIN_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun" )

# now get system paths. These variables are used in configure substitutions below

# block below used to patch the minconf - add a slash at the end of 'binlog_path' section
configure_config ( lib/manticore )

configure_file ( dist/rpm/manticore.logrotate.in "${MANTICORE_BINARY_DIR}/manticore.logrotate" @ONLY )
configure_file ( dist/rpm/manticore_s.post.in "${MANTICORE_BINARY_DIR}/manticore_s.post" @ONLY )
configure_file ( dist/rpm/manticore_s-server.post.in "${MANTICORE_BINARY_DIR}/config/server/manticore_s.post" @ONLY )
configure_file ( dist/rpm/manticore_s-systemd.post.in "${MANTICORE_BINARY_DIR}/config/systemd/manticore_s.post" @ONLY )

configure_file ( "dist/rpm/manticore.tmpfiles.in" "${MANTICORE_BINARY_DIR}/manticore.tmpfiles" @ONLY )
configure_file ( "dist/rpm/manticore.generator.in" "${MANTICORE_BINARY_DIR}/manticore-search-generator" @ONLY )

# installation

# stuff going to /etc
# CMAKE_INSTALL_SYSCONFDIR					etc 					/etc
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT server )
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.logrotate DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT systemd RENAME manticore )


# stuff going to /var
# CMAKE_INSTALL_LOCALSTATEDIR				var 					/var
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore/data COMPONENT server )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_RUNSTATEDIR}/manticore COMPONENT server )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT server )

set ( CMAKE_INSTALL_LIB "lib" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_LIB CMAKE_INSTALL_LIB LIBDIR )

# stuff that should go to /lib -> actually to /usr/lib
# CMAKE_INSTALL_LIBDIR						usr/lib64 				/usr/lib64
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.tmpfiles DESTINATION ${CMAKE_INSTALL_LIB}/tmpfiles.d COMPONENT server RENAME manticore.conf )
install ( PROGRAMS ${MANTICORE_BINARY_DIR}/manticore-search-generator DESTINATION ${CMAKE_INSTALL_LIB}/systemd/system-generators COMPONENT systemd )


# binaries go to /usr/bin (here is only new cluster, rest is in main file, installing targets)
# CMAKE_INSTALL_BINDIR						usr/bin 				/usr/bin
install ( PROGRAMS "dist/rpm/manticore_new_cluster" DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT systemd )


#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# stuff going to /usr/share/man, /usr/share/doc
# CMAKE_INSTALL_MANDIR						usr/share/man 			/usr/share/man
# CMAKE_INSTALL_DOCDIR						usr/share/doc/manticore /usr/share/doc/manticore
install ( FILES doc/indexer.1 doc/indextool.1 doc/spelldump.1 doc/wordbreaker.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT tools )
install ( FILES doc/searchd.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT server )
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT server RENAME manticore.conf.dist )
install ( FILES COPYING example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT server )

# stuff going to /usr/share/manticore
# CMAKE_INSTALL_DATAROOTDIR					usr/share 				/usr/share
# CMAKE_INSTALL_DATADIR						usr/share 				/usr/share
install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server )
install ( FILES INSTALL DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules COMPONENT server )

if (WITH_ICU)
	install_icudata ( ${FULL_SHARE_DIR}/icu )
endif ()

if (NOT NOAPI)
	install ( DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server )
endif ()

if (SPLIT)
	get_cmake_property ( CPACK_COMPONENTS_ALL COMPONENTS )
	list ( APPEND CPACK_COMPONENTS_ALL "meta" )
endif ()
