# SphinxSE

SphinxSE — это движок хранения MySQL, который можно скомпилировать в серверы MySQL/MariaDB, используя их модульную архитектуру.

Несмотря на своё название, SphinxSE *не* хранит никаких данных самостоятельно. Вместо этого он служит встроенным клиентом, который позволяет серверу MySQL взаимодействовать с `searchd`, выполнять поисковые запросы и получать результаты поиска. Вся индексация и поиск происходят вне MySQL.

Некоторые распространённые применения SphinxSE включают:
* Упрощение портирования приложений MySQL Full-Text Search (FTS) на Manticore;
* Обеспечение использования Manticore с языками программирования, для которых ещё нет родных API;
* Предоставление оптимизаций, когда требуется дополнительная обработка результатов Manticore на стороне MySQL (например, JOIN с таблицами оригинальных документов или дополнительная фильтрация на стороне MySQL).

## Установка SphinxSE

Вам потребуется получить исходники MySQL, подготовить их и затем перекомпилировать бинарный файл MySQL. Исходники MySQL (mysql-5.x.yy.tar.gz) можно получить с сайта <http://dev.mysql.com>.

### Компиляция MySQL 5.0.x с SphinxSE

1.  скопируйте патч `sphinx.5.0.yy.diff` в каталог исходников MySQL и выполните
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
Если нет .diff файла именно для нужной версии сборки, попробуйте применить .diff с ближайшими номерами версий. Важно, чтобы патч применился без конфликтов.
2.  в каталоге исходников MySQL выполните
```bash
$ sh BUILD/autorun.sh
```
3.  в каталоге исходников MySQL создайте каталог `sql/sphinx` и скопируйте туда все файлы из каталога `mysqlse` исходников Manticore. Например:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4.  сконфигурируйте MySQL и включите новый движок:
```bash
$ ./configure --with-sphinx-storage-engine
```
5.  соберите и установите MySQL:
```bash
$ make
$ make install
```

### Компиляция MySQL 5.1.x с SphinxSE

1. В каталоге исходников MySQL создайте каталог `storage/sphinx` и скопируйте все файлы из каталога `mysqlse` исходников Manticore в это новое место. Например:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2.  В каталоге исходников MySQL выполните:
```bash
$ sh BUILD/autorun.sh
```
3. Сконфигурируйте MySQL и включите движок Manticore:
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

