#!/bin/bash
# docker run --rm -v $(pwd):/sysroot centos:8 bash /sysroot/in_centos.sh
# docker run -it --rm -v $(pwd):/sysroot centos:8 bash

arch=$(uname -m)
export arch
#export distr=$1
export distr=rhel8

# centos8 reached eol at 31.12.2021, repo was removed. So, need to patch to use vault instead of mirror.
sed -i -e "s|mirrorlist=|#mirrorlist=|g" /etc/yum.repos.d/CentOS-*
sed -i -e "s|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g" /etc/yum.repos.d/CentOS-*

yum install -y \
        gcc \
        gcc-c++ \
        which

# . /sysroot/mkboost.sh

echo "install rest of dependencies"
yum install -y epel-release
yum install -y \
        mariadb-connector-c-devel \
        expat-devel \
        postgresql-devel \
        unixODBC-devel \
        jemalloc-devel \
        openssl-devel \
        libzstd-devel

. /sysroot/mksysrootrhel.sh
