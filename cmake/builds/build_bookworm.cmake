# ---------- bookworm ----------
# Above line is mandatory!
# rules to build deb package for Debian bookworm

message ( STATUS "Will create DEB for Debian (bookworm)" )

set ( flavour "debian" )

# we provide explicit dependencies, so shlideps is not necessary
set ( disable_shlideps ON )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libstdc++6 (>= 12)" )

# 'server-core'
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libssl3 (>= 3.0.0), libstdc++6 (>= 12), zlib1g (>= 1:1.2.0)" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libstdc++6 (>= 12), zlib1g (>= 1:1.1.4)" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmariadb3, libpq5, libexpat1, libodbc1" )

include ( builds/CommonDeb )
