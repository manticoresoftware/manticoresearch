#!/bin/bash
set -e
arch=$(uname -m)
mkdir "$arch"
docker run --rm -v "$PWD:/sysroot" almalinux:9.0 bash /sysroot/in_alma9.sh

cd "$arch"
echo "pack archives"
for a in *.tar; do
  echo "pack $a..."
  zstd "$a" && rm -f "$_";
done
