# MySQL protocol

Manticore Search implements an SQL interface using MySQL protocol, which allows any MySQL client, library or connector to be used for connecting to Manticore Search and work with it as if it would be MySQL server, not Manticore.

However the SQL dialect is different. It implements only a subset of SQL commands or functions available in MySQL. In addition, there are clauses and functions that are specific to Manticore Search. The most eloquent example is the `MATCH()` clause which allows setting the full-text search.

Manticore Search doesn't support server-side prepared statements. Client-side prepared statements can be used with Manticore. It must be noted that Manticore implements the multi value (MVA) data type for which there is no equivalent in MySQL or libraries implementing prepared statements. In these cases, the MVA values will need to be crafted in the raw query.

Some MySQL clients/connectors demand values for user/password and/or database name. Since Manticore Search does not have the concept of database and there is no user access control yet implemented, these can be set  arbitrarily as Manticore will simply ignore the values.

## Configuration

**The default port for the SQL interface is 9306** and it's enabled by default. 

In the searchd section of the configuration file the MySQL port can be defined by `listen` directive like this:

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
...
}
``` 

Because Manticore doesn't have yet user authentication implemented make sure the MySQL port can't be accessed by anyone outside your network.

### VIP connection
A separate MySQL port can be used to perform 'VIP' connections. A connection to this port bypasses the thread pool and always forcibly creates a new dedicated thread. That's useful for managing in case of a severe overload when the server would either stall or not let you connect via a regular port.

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
   listen = 127.0.0.1:9307:mysql_vip
...
}
``` 

## Connecting via standard MySQL client
The easiest way to connect to Manticore is by using a standard MySQL client:

```shell
mysql -P9306 -h0
```

## Secured MySQL connection

The MySQL protocol supports [SSL encryption](Security/SSL.md). The secured connections can be made on the same `mysql` listening port.

## Compressed MySQL connection

Compression can be used with MySQL Connections and available to clients by default. The client just need to specify the connection to use compression.

An example with the MySQL client:

```shell
mysql -P9306 -h0 -C
```

Compression can be used in both secured and non-secured connections.
 
## Notes on MySQL connectors
The official MySQL connectors can be used to connect to Manticore Search, however they might require certain settings passed in the DSN string as the connector can try running certain SQL commands not implemented yet in Manticore.

JDBC Connector 6.x and above require Manticore Search 2.8.2 or greater and the DSN string should contain the following options:
```ini
jdbc:mysql://IP:PORT/DB/?characterEncoding=utf8&maxAllowedPacket=512000&serverTimezone=XXX
```

By default Manticore Search will report it's own version to the connector, however this may cause some troubles. To overcome that `mysql_version_string` directive in searchd section of the configuration should be set to a version lower than 5.1.1:

```ini
searchd {
...
   mysql_version_string = 5.0.37
...
}
``` 

.NET MySQL connector uses connection pools by default. To correctly get the statistics of `SHOW META`, queries along with `SHOW META` command should be sent as a single multistatement (`SELECT ...;SHOW META`). If pooling is enabled option `Allow Batch=True` is required to be added to the connection string to allow multistatements:
```ini
Server=127.0.0.1;Port=9306;Database=somevalue;Uid=somevalue;Pwd=;Allow Batch=True;
```

## Notes on ODBC connectivity
Manticore can be accessed using ODBC. It's recommended to set `charset=UTF8` in the ODBC string. Some ODBC drivers will not like the reported version by the Manticore server as they will see it as a very old MySQL server. This can be overridden with `mysql_version_string` option.

## Comment syntax

Manticore SQL over MySQL supports C-style comment syntax. Everything from an opening `/*` sequence to a closing `*/` sequence is ignored. Comments can span multiple lines, can not nest, and should not get logged. MySQL specific `/*! ... */` comments are also currently ignored. (As the comments support was rather added for better compatibility with `mysqldump` produced dumps, rather than improving general query interoperability between Manticore and MySQL.)

```sql
SELECT /*! SQL_CALC_FOUND_ROWS */ col1 FROM table1 WHERE ...
```
