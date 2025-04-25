# 通配符搜索设置

通配符搜索是一种常见的文本搜索类型。在 Manticore 中，它是在字典级别执行的。默认情况下，普通表和 RT 表都使用一种称为 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 的字典类型。在此模式下，单词按原样存储，因此启用通配符不会影响表的大小。当执行通配符搜索时，字典会被搜索以查找通配词的所有可能扩展。这种扩展在查询时可能会在计算上造成问题，尤其是在中缀的情况下，当通配符添加在单词的开头和结尾时，扩展的单词会提供许多扩展或者有巨大的命中列表。为了避免这种问题，可以使用 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit)。

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

此设置决定了索引和搜索的最小单词前缀长度。默认情况下，它设置为 0，这意味着不允许前缀。

前缀允许通过 `wordstart*` 通配符进行通配符搜索。

例如，如果单词 "example" 的索引为 min_prefix_len=3，则可以通过搜索 "exa", "exam", "examp", "exampl" 以及完整的单词找到它。

请注意，使用 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 时，min_prefix_len 将影响全文索引的大小，因为每个单词扩展将额外存储。

Manticore 可以将完美的单词匹配与前缀匹配区分开，并在满足以下条件时对前者进行更高的排名：
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1（默认关闭），
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1（也默认关闭）

请注意，在 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 模式或上述任何选项禁用时，无法区分前缀和完整单词，完美单词匹配也无法获得更高排名。

当 [minimum infix length](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 设置为正数时，最小前缀长度始终被视为 1。


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


<!-- example min_infix_len -->

```ini
min_infix_len = length
```
min_infix_len 设置决定了索引和搜索的中缀前缀的最小长度。此项是可选的，默认值为 0，这意味着不允许中缀。允许的最小非零值为 2。

启用时，中缀允许使用如下的术语模式进行通配符搜索 `start*`, `*end`, `*middle*`，等等。这也允许您禁用如果搜索成本太高的过短的通配符。

如果满足以下条件，Manticore 可以将完美的单词匹配与中缀匹配区分开，并对前者进行更高的排名：
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1（默认关闭），
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1（也默认关闭）

请注意，在 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 模式或上述任何选项禁用时，无法区分中缀和完整单词，因此完美单词匹配也无法获得更高排名。
中缀通配符搜索查询的时间可以有很大差异，这取决于子字符串实际扩展为多少个关键字。像 `*in*` 或 `*ti*` 这样的短而频繁的音节可能会扩展为过多的关键字，这些关键字都需要被匹配和处理。因此，为了普遍启用子字符串搜索，您应该将 min_infix_len 设置为 2。为了限制过短通配符的通配符搜索的影响，您可能需要将其设置得更高。

中缀必须至少为 2 个字符长，并且出于性能原因，不允许使用 `*a*` 这样的通配符。

当 min_infix_len 设置为正数时，[最小前缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 被认为是 1。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 字词，中缀和前缀不能同时启用。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 和其他字段可以通过 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields) 声明前缀，禁止在两个列表中声明同一字段。

如果 dict=keywords，除了通配符 `*` 之外，还可以使用两个其他通配符字符：
* `?` 可以匹配任何（一个）字符： `t?st` 将匹配 `test`，但不匹配 `teast`
* `%` 可以匹配零个或一个字符： `tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`


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

prefix_fields 设置用于将前缀索引限制为特定的全文字段，在 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式下。默认情况下，所有字段都以前缀模式索引，但因为前缀索引可能会影响索引和搜索性能，所以可能希望将其限制为某些字段。

要将前缀索引限制为特定字段，请使用 prefix_fields 设置，后跟以逗号分隔的字段名称列表。如果不设置 prefix_fields，则所有字段将以前缀模式进行索引。

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

infix_fields 设置允许您指定需要限制中缀索引的全文字段列表。这仅适用于 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，并且是可选的，默认情况下是以中缀模式索引所有字段。
该设置类似于 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，但允许您将中缀索引限制为特定字段。


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

max_substring_len 指令设置要为前缀或中缀搜索索引的最大子字符串长度。此设置是可选的，默认为 0（这意味着索引所有可能的子字符串）。它仅适用于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)。

