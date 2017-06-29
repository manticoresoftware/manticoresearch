#! /bin/sh

autoheader \
&& aclocal  \
&& automake --foreign --add-missing \
&& autoconf
