# 在 MacOS 上安装 Manticore

## 通过 Homebrew 包管理器

使用以下命令安装 Manticore：

```bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

启动 Manticore 服务：

```bash
brew services start manticoresearch
```

Manticore 的默认配置文件位于以下路径之一：

- `/usr/local/etc/manticoresearch/manticore.conf`
- `/opt/homebrew/etc/manticoresearch/manticore.conf`

如果你计划使用[索引器](../Creating_a_table/Local_tables/Plain_table.md)从 MySQL、PostgreSQL 或其他通过 ODBC 的数据库提取数据，可能需要额外的库，例如 `mysql@5.7`、`libpq` 和 `unixodbc`。

###### 开发包
如果你偏好使用“Nightly”（开发版本），可以运行：
```bash
brew tap manticoresoftware/tap-dev
brew install manticoresoftware/tap-dev/manticoresearch-dev manticoresoftware/tap-dev/manticore-extra-dev manticoresoftware/tap-dev/manticore-language-packs
brew services start manticoresearch-dev
```

<!-- proofread -->
