# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` — это SQL-запрос, который отображает дополнительную мета-информацию о выполняемом запросе, включая время выполнения запроса, статистику по ключевым словам и информацию о используемых вторичных индексах. Синтаксис:

Включённые элементы:
* `total`: Количество совпадений, которые действительно были извлечены и отправлены клиенту. Это значение обычно ограничивается опцией поиска [LIMIT/size](../Searching/Pagination.md#Pagination-of-search-results).
* `total_found`:
  - Оценочное общее число совпадений для запроса в индексе. Если необходим точный подсчет, используйте `SELECT COUNT(*)` вместо опоры на это значение.
  - Для запросов с `GROUP BY` значение `total_found` представляет количество групп, а не отдельных совпадений.
  - При использовании `HAVING` с `GROUP BY` `total_found` отражает количество групп **после** применения фильтра `HAVING`. Это обеспечивает корректную пагинацию с условиями `HAVING`.
  - Для запросов [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) `total_found` по-прежнему отражает количество групп, независимо от значения `N`.
* `total_relation`: Указывает, является ли значение `total_found` точным или оценочным.
  - Если Manticore не может определить точное значение `total_found`, в этом поле будет отображено `total_relation: gte`, что означает, что фактическое количество совпадений **Больше Или Равно** заявленному в `total_found`.
  - Если значение `total_found` точно, будет отображено `total_relation: eq`.
* `time`: Продолжительность (в секундах) обработки поискового запроса.
* `keyword[N]`: N-й ключевой слово, использованное в поисковом запросе. Обратите внимание, что ключевое слово может быть представлено с использованием подстановочных символов, например, `abc*`.
* `docs[N]`: Общее количество документов (или записей), содержащих N-е ключевое слово из поискового запроса. Если ключевое слово представлено с подстановочным знаком, это значение представляет сумму документов для всех расширенных подслов, возможно превышая фактическое количество совпавших документов.
* `hits[N]`: Общее количество вхождений (или попаданий) N-го ключевого слова во всех документах.
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
`SHOW META` может отображать счётчики ввода/вывода и CPU, но они будут доступны только если searchd был запущен с переключателями `--iostats` и `--cpustats` соответственно.

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
Дополнительные значения, такие как `predicted_time`, `dist_predicted_time`, `local_fetched_docs`, `local_fetched_hits`, `local_fetched_skips` и их соответствующие аналоги `dist_fetched_*` будут доступны только в случае, если `searchd` был настроен с [предсказанными затратами времени](../Server_settings/Searchd.md#predicted_time_costs) и запрос содержал `predicted_time` в разделе `OPTION`.

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

`SHOW META` должен выполняться сразу после запроса в **той же самой** сессии. Поскольку некоторые MySQL коннекторы/библиотеки используют connection pools, выполнение `SHOW META` как отдельного запроса может привести к неожиданным результатам, таким как получение метаданных другого запроса. В таких случаях (и как общее рекомендуемое решение) выполняйте составной запрос, объединяющий основной запрос вместе с `SHOW META`. Некоторые коннекторы/библиотеки поддерживают много-запросы в одном вызове, в то время как другие требуют использования специального метода для много-запросов или установки определённых опций при настройке соединения.

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

Также можно использовать необязательное выражение LIKE, которое позволяет выбрать только переменные, соответствующие определённому шаблону. Синтаксис шаблона следует стандартным SQL подстановочным символам, где `%` обозначает любое количество любых символов, а `_` обозначает одиночный символ.

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

При использовании [фасетного поиска](../Searching/Faceted_search.md) можно проверить поле `multiplier` в выводе `SHOW META`, чтобы определить, сколько запросов было выполнено в оптимизированной группе.

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

Когда [оптимизатор запросов на основе стоимости](../Searching/Cost_based_optimizer.md) выбирает использование `DocidIndex`, `ColumnarScan` или `SecondaryIndex` вместо обычного фильтра, это отражается в команде `SHOW META`.

Переменная `index` отображает имена и типы вторичных индексов, использованных при выполнении запроса. Процент указывает, сколько дисковых чанков (для RT-таблиц) или псевдо-шард (для обычных таблиц) использовали вторичный индекс.

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

`SHOW META` после запроса `CALL PQ` включает:

* `total` — общее время, затраченное на сопоставление документа(ов)
* `queries_matched` — количество сохранённых запросов, совпавших с документом(ами)
* `document_matches` — количество документов, совпавших с запросами, сохранёнными в таблице
* `total_queries_stored` — общее число запросов, сохранённых в таблице
* `term_only_queries` — число запросов в таблице, содержащих только термы; остальные запросы используют расширенный синтаксис запросов.

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

Он включает следующие дополнительные записи:

* `Setup` - Время, затраченное на начальную настройку процесса сопоставления, такое как разбор документов и установка опций
* `Queries failed` - Количество запросов, которые завершились неудачей
* `Fast rejected queries` - Количество запросов, которые не были полностью оценены, но быстро сопоставлены и отклонены с использованием фильтров или других условий
* `Time per query` - Детальное время для каждого запроса
* `Time of matched queries` - Общее время, затраченное на запросы, которые сопоставились с какими-либо документами


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

