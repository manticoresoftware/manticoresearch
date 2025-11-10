# 从源码编译 Manticore

从源码编译 Manticore Search 可以实现自定义构建配置，例如禁用某些功能或添加新的补丁进行测试。例如，您可能希望从源码编译并禁用内嵌的 ICU，以便使用系统上安装的不同版本，该版本可以独立于 Manticore 升级。如果您有兴趣为 Manticore Search 项目做贡献，这也非常有用。

## 使用 CI Docker 构建
为了准备[官方发布和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和一个特殊的构建镜像。该镜像包含必要的工具，并设计为与外部 sysroots 一起使用，因此一个容器可以为所有操作系统构建包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 构建该镜像，或者使用来自 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 的镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，您还需要指定以下环境变量：

* `DISTR`：目标平台：`bionic`、`focal`、`jammy`、`buster`、`bullseye`、`bookworm`、`rhel7`、`rhel8`、`rhel9`、`rhel10`、`macos`、`windows`、`freebsd13`
* `arch`：架构：`x86_64`、`x64`（Windows 用）、`aarch64`、`arm64`（Macos 用）
* `SYSROOT_URL`：系统根目录归档的 URL。除非您自己构建 sysroots（说明见[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)），否则可以使用 https://repo.manticoresearch.com/repository/sysroots。
* 使用 CI 工作流文件作为参考，查找您可能需要使用的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

要查找 `DISTR` 和 `arch` 的可能值，可以参考目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/，其中包含所有支持组合的 sysroots。

之后，在 Docker 容器内构建包就像调用以下命令一样简单：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，要为 Ubuntu Jammy 创建一个类似于 Manticore 核心团队提供的官方版本的包，您应在包含 Manticore Search 源代码的目录中执行以下命令。该目录是从 https://github.com/manticoresoftware/manticoresearch 克隆的仓库的根目录：

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
需要使用较长的源代码目录路径，否则在某些情况下（例如 Centos）可能无法构建源代码。

同样，您不仅可以为流行的 Linux 发行版构建二进制文件或包，还可以为 FreeBSD、Windows 和 macOS 构建。

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

生成 SRPM 后，您可以使用它来构建完整的二进制 RPM 包集：

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
> - 了解二进制文件的生成方式
> - 满足开源许可证合规性要求

## 手动构建

不使用构建 Docker 编译 Manticore **不推荐**，但如果您需要这样做，以下是您可能需要了解的内容：

### 所需工具

* C++ 编译器
  * 在 Linux 上 - 可使用 GNU（4.7.2 及以上）或 Clang
  * 在 Windows 上 - Microsoft Visual Studio 2019 及以上（社区版足够）
  * 在 macOS 上 - Clang（来自 XCode 的命令行工具，使用 `xcode-select --install` 安装）
* Bison、Flex - 在大多数系统上作为包提供，在 Windows 上可通过 cygwin 框架获得。
* Cmake - 所有平台通用（需要版本 3.19 或以上）

### 获取源码

#### 从 git

Manticore 源代码托管在 [GitHub](https://github.com/manticoresoftware/manticoresearch)。
要获取源代码，请克隆仓库，然后检出所需的分支或标签。`master` 分支代表主开发分支。发布时，会创建一个版本标签，如 `3.6.0`，并启动当前发布的新分支，在此例中为 `manticore-3.6.0`。版本分支的最新提交用于构建所有二进制发布。例如，要获取版本 3.6.0 的源码，可以运行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 从归档

您可以通过 GitHub 的“Download ZIP”按钮下载所需代码。`.zip` 和 `.tar.gz` 格式均适用。

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

CMake 会检测可用功能并据此配置构建。默认情况下，如果功能可用，则视为启用。脚本还会下载并构建一些外部库，假设您想使用它们。隐式地，您获得了最大数量功能的支持。

您也可以通过标志和选项显式配置构建。要启用功能 `FOO`，请在 CMake 调用中添加 `-DFOO=1`。
要禁用它，请使用 `-DFOO=0`。如果没有明确说明，启用不可用的功能（例如 MS Windows 构建上的 `WITH_GALERA`）将导致配置失败并报错。禁用功能，除了将其排除在构建之外，还会禁用对系统的检测，并禁用任何相关外部库的下载/构建。

#### 配置标志和选项

- **USE_SYSLOG** - 允许在[查询日志](../Logging/Query_logging.md)中使用 `syslog`。
- **WITH_GALERA** - 启用搜索守护进程上的复制支持。构建时将配置支持，并下载、构建 Galera 库的源代码，并将其包含在发行版/安装中。通常，构建时启用 Galera 是安全的，但不分发库本身（因此没有 Galera 模块，也没有复制）。但是，有时您可能需要显式禁用它，例如如果您想构建一个设计上不能加载任何库的静态二进制文件，以至于即使守护进程内部存在对 'dlopen' 函数的调用也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库进行构建。这对于像 [REGEX()](../Functions/String_functions.md#REGEX%28%29) 和 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 功能是必要的。
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 源代码，编译并静态链接它们，使最终的二进制文件不依赖系统中共享的 `RE2` 库。
- **WITH_STEMMER** - 使用 Snowball 词干提取库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 源代码，编译并静态链接它们，使最终的二进制文件不依赖系统中共享的 `libstemmer` 库。
- **WITH_ICU** - 使用 ICU（国际Unicode组件）库进行构建。它用于中文文本分词。当使用 morphology=`icu_chinese` 时会使用它。
- **WITH_JIEBA** - 使用 Jieba 中文文本分词工具进行构建。它用于中文文本分词。当使用 morphology=`jieba_chinese` 时会使用它。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 源代码，编译并静态链接它们，使最终的二进制文件不依赖系统中共享的 `icu` 库。还将 ICU 数据文件包含在安装/发行版中。静态链接 ICU 的目的是拥有已知版本的库，以确保行为确定且不依赖任何系统库。您很可能更愿意使用系统 ICU，因为它可能会随着时间更新，无需重新编译 Manticore 守护进程。在这种情况下，您需要显式禁用此选项。这也会节省 ICU 数据文件（约 30M）占用的空间，因为它不会包含在发行版中。
- **WITH_SSL** - 用于支持 HTTPS 以及与守护进程的加密 MySQL 连接。系统的 OpenSSL 库将链接到守护进程。这意味着启动守护进程时需要 OpenSSL。对于支持 HTTPS 是强制性的，但对服务器本身不是严格强制的（即无 SSL 意味着无法通过 HTTPS 连接，但其他协议仍然可用）。Manticore 可以使用从 1.0.2 到 1.1.1 版本的 SSL 库，但请注意 **为了安全起见，强烈建议使用最新的 SSL 库**。目前仅支持 v1.1.1，其他版本已过时（参见 [openssl 发布策略](https://www.openssl.org/policies/releasestrat.html)）。
- **WITH_ZLIB** - 索引器用于处理来自 MySQL 的压缩列。守护进程用于支持压缩的 MySQL 协议。
- **WITH_ODBC** - 索引器用于支持来自 ODBC 提供程序的索引源（通常是 UnixODBC 和 iODBC）。在 MS Windows 上，ODBC 是处理 MS SQL 源的正确方式，因此索引 `MSSQL` 也意味着启用此标志。
- **DL_ODBC** - 不链接 ODBC 库。如果链接了 ODBC，但不可用，即使您想处理与 ODBC 无关的内容，也无法启动索引器工具。此选项要求索引器仅在需要处理 ODBC 源时运行时加载库。
- **ODBC_LIB** - ODBC 库文件的名称。索引器在处理 ODBC 源时会尝试加载该文件。此选项会根据可用的 ODBC 共享库自动写入。您也可以通过在运行索引器之前设置环境变量 `ODBC_LIB` 并提供替代库的正确路径来覆盖该名称。
- **WITH_EXPAT** - 索引器用于支持索引 xmlpipe 源。
- **DL_EXPAT** - 不链接 EXPAT 库。如果链接了 EXPAT，但不可用，即使您想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 源时运行时加载库。
- **EXPAT_LIB** - EXPAT 库文件的名称。索引器在处理 xmlpipe 源时会尝试加载该文件。此选项会根据可用的 EXPAT 共享库自动写入。您也可以通过在运行索引器之前设置环境变量 EXPAT_LIB 并提供替代库的正确路径来覆盖该名称。
- **WITH_ICONV** - 支持索引器在索引 xmlpipe 源时使用不同编码。
- **DL_ICONV** - 不链接 iconv 库。如果链接了 iconv，但不可用，即使您想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 源时运行时加载库。
- **ICONV_LIB** - iconv 库文件的名称。当您想处理 xmlpipe 源时，索引器将尝试加载该文件。此选项是根据可用的 iconv 共享库自动写入的。您也可以在运行时通过提供环境变量 `ICONV_LIB` 并指定替代库的正确路径来覆盖该名称，然后再运行索引器。
- **WITH_MYSQL** - 索引器用来支持索引 MySQL 源。
- **DL_MYSQL** - 不与 MySQL 库链接。如果 MySQL 已链接但不可用，即使您想处理与 MySQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在您想处理 MySQL 源时才在运行时加载该库。
- **MYSQL_LIB** -- MySQL 库文件的名称。当您想处理 MySQL 源时，索引器将尝试加载该文件。此选项是根据可用的 MySQL 共享库自动写入的。您也可以在运行时通过提供环境变量 `MYSQL_LIB` 并指定替代库的正确路径来覆盖该名称，然后再运行索引器。
- **WITH_POSTGRESQL** - 索引器用来支持索引 PostgreSQL 源。
- **DL_POSTGRESQL** - 不与 PostgreSQL 库链接。如果 PostgreSQL 已链接但不可用，即使您想处理与 PostgreSQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在您想处理 PostgreSQL 源时才在运行时加载该库。
- **POSTGRESQL_LIB** - postgresql 库文件的名称。当处理 postgresql 源时，索引器将尝试加载指定的 postgresql 库文件。此选项是根据可用的 postgresql 共享库自动确定的。您也可以在运行时通过提供环境变量 `POSTGRESQL_LIB` 并指定替代库的正确路径来覆盖该名称，然后再运行索引器。
- **LOCALDATADIR** - 守护进程存储 binlog 的默认路径。如果在守护进程的运行时配置（即文件 `manticore.conf`，与此构建配置无关）中未提供该路径或显式禁用，binlog 将放置在此路径。它通常是绝对路径，但不要求必须是，亦可使用相对路径。您可能不需要更改配置定义的默认值，该默认值根据目标系统可能是 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data`。
- **FULL_SHARE_DIR** - 存储所有资源的默认路径。可以在启动任何使用该文件夹中文件的工具之前，通过环境变量 `FULL_SHARE_DIR` 覆盖此路径。这是一个重要路径，因为许多内容默认期望在此找到，包括预定义的字符集表、停用词、manticore 模块和 icu 数据文件，均放置在该文件夹中。配置脚本通常将此路径确定为类似 `/usr/share/manticore` 或 `/usr/local/share/manticore`。
- **DISTR_BUILD** - 用于发布包选项的快捷方式。它是一个字符串值，表示目标平台的名称。可以用它代替手动配置所有选项。在 Debian 和 Redhat Linux 上，默认值可能通过轻量级检测确定并设置为通用的 'Debian' 或 'RHEL'。否则，该值未定义。
- **PACK** - 更方便的快捷方式。它读取 `DISTR` 环境变量，将其赋值给 **DISTR_BUILD** 参数，然后按常规工作。这在准备好的构建系统（如 Docker 容器）中非常有用，其中 `DISTR` 变量在系统级别设置，反映容器所针对的目标系统。
- **CMAKE_INSTALL_PREFIX** (路径) - Manticore 预期安装的位置。构建不会执行任何安装，但会准备安装规则，这些规则在您运行 `cmake --install` 命令或创建包然后安装时执行。该前缀可以随时更改，即使在安装期间，也可以通过调用
  `cmake --install . --prefix /path/to/installation` 来更改。然而，在配置时，此变量用于初始化 `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。例如，在配置时将其设置为 `/my/custom` 会将 `LOCALDATADIR` 硬编码为 `/my/custom/var/lib/manticore/data`，将 `FULL_SHARE_DIR` 硬编码为
  `/my/custom/usr/share/manticore`。
- **BUILD_TESTING** (布尔值) 是否支持测试。如果启用，构建完成后，您可以运行 'ctest' 来测试构建。请注意，测试意味着额外的依赖项，如至少需要 PHP cli、Python 和可用的带测试数据库的 MySQL 服务器。默认情况下，此参数为开启状态。因此，对于“仅构建”，您可能希望通过显式指定 'off' 来禁用该选项。
- **BUILD_SRPMS** (布尔值) 是否显示构建源 RPM（SRPM）的说明。由于 CPack 在基于组件的打包方面的限制，SRPM 不能直接与二进制 RPM 一起生成。启用时，构建系统将显示使用源配置方法正确生成 SRPM 的说明。默认情况下，此参数为关闭。
- **LIBS_BUNDLE** - 包含不同库的文件夹路径。这主要与 Windows 构建相关，但如果您需要频繁构建以避免每次都下载第三方源，也可能有用。默认情况下，配置脚本不会修改此路径；您应手动将所有内容放入其中。例如，当我们想支持一个词干提取器时，源代码将从 Snowball 主页下载，然后解压、配置、构建等。相反，您可以将原始源代码压缩包（即 `libstemmer_c.tgz`）存放在此文件夹中。下次想从头构建时，配置脚本将首先在该包中查找，如果找到词干提取器，就不会再从互联网下载。
- **CACHEB** - 存储第三方库构建的文件夹路径。通常像 galera、re2、icu 等功能，首先从网络下载或从捆绑包中获取，然后解包、构建并安装到一个临时的内部文件夹中。在构建 manticore 时，该文件夹被用作支持所需功能的资源所在位置。最终，这些库要么与 manticore 链接（如果它是一个库）；要么直接进入分发/安装（如 galera 或 icu 数据）。当 **CACHEB** 被定义为 cmake 配置参数或系统环境变量时，它被用作这些构建的目标文件夹。该文件夹可能会在多次构建间保留，因此存储在其中的库不会被重新构建，从而大大缩短整个构建过程。
注意，有些选项是成三元组组织的：`WITH_XXX`、`DL_XXX` 和 `XXX_LIB` —— 比如对 mysql、odbc 等的支持。`WITH_XXX` 决定后两个选项是否生效。即，如果你将 `WITH_ODBC` 设置为 `0`，那么提供 `DL_ODBC` 和 `ODBC_LIB` 就没有意义，这两个选项在整个功能被禁用时不会生效。此外，`XXX_LIB` 在没有 `DL_XXX` 的情况下也没有意义，因为如果不使用 `DL_XXX` 选项，则不会使用动态加载，`XXX_LIB` 提供的名称也就无用。这是默认的自省机制所使用的。
另外，使用 `iconv` 库假设依赖 `expat`，如果后者被禁用，则前者无效。
还有一些库可能总是可用，因此没有必要避免与它们链接。例如，在 Windows 上是 ODBC；在 macOS 上是 Expat、iconv 以及可能的其他库。默认的自省机制会识别这些库，并仅为它们生成 `WITH_XXX`，而不生成 `DL_XXX` 和 `XXX_LIB`，这使得配置更简单。
使用某些选项时，配置过程可能如下所示：


mkdir build && cd build

cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..

除了通用配置值外，你还可以查看配置后保留在构建文件夹中的 `CMakeCache.txt` 文件。任何在该文件中定义的值都可以在运行 cmake 时显式重新定义。例如，你可以运行 `cmake -DHAVE_GETADDRINFO_A=FALSE ...`，这次配置将不会使用该变量的已有值，而是使用你提供的值。

#### 特定环境变量

```bash
环境变量用于提供某种全局设置，这些设置存储在构建配置之外且始终存在。为了持久化，它们可以通过多种方式在系统中全局设置——比如添加到 `.bashrc` 文件，或者如果你使用基于 Docker 的构建系统，则嵌入到 Dockerfile 中，或者在 Windows 的系统偏好环境变量中设置。你也可以在 shell 中使用 `export VAR=value` 设置短期变量。甚至更短暂的方式是，在调用 cmake 时前置变量赋值，如 `CACHEB=/my/cache cmake ...` ——这样变量只在这次调用中生效，下一次调用不可见。
其中一些变量是 cmake 和其他工具通用的，比如 `CXX` 用于指定当前 C++ 编译器，`CXX_FLAGS` 用于提供编译器标志等。
```

然而，我们有一些专门针对 manticore 配置的变量，这些变量是专门为我们的构建设计的。

- **CACHEB** - 与配置中的 **CACHEB** 选项相同

- **LIBS_BUNDLE** - 与配置中的 **LIBS_BUNDLE** 选项相同

- **DISTR** - 当使用 `-DPACK=1` 时，用于初始化 `DISTR_BUILD` 选项。

- **DIAGNOSTIC** - 使 cmake 配置输出更详细，解释所有发生的事情

- **WRITEB** - 假设 **LIBS_BUNDLE** 已设置，如果启用，将下载不同工具的源代码归档文件到 LIBS_BUNDLE 文件夹。也就是说，如果出现了新的 stemmer 版本，你可以手动删除 bundle 中的 libstemmer_c.tgz，然后运行一次性命令 `WRITEB=1 cmake ...` ——它会在 bundle 中找不到 stemmer 源码，然后从供应商网站下载到 bundle 中（如果不使用 WRITEB，则会下载到构建中的临时文件夹，构建文件夹清理后会消失）。
配置结束时，你可以看到一个可用功能列表，如下所示：
-- Enabled features compiled in:
* Galera, replication of tables
* re2, a regular expression library

* stemmer, stemming library (Snowball)

```
* icu, International Components for Unicode
* OpenSSL, for encrypted networking
* ZLIB, for compressed data and networking
* ODBC, for indexing MSSQL (windows) and generic ODBC sources with indexer
* EXPAT, for indexing xmlpipe sources with indexer
* Iconv, for support of different encodings when indexing xmlpipe sources with indexer
* MySQL, for indexing MySQL sources with indexer
* PostgreSQL, for indexing PostgreSQL sources with indexer
### 构建
cmake --build . --config RelWithDebInfo
### 安装
运行以下命令进行安装：
```

cmake --install . --config RelWithDebInfo

```bash
如果要安装到自定义（非默认）文件夹，运行
```

cmake --install . --prefix path/to/build --config RelWithDebInfo

### 构建包

```bash
要构建包，使用目标 `package`。它将根据 `-DDISTR_BUILD` 选项的选择构建包。默认情况下，它是一个包含所有二进制文件和辅助文件的简单 .zip 或 .tgz 归档。
```

cmake --build . --target package --config RelWithDebInfo

```bash
## 关于构建的一些高级内容
```

### 单配置下的重新编译（更新）

如果你没有更改源码和构建路径，只需进入你的构建文件夹并运行：

```bash
cmake .
```

cmake --build . --clean-first --config RelWithDebInfo

如果由于某种原因这不起作用，你可以删除构建文件夹中的 `CMakeCache.txt` 文件。完成此步骤后，你必须再次运行 cmake，指向源码文件夹并配置选项。

如果这仍然无效，只需清空你的构建文件夹并从头开始。

```bash
### 构建类型
简而言之——只需使用上文所述的 `--config RelWithDebInfo`。这样不会出错。
```

我们使用两种构建类型。开发时使用 `Debug` —— 它为优化和其他方面分配编译器标志，使调试运行支持逐步执行。然而，生成的二进制文件较大且运行较慢，不适合生产环境。
发布时使用另一种类型——`RelWithDebInfo` —— 意味着“带调试信息的发布构建”。它生成带有嵌入调试信息的生产二进制文件。调试信息随后被拆分到单独的调试信息包中，这些包与发布包一起存储，可用于崩溃等问题的调查和修复。Cmake 还提供 `Release` 和 `MinSizeRel`，但我们不使用它们。如果构建类型不可用，cmake 会生成一个 `noconfig` 构建。

#### 构建系统生成器

生成器分为两种类型：单配置和多配置。

- 单配置需要在配置期间通过 `CMAKE_BUILD_TYPE` 参数提供构建类型。如果未定义，构建将回退到 `RelWithDebInfo` 类型，这适用于仅从源代码构建 Manticore 而不参与开发的情况。对于显式构建，您应提供构建类型，例如 `-DCMAKE_BUILD_TYPE=Debug`。

- 多配置在构建期间选择构建类型。应通过 `--config` 选项提供，否则将构建一种 `noconfig`，这是不理想的。因此，您应始终指定构建类型，例如 `--config Debug`。

如果您想指定构建类型，但不想关心它是“单配置”还是“多配置”生成器——只需在两个地方都提供必要的键。即，使用 `-DCMAKE_BUILD_TYPE=Debug` 进行配置，然后使用 `--config Debug` 进行构建。只要确保两个值相同即可。如果目标构建器是单配置，它将使用配置参数。如果是多配置，配置参数将被忽略，但正确的构建配置将由 `--config` 键选择。

如果您想要 `RelWithDebInfo`（即仅为生产构建）并且知道您处于单配置平台（除 Windows 外的所有平台）——您可以在 cmake 调用时省略 `--config` 标志。默认的 `CMAKE_BUILD_TYPE=RelWithDebInfo` 将被配置并使用。所有“构建”、“安装”和“构建包”的命令将因此变得更简短。

#### 显式选择构建系统生成器

Cmake 是一个本身不执行构建的工具，但它为本地构建系统生成规则。
通常，它能很好地确定可用的构建系统，但有时您可能需要显式提供生成器。您可以运行 `cmake -G` 并查看可用生成器列表。

- 在 Windows 上，如果您安装了多个版本的 Visual Studio，您可能需要指定使用哪一个，如：

cmake -G "Visual Studio 16 2019" ....

- On all other platforms - usually Unix Makefiles are used, but you can specify another one, such as Ninja, or Ninja Multi-Config, as:

  Multi-Config`, as:
  cmake -GNinja ...
  或者

  cmake -G"Ninja Multi-Config" ...
Ninja 多配置非常有用，因为它是真正的“多配置”，并且在 Linux/macOS/BSD 上可用。使用此生成器，您可以将配置类型的选择推迟到构建时，并且可以在同一个构建文件夹中构建多个配置，只需更改 `--config` 参数。
```bash
### 注意事项
  ```
1. 如果您最终想构建一个功能完整的 RPM 包，构建目录的路径必须足够长，以便正确构建调试符号。
例如 `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`。这是因为 RPM 工具在构建调试信息时会修改编译二进制文件的路径，它可能会覆盖现有空间而不会分配更多空间。上述长路径有 100 个字符，这对于这种情况来说足够了。
```bash
## 外部依赖
  ```
如果您想使用某些库，它们应该是可用的。
```bash
- 用于索引（`indexer` 工具）：`expat`、`iconv`、`mysql`、`odbc`、`postgresql`。没有它们，您只能处理 `tsv` 和 `csv` 源。
```
- 用于服务查询（`searchd` 守护进程）：可能需要 `openssl`。

- 对于所有（必需的，强制性的！）我们需要 Boost 库。最低版本是 1.61.0，然而，我们使用更新的 1.75.0 版本构建二进制文件。更高版本（如 1.76）也应该可以。在 Windows 上，您可以从其网站（boost.org）下载预编译的 Boost 并安装到默认建议路径（即 `C:\\boost...`）。在 MacOS 上，brew 提供的版本是可以的。在 Linux 上，您可以检查官方仓库中的可用版本，如果不符合要求，可以从源代码构建。我们需要组件 'context'，如果您还想从源代码构建 Galera 库，也可以构建组件 'system' 和 'program_options'。请查看 `dist/build_dockers/xxx/boost_175/Dockerfile`，那里有一个简短的自述脚本/说明，介绍如何操作。

在构建系统上，您需要安装这些包的 'dev' 或 'devel' 版本（例如 libmysqlclient-devel、unixodbc-devel 等。具体包名请查看我们的 dockerfiles）。
在运行系统上，这些包至少应以最终（非开发）版本存在。（devel 版本通常更大，因为它们不仅包含目标二进制文件，还包括不同的开发内容，如头文件等）。

### 在 Windows 上构建

除了必要的前置条件外，您可能需要预编译的 `expat`、`iconv`、`mysql` 和 `postgresql` 客户端库。您必须自己构建它们，或者联系我们获取我们的构建包（一个简单的 zip 压缩包，其中包含这些目标的文件夹）。
- ODBC 不是必需的，因为它是系统库。
- OpenSSL 可以从源代码构建，或者从 https://slproweb.com/products/Win32OpenSSL.html 下载预编译版本（如 cmake 内部脚本 FindOpenSSL 中所述）。
- Boost 可以从 https://www.boost.org/ 下载预编译版本。

### 查看已编译内容

运行 `indexer -h`。它将显示哪些功能已配置和构建（无论是显式的还是自动检测的，都无关紧要）：

Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1

-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data

-DFULL_SHARE_DIR=/usr/share/manticore

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

