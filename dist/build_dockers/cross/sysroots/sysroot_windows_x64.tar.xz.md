# Extracting sysroot on windows

Look to the header of `windows.cmake` or `windows-cl.cmake`, follow instruction.

Final structure of SDK should look like:

```
$ tree -L 5
.
├── VC
│   └── Tools
│       └── MSVC
│           └── 14.25.28610
│               ├── include
│               └── lib
├── Windows Kits
│   └── 10
│       ├── Include
│       │   └── 10.0.18362.0
│       │       ├── cppwinrt
│       │       ├── shared
│       │       ├── ucrt
│       │       ├── um
│       │       └── winrt
│       └── Lib
│           └── 10.0.18362.0
│               ├── ucrt
│               ├── ucrt_enclave
│               └── um
└── WinSDK -> Windows Kits
```

Notice symlink to `Windows Kits` named `WinSDK` (you should create it yourself, it is used in `windows.cmake` toolchain).

Also notice version numbers (14.25.28610, 10.0.18362.0). If you have different, find them in toolchain files and correct to the one you have.

Go ahead, below is structure of the root of the archive:

```
$ tree -L 3
.
├── diskc
│   └── winbundle
│       ├── Expat-1.95.8-x64
│       │   ├── Libs
│       │   └── Source
│       ├── EXPATConfig.cmake
│       ├── IconvConfig.cmake
│       ├── libiconv-1.9.1-x64
│       │   ├── include
│       │   └── lib
│       ├── MysqlConfig.cmake
│       ├── mysql-x64
│       │   ├── include
│       │   └── lib
│       ├── pgsql-x64
│       │   ├── bin
│       │   ├── include
│       │   └── lib
│       └── PostgreSQLConfig.cmake
└── sdk
    ├── VC
    │   └── Tools
    ├── Windows Kits
    │   └── 10
    └── WinSDK -> Windows Kits
```

You see folder `sdk` we packed. Second one is `diskc` which includes `winbundle` with necessary modules. `*.cmake` files there used as modules to `find_package`, they just created imported targets, addressing content of the real folder. We thrown out all content not necessary for build.

Add file `windows.cmake`, rename it to `toolchain.cmake`

On finish, pack it into archive. It should contain VC and WinSDK folders in the root.

```bash
tar -cf - diskc sdk toolchain.cmake | xz -9 > sysroot_windows_x64.tar.xz
```

Notice, arch here is 'x64', that is Windows naming.

Archive `sysroot_windows_x64.tar.xz` (the only one; we don't have same for arm64) is the target.
It is stored in `/work/cross/sysroots` folder on our dev server, and also available online to build docker
from `https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/`.