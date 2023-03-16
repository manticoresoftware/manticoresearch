# Importing table

If you decide to migrate from [Plain mode](../../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) to [RT mode](../../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) and in some other cases, real-time and percolate tables built in the Plain mode can be imported to Manticore running in the RT mode using the `IMPORT TABLE` statement. The general syntax is as follows:

<!-- example import -->

```sql
IMPORT TABLE table_name FROM 'path'
```

where the 'path' parameter must be set as: `/your_backup_folder/your_backup_name/data/your_table_name/your_table_name`

<!-- request -->
```bash
mysql -P9306 -h0 -e 'create table t(f text)'

mysql -P9306 -h0 -e "backup table t to /tmp/"

mysql -P9306 -h0 -e "drop table t"

BACKUP_NAME=$(ls /tmp | grep 'backup-' | tail -n 1)

mysql -P9306 -h0 -e "import table t from '/tmp/$BACKUP_NAME/data/t/t'

mysql -P9306 -h0 -e "show tables"
```
<!-- end -->

Executing this command makes all the table files of the specified table copied to [data_dir](../../../Server_settings/Searchd.md#data_dir). All the external table files such as wordforms, exceptions and stopwords are also copied to the same `data_dir`.
`IMPORT TABLE` has the following limitations:
* paths to the external files that were originally specified in the config file must be absolute
* only real-time and percolate tables are supported
* plain tables need to be preliminarily (in the plain mode) converted to real-time tables via [ATTACH TABLE](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_a_plain_table_to_RT_table.md)

## indexer --print-rt

<!-- example print_rt -->
If the above method for migrating plain table to RT table is not possible you may use `indexer --print-rt` to dump data from plain table directly without the need to convert it to RT type table and then import dump into RT table right from command line.

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
