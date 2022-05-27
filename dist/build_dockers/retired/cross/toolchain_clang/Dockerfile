FROM ubuntu:focal
ENV DEBIAN_FRONTEND noninteractive

ARG llvm=13
RUN apt-get update && apt-get install -y \
    ninja-build \
    git \
    bison \
    flex \
    wget \
    software-properties-common \
    rpm \
    elfutils \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
&& add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-$llvm main" \
&& apt-get install -y \
    clang-$llvm \
    clang-tools-$llvm \
    lld-$llvm \
&& rm -rf /var/lib/apt/lists/* \
&& ln -s /usr/lib/llvm-$llvm/bin/llvm-objcopy /usr/local/bin/objcopy

# notice last line with symlink of objcopy. That is need as system objcopy can't deal with alien arch

# docker build -t toolchain_clang:13 -t registry.gitlab.com/manticoresearch/dev/toolchain_clang:13 .
