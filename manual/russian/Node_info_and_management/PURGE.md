# PURGE

`PURGE` удаляет выбранные данные, связанные с таблицей. Цель очистки определяет, что именно будет удалено.

```sql
PURGE <target> FROM TABLE <table_name>;
```

Единственная поддерживаемая сейчас цель — `BULK_IMPORT`.

## PURGE BULK_IMPORT

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE BULK_IMPORT` удаляет весь корневой каталог staging, используемый [bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import), включая каталоги и файлы, оставшиеся после прерванной загрузки. Запускайте его только после того, как убедитесь, что для таблицы не выполняется загрузка напрямую на диск.

Этот оператор требует существующую локальную real-time таблицу, которая не входит в кластер репликации. Он удаляет только состояние staging для direct-to-disk и не изменяет схему таблицы или проиндексированные документы. Если корневой каталог staging отсутствует, оператор завершается успешно, не внося никаких изменений. В режиме configless `DROP TABLE` также удаляет корневой каталог staging для direct-to-disk у таблицы.

<!-- proofread -->
