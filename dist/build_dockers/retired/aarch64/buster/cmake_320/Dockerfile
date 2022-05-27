FROM buster_boost:175

# update stuff that may exipre. Fixme! move ninja to base_build
RUN apt-get update && apt-get install -y \
    ca-certificates \
    ninja-build \
&& rm -rf /var/lib/apt/lists/*

# add cmake as separate layer
ARG cmakever="3.20.5"
ARG cmakesuff="${cmakever}-linux"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakesuff}-aarch64.tar.gz \
    && tar -zxf cmake-${cmakesuff}-aarch64.tar.gz \
    && rm cmake-${cmakesuff}-aarch64.tar.gz
ENV PATH $PATH:/cmake-${cmakesuff}-aarch64/bin

CMD []

# docker build -t buster_cmake:320 -t registry.gitlab.com/manticoresearch/dev/buster_cmake:320a .
