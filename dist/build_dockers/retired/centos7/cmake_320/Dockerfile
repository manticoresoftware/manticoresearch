FROM centos7_boost:175

# update stuff that may exipre
RUN yum install -y http://repo.okay.com.mx/centos/7/x86_64/release/okay-release-1-1.noarch.rpm \
&& yum update -y ca-certificates \
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

# docker build -t centos7_cmake:320 -t registry.gitlab.com/manticoresearch/dev/centos7_cmake:320 -t registry.gitlab.com/manticoresearch/dev/centos7_cmake:latest .
