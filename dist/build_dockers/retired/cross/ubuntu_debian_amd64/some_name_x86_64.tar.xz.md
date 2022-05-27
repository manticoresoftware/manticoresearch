# Archive with amd64 sysroots

See `README.md` in folder `sysroots` for details.

All files `*_x86_64.tar`, produced according to this README, should be then compressed:

```bash
for a in *._x86_64.tar; do xz -9 $a; done
```

Resulting files should be placed together into folder where Dockerfile lives.
Note, compression level is important here, as files go packed directly into docker image.

All the files also stored in `/work/cross/sysroots` folder on our dev server.