#!/bin/bash

# file runs in the docker, invoked by the command in script redhat.sh
# docker run --rm -v $(pwd):/sysroot almalinux:10 bash /sysroot/in_rhel10.sh
# docker run -it --rm -v $(pwd):/sysroot almalinux:10 bash

arch=$(uname -m)
export arch
export distr=rhel10

dnf install -y gcc gcc-c++ which gzip

. /sysroot/mkboost.sh

echo "install rest of dependencies"
dnf install -y epel-release
dnf upgrade --refresh -y
dnf config-manager --set-enabled crb

. /sysroot/packages_$distr.txt
. /sysroot/mksysrootrhel.sh
