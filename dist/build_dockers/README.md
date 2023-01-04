# Official build docker

This directory contains the sources for a Docker image that is used for building official packages of Manticore Search. It also includes instructions and scripts for extracting and packing the necessary files for different distros/packages.

At the bottom of the `Dockerfile`, you will find the commands you need to run to build the image. The name of the image is the one used in `dist/gitlab-release.yml` for actual builds. If you wish, you can make the name shorter.

Note that Docker is **NOT** standalone and requires an internet connection to fetch sysroot archives. However, you can have a copy of the archive available locally and adjust the URL accordingly.

## Running the docker

You need to provide a few environment variables: `DISTR`, `arch`, and `SYSROOT_URL` and some others. It's best to use the CI yaml file as a reference - https://github.com/manticoresoftware/manticoresearch/blob/master/dist/gitlab-release.yml

For example,
```bash
docker run -it --rm \
-v $(pwd):/manticore \
-e DISTR=bullseye \
-e arch=x86_64 \
-e SYSROOT_URL=https://repo.manticoresearch.com/repository/sysroots \
-e boosboost=boost_nov22 \
-e sysroot=roots_nov22 \
93ba969c68b2 bash
```
Also, you will most likely need to mount a folder with sources if you are not going to fetch them another way, like with git. Just add `-v /manticore/on/host:/manticore`, or something similar, to have the `/manticore/` folder with sources mounted inside the docker.

### Building package, same as release

```bash
cd /path/to/sources
mkdir build && cd build
cmake -DPACK=1 ..
cmake --build . --target package
```

### Some internal details

Docker includes only the build system and is agnostic to the target platform. However, with the docker manifest, it is possible to create a multi-arch image (see details at the bottom of the Dockerfile).

On start of the container, it fetches the necessary artifacts of the target platform and architecture (i.e., sysroot, boost libraries, boost headers, and toolchain file).

The build is performed via cross-compilation with clang. The toolchain file, provided in the system-root archive, is defined via the `CMAKE_TOOLCHAIN_FILE` env. var. and is picked automatically. It determines everything necessary to build the target binaries and packages.

Clang is powerful enough to produce binaries for Mac OS and Windows, in addition to the usual elf executables for Linux/FreeBSD, so it is quite common to run Docker on a Raspberry PI and create a distribution for MS Windows.
