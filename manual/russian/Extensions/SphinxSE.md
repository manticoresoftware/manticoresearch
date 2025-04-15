# SphinxSE

SphinxSE — это механизм хранения MySQL, который можно скомпилировать в серверах MySQL/MariaDB с использованием их модульной архитектуры.

Несмотря на свое название, SphinxSE *не* хранит никаких данных само по себе. Вместо этого он служит встроенным клиентом, который позволяет серверу MySQL взаимодействовать с `searchd`, выполнять поисковые запросы и получать результаты поиска. Вся индексация и поиск происходят вне MySQL.

Некоторые распространенные приложения SphinxSE включают:
* Упрощение портирования приложений MySQL Full-Text Search (FTS) на Manticore;
* Обеспечение использования Manticore с языками программирования, для которых еще не доступны родные API;
* Предоставление оптимизаций, когда требуется дополнительная обработка набора результатов Manticore на стороне MySQL (например, JOIN-ы с оригинальными таблицами документов или дополнительная фильтрация на стороне MySQL).

## Установка SphinxSE

Вам нужно будет получить копию исходников MySQL, подготовить их, а затем перекомпилировать бинарный файл MySQL. Исходники MySQL (mysql-5.x.yy.tar.gz) можно получить с веб-сайта <http://dev.mysql.com>.

### Компиляция MySQL 5.0.x с SphinxSE

1. Скопируйте файл патча `sphinx.5.0.yy.diff` в директорию исходников MySQL и выполните команду
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
Если для конкретной версии, которую вы хотите построить, нет файла .diff, попробуйте применить .diff с ближайшими номерами версий. Важно, чтобы патч применился без отказов.
2. В директории исходников MySQL выполните команду
```bash
$ sh BUILD/autorun.sh
```
3. В директории исходников MySQL создайте директорию `sql/sphinx` и скопируйте все файлы из директории `mysqlse` из исходников Manticore туда. Например:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4. Настройте MySQL и включите новый движок:
```bash
$ ./configure --with-sphinx-storage-engine
```
5. Соберите и установите MySQL:
```bash
$ make
$ make install
```

### Компиляция MySQL 5.1.x с SphinxSE

1. В директории исходников MySQL создайте директорию `storage/sphinx` и скопируйте все файлы из директории `mysqlse` в исходниках Manticore в это новое место. Например:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2. В директории исходников MySQL выполните команду:
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

Чтобы проверить, что SphinxSE успешно скомпилирован в MySQL, запустите новый сервер, выполните клиент MySQL и запросите `SHOW ENGINES`. Вы должны увидеть список всех доступных движков. Manticore должен присутствовать, а столбец "Support" должен отображать "YES":

<!-- request -->

``` sql
mysql> show engines;
```

<!-- response -->
```sql
+------------+----------+-------------------------------------------------------------+
| Engine     | Support  | Comment                                                     |
+------------+----------+-------------------------------------------------------------+
| MyISAM     | DEFAULT  | Дефолтный движок на MySQL 3.23 с отличной производительностью      |
  ...
| SPHINX     | YES      | Движок хранения Manticore                                       |
  ...
+------------+----------+-------------------------------------------------------------+
13 rows in set (0.00 sec)
```

<!-- end -->

## Использование SphinxSE

Чтобы выполнить поиск с использованием SphinxSE, вам нужно создать специальную "поисковую таблицу" ENGINE=SPHINX и затем использовать оператор `SELECT` с полным текстовым запросом, размещенным в операторе `WHERE` для столбца запроса.

Вот пример оператора создания и запроса поиска:

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

В поисковой таблице первые три столбца *должны* иметь следующие типы: `INTEGER UNSIGNED` или `BIGINT` для 1-го столбца (ID документа), `INTEGER` или `BIGINT` для 2-го столбца (вес совпадения) и `VARCHAR` или `TEXT` для 3-го столбца (ваш запрос). Это сопоставление фиксировано; вы не можете опустить любой из этих трех обязательных столбцов, перемещать их или изменять их типы. Кроме того, столбец запроса должен быть индексирован, в то время как все остальные должны оставаться неиндексированными. Имена столбцов игнорируются, поэтому вы можете использовать любые произвольные названия.

