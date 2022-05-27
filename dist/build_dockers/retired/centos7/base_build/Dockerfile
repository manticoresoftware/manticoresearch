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
        systemd-units \
        python-devel \
        wget \
        centos-release-scl \
&& yum-config-manager --enable rhel-server-rhscl-7-rpms \
&& yum install -y devtoolset-8 \
&& scl enable devtoolset-8 bash \
&& yum clean all \
&& rm -rf /var/cache/yum \
&& mkdir /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

RUN cd / && wget https://dl.min.io/client/mc/release/linux-amd64/mc \
    && chmod +x mc && mv mc /usr/local/bin/

ENV PATH /opt/rh/devtoolset-8/root/usr/bin/:$PATH
ENV LD_LIBRARY_PATH /opt/rh/devtoolset-8/root/usr/lib64/:$LD_LIBRARY_PATH

WORKDIR /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
ENV DISTR rhel7
CMD []

# docker build -t base_build:centos7 .
