message ( STATUS "Will create DEB for Ubuntu 22.04 (jammy)" )

# packaging for new distro: set to OFF disable_shlideps, and comment out line with CPACK_DEBIAN_LIB_PACKAGE_DEPENDS
# first time you need to build once in target system, so that shlideps will be correctly detected.
# open properties of built .deb and copy dependency line into CPACK_DEBIAN_LIB_PACKAGE_DEPENDS. Then set ON to disable_shlideps
# Since that moment dependencies are 'frozen', and shlideps will not be used, and we can perform cross-build on alien arch.
set ( disable_shlideps ON )
set ( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.34), libgcc-s1 (>= 3.0), libssl3 (>= 3.0.0~~alpha1), libstdc++6 (>= 11)" )

include ( CommonDeb )
