.. _reporting_bugs:

Reporting bugs
==========================

Unfortunately, Manticore is not yet 100% bug free (even though we're
working hard towards that), so you might occasionally run into some
issues.

Reporting as much as possible about each bug is very important - because
to fix it, we need to be able either to reproduce and fix the bug, or to
deduce what's causing it from the information that you provide. So here
are some instructions on how to do that.

Bug-tracker
-----------
Please issue the Github issue tracker https://github.com/manticoresoftware/manticore/issues. Create a new ticket and
describe your bug in details so both you and developers can save their
time.

Crashes
-------

In case of crashes we sometimes can get enough info to fix from
backtrace.

Manticore tries to write crash backtrace to its log file. It may look like
this:

::


    ./indexer(_Z12sphBacktraceib+0x2d6)[0x5d337e]
    ./indexer(_Z7sigsegvi+0xbc)[0x4ce26a]
    /lib64/libpthread.so.0[0x3f75a0dd40]
    /lib64/libc.so.6(fwrite+0x34)[0x3f74e5f564]
    ./indexer(_ZN27CSphCharsetDefinitionParser5ParseEPKcR10CSphVectorI14CSphRemapRange16CSphVe
    ctorPolicyIS3_EE+0x5b)[0x51701b]
    ./indexer(_ZN13ISphTokenizer14SetCaseFoldingEPKcR10CSphString+0x62)[0x517e4c]
    ./indexer(_ZN17CSphTokenizerBase14SetCaseFoldingEPKcR10CSphString+0xbd)[0x518283]
    ./indexer(_ZN18CSphTokenizer_SBCSILb0EEC1Ev+0x3f)[0x5b312b]
    ./indexer(_Z22sphCreateSBCSTokenizerv+0x20)[0x51835c]
    ./indexer(_ZN13ISphTokenizer6CreateERK21CSphTokenizerSettingsPK17CSphEmbeddedFilesR10CSphS
    tring+0x47)[0x5183d7]
    ./indexer(_Z7DoIndexRK17CSphConfigSectionPKcRK17SmallStringHash_TIS_EbP8_IO_FILE+0x494)[0x
    4d31c8]
    ./indexer(main+0x1a17)[0x4d6719]
    /lib64/libc.so.6(__libc_start_main+0xf4)[0x3f74e1d8a4]
    ./indexer(__gxx_personality_v0+0x231)[0x4cd779]

This is an example of a good backtrace - we can see mangled function
names here.

But sometimes backtrace may look like this:

::


    /opt/piler/bin/indexer[0x4c4919]
    /opt/piler/bin/indexer[0x405cf0]
    /lib/x86_64-linux-gnu/libpthread.so.0(+0xfcb0)[0x7fc659cb6cb0]
    /opt/piler/bin/indexer[0x4237fd]
    /opt/piler/bin/indexer[0x491de6]
    /opt/piler/bin/indexer[0x451704]
    /opt/piler/bin/indexer[0x40861a]
    /opt/piler/bin/indexer[0x40442c]
    /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xed)[0x7fc6588aa76d]
    /opt/piler/bin/indexer[0x405b89]

Developers can get nothing useful from those cryptic numbers. They're
ordinary humans and want to see function names. To help them you need to
provide symbols (function and variable names). If you've installed
sphinx by building from the sources, run the following command over your
binary:

::


    nm -n indexer > indexer.sym

Attach this file to bug report along with backtrace. You should however
ensure that the binary is not stripped. Our official binary packages
should be fine. (That, or we have the symbols stored.) However, if you
manually build Manticore from the source tarball, do not run ``strip``
utility on that binary, and/or do not let your build/packaging system do
that!

Uploading your data
-------------------

To fix your bug developers often need to reproduce it on their machines.
To do this they need your sphinx.conf, index files, binlog (if present),
sometimes data to index (like SQL tables or XMLpipe2 data files) and
queries.

Attach your data to ticket. In case it's too big to attach ask
developers and they give you an address to write-only FTP created
exactly for such puproses.
