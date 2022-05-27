FROM centos8_boost:175

# update stuff that may exipre
RUN yum update -y ca-certificates \
&& dnf --enablerepo=powertools install -y ninja-build \
&& yum clean all \
&& rm -rf /var/cache/yum

# add cmake as separate layer
ARG cmakever="3.20.5"
ARG cmakesuff="${cmakever}-linux"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakesuff}-aarch64.tar.gz \
    && tar -zxf cmake-${cmakesuff}-aarch64.tar.gz \
    && rm cmake-${cmakesuff}-aarch64.tar.gz
ENV PATH $PATH:/cmake-${cmakesuff}-aarch64/bin

CMD []

# docker build -t centos8_cmake:320 -t registry.gitlab.com/manticoresearch/dev/centos8_cmake:320a .
