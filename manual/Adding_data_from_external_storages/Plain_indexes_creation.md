# Plain indexes creation

Plain indexes are indexes that are created one-time by fetching data at creation from one or several sources. A plain index is immutable as documents cannot be added or deleted during it's lifespan. It is only possible to update values of numeric attributes (including MVA). Refreshing the data is only possible by recreating the whole index.

Plain indexes are available only in [Plain mode](Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain mode%29) and their definition is made of an index declaration and one or several source declarations. The data gathering and index creation is not made by the `searchd` server, but by the auxiliary tool `indexer`.

**Indexer** is a command line tool that can be called directly from the command line or from shell scripts.

It can accept a number of arguments when called, but there are also several settings of it's own in the Manticore configuration file.

In the typical scenario, indexer does the following:
* fetches the data from the source
* builds the plain index
* writes the index files
* (optional) informs search server about the new index which triggers index rotation

## Indexer tool  
General syntax for `indexer` is as follows:

```shell
indexer [OPTIONS] [indexname1 [indexname2 [...]]]
```

An important thing to keep in mind when creating indexes with `indexer` is that the generated index files must be made with permissions that allow `searchd` to read, write and even delete them. In case of Linux official packages `searchd` runs under `manticore` user. In this case `indexer` must also run under `manticore` user:

```shell
sudo -u manticore indexer ...
```

If you are running `searchd` (Manticore Search server) differently you might need to omit `sudo -u manticore`, just make sure that the user under which your `searchd` instance is running has read/write permissions to your indexes that you generate using `indexer`.

Essentially you would list the different possible indexes (that you would later make available to search) in `manticore.conf`, so when calling `indexer`, as a minimum you need to be telling it what index (or indexes) you want to index. If `manticore.conf` contained details on 2 indexes, `mybigindex` and `mysmallindex`, you could do the following:

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

Wildcarding on index names is also supported. The following wildcard tokens can be used:
* `?` matches any single character
* `*` matches any count of any characters
* `%` matches none or any single character

```shell
sudo -u manticore indexer indexpart*main --rotate
```

The exit codes are as follows:
* 0 - everything went ok
* 1 - there was a problem while indexing (and if –-rotate was specified, it was skipped)
* 2 - indexing went ok, but –-rotate attempt failed

### Indexer command line arguments
* `--config <file>` (`-c <file>` for short) tells `indexer` to use the given file as its configuration. Normally, it will look for `manticore.conf` in the installation directory (e.g. `/etc/manticoresearch/manticore.conf`), followed by the current directory you are in when calling `indexer` from the shell. This is most of use in shared environments where the binary files are installed in a global folder, e.g. `/usr/bin/`, but you want to provide users with the ability to make their own custom Manticore set-ups, or if you want to run multiple instances on a single server. In cases like those you could allow them to create their own `manticore.conf` files and pass them to `indexer` with this option. For example:

```shell
sudo -u manticore indexer --config /home/myuser/manticore.conf myindex
```

* `--all` tells `indexer` to update every index listed in `manticore.conf` instead of listing individual indexes. This would be useful in small configurations or cron-kind or maintenance jobs where the entire index set will get rebuilt each day or week or whatever period is best. Example usage:

```shell
sudo -u manticore indexer --config /home/myuser/manticore.conf --all
```

