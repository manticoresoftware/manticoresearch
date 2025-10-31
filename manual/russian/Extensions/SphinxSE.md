# SphinxSE

SphinxSE — это движок хранения MySQL, который можно скомпилировать в серверы MySQL/MariaDB, используя их модульную архитектуру.

Несмотря на своё название, SphinxSE *не* хранит данные самостоятельно. Вместо этого он служит встроенным клиентом, который позволяет серверу MySQL взаимодействовать с `searchd`, выполнять поисковые запросы и получать результаты поиска. Вся индексация и поиск происходят вне MySQL.

Некоторые распространённые применения SphinxSE включают:
* Упрощение переноса приложений MySQL Full-Text Search (FTS) на Manticore;
* Обеспечение использования Manticore с языками программирования, для которых нативные API ещё недоступны;
* Предоставление оптимизаций, когда требуется дополнительная обработка результатов Manticore на стороне MySQL (например, JOIN с оригинальными таблицами документов или дополнительная фильтрация на стороне MySQL).

## Установка SphinxSE

Вам потребуется получить исходники MySQL, подготовить их и затем перекомпилировать бинарный файл MySQL. Исходники MySQL (mysql-5.x.yy.tar.gz) можно получить с сайта <http://dev.mysql.com>.

### Компиляция MySQL 5.0.x с SphinxSE

1.  скопируйте патч-файл `sphinx.5.0.yy.diff` в директорию с исходниками MySQL и выполните
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
Если нет .diff файла именно для нужной версии:   сборки, попробуйте применить .diff с ближайшими номерами версий. Важно, чтобы патч применился без отклонений.
2.  в директории с исходниками MySQL выполните
```bash
$ sh BUILD/autorun.sh
```
3.  в директории с исходниками MySQL создайте каталог `sql/sphinx` и скопируйте туда все файлы из каталога `mysqlse` исходников Manticore. Пример:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4.  настройте MySQL и включите новый движок:
```bash
$ ./configure --with-sphinx-storage-engine
```
5.  соберите и установите MySQL:
```bash
$ make
$ make install
```

### Компиляция MySQL 5.1.x с SphinxSE

1. В директории с исходниками MySQL создайте каталог `storage/sphinx` и скопируйте все файлы из каталога `mysqlse` исходников Manticore в это новое место. Например:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2.  В директории с исходниками MySQL выполните:
```bash
$ sh BUILD/autorun.sh
```
3. Настройте MySQL и включите движок Manticore:
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

Чтобы проверить, что SphinxSE успешно скомпилирован в MySQL, запустите вновь собранный сервер, выполните клиент MySQL и выполните запрос `SHOW ENGINES`. Вы должны увидеть список всех доступных движков. Manticore должен присутствовать, а в столбце "Support" должно быть "YES":

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

Для поиска с помощью SphinxSE необходимо создать специальную "поисковую таблицу" с ENGINE=SPHINX, а затем использовать оператор `SELECT` с полнотекстовым запросом, помещённым в условие `WHERE` для колонки запроса.

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

В поисковой таблице первые три колонки *должны* иметь следующие типы: `INTEGER UNSIGNED` или `BIGINT` для первой колонки (ID документа), `INTEGER` или `BIGINT` для второй колонки (вес совпадения) и `VARCHAR` или `TEXT` для третьей колонки (ваш запрос). Это сопоставление фиксировано; нельзя пропускать какие-либо из этих трёх обязательных колонок, менять их порядок или типы. Кроме того, колонка запроса должна быть индексирована, а все остальные — не индексированы. Имена колонок игнорируются, поэтому можно использовать любые произвольные имена.

Дополнительные колонки должны быть одного из типов: `INTEGER`, `TIMESTAMP`, `BIGINT`, `VARCHAR` или `FLOAT`. Они будут связаны с атрибутами, предоставленными в наборе результатов Manticore по имени, поэтому их имена должны совпадать с именами атрибутов, указанными в `sphinx.conf`. Если в результатах поиска Manticore нет соответствующего имени атрибута, в колонке будут значения `NULL`.

Специальные "виртуальные" имена атрибутов также могут быть связаны с колонками SphinxSE. Для этого используйте `_sph_` вместо `@`. Например, чтобы получить значения виртуальных атрибутов `@groupby`, `@count` или `@distinct`, используйте имена колонок `_sph_groupby`, `_sph_count` или `_sph_distinct` соответственно.

Строковый параметр `CONNECTION` используется для указания хоста, порта и таблицы Manticore. Если в `CREATE TABLE` не указана строка подключения, предполагается имя таблицы `*` (т.е. поиск по всем таблицам) и `localhost:9312`. Синтаксис строки подключения следующий:

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

Вы можете изменить строку подключения по умолчанию позже:

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

Также можно переопределять эти параметры для каждого запроса.

Как показано в примере, и текст запроса, и параметры поиска должны быть помещены в условие `WHERE` для колонки поискового запроса (т.е. третьей колонки). Опции разделяются точкой с запятой, а имена и значения — знаком равенства. Можно указать любое количество опций. Доступные опции:

