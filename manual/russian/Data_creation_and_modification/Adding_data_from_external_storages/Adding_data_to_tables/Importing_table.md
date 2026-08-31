# Импорт таблицы

Если вы решите перейти с [Plain режима](../../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) на [RT режим](../../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) или в некоторых других случаях, таблицы реального времени и percolate, созданные в Plain режиме, могут быть импортированы в Manticore, работающий в RT режиме, с помощью оператора `IMPORT TABLE`. Общий синтаксис следующий:

<!-- example import -->

```sql
IMPORT TABLE table_name FROM 'path'
```

где параметр 'path' должен быть задан следующим образом: `/your_backup_folder/your_backup_name/data/your_table_name/your_table_name`

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

Выполнение этой команды копирует все файлы таблицы указанной таблицы в [data_dir](../../../Server_settings/Searchd.md#data_dir). Все внешние файлы таблицы, такие как wordforms, exceptions и stopwords, также копируются в тот же `data_dir`.
`IMPORT TABLE` имеет следующие ограничения:
* пути к внешним файлам, которые изначально были указаны в файле конфигурации, должны быть абсолютными
* поддерживаются только таблицы реального времени и percolate
* plain таблицы необходимо предварительно (в plain режиме) конвертировать в таблицы реального времени с помощью [ATTACH TABLE](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)

Обратите внимание, что команда `IMPORT TABLE` не поддерживает таблицы, созданные в версиях старше 5.0.0.

## indexer --print-rt

<!-- example print_rt -->
Если вышеописанный метод миграции plain таблицы в RT таблицу невозможен, вы можете использовать `indexer --print-rt` для выгрузки данных из plain таблицы напрямую без необходимости конвертировать её в таблицу типа RT, а затем импортировать дамп в RT таблицу прямо из командной строки.

Однако этот метод имеет несколько ограничений:
* Поддерживаются только источники на основе SQL
* MVAs не поддерживаются

<!-- request -->
```bash
/usr/bin/indexer --rotate --config /etc/manticoresearch/manticore.conf --print-rt my_rt_index my_plain_index > /tmp/dump_regular.sql

mysql -P $9306 -h0 -e "truncate table my_rt_index"

mysql -P 9306 -h0 < /tmp/dump_regular.sql

rm /tmp/dump_regular.sql
```
<!-- end -->
<!-- proofread -->

