FROM registry.gitlab.com/manticoresearch/dev/toolchain_clang:13

# update stuff that may exipre.
# RUN apt-get update && apt-get install -y ca-certificates && rm -rf /var/lib/apt/lists/*

# add cmake as separate layer (easier to upgrade). That is 2-nd layer.
ARG cmakever="3.23.1"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-linux-$(uname -m).tar.gz \
    && tar -zxf cmake-${cmakever}-linux-$(uname -m).tar.gz \
    && rm cmake-${cmakever}-linux-$(uname -m).tar.gz \
    && export arch=$(uname -m)
ENV PATH $PATH:/cmake-${cmakever}-linux-x86_64/bin:/cmake-${cmakever}-linux-aarch64/bin

# docker build -t cmake_clang:13_323 -t registry.gitlab.com/manticoresearch/dev/cmake_clang:13_323 .
