# 在 Debian 或 Ubuntu 上安装 Manticore

### 支持的发行版：

* Debian
  * 10.0 (Buster)
  * 11.0 (Bullseye)
  * 12.0 (Bookworm)

* Ubuntu
  * 18.04 (Bionic)
  * 20.04 (Focal)
  * 21.04 (Hirsute Hippo)
  * 22.04 (Jammy)
  * 24.04 (Noble)

* Mint
  * 19
  * 20
  * 21

### APT 仓库
在 Ubuntu/Debian/Mint 上安装 Manticore 的最简单方法是使用我们的 APT 仓库。

安装仓库：
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
```
（如果未安装 `wget`，请先安装它；如果 `apt-key` 失败，请安装 `gnupg2`。）

安装 Manticore Search：
```
sudo apt install manticore manticore-extra
```

如果你是从旧版本升级到 Manticore 6，建议首先移除旧的软件包，以避免由于包结构更新而导致的冲突：

```bash
sudo apt remove manticore*
```

这不会删除你的数据和配置文件。

###### 开发包
如果你更喜欢“Nightly”（开发版），可以运行：
```bash
wget https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb && \
sudo dpkg -i manticore-dev-repo.noarch.deb && \
sudo apt -y update && \
sudo apt -y install manticore manticore-extra manticore-common manticore-server manticore-server-core manticore-tools manticore-executor manticore-buddy manticore-backup manticore-columnar-lib manticore-server-core-dbgsym manticore-tools-dbgsym manticore-columnar-lib-dbgsym manticore-icudata-65l manticore-galera manticore-galera-dbgsym manticore-language-packs
```

### 独立的 DEB 包
要从 Manticore 仓库下载独立的 DEB 文件，请按照 [这里的说明](https://manticoresearch.com/install/)。

### 其他可能需要的包
#### 对于索引器
Manticore 包依赖于 zlib 和 ssl 库，其他没有严格要求。然而，如果你计划使用[索引器](https://manual.manticoresearch.com/Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation#Indexer-tool)从外部存储创建表，则需要安装相应的客户端库。运行索引器并查看输出顶部可以找到具体需要的库：

```bash
$ sudo -u manticore indexer
Manticore 3.5.4 13f8d08d@201211 release
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)

Built by gcc/clang v 5.4.0,

Built on Linux runner-0277ea0f-project-3858465-concurrent-0 4.19.78-coreos #1 SMP Mon Oct 14 22:56:39 -00 2019 x86_64 x86_64 x86_64 GNU/Linux

Configured by CMake with these definitions: -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISTR_BUILD=xenial -DUSE_SSL=ON -DDL_UNIXODBC=1 -DUNIXODBC_LIB=libodbc.so.2 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DUSE_LIBICONV=1 -DDL_MYSQL=1 -DMYSQL_LIB=libmysqlclient.so.20 -DDL_PGSQL=1 -DPGSQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/data -DFULL_SHARE_DIR=/usr/share/manticore -DUSE_ICU=1 -DUSE_BISON=ON -DUSE_FLEX=ON -DUSE_SYSLOG=1 -DWITH_EXPAT=1 -DWITH_ICONV=ON -DWITH_MYSQL=1 -DWITH_ODBC=ON -DWITH_POSTGRESQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 -DWITH_ZLIB=ON -DGALERA_SOVERSION=31 -DSYSCONFDIR=/etc/manticoresearch
```

在输出中，你可以看到 Manticore 的版本信息和配置选项，其中提到了诸如  **libodbc.so.2**, **libexpat.so.1**, **libmysqlclient.so.20**,和 **libpq.so.5**等库。

以下是不同 Debian/Ubuntu 版本的客户端库参考表：

| 发行版 | MySQL | PostgreSQL | XMLpipe | UnixODBC |
| - | - | - | - | - |
| Ubuntu Trusty | libmysqlclient.so.18 | libpq.so.5 | libexpat.so.1 | libodbc.so.1 |
| Ubuntu Bionic | libmysqlclient.so.20 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Focal | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Hirsute | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Ubuntu Jammy | libmysqlclient.so.21 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Jessie | libmysqlclient.so.18 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Buster | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Bullseye | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |
| Debian Bookworm | libmariadb.so.3 | libpq.so.5 | libexpat.so.1 | libodbc.so.2 |

要查找提供这些库的包，可以使用 `apt-file` 工具，例如：

```bash
apt-file find libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.2.0
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.3.6
```

注意，你只需要根据实际使用的存储类型安装相应的库。如果你只打算从 MySQL 构建表，则只需要安装 MySQL 库（例如 `libmysqlclient20`）。

最后，安装所需的库：

```bash
sudo apt-get install libmysqlclient20 libodbc1 libpq5 libexpat1
```

如果你不打算使用索引器工具，则无需查找和安装任何库。

为了启用 CJK 分词支持，官方软件包中包含嵌入式 ICU 库和 ICU 数据文件。它们与系统上可用的任何 ICU 运行时库是独立的，并且无法升级。

#### 乌克兰词形还原器
词形还原器需要 Python 3.9 及以上版本。 **确保已安装并配置为 `--enable-shared`。**

以下是在 Debian 和 Ubuntu 上安装 Python 3.9 和乌克兰词形还原器的步骤：

```bash
# 从 APT 仓库安装 Manticore Search 和乌克兰词形还原器
cd ~
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt -y update
sudo apt -y install manticore manticore-lemmatizer-uk

# 安装构建 Python 所需的软件包
sudo apt -y update
sudo apt -y install wget build-essential libreadline-dev libncursesw5-dev libssl-dev libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev libffi-dev zlib1g-dev

# 下载、构建并安装 Python 3.9
cd ~
wget https://www.python.org/ftp/python/3.9.4/Python-3.9.4.tgz
tar xzf Python-3.9.4.tgz
cd Python-3.9.4
./configure --enable-optimizations --enable-shared
sudo make -j8 altinstall

# 更新链接器缓存
sudo ldconfig

# 安装 pymorphy2 和乌克兰词典
sudo LD_LIBRARY_PATH=~/Python-3.9.4 pip3.9 install pymorphy2[fast]
sudo LD_LIBRARY_PATH=~/Python-3.9.4 pip3.9 install pymorphy2-dicts-uk
```
<!-- proofread -->
