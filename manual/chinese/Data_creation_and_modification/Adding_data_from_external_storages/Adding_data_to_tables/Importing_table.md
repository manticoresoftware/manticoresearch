# 导入表

如果您决定从[Plain 模式](../../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)迁移到[RT 模式](../../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)或在其他某些情况下，可以使用 `IMPORT TABLE` 语句将 Plain 模式下构建的实时和 percolate 表导入到运行 RT 模式的 Manticore。其通用语法如下：

<!-- example import -->

```sql
IMPORT TABLE table_name FROM 'path'
```

其中 'path' 参数必须设置为：`/your_backup_folder/your_backup_name/data/your_table_name/your_table_name`

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

执行此命令会将指定表的所有表文件复制到 [data_dir](../../../Server_settings/Searchd.md#data_dir)。所有外部表文件，如 wordforms、exceptions 和 stopwords，也会被复制到相同的 `data_dir`。
`IMPORT TABLE` 有以下限制：
* 配置文件中最初指定的外部文件路径必须是绝对路径
* 仅支持实时和 percolate 表
* plain 表需要预先（在 plain 模式下）通过 [ATTACH TABLE](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) 转换为实时表

注意，`IMPORT TABLE` 命令不支持版本低于 5.0.0 创建的表。

## indexer --print-rt

<!-- example print_rt -->
如果上述将 plain 表迁移为 RT 表的方法不可行，您可以使用 `indexer --print-rt` 直接从 plain 表导出数据，无需将其转换为 RT 类型表，然后直接从命令行将导出数据导入 RT 表。

不过此方法有一些限制：
* 仅支持基于 SQL 的数据源
* 不支持 MVA

<!-- request -->
```bash
/usr/bin/indexer --rotate --config /etc/manticoresearch/manticore.conf --print-rt my_rt_index my_plain_index > /tmp/dump_regular.sql

mysql -P $9306 -h0 -e "truncate table my_rt_index"

mysql -P 9306 -h0 < /tmp/dump_regular.sql

rm /tmp/dump_regular.sql
```
<!-- end -->
<!-- proofread -->

