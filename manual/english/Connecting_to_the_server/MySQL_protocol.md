# MySQL protocol

Manticore Search implements an SQL interface using the MySQL protocol, allowing any MySQL library or connector and many MySQL clients to be used to connect to Manticore Search and work with it as if it were a MySQL server, not Manticore.

However, the SQL dialect is different and implements only a subset of the SQL commands or functions available in MySQL. Additionally, there are clauses and functions that are specific to Manticore Search, such as the `MATCH()` clause for full-text search.

Manticore Search supports server-side [prepared statements](../Connecting_to_the_server/MySQL_protocol.md#prepared-statements) over the MySQL protocol. Client-side prepared statements can also be used. It is important to note that Manticore implements the multi-value (MVA) and `float_vector` data types, which have no equivalents in MySQL or libraries implementing prepared statements. In these cases, values must be crafted in the raw query as comma-separated list of numbers.

Some MySQL clients/connectors require values for user/password and/or database name. Since Manticore Search does not have the concept of databases and there is no user access control implemented, these values can be set arbitrarily as Manticore will simply ignore them.

## Configuration

**The default port for the SQL interface is 9306** and it's enabled by default.

You can configure the MySQL port in the searchd section of the configuration file using the `listen` directive like this:

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
...
}
```

Keep in mind that Manticore doesn't have user authentication, so make sure that the MySQL port is not accessible to anyone outside of your network.

### VIP connection
A separate MySQL port can be used for performing "VIP" connections. When connecting to this port, the thread pool is bypassed, and a new dedicated thread is always created. This is useful in cases of severe overload, where the server would either stall or prevent a connection through the regular port.

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

The MySQL protocol supports [SSL encryption](../Security/SSL.md). Secure connections can be made on the same `mysql` listening port.

## Compressed MySQL connection

Compression can be used with MySQL connections and is available to clients by default. The client just needs to specify that the connection should use compression.

An example using the MySQL client:

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

# Prepared statements

Manticore supports prepared statements over MySQL protocol. 

In databases, [prepared statements](https://en.wikipedia.org/wiki/Prepared_statement) are a way to run queries where the SQL code is kept separate from the input data. Instead of writing a full SQL query with the values included directly in it, you write the query once using placeholders and then supply the values separately.

Some client libraries (for example, [sqlx](https://docs.rs/sqlx/latest/sqlx/)) communicate with databases only in this way.

## Why are prepared statements important?

* **Security (Preventing SQL Injection):** This is the most critical reason. SQL injection is a common web vulnerability in which attackers insert malicious SQL into queries. Prepared statements ensure that user input is treated strictly as data, not executable code, preventing it from being interpreted as part of the SQL command.

* **Readability & Maintainability:** Prepared statements improve code clarity by separating SQL logic from input data, making the code easier to read and maintain.

* **Performance:** The database can cache query plans. If the same parameterized query runs multiple times with different values, the database can reuse the cached execution plan instead of parsing and optimizing the query each time, which can improve performance.

<!-- example prepared-statements -->
## How Prepared Statements Work

1. **Prepare the query:** Send the SQL statement with placeholders (such as `?` or `?VEC?`) to Manticore. Manticore parses and compiles the statement, stores it, and returns a unique ID that can be used to execute it later.
2. **Bind parameters:** Send the values for the placeholders separately. Manticore treats these values strictly as data (not SQL code) and safely inserts them into the stored statement.
3. **Execute the query:** Manticore executes the statement using the precompiled plan together with the provided parameter values.

<!-- intro -->
### Examples

<!-- request SQL -->
```
INSERT INTO table (id, floatvec) VALUES (?, (?VEC?))
```

<!-- request PHP -->
```php
$stmt = $mysqli->prepare("INSERT INTO tbl (id, str, floatvec) VALUES (0, ?, (?VEC?))");
$str = "I'm a string";
$vec = "0.1,0.2,0.3";
$stmt->bind_param("ss", $str, $vec);
$stmt->execute();
```

<!-- response PHP -->

This will execute:
```
INSERT INTO tbl (id, str, floatvec) VALUES (0, 'I\'m a string', (0.1,0.2,0.3))
```

<!-- end -->

## Parameter Placeholders

* `?` (question mark): Represents a single parameter (integer, float, or string). String values are processed automatically — for example, special characters such as single quotes are escaped, and the value is enclosed in quotes. Boolean values can be passed as strings (`'true'`, `'false'`).
* `?VEC?`: Represents a list of numeric values provided as a string (for example, `1,2,3` or `0.3,15E+3, 20 ,INF`). Only numbers, commas, and spaces are allowed. After validation, the values are inserted exactly as provided (without additional escaping or quotes).

## Vector Parameters (MVA/float_vector)

In the standard Manticore SQL syntax [MVA](../Data_types.md#Multi-value-integer-%28MVA%29) or [float vectors](Creating_a_table/Data_types.md#Float-vector) are written as a list of values enclosed in parentheses, for example `(1, 2, 3)`. 

When using prepared statements, include the parentheses directly in the query, and use the `?VEC?` placeholder only for the values inside them. For example:

```php
$stmt = $mysqli->prepare("INSERT INTO tbl (id, floatvec) VALUES (0, (?VEC?))");
$vec = "0.1,0.2,0.3";
$stmt->bind_param("s", $vec);
```

## Limitations

1. Only one SQL statement is allowed per prepared statement. [Multi-queries](../Searching/Multi-queries.md#Multi-queries) (for example, `SELECT ...; SHOW META`) are not supported. If you need to run multiple statements, prepare a separate statement for each one and execute them in sequence in the same session.
2. Some drivers send numeric parameters as DOUBLE (e.g., `mysql2` for Node.js). If you need strict integer behavior (such as rejecting negative IDs), send integers as strings or use driver-specific integer types (e.g., `BigInt`).
3. **Rust sqlx:** When reading result set rows, use column indices rather than column names. Column names are present in the result set, but sqlx does not use them for mapping. For example, use `row.try_get(0)?` instead of `row.try_get("id")?`.
