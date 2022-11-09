# this file included from mkboost.sh

# which boost is to use. 1_80_0 will use file boost_1_80_0.tar.gz
boostver=1_80_0

# which libraries to build. Line passed directly bo './b2 install'
boostlibs="--with-context --with-system --with-fiber --with-program_options --with-stacktrace --with-filesystem"
