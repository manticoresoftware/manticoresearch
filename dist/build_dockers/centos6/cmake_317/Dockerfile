FROM centos6_boost:172

# update stuff that may exipre
RUN yum update -y ca-certificates \
&& yum clean all \
&& rm -rf /var/cache/yum \
&& mc update

# add cmake as separate layer
ARG cmakever="3.17.3"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-Linux-x86_64.tar.gz \
    && tar -zxf cmake-${cmakever}-Linux-x86_64.tar.gz \
    && rm cmake-${cmakever}-Linux-x86_64.tar.gz
ENV PATH $PATH:/cmake-${cmakever}-Linux-x86_64/bin

CMD []

# docker build -t centos6_cmake:317 -t registry.gitlab.com/manticoresearch/dev/centos6_cmake:317 -t registry.gitlab.com/manticoresearch/dev/centos6_cmake:latest .
