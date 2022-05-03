FROM registry.gitlab.com/manticoresearch/dev/jammy_boost:178

# update stuff that may exipre.
RUN apt-get update && apt-get install -y ca-certificates && rm -rf /var/lib/apt/lists/*

# add cmake as separate layer
ARG cmakever="3.23.1"
ARG cmakesuff="${cmakever}-linux"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakesuff}-x86_64.tar.gz \
    && tar -zxf cmake-${cmakesuff}-x86_64.tar.gz \
    && rm cmake-${cmakesuff}-x86_64.tar.gz
ENV PATH $PATH:/cmake-${cmakesuff}-x86_64/bin

CMD []

# docker build -t jammy_cmake:323 -t registry.gitlab.com/manticoresearch/dev/jammy_cmake:323 .
