# 导入表


如果你决定从 [Plain mode](../../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) 迁移到 [RT mode](../../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) 或在其他一些情况下，使用 `IMPORT TABLE` 语句可以将以 Plain mode 构建的实时和 percolate 表导入运行在 RT mode 的 Manticore。一般语法如下：


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


执行此命令会将指定表的所有表文件复制到 [data_dir](../../../Server_settings/Searchd.md#data_dir)。所有外部表文件，例如 wordforms、exceptions 和 stopwords 也会被复制到同一个 `data_dir`。

`IMPORT TABLE` 有以下限制：

* 在配置文件中最初指定的外部文件路径必须是绝对路径

* 只支持实时和 percolate 表

* 普通表需要在 [ATTACH TABLE](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) 中预先（在普通模式下）转换为实时表


请注意，`IMPORT TABLE` 命令不支持在 5.0.0 之前的版本中创建的表。


## indexer --print-rt


<!-- example print_rt -->
如果上述将普通表迁移到 RT 表的方法不可行，你可以使用 `indexer --print-rt` 直接从普通表转储数据，而无需将其转换为 RT 类型表，然后将转储的内容从命令行导入到 RT 表。


该方法有一些限制：

