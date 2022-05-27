# Archive with arm64 sysroots

See `README.md` in folder `sysroots` for details.

All files `*_aarch64.tar`, produced according to this README, should be then compressed:

```bash
for a in *._aarch64.tar; do xz -9 $a; done
```

Resulting files should be placed together into folder where Dockerfile lives.
Note, compression level is important here, as files go packed directly into docker image.

All the files also stored in `/work/cross/sysroots` folder on our dev server.