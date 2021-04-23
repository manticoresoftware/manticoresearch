# Migration from Sphinx Search

## Sphinx 2.x -> Manticore 2.x
Manticore Search 2.x maintains compatibility with Sphinxsearch 2.x and can load existing indexes created by Sphinxsearch. In most cases, upgrading is just a matter of replacing the binaries.

Instead of sphinx.conf (in Linux normally located at `/etc/sphinxsearch/sphinx.conf`) Manticore by default uses `/etc/manticoresearch/manticore.conf`. It also runs under a different user and use different folders.

Systemd service name has changed from `sphinx/sphinxsearch` to `manticore` and the service runs under user `manticore` (Sphinx was using `sphinx` or `sphinxsearch`). It also uses a different folder for the PID file.

The folders used by default are `/var/lib/manticore`, `/var/log/manticore`, `/var/run/manticore`. You can still use the existing Sphinx config, but you need to manually change permissions for `/var/lib/sphinxsearch` and `/var/log/sphinxsearch` folders. Or, just rename globally 'sphinx' to 'manticore' in system files. If you use other folders (for data, wordforms files etc.) the ownership must be also switched to user `manticore`. The `pid_file` location should be changed to match the manticore.service to `/var/run/manticore/searchd.pid`.

If you want to use the Manticore folder instead, the index files need to be moved to the new data folder (`/var/lib/manticore`) and the permissions to be changed to user `manticore`.

## Sphinx 2.x / Manticore 2.x -> Manticore 3.x
Upgrading from Sphinx / Manticore 2.x to 3.x is not straightforward, because the index storage engine received a massive upgrade and the new searchd can't load older indexes and upgrade them to new format on-the-fly.