Дополнительные столбцы должны быть либо `INTEGER`, `TIMESTAMP`, `BIGINT`, `VARCHAR` или `FLOAT`. Они будут привязаны к атрибутам, предоставленным в результате Manticore, по имени, поэтому их имена должны совпадать с именами атрибутов, указанными в `sphinx.conf`. Если в результатах поиска Manticore нет совпадающего имени атрибута, столбец будет иметь значения `NULL`.

Специальные "виртуальные" имена атрибутов также могут быть связаны со столбцами SphinxSE. Используйте `_sph_` вместо `@` для этой цели. Например, чтобы получить значения виртуальных атрибутов `@groupby`, `@count` или `@distinct`, используйте имена столбцов `_sph_groupby`, `_sph_count` или `_sph_distinct`, соответственно.

Параметр строки `CONNECTION` используется для указания хоста Manticore, порта и таблицы. Если строка соединения не указана в `CREATE TABLE`, предполагается имя таблицы `*` (т.е. поиск по всем таблицам) и `localhost:9312`. Синтаксис строки соединения следующий:

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

Вы можете изменить строку соединения по умолчанию позже:

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

Вы также можете переопределить эти параметры на уровне отдельного запроса.
Как показано в примере, как текст запроса, так и параметры поиска должны быть размещены в условии `WHERE` в колонне запроса поиска (т.е. в 3-й колонне). Параметры разделяются точкой с запятой, а их имена от значений - знаком равенства. Можно указать любое количество параметров. Доступные параметры:

* query - текст запроса;
* mode - режим совпадения. Должен быть одним из "all", "any", "phrase", "boolean" или "extended". По умолчанию - "all";
* sort - режим сортировки совпадений. Должен быть одним из "relevance", "attr_desc", "attr_asc", "time_segments" или "extended". Во всех режимах, кроме "relevance", также требуется имя атрибута (или клаузула сортировки для "extended") после двоеточия:
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset - смещение в наборе результатов; по умолчанию - 0;
* limit - количество совпадений для извлечения из набора результатов; по умолчанию - 20;
* index - имена таблиц для поиска:
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid - минимальный и максимальный идентификатор документа для совпадения;
* weights - список весов, разделенный запятыми, которые будут назначены полям полного текста Manticore:
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter - имя атрибута, разделенное запятыми, и набор значений для совпадения:
```sql
# включать только группы 1, 5 и 19
... WHERE query='test;filter=group_id,1,5,19;';
# исключить группы 3 и 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range - имя атрибута Manticore, разделенное запятыми (целое число или большой целый), и минимальные и максимальные значения для совпадения:
```sql
# включать группы с 3 по 7, включительно
... WHERE query='test;range=group_id,3,7;';
# исключить группы с 5 по 25
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange - имя атрибута Manticore, разделенное запятыми (число с плавающей запятой), и минимальные и максимальные значения для совпадения:
```sql
# фильтровать по размеру числа с плавающей запятой
... WHERE query='test;floatrange=size,2,3;';
# выбрать все результаты в пределах 1000 метров от геоантенны
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches - значение максимального количества совпадений для запроса, как в [max_matches search option](../Searching/Options.md#max_matches):
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff - максимальное разрешенное количество совпадений, как в [cutoff search option](../Searching/Options.md#cutoff):
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - максимальное разрешенное время запроса (в миллисекундах), как в [max_query_time search option](../Searching/Options.md#max_query_time):
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - функция группировки и атрибут. Читайте [это](../Searching/Grouping.md#Just-Grouping) о группировке результатов поиска:
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - клаузула сортировки по группам:
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - атрибут для вычисления [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT%28DISTINCT-field%29) для режима группировки:
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - список имен таблиц и весов, разделенный запятыми, которые будут использоваться при поиске через несколько таблиц:
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - список весов по каждому полю, разделенный запятыми, которые могут быть использованы ранкером:
```sql
... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';
```
* comment - строка для пометки этого запроса в журнале запросов, как в [comment search option](../Searching/Options.md#comment):
```sql
... WHERE query='test;comment=marker001;';
```
* select - строка с выражениями для вычисления:
```sql
... WHERE query='test;select=2*a+3*** as myexpr;';
```
* host, port - удаленное имя хоста `searchd` и TCP порт соответственно:
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - функция ранжирования для использования с режимом "расширенного" совпадения, как в [ranker](../Searching/Options.md#ranker). Известные значения: "proximity_bm25", "bm25", "none", "wordcount", "proximity", "matchany", "fieldmask", "sph04", "expr:EXPRESSION" синтаксис для поддержки ранжировщика, основанного на выражении (где EXPRESSION должна быть заменена на вашу конкретную формулу ранжирования), и "export:EXPRESSION":
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
Функция ранжирования "export" работает аналогично ranker=expr, но сохраняет значения факторов для каждого документа, в то время как ranker=expr отбрасывает их после вычисления окончательного значения `WEIGHT()`. Имейте в виду, что ranker=export предназначен для случайного использования, например, для обучения функции машинного обучения (ML) или вручную определения вашей собственной функции ранжирования и не должен использоваться в реальной эксплуатации. При использовании этого ранжировщика вы, скорее всего, захотите изучить вывод функции `RANKFACTORS()`, которая генерирует строку, содержащую все факторные значения на уровне полей для каждого документа.

<!-- пример SQL Example_2 -->
<!-- запрос -->

``` sql
SELECT *, WEIGHT(), RANKFACTORS()
    FROM myindex
    WHERE MATCH('dog')
    OPTION ranker=export('100*bm25');
