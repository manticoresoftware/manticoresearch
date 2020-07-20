FROM xenial_boost:172

# update stuff that may exipre
RUN apt-get update && apt-get install -y \
    ca-certificates \
&& rm -rf /var/lib/apt/lists/* \
&& mc update

# add cmake as separate layer
ARG cmakever="3.17.3"
RUN wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-Linux-x86_64.tar.gz \
    && tar -zxf cmake-${cmakever}-Linux-x86_64.tar.gz \
    && rm cmake-${cmakever}-Linux-x86_64.tar.gz
ENV PATH $PATH:/cmake-${cmakever}-Linux-x86_64/bin

CMD []

# docker build -t xenial_cmake:317 -t registry.gitlab.com/manticoresearch/dev/xenial_cmake:317 -t registry.gitlab.com/manticoresearch/dev/xenial_cmake:latest .
