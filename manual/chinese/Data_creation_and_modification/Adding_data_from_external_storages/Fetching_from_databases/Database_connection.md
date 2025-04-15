# 数据库连接

源定义必须包含连接的设置，这包括主机、端口、用户凭证或驱动程序的特定设置。

## sql_host

要连接的数据库服务器主机。请注意，MySQL 客户端库根据主机名选择是通过 TCP/IP 连接还是通过 UNIX 套接字连接。具体来说，“localhost”将强制使用 UNIX 套接字（这是默认且通常推荐的模式），而“127.0.0.1”将强制使用 TCP/IP。

## sql_port

要连接的服务器 IP 端口。 
对于 `mysql`，默认是 3306，对于 `pgsql`，是 5432。

## sql_db

在建立连接后使用的 SQL 数据库，并在其中执行进一步的查询。

## sql_user

用于连接的用户名。

## sql_pass

连接时使用的用户密码。如果密码包括 `#`（可以用于在配置文件中添加注释），您可以使用 `\` 进行转义。

## sql_sock

连接到本地数据库服务器的 UNIX 套接字名称。请注意，是否实际使用该值取决于 `sql_host` 设置。

```ini
sql_sock = /var/lib/mysql/mysql.sock
```

## 驱动程序的特定设置

### MySQL

#### mysql_connect_flags

MySQL 客户端连接标志。可选，默认值为 0（不设置任何标志）。

此选项必须包含一个整数值，表示标志的总和。该值将被传递给 mysql_real_connect()，逐字不变。mysql_com.h 包含文件中列出了标志。与索引相关的特别有趣的标志及其各自的值如下：

* CLIENT_COMPRESS = 32; 可以使用压缩协议
* CLIENT_SSL = 2048; 在握手后切换到 SSL
* CLIENT_SECURE_CONNECTION = 32768; 新的 4.1 身份验证
例如，您可以指定 2080（2048+32）来同时使用压缩和 SSL，或者 32768 仅使用新的身份验证。最初，此选项是为了能够在索引器和 mysqld 在不同主机上时使用压缩。在 1 Gbps 的链接上，压缩很可能会影响索引时间，尽管它减少了网络流量，从理论上和实践中都是如此。然而，在 100 Mbps 的链接上启用压缩可能会显著改善索引时间（报告称总索引时间改善高达 20-30%）。您的实际效果可能会有所不同。

```ini
mysql_connect_flags = 32 # 启用压缩
```

### SSL 证书设置

* `mysql_ssl_cert` - SSL 证书路径
* `mysql_ssl_key` - SSL 密钥文件路径
* `mysql_ssl_ca` - CA 证书路径


### unpack_mysqlcompress

```ini
unpack_mysqlcompress_maxsize = 1M
```

使用 MySQL `UNCOMPRESS()` 算法解压的列。多值，可选，默认值为一个空的列列表。

使用此指令指定的列将由索引器通过 MySQL `COMPRESS()` 和 `UNCOMPRESS()` 函数使用修改后的 zlib 算法进行解压。当在不同于数据库的机器上进行索引时，这让您能够卸载数据库并节省网络流量。此功能仅在构建时可用 zlib 和 zlib-devel 两者都可用时才可用。

```ini
unpack_mysqlcompress = body_compressed
unpack_mysqlcompress = description_compressed
```

默认情况下，解压数据使用 16M 的缓冲区。通过设置 `unpack_mysqlcompress_maxsize` 可以更改此值。

使用 unpack_mysqlcompress 时，由于实现复杂性，无法从压缩数据中推导出所需的缓冲区大小。因此，必须提前预分配缓冲区，解压后的数据不能超过缓冲区大小。

### unpack_zlib

```ini
unpack_zlib = col1
unpack_zlib = col2
```

使用 zlib（即 deflate，即 gunzip）解压的列。多值，可选，默认值为一个空列列表。仅适用于源类型 `mysql` 和 `pgsql`。

使用此指令指定的列将由 `indexer` 使用标准 zlib 算法（称为 deflate，也由 `gunzip` 实现）进行解压。当在不同于数据库的机器上进行索引时，这让您能够卸载数据库并节省网络流量。此功能仅在构建时可用 zlib 和 zlib-devel 两者都可用时才可用。

### MSSQL

MS SQL Windows 身份验证标志。是否在连接到 MS SQL Server 时使用当前登录的 Windows 账户凭证进行身份验证。

```ini
mssql_winauth = 1
```

### ODBC

使用 ODBC 的源需要 DSN（数据源名称）字符串的存在，可以使用 `odbc_dsn` 设置。

```ini
odbc_dsn = Driver={Oracle ODBC Driver};Dbq=myDBName;Uid=myUsername;Pwd=myPassword
```

请注意，格式取决于所使用的具体 ODBC 驱动程序。
<!-- proofread -->
