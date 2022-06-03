FROM ubuntu:focal
ENV DEBIAN_FRONTEND noninteractive

ARG llvm=13
RUN apt-get update && apt-get install -y \
    make \
    ninja-build \
    git \
    bison \
    flex \
    wget \
    software-properties-common \
    rpm \
    elfutils \
    nsis \
    zip \
&& wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
&& add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-$llvm main" \
&& apt-get install -y \
    clang-$llvm \
    clang-tools-$llvm \
    lld-$llvm \
&& rm -rf /var/lib/apt/lists/* \
&& ln -s /usr/lib/llvm-$llvm/bin/llvm-objcopy /usr/local/bin/objcopy \
&& mkdir /zipdll && cd /zipdll \
&& wget https://nsis.sourceforge.io/mediawiki/images/d/d9/ZipDLL.zip \
&& unzip ZipDLL.zip \
&& mv ZipDLL.dll /usr/share/nsis/Plugins/x86-ansi/ \
&& rm -rf /zipdll

# notice symlink of objcopy. That is need as system objcopy can't deal with alien arch

# 2-nd layer, cmake
ARG cmakever="3.23.2"
RUN cd / \
&& wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-linux-$(uname -m).tar.gz \
&& tar -zxf cmake-${cmakever}-linux-$(uname -m).tar.gz \
&& rm cmake-${cmakever}-linux-$(uname -m).tar.gz \
&& export arch=$(uname -m)
ENV PATH $PATH:/cmake-${cmakever}-linux-x86_64/bin:/cmake-${cmakever}-linux-aarch64/bin

# 3-rd layer. Everything to build, except actual sysroot archives
RUN git config --global --add safe.directory '*'
ENV CMAKE_GENERATOR=Ninja
ENV CMAKE_TOOLCHAIN_FILE=/sysroot/root/toolchain.cmake
ADD entry_point.sh /sysroot/root/
ENTRYPOINT ["bash", "/sysroot/root/entry_point.sh"]
CMD []

## on amd:
# docker build -t external_toolchain:clang13_cmake3232 -t registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232amd .
# docker push registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232amd

# on arm
# docker build -t external_toolchain:clang13_cmake3232 -t registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232arm .
# docker push registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232arm

# on any (amd or arm)
# docker rmi registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232
# docker manifest rm registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232
# docker manifest create registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232 --amend registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232amd --amend registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232arm
# docker manifest push registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232
