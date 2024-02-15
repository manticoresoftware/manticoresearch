message ( STATUS "Will create DEB for Debian (bookworm)" )

# packaging for new distro: look into jammy.cmake
set ( disable_shlideps ON )
set ( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.3), libssl3 (>= 3.0.0), libstdc++6 (>= 12)" )

include ( CommonDeb )
