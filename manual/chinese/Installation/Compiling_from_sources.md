# 从源代码编译 Manticore

从源代码编译 Manticore Search 可以实现自定义构建配置，例如禁用某些功能或添加新补丁以进行测试。例如，您可能希望从源代码编译并禁用内嵌的 ICU，以便使用系统中安装的可以独立升级的不同版本。这对于有意为 Manticore Search 项目贡献代码的人也很有用。

## 使用 CI Docker 构建
为了准备[官方发布及开发包](https://repo.manticoresearch.com/)，我们使用 Docker 及特殊的构建镜像。该镜像包含必要的工具，设计用于外部 sysroot，因此一个容器可以为所有操作系统构建包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 构建该镜像，或使用 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 上的镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，还需要指定以下环境变量：

* `DISTR`：目标平台：`bionic`，`focal`，`jammy`，`buster`，`bullseye`，`bookworm`，`rhel7`，`rhel8`，`rhel9`，`rhel10`，`macos`，`windows`，`freebsd13`
* `arch`：架构：`x86_64`，`x64`（Windows 用），`aarch64`，`arm64`（Macos 用）
* `SYSROOT_URL`：系统根目录归档的 URL。您可以使用 https://repo.manticoresearch.com/repository/sysroots，除非您自己构建 sysroots（构建说明见[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)）。
* 使用 CI 工作流程文件作为参考，查找您可能需要的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

要查找 `DISTR` 和 `arch` 的可能值，可以参考目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/，该目录包含所有支持组合的 sysroots。

之后，在 Docker 容器内构建包只需执行：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，要为 Ubuntu Jammy 创建一个类似于 Manticore Core Team 提供的官方版本的包，应在包含 Manticore Search 源码的目录中执行以下命令。该目录为从 https://github.com/manticoresoftware/manticoresearch 克隆的仓库根目录：

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
要求使用较长的源代码目录路径，某些情况下（如 Centos）否则可能无法构建。

同样，您可以构建二进制文件或包，不仅支持主流 Linux 发行版，还包括 FreeBSD、Windows 以及 macOS。

#### 使用 Docker 构建 SRPM

您也可以使用相同的特殊 docker 镜像构建 SRPM：

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

这将生成包含所有源代码的源 RPM (`.src.rpm` 文件)。

#### 从 SRPM 构建二进制 RPM

生成 SRPM 后，您可以用它构建完整的二进制 RPM 包集：

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

> 注意：**从 SRPM 构建 RPM 需要确保 SRPM 列出的所有依赖都已完全安装，这可能比较复杂。** SRPM 仍然有用处：
> - 审核构建过程或检查源代码和 spec 文件
> - 对构建进行自定义修改或打补丁
> - 了解二进制文件的生成过程
> - 满足开源许可证合规要求

## 手动构建

不推荐不使用 Docker 构建 Manticore，但如果您需要手动构建，以下是您可能需要了解的内容：

### 必需工具

* C++ 编译器
  * Linux 下可使用 GNU (4.7.2 及以上) 或 Clang
  * Windows 下使用 Microsoft Visual Studio 2019 及以上版本（社区版即可）
  * macOS 上使用 Clang（XCode 命令行工具，使用 `xcode-select --install` 安装）。
* Bison，Flex - 大多数系统有对应软件包，Windows 下可在 cygwin 框架中获取。
* Cmake - 所有平台均使用（需版本 3.19 及以上）

### 获取源码

#### 从 git

Manticore 源代码托管于 [GitHub](https://github.com/manticoresoftware/manticoresearch)。
获取源代码方法为克隆仓库，然后检出所需的分支或标签。`master` 分支为主开发分支。发布时会创建版本标签，如 `3.6.0`，并基于该版本新建发布分支，例如 `manticore-3.6.0`。经过所有更改后的版本分支最新代码作为所有二进制发布的源码。例如，要获取版本 3.6.0 的源码，可以执行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 从归档文件

您也可以通过 GitHub 上的“Download ZIP”按钮下载所需代码。`.zip` 和 `.tar.gz` 格式均可。

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### 配置

Manticore 使用 CMake。假设您在克隆的仓库根目录下：

```bash
mkdir build && cd build
cmake ..
```

CMake 会检测可用功能并据此配置构建。默认情况下，所有存在的功能均视为启用。脚本还会下载并构建部分外部库，假设您希望使用它们。隐式地，您会获得最多功能支持。

您也可以用标志和选项显式配置构建。要启用功能 `FOO`，在 CMake 调用中添加 `-DFOO=1`。
要禁用它，请使用 `-DFOO=0`。如果没有明确说明，启用一个不可用的功能((例如在 MS Windows 构建中使用 `WITH_GALERA`))将导致配置失败并报错。禁用一个功能，除了从构建中排除它，还会禁用在系统上的检测，并禁用任何相关外部库的下载/构建。

#### 配置标志和选项

- **USE_SYSLOG** - 允许在 [query logging](../Logging/Query_logging.md) 中使用`syslog`。
- **WITH_GALERA** -启用对搜索守护进程复制的支持。构建时将配置支持，并且 Galera 库的源码将被下载、构建并包含在发行/安装包中。通常，使用 Galera 构建是安全的，但不分发该库本身（因此没有 Galera 模块，也没有复制功能）。但是，有时你可能需要显式禁用它，例如当你想构建一个按设计无法加载任何库的静态二进制时，即使守护进程内部存在对 'dlopen' 函数的调用，也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库进行构建。对于诸如 [REGEX()](../Functions/String_functions.md#REGEX%28%29) 等函数，以及 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)，这是必要的。
  功能。
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 源码，编译并静态链接，从而使最终二进制文件不依赖于系统中共享  `RE2` 库的存在。
- **WITH_STEMMER** - 使用 Snowball 词干提取库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 源码，编译并静态链接，从而使最终二进制文件不依赖于系统中共享  `libstemmer` 库的存在。
- **WITH_ICU** -  使用 ICU (International Components for Unicode) 库进行构建。它用于对中文文本进行分词。当使用 morphology=`icu_chinese` 时会使用该库。
- **WITH_JIEBA** -  使用 Jieba 中文分词工具进行构建。它用于对中文文本进行分词。当使用 morphology=`jieba_chinese` 时会使用该工具。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 源码，编译并静态链接，从而使最终二进制文件不依赖于系统中共享 `icu` 库的存在。同时将 ICU 数据文件包含到安装/发行包中。静态链接 ICU 的目的是使用一个已知版本的库，以便行为确定且不依赖任何系统库。你更可能偏好使用系统 ICU，因为它可以随着时间更新而无需重新编译 Manticore 守护进程。在这种情况下，你需要显式禁用此选项。这也将节省由 ICU 数据文件占用的一些空间 (约 30M)，因为它不会包含在发行包中。
- **WITH_SSL** - 用于支持 HTTPS 以及加密的 MySQL 与守护进程之间的连接。系统的 OpenSSL 库将链接到守护进程。这意味着启动守护进程时需要 OpenSSL。对于 HTTPS 支持这是必需的，但对于服务器并非严格必需（即没有 SSL 则无法通过 HTTPS 连接，但其他协议仍可工作）。Manticore 可以使用从 1.0.2 到 1.1.1 的 SSL 库版本，然而请注意 **为安全起见，强烈建议使用最新的 SSL
  库**。目前仅支持 v1.1.1，其余版本已过时 (
  请参阅 [openssl release strategy](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - 索引器使用它来处理来自 MySQL 的压缩列。守护进程使用它来支持压缩的 MySQL 协议。
- **WITH_ODBC** - 索引器使用它来支持来自 ODBC 提供程序的源的索引（通常是 UnixODBC 和 iODBC）。在 MS Windows 上，ODBC 是处理 MS SQL 源的正确方式，因此对 `MSSQL` 的索引也意味着该标志被启用。
- **DL_ODBC** - 不与 ODBC 库链接。如果已经链接了 ODBC，但不可用，即使想处理与 ODBC 无关的内容，也无法启动 indexer 工具。此选项要求索引器仅在需要处理 ODBC 源时在运行时加载该库。
- **ODBC_LIB** - ODBC 库文件的名称。索引器在处理 ODBC 源时将尝试加载该文件。此选项通过可用的 ODBC 共享库检测自动生成。您也可以在运行索引器之前，通过环境变量 `ODBC_LIB` 提供替代库的正确路径来覆盖该名称。
- **WITH_EXPAT** - 索引器使用它来支持索引 xmlpipe 源。
- **DL_EXPAT** - 不与 EXPAT 库链接。如果已链接 EXPAT，但不可用，即使想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 源时在运行时加载该库。
- **EXPAT_LIB** - EXPAT 库文件的名称。索引器在处理 xmlpipe 源时将尝试加载该文件。此选项通过可用的 EXPAT 共享库检测自动生成。您也可以在运行索引器之前，通过环境变量 EXPAT_LIB 提供替代库的正确路径来覆盖该名称。
- **WITH_ICONV** - 在使用索引器索引 xmlpipe 源时，支持不同编码。
- **DL_ICONV** - 不与 iconv 库链接。如果已链接 iconv，但不可用，即使想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 源时在运行时加载该库。
- **ICONV_LIB** - iconv 库文件的名称。索引器在处理 xmlpipe 源时会尝试加载该文件。此选项是根据可用的 iconv 共享库自动写入的。您也可以在运行时通过在运行索引器之前提供环境变量 `ICONV_LIB`，指定替代库的正确路径来覆盖该名称。
- **WITH_MYSQL** - 索引器用来支持索引 MySQL 源。
- **DL_MYSQL** - 不链接 MySQL 库。如果 MySQL 已链接但不可用，即使您想处理与 MySQL 无关的内容，也无法启动`indexer`工具。此选项要求索引器仅在处理 MySQL 源时才在运行时加载库。
- **MYSQL_LIB** -- MySQL 库文件的名称。索引器在处理 MySQL 源时会尝试加载该文件。此选项是根据可用的 MySQL 共享库自动写入的。您也可以在运行时通过在运行索引器之前提供环境变量 `MYSQL_LIB`，指定替代库的正确路径来覆盖该名称。
- **WITH_POSTGRESQL** - 索引器用来支持索引 PostgreSQL 源。
- **DL_POSTGRESQL** - 不链接 PostgreSQL 库。如果 PostgreSQL 已链接但不可用，即使您想处理与 PostgreSQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在处理 PostgreSQL 源时才在运行时加载库。
- **POSTGRESQL_LIB** - postgresql 库文件的名称。索引器在处理 postgresql 源时会尝试加载指定的 postgresql 库文件。此选项是根据可用的 postgresql 共享库自动确定的。您也可以在运行时通过在运行索引器之前提供环境变量 `POSTGRESQL_LIB`，指定替代库的正确路径来覆盖该名称。
- **LOCALDATADIR** - 守护进程存储二进制日志（binlogs）的默认路径。如果守护进程运行时配置（即文件 `manticore.conf`，与此构建配置无关）未提供或显式禁用该路径，二进制日志将放置在此路径。它通常是绝对路径，但不要求必须是，亦可使用相对路径。根据目标系统，您可能不需要更改配置中定义的默认值，该值可能是 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data` 之类的路径。
- **FULL_SHARE_DIR** - 存储所有资源的默认路径。可在启动任何使用该文件夹中文件的工具前，通过环境变量 `FULL_SHARE_DIR` 覆盖该路径。这是一个重要的路径，因为默认期望许多内容位于此处，包括预定义的字符集表、停用词、manticore 模块和 icu 数据文件，均放在该文件夹。配置脚本通常将此路径确定为类似 `/usr/share/manticore` 或 `/usr/local/share/manticore`。
- **DISTR_BUILD** - 用于发布包选项的快捷方式。它是包含目标平台名称的字符串值。可以用它来替代手动配置所有选项。在 Debian 和 Redhat Linux 上，默认值可能通过轻量的检测确定，并设置为通用的 'Debian' 或 'RHEL'。否则，该值未定义。
- **PACK** - 一个更方便的快捷方式。它读取 `DISTR` 环境变量，将其赋值给 **DISTR_BUILD** 参数，然后正常工作。这在准备好的构建系统（如 Docker 容器）中非常有用，其中 `DISTR` 变量在系统级别设置，反映容器的目标系统。
- **CMAKE_INSTALL_PREFIX** (路径) - Manticore 预期安装的位置。构建不执行任何安装，但会准备安装规则，当您运行 `cmake --install` 命令或创建并安装包时执行。该前缀可以随时更改，甚至在安装过程中也可以通过执行
  `cmake --install . --prefix /path/to/installation` 来更改。但在配置时，此变量用于初始化 `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。例如，在配置时将其设置为 `/my/custom`
  会将 `LOCALDATADIR` 硬编码为 `/my/custom/var/lib/manticore/data`，`FULL_SHARE_DIR` 硬编码为
  `/my/custom/usr/share/manticore`。
- **BUILD_TESTING** (布尔值) 是否支持测试。启用后，构建完成后可运行 'ctest' 来测试构建。测试会引入额外依赖，如至少需要 PHP cli、Python 和带测试数据库的可用 MySQL 服务器。默认此参数为开启。所以如果只想“构建”，可能需要通过明确指定 'off' 来禁用该选项。
- **BUILD_SRPMS** (布尔值) 是否显示构建源码 RPM（SRPM）的指令。由于 CPack 不支持基于组件的打包，无法直接同时生成 SRPM 和二进制 RPM。启用后，构建系统将显示使用源码配置方式生成正确 SRPM 的指令。默认此参数关闭。
- **LIBS_BUNDLE** - 存放各种库的文件夹路径。这主要与 Windows 构建相关，也可能在需要频繁构建时有用，以避免每次都下载第三方源码。默认情况下，配置脚本不会修改此路径；您需要手动放置所有内容。例如，当需要支持 stemmer 时，源码会从 Snowball 主页下载、解压、配置、构建等。相反，您可以将原始源码压缩包（即 `libstemmer_c.tgz`）存放在此文件夹中。下次从头构建时，配置脚本会先在 bundle 中查找，如果找到 stemmer 就不会再从互联网下载。
- **CACHEB** - 指向一个存储第三方库构建的文件夹的路径。通常，像 galera、re2、icu 等功能首先会被下载或从包中获取，然后解压、构建并安装到一个临时的内部文件夹中。在构建 manticore 时，该文件夹被用作支持所需功能的对象的存放地。最后，它们要么与 manticore 链接，若它是一个库；要么直接分发/安装（比如 galera 或 icu 数据）。当 **CACHEB** 定义为 cmake 配置参数或系统环境变量时，它被用作这些构建的目标文件夹。这个文件夹可能会在构建之间保留，这样储存在那里的库将不再被重建，从而大大缩短整个构建过程。


注意，有些选项以三重形式组织：`WITH_XXX`、`DL_XXX` 和 `XXX_LIB` - 如 mysql、odbc 等的支持。`WITH_XXX` 决定接下来的两个选项是否有效。即，如果你将 `WITH_ODBC` 设置为 `0` - 那么提供 `DL_ODBC` 和 `ODBC_LIB` 就毫无意义，当整个功能被禁用时，这两个选项将不起作用。而 `XXX_LIB` 没有 `DL_XXX` 是没有意义的，因为如果你不想要 `DL_XXX` 选项，将不会使用动态加载，并且 `XXX_LIB` 提供的名称是无用的。这在默认内省中使用。

此外，使用 `iconv` 库假定 `expat` 并且如果后者被禁用是没有意义的。

另外，一些库可能总是可用的，因此，没有理由避免与它们的链接。例如，在 Windows 上是 ODBC。在 macOS 上是 Expat、iconv 和可能还有其他库。默认内省确定这样的库，并有效地仅为它们发出 `WITH_XXX`，没有 `DL_XXX` 和 `XXX_LIB`，这使得事情变得更简单。

配置一些选项时可能会这样的展示：

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

除了常规配置值，你还可以查看运行配置后在构建文件夹中留下的 `CMakeCache.txt` 文件。当运行 cmake 时，任何定义的值都可以显式地重新定义。例如，你可以运行 `cmake -DHAVE_GETADDRINFO_A=FALSE ...`，该配置运行不会假设该变量的调查值，而将使用你提供的值。

#### 特定环境变量

环境变量对于提供存储在构建配置之外并始终存在的某种全局设置是有用的。为了持久化，它们可以通过不同的方式在系统上全局设置 - 比如将它们添加到 `.bashrc` 文件中、如果你生产基于 docker 的构建系统则将它们嵌入 Dockerfile 中，或者在 Windows 上将它们写入系统首选项环境变量中。此外，你可以在 shell 中使用 `export VAR=value` 设置它们为短期存在的。或者更简短，通过在 cmake 调用前加上变量值，如 `CACHEB=/my/cache cmake ...` - 以这种方式它仅在本次调用中工作，并且在下次调用中将不可见。

其中一些变量通常被 cmake 和其他工具使用。比如 `CXX` 确定当前的 C++ 编译器，或者 `CXX_FLAGS` 提供编译器标志等。

然而，我们有一些变量是特定于 manticore 配置的，它们是专门为我们的构建发明的。

- **CACHEB** - 与配置选项 **CACHEB** 相同
- **LIBS_BUNDLE** - 与配置选项 **LIBS_BUNDLE** 相同
- **DISTR** - 当使用 `-DPACK=1` 时用于初始化 `DISTR_BUILD` 选项。
- **DIAGNOSTIC** - 使 cmake 配置的输出更详细，解释所有发生的情况
- **WRITEB** - 假定 **LIBS_BUNDLE** 并且如果设置，将下载不同工具的源代码压缩包到 LIBS_BUNDLE 文件夹中。也就是说，如果有新版本的分词器发布 - 你可以手动从包中删除 libstemmer_c.tgz，然后运行一次性 `WRITEB=1 cmake ...` - 它将在包中找不到分词器的源代码，然后从供应商网站下载到包中（没有 WRITEB 会下载到构建中的临时文件夹，当你清除构建文件夹时它会消失）。

在配置结束时，你可能会看到可用的以及将使用的列表如下所示：

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

要安装到自定义（非默认）文件夹，运行

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### 构建包

要构建包，使用目标 `package`。它将根据 `-DDISTR_BUILD` 选项提供的选择构建包。默认情况下，它将是一个包含所有二进制文件和辅助文件的简单 .zip 或 .tgz 压缩包。

```bash
cmake --build . --target package --config RelWithDebInfo
```

## 一些关于构建的高级事项

### 单配置下的重新编译（更新）

如果你没有改变源代码和构建的路径，只需移动到构建文件夹并运行：

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

如果由于某种原因它不起作用，你可以删除位于构建文件夹中的 `CMakeCache.txt` 文件。在这一步之后你
必须再次运行 cmake，指向源文件夹并配置选项。

如果它仍然没有帮助，只需清除构建文件夹并从头开始。

### 构建类型

简而言之 - 就像上面写的那样使用 `--config RelWithDebInfo`。这样做不会出错。

我们使用两种构建类型。对于开发，它是 `Debug` - 它以一种对开发非常友好的方式分配编译器优化和其他的标志，这意味着调试运行具有逐步执行功能。然而，生成的二进制文件对于生产来说相当大且慢。

对于发布，我们使用另一种类型 - `RelWithDebInfo` - 这意味着“带有调试信息的发布构建”。它生成带有嵌入式调试信息的生产二进制文件。后者随后分离到单独的调试信息包中与发布包一起存储，并可能在出现一些问题如崩溃时使用 - 用于调查和修复错误。Cmake 还提供`Release` 和 `MinSizeRel`，但我们不使用它们。如果构建类型不可用，cmake 将进行 `noconfig` 构建。

#### 构建系统生成器

生成器有两种类型：单配置和多配置。

- 单配置需要在配置期间提供构建类型，通过 `CMAKE_BUILD_TYPE` 参数。如果未定义，构建将回退到 `RelWithDebInfo` 类型，这适合于您只想从源代码构建 Manticore，而不参与开发。对于显式构建，您应该提供构建类型，例如 `-DCMAKE_BUILD_TYPE=Debug`。
- 多配置在构建期间选择构建类型。它应该通过 `--config` 选项提供，否则将构建一种 `noconfig`，这是不理想的。因此，您应该始终指定构建类型，例如 `--config Debug`。

如果您想指定构建类型，但不想关心它是“单”配置还是“多”配置生成器 - 只需在两个地方提供必要的键。即，用 `-DCMAKE_BUILD_TYPE=Debug` 配置，然后用 `--config Debug` 构建。只需确保两个值相同。如果目标生成器是单配置，它将使用配置参数。如果它是多配置，配置参数将被忽略，但正确的构建配置将通过 `--config` 键选择。

如果您想要 `RelWithDebInfo`（即仅为生产构建），并且知道您在一个单配置平台上（即所有平台，除非 Windows） - 您可以在 cmake 调用时省略 `--config` 标志。然后将配置默认的 `CMAKE_BUILD_TYPE=RelWithDebInfo`，并将其使用。所有的“构建”、“安装”和“构建包”的命令将变得更短。

#### 显式选择构建系统生成器

Cmake 是一个不会自己进行构建的工具，但它会生成本地构建系统的规则。
通常，它很好的确定可用的构建系统，但有时您可能需要显式提供生成器。您
可以运行 `cmake -G` 并查看可用生成器的列表。

- 在 Windows 上，如果您安装了多个版本的 Visual Studio，您可能需要指定要使用的版本，
如：
```bash
cmake -G "Visual Studio 16 2019" ....
  ```
- On all other platforms - usually Unix Makefiles are used, but you can specify another one, such as Ninja, or Ninja Multi-Config, as:
  Multi-Config`, as:
```bash
  cmake -GNinja ...
  ```
  或
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja 多配置非常有用，因为它确实是“多配置”，可用于 Linux/macOS/BSD。使用此生成器，您可以将选择配置类型的操作转移到构建时，并且您可以在同一个构建文件夹中构建多个配置，只需更改 `--config` 参数。

### 注意事项

1. 如果您想最终构建一个全功能的 RPM 包，构建目录的路径必须足够长，以便正确构建调试符号。
例如 `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`。这是因为 RPM 工具在构建调试信息时会修改编译后二进制文件的路径，它可能会覆盖现有空间而不分配更多。上述长路径有 100 个字符，对于这种情况是相当足够的。

## 外部依赖

一些库如果您想使用它们则必须可用。
- 对于索引（`indexer` 工具）：`expat`、`iconv`、`mysql`、`odbc`、`postgresql`。没有它们，您只能处理 `tsv` 和 `csv` 源。
- 对于处理查询（`searchd` 守护进程）：可能需要 `openssl`。
- 对于所有（必需，强制！）我们需要 Boost 库。最小版本是 1.61.0，然而，我们用更新的版本 1.75.0 构建二进制文件。即使是更近期的版本（如 1.76）也应该可以。在 Windows 上，您可以从他们的网站（boost.org）下载预构建的 Boost，并安装到默认建议的路径中（即 `C:\\boost...`）。在 MacOs 上，brew 提供的版本可以。对于 Linux，您可以检查官方仓库中可用的版本，如果不符合要求，您可以从源代码构建。我们需要组件 'context'，您还可以构建组件 'system' 和 'program_options'，如果您还想从源代码构建 Galera 库，它们将是必要的。有关如何执行此操作的简短自文档脚本/说明，请查看 `dist/build_dockers/xxx/boost_175/Dockerfile`。

在构建系统上，您需要安装这些包的“开发”或“devel”版本（即 - libmysqlclient-devel、unixodbc-devel 等。请查看我们的 dockerfile 以获取具体包的名称）。

在运行系统上，这些包至少应在最终（非开发）变体中存在。（开发变体通常更大，因为它们不仅包含目标二进制文件，还包括不同的开发内容，如包含头文件等）。

### 在 Windows 上构建

除了必要的前提条件外，您可能需要预构建的 `expat`、`iconv`、`mysql` 和 `postgresql` 客户端库。您必须自己构建它们或联系我们获取我们的构建包（一个简单的 zip 存档，里面包含这些目标的文件夹）。

- ODBC 不是必需的，因为它是系统库。
- OpenSSL 可以从源代码构建，或从 https://slproweb.com/products/Win32OpenSSL.html 下载预构建版本（如 cmake 内部脚本 FindOpenSSL 中所述）。
- Boost 可以从 https://www.boost.org/ releases 下载预构建版本。

### 查看编译了什么

运行 `indexer -h`。它将显示配置和构建了哪些功能（无论它们是显式的还是调查的，无所谓）：

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->

