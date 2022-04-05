FROM registry.gitlab.com/manticoresearch/dev/base_cmake:323

ADD MacOSX11.3.sdk.tar.xz /cross/bigsur/
ENV OPENSSL_ROOT_DIR=/opt/openssl@1.1

ADD toolchain.cmake /cross/bigsur/
ENV CMAKE_TOOLCHAIN_FILE /cross/bigsur/toolchain.cmake

# docker build -t cross_bigsur:sdk -t registry.gitlab.com/manticoresearch/dev/cross_bigsur:sdk .
