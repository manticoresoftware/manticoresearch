# 从源码编译 Manticore

从源码编译 Manticore Search 可以实现自定义构建配置，例如禁用某些功能或添加新补丁进行测试。例如，您可能希望从源码编译并禁用内嵌的 ICU，以便使用系统上安装的不同版本，该版本可以独立于 Manticore 升级。如果您有兴趣为 Manticore Search 项目做贡献，这也很有用。

## 使用 CI Docker 构建
为了准备[官方发布和开发包](https://repo.manticoresearch.com/)，我们使用 Docker 和特殊的构建镜像。该镜像包括必需的工具，设计成与外部 sysroots 一起使用，因此一个容器可以为所有操作系统构建包。您可以使用 [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) 和 [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) 构建镜像，或者使用 [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags) 上的镜像。这是为任何支持的操作系统和架构创建二进制文件的最简单方法。运行容器时，您还需要指定以下环境变量：

* `DISTR`：目标平台：`bionic`、`focal`、`jammy`、`buster`、`bullseye`、`bookworm`、`rhel7`、`rhel8`、`rhel9`、`rhel10`、`macos`、`windows`、`freebsd13`
* `arch`：架构：`x86_64`、`x64`（针对 Windows）、`aarch64`、`arm64`（针对 macOS）
* `SYSROOT_URL`：系统根目录归档的 URL。除非您自己构建 sysroots（说明可见[这里](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)），否则可以使用 https://repo.manticoresearch.com/repository/sysroots 。
* 您可以参考 CI 工作流程文件，查找可能需要使用的其他环境变量：
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

若要查找 `DISTR` 和 `arch` 的可能值，您可以参考目录 https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/ ，它包含所有支持组合的 sysroots。

然后，在 Docker 容器内构建包只需调用：

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

例如，要为 Ubuntu Jammy 创建一个类似官方版本 Manticore 核心团队提供的包，应在包含 Manticore Search 源代码的目录中执行以下命令。该目录是从 https://github.com/manticoresoftware/manticoresearch 克隆的仓库根目录：

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

# 以下命令在 docker shell 内执行
cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/
mkdir build && cd build
cmake -DPACK=1 ..
export CMAKE_TOOLCHAIN_FILE=$(pwd)/dist/build_dockers/cross/linux.cmake
cmake --build .
# 如果要构建包：
# cmake --build . --target package
```
仓库源码目录路径较长是必需的，否则在某些情况下（如 Centos）可能构建失败。

以同样方式，您不仅可以为流行的 Linux 发行版构建二进制文件或包，还可以为 FreeBSD、Windows 和 macOS 构建。

#### 使用 Docker 构建 SRPM

您也可以使用相同的特殊 Docker 镜像构建 SRPM：

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

# 以下命令在 docker shell 内执行
cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/
mkdir build && cd build
cmake -DPACK=1 ..
export CMAKE_TOOLCHAIN_FILE=$(pwd)/../dist/build_dockers/cross/linux.cmake
# 现在在构建目录生成了 CPackSourceConfig.cmake 文件
cpack -G RPM --config ./CPackSourceConfig.cmake
```

这将生成包含所有源代码的源 RPM（`.src.rpm` 文件）。

#### 使用 SRPM 构建二进制 RPM

生成 SRPM 后，您可以使用它构建完整的二进制 RPM 包集合：

```bash
# 安装构建工具和依赖
dnf install -y rpm-build cmake gcc-c++ boost-devel epel-release

# 自动安装 SRPM 依赖
dnf builddep -y manticore-*.src.rpm

# 从 SRPM 构建所有二进制 RPM
rpmbuild --rebuild manticore-*.src.rpm

# 查找生成的包
ls ~/rpmbuild/RPMS/*/manticore*
```

> 注意：**要从 SRPM 构建 RPM，您需要确保 SRPM 中列出的所有依赖项都已完全安装，这可能很具挑战性。** SRPM 仍然可以用于：
> - 审核构建过程或检查源文件和规格文件
> - 对构建进行自定义修改或补丁
> - 理解二进制文件是如何生成的
> - 满足开源许可证合规要求

## 手动构建

在不使用构建 Docker 的情况下编译 Manticore **不推荐**，但如果您需要这样做，这里是您可能需要知道的事项：

### 所需工具

* C++ 编译器
  * 在 Linux 中 - 可以使用 GNU（4.7.2 及以上）或 Clang
  * 在 Windows 中 - Microsoft Visual Studio 2019 及以上（社区版即可）
  * 在 macOS 上 - Clang（来自 XCode 的命令行工具，使用 `xcode-select --install` 安装）。
* Bison、Flex - 在大多数系统上，它们作为软件包可用，在 Windows 上它们可以在 Cygwin 框架中找到。
* Cmake - 在所有平台上使用（需要版本 3.19 或以上）

### 获取源代码

#### 从 Git 获取

Manticore 源代码托管在 [GitHub](https://github.com/manticoresoftware/manticoresearch) 上。
要获取源代码，请克隆该仓库，然后检查所需的分支或标签。`master` 分支表示主要开发分支。发布时，会创建一个版本标签，例如 `3.6.0`，并为当前版本启动一个新分支，在本例中为 `manticore-3.6.0`。在所有更改完成后，版本分支的头部用于构建所有二进制发布。例如，要获取版本 3.6.0 的源代码，您可以运行：

```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```

#### 从档案获取

您可以通过使用“下载 ZIP”按钮从 GitHub 下载所需的代码。 .zip 和 .tar.gz 格式均可。

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

CMake 将调查可用的功能并根据它们配置构建。默认情况下，所有可用的功能都被认为是启用的。该脚本还会下载和构建一些外部库，假设您想要使用它们。隐式地，您将获得对最大数量功能的支持。

您还可以通过标志和选项显式配置构建。要启用特性 `FOO` 请将 `-DFOO=1` 添加到 CMake 调用中。
要禁用它，请使用 `-DFOO=0`。如果没有明确说明，启用一个不可用的功能（例如在 MS Windows 构建中使用 `WITH_GALERA`）将导致配置失败并报错。禁用一个功能，除了将其排除在构建之外，还会在系统上禁用其调查，并禁用任何相关外部库的下载/构建。

#### 配置标志和选项

- **USE_SYSLOG** - 允许在[查询日志记录](../Logging/Query_logging.md)中使用 `syslog`。
- **WITH_GALERA** - 启用搜索守护程序的复制支持。将为构建配置支持，并下载 Galera 库的源代码，构建并包含在分发/安装中。通常，使用 Galera 进行构建是安全的，但不分发库本身（因此没有 Galera 模块，没有复制）。但是，有时您可能需要显式禁用它，例如如果您想构建一个设计上无法加载任何库的静态二进制文件，这样即使守护程序内部调用 `dlopen` 函数的存在也会导致链接错误。
- **WITH_RE2** - 使用 RE2 正则表达式库进行构建。这对于像 [REGEX()](../Functions/String_functions.md#REGEX%28%29) 和 [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
  特性非常必要。
- **WITH_RE2_FORCE_STATIC** - 下载 RE2 的源代码，进行编译，并静态链接，因此最终的二进制文件将不依赖于系统中存在的共享 `RE2` 库。
- **WITH_STEMMER** - 使用 Snowball 词干库进行构建。
- **WITH_STEMMER_FORCE_STATIC** - 下载 Snowball 源代码，进行编译，并静态链接，因此最终的二进制文件将不依赖于系统中存在的共享 `libstemmer` 库。
- **WITH_ICU** - 使用 ICU（国际 Unicode 组件）库进行构建。它用于分割中文文本。当使用 morphology=`icu_chinese` 时将使用它。
- **WITH_JIEBA** - 使用 Jieba 中文文本分割工具进行构建。它用于分割中文文本。当使用 morphology=`jieba_chinese` 时将使用它。
- **WITH_ICU_FORCE_STATIC** - 下载 ICU 源代码，进行编译，并静态链接，因此最终的二进制文件将不依赖于系统中存在的共享 `icu` 库。还将 ICU 数据文件包含到安装/分发中。静态链接 ICU 的目的是拥有一个已知版本的库，以便行为是确定的，而不是依赖于任何系统库。您很可能更倾向于使用系统的 ICU，因为它可能会随着时间的推移而更新，而无需重新编译 Manticore 守护程序。在这种情况下，您需要显式禁用此选项。这还将节省您一些占用的空间，因为 ICU 数据文件（约 30M）将不包含在分发中。
- **WITH_SSL** - 用于支持 HTTPS，以及与守护进程的加密 MySQL 连接。系统的 OpenSSL 库将链接到守护进程。这意味着启动守护进程时需要 OpenSSL。这对于支持 HTTPS 是强制性的，但对于服务器来说不是绝对强制的（即，没有 SSL 意味着无法通过 HTTPS 连接，但其他协议仍然可以使用）。Manticore 可能使用 OpenSSL 版本从 1.0.2 到 1.1.1，不过请注意，**为了安全起见，强烈建议使用最新的 SSL
  库**。当前仅支持 v1.1.1，其他版本已过时（
  参见 [openssl release strategy](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - 索引器用来处理来自 MySQL 的压缩列。守护进程用来支持压缩的 MySQL 协议。
- **WITH_ODBC** - 索引器用来支持从 ODBC 提供程序索引数据源（通常是 UnixODBC 和 iODBC）。在 MS Windows 平台上，ODBC 是处理 MS SQL 数据源的正确方式，因此索引 `MSSQL` 也意味着需要这个标志。
- **DL_ODBC** - 不链接 ODBC 库。如果链接了 ODBC 库但该库不可用，即使想处理与 ODBC 无关的内容，也无法启动索引器工具。此选项要求索引器仅在需要处理 ODBC 数据源时才在运行时加载该库。
- **ODBC_LIB** - ODBC 库文件的名称。索引器将在处理 ODBC 数据源时尝试加载该文件。该选项通过对可用 ODBC 共享库的检测自动写入。运行时也可以通过设置环境变量 `ODBC_LIB`，提供指向替代库的正确路径来覆盖此名称。
- **WITH_EXPAT** - 索引器用来支持索引 xmlpipe 数据源。
- **DL_EXPAT** - 不链接 EXPAT 库。如果链接了 EXPAT 库但该库不可用，即使想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 数据源时才在运行时加载该库。
- **EXPAT_LIB** - EXPAT 库文件的名称。索引器将在处理 xmlpipe 数据源时尝试加载该文件。该选项通过对可用 EXPAT 共享库的检测自动写入。运行时也可以通过设置环境变量 EXPAT_LIB，提供指向替代库的正确路径来覆盖此名称。
- **WITH_ICONV** - 支持索引 xmlpipe 数据源时使用不同编码。
- **DL_ICONV** - 不链接 iconv 库。如果链接了 iconv 库但该库不可用，即使想处理与 xmlpipe 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 xmlpipe 数据源时才在运行时加载该库。
- **ICONV_LIB** - iconv 库文件的名称。索引器将在处理 xmlpipe 数据源时尝试加载该文件。该选项通过对可用 iconv 共享库的检测自动写入。运行时也可以通过设置环境变量 `ICONV_LIB`，提供指向替代库的正确路径来覆盖此名称。
- **WITH_MYSQL** - 索引器用于支持索引 MySQL 数据源。
- **DL_MYSQL** - 不链接 MySQL 库。如果链接了 MySQL 库但该库不可用，即使想处理与 MySQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 MySQL 数据源时才在运行时加载该库。
- **MYSQL_LIB** -- MySQL 库文件的名称。索引器将在处理 MySQL 数据源时尝试加载该文件。该选项通过对可用 MySQL 共享库的检测自动写入。运行时也可以通过设置环境变量 `MYSQL_LIB`，提供指向替代库的正确路径来覆盖此名称。
- **WITH_POSTGRESQL** - 索引器用于支持索引 PostgreSQL 数据源。
- **DL_POSTGRESQL** - 不链接 PostgreSQL 库。如果链接了 PostgreSQL 库但该库不可用，即使想处理与 PostgreSQL 无关的内容，也无法启动 `indexer` 工具。此选项要求索引器仅在需要处理 PostgreSQL 数据源时才在运行时加载该库。
- **POSTGRESQL_LIB** - PostgreSQL 库文件的名称。索引器在处理 postgresql 数据源时将尝试加载指定的 postgresql 库文件。该选项通过对可用 postgresql 共享库的检测自动确定。运行时也可以通过设置环境变量 `POSTGRESQL_LIB`，提供指向替代库的正确路径来覆盖此名称。
- **LOCALDATADIR** - 守护进程存储 binlog 的默认路径。如果守护进程的运行时配置（即与此构建配置无关的 `manticore.conf` 文件）中未提供此路径或显式禁用，则 binlog 将会放在此路径。通常这是一个绝对路径，不过也可以使用相对路径。您通常无需更改配置中定义的默认值，其依目标系统不同，可能类似 `/var/data`，`/var/lib/manticore/data`，或 `/usr/local/var/lib/manticore/data`。
- **FULL_SHARE_DIR** - 存放所有资源的默认路径。启动任何使用该文件夹内容的工具之前，可以通过环境变量 `FULL_SHARE_DIR` 覆盖此路径。此路径十分重要，因为许多内容默认期望在这里找到，包括预定义的字符集表、停用词、manticore 模块和 icu 数据文件，这些都放在该文件夹中。配置脚本通常将此路径确定为类似 `/usr/share/manticore` 或 `/usr/local/share/manticore`。
- **DISTR_BUILD** - 发布包选项的快捷方式。这是一个字符串值，表示目标平台的名称。它可以用来代替手动配置所有选项。在 Debian 和 Redhat Linux 上，默认值可能通过轻度自省确定并设置为通用的 'Debian' 或 'RHEL'。否则，该值未定义。
- **PACK** - 更方便的快捷方式。它读取 `DISTR` 环境变量，将其分配给 **DISTR_BUILD** 参数，然后正常工作。这在构建预准备的构建系统（如 Docker 容器）时非常有用，在这些环境中，`DISTR` 变量在系统级别设置并反映容器所针对的目标系统。
- **CMAKE_INSTALL_PREFIX** (路径) - Manticore 预计安装的位置。构建不执行任何安装，但它准备在您运行 `cmake --install` 命令或创建包并随后安装时执行的安装规则。前缀可以随时更改，甚至在安装过程中，通过调用
  `cmake --install . --prefix /path/to/installation`。然而，在配置时，该变量用于初始化 `LOCALDATADIR` 和 `FULL_SHARE_DIR` 的默认值。例如，在配置时将其设置为 `/my/custom`
  将 `LOCALDATADIR` 硬编码为 `/my/custom/var/lib/manticore/data`，并将 `FULL_SHARE_DIR` 硬编码为
  `/my/custom/usr/share/manticore`。
- **BUILD_TESTING** (布尔值) 是否支持测试。如果启用，在构建后，您可以运行 'ctest' 来测试构建。请注意，测试意味着额外的依赖，例如至少需要 PHP cli、Python 和可用的带有测试数据库的 MySQL 服务器。默认情况下，此参数处于开启状态。因此，对于“仅构建”，您可能希望通过显式指定 'off' 值来禁用该选项。
- **BUILD_SRPMS** (布尔值) 是否显示构建源 RPM (SRPMs) 的说明。由于 CPack 在基于组件的打包上的限制，SRPM 不能与二进制 RPM 直接生成。当启用时，构建系统将显示使用源配置方法进行适当 SRPM 生成的说明。默认情况下，此参数处于关闭状态。
- **LIBS_BUNDLE** - 存放不同库的文件夹路径。这主要与 Windows 构建相关，但如果您需要经常构建以避免每次都下载第三方源，它也可能有用。默认情况下，此路径不会被配置脚本修改；您应该手动将所有内容放在那里。当，例如，我们想要对一个词干提取器提供支持时 - 源文件将从 Snowball 主页面下载，然后解压、配置、构建等。相反，您可以将原始源 tarball（即 `libstemmer_c.tgz`）存储在此文件夹中。下次您想从头开始构建时，配置脚本将首先查找该捆绑包，如果在那里找到词干提取器，则不会再从 Internet 下载。
- **CACHEB** - 存储 3 方库构建的文件夹路径。通常像 galera、re2、icu 等功能首先下载或从捆绑包中获取，然后解压、构建并安装到一个临时内部文件夹。在构建 Manticore 时，该文件夹将用于支持所请求功能所需的内容。最后，它们要么与 Manticore 链接（如果是库），要么直接用于分发/安装（例如 galera 或 icu 数据）。当 **CACHEB** 被定义为 cmake 配置参数或系统环境变量时，它被用作该构建的目标文件夹。该文件夹可以在构建之间保持，这样存储在其中的库将不再被重建，从而使整个构建过程更短。

注意，有些选项以三元组的方式组织：`WITH_XXX`，`DL_XXX` 和 `XXX_LIB` - 像 mysql、odbc 的支持一样。`WITH_XXX` 决定后两个选项是否有效。也就是说，如果您将 `WITH_ODBC` 设置为 `0` - 提供 `DL_ODBC` 和 `ODBC_LIB` 就没有意义，如果整个功能被禁用，这两个将没有效果。此外，`XXX_LIB` 在没有 `DL_XXX` 的情况下没有意义，因为如果您不想要 `DL_XXX` 选项，则不会使用动态加载，`XXX_LIB` 提供的名称无用。这由默认自省使用。

此外，使用 `iconv` 库假定 `expat` 是启用的，如果后者被禁用则没有意义。

此外，一些库可能总是可用，因此，没有必要避免与它们的链接。例如，在 Windows 中，这是 ODBC。在 macOS 中是 Expat、iconv 以及可能的其他库。默认自省确定这些库，并仅为它们有效地发出 `WITH_XXX`，而不需要 `DL_XXX` 和 `XXX_LIB`，这简化了过程。

在某些选项的配置过程中，配置可能看起来像：

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

除了一般配置值外，您还可以检查在运行配置后留在构建文件夹中的文件 `CMakeCache.txt`。在那里定义的任何值在运行 cmake 时都可以被显式重新定义。例如，您可以运行 `cmake -DHAVE_GETADDRINFO_A=FALSE ...`，此配置运行将不假定该变量的调查值，而是使用您提供的值。

#### 特定环境变量

环境变量对于提供某种全局设置非常有用，这些设置存储在构建配置之外且始终存在。为了持久性，可以使用不同方式在系统上全局设置它们 - 例如将它们添加到 `.bashrc` 文件中，或者如果您创建一个基于 Docker 的构建系统，则将它们嵌入到 Dockerfile 中，或者在 Windows 的系统首选项环境变量中写入它们。此外，您可以使用 `export VAR=value` 在 shell 中设置它们为短期存在。或者，更简短地，通过将值放在 cmake 调用前面，例如 `CACHEB=/my/cache cmake ...` - 这样它只在此调用时有效，而在下次调用时将不可见。
有些此类变量通常由 cmake 及其他一些工具使用。这些变量例如 `CXX`（决定当前的 C++ 编译器），或 `CXX_FLAGS`（用于提供编译器标志）等。

然而，我们有一些特定于 manticore 配置的变量，它们是专为我们的构建而设计的。

- **CACHEB** - 与配置选项 **CACHEB** 相同
- **LIBS_BUNDLE** - 与配置选项 **LIBS_BUNDLE** 相同
- **DISTR** - 在使用 `-DPACK=1` 时用于初始化 `DISTR_BUILD` 选项。
- **DIAGNOSTIC** - 使 cmake 配置输出更加详细，解释所有发生的操作
- **WRITEB** - 假定设置了 **LIBS_BUNDLE**，如果该变量被设置，它会将不同工具的源文件归档下载到 LIBS_BUNDLE 文件夹。也就是说，如果出现了新的 stemmer 版本，你可以手动从 bundle 中删除 libstemmer_c.tgz，然后运行一次性命令 `WRITEB=1 cmake ...` —— 此时它将在 bundle 中找不到 stemmer 的源代码，然后会从供应商站点下载它们到 bundle（如果不使用 WRITEB 则会将它们下载到构建内部的临时文件夹，并在清除构建文件夹后消失）。

在配置结束时，你可以看到可用并将被使用的内容列表，如：

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

若要安装到自定义（非默认）文件夹，请运行

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### 构建软件包

构建软件包时，使用目标 `package`。它将根据 `-DDISTR_BUILD` 选项提供的选择来构建软件包。默认情况下，它会生成一个包含所有二进制文件和辅助文件的简单 .zip 或 .tgz 归档。

```bash
cmake --build . --target package --config RelWithDebInfo
```

## 构建的高级操作

### 单配置下的重新编译（更新）

如果你没有更改源码和构建路径，只需切换到你的构建文件夹并运行：

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

如果由于某种原因它不起作用，你可以删除位于构建文件夹中的 `CMakeCache.txt` 文件。完成此步骤后你
必须再次运行 cmake，指向源文件夹并配置选项。

如果仍然无效，只需清空你的构建文件夹并从头开始。

### 构建类型

简而言之——只需如上所述使用 `--config RelWithDebInfo`。这样不会出错。

我们使用两种构建类型。对于开发，使用 `Debug` —— 它会设置优化等编译器标志，使其非常适合开发，支持逐步调试运行。然而，生成的二进制文件体积较大且在生产环境中运行较慢。

对于发布，我们使用另一种类型——`RelWithDebInfo`——表示“发布构建并包含调试信息”。它会生成带有嵌入调试信息的生产环境二进制文件。之后，这些调试信息会被拆分到独立的 debuginfo 软件包，与发布软件包一起存放，并可在出现崩溃等问题时用于调查和修复。Cmake 还提供了 `Release` 和 `MinSizeRel`，但我们不使用它们。如果指定的构建类型不可用，cmake 将进行 `noconfig` 构建。

#### 构建系统生成器

有两种生成器类型：单配置和多配置。

- 单配置生成器需要在配置阶段通过 `CMAKE_BUILD_TYPE` 参数提供构建类型。如果未定义，该构建将回退到 `RelWithDebInfo` 类型，适用于仅想从源代码构建 Manticore 而不参与开发的情况。对于显式构建，应提供构建类型，例如 `-DCMAKE_BUILD_TYPE=Debug`。
- 多配置生成器在构建阶段选择构建类型。应使用 `--config` 选项提供构建类型，否则将进行一种不受欢迎的 `noconfig` 构建。因此，你应始终指定构建类型，例如 `--config Debug`。

如果你想指定构建类型但不想在意它是“单配置”还是“多配置”生成器，只需在两处都提供必要的键即可。即使用 `-DCMAKE_BUILD_TYPE=Debug` 进行配置，然后使用 `--config Debug` 进行构建。只要确保两个值相同即可。如果构建器是单配置，它会使用配置参数；如果是多配置，则会忽略配置参数，但会通过 `--config` 键选择正确的构建配置。

如果你想要 `RelWithDebInfo`（即仅进行生产环境构建）并且知道你所在的平台是单配置（也就是除 Windows 之外的所有平台），那么在调用 cmake 时可以省略 `--config` 标志。此时会配置并使用默认的 `CMAKE_BUILD_TYPE=RelWithDebInfo`。“构建”、“安装”和“构建软件包”的所有命令也将变得更简短。

#### 显式选择构建系统生成器

Cmake 本身不执行构建操作，而是为本地构建系统生成规则。
通常，它能够很好地确定可用的构建系统，但有时你可能需要显式提供生成器。你
可以运行 `cmake -G` 并查看可用生成器的列表。

- 在 Windows 上，如果安装了多个版本的 Visual Studio，可能需要指定使用哪个版本，
如：
```bash
cmake -G "Visual Studio 16 2019" ....
  ```
- 在其他所有平台上 - 通常使用 Unix Makefiles，但你也可以指定其他生成器，如 Ninja，或 Ninja Multi-Config，如：
  Multi-Config`，如：
```bash
  cmake -GNinja ...
  ```
  或
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja Multi-Config 非常有用，因为它是真正的“多配置”并且可用于 Linux/macOS/BSD。使用这个生成器，你可以将配置类型的选择推迟到构建时，并且可以在同一构建文件夹中构建多个配置，只需更改 `--config` 参数。

### 注意事项

1. 如果你最终想构建一个功能齐全的 RPM 软件包，构建目录的路径必须足够长，以便正确构建调试符号。
例如 `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`。这是因为 RPM 工具在构建调试信息时会修改已编译二进制文件的路径，它只能覆盖已有空间而不会分配更多空间。前述很长的路径有 100 个字符，这对于这种情况已经足够。

## 外部依赖

如果你想使用某些功能，某些库必须可用。
- 用于索引（`indexer` 工具）：`expat`、`iconv`、`mysql`、`odbc`、`postgresql`。没有它们，你只能处理 `tsv` 和 `csv` 源。
- 用于提供查询服务（`searchd` 守护进程）：可能需要 `openssl`。
- 对于所有（必须的，强制的！）我们需要 Boost 库。最低版本是 1.61.0，但我们是用更新的 1.75.0 版本构建二进制文件。更高版本（如 1.76）也应该没问题。在 Windows 上，你可以从官方网站 (boost.org) 下载预编译的 Boost 并安装到默认建议的路径（即 `C:\\boost...`）。在 MacOs 上，brew 提供的版本是可以的。在 Linux 上，你可以检查官方仓库中的可用版本，如果不符合要求，可以从源码构建。我们需要组件 'context'，你也可以构建 'system' 和 'program_options'，如果你还想从源码构建 Galera 库，它们将是必需的。请查看 `dist/build_dockers/xxx/boost_175/Dockerfile`，那里有一个简单的自述脚本/说明如何操作。

在构建系统中，你需要安装这些包的 'dev' 或 'devel' 版本（即 libmysqlclient-devel、unixodbc-devel 等。具体包名可参见我们的 dockerfiles）。

在运行系统中，这些包至少应以最终的（非开发）版本存在。（devel 版本通常更大，因为它们不仅包含目标二进制文件，还包含不同的开发资料，如头文件等）。

### 在 Windows 上构建

除了必要的前提条件外，你可能还需要预编译的 `expat`、`iconv`、`mysql` 和 `postgresql` 客户端库。你需要自行构建它们或联系我们以获取我们的构建包（一个简单的 zip 压缩档，里面包含这些目标的文件夹）。

- ODBC 不必要，因为它是系统库。
- OpenSSL 可以从源码构建，或从 https://slproweb.com/products/Win32OpenSSL.html 下载预编译版本（如 cmake 内部脚本 FindOpenSSL 所述）。
- Boost 可以从 https://www.boost.org/ 下载预编译版本。

### 查看已编译内容

运行 `indexer -h`。它会显示已配置和构建的功能（无论是显式还是检测到的，都无所谓）：

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->



