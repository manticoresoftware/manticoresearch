# file included from build_centos7/centos8/alma9
# $distr and $image should be set on inclusion of this file

arch=$(uname -m)
mkdir $arch
docker run --rm -v $(pwd):/sysroot $image bash /sysroot/in_$distr.sh

. finalize.sh

