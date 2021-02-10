# Rotating an index 

Index rotation is a procedure in which the searchd server looks upon new versions of defined indexes in the configuration. Rotation is subject only to Plain mode of operation.

There can be two cases:

* for plain indexes that are already loaded
* indexes added in configuration, but not loaded yet

In the first case, indexer cannot put the new version of the index online as the running copy is locked and loaded by `searchd`. In this case `indexer` needs to be called with [--rotate](../Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-command-line-arguments) parameter. If rotate is used, indexes creates  new index files with `.new.` in their name and sends a *HUP* signal to `searchd` informing it about the new version. The `searchd` will perform a lookup and will put in place the new version of the index and discard the old one. In some cases it might be desired to create the new version of the index but not perform the rotate as soon as possible. For example it might be desired to check first the health of the new index versions. In this case, `indexer` can accept ``--nohup`` parameter which will forbid sending the HUP signal to the server.

New indexes can be loaded by rotation, however the regular handling of HUP signal is to check for new indexes only if configuration has changed since server startup. If the index was already defined in the configuration, the index should be first created by running `indexer` without rotation and perform [RELOAD INDEXES](../Adding_data_from_external_storages/Rotating_an_index.md#RELOAD-INDEXES) statement instead.


There are also two specialized statements can be used to perform rotations on indexes:

## RELOAD INDEX

```sql
RELOAD INDEX idx [ FROM '/path/to/index_files' ];
```

`RELOAD INDEX` allows you to rotate indexes using SQL.

It has two modes of operation. First one (without specifying a path) makes Manticore server check for new index files in directory specified in [path](../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#path). New index files must have a idx.new.sp? names.

And if you additionally specify a path, server will look for index files in specified directory, move them to index [path](../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#path), rename from index\_files.sp? to idx.new.sp? and rotate them.

```sql
mysql> RELOAD INDEX plain_index;
mysql> RELOAD INDEX plain_index FROM '/home/mighty/new_index_files';
```

## RELOAD INDEXES 

```sql
RELOAD INDEXES;
```

Works same as system HUP signal. Initiates index rotation. Unlike regular HUP signalling (which can come from `kill` or indexer ), the statement forces lookup on possible indexes to rotate even if the configuration has no changes since the startup of the server.

Depending on the value of [seamless_rotate](../Server_settings/Searchd.md#seamless_rotate) setting, new queries might be shortly stalled; clients will receive temporary errors. Command is non-blocking (i.e., returns immediately).

```sql
mysql> RELOAD INDEXES;
Query OK, 0 rows affected (0.01 sec)
```

# Seamless rotate

The rotate assumes old index version is discarded and new index version is loaded and replace the existing one. During this swapping, the server needs also to serve incoming queries made on the index that is going to be updated. To not have stalls of the queries, the server implements by default a seamless rotate of the index as described below. 

Indexes may contain some data that needs to be precached in RAM. At the moment, ``.spa``, ``.spb``, ``.spi`` and ``.spm`` files are fully precached (they contain attribute data, blob attribute data, keyword index and killed row map, respectively.) Without seamless rotate, rotating an index tries to use as little RAM as possible and works as follows:

1. new queries are temporarily rejected (with "retry" error code);
2. ``searchd`` waits for all currently running queries to finish;
3. old index is deallocated and its files are renamed;
4. new index files are renamed and required RAM is allocated;
5. new index attribute and dictionary data is preloaded to RAM;
6. ``searchd`` resumes serving queries from new index.

However, if there's a lot of attribute or dictionary data, then preloading step could take noticeable time - up to several minutes in case of preloading 1-5+ GB files.

With seamless rotate enabled, rotation works as follows:

1. new index RAM storage is allocated
2. new index attribute and dictionary data is asynchronously preloaded to RAM
3. on success, old index is deallocated and both indexes' files are renamed
4. on failure, new index is deallocated
5. at any given moment, queries are served either from old or new index copy

Seamless rotate comes at the cost of higher **peak** memory usage during the rotation (because both old and new copies of ``.spa/.spb/.spi/.spm`` data need to be in RAM while preloading new copy). Average usage stays the same.

Example:

```ini
seamless_rotate = 1
```