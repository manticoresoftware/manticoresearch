# Компактизация таблицы

С течением времени RT таблицы могут стать фрагментированными на множество дисковых частей и/или загрязнены удаленными, но не очищенными данными, что влияет на скорость поиска. В таких случаях необходима оптимизация. В основном процесс оптимизации объединяет дисковые части (N-путевое слияние), удаляя документы, которые были ранее удалены с помощью операторов DELETE.

С версии Manticore 4 этот процесс происходит [автоматически по умолчанию](../Server_settings/Searchd.md#auto_optimize). Однако вы также можете использовать следующие команды для ручного запуска компактизации таблицы.

## OPTIMIZE TABLE

<!--
data for the following examples:

DROP TABLE IF EXISTS rt;
CREATE TABLE rt(title text);
INSERT INTO rt(title) VALUES
('doc one'),
('doc two'),
('doc three');
-->
<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

Оператор `OPTIMIZE` добавляет RT таблицу в очередь оптимизации, которая будет обработана в фоновом потоке.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt"
```

<!-- end -->

### Число оптимизированных дисковых частей

<!-- example optimize_cutoff -->

По умолчанию OPTIMIZE объединяет дисковые части RT таблицы до числа, не превышающего количество логических ядер CPU умноженное на 2.

Но если таблица имеет атрибуты с KNN индексами, этот порог другой. В этом случае он устанавливается как количество физических ядер CPU, делённое на 2, для улучшения скорости поиска по KNN.

Вы также можете управлять количеством оптимизированных дисковых частей вручную с помощью опции `cutoff`.

Дополнительные опции включают:
* Серверная настройка [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) для изменения стандартного порога
* Настройка для каждой таблицы [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION cutoff=4"
```

<!-- end -->

### Запуск в foreground режиме

<!-- example optimize_sync -->

При использовании `OPTION sync=1` (по умолчанию 0), команда будет ждать завершения процесса оптимизации перед возвратом результата. Если соединение прервано, оптимизация продолжит выполнение на сервере.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION sync=1"
```

<!-- end -->

### Снижение нагрузки на IO

Оптимизация может быть длительным и интенсивным процессом для I/O. Оператор `OPTIMIZE` добавляет задачу в пул фоновых работников. Вы можете контролировать, сколько задач выполняется параллельно, с помощью [parallel_chunk_merges](../Server_settings/Searchd.md#parallel_chunk_merges) и сколько частей объединяет каждая задача с помощью [merge_chunks_per_job](../Server_settings/Searchd.md#merge_chunks_per_job). Оптимизационные работники могут быть ограничены по I/O, и вы можете управлять максимальным количеством I/O операций в секунду и максимальным размером I/O операций с помощью директив [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) и [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) соответственно.

Во время оптимизации RT таблица, которая оптимизируется, остается онлайн и доступна для поиска и обновлений практически всё время. Она блокируется на очень короткий период, когда пара дисковых частей успешно объединяется, для переименования старых и новых файлов и обновления заголовка таблицы.

### Оптимизация кластерных таблиц

Если [auto_optimize](../Server_settings/Searchd.md#auto_optimize) не отключена, таблицы оптимизируются автоматически.

Если вы столкнулись с непредвиденными SST или хотите, чтобы таблицы на всех узлах кластера были бинарно идентичными, вам нужно:
1. Отключить [auto_optimize](../Server_settings/Searchd.md#auto_optimize).
2. Вручную оптимизировать таблицы

<!-- example cluster_manual_drop -->
На одном из узлов удалить таблицу из кластера:
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster DROP myindex"
```

<!-- end -->
<!-- example cluster_manual_optimize -->
<!--
data for the following example:

DROP TABLE IF EXISTS myindex;
CREATE TABLE myindex(title text);
INSERT INTO myindex(title) VALUES ('cluster doc');
-->
Оптимизировать таблицу:
<!-- request SQL -->
```sql
OPTIMIZE TABLE myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE myindex"
```

<!-- end -->
<!-- example cluster_manual_add -->
Вернуть таблицу в кластер:
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster ADD myindex"
```

<!-- end -->
Когда таблица добавляется обратно, новые файлы, созданные процессом оптимизации, будут реплицированы на другие узлы кластера.
Любые локальные изменения, сделанные в таблице на других узлах, будут потеряны.

Модификации данных таблицы (вставки, замены, удаления, обновления) должны либо:

1. Быть отложены, либо
2. Быть направлены на узлы, где выполняется процесс оптимизации.

Обратите внимание, что когда таблица находится вне кластера, команды insert/replace/delete/update должны ссылаться на нее без префикса имени кластера (для SQL операторов или свойства cluster в случае HTTP JSON запроса), иначе они завершатся ошибкой.
После добавления таблицы назад в кластер, вы должны продолжить операции записи на таблицу и включать префикс имени кластера опять, иначе они завершатся ошибкой.

Операции поиска доступны как обычно во время процесса на любом из узлов.

<!-- proofread -->
