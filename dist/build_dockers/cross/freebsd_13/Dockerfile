FROM focal_cmake:322

ADD sysroot_fbsd_13.tar.xz /cross/freebsd/sysroot/x86_64

ADD toolchain.cmake /cross/freebsd/
ENV CMAKE_TOOLCHAIN_FILE /cross/freebsd/toolchain.cmake

# docker build -t cross_freebsd:13 -t registry.gitlab.com/manticoresearch/dev/cross_freebsd:13 .
