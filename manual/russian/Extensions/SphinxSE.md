# SphinxSE

SphinxSE — это механизм хранения MySQL, который можно встроить в серверы MySQL/MariaDB с помощью их подключаемой архитектуры.

Несмотря на название, SphinxSE сам по себе *не* хранит никаких данных. Вместо этого он служит встроенным клиентом, который позволяет серверу MySQL взаимодействовать с `searchd`, выполнять поисковые запросы и получать результаты поиска. Все операции индексирования и поиска происходят вне MySQL.

Некоторые типичные сценарии использования SphinxSE включают:
* Упрощение перевода приложений с использованием полнотекстового поиска (FTS) MySQL на Manticore;
* Возможность использования Manticore с языками программирования, для которых пока нет нативных API;
* Предоставление оптимизаций, когда требуется дополнительная обработка результирующего набора Manticore на стороне MySQL (например, JOIN с таблицами исходных документов или дополнительная фильтрация на стороне MySQL).

## Установка SphinxSE

Вам потребуется получить копию исходного кода MySQL, подготовить её, а затем пересобрать бинарный файл MySQL. Исходный код MySQL (mysql-5.x.yy.tar.gz) можно получить с сайта <http://dev.mysql.com>.

### Сборка MySQL 5.0.x с SphinxSE

1.  Скопируйте файл патча `sphinx.5.0.yy.diff` в каталог исходного кода MySQL и выполните
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
Если нет .diff файла для вашей конкретной версии, попробуйте применить .diff с наиболее близкими номерами версий. Важно, чтобы патч применился без конфликтов.
2.  В каталоге исходного кода MySQL выполните
```bash
$ sh BUILD/autorun.sh
```
3.  В каталоге исходного кода MySQL создайте каталог `sql/sphinx` и скопируйте туда все файлы из каталога `mysqlse` из исходного кода Manticore. Пример:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4.  Сконфигурируйте MySQL и включите новый механизм:
```bash
$ ./configure --with-sphinx-storage-engine
```
5.  Соберите и установите MySQL:
```bash
$ make
$ make install
```

### Сборка MySQL 5.1.x с SphinxSE

1. В каталоге исходного кода MySQL создайте каталог `storage/sphinx` и скопируйте все файлы из каталога `mysqlse` в исходном коде Manticore в это новое место. Например:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2.  В каталоге исходного кода MySQL выполните:
```bash
$ sh BUILD/autorun.sh
```
3. Сконфигурируйте MySQL и включите механизм Manticore:
```bash
$ ./configure --with-plugins=sphinx
```
4. Соберите и установите MySQL:
```bash
$ make
$ make install
```

### Проверка установки SphinxSE


<!-- example Example_1 -->

Чтобы убедиться, что SphinxSE успешно встроен в MySQL, запустите вновь собранный сервер, запустите клиент MySQL и выполните запрос `SHOW ENGINES`. Вы должны увидеть список всех доступных механизмов. Manticore должен присутствовать, а в столбце "Support" должно отображаться "YES":

<!-- request -->

``` sql
mysql> show engines;
```

<!-- response -->
```sql
+------------+----------+-------------------------------------------------------------+
| Engine     | Support  | Comment                                                     |
+------------+----------+-------------------------------------------------------------+
| MyISAM     | DEFAULT  | Default engine as of MySQL 3.23 with great performance      |
  ...
| SPHINX     | YES      | Manticore storage engine                                       |
  ...
+------------+----------+-------------------------------------------------------------+
13 rows in set (0.00 sec)
```

<!-- end -->

## Использование SphinxSE

Для поиска с помощью SphinxSE вам необходимо создать специальную "поисковую таблицу" с ENGINE=SPHINX, а затем использовать оператор `SELECT`, поместив полнотекстовый запрос в условие `WHERE` для столбца запроса.

Вот пример создания таблицы и поискового запроса:

```sql
CREATE TABLE t1
(
    id          INTEGER UNSIGNED NOT NULL,
    weight      INTEGER NOT NULL,
    query       VARCHAR(3072) NOT NULL,
    group_id    INTEGER,
    INDEX(query)
) ENGINE=SPHINX CONNECTION="sphinx://localhost:9312/test";

SELECT * FROM t1 WHERE query='test it;mode=any';
```

