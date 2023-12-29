# Galera packaging stuff

Here is helping stuff for packaging Galera for different distros.

Galera lives in own repo, and it's build is standalone.
However, for distribution that is more comfortable to have ready packages, then copy binary module.

These stuff is invoked specifically, when env PACK_GALERA is set, and also when build is configured with PACK=1 param,
like this:

```bash
# export DISTR=rhel8
# export arch=x86_64
export PACK_GALERA=1
mkdir build && cd build
cmake -DPACK=1 /path/to/manticore/sources
```

If all the conditions met, we merge content of this folder into galera-src/cmake folder, and then perform building of
packages with `cmake --build . --target package`. Finally, it produces a couple of packages - one with galera, second
with debuginfo.

These files distributed here to have more granular control over metainfo content, especially when we just add support of
a new distro.

* `packaging.cmake` is main entry point. It is included from Galera building
* `metainfo.cmake` provides metainfo for all the packages
* `description.cmake` is part of metainfo; some packages need it in separate file, not inlined
* `CommonDeb.cmake` provides debian-specific stuff
* `CommonRpm.cmake` provides rpm-specific stuff
* `build_*.cmake` provides individual distro environments