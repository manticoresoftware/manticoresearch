# Set of scripts to create Debian & Ubuntu sysroots

These sysroots are necessary to cross-compile for debian and ubuntu targets.
Scripts need to be run on linux with docker and zstd (optional). Arch of the host (run `uname -m` to see) determines arch of produced sysroots.

Prepare clean folder where sysroots will be collected.

Place there content of this filder.

Place there archive with boost sources. Archive may be fetched, for example, with command
```bash
wget https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.gz
```

Make all `build_*.sh` executable:
```bash
chmod +x build_*.sh
```
Run any executable. On complete it will leave you a folder, according to your arch, with created sysroot, and also `boost_include.tar` with boost include headers, they're not depends on arch.

Then you may edit the file `mkboost.sh` and comment out line which create these headers, as they're all the same every time, no need to waste time regenerating it again and again.

Run the rest of executables. They will produce collection of sysroots for ubuntu (xenial, bionic, focal, jammy) and debian (stretch, bullseye, buster).

To produce sysroots for another arch, you need to repeat all the steps on the host with target arch and same prerequisites (docker, zstd). You may comment out packaging of boost headers there, as one archive with headers is enough for all architectures.

## What is created and how to manage
Every script produces two archives - one with sysroot, and one with compiled libraries of boost.
Both placed in folder according to arch.
For example, on `arm64` (aka `aarch64`) system, `build_jammy.sh` will produce two files:

```bash
aarch64/boost_jammy_aarch64.tar.zst
aarch64/sysroot_jammy_aarch64.tar.zst
```

the same script run on intel (aka `x86_64`) will produce

```bash
x86_64/boost_jammy_x86_64.tar.zst
x86_64/sysroot_jammy_x86_64.tar.zst
```

and so on.

`boost*.tar.zstd` are tiny (about 200k each), `sysroot*.tar.zst` bigger (about 30-80M each). Zstd archive format is
choosen arbitrary as quite effective and fast even on slow systems (like raspberry pi). When you collect all sysroots,
you may repack them into much more effective `*.tar.zx` (aka `.txz`). xz archiver is much slower, but will pack your
sysroots in about x2 times comparing to zstd.

On the host machine you need then repack archives with `xz`, and also add file `linux.cmake`, renamed to `toolchain.cmake` to the root of content of `sysroot*` archives (that is - aside `usr`, `lib`, and whatever in the root of sysroot):

```
.
├── lib
├── toolchain.cmake
└── usr
```

Repacking may be performed with something like

```bash
tar -cf - usr lib toolchain.cmake | xz -9> sysroot_bionic_aarch64.tar.xz
```

Full collection of all sysroots + boost + boost_include, for all distros, for x86_64 + aarch64, repacked with `zx -9`,
takes about 550M of space.

Set of packages included into sysroots are determined by `*.apt` files. There is one common `ubuntu.apt`, and separate
for debians (`stretch.apt`, `buster.apt`, `bullseye.apt`). Notice, they includes only necessary dependencies and nothing
more.

## Technical details

We settle into basic docker container, as `ubuntu:xenial`, or `debian:buster`. Then we install package `build-essential` and build boost with it. On finish we pack includes (which may be commented out) and built libraries.

Then we install another packages need to be embedded into sysroot. Command for it is determined in arbitrary *.apt file.

We patch important symlinks from absolute paths into relative (usually there are only few of them), and also patch a couple of ld scripts to also use relative paths.

Finally, we pack sysroot into tarball and leave the docker. Outside we pack collected tarballs with zstd, and job is done.
