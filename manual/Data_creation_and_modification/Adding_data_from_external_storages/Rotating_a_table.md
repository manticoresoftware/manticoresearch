# Rotating a table

Table rotation is a procedure in which the searchd server looks upon new versions of defined tables in the configuration. Rotation is subject only to Plain mode of operation.

There can be two cases:

* for plain tables that are already loaded
* tables added in configuration, but not loaded yet

In the first case, indexer cannot put the new version of the table online as the running copy is locked and loaded by `searchd`. In this case `indexer` needs to be called with [--rotate](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) parameter. If rotate is used, `indexer` creates new table files with `.new.` in their names and sends a *HUP* signal to `searchd` informing it about the new version. The `searchd` will perform a lookup and will put in place the new version of the table and discard the old one. In some cases it might be desired to create the new version of the table but not perform the rotate as soon as possible. For example it might be desired to check first the health of the new table versions. In this case, `indexer` can accept `--nohup` parameter which will forbid sending the HUP signal to the server.

New tables can be loaded by rotation, however the regular handling of HUP signal is to check for new tables only if configuration has changed since server startup. If the table was already defined in the configuration, the table should be first created by running `indexer` without rotation and perform [RELOAD TABLES](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) statement instead.


There are also two specialized statements can be used to perform rotations on tables:

## RELOAD TABLE

```sql
RELOAD TABLE tbl [ FROM '/path/to/table_files' ];
```

`RELOAD TABLE` allows you to rotate tables using SQL.

It has two modes of operation. First one (without specifying a path) makes Manticore server check for new table files in directory specified in [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path). New table files must have names `tbl.new.sp?`.

And if you additionally specify a path, the server will look for the table files in the specified directory, will move them to the table [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path), rename from `tbl.sp?` to `tbl.new.sp?` and will rotate them.

```sql
mysql> RELOAD TABLE plain_table;
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new_table_files';
```

## RELOAD TABLES

```sql
RELOAD TABLES;
```

Works same as system HUP signal. Initiates table rotation. Unlike regular HUP signalling (which can come from `kill` or indexer ), the statement forces lookup on possible tables to rotate even if the configuration has no changes since the startup of the server.

Depending on the value of [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) setting, new queries might be shortly stalled; clients will receive temporary errors. Command is non-blocking (i.e., returns immediately).

```sql
mysql> RELOAD TABLES;
Query OK, 0 rows affected (0.01 sec)
```

# Seamless rotate

The rotate assumes old table version is discarded and new table version is loaded and replace the existing one. During this swapping, the server needs also to serve incoming queries made on the table that is going to be updated. To not have stalls of the queries, the server implements by default a seamless rotate of the table as described below.

Tables may contain some data that needs to be precached in RAM. At the moment, `.spa`, `.spb`, `.spi` and `.spm` files are fully precached (they contain attribute data, blob attribute data, keyword table and killed row map, respectively.) Without seamless rotate, rotating a table tries to use as little RAM as possible and works as follows:

1. new queries are temporarily rejected (with "retry" error code);
2. `searchd` waits for all currently running queries to finish;
3. old table is deallocated and its files are renamed;
4. new table files are renamed and required RAM is allocated;
5. new table attribute and dictionary data is preloaded to RAM;
6. `searchd` resumes serving queries from new table.

However, if there's a lot of attribute or dictionary data, then preloading step could take noticeable time - up to several minutes in case of preloading 1-5+ GB files.

With seamless rotate enabled, rotation works as follows:

1. new table RAM storage is allocated
2. new table attribute and dictionary data is asynchronously preloaded to RAM
3. on success, old table is deallocated and both tables' files are renamed
4. on failure, new table is deallocated
5. at any given moment, queries are served either from old or new table copy

Seamless rotate comes at the cost of higher **peak** memory usage during the rotation (because both old and new copies of `.spa/.spb/.spi/.spm` data need to be in RAM while preloading new copy). Average usage stays the same.

Example:

```ini
seamless_rotate = 1
```
