# Настройки поиска с подстановочными знаками

Поиск с подстановочными знаками — это распространённый тип текстового поиска. В Manticore он выполняется на уровне словаря. По умолчанию как обычные, так и RT-таблицы используют тип словаря, называемый [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). В этом режиме слова хранятся в исходном виде, поэтому включение подстановочных знаков не влияет на размер таблицы. При выполнении поиска с подстановочными знаками словарь просматривается для нахождения всех возможных расширений слова с подстановочными знаками. Такое расширение может быть проблематичным с точки зрения вычислений во время запроса, если расширенное слово даёт много вариантов или варианты с огромными списками попаданий, особенно в случае инфиксов, когда подстановочный знак добавляется в начало и конец слова. Чтобы избежать таких проблем, можно использовать [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

Этот параметр определяет минимальную длину префикса слова для индексации и поиска. По умолчанию он установлен в 0, что означает, что префиксы не разрешены.

Префиксы позволяют выполнять поиск с подстановочными знаками вида `wordstart*`.

Например, если слово "example" индексируется с min_prefix_len=3, его можно найти, выполнив поиск по "exa", "exam", "examp", "exampl", а также по полному слову.

Обратите внимание, что при использовании [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc min_prefix_len повлияет на размер полнотекстового индекса, так как каждое расширение слова будет храниться дополнительно.

Manticore может различать точные совпадения слов и совпадения по префиксу и ранжировать первые выше, если выполнены следующие условия:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включено по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (выключено по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключено по умолчанию)

Обратите внимание, что при использовании режима [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любого из вышеуказанных параметров невозможно различать префиксы и полные слова, и точные совпадения слов не могут быть ранжированы выше.

Когда [минимальная длина инфикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) установлена в положительное число, минимальная длина префикса всегда считается равной 1.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_prefix_len = '3'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) min_prefix_len = '3'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_prefix_len' => '3'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  min_prefix_len = 3

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## min_infix_len


<!-- example min_infix_len -->

```ini
min_infix_len = length
```
Параметр min_infix_len определяет минимальную длину префикса инфикса для индексации и поиска. Он является необязательным, и его значение по умолчанию равно 0, что означает, что инфиксы не разрешены. Минимально допустимое ненулевое значение — 2.

При включении инфиксы позволяют выполнять поиск с подстановочными знаками с шаблонами терминов, такими как `start*`, `*end`, `*middle*` и так далее. Это также позволяет отключить слишком короткие подстановочные знаки, если их поиск слишком затратен.

Если выполнены следующие условия, Manticore может различать точные совпадения слов и совпадения по инфиксу и ранжировать первые выше:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включено по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (выключено по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключено по умолчанию)

Обратите внимание, что при использовании режима [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любого из вышеуказанных параметров невозможно различать инфиксы и полные слова, и, следовательно, точные совпадения слов не могут быть ранжированы выше.

Время выполнения запроса поиска с подстановочными знаками в инфиксах может сильно варьироваться в зависимости от того, на сколько ключевых слов будет расширяться подстрока. Короткие и частые слоги, такие как `*in*` или `*ti*`, могут расширяться до слишком большого количества ключевых слов, все из которых нужно будет сопоставить и обработать. Поэтому для общего включения поиска подстрок обычно устанавливают min_infix_len в 2. Чтобы ограничить влияние поиска с подстановочными знаками с слишком короткими подстановочными знаками, можно установить его выше.

Инфиксы должны быть длиной не менее 2 символов, и подстановочные знаки вида `*a*` не разрешены по причинам производительности.

Когда min_infix_len установлен в положительное число, [минимальная длина префикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) считается равной 1. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) нельзя одновременно включать инфиксы и префиксы. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и других полей, чтобы иметь префиксы, объявленные с помощью [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), запрещено объявлять одно и то же поле в обоих списках.

Если dict=keywords, кроме подстановочного знака `*`, можно использовать ещё два символа подстановки:
* `?` может соответствовать любому (одному) символу: `t?st` совпадёт с `test`, но не с `teast`
* `%` может соответствовать нулю или одному символу: `tes%` совпадёт с `tes` или `test`, но не с `testing`


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_infix_len = '3'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) min_infix_len = '3'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_infix_len' => '3'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_infix_len = '3'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_infix_len = '3'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_infix_len = '3'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  min_infix_len = 3

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## prefix_fields

<!-- example prefix_fields -->

```ini
prefix_fields = field1[, field2, ...]
```

Настройка prefix_fields используется для ограничения индексирования префиксов конкретными полнотекстовыми полями в режиме [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). По умолчанию все поля индексируются в режиме префикса, но поскольку индексирование префиксов может влиять как на производительность индексирования, так и на поиск, может потребоваться ограничить его определёнными полями.

Чтобы ограничить индексирование префиксов конкретными полями, используйте настройку prefix_fields с последующим списком имён полей, разделённых запятыми. Если prefix_fields не задан, то все поля будут индексироваться в режиме префикса.

<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
table products {
  prefix_fields = title, name
  min_prefix_len = 3
  dict = crc
```
<!-- end -->

## infix_fields

<!-- example infix_fields -->

```ini
infix_fields = field1[, field2, ...]
```

Настройка infix_fields позволяет указать список полнотекстовых полей, для которых будет ограничено индексирование инфиксов. Это применяется только к режиму [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и является необязательным, по умолчанию индексируются все поля в режиме инфикса.
Эта настройка похожа на [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict), но позволяет ограничить индексирование инфиксов конкретными полями.


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
table products {
  infix_fields = title, name
  min_infix_len = 3
  dict = crc
```
<!-- end -->

## max_substring_len

<!-- example max_substring_len -->

```ini
max_substring_len = length
```

Директива max_substring_len задаёт максимальную длину подстроки, которая будет индексироваться для поиска по префиксу или инфиксу. Эта настройка необязательна, и её значение по умолчанию равно 0 (что означает, что индексируются все возможные подстроки). Она применяется только к режиму [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict).

По умолчанию индексирование подстрок в режиме [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) индексирует **все** возможные подстроки как отдельные ключевые слова, что может привести к чрезмерно большому полнотекстовому индексу. Поэтому директива max_substring_len позволяет пропускать слишком длинные подстроки, которые, вероятно, никогда не будут искаться.

Например, тестовая таблица из 10 000 блог-постов занимает разное количество дискового пространства в зависимости от настроек:
* 6.4 МБ базовый уровень (без подстрок)
* 24.3 МБ (3.8x) с min_prefix_len = 3
* 22.2 МБ (3.5x) с min_prefix_len = 3, max_substring_len = 8
* 19.3 МБ (3.0x) с min_prefix_len = 3, max_substring_len = 6
* 94.3 МБ (14.7x) с min_infix_len = 3
* 84.6 МБ (13.2x) с min_infix_len = 3, max_substring_len = 8
* 70.7 МБ (11.0x) с min_infix_len = 3, max_substring_len = 6

Таким образом, ограничение максимальной длины подстроки может сэкономить 10-15% размера таблицы.

При использовании режима dict=keywords влияние длины подстроки на производительность отсутствует. Поэтому эта директива не применяется и намеренно запрещена в этом случае. Однако при необходимости вы всё равно можете ограничить длину подстроки, которую ищете, в коде приложения.


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
table products {
  max_substring_len = 12
  min_infix_len = 3
  dict = crc
```
<!-- end -->

## expand_keywords

<!-- example expand_keywords -->

```ini
expand_keywords = {0|1|exact|star}
```

Эта настройка расширяет ключевые слова их точными формами и/или со звёздочками, когда это возможно. Поддерживаемые значения:
* 1 - расширять и точной формой, и формой со звёздочками. Например, `running` станет `(running | *running* | =running)`
* `exact` - расширять ключевое слово только точной формой. Например, `running` станет `(running | =running)`
* `star` - расширять ключевое слово, добавляя `*` вокруг него. Например, `running` станет `(running | *running*)`
Эта настройка необязательна, значение по умолчанию — 0 (ключевые слова не расширяются).

Запросы к таблицам с включённой функцией `expand_keywords` внутренне расширяются следующим образом: если таблица была построена с включённым индексированием префиксов или инфиксов, каждое ключевое слово внутренне заменяется дизъюнкцией самого ключевого слова и соответствующего префикса или инфикса (ключевое слово со звёздочками). Если таблица была построена с включённым стеммингом и [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words), также добавляется точная форма.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) expand_keywords = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) expand_keywords = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'expand_keywords' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) expand_keywords = '1'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) expand_keywords = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) expand_keywords = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  expand_keywords = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example expand_keywords2 -->

Расширенные запросы, естественно, выполняются дольше, но могут улучшить качество поиска, так как документы с точными совпадениями форм обычно ранжируются выше, чем документы с совпадениями по стеммам или инфиксам.

**Обратите внимание, что существующий синтаксис запросов не позволяет эмулировать такое расширение**, поскольку внутреннее расширение работает на уровне ключевых слов и расширяет ключевые слова внутри операторов фраз и кворума тоже (что невозможно через синтаксис запросов). Посмотрите на примеры и как expand_keywords влияет на веса результатов поиска и как "runsy" находится по запросу "runs" без необходимости добавлять звёздочку:

<!-- intro -->
##### expand_keywords включён
<!-- request expand_keywords_enabled -->
```sql
mysql> create table t(f text) min_infix_len='2' expand_keywords='1' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('runs');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    2 | runs    |     1560 |
|    1 | running |     1500 |
|    3 | runsy   |     1500 |
+------+---------+----------+
3 rows in set (0.01 sec)

mysql> drop table t;
Query OK, 0 rows affected (0.01 sec)

mysql> create table t(f text) min_infix_len='2' expand_keywords='exact' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('running');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1590 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 rows in set (0.00 sec)
```

<!-- intro -->
##### expand_keywords отключён

<!-- request expand_keywords_disabled -->

```sql
mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('runs');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1500 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 rows in set (0.00 sec)

mysql> drop table t;
Query OK, 0 rows affected (0.01 sec)

mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('running');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1500 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 rows in set (0.00 sec)
```
<!-- end -->

This directive does not affect [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) in any way, it only affects [searchd](../../Starting_the_server/Manually.md).


## expansion_limit

```ini
expansion_limit = number
```

Максимальное количество расширенных ключевых слов для одного шаблона с подстановочными знаками. Подробнее [здесь](../../Server_settings/Searchd.md#expansion_limit).

<!-- proofread -->