* `--rotate` is used for rotating indexes. Unless you have the situation where you can take the search function offline without troubling users you will almost certainly need to keep search running whilst indexing new documents. `--rotate` creates a second index, parallel to the first (in the same place, simply including `.new` in the filenames). Once complete, `indexer` notifies `searchd` via sending the `SIGHUP` signal, and the `searchd` will attempt to rename the indexes (renaming the existing ones to include `.old` and renaming the `.new` to replace them), and then will start serving from the newer files. Depending on the setting of [seamless_rotate](Server_settings/Searchd.md#seamless_rotate) there may be a slight delay in being able to search the newer indexes. In case multiple indexes are rotated at once which are chained by [killlist_target](Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#killlist_target) relations rotation will start with the indexes that are not targets and finish with the ones at the end of target chain. Example usage:

```shell
sudo -u manticore indexer --rotate --all
```

* `--quiet` tells `indexer` not to output anything, unless there is an error. This is mostly used for cron-type or other scripted jobs where the output is irrelevant or unnecessary, except in the event of some kind of error. Example usage:

```shell
sudo -u manticore indexer --rotate --all --quiet
```

* `--noprogress` does not display progress details as they occur. Instead, the final status details (such as documents indexed, speed of indexing and so on are only reported at completion of indexing. In instances where the script is not being run on a console (or 'tty'), this will be on by default. Example usage:

```shell
sudo -u manticore indexer --rotate --all --noprogress
```

* `--buildstops <outputfile.text> <N>` reviews the index source, as if it were indexing the data, and produces a list of the terms that are being indexed. In other words, it produces a list of all the searchable terms that are becoming part of the index. Note, it does not update the index in question, it simply processes the data **as if** it were indexing, including running queries defined with [sql_query_pre](Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_pre) or [sql_query_post](Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_post). `outputfile.txt` will contain the list of words, one per line, sorted by frequency with most frequent first, and `N` specifies the maximum number of words that will be listed. If it's sufficiently large to encompass every word in the index, only that many words will be returned. Such a dictionary list could be used for client application features around "Did you mean…" functionality, usually in conjunction with `--buildfreqs`, below. Example:

```shell
sudo -u manticore indexer myindex --buildstops word_freq.txt 1000
```

This would produce a document in the current directory, `word_freq.txt` with the 1,000 most common words in 'myindex', ordered by most common first. Note that the file will pertain to the last index indexed when specified with multiple indexes or `--all` (i.e. the last one listed in the configuration file)
* `--buildfreqs` works with `--buildstops` (and is ignored if `--buildstops` is not specified). As `--buildstops` provides the list of words used within the index, `--buildfreqs` adds the quantity present in the index, which would be useful in establishing whether certain words should be considered stopwords if they are too prevalent. It will also help with developing "Did you mean…" features where you need to know how much more common a given word compared to another, similar one. Example:

```shell
sudo -u manticore indexer myindex --buildstops word_freq.txt 1000 --buildfreqs
```

This would produce the `word_freq.txt` as above, however after each word would be the number of times it occurred in the index in question.
* `--merge <dst-index> <src-index>` is used for physically merging indexes together, for example if you have a [main+delta scheme](Creating_an_index/Local_indexes/Plain_index.md#Main+delta), where the main index rarely changes, but the delta index is rebuilt frequently, and `--merge` would be used to combine the two. The operation moves from right to left - the contents of `src-index` get examined and physically combined with the contents of `dst-index` and the result is left in `dst-index`. In pseudo-code, it might be expressed as: `dst-index += src-index` An example:

```shell
sudo -u manticore indexer --merge main delta --rotate
```

In the above example, where the main is the master, rarely modified index, and delta is the less frequently modified one, you might use the above to call `indexer` to combine the contents of the delta into the main index and rotate the indexes.
* `--merge-dst-range <attr> <min> <max>` runs the filter range given upon merging. Specifically, as the merge is applied to the destination index (as part of `--merge`, and is ignored if `--merge` is not specified), `indexer` will also filter the documents ending up in the destination index, and only documents will pass through the filter given will end up in the final index. This could be used for example, in an index where there is a 'deleted' attribute, where 0 means 'not deleted'. Such an index could be merged with:

```shell
sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
```

Any documents marked as deleted (value 1) would be removed from the newly-merged destination index. It can be added several times to the command line, to add successive filters to the merge, all of which must be met in order for a document to become part of the final index.
* `--merge-killlists` (and its shorter alias `--merge-klists`) changes the way kill lists are processed when merging indexes. By default, both kill lists get discarded after a merge. That supports the most typical main+delta merge scenario. With this option enabled, however, kill lists from both indexes get concatenated and stored into the destination index. Note that a source (delta) index kill list will be used to suppress rows from a destination (main) index at all times.
* `--keep-attrs` allows to reuse existing attributes on reindexing. Whenever the index is rebuilt, each new document id is checked for presence in the "old" index, and if it already exists, its attributes are transferred to the "new" index; if not found, attributes from the new index are used. If the user has updated attributes in the index, but not in the actual source used for the index, all updates will be lost when reindexing; using –keep-attrs enables saving the updated attribute values from the previous index. It is possible to specify a path for index files to used instead of reference path from config:

```shell
sudo -u manticore indexer myindex --keep-attrs=/path/to/index/files
```

* `--keep-attrs-names=<attributes list>` allows to specify attributes to reuse from existing index on reindexing. By default all attributes from existed index reused at new "index":

```shell
sudo -u manticore indexer myindex --keep-attrs=/path/to/index/files --keep-attrs-names=update,state
```

* `--dump-rows <FILE>` dumps rows fetched by SQL source(s) into the specified file, in a MySQL compatible syntax. Resulting dumps are the exact representation of data as received by `indexer` and help to repeat indexing-time issues. The command performs fetching from the source and creates both index files and the dump file.
* `--print-rt <rt_index> <index>` outputs fetched data from source as INSERTs for a real-time index. The first lines of the dump will contain the real-time fields and attributes (as a reflection of the plain index fields and attributes). The command performs fetching from the source and creates both index files and the dump output. The command can be used as `sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`.
* `--verbose [debug|debugv|debugvv]` guarantees that every row that caused problems indexing (duplicate, zero, or missing document ID; or file field IO issues; etc) will be reported. By default, this option is off, and problem summaries may be reported instead. Also you can use one of the optional parameters (debug, debugv, or debugvv) and it will switch on debug output from different parts of indexing process. They are similar to `searchd`'s parameters --logdebug, --logdebugv, --logdebugvv, but cause output to stdout instead of logging.
* `--sighup-each` is useful when you are rebuilding many big indexes, and want each one rotated into `searchd` as soon as possible. With `--sighup-each`, `indexer` will send the SIGHUP signal to searchd after successfully completing work on each index. (The default behavior is to send a single SIGHUP after all the indexes are built).
* `--nohup` is useful when you want to check your index with indextool before actually rotating it. indexer won't send the SIGHUP if this option is on.
* `--print-queries` prints out SQL queries that `indexer` sends to the database, along with SQL connection and disconnection events. That is useful to diagnose and fix problems with SQL sources.
* `--help` (`-h` for short) lists all of the parameters that can be called in `indexer`.
* `-v` shows `indexer` version.

### Indexer configuration settings
You can also configure indexer behaviour in Manticore configuration file in section `indexer`:

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

Our [lemmatizer](Server_settings/Common.md#lemmatizer_base) implementation uses a compressed dictionary format that enables a space/speed tradeoff. It can either perform lemmatization off the compressed data, using more CPU but less RAM, or it can decompress and precache the dictionary either partially or fully, thus using less CPU but more RAM. And the lemmatizer_cache directive lets you control how much RAM exactly can be spent for that uncompressed dictionary cache.

Currently, the only available dictionaries are [ru.pak, en.pak, and de.pak](https://manticoresearch.com/downloads/). These are the Russian, English and German dictionaries. The compressed dictionary is approximately 2 to 10 MB in size. Note that the dictionary stays in memory at all times, too. The default cache size is 256 KB. The accepted cache sizes are 0 to 2047 MB. It's safe to raise the cache size too high; the lemmatizer will only use the needed memory. For instance, the entire Russian dictionary decompresses to approximately 110 MB; and thus setting lemmatizer_cache anywhere higher than that will not affect the memory use: even when 1024 MB is allowed for the cache, if only 110 MB is needed, it will only use those 110 MB.

On our benchmarks, the total indexing time with different cache sizes was as follows:
* 9.07 sec, morphology = lemmatize_ru, lemmatizer_cache = 0
* 8.60 sec, morphology = lemmatize_ru, lemmatizer_cache = 256K
* 8.33 sec, morphology = lemmatize_ru, lemmatizer_cache = 8M
* 7.95 sec, morphology = lemmatize_ru, lemmatizer_cache = 128M
* 6.85 sec, morphology = stem_ru (baseline)

Your mileage may vary, but a simple rule of thumb would be to either go with the small default 256 KB cache when pressed for memory, or spend 128 MB extra RAM and cache the entire dictionary for maximum indexing performance.

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

Maximum file field adaptive buffer size, bytes. Optional, default is 8MB, minimum is 1MB.

File field buffer is used to load files referred to from [sql_file_field](Adding_data_from_external_storages/Fetching_from_databases/Indexing_fetched_data.md#sql_file_field) columns. This buffer is adaptive, starting at 1 MB at first allocation, and growing in 2x steps until either file contents can be loaded, or maximum buffer size, specified by `max_file_field_buffer` directive, is reached.

Thus, if there are no file fields are specified, no buffer is allocated at all. If all files loaded during indexing are under (for example) 2 MB in size, but `max_file_field_buffer` value is 128 MB, peak buffer usage would still be only 2 MB. However, files over 128 MB would be entirely skipped.

#### max_iops

```ini
max_iops = 40
```

Maximum I/O operations per second, for I/O throttling. Optional, default is 0 (unlimited).

I/O throttling related option. It limits maximum count of I/O operations (reads or writes) per any given second. A value of 0 means that no limit is imposed.

`indexer` can cause bursts of intensive disk I/O during indexing, and it might be desired to limit its disk activity (and keep something for other programs running on the same machine, such as `searchd`). I/O throttling helps to do that. It works by enforcing a minimum guaranteed delay between subsequent disk I/O operations performed by `indexer`. Limiting indexing I/O can help reduce search performance degradation caused by indexing.

#### max_iosize

```ini
max_iosize = 1048576
```

Maximum allowed I/O operation size, in bytes, for I/O throttling. Optional, default is 0 (unlimited).

I/O throttling related option. It limits maximum file I/O operation (read or write) size for all operations performed by `indexer`. A value of 0 means that no limit is imposed. Reads or writes that are bigger than the limit will be split in several smaller operations, and counted as several operation by [max_iops](Adding_data_from_external_storages/Plain_indexes_creation.md#max_iops) setting. At the time of this writing, all I/O calls should be under 256 KB (default internal buffer size) anyway, so `max_iosize` values higher than 256 KB must not affect anything.

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

Maximum allowed field size for XMLpipe2 source type, bytes. Optional, default is 2 MB.

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # same, but in KB
# mem_limit = 268435456 # same, but in bytes
```

Indexing RAM usage limit. Optional, default is 128M. Enforced memory usage limit that the `indexer` will not go above. Can be specified in bytes, or kilobytes (using K postfix), or megabytes (using M postfix); see the example. This limit will be automatically raised if set to extremely low value causing I/O buffers to be less than 8 KB; the exact lower bound for that depends on the indexed data size. If the buffers are less than 256 KB, a warning will be produced.

Maximum possible limit is 2047M. Too low values can hurt indexing speed, but 256M to 1024M should be enough for most if not all datasets. Setting this value too high can cause SQL server timeouts. During the document collection phase, there will be periods when the memory buffer is partially sorted and no communication with the database is performed; and the database server can timeout. You can resolve that either by raising timeouts on SQL server side or by lowering `mem_limit`.  

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

How to handle IO errors in file fields. Optional, default is `ignore_field`.
When there is a problem indexing a file referenced by a file field ([sql_file_field](Adding_data_from_external_storages/Fetching_from_databases/Indexing_fetched_data.md#sql_file_field)), `indexer` can either index the document, assuming empty content in this particular field, or skip the document, or fail indexing entirely. `on_file_field_error` directive controls that behavior. The values it takes are:
* `ignore_field`, index the current document without field;
* `skip_document`, skip the current document but continue indexing;
* `fail_index`, fail indexing with an error message.

The problems that can arise are: open error, size error (file too big), and data read error. Warning messages on any problem will be given at all times, irregardless of the phase and the `on_file_field_error` setting.

Note that with `on_file_field_error = skip_document` documents will only be ignored if problems are detected during an early check phase, and **not** during the actual file parsing phase. `indexer` will open every referenced file and check its size before doing any work, and then open it again when doing actual parsing work. So in case a file goes away between these two open attempts, the document will still be indexed.

#### write_buffer

```ini
write_buffer = 4M
```    

Write buffer size, bytes. Optional, default is 1MB. Write buffers are used to write both temporary and final index files when indexing. Larger buffers reduce the number of required disk writes. Memory for the buffers is allocated in addition to [mem_limit](Adding_data_from_external_storages/Plain_indexes_creation.md#mem_limit). Note that several (currently up to 4) buffers for different files will be allocated, proportionally increasing the RAM usage.
