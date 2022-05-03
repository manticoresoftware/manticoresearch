# ---------- jammy ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 22.04 (jammy)

message ( STATUS "Will create DEB for Ubuntu 22.04 (jammy)" )

set (flavour "ubuntu")

set ( disable_shlideps ON )
set ( SPLIT ON )

set (icudata_name "manticore-icudata-65l")

# main meta
set ( CPACK_DEBIAN_META_PACKAGE_NAME manticore )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libstdc++6 (>= 12)")

# 'server'
set ( CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libssl3 (>= 3.0.0~~alpha1), libstdc++6 (>= 12), zlib1g (>= 1:1.2.0)")
set ( CPACK_DEBIAN_SERVER_PACKAGE_RECOMMENDS "manticore-systemd, libzstd1, ${icudata_name}" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libstdc++6 (>= 12), zlib1g (>= 1:1.1.4)")
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient21, libpq5, libexpat1, libodbc2" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "${icudata_name}" )

include ( builds/CommonDeb )