默认情况下，[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 中的子字符串索引将 **所有** 可能的子字符串作为单独的关键字索引，这可能导致全文索引过大。因此，max_substring_len 指令允许您跳过可能永远不会被搜索的过长子字符串。

例如，一个包含 10,000 篇博客文章的测试表占用的磁盘空间量取决于设置：
* 6.4 MB 基线（无子字符串）
* 24.3 MB（3.8 倍）在 min_prefix_len = 3 的情况下
* 22.2 MB (3.5x) with min_prefix_len = 3, max_substring_len = 8
* 19.3 MB (3.0x) with min_prefix_len = 3, max_substring_len = 6
* 94.3 MB (14.7x) with min_infix_len = 3
* 84.6 MB (13.2x) with min_infix_len = 3, max_substring_len = 8
* 70.7 MB (11.0x) with min_infix_len = 3, max_substring_len = 6

因此，限制最大子字符串长度可以节省表大小的10-15%。

当使用 dict=keywords 模式时，子字符串长度没有性能影响。因此，此指令在这种情况下不适用，并且故意被禁止。但是，如果需要，您仍然可以在应用代码中限制要搜索的子字符串的长度。


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

此设置在可能的情况下使用其确切形式和/或带星号的形式扩展关键字。支持的值有：
* 1 - 扩展为确切形式和带星号的形式。例如，`running` 将变为 `(running | *running* | =running)`
* `exact` - 仅使用其确切形式增强关键字。例如，`running` 将变为 `(running | =running)`
* `star` - 通过在关键字周围添加 `*` 来增强关键字。例如，`running` 将变为 `(running | *running*)`
此设置是可选的，默认值为 0（关键字未扩展）。

针对启用了 `expand_keywords` 特性的表的查询会被内部扩展，如下所示：如果该表是使用前缀或中缀索引启用构建的，则每个关键字都将内部替换为该关键字本身和相应的前缀或中缀（带星号的关键字）的析取。如果该表是同时启用词干剖析和 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 的，则还会添加确切形式。

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

扩展查询自然完成的时间更长，但可能提高搜索质量，因为具有确切形式匹配的文档通常应具有比具有词干或中缀匹配的文档更高的排名。

**请注意，现有查询语法不允许模拟这种扩展**，因为内部扩展在关键字级别上工作，并且在短语或法定运算符内扩展关键字（这是通过查询语法无法实现的）。查看示例，以及 expand_keywords 如何影响搜索结果权重，以及“runsy”如何通过“runs” 被找到，而无需添加星号：

<!-- intro -->
##### expand_keywords is enabled
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
##### expand_keywords is disabled

<!-- request expand_keywords_disabled -->

```sql
mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
查询 OK，0 行受影响，1 个警告 (0.00 秒)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
查询 OK，3 行受影响 (0.00 秒)

mysql> select *, weight() from t where match('runs');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1500 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 rows in set (0.00 sec)

mysql> drop table t;
查询 OK，0 行受影响 (0.01 秒)

mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
查询 OK，0 行受影响，1 个警告 (0.00 秒)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
查询 OK，3 行受影响 (0.00 秒)

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

此指令不会以任何方式影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)，只会影响 [searchd](../../Starting_the_server/Manually.md)。


## expansion_limit

```ini
expansion_limit = number
```

单个通配符的最大扩展关键字数量。详细信息见 [here](../../Server_settings/Searchd.md#expansion_limit)。

<!-- proofread -->

# 通配符搜索设置

通配符搜索是一种常见的文本搜索类型。在Manticore中，它是在字典级别上执行的。默认情况下，普通表和RT表使用称为[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)的字典类型。在这种模式下，单词按原样存储，因此启用通配符不会影响表的大小。当执行通配符搜索时，会搜索字典以查找通配符单词的所有可能扩展。这种扩展在查询时可能会导致计算问题，尤其是在通配符添加在单词的开始和结束时，因为扩展的单词可能提供许多扩展或有庞大的命中列表。为避免此类问题，可以使用[expansion_limit](../../Server_settings/Searchd.md#expansion_limit)。

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

此设置决定了用于索引和搜索的最小单词前缀长度。默认情况下，它设置为0，表示不允许前缀。

前缀允许通过`wordstart*`通配符进行通配符搜索。

例如，如果单词“example”以min_prefix_len=3进行索引，则可以通过搜索“exa”、“exam”、“examp”、“exampl”以及完整单词找到它。

请注意，使用[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc时，min_prefix_len将影响全文索引的大小，因为每个单词扩展将额外存储。

Manticore可以区分完美单词匹配和前缀匹配，并在满足以下条件时将前者排名更高：
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1（默认关闭），
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1（也是默认关闭）

请注意，当[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc模式或上述任何选项禁用时，无法区分前缀和完整单词，完美单词匹配无法排名更高。

当[minimum infix length](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)设置为正数时，最小前缀长度始终被认为是1。


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
min_infix_len设置决定了用于索引和搜索的插入前缀的最小长度。它是可选的，默认值为0，这意味着不允许插入。允许的最小非零值为2。

启用时，插入允许使用像`start*`、`*end`、`*middle*`等术语模式进行通配符搜索。它还允许您禁用过短的通配符，如果它们的搜索代价过高。

如果满足以下条件，Manticore可以区分完美单词匹配和插入匹配并将前者排名更高：
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1（默认关闭），
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1（也是默认关闭）

请注意，使用[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc模式或上述任何选项禁用时，无法区分插入和完整单词，因此完美单词匹配不能排名更高。

插入通配符搜索查询时间可能会大幅变化，具体取决于子字符串实际上会扩展到多少个关键词。像`*in*`或`*ti*`这样的短且常见的音节可能会扩展到过多关键词，而所有这些都需要匹配和处理。因此，为了普遍启用子字符串搜索，您可以将min_infix_len设置为2。为了限制来自过短通配符的通配符搜索的影响，您可以将其设置得更高。

插入的长度必须至少为2个字符，出于性能原因，不允许使用`*a*`等通配符。
当 min_infix_len 设置为正数时，[最小前缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)被视为1。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 而言，字词中缀和前缀不能同时启用。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 和其他字段，要使用 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields) 声明前缀，不能在两个列表中声明相同的字段。

如果 dict=keywords，除了通配符 `*` 外，还可以使用两个其他通配符：
* `?` 可以匹配任何（一个）字符：`t?st` 将匹配 `test`，但不匹配 `teast`
* `%` 可以匹配零个或一个字符：`tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`


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

prefix_fields 设置用于限制在 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式下特定全文字段的前缀索引。默认情况下，所有字段都在前缀模式下索引，但由于前缀索引可能影响索引和搜索性能，可能希望将其限制在某些字段上。

要将前缀索引限制为特定字段，请使用 prefix_fields 设置，后跟以逗号分隔的字段名称列表。如果未设置 prefix_fields，则所有字段都将在前缀模式下索引。

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

infix_fields 设置允许您指定一个全文字段列表，以限制中缀索引。这仅适用于 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，并且是可选的，默认是在中缀模式下索引所有字段。
此设置类似于 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，但允许您将中缀索引限制为特定字段。


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

max_substring_len 指令设置要为前缀或中缀搜索索引的最大子串长度。此设置是可选的，其默认值为 0（表示索引所有可能的子串）。它仅适用于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)。

默认情况下，[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 中的子串索引会将**所有**可能的子串作为单独的关键词索引，这可能导致全文索引过大。因此，max_substring_len 指令允许您跳过可能永远不会被搜索的太长的子串。

例如，对于一个 10,000 篇博客文章的测试表，根据不同设置，占用的磁盘空间会有所不同：
* 6.4 MB 基准（无子串）
* 24.3 MB（3.8x）使用 min_prefix_len = 3
* 22.2 MB（3.5x）使用 min_prefix_len = 3，max_substring_len = 8
* 19.3 MB（3.0x）使用 min_prefix_len = 3，max_substring_len = 6
* 94.3 MB（14.7x）使用 min_infix_len = 3
* 84.6 MB（13.2x）使用 min_infix_len = 3，max_substring_len = 8
* 70.7 MB（11.0x）使用 min_infix_len = 3，max_substring_len = 6

因此，限制最大子串长度可以节省 10-15% 的表大小。

在使用 dict=keywords 模式时，子串长度没有性能影响。因此，此指令不适用，并且有意禁止使用。但是，如果需要，您仍可以在应用程序代码中限制要搜索的子串长度。


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

此设置在可能的情况下扩展精确形式和/或带星号的关键字。支持的值为：
* 1 -  扩展为精确形式和带星号的形式。例如，`running` 将变为 `(running | *running* | =running)`
* `exact` - 仅使用其精确形式增强关键字。例如，`running` 将变为 `(running | =running)`
* `star` - 通过在关键字周围添加 `*` 来增强关键字。例如，`running` 将变为 `(running | *running*)`
此设置是可选的，默认值为 0（关键字未被扩展）。

针对启用了 `expand_keywords` 特性的表的查询会被内部扩展如下：如果表是使用前缀或中缀索引启用构建的，则每个关键字会被内部替换为关键字本身和相应的前缀或中缀（带星号的关键字）的析取。如果表是同时启用了词干化和 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 的，则也会添加精确形式。

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

扩展的查询自然需要更长的时间完成，但可能会改善搜索质量，因为精确匹配的文档通常应排名高于词干或中缀匹配的文档。

**请注意，现有的查询语法不允许模拟这种扩展**，因为内部扩展是在关键字级别上进行的，并且也会扩展短语或投票操作符内的关键字（通过查询语法无法实现）。请查看示例以及 `expand_keywords` 如何影响搜索结果权重，以及如何通过 "runs" 找到 "runsy" 而无需添加星号：

<!-- intro -->
##### expand_keywords is enabled
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
##### expand_keywords is disabled

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

此指令不以任何方式影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)，它只影响 [searchd](../../Starting_the_server/Manually.md).
## expansion_limit

```ini
expansion_limit = number
```

单个通配符的最大扩展关键字数量。详情请见[这里](../../Server_settings/Searchd.md#expansion_limit)。

<!-- proofread -->



