message ( STATUS "Will create DEB for Ubuntu 20.04 (focal)" )

# packaging for new distro: look into jammy.cmake
set ( disable_shlideps ON )
set ( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.29), libgcc-s1 (>= 3.0), libssl1.1 (>= 1.1.0), libstdc++6 (>= 9)" )

include ( CommonDeb )
