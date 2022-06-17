# Database connection

The source definition must contain the settings of the connection, this includes the host,port, user credentials or specific settings of a driver.

## sql_host

The database server host to connect to. Note that MySQL client library chooses whether to connect over TCP/IP or over UNIX socket based on the host name. Specifically "localhost" will force it to use UNIX socket (this is the default and generally recommended mode) and "127.0.0.1" will force TCP/IP usage.

## sql_port

The server IP port to connect to.
For `mysql` default is 3306 and for `pgsql` is 5432.

## sql_db

The SQL database to use after the connection is established and perform further queries within.

## sql_user

The username used for connecting.

## sql_pass

The user password to use when connecting. If the password includes `#` (which can be used to add comments in the configuration file) you can escape it with `\`.

## sql_sock

UNIX socket name to connect to for local database servers. Note that it depends on `sql_host` setting whether this value will actually be used.

```ini
sql_sock = /var/lib/mysql/mysql.sock
```

## Specific settings for drivers

### MySQL

#### mysql_connect_flags

MySQL client connection flags. Optional, default value is 0 (do not set any flags).

This option must contain an integer value with the sum of the flags. The value will be passed to mysql_real_connect() verbatim. The flags are enumerated in mysql_com.h include file. Flags that are especially interesting in regard to indexing, with their respective values, are as follows:

* CLIENT_COMPRESS = 32; can use compression protocol
* CLIENT_SSL = 2048; switch to SSL after handshake
* CLIENT_SECURE_CONNECTION = 32768; new 4.1 authentication
For instance, you can specify 2080 (2048+32) to use both compression and SSL, or 32768 to use new authentication only. Initially, this option was introduced to be able to use compression when the indexer and mysqld are on different hosts. Compression on 1 Gbps links is most likely to hurt indexing time though it reduces network traffic, both in theory and in practice. However, enabling compression on 100 Mbps links may improve indexing time significantly (up to 20-30% of the total indexing time improvement was reported). Your mileage may vary.

```ini
mysql_connect_flags = 32 # enable compression
```

### SSL certificate settings

* `mysql_ssl_cert` - path to SSL certificate
* `mysql_ssl_key` - path to SSL key file
* `mysql_ssl_ca` - path to CA certificate


### unpack_mysqlcompress

```ini
unpack_mysqlcompress_maxsize = 1M
```

Columns to unpack using MySQL `UNCOMPRESS()` algorithm. Multi-value, optional, default value is empty list of columns.

Columns specified using this directive will be unpacked by indexer using modified zlib algorithm used by MySQL `COMPRESS()` and `UNCOMPRESS()` functions. When indexing on a different box than the database, this lets you offload the database, and save on network traffic. The feature is only available if zlib and zlib-devel were both available during build time.

```ini
unpack_mysqlcompress = body_compressed
unpack_mysqlcompress = description_compressed
```

By default a buffer of 16M is used for uncompressing the data. This can be changed by setting `unpack_mysqlcompress_maxsize`.

When using unpack_mysqlcompress, due to implementation intricacies it is not possible to deduce the required buffer size from the compressed data. So the buffer must be preallocated in advance, and unpacked data can not go over the buffer size.

### unpack_zlib

```ini
unpack_zlib = col1
unpack_zlib = col2
```

Columns to unpack using zlib (aka deflate, aka gunzip). Multi-value, optional, default value is empty list of columns. Applies to source type `mysql` and `pgsql` only.

Columns specified using this directive will be unpacked by `indexer` using standard zlib algorithm (called deflate and also implemented by `gunzip`). When indexing on a different box than the database, this lets you offload the database, and save on network traffic. The feature is only available if zlib and zlib-devel were both available during build time.

### MSSQL

MS SQL Windows authentication flag. Whether to use currently logged in Windows account credentials for authentication when connecting to MS SQL Server.

```ini
mssql_winauth = 1
```

### ODBC

Sources using ODBC requires the presence of a DSN (Data Source Name) string which can be set with `odbc_dsn`.

```ini
odbc_dsn = Driver={Oracle ODBC Driver};Dbq=myDBName;Uid=myUsername;Pwd=myPassword
```

Please note that the format depends on specific ODBC driver used.
