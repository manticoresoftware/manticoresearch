# Packaging subset of homebrew

This file will be named `boost_macos_x86_64.tar.xz`, and `boost_macos_arm64.tar.xz`, however it includes all need subset of homebrew, not just only Boost. That is because such naming scheme is used by build docker's entry point.

First, make full copy of homebrew (that is located in `/opt/homebrew` folder on arm (M1), or in `/usr/local` on intel systems).

Go to `Cellar` folder, and cleanup it from non-necessary packages.
You need:

* boost
* mysql-client
* openssl
* postgresql
* unixodbc
* zstd

In each package we need only `lib` and `include` subfolders, all the rest (like `bin`, `share`, and free files) we can safety drop.

You can go deeper and cleanup boost libraries. Leave `context`, `coroutine`, `fiber`, `filesystem`, `program_options`, `stacktrace` and `system`; also `headers` (it doesn't have binary lib, but `boost_headers-*` in `lib/cmake` folder is necessary). You can drop all lib/*.dylib, as we use only statically linked boost libraries.

Just keep folders with these names (or, starting with these names). Openssl could be `openssl@1.1`, or `openssl@3` - first
one is dependency for mysql-client, second one is standalone. It has sense to keep openssl@1.1 to avoid duplicates of
the same lib with two different versions.

The walk thorough 'include', 'lib', and 'opt' folders; cleanup all dead symlinks (as you remove folders from Cellar -
references from these folders will became dead).

Finally, go to folder `Cellar/boost/1.80.0/` and replace `include`, with symlink to `../../../../cache/common/boost/include`.

Below is the example how final folder tree may look. Notice, that is not necessary actual tree, but just provides you the idea how it should look.

```
.
├── Cellar
│   ├── boost
│   │   └── 1.80.0
│   │       ├── include -> ../../../../cache/common/boost/include
│   │       └── lib
│   ├── mysql-client
│   │   └── 8.0.28
│   │       ├── include
│   │       └── lib
│   ├── openssl@1.1
│   │   └── 1.1.1n
│   │       ├── include
│   │       └── lib
│   ├── openssl@3
│   │   └── 3.0.2
│   │       ├── include
│   │       └── lib
│   ├── postgresql
│   │   └── 14.2_1
│   │       ├── include
│   │       └── lib
│   ├── unixodbc
│   │   └── 2.3.9_1
│   │       ├── include
│   │       └── lib
│   └── zstd
│       └── 1.5.2
│           ├── include
│           └── lib
├── include
│   ├── autotest.h -> ../Cellar/unixodbc/2.3.9_1/include/autotest.h
│   ├── boost -> ../Cellar/boost/1.78.0_1/include/boost
│   ├── ecpg_config.h -> ../Cellar/postgresql/14.2_1/include/ecpg_config.h
│   ├── ecpgerrno.h -> ../Cellar/postgresql/14.2_1/include/ecpgerrno.h
│   ├── ecpg_informix.h -> ../Cellar/postgresql/14.2_1/include/ecpg_informix.h
│   ├── ecpglib.h -> ../Cellar/postgresql/14.2_1/include/ecpglib.h
...
│   ├── zstd_errors.h -> ../Cellar/zstd/1.5.2/include/zstd_errors.h
│   └── zstd.h -> ../Cellar/zstd/1.5.2/include/zstd.h
├── lib
│   ├── cmake
│   │   ├── Boost-1.80.0
│   │   ├── boost_context-1.80.0
│   │   ├── boost_coroutine-1.80.0
│   │   ├── BoostDetectToolset-1.80.0.cmake -> ../../Cellar/boost/1.80.0/lib/cmake/BoostDetectToolset-1.80.0.cmake
│   │   ├── boost_fiber-1.80.0
│   │   ├── boost_headers-1.80.0
│   │   ├── boost_program_options-1.80.0
│   │   ├── boost_stacktrace_addr2line-1.80.0
│   │   ├── boost_stacktrace_basic-1.80.0
│   │   ├── boost_stacktrace_noop-1.80.0
│   │   ├── boost_system-1.80.0
│   │   └── zstd -> ../../Cellar/zstd/1.5.2/lib/cmake/zstd
│   ├── libboost_context-mt.a -> ../Cellar/boost/1.80.0/lib/libboost_context-mt.a
│   ├── libboost_context-mt.dylib -> ../Cellar/boost/1.80.0/lib/libboost_context-mt.dylib
...
│   ├── libzstd.dylib -> ../Cellar/zstd/1.5.2/lib/libzstd.dylib
│   ├── pkgconfig
│   └── postgresql -> ../Cellar/postgresql/14.2_1/lib/postgresql
└── opt
    ├── boost -> ../Cellar/boost/1.80.0
    ├── boost@1.78 -> ../Cellar/boost/1.80.0
    ├── mysql-client -> ../Cellar/mysql-client/8.0.28
    ├── mysql-client@8.0 -> ../Cellar/mysql-client/8.0.28
    ├── openssl -> ../Cellar/openssl@3/3.0.2
    ├── openssl@1.1 -> ../Cellar/openssl@1.1/1.1.1n
    ├── openssl@3 -> ../Cellar/openssl@3/3.0.2
    ├── postgres -> ../Cellar/postgresql/14.2_1
    ├── postgresql -> ../Cellar/postgresql/14.2_1
    ├── postgresql@14 -> ../Cellar/postgresql/14.2_1
    ├── pzstd -> ../Cellar/zstd/1.5.2
    ├── unixodbc -> ../Cellar/unixodbc/2.3.9_1
    └── zstd -> ../Cellar/zstd/1.5.2

```

Pack the content; it should have folders `Cellar`, `include`, `lib`, and `opt` in the root.

```bash
tar -cf - Cellar include lib opt | xz -9 -f > boost_macos_ARCH.tar.xz
```
( where `ARCH` is `arm64` on M1 machine, or `x86_64` on intel. )

Both archives (`boost_macos_x86_64.tar.xz`, and `boost_macos_arm64.tar.xz`) are results. They occupy about 9M each.
They're stored in `/work/cross/sysroots` folder on our dev server, and also available online to build docker from `https://repo.manticoresearch.com/repository/sysroots/boost_178/`.
