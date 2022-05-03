# ---------- stretch ----------
# Above line is mandatory!
# rules to build deb package for Debian stretch

message ( STATUS "Will create DEB for Debian (stretch)" )

set (flavour "debian")

# some stretch-specific variables and files
set ( disable_shlideps ON )

set ( icudata_name "manticore-icudata-65l" )

# main (everything)
set ( CPACK_DEBIAN_MAIN_PACKAGE_DEPENDS "libc6 (>= 2.24), libgcc1 (>= 1:3.3), libssl1.1 (>= 1.1.0), libstdc++6 (>= 5.2), zlib1g (>= 1:1.2.0)" )
set ( CPACK_DEBIAN_MAIN_PACKAGE_SUGGESTS "libmysqlclient18, libpq5, libexpat1, libodbc1" )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.24), libgcc1 (>= 1:3.3), libstdc++6 (>= 5.2)" )

# 'server'
set ( CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "libc6 (>= 2.24), libgcc1 (>= 1:3.3), libssl1.1 (>= 1.1.0), libstdc++6 (>= 5.2), zlib1g (>= 1:1.2.0)" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_RECOMMENDS "manticore-systemd, ${icudata_name}" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.24), libgcc1 (>= 1:3.3), libstdc++6 (>= 5.2), zlib1g (>= 1:1.1.4)" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient18, libpq5, libexpat1, libodbc1" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "${icudata_name}" )

include ( builds/CommonDeb )
