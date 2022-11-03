#!/bin/bash
# docker run --rm -v $(pwd):/sysroot almalinux:9 bash /sysroot/in_alma9.sh
# docker run -it --rm -v $(pwd):/sysroot almalinux:9 bash

arch=$(uname -m)
export arch
export distr=rhel9

dnf install -y gcc gcc-c++ which

. /sysroot/mkboost.sh

echo "install rest of dependencies"
dnf install -y epel-release
dnf upgrade --refresh -y
dnf config-manager --set-enabled crb
dnf install -y \
        mariadb-connector-c-devel \
        expat-devel \
        postgresql-devel \
        unixODBC-devel \
        jemalloc-devel \
        openssl-devel \
        libzstd-devel

. /sysroot/mksysrootrhel.sh
