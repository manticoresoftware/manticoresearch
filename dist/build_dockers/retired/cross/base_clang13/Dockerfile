FROM ubuntu:focal
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
    ninja-build \
    zstd \
    git \
    bison \
    flex \
&& rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    wget \
    software-properties-common \
&& wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-13 main" \
&& apt-get update && apt-get install -y \
    clang-13 \
    lld-13 \
&& rm -rf /var/lib/apt/lists/*

CMD []

# docker build -t base_clang:13 -t registry.gitlab.com/manticoresearch/dev/base_clang:13 .
