# Creating a local index

There are 2 different approaches to deal with indexes in Manticore:
## Online schema management (RT mode)
Real-time mode requires no index definition in the configuration file, but presence of [data_dir](../Server_settings/Searchd.md#data_dir) directive in `searchd` section is mandatory. Index files are stored inside the `data_dir`.

Replication is available only in this mode.

In this mode you can use SQL commands like `CREATE TABLE`, `ALTER TABLE` and `DROP TABLE` to create and change index schema and drop it. This mode is especially useful for **real-time** and **percolate indexes**.

Index names are case insensitive in RT mode.

## Defining index schema in config (Plain mode)
In this mode you can specify index schema in config which will be read on Manticore start and if the index doesn't exist yet it will be created. This mode is especially useful for **plain indexes** that are built upon indexing data from an external storage.

Dropping indexes is only possible by removing them from the configuration file or by removing the path setting and sending a HUP signal to the server or restarting it.

Index names are case sensitive in this mode.

All index types are supported in this mode.


## Index types and modes


| Index type  | RT mode        | Plain mode  |
|-------------|----------------|-------------|
| Real-time   | supported      | supported   |
| Plain       | not supported  | supported   |
| Percolate   | supported      | supported   |
| Distributed | supported      | supported   |
| Template    | not supported  | supported   |
