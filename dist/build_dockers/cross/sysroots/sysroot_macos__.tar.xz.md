# Extracting sysroot

This file will be named `sysroot_macos_x86_64.tar.xz`, and `sysroot_macos_arm64.tar.xz`. Notice, that is one and same file (so, you may just make one real archive, and second as symlink).

Follow instructions on `https://github.com/tpoechtrager/osxcross#packaging-the-sdk`.

You can clean-up SDK, as we actually don't need apple frameworks, and they includes huge part of the SDK. The only things we need are shown below:

```
$ tree -L 3
.
└── MacOSX11.3.sdk
    └── usr
        ├── include
        └── lib
```

Add file `osx.cmake`, rename it to `toolchain.cmake`

```bash
tar -cf - MacOSX11.3.sdk toolchain.cmake | xz -9 > sysroot_macos_arm64.tar.xz
ln -s sysroot_macos_arm64.tar.xz sysroot_macos_x86_64.tar.xz
```

If you have another version of SDK (not 11.3) you need also to correct `CMAKE_SYSROOT` in `osx.cmake`

Both archives (`sysroot_macos_x86_64.tar.xz`, and `sysroot_macos_arm64.tar.xz`) are results. They occupy about 3M.
They're stored in `/work/cross/sysroots` folder on our dev server, and also available online to build docker
from `https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/`.