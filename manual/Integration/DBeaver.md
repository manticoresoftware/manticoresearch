# Integration with DBeaver

[DBeaver](https://dbeaver.io/) is a SQL client software application and a database administration tool. For MySQL databases, it applies the JDBC application programming interface to interact with them via a JDBC driver. 

Manticore allows you to use DBEaver for working with data stored in Manticore tables the same way as if it was stored in a MySQL database.

## Settings to use

To start working with Manticore in DBeaver, do the following steps:

- Choose a `New database connection` option in DBeaver's UI
- Choose `SQL` -> `MySQL` as DBeaver's database driver 
- Set the `Server host` and `Port` options corresponding to the host and port of your Manticore instance  (keep `database` field empty) 
- Set `root/<empty password>` as authentication credentials.


## Functions avaliable

Since Manticore does not support MySQL in full, only a part of DBeaver's functionality is available when working with Manticore.

You will be able to: 

- view, create, delete and rename tables
- add and drop table columns 
- insert, delete and update column data.

You will not be able to:

- use database integrity checks mechanisms ( `MyISAM` will be set as the only storage engine available)
- use MySQL procedures, triggers, events, etc.
- manage database users
- set other database administration options


## Data type handling

Some MySQL data types are not currently supported by Manticore and, therefore, cannot be used when creating a new table with DBeaver. Also, a few of the supported data types are converted to the most similar Manticore types with type precision being ignored in such conversion. Below is the list of supported MySQL data types as well as the Manticore types they are mapped to:

- `BIGINT UNSIGNED` => `bigint`
- `BOOL` => `boolean`
- `DATE`, `DATETIME`, `TIMESTAMP`  => `timestamp`
- `FLOAT` => `float`
- `INT` => `int`
- `INT UNSIGNED`, `SMALLINT UNSIGNED`, `TINYINT UNSIGNED`, `BIT` => `uint`
- `JSON` => `json`
- `TEXT`, `LONGTEXT`, `MEDIUMTEXT`, `TINYTEXT`, `BLOB`, `LONGBLOB`, `MEDIUMBLOB`, `TINYBLOB`  => `text`
- `VARCHAR`, `LONG VARCHAR`, `BINARY`, `CHAR`, 'VARBINARY', `LONG VARBINARY`  => `string`

You can find more details about Manticore data types [here](Creating_a_table/Data_types#Data-types).

### About date types

Manticore is able to handle the `DATE`, `DATETIME` and `TIMESTAMP` data types, however, this reqiures Manticore's [Buddy](Starting_the_server/Docker#Manticore-Columnar-Library-and-Manticore-Buddy) enabled. Otherwise, an attempt to operate with one of these types will result in an error.

Note that the `TIME` type is not supported.
  
