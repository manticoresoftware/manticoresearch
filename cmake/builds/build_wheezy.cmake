# ---------- wheezy ----------
# Above line is mandatory!
# rules to build deb package for Debian wheezy

message ( STATUS "Will create DEB for Debian (wheezy)" )

# m.b. postinst.xenial, postinst.debian and postinst.trusty
FILE ( READ dist/deb/postinst.debian POSTINST_SPECIFIC )

# m.b. prerm.ubuntu, prerm.debian
configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/prerm.debian"
		"${MANTICORE_BINARY_DIR}/prerm" COPYONLY )

# m.b. posrtrm.systemd, posrtm.wheezy targeted to posrtm, and also preinst.trusty targeted to preinst
configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/dist/deb/postrm.wheezy"
		"${MANTICORE_BINARY_DIR}/postrm" COPYONLY )

# m.b. posrtrm, preinst - see also above
set ( EXTRA_SCRIPTS "${MANTICORE_BINARY_DIR}/postrm;" )

include ( builds/CommonDeb )

# some wheezy-specific variables and files
set ( DISTR_SUFFIX "~wheezy_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}" )
