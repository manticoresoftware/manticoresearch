# docker build -t registry.gitlab.com/manticoresearch/dev/centos8_cmake317 .

FROM centos:8

RUN yum install -y \
        gcc \
        gcc-c++ \
        make \
        git \
        flex \
        bison \
        rpm-build \
        mariadb-connector-c-devel \
        expat-devel \
        postgresql-devel \
        unixODBC-devel \
        systemd-units \
        wget \
&& yum clean all \
&& rm -rf /var/cache/yum \
&& mkdir /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

# things that sometimes expire - on separate layer
RUN yum upgrade -y && yum install -y \
    ca-certificates \
&& yum clean all

# boost on separate layer
RUN mkdir /boost && cd /boost \
&& wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz \
&& tar -zxf boost_1_72_0.tar.gz && cd boost_1_72_0 \
&& ./bootstrap.sh \
&& ./b2 --with-context \
&& ./b2 install --with-context \
&& cd / && rm -rf boost

# add cmake as separate layer
RUN wget https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-Linux-x86_64.tar.gz \
&& tar -zxf cmake-3.17.3-Linux-x86_64.tar.gz \
&& rm cmake-3.17.3-Linux-x86_64.tar.gz
#ADD https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-Linux-x86_64.tar.gz /
ENV PATH $PATH:/cmake-3.17.3-Linux-x86_64/bin

# cloud client - expires every 2 months, put at the highest level
RUN wget https://dl.min.io/client/mc/release/linux-amd64/mc \
&& mv mc /usr/local/bin/ \
&& chmod a+x /usr/local/bin/mc

# this huge path need for building debug package (len of source path
# must be > then len of path where sources will came in debug pkg)
WORKDIR /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
ENV DISTR rhel8
CMD []
