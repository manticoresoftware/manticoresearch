# ---------- bionic ----------
# Above line is mandatory!
# rules to build deb package for Ubuntu 18.04 (bionic)

message ( STATUS "Will create DEB for Ubuntu 18.04 (bionic)" )

set (flavour "ubuntu")

# we provide explicit dependencies, so shlideps is not necessary
set ( disable_shlideps ON )

# 'converter'
set ( CPACK_DEBIAN_CONVERTER_PACKAGE_DEPENDS "libc6 (>= 2.27), libgcc1 (>= 1:3.4), libstdc++6 (>= 5.2)" )

# 'server'
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_DEPENDS "manticore-common, libc6 (>= 2.27), libgcc1 (>= 1:3.4), libssl1.1 (>= 1.1.0), libstdc++6 (>= 5.2), zlib1g (>= 1:1.2.0)" )
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_RECOMMENDS "manticore-server" )

# 'tools'
set ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "libc6 (>= 2.27), libgcc1 (>= 1:3.4), libstdc++6 (>= 5.2), zlib1g (>= 1:1.1.4)" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_SUGGESTS "libmysqlclient20, libpq5, libexpat1, libodbc1" )

include ( builds/CommonDeb )
