# Archive with Boost headers

See `README.md` in folder `sysroots` for details.

File `boost_include.tar`, produced according to this README, should be then compressed:

```bash
xz boost_include.tar
```

Resulting `boost_include_tar.zx` should be placed together into folder where Dockerfile lives.

All the files also stored in `/work/cross/sysroots` folder on our dev server.