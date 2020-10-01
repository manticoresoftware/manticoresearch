# Compiling Manticore from source 

Compiling from sources can be used for custom build configurations, such as disabling some features, adding new or testing patches, if you want to contribute. For example, you can compile from sources disabling embedded ICU, if you want to replace it with another one installed in your system with possibility to upgrade it independently from Manticore.

## Required tools 

* C++ compiler
  * in Linux - GNU gcc (4.7.2 and above) or clang can be used
  * in Windows - Microsoft Visual Studio 2015 and above (community edition is enough)
  * on Mac OS - XCode
* Cmake - used on all the platforms (version 3.13 or above required)

## Available features and their dependencies

Manticore consists of different tools. The main one is Manticore search server - `searchd`. Features available in the server depend on different third-party libraries:

* SSL (for HTTPS implementation)
* Galera (for replication)
* Stemmer (for language stemming)
* ICU (for support of segmentation for CJK languages)
* RE2 (for regular expressions)

Some features, like support of AOT lemmatization and basic stemming are embedded and don't need any libraries.

Another big tool is `indexer`, which [creates plain indexes from different sources](Adding_data_from_external_storages/Plain_indexes_creation.md). There may be different external storages. The list of libraries which indexer can use (and depend from) includes:
* mysql
* postresql
* expat
* odbc and others. 

None of them are mandatory by default, but they are obviously necessary if you want to index a source, provided by a particular storage.

Internally manticore consists of big library 'libsphinx' with which different tools are linked. Because of the fact, that some dependencies are mandatory for that library, they also apply to all the tools despite the fact whether they are used or not there. The common dependencies are:

* By default, only indexing of mysql sources is expected. So, the configuration script will search for mysql dev client lib, and if nothing found, will fail. To have the possibility of indexing mysql, you need at least dev version of MySQL library. Usually it is provided in a package named `libmysqlclient-dev` or `mysql-devel`, depending on Linux flavour you use. Also, different derivatives, as dev package from mariadb (which are `libmariadb-dev` or `mariadb-devel`) might be ok. If you have mysql or derivative installed by some custom path, set env variable `MYSQL_DIR` to that path for configuration. Configuration will look for available program `mysql_config`, and use data, provided by it. **OR** will look for header `mysql.h` and library `mysqlclient`, if no `mysql_config` program found. If you're **not** going to use mysql sources you can explicitly set `-DWITH_MYSQL=0` or `-DWITH_MYSQL=NO` as config parameter.
* `git`, `flex`, `bison` - needed if the sources are cloned from git repository. If you use official tarball with sources, they are not necessary (git is used to pick commit hash). Flex and bison are used to build parsers. In tarball sources the version is hardcoded, and parses also pre-builded into C-sources, so these tools are not necessary.
* RE2 - used for [regexp_filter](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) feature. For using the feature Manticore must be configured with `-DWITH_RE2=1`, otherwise it will not be available. If configured, system-wide RE2 will be searched, and if nothing  found, configuration will download RE2 sources and build the feature as embedded.
* stemmer - used for additional language stemmers. Might be configured by `-DWITH_STEMMER=1`. If required, will be searched in the system and linked. If not found - configuration will download snowball sources and build the feature as embedded.
* ICU - for CJK languages. It replaces previous RLP platform, also used for that purpose. By default the ICU is configured as embedded, and will be built from sources. ICU, RE2 and stemmer may be either searched and used as shared libraries, provided by your system or explicitly build from sources and statically linked forever. The first option makes the binaries smaller and more flexible for upgrade (that is simple: upgrade a library in the system and take all benefits/fixes of the upgrade). By default the RE2 and stemmer libraries supposed to be used from system, and ICU configured to be built as static from sources. You can manually tune that behaviour by providing boolean options `-DWITH_ICU_FORCE_STATIC=`, `-DWITH_RE2_FORCE_STATIC=`, and `-DWITH_STEMMER_FORCE_STATIC=`.

### For indexing (performed by running `indexer` tool)
* Indexing of postgresql is supported by `postgresql-devel` or `libpq-dev` packages. Absence of these packages is not fatal, your tool just will not be able to index postgresql source then. The feature can be switched on using `-DWITH_PGSQL=` option.
* Indexing of xmlpipe is supported by `expat` library, provided by `expat-devel` or `libexpat-dev` package. The feature is automatically switched on or off depending on availability of expat library. It ca also be manually tuned with help of option `-DWITH_EXPAT=1`.
* Indexing of generic ODBC source is supported by either `iodbc` or `unixodbc` libraries that are provided by `unixODBC-devel` or `unixodbc-dev` or iodbc alternatively. The feature is automatically switched on or off depending on availability of client library. Use `-DWITH_ODBC=` to tune it manually.

When used with a source indexer will try to dynamically load the necessary runtime library, and if nothing is available it will report an error. So, it is reasonable to provide _all_ dev packages for building, and then in runtime provide only actually necessary client libraries.

