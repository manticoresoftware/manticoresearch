# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` — это оператор SQL, который отображает дополнительную метаинформацию о выполняемом запросе, включая время запроса, статистику по ключевым словам и информацию об используемых вторичных индексах. Синтаксис следующий:

Включённые элементы:
* `total`: Количество совпадений, фактически полученных и отправленных клиенту. Обычно это значение ограничивается опцией поиска [LIMIT/size](../Searching/Pagination.md#Pagination-of-search-results).
* `total_found`:
  - Оценочное общее количество совпадений по запросу в индексе. Если вам нужно точное число совпадений, используйте `SELECT COUNT(*)`, а не полагайтесь на это значение.
  - Для запросов с `GROUP BY` `total_found` представляет количество групп, а не отдельных совпадений.
  - Для запросов [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) `total_found` всё равно обозначает количество групп вне зависимости от значения `N`.
* `total_relation`: Указывает, является ли значение `total_found` точным или оценочным.
  - Если Manticore не может определить точное значение `total_found`, в поле будет показано `total_relation: gte`, что значит, что реальное количество совпадений **больше или равно** указанному `total_found`.
  - Если значение `total_found` точное, будет показано `total_relation: eq`.
* `time`: Время (в секундах), затраченное на выполнение поискового запроса.
* `keyword[N]`: N-ое ключевое слово, использованное в поисковом запросе. Обратите внимание, что ключевое слово может быть представлено с помощью шаблона, например, `abc*`.
* `docs[N]`: Общее количество документов (или записей), содержащих N-ое ключевое слово из запроса. Если ключевое слово представлено шаблоном, данное значение — сумма документов для всех расширенных под-ключевых слов, что может превышать реальное количество совпавших документов.
* `hits[N]`: Общее количество вхождений (или совпадений) N-ого ключевого слова во всех документах.
* `index`: Информация об используемом индексе (например, вторичный индекс).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id, story_author FROM hn_small WHERE MATCH('one|two|three') and comment_ranking > 2 limit 5;
show meta;
```

<!-- response SQL -->

```sql
+---------+--------------+
| id      | story_author |
+---------+--------------+
|  151171 | anewkid      |
|  302758 | bks          |
|  805806 | drRoflol     |
| 1099245 | tnorthcutt   |
|  303252 | whiten       |
+---------+--------------+
5 rows in set (0.00 sec)

+----------------+---------------------------------------+
| Variable_name  | Value                                 |
+----------------+---------------------------------------+
| total          | 5                                     |
| total_found    | 2308                                  |
| total_relation | eq                                    |
| time           | 0.001                                 |
| keyword[0]     | one                                   |
| docs[0]        | 224387                                |
| hits[0]        | 310327                                |
| keyword[1]     | three                                 |
| docs[1]        | 18181                                 |
| hits[1]        | 21102                                 |
| keyword[2]     | two                                   |
| docs[2]        | 63251                                 |
| hits[2]        | 75961                                 |
| index          | comment_ranking:SecondaryIndex (100%) |
+----------------+---------------------------------------+
14 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta iostats cpustats -->
`SHOW META` может отображать счётчики ввода-вывода и процессора, но они будут доступны только если searchd был запущен с опциями `--iostats` и `--cpustats`, соответственно.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,channel_id FROM records WHERE MATCH('one|two|three') limit 5;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+-----------------------+--------+
| Variable_name         | Value  |
+-----------------------+--------+
| total                 | 5      |
| total_found           | 266385 |
| total_relation        | eq     |
| time                  | 0.011  |
| cpu_time              | 18.004 |
| agents_cpu_time       | 0.000  |
| io_read_time          | 0.000  |
| io_read_ops           | 0      |
| io_read_kbytes        | 0.0    |
| io_write_time         | 0.000  |
| io_write_ops          | 0      |
| io_write_kbytes       | 0.0    |
| agent_io_read_time    | 0.000  |
| agent_io_read_ops     | 0      |
| agent_io_read_kbytes  | 0.0    |
| agent_io_write_time   | 0.000  |
| agent_io_write_ops    | 0      |
| agent_io_write_kbytes | 0.0    |
| keyword[0]            | one    |
| docs[0]               | 224387 |
| hits[0]               | 310327 |
| keyword[1]            | three  |
| docs[1]               | 18181  |
| hits[1]               | 21102  |
| keyword[2]            | two    |
| docs[2]               | 63251  |
| hits[2]               | 75961  |
+-----------------------+--------+
27 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta predicted_time -->
Дополнительные значения, такие как `predicted_time`, `dist_predicted_time`, `local_fetched_docs`, `local_fetched_hits`, `local_fetched_skips`, а также соответствующие им `dist_fetched_*`, будут доступны только если `searchd` был настроен с [предсказанием временных затрат](../Server_settings/Searchd.md#predicted_time_costs) и запрос содержал `predicted_time` в разделе `OPTION`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') limit 5 option max_predicted_time=100;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

mysql> show meta;
+---------------------+--------+
| Variable_name       | Value  |
+---------------------+--------+
| total               | 5      |
| total_found         | 266385 |
| total_relation      | eq     |
| time                | 0.012  |
| local_fetched_docs  | 307212 |
| local_fetched_hits  | 407390 |
| local_fetched_skips | 24     |
| predicted_time      | 56     |
| keyword[0]          | one    |
| docs[0]             | 224387 |
| hits[0]             | 310327 |
| keyword[1]          | three  |
| docs[1]             | 18181  |
| hits[1]             | 21102  |
| keyword[2]          | two    |
| docs[2]             | 63251  |
| hits[2]             | 75961  |
+---------------------+--------+
17 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta single statement -->

`SHOW META` должен выполняться сразу после запроса в **той же** сессии. Поскольку некоторые MySQL коннекторы/библиотеки используют пулы соединений, выполнение `SHOW META` в отдельном операторе может привести к неожиданным результатам, таким как получение метаданных от другого запроса. В таких случаях (а также в общем рекомендуется) выполняйте составной запрос, содержащий как сам запрос, так и `SHOW META`. Некоторые коннекторы/библиотеки поддерживают мультизапросы в одном вызове, тогда как другие требуют отдельного метода для мультизапросов или установки специальных опций при подключении.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') LIMIT 5; SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
| time           | 0.011  |
| keyword[0]     | one    |
| docs[0]        | 224387 |
| hits[0]        | 310327 |
| keyword[1]     | three  |
| docs[1]        | 18181  |
| hits[1]        | 21102  |
| keyword[2]     | two    |
| docs[2]        | 63251  |
| hits[2]        | 75961  |
+----------------+--------+
13 rows in set (0.00 sec)
```

<!-- end -->

<!-- example SHOW META LIKE -->

Вы также можете использовать опциональное выражение LIKE, которое позволяет выбрать только переменные, соответствующие определённому шаблону. Синтаксис шаблона соответствует стандартным SQL шаблонам, где `%` обозначает любое количество любых символов, а `_` — один символ.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW META LIKE 'total%';
```

<!-- response SQL -->

```sql
+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
+----------------+--------+
3 rows in set (0.00 sec)
```

<!-- end -->

## SHOW META и фасеты

<!-- example show meta facets -->

При использовании [фасетного поиска](../Searching/Faceted_search.md) вы можете просмотреть поле `multiplier` в выводе `SHOW META`, чтобы определить, сколько оптимизированных запросов было выполнено в группе.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET brand_id FACET price FACET categories;
SHOW META LIKE 'multiplier';
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
...

+----------+----------+
| brand_id | count(*) |
+----------+----------+
|        1 |     1013 |
...

+-------+----------+
| price | count(*) |
+-------+----------+
|   306 |        7 |
...

+------------+----------+
| categories | count(*) |
+------------+----------+
|         10 |     2436 |
...

+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| multiplier    | 4     |
+---------------+-------+
1 row in set (0.00 sec)
```

<!-- end -->

## SHOW META и оптимизатор запросов

<!-- example of show meta vs query optimizer -->

Когда [оптимизатор запросов на основе оценки стоимости](../Searching/Cost_based_optimizer.md) выбирает использование `DocidIndex`, `ColumnarScan` или `SecondaryIndex` вместо обычного фильтра, это отражается в команде `SHOW META`.

Переменная `index` отображает имена и типы вторичных индексов, используемых во время выполнения запроса. Процент показывает, сколько дисковых чанков (для РТ-таблицы) или псевдо-шардов (для обычной таблицы) использовали вторичный индекс.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT count(*) FROM taxi1 WHERE tip_amount = 5;
SHOW META;
```

<!-- response SQL -->

```sql
+----------------+----------------------------------+
| Variable_name  | Value                            |
+----------------+----------------------------------+
| total          | 1                                |
| total_found    | 1                                |
| total_relation | eq                               |
| time           | 0.016                            |
| index          | tip_amount:SecondaryIndex (100%) |
+----------------+----------------------------------+
5 rows in set (0.00 sec)
```

<!-- end -->

## SHOW META для PQ таблиц

<!-- example show meta PQ -->

`SHOW META` можно использовать после выполнения [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ), в этом случае вывод будет отличаться.

`SHOW META` после `CALL PQ` включает:

* `total` — Общее время, затраченное на совпадение с документом(ами)
* `queries_matched` — Количество сохранённых запросов, соответствующих документам
* `document_matches` — Количество документов, которые соответствовали сохранённым запросам
* `total_queries_stored` — Общее количество запросов, сохранённых в таблице
* `term_only_queries` — Количество запросов в таблице, содержащих термы; остальные запросы используют расширенный синтаксис запросов.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }')); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-----------------------+-----------+
| Variable name         | Value     |
+-----------------------+-----------+
| total                 | 0.000 sec |
| queries_matched       | 1         |
| queries_failed        | 0         |
| document_matched      | 1         |
| total_queries_stored  | 2         |
| term_only_queries     | 2         |
| fast_rejected_queries | 1         |
+-----------------------+-----------+
7 rows in set (0.00 sec)
```

<!-- end -->

<!-- example call pq verbose meta  -->

Использование `CALL PQ` с опцией `verbose` предоставляет более подробный вывод.

Это включает следующие дополнительные записи:

* `Setup` - Время, затраченное на начальную настройку процесса сопоставления, такое как разбор документов и установка параметров
* `Queries failed` - Количество неудачных запросов
* `Fast rejected queries` - Количество запросов, которые не были полностью обработаны, но быстро сопоставлены и отклонены с помощью фильтров или других условий
* `Time per query` - Подробное время для каждого запроса
* `Time of matched queries` - Общее время, затраченное на запросы, которые совпали с любыми документами


<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }'), 1 as verbose); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-------------------------+-----------+
| Variable name           | Value     |
+-------------------------+-----------+
| total                   | 0.000 sec |
| setup                   | 0.000 sec |
| queries_matched         | 1         |
| queries_failed          | 0         |
| document_matched        | 1         |
| total_queries_stored    | 2         |
| term_only_queries       | 2         |
| fast_rejected_queries   | 1         |
| time_per_query          | 69        |
| time_of_matched_queries | 69        |
+-------------------------+-----------+
10 rows in set (0.00 sec)
```

<!-- end -->

<!-- proofread -->

