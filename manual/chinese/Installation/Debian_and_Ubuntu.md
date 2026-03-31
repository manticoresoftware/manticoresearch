# 在 Debian 或 Ubuntu 上安装 Manticore

### 支持的版本：

* Debian
  * 11.0 (Bullseye)
  * 12.0 (Bookworm)
  * 13.0 (Trixie)

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
在 Ubuntu/Debian/Mint 上安装 Manticore 最简单的方法是使用我们的 APT 仓库。

安装仓库：
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
```
（如果未安装，请安装 `wget`；如果 `apt-key` 失败，请安装 `gnupg2`）。

然后安装 Manticore Search：
```
sudo apt install manticore
```

如果您是从旧版本升级到 Manticore 6，建议先删除旧包以避免由于包结构更新导致的冲突：

```bash
sudo apt remove manticore*
```

它不会删除您的数据和配置文件。

###### 开发包
如果您更喜欢使用“Nightly”（开发）版本，请执行以下操作：
```bash
wget https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb && \
sudo dpkg -i manticore-dev-repo.noarch.deb && \
sudo apt -y update && \
sudo apt -y install manticore
```

### 独立 DEB 包
要从 Manticore 仓库下载独立的 DEB 文件，请遵循 https://manticoresearch.com/install/ 上的说明。

### 您可能需要的其他包
#### 对于 indexer
Manticore 包依赖于 zlib 和 ssl 库，其他内容不是必需的。但是，如果您计划使用 [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 从外部存储创建表，您需要安装适当的客户端库。要找出 `indexer` 需要的具体库，请运行它并查看输出顶部的内容：

```bash
$ sudo -u manticore indexer
Manticore 3.5.4 13f8d08d@201211 release
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)
Copyright (c) 2017-2020, Manticore Software LTD (https://manticoresearch.com)

Built by gcc/clang v 5.4.0,

Built on Linux runner-0277ea0f-project-3858465-concurrent-0 4.19.78-coreos #1 SMP Mon Oct 14 22:56:39 -00 2019 x86_64 x86_64 x86_64 GNU/Linux

Configured by CMake with these definitions: -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDISTR_BUILD=xenial -DUSE_SSL=ON -DDL_UNIXODBC=1 -DUNIXODBC_LIB=libodbc.so.2 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DUSE_LIBICONV=1 -DDL_MYSQL=1 -DMYSQL_LIB=libmysqlclient.so.20 -DDL_PGSQL=1 -DPGSQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/data -DFULL_SHARE_DIR=/usr/share/manticore -DUSE_ICU=1 -DUSE_BISON=ON -DUSE_FLEX=ON -DUSE_SYSLOG=1 -DWITH_EXPAT=1 -DWITH_ICONV=ON -DWITH_MYSQL=1 -DWITH_ODBC=ON -DWITH_POSTGRESQL=1 -DWITH_RE2=1 -DWITH_STEMMER=1 -DWITH_ZLIB=ON -DGALERA_SOVERSION=31 -DSYSCONFDIR=/etc/manticoresearch
```

这里可以看到 **libodbc.so.2**、**libexpat.so.1**、**libmysqlclient.so.20** 和 **libpq.so.5** 的提及。

以下是不同 Debian/Ubuntu 版本的所有客户端库列表的参考表：

| Distr | MySQL | PostgreSQL | XMLpipe | UnixODBC |
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

要查找提供这些库的包，您可以使用例如 `apt-file`：

```bash
apt-file find libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.2.0
libmysqlclient20: /usr/lib/x86_64-linux-gnu/libmysqlclient.so.20.3.6
```

请注意，您只需要为将要使用的存储类型安装库。因此，如果您计划仅从 MySQL 构建表，可能只需安装 MySQL 库（在上述情况下为 `libmysqlclient20`）。

最后，安装所需的包：

```bash
sudo apt-get install libmysqlclient20 libodbc1 libpq5 libexpat1
```

如果您根本不会使用 `indexer` 工具，则不需要查找和安装任何库。

要启用 CJK 分词支持，官方包中包含嵌入式 ICU 库的二进制文件，并包含 ICU 数据文件。它们与系统上可能可用的任何 ICU 运行时库无关，且无法升级。
<!-- proofread -->