* query — текст запроса;
* mode — режим сопоставления. Должен быть одним из "all", "any", "phrase", "boolean" или "extended". По умолчанию "all";
* sort — режим сортировки совпадений. Должен быть одним из "relevance", "attr_desc", "attr_asc", "time_segments" или "extended". Во всех режимах, кроме "relevance", после двоеточия требуется имя атрибута (или сортировочное выражение для "extended"):
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset — смещение в наборе результатов; по умолчанию 0;
* limit — количество совпадений для извлечения из набора результатов; по умолчанию 20;
* index — имена таблиц для поиска:
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid — минимальный и максимальный ID документа для совпадения;
* weights — список весов, разделённых запятыми, которые будут назначены полнотекстовым полям Manticore:
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter — имя атрибута и набор значений для совпадения, разделённые запятыми:
```sql
# only include groups 1, 5 and 19
... WHERE query='test;filter=group_id,1,5,19;';
# exclude groups 3 and 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range — имя атрибута Manticore (integer или bigint) и минимальное и максимальное значения для совпадения, разделённые запятыми:
```sql
# include groups from 3 to 7, inclusive
... WHERE query='test;range=group_id,3,7;';
# exclude groups from 5 to 25
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange — имя атрибута Manticore (с плавающей точкой) и минимальное и максимальное значения для совпадения, разделённые запятыми:
```sql
# filter by a float size
... WHERE query='test;floatrange=size,2,3;';
# pick all results within 1000 meter from geoanchor
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches — максимальное количество совпадений для запроса, как в [опции поиска max_matches](../Searching/Options.md#max_matches):
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff — максимальное допустимое количество совпадений, как в [опции поиска cutoff](../Searching/Options.md#cutoff):
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - максимальное допустимое время выполнения запроса (в миллисекундах), как в [опции поиска max_query_time](../Searching/Options.md#max_query_time):
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - функция группировки и атрибут. Прочитайте [это](../Searching/Grouping.md#Just-Grouping) о группировке результатов поиска:
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - условие сортировки групп:
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - атрибут для вычисления [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT%28DISTINCT-field%29) при выполнении группировки:
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - список через запятую имен таблиц и весов, используемых при поиске по нескольким таблицам:
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - список через запятую весов для каждого поля, которые могут использоваться ранжировщиком:
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
* host, port - имя удалённого хоста `searchd` и TCP-порт соответственно:
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - функция ранжирования для использования с режимом "extended" сопоставления, как в [ranker](../Searching/Options.md#ranker). Известные значения: "proximity_bm25", "bm25", "none", "wordcount", "proximity", "matchany", "fieldmask", "sph04", синтаксис "expr:EXPRESSION" для поддержки ранжировщика на основе выражений (где EXPRESSION следует заменить вашей конкретной формулой ранжирования), и "export:EXPRESSION":
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
Ранжировщик "export" работает аналогично ranker=expr, но сохраняет значения факторов для каждого документа, в то время как ranker=expr отбрасывает их после вычисления итогового значения `WEIGHT()`. Имейте в виду, что ranker=export предназначен для редкого использования, например, для обучения функции машинного обучения (ML) или ручного определения собственной функции ранжирования, и не должен использоваться в реальной эксплуатации. При использовании этого ранжировщика, скорее всего, вы захотите изучить вывод функции `RANKFACTORS()`, которая генерирует строку, содержащую все факторы на уровне полей для каждого документа.

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

* geoanchor - якорь геодистанции. Подробнее о геопоиске [в этом разделе](../Searching/Geo_search.md). Принимает 4 параметра: имена атрибутов широты и долготы, а также координаты якорной точки соответственно:
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

Очень **важное** замечание: **гораздо** эффективнее позволить Manticore выполнять сортировку, фильтрацию и нарезку набора результатов, чем увеличивать максимальное количество совпадений и использовать `WHERE`, `ORDER BY` и `LIMIT` на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore применяет различные оптимизации и выполняет эти задачи лучше, чем MySQL. Во-вторых, меньше данных нужно упаковывать searchd, передавать и распаковывать SphinxSE.


### Важное замечание о сохранённых полях при использовании SphinxSE

Начиная с версии 5.0.0, Manticore по умолчанию сохраняет все поля. При использовании Manticore вместе с MySQL или MariaDB через SphinxSE обычно нет смысла сохранять все поля, так как оригиналы уже хранятся в MySQL/MariaDB. В таких конфигурациях рекомендуется явно отключить сохранение полей для соответствующей таблицы Manticore, установив:

```
stored_fields =
```

См. справочник по настройке: [stored_fields](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields).

Если оставить значение по умолчанию (все поля сохранены) и затем выбрать много документов одновременно через SphinxSE, может быть превышен внутренний лимит движка, и вы получите ошибку вида:

"bad searchd response length"

Установка `stored_fields =` предотвращает отправку больших сохранённых данных обратно в MySQL/MariaDB и предотвращает эту ошибку в типичных интеграциях SphinxSE.


### ПОКАЗАТЬ СТАТУС ENGINE SPHINX

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

Также вы можете получить эту информацию через переменные состояния. Учтите, что для этого способа не требуются права суперпользователя.

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

Таблицы поиска SphinxSE можно объединять с таблицами, использующими другие движки. Вот пример с таблицей "documents" из example.sql:

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

Бинарный файл, предоставляющий UDF, называется `sphinx.so` и должен автоматически собираться и устанавливаться в соответствующее место вместе с SphinxSE. Если по какой-то причине он не устанавливается автоматически, найдите `sphinx.so` в каталоге сборки и скопируйте его в каталог плагинов вашего экземпляра MySQL. После этого зарегистрируйте UDF с помощью следующего оператора:

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

Имя функции *должно* быть sphinx_snippets; нельзя использовать произвольное имя. Аргументы функции следующие:

**Прототип:** function sphinx_snippets ( document, table, words [, options] );

Аргументы document и words могут быть строками или столбцами таблицы. Опции должны указываться так: `'value' AS option_name`. Для списка поддерживаемых опций смотрите раздел [Highlighting](../Searching/Highlighting.md). Единственная дополнительная опция, специфичная для UDF, называется `sphinx` и позволяет указать расположение searchd (хост и порт).

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

