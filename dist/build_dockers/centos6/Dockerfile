# docker build -t registry.gitlab.com/manticoresearch/dev/centos6_cmake314 .

FROM centos:6

RUN  yum install -y wget \
&& wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo \
&& yum upgrade -y \
&& yum install -y \
    make \
    git \
    flex \
    bison \
    devtoolset-2-gcc \
    devtoolset-2-binutils \
    devtoolset-2-gcc-c++ \
    mysql-devel \
    expat-devel \
    postgresql-devel \
    unixODBC-devel \
    libicu-devel \
    rpm-build \
    systemd-units \
    boost-devel \
&& yum clean all \
&& mkdir /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

# file taken from https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5-Linux-x86_64.tar.gz
ADD cmake-3.14.5-Linux-x86_64.tar.gz /
ENV PATH $PATH:/opt/rh/devtoolset-2/root/usr/bin:/cmake-3.14.5-Linux-x86_64/bin
ENV DISTR rhel6

# this huge path need for building debug package (len of source path
# must be > then len of path where sources will came in debug pkg)
WORKDIR /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
CMD []
