# file included from build_buster/bullseye/bookworm/focal/bionic/jammy
# $distr and $image should be set on inclusion of this file

arch=$(uname -m)
mkdir $arch
docker run --rm -v $(pwd):/sysroot $image bash /sysroot/in_debian.sh $distr

. finalize.sh
