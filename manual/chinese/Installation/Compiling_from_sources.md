# 从源码编译 Manticore

从源码编译 Manticore Search 可以实现自定义构建配置，例如禁用某些功能或添加新的补丁进行测试。例如，您可能希望从源码编译并禁用内嵌的 ICU，以便使用系统上安装的、可以独立于 Manticore 升级的不同版本。如果您有兴趣为 Manticore Search 项目贡献代码，这也非常有用。

## 使用 CI Docker 构建
为了准备[官方发布和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和一个特殊的构建镜像。该镜像包括必要的工具，设计用于与外部 sysroots 一起使用，因此一个容器可以构建所有操作系统的包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 构建镜像，或者使用来自 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 的镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，您还需要指定以下环境变量：

* `DISTR`：目标平台：`bionic`，`focal`，`jammy`，`buster`，`bullseye`，`bookworm`，`rhel7`，`rhel8`，`rhel9`，`rhel10`，`macos`，`windows`，`freebsd13`
* `arch`：架构：`x86_64`，`x64`（针对 Windows），`aarch64`，`arm64`（针对 Macos）
* `SYSROOT_URL`：系统根目录归档的 URL。您可以使用 https://repo.manticoresearch.com/repository/sysroots，除非您自己构建 sysroots（说明见[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)）。
* 可参考 CI 工作流程文件查找您可能需要的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

要查找 `DISTR` 和 `arch` 的可能取值，可以参考目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/，其中包含所有支持组合的 sysroots。

之后，在 Docker 容器内构建包就像调用一样简单：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，为 Ubuntu Jammy 创建一个与 Manticore Core 团队官方版本类似的包，您应在包含 Manticore Search 源代码的目录中执行以下命令。此目录是从 https://github.com/manticoresoftware/manticoresearch 克隆的仓库根目录：

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
需要较长的源码目录路径，否则在某些情况下（例如 Centos）可能构建失败。

同样，您不仅可以为流行的 Linux 发行版构建二进制文件或包，还可以为 FreeBSD、Windows 和 macOS 构建。

#### 使用 Docker 构建 SRPMs

您还可以使用相同的特殊 Docker 镜像构建 SRPMs：

```bash
docker run -it --rm \
-e CACHEB="../cache" \
-e DIAGNOSTIC=1 \
-e PACK_ICUDATA=0 \
-e NO_TESTS=1 \
-e DISTR=rhel8 \
-e boost=boost_rhel_feb17 \
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
export CMAKE_TOOLCHAIN_FILE=$(pwd)/../dist/build_dockers/cross/linux.cmake
# The CPackSourceConfig.cmake file is now generated in the build directory
cpack -G RPM --config ./CPackSourceConfig.cmake
```

这将生成包含所有源代码的源码 RPM (`.src.rpm` 文件)。

#### 从 SRPM 构建二进制 RPMs

生成 SRPM 后，您可以使用它构建完整的二进制 RPM 软件包集：

```bash
# Install build tools and dependencies
dnf install -y rpm-build cmake gcc-c++ boost-devel epel-release

# Install SRPM dependencies automatically
dnf builddep -y manticore-*.src.rpm

# Build all binary RPMs from the SRPM
rpmbuild --rebuild manticore-*.src.rpm

# Find the generated packages
ls ~/rpmbuild/RPMS/*/manticore*
```

> 注意：**要从 SRPM 构建 RPM，您需要确保 SRPM 中列出的所有依赖项都已完全安装，这可能具有挑战性。** SRPM 仍然有用的场景包括：
> - 审计构建过程或检查源代码和 spec 文件
> - 对构建进行自定义修改或补丁
> - 了解如何生成二进制文件
> - 满足开源许可合规要求

## 手动构建

不使用构建 Docker 编译 Manticore 是**不推荐的**，但如果您需要这样做，以下信息可能对您有用：

### 必需工具

* C++ 编译器
  * 在 Linux 上 - 可使用 GNU（4.7.2 及以上）或 Clang
  * 在 Windows 上 - Microsoft Visual Studio 2019 及以上（社区版足够）
  * 在 macOS 上 - Clang（来自 XCode 命令行工具，使用 `xcode-select --install` 安装）
* Bison，Flex - 在大多数系统中可作为包获得，在 Windows 上可通过 cygwin 框架获得。
* Cmake - 所有平台通用（需要版本 3.19 或以上）

### 获取源码

#### 从 git

Manticore 源代码托管在 [GitHub](https://github.com/manticoresoftware/manticoresearch)。
要获取源码，请克隆仓库，然后检出所需的分支或标签。分支 `master` 代表主开发分支。发布时，会创建一个版本标记，如 `3.6.0`，并开启当前版本的新分支，这里是 `manticore-3.6.0`。版本分支的最新提交用作构建所有二进制发布的源码。例如，要获取版本 3.6.0 的源码，您可以运行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 从归档

您可以通过 GitHub 的 “Download ZIP” 按钮下载所需代码。`.zip` 和 `.tar.gz` 格式均适用。

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### 配置

Manticore 使用 CMake。假设您处于克隆仓库的根目录：

```bash
mkdir build && cd build
cmake ..
```

CMake 会检测可用功能并据此配置构建。默认情况下，如果功能可用，则都视为启用。脚本还会下载并构建一些外部库，假设您希望使用它们。隐含地，您将支持最多数量的功能。

您也可以通过标志和选项显式配置构建。启用功能 `FOO` 可以在 CMake 调用中添加 `-DFOO=1`。
To disable it, use `-DFOO=0`. If not explicitly noted, enabling a feature that is not available（such as `WITH_GALERA` on an MS Windows build）will cause the configuration to fail with an error. Disabling a feature, apart from excluding it from the build, also disables its investigation on the system and disables the downloading/building of any related external libraries.

#### 配置标志和选项

- **USE_SYSLOG** - 允许在 [查询日志](../Logging/Query_logging.md) 中使用 `syslog`。
- **WITH_GALERA** - 启用搜索守护进程的复制支持。构建时将配置对该支持，并且 Galera 库的源代码将被下载、构建并包含在发行版/安装包中。通常，构建时启用 Galera 是安全的，但不分发库本身（因此没有 Galera 模块，也没有复制功能）。但是，有时你可能需要显式禁用它，例如如果你想构建一个设计上无法加载任何库的静态二进制文件，那么即使守护进程内部存在对 'dlopen' 函数的调用也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库构建。这对于类似 [REGEX()](../Functions/String_functions.md#REGEX%28%29) 和 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 的函数是必要的
  功能。
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 源码，编译并进行静态链接，以使最终二进制文件不依赖系统中共享的 `RE2` 库。
- **WITH_STEMMER** - 使用 Snowball 词干提取库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 源码，编译并进行静态链接，以使最终二进制文件不依赖系统中共享的 `libstemmer` 库。
- **WITH_ICU** -  使用 ICU (International Components for Unicode) 库进行构建。它用于对中文文本进行分词。当使用 morphology=`icu_chinese` 时会启用。
- **WITH_JIEBA** -  使用 Jieba 中文分词工具进行构建。它用于对中文文本进行分词。当使用 morphology=`jieba_chinese` 时会启用。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 源码，编译并进行静态链接，以使最终二进制文件不依赖系统中共享的 `icu` 库。同时将 ICU 数据文件包含在安装包/发行版中。静态链接 ICU 的目的是使用已知版本的库，以确保行为确定且不依赖任何系统库。你更可能倾向于使用系统 ICU，因为它可能随时间更新而无需重新编译 Manticore 守护进程。在这种情况下，你需要显式禁用此选项。这样还可以节省 ICU 数据文件（约 30M）占用的空间，因为它不会包含在发行版中。
- **WITH_SSL** - 用于支持 HTTPS，以及与守护进程的 MySQL 加密连接。系统 OpenSSL 库将链接到守护进程。这意味着启动守护进程需要 OpenSSL。这对于支持 HTTPS 是强制性的，但对服务器来说并非绝对必要（即没有 SSL 则无法通过 HTTPS 连接，但其他协议仍然可用）。Manticore 可以使用从 1.0.2 到 1.1.1 的 SSL 库版本，然而请注意 **为了安全起见，强烈建议使用最新的 SSL
  库**。目前仅支持 v1.1.1，其余版本均已过时（
  参见 [OpenSSL 发布策略](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - 索引器用于处理 MySQL 的压缩列。守护进程用于提供对压缩 MySQL 协议的支持。
- **WITH_ODBC** - 索引器用于支持对 ODBC 提供者（通常是 UnixODBC 和 iODBC）源进行索引。在 MS Windows 上，ODBC 是处理 MS SQL 源的合适方式，因此索引 `MSSQL` 也意味着需要此标志。
- **DL_ODBC** - 不与 ODBC 库进行链接。如果已链接 ODBC 库但不可用，则即使要处理与 ODBC 无关的内容，也无法启动索引器工具。此选项要求索引器仅在需要处理 ODBC 源时在运行时加载该库。
- **ODBC_LIB** - ODBC 库文件的名称。索引器在处理 ODBC 源时将尝试加载该文件。此选项会根据可用的 ODBC 共享库检测自动写入。你也可以在运行索引器之前，通过设置环境变量 `ODBC_LIB` 为另一个库的正确路径来重写此名称。
- **WITH_EXPAT** - 索引器用于支持对 xmlpipe 源进行索引。
- **DL_EXPAT** - 不与 EXPAT 库进行链接。如果已链接 EXPAT 库但不可用，则即使要处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 源时在运行时加载该库。
- **EXPAT_LIB** - EXPAT 库文件的名称。索引器在处理 xmlpipe 源时将尝试加载该文件。此选项会根据可用的 EXPAT 共享库检测自动写入。你也可以在运行索引器之前，通过设置环境变量 EXPAT_LIB 为另一个库的正确路径来重写此名称。
- **WITH_ICONV** - 用于在使用索引器索引 xmlpipe 源时支持不同的编码。
- **DL_ICONV** - 不与 iconv 库进行链接。如果已链接 iconv 库但不可用，则即使要处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 源时在运行时加载该库。
- **ICONV_LIB** - iconv 库文件的名称。当你想处理 xmlpipe 源时，indexer 将尝试加载该文件。此选项是根据可用的 iconv 共享库自动生成的。你也可以在运行 indexer 之前，通过设置环境变量 `ICONV_LIB` 为替代库的正确路径来在运行时覆盖该名称。
- **WITH_MYSQL** - indexer 用于支持索引 MySQL 源。
- **DL_MYSQL** - 不与 MySQL 库链接。如果 MySQL 已链接但不可用，即使你想处理与 MySQL 无关的内容，也无法启动`indexer`工具。此选项要求 indexer 仅在需要处理 MySQL 源时在运行时加载该库。
- **MYSQL_LIB** -- MySQL 库文件的名称。当你想处理 MySQL 源时，indexer 将尝试加载该文件。此选项是根据可用的 MySQL 共享库自动生成的。你也可以在运行 indexer 之前，通过设置环境变量 `MYSQL_LIB` 为替代库的正确路径来覆盖该名称。
- **WITH_POSTGRESQL** - indexer 用于支持索引 PostgreSQL 源。
- **DL_POSTGRESQL** - 不与 PostgreSQL 库链接。如果 PostgreSQL 已链接但不可用，即使你想处理与 PostgreSQL 无关的内容，也无法启动 `indexer` ool。此选项要求 indexer 仅在需要处理 PostgreSQL 源时在运行时加载该库。
- **POSTGRESQL_LIB** - postgresql 库文件的名称。indexer 将尝试在处理 postgresql 源时加载指定的 postgresql 库文件。此选项是根据可用的 postgresql 共享库自动确定的。你也可以在运行 indexer 之前，通过设置环境变量  `POSTGRESQL_LIB` 为替代库的正确路径来覆盖该名称。
- **LOCALDATADIR** - 守护进程存储 binlogs 的默认路径。如果在守护进程的运行时配置中未提供此路径或已显式禁用（即与此构建配置无关的 `manticore.conf` 文件），binlogs 将放置在此路径。它通常是一个绝对路径，但也可以使用相对路径。你可能无需更改配置定义的默认值，该默认值取决于目标系统，可能类似于 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data`。
- **FULL_SHARE_DIR** - 存储所有资源的默认路径。你可以在启动任何使用该文件夹中文件的工具之前，通过设置环境变量 `FULL_SHARE_DIR` 覆盖此路径。此路径非常重要，因为默认情况下会在此处查找许多内容。这些内容包括预定义的字符集表、停用词列表、manticore 模块和 icu 数据文件，均放置在该文件夹中。配置脚本通常将此路径确定为类似 `/usr/share/manticore` 或 `/usr/local/share/manticore`。
- **DISTR_BUILD** - 用于发布软件包选项的快捷方式。它是一个字符串值，表示目标平台的名称。可以用它来替代手动配置所有选项。在 Debian 和 Redhat Linux 上，默认值可能通过简单检测确定，并设置为通用的 'Debian' 或 'RHEL'。否则，该值未定义。
- **PACK** - 更便捷的快捷方式。它读取 `DISTR` 环境变量，将其赋值给 **DISTR_BUILD** 参数，然后按常规方式工作。当在诸如 Docker 容器等预配置的构建系统中构建时非常有用，此时  `DISTR` 变量在系统级别设置，并反映容器针对的目标系统。
- **CMAKE_INSTALL_PREFIX** (path) - 期望安装 Manticore 的位置。构建过程中不会执行任何安装，但它会准备安装规则，这些规则将在你运行 `cmake --install` 命令或创建并安装软件包时执行。可以在任何时候更改前缀，即使在安装期间，也可以通过调用
  `cmake --install . --prefix /path/to/installation`。但是，在配置时，此变量用于初始化  `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。例如，将其在配置时设置为 `/my/custom`
  在配置时将 `LOCALDATADIR` 硬编码为 `/my/custom/var/lib/manticore/data`，并将 `FULL_SHARE_DIR` 硬编码为
  `/my/custom/usr/share/manticore`。
- **BUILD_TESTING** (bool) 是否支持测试。如果启用，构建完成后可以运行 'ctest' 进行测试。请注意，测试意味着会有额外的依赖项，例如至少需要安装 PHP cli、Python，以及具有测试数据库的可用 MySQL 服务器。默认情况下，此参数为开启。 因此，如果只想构建，可能需要通过显式指定 'off' 来禁用该选项。
- **BUILD_SRPMS** (bool) 是否显示构建 Source RPMs (SRPMs) 的说明。由于 CPack 在基于组件的打包方面存在限制，SRPMs 无法与二进制 RPMs 一起直接生成。启用时，构建系统将显示使用源配置方法生成 SRPM 的正确说明。默认情况下，此参数为 off。
- **LIBS_BUNDLE** - 含有不同库的文件夹路径。这主要与 Windows 构建相关，但如果你需要频繁构建以避免每次都下载第三方源，则也非常有用。默认情况下，配置脚本不会修改此路径；你需要手动将所有内容放入该文件夹。例如，当我们需要支持一个 stemmer 时，源代码将从 Snowball 主页下载，然后解压、配置、构建等。相反，你可以将原始源代码压缩包（即 `libstemmer_c.tgz`）存放在此文件夹中。下次从头构建时，配置脚本将首先在 bundle 中查找，如果发现 stemmer，就不会再次从互联网下载。
- **CACHEB** - 指向一个存放第三方库构建产物的文件夹路径。通常诸如 galera、re2、icu 等特性首先会从网络下载或从捆绑包中获取，然后解包、构建并安装到一个临时的内部文件夹中。当构建 manticore 时，该文件夹将作为运行所需特性的资源所在位置。最终，如果 manticore 是一个库，这些特性会与 manticore 链接；如果是像 galera 或 icu 数据这样的，则直接进行分发/安装。当 **CACHEB** 被定义为 cmake 配置参数或系统环境变量时，它被用作这些构建的目标文件夹。此文件夹可以跨构建保留，以便其中存储的库不会被重复构建，从而大大缩短整个构建过程。


Note, that some options are organized in triples: `WITH_XXX`, `DL_XXX` and `XXX_LIB` - like support of mysql, odbc, etc. `WITH_XXX` determines whether next two have an effect or not. I.e., if you set `WITH_ODBC` to `0` - there is no sence to provide `DL_ODBC` and `ODBC_LIB`, and these two will have no effect if the whole feature is disabled. Also, `XXX_LIB` has no sense without `DL_XXX`, because if you don't want `DL_XXX` option, dynamic loading will not be used, and name provided by `XXX_LIB` is useless. That is used by default introspection.

Also, using the `iconv` library assumes `expat` and is useless if the last is disabled.

Also, some libraries may be always available, and so, there is no sense to avoid linkage with them. For example, in Windows that is ODBC. On macOS that is Expat, iconv, and m.b. others. Default introspection determines such libraries and effectively emits only `WITH_XXX` for them, without `DL_XXX` and `XXX_LIB`, that makes the things simpler.

With some options in game configuring might look like:

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

Apart general configuration values, you may also investigate file `CMakeCache.txt` which is left in build folder right after you run configuration. Any values defined there might be redefined explicitly when running cmake. For example, you may run `cmake -DHAVE_GETADDRINFO_A=FALSE ...`, and that config run will not assume investigated value of that variable, but will use one you've provided.

#### 特定环境变量

Environment variables are useful for providing some kind of global settings which are stored aside from build configuration and are always present. For persistence, they may be set globally on the system using different ways - like adding them to the `.bashrc` file, or embedding them into a Dockerfile if you produce a docker-based build system, or writing them in system preferences environment variables on Windows. Also, you may set them short-lived using `export VAR=value` in the shell. Or even shorter, by prepending values to the cmake call, like `CACHEB=/my/cache cmake ...` - this way it will only work on this call and will not be visible on the next.

Some of such variables are known to be used in general by cmake and some other tools. That is things like `CXX` which determines the current C++ compiler, or `CXX_FLAGS` to provide compiler flags, etc.

However, we have some variables that are specific to manticore configuration, which are invented solely for our builds.

- **CACHEB** - 与配置中的 **CACHEB** 选项等效
- **LIBS_BUNDLE** - 与配置中的 **LIBS_BUNDLE** 选项等效
- **DISTR** - 在使用 `-DPACK=1` 时用于初始化 `DISTR_BUILD` 选项。
- **DIAGNOSTIC** - 使 cmake 配置输出更加详细，解释所有执行过程
- **WRITEB** - 依赖 **LIBS_BUNDLE**，如果设置，则会将各种工具的源归档文件下载到 LIBS_BUNDLE 文件夹中。也就是说，如果出现了一个新的 stemmer 版本，您可以手动从 bundle 中删除 libstemmer_c.tgz，然后运行一次性命令 `WRITEB=1 cmake ...`——此时它会在 bundle 中找不到 stemmer 的源代码，然后从官方站点下载到 bundle 中（如果不使用 WRITEB，则会下载到构建目录内的临时文件夹，清空构建目录时会被删除）。

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

### 构建

```bash
cmake --build . --config RelWithDebInfo
```

### 安装

To install run:

```bash
cmake --install . --config RelWithDebInfo
```

to install into custom (non-default) folder, run

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### 打包构建

For building a package, use the target `package`. It will build the package according to the selection provided by the `-DDISTR_BUILD` option. By default, it will be a simple .zip or .tgz archive with all binaries and supplementary files.

```bash
cmake --build . --target package --config RelWithDebInfo
```

## 一些高级构建事项

### 单配置下的重新编译（更新）

If you haven't changed the path for sources and build, simply move to your build folder and run:

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

If by any reason it doesn't work, you can delete file `CMakeCache.txt` located in the build folder. After this step you
必须再次运行 cmake，指向源码文件夹并配置相应选项。

If it also doesn't help, just wipe out your build folder and begin from scratch.

### 构建类型

Briefly - just use `--config RelWithDebInfo` as written above. It will make no mistake.

We use two build types. For development, it is `Debug` - it assigns compiler flags for optimization and other things in a way that it is very friendly for development, meaning the debug runs with step-by-step execution. However, the produced binaries are quite large and slow for production.

For releasing, we use another type - `RelWithDebInfo` - which means 'release build with debug info'. It produces production binaries with embedded debug info. The latter is then split away into separate debuginfo packages which are stored aside with release packages and might be used in case of some issues like crashes - for investigation and bugfixing. Cmake also provides `Release` and `MinSizeRel`, but we don't use them. If the build type is not available, cmake will make a `noconfig` build.

#### 构建系统生成器

There are two types of generators: single-config and multi-config.

- 单配置（Single-config）需要在配置时通过 `CMAKE_BUILD_TYPE` 参数提供构建类型。如果未定义，则构建将退回到 `RelWithDebInfo` 类型，这对于仅想从源码构建 Manticore 而不参与开发的情况是合适的。对于需要明确指定的构建，您应该提供一个构建类型，例如 `-DCMAKE_BUILD_TYPE=Debug`。
- 多配置（Multi-config）在构建时选择构建类型。应通过 `--config` 选项提供，否则它将构建一种 `noconfig`，这并不理想。因此，您应始终指定构建类型，如 `--config Debug`。

如果你想指定构建类型，但又不想关心是“单配置”还是“多配置”生成器——只需在两个地方都提供必要的参数即可。例如，用 `-DCMAKE_BUILD_TYPE=Debug` 进行配置，然后用 `--config Debug` 进行构建。只要确保二者值一致即可。如果目标构建器是单配置，它将消费配置参数。如果是多配置，配置参数会被忽略，但正确的构建配置将按 `--config` 参数选择。

如果你想要 `RelWithDebInfo`（即只是为了生产构建）并且知道你在单配置平台上（除了 Windows 以外的都属于此类）——则可以在 cmake 调用时省略 `--config` 参数。此时将会配置并使用默认的 `CMAKE_BUILD_TYPE=RelWithDebInfo`。这样所有“构建”、“安装”和“构建包”的命令都会更短。

#### 明确选择构建系统生成器

Cmake 是一个自身不执行构建的工具，而是为本地构建系统生成规则。
通常它能够很好地检测可用的构建系统，但有时你可能需要显式提供一个生成器。你
可以运行 `cmake -G` 并查看可用生成器列表。

- 在 Windows 上，如果你安装了多个版本的 Visual Studio，可能需要指定要使用哪个版本，
例如：
```bash
cmake -G "Visual Studio 16 2019" ....
  ```
- On all other platforms - usually Unix Makefiles are used, but you can specify another one, such as Ninja, or Ninja Multi-Config, as:
  Multi-Config`, as:
```bash
  cmake -GNinja ...
  ```
  或者
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja Multi-Config 非常实用，因为它是真正的“多配置”并在 Linux/macOS/BSD 上都可用。使用此生成器，你可以将配置类型的选择推迟到构建时，同时你也可以在同一个构建文件夹中构建多个配置，只需更改 `--config` 参数即可。

### 注意事项

1. 如果你最终要构建功能齐全的 RPM 包，构建目录的路径必须足够长，以便正确构建调试符号。
例如 `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`。这是因为 RPM 工具在构建调试信息时会修改编译二进制文件上的路径，它会直接写覆盖现有空间，不会再分配更多。上述长路径有 100 个字符，这样的情况下就足够了。

## 外部依赖

如果你想使用某些库，有些库需要是可用的。
- 用于索引（`indexer` 工具）：`expat`、`iconv`、`mysql`、`odbc`、`postgresql`。没有它们，你只能处理 `tsv` 和 `csv` 源。
- 用于查询服务（`searchd` 守护进程）：可能需要 `openssl`。
- 对于全部（必须的，强制！）我们需要 Boost 库。最低版本为 1.61.0，但我们用更新的 1.75.0 构建二进制文件。更高版本（如 1.76）通常也没问题。Windows 上可从官网（boost.org）下载预编译 Boost 并安装到默认建议路径（如 `C:\\boost...`）。MacOs 上 brew 提供的版本即可。Linux 上可查阅官方仓库中的可用版本，如果不符合要求，可自行编译。我们需要 'context' 组件，你也可以编译 'system' 和 'program_options' 组件，如果你还需要从源码构建 Galera 库的话。可查看 `dist/build_dockers/xxx/boost_175/Dockerfile`，其中有简短的自注释脚本/说明。

在构建系统上，你需要安装这些包的 'dev' 或 'devel' 版本（如 libmysqlclient-devel、unixodbc-devel 等。具体包名称可查看我们的 dockerfile）。

在运行系统上，这些包至少要有最终（非开发）版本。（devel 版一般更大，还含有目标二进制外的开发工具，如头文件等）。

### Windows 上的构建

除了必要的前置条件，你可能还需要预编译的 `expat`、`iconv`、`mysql` 和 `postgresql` 客户端库。你需要自己编译，或联系我们获取我们的构建包（一个简单的 zip 压缩包，包含这些目标的文件夹）。

- ODBC 不必担心，因为它是系统库。
- OpenSSL 可以自行从源码编译，或从 https://slproweb.com/products/Win32OpenSSL.html 下载预编译版本（如 cmake FindOpenSSL 内部脚本中所述）。
- Boost 可从 https://www.boost.org/ releases 下载预编译版本。

### 查看编译了哪些内容

运行 `indexer -h`。它会显示哪些功能被配置和构建（无论是显式还是自动检测到的都显示）：

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->

