#!/bin/bash

echo "build boost"
boostver=1_78_0
cd /
ln -s /sysroot/boost_${boostver}.tar.gz boost_${boostver}.tar.gz
tar -zxf boost_${boostver}.tar.gz
cd boost_${boostver}
./bootstrap.sh
./b2 install --with-context --with-system --with-fiber --with-program_options --with-stacktrace

echo "make link to shared boost includes"
cd /usr/local

# comment out this line after first run, as it produces one and same archive everytime
tar -cf - include > /sysroot/boost_include.tar

rm -rf include
ln -s ../../../cache/common/boost/include include

echo "pack boost libraries"
tar -cf - /usr/local/lib /usr/local/include > /sysroot/$arch/boost_${distr}_$arch.tar

