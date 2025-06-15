# 从源码编译 Manticore

从源码编译 Manticore Search 可以实现自定义构建配置，比如禁用某些功能或添加新的补丁进行测试。例如，您可能希望从源码编译并禁用内嵌的 ICU，以便使用系统上安装的不同版本，该版本可以独立于 Manticore 升级。如果您有兴趣为 Manticore Search 项目做贡献，这也是非常有用的。

## 使用 CI Docker 构建
为了准备[官方发布和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和一个特殊的构建镜像。该镜像包含必备工具，设计为配合外部 sysroots 使用，这样一个容器就可以为所有操作系统构建软件包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 构建该镜像，或者使用 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 上的镜像。这是为任何支持的平台和架构创建二进制文件的最简单方法。运行容器时，您还需要指定以下环境变量：

* `DISTR`：目标平台：`bionic`、`focal`、`jammy`、`buster`、`bullseye`、`bookworm`、`rhel7`、`rhel8`、`rhel9`、`macos`、`windows`、`freebsd13`
* `arch`：架构：`x86_64`、`x64`（Windows 用）、`aarch64`、`arm64`（Macos 用）
* `SYSROOT_URL`：系统根目录归档文件的 URL。除非您自己构建 sysroots（可查看[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)的说明），否则可以使用 https://repo.manticoresearch.com/repository/sysroots。
* 使用 CI 工作流程文件作为参考，查找您可能需要使用的其它环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

要查找 `DISTR` 和 `arch` 的可能取值，可以参考目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/，这里包含了所有支持的组合的 sysroots。

之后，在 Docker 容器内构建软件包就像调用下面这样简单：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，要为 Ubuntu Jammy 创建一个类似于 Manticore 核心团队提供的官方版本的软件包，您应在包含 Manticore Search 源代码的目录中执行以下命令。该目录是从 https://github.com/manticoresoftware/manticoresearch 克隆的仓库根目录：

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
需要使用较长的源码目录路径，否则某些情况下（例如 Centos）可能无法成功构建源码。

同样的流程不仅可以用于为常见的 Linux 发行版构建二进制文件/软件包，也可用于 FreeBSD、Windows 和 macOS。

## 手动编译

不推荐不使用构建 Docker 直接编译 Manticore，但如果您必须这样做，以下信息可能会对您有帮助：

### 必需工具

* C++ 编译器
  * 在 Linux 上 - 可以使用 GNU（4.7.2 及以上）或 Clang
  * 在 Windows 上 - 使用 Microsoft Visual Studio 2019 及以上版本（社区版即可）
  * 在 macOS 上 - 使用 Clang（来自 XCode 的命令行工具，使用 `xcode-select --install` 进行安装）
* Bison、Flex - 大多数系统作为软件包提供，Windows 上则可在 cygwin 框架中获得
* Cmake - 各平台均使用（要求版本 3.19 或更高）

### 获取源码

#### 从 git 获取

Manticore 源代码[托管在 GitHub](https://github.com/manticoresoftware/manticoresearch)。
获取源码需克隆仓库然后切换到所需的分支或标签。`master` 分支是主要开发分支。发布版本时，会创建如 `3.6.0` 的版本标签，并启动当前版本的新分支，如 `manticore-3.6.0`。版本分支的最终提交用于构建所有二进制发布。比如，要获取版本 3.6.0 的源码，可以运行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 从归档包获取

您可以通过 GitHub 的“Download ZIP”按钮下载相应源码。.zip 和 .tar.gz 格式均可。

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### 配置

Manticore 使用 CMake。假设您处于克隆仓库的根目录下：

```bash
mkdir build && cd build
cmake ..
```

CMake 会检测可用功能并据此配置构建。默认情况下，如果功能可用，均视为启用。脚本也会下载并构建部分外部库，假定您会使用它们。这样隐式地支持了最大的功能数量。

您也可以通过标志和选项显式配置构建。要启用功能 `FOO`，向 CMake 调用中添加 `-DFOO=1`。
要禁用它，使用 `-DFOO=0`。如果未特别注明，启用不可用的功能（例如 Windows 构建中的 `WITH_GALERA`）会导致配置失败报错。禁用功能除排除它的构建外，还会禁用对系统的检测及任何相关外部库的下载/构建。

#### 配置标志和选项

- **USE_SYSLOG** - 允许在[查询日志](../Logging/Query_logging.md)中使用 `syslog`。
- **WITH_GALERA** - 启用对搜索守护进程的复制支持。支持将为构建配置，并且 Galera 库的源代码将被下载、构建并包括在分发/安装中。通常，使用 Galera 构建是安全的，但不分发库本身（因此没有 Galera 模块，没有复制）。然而，有时您可能需要显式地禁用它，例如如果您想构建一个静态二进制文件，在设计上无法加载任何库，以至于即使在守护进程内部调用 'dlopen' 函数也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库进行构建。这对于像 [REGEX()](../Functions/String_functions.md#REGEX%28%29) 和 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
  功能是必要的。
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 的源代码，编译它们，并静态链接它们，以便最终的二进制文件将不依赖于您系统中共享的 `RE2` 库的存在。
- **WITH_STEMMER** - 使用 Snowball 词干库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 源代码，编译它们，并静态链接它们，以便最终的二进制文件将不依赖于您系统中共享的 `libstemmer` 库的存在。
- **WITH_ICU** - 使用 ICU（国际化组件用于 Unicode）库进行构建。它用于分割中文文本。当使用 morphology=`icu_chinese` 时会使用它。
- **WITH_JIEBA** - 使用 Jieba 中文文本分割工具进行构建。它用于分割中文文本。当使用 morphology=`jieba_chinese` 时会使用它。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 源代码，编译它们，并静态链接它们，以便最终的二进制文件将不依赖于您系统中共享的 `icu` 库的存在。还将 ICU 数据文件包含在安装/分发中。静态链接的 ICU 目的是拥有一个已知版本的库，以便行为是确定的，不依赖于任何系统库。您可能更愿意使用系统 ICU，因为它可能会随着时间的推移而更新，而无需重新编译 Manticore 守护进程。在这种情况下，您需要显式禁用此选项。这也将为您节省一些占用的空间，由于 ICU 数据文件（约 30M）将不会包含在分发中。
- **WITH_SSL** - 用于支持 HTTPS，以及加密与守护进程的 MySQL 连接。系统 OpenSSL 库将与守护进程链接。这意味着启动守护进程将需要 OpenSSL。这对于支持 HTTPS 是强制性的，但对于服务器并不是严格要求（即没有 SSL 意味着无法通过 HTTPS 连接，但其他协议将正常工作）。Manticore 可以使用从 1.0.2 到 1.1.1 的 SSL 库版本，但请注意 **出于安全原因，强烈建议使用最新的 SSL
  库**。目前仅支持 v1.1.1，其余版本已过时（
  参见 [openssl release strategy](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - 被索引器用于处理来自 MySQL 的压缩列。被守护进程用来提供对压缩 MySQL 协议的支持。
- **WITH_ODBC** - 被索引器用于支持来自 ODBC 提供者的索引源（它们通常是 UnixODBC 和 iODBC）。在 MS Windows 上，使用 ODBC 是与 MS SQL 源工作的正确方法，因此索引 `MSSQL` 也意味着这个标志。
- **DL_ODBC** - 不与 ODBC 库链接。如果链接了 ODBC，但不可用，即使您想处理与 ODBC 无关的内容，您也无法启动索引器工具。此选项要求索引器仅在您想处理 ODBC 源时在运行时加载库。
- **ODBC_LIB** - ODBC 库文件的名称。索引器将在您想处理 ODBC 源时尝试加载该文件。此选项是自动从可用的 ODBC 共享库调查中写入的。您还可以通过在运行索引器之前提供环境变量 `ODBC_LIB` ，并指定替代库的正确路径来覆盖该名称。
- **WITH_EXPAT** - 被索引器用于支持索引 xmlpipe 源。
- **DL_EXPAT** - 不与 EXPAT 库链接。如果链接了 EXPAT，但不可用，即使您想处理与 xmlpipe 无关的内容，您也无法启动 `indexer` 工具。此选项要求索引器仅在您想处理 xmlpipe 源时在运行时加载库。
- **EXPAT_LIB** - EXPAT 库文件的名称。当您想处理 xmlpipe 源时，索引器将尝试加载该文件。此选项是自动从可用的 EXPAT 共享库调查中写入的。您还可以通过在运行索引器之前提供环境变量 EXPAT_LIB，并指定替代库的正确路径来覆盖该名称。
- **WITH_ICONV** - 用于支持在使用索引器索引 xmlpipe 源时的不同编码。
- **DL_ICONV** - 不与 iconv 库链接。如果链接了 iconv，但不可用，即使您想处理与 xmlpipe 无关的内容，您也无法启动 `indexer` 工具。此选项要求索引器仅在您想处理 xmlpipe 源时在运行时加载库。
- **ICONV_LIB** - iconv 库文件的名称。当您想处理 xmlpipe 源时，索引器将尝试加载该文件。此选项是自动从可用的 iconv 共享库调查中写入的。您还可以通过在运行索引器之前提供环境变量 `ICONV_LIB` ，并指定替代库的正确路径来覆盖该名称。
- **WITH_MYSQL** - 被索引器用于支持索引 MySQL 源。
- **DL_MYSQL** - 不要链接 MySQL 库。如果 MySQL 已链接但不可用，即使您想处理与 MySQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器在运行时仅在您想处理 MySQL 数据源时加载库。
- **MYSQL_LIB** -- MySQL 库文件的名称。当您想处理 MySQL 数据源时，索引器将尝试加载该文件。此选项是通过可用的 MySQL 共享库调查自动写入的。您还可以通过在运行索引器之前提供环境变量 `MYSQL_LIB` 和替代库的正确路径在运行时覆盖该名称。
- **WITH_POSTGRESQL** - 索引器用于支持索引 PostgreSQL 源。
- **DL_POSTGRESQL** - 不要链接 PostgreSQL 库。如果 PostgreSQL 已链接但不可用，即使您想处理与 PostgreSQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器在运行时仅在您想处理 PostgreSQL 数据源时加载库。
- **POSTGRESQL_LIB** - PostgreSQL 库文件的名称。当处理 PostgreSQL 数据源时，索引器将尝试加载指定的 PostgreSQL 库文件。此选项是通过可用的 PostgreSQL 共享库调查自动确定的。您还可以通过在运行索引器之前提供环境变量 `POSTGRESQL_LIB` 和替代库的正确路径在运行时覆盖该名称。
- **LOCALDATADIR** - 守护进程存储 binlog 的默认路径。如果该路径未提供或在守护进程的运行时配置中显式禁用（即文件 `manticore.conf`，与此构建配置无关），则 binlog 将放置在此路径中。通常这是一个绝对路径，但并不要求必须是，亦可以使用相对路径。您可能不需要更改配置中定义的默认值，该值根据目标系统可能类似于 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data`。
- **FULL_SHARE_DIR** - 存储所有资产的默认路径。可以通过在启动任何利用该文件夹中文件的工具之前提供环境变量 `FULL_SHARE_DIR` 来覆盖此路径。这是一个重要路径，因为默认情况下期望在此找到许多东西。这些包括预定义的字符集表、停用词、manticore 模块和 icu 数据文件，所有这些都放在该文件夹中。配置脚本通常将此路径确定为类似于 `/usr/share/manticore` 或 `/usr/local/share/manticore`。
- **DISTR_BUILD** - 发布包选项的快捷方式。这是一个字符串值，包含目标平台的名称。它可以用于替代手动配置所有选项。在 Debian 和 Redhat Linux 系统中，默认值可能通过轻量级的自省确定并设置为通用的 'Debian' 或 'RHEL'。否则，该值是未定义的。
- **PACK** - 一个更方便的快捷方式。它读取 `DISTR` 环境变量，将其分配给 **DISTR_BUILD** 参数，然后正常工作。这在像 Docker 容器这样的准备好构建的系统中非常有用，因为 `DISTR` 变量是在系统级别设置的，并且反映了容器所针对的目标系统。
- **CMAKE_INSTALL_PREFIX** (路径) - Manticore 预计将要安装的位置。构建不执行任何安装，但它准备在您运行 `cmake --install` 命令或创建一个包然后安装时执行的安装规则。该前缀可以在任何时间更改，甚至在安装过程中，通过调用
  `cmake --install . --prefix /path/to/installation`。但是，在配置时，此变量用于初始化 `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。例如，在配置时将其设置为 `/my/custom`
  时间将固定 `LOCALDATADIR` 为 `/my/custom/var/lib/manticore/data`，并且 `FULL_SHARE_DIR` 为
  `/my/custom/usr/share/manticore`。
- **BUILD_TESTING** (布尔) 是否支持测试。如果启用，在构建后，您可以运行 'ctest' 测试构建。请注意，测试意味着增加依赖关系，例如至少需要 PHP cli、Python 和可用的带有测试数据库的 MySQL 服务器。默认情况下，此参数是开启的。因此，对于 '仅构建'，您可能希望通过显式指定 'off' 值来禁用该选项。
- **LIBS_BUNDLE** - 具有不同库的文件夹路径。这主要与 Windows 构建相关，但如果您经常构建以避免每次下载第三方源，也可能会有所帮助。默认情况下，配置脚本不会修改该路径；您应手动将所有内容放在那里。当我们想要支持一个词干提取器时 - 源将从 Snowball 主页下载，然后提取、配置、构建等。相反，您可以将原始源 tarball（即 `libstemmer_c.tgz`）存储在该文件夹中。下次您想从头开始构建时，配置脚本将首先在捆绑包中查找，如果在那里找到词干提取器，则不会再次从 Internet 下载它。
- **CACHEB** - 存储第三方库构建的文件夹路径。通常像 galera、re2、icu 等功能首先被下载或从捆绑包中获取，然后解压、构建并安装到临时内部文件夹。当构建 manticore 时，该文件夹随后被用作支持所请求功能所需内容的地方。最后，它们要么链接到 manticore（如果它是一个库）；要么直接进入分发/安装（如 galera 或 icu 数据）。当 **CACHEB** 被定义为 cmake 配置参数或作为系统环境变量时，它被用作该构建的目标文件夹。此文件夹可以在构建之间保留，这样存储的库就不会再被重建，从而使整个构建过程大大缩短。


注意，有些选项是以三元组的形式组织的： `WITH_XXX`， `DL_XXX` 和 `XXX_LIB` ——比如支持 mysql、odbc 等。 `WITH_XXX` 决定接下来的两个选项是否有效。也就是说，如果你将 `WITH_ODBC` 设置为 `0`，那么提供 `DL_ODBC` 和 `ODBC_LIB` 就没有意义，如果整个功能被禁用，这两个选项将没有效果。此外，没有 `DL_XXX` 的 `XXX_LIB` 也是没有意义的，因为如果你不想要 `DL_XXX` 选项，动态加载将不会被使用，并且 `XXX_LIB` 提供的名称是无用的。这是默认的自省所使用的。

此外，使用 `iconv` 库需要 `expat` ，如果最后一个被禁用则是无用的。

另外，有些库可能始终可用，因此，避免与它们链接是没有意义的。例如，在 Windows 上，这是 ODBC。在 macOS 上，这是 Expat、iconv 和其他库。默认自省确定这些库，并有效地只发出 `WITH_XXX`，而不包含 `DL_XXX` 和 `XXX_LIB`，这使事情变得更简单。

在游戏配置中，某些选项可能看起来像：

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

除了一般配置值外，你还可以查阅在你运行配置后留下的 `CMakeCache.txt` 文件，该文件位于构建文件夹中。那里定义的任何值都可以在运行 cmake 时显式重新定义。例如，你可以运行 `cmake -DHAVE_GETADDRINFO_A=FALSE ...`，这个配置运行将不会假设该变量的调查值，而是使用你提供的值。

#### 特定环境变量

环境变量对于提供某种全球设置是有用的，这些设置存储在构建配置之外，并且始终存在。为了持久性，它们可以通过不同方式在系统上全局设置 - 例如，添加到 `.bashrc` 文件中，或者如果你生产基于 Docker 的构建系统，则将它们嵌入到 Dockerfile 中，或者在 Windows 上的系统偏好环境变量中写入它们。此外，你还可以通过在 shell 中使用 `export VAR=value` 设置它们为短期的。或者更简单，通过在 cmake 调用前添加值，例如 `CACHEB=/my/cache cmake ...` - 这样它只在这次调用中有效，并且在下次调用时不可见。

这些变量中有一些是 cmake 和其他一些工具普遍使用的。比如 `CXX` 用于确定当前的 C++ 编译器，或者 `CXX_FLAGS` 用于提供编译器标志等。

然而，我们有一些特定于 manticore 配置的变量，它们是专门为我们的构建而发明的。

- **CACHEB** - 与配置 **CACHEB** 选项相同
- **LIBS_BUNDLE** - 与配置 **LIBS_BUNDLE** 选项相同
- **DISTR** - 用于在使用 `-DPACK=1` 时初始化 `DISTR_BUILD` 选项。
- **DIAGNOSTIC** - 使 cmake 配置的输出更加详细，解释发生的一切
- **WRITEB** - 假设 **LIBS_BUNDLE**，并且如果设置，将为不同工具下载源归档文件到 LIBS_BUNDLE 文件夹。也就是说，如果一个新版本的 stemmer 发布，你可以手动从包中删除 libstemmer_c.tgz，然后运行一次性 `WRITEB=1 cmake ...` - 它将找不到包中的 stemmer 源，然后将从供应商网站下载到包中（如果没有 WRITEB，它将下载到构建内部的临时文件夹中，并在你清空构建文件夹时消失）。

在配置结束时，你可以看到可用内容，并将其以如下列表形式使用：

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

要安装到自定义（非默认）文件夹中，请运行

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### 打包构建

要构建一个包，请使用目标 `package`。它将根据 `-DDISTR_BUILD` 选项提供的选择构建包。默认情况下，它将是一个简单的 .zip 或 .tgz 归档，包含所有二进制文件和附加文件。

```bash
cmake --build . --target package --config RelWithDebInfo
```

## 关于构建的一些高级内容

### 单配置下的重新编译（更新）

如果你没有更改源和构建的路径，只需移动到你的构建文件夹并运行：

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

如果由于某种原因它不起作用，你可以删除位于构建文件夹中的 `CMakeCache.txt` 文件。在这一步之后你
必须再次运行 cmake，指向源文件夹并配置选项。

如果这也没有帮助，只需清空你的构建文件夹并从头开始。

### 构建类型

简言之 - 只需使用 `--config RelWithDebInfo`，如上所述。这不会出错。

我们使用两种构建类型。对于开发，使用 `Debug` - 它以一种非常适合开发的方式分配优化和其他事物的编译器标志，这意味着调试运行是逐步执行的。然而，生成的二进制文件在生产中相当大且慢。

对于发布，我们使用另一种类型 - `RelWithDebInfo` - 意思是“带调试信息的发布构建”。它生成带有嵌入调试信息的生产二进制文件。后者随后被分拆到单独的 debuginfo 包中，这些包与发布包一起存储，可以在发生崩溃等问题时用于调查和修复错误。cmake 还提供 `Release` 和 `MinSizeRel` ，但我们不使用它们。如果构建类型不可用，cmake 将进行 `noconfig` 构建。

#### 构建系统生成器

有两种类型的生成器：单配置和多配置。

- 单配置在配置期间需要提供构建类型，通过 `CMAKE_BUILD_TYPE` 参数。如果未定义，构建将回退到 `RelWithDebInfo` 类型，适合你仅想从源代码构建 Manticore 而不是参与开发。对于显式构建，你应该提供构建类型，例如 `-DCMAKE_BUILD_TYPE=Debug`。
- 多配置在构建期间选择构建类型。它应该通过 `--config` 选项提供，否则它将构建一种 `noconfig`，这是不理想的。因此，你应该始终指定构建类型，例如 `--config Debug`。

如果您想指定构建类型，但又不想关心是'单个'还是'多重'配置生成器 - 只需在两个地方提供必要的键。即，使用`-DCMAKE_BUILD_TYPE=Debug`进行配置，然后使用`--config Debug`进行构建。只需确保这两个值相同。如果目标构建器是单配置，它将消耗配置参数。如果是多配置，配置参数将被忽略，但将通过`--config`键选择正确的构建配置。

如果您想要`RelWithDebInfo`（即仅为生产构建）并知道您在单配置平台上（即除了Windows的所有平台） - 您可以在cmake调用时省略`--config`标志。然后将配置默认的`CMAKE_BUILD_TYPE=RelWithDebInfo`并使用它。所有针对'构建'、'安装'和'构建包'的命令将变得更简短。

#### 显式选择构建系统生成器

Cmake是一个不自己进行构建的工具，但它为本地构建系统生成规则。
通常，它能够很好地确定可用的构建系统，但有时您可能需要显式提供生成器。您
可以运行`cmake -G`并查看可用生成器的列表。

- 在Windows上，如果您安装了多个版本的Visual Studio，您可能需要指定要使用的版本，
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
Ninja多配置非常有用，因为它确实是'多配置'并且可以在Linux/macOS/BSD上使用。使用此生成器，您可以将构建时的配置类型选择移到构建时，并且您可以在同一个构建文件夹中构建多个配置，只需更改`--config`参数。

### 注意事项

1. 如果您想最终构建一个功能齐全的RPM包，构建目录的路径必须足够长，以便正确构建调试符号。
例如，像`/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`。这是因为RPM工具在构建调试信息时会修改编译二进制文件的路径，并且它只能覆盖现有空间，而不会分配更多空间。上述长路径有100个字符，这对于这种情况来说是足够的。

## 外部依赖

如果您想使用某些库，它们应该是可用的。
- 对于索引（`indexer`工具）：`expat`、`iconv`、`mysql`、`odbc`、`postgresql`。没有它们，您只能处理`tsv`和`csv`源。
- 对于服务查询（`searchd`守护进程）：`openssl`可能是必要的。
- 对于所有（必需，强制性！）我们需要Boost库。最小版本为1.61.0，但我们使用更新版本1.75.0构建二进制文件。更近期的版本（如1.76）也应该可以。在Windows上，您可以从其网站（boost.org）下载预构建的Boost，并安装到默认建议的路径（即`C:\\boost...`）。在MacOs上，brew提供的版本可以。在Linux上，您可以检查官方仓库中可用的版本，如果不符合要求，可以从源代码构建。我们需要'context'组件，您还可以构建'系统'和'程序选项'组件，如果您还想从源代码构建Galera库，它们将是必要的。请查看`dist/build_dockers/xxx/boost_175/Dockerfile`，了解如何操作的简短自我文档脚本/说明。

在构建系统上，您需要安装这些软件包的'dev'或'devel'版本（即 - libmysqlclient-devel，unixodbc-devel等。请查看我们的dockerfiles以获取具体软件包的名称）。

在运行系统上，这些软件包至少应存在于最终（非开发）版本中。（开发版本通常较大，因为它们不仅包括目标二进制文件，还包括不同的开发内容，如包含头文件等）。

### 在Windows上构建

除了必要的先决条件外，您可能还需要预构建的`expat`、`iconv`、`mysql`和`postgresql`客户端库。您必须自己构建它们，或者与我们联系以获取我们的构建包（一个简单的zip档案，其中包含这些目标的文件夹）。

- ODBC不是必需的，因为它是一个系统库。
- OpenSSL可以从源代码构建或从https://slproweb.com/products/Win32OpenSSL.html下载预构建版本（如在寻找OpenSSL的cmake内部脚本中所提到的）。
- Boost可以从https://www.boost.org/发布中下载预构建版本。

### 查看编译了什么

运行`indexer -h`。它将显示哪种特性已被配置和构建（无论是显式的还是调查的，都没关系）：

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->

