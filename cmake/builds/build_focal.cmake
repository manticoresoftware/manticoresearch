# ---------- focal ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 20.04 (focal)

message ( STATUS "Will create DEB for Ubuntu 20.04 (focal)" )

set (flavour "ubuntu")

# we provide explicit dependencies, so shlideps is not necessary
set ( disable_shlideps ON )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.0), libstdc++6 (>= 5.2)" )

# 'server-core'
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.0), libssl1.1 (>= 1.1.0), libstdc++6 (>= 9), zlib1g (>= 1:1.2.0)" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.0), libstdc++6 (>= 5.2), zlib1g (>= 1:1.1.4)" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient21, libpq5, libexpat1, libodbc1" )

include ( builds/CommonDeb )
