# 从源码编译 Manticore

从源码编译 Manticore Search 可以实现自定义构建配置，例如禁用某些功能或添加新的补丁以供测试。例如，您可能希望从源码编译并禁用内嵌的 ICU，以便使用系统上安装的可独立升级的不同版本的 ICU。如果您有兴趣为 Manticore Search 项目做贡献，这也非常有用。

## 使用 CI Docker 构建
为了准备[官方发行版和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和一个特殊的构建镜像。该镜像包含必要的工具，并设计为与外部 sysroots 一起使用，从而可以用一个容器构建所有操作系统的安装包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 构建该镜像，或者直接使用 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 上的镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，您还需要指定以下环境变量：

* `DISTR`：目标平台，例如：`bionic`、`focal`、`jammy`、`buster`、`bullseye`、`bookworm`、`rhel7`、`rhel8`、`rhel9`、`rhel10`、`macos`、`windows`、`freebsd13`
* `arch`：架构，例如：`x86_64`、`x64`（针对 Windows）、`aarch64`、`arm64`（针对 Macos）
* `SYSROOT_URL`：系统根文件归档的 URL。除非您自己构建 sysroots（构建说明见[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)），否则可以使用 https://repo.manticoresearch.com/repository/sysroots 。
* 使用 CI 工作流文件作为参考，以查找可能还需要使用的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

要寻找 `DISTR` 和 `arch` 的可能取值，可以参考目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/，该目录包含所有支持组合的 sysroots。

此后，在 Docker 容器内部构建包只需调用：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，若要为 Ubuntu Jammy 创建一个类似官方版本的安装包，您应该在包含 Manticore Search 源码的目录中执行以下命令。此目录是从 https://github.com/manticoresoftware/manticoresearch 克隆的仓库的根目录：

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
需要使用较长的源目录路径，否则在某些情况下（如 Centos）可能无法成功构建源码。

相同的过程不仅适用于流行的 Linux 发行版，也适用于 FreeBSD、Windows 和 macOS 构建二进制文件/安装包。

## 手动构建

不使用构建 Docker 编译 Manticore **不推荐**，但如果您需要这样操作，以下是您可能需要了解的内容：

### 必备工具

* C++ 编译器
  * 在 Linux 上 - 可以使用 GNU (4.7.2 及以上) 或 Clang
  * 在 Windows 上 - 使用 Microsoft Visual Studio 2019 及以上版本（社区版足够）
  * 在 macOS 上 - Clang（来自 XCode 命令行工具，安装方法：`xcode-select --install`）。
* Bison、Flex - 大多数系统中均可通过包管理器获得，Windows 上它们位于 cygwin 框架中。
* Cmake - 用于所有平台（需要版本 3.19 或以上）

### 获取源码

#### 使用 git

Manticore 源代码[托管在 GitHub](https://github.com/manticoresoftware/manticoresearch)。
要获取源码，克隆仓库后检出所需分支或标签。`master` 分支代表主开发分支。发布时会创建一个版本标签，例如 `3.6.0`，且会启动当前发布的新分支，例如 `manticore-3.6.0`。版本分支头部在所有更改后被用作构建所有二进制发行版的源代码。例如，要获取 3.6.0 版本的源码，可以运行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 使用压缩包

您也可以通过 GitHub 的“Download ZIP”按钮下载所需代码。`.zip` 和 `.tar.gz` 格式都适用。

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

CMake 将检查可用功能并根据其配置构建。默认情况下，所有可用功能均视为启用。脚本还会下载并构建一些外部库，假设您想使用它们。隐含地，您获得最大数量功能的支持。

您还可以通过标志和选项明确配置构建。要启用功能 `FOO`，在 CMake 调用中添加 `-DFOO=1`。
要禁用该功能，使用 `-DFOO=0`。如果未特别说明，启用不可用的功能（例如在 Windows 构建中启用 `WITH_GALERA`）将导致配置失败并报错。禁用功能除了排除其构建外，还会禁止在系统上调查该功能，并禁止下载/构建任何相关的外部库。

#### 配置标志和选项

- **USE_SYSLOG** - 允许在[查询日志](../Logging/Query_logging.md)中使用 `syslog`。
- **WITH_GALERA** -启用搜索守护进程的复制支持。构建时将配置此支持，并将下载、构建并包含 Galera 库的源代码到发行/安装中。通常，使用 Galera 构建是安全的，但不会分发库本身（因此没有 Galera 模块，也没有复制）。然而，有时您可能需要显式禁用它，例如如果您想构建一个静态二进制文件，该文件设计上无法加载任何库，这样即使守护进程内部存在对 'dlopen' 函数的调用也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库进行构建。这对于诸如 [REGEX()](../Functions/String_functions.md#REGEX%28%29)，以及 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
  功能。
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 的源代码，进行编译，并静态链接，这样最终的二进制文件就不依赖于系统中共享的  `RE2` 库。
- **WITH_STEMMER** - 使用 Snowball 词干提取库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 的源代码，进行编译，并静态链接，这样最终的二进制文件就不依赖于系统中共享的  `libstemmer` 库。
- **WITH_ICU** -  使用 ICU (International Components for Unicode) 库进行构建。它用于对中文文本进行分词。当设置 morphology=`icu_chinese` 时将使用它。
- **WITH_JIEBA** -  使用 Jieba 中文分词工具进行构建。它用于对中文文本进行分词。当设置 morphology=`jieba_chinese` 时将使用它。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 的源代码，进行编译，并静态链接，这样最终的二进制文件就不依赖于系统中共享的 `icu` 库。还会将 ICU 数据文件包含到安装/发行包中。静态链接 ICU 的目的是使用一个已知版本的库，以便行为确定且不依赖于任何系统库。您很可能更倾向于使用系统 ICU，因为它可能会随着时间更新，而无需重新编译 Manticore 守护进程。在这种情况下，您需要显式禁用此选项。这还会节省一些由 ICU 数据文件（约 30M）占用的空间，因为它不会包含在发行包中。
- **WITH_SSL** - 用于支持 HTTPS 以及与守护进程的加密 MySQL 连接。系统的 OpenSSL 库将链接到守护进程，这意味着启动守护进程时需要 OpenSSL。这对于支持 HTTPS 是必须的，但对于服务器本身并非严格必要（即没有 SSL 意味着无法通过 HTTPS 连接，但其他协议仍可工作）。SSL 库版本从 1.0.2 到 1.1.1 都可被 Manticore 使用，  但请注意 **为了安全起见，强烈建议使用尽可能新的 SSL
  库**。目前仅支持 v1.1.1，其余版本已过时 (
  请参阅 [openssl release strategy](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - 索引器用于处理来自 MySQL 的压缩列。守护进程用于提供对压缩 MySQL 协议的支持。
- **WITH_ODBC** - 索引器用于支持对来自 ODBC 提供者（通常是 UnixODBC 和 iODBC）的源进行索引。在 MS Windows 上，ODBC 是使用 MS SQL 源的正确方式，因此对 `MSSQL` 的索引也隐含此标志。
- **DL_ODBC** - 不与 ODBC 库链接。如果链接了 ODBC 库但不可用，即使您想处理与 ODBC 无关的内容，也无法启动 indexer 工具。此选项会在您想处理 ODBC 源时才在运行时让索引器加载该库。
- **ODBC_LIB** - ODBC 库文件的名称。当您想处理 ODBC 源时，索引器将尝试加载该文件。此选项会根据可用的 ODBC 共享库自动写入。您也可以在运行索引器之前通过设置环境变量 `ODBC_LIB` 为替代库的正确路径来在运行时覆盖该名称。
- **WITH_EXPAT** - 索引器用于支持对 xmlpipe 源进行索引。
- **DL_EXPAT** - 不与 EXPAT 库链接。如果链接了 EXPAT 库但不可用，即使您想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项会在您想处理 xmlpipe 源时才在运行时让索引器加载该库。
- **EXPAT_LIB** - EXPAT 库文件的名称。当您想处理 xmlpipe 源时，索引器将尝试加载该文件。此选项会根据可用的 EXPAT 共享库自动写入。您也可以在运行索引器之前通过设置环境变量 EXPAT_LIB 为替代库的正确路径来在运行时覆盖该名称。
- **WITH_ICONV** - 在使用索引器索引 xmlpipe 源时支持不同编码。
- **DL_ICONV** - 不与 iconv 库链接。如果链接了 iconv 库但不可用，即使您想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项会在您想处理 xmlpipe 源时才在运行时让索引器加载该库。
- **ICONV_LIB** - iconv 库文件的名称。当您想处理 xmlpipe 源时，索引器将尝试加载该文件。此选项会根据可用的 iconv 共享库自动写入。您也可以在运行索引器之前通过设置环境变量 `ICONV_LIB` 为替代库的正确路径来在运行时覆盖该名称。
- **WITH_MYSQL** - 索引器用于支持对 MySQL 源进行索引。
- **DL_MYSQL** - 不要在链接时包含 MySQL 库。如果 MySQL 已被链接但不可用，即使您想处理与 MySQL 无关的内容，也无法启动 `indexer` 工具。此选项会要求 indexer 仅在需要处理 MySQL 源时，才在运行时加载该库。
- **MYSQL_LIB** -- MySQL 库文件的名称。indexer 会在您想要处理 MySQL 源时尝试加载该文件。此选项会根据可用的 MySQL 共享库自动写入。您也可以在运行时通过在运行 indexer 之前设置环境变量 `MYSQL_LIB` 并提供指向替代库的正确路径来覆盖该名称。
- **WITH_POSTGRESQL** - indexer 用于支持对 PostgreSQL 源进行索引。
- **DL_POSTGRESQL** - 不要链接 PostgreSQL 库。如果 PostgreSQL 已被链接但不可用，即使您想处理与 PostgreSQL 无关的内容，也无法启动 `indexer` ool。此选项会要求 indexer 仅在需要处理 PostgreSQL 源时，才在运行时加载该库。
- **POSTGRESQL_LIB** - postgresql 库文件的名称。indexer 会在处理 postgresql 源时尝试加载指定的 postgresql 库文件。此选项由可用 postgresql 共享库自动确定。您也可以在运行时通过在运行 indexer 之前设置环境变量  `POSTGRESQL_LIB` 并提供指向替代库的正确路径来覆盖该名称。
- **LOCALDATADIR** - 守护进程存储 binlogs 的默认路径。如果未提供此路径或在守护进程运行时配置中显式禁用（即文件 `manticore.conf`，此文件与此构建配置无关），则 binlogs 将放置在此路径中。它通常是绝对路径，但并非必须，也可以使用相对路径。您通常不需要更改配置中定义的默认值，根据目标系统，这可能类似于 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data`。
- **FULL_SHARE_DIR** - 存储所有资源的默认路径。可以在启动任何使用该文件夹中文件的工具之前，通过环境变量 `FULL_SHARE_DIR` 来覆盖该路径。这是一个重要路径，因为许多内容默认期望在此处找到，包括预定义的字符集表、停用词表、manticore 模块和 icu 数据文件，均放在该文件夹中。配置脚本通常将此路径确定为类似 `/usr/share/manticore` 或 `/usr/local/share/manticore`。
- **DISTR_BUILD** - 发布软件包选项的快捷方式。此参数是一个字符串，表示目标平台的名称。它可用于替代手动配置所有选项。在 Debian 和 Redhat Linux 上，默认值可能通过轻量检测确定并设置为通用的 'Debian' 或 'RHEL'。否则，此值未定义。
- **PACK** - 更方便的快捷方式。它读取环境变量 `DISTR`，将其赋值给 **DISTR_BUILD** 参数，然后照常工作。当在预先配置的构建系统（例如 Docker 容器）中构建时，这非常有用，因为在系统级别设置了 `DISTR` 变量，并反映了容器所针对的目标系统。
- **CMAKE_INSTALL_PREFIX** (path) - 指定 Manticore 的安装目录。构建过程不执行任何安装操作，但会准备安装规则，这些规则在您运行 `cmake --install` 命令或创建包并随后安装时执行。该前缀可以在任何时候更改，即使在安装过程中，也可以通过调用
  `cmake --install . --prefix /path/to/installation`. 不过，在配置阶段，此变量用于初始化 `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。 例如，将其在配置阶段设置为 `/my/custom`
  时间将把 `LOCALDATADIR` 硬编码为 `/my/custom/var/lib/manticore/data`，并将 `FULL_SHARE_DIR` 硬编码为
  `/my/custom/usr/share/manticore`.
- **BUILD_TESTING** (bool) 是否支持测试。如果启用，则在构建后可运行 'ctest' 来测试构建。注意，测试意味着额外的依赖项，例如至少需要 PHP cli、Python 以及可用的带有测试数据库的 MySQL 服务器。默认情况下，此参数为开启状态。因此，如果只想构建，您可能希望通过显式指定 'off' 值来禁用该选项。
- **LIBS_BUNDLE** - 存放各种库的文件夹路径。这主要与 Windows 构建相关，但如果您经常需要构建以避免每次都下载第三方源，也可能非常有用。默认情况下，配置脚本不会修改此路径；您需要手动将所有内容放入该文件夹。例如，当我们想支持词干提取器时，源码将从 Snowball 首页下载，然后解压、配置、构建等。相反，您可以将原始源代码压缩包（即 `libstemmer_c.tgz`）存放在此文件夹中。下次从头构建时，配置脚本会首先在此 bundle 中查找，如果在那里找到词干提取器，就不会再次从互联网下载。
- **CACHEB** - 存储第三方库构建结果的文件夹路径。通常，像 galera、re2、icu 等功能首先从网络下载或从 bundle 获取，然后解压、构建并安装到一个临时的内部文件夹。当构建 manticore 时，该文件夹会作为所需功能的现成环境。最后，这些组件要么与 manticore 链接（如果它是库），要么直接用于分发/安装（如 galera 或 icu 数据）。当 **CACHEB** 被定义为 cmake 配置参数或系统环境变量时，它用作这些构建的目标文件夹。此文件夹可跨多次构建保留，从而使存储的库不再重复构建，大大缩短整个构建过程。


请注意，某些选项是以三元组的形式组织的：`WITH_XXX`、`DL_XXX` 和 `XXX_LIB`——比如对 mysql、odbc 等的支持。`WITH_XXX` 决定了后面两个选项是否有效。也就是说，如果你将 `WITH_ODBC` 设为 `0`——那么就没有必要再提供 `DL_ODBC` 和 `ODBC_LIB`，如果整个特性被禁用，这两个选项将不起作用。此外，如果没有 `DL_XXX`，`XXX_LIB` 也没有意义，因为如果你不想要 `DL_XXX` 选项，则不会使用动态加载，而 `XXX_LIB` 指定的名称也将无用。这是由默认的内省机制控制的。

另外，使用 `iconv` 库需要依赖 `expat`，如果后者被禁用，则前者也没有用。

还有些库可能始终可用，因此也就没有必要避开与它们的链接。例如，在 Windows 上的是 ODBC。在 macOS 上的是 Expat、iconv，以及或许还有其他库。默认内省机制可以检查到这些库，并有效地只输出 `WITH_XXX`，而不包含 `DL_XXX` 和 `XXX_LIB`，使问题更为简单。

有了这些选项后，配置操作可以如下进行：

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

除了常规配置项以外，你还可以查看 `CMakeCache.txt` 文件，该文件在你运行配置后会留在构建文件夹中。其中定义的所有值可以在运行 cmake 时被显式重定义。例如，你可以运行 `cmake -DHAVE_GETADDRINFO_A=FALSE ...`，这样本次配置就不会采用之前探测出来的变量值，而是直接使用你指定的值。

#### 特定环境变量

环境变量用于提供一些全局设定，这些设置与构建配置分离，始终存在。为了持久保存，它们可以通过多种方法在系统上全局设置——比如添加到 `.bashrc` 文件，或在基于 docker 的构建系统中写到 Dockerfile 中，或记录到 Windows 的系统环境变量设置中。另外，你也可以用 `export VAR=value` 在 shell 中设置临时变量。甚至更短，可以在 cmake 调用前加上变量，如 `CACHEB=/my/cache cmake ...`——这样该变量仅对本次调用有效，下一次就不可见了。

有些此类变量通常被 cmake 及其他工具通用使用。例如像 `CXX` 用来指定当前的 C++ 编译器，或者 `CXX_FLAGS` 用来提供编译器参数，等等。

然而，我们有一些专属于 manticore 配置的变量，这些变量仅用于我们自己的构建过程。

- **CACHEB** - 与配置项 **CACHEB** 选项相同
- **LIBS_BUNDLE** - 与配置项 **LIBS_BUNDLE** 选项相同
- **DISTR** - 在使用 `-DPACK=1` 时用于初始化 `DISTR_BUILD` 选项。
- **DIAGNOSTIC** - 使 cmake 配置输出变得更详细，解释所有正在发生的事情
- **WRITEB** - 假定选择了 **LIBS_BUNDLE**，如果设置，将下载各类工具的源代码归档文件到 LIBS_BUNDLE 文件夹。也就是说，如果有新的 stemmer 版本发布——你可以手动移除 bundle 中的 libstemmer_c.tgz，然后运行一次性的 `WRITEB=1 cmake ...`——它会发现 bundle 里没有 stemmer 源码，于是就会从官方站点下载到 bundle（如果没有设置 WRITEB，则会下载到 build 的临时文件夹中且在你清除构建目录时也会删除）。

配置结束时，你可以看到类似如下的可用功能列表：

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

要安装请运行：

```bash
cmake --install . --config RelWithDebInfo
```

如需安装到自定义（非默认）目录，请运行

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### 构建安装包

如需构建安装包，请使用目标 `package`。它将根据 `-DDISTR_BUILD` 选项指定的方案构建安装包。默认情况下，它会是一个包含所有二进制文件及辅助文件的 .zip 或 .tgz 格式归档。

```bash
cmake --build . --target package --config RelWithDebInfo
```

## 有关构建的一些高级说明

### 单一配置下的重新编译（更新）

如果没有变更源码路径与构建路径，只需进入你的构建目录并运行：

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

如果因某些原因这不起作用，你可以删除构建文件夹下的 `CMakeCache.txt` 文件。之后你
需要重新运行 cmake，指向源码文件夹并配置相关选项。

如果这仍然无效，只能删除整个构建文件夹然后重新开始。

### 构建类型

简而言之——如同前面所写，直接用 `--config RelWithDebInfo` 即可，绝不会出错。

我们采用两种构建类型。开发用的是 `Debug`——它配置的编译参数更适合开发调试，即支持逐步执行的调试。但生成的二进制体积较大，并不适合生产环境。

发布版本用的是 `RelWithDebInfo`——即“带有调试信息的发布构建”。它产出的二进制用于生产，但嵌入了调试信息。调试信息随后会被分离出来，作为单独的 debuginfo 包和发布包一同存放，以便日后出现崩溃等问题时进行调查和修复。Cmake 还提供 `Release` 和 `MinSizeRel`，但我们并未使用。如果构建类型无效，cmake 会做一个 `noconfig` 构建。

#### 构建系统生成器

生成器分为两种：单一配置和多配置。

- 单一配置需要在配置阶段通过 `CMAKE_BUILD_TYPE` 参数指定构建类型。如果未设置，则默认用 `RelWithDebInfo`，这对于只想从源码构建 Manticore 而不是参与开发的人来说很合适。若需显式指定构建类型，需提供，如 `-DCMAKE_BUILD_TYPE=Debug`。
- 多配置则是在构建阶段选择构建类型。应通过 `--config` 选项指定，否则将默认构建为 `noconfig`，这不是我们希望的。所以你应始终指定构建类型，如 `--config Debug`。

如果您想指定构建类型，但又不想关心它是“单配置”还是“多配置”生成器——只需在两个地方都提供必要的键即可。例如，使用 `-DCMAKE_BUILD_TYPE=Debug` 进行配置，然后使用 `--config Debug` 进行构建。只要确保这两个值相同即可。如果目标构建器是单配置的，它将使用该配置参数。如果是多配置的，则会忽略配置参数，但正确的构建配置将由 `--config` 键选择。

如果您想要 `RelWithDebInfo`（即仅构建生产版本）并且知道您使用的是单配置平台（仅 Windows 之外的所有平台），则可以在 cmake 调用中省略 `--config` 标志。默认的 `CMAKE_BUILD_TYPE=RelWithDebInfo` 将被配置并使用。这样，所有关于“构建”、“安装”和“构建包”的命令将变得更简短。

#### 显式选择构建系统生成器

Cmake 是一个工具，它本身不执行构建，而是为本地构建系统生成规则。
通常，它能很好地确定可用的构建系统，但有时您可能需要显式提供生成器。您
可以运行 `cmake -G` 并查看可用生成器列表。

- 在 Windows 上，如果您安装了多个 Visual Studio 版本，您可能需要指定使用哪一个，
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
Ninja 多配置非常有用，因为它是真正的“多配置”，并且可在 Linux/macOS/BSD 上使用。使用此生成器，您可以将配置类型的选择推迟到构建时，而且可以在同一个构建目录中构建多个配置，只需更改 `--config` 参数。

### 注意事项

1. 如果您最终想构建全功能的 RPM 包，构建目录的路径必须足够长，以便正确生成调试符号。
例如像 `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`。这是因为 RPM 工具在生成调试信息时会修改编译后二进制文件上的路径，而它可能只会覆盖已有空间，不会重新分配更多。上述长路径有 100 个字符，对这种情况来说已经足够。

## 外部依赖

如果您想使用某些库，必须确保它们可用。
- 用于索引（`indexer` 工具）：`expat`，`iconv`，`mysql`，`odbc`，`postgresql`。没有它们，您只能处理 `tsv` 和 `csv` 源。
- 用于服务查询（`searchd` 守护进程）：可能需要 `openssl`。
- 对于所有（必需，强制）我们都需要 Boost 库。最低版本是 1.61.0，然而我们使用更新的版本 1.75.0 来构建二进制文件。更近期的版本（如 1.76）也应该没问题。在 Windows 上，您可以从 Boost 官方网站（boost.org）下载预编译版本并安装到默认推荐路径（例如 `C:\\boost...`）。在 MacOS 上，通过 brew 提供的版本是合适的。在 Linux 上，您可以检查官方仓库中的可用版本，如果不满足要求，可以从源码构建。我们需要组件 `context`，如果您还想从源码构建 Galera 库，也可能需要构建组件 `system` 和 `program_options`。关于如何操作，可以查看 `dist/build_dockers/xxx/boost_175/Dockerfile`，那里有一个简洁的自说明脚本/指南。

在构建系统上，您需要安装这些包的 'dev' 或 'devel' 版本（即 - libmysqlclient-devel、unixodbc-devel 等，具体包名请参考我们的 dockerfile）。

在运行系统上，这些包至少应以最终（非开发）版本存在。（devel 版本通常体积更大，因为它们不仅包含目标二进制文件，还包括各种开发内容，如头文件等）。

### 在 Windows 上构建

除了必要的前置条件，您可能还需要预编译的 `expat`，`iconv`，`mysql` 和 `postgresql` 客户端库。您必须自己构建它们，或者联系我们获取我们的构建包（一个简单的 zip 归档，里面包含这些目标文件夹）。

- ODBC 不必要，因为它是系统库。
- OpenSSL 可以从源码构建，也可以从 https://slproweb.com/products/Win32OpenSSL.html 下载预编译版本（如 cmake 的 FindOpenSSL 内部脚本所述）。
- Boost 可以从 https://www.boost.org/ 下载预编译版本。

### 查看已编译内容

运行 `indexer -h`。它会显示已经配置和构建的功能（无论是显式的还是自动检测的，都一样）：

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->