В поисковой таблице первые три столбца *должны* иметь следующие типы: `INTEGER UNSIGNED` или `BIGINT` для 1-го столбца (ID документа), `INTEGER` или `BIGINT` для 2-го столбца (вес совпадения) и `VARCHAR` или `TEXT` для 3-го столбца (ваш запрос). Это соответствие фиксировано; вы не можете опустить какой-либо из этих трёх обязательных столбцов, поменять их местами или изменить их типы. Кроме того, столбец запроса должен быть проиндексирован, а все остальные — нет. Имена столбцов игнорируются, поэтому вы можете использовать произвольные имена.

Дополнительные столбцы должны быть типа `INTEGER`, `TIMESTAMP`, `BIGINT`, `VARCHAR` или `FLOAT`. Они будут привязаны к атрибутам, предоставляемым в результирующем наборе Manticore, по имени, поэтому их имена должны совпадать с именами атрибутов, указанными в `sphinx.conf`. Если в результатах поиска Manticore нет соответствующего имени атрибута, столбец будет содержать значения `NULL`.

Специальные "виртуальные" имена атрибутов также могут быть привязаны к столбцам SphinxSE. Используйте для этого `_sph_` вместо `@`. Например, чтобы получить значения виртуальных атрибутов `@groupby`, `@count` или `@distinct`, используйте имена столбцов `_sph_groupby`, `_sph_count` или `_sph_distinct` соответственно.

Параметр строки `CONNECTION` используется для указания хоста, порта и таблицы Manticore. Если строка подключения не указана в `CREATE TABLE`, предполагается имя таблицы `*` (т.е. поиск по всем таблицам) и `localhost:9312`. Синтаксис строки подключения следующий:

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

Вы можете изменить строку подключения по умолчанию позже:

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

Вы также можете переопределить эти параметры для каждого запроса.

Как показано в примере, текст запроса и параметры поиска должны быть помещены в условие `WHERE` для столбца поискового запроса (т.е. 3-го столбца). Параметры разделяются точкой с запятой, а их имена от значений — знаком равенства. Можно указать любое количество параметров. Доступные параметры:

* query - текст запроса;
* mode - режим соответствия. Должен быть одним из "all", "any", "phrase", "boolean" или "extended". По умолчанию "all";
* sort - режим сортировки совпадений. Должен быть одним из "relevance", "attr_desc", "attr_asc", "time_segments" или "extended". Во всех режимах, кроме "relevance", после двоеточия также требуется указать имя атрибута (или условие сортировки для "extended"):
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset - смещение в наборе результатов; по умолчанию 0;
* limit - количество совпадений для извлечения из набора результатов; по умолчанию 20;
* index - имена таблиц для поиска:
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid - минимальный и максимальный идентификатор документа для соответствия;
* weights - список весов, разделенных запятыми, для назначения полнотекстовым полям Manticore:
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter - имя атрибута и набор значений для соответствия, разделенные запятыми:
```sql
# only include groups 1, 5 and 19
... WHERE query='test;filter=group_id,1,5,19;';
# exclude groups 3 and 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range - имя атрибута Manticore (целочисленного или bigint), минимальное и максимальное значения для соответствия, разделенные запятыми:
```sql
# include groups from 3 to 7, inclusive
... WHERE query='test;range=group_id,3,7;';
# exclude groups from 5 to 25
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange - имя атрибута Manticore (с плавающей точкой), минимальное и максимальное значения для соответствия, разделенные запятыми:
```sql
# filter by a float size
... WHERE query='test;floatrange=size,2,3;';
# pick all results within 1000 meter from geoanchor
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches - значение maxmatches для запроса, как в [опции поиска max_matches](../Searching/Options.md#max_matches):
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff - максимально допустимое количество совпадений, как в [опции поиска cutoff](../Searching/Options.md#cutoff):
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - максимально допустимое время выполнения запроса (в миллисекундах), как в [опции поиска max_query_time](../Searching/Options.md#max_query_time):
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - функция группировки и атрибут. Прочтите [это](../Searching/Grouping.md#Just-Grouping) о группировке результатов поиска:
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - условие сортировки для группировки:
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - атрибут для вычисления [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT%28DISTINCT-field%29) при выполнении группировки:
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - список имен таблиц и весов, разделенных запятыми, для использования при поиске по нескольким таблицам:
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - список весов для каждого поля, разделенных запятыми, которые могут использоваться ранкером:
```sql
... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';
```
* comment - строка для пометки этого запроса в журнале запросов, как в [опции поиска comment](../Searching/Options.md#comment):
```sql
... WHERE query='test;comment=marker001;';
```
* select - строка с выражениями для вычисления:
```sql
... WHERE query='test;select=2*a+3*** as myexpr;';
```
* host, port - имя удаленного хоста `searchd` и TCP-порт соответственно:
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - функция ранжирования для использования с режимом соответствия "extended", как в [ranker](../Searching/Options.md#ranker). Известные значения: "proximity_bm25", "bm25", "none", "wordcount", "proximity", "matchany", "fieldmask", "sph04", синтаксис "expr:EXPRESSION" для поддержки ранкера на основе выражений (где EXPRESSION следует заменить на вашу конкретную формулу ранжирования) и "export:EXPRESSION":
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
Ранкер "export" функционирует аналогично ranker=expr, но сохраняет значения факторов для каждого документа, в то время как ranker=expr отбрасывает их после вычисления итогового значения `WEIGHT()`. Имейте в виду, что ranker=export предназначен для эпизодического использования, например, для обучения функции машинного обучения (ML) или ручного определения собственной функции ранжирования, и не должен использоваться в реальной производственной среде. При использовании этого ранкера вам, вероятно, захочется изучить вывод функции `RANKFACTORS()`, которая генерирует строку, содержащую все факторы на уровне полей для каждого документа.

<!-- example SQL Example_2 -->
<!-- request -->

``` sql
SELECT *, WEIGHT(), RANKFACTORS()
    FROM myindex
    WHERE MATCH('dog')
    OPTION ranker=export('100*bm25');
