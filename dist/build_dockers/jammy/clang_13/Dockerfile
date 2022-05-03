FROM registry.gitlab.com/manticoresearch/dev/base_build:jammy
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
    software-properties-common \
&& rm -rf /var/lib/apt/lists/* \
&& wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

ARG llvmver="13"
RUN add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-${llvmver} main" \
&& apt-get update && apt-get install -y \
    clang-13 \
&& rm -rf /var/lib/apt/lists/*

ENV CC clang-${llvmver}
ENV CXX clang++-${llvmver}

CMD []

# docker build -t jammy_clang:13 -t registry.gitlab.com/manticoresearch/dev/jammy_clang:13 .
