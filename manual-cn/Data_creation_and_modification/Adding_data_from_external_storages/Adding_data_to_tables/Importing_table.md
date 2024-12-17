# 导入表

如果你决定从[普通模式](../../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)迁移到[实时模式](../../../Creating_a_table/Local_tables.md#Online-schema-management-(RT-mode))或在其他情况下，可以使用`IMPORT TABLE`语句将以普通模式构建的实时表和渗透表导入到运行实时模式的Manticore中。其一般语法如下：

<!-- example import -->

```sql
IMPORT TABLE table_name FROM 'path'
```

其中'path'参数必须设置为：`/your_backup_folder/your_backup_name/data/your_table_name/your_table_name`

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

执行此命令会将指定表的所有表文件复制到[data_dir](../../../Server_settings/Searchd.md#data_dir)中。所有外部表文件（如词形、例外和停用词文件）也将复制到相同的`data_dir`。

`IMPORT TABLE`具有以下限制：

- 原始配置文件中指定的外部文件路径必须是绝对路径。
- 仅支持实时表和渗透表。
- 普通表需要预先（在普通模式下）通过[ATTACH TABLE](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)转换为实时表。

请注意，`IMPORT TABLE`命令不支持5.0.0版本之前创建的表。

## indexer --print-rt

<!-- example print_rt -->如果无法使用上述方法将普通表迁移到实时表，则可以使用`indexer --print-rt`直接从普通表中转储数据，而无需将其转换为实时表类型，然后直接从命令行将转储的数据导入到实时表。

此方法有一些限制：

- 仅支持基于SQL的数据源。
- 不支持MVA（多值属性）。

<!-- request -->
```bash
/usr/bin/indexer --rotate --config /etc/manticoresearch/manticore.conf --print-rt my_rt_index my_plain_index > /tmp/dump_regular.sql

mysql -P $9306 -h0 -e "truncate table my_rt_index"

mysql -P 9306 -h0 < /tmp/dump_regular.sql

rm /tmp/dump_regular.sql
```
<!-- end -->
<!-- proofread -->
