#!/bin/bash

# distr should be set on inclusion of this file

arch=$(uname -m)
mkdir $arch
docker run --rm -v $(pwd):/sysroot ubuntu:$distr bash /sysroot/in_ubuntu.sh $distr

cd $arch
echo "pack archives"
for a in *.tar; do
  echo "pack $a..."
  zstd $a && rm -f $a;
done


