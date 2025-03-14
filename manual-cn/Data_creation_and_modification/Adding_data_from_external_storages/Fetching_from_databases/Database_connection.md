# 数据库连接

源定义必须包含连接的设置，这包括主机、端口、用户凭据或驱动的特定设置。

## sql_host

要连接的数据库服务器主机。注意，MySQL客户端库会根据主机名决定是通过TCP/IP还是通过UNIX套接字进行连接。特别地，"localhost" 将强制使用UNIX套接字（这是默认且通常推荐的模式），而 "127.0.0.1" 将强制使用TCP/IP。

## sql_port

要连接的服务器IP端口。
对于 `mysql`，默认端口是3306，`pgsql`的默认端口是5432。

## sql_db

连接建立后要使用的SQL数据库，并在此数据库中执行后续查询。

## sql_user

用于连接的用户名。

## sql_pass

连接时使用的用户密码。如果密码中包含 `#`（这可能在配置文件中用于添加注释），可以用 `\` 来转义。

## sql_sock

用于本地数据库服务器连接的UNIX套接字名。请注意，是否实际使用此值取决于 `sql_host` 设置。

```ini
sql_sock = /var/lib/mysql/mysql.sock
```

## 驱动的特定设置

### MySQL

#### mysql_connect_flags

MySQL客户端连接标志。可选，默认值为0（不设置任何标志）。

此选项必须包含一个整数值，代表标志的总和。该值将按原样传递给 `mysql_real_connect()`。这些标志在 `mysql_com.h` 文件中列出。以下是一些与索引相关且特别有趣的标志及其相应的值：

  - CLIENT_COMPRESS = 32；可以使用压缩协议
  - CLIENT_SSL = 2048；握手后切换到SSL
  - CLIENT_SECURE_CONNECTION = 32768；新的4.1认证 例如，你可以指定 2080（2048+32）来同时使用压缩和SSL，或者指定32768来只使用新认证。此选项最初是为了解决在 `indexer` 和 `mysqld` 位于不同主机上时使用压缩的问题。在1Gbps链路上，压缩可能会降低索引时间，尽管在理论和实践中会减少网络流量。然而，在100Mbps链路上启用压缩可能会显著提高索引时间（总索引时间改善的20-30％曾被报告）。实际效果视情况而定。

```ini
mysql_connect_flags = 32 # enable compression
```

### SSL 证书设置

* `mysql_ssl_cert` - SSL证书路径
* `mysql_ssl_key` - SSL密钥文件路径
* `mysql_ssl_ca` - CA证书路径


### unpack_mysqlcompress

```ini
unpack_mysqlcompress_maxsize = 1M
```

使用MySQL `UNCOMPRESS()`算法解压的列。可选，默认值为空的列列表。

指定的列将使用 `indexer` 通过MySQL `COMPRESS()` 和 `UNCOMPRESS()` 函数所使用的修改后的zlib算法进行解压。如果在不同的主机上进行索引，这可以减轻数据库的负担并节省网络流量。该功能仅在构建时同时有 `zlib` 和 `zlib-devel` 可用时提供。

```ini
unpack_mysqlcompress = body_compressed
unpack_mysqlcompress = description_compressed
```

默认情况下，使用16M的缓冲区来解压数据。可以通过设置 `unpack_mysqlcompress_maxsize` 来更改此值。

### unpack_zlib

```ini
unpack_zlib = col1
unpack_zlib = col2
```

使用zlib（即deflate或gunzip）解压的列。多值，可选，默认值为空的列列表。适用于 `mysql` 和 `pgsql` 源类型。

指定的列将通过 `indexer` 使用标准的zlib算法（称为deflate，也由 `gunzip` 实现）解压。如果在不同的主机上进行索引，这可以减轻数据库的负担并节省网络流量。该功能仅在构建时同时有 `zlib` 和 `zlib-devel` 可用时提供。

### MSSQL

MS SQL Windows身份验证标志。用于在连接MS SQL Server时是否使用当前登录的Windows账户凭据进行身份验证。

```ini
mssql_winauth = 1
```

### ODBC

使用ODBC的源需要提供一个DSN（数据源名称）字符串，可通过 `odbc_dsn` 进行设置。

```ini
odbc_dsn = Driver={Oracle ODBC Driver};Dbq=myDBName;Uid=myUsername;Pwd=myPassword
```

请注意，格式取决于所使用的具体ODBC驱动。
<!-- proofread -->