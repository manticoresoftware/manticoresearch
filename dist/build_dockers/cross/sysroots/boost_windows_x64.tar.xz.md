# Extracting Boost on windows

Boost for windows is official download from `boost.org`.

OpenSSL is taken from `https://slproweb.com/download/Win64OpenSSL-1_1_1k.msi`

Both packed int `boost` archive with following structure:

```
.
└── diskc
    ├── boost_1_78_0
    │   ├── boost -> ../../../cache/common/boost/include/boost
    │   └── lib64-msvc-14.2
    ├── OpenSSLConfig.cmake
    └── OpenSSL-Win64
        ├── include
        ├── lib
        ├── libcrypto-1_1-x64.dll
        └── libssl-1_1-x64.dll
```

Notice, boost headers are removed and replaced with symlink. You should do it to avoid duplicates (target of that symlink came from `boost_include.tar.xz`). Also, you can clean-up boost from unnecessary libraries; follow the same recommendations as for `boost_macos...`.

Pack the content into archive
```bash
tar -cf - diskc | xz -9 -f > boost_windows_x64.tar.xz
```

Archive `boost_windows_x64.tar.xz` is the target.
It stored in `/work/cross/sysroots` folder on our dev server, and also available online to build docker
from `https://repo.manticoresearch.com/repository/sysroots/boost_178/`.
