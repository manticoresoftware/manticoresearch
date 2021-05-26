# ---------- stretch ----------
# Above line is mandatory!
# rules to build deb package for Debian stretch

message ( STATUS "Will create DEB for Debian (stretch)" )

set (flavour "debian")

# some stretch-specific variables and files
set (CPACK_DEBIAN_PACKAGE_SUGGESTS "libmysqlclient18, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient18, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "manticore-icudata")
set (CPACK_DEBIAN_BIN_PACKAGE_RECOMMENDS "manticore-icudata")

include ( builds/CommonDeb )