```

<!-- response -->

``` sql
*************************** 1\. row ***************************
           id: 555617
    published: 1110067331
   channel_id: 1059819
        title: 7
      content: 428
     weight(): 69900
rankfactors(): bm25=699, bm25a=0.666478, field_mask=2,
doc_word_count=1, field1=(lcs=1, hit_count=4, word_count=1,
tf_idf=1.038127, min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532,
min_hit_pos=120, min_best_span_pos=120, exact_hit=0,
max_window_hits=1), word1=(tf=4, idf=0.259532)
*************************** 2\. row ***************************
           id: 555313
    published: 1108438365
   channel_id: 1058561
        title: 8
      content: 249
     weight(): 68500
rankfactors(): bm25=685, bm25a=0.675213, field_mask=3,
doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1,
tf_idf=0.259532, min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532,
min_hit_pos=8, min_best_span_pos=8, exact_hit=0, max_window_hits=1),
field1=(lcs=1, hit_count=2, word_count=1, tf_idf=0.519063,
min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532, min_hit_pos=36,
min_best_span_pos=36, exact_hit=0, max_window_hits=1), word1=(tf=3,
idf=0.259532)
```

<!-- end -->

* geoanchor - якорь геодистанции. Узнайте больше о геопоиске [в этом разделе](../Searching/Geo_search.md). Принимает 4 параметра, которые являются именами атрибутов широты и долготы и координатами точки якоря соответственно:
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

Одно **очень важное** замечание: **гораздо** эффективнее позволить Manticore обрабатывать сортировку, фильтрацию и нарезку набора результатов, чем увеличивать максимальное количество совпадений и использовать предложения `WHERE`, `ORDER BY` и `LIMIT` на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore использует множество оптимизаций и выполняет эти задачи лучше, чем MySQL. Во-вторых, меньше данных потребуется упаковывать searchd, передавать и распаковывать SphinxSE.


### Важное замечание о хранимых полях при использовании SphinxSE

Начиная с версии 5.0.0, Manticore по умолчанию хранит все поля. Когда Manticore используется вместе с MySQL или MariaDB через SphinxSE, хранение всех полей обычно не имеет смысла, потому что оригиналы уже хранятся в MySQL/MariaDB. В таких конфигурациях рекомендуется явно отключить хранимые поля для задействованной таблицы Manticore, установив:

```
stored_fields =
```

См. справочник по настройке: [stored_fields](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields).

Если вы сохраните значение по умолчанию (все поля хранятся) и затем выберете много документов сразу через SphinxSE, может быть превышен внутренний лимит в движке, и вы можете получить ошибку типа:

"bad searchd response length"

Установка `stored_fields =` позволяет избежать отправки больших хранимых полезных нагрузок обратно в MySQL/MariaDB и предотвращает эту ошибку в типичных интеграциях SphinxSE.


### SHOW ENGINE SPHINX STATUS

<!-- example Example_3 -->

Вы можете получить дополнительную информацию, связанную с результатами запроса, с помощью оператора `SHOW ENGINE SPHINX STATUS`:

<!-- request -->


``` sql
mysql> SHOW ENGINE SPHINX STATUS;
```

<!-- response -->
``` sql
+--------+-------+-------------------------------------------------+
| Type   | Name  | Status                                          |
+--------+-------+-------------------------------------------------+
| SPHINX | stats | total: 25, total found: 25, time: 126, words: 2 |
| SPHINX | words | sphinx:591:1256 soft:11076:15945                |
+--------+-------+-------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->


