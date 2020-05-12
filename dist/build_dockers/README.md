# Official build dockers

This directory contains sources for docker images which are used for building official packages of Manticoresearch.

Go to any interesting directory and look the content of `Dockerfile`.
At the top you will see the command which will build docker image.
The name of the image is the one used in `dist/gitlab-release.yml` for actual builds. Feel free to make shorter name,
if desired.

Before building docker image you need to download cmake archive and place it near Dockerfile (see the address there).
That is done this way, since one and same archive used for all builders, and we're too lazy to download it each time.

## Building package in docker

There are two flavours: redhat-based (centosN) and debian-based (the rest).
Debian provides basic build sequence, if you perform these steps you'll take package similar to official release
(apart possible signing, date of creation and build machine name).
On redhat we also add step to make path quite long, since rpm build tools need it for making debug symbol packages.


### Example of building deb for debian/ubuntu, relying on system ICU instead of embedded one

```bash
RELEASE_TAG="noicu"
mkdir build && cd build
cmake -DSPHINX_TAG=$RELEASE_TAG -DDISTR_BUILD=$DISTR -DWITH_ICU_FORCE_STATIC=0 ..
make -j4 package
```

### Building rpm for redhat/centos

```bash
ln -s $(pwd) /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789/src
cd /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789/src

# below is common for redhat and debian
RELEASE_TAG="noicu"
mkdir build && cd build
cmake -DSPHINX_TAG=$RELEASE_TAG -DDISTR_BUILD=$DISTR -DWITH_ICU_FORCE_STATIC=0 ..
make -j4 package
```

### Using for custom build/debug

Add aliases to your `~/.bash_aliases`, like:

```bash
alias buildcentos7='docker run -it --rm -v /manticore/sources:/manticore registry.gitlab.com/manticoresearch/dev/centos7_cmake314 bash'
alias buildxenial='docker run -it --rm -v /manticore/sources:/manticore registry.gitlab.com/manticoresearch/dev/xenial_cmake314 bash'
# ... (etc)
```

Then simple typing
```bash
buildxenial
```
will run docker for building xenial image, mount sources to the docker and run shell inside it.