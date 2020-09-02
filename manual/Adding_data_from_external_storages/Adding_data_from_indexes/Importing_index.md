# Importing index

When searchd is switched from [Plain mode](Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain mode%29) to [RT mode](Creating_an_index/Local_indexes.md#Online-schema-management-%28RT-mode%29), real-time and percolate indexes can be imported using the `IMPORT TABLE` statement. The general syntax is as follows:

```sql
IMPORT TABLE table_name FROM 'path'
```

All index files are copied to [data_dir](Server_settings/Searchd.md#data_dir). All external index files such as wordforms, exceptions and stopwords are also copied to data_dir. 
`IMPORT TABLE` has the following limitations:
* paths to external files that were originally specified in the config file must be absolute
* only real-time and percolate indexes are supported
* plain indexes need to be converted to real-time indexes via [ATTACH INDEX](Adding_data_from_external_storages/Adding_data_from_indexes/Attaching_a_plain_index_to_RT_index.md)
