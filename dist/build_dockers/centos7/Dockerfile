# docker build -t registry.gitlab.com/manticoresearch/dev/centos7_cmake314 .

FROM centos:7

RUN yum install -y \
        gcc \
        gcc-c++ \
        make \
        git \
        flex \
        bison \
        rpm-build \
        mysql-devel \
        expat-devel \
        postgresql-devel \
        unixODBC-devel \
        libicu-devel \
        systemd-units \
        boost-devel \
        python-devel \
&& yum clean all \
&& rm -rf /var/cache/yum \
&& mkdir /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

# add cmake as separate layer
# file taken from https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5-Linux-x86_64.tar.gz
ADD cmake-3.14.5-Linux-x86_64.tar.gz /
ENV PATH $PATH:/cmake-3.14.5-Linux-x86_64/bin
ENV DISTR rhel7

ADD boost_1_73_0.tar.gz /
RUN cd /boost_1_73_0 \
    && ./bootstrap.sh \
    && ./b2
ENV INCLUDE /boost_1_73_0:$INCLUDE
ENV LIBRARY_PATH /boost_1_73_0/stage/lib:$LIBRARY_PATH

# this huge path need for building debug package (len of source path
# must be > then len of path where sources will came in debug pkg)
WORKDIR /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
CMD []

