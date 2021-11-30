# ---------- bullseye ----------
# Above line is mandatory!
# rules to build deb package for Debian bullseye

message ( STATUS "Will create DEB for Debian (bullseye)" )

set (flavour "debian")

# some bullseye-specific variables and files
set (CPACK_DEBIAN_PACKAGE_SUGGESTS "libmariadb3, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmariadb3, libpq5, libexpat1, libodbc1")
set (CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "manticore-icudata")
set (CPACK_DEBIAN_BIN_PACKAGE_RECOMMENDS "manticore-icudata")

include ( builds/CommonDeb )