<!-- example Example_4 -->

Вы также можете получить доступ к этой информации через переменные состояния. Имейте в виду, что использование этого метода не требует прав суперпользователя.

<!-- request -->

``` sql
mysql> SHOW STATUS LIKE 'sphinx_%';
```

<!-- response -->
``` sql
+--------------------+----------------------------------+
| Variable_name      | Value                            |
+--------------------+----------------------------------+
| sphinx_total       | 25                               |
| sphinx_total_found | 25                               |
| sphinx_time        | 126                              |
| sphinx_word_count  | 2                                |
| sphinx_words       | sphinx:591:1256 soft:11076:15945 |
+--------------------+----------------------------------+
5 rows in set (0.00 sec)
```

<!-- end -->


<!-- example SQL Example_5 -->

Таблицы поиска SphinxSE могут быть объединены с таблицами, использующими другие движки. Вот пример с использованием таблицы "documents" из example.sql:

<!-- request -->

``` sql
mysql> SELECT content, date_added FROM test.documents docs
-> JOIN t1 ON (docs.id=t1.id)
-> WHERE query="one document;mode=any";

mysql> SHOW ENGINE SPHINX STATUS;
```

<!-- response -->

``` sql
+-------------------------------------+---------------------+
| content                             | docdate             |
+-------------------------------------+---------------------+
| this is my test document number two | 2006-06-17 14:04:28 |
| this is my test document number one | 2006-06-17 14:04:28 |
+-------------------------------------+---------------------+
2 rows in set (0.00 sec)

+--------+-------+---------------------------------------------+
| Type   | Name  | Status                                      |
+--------+-------+---------------------------------------------+
| SPHINX | stats | total: 2, total found: 2, time: 0, words: 2 |
| SPHINX | words | one:1:2 document:2:2                        |
+--------+-------+---------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

## Построение сниппетов через MySQL


SphinxSE также включает UDF-функцию, которая позволяет создавать сниппеты с помощью MySQL. Эта функциональность аналогична [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting), но доступна через MySQL+SphinxSE.

Двоичный файл, предоставляющий UDF, называется `sphinx.so` и должен автоматически собираться и устанавливаться в соответствующее место вместе с SphinxSE. Если по какой-то причине он не устанавливается автоматически, найдите `sphinx.so` в директории сборки и скопируйте его в каталог плагинов вашего экземпляра MySQL. После этого зарегистрируйте UDF следующей командой:

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

Имя функции *обязательно* должно быть sphinx_snippets; использовать произвольное имя нельзя. Аргументы функции следующие:

**Прототип:** function sphinx_snippets ( document, table, words [, options] );

Аргументы document и words могут быть строками или столбцами таблицы. Опции должны быть указаны следующим образом: `'value' AS option_name`. Для списка поддерживаемых опций обратитесь к разделу [Выделение текста](../Searching/Highlighting.md). Единственная дополнительная опция, специфичная для UDF, называется `sphinx` и позволяет указать расположение searchd (хост и порт).

Примеры использования:

```sql
SELECT sphinx_snippets('hello world doc', 'main', 'world',
    'sphinx://192.168.1.1/' AS sphinx, true AS exact_phrase,
    '[**]' AS before_match, '[/**]' AS after_match)
FROM documents;

SELECT title, sphinx_snippets(text, 'table', 'mysql php') AS text
    FROM sphinx, documents
    WHERE query='mysql php' AND sphinx.id=documents.id;
```
<!-- proofread -->

