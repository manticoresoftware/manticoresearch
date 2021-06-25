# Common rpm-specific build variables
cmake_minimum_required (VERSION 3.17)

set (CPACK_GENERATOR "RPM")
set (CPACK_PACKAGING_INSTALL_PREFIX "/")
set (CPACK_RPM_FILE_NAME "RPM-DEFAULT")

set (CPACK_RPM_PACKAGE_RELEASE 1)
set (CPACK_RPM_PACKAGE_RELEASE_DIST ON)
set (CPACK_RPM_PACKAGE_GROUP "Applications/Internet")

set (CPACK_RPM_MAIN_PACKAGE_NAME "manticore")

set (CPACK_RPM_META_PACKAGE_NAME "manticore-all")
set (CPACK_RPM_META_PACKAGE_REQUIRES "manticore-server, manticore-tools")
set (CPACK_RPM_META_PACKAGE_CONFLICTS "manticore, sphinx")

set (CPACK_RPM_META_BUILD_SOURCE_DIRS_PREFIX OFF)

set (CPACK_RPM_APPLICATIONS_PACKAGE_NAME "manticore-server")
set (CPACK_RPM_APPLICATIONS_FILE_NAME "RPM-DEFAULT")
set (CPACK_RPM_APPLICATIONS_INSTALL_WITH_EXEC ON)
set (CPACK_RPM_APPLICATIONS_PACKAGE_OBSOLETES "sphinx")
set (CPACK_RPM_APPLICATIONS_PACKAGE_CONFLICTS "sphinx")

set (CPACK_RPM_ICUDATA_PACKAGE_NAME "manticore-icudata")
set (CPACK_RPM_ICUDATA_FILE_NAME "RPM-DEFAULT")
# other vars defined in CommonInfo

set (CPACK_RPM_TOOLS_PACKAGE_NAME "manticore-tools")
set (CPACK_RPM_TOOLS_PACKAGE_CONFLICTS "sphinx, manticore <= 3.5.0_200722.1d34c49")
set (CPACK_RPM_TOOLS_FILE_NAME "RPM-DEFAULT")

set (CPACK_RPM_DEVEL_PACKAGE_NAME "manticore-devel")
set (CPACK_RPM_DEVEL_FILE_NAME "RPM-DEFAULT")

set (CPACK_RPM_MAIN_DEBUGINFO_PACKAGE ON)
set (CPACK_RPM_APPLICATIONS_DEBUGINFO_PACKAGE ON)
set (CPACK_RPM_TOOLS_DEBUGINFO_PACKAGE ON)
set (CPACK_RPM_CONVERTER_DEBUGINFO_PACKAGE ON)
set (CPACK_RPM_ICUDATA_DEBUGINFO_PACKAGE OFF)
set (CPACK_RPM_DEVEL_DEBUGINFO_PACKAGE OFF)
set (CPACK_RPM_META_DEBUGINFO_PACKAGE OFF)

#set ( CPACK_BUILD_SOURCE_DIRS OFF )
string (LENGTH "${CMAKE_SOURCE_DIR}" source_dir_len_)
if (source_dir_len_ LESS 75)
	message (STATUS "set src prefix to /tmp/m due to too long path")
	set (CPACK_RPM_BUILD_SOURCE_DIRS_PREFIX "/tmp")
endif ()

#set ( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "/usr/include" )
if (SPLIT)
	set (CPACK_RPM_APPLICATIONS_USER_FILELIST
			"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
			"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
			)
else ()
	set (CPACK_RPM_MAIN_USER_FILELIST
			"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
			"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
			)
endif ()

