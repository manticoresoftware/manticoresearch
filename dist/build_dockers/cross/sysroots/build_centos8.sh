#!/bin/bash

arch=$(uname -m)
mkdir $arch
docker run --rm -v $(pwd):/sysroot centos:8 bash /sysroot/in_centos8.sh

cd $arch
echo "pack archives"
for a in *.tar; do
  echo "pack $a..."
  zstd $a && rm -f $a;
done
