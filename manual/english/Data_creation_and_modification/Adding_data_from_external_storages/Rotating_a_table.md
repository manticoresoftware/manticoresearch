# Rotating a table

Table rotation is a procedure in which the searchd server looks for new versions of defined tables in the configuration. Rotation is supported only in Plain mode of operation.

There can be two cases:

* for plain tables that are already loaded
* tables added in configuration, but not loaded yet

In the first case, the indexer cannot put the new version of the table online as the running copy is locked and loaded by `searchd`. In this case `indexer` needs to be called with the [--rotate](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) parameter. If rotate is used, `indexer` creates new table files with `.new.` in their names and sends a *HUP* signal to `searchd` informing it about the new version. The `searchd` will perform a lookup and will put the new version of the table in place and discard the old one. In some cases, it might be desired to create the new version of the table but not perform rotate as soon as possible. For example, it might be desired to first check the health of the new table versions. In this case,  `indexer` can accept the`--nohup` parameter which will forbid sending the HUP signal to the server.

New tables can be loaded by rotation; however, the regular handling of the HUP signal is to check for new tables only if the configuration has changed since server startup. If the table was already defined in the configuration, the table should be first created by running `indexer` without rotation and perform the [RELOAD TABLES](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) statement instead.

There are also two specialized statements that can be used to perform rotations on tables:

## RELOAD TABLE

```sql
RELOAD TABLE tbl [ FROM '/path/to/table_files' [ OPTION switchover=1 ] ];
```

The `RELOAD TABLE` command enables table rotation via SQL.

This command functions in three modes. In the first mode, without specifying a path, the Manticore server checks for new table files in the directory indicated by the [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path). New table files must be named as `tbl.new.sp?`.

If you specify a path, the server searches for table files in that directory, relocates them to the table [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path), renames them from `tbl.sp?` to `tbl.new.sp?`, and rotates them.

The third mode, activated by `OPTION switchover=1`, switches the index to the new path. Here, the daemon tries to load the table directly from the new path without moving the files. If loading is successful, this new index supersedes the old one.

Also, the daemon writes a unique link file (`tbl.link`) in the directory specified by [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path), maintaining persistent redirection.

If you revert a redirected index to the path specified in the configuration, the daemon will detect this and delete the link file.

Once redirected, the daemon retrieves the table from the newly linked path. When rotating, it looks for new table versions at the newly redirected path. Bear in mind, the daemon checks the configuration for common errors, like duplicate paths across different tables. However, it won't identify if multiple tables point to the same path via redirection. Under normal operations, tables are locked with the `.spl` file, but disabling the lock may cause problems. If there's an error (e.g., the path is inaccessible for any reason), you should manually correct (or simply delete) the link file.

`indextool` follows the link file, but other tools (`indexer`, `index_converter`, etc.) do not recognize the link file and consistently use the path defined in the configuration file, ignoring any redirection. Thus, you can inspect the index with `indextool`, and it will read from the new location. However, more complex operations like merging will not acknowledge any link file.

```sql
mysql> RELOAD TABLE plain_table;
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new_table_files';
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new/place/for/table/table_files' OPTION switchover=1;
```

## RELOAD TABLES

```sql
RELOAD TABLES;
```

This command functions similarly to the HUP system signal, triggering a rotation of tables. Nevertheless, it doesn't exactly mirror the typical HUP signal (which can come from a `kill -HUP` command or `indexer --rotate`). This command actively searches for any tables needing rotation and is capable of re-reading the configuration. Suppose you launch Manticore in plain mode with a config file that points to a nonexistent plain table. If you then attempt to `indexer --rotate` the table, the new table won't be recognized by the server until you execute `RELOAD TABLES` or restart the server.

Depending on the value of the [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) setting, new queries might be shortly stalled, and clients will receive temporary errors.

```sql
mysql> RELOAD TABLES;
Query OK, 0 rows affected (0.01 sec)
```

# Seamless rotate

The rotate assumes old table version is discarded and new table version is loaded and replaces the existing one. During this swapping, the server needs to also serve incoming queries made on the table that is going to be updated. To avoid stalls of the queries, the server implements a seamless rotate of the table by default, as described below.

Tables may contain data that needs to be precached in RAM. At the moment, `.spa`, `.spb`, `.spi` and `.spm` files are fully precached (they contain attribute data, blob attribute data, keyword table, and killed row map, respectively). Without seamless rotate, rotating a table tries to use as little RAM as possible and works as follows:

1. New queries are temporarily rejected (with "retry" error code).
2. `searchd` waits for all currently running queries to finish.
3. Old table is deallocated and its files are renamed.
4. New table files are renamed and required RAM is allocated.
5. New table attribute and dictionary data is preloaded to RAM.
6. `searchd` resumes serving queries from the new table.

However, if there's a lot of attribute or dictionary data, then the preloading step could take noticeable time - up to several minutes in case of preloading 1-5+ GB files.

With seamless rotate enabled, rotation works as follows:

1. New table RAM storage is allocated.
2. New table attribute and dictionary data is asynchronously preloaded to RAM.
3. On success, old table is deallocated and both tables' files are renamed.
4. On failure, new table is deallocated.
5. At any given moment, queries are served either from old or new table copy.

Seamless rotate comes at the cost of higher **peak** memory usage during the rotation (because both old and new copies of `.spa/.spb/.spi/.spm` data need to be in RAM while preloading the new copy). However, average usage stays the same.

Example:

```ini
seamless_rotate = 1
```
<!-- proofread -->