```

<!-- ответ -->

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

* geoanchor - географическое расстояние якорь. Узнайте больше о Гео-поиске [в этом разделе](../Searching/Geo_search.md). Принимает 4 параметра, которые являются названиями атрибутов широты и долготы, а также соответственно координатами точки привязки:
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

Одно **очень важное** замечание заключается в том, что **гораздо** более эффективно позволить Manticore обрабатывать сортировку, фильтрацию и нарезку набора результатов, чем увеличивать максимальное количество совпадений и использовать предложения `WHERE`, `ORDER BY` и `LIMIT` на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore использует различные оптимизации и выполняет эти задачи лучше, чем MySQL. Во-вторых, для передачи данных требуется меньше данных, которые должны быть упакованы searchd, переданы и распакованы SphinxSE.


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

Вы также можете получить доступ к этой информации через переменные состояния. Имейте в виду, что использование этого метода не требует привилегий суперпользователя.

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

## Создание сниппетов с помощью MySQL


SphinxSE также имеет функцию UDF, которая позволяет вам создавать сниппеты с использованием MySQL. Эта функциональность аналогична [HIGHLIGHT()](../../Searching/Highlighting.md#Highlighting), но может быть доступна через MySQL+SphinxSE.

Бинарный файл, предоставляющий UDF, называется `sphinx.so` и должен быть автоматически создан и установлен в соответствующее место вместе с SphinxSE. Если по какой-то причине он не устанавливается автоматически, найдите `sphinx.so` в директории сборки и скопируйте его в директорию плагинов вашего экземпляра MySQL. После этого зарегистрируйте UDF с помощью следующего оператора:

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

Имя функции *должно* быть sphinx_snippets; вы не можете использовать произвольное имя. Аргументы функции следующие:

**Прототип:** function sphinx_snippets ( document, table, words [, options] );

Аргументы document и words могут быть либо строками, либо столбцами таблицы. Параметры должны быть указаны таким образом: `'value' AS option_name`. Для получения списка поддерживаемых опций смотрите раздел [Подсветка](../Searching/Highlighting.md). Единственная специфическая для UDF дополнительная опция называется `sphinx` и позволяет вам указать расположение searchd (хост и порт).

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

