# Extracting sysroot

On fresh FreeBSD 13 install packages:

```bash
portsnap fetch
portsnap extract
pkg install mariadb105-client postgresql-libpqxx unixODBC expat boost-libs
```

Then pack `/usr` and `/lib` folders into tarball, like:

```bash
tar -cf - /usr /lib | xz -f > /sysroot_fbsd_13.tar.xz
```
Store that archive and put aside Dockerfile.

Prepared archive is also stored on our dev server, in `/work/docker` folder.