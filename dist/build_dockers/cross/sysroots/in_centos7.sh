#!/bin/bash
# docker run --rm -v $(pwd):/sysroot centos:7 bash /sysroot/in_centos.sh
# docker run -it --rm -v $(pwd):/sysroot centos:7 bash

arch=$(uname -m)
export arch
#export distr=$1
export distr=rhel7

yum install -y centos-release-scl
yum install -y devtoolset-8
export PATH="/opt/rh/devtoolset-8/root/usr/bin/:$PATH"
export LD_LIBRARY_PATH="/opt/rh/devtoolset-8/root/usr/lib64/:$LD_LIBRARY_PATH"

 . /sysroot/mkboost.sh

echo "install rest of dependencies"
yum install -y epel-release
yum install -y \
        mysql-devel \
        expat-devel \
        postgresql-devel \
        unixODBC-devel \
        jemalloc-devel \
        openssl-devel \
        libzstd-devel

. /sysroot/mksysrootrhel.sh
