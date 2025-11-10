# 数据库连接

源定义必须包含连接的设置，这包括主机、端口、用户凭据或驱动程序的特定设置。

## sql_host

要连接的数据库服务器主机。请注意，MySQL 客户端库会根据主机名选择是通过 TCP/IP 还是通过 UNIX 套接字连接。具体来说，“localhost”将强制使用 UNIX 套接字（这是默认且通常推荐的模式），而“127.0.0.1”将强制使用 TCP/IP。

## sql_port

要连接的服务器 IP 端口。
对于 `mysql`，默认端口是 3306，对于 `pgsql`，是 5432。

## sql_db

连接建立后要使用的 SQL 数据库，并在其中执行进一步的查询。

## sql_user

用于连接的用户名。

## sql_pass

连接时使用的用户密码。如果密码中包含 `#`（配置文件中可用于添加注释），可以用 `\` 转义。

## sql_sock

用于本地数据库服务器连接的 UNIX 套接字名称。请注意，是否实际使用此值取决于 `sql_host` 设置。

```ini
sql_sock = /var/lib/mysql/mysql.sock
```

## 驱动程序的特定设置

### MySQL

#### mysql_connect_flags

MySQL 客户端连接标志。可选，默认值为 0（不设置任何标志）。

此选项必须包含标志值的整数和。该值将原样传递给 mysql_real_connect()。标志在 mysql_com.h 头文件中列举。与索引相关特别有趣的标志及其对应值如下：

* CLIENT_COMPRESS = 32；可使用压缩协议
* CLIENT_SSL = 2048；握手后切换到 SSL
* CLIENT_SECURE_CONNECTION = 32768；新的 4.1 认证
例如，您可以指定 2080（2048+32）以同时使用压缩和 SSL，或指定 32768 仅使用新认证。最初引入此选项是为了在索引器和 mysqld 位于不同主机时能够使用压缩。理论和实践中，在 1 Gbps 链路上启用压缩很可能会影响索引时间，尽管它减少了网络流量。然而，在 100 Mbps 链路上启用压缩可能显著改善索引时间（据报道总索引时间提升了 20-30%）。实际效果可能因情况而异。

```ini
mysql_connect_flags = 32 # enable compression
```

### SSL 证书设置

* `mysql_ssl_cert` - SSL 证书路径
* `mysql_ssl_key` - SSL 密钥文件路径
* `mysql_ssl_ca` - CA 证书路径


### unpack_mysqlcompress

```ini
unpack_mysqlcompress_maxsize = 1M
```

使用 MySQL `UNCOMPRESS()` 算法解压的列。多值，可选，默认值为空列列表。

使用此指令指定的列将由索引器使用 MySQL `COMPRESS()` 和 `UNCOMPRESS()` 函数所用的修改版 zlib 算法解压。当索引器与数据库不在同一台机器上时，这可以减轻数据库负载并节省网络流量。此功能仅在构建时同时具备 zlib 和 zlib-devel 时可用。

```ini
unpack_mysqlcompress = body_compressed
unpack_mysqlcompress = description_compressed
```

默认情况下，解压数据时使用 16M 的缓冲区。可以通过设置 `unpack_mysqlcompress_maxsize` 来更改。

使用 unpack_mysqlcompress 时，由于实现细节，无法从压缩数据推断所需缓冲区大小。因此，缓冲区必须预先分配，且解压后的数据不能超过缓冲区大小。

### unpack_zlib

```ini
unpack_zlib = col1
unpack_zlib = col2
```

使用 zlib（又名 deflate，或 gunzip）解压的列。多值，可选，默认值为空列列表。仅适用于源类型 `mysql` 和 `pgsql`。

使用此指令指定的列将由 `indexer` 使用标准 zlib 算法（称为 deflate，也由 `gunzip` 实现）解压。当索引器与数据库不在同一台机器上时，这可以减轻数据库负载并节省网络流量。此功能仅在构建时同时具备 zlib 和 zlib-devel 时可用。

### MSSQL

MS SQL Windows 认证标志。连接 MS SQL Server 时是否使用当前登录的 Windows 账户凭据进行认证。

```ini
mssql_winauth = 1
```

### ODBC

使用 ODBC 的源需要存在 DSN（数据源名称）字符串，可通过 `odbc_dsn` 设置。

```ini
odbc_dsn = Driver={Oracle ODBC Driver};Dbq=myDBName;Uid=myUsername;Pwd=myPassword
```

请注意，格式取决于所使用的具体 ODBC 驱动程序。
<!-- proofread -->

