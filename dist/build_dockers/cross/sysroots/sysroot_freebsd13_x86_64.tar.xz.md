# Extracting sysroot

On fresh FreeBSD 13 install packages:

```bash
portsnap fetch
portsnap extract
pkg install mariadb105-client postgresql-libpqxx unixODBC expat boost-libs
```

Add file `freebsd.cmake`, rename it to `toolchain.cmake`
Then pack `/usr` and `/lib` folders into tarball, like:

```bash
tar -cf - /usr /lib toolchain.cmake | xz -9 > sysroot_freebsd13_x86_64.tar.xz
```

Archive `sysroot_freebsd13_x86_64.tar.xz` (the only one; we don't have same for arm64) includes both sysroot and boost. That is because FreeBSD uses patched boost sources, so, we keep them intact.
Also, we don't pack `boost_freebsd*`, separately, that is intention. Build docker's entry point will report an error because of the absence of the file, but it is not fatal.
They're stored in `/work/cross/sysroots` folder on our dev server, and also available online to build docker
from `https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/`.