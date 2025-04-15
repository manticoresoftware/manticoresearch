# Integration with DBeaver

> NOTE: The integration with DBeaver requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[DBeaver](https://dbeaver.io/) is a SQL client software application and a database administration tool. For MySQL databases, it applies the JDBC application programming interface to interact with them via a JDBC driver.

Manticore allows you to use DBeaver for working with data stored in Manticore tables the same way as if it was stored in a MySQL database.

## Settings to use

To start working with Manticore in DBeaver, follow these steps:

- Choose the `New database connection` option in DBeaver's UI
- Choose `SQL` -> `MySQL` as DBeaver's database driver
- Set the `Server host` and `Port` options corresponding to the host and port of your Manticore instance (keep the `database` field empty)
- Set `root/<empty password>` as authentication credentials


## Functions available

Since Manticore does not fully support MySQL, only a part of DBeaver's functionality is available when working with Manticore.

You will be able to:
- View, create, delete, and rename tables
- Add and drop table columns
- Insert, delete, and update column data

You will not be able to:
- Use database integrity check mechanisms (`MyISAM` will be set as the only storage engine available)
- Use MySQL procedures, triggers, events, etc.
- Manage database users
- Set other database administration options


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
- `VARCHAR`, `LONG VARCHAR`, `BINARY`, `CHAR`, `VARBINARY`, `LONG VARBINARY`  => `string`

You can find more details about Manticore data types [here](Creating_a_table/Data_types.md#Data-types).

### About date types

Manticore is able to handle the `DATE`, `DATETIME` and `TIMESTAMP` data types, however, this reqiures Manticore's [Buddy](Starting_the_server/Docker.md#Manticore-Columnar-Library-and-Manticore-Buddy) enabled. Otherwise, an attempt to operate with one of these types will result in an error.

Note that the `TIME` type is not supported.

## Possible caveats

- DBeaver's `Preferences` -> `Connections` -> `Client identification` option must not be turned off or overridden.
  To work correctly with DBeaver, Manticore needs to distinguish its requests from others. For this, it uses client notification info sent by DBeaver in request headers. Disabling client notification will break that detection and, therefore, Manticore's correct functionality.

- When trying to update data in your table for the first time, you'll see the `No unique key` popup message and will be asked to define a custom unique key.
  When you get this message, perform the following steps:

  - Choose the `Custom Unique Key` option
  - Choose only the `id` column in the columns list
  - Press `Ok`

  After that, you'll be able to update your data safely.
