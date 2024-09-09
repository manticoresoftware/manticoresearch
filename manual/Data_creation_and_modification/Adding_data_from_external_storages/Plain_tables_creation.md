# Plain tables creation

Plain tables are tables that are created one-time by fetching data at creation from one or several sources. A plain table is immutable as documents cannot be added or deleted during its lifespan. It is only possible to update values of numeric attributes (including MVA). Refreshing the data is only possible by recreating the whole table.

Plain tables are available only in the [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) and their definition is made up of a table declaration and one or several source declarations. The data gathering and table creation are not made by the `searchd` server but by the auxiliary tool `indexer`.

**Indexer** is a command-line tool that can be called directly from the command line or from shell scripts.

It can accept a number of arguments when called, but there are also several settings of its own in the Manticore configuration file.

In the typical scenario, indexer does the following:
* Fetches the data from the source
* Builds the plain table
* Writes the table files
* (Optional) Informs the search server about the new table which triggers table rotation

## Indexer tool
The `indexer` tool is used to create plain tables in Manticore Search. It has a general syntax of:

```shell
indexer [OPTIONS] [table_name1 [table_name2 [...]]]
```

When creating tables with `indexer`, the generated table files must be made with permissions that allow `searchd` to read, write, and delete them. In case of the official Linux packages, `searchd` runs under the `manticore` user. Therefore, `indexer` must also run under the `manticore` user:

```shell
sudo -u manticore indexer ...
```

If you are running `searchd` differently, you might need to omit `sudo -u manticore`. Just make sure that the user under which your `searchd` instance is running has read/write permissions to the tables generated using `indexer`.

To create a plain table, you need to list the table(s) you want to process. For example, if your `manticore.conf` file contains details on two tables, `mybigindex` and `mysmallindex`, you could run:

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

You can also use wildcard tokens to match table names:

* `?` matches any single character
* `*` matches any count of any characters
* `%` matches none or any single character

```shell
sudo -u manticore indexer indexpart*main --rotate
```

The exit codes for indexer are as follows:

* 0: everything went OK
* 1: there was a problem while indexing (and if `--rotate` was specified, it was skipped) or an operation emitted a warning
* 2: indexing went OK, but the `--rotate` attempt failed

### Indexer systemd service

You can also start `indexer` using the following systemctl unit file:

```shell
systemctl start --no-block manticore-indexer
```

Or, in case you want to build a specific table:

```shell
systemctl start --no-block manticore-indexer@specific-table-name
```

Use the `systemctl set-environment INDEXER_CONFIG` command to run the Indexer with a custom configuration, which replaces the default settings.

The `systemctl set-environment INDEXER_ARGS` command lets you add custom startup options for the Indexer. For a complete list of command-line options, see [here](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments).

For instance, to start the Indexer in quiet mode, run:
```bash
systemctl set-environment INDEXER_ARGS='--quiet'
systemctl restart manticore-indexer
```

To revert the changes, run:
```bash
systemctl set-environment INDEXER_ARGS=''
systemctl restart manticore-indexer
```

### Indexer command line arguments
* `--config <file>` (`-c <file>` for short) tells `indexer` to use the given file as its configuration. Normally, it will look for `manticore.conf` in the installation directory (e.g. `/etc/manticoresearch/manticore.conf`), followed by the current directory you are in when calling `indexer` from the shell. This is most useful in shared environments where the binary files are installed in a global folder, e.g. `/usr/bin/`, but you want to provide users with the ability to make their own custom Manticore set-ups, or if you want to run multiple instances on a single server. In cases like those you could allow them to create their own `manticore.conf` files and pass them to `indexer` with this option. For example:

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf mytable
  ```

* `--all` tells `indexer` to update every table listed in `manticore.conf` instead of listing individual tables. This would be useful in small configurations or cron-kind or maintenance jobs where the entire table set will get rebuilt each day or week or whatever period is best. Please note that since `--all` tries to update all found tables in the configuration, it will issue a warning if it encounters RealTime tables and the exit code of the command will be `1` not `0` even if the plain tables finished without issue. Example usage:

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --all
  ```

