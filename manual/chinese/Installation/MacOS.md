# 在 MacOS 上安装 Manticore

## 通过 Homebrew 包管理器

```bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

以 brew 服务的方式启动 Manticore：

```bash
brew services start manticoresearch
```

Manticore 的默认配置文件位于 `/usr/local/etc/manticoresearch/manticore.conf` 或 `/opt/homebrew/etc/manticoresearch/manticore.conf`。

如果你打算使用 [indexer](../Creating_a_table/Local_tables/Plain_table.md) 从 MySQL、PostgreSQL 或通过 ODBC 的其他数据库中获取数据，可能需要额外的库，如 `mysql@5.7`、`libpq` 和 `unixodbc`。

#### 开发包
如果你更喜欢“Nightly”（开发）版本，请执行：
```bash
brew tap manticoresoftware/tap-dev
brew install manticoresoftware/tap-dev/manticoresearch-dev manticoresoftware/tap-dev/manticore-extra-dev manticoresoftware/tap-dev/manticore-language-packs
brew services start manticoresearch-dev
```

<!--
## 从带有二进制文件的压缩包安装

从[官方网站](https://manticoresearch.com/install/)下载并解压到文件夹：

```bash
mkdir manticore

cd manticore

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-5.0.2-220530-348514c86-main.tar.gz

tar -xf manticore-5.0.2-220530-348514c86-main.tar.gz

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

tar -xf manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

## Start Manticore
FULL_SHARE_DIR=./share/manticore ./bin/searchd -c ./etc/manticoresearch/manticore.conf

## Run indexer
FULL_SHARE_DIR=./share/manticore ./bin/indexer -c ./etc/manticoresearch/manticore.conf
```

解压后，Manticore 的配置文件位于 `./etc/manticoresearch/manticore.conf`。

-->

<!-- proofread -->

