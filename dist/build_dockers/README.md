# Official build dockers

This directory contains sources for docker images which are used for building official packages of Manticoresearch.

Go to any interesting directory and look the content of `Dockerfile`.
At the bottom you will see the command which will build docker image - so, if you do everything by hands, just print
the Dockerfile and take the last line as rule. The name of the image is the one used in `dist/gitlab-release.yml` for
actual builds. Feel free to make shorter name, if desired.

## Building package in docker

There are two flavours: redhat-based (centosN) and debian-based (the rest).
Debian provides basic build sequence, if you perform these steps you'll take package similar to official release
(apart possible signing, date of creation and build machine name). On redhat we also add step to make path quite long,
since rpm build tools need it for making debug symbol packages, however it is already set as `WORKDIR`, so no interaction
usually need.

### Example of building package, same as release

```bash
mkdir build && cd build
cmake -DPACK=1 /path/to/sources
make -j4 package
```

### Example of building package using system ICU instead of embedded

```bash
mkdir build && cd build
cmake -DPACK=1 -DSPHINX_TAG="noicu" -DWITH_ICU_FORCE_STATIC=0 /path/to/sources
make -j4 package
```

### Building rpm for redhat/centos

If you want to build final packages, note that cmake packs debuginfo with substituting textual path directly
in built binaries. That need original paths to be quite long (since it can't expand them). To achieve it we provide
special long-named directory which is workdir when you run docker. Just link or copy sources to that folder and build
from it, as:

```bash
ln -s $(pwd) /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789/src
cd /manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789/src
# ... (etc)
```

### Using for custom build/debug

Add aliases to your `~/.bash_aliases`, like:

```bash
alias buildcentos7='docker run -it --rm -v /manticore/sources:/manticore registry.gitlab.com/manticoresearch/dev/centos7_cmake bash'
alias buildxenial='docker run -it --rm -v /manticore/sources:/manticore registry.gitlab.com/manticoresearch/dev/xenial_cmake bash'
# ... (etc)
```

Then simple typing
```bash
buildxenial
```
will run docker for building xenial image, mount sources to the docker and run shell inside it.

### Couple of internal details

Each docker made from 3 layers:
1. The deepest - essential build stuff and libraries, except boost and icu. Once built it should not be rebuild anymore,
since packets are changed from time to time, and similar run to build the image in future will not produce the same
image. Let's keep things consistent! Also it defines env `DISTR` which is used in builds instead of manual ping-poing,
and maybe `CXXFLAGS` to provide compiler-specific flags, if they necessary for that distro to build in.
2. Layer with boost library. We use boost built from sources. That is dedicated layer to update the boost if we need it.
3. Layer with cmake and expiring things (mc, ca-certificates). They are changing quite often. 

Also 3 oneline scripts provided to build each layer, and one `build.sh` to rule them all (however line for building base
image is commented there, since intented to be used only once). `distr.txt` contains name of the distr, however it is
also automated by taking name of the folder where all this stuff placed.