* `--rotate` is used for rotating tables. Unless you have the situation where you can take the search function offline without troubling users you will almost certainly need to keep search running whilst indexing new documents. `--rotate` creates a second table, parallel to the first (in the same place, simply including `.new` in the filenames). Once complete, `indexer` notifies `searchd` via sending the `SIGHUP` signal, and the `searchd` will attempt to rename the tables (renaming the existing ones to include `.old` and renaming the `.new` to replace them), and then will start serving from the newer files. Depending on the setting of [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) there may be a slight delay in being able to search the newer tables. In case multiple tables are rotated at once which are chained by [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) relations rotation will start with the tables that are not targets and finish with the ones at the end of target chain. Example usage:

  ```shell
  sudo -u manticore indexer --rotate --all
  ```
* `--quiet` tells `indexer` ot to output anything, unless there is an error. This is mostly used for cron-type or other scripted jobs where the output is irrelevant or unnecessary, except in the event of some kind of error. Example usage:

  ```shell
  sudo -u manticore indexer --rotate --all --quiet
  ```
* `--noprogress` does not display progress details as they occur. Instead, the final status details (such as documents indexed, speed of indexing and so on are only reported at completion of indexing. In instances where the script is not being run on a console (or 'tty'), this will be on by default. Example usage:

  ```shell
  sudo -u manticore indexer --rotate --all --noprogress
  ```
* `--buildstops <outputfile.text> <N>` reviews the table source, as if it were indexing the data, and produces a list of the terms that are being indexed. In other words, it produces a list of all the searchable terms that are becoming part of the table. Note, it does not update the table in question, it simply processes the data as if it were indexing, including running queries defined with [sql_query_pre](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_pre) or [sql_query_post](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_post). `outputfile.txt` will contain the list of words, one per line, sorted by frequency with most frequent first, and `N` specifies the maximum number of words that will be listed. If it's sufficiently large to encompass every word in the table, only that many words will be returned. Such a dictionary list could be used for client application features around "Did you mean…" functionality, usually in conjunction with `--buildfreqs`, below. Example:

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000
  ```

  This would produce a document in the current directory, `word_freq.txt`, with the 1,000 most common words in 'mytable', ordered by most common first. Note that the file will pertain to the last table indexed when specified with multiple tables or `--all` (i.e. the last one listed in the configuration file)

* `--buildfreqs` works with `--buildstops` (and is ignored if `--buildstops` is not specified). As `--buildstops` provides the list of words used within the table, `--buildfreqs` adds the quantity present in the table, which would be useful in establishing whether certain words should be considered stopwords if they are too prevalent. It will also help with developing "Did you mean…" features where you need to know how much more common a given word compared to another, similar one. For example:

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000 --buildfreqs
  ```

  This would produce the `word_freq.txt` as above, however after each word would be the number of times it occurred in the table in question.

* `--merge <dst-table> <src-table>` is used for physically merging tables together, for example if you have a [main+delta scheme](../../Creating_a_table/Local_tables/Plain_table.md#Main+delta-scenario), where the main table rarely changes, but the delta table is rebuilt frequently, and `--merge` would be used to combine the two. The operation moves from right to left - the contents of `src-table` get examined and physically combined with the contents of `dst-table` and the result is left in `dst-table`. In pseudo-code, it might be expressed as: `dst-table += src-table` An example:

  ```shell
  sudo -u manticore indexer --merge main delta --rotate
  ```

  In the above example, where the main is the master, rarely modified table, and the delta is more frequently modified one, you might use the above to call `indexer` to combine the contents of the delta into the main table and rotate the tables.

* `--merge-dst-range <attr> <min> <max>` runs the filter range given upon merging. Specifically, as the merge is applied to the destination table (as part of `--merge`, and is ignored if `--merge` is not specified), `indexer` will also filter the documents ending up in the destination table, and only documents will pass through the filter given will end up in the final table. This could be used for example, in a table where there is a 'deleted' attribute, where 0 means 'not deleted'. Such a table could be merged with:

  ```shell
  sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
  ```

  Any documents marked as deleted (value 1) will be removed from the newly-merged destination table. It can be added several times to the command line, to add successive filters to the merge, all of which must be met in order for a document to become part of the final table.

* --`merge-killlists` (and its shorter alias `--merge-klists`) changes the way kill lists are processed when merging tables. By default, both kill lists get discarded after a merge. That supports the most typical main+delta merge scenario. With this option enabled, however, kill lists from both tables get concatenated and stored into the destination table. Note that a source (delta) table kill list will be used to suppress rows from a destination (main) table at all times.
* `--keep-attrs` allows to reuse existing attributes on reindexing. Whenever the table is rebuilt, each new document id is checked for presence in the "old" table, and if it already exists, its attributes are transferred to the "new" table; if not found, attributes from the new table are used. If the user has updated attributes in the table, but not in the actual source used for the table, all updates will be lost when reindexing; using `--keep-attrs` enables saving the updated attribute values from the previous table. It is possible to specify a path for table files to be used instead of the reference path from the config:

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/index/files
  ```

* `--keep-attrs-names=<attributes list>` allows you to specify attributes to reuse from an existing table on reindexing. By default, all attributes from the existing table are reused in the new table:

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/table/files --keep-attrs-names=update,state
  ```

* `--dump-rows <FILE>` dumps rows fetched by SQL source(s) into the specified file, in a MySQL compatible syntax. The resulting dumps are the exact representation of data as received by `indexer` and can help repeat indexing-time issues. The command performs fetching from the source and creates both table files and the dump file.
* `--print-rt <rt_index> <table>` outputs fetched data from the source as INSERTs for a real-time table. The first lines of the dump will contain the real-time fields and attributes (as a reflection of the plain table fields and attributes). The command performs fetching from the source and creates both table files and the dump output. The command can be used as `sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`. Only SQL-based sources are supported. MVAs are not supported.
* `--sighup-each`  is useful when you are rebuilding many big tables and want each one rotated into `searchd` as soon as possible. With `--sighup-each`, `indexer` will send the SIGHUP signal to searchd after successfully completing work on each table. (The default behavior is to send a single SIGHUP after all the tables are built).
* `--nohup` is useful when you want to check your table with indextool before actually rotating it. indexer won't send the SIGHUP if this option is on. Table files are renamed to .tmp. Use indextool to rename table files to .new and rotate it. Example usage:

  ```shell
  sudo -u manticore indexer --rotate --nohup mytable
  sudo -u manticore indextool --rotate --check mytable
  ```

* `--print-queries` prints out SQL queries that `indexer` sends to the database, along with SQL connection and disconnection events. That is useful to diagnose and fix problems with SQL sources.
* `--help` (`-h` for short) lists all the parameters that can be called in `indexer`.
* `-v` shows `indexer` version.

### Indexer configuration settings
You can also configure indexer behavior in the Manticore configuration file in the `indexer` section:

```ini
indexer {
...
}
```

#### lemmatizer_cache

```ini
lemmatizer_cache = 256M
```
Lemmatizer cache size. Optional, default is 256K.

Our [lemmatizer](../../Server_settings/Common.md#lemmatizer_base) implementation uses a compressed dictionary format that enables a space/speed tradeoff. It can either perform lemmatization off the compressed data, using more CPU but less RAM, or it can decompress and precache the dictionary either partially or fully, thus using less CPU but more RAM. The lemmatizer_cache directive lets you control how much RAM exactly can be spent for that uncompressed dictionary cache.

Currently, the only available dictionaries are [ru.pak, en.pak, and de.pak](https://manticoresearch.com/install/). These are the Russian, English, and German dictionaries. The compressed dictionary is approximately 2 to 10 MB in size. Note that the dictionary stays in memory at all times too. The default cache size is 256 KB. The accepted cache sizes are 0 to 2047 MB. It's safe to raise the cache size too high; the lemmatizer will only use the needed memory. For example, the entire Russian dictionary decompresses to approximately 110 MB; thus setting`lemmatizer_cache` higher than that will not affect the memory use. Even when 1024 MB is allowed for the cache, if only 110 MB is needed, it will only use those 110 MB.

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

Maximum file field adaptive buffer size in bytes. Optional, default is 8MB, minimum is 1MB.

The file field buffer is used to load files referred to from [sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field) columns. This buffer is adaptive, starting at 1 MB at first allocation, and growing in 2x steps until either the file contents can be loaded or the maximum buffer size, specified by the `max_file_field_buffer` directive, is reached.

Thus, if no file fields are specified, no buffer is allocated at all. If all files loaded during indexing are under (for example) 2 MB in size, but the `max_file_field_buffer` value is 128 MB, the peak buffer usage would still be only 2 MB. However, files over 128 MB would be entirely skipped.

#### max_iops

```ini
max_iops = 40
```

Maximum I/O operations per second, for I/O throttling. Optional, default is 0 (unlimited).

I/O throttling related option. It limits the maximum count of I/O operations (reads or writes) per any given second. A value of 0 means that no limit is imposed.

`indexer` can cause bursts of intensive disk I/O during building a table, and it might be desirable to limit its disk activity (and reserve something for other programs running on the same machine, such as `searchd`). I/O throttling helps to do that. It works by enforcing a minimum guaranteed delay between subsequent disk I/O operations performed by `indexer`. Throttling I/O can help reduce search performance degradation caused by building. This setting is not effective for other kinds of data ingestion, e.g. inserting data into a real-time table.

#### max_iosize

```ini
max_iosize = 1048576
```

Maximum allowed I/O operation size, in bytes, for I/O throttling. Optional, default is 0 (unlimited).

I/O throttling related option. It limits the maximum file I/O operation (read or write) size for all operations performed by `indexer`. A value of 0 means that no limit is imposed. Reads or writes that are bigger than the limit will be split into several smaller operations, and counted as several operations by the [max_iops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#max_iops) setting. At the time of this writing, all I/O calls should be under 256 KB (default internal buffer size) anyway, so max_iosize values higher than 256 KB should not have any effect.

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

Maximum allowed field size for XMLpipe2 source type, in bytes. Optional, default is 2 MB.

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # same, but in KB
# mem_limit = 268435456 # same, but in bytes
```

Plain table building RAM usage limit. Optional, default is 128 MB. Enforced memory usage limit that the `indexer` will not go above. Can be specified in bytes, or kilobytes (using K postfix), or megabytes (using M postfix); see the example. This limit will be automatically raised if set to an extremely low value causing I/O buffers to be less than 8 KB; the exact lower bound for that depends on the built data size. If the buffers are less than 256 KB, a warning will be produced.

The maximum possible limit is 2047M. Too low values can hurt plain table building speed, but 256M to 1024M should be enough for most, if not all datasets. Setting this value too high can cause SQL server timeouts. During the document collection phase, there will be periods when the memory buffer is partially sorted and no communication with the database is performed; and the database server can timeout. You can resolve that either by raising timeouts on the SQL server side or by lowering `mem_limit`.

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

How to handle IO errors in file fields. Optional, default is `ignore_field`.
When there is a problem indexing a file referenced by a file field ([sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)), `indexer` can either process the document, assuming empty content in this particular field, or skip the document, or fail indexing entirely. `on_file_field_error` directive controls that behavior. The values it takes are:
* `ignore_field`, process the current document without field;
* `skip_document`, skip the current document but continue indexing;
* `fail_index`, fail indexing with an error message.

The problems that can arise are: open error, size error (file too big), and data read error. Warning messages on any problem will be given at all times, regardless of the phase and the `on_file_field_error` setting.

Note that with `on_file_field_error = skip_document` documents will only be ignored if problems are detected during an early check phase, and **not** during the actual file parsing phase. `indexer` will open every referenced file and check its size before doing any work, and then open it again when doing actual parsing work. So in case a file goes away between these two open attempts, the document will still be indexed.

#### write_buffer

```ini
write_buffer = 4M
```

Write buffer size, bytes. Optional, default is 1MB. Write buffers are used to write both temporary and final table files when indexing. Larger buffers reduce the number of required disk writes. Memory for the buffers is allocated in addition to [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit). Note that several (currently up to 4) buffers for different files will be allocated, proportionally increasing the RAM usage.

#### ignore_non_plain

```ini
ignore_non_plain = 1
```

`ignore_non_plain` allows you to completely ignore warnings about skipping non-plain tables. The default is 0 (not ignoring).
<!-- proofread -->


### Schedule indexer via systemd

There are two approaches to scheduling indexer runs. The first way is the classical method of using crontab. The second way is using a systemd timer with a user-defined schedule. To create the timer unit files, you should place them in the appropriate directory where systemd looks for such unit files. On most Linux distributions, this directory is typically `/etc/systemd/system`. Here's how to do it:

1. Create a timer unit file for your custom schedule:
   ```shell
   cat << EOF > /etc/systemd/system/manticore-indexer@.timer
   [Unit]
   Description=Run Manticore Search's indexer on schedule
   [Timer]
   OnCalendar=minutely
   RandomizedDelaySec=5m
   Unit=manticore-indexer@%i.service
   [Install]
   WantedBy=timers.target
   EOF
   ```
   More on the `OnCalendar` syntax and examples can be found [here](https://www.freedesktop.org/software/systemd/man/latest/systemd.time.html#Calendar%20Events).

2. Edit the timer unit for your specific needs.
3. Enable the timer:
   ```shell
   systemctl enable manticore-indexer@idx1.timer
   ```
4. Start the timer:
   ```shell
   systemctl start manticore-indexer@idx1.timer
   ```
5. Repeat steps 2-4 for any additional timers.
