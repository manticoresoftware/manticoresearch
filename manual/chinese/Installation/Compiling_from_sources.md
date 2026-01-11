# 从源码编译 Manticore

从源码编译 Manticore Search 可以实现自定义的构建配置，例如禁用某些功能或添加新的补丁进行测试。例如，您可能希望从源码编译并禁用嵌入式 ICU，以便使用系统上安装的、可以独立于 Manticore 升级的不同版本。如果您有兴趣为 Manticore Search 项目做贡献，这也很有用。

## 使用 CI Docker 构建
为了准备[官方发布和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和一个特殊的构建镜像。该镜像包含必要的工具，并设计为与外部系统根目录（sysroots）一起使用，因此一个容器可以为所有操作系统构建软件包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 来构建该镜像，或者使用 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 上的镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，您还需要指定以下环境变量：

* `DISTR`：目标平台：`bionic`、`focal`、`jammy`、`buster`、`bullseye`、`bookworm`、`rhel7`、`rhel8`、`rhel9`、`rhel10`、`macos`、`windows`、`freebsd13`
* `arch`：架构：`x86_64`、`x64`（适用于 Windows）、`aarch64`、`arm64`（适用于 Macos）
* `SYSROOT_URL`：系统根目录归档文件的 URL。除非您自己构建系统根目录（说明可以在[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)找到），否则可以使用 https://repo.manticoresearch.com/repository/sysroots。
* 参考 CI 工作流文件以查找您可能需要使用的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

要查找 `DISTR` 和 `arch` 的可能值，您可以参考目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/，因为它包含了所有支持组合的系统根目录。

之后，在 Docker 容器内构建软件包就像调用以下命令一样简单：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，要创建一个类似于 Manticore 核心团队提供的官方版本的 Ubuntu Jammy 软件包，您应该在包含 Manticore Search 源代码的目录中执行以下命令。该目录是从 https://github.com/manticoresoftware/manticoresearch 克隆的仓库的根目录：

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
在某些情况下（例如 Centos），需要较长的源代码目录路径，否则可能无法成功构建源代码。

同样地，您不仅可以为流行的 Linux 发行版构建二进制文件或软件包，还可以为 FreeBSD、Windows 和 macOS 构建。

#### 使用 Docker 构建 SRPM

您也可以使用相同的特殊 Docker 镜像来构建 SRPM：

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

这将生成一个包含所有源代码的源 RPM（`.src.rpm` 文件）。

#### 从 SRPM 构建二进制 RPM

生成 SRPM 后，您可以使用它来构建完整的二进制 RPM 软件包集：

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

> 注意：**要从 SRPM 构建 RPM，您需要确保 SRPM 中列出的所有依赖项都已完全安装，这可能具有挑战性。** SRPM 在以下方面仍然有用：
> - 审计构建过程或检查源代码和 spec 文件
> - 对构建进行自定义修改或打补丁
> - 了解二进制文件是如何生成的
> - 满足开源许可证合规性要求

## 手动构建

不使用构建 Docker 来编译 Manticore **不推荐**，但如果您需要这样做，以下是一些您可能需要了解的信息：

### 所需工具

* C++ 编译器
  * 在 Linux 中 - 可以使用 GNU（4.7.2 及以上版本）或 Clang
  * 在 Windows 中 - Microsoft Visual Studio 2019 及以上版本（社区版即可）
  * 在 macOS 中 - Clang（来自 XCode 的命令行工具，使用 `xcode-select --install` 安装）。
* Bison, Flex - 在大多数系统上，它们可以作为软件包使用；在 Windows 上，它们在 cygwin 框架中可用。
* Cmake - 在所有平台上使用（需要 3.19 或更高版本）

### 获取源代码

#### 从 git

Manticore 源代码[托管在 GitHub 上](https://github.com/manticoresoftware/manticoresearch)。
要获取源代码，请克隆仓库，然后检出所需的分支或标签。分支 `master` 代表主要的开发分支。发布时，会创建一个版本标签，例如 `3.6.0`，并为当前版本启动一个新分支，在本例中为 `manticore-3.6.0`。该版本分支在所有更改后的头部被用作构建所有二进制发布的源代码。例如，要获取 3.6.0 版本的源代码，您可以运行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 从归档文件

您可以通过使用 "Download ZIP" 按钮从 GitHub 下载所需的代码。.zip 和 .tar.gz 格式都适用。

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### 配置

Manticore 使用 CMake。假设您在克隆的仓库的根目录内：

```bash
mkdir build && cd build
cmake ..
```

CMake 将检查可用的功能并根据它们配置构建。默认情况下，如果可用，所有功能都被视为启用。该脚本还会下载并构建一些外部库，假设您想使用它们。隐式地，您将获得最大数量功能的支持。

您也可以通过标志和选项显式地配置构建。要启用功能 `FOO`，请在 CMake 调用中添加 `-DFOO=1`。
要禁用它，请使用 `-DFOO=0`。如果没有明确说明，启用一个不可用的功能（例如，在 MS Windows 构建中 `WITH_GALERA`）会导致配置失败并出现错误。禁用一个功能不仅会从构建中排除该功能，还会禁用对该系统的检查，并禁用任何相关外部库的下载/构建。  

#### 配置标志和选项

- **USE_SYSLOG** - 允许在 [query logging](../Logging/Query_logging.md) 中使用`syslog`。  
- **WITH_GALERA** -启用对搜索守护进程复制的支持。将在构建时配置此支持，并下载、构建并包含 Galera 库的源代码到发行/安装包中。通常，使用 Galera 构建是安全的，但不分发库本身（即没有 Galera 模块，没有复制）。然而，有时您可能需要显式地禁用它，例如如果您想构建一个设计上无法加载任何库的静态二进制文件，以便即使守护进程内部存在对 'dlopen' 函数的调用也会导致链接错误。  
- **WITH_RE2** - 使用 RE2 正则表达式库构建。这对于 [REGEX()](../Functions/String_functions.md#REGEX%28%29) 等函数以及 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)  
  功能。  
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 的源代码，编译并静态链接，以便最终二进制文件不依赖系统中共享的  `RE2` 库。  
- **WITH_STEMMER** - 使用 Snowball 词干提取库构建。  
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 的源代码，编译并静态链接，以便最终二进制文件不依赖系统中共享的  `libstemmer` 库。  
- **WITH_ICU** -  使用 ICU (International Components for Unicode) 库构建。它用于对中文文本进行分词。当设置 morphology=`icu_chinese` 时会使用它。  
- **WITH_JIEBA** -  使用 Jieba 中文分词工具构建。它用于对中文文本进行分词。当设置 morphology=`jieba_chinese` 时会使用它。  
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 的源代码，编译并静态链接，以便最终二进制文件不依赖系统中共享的 `icu` 库。还会将 ICU 数据文件包含到安装/发行包中。静态链接的 ICU 的目的是使用已知版本的库，以确保行为确定且不依赖任何系统库。您更可能希望使用系统 ICU，因为它可以随着时间更新而无需重新编译 Manticore 守护进程。在这种情况下，您需要显式禁用此选项。这样还可以节省 ICU 数据文件占用的空间（约 30M），因为它不会包含在发行包中。  
- **WITH_SSL** - 用于支持 HTTPS 以及与守护进程的加密 MySQL 连接。系统的 OpenSSL 库将链接到守护进程。这意味着启动守护进程时需要 OpenSSL。对于支持 HTTPS，这是强制性的，但对于服务器本身并非严格必要（即没有 SSL 则无法通过 HTTPS 连接，但其他协议仍可工作）。Manticore 可以使用从 1.0.2 到 1.1.1 的 SSL 库版本，  但请注意 **出于安全考虑，强烈建议使用最新的 SSL  
  库**。目前仅支持 v1.1.1，其余版本已过时（  
  参见 [openssl 发布策略](https://www.openssl.org/policies/releasestrat.html)  
- **WITH_ZLIB** - 索引器用于处理来自 MySQL 的压缩列。守护进程用于提供对压缩 MySQL 协议的支持。  
- **WITH_ODBC** - 索引器用于支持来自 ODBC 提供者的索引源（它们通常是 UnixODBC 和 iODBC）。在 MS Windows 上，ODBC 是处理 MS SQL 源的正确方法，因此对 `MSSQL` 的索引也意味着启用此标志。  
- **DL_ODBC** - 不与 ODBC 库链接。如果 ODBC 已链接但不可用，则即使您想处理与 ODBC 无关的内容，也无法启动 indexer 工具。此选项要求索引器仅在处理 ODBC 源时才在运行时加载该库。  
- **ODBC_LIB** - ODBC 库文件的名称。当您想处理 ODBC 源时，索引器将尝试加载该文件。此选项根据可用的 ODBC 共享库检测自动写入。您也可以在运行索引器之前，通过设置环境变量 `ODBC_LIB` 并指定备用库的正确路径来覆盖该名称。  
- **WITH_EXPAT** - 索引器用于支持对 xmlpipe 源的索引。  
- **DL_EXPAT** - 不与 EXPAT 库链接。如果 EXPAT 已链接但不可用，则即使您想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在处理 xmlpipe 源时才在运行时加载该库。  
- **EXPAT_LIB** - EXPAT 库文件的名称。当您想处理 xmlpipe 源时，索引器将尝试加载该文件。此选项根据可用的 EXPAT 共享库检测自动写入。您也可以在运行索引器之前，通过设置环境变量 `EXPAT_LIB` 并指定备用库的正确路径来覆盖该名称。  
- **WITH_ICONV** - 在使用索引器索引 xmlpipe 源时，用于支持不同的编码。  
- **DL_ICONV** - 不与 iconv 库链接。如果 iconv 已链接但不可用，则即使您想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在处理 xmlpipe 源时才在运行时加载该库。  
- **ICONV_LIB** - iconv 库文件的名称。当您想处理 xmlpipe 源时，索引器将尝试加载该文件。此选项根据可用的 iconv 共享库检测自动写入。您也可以在运行索引器之前，通过设置环境变量 `ICONV_LIB` 并指定备用库的正确路径来覆盖该名称。  
- **WITH_MYSQL** - 索引器用于支持索引 MySQL 源。  
- **DL_MYSQL** - 不与 MySQL 库链接。如果 MySQL 已链接但不可用，则即使您想处理与 MySQL 无关的内容，也无法启动 the`indexer` 工具。此选项要求索引器仅在处理 MySQL 源时才在运行时加载该库。  
- **MYSQL_LIB** -- MySQL 库文件的名称。当您想处理 MySQL 源时，索引器将尝试加载该文件。此选项根据可用的 MySQL 共享库检测自动写入。您也可以在运行索引器之前，通过设置环境变量 `MYSQL_LIB` 并指定备用库的正确路径来覆盖该名称。  
- **WITH_POSTGRESQL** - 索引器用于支持索引 PostgreSQL 源。  
- **DL_POSTGRESQL** - 不与 PostgreSQL 库链接。如果 PostgreSQL 已链接但不可用，则即使您想处理与 PostgreSQL 无关的内容，也无法启动 `indexer` ool。此选项要求索引器仅在处理 PostgreSQL 源时才在运行时加载该库。  
- **POSTGRESQL_LIB** - PostgreSQL 库文件的名称。索引器在处理 PostgreSQL 源时将尝试加载指定的 PostgreSQL 库文件。此选项根据可用的 PostgreSQL 共享库检测自动确定。您也可以在运行索引器之前，通过提供环境变量  `POSTGRESQL_LIB` 并指定备用库的正确路径来覆盖该名称。  
- **LOCALDATADIR** - 守护进程存储 binlogs 的默认路径。如果此路径未在守护进程的运行时配置中提供或被显式禁用（即与此构建配置无关的 `manticore.conf` 文件），则 binlogs 将放置在此路径。它通常是绝对路径，但也可以使用相对路径。您通常无需更改此配置定义的默认值，根据目标系统，可能类似 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data`。  
- **FULL_SHARE_DIR** - 存储所有资源的默认路径。在启动任何使用该文件夹中文件的工具之前，可以通过环境变量 `FULL_SHARE_DIR` 覆盖此路径。这是一个重要路径，因为许多内容默认都期望在此处找到，包括预定义字符集表、停用词、manticore 模块和 icu 数据文件，均放置于该文件夹中。配置脚本通常会将此路径确定为类似 `/usr/share/manticore` 或 `/usr/local/share/manticore`。  
- **DISTR_BUILD** - 发布软件包选项的快捷方式。它是一个字符串值，表示目标平台的名称。可以用它来代替手动配置所有选项。在 Debian 和 Redhat Linux 上，默认值可能通过轻量级检测确定并设置为通用的 'Debian' 或 'RHEL'。否则，该值未定义。  
- **PACK** - 更方便的快捷方式。它读取环境变量  `DISTR`，将其赋值给 **DISTR_BUILD** 参数，然后按常规方式工作。这在使用预先准备的构建系统（如 Docker 容器）中构建时非常有用，  `DISTR` 变量在系统级别设置，并反映容器所针对的目标系统。  
- **CMAKE_INSTALL_PREFIX** (path) - Manticore 预期安装的位置。构建不会执行任何安装，但会准备在运行 `cmake --install` 命令或创建包然后安装时执行的安装规则。前缀可以随时更改，即使在安装过程中，也可以通过调用  
  `cmake --install . --prefix /path/to/installation`。但是，在配置时，此变量用于初始化  `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。例如，在配置时将其设置为 `/my/custom` 会  
  强制将 `LOCALDATADIR` 硬编码为 `/my/custom/var/lib/manticore/data`，并将 `FULL_SHARE_DIR` 硬编码为  
  `/my/custom/usr/share/manticore`。  
- **BUILD_TESTING** (bool) 是否支持测试。如果启用，则在构建后可以运行 'ctest' 来测试构建。请注意，测试需要额外的依赖，例如至少需要 PHP cli、Python 以及可用的 MySQL 服务器和测试数据库。默认情况下，此参数为开启。因此，如果仅想构建，您可能需要通过显式指定 'off' 值来禁用此选项。  
- **BUILD_SRPMS** (bool) 是否显示构建 Source RPM (SRPM) 的说明。由于 CPack 在基于组件的打包方面的限制，SRPM 无法与二进制 RPM 同时生成。启用时，构建系统将显示使用源配置方法正确生成 SRPM 的说明。默认情况下，此参数为关闭。  
- **LIBS_BUNDLE** - 存放各种库的文件夹路径。这主要与 Windows 构建相关，但如果您需要频繁构建以避免每次都下载第三方源代码，也可能很有帮助。默认情况下，配置脚本不会修改此路径；您应手动将所有内容放在其中。例如，当我们需要支持词干提取器时，会从 Snowball 主页下载源代码包，然后解压、配置、构建等。相反，您可以将原始源代码 tarball（即 `libstemmer_c.tgz`）存放在此文件夹中。下次从头开始构建时，配置脚本将首先在 bundle 中查找，如果在那里找到词干提取器，就不会再次从 Internet 下载。  
- **CACHEB** - 存放第三方库构建结果的文件夹路径。通常，在使用 galera、re2、icu 等功能时，会先从 bundle 下载或获取源代码，然后解压、构建并安装到一个临时内部文件夹。当构建 manticore 时，该文件夹将作为所需功能的依赖所在位置。最后，它们要么与 manticore 链接（如果是库）；要么直接进入发行/安装包（如 galera 或 icu 数据）。当 **CACHEB** 被定义为 cmake 配置参数或系统环境变量时，它将被用作这些构建的目标文件夹。此文件夹可以在多次构建之间保留，这样存储在其中的库就不需要再次构建，从而大大缩短了整个构建过程。


注意，有些选项是以三元组的形式组织的：`WITH_XXX`，`DL_XXX` 和 `XXX_LIB` - 比如 MySQL、ODBC 等的支持。`WITH_XXX` 决定了接下来两个选项是否有作用。也就是说，如果你将 `WITH_ODBC` 设置为 `0`，那么就没有必要提供 `DL_ODBC` 和 `ODBC_LIB`，如果整个功能被禁用，这两个选项也不会起作用。同样，如果没有 `DL_XXX` 选项，`XXX_LIB` 也没有意义，因为如果不使用动态加载，提供的 `XXX_LIB` 名称就无用武之地。这默认使用了内部反查。

另外，使用 `iconv` 库假设了 `expat` 的存在，并且在 `expat` 被禁用时是无用的。

另外，一些库可能总是可用的，因此避免与它们链接是没有意义的。例如，在 Windows 中是 ODBC，在 macOS 中是 Expat、iconv 及其他一些库。默认的内部反查会确定这些库，并仅发出 `WITH_XXX` 而不是 `DL_XXX` 和 `XXX_LIB`，这样可以使事情更简单。

对于某些选项，配置文件可能会看起来像这样：

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

除了通用配置值外，你还可以检查 `CMakeCache.txt` 文件，该文件会在你运行配置后留在构建文件夹中。任何在那里定义的值都可以在运行 cmake 时显式地重新定义。例如，你可以运行 `cmake -DHAVE_GETADDRINFO_A=FALSE ...`，这样配置运行不会假设该变量的已调查值，而是使用你提供的值。

#### 特定环境变量

环境变量用于提供一些全局设置，这些设置存储在构建配置之外，并始终存在。为了持久化，可以在系统中以不同的方式设置它们 - 例如，将其添加到 `.bashrc` 文件中，或嵌入到 Dockerfile 中（如果你生成基于 Docker 的构建系统），或在 Windows 的系统偏好设置环境中编写它们。你也可以使用 `export VAR=value` 在 shell 中临时设置它们。甚至更短，通过在 cmake 调用前预置值，如 `CACHEB=/my/cache cmake ...` - 这样它只会在这次调用中工作，并不会在下次调用中可见。

一些这样的变量通常由 cmake 和其他工具使用，例如 `CXX`，它确定当前使用的 C++ 编译器，或者 `CXX_FLAGS` 提供编译器标志等。

然而，我们有一些特定于 Manticore 配置的变量，这些变量仅为我们的构建而发明。

- **CACHEB** - 同配置中的 **CACHEB** 选项
- **LIBS_BUNDLE** - 同配置中的 **LIBS_BUNDLE** 选项
- **DISTR** - 当使用 `-DPACK=1` 时，用于初始化 `DISTR_BUILD` 选项。
- **DIAGNOSTIC** - 使 cmake 配置输出更加详细，解释所有发生的事情
- **WRITEB** - 假设 **LIBS_BUNDLE** 并且如果设置了它，将会下载不同工具的源代码文件到 LIBS_BUNDLE 文件夹中。也就是说，如果新的词干版本发布出来 - 你可以手动从捆绑包中删除 libstemmer_c.tgz，然后运行一次性的 `WRITEB=1 cmake ...` - 它将不会在捆绑包中找到词干的源代码，然后会从供应商站点下载它们到捆绑包中（不使用 WRITEB 时，它们会被下载到构建文件夹内的一个临时文件夹中，并在你清除构建文件夹时消失）。

配置结束时，你可能会看到将要使用的一系列内容：

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

要安装，请运行：

```bash
cmake --install . --config RelWithDebInfo
```

要安装到自定义（非默认）文件夹，请运行

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### 构建包

要构建包，请使用目标 `package`。它将根据 `-DDISTR_BUILD` 选项提供的选择来构建包。默认情况下，它将是一个包含所有二进制文件和辅助文件的简单的 .zip 或 .tgz 存档。

```bash
cmake --build . --target package --config RelWithDebInfo
```

## 关于构建的一些高级事项

### 单配置的重新编译（更新）

如果你没有更改源路径和构建路径，只需移动到你的构建文件夹并运行：

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

如果由于某种原因不起作用，可以删除构建文件夹中的 `CMakeCache.txt` 文件。在此步骤之后，你需要再次运行 cmake，指向源文件夹并配置选项。

如果这也不起作用，只需清除你的构建文件夹并从头开始。

### 构建类型

简而言之 - 就是使用上面写的 `--config RelWithDebInfo`。它不会出错。

我们使用两种构建类型。开发时使用 `Debug` - 它以非常有利于开发的方式分配编译器标志和其他内容，意味着调试运行时可以逐行执行。但是，生成的二进制文件对于生产来说既大又慢。

发布时，我们使用另一种类型 - `RelWithDebInfo` - 这意味着“带有调试信息的发布构建”。它生成带有嵌入调试信息的生产二进制文件。后者随后会被拆分成单独的调试信息包，并与发布的包一起存储，以便在出现问题如崩溃时进行调查和修复。Cmake 还提供了 `Release` 和 `MinSizeRel`，但我们不使用它们。如果构建类型不可用，cmake 将创建一个 `noconfig` 构建。

#### 构建系统生成器

有两种类型的生成器：单配置和多配置。

- 单配置需要在配置期间提供的构建类型，通过 `CMAKE_BUILD_TYPE` 参数。如果没有定义，则构建将回退到 `RelWithDebInfo` 类型，适合你只是从源代码构建 Manticore 而不参与开发。对于显式构建，你应该提供一个构建类型，如 `-DCMAKE_BUILD_TYPE=Debug`。
- 多配置在构建期间选择构建类型。它应该通过 `--config` 选项提供，否则将构建一种称为 `noconfig` 的类型，这是不希望的。所以，你应该始终指定构建类型，如 `--config Debug`。

如果您希望指定构建类型但不关心它是 '单' 配置还是 '多' 配置生成器——只需在两个地方都提供必要的键。即，配置时使用 `-DCMAKE_BUILD_TYPE=Debug`，然后构建时使用 `--config Debug`。只需确保这两个值相同。如果目标构建器是单配置的，它会使用配置参数。如果是多配置的，配置参数会被忽略，但正确的构建配置会被 `--config` 键选择。

如果您想要 `RelWithDebInfo`（即仅构建用于生产）并且知道您在一个单配置平台（即除了 Windows 之外的所有平台）——您可以在在 cmake 调用中省略 `--config` 标志。那么默认 `CMAKE_BUILD_TYPE=RelWithDebInfo` 会被配置并使用。所有 '构建'、'安装' 和 '构建包' 命令会变得更短。

#### 明确选择构建系统生成器

Cmake 是工具本身不执行构建，但它生本地构建系统的规则。
通常，它很好地确定可用构建系统，但有时您可能需要明确提供生成器。您
可以运行 `cmake -G` 并查看可用生成器列表。

- 在 Windows，如果您有多个 Visual Studio 版本安装，您可能需要指定使用哪一个，
如：
```bash
cmake -G "Visual Studio 16 2019" ....
  ```
- 在所有其他平台——通常使用 Unix Makefiles，但您可以指定另一个，如 Ninja，或 Ninja 多配置，如：
  多配置`，如：
```bash
  cmake -GNinja ...
  ```
  或
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja 多配置相当有用，因为它真的 '多配置' 并可用在 Linux/macOS/BSD。用此生成器，您可能将选择配置类型到构建时间，并且您可能在同一构建文件夹构建多个配置，仅改变 `--config` 参数。

### 注意事项

1. 如果您最终想构建全功能 RPM 包，构建目录路径必须足够长以便正确构建调试符号。
如 `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`，例如。这是因为 RPM 工具在构建调试信息时会修改编译二进制路径，并且它可能仅覆盖现有空间不会分配更多。前述长路径有 100 字符，足够为此情况。

## 外部依赖

如果您想使用它们，某些库应该可用。
- 对于索引（`indexer` 工具）：`expat`，`iconv`，`mysql`，`odbc`，`postgresql`。没有它们，您仅能处理 `tsv` 和 `csv` 源。
- 对于服务查询（`searchd` 守护）：可能需要 `openssl`。
- 对于所有（必需，强制！）我们需要 Boost 库。最低版本是 1.61.0，然而，我们构建二进制用较新版本 1.75.0。更近期版本（如 1.76）也应也正常。在 Windows，您可从他们的网站（boost.org）下载预构建 Boost 并安装到默认建议路径（即 `C:\\boost...`）。在 MacOs，brew 提供的版本正常。在 Linux，您可检查官方仓库可用版本，如果不匹配要求，您可从源码构建。我们需要组件 'context'，您也可构建组件 'system' 和 'program_options'，它们会必要如果您也想从源码构建 Galera 库。查看 `dist/build_dockers/xxx/boost_175/Dockerfile` 为简短自文档脚本/指令如何做。

在构建系统，您需要这些包安装的 'dev' 或 'devel' 版本（即 - libmysqlclient-devel，unixodbc-devel，等。查看我们 dockerfile 为具体包名称）。

在运行系统，这些包应至少存在最终（非 dev）变。（devel 变通常更大，因为它们包含不仅目标二进制，而且不同开发东西如包含头，等）。

### 在 Windows 构建

除了必要前提，您可能需要预构建 `expat`，`iconv`，`mysql`，和 `postgresql` 客户端库。您必须要么自己构建它们要么联系我们获取我们构建包（简单 zip 归档，其中这些目标文件夹位于）。

- ODBC 不必要，因为它系统库。
- OpenSSL 可从源码构建或从 https://slproweb.com/products/Win32OpenSSL.html 下载预构建（如 cmake 内部脚本 FindOpenSSL 提及）。
- Boost 可从 https://www.boost.org/ 发布下载预构建。

### 查看编译内容

运行 `indexer -h`。它会显示哪些功能配置并构建（无论它们明确或探测，不重要）：

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->

