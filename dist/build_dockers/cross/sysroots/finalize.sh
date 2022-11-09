#!/bin/bash

# file included from debian/ubuntu/redhat.sh, but also may be run as standalone.
# nothing required to defined; file runs from host context
# That is final step (m.b. not necessary in general)

arch=$(uname -m)

cd $arch
echo "pack archives"
for a in *.tar; do
  echo "pack $a..."
  zstd $a && rm -f $a;
done
