# Creating a local table

In Manticore Search, there are two ways to manage tables:
## Online schema management (RT mode)
Real-time mode requires no table definition in the configuration file. However, the [data_dir](../Server_settings/Searchd.md#data_dir) directive in the `searchd` section is mandatory. Index files are stored inside the `data_dir`.

Replication is only available in this mode.

You can use SQL commands such as `CREATE TABLE`, `ALTER TABLE` and `DROP TABLE` to create and modify table schema, and to drop it. This mode is particularly useful for **real-time** and **percolate tables**.

Table names are converted to lowercase when created.

## Defining table schema in config (Plain mode)
In this mode, you can specify the table schema in the configuration file. Manticore reads this schema on startup and creates the table if it doesn't exist yet. This mode is particularly useful for **plain tables** that use data from an external storage.

To drop a table, remove it from the configuration file or remove the path setting and send a HUP signal to the server or restart it.

Table names are case-sensitive in this mode.

All table types are supported in this mode.


## Table types and modes


| Table type  | RT mode        | Plain mode  |
|-------------|----------------|-------------|
| Real-time   | supported      | supported   |
| Plain       | not supported  | supported   |
| Percolate   | supported      | supported   |
| Distributed | supported      | supported   |
| Template    | not supported  | supported   |

<!-- proofread -->
