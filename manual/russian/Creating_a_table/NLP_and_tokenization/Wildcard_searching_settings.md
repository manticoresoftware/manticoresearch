# Настройки поиска по шаблону

Поиск по шаблону - это распространенный тип текстового поиска. В Manticore он выполняется на уровне словаря. По умолчанию как обычные, так и RT таблицы используют тип словаря, называемый [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). В этом режиме слова хранятся так, как есть, поэтому включение поиска по шаблону не влияет на размер таблицы. Когда выполняется поиск с подстановочным знаком, происходит поиск в словаре для нахождения всех возможных расширений слова с подстановочным знаком. Это расширение может быть проблематичным с точки зрения вычислений во время запроса, когда расширенное слово предоставляет много расширений или расширений, которые имеют огромные списки совпадений, особенно в случае инфиксов, когда подстановочный знак добавляется в начале и в конце слова. Чтобы избежать таких проблем, можно использовать [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

## min_prefix_len

<!-- пример min_prefix_len -->

```ini
min_prefix_len = length
```

Эта настройка определяет минимальную длину префикса слова для индексации и поиска. По умолчанию она установлена на 0, что означает, что префиксы не допускаются.

Префиксы позволяют выполнять поиск по шаблону с использованием подстановочных знаков `wordstart*`.

Например, если слово "example" индексируется с min_prefix_len=3, его можно найти, осуществляя поиск по "exa", "exam", "examp", "exampl", а также по полному слову.

Обратите внимание, что при использовании [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc min_prefix_len будет влиять на размер полнотекстового индекса, так как каждое расширение слова будет храниться дополнительно.

Manticore может различать точные совпадения слов и совпадения по префиксам и ранжировать последние выше, если выполнены следующие условия:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (по умолчанию включено)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (по умолчанию выключено),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также по умолчанию выключено)

Обратите внимание, что в режиме [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любой из вышеуказанных опций невозможно различить префиксы и полные слова, и точные совпадения слов не могут быть оценены выше.

Когда [минимальная длина инфикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) установлена на положительное число, минимальная длина префикса всегда считается равной 1.


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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = '3'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = '3'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = '3'');
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


<!-- пример min_infix_len -->

```ini
min_infix_len = length
```
Настройка min_infix_len определяет минимальную длину инфикса для индексации и поиска. Она является необязательной, а её значение по умолчанию равно 0, что означает, что инфиксы не допускаются. Минимально допустимое ненулевое значение равно 2.

Когда включено, инфиксы позволяют выполнять поиск по шаблону с такими терминами, как `start*`, `*end`, `*middle*` и так далее. Это также позволяет отключать слишком короткие подстановочные знаки, если их поиск слишком затратен.

Если выполнены следующие условия, Manticore может различать точные совпадения слов и совпадения по инфиксам и ранжировать последние выше:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (по умолчанию включено)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (по умолчанию выключено),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также по умолчанию выключено)

Обратите внимание, что в режиме [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любой из вышеуказанных опций нет возможности различить инфиксы и полные слова, и, следовательно, точные совпадения слов не могут быть оценены выше.
Время поиска по шаблону с подстрокой может сильно варьироваться в зависимости от того, на сколько ключевых слов на самом деле развернется подстрока. Короткие и частые слоги, такие как `*in*` или `*ti*`, могут развернуться в слишком большое количество ключевых слов, все из которых нужно будет сопоставить и обработать. Поэтому, чтобы в общем позволить поиск подстрок, следует установить min_infix_len на 2. Чтобы ограничить влияние поиска по шаблону с слишком короткими шаблонами, вы можете установить его выше.

Подстроки должны быть длиной не менее 2 символов, и шаблоны, такие как `*a*`, не допускаются по соображениям производительности.

Когда min_infix_len установлен на положительное число, [минимальная длина префикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) считается равной 1. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) слова нельзя одновременно включать и инкапсуляцию, и префиксацию. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и других полей, чтобы префиксы были объявлены с помощью [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), запрещено объявлять одно и то же поле в обоих списках.

Если dict=keywords, помимо шаблона `*` могут быть использованы два других символа шаблона:
* `?` может соответствовать любому (одному) символу:  `t?st` подойдет к `test`, но не к `teast`
* `%` может соответствовать нулю или одному символу:  `tes%` подойдет к `tes` или `test`, но не к `testing`


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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = '3'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = '3'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = '3'');
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

Настройка prefix_fields используется для ограничения индексации префиксов для определенных полных текстовых полей в режиме [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). По умолчанию все поля индексируются в режиме префиксов, но так как индексация префиксов может повлиять как на производительность индексации, так и на производительность поиска, может быть желаемо ограничить ее определенными полями.

Чтобы ограничить индексацию префиксов для конкретных полей, используйте настройку prefix_fields, за которой следует список имен полей, разделенных запятыми. Если prefix_fields не установлено, то все поля будут индексироваться в режиме префиксов.

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

Настройка infix_fields позволяет вам указать список полных текстовых полей, чтобы ограничить индексацию инкапсуляции. Это применяется только к [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и является необязательным, с тем, что по умолчанию индексируются все поля в режиме инкапсуляции.
Эта настройка аналогична [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict), но вместо этого позволяет вам ограничить индексацию инкапсуляции для определенных полей.


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

Директива max_substring_len устанавливает максимальную длину подстроки, которая будет индексироваться для поиска по префиксу или инкапсуляции. Эта настройка необязательна, и ее значение по умолчанию равно 0 (что означает, что все возможные подстроки индексируются). Она применяется только к [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict).

По умолчанию индексация подстрок в [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) индексирует  **все** возможные подстроки как отдельные ключевые слова, что может привести к бешеному размеру полного текстового индекса. Поэтому директива max_substring_len позволяет вам пропустить слишком длинные подстроки, по которым, вероятно, никогда не будет выполнен поиск.

Например, тестовая таблица из 10 000 записей блогов занимает разное количество места на диске в зависимости от настроек:
* 6.4 МБ базовая (без подстрок)
* 24.3 МБ (3.8x) с min_prefix_len = 3
* 22.2 MB (3.5x) с min_prefix_len = 3, max_substring_len = 8
* 19.3 MB (3.0x) с min_prefix_len = 3, max_substring_len = 6
* 94.3 MB (14.7x) с min_infix_len = 3
* 84.6 MB (13.2x) с min_infix_len = 3, max_substring_len = 8
* 70.7 MB (11.0x) с min_infix_len = 3, max_substring_len = 6

Поэтому ограничение максимальной длины подстроки может сэкономить 10-15% размера таблицы.

При использовании режима dict=keywords нет влияния на производительность, связанного с длиной подстроки. Поэтому этот директив не применим и намеренно запрещен в этом случае. Однако, если требуется, вы все равно можете ограничить длину подстроки, которую вы ищете в коде приложения.


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

Эта настройка расширяет ключевые слова их точными формами и/или с помощью звездочек, когда это возможно. Поддерживаемые значения:
* 1 - расширить как точную форму, так и форму со звездочками. Например, `running` станет `(running | *running* | =running)`
* `exact` - дополнить ключевое слово только его точной формой. Например, `running` станет `(running | =running)`
* `star` - дополнить ключевое слово, добавив `*` вокруг него. Например, `running` станет `(running | *running*)`
Эта настройка является необязательной, а значение по умолчанию равно 0 (ключевые слова не расширяются).

Запросы к таблицам с включенной функцией `expand_keywords` внутренне расширяются следующим образом: если таблица была построена с учетом включенного индексирования префикса или инфикса, каждое ключевое слово заменяется внутренне на дизъюнкцию самого ключевого слова и соответствующего префикса или инфикса (ключевое слово со звёздочками). Если таблица была построена с включенным как стеммингом, так и [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words), точная форма также добавляется.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = '1'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = '1'');
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

Расширенные запросы занимают, естественно, больше времени на выполнение, но могут улучшить качество поиска, так как документы с точными совпадениями форм должны оценки в целом выше, чем документы со стеммингом или инфиксными совпадениями.

**Обратите внимание, что существующий синтаксис запроса не позволяет эмулировать такого рода расширение**, потому что внутреннее расширение работает на уровне ключевых слов и расширяет ключевые слова внутри операторов фраз или кворумов тоже (что невозможно сделать через синтаксис запроса). Посмотрите на примеры и как expand_keywords влияет на веса результатов поиска и как "runsy" находится через "runs" без необходимости добавления звезды:

<!-- intro -->
##### expand_keywords включено
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
##### expand_keywords отключено

<!-- request expand_keywords_disabled -->

```sql
mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
Запрос выполнен, 0 затронутых строк, 1 предупреждение (0.00 сек)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Запрос выполнен, 3 затронутых строк (0.00 сек)

mysql> select *, weight() from t where match('runs');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1500 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 строки в наборе (0.00 сек)

mysql> drop table t;
Запрос выполнен, 0 затронутых строк (0.01 сек)

mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
Запрос выполнен, 0 затронутых строк, 1 предупреждение (0.00 сек)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Запрос выполнен, 3 затронутых строк (0.00 сек)

mysql> select *, weight() from t where match('running');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1500 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 строки в наборе (0.00 сек)
```
<!-- end -->

Это директива не влияет на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) любым образом, она только влияет на [searchd](../../Starting_the_server/Manually.md).


## expansion_limit

```ini
expansion_limit = number
```

Максимальное количество расширенных ключевых слов для одного подстановочного знака. Подробности [здесь](../../Server_settings/Searchd.md#expansion_limit).

<!-- proofread -->

# Настройки поиска по подстановочным знакам

Поиск по подстановочным знакам - это распространенный тип текстового поиска. В Manticore он выполняется на уровне словаря. По умолчанию как обычные, так и RT таблицы используют тип словаря, называемый [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). В этом режиме слова хранятся как есть, поэтому включение подстановочного знака не влияет на размер таблицы. Когда выполняется поиск по подстановочным знакам, словарь просматривается для поиска всех возможных расширений подстановочного слова. Это расширение может быть проблематичным с точки зрения вычислений во время запроса, когда расширенное слово предоставляет множество расширений или расширений с огромными списками совпадений, особенно в случае инфиксов, когда подстановочный знак добавляется в начало и конец слова. Чтобы избежать подобных проблем, можно использовать [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

Эта настройка определяет минимальную длину префикса слова для индексирования и поиска. По умолчанию он установлен в 0, что означает, что префиксы не допускаются.

Префиксы позволяют выполнять поиск по подстановочным знакам с помощью подстановочных знаков `wordstart*`.

Например, если слово "example" индексируется с min_prefix_len=3, его можно найти, выполнив поиск по "exa", "exam", "examp", "exampl", а также по полному слову.

Обратите внимание, что с [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc min_prefix_len будет влиять на размер полнотекстового индекса, поскольку каждое расширение слова будет храниться дополнительно.

Manticore может различать идеальные совпадения слов с префиксами и оценивать последние выше, если выполнены следующие условия:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включен по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (выключен по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключен по умолчанию)

Обратите внимание, что с любым из вышеупомянутых опций, выключенных или с [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc режимом, невозможно дифференцировать префиксы и полные слова, и идеальные совпадения слов не могут быть оценены выше.

Когда [минимальная длина инфикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) установлена на положительное число, минимальная длина префикса всегда считается равной 1.


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'");
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
Настройка min_infix_len определяет минимальную длину инфикс-префикса для индексирования и поиска. Она является необязательной, и ее значение по умолчанию равно 0, что означает, что инфиксы не допускаются. Минимально допустимое ненулевое значение равно 2.

Когда включено, инфиксы позволяют выполнять поиск по подстановочным знакам с паттернами термина, такими как `start*`, `*end`, `*middle*` и так далее. Они также позволяют отключить слишком короткие подстановочные знаки, если они слишком затратны для поиска.

Если выполнены следующие условия, Manticore может различать идеальные совпадения слов и совпадения по инфиксам и оценивать последние выше:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включен по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (выключен по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключен по умолчанию)

Обратите внимание, что с режимом [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или с любой из вышеупомянутых опций, выключенных, невозможно различать инфиксы и полные слова, и, следовательно, идеальные совпадения слов не могут быть оценены выше.

Время выполнения запроса поиска по инфиксам может сильно варьироваться в зависимости от того, на сколько ключевых слов на самом деле расширится подстрока. Короткие и частые слоги, такие как `*in*` или `*ti*`, могут расшириться до слишком множества ключевых слов, и все они должны быть сопоставлены и обработаны. Поэтому, чтобы в общем включить поиски по подстроке, вам следует установить min_infix_len на 2. Чтобы ограничить влияние поиска по подстановочным знакам с слишком короткими подстановочными знаками, вы можете установить его на более высокое значение.

Инфиксы должны быть не менее 2 символов в длину, и подстановочные знаки, такие как `*a*`, не допускаются по соображениям производительности.
Когда min_infix_len установлен на положительное число, [минимальная длина префикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) считается 1. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) невозможно одновременно включить как инфиксацию, так и префиксацию. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и других полей с префиксами, объявленными с помощью [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), запрещено объявлять одно и то же поле в обоих списках.

Если dict=keywords, кроме подстановочного знака `*` можно использовать два других подстановочных символа:
* `?` может соответствовать любому (одному) символу: `t?st` будет соответствовать `test`, но не `teast`
* `%` может соответствовать нулю или одному символу: `tes%` будет соответствовать `tes` или `test`, но не `testing`


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_infix_len = '3'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_infix_len = '3'");
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

Настройка prefix_fields используется для ограничения индексирования префиксов до определенных полнотекстовых полей в режиме [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). По умолчанию все поля индексируются в режиме префиксов, но поскольку индексирование префиксов может влиять как на производительность индексации, так и на производительность поиска, может быть полезно ограничить его определенными полями.

Чтобы ограничить индексирование префиксов конкретными полями, используйте настройку prefix_fields, за которой следует список имен полей, разделенных запятыми. Если prefix_fields не установлен, все поля будут индексироваться в режиме префиксов.

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

Настройка infix_fields позволяет указать список полнотекстовых полей, чтобы ограничить индексирование инфиксов. Это относится только к [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и является необязательным, при этом по умолчанию все поля индексируются в режиме инфиксов.
Эта настройка аналогична [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict), но вместо этого позволяет ограничить индексирование инфиксов до определенных полей.


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

Директива max_substring_len устанавливает максимальную длину подстроки, которая будет индексироваться для поиска по префиксам или инфиксам. Эта настройка является необязательной, и ее значение по умолчанию равно 0 (что означает, что индексируются все возможные подстроки). Она применяется только к [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict).

По умолчанию индексирование подстрок в [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) индексирует **все** возможные подстроки как отдельные ключевые слова, что может привести к чрезмерно большому полнотекстовому индексу. Поэтому директива max_substring_len позволяет пропустить слишком длинные подстроки, которые, вероятно, никогда не будут запрашиваться.

Например, тестовая таблица из 10 000 блог-постов занимает разное количество дискового пространства в зависимости от настроек:
* 6.4 МБ базовый уровень (без подстрок)
* 24.3 МБ (3.8x) с min_prefix_len = 3
* 22.2 МБ (3.5x) с min_prefix_len = 3, max_substring_len = 8
* 19.3 МБ (3.0x) с min_prefix_len = 3, max_substring_len = 6
* 94.3 МБ (14.7x) с min_infix_len = 3
* 84.6 МБ (13.2x) с min_infix_len = 3, max_substring_len = 8
* 70.7 МБ (11.0x) с min_infix_len = 3, max_substring_len = 6

Поэтому ограничение максимальной длины подстроки может сэкономить 10-15% размера таблицы.

Когда используется режим dict=keywords, нет никакого влияния на производительность, связанного с длиной подстроки. Поэтому эта директива не применима и намеренно запрещена в этом случае. Однако, если необходимо, вы все равно можете ограничить длину подстроки, которую ищете в коде приложения.


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

Эта настройка расширяет ключевые слова их точными формами и/или звездами, когда это возможно. Поддерживаемые значения:
* 1 -  расширить до точной формы и формы со звездами. Например, `running` станет `(running | *running* | =running)`
* `exact` - - дополнить ключевое слово только его точной формой. Например, `running` станет `(running | =running)`
* `star` - дополнить ключевое слово, добавив `*` вокруг него. Например, `running` станет `(running | *running*)`
Эта настройка является необязательной, и значение по умолчанию равно 0 (ключевые слова не расширяются).

Запросы к таблицам с включенной функцией `expand_keywords` внутренне расширяются следующим образом: если таблица была построена с включенным префиксным или инфиксным индексированием, каждое ключевое слово внутренне заменяется на дизъюнкцию самого ключевого слова и соответствующего префикса или инфикса (ключевое слово со звездами). Если таблица была построена с включенной как стеммингом, так и [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words), точная форма также добавляется.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) expand_keywords = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) expand_keywords = '1'");
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

Расширенные запросы, как правило, требуют больше времени для завершения, но могут улучшить качество поиска, так как документы с точными совпадениями формы должны оцениваться выше, чем документы с совпадениями по стеммингам или инфиксам.

**Обратите внимание, что существующий синтаксис запроса не позволяет эмулировать такой тип расширения**, потому что внутреннее расширение работает на уровне ключевых слов и расширяет ключевые слова внутри операторов фразы или кворума тоже (что невозможно через синтаксис запроса). Посмотрите на примеры и как expand_keywords влияет на веса результатов поиска и как "runsy" находится с помощью "runs" без необходимости добавлять звезду:

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

Эта директива не влияет на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) никаким образом, она влияет только на [searchd](../../Starting_the_server/Manually.md).
## expansion_limit

```ini
expansion_limit = number
```

Максимальное количество расширенных ключевых слов для одного подстановочного знака. Подробности см. [здесь](../../Server_settings/Searchd.md#expansion_limit).

<!-- proofread -->



