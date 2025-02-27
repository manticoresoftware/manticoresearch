FROM ubuntu:focal
# Note, bison in Jammy leads to some updated error messages and some of them look wrong, e.g.
# "expecting end of file" instead of "expecting $end"
ENV DEBIAN_FRONTEND noninteractive

# 1-st layer, basic toolchain stuff
RUN apt-get update && apt-get install -y \
    make \
    ninja-build \
    git \
    bison \
    flex \
    curl \
    software-properties-common \
    rpm \
    elfutils \
    nsis \
    zip \
    pkg-config \
&& rm -rf /var/lib/apt/lists/* \
&& git config --global --add safe.directory '*' \
&& mkdir /nsisplugs && cd /nsisplugs \
&& curl -O https://nsis.sourceforge.io/mediawiki/images/d/d9/ZipDLL.zip \
    -O https://nsis.sourceforge.io/mediawiki/images/1/1c/Nsisunz.zip \
&& unzip ZipDLL.zip && rm ZipDLL.zip \
&& mv ZipDLL.dll /usr/share/nsis/Plugins/x86-ansi/ \
&& unzip Nsisunz.zip && rm Nsisunz.zip \
&& mv nsisunz/Release/nsisunz.dll /usr/share/nsis/Plugins/x86-unicode/ \
&& rm -rf /nsisplugs

# 2-nd layer, llvm. Notice ENV vs ARG here
ENV llvm=16
RUN curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
&& add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-$llvm main" \
&& apt-get install -y \
    clang-$llvm \
    clang-tools-$llvm \
    lld-$llvm \
&& rm -rf /var/lib/apt/lists/* \
&& ln -s /usr/lib/llvm-$llvm/bin/llvm-objcopy /usr/local/bin/objcopy
# notice symlink of objcopy. That is need as system objcopy can't deal with alien arch
ENV PATH $PATH:/usr/lib/llvm-${llvm}/bin

# 3-rd layer, cmake
ENV CMAKE_GENERATOR=Ninja
ENV CMAKE_TOOLCHAIN_FILE=/sysroot/root/toolchain.cmake
ARG cmakever="3.31.2"
RUN cd / \
&& curl -k -L https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-linux-x86_64.tar.gz | tar -zx
ENV PATH $PATH:/cmake-${cmakever}-linux-x86_64/bin

# last layer - vcpkg
ENV VCPKG_ROOT=/vcpkg
ENV VCPKG_DISABLE_METRICS=1
ENV VCPKG_FORCE_SYSTEM_BINARIES=1
ENV PATH=$VCPKG_ROOT:$PATH
RUN cd / && git clone https://github.com/microsoft/vcpkg.git \
&& cd vcpkg \
&& ./bootstrap-vcpkg.sh \
&& vcpkg install vcpkg-cmake vcpkg-cmake-config vcpkg-cmake-get-vars

ADD entry_point.sh /sysroot/root/
ENTRYPOINT ["bash", "/sysroot/root/entry_point.sh"]
CMD []

# docker buildx create --name manticore_build --platform linux/amd64
# docker buildx use manticore_build
# docker buildx build --platform linux/amd64 --push -t manticoresearch/external_toolchain:vcpkg331_20250114 .

# deprecated:
# on amd:
# docker build -t external_toolchain:clang15_cmake3263 -t registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263amd .
# docker push registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263amd
# on arm:
# docker build -t external_toolchain:clang15_cmake3263 -t registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263arm .
# docker push registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263arm
# on any (amd or arm):
# docker rmi registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263
# docker manifest rm registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263
# docker manifest create registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263 --amend registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263amd --amend registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263arm
# docker manifest push registry.gitlab.com/manticoresearch/dev/external_toolchain:clang15_cmake3263

# docker build -t external_toolchain:vcpkg331 -t registry.gitlab.com/manticoresearch/dev/external_toolchain:vcpkg331 .
# docker push registry.gitlab.com/manticoresearch/dev/external_toolchain:vcpkg331