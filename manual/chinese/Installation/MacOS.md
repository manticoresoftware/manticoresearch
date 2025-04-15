# 在MacOS上安装Manticore

## 通过Homebrew包管理器

```bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

将Manticore作为brew服务启动：

```bash
brew services start manticoresearch
```

Manticore的默认配置文件位于 `/usr/local/etc/manticoresearch/manticore.conf` 或 `/opt/homebrew/etc/manticoresearch/manticore.conf`。

如果您计划使用 [indexer](../Creating_a_table/Local_tables/Plain_table.md) 从MySQL、PostgreSQL或其他通过ODBC的数据库获取数据，您可能需要额外的库，例如 `mysql@5.7`、`libpq` 和 `unixodbc`。

#### 开发包
如果您更喜欢“夜间版”（开发）版本，请执行：
```bash
brew tap manticoresoftware/tap-dev
brew install manticoresoftware/tap-dev/manticoresearch-dev manticoresoftware/tap-dev/manticore-extra-dev manticoresoftware/tap-dev/manticore-language-packs
brew services start manticoresearch-dev
```

<!--
## 从包含二进制文件的tar包安装

从[网站](https://manticoresearch.com/install/)下载并解压到一个文件夹：

```bash
mkdir manticore

cd manticore

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-5.0.2-220530-348514c86-main.tar.gz

tar -xf manticore-5.0.2-220530-348514c86-main.tar.gz

wget https://repo.manticoresearch.com/repository/manticoresearch_macos/release/manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

tar -xf manticore-columnar-lib-1.15.4-220522-2fef34e-osx10.14.4-x86_64.tar.gz

# 启动Manticore
FULL_SHARE_DIR=./share/manticore ./bin/searchd -c ./etc/manticoresearch/manticore.conf

# 运行indexer
FULL_SHARE_DIR=./share/manticore ./bin/indexer -c ./etc/manticoresearch/manticore.conf
```

Manticore配置文件是 `./etc/manticoresearch/manticore.conf`，在您解压缩归档后。

-->

<!-- proofread -->
