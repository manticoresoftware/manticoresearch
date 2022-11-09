#!/bin/bash
# file runs in the docker, invoked by the command in script debian.sh
# $distr should be passed as 1-st param

# docker run --rm -v $(pwd):/sysroot debian:buster bash /sysroot/in_debian.sh buster
# docker run -it --rm -v $(pwd):/sysroot debian:buster bash
# docker run --rm -v $(pwd):/sysroot debian:bullseye bash /sysroot/in_debian.sh bullseye
# docker run -it --rm -v $(pwd):/sysroot debian:bullseye bash
# docker run --rm -v $(pwd):/sysroot debian:bookworm bash /sysroot/in_debian.sh bookworm
# docker run -it --rm -v $(pwd):/sysroot debian:bookworm bash
# docker run --rm -v $(pwd):/sysroot ubuntu:bionic bash /sysroot/in_debian.sh bionic
# docker run -it --rm -v $(pwd):/sysroot ubuntu:bionic bash
# docker run --rm -v $(pwd):/sysroot ubuntu:focal bash /sysroot/in_debian.sh focal
# docker run -it --rm -v $(pwd):/sysroot ubuntu:focal bash
# docker run --rm -v $(pwd):/sysroot ubuntu:jammy bash /sysroot/in_debian.sh jammy
# docker run -it --rm -v $(pwd):/sysroot ubuntu:jammy bash

export DEBIAN_FRONTEND=noninteractive
arch=$(uname -m)
export arch
export distr=$1

apt-get update && apt-get install -y \
    build-essential

 . /sysroot/mkboost.sh

echo "install rest of dependencies"
. /sysroot/packages_$distr.txt
. /sysroot/mksysroot.sh
