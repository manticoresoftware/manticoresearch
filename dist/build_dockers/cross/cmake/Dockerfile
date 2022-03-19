FROM focal_clang:12

# update stuff that may exipre.
RUN apt-get update && apt-get install -y ca-certificates && rm -rf /var/lib/apt/lists/*

# add cmake as separate layer
ARG cmakever="3.22.3"
ARG cmakesuff="${cmakever}-linux"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakesuff}-x86_64.tar.gz \
    && tar -zxf cmake-${cmakesuff}-x86_64.tar.gz \
    && rm cmake-${cmakesuff}-x86_64.tar.gz
ENV PATH $PATH:/cmake-${cmakesuff}-x86_64/bin

CMD []

# docker build -t focal_cmake:322 -t registry.gitlab.com/manticoresearch/dev/focal_cmake:322 .
