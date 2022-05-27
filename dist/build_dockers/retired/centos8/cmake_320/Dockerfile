FROM centos8_boost:175

# update stuff that may exipre
RUN yum update -y ca-certificates \
&& yum install -y 'dnf-command(config-manager)' \
&& yum config-manager --set-enabled PowerTools \
&& yum install -y ninja-build \
&& yum clean all \
&& rm -rf /var/cache/yum \
&& mc update

# add cmake as separate layer
ARG cmakever="3.20.2"
ARG cmakesuff="${cmakever}-linux"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakesuff}-x86_64.tar.gz \
    && tar -zxf cmake-${cmakesuff}-x86_64.tar.gz \
    && rm cmake-${cmakesuff}-x86_64.tar.gz
ENV PATH $PATH:/cmake-${cmakesuff}-x86_64/bin

CMD []

# docker build -t centos8_cmake:320 -t registry.gitlab.com/manticoresearch/dev/centos8_cmake:320 -t registry.gitlab.com/manticoresearch/dev/centos8_cmake:latest .
