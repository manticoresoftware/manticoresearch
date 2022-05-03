# ---------- xenial ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 16.04 (xenial)

message ( STATUS "Will create DEB for Ubuntu 16.04 (Xenial)" )

set (flavour "ubuntu")

# some xenial-specific variables and files
set ( disable_shlideps ON )

set ( icudata_name "manticore-icudata-65l" )

# main (everything)
set ( CPACK_DEBIAN_MAIN_PACKAGE_DEPENDS "libc6 (>= 2.17), libgcc1 (>= 1:3.3), libssl1.0.0 (>= 1.0.2~beta3), libstdc++6 (>= 5.2), zlib1g (>= 1:1.2.0)" )
set ( CPACK_DEBIAN_MAIN_PACKAGE_SUGGESTS "libmysqlclient20, libpq5, libexpat1, libodbc1" )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.17), libgcc1 (>= 1:3.3), libstdc++6 (>= 5.2)" )

# 'server'
set ( CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "libc6 (>= 2.17), libgcc1 (>= 1:3.3), libssl1.0.0 (>= 1.0.2~beta3), libstdc++6 (>= 5.2), zlib1g (>= 1:1.2.0)" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_RECOMMENDS "manticore-systemd, ${icudata_name}" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.17), libgcc1 (>= 1:3.3), libstdc++6 (>= 5.2), zlib1g (>= 1:1.1.4)" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient20, libpq5, libexpat1, libodbc1" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "${icudata_name}" )


include ( builds/CommonDeb )
