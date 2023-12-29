# Galera packaging stuff

Here are some helpful resources for packaging Galera for different distributions.

Galera lives in its own repository, and its build is standalone.
However, for distributions, it is more comfortable to have ready packages rather than copying binary modules.

These steps are invoked specifically when the environment variable `PACK_GALERA` is set, and also when the build is configured with `PACK=1` parameter, like this:

```bash
# export DISTR=rhel8
# export arch=x86_64
export PACK_GALERA=1
mkdir build && cd build
cmake -DPACK=1 /path/to/manticore/sources
```

If all the conditions are met, we merge the content of this folder into the `galera-src/cmake` folder, and then  perform the building of packages with `cmake --build . --target package`. Finally, it produces a couple of packages - one with Galera, and the second with debug info.

These files are distributed here to have more granular control over the metadata content, especially when we are just adding support for a new distribution.

* `packaging.cmake` is the main entry point. It is included from the Galera build.
* `metainfo.cmake` provides metadata for all the packages.
* `description.cmake` is part of the metadata; some packages need it in a separate file, not inlined.
* `CommonDeb.cmake` provides Debian-specific stuff.
* `CommonRpm.cmake` provides RPM-specific stuff.
* `build_*.cmake` provides individual distribution environments.

## CI for Packaging and Publishing Galera Packages

The GitHub CI workflow file is located in `.github/workflows/pack_publish_galera.yml`. It's triggered when `cmake/GetGALERA.cmake` or `galera_packaging/*` is updated and one of the following conditions is met:
- Pull request to the `master` branch and:
  - Label `pack` to build the packages
  - Label `publish` to upload the packages to https://repo.manticoresearch.com/
- Push to one of the following branches: `master`, `maintenance-release`, `manticore-*`
