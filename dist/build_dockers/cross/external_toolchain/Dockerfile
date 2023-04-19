FROM ubuntu:focal
ENV DEBIAN_FRONTEND noninteractive

# 1-st layer, basic toolchain stuff
ENV llvm=15
RUN apt-get update && apt-get install -y \
    make \
    ninja-build \
    git \
    bison \
    flex \
    curl \
    software-properties-common \
&& rm -rf /var/lib/apt/lists/*

# 2-nd layer, llvm
ENV llvm=15
RUN curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
&& add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-$llvm main" \
&& apt-get install -y \
    clang-$llvm \
    clang-tools-$llvm \
    lld-$llvm \
&& rm -rf /var/lib/apt/lists/* \
&& ln -s /usr/lib/llvm-$llvm/bin/llvm-objcopy /usr/local/bin/objcopy
# notice symlink of objcopy. That is need as system objcopy can't deal with alien arch

# 3-rd layer - rpm, elfutils, nsis + plugins, zip
RUN apt-get update && apt-get install -y \
    rpm \
    elfutils \
    nsis \
    zip \
&& rm -rf /var/lib/apt/lists/* \
&& mkdir /nsisplugs && cd /nsisplugs \
&& curl -O https://nsis.sourceforge.io/mediawiki/images/d/d9/ZipDLL.zip \
    -O https://nsis.sourceforge.io/mediawiki/images/1/1c/Nsisunz.zip \
&& unzip ZipDLL.zip && rm ZipDLL.zip \
&& mv ZipDLL.dll /usr/share/nsis/Plugins/x86-ansi/ \
&& unzip Nsisunz.zip && rm Nsisunz.zip \
&& mv nsisunz/Release/nsisunz.dll /usr/share/nsis/Plugins/x86-unicode/ \
&& rm -rf /nsisplugs

# 4-th layer, cmake
ENV cmakever="3.26.3"
RUN cd / \
&& export arch=$(uname -m) \
&& curl -L https://github.com/Kitware/CMake/releases/download/v$cmakever/cmake-$cmakever-linux-$arch.tar.gz | tar -zx
ENV PATH $PATH:/cmake-$cmakever-linux-x86_64/bin:/cmake-$cmakever-linux-aarch64/bin

# 5-th layer. Everything to build, except actual sysroot archives
ENV CMAKE_GENERATOR=Ninja
ENV CMAKE_TOOLCHAIN_FILE=/sysroot/root/toolchain.cmake
RUN git config --global --add safe.directory '*'
ADD entry_point.sh /sysroot/root/
ENTRYPOINT ["bash", "/sysroot/root/entry_point.sh"]
CMD []

# docker buildx create --name manticore_build --platform linux/amd64,linux/arm64
# docker buildx use manticore_build
# docker buildx build --platform linux/amd64,linux/arm64 --push -t manticoresearch/external_toolchain:clang15_cmake3263 .

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
