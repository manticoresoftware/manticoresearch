# ---------- trusty ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 14.04 (Trusty)

message ( STATUS "Will create DEB for Ubuntu 14.04 (Trusty)" )

# m.b. postinst.xenial, postinst.debian and postinst.trusty
FILE ( READ dist/deb/postinst.trusty POSTINST_SPECIFIC)

# m.b. prerm.ubuntu, prerm.debian
configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/prerm.ubuntu"
		"${MANTICORE_BINARY_DIR}/prerm" COPYONLY )

# m.b. posrtrm.systemd, posrtm.wheezy targeted to posrtm, and also preinst.trusty targeted to preinst
configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/preinst.trusty"
		"${MANTICORE_BINARY_DIR}/preinst" COPYONLY )

# m.b. posrtrm, preinst - see also above
set ( EXTRA_SCRIPTS "${MANTICORE_BINARY_DIR}/preinst;" )

include ( builds/CommonDeb )

# everywhere, but not on wheezy
install ( FILES dist/deb/manticore.service
		DESTINATION /lib/systemd/system COMPONENT adm )

# some trusty-specific variables and files
set ( DISTR_SUFFIX "~trusty_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}" )