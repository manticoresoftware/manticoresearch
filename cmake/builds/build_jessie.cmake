# ---------- jessie ----------
# Above line is mandatory!
# rules to build deb package for Debian jessie

message ( STATUS "Will create DEB for Debian (jessie)" )

# m.b. postinst.xenial, postinst.debian and postinst.trusty
FILE ( READ dist/deb/postinst.debian.in POSTINST_SPECIFIC )

# m.b. prerm.ubuntu, prerm.debian
configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/prerm.debian.in"
		"${MANTICORE_BINARY_DIR}/prerm" @ONLY )

# m.b. posrtrm.systemd, posrtm.wheezy targeted to posrtm, and also preinst.trusty targeted to preinst
configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/postrm.systemd"
		"${MANTICORE_BINARY_DIR}/postrm" COPYONLY )


# m.b. posrtrm, preinst - see also above
set ( EXTRA_SCRIPTS "${MANTICORE_BINARY_DIR}/postrm;" )

include ( builds/CommonDeb )

configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/manticore.service.in"
               "${MANTICORE_BINARY_DIR}/manticore.service" @ONLY )

install ( FILES "${MANTICORE_BINARY_DIR}/manticore.service"
               DESTINATION /lib/systemd/system COMPONENT adm )

configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/manticore.generator.in"
		"${MANTICORE_BINARY_DIR}/manticore-generator" @ONLY )

install ( FILES "${MANTICORE_BINARY_DIR}/manticore-generator"
		DESTINATION  /lib/systemd/system-generators  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ COMPONENT adm )
		
# some jessie-specific variables and files
set ( DISTR_SUFFIX "~jessie_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}" )