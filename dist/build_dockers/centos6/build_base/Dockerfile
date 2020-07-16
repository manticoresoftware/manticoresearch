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
    rpm-build \
    systemd-units \
    wget \
&& yum clean all \
&& mkdir /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
ENV PATH $PATH:/opt/rh/devtoolset-2/root/usr/bin

ADD https://dl.min.io/client/mc/release/linux-amd64/mc /
RUN chmod +x mc && mv mc /usr/local/bin/

WORKDIR /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
ENV DISTR rhel6
CMD []

# docker build -t build_base:centos6 .