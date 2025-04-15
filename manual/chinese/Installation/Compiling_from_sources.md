# 从源代码编译 Manticore

从源代码编译 Manticore Search 使得可以进行自定义构建配置，例如禁用某些功能或添加新的补丁进行测试。例如，您可能希望从源代码编译并禁用嵌入的 ICU，以便使用您系统上独立安装并可以升级的不同版本。这在您有兴趣为 Manticore Search 项目作出贡献时也非常有用。

## 使用 CI Docker 构建
为了准备 [官方发布和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和一个特殊的构建镜像。此镜像包括必要的工具，并旨在与外部 sysroots 一起使用，因此一个容器可以为所有操作系统构建软件包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 来构建镜像，或使用来自 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 的镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，您还需要指定以下环境变量：

* `DISTR`: 目标平台: `bionic`, `focal`, `jammy`, `buster`, `bullseye`, `bookworm`, `rhel7`, `rhel8`, `rhel9`, `macos`, `windows`, `freebsd13`
* `arch`: 架构: `x86_64`, `x64`（用于 Windows），`aarch64`, `arm64`（用于 Macos）
* `SYSROOT_URL`: 指向系统根目录存档的 URL。除非您自己构建 sysroots（说明可以在 [这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots) 找到），否则可以使用 https://repo.manticoresearch.com/repository/sysroots 。
* 使用 CI 工作流文件作为参考以查找您可能需要使用的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

要查找 `DISTR` 和 `arch` 的可能值，您可以使用目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/ 作为参考，因为它包含所有支持组合的 sysroots。

之后，在 Docker 容器内构建软件包就像调用以下命令一样简单：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，要为 Ubuntu Jammy 创建与 Manticore Core Team 提供的官方版本类似的软件包，您应在包含 Manticore Search 源代码的目录中执行以下命令。此目录是从 https://github.com/manticoresoftware/manticoresearch 克隆的代码库的根目录：

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

# 以下是在 docker shell 中运行的
cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/
mkdir build && cd build
cmake -DPACK=1 ..
export CMAKE_TOOLCHAIN_FILE=$(pwd)/dist/build_dockers/cross/linux.cmake
cmake --build .
# 或者如果您想构建软件包：
# cmake --build . --target package
```
长源目录路径是必需的，否则在某些情况下（例如 Centos）可能会构建失败。

相同的过程可以用来构建不仅适用于流行 Linux 发行版的二进制文件/软件包，还适用于 FreeBSD、Windows 和 macOS。

## 手动构建

在不使用构建 Docker 的情况下编译 Manticore **不推荐**，但如果您需要这样做，以下是您需要知道的内容：

### 必需工具

* C++ 编译器
  * 在 Linux - 可以使用 GNU（4.7.2 及以上）或 Clang
  * 在 Windows - Microsoft Visual Studio 2019 及以上（社区版即可）
  * 在 macOS - Clang（来自 XCode 的命令行工具，请使用 `xcode-select --install` 进行安装）。
* Bison、Flex - 在大多数系统上，它们作为软件包提供，在 Windows 中可在 cygwin 框架中找到。
* Cmake - 在所有平台上使用（需要版本 3.19 或更高）

### 获取源代码

#### 从 git

Manticore 源代码 [托管在 GitHub](https://github.com/manticoresoftware/manticoresearch)。   
要获取源代码，请克隆代码库，然后检出所需的分支或标签。`master` 分支表示主要开发分支。在发布时，会创建一个版本标签，例如 `3.6.0`，并为当前版本启动一个新分支，在这种情况下为 `manticore-3.6.0`。在所有更改之后版本分支的头用于构建所有二进制版本。例如，要获取版本 3.6.0 的源代码，您可以运行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 从存档

您可以通过使用 "Download ZIP" 按钮从 GitHub 下载所需的代码。 .zip 和 .tar.gz 格式都是合适的。

```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```

### 配置

Manticore 使用 CMake。假设您在克隆的代码库的根目录内：

```bash
mkdir build && cd build
cmake ..
```

CMake 将检查可用的特性，并根据它们配置构建。默认情况下，如果特性可用，则视为已启用。该脚本还会下载并构建一些外部库，假设你想使用它们。隐式地，你将获得最多特性支持。

你也可以通过标志和选项显式配置构建。  要启用特性 `FOO`，请在 CMake 调用中添加 `-DFOO=1`。
要禁用它，请使用 `-DFOO=0`。如果没有明确说明，启用一个不可用的特性（例如在 MS Windows 构建中使用 `WITH_GALERA`）将导致配置错误并失败。禁用一个特性不仅会将其从构建中排除，还会禁用系统对其的检查以及相关外部库的下载/构建。

#### Configuration flags and options

- **USE_SYSLOG** - 允许在 [query logging](Logging/Query_logging.md) 中使用`syslog`.
- **WITH_GALERA** - 启用搜索守护进程复制支持。构建时将配置复制支持，并下载、构建 Galera 库的源代码，并将其包含在发行版/安装包中。通常，使用 Galera 进行构建是安全的，但不分发该库本身（因此没有 Galera 模块，也没有复制功能）。然而，有时你可能需要显式禁用它，例如当你想构建一个设计上不能加载任何库的静态二进制文件，即使守护进程内存在对 'dlopen' 函数的调用也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库进行构建。这对于诸如 [REGEX()](../Functions/String_functions.md#REGEX%28%29) 这样的函数以及 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 功能是必需的.
  功能.
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 的源代码，编译它们，并进行静态链接，使得最终的二进制文件不依赖于系统中共享的 `RE2` 库。
- **WITH_STEMMER** - 使用 Snowball 词干提取库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 的源代码，编译它们，并进行静态链接，使得最终的二进制文件不依赖于系统中共享的 `libstemmer` 库。
- **WITH_ICU** -  使用 ICU (International Components for Unicode) 库进行构建。它用于对中文文本进行分词。当 morphology=`icu_chinese` 被使用时，将启用此功能。
- **WITH_JIEBA** -  使用结巴中文分词工具进行构建。它用于对中文文本进行分词。当 morphology=`jieba_chinese` 被使用时，将启用此功能。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 的源代码，编译它们，并进行静态链接，使得最终的二进制文件不依赖于系统中共享的 `icu` 库。同时将 ICU 数据文件包含在安装/发行版中。静态链接 ICU 的目的是获得已知版本的库，从而行为确定，不依赖于任何系统库。你很可能会更倾向于使用系统 ICU，因为它可能会随时间更新而无需重新编译 Manticore 守护进程。在这种情况下，你需要显式禁用此选项。这也将节省一些由 ICU 数据文件占用的空间（约 30M），因为它不会包含在发行版中。
- **WITH_SSL** - 用于支持 HTTPS 以及守护进程的加密 MySQL 连接。系统的 OpenSSL 库将会链接到守护进程。这意味着启动守护进程时需要 OpenSSL。此选项对支持 HTTPS 是必需的，但对服务器来说并非严格必需（即没有 SSL 意味着无法通过 HTTPS 连接，但其他协议仍然可用）。Manticore 可以使用从 1.0.2 到 1.1.1 的 SSL 库版本，但请注意 **出于安全考虑，强烈建议使用最新的 SSL
  库**。目前仅支持 v1.1.1，其余版本均已过时 (
  参见 [openssl release strategy](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - 被 indexer 用于处理 MySQL 压缩列，并被守护进程用于支持压缩 MySQL 协议。
- **WITH_ODBC** - 被 indexer 用于支持索引来自 ODBC 提供者（通常是 UnixODBC 和 iODBC）的数据源。在 MS Windows 上，ODBC 是处理 MS SQL 数据源的正确方式，因此索引 `MSSQL` 也意味着启用此标志。
- **DL_ODBC** - 不与 ODBC 库链接。如果 ODBC 被链接但不可用，即使你想处理与 ODBC 无关的内容，也无法启动 indexer 工具。此选项要求 indexer 仅在需要处理 ODBC 数据源时在运行时加载该库。
- **ODBC_LIB** - ODBC 库文件的名称。当你需要处理 ODBC 数据源时，indexer 将尝试加载该文件。此选项将根据可用的 ODBC 共享库自动生成。你也可以在运行 indexer 之前，通过提供环境变量 `ODBC_LIB` 以及指向替代库的正确路径来覆盖该名称。
- **WITH_EXPAT** - 被 indexer 用于支持索引 xmlpipe 数据源。
- **DL_EXPAT** - 不与 EXPAT 库链接。如果 EXPAT 被链接但不可用，即使你想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求 indexer 仅在需要处理 xmlpipe 数据源时在运行时加载该库。
- **EXPAT_LIB** - EXPAT 库文件的名称。当你需要处理 xmlpipe 数据源时，indexer 将尝试加载该文件。此选项将根据可用的 EXPAT 共享库自动生成。你也可以在运行 indexer 之前，通过提供环境变量 EXPAT_LIB 以及指向替代库的正确路径来覆盖该名称。
- **WITH_ICONV** - 用于支持在索引 xmlpipe 源时使用不同编码的索引器。
- **DL_ICONV** - 不与 iconv 库链接。如果 iconv 已链接但不可用，即使您希望处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在您希望处理 xmlpipe 源时在运行时加载该库。
- **ICONV_LIB** - iconv 库文件的名称。当您希望处理 xmlpipe 源时，索引器将尝试加载该文件。此选项是通过对可用的 iconv 共享库进行调查自动写入的。您还可以通过在运行索引器之前提供环境变量 `ICONV_LIB` 并指定一个替代库的正确路径来覆盖该名称。
- **WITH_MYSQL** - 索引器用于支持索引 MySQL 源。
- **DL_MYSQL** - 不与 MySQL 库链接。如果 MySQL 已链接但不可用，即使您希望处理与 MySQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在您希望处理 MySQL 源时在运行时加载该库。
- **MYSQL_LIB** -- MySQL 库文件的名称。当您希望处理 MySQL 源时，索引器将尝试加载该文件。此选项是通过对可用的 MySQL 共享库进行调查自动写入的。您还可以通过在运行索引器之前提供环境变量 `MYSQL_LIB` 并指定一个替代库的正确路径来覆盖该名称。
- **WITH_POSTGRESQL** - 索引器用于支持索引 PostgreSQL 源。
- **DL_POSTGRESQL** - 不与 PostgreSQL 库链接。如果 PostgreSQL 已链接但不可用，即使您希望处理与 PostgreSQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在您希望处理 PostgreSQL 源时在运行时加载该库。
- **POSTGRESQL_LIB** - postgresql 库文件的名称。当处理 PostgreSQL 源时，索引器将尝试加载指定的 PostgreSQL 库文件。此选项是通过对可用的 PostgreSQL 共享库进行调查自动确定的。您还可以通过在运行索引器之前提供环境变量 `POSTGRESQL_LIB` 并指定一个替代库的正确路径来覆盖该名称。
- **LOCALDATADIR** - 守护进程存储 binlogs 的默认路径。如果未提供此路径或在守护进程的运行时配置中明确禁用（即文件 `manticore.conf`，与此构建配置无关），binlogs 将放置在此路径中。它通常是一个绝对路径，但并不要求是，也可以使用相对路径。您可能不需要更改配置定义的默认值，这个值可能会根据目标系统而有所不同，例如 `/var/data`、`/var/lib/manticore/data` 或 `/usr/local/var/lib/manticore/data`。
- **FULL_SHARE_DIR** - 存储所有资产的默认路径。可以通过在启动任何使用该文件夹中的文件的工具之前设置环境变量 `FULL_SHARE_DIR` 来覆盖此路径。这是一个重要路径，因为许多内容默认期望在此找到。这些包括预定义的字符集表、停用词、manticore 模块和 icu 数据文件，全部放置在该文件夹中。配置脚本通常确定该路径为像 `/usr/share/manticore` 或 `/usr/local/share/manticore` 这样的值。
- **DISTR_BUILD** - 释放包选项的快捷方式。这是一个字符串值，表示目标平台的名称。可以用来替代手动配置所有选项。在 Debian 和 Redhat Linux 上，默认值可能会通过轻度自省来确定并设置为通用的 'Debian' 或 'RHEL'。否则，值未定义。
- **PACK** - 一个更方便的快捷方式。它读取 `DISTR` 环境变量，将其分配给 **DISTR_BUILD** 参数，然后像往常一样工作。在准备好的构建系统中构建时，这非常有用，比如 Docker 容器，其中 `DISTR` 变量在系统级别设置并反映容器所针对的目标系统。
- **CMAKE_INSTALL_PREFIX** (path) - 预计 Manticore 将安装的位置。构建不会执行任何安装，但它准备在您运行 `cmake --install` 命令或创建包并随后安装时执行的安装规则。可以在任何时间改变前缀，即使在安装过程中，也可以通过调用
  `cmake --install . --prefix /path/to/installation` 来实现。然而，在配置时，此变量用于初始化 `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。例如，在配置时将其设置为 `/my/custom`
  将硬编码 `LOCALDATADIR` 为 `/my/custom/var/lib/manticore/data`，并将 `FULL_SHARE_DIR` 硬编码为
  `/my/custom/usr/share/manticore`。
- **BUILD_TESTING** (bool) 是否支持测试。如果启用，在构建后，您可以运行 'ctest' 以测试构建。请注意，测试意味着额外的依赖项，如至少需要 PHP cli、Python 和可用的 MySQL 服务器以及测试数据库。默认情况下，此参数为开启状态。因此，对于“仅构建”，您可能希望通过明确指定“off”值来禁用该选项。
- **LIBS_BUNDLE** - 指向一个包含不同库的文件夹的路径。这主要与Windows构建相关，但如果您需要经常构建，以避免每次都下载第三方源，这也是有帮助的。默认情况下，此路径不会被配置脚本修改；您应该手动将所有内容放入此处。当我们想要支持一个词干分析器时——源代码将从Snowball主页下载，然后提取、配置、构建等。相反，您可以将原始源归档（即`libstemmer_c.tgz`）存储在此文件夹中。下次您想要从头开始构建时，配置脚本将首先在捆绑包中查找，如果在那里找到词干分析器，它将不会再次从互联网上下载。
- **CACHEB** - 指向一个包含第三方库的存储构建的文件夹的路径。通常像galera、re2、icu等特性首先从捆绑包中下载或获取，然后解压、构建，并安装到一个临时内部文件夹中。构建manticore时，该文件夹将作为支持所请求功能所需文件的存放位置。最后，它们要么与manticore链接（如果它是一个库）；要么直接进入分发/安装（例如galera或icu数据）。当**CACHEB**被定义为cmake配置参数或系统环境变量时，它被用作该构建的目标文件夹。此文件夹可以在构建之间保持，因此那里存储的库将不再重新构建，从而使整个构建过程缩短。

注意，一些选项以三元组的形式组织：`WITH_XXX`、`DL_XXX`和`XXX_LIB`——例如mysql、odbc等的支持。`WITH_XXX`决定后面两个是否有效。换句话说，如果将`WITH_ODBC`设置为`0`——那么提供`DL_ODBC`和`ODBC_LIB`就没有意义，如果整个功能被禁用，这两个也将没有效果。另外，`XXX_LIB`没有`DL_XXX`就没有意义，因为如果您不想要`DL_XXX`选项，则不会使用动态加载，`XXX_LIB`提供的名称也是无用的。这是默认的自省方式。

此外，使用`iconv`库假定有`expat`，如果后者被禁用则无用。

此外，某些库可能始终可用，因此避免与它们链接是没有意义的。例如，在Windows上是ODBC。在macOS上是Expat、iconv以及其他可能的库。默认自省将确定这些库，并有效地只为它们发出`WITH_XXX`，不包括`DL_XXX`和`XXX_LIB`，这样使事情变得更简单。

在某些选项设置中，配置可能看起来像：

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

除了常规配置值，您还可以调查文件`CMakeCache.txt`，该文件在您运行配置后会保留在构建文件夹中。任何在其中定义的值在运行cmake时可以被显式重定义。例如，您可以运行`cmake -DHAVE_GETADDRINFO_A=FALSE ...`，这个配置运行将不会假设该变量的调查值，而是将使用您提供的值。

#### 特定环境变量

环境变量对于提供某种全局设置非常有用，这些设置与构建配置分开存储并且始终存在。为了持久性，它们可以通过多种方式全局设置在系统中——例如，将它们添加到`.bashrc`文件中，或者如果您生成基于Docker的构建系统，则将它们嵌入到Dockerfile中，或者在Windows系统首选项环境变量中写入它们。另外，您可以使用`export VAR=value`在Shell中设置它们为短期生存。甚至更短，通过在cmake调用前添加值，例如`CACHEB=/my/cache cmake ...`——这样它将仅在此调用中工作，在下次调用中将不可见。

其中一些变量被cmake和其他一些工具普遍使用。比如`CXX`决定当前的C++编译器，或者`CXX_FLAGS`用于提供编译器标志等。

然而，我们有一些特定于manticore配置的变量，它们是专门为我们的构建而发明的。

- **CACHEB** - 与配置**CACHEB**选项相同
- **LIBS_BUNDLE** - 与配置**LIBS_BUNDLE**选项相同
- **DISTR** - 用于在使用`-DPACK=1`时初始化`DISTR_BUILD`选项。
- **DIAGNOSTIC** - 使cmake配置的输出更加详细，解释发生的一切
- **WRITEB** - 假定**LIBS_BUNDLE**，如果设置，将为不同工具下载源归档文件到LIBS_BUNDLE文件夹。也就是说，如果有一个新版本的词干分析器发布——您可以手动删除捆绑包中的libstemmer_c.tgz，然后运行一次性的`WRITEB=1 cmake ...`——它将找不到捆绑包中的词干分析器源，然后将从供应商网站下载到捆绑包（如果没有WRITEB，它将下载到构建内的临时文件夹，并在您清空构建文件夹时消失）。

在配置结束时，您可能会看到可用的并将被使用的功能列表，如下所示：

```
-- 已启用的编译功能：
* Galera，表的复制
* re2，一个正则表达式库
* stemmer，词干分析库（Snowball）
* icu，国际化组件（Unicode）
* OpenSSL，用于加密网络
* ZLIB，用于压缩数据和网络
* ODBC，用于索引MSSQL（Windows）和通用ODBC源的索引器
* EXPAT，用于索引xmlpipe源的索引器
* Iconv，用于在索引xmlpipe源时支持不同的编码的索引器
* MySQL，用于索引MySQL源的索引器
* PostgreSQL，用于索引PostgreSQL源的索引器
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
- 对于所有（必须的，强制的！）我们需要 Boost 库。最低版本为 1.61.0，然而，我们使用更新的版本 1.75.0 构建二进制文件。更近期的版本（如 1.76）也应该可以。在 Windows 上，您可以从他们的网站（boost.org）下载预构建的 Boost 并安装到默认建议的路径（即 `C:\\boost...`）。在 MacOs 上，brew 中提供的版本可以使用。在 Linux 上，您可以检查官方存储库中可用的版本，如果不符合要求，您可以从源代码进行构建。我们需要组件 'context'，您还可以构建组件 'system' 和 'program_options'，如果您还想从源代码构建 Galera 库，它们将是必需的。请查看 `dist/build_dockers/xxx/boost_175/Dockerfile` 获取关于如何执行的简短自文档化脚本/说明。

在构建系统中，您需要安装这些软件包的 'dev' 或 'devel' 版本（即 - libmysqlclient-devel, unixodbc-devel 等。请查看我们的 dockerfile 以获取具体软件包的名称）。

在运行系统中，这些软件包至少应该出现在最终（非开发）版本中。（devel 版本通常更大，因为它们不仅包含目标二进制文件，还包含不同的开发内容，如头文件等）。

### 在 Windows 上构建

除了必要的前提条件，您可能需要预构建的 `expat`、`iconv`、`mysql` 和 `postgresql` 客户端库。您必须自己构建它们，或者联系我以获取我们的构建包（一个简单的 zip 归档，其中包含这些目标的文件夹）。

- ODBC 不是必须的，因为它是一个系统库。
- OpenSSL 可以从源代码构建，或从 https://slproweb.com/products/Win32OpenSSL.html 下载预构建的版本（如在 FindOpenSSL 的 cmake 内部脚本中所述）。
- Boost 可以从 https://www.boost.org/ releases 下载预构建的版本。

### 查看已编译内容

运行 `indexer -h`。它将显示哪些功能已配置和构建（无论它们是显式的还是经过调查的，均无所谓）：

```
在 Linux x86_64 上由 GNU 8.3.1 编译器构建。

使用这些定义配置： -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->
























