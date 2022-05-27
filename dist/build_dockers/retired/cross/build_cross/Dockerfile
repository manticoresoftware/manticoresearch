FROM registry.gitlab.com/manticoresearch/dev/cmake_clang:13_323

# 3-rd layer. Everything to build, except actual sysroot archives

ADD boost_include.tar.xz /sysroot/cache/common/boost
COPY toolchain.cmake /sysroot
ENV CMAKE_TOOLCHAIN_FILE /sysroot/toolchain.cmake
ADD entry_point.sh /sysroot/root/

# docker build -t build_cross:clang13_cmake323 -t registry.gitlab.com/manticoresearch/dev/build_cross:clang13_cmake323 .
