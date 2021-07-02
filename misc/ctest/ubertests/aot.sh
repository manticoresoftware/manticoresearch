#!/bin/bash
[ -e /work/aot ] && exit 0
mkdir -p /work/aot && cd /work/aot
wget https://repo.manticoresearch.com/repository/morphology/de.pak.tgz
tar -zxf de.pak.tgz
wget https://repo.manticoresearch.com/repository/morphology/en.pak.tgz
tar -zxf en.pak.tgz
wget https://repo.manticoresearch.com/repository/morphology/ru.pak.tgz
tar -zxf ru.pak.tgz
rm *.tgz
cd -
