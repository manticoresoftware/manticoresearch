# Настройки поиска с подстановочными знаками

Поиск с подстановочными знаками (wildcard) — это распространенный тип текстового поиска. В Manticore он выполняется на уровне словаря. По умолчанию как обычные таблицы, так и RT-таблицы используют тип словаря под названием [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). В этом режиме слова хранятся как есть, поэтому включение поиска с подстановочными знаками не влияет на размер таблицы. При выполнении такого поиска словарь просматривается, чтобы найти все возможные варианты раскрытия слова с подстановочным знаком. Это раскрытие может быть проблематичным с точки зрения вычислительных затрат в момент выполнения запроса, когда раскрываемое слово дает много вариантов или варианты имеют огромные списки совпадений (hitlists), особенно в случае инфиксов, когда подстановочный знак добавляется в начало и конец слова. Чтобы избежать таких проблем, можно использовать [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

Эта настройка определяет минимальную длину префикса слова для индексации и поиска. По умолчанию установлено значение 0, что означает, что префиксы не разрешены.

Префиксы позволяют выполнять поиск с подстановочными знаками с помощью шаблонов вида `началослова*`.

Например, если слово "example" проиндексировано с min_prefix_len=3, его можно найти, выполнив поиск по "exa", "exam", "examp", "exampl", а также по полному слову.

Обратите внимание, что при использовании [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc параметр min_prefix_len повлияет на размер полнотекстового индекса, поскольку каждое раскрытие слова будет храниться дополнительно.

Manticore может различать точные совпадения слов и совпадения по префиксу и ранжировать первые выше, если выполняются следующие условия:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включено по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (выключено по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключено по умолчанию)

Обратите внимание, что в режиме [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любой из вышеуказанных опций невозможно различить префиксы и полные слова, и точные совпадения слов не могут быть ранжированы выше.

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
Настройка min_infix_len определяет минимальную длину инфиксного префикса для индексации и поиска. Она является необязательной, и ее значение по умолчанию равно 0, что означает, что инфиксы не разрешены. Минимально допустимое ненулевое значение — 2.

При включении инфиксы позволяют выполнять поиск с подстановочными знаками с шаблонами терминов, такими как `начало*`, `*конец`, `*середина*` и так далее. Это также позволяет отключать слишком короткие подстановочные знаки, если поиск по ним слишком затратен.

Если выполняются следующие условия, Manticore может различать точные совпадения слов и совпадения по инфиксу и ранжировать первые выше:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включено по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (выключено по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключено по умолчанию)

Обратите внимание, что в режиме [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любой из вышеуказанных опций нет возможности различить инфиксы и полные слова, и, следовательно, точные совпадения слов не могут быть ранжированы выше.

Время выполнения запроса с инфиксным поиском с подстановочными знаками может сильно варьироваться в зависимости от того, на сколько ключевых слов фактически раскроется подстрока. Короткие и частые слоги, такие как `*in*` или `*ti*`, могут раскрыться в слишком много ключевых слов, все из которых необходимо сопоставить и обработать. Поэтому для общего включения поиска по подстрокам обычно устанавливают min_infix_len равным 2. Чтобы ограничить влияние поиска с подстановочными знаками с слишком короткими шаблонами, можно установить большее значение.

Инфиксы должны быть длиной не менее 2 символов, и подстановочные знаки вида `*a*` не разрешены по соображениям производительности.

Когда min_infix_len установлен в положительное число, [минимальная длина префикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) считается равной 1. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) инфиксирование и префиксирование слов не могут быть включены одновременно. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и других полей, для которых префиксы объявлены с помощью [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), запрещено объявлять одно и то же поле в обоих списках.

Если dict=keywords, помимо подстановочного знака `*` можно использовать еще два других:
* `?` может соответствовать любому (одному) символу: `t?st` соответствует `test`, но не `teast`
* `%` может соответствовать нулю или одному символу: `tes%` соответствует `tes` или `test`, но не `testing`


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

Настройка prefix_fields используется для ограничения префиксного индексирования определенными полнотекстовыми полями в режиме [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). По умолчанию все поля индексируются в префиксном режиме, но поскольку префиксное индексирование может влиять как на производительность индексации, так и на производительность поиска, может потребоваться ограничить его определенными полями.

Чтобы ограничить префиксное индексирование определенными полями, используйте настройку prefix_fields, за которой следует список имен полей, разделенных запятыми. Если prefix_fields не задана, то все поля будут индексироваться в префиксном режиме.

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

Настройка infix_fields позволяет указать список полнотекстовых полей, чтобы ограничить инфиксное индексирование только ими. Это применимо только к [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и является необязательным; по умолчанию все поля индексируются в инфиксном режиме.
Эта настройка похожа на [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict), но вместо этого позволяет ограничить инфиксное индексирование определенными полями.


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

Директива max_substring_len устанавливает максимальную длину подстроки, которая будет индексироваться для префиксного или инфиксного поиска. Эта настройка необязательна, и её значение по умолчанию равно 0 (что означает, что индексируются все возможные подстроки). Она применяется только к [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict).

По умолчанию индексирование подстрок в [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) индексирует **все** возможные подстроки как отдельные ключевые слова, что может привести к чрезмерно большому полнотекстовому индексу. Поэтому директива max_substring_len позволяет пропускать слишком длинные подстроки, которые, вероятно, никогда не будут искаться.

Например, тестовая таблица из 10 000 записей блога занимает разный объем дискового пространства в зависимости от настроек:
* 6.4 МБ базовый (без подстрок)
* 24.3 МБ (в 3.8 раза больше) с min_prefix_len = 3
* 22.2 МБ (в 3.5 раза больше) с min_prefix_len = 3, max_substring_len = 8
* 19.3 МБ (в 3.0 раза больше) с min_prefix_len = 3, max_substring_len = 6
* 94.3 МБ (в 14.7 раза больше) с min_infix_len = 3
* 84.6 МБ (в 13.2 раза больше) с min_infix_len = 3, max_substring_len = 8
* 70.7 МБ (в 11.0 раза больше) с min_infix_len = 3, max_substring_len = 6

Таким образом, ограничение максимальной длины подстроки может сэкономить 10-15% размера таблицы.

При использовании режима dict=keywords нет влияния на производительность, связанного с длиной подстроки. Поэтому эта директива неприменима и намеренно запрещена в этом случае. Однако, если требуется, вы все равно можете ограничить длину подстроки, которую ищете, в коде приложения.


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

Эта настройка расширяет ключевые слова их точными формами и/или звездочками, когда это возможно. Поддерживаемые значения:
* 1 - расширять как до точной формы, так и до формы со звездочками. Например, `running` станет `(running | *running* | =running)`
* `exact` - дополнять ключевое слово только его точной формой. Например, `running` станет `(running | =running)`
* `star` - дополнять ключевое слово, добавляя `*` вокруг него. Например, `running` станет `(running | *running*)`
Эта настройка необязательна, и значение по умолчанию равно 0 (ключевые слова не расширяются).

Запросы к таблицам с включенной функцией `expand_keywords` внутренне расширяются следующим образом: если таблица была построена с включенным префиксным или инфиксным индексированием, каждое ключевое слово внутренне заменяется на дизъюнкцию самого ключевого слова и соответствующего префикса или инфикса (ключевое слово со звездочками). Если таблица была построена с включенными стеммингом и [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words), также добавляется точная форма.

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

Расширенные запросы, естественно, выполняются дольше, но могут улучшить качество поиска, поскольку документы с точными совпадениями форм должны ранжироваться, как правило, выше, чем документы со стеммированными или инфиксными совпадениями.

**Обратите внимание, что существующий синтаксис запросов не позволяет эмулировать этот тип расширения**, поскольку внутреннее расширение работает на уровне ключевых слов и расширяет ключевые слова даже внутри операторов фразы или кворума (что невозможно через синтаксис запросов). Взгляните на примеры и на то, как expand_keywords влияет на веса результатов поиска и как "runsy" находится по "runs" без необходимости добавлять звездочку:

<!-- intro -->
##### expand_keywords включен
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
##### expand_keywords отключен

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

Эта директива никак не влияет на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), она влияет только на [searchd](../../Starting_the_server/Manually.md).


## expansion_limit

```ini
expansion_limit = number
```

Максимальное количество расширенных ключевых слов для одного шаблона с подстановочными знаками. Подробнее см. [здесь](../../Server_settings/Searchd.md#expansion_limit).

<!-- proofread -->

