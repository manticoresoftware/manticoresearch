# Параметры поиска с подстановочными знаками

Поиск с подстановочными знаками — это распространённый тип текстового поиска. В Manticore он выполняется на уровне словаря. По умолчанию как простые, так и RT-таблицы используют тип словаря, называемый [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). В этом режиме слова хранятся в том виде, в каком они есть, поэтому включение подстановочных знаков не влияет на размер таблицы. При выполнении поиска с подстановочными знаками словарь просматривается, чтобы найти все возможные расширения подстановочного слова. Такое расширение может быть проблематичным с точки зрения вычислений во время запроса, если расширенное слово даёт много расширений или расширения с большими списками попаданий, особенно в случае инфиксов, когда подстановочный знак добавляется в начало и конец слова. Чтобы избежать таких проблем, можно использовать параметр [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

Этот параметр определяет минимальную длину префикса слова для индексирования и поиска. По умолчанию установлен в 0, что означает, что префиксы не разрешены.

Префиксы позволяют выполнять поиск с подстановочными знаками с использованием `wordstart*`.

Например, если слово "example" индексируется с min_prefix_len=3, его можно найти, выполнив поиск по "exa", "exam", "examp", "exampl", а также по полному слову.

Обратите внимание, что при использовании [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc параметр min_prefix_len повлияет на размер полнотекстового индекса, поскольку каждое расширение слова будет храниться дополнительно.

Manticore может различать точные совпадения слов и совпадения по префиксу, при этом первые могут иметь более высокий ранг, если выполняются следующие условия:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включено по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (выключено по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключено по умолчанию)

Обратите внимание, что при использовании режима [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любого из вышеуказанных параметров невозможно отличить префиксы от полных слов, и точные совпадения слов не могут иметь более высокий ранг.

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
Параметр min_infix_len определяет минимальную длину префикса инфикса для индексирования и поиска. Он является необязательным, и его значение по умолчанию — 0, что означает, что инфиксы не разрешены. Минимально допустимое ненулевое значение — 2.

При включении инфиксы позволяют выполнять поиск с подстановочными знаками с шаблонами типа `start*`, `*end`, `*middle*` и так далее. Это также позволяет отключать слишком короткие подстановочные знаки, если они слишком дорогие для поиска.

Если выполняются следующие условия, Manticore может отличать точные совпадения слов от совпадений по инфиксу и ставить первые выше:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (включено по умолчанию)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (выключено по умолчанию),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (также выключено по умолчанию)

Обратите внимание, что при использовании режима [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc или при отключении любого из указанных параметров нет возможности отличить инфиксы от полных слов, поэтому точные совпадения слов не могут иметь более высокий ранг.

Время выполнения поиска с подстановочными знаками на инфиксах может значительно варьироваться в зависимости от того, на сколько ключевых слов будет расширен подстрочный запрос. Короткие и часто встречающиеся слоги, такие как `*in*` или `*ti*`, могут расшириться слишком многими ключевыми словами, которые все нужно будет сопоставить и обработать. Поэтому для общего включения поиска подстрок рекомендуют устанавливать min_infix_len равным 2. Чтобы ограничить влияние поиска с слишком короткими подстановочными знаками, можно установить значение выше.

Инфиксы должны быть длиной не менее 2 символов, и подстановочные знаки типа `*a*` по соображениям производительности не разрешены.

При установке min_infix_len в положительное число [минимальная длина префикса](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) считается равной 1. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) одновременно не могут быть включены и инфиксы, и префиксы. Для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и других полей, у которых указаны префиксы через [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), запрещено заявлять одно и то же поле в обоих списках.

Если dict=keywords, помимо подстановочного знака `*` можно использовать два других символа подстановки:
* `?` может соответствовать любому (одному) символу:  `t?st` найдёт `test`, но не `teast`
* `%` может соответствовать нулю или одному символу:  `tes%` найдёт `tes` или `test`, но не `testing`


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

Настройка prefix_fields используется для ограничения индексирования префиксов только конкретными полнотекстовыми полями в режиме [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). По умолчанию все поля индексируются в префиксном режиме, но так как индексирование префиксов может влиять на производительность как индексации, так и поиска, возможно, нужно ограничить его определёнными полями.

Чтобы ограничить индексирование префиксов конкретными полями, используйте настройку prefix_fields с последующим перечнем имён полей через запятую. Если prefix_fields не задано, то все поля будут индексироваться в префиксном режиме.

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

Настройка infix_fields позволяет указать перечень полнотекстовых полей, для которых ограничивается индексирование инфиксов. Это применяется только для [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) и является необязательной, по умолчанию индексируются все поля в инфиксном режиме.
Эта настройка схожа с [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict), но вместо этого позволяет ограничить индексирование инфиксов определёнными полями.


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

Директива max_substring_len задаёт максимальную длину подстроки, которая индексируется для префиксных или инфиксных поисков. Эта настройка необязательная, и её значение по умолчанию равно 0 (что означает индексирование всех возможных подстрок). Она применяется только для [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict).

По умолчанию индексирование подстрок в [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) индексирует **все** возможные подстроки как отдельные ключевые слова, что может привести к чрезмерно большому полнотекстовому индексу. Поэтому директива max_substring_len позволяет пропускать слишком длинные подстроки, по которым, вероятно, никогда не будут искать.

Например, тестовая таблица из 10 000 блог-постов занимает разное количество дискового пространства в зависимости от настроек:
* 6,4 МБ базово (без подстрок)
* 24,3 МБ (в 3,8 раза) при min_prefix_len = 3
* 22,2 МБ (в 3,5 раза) при min_prefix_len = 3, max_substring_len = 8
* 19,3 МБ (в 3,0 раза) при min_prefix_len = 3, max_substring_len = 6
* 94,3 МБ (в 14,7 раза) при min_infix_len = 3
* 84,6 МБ (в 13,2 раза) при min_infix_len = 3, max_substring_len = 8
* 70,7 МБ (в 11,0 раза) при min_infix_len = 3, max_substring_len = 6

Поэтому ограничение максимальной длины подстрок может сэкономить 10-15 % размера таблицы.

При использовании режима dict=keywords влияние длины подстрок на производительность отсутствует. Поэтому эта директива не применяется и намеренно запрещена в таком случае. Однако, если требуется, вы всё равно можете ограничить длину подстроки для поиска в коде приложения.


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

Эта настройка расширяет ключевые слова их точными формами и/или с звёздочками, когда это возможно. Поддерживаемые значения:
* 1 — расширение и до точной формы, и до формы со звёздочками. Например, `running` станет `(running | *running* | =running)`
* `exact` — дополнить ключевое слово только точной формой. Например, `running` станет `(running | =running)`
* `star` — дополнить ключевое слово добавлением `*` вокруг него. Например, `running` станет `(running | *running*)`
Эта настройка необязательная, значение по умолчанию 0 (ключевые слова не расширяются).

Запросы к таблицам с включённой функцией `expand_keywords` внутренне расширяются так: если таблица построена с включённым префиксным или инфиксным индексированием, каждый ключ заменяется дизъюнкцией ключевого слова и соответствующей формы с префиксом или инфиксом (со звёздочками). Если таблица построена с включённым стеммингом и [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words), то также добавляется точная форма.

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

Расширенные запросы естественно выполняются дольше, но могут улучшить качество поиска, поскольку документы с точным совпадением форм обычно получают более высокий рейтинг, чем документы с совпадениями по стеммам или инфиксам.

**Обратите внимание, что существующий синтаксис запросов не позволяет эмулировать такое расширение**, так как внутреннее расширение работает на уровне ключевых слов и расширяет ключевые слова внутри операторов фраз или кворума (что невозможнo через синтаксис запроса). Посмотрите примеры и как expand_keywords влияет на веса результатов поиска, а такое слово как "runsy" находится через "runs" без необходимости добавлять звёздочку:

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

Это директива никак не влияет на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), она влияет только на [searchd](../../Starting_the_server/Manually.md).


## expansion_limit

```ini
expansion_limit = number
```

Максимальное количество расширенных ключевых слов для одного wildcard. Подробности [здесь](../../Server_settings/Searchd.md#expansion_limit).

<!-- proofread -->

