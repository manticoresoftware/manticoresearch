#!/bin/bash
[ -e /work/aot ] && exit 0
echo "Fetching aot..."
mkdir -p /work/aot && cd /work/aot
wget https://repo.manticoresearch.com/repository/morphology/de.pak.tar.xz -q
tar -Jxf de.pak.tar.xz
wget https://repo.manticoresearch.com/repository/morphology/en.pak.tar.xz -q
tar -Jxf en.pak.tar.xz
wget https://repo.manticoresearch.com/repository/morphology/ru.pak.tar.xz -q
tar -Jxf ru.pak.tar.xz
rm *.tar.xz
cd -
echo "Fetching aot done"
