# Official build docker

This directory contains sources for docker image which is used for building official packages of Manticoresearch, and also instructions and scripts to extract and pack necessary stuff for different distros/packages.

At the bottom of `Dockerfile` there is commands you need to run t build the image. The name of the image is the one used in `dist/gitlab-release.yml` for actual builds. Feel free to make shorter name, if desired.

Notice, docker is **NOT** standalone, it needs internet to fetch sysroot archives. However you can have your copy of that archive available somewhere and adjust the url.

## Running the docker

You need to provide 3 env variables: `DISTR`, `arch`, and `SYSROOT_URL`.

For example,
```bash
docker run -it --rm -v /sphinx/sphinxfrommac:/manticore \
-e DISTR=bionic \
-e arch=x86_64 \
-e SYSROOT_URL=https://repo.manticoresearch.com/repository/sysroots \
registry.gitlab.com/manticoresearch/dev/external_toolchain:clang13_cmake3232 bash
```
Also you most probably need to mount folder with sources, if you're not going to fetch them another way, like with git. Just add `-v /manticore/on/host:/manticore`, or something similar to have folder `/manticore` with sources mounted inside the docker.

### Building package, same as release

```bash
cd /path/to/sources
mkdir build && cd build
cmake -DPACK=1 ..
cmake --build . --target package
```

### Some internal details

Docker includes only build system, agnostic to target platform. Docker itself is targeted to arch of the machine where it is built, however with docker manifest it is possible to create multi-arch image (see details in the bottom of Dockerfile).

On entry point we fetch necessary artifacts of target platform and architecture (that is - sysroot, boost libraries, boost headers, and toolchain file).

Build itself is performed via cross-compilation with clang-13. Toolchain file, provided in system-root archive, is defined via `CMAKE_TOOLCHAIN_FILE` env, and is picked automatically. It files determines everything necessary to build target binaries and packages.

Clang-13 is powerful enough to produce binaries for Mac OS and Windows, aside usual elf executables for linux/freebsd, so it is quite usual, say to run docker on Raspberry PI and create distribution for MS Windows.