Manticore Search 3 got a redesigned index storage. Indexes created with Manticore/Sphinx 2.x cannot be loaded by Manticore Search 3 without a [conversion](../Installation/Migration_from_Sphinx.md#index_converter). Because of the 4GB limitation, a real-time index in 2.x could still have several disk chunks after an optimize operation. After upgrading to 3.x, these indexes can now be optimized to 1-disk chunk with the usual [OPTIMIZE](../Securing_and_compacting_an_index/Compacting_an_index.md#OPTIMIZE-INDEX) command. Index files also changed. The only component that didn't get any structural changes is the `.spp` file (hitlists). `.sps` (strings/json) and `.spm` (MVA) are now held by `.spb` (var-length attributes). The new format has an `.spm` file present, but it's used for row map (previously it was dedicated for MVA attributes). The new extensions added are `.spt` (docid lookup), `.sphi` ( secondary index histograms), `.spds` (document storage). In case you are using scripts that manipulate index files, they should be adapted for the new file extensions.

The upgrade procedure may differ depending on your setup (number of servers in the cluster, whether you have HA or not etc.), but in general it's about creating new 3.x index versions and replacing your existing ones with them along with replacing older 2.x binaries with the new ones.

There are two special requirements to take care:

* Real-time indexes need to be flushed using [FLUSH RAMCHUNK](../Securing_and_compacting_an_index/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK)
* Plain indexes with kill-lists require adding a new directive in index configuration (see [killlist_target](../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#killlist_target))

Manticore Search 3 includes a new tool - [index_converter](../Installation/Migration_from_Sphinx.md#index_converter) - that can convert Sphinx 2.x / Manticore 2.x indexes to 3.x format. `index_converter` comes in a separate package which should be installed first. Using the convert tool create 3.x versions of your indexes. `index_converter` can write the new files in the existing data folder and backup the old files or it can write the new files to a chosen folder.

## Basic upgrade instruction

If you have a single server:

* install manticore-converter package
* use index_converter to create new versions of the indexes in a different folder than the existing data folder ( using –output-dir option)
* stop existing Manticore/Sphinx, upgrade to 3.0, move the new indexes to data folder, start Manticore

To get a minimal downtime, you can copy 2.x indexes, config (you'll need to edit paths here for indexes, logs and different ports) and binaries to a separate location and start this on a separate port and point your application to it. After upgrade is made to  3.0 and the new server is started, you can point back the application to the normal ports. If all is good, stop the 2.x copy and delete the files to free the space.

If you have a spare box (like a testing or staging server), you can do there first the index upgrade and even install Manticore 3 to perform several tests and if everything is ok copy the new index files to the production server. If you have multiple servers which can be pulled out from production, do it one by one and perform the upgrade on each. For distributed setups, 2.x searchd can work as a master with 3.x nodes, so you can do upgrading on the data nodes first and at the end the master node.

There have been no changes made on how clients should connect to the engine or any change in querying mode or queries behavior.

## kill-lists in Sphinx / Manticore 2.x vs Manticore 3.x
[Kill-lists](../Adding_data_from_external_storages/Adding_data_from_indexes/Killlist_in_plain_indexes.md) have been redesigned in Manticore Search 3. In previous versions kill-lists were applied on the result set provided by each previous searched index on query time.

Thus In 2.x the index order at query time mattered. For example if a delta index had a kill-list in order to apply it against the main index the order had to be main, delta (either in a distributed index or in the FROM clause).

In Manticore 3 kill-lists are applied to an index when it's loaded during searchd startup or gets rotated. New directive [killlist_target](../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#killlist_target) in index configuration specifies target indexes and defines which doc ids from the source index should be used for suppression. These can be ids from the defined kill-list, actual doc ids of the index or the both.

Documents from the kill-lists are deleted from the target indexes, they are not returned in results even if the search doesn't include the index that provided the kill-lists. Because of that the order of indexes for searching does not matter any more. Now delta,main and main,delta will provide the same results.

In previous versions indexes were rotated following the order from the configuration file. In Manticore 3 index rotation order is much smarter and works in accordance with killlist targets. Before starting to rotate indexes the server looks for chains of indexes by killlist_target definitions. It will then first rotate indexes not referenced anywhere as kill-lists targets. Next it will rotate indexes targeted by already rotated indexes and so on. For example if we do `indexer --all` and we have 3 indexes : main, delta_big (which targets at the main) and delta_small (with target at delta_big), first the delta_small is rotated, then delta_big and finally the main. This is to ensure that when a dependent index is rotated it gets the most actual kill-list from other indexes.

## Configuration keys removed in Manticore 3.x
* `docinfo` - everything is now extern
* `inplace_docinfo_gap` - not needed anymore
* `mva_updates_pool` - MVAs don’t have anymore a dedicated pool for updates, as now they can be updated directly in the blob (see below).

## Updating var-length attributes in Manticore 3.x
String, JSON and MVA attributes can be updated in Manticore 3.x using `UPDATE` statement.

In 2.x string attributes required `REPLACE`, for JSON it was only possible to update scalar properties (as they were fixed-width) and MVAs could be updated using the MVA pool. Now updates are performed directly on the blob component. One setting that may require tuning is [attr_update_reserve](../Updating_documents/UPDATE.md#attr_update_reserve) which allows changing the allocated extra space at the end of the blob used to avoid frequent resizes in case the new values are bigger than the existing values in the blob.

## Document IDs in Manticore 3.x
Doc ids used to be UNSIGNED 64-bit integers. Now they are POSITIVE SIGNED 64-bit integers.

## RT mode in Manticore 3.x
Read here about the [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode)

## Special suffixes since Manticore 3.x
Manticore 3.x recognizes and parses special suffixes which makes easier to use numeric values with special meaning. Common form for them is integer number + literal, like 10k or 100d, but not 40.3s (since 40.3 is not integer), or not 2d 4h (since there are two, not one value). Literals are case-insensitive, so 10W is the same as 10w. There are 2 types of such suffixes currently supported:

* Size suffixes - can be used in parameters that define size of something (memory buffer, disk file, limit of RAM, etc. ) in bytes. "Naked" numbers in that places mean literally size in bytes (octets). Size values take suffix `k` for kilobytes (1k=1024), `m` for megabytes (1m=1024k), `g` for gigabytes (1g=1024m) and `t` for terabytes (1t=1024g).
* Time suffixes - can be used in parameters defining some time interval values like delays, timeouts, etc. "Naked" values for those parameters usually have documented scale, and you must know if their numbers, say, 100, means '100 seconds' or '100 milliseconds'. However instead of guessing you just can write suffixed value and it will be fully determined by it's suffix. Time values take suffix `us` for useconds (microseconds), `ms` for milliseconds, `s` for seconds, `m` for minutes, `h` for hours, `d` for days and `w` for weeks.

## index_converter

`index_converter` is a tool for converting indexes created with Sphinx/Manticore Search 2.x to Manticore Search 3.x index format. The tool can be used in several different ways:

#### Convert one index at a time

```ini
$ index_converter --config /home/myuser/manticore.conf --index indexname
```

#### Convert all indexes

```ini
$ index_converter --config /home/myuser/manticore.conf --all
```

#### Convert indexes found in a folder

```ini
$ index_converter  --path /var/lib/manticoresearch/data --all
```

New version of the index is written by default in the same folder. Previous version files are saved with .old extension in their name. An exception is .spp (hitlists) file which is the only index component that didn’t have any change in the new format.

You can save the new index version to a different folder using –output-dir option

```ini
$ index_converter --config /home/myuser/manticore.conf --all --output-dir /new/path
```

#### Convert kill lists

A special case is for indexes containing kill-lists. As the behaviour of how kill-lists works has changed (see [killlist_target](../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#killlist_target)), the delta index should know which are the target indexes for applying the kill-lists. There are 3 ways to have a converted index ready for setting targeted indexes for applying kill-lists:

* Use `-–killlist-target` when converting an index
  ```ini
  $ index_converter --config /home/myuser/manticore.conf --index deltaindex --killlist-target mainindex:kl
  ```
* Add killlist_target in the configuration before doing the conversion
* use [ALTER ... KILLIST_TARGET](../Adding_data_from_external_storages/Adding_data_from_indexes/Killlist_in_plain_indexes.md#killlist_target) command after conversion

#### Complete list of index_converter options

Here's the complete list of `index_converter` options:

* `--config <file>` (`-c <file>` for short) tells index_converter to use the given file as its configuration. Normally, it will look for manticore.conf in the installation directory (e.g. `/usr/local/sphinx/etc/manticore.conf` if installed into `/usr/local/sphinx`), followed by the current directory you are in when calling index_converter from the shell.
* `--index` specifies which index should be converted
* `--path` - instead of using a config file, a path containing index(es) can be used
* `--strip-path` - strips path from filenames referenced by index: stopwords, exceptions and wordforms
* `--large-docid` - allows to convert documents with ids larger than 2^63 and display a warning, otherwise it will just exit on the large id with an error. This option was added as in Manticore 3.x doc ids are signed bigint, while previously they were unsigned
* `--output-dir <dir>` - writes the new files in a chosen folder rather than the same location as with the existing index files. When this option set, existing index files will remain untouched at their location.
* `--all` - converts all indexes from the config
* `--killlist-target <targets>` - sets the target indexes for which kill-lists will be applied. This option should be used only in conjunction with `--index` option
