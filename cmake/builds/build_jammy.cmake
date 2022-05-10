# ---------- jammy ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 22.04 (jammy)

message ( STATUS "Will create DEB for Ubuntu 22.04 (jammy)" )

set (flavour "ubuntu")

# we provide explicit dependencies, so shlideps is not necessary
set ( disable_shlideps ON )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libstdc++6 (>= 12)")

# 'server'
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libssl3 (>= 3.0.0~~alpha1), libstdc++6 (>= 12), zlib1g (>= 1:1.2.0)")
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_RECOMMENDS "manticore-server, libzstd1" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libstdc++6 (>= 12), zlib1g (>= 1:1.1.4)")
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient21, libpq5, libexpat1, libodbc2" )

include ( builds/CommonDeb )
