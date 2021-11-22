# Importing index

If you decide to migrate from [Plain mode](../../Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain mode%29) to [RT mode](../../Creating_an_index/Local_indexes.md#Online-schema-management-%28RT-mode%29) and in some other cases, real-time and percolate indexes built in Plain mode can be imported to Manticore running in RT mode using the `IMPORT TABLE` statement. The general syntax is as follows:

```sql
IMPORT TABLE table_name FROM 'path'
```

Executing this command makes all the index files of the specified index copied to [data_dir](../../Server_settings/Searchd.md#data_dir). All the external index files such as wordforms, exceptions and stopwords are also copied to the same `data_dir`.
`IMPORT TABLE` has the following limitations:
* paths to the external files that were originally specified in the config file must be absolute
* only real-time and percolate indexes are supported
* plain indexes need to be preliminarily (in the plain mode) converted to real-time indexes via [ATTACH INDEX](../../Adding_data_from_external_storages/Adding_data_from_indexes/Attaching_a_plain_index_to_RT_index.md)

## indexer --print-rt

<!-- example print_rt -->
If the above method for migrating plain index to RT index is not possible you may use `indexer --print-rt` to dump data from plain index directly without the need to convert it to RT type index and then import dump into RT index right from command line.

This method has few limitations though:
* Only sql-based sources are supported
* MVAs are not supported

<!-- request -->
```bash
/usr/bin/indexer --rotate --config /etc/manticoresearch/manticore.conf --print-rt my_rt_index my_plain_index > /tmp/dump_regular.sql

mysql -P $9306 -h0 -e "truncate table my_rt_index"

mysql -P 9306 -h0 < /tmp/dump_regular.sql

rm /tmp/dump_regular.sql
```
<!-- end -->

