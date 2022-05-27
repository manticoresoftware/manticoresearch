#!/bin/bash
# docker run --rm -v $(pwd):/sysroot ubuntu:xenial bash /sysroot/in_xenial.sh
# docker run -it --rm -v $(pwd):/sysroot ubuntu:xenial bash

export DEBIAN_FRONTEND=noninteractive
arch=$(uname -m)
export arch
export distr=$1

apt-get update && apt-get install -y \
    build-essential

 . /sysroot/mkboost.sh

echo "install rest of dependencies"
. /sysroot/ubuntu.apt

. /sysroot/mksysroot.sh
