#!/bin/bash
# file runs in the docker, invoked by the command in script redhat.sh
# docker run --rm -v $(pwd):/sysroot centos:7 bash /sysroot/in_rhel7.sh
# docker run -it --rm -v $(pwd):/sysroot centos:7 bash

arch=$(uname -m)
export arch
export distr=rhel7

yum install -y centos-release-scl
yum install -y devtoolset-8
export PATH="/opt/rh/devtoolset-8/root/usr/bin/:$PATH"
export LD_LIBRARY_PATH="/opt/rh/devtoolset-8/root/usr/lib64/:$LD_LIBRARY_PATH"

 . /sysroot/mkboost.sh

echo "install rest of dependencies"
yum install -y epel-release

. /sysroot/packages_$distr.txt
. /sysroot/mksysrootrhel.sh
