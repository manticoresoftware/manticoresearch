# 数据库连接

源定义必须包含连接的设置，包括主机、端口、用户凭据或驱动程序的特定设置。

## sql_host

要连接的数据库服务器主机。注意，MySQL客户端库基于主机名选择通过TCP/IP或UNIX套接字连接。具体来说，“localhost”将强制使用UNIX套接字（这是默认且通常推荐的模式），而“127.0.0.1”将强制使用TCP/IP。

## sql_port

要连接的服务器IP端口。
对 `mysql` 默认是3306，而对 `pgsql` 是5432。

## sql_db

连接建立后使用的SQL数据库，并在其中执行进一步查询。

## sql_user

用于连接的用户名。

## sql_pass

连接时使用的用户密码。如果密码中包含 `#`（可以在配置文件中用作注释），可以用 `\` 转义它。

## sql_sock

本地数据库服务器要连接的UNIX套接字名。注意这取决于 `sql_host` 设置此值是否会实际被使用。

```ini
sql_sock = /var/lib/mysql/mysql.sock
```

## 驱动程序的特定设置

### MySQL

#### mysql_connect_flags

MySQL客户端连接标志。可选，默认值为0（不设置任何标志）。

该选项必须包含标志的整数和。该值将按原样传递给 mysql_real_connect()。标志在 mysql_com.h 头文件中列出。与索引相关尤其有趣的标志及其对应值如下：

* CLIENT_COMPRESS = 32；可以使用压缩协议
* CLIENT_SSL = 2048；握手后切换到SSL
* CLIENT_SECURE_CONNECTION = 32768；新的4.1认证
例如，你可以指定2080（2048+32）以同时使用压缩和SSL，或指定32768仅使用新认证。最初引入此选项是为了能在索引器和mysqld处于不同主机时使用压缩。虽然压缩减少了网络流量，在1 Gbps链路上可能反而影响索引时间，无论是在理论上还是实践中。不过，在100 Mbps链路上启用压缩可能显著改善索引时间（报告显示总索引时间改善达20-30%）。具体效果可能因情况而异。

```ini
mysql_connect_flags = 32 # enable compression
```

### SSL证书设置

* `mysql_ssl_cert` - SSL证书路径
* `mysql_ssl_key` - SSL密钥文件路径
* `mysql_ssl_ca` - CA证书路径


### unpack_mysqlcompress

```ini
unpack_mysqlcompress_maxsize = 1M
```

使用MySQL `UNCOMPRESS()`算法解压的列。多值，可选，默认值为空列列表。

通过此指令指定的列将由索引器使用MySQL `COMPRESS()`和`UNCOMPRESS()`函数采用的修改zlib算法解压。当索引器部署在与数据库不同的机器上时，可以减轻数据库负载并节省网络流量。此功能仅在构建时同时存在zlib和zlib-devel库时可用。

```ini
unpack_mysqlcompress = body_compressed
unpack_mysqlcompress = description_compressed
```

默认情况下，解压数据使用16M的缓冲区。可以通过设置 `unpack_mysqlcompress_maxsize` 来更改。

使用 unpack_mysqlcompress 时，由于实现细节，无法从压缩数据中推断所需缓冲区大小。因此缓冲区必须提前预分配，解压后的数据不能超过缓冲区大小。

### unpack_zlib

```ini
unpack_zlib = col1
unpack_zlib = col2
```

使用zlib（即deflate，也称gunzip）解压的列。多值，可选，默认值为空列列表。仅适用于`mysql`和`pgsql`源类型。

通过此指令指定的列将由 `indexer` 使用标准zlib算法（称为deflate，也由`gunzip`实现）解压。当索引器部署在与数据库不同的机器上时，可以减轻数据库负载并节省网络流量。此功能仅在构建时同时存在zlib和zlib-devel库时可用。

### MSSQL

MS SQL Windows身份认证标志。连接MS SQL Server时是否使用当前登录的Windows账户凭据进行认证。

```ini
mssql_winauth = 1
```

### ODBC

使用ODBC的源需要存在DSN（数据源名称）字符串，可以用 `odbc_dsn` 设置。

```ini
odbc_dsn = Driver={Oracle ODBC Driver};Dbq=myDBName;Uid=myUsername;Pwd=myPassword
```

请注意格式依赖于所使用的具体ODBC驱动程序。
<!-- proofread -->