Чтобы проверить, что SphinxSE успешно скомпилирован в MySQL, запустите вновь собранный сервер, выполните клиент MySQL и запрос `SHOW ENGINES`. Вы увидите список всех доступных движков. Должен присутствовать Manticore, а в колонке "Support" должно стоять "YES":

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

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "show engines;"
```

<!-- response JSON -->
```JSON
[
  {
    "total": 1,
    "error": "",
    "warning": "",
    "columns": [
      {
        "Engine": {
          "type": "string"
        }
      },
      {
        "Support": {
          "type": "string"
        }
      },
      {
        "Comment": {
          "type": "string"
        }
      },
      {
        "Transactions": {
          "type": "string"
        }
      },
      {
        "XA": {
          "type": "string"
        }
      },
      {
        "Savepoints": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Engine": "MyISAM",
        "Support": "DEFAULT",
        "Comment": "MyISAM storage engine",
        "Transactions": "NO",
        "XA": "NO",
        "Savepoints": "NO"
      }
    ]
  }
]
```

<!-- end -->

## Использование SphinxSE

Чтобы выполнить поиск с помощью SphinxSE, необходимо создать специальную таблицу с `ENGINE=SPHINX` — «поисковую таблицу», а затем использовать оператор `SELECT` с полнотекстовым запросом, помещённым в условие `WHERE` для колонки запроса.

Вот пример оператора создания и поискового запроса:

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

В поисковой таблице первые три колонки *должны* иметь следующие типы: `INTEGER UNSIGNED` или `BIGINT` для первой колонки (ID документа), `INTEGER` или `BIGINT` для второй (вес совпадения), и `VARCHAR` или `TEXT` для третьей колонки (ваш запрос). Это фиксированное соответствие; вы не можете опускать ни одну из этих трёх обязательных колонок, менять их порядок или типы. Кроме того, колонка с запросом должна иметь индекс, а все остальные — не индексироваться. Имена колонок игнорируются, поэтому можно использовать любые имена.

Дополнительные колонки должны иметь типы `INTEGER`, `TIMESTAMP`, `BIGINT`, `VARCHAR` или `FLOAT`. Они будут связаны по имени с атрибутами, предоставленными в результирующем наборе Manticore, поэтому их имена должны совпадать с именами атрибутов, указанными в `sphinx.conf`. Если в результатах поиска Manticore нет соответствующего имени атрибута, колонка будет иметь значения `NULL`.

Также можно к колонкам SphinxSE привязывать специальные «виртуальные» имена атрибутов. Для этого используйте `_sph_` вместо `@`. Например, чтобы получить значения виртуальных атрибутов `@groupby`, `@count` или `@distinct`, используйте имена колонок `_sph_groupby`, `_sph_count` или `_sph_distinct` соответственно.

Параметр строки `CONNECTION` используется для указания хоста Manticore, порта и таблицы. Если при `CREATE TABLE` строка соединения не указана, предполагается имя таблицы `*` (т.е. поиск по всем таблицам) и `localhost:9312`. Синтаксис строки соединения следующий:

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

Позже вы можете изменить строку соединения:

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

Также можно переопределить эти параметры для каждого запроса.

Как показано в примере, и текст запроса, и опции поиска должны быть помещены в условие `WHERE` для колонки поиска (т.е. для третьей колонки). Опции отделяются точкой с запятой, а имя опции и её значение разделяются знаком равенства. Можно указать любое количество опций. Доступные опции:

* query — текст запроса;
* mode — режим сопоставления. Должен быть один из "all", "any", "phrase", "boolean" или "extended". По умолчанию "all";
* sort — режим сортировки совпадений. Должен быть один из "relevance", "attr_desc", "attr_asc", "time_segments" или "extended". Во всех режимах, кроме "relevance", после двоеточия требуется имя атрибута (или сортировочное выражение для "extended"):
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset — смещение в результирующем наборе; по умолчанию 0;
* limit — количество совпадений для получения из результирующего набора; по умолчанию 20;
* index — имена таблиц для поиска:
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid — минимальный и максимальный ID документов для сопоставления;
* weights — список весов через запятую для присвоения полнотекстовым полям Manticore:
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter — имя атрибута и набор значений для сопоставления, разделённые запятой:
```sql
# only include groups 1, 5 and 19
... WHERE query='test;filter=group_id,1,5,19;';
# exclude groups 3 and 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range — имя атрибута Manticore (integer или bigint), минимальное и максимальное значения для сопоставления, через запятую:
```sql
# include groups from 3 to 7, inclusive
... WHERE query='test;range=group_id,3,7;';
# exclude groups from 5 to 25
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange — имя атрибута Manticore с плавающей точкой, минимальное и максимальное значения для сопоставления, через запятую:
```sql
# filter by a float size
... WHERE query='test;floatrange=size,2,3;';
# pick all results within 1000 meter from geoanchor
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches — максимальное число совпадений на запрос, как в опции поиска [max_matches](../Searching/Options.md#max_matches):
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff — максимально допустимое число совпадений, как в опции поиска [cutoff](../Searching/Options.md#cutoff):
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - максимальное допустимое время выполнения запроса (в миллисекундах), как в [опции поиска max_query_time](../Searching/Options.md#max_query_time):
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - функция группировки и атрибут. Подробнее об [группировке результатов поиска](../Searching/Grouping.md#Just-Grouping):
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - параметр сортировки при группировке:
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - атрибут для вычисления [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT%28DISTINCT-field%29) при выполнении группировки:
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - список через запятую с именами таблиц и весами, используемыми при поиске по нескольким таблицам:
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - список через запятую с весами для отдельных полей, которые могут использоваться ранжировщиком:
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
* host, port - имя хоста удалённого `searchd` и TCP-порт соответственно:
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - функция ранжирования, используемая в режиме "extended" сопоставления, как в [ranker](../Searching/Options.md#ranker). Известные значения: "proximity_bm25", "bm25", "none", "wordcount", "proximity", "matchany", "fieldmask", "sph04", синтаксис "expr:EXPRESSION" для поддержки ранжировщика на основе выражения (где EXPRESSION следует заменить на вашу конкретную формулу ранжирования), а также "export:EXPRESSION":
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
Ранжировщик "export" работает аналогично ranker=expr, но сохраняет значения факторов для каждого документа, в то время как ranker=expr отбрасывает их после вычисления окончательного значения `WEIGHT()`. Имейте в виду, что ranker=export предназначен для редкого использования, например, для обучения функции машинного обучения (ML) или ручного определения собственной функции ранжирования, и не должен использоваться в реальной производственной среде. При использовании этого ранжировщика, вероятно, вы захотите изучить вывод функции `RANKFACTORS()`, которая генерирует строку, содержащую все факторы на уровне поля для каждого документа.

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

* geoanchor - геодистанционная привязка. Подробнее о гео-поиске [в этом разделе](../Searching/Geo_search.md). Принимает 4 параметра, которые являются именами атрибутов широты и долготы, а также координатами якорной точки соответственно:
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

Одно **очень важное** замечание: **намного** эффективнее позволить Manticore выполнять сортировку, фильтрацию и нарезку результирующего набора, чем увеличивать количество максимальных совпадений и использовать выражения `WHERE`, `ORDER BY` и `LIMIT` на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore использует разнообразные оптимизации и выполняет эти задачи лучше, чем MySQL. Во-вторых, меньше данных нужно будет упаковывать searchd, передавать и распаковывать SphinxSE.


### Важное замечание о сохранённых полях при использовании SphinxSE

Начиная с версии 5.0.0, Manticore по умолчанию сохраняет все поля. Когда Manticore используется вместе с MySQL или MariaDB через SphinxSE, обычно нет смысла сохранять все поля, поскольку оригиналы уже сохранены в MySQL/MariaDB. В таких случаях рекомендуется явно отключить сохранение полей для соответствующей таблицы Manticore с помощью параметра:

```
stored_fields =
```

См. справку по настройке: [stored_fields](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields).

Если оставить значение по умолчанию (все поля сохраняются) и при этом выполнить выборку большого количества документов через SphinxSE, может быть превышен внутренний лимит в движке, и вы получите ошибку вида:

"bad searchd response length"

Установка `stored_fields =` позволяет избежать отправки больших сохранённых данных обратно в MySQL/MariaDB и предотвращает эту ошибку при типичной интеграции с SphinxSE.


### SHOW ENGINE SPHINX STATUS

<!-- example Example_3 -->

Вы можете получить дополнительную информацию, связанную с результатами запроса, используя оператор `SHOW ENGINE SPHINX STATUS`:

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

Также эту информацию можно получить через переменные статуса. Учтите, что этот способ не требует прав суперпользователя.

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

Таблицы поиска SphinxSE могут объединяться с таблицами, использующими другие движки. Вот пример с таблицей "documents" из example.sql:

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

## Создание сниппетов через MySQL


SphinxSE также содержит функцию UDF, которая позволяет создавать сниппеты с помощью MySQL. Эта функциональность похожа на [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting), но доступна через MySQL+SphinxSE.

Бинарный файл с реализацией UDF называется `sphinx.so` и должен автоматически собираться и устанавливаться в нужное место вместе с SphinxSE. Если он не установился автоматически по какой-то причине, найдите `sphinx.so` в каталоге сборки и скопируйте его в директорию плагинов вашей инсталляции MySQL. После этого зарегистрируйте UDF следующей командой:

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

Имя функции *обязательно* должно быть sphinx_snippets; нельзя использовать произвольное имя. Аргументы функции таковы:

**Прототип:** function sphinx_snippets ( document, table, words [, options] );

Аргументы document и words могут быть как строками, так и столбцами таблицы. Опции должны указываться в формате: `'value' AS option_name`. Список поддерживаемых опций см. в разделе [Подсветка](../Searching/Highlighting.md). Единственной дополнительной опцией, специфичной для UDF, является `sphinx`, которая позволяет указать расположение searchd (хост и порт).

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

