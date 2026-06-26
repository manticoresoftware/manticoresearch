# 数据库连接

源定义必须包含连接设置，这包括主机、端口、用户凭据或驱动程序的特定设置。

## sql_host

要连接的数据库服务器主机。注意 MySQL 客户端库会根据主机名选择是通过 TCP/IP 还是通过 UNIX 套接字进行连接。具体来说，“localhost”将强制使用 UNIX 套接字（这是默认且通常推荐的方式），而“127.0.0.1”将强制使用 TCP/IP。

## sql_port

要连接的服务器 IP 端口号。
对于 `mysql`，默认值为 3306，而对于 `pgsql`，默认值为 5432。

## sql_db

在建立连接后使用的 SQL 数据库，并在此数据库内执行进一步查询。

## sql_user

用于连接的用户名。

## sql_pass

连接时使用的用户密码。如果密码包含 `#`（可以在配置文件中用它来添加注释），可以使用 `\` 进行转义。

## sql_sock

要连接的本地数据库服务器的 UNIX 套接字名称。请注意，是否使用此值取决于 `sql_host` 设置。

```ini
sql_sock = /var/lib/mysql/mysql.sock
```

## 驱动程序的特定设置

### MySQL

#### mysql_connect_flags

MySQL 客户端连接标志。可选，默认值为 0（不设置任何标志）。

此选项必须包含一个整数值，该值为标志的总和。该值将原封不动地传递给 mysql_real_connect()。这些标志在 mysql_com.h 包含文件中枚举。特别感兴趣的与索引相关的标志及其相应的值如下：

* CLIENT_COMPRESS = 32；可以使用压缩协议
* CLIENT_SSL = 2048；握手后切换到 SSL
* CLIENT_SECURE_CONNECTION = 32768；新的 4.1 认证
例如，您可以指定 2080（2048 + 32）以同时使用压缩和 SSL，或者指定 32768 仅使用新认证。最初引入此选项是为了能够在不同的主机上使用压缩协议。在 1 Gbps 链路上，尽管压缩减少了网络流量，但实际测试表明，启用压缩可能会显著增加索引时间（报告的总索引时间改进幅度可达 20-30%）。您的体验可能有所不同。

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

要使用 MySQL `UNCOMPRESS()` 算法解压的列。多值，可选，默认值为空列表。

使用此指令指定的列将由索引器使用 MySQL `COMPRESS()` 和 `UNCOMPRESS()` 函数使用的修改过的 zlib 算法进行解压。当在数据库之外进行索引时，这可以让您卸载数据库并节省网络流量。此功能仅在编译时 zlib 和 zlib-devel 都可用时才可用。

```ini
unpack_mysqlcompress = body_compressed
unpack_mysqlcompress = description_compressed
```

默认情况下，用于解压数据的缓冲区大小为 16M。可以通过设置 `unpack_mysqlcompress_maxsize` 来更改此大小。

使用 `unpack_mysqlcompress` 时，由于实现细节的原因，无法从压缩数据中推断出所需的缓冲区大小。因此，缓冲区必须提前分配，解压后的数据不能超过缓冲区大小。

### unpack_zlib

```ini
unpack_zlib = col1
unpack_zlib = col2
```

要使用 zlib（又称 deflate 或 gunzip）解压的列。多值，可选，默认值为空列表。仅适用于 `mysql` 和 `pgsql` 类型的源。

使用此指令指定的列将由 `indexer` 使用标准 zlib 算法（称为 deflate 并且也由 gunzip 实现）进行解压。当在数据库之外进行索引时，这可以让您卸载数据库并节省网络流量。此功能仅在编译时 zlib 和 zlib-devel 都可用时才可用。

### MSSQL

MS SQL Windows 认证标志。连接到 MS SQL Server 时是否使用当前登录的 Windows 账户凭据进行身份验证。

```ini
mssql_winauth = 1
```

### ODBC

使用 ODBC 的源需要设置 DSN（数据源名称）字符串，可以通过 `odbc_dsn` 设置。

```ini
odbc_dsn = Driver={Oracle ODBC Driver};Dbq=myDBName;Uid=myUsername;Pwd=myPassword
```

请注意，格式取决于所使用的具体 ODBC 驱动程序。
<!-- proofread -->

