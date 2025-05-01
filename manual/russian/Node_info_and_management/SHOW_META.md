# ПОКАЗАТЬ МЕТА

```sql
ПОКАЗАТЬ МЕТА [ LIKE pattern ]
```

<!-- example show meta -->
`ПОКАЗАТЬ МЕТА` является SQL оператором, который отображает дополнительную метаинформацию о обработанном запросе, включая время запроса, статистику по ключевым словам и информацию о использованных вторичных индексах. Синтаксис выглядит так:

Включенные элементы:
* `total`: Количество соответствий, которые были фактически извлечены и отправлены клиенту. Это значение обычно ограничено поисковым параметром [LIMIT/size](Searching/Pagination.md#Pagination-of-search-results).
* `total_found`:
  - Оценочное общее количество соответствий для запроса в индексе. Если вам нужно точное количество соответствий, используйте `SELECT COUNT(*)`, а не полагайтесь на это значение.
  - Для запросов с `GROUP BY` значение `total_found` представляет собой количество групп, а не индивидуальных соответствий.
  - Для запросов [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows), `total_found` по-прежнему представляет собой количество групп, независимо от значения `N`.
* `total_relation`: Указывает, является ли значение `total_found` точным или оценочным.
  - Если Manticore не может определить точное значение `total_found`, это поле отобразит `total_relation: gte`, что означает, что фактическое количество соответствий **Больше Или Равно** указанному `total_found`.
  - Если значение `total_found` точное, будет отображено `total_relation: eq`.
* `time`: Продолжительность (в секундах), необходимая для обработки поискового запроса.
* `keyword[N]`: n-й ключевое слово, использованное в поисковом запросе. Обратите внимание, что ключевое слово может быть представлено в виде подстановочного знака, например, `abc*`.
* `docs[N]`: Общее количество документов (или записей), содержащих n-е ключевое слово из поискового запроса. Если ключевое слово представлено в виде подстановочного знака, это значение представляет собой сумму документов для всех расширенных подключевых слов, потенциально превышая фактическое количество совпавших документов.
* `hits[N]`: Общее количество вхождений (или попаданий) n-го ключевого слова во всех документах.
* `index`: Информация о используемом индексе (например, вторичный индекс).

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
`ПОКАЗАТЬ МЕТА` может отображать счетчики I/O и CPU, но они будут доступны только в том случае, если searchd был запущен с переключателями `--iostats` и `--cpustats`, соответственно.

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
Дополнительные значения, такие как `predicted_time`, `dist_predicted_time`, `local_fetched_docs`, `local_fetched_hits`, `local_fetched_skips` и их соответствующие `dist_fetched_*` аналоги, будут доступны только в том случае, если `searchd` был настроен с [предсказанными временными затратами](../Server_settings/Searchd.md#predicted_time_costs) и запрос включал `predicted_time` в клаузе `OPTION`.

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

`SHOW META` должен быть выполнен сразу после запроса в **той же** сессии. Поскольку некоторые соединители/библиотеки MySQL используют пулы соединений, выполнение `SHOW META` в отдельном запросе может привести к неожиданным результатам, таким как получение метаданных из другого запроса. В этих случаях (и обычно рекомендуется) выполните множественный запрос, содержащий как запрос, так и `SHOW META`. Некоторые соединители/библиотеки поддерживают множественные запросы в одном методе для одного запроса, в то время как другие могут требовать использования специального метода для множественных запросов или установки конкретных опций во время настройки соединения.

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

Вы также можете использовать необязательную клаузу LIKE, которая позволяет вам выбрать только те переменные, которые соответствуют определенному шаблону. Синтаксис шаблона следует стандартным подстановочным знакам SQL, где `%` представляет любое количество любых символов, а `_` представляет один символ.

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

При использовании [фасетного поиска](../Searching/Faceted_search.md) вы можете изучить поле `multiplier` в выводе `SHOW META`, чтобы определить, сколько запросов было выполнено в оптимизированной группе.

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

## SHOW META и оптимизатор запроса

<!-- example of show meta vs query optimizer -->
Когда [оптимизатор запросов на основе стоимости](../Searching/Cost_based_optimizer.md) выбирает использование `DocidIndex`, `ColumnarScan` или `SecondaryIndex` вместо простого фильтра, это отражается в команде `SHOW META`.

Переменная `index` отображает имена и типы вторичных индексов, используемых во время выполнения запроса. Процент указывает, сколько дисковых кусочков (в случае таблицы RT) или псевдо-шардов (в случае простой таблицы) использовали вторичный индекс.

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

`SHOW META` может быть использован после выполнения оператора [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ), в этом случае он предоставляет другой вывод.

`SHOW META` после оператора `CALL PQ` включает:

* `total` - Общее время, затраченное на сопоставление документа(ов)
* `queries_matched` - Количество сохраненных запросов, которые соответствуют документу(ам)
* `document_matches` - Количество документов, соответствующих запросам, сохраненным в таблице
* `total_queries_stored` - Общее количество запросов, сохраненных в таблице
* `term_only_queries` - Количество запросов в таблице, которые имеют термины; оставшиеся запросы используют расширенный синтаксис запросов.

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

Использование `CALL PQ` с опцией `verbose` предоставляет более детальный вывод.

Он включает в себя следующие дополнительные записи:

* `Setup` - Время, потраченное на начальную настройку процесса сопоставления, такое как разбор документов и установка параметров
* `Queries failed` - Количество недействительных запросов
* `Fast rejected queries` - Количество запросов, которые не были полностью оценены, но были быстро сопоставлены и отклонены с использованием фильтров или других условий
* `Time per query` - Детализированное время для каждого запроса
* `Time of matched queries` - Общее время, затраченное на запросы, которые соответствуют любым документам


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