### For serving queries (performed by `searchd` binary)

For the server these dependencies may be in play:

* If you're going to work over https - you need dev version of **ssl** lib. Usually it comes in a package named like `libssl-dev` (debian-based) or `openssl-devel` (redhat-based). That is mandatory for support of https, but not strictly mandatory for the server (i.e. no ssl means no possibility to connect by https, but other protocols will work). SSL library versions starting from 1.0.2 to 1.1.1 may be used by Manticore, however note that **for the sake of security it's highly recommended to use the freshest possible SSL library**. For now only v1.1.1 is supported, the rest are outdated (see [openssl release strategy](https://www.openssl.org/policies/releasestrat.html) for details)
* if you want replication functionality - Galera library has to be built. It will be downloaded and included into the build, however it requires by itself `boost` library. Having `libboost-all-dev` on debian-based, or `boost-devel` on redhat should be enough. Also, it requires ssl (despite of the requirements of the server).
    
## General building guide

### From git

Manticore sources are [hosted on github](https://github.com/manticoresoftware/manticoresearch). Clone the repo, then checkout desired branch or tag. Our public git workfow contains only main `master` branch, which represents bleeding-edge of development. On release we create a versioned tag, like `3.5.2`, and start a new branch for current release, in this case `manticore-3.5.2`. The head of the versioned branch after all changes is used as source to build all binary releases. For example, to take sources of version 3.5.2 you can run:

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.5.2
```

When using sources from GitHub you'll need `flex` and `bison` tools, since all internal parsers are provided as lex/yacc sources.

### From tarball

Tarballs are available [here](https://manticoresearch.com/downloads/). Look for "Source tar.gz". Those provided by github 'Source code' archives are not what you want, so avoid using them (mainly they lack the git version which we use to make version string). The tarball sources have pre-built lexers and parsers, so flex and bison tools are not required for a build.

```bash
wget -c https://repo.manticoresearch.com/repository/manticoresearch_source/release/manticore-3.5.2-200722-6903305-release.tar.gz
tar -zxf manticore-3.5.2-*.tar.gz
cd manticore-3.5.2-*
```

### Configuring

Manticore uses cmake for pre-compiling configuration. To use it make a build directory somewhere, go to it, then invoke cmake, pointing it to the source dir. Simplest is to create the build directory inside unpacked sources. 

```bash
cd manticore-3.5.2
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

The cmake script will investigate available features and configure the build according to them.

### Caveats

1. If you want to configure without mysql, provide explicitly `-DWITH_MYSQL=0`, otherwise configuring will fail if mysql is absent.
2. If you want to finally build full-featured RPM package, path to build directory must be long enough in order to correctly build debug symbols. Like `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`, for example. That is because RPM tools modify the path over compiled binaries when building debug info, and it can just write over existing room and won't allocate more. Above mentioned long path has 100 chars and that is quite enough for such case.

### Configuration options

Options may be either provided in the form `-DNAME=VALUE` or via any available GUI or TUI interface interactively. On console systems usually together with `cmake` is also provided `ccmake` tool, which provides friendly access to the configuration.

* `CMAKE_BUILD_TYPE` (string) - can be Debug, Release, MinSizeRel and RelWithDebInfo (default). Usually we use just 'Debug' and 'RelWithDebInfo'. The first produces slower binaries, well applicable for testing and debugging features. The last is for production.
* `CMAKE_INSTALL_PREFIX` (path) - where to install the project. Building itself installs nothing, but prepares installation rules which are executed once you run 'make install' command, or create a package. It has a default depending on the cmake (on Linux usually /usr/local).
* `DISABLE_TESTING` (bool) whether to support testing. If enabled, after the build you cam run 'ctest' and test the build. Note that testing implies additional dependencies, like at least presence of PHP cli, python and available mysql server with test database. For 'just build', just disable the option, you don't need to care about it.
* `USE_BISON`, `USE_FLEX` (bool) - enabled by default, specifies whether to enable bison and flex tools. You can disable them building from tarball, as the necessary files already pre-generated and packed inside
* `LIBS_BUNDLE` - path to a folder with different libraries. This is mostly relevant for Windows building, but may be also helpful if you build quite often, in order to avoid downloading third-party sources each time.
* `DISTR_BUILD` - in case the target is packaging, it specifies the target operating system. Supported values are: `bionic`, `buster`, `debian`, `default`, `jessie`, `macos`, `macosbrew`, `rhel6`, `rhel7`, `rhel8`, `stretch`, `trusty`, `wheezy`, `xenial`. Providing `DISTR_BUILD` will cause to configure CMAKE_BUILD_TYPE as RelWithDebInfo, and WITH_MYSQL, WITH_EXPAT, WITH_PGSQL, WITH_RE2, WITH_STEMMER, and DISABLE_TESTING as 1. This option is intended for building packages. For example, running `cmake -DDISTR_BUILD=rhel8`, then `make package` will build RPM packaged for Red Hat Enterprise 8.

### Building on Linux systems

One line to get all dependencies on Debian/Ubuntu:

```bash
apt-get install build-essential cmake unixodbc-dev libpq-dev libexpat-dev libmysqlclient-dev libicu-dev libssl-dev libboost-system-dev libboost-program-options-dev git flex bison
```

Note: on Debian 9 (Stretch) package `libmysqlclient-dev` is absent. Use `default-libmysqlclient-dev` there instead.

One line to get all dependencies on Redhat/Centos:

```bash
yum install gcc gcc-c++ make cmake mysql-devel expat-devel postgresql-devel unixODBC-devel libicu-devel openssl-devel boost-devel rpm-build systemd-units  git flex bison
```

RHEL/CentOS 6  ships with an old version of the GCC compiler, which doesn't support `-std=c++11` flag, for compiling use `devtools` repository:

```bash
wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo
yum upgrade -y
yum install -y devtoolset-2-gcc devtoolset-2-binutils devtoolset-2-gcc-c++
export PATH=/opt/rh/devtoolset-2/root/usr/bin:$PATH
```

#### Compiling

To compile run:

```bash
make -j4
```

To install run:

```bash
make -j4 install
```

For building package use target `package`. It will build package according to selection, provided by `-DDISTR_BUILD` option. By default it will be a simple zip archive with binaries and supplementary files.

```bash
make -j4 package
```

For building source tarball for future, use target `tarball`. All lexers and stemmers will be created running flex/bison. Then the current version will be embedded into a header file, stemmer/re2 will be embedded into sources (if configured), and the final folder will be packed into the tarball for distribution.

```bash
make tarball
```

### Building for Linux systems via Docker

For preparing official packages we use docker containers. They include all necessary environment components and are proved as working solutions by our own builds. You can recreate any of them using Dockerfiles and `README.md` instruction, provided in `dist/build_dockers/` folder of the sources. That is easiest way to make the binaries for any supported Linux distribution, and also make packages there. Each docker provides `DISTR` environment variable, which can be passed directly to `DISTR_BUILD` config value (as `-DDISTR_BUILD=$DISTR` clause to cmake).

For example, to create RedHat 7 package 'as official', but without embedded ICU with it's big datafile, you may execute (implies that sources are placed in `/manticore/sources` folder of the host):

```bash
docker run -it --rm -v /manticore/sources:/manticore registry.gitlab.com/manticoresearch/dev/bionic_cmake314 bash
# following is inside docker shell. By default, workdir will be in the source folder, mounted as volume from the host. 
RELEASE_TAG="noicu"
mkdir build && cd build
cmake -DSPHINX_TAG=$RELEASE_TAG -DDISTR_BUILD=$DISTR -DWITH_ICU_FORCE_STATIC=0 ..
make -j4 package
```  

### Building on Windows

For building on Windows you need:

* Visual Studio
* Cmake for Windows
* prebuilt Expat, MySQL and PostgreSQL in bundle directory.

If you build from git clone, you also need to provide `git`, `flex` and `bison` tools. They may be found in `cygwin` framework. When building from tarball these tools are not necessary. Building might be performed from `cmd` or from cygwin console.

For a simple building on x64:

```cmd
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DLIBS_BUNDLE="C:\bundle" "C:\manticore"
cmake -DWITH_PGSQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 .
cmake --build . --target package --config RelWithDebInfo
```

### Building on FreeBSD

Support for FreeBSD is limited and successful compiling is not guaranteed. We recommend checking the issue tracker for unresolved issues on this platform before trying to compile latest versions.

FreeBSD uses clang instead of gcc as system compiler and it is installed by default.

First install required packages:

```bash
pkg install cmake bison flex
```

To compile a version without optional dependencies:

```bash
cmake -DUSE_GALERA=0 -DWITH_MYSQL=0 -DDISABLE_TESTING=1 ../manticoresearch/
make
``` 

Except for Galera the rest of optional dependencies can be installed so:

```bash
pkg install mariadb103-client postgresql-libpqxx unixODBC icu expat
```

(you can replace `mariadb103-client` with the MySQL client package of your choice)

Building with all optional features and installation system-wide:

```bash
cmake -DUSE_GALERA=0 -DWITH_PGSQL=1 -DDISABLE_TESTING=1 -DCMAKE_INSTALL_PREFIX=/ -DCMAKE_INSTALL_LOCALSTATEDIR=/var ../manticoresearch/
make
make install
```
   
## Recompilation (update)

If you didn't change the path for sources and build, just move to you build folder and run:

```bash
cmake .
make clean
make
```

If by any reason it doesn't work, you can delete file `CMakeCache.txt` located in the build folder. After this step you have to run cmake again, pointing to the source folder and configuring the options.

If it also doesn't help, just wipe out your build folder and begin from scratch.
