message ( STATUS "Will create DEB for Debian (buster)" )

# packaging for new distro: look into jammy.cmake
set ( disable_shlideps ON )
set ( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.28), libgcc1 (>= 1:3.4), libssl1.1 (>= 1.1.0), libstdc++6 (>= 5.2)" )

include ( CommonDeb )
