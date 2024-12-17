# 从源代码编译 Manticore

从源代码编译 Manticore Search 允许自定义构建配置，例如禁用某些功能或添加测试补丁。例如，你可能想要禁用嵌入的 ICU，以便使用系统上安装的可以独立升级的不同版本。这对于希望为 Manticore Search 项目贡献代码的人也很有用。

## 使用 CI Docker 构建
要准备[官方发布和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和一个特殊的构建镜像。该镜像包含必要的工具，并设计为与外部 sysroot 一起使用，因此一个容器可以为所有操作系统构建包。你可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 构建该镜像，或者使用 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 上的现成镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，你还需要指定以下环境变量：

* `DISTR`: 目标平台
* `arch`: 架构
* `SYSROOT_URL`: 系统根目录档案的 URL。你可以使用 https://repo.manticoresearch.com/repository/sysroots，除非你自己构建 sysroots（可以在[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)找到说明）。
* 使用 CI 工作流程文件作为参考，查找你可能需要使用的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml


要查找 `DISTR` 和 `arch` 的可能值，可以参考 [sysroots 目录](https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/)，该目录包含所有支持组合的 sysroots。

构建包的命令如下：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，要为 Ubuntu Jammy 创建类似于官方版本 Manticore Core Team 提供的包，你应该在包含 [Manticore Search 源代码](https://github.com/manticoresoftware/manticoresearch)的目录中执行以下命令：

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
-v $(pwd):/manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa \
manticoresearch/external_toolchain:clang16_cmake3263 bash

# following is to be run inside docker shell
cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/
mkdir build && cd build
cmake -DPACK=1 ..

cmake --build .
# or if you want to build packages:
# cmake --build . --target package
```
需要较长的源目录路径，否则在某些情况下（例如 CentOS）可能会导致构建源代码失败。

同样的流程不仅适用于常见的 Linux 发行版，还可以用于构建 FreeBSD、Windows 和 macOS 上的二进制文件/软件包。

## 手动构建

不推荐在不使用 Docker 构建的情况下编译 Manticore，但如果你需要这样做，以下是你需要了解的内容：

### 所需工具

* C++ 编译器
  * Linux：可以使用 GNU（4.7.2 及以上）或 Clang
  * Windows：Microsoft Visual Studio 2019 及以上（社区版即可）
  * MacOS：Clang（来自 XCode 的命令行工具，使用 `xcode-select --install` 安装）
* Bison, Flex - 在大多数系统上，它们作为软件包可用，Windows 上可以通过 Cygwin 框架获得。
* Cmake - 在所有平台上使用（需要 3.19 或以上版本）。

### 获取源代码

#### **从 Git 获取** 

Manticore 源代码[托管在 GitHub 上](https://github.com/manticoresoftware/manticoresearch)。要获取源代码，克隆仓库并检查所需的分支或标签。`master` 分支表示主要开发分支。发布后，创建一个版本标签，例如 3.6.0，并开始一个新的当前版本分支，此时为 `manticore-3.6.0`。版本分支的头部在所有更改后被用作构建所有二进制版本的源。例如，要获取 3.6.0 版本的源代码，可以运行：



```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### **从压缩包获取** 

你可以通过使用 GitHub 上的“Download ZIP”按钮下载所需的代码。`.zip` 和 `.tar.gz` 格式均可。

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### 配置

Manticore 使用 CMake 进行配置。假设你在克隆仓库的根目录下，执行以下命令：

```bash
mkdir build && cd build
cmake ..
```

CMake 会检查可用的特性并根据这些特性配置构建。默认情况下，如果可用，所有特性都会被认为是启用的。该脚本还会下载和构建一些外部库，假设你想使用它们，从而隐式获得最大特性的支持。

你也可以使用标志和选项显式配置构建。例如，要启用特性 FOO，可以在 CMake 调用中添加 `-DFOO=1`。要禁用它，则使用 `-DFOO=0`。如果未明确指出，启用不可用的特性（如在 Windows 构建中使用 `WITH_GALERA`）将导致配置失败。禁用特性将排除该特性，同时禁用对系统的调查和相关外部库的下载/构建。

#### 配置标志和选项

- **USE_SYSLOG** - 允许在[查询日志](../Logging/Query_logging.md)使用 syslog。
- **WITH_GALERA** - 启用搜索守护进程的复制支持。将为构建配置支持，并下载、构建 Galera 库的源代码并包含在发行版中。通常，使用 Galera 进行构建是安全的，但不建议分发该库本身（即没有 Galera 模块和复制）。有时你可能需要显式禁用它，例如如果你想构建一个设计上无法加载任何库的静态二进制文件，这样即使在守护进程内调用 `dlopen` 函数也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库进行构建。对于 [REGEX()](../Functions/String_functions.md#REGEX%28%29)和  [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)功能是必需的。
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 的源代码，编译并静态链接，以便最终的二进制文件不依赖于系统中共享的 RE2 库。
- **WITH_STEMMER** - 使用 Snowball 词干提取库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 源代码，编译并静态链接，以便最终的二进制文件不依赖于系统中共享的 libstemmer 库。
- **WITH_ICU** -  使用 ICU（国际化组件库）进行构建。此库用于中文分词和文本切分，当配置 morphology=`icu_chinese` 时会使用它。
- **WITH_JIEBA** -  使用 Jieba 中文分词工具进行构建。该工具用于对中文文本进行分词，当配置 morphology='jieba_chinese' 时会使用它。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 的源代码，编译并静态链接，以便最终的二进制文件不依赖于系统中共享的 ICU 库。同时将 ICU 数据文件包含在安装/发行版中。静态链接 ICU 的目的是拥有一个已知版本的库，以确保行为是确定的，而不依赖于任何系统库。你可能更希望使用系统的 ICU，因为它可以随着时间更新，无需重新编译 Manticore 守护进程。在这种情况下，你需要显式禁用此选项，这样可以节省约 30M 的 ICU 数据文件空间，因为该文件将不包含在发行版中。
- **WITH_SSL** - 用于支持 HTTPS 和加密的 MySQL 连接到守护进程。系统的 OpenSSL 库将链接到守护进程，这意味着启动守护进程时需要 OpenSSL。支持 HTTPS 是必需的，但对于服务器来说并非绝对必要（即没有 SSL 连接意味着无法通过 HTTPS 连接，但其他协议仍然可以使用）。Manticore 支持从 1.0.2 到 1.1.1 版本的 SSL 库，但**为了安全起见，强烈建议使用最新版本的 SSL 库**。目前仅支持 1.1.1 版本，其他版本已过时（参见 [openssl 发布策略](https://www.openssl.org/policies/releasestrat.html)）。
- **WITH_ZLIB** - 索引器用于处理来自 MySQL 的压缩列，守护进程用于支持压缩的 MySQL 协议。
- **WITH_ODBC** - 索引器支持从 ODBC 提供者索引数据源（通常是 UnixODBC 和 iODBC）。在 Windows 上，ODBC 是处理 MS SQL 源的合适方式，因此索引 MSSQL 也需要启用此标志。
- **DL_ODBC** - 不链接 ODBC 库。如果链接了 ODBC，但不可用，即使你想处理与 ODBC 无关的内容，也无法启动索引工具。此选项要求索引器在运行时仅在处理 ODBC 源时加载库。
- **ODBC_LIB** - ODBC 库文件的名称。索引器在处理 ODBC 源时将尝试加载该文件。此选项会根据可用的 ODBC 共享库自动写入。你也可以在运行时通过提供环境变量 ODBC_LIB 和正确路径来覆盖该名称。
- **WITH_EXPAT** - 索引器用于支持从 xmlpipe 源索引。
- **DL_EXPAT** - 不链接 EXPAT 库。如果链接了 EXPAT，但不可用，即使你想处理与 xmlpipe 无关的内容，也无法启动索引工具。此选项要求索引器在运行时仅在处理 xmlpipe 源时加载库。
- **EXPAT_LIB** - EXPAT 库文件的名称。索引器在处理 xmlpipe 源时将尝试加载该文件。此选项会根据可用的 EXPAT 共享库自动写入。你也可以在运行时通过提供环境变量 EXPAT_LIB 和正确路径来覆盖该名称。
- **WITH_ICONV** - 支持在索引 xmlpipe 源时使用不同编码。
- **DL_ICONV** - 不链接 iconv 库。如果链接了 iconv，但不可用，即使你想处理与 xmlpipe 无关的内容，也无法启动索引工具。此选项要求索引器在运行时仅在处理 xmlpipe 源时加载库。
- **ICONV_LIB** - iconv 库文件的名称。索引器在处理 xmlpipe 源时将尝试加载该文件。此选项会根据可用的 iconv 共享库自动写入。你也可以在运行时通过提供环境变量 ICONV_LIB 和正确路径来覆盖该名称。
- **WITH_MYSQL** - 索引器用于支持从 MySQL 源索引。
- **DL_MYSQL** - 不链接 MySQL 库。如果链接了 MySQL，但不可用，即使你想处理与 MySQL 无关的内容，也无法启动索引工具。此选项要求索引器在运行时仅在处理 MySQL 源时加载库。
- **MYSQL_LIB** - MySQL 库文件的名称。索引器在处理 MySQL 源时将尝试加载该文件。此选项会根据可用的 MySQL 共享库自动写入。你也可以在运行时通过提供环境变量 MYSQL_LIB 和正确路径来覆盖该名称。
- **WITH_POSTGRESQL** - 索引器用于支持从 PostgreSQL 源索引。
- **DL_POSTGRESQL** - 不链接 PostgreSQL 库。如果链接了 PostgreSQL，但不可用，即使你想处理与 PostgreSQL 无关的内容，也无法启动索引工具。此选项要求索引器在运行时仅在处理 PostgreSQL 源时加载库。
- **POSTGRESQL_LIB** - PostgreSQL 库文件的名称。索引器在处理 PostgreSQL 源时将尝试加载该文件。此选项会根据可用的 PostgreSQL 共享库自动写入。你也可以在运行时通过提供环境变量 POSTGRESQL_LIB 和正确路径来覆盖该名称。
- **LOCALDATADIR** - 守护进程存储 binlogs 的默认路径。如果未提供该路径或在守护进程的运行时配置（即与此构建配置无关的 manticore.conf 文件）中显式禁用，binlogs 将放置在此路径下。通常是绝对路径，但也可以使用相对路径。你可能不需要更改配置中定义的默认值，这通常取决于目标系统，例如可能是 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data`。
- **FULL_SHARE_DIR** - 存储所有资产的默认路径。在启动任何使用该文件夹中文件的工具之前，可以通过环境变量 FULL_SHARE_DIR 覆盖此路径。这个路径非常重要，因为默认情况下许多东西都期望在这里找到。这包括预定义的字符集表、停用词、Manticore 模块和 ICU 数据文件，所有这些都放在该文件夹中。配置脚本通常会将此路径确定为 `/usr/share/manticore` 或 `/usr/local/share/manticore`。
- **DISTR_BUILD** - 发布包选项的快捷方式。它是一个字符串值，表示目标平台的名称。可以用它替代手动配置所有选项。在 Debian 和 Redhat Linux 系统上，默认值可能通过轻量级自省确定并设置为通用的 “Debian” 或 “RHEL”。否则，该值未定义。
- **PACK** - 更便捷的快捷方式。它读取 DISTR 环境变量，将其分配给 DISTR_BUILD 参数，然后正常工作。当在准备好的构建系统（如 Docker 容器）中构建时，这非常有用，因为 DISTR 变量在系统级别设置，反映容器目标系统。
- **CMAKE_INSTALL_PREFIX** (path) - Manticore 预期安装的路径。构建不执行任何安装，但会准备安装规则，这些规则在你运行 `cmake --install` 命令或创建包并安装时执行。前缀可以在任何时候更改，甚至在安装过程中，可以通过调用 `cmake --install . --prefix /path/to/installation` 来修改。然而，在配置时，该变量用于初始化 LOCALDATADIR 和 FULL_SHARE_DIR 的默认值。例如，在配置时将其设置为 `/my/custom`，将硬编码 LOCALDATADIR 为 `/my/custom/var/lib/manticore/data`，而 FULL_SHARE_DIR 为 `/my/custom/usr/share/manticore`。
- **BUILD_TESTING** (bool) - 是否支持测试。如果启用，构建后你可以运行 `ctest` 来测试构建。请注意，测试需要额外的依赖项，例如至少需要 PHP CLI、Python 和可用的MySQL 服务器及测试数据库。默认情况下，该参数是开启的。因此，对于“仅构建”，你可能想通过显式指定“off”值来禁用该选项。
- **LIBS_BUNDLE** - 包含不同库的文件夹路径。这主要与 Windows 构建相关，但如果你需要频繁构建以避免每次下载第三方源，也可能有所帮助。默认情况下，配置脚本不会修改此路径；你应该手动将所有内容放在那里。当我们想要支持一个词干提取器时，源代码将从 Snowball 官方网站下载，然后解压、配置、构建等。相反，你可以将原始源压缩包（即 libstemmer_c.tgz）存储在该文件夹中。下次想要从头开始构建时，配置脚本将首先在捆绑包中查找，如果在那里找到词干提取器，则不会再从互联网下
- **CACHEB** - 存储第三方库构建的文件夹路径。通常，像 Galera、RE2、ICU 等功能首先下载或从捆绑包获取，然后解压、构建并安装到临时内部文件夹中。在构建 Manticore 时，该文件夹用作支持所请求功能所需内容的存放地点。最后，如果是库则与 Manticore 链接；如果是 Galera 或 ICU 数据，则直接进入发行版/安装。当 CACHEB 被定义为 cmake 配置参数或系统环境变量时，它被用作该构建的目标文件夹。这个文件夹可以在构建之间保留，以便存储在那里的库不再重建，从而使整个构建过程更短。


注意，有些选项是成组三个一组组织的：`WITH_XXX`、`DL_XXX` 和 `XXX_LIB`，例如 MySQL、ODBC 等。`WITH_XXX` 决定后两个选项是否有效。即，如果你将 `WITH_ODBC` 设置为 0，那么提供 `DL_ODBC` 和 `ODBC_LIB` 将没有意义，并且这两个选项在整个功能被禁用时将无效。此外，`XXX_LIB` 在没有 `DL_XXX` 的情况下也没有意义，因为如果你不想使用 `DL_XXX` 选项，动态加载将不会被使用，而 `XXX_LIB` 提供的名称将毫无用处。这是默认的自省机制所使用的。

同时，使用 `iconv` 库也需要依赖 `expat`，如果 `expat` 被禁用，那么使用 `iconv` 就没有意义。

还有一些库可能总是可用，因此没有必要避免与它们链接。例如，在 Windows 上是 ODBC；在 macOS 上是 Expat、iconv 以及可能的其他库。默认的自省机制会确定这些库，并仅针对它们发出 `WITH_XXX`，而不包括 `DL_XXX` 和 `XXX_LIB`，这样简化了配置过程。

在配置时，某些选项的命令可能如下所示：

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

除了常规的配置值外，你还可以检查在运行配置后保留在构建文件夹中的 `CMakeCache.txt` 文件。该文件中定义的任何值都可以在运行 `cmake` 时显式重定义。例如，你可以运行 `cmake -DHAVE_GETADDRINFO_A=FALSE ...`，这样配置运行将不会采用该变量的调查值，而是使用你提供的值。

#### 特定环境变量

环境变量对于提供一些全局设置非常有用，这些设置存储在构建配置之外，并始终存在。为了持久化，可以通过不同的方式在系统上全局设置它们，比如将其添加到 `.bashrc` 文件中，或嵌入到 Dockerfile 中，如果你使用基于 Docker 的构建系统，或者在 Windows 的系统偏好环境变量中编写它们。你也可以通过在终端中使用 `export VAR=value` 设置短期变量，或者更简洁地通过在 cmake 调用前添加值，例如 `CACHEB=/my/cache cmake ...`，这种方式仅在该调用有效，且在下次调用时不可见。

一些变量在 CMake 和其他工具中是通用的，比如 `CXX`（当前 C++ 编译器）或 `CXX_FLAGS`（编译器标志）等。

然而，我们有一些特定于 Manticore 配置的变量，这些变量专门为我们的构建而设计。

- **CACHEB** - 与配置中的 **CACHEB** 选项相同
- **LIBS_BUNDLE** - 与配置中的 **LIBS_BUNDLE** 选项相同
- **DISTR** - 用于初始化 DISTR_BUILD 选项，当使用 `-DPACK=1` 时
- **DIAGNOSTIC** - 使 CMake 配置的输出更加详细，解释发生的每一件事
- **WRITEB** - 假定 **LIBS_BUNDLE**，并在设置时将不同工具的源归档文件下载到 LIBS_BUNDLE 文件夹。例如，当新的 stemmer 版本发布时，可以手动删除 `libstemmer_c.tgz`，然后运行一次性 `WRITEB=1 cmake ...`，它会从供应商网站下载源到包中（如果没有 `WRITEB`，则会下载到构建中的临时文件夹，构建删除后将消失）。

在配置结束时，你可以看到可用并将被使用的功能列表，例如：

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

安装命令：

```bash
cmake --install . --config RelWithDebInfo
```

要安装到自定义（非默认）文件夹，使用：

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### 打包构建

要构建包，使用 `package` 目标。它将根据 `-DDISTR_BUILD` 选项提供的选择构建包。默认情况下，它将生成一个简单的 `.zip` 或 `.tgz` 存档，包含所有二进制文件和补充文件。

```bash
cmake --build . --target package --config RelWithDebInfo
```

## 关于构建的一些高级内容

### 单配置的重新编译（更新）

如果你没有更改源和构建的路径，只需进入构建文件夹并运行：

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

如果出于某种原因不起作用，可以删除构建文件夹中的 `CMakeCache.txt` 文件。然后需要再次运行 cmake，指向源文件夹并配置选项。

如果问题仍然存在，可以清空构建文件夹并重新开始。

### 构建类型

我们使用两种构建类型。开发时使用 Debug，它分配编译器标志进行优化等，使其非常适合开发，即调试时可以逐步执行。然而，生成的二进制文件较大且在生产环境中运行较慢。

发布时，我们使用另一种类型 RelWithDebInfo，意思是“带有调试信息的发布版本”。它生成嵌入了调试信息的生产环境二进制文件，之后这些调试信息会被分离成单独的 debuginfo 包，与发布包一起存储，在遇到崩溃等问题时可用于调查和修复错误。CMake 还提供了 Release 和 MinSizeRel 构建类型，但我们不使用它们。如果未指定构建类型，CMake 将执行 noconfig 构建。

#### 构建系统生成器

有两种生成器类型：单配置和多配置。

- **单配置**：在配置时需要提供构建类型（CMAKE_BUILD_TYPE 参数）。如果未定义，构建将回退到 RelWithDebInfo 类型。显式构建时，应该提供构建类型，例如 `-DCMAKE_BUILD_TYPE=Debug`。
- **多配置**：在构建时选择构建类型。必须提供 `--config` 选项，否则将构建无配置类型。始终应指定构建类型，例如 `--config Debug`。

如果想指定构建类型但不关心是“单”还是“多”配置生成器，可以在两个地方都提供必要的键。例如，使用 `-DCMAKE_BUILD_TYPE=Debug` 进行配置，然后使用 `--config Debug` 进行构建。确保这两个值相同。

如果想要 RelWithDebInfo（即仅用于生产构建）并且知道自己在单配置平台上（即除了 Windows 之外的所有平台），可以在 CMake 调用时省略 `--config` 标志。默认的 `CMAKE_BUILD_TYPE=RelWithDebInfo` 将会被配置并使用。这将使所有构建、安装和打包的命令更简洁。

#### 显式选择构建系统生成器

CMake 是一个不执行构建的工具，而是生成本地构建系统的规则。通常，它能很好地确定可用的构建系统，但有时你可能需要显式提供生成器。你可以运行 `cmake -G` 并查看可用生成器的列表。

- 在 Windows 上，如果安装了多个版本的 Visual Studio，可能需要指定使用哪个版本，例如：
```bash
cmake -G "Visual Studio 16 2019" ....
```
- 在所有其他平台上，通常使用 Unix Makefiles，但你可以指定其他生成器，例如 Ninja，或 Ninja Multi-Config：
```bash
  cmake -GNinja ...
```
  或
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja Multi-Config 非常有用，因为它确实是“多配置”，可用于 Linux/macOS/BSD。使用此生成器，可以将选择配置类型的工作转移到构建时，并且可以在同一个构建文件夹中构建多个配置，仅通过更改 `--config` 参数。

### 注意事项

1. 如果要最终构建一个功能完整的 RPM 包，构建目录的路径必须足够长，以正确构建调试符号。例如，路径应为 `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`。这是因为 RPM 工具在构建调试信息时会修改编译二进制文件的路径，可能会覆盖现有的空间而不分配更多。上述长路径有 100 个字符，对于这种情况是足够的。

## 外部依赖

某些库必须可用，以便使用相应的功能。
- 索引（Indexer 工具）：需要以下库：`expat`、`iconv`、`mysql`、`odbc`、`postgresql`。没有这些库，你只能处理 `tsv` 和 `csv` 源。
- 查询服务（searchd 守护进程）：可能需要 `openssl`。

- 必需库：我们需要 Boost 库。最低版本要求是 1.61.0，然而，我们构建的二进制文件使用的是更新版本 1.75.0。更高版本（如 1.76）也应该可以。

  - **Windows**：可以从 Boost 官方网站（boost.org）下载预构建的 Boost 并安装到默认建议路径（即 `C:\boost...`）。
  - **macOS**：使用 brew 提供的版本。
  - **Linux**：检查官方仓库中可用的版本，如果不符合要求，可以从源代码构建。我们需要 `context` 组件，此外，如果你还想从源代码构建 Galera 库，可以构建 `system` 和 `program_options` 组件。可以查看 `dist/build_dockers/xxx/boost_175/Dockerfile`，其中有简短的自文档脚本/说明。

在构建系统上，需要安装这些软件包的 `dev` 或 `devel` 版本（例如 `libmysqlclient-devel`、`unixodbc-devel` 等）。可以查看我们的 Dockerfile 以获取具体软件包名称。

在运行系统上，这些软件包应至少存在于最终（非开发）版本中（开发版本通常较大，因为它们包含目标二进制文件以及其他开发内容，如包含头文件等）。

### 在 Windows 上构建

除了必要的先决条件外，你可能还需要预构建的 `expat`、`iconv`、`mysql` 和 `postgresql` 客户端库。你可以自己构建它们，或者联系我们获取我们的构建包（一个简单的 zip 压缩档案，里面包含这些目标的文件夹）。

- **ODBC**：不需要，因为它是系统库。
- **OpenSSL**：可以从源代码构建，或者从 [Win32OpenSSL](https://slproweb.com/products/Win32OpenSSL.html) 下载预构建版本（如 CMake 内部脚本在 `FindOpenSSL` 中所述）。
- **Boost**：可以从 Boost 官方网站 下载预构建版本。

### 查看已编译内容

运行 `indexer -h`。它将显示已配置和构建的功能（无论是显式还是调查的，都无所谓）。

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->
