# Compiling Manticore from source

Compiling Manticore Search from sources enables custom build configurations, such as disabling certain features or adding new patches for testing. For example, you may want to compile from sources and disable the embedded ICU in order to use a different version installed on your system that can be upgraded independently of Manticore. This is also useful if you are interested in contributing to the Manticore Search project.

## Building using CI Docker
To prepare [official release and development packages](https://repo.manticoresearch.com/), we use Docker and a special building image. This image includes essential tooling and is designed to be used with external sysroots, so one container can build packages for all operating systems. You can build the image using the [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) and [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) or use an image from [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags). This is the easiest way to create binaries for any supported operating system and architecture. You'll also need to specify the following environment variables when running the container:

* `DISTR`: the target platform: `bionic`, `focal`, `jammy`, `buster`, `bullseye`, `bookworm`, `rhel7`, `rhel8`, `rhel9`, `macos`, `windows`, `freebsd13`
* `arch`: the architecture: `x86_64`, `x64` (for Windows), `aarch64`, `arm64` (for Macos)
* `SYSROOT_URL`: the URL to the system roots archives. You can use https://repo.manticoresearch.com/repository/sysroots unless you are building the sysroots yourself (instructions can be found [here](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)).
* Use the CI workflow files as a reference to find the other environment variables you might need to use:
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

To find possible values for `DISTR` and `arch`, you can use the directory https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/ as a reference, as it includes sysroots for all supported combinations.

After that, building packages inside the Docker container is as easy as calling:

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

For instance, to create a package for Ubuntu Jammy that is similar to the official version Manticore Core Team provides, you should execute the following commands in the directory containing the Manticore Search sources. This directory is the root of a cloned repository from https://github.com/manticoresoftware/manticoresearch:

```bash
docker run -it --rm \
-e CACHEB="../cache" \
-e DIAGNOSTIC=1 \
-e PACK_ICUDATA=0 \
-e NO_TESTS=1 \
-e DISTR=jammy \
-e boost=boost_nov22 \
-e sysroot=roots_nov22 \
-e arch=x86_64 \
-e CTEST_CMAKE_GENERATOR=Ninja \
-e CTEST_CONFIGURATION_TYPE=RelWithDebInfo \
-e WITH_COVERAGE=0 \
-e SYSROOT_URL="https://repo.manticoresearch.com/repository/sysroots" \
-e HOMEBREW_PREFIX="" \
-e PACK_GALERA=0 \
-e UNITY_BUILD=1 \
-v $(pwd):/manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa \
manticoresearch/external_toolchain:vcpkg331_20250114 bash

# following is to be run inside docker shell
cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/
mkdir build && cd build
cmake -DPACK=1 ..
export CMAKE_TOOLCHAIN_FILE=$(pwd)/dist/build_dockers/cross/linux.cmake
cmake --build .
# or if you want to build packages:
# cmake --build . --target package
```
The long source directory path is required or it may fail to build the sources in some cases (e.g. Centos).

The same process can be used to build binaries/packages not only for popular Linux distributions, but also for FreeBSD, Windows, and macOS.

## Building manually

Compiling Manticore without using the building Docker is **not recommended**, but if you need to do it, here's what you may need to know:

### Required tools

* C++ compiler
  * In Linux - GNU (4.7.2 and above) or Clang can be used
  * In Windows - Microsoft Visual Studio 2019 and above (community edition is enough)
  * On macOS - Clang (from command line tools of XCode, use `xcode-select --install` to install).
* Bison, Flex - on most systems, they are available as packages, on Windows they are available in the cygwin framework.
* Cmake - used on all platforms (version 3.19 or above required)

### Fetching sources

#### From git

Manticore source code is [hosted on GitHub](https://github.com/manticoresoftware/manticoresearch).   
To obtain the source code, clone the repository and then check out the desired branch or tag. The branch `master` represents the main development branch. Upon release, a versioned tag is created, such as `3.6.0` and a new branch for the current release is started, in this case `manticore-3.6.0`. The head of the versioned branch after all changes is used as source to build all binary releases. For example, to take sources of version 3.6.0 you can run:

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### From archive

You can download the desired code from GitHub by using the "Download ZIP" button. Both .zip and .tar.gz formats are suitable.

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### Configuring

Manticore uses CMake. Assuming you are inside the root directory of the cloned repository:

```bash
mkdir build && cd build
cmake ..
```

CMake will investigate available features and configure the build according to them. By default, all features are considered enabled if they are available. The script also downloads and builds some external libraries, assuming that you want to use them. Implicitly, you get support for the maximal number of features.

You can also configure the build explicitly with flags and options.  To enable feature `FOO` add `-DFOO=1` to the CMake call.
To disable it, use `-DFOO=0`. If not explicitly noted, enabling a feature that is not available((such as `WITH_GALERA` on an MS Windows build)) will cause the configuration to fail with an error. Disabling a feature, apart from excluding it from the build, also disables its investigation on the system and disables the downloading/building of any related external libraries.

#### Configuration flags and options

- **USE_SYSLOG** - allows the use of`syslog` in [query logging](Logging/Query_logging.md).
- **WITH_GALERA** -Enables support for replication on the search daemon. Support will be configured for the build, and the sources for the Galera library will be downloaded, built, and included in the distribution/installation. Usually, it is safe to build with Galera, but not distribute the library itself (so no Galera module, no replication). However, sometimes you may need to explicitly disable it, such as if you want to build a static binary that by design cannot load any libraries, so that even the presence of a call to the 'dlopen' function inside the daemon will cause a link error.
- **WITH_RE2** - Builds with the use of the RE2 regular expression library. This is necessary for functions like [REGEX()](../Functions/String_functions.md#REGEX%28%29), and the [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
  feature.
- **WITH_RE2_FORCE_STATIC** - Downloads the sources of RE2, compiles them, and links them statically, so that the final binaries will not depend on the presence of a shared  `RE2` library in your system.
- **WITH_STEMMER** - Builds with the use of the Snowball stemming library.
- **WITH_STEMMER_FORCE_STATIC** - Downloads the Snowball sources, compiles them, and links them statically, so that the final binaries will not depend on the presence of a shared  `libstemmer` library in your system.
- **WITH_ICU** -  Builds with the ICU (International Components for Unicode) library. It is used for segmenting Chinese text. It is utilized when morphology=`icu_chinese` is used.
- **WITH_JIEBA** -  Builds with the Jieba Chinese text segmentation tool. It is used for segmenting Chinese text. It is utilized when morphology=`jieba_chinese` is used.
- **WITH_ICU_FORCE_STATIC** - Downloads the ICU sources, compiles them, and links them statically, so that the final binaries will not depend on the presence of a shared `icu` library in your system. Also includes the ICU data file into the installation/distribution. The purpose of a statically linked ICU is to have a library of a known version, so that behavior is determined and not dependent on any system libraries. You will most likely prefer to use the system ICU instead, as it may be updated over time without the need to recompile the Manticore daemon. In this case, you need to explicitly disable this option. This will also save you some space occupied by the ICU data file (about 30M), as it will not be included in the distribution.
- **WITH_SSL** - Used for support for HTTPS, and also encrypted MySQL connections to the daemon. The system OpenSSL library will be linked to the daemon. This implies that OpenSSL will be required to start the daemon. This is mandatory for support for HTTPS, but not strictly mandatory for the server (i.e. no SSL means no possibility to connect via HTTPS, but other protocols will work). SSL library versions starting from 1.0.2 to 1.1.1 may be used by Manticore,  however note that **for the sake of security it's highly recommended to use the freshest possible SSL
  library**. For now only v1.1.1 is supported, the rest are outdated (
  see [openssl release strategy](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - used by the indexer to work with compressed columns from MySQL. Used by the daemon to provide support for the compressed MySQL protocol.
- **WITH_ODBC** - used by the indexer to support indexing sources from ODBC providers (they're typically UnixODBC and iODBC). On MS Windows, ODBC is the proper way to work with MS SQL sources, so indexing of `MSSQL` also implies this flag.
- **DL_ODBC** - don't link with the ODBC library. If ODBC is linked, but not available, you can't start indexer tool even if you want to process something not related to ODBC. This option asks the indexer to load the library at runtime only when you want to deal with ODBC source.
- **ODBC_LIB** - name of the ODBC library file. The indexer will try to load that file when you want to process ODBC source. This option is written automatically from available ODBC shared library investigation. You can also override that name at runtime by providing the environment variable `ODBC_LIB` with the proper path to an alternative library before running the indexer.
- **WITH_EXPAT** - used by the indexer to support indexing xmlpipe sources.
- **DL_EXPAT** - don't link with the EXPAT library. If EXPAT is linked, but not available, you can't start `indexer` tool even if you want to process something not related to xmlpipe. This option asks the indexer to load the library at runtime only when you want to deal with xmlpipe source.
- **EXPAT_LIB** - name of the EXPAT library file. The indexer will try to load that file when you want to process xmlpipe source. This option is written automatically from available EXPAT shared library investigation. You can also override that name at runtime by providing the environment variable EXPAT_LIB with the proper path to an alternative library before running the indexer.
- **WITH_ICONV** - for support different encodings when indexing xmlpipe sources with the indexer.
- **DL_ICONV** - don't link with the iconv library. If iconv is linked, but not available, you can't start `indexer` tool even if you want to process something not related to xmlpipe. This option asks the indexer to load the library at runtime only when you want to deal with xmlpipe source.
- **ICONV_LIB** - name of the iconv library file. The indexer will try to load that file when you want to process xmlpipe source. This option is written automatically from the available iconv shared library investigation. You can also override that name at runtime by providing the environment variable `ICONV_LIB` with the proper path to an alternative library before running the indexer.
- **WITH_MYSQL** - used by the indexer to support indexing MySQL sources.
- **DL_MYSQL** - don't link with the MySQL library. If MySQL is linked, but not available, you can't start the`indexer` tool even if you want to process something not related to MySQL. This option asks the indexer to load the library at runtime only when you want to deal with a MySQL source.
- **MYSQL_LIB** -- name of the MySQL library file. The indexer will try to load that file when you want to process a MySQL source. This option is written automatically from the available MySQL shared library investigation. You can also override that name at runtime by providing the environment variable `MYSQL_LIB` with the proper path to an alternative library before running the indexer.
- **WITH_POSTGRESQL** - used by the indexer to support indexing PostgreSQL sources.
- **DL_POSTGRESQL** - don't link with the PostgreSQL library. If PostgreSQL is linked, but not available, you can't start the `indexer` ool even if you want to process something not related to PostgreSQL. This option asks the indexer to load the library at runtime only when you want to deal with a PostgreSQL source.
- **POSTGRESQL_LIB** - name of postgresql library file. The indexer will attempt to load the specified postgresql library file when processing a postgresql source. This option is automatically determined from available postgresql shared library investigation. You can also override the name at runtime by providing the environment variable  `POSTGRESQL_LIB` with the proper path to an alternative library before running the indexer.
- **LOCALDATADIR** - default path where the daemon stores binlogs. If this path is not provided or explicitly disabled in the daemon's runtime config (i.e. the file `manticore.conf`, which is not related to this build configuration), binlogs will be placed in this path. It is typically an absolute path, however, it is not required to be and relative paths can also be used. You probably would not need to change the default value defined by the configuration, which, depending on the target system, might be something like `/var/data`, `/var/lib/manticore/data`, or `/usr/local/var/lib/manticore/data`.
- **FULL_SHARE_DIR** - default path where all assets are stored. It can be overridden by the environment variable `FULL_SHARE_DIR` before starting any tool that utilizes files from that folder. This is an important path as many things are expected to be found there by default. These include predefined charset tables, stopwords, manticore modules, and icu data files, all placed in that folder. The configuration script usually determines this path to be something like `/usr/share/manticore`, or `/usr/local/share/manticore`.
- **DISTR_BUILD** - a shortcut for the options for releasing packages. This is a string value with the name of the target platform. It can be used instead of manually configuring all the options. On Debian and Redhat Linuxes, the default value might be determined by light introspection and set to a generic 'Debian' or 'RHEL'. Otherwise, the value is not defined.
- **PACK** - an even more convenient shortcut. It reads the `DISTR` environment variable, assigns it to the **DISTR_BUILD** parameter, and then works as usual. This is very useful when building in prepared build systems, like Docker containers, where the  `DISTR` variable is set at the system level and reflects the target system for which the container is intended.
- **CMAKE_INSTALL_PREFIX** (path) - where Manticore is expected to be installed. Building does not perform any installations, but it prepares the installation rules that are executed when you run the `cmake --install` command or create a package and then install it. The prefix can be changed at any time, even during installation, by invoking
  `cmake --install . --prefix /path/to/installation`. However, at config time, this variable is used to initialize the default values of  `LOCALDATADIR` and `FULL_SHARE_DIR`. For example, setting it to `/my/custom` at configure
  time will hardcode `LOCALDATADIR` as `/my/custom/var/lib/manticore/data`, and `FULL_SHARE_DIR` as
  `/my/custom/usr/share/manticore`.
- **BUILD_TESTING** (bool) whether to support testing. If enabled, after the build, you can run 'ctest' and test the build. Note that testing implies additional dependencies, like at least the presence of PHP cli, Python, and an available MySQL server with a test database. By default, this parameter is on. So, for 'just build', you might want to disable the option by explicitly specifying 'off' value.
- **LIBS_BUNDLE** - path to a folder with different libraries. This is mostly relevant for Windows building, but may also be helpful if you have to build often in order to avoid downloading third-party sources each time. By default, this path is never modified by the configuration script; you should put everything there manually. When, say, we want support for a stemmer - the sources will be downloaded from Snowball homepage, then extracted, configured, built, etc. Instead, you can store the original source tarball (which is `libstemmer_c.tgz`) in this folder. Next time you want to build from scratch, the configuration script will first look up in the bundle, and if it finds the stemmer there, it will not download it again from the Internet.
- **CACHEB** - path to a folder with stored builds of 3-rd party libraries. Usually features like galera, re2, icu, etc. first downloaded or being got from bundle, then unpacked, built, and installed into a temporary internal folder. When building manticore, that folder is then used as the place where the things required to support the asked feature are live. Finally, they either link with manticore, if it is a library; either go directly to distribution/installation (like galera or icu data). When **CACHEB** is defined either as cmake config param, either as a system environment variable, it is used as the target folder for that builds. This folder might be kept across builds, so that stored libraries there will not be rebuilt anymore, making the whole build process much shorter.


Note, that some options are organized in triples: `WITH_XXX`, `DL_XXX` and `XXX_LIB` - like support of mysql, odbc, etc. `WITH_XXX` determines whether next two have an effect or not. I.e., if you set `WITH_ODBC` to `0` - there is no sence to provide `DL_ODBC` and `ODBC_LIB`, and these two will have no effect if the whole feature is disabled. Also, `XXX_LIB` has no sense without `DL_XXX`, because if you don't want `DL_XXX` option, dynamic loading will not be used, and name provided by `XXX_LIB` is useless. That is used by default introspection.

Also, using the `iconv` library assumes `expat` and is useless if the last is disabled.

Also, some libraries may be always available, and so, there is no sense to avoid linkage with them. For example, in Windows that is ODBC. On macOS that is Expat, iconv, and m.b. others. Default introspection determines such libraries and effectively emits only `WITH_XXX` for them, without `DL_XXX` and `XXX_LIB`, that makes the things simpler.

With some options in game configuring might look like:

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

Apart general configuration values, you may also investigate file `CMakeCache.txt` which is left in build folder right after you run configuration. Any values defined there might be redefined explicitly when running cmake. For example, you may run `cmake -DHAVE_GETADDRINFO_A=FALSE ...`, and that config run will not assume investigated value of that variable, but will use one you've provided.

#### Specific environment variables

Environment variables are useful for providing some kind of global settings which are stored aside from build configuration and are always present. For persistence, they may be set globally on the system using different ways - like adding them to the `.bashrc` file, or embedding them into a Dockerfile if you produce a docker-based build system, or writing them in system preferences environment variables on Windows. Also, you may set them short-lived using `export VAR=value` in the shell. Or even shorter, by prepending values to the cmake call, like `CACHEB=/my/cache cmake ...` - this way it will only work on this call and will not be visible on the next.

Some of such variables are known to be used in general by cmake and some other tools. That is things like `CXX` which determines the current C++ compiler, or `CXX_FLAGS` to provide compiler flags, etc.

However, we have some variables that are specific to manticore configuration, which are invented solely for our builds.

- **CACHEB** - same as the config **CACHEB** option
- **LIBS_BUNDLE** - same as the config **LIBS_BUNDLE** option
- **DISTR** - used to initialize the `DISTR_BUILD` option when `-DPACK=1` is used.
- **DIAGNOSTIC** - makes the output of cmake configuration much more verbose, explaining everything happening
- **WRITEB** - assumes **LIBS_BUNDLE** and, if set, will download source archive files for different tools to the LIBS_BUNDLE folder. That is, if a fresh version of the stemmer comes out - you can manually remove libstemmer_c.tgz from the bundle and then run a one-shot `WRITEB=1 cmake ...` - it will not find the stemmer's sources in the bundle and will then download them from the vendor's site to the bundle (without WRITEB it will download them into a temporary folder inside the build and will disappear when you wipe the build folder).

At the end of configuration, you may see what is available and will be used in a list like this one:

```
-- Enabled features compiled in:
* Galera, replication of tables
* re2, a regular expression library
* stemmer, stemming library (Snowball)
* icu, International Components for Unicode
* OpenSSL, for encrypted networking
* ZLIB, for compressed data and networking
* ODBC, for indexing MSSQL (windows) and generic ODBC sources with indexer
* EXPAT, for indexing xmlpipe sources with indexer
* Iconv, for support of different encodings when indexing xmlpipe sources with indexer
* MySQL, for indexing MySQL sources with indexer
* PostgreSQL, for indexing PostgreSQL sources with indexer
```

### Building

```bash
cmake --build . --config RelWithDebInfo
```

### Installation

To install run:

```bash
cmake --install . --config RelWithDebInfo
```

to install into custom (non-default) folder, run

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### Building packages

For building a package, use the target `package`. It will build the package according to the selection provided by the `-DDISTR_BUILD` option. By default, it will be a simple .zip or .tgz archive with all binaries and supplementary files.

```bash
cmake --build . --target package --config RelWithDebInfo
```

## Some advanced things about building

### Recompilation (update) on single-config

If you haven't changed the path for sources and build, simply move to your build folder and run:

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

If by any reason it doesn't work, you can delete file `CMakeCache.txt` located in the build folder. After this step you
have to run cmake again, pointing to the source folder and configuring the options.

If it also doesn't help, just wipe out your build folder and begin from scratch.

### Build types

Briefly - just use `--config RelWithDebInfo` as written above. It will make no mistake.

We use two build types. For development, it is `Debug` - it assigns compiler flags for optimization and other things in a way that it is very friendly for development, meaning the debug runs with step-by-step execution. However, the produced binaries are quite large and slow for production.

For releasing, we use another type - `RelWithDebInfo` - which means 'release build with debug info'. It produces production binaries with embedded debug info. The latter is then split away into separate debuginfo packages which are stored aside with release packages and might be used in case of some issues like crashes - for investigation and bugfixing. Cmake also provides `Release` and `MinSizeRel`, but we don't use them. If the build type is not available, cmake will make a `noconfig` build.

#### Build system generators

There are two types of generators: single-config and multi-config.

- Single-config needs the build type provided during configuration, via the `CMAKE_BUILD_TYPE` parameter. If it is not defined, the build will fall back to the `RelWithDebInfo` type which is suitable if you just want to build Manticore from sources and not participate in development. For explicit builds, you should provide a build type, like `-DCMAKE_BUILD_TYPE=Debug`.
- Multi-config selects the build type during the build. It should be provided with the `--config` option, otherwise it will build a kind of `noconfig`, which is not desirable. So, you should always specify the build type, like `--config Debug`.

If you want to specify the build type but don't want to care about whether it is a 'single' or 'multi' config generator - just provide the necessary keys in both places. I.e., configure with `-DCMAKE_BUILD_TYPE=Debug`, and then build with `--config Debug`. Just be sure that both values are the same. If the target builder is a single-config, it will consume the configuration param. If it is multi-config, the configuration param will be ignored, but the correct build configuration will be selected by the `--config` key.

If you want `RelWithDebInfo` (i.e. just build for production) and know you're on a single-config platform (that is all, except Windows) - you can omit the `--config` flag on the cmake invocation. The default `CMAKE_BUILD_TYPE=RelWithDebInfo` will be configured then, and used. All the commands for 'building', 'installation' and 'building package' will become shorter then.

#### Explicitly select build system generators

Cmake is the tool that doesn't perform building by itself, but it generates rules for the local build system.
Usually, it determines the available build system well, but sometimes you might need to provide a generator explicitly. You
can run `cmake -G` and review the list of available generators.

- On Windows, if you have more than one version of Visual Studio installed, you might need to specify which one to use,
as:
```bash
cmake -G "Visual Studio 16 2019" ....
  ```
- On all other platforms - usually Unix Makefiles are used, but you can specify another one, such as Ninja, or Ninja Multi-Config, as:
  Multi-Config`, as:
```bash
  cmake -GNinja ...
  ```
  or
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja Multi-Config is quite useful as it is really 'multi-config' and available on Linux/macOS/BSD. With this generator, you may shift the choosing of configuration type to build time, and also you may build several configurations in one and the same build folder, changing only the  `--config` param.

### Caveats

1. If you want to finally build a full-featured RPM package, the path to the build directory must be long enough in order to correctly build debug symbols.
Like `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`, for example. That is because RPM tools modify the path over compiled binaries when building debug info, and it can just write over existing room and won't allocate more. The aforementioned long path has 100 chars and that is quite enough for such a case.

## External dependencies

Some libraries should be available if you want to use them.
- For indexing (`indexer` tool): `expat`, `iconv`, `mysql`, `odbc`, `postgresql`. Without them, you can only process `tsv` and `csv` sources.
- For serving queries (`searchd` daemon): `openssl` might be necessary.
- For all (required, mandatory!) we need the Boost library. The minimal version is 1.61.0, however, we build the binaries with a fresher version 1.75.0. Even more recent versions (like 1.76) should also be okay. On Windows, you can download pre-built Boost from their site (boost.org) and install it into the default suggested path (i.e. `C:\\boost...`). On MacOs, the one provided in brew is okay. On Linux, you can check the available version in official repositories, and if it doesn't match requirements, you can build from sources. We need the component 'context', you can also build components 'system' and 'program_options', they will be necessary if you also want to build Galera library from the sources. Look into `dist/build_dockers/xxx/boost_175/Dockerfile` for a short self-documented script/instruction on how to do it.

On the build system, you need the 'dev' or 'devel' versions of these packages installed (i.e. - libmysqlclient-devel, unixodbc-devel, etc. Look to our dockerfiles for the names of concrete packages).

On run systems, these packages should be present at least in the final (non-dev) variants. (devel variants usually larger, as they include not only target binaries, but also different development stuff like include headers, etc.).

### Building on Windows

Apart from necessary prerequisites, you might need prebuilt `expat`, `iconv`, `mysql`, and `postgresql` client libraries. You have to either build them yourself or contact us to get our build bundle (a simple zip archive where the folder with these targets is located).

- ODBC is not necessary as it is a system library.
- OpenSSL might be built from sources or downloaded prebuilt from https://slproweb.com/products/Win32OpenSSL.html (as mentioned in the cmake internal script on FindOpenSSL).
- Boost might be downloaded pre-built from https://www.boost.org/ releases.

### See what is compiled

Run `indexer -h`. It will show which features were configured and built (whether they're explicit or investigated, doesn't matter):

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->