set (CPACK_RPM_SPEC_MORE_DEFINE
		"%define _scripts ${MANTICORE_BINARY_DIR}
%define manticore_user manticore
%define manticore_group manticore")

SET (CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)")

set (SCR "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm") # a shortcut
if (SPLIT)
	set (CPACK_RPM_APPLICATIONS_BUILDREQUIRES "systemd-units")
	set (CPACK_RPM_APPLICATIONS_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore_s.post")
	set (CPACK_RPM_APPLICATIONS_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun")
	set (CPACK_RPM_APPLICATIONS_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun")
else ()
	set (CPACK_RPM_BUILDREQUIRES "systemd-units")
	set (CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${MANTICORE_BINARY_DIR}/manticore_s.post")
	set (CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.postun")
	set (CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${SCR}/manticore_s.preun")
endif ()


# now get system paths. These variables are used in configure substitutions below
set (CMAKE_INSTALL_PREFIX "${CPACK_PACKAGING_INSTALL_PREFIX}" CACHE PATH "prefix from distr build" FORCE)
include (GNUInstallDirs)

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set (CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore")
GNUInstallDirs_get_absolute_install_dir (CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR)

set (CMAKE_INSTALL_LIB "lib")
GNUInstallDirs_get_absolute_install_dir (CMAKE_INSTALL_FULL_LIB CMAKE_INSTALL_LIB LIBDIR)

# block below used to patch the minconf - add a slash at the end of 'binlog_path' section
configure_config (lib/manticore)

configure_file (dist/rpm/manticore.logrotate.in "${MANTICORE_BINARY_DIR}/manticore.logrotate" @ONLY)
configure_file (dist/rpm/manticore_s.post.in "${MANTICORE_BINARY_DIR}/manticore_s.post" @ONLY)
configure_file (dist/rpm/manticore.post.in "${MANTICORE_BINARY_DIR}/manticore.post" @ONLY)

configure_file ("dist/rpm/manticore.tmpfiles.in" "${MANTICORE_BINARY_DIR}/manticore.tmpfiles" @ONLY)
configure_file ("dist/rpm/manticore.generator.in" "${MANTICORE_BINARY_DIR}/manticore-search-generator" @ONLY)

# installation

# stuff going to /etc
# CMAKE_INSTALL_SYSCONFDIR					etc 					/etc
install (FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT applications)
install (FILES ${MANTICORE_BINARY_DIR}/manticore.logrotate DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT applications RENAME manticore)


# stuff going to /var
# CMAKE_INSTALL_LOCALSTATEDIR				var 					/var
install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore/data COMPONENT applications)
install (DIRECTORY DESTINATION ${CMAKE_INSTALL_RUNSTATEDIR}/manticore COMPONENT applications)
install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT applications)

# stuff that should go to /lib -> actually to /usr/lib
# CMAKE_INSTALL_LIBDIR						usr/lib64 				/usr/lib64
install (FILES ${MANTICORE_BINARY_DIR}/manticore.tmpfiles DESTINATION ${CMAKE_INSTALL_LIB}/tmpfiles.d COMPONENT applications RENAME manticore.conf)
install (PROGRAMS ${MANTICORE_BINARY_DIR}/manticore-search-generator DESTINATION ${CMAKE_INSTALL_LIB}/systemd/system-generators COMPONENT applications)


# binaries go to /usr/bin (here is only new cluster, rest is in main file, installing targets)
# CMAKE_INSTALL_BINDIR						usr/bin 				/usr/bin
install (PROGRAMS "dist/rpm/manticore_new_cluster" DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)


# stuff going to /usr/share/man, /usr/share/doc
# CMAKE_INSTALL_MANDIR						usr/share/man 			/usr/share/man
# CMAKE_INSTALL_DOCDIR						usr/share/doc/manticore /usr/share/doc/manticore
install (FILES doc/indexer.1 doc/indextool.1 doc/spelldump.1 doc/wordbreaker.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT tools)
install (FILES doc/searchd.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT applications)
install (FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT applications RENAME manticore.conf.dist)
install (FILES COPYING example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT applications)

# stuff going to /usr/share/manticore
# CMAKE_INSTALL_DATAROOTDIR					usr/share 				/usr/share
# CMAKE_INSTALL_DATADIR						usr/share 				/usr/share
install (DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT applications)
install (FILES INSTALL DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT meta)
install (DIRECTORY DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules COMPONENT applications)

SET (FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore")

if (WITH_ICU)
	install_icudata (${FULL_SHARE_DIR}/icu)
endif ()

if (NOT NOAPI)
	install (DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT applications)
endif ()

SET (LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/manticore/data") # will be used also in the app
set (installed ON)
