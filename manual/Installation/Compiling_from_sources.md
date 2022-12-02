# Compiling Manticore from source

Compiling Manticore Search from sources can be used for custom build configurations, such as disabling some features, adding new or testing patches, if you want to contribute. For example, you can compile from sources disabling embedded ICU, if you want to replace it with another one installed in your system with possibility to upgrade it independently from Manticore.

## Building using CI docker

To prepare [official release and dev packages](https://repo.manticoresearch.com/) we use Docker and a special building image. It includes an essential toolchain and is designed to be used with external sysroots, so one container can build packages for all operating systems. You can build the image using [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) and [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md). That is the easiest way to make binaries for any supported operating system and architecture. Once you build the image whe you run the container from it you need to specify 3 environment variables:
* Target platform `DISTR`
* Architecture `arch`
* URL to system roots archives `SYSROOT_URL`. Just use `https://repo.manticoresearch.com/repository/sysroots` unless you build the sysroots yourself (the instruction is [here](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)).

You can use directory https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/ as a cheatsheet to find out possible `DISTR` and `arch` values since it includes sysroots for all the supported combinations.

After that inside the docker container building packages is as easy as calling:

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

For example, to create the same RedHat 7 package as official, but without embedded ICU with it's big datafile, you can
execute the following (supposed that the sources are placed in `/manticore/sources/` on the host):

```bash
docker run -it --rm -e SYSROOT_URL=https://repo.manticoresearch.com/repository/sysroots \
-e arch=x86_64 \
-e DISTR=rhel7 \
-v /manticore/sources:/manticore \
<docker image> bash

# following is to be run inside docker shell
cd /manticore/
RELEASE_TAG="noicu"
mkdir build && cd build
cmake -DPACK=1 -DBUILD_TAG=$RELEASE_TAG -DWITH_ICU_FORCE_STATIC=0 ..
cmake --build . --target package
```

The same way you can build binaries/packages not only for popular Linux distributions, but also for FreeBSD, Windows and macOS.

## Building manually

### Required tools

* C++ compiler
  * in Linux - GNU (4.7.2 and above) or Clang can be used
  * in Windows - Microsoft Visual Studio 2019 and above (community edition is enough)
  * on Mac OS - Clang (from command line tools of XCode, use `xcode-select --install` to install).
* Bison, Flex - on most of the systems available as packages, on Windows available in cygwin framework.
* Cmake - used on all the platforms (version 3.19 or above required)

### Fetching sources

#### From git

Manticore source code is [hosted on GitHub](https://github.com/manticoresoftware/manticoresearch). Clone the repo, then checkout a desired branch or tag. Branch `master` represents main development branch. Upon release we create a versioned tag, like `3.6.0`, and start a new branch for current release, in this case `manticore-3.6.0`. The head of the versioned branch after all changes is used as source to build all binary releases. For example, to take sources of version 3.6.0 you can run:

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### From archive

You can download desired code from github by using 'download zip' button. Both .zip and .tar.gz are suitable.

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### Configuring

Manticore uses cmake. Assuming you're inside the root dir of the cloned repository.

```bash
mkdir build && cd build
cmake ..
```

Cmake will investigate available features and configure the build according to them. By default all features are considered enabled, if they're available. Also script downloads and builds some external libraries assuming you want to use them. Implicitly you get support of maximal number of features.

Also, you can rule configuration explicitly, with flags and options. To demand feature `FOO` add `-DFOO=1` to cmake call.
To disable it - same way, `-DFOO=0`. If not explicitly noticed, enabling of not available feature (say, `WITH_GALERA` on
MS Windows build) will cause configuration to fail with error. Disabling of a feature, apart excluding it from build, also
disables it's investigation on the system, and disables their downloading/building, as it would be done for some external
libs in case of implicit configuration.

#### Configuration flags and options

- **USE_SYSLOG** - allows to use `syslog` in [query logging](Logging/Query_logging.md).
- **WITH_GALERA** - support replication on search daemon. Support will be configured for the build. Also, sources of Galera library will be downloaded, built and final module will be included into distribution/installation. Usually it is safe if you build with galera, but not
distribute the library itself (so, no galera module - no replication). But sometimes you may need to explicitly disable it. Say, if you
want to build static binary which by desing can't load any libraries, so that even presence of call to 'dlopen' function inside daemon
will cause link error.
- **WITH_RE2** - build with using RE2 regular expression library. It is necessary for functions like [REGEX()](../Functions/String_functions.md#REGEX%28%29), and [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
  feature.
- **WITH_RE2_FORCE_STATIC** - download sources of RE2, compile them and link with them statically, so that final binaries will not depend on
presence of shared `RE2` library in your system.
- **WITH_STEMMER** - build with using Snowball stemming library.
- **WITH_STEMMER_FORCE_STATIC** - download snowball sources, compile them and link with them statically, so that final
  binaries will not depend on presence of shared `libstemmer` library in your system.
- **WITH_ICU** - build with using icu, International Components for Unicode library. That is used in tokenization of Chineze, for text
segmentation. It is in game when morplology like `icu_chinese` in use.
- **WITH_ICU_FORCE_STATIC** - download icu sources, compile them and link with them statically, so that final binaries will not depend on presence of shared `icu` library in your system. Also include icu data file into installation/distribution. Purpose of statically linked ICU - is to have the library of known version, so that behaviour is determined and not depends on any system libraries. You most probably would prefer to use system ICU instead, because it may be updated in time without need to recompile manticore daemon. In this case you need to explicitly disable this option. That will also save you some place occupied by icu data file (about 30M), as it will NOT be included into distribution then.
- **WITH_SSL** - used for support https, and also encrypted mysql connections to the daemon. System OpenSSL library will be linked to daemon. That implies, that OpenSSL will be required to start the daemon.
  That is mandatory for support of https, but not strictly mandatory for the server (i.e. no ssl means no possibility to
  connect by https, but other protocols will work). SSL library versions starting from 1.0.2 to 1.1.1 may be used by
  Manticore, however note that **for the sake of security it's highly recommended to use the freshest possible SSL
  library**. For now only v1.1.1 is supported, the rest are outdated (
  see [openssl release strategy](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - used by indexer to work with compressed columns from mysql. Used by daemon to provide support of compressed mysql protocol.
- **WITH_ODBC** - used by indexer to support indexing sources from ODBC providers (they're typically UnixODBC and iODBC). On MS Windows ODBC is
the proper way to work witn MS SQL sources, so indexing of `MSSQL` also implies this flag.
- **DL_ODBC** - don't link with ODBC library. If ODBC is linked, but not available, you can't start `indexer` tool even if you want to process something not related to ODBC. This option asks indexer to load the library in runtime only when you want to deal with ODBC source.
- **ODBC_LIB** - name of ODBC library file. Indexer will try to load that file when you want to process ODBC source. That option is written automatically from available ODBC shared library investigation. You can also override that name on runtime, providing environment variable `ODBC_LIB` with proper path to alternative library before running indexer.
- **WITH_EXPAT** - used by indexer to support indexing xmlpipe sources.
- **DL_EXPAT** - don't link with EXPAT library. If EXPAT is linked, but not available, you can't start `indexer` tool
  even if you want to process something not related to xmlpipe. This option asks indexer to load the library in runtime only
  when you want to deal with xmlpipe source.
- **EXPAT_LIB** - name of EXPAT library file. Indexer will try to load that file when you want to process xmlpipe source. That
  option is written automatically from available EXPAT shared library investigation. You can also override that name on
  runtime, providing environment variable `EXPAT_LIB` with proper path to alternative library before running indexer.
- **WITH_ICONV** - for support different encodings when indexing xmlpipe sources with indexer.
- **DL_ICONV** - don't link with iconv library. If iconv is linked, but not available, you can't start `indexer` tool
    even if you want to process something not related to xmlpipe. This option asks indexer to load the library in runtime
    only when you want to deal with xmlpipe source.
- **ICONV_LIB** - name of iconv library file. Indexer will try to load that file when you want to process xmlpipe source.
  That option is written automatically from available iconv shared library investigation. You can also override that
  name on runtime, providing environment variable `ICONV_LIB` with proper path to alternative library before running
  indexer.
- **WITH_MYSQL** - used by indexer to support indexing mysql sources.
- **DL_MYSQL** - don't link with mysql library. If mysql is linked, but not available, you can't start `indexer` tool
  even if you want to process something not related to mysql. This option asks indexer to load the library in runtime
  only when you want to deal with mysql source.
- **MYSQL_LIB** - name of mysql library file. Indexer will try to load that file when you want to process mysql source.
  That option is written automatically from available mysql shared library investigation. You can also override that
  name on runtime, providing environment variable `MYSQL_LIB` with proper path to alternative library before running
  indexer.
- **WITH_POSTGRESQL** - used by indexer to support indexing postgresql sources.
- **DL_POSTGRESQL** - don't link with postgresql library. If postgresql is linked, but not available, you can't start `indexer` tool
  even if you want to process something not related to postgresql. This option asks indexer to load the library in runtime
  only when you want to deal with postgresql source.
- **POSTGRESQL_LIB** - name of postgresql library file. Indexer will try to load that file when you want to process
  postgresql source.
  That option is written automatically from available postgresql shared library investigation. You can also override that
  name on runtime, providing environment variable `POSTGRESQL_LIB` with proper path to alternative library before running
  indexer.
- **LOCALDATADIR** - default path where daemon stores binlog. If that path is not provided or disabled explicitly in daemon's
runtime config (that is file `manticore.conf`, no way related to this build configuration), binlogs will be placed to this path.
It is assumed to be absolute, however that is not strictly necessary, and you may play with relative values also. You most
probably would not, however, change default value defined by configuration, which, depending on target system, might be
something like `/var/data`, `/var/lib/manticore/data`, or `/usr/local/var/lib/manticore/data`.
- **FULL_SHARE_DIR** - default path where all assets are stored. It may be overriden by environment variable `FULL_SHARE_DIR` before starting
any tool which uses files from that folder. That is quite important path, as many things are by default expected there.
That are - predefined charset tables, stopwords, manticore modules and icu data files - all placed into that folder. Configuration
script usually determines that path to be something like `/usr/share/manticore`, or `/usr/local/share/manticore`.
- **DISTR_BUILD** - shortcut of the options for releasing packages. That is string value with the name of the target platform. It may be used instead of manually configuring all the stuff. On debian and redhat linuxes default value might be determined by light introspection and set to generic 'debian' or 'rhel'. Otherwise value is not defined.
- **PACK** - even more shortcut. It reads `DISTR` environment variable, assigns it to **DISTR_BUILD** param and then works as usual.
That is very useful when building in prepared build systems, like docker containers, where that `DISTR` variable is set on system level and reflects target system for which such container intended.
- **CMAKE_INSTALL_PREFIX** (path) - where manticore except itself installed. Building installs
  nothing, but prepares installation rules which are executed once you run `cmake --install` command, or
  create a package and then install it. Prefix may be freely changed anytime, even during install - by invoking
  `cmake --install . --prefix /path/to/installation`. However, at config time this variable once used to initialize
  default values of `LOCALDATADIR` and `FULL_SHARE_DIR`. So, for example, setting it to `/my/custom` at configure
  time will hardcode `LOCALDATADIR` as `/my/custom/var/lib/manticore/data`, and `FULL_SHARE_DIR` as
  `/my/custom/usr/share/manticore`.
- **BUILD_TESTING** (bool) whether to support testing. If enabled, after the build you can run 'ctest' and test the
  build. Note that testing implies additional dependencies, like at least presence of PHP cli, python and available
  mysql server with test database. By default this param is on. So, for 'just build', you might want to disable the
  option by explicitly specifying 'off' value.
- **LIBS_BUNDLE** - path to a folder with different libraries. This is mostly relevant for Windows building, but may be also helpful if you have to build often in order to avoid downloading third-party sources each time. By default this path is never modified by the configuration script; you should put everything there manually. When, say, we want the support of stemmer - the sources will be downloaded from Snowball homepage, then extracted, configured, built, etc. Instead you can store the original source tarball (which is `libstemmer_c.tgz`) in this folder. Next time you want to build from scratch, the configuration script will first look up in the bundle, and if it finds the stemmer there, it will not download it again from the Internet.
- **CACHEB** - path to a folder with stored builds of 3-rd party libraries. Usually features like galera, re2, icu, etc. first downloaded
or being got from bundle, then unpacked, built and installed into temporary internal folder. When building manticore that folder is then used as the place where the things required to support asked feature are live. Finally they either link with manticore, if it is library; either go directly to distribution/installation (like galera or icu data). When **CACHEB** is defined either as cmake config param, either as system environment variable, it is used as target folder for that builds. This folder might be kept across builds, so that stored libraries there will not be rebuilt anymore, making whole build process much shorter.


Note, that some options organized in triples: `WITH_XXX`, `DL_XXX` and `XXX_LIB` - like support of mysql, odbc, etc. `WITH_XXX` determines whether next two has effect or not. I.e., if you set `WITH_ODBC` to `0` - there is no sence to provide `DL_ODBC` and `ODBC_LIB`, and these two will have no effect if whole feature is disabled. Also, `XXX_LIB` has no sense without `DL_XXX`, because if you don't want `DL_XXX` option, dynamic loading will not be used, and name provided by `XXX_LIB` is useless. That is used by default introspection.

Also, using `iconv` library assumes `expat` and is useless if last is disabled.

Also, some libraries may be always available, and so, there is no sense to avoid linkage with them. For example, in windows that is ODBC. On Mac Os that is Expat, iconv and m.b. others. Default introspection determines such libraries and effectively emits only `WITH_XXX` for them, without `DL_XXX` and `XXX_LIB`, that makes the things simpler.

With some options in game configuring might look like:

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

Apart general configuration values, you may also investigate file `CMakeCache.txt` which is left in build folder right after you run configuration. Any values defined there might be redefined explicitly when running cmake. For example, you may run `cmake -DHAVE_GETADDRINFO_A=FALSE ...`, and that config run will not assume investigated value of that variable, but will use one you've provided.

#### Specific environment variables

Environment variables are useful to provide some kind of global settings which are stored aside build configuration and just present 'always'.
For persistency they may be set globally on the system using different ways - like adding them to `.bashrc` file, or embedding into Dockerfile if you produce docker-based build system, or write in system preferences environment variables on Windows. Also you may set them short-live using `export VAR=value` in the shell. Or even shorter, prepending values to cmake call, like `CACHEB=/my/cache cmake ...` - this way it will only work on this call and will not be visible on the next.

Some of such variables are known to be used in general by cmake and some other tools. That is things like `CXX` which determines current C++ compiler, or `CXX_FLAGS` to provide compiler flags, etc.

However we have some of the variables specific to manticore configuration, which are invented solely for our builds.

- **CACHEB** - same as config **CACHEB** option
- **LIBS_BUNDLE** - same as config **LIBS_BUNDLE** option
- **DISTR** - used to initialize `DISTR_BUILD` option when `-DPACK=1` is used.
- **DIAGNOSTIC** - make output of cmake configuration much more verbose, explaining every thing happening
- **WRITEB** - assumes **LIBS_BUNDLE**, and if set, will download source archive files for different tools to LIBS_BUNDLE folder. That is, if fresh version of stemmer came out - you can manually remove libstemmer_c.tgz from the bundle, and then run oneshot `WRITEB=1 cmake ...` - it will not found stemmer's sources in the bundle, and then download them from vendor's site to the bundle (without WRITEB it will download them into some temporary folder inside build and it will disappear as you wipe the build folder).

At the end of configuration you may see what is available and will be used in the list like this one:

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
 * Iconv, for support different encodings when indexing xmlpipe sources with indexer
 * Mysql, for indexing mysql sources with indexer
 * PostgreSQL, for indexing postgresql sources with indexer
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

For building package use target `package`. It will build package according to selection, provided by `-DDISTR_BUILD`
option. By default it will be a simple .zip or .tgz archive with all binaries and supplement files.

```bash
cmake --build . --target package --config RelWithDebInfo
```

## Some advanced things about building

### Recompilation (update) on single-config

If you didn't change the path for sources and build, just move to you build folder and run:

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

If by any reason it doesn't work, you can delete file `CMakeCache.txt` located in the build folder. After this step you
have to run cmake again, pointing to the source folder and configuring the options.

If it also doesn't help, just wipe out your build folder and begin from scratch.

### Build types

Shortly - just do `--config RelWithDebInfo` as written above. It will make no mistake.

We use two build types. For development it is `Debug` - it assigns compiler flags for optimization and other things the way that it is very friendly for development, in means debug runs with step-by-step execution. However, produced binaries are quite large and slow for production.

For releasing we use another type - `RelWithDebInfo` - which means 'release build with debug info'. It produces production binaries with embedded debug info. The latter is then split away into separate debuginfo packages which are stored aside with release packages and might be used in case of some issues like crashes - for investigation and bugfixing. Cmake also provides `Release` and `MinSizeRel`, but we don't use them. If build type is not available, cmake will make `noconfig` build.

#### Build system generators

There are two types of generators: single-config and multi-config.

- single-config needs build type provided on configuration, via `CMAKE_BUILD_TYPE` parameter. If it is not defined, build
  fall-back to `RelWithDebInfo` type which is quite well if you want just build manticore from sources and not going to
  participate in development. For explicit build you should provide build type, like `-DCMAKE_BUILD_TYPE=Debug`.
- multi-config selects build type during the build. It should be provided with `--config` option, otherwise it will
  build kind of `noconfig`, which is quite strange and not desirable. So, you should always specify build type, like `--config Debug`.

If you want to specify build type, but don't want to care about whether it is 'single' or 'multi' config generator -
just provide necessary keys in both places. I.e., configure with `-DCMAKE_BUILD_TYPE=Debug`, and then build with `--config Debug`.
Just be sure that both values are same. If target builder is single-config, it will consume configuration param.
If it is multi-config, configuration param will be ignored, but correct build configuration will then be selected by --config key.

If you want RelWihtDebInfo (i.e. just build for production) and know you're on single-config platform (that is all, except Windows) - you can omit `--config` flag on cmake invocation. Default `CMAKE_BUILD_TYPE=RelWithDebInfo` will be configured then, and used.
All the commands for 'building', 'installation' and 'building package' will become shorter then.

#### Explicitly select build system generators

Cmake is the tool which is not performing building by itself, but it generates rules for local build system.
Usually it determines available build system well, but sometimes you might need to provide generator explicitly. You
can run `cmake -G` and review the list of available generators.

- on Windows, if you have more than one version ov Visual Studio installed, you might need to specify which one to use,
  as:
 ```bash
  cmake -G "Visual Studio 16 2019" ....
  ```
- on all other platforms - usually `Unix makefiles` are in game, but you can specify another one, as `Ninja`, or `Ninja
  Multi-Config`, as:
```bash
  cmake -GNinja ...
  ```
  or
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja Multi-Config is quite useful, as it is really 'multi-config', and available on linux/macos/bsd. With this
generator you may shift choosing of configuration type to build time, and also you may build several configurations
in one and same build folder, changing only `--config` param.

### Caveats

1. If you want to finally build full-featured RPM package, path to build directory must be long enough in order to
   correctly build debug symbols.
   Like `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`, for
   example. That is because RPM tools modify the path over compiled binaries when building debug info, and it can just
   write over existing room and won't allocate more. Above mentioned long path has 100 chars and that is quite enough
   for such case.

## External dependencies

Some libraries should be available if you want to use them.
- for indexing (`indexer` tool): `expat`, `iconv`, `mysql`, `odbc`, `postgresql`. Without them, you could only process
  `tsv` and `csv` sources.
- for serving queries (`searchd` daemon): `openssl` might be necessary.
- for all (required, mandatory!) we need Boost library. Minimal version is 1.61.0, however we build the binaries with fresher 1.75.0.
Even more fresh (like 1.76) should also be ok. On Windows you can download pre-built Boost from their site (boost.org) and
install into default suggested path (that is C:\\boost...). On Mac Os the one provided in brew is ok. On linuxes you can check
available version in official repositories, and if it doesn't match requirements you can build from sources. We need
component 'context', you can also build components 'system' and 'program_options', they will be necessary if you also want
to build Galera library from the sources. Look into `dist/build_dockers/xxx/boost_175/Dockerfile` for a short self-documented
script/instruction how to do it.

On build system you need 'dev' or 'devel' versions of that packages installed (i.e. - libmysqlclient-devel, unixodbc-devel, etc. Look to our dockerfiles for the names of concrete packages).

On run systems these packages should present at least in final (non-dev) variants. (devel variants usually larger, as they include not only target binaries, but also different development stuff like include headers, etc.).

### Building on Windows

Apart necessary pre-requisites, you might need prebuilt `expat`, `iconv`, `mysql` and `postgresql` client libraries. You have either to build them yourself, either contact us to get our build bundle (that is simple zip archive where folder with these targets located).

- ODBC is not necessary, as it is system library.
- OpenSSL might be build from sources, or download prebuilt from https://slproweb.com/products/Win32OpenSSL.html (that is mentioned in cmake internal script on FindOpenSSL).
- Boost might be downloaded pre-built from https://www.boost.org/ releases.

### See what is compiled

Run `indexer -h`. It will say which features was configured and built (whenever they're explicit, or investigated, doesn't matter):

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
