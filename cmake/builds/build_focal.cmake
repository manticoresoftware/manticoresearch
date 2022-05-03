# ---------- focal ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 20.04 (focal)

message ( STATUS "Will create DEB for Ubuntu 20.04 (focal)" )

set (flavour "ubuntu")

# some focal-specific variables and files
set ( disable_shlideps ON )

set ( icudata_name "manticore-icudata-65l" )

# main (everything)
set ( CPACK_DEBIAN_MAIN_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.4), libssl1.1 (>= 1.1.0), libstdc++6 (>= 9), zlib1g (>= 1:1.2.0)" )
set ( CPACK_DEBIAN_MAIN_PACKAGE_SUGGESTS "libmysqlclient21, libpq5, libexpat1, libodbc1" )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.4), libstdc++6 (>= 5.2)" )

# 'server'
set ( CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.4), libssl1.1 (>= 1.1.0), libstdc++6 (>= 9), zlib1g (>= 1:1.2.0)" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_RECOMMENDS "manticore-systemd, ${icudata_name}" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.4), libstdc++6 (>= 5.2), zlib1g (>= 1:1.1.4)" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient21, libpq5, libexpat1, libodbc1" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "${icudata_name}" )

include ( builds/CommonDeb )
