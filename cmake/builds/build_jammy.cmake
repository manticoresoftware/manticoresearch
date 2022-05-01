# ---------- jammy ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 22.04 (jammy)

message ( STATUS "Will create DEB for Ubuntu 22.04 (jammy)" )

set (flavour "ubuntu")

# some jammy-specific variables and files
set (CPACK_DEBIAN_PACKAGE_SUGGESTS "libmysqlclient21, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient21, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "manticore-icudata")
set (CPACK_DEBIAN_BIN_PACKAGE_RECOMMENDS "manticore-icudata")

include ( builds/CommonDeb )
