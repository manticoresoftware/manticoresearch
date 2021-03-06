# ---------- xenial ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 16.04 (xenial)

message ( STATUS "Will create DEB for Ubuntu 16.04 (Xenial)" )

set (flavour "ubuntu")

# some xenial-specific variables and files
set (CPACK_DEBIAN_PACKAGE_SUGGESTS "libmysqlclient20, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient20, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "manticore-icudata")
set (CPACK_DEBIAN_BIN_PACKAGE_RECOMMENDS "manticore-icudata")

include ( builds/CommonDeb )
