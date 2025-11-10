# 通配符搜索设置

通配符搜索是一种常见的文本搜索类型。在 Manticore 中，它是在字典级别执行的。默认情况下，普通表和 RT 表都使用一种称为 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 的字典类型。在此模式下，单词按原样存储，因此启用通配符不会影响表的大小。当执行通配符搜索时，会搜索字典以查找通配符单词的所有可能扩展。当扩展的单词提供许多扩展或扩展具有庞大命中列表时，尤其是在通配符添加在单词的开头和结尾的中缀情况下，这种扩展在查询时的计算可能会有问题。为避免此类问题，可以使用 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit)。

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

此设置确定要索引和搜索的最小单词前缀长度。默认值为 0，表示不允许前缀。

前缀允许通过 `wordstart*` 通配符进行通配符搜索。

例如，如果单词 "example" 使用 min_prefix_len=3 进行索引，则可以通过搜索 "exa"、"exam"、"examp"、"exampl" 以及完整单词找到它。

请注意，使用 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 时，min_prefix_len 会影响全文索引的大小，因为每个单词扩展都会额外存储。

如果满足以下条件，Manticore 可以区分完全匹配的单词和前缀匹配，并将前者排名更高：
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1（默认关闭）
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1（也默认关闭）

请注意，在 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 模式或上述任何选项禁用时，无法区分前缀和完整单词，且无法将完全匹配的单词排名更高。

当 [minimum infix length](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 设置为正数时，最小前缀长度始终视为 1。


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
min_infix_len 设置确定要索引和搜索的中缀前缀的最小长度。此设置是可选的，默认值为 0，表示不允许中缀。允许的最小非零值为 2。

启用后，中缀允许使用类似 `start*`、`*end`、`*middle*` 等模式进行通配符搜索。它还允许您禁用过短的通配符，以防它们搜索成本过高。

如果满足以下条件，Manticore 可以区分完全匹配的单词和中缀匹配，并将前者排名更高：
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1（默认关闭）
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1（也默认关闭）

请注意，在 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 模式或上述任何选项禁用时，无法区分中缀和完整单词，因此无法将完全匹配的单词排名更高。

中缀通配符搜索的查询时间可能变化很大，取决于子字符串实际扩展到多少关键字。像 `*in*` 或 `*ti*` 这样短且频繁的音节可能会扩展到过多关键字，所有这些都需要匹配和处理。因此，为了通常启用子字符串搜索，您可以将 min_infix_len 设置为 2。为了限制过短通配符搜索的影响，您可以将其设置得更高。

中缀必须至少有 2 个字符长，出于性能原因，不允许使用像 `*a*` 这样的通配符。

当 min_infix_len 设置为正数时，[最小前缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 被视为 1。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，单词中缀和前缀不能同时启用。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 和其他字段，如果使用了 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields) 声明了前缀，则禁止在两个列表中声明相同字段。

如果 dict=keywords，除了通配符 `*` 外，还可以使用另外两个通配符字符：
* `?` 可以匹配任意（一个）字符：`t?st` 将匹配 `test`，但不匹配 `teast`
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

prefix_fields 设置用于限制前缀索引仅应用于 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式下的特定全文字段。默认情况下，所有字段都会以前缀模式进行索引，但由于前缀索引可能影响索引和搜索性能，因此可能希望将其限制在某些字段上。

要将前缀索引限制为特定字段，请使用 prefix_fields 设置，后跟以逗号分隔的字段名称列表。如果未设置 prefix_fields，则所有字段都将在前缀模式下进行索引。

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

infix_fields 设置允许您指定一个全文字段列表，以限制中缀索引的应用范围。这仅适用于 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式，且是可选的，默认情况下所有字段都以中缀模式进行索引。
此设置类似于 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，但它允许您将中缀索引限制在特定字段上。


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

max_substring_len 指令设置要为前缀或中缀搜索索引的最大子字符串长度。此设置是可选的，默认值为 0（表示索引所有可能的子字符串）。它仅适用于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)。

默认情况下，[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 中的子字符串索引会将 **所有** 可能的子字符串作为单独的关键字进行索引，这可能导致全文索引过大。因此，max_substring_len 指令允许您跳过过长且可能永远不会被搜索的子字符串。

例如，一个包含 10,000 篇博客文章的测试表根据设置占用不同的磁盘空间：
* 6.4 MB 基线（无子字符串）
* 24.3 MB（3.8 倍）使用 min_prefix_len = 3
* 22.2 MB（3.5 倍）使用 min_prefix_len = 3，max_substring_len = 8
* 19.3 MB（3.0 倍）使用 min_prefix_len = 3，max_substring_len = 6
* 94.3 MB（14.7 倍）使用 min_infix_len = 3
* 84.6 MB（13.2 倍）使用 min_infix_len = 3，max_substring_len = 8
* 70.7 MB（11.0 倍）使用 min_infix_len = 3，max_substring_len = 6

因此，限制最大子字符串长度可以节省 10-15% 的表大小。

在使用 dict=keywords 模式时，子字符串长度不会影响性能。因此，该指令不适用且在这种情况下被故意禁止。不过，如果需要，您仍然可以在应用代码中限制搜索子字符串的长度。


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

此设置在可能的情况下扩展关键字为其精确形式和/或带星号的形式。支持的值有：
* 1 - 同时扩展为精确形式和带星号的形式。例如，`running` 将变为 `(running | *running* | =running)`
* `exact` - 仅用其精确形式扩展关键字。例如，`running` 将变为 `(running | =running)`
* `star` - 通过在关键字周围添加 `*` 来扩展关键字。例如，`running` 将变为 `(running | *running*)`
此设置是可选的，默认值为 0（不扩展关键字）。

启用 `expand_keywords` 功能的表的查询会在内部进行如下扩展：如果表启用了前缀或中缀索引，则每个关键字都会被内部替换为关键字本身与相应的前缀或中缀（带星号的关键字）的析取。如果表同时启用了词干提取和 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)，还会添加精确形式。

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

扩展后的查询自然需要更长时间完成，但可能提升搜索质量，因为精确形式匹配的文档通常应比词干或中缀匹配的文档排名更高。

**注意，现有的查询语法不允许模拟这种扩展**，因为内部扩展是在关键字级别进行的，并且会扩展短语或仲裁操作符内的关键字（这是查询语法无法实现的）。请查看示例，了解 expand_keywords 如何影响搜索结果权重，以及如何在不添加星号的情况下通过 "runs" 找到 "runsy"：

<!-- intro -->
##### 启用 expand_keywords
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
##### 禁用 expand_keywords

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

该指令不会以任何方式影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)，它只影响[searchd](../../Starting_the_server/Manually.md)。


## expansion_limit

```ini
expansion_limit = number
```

单个通配符的最大扩展关键字数。详细信息见[这里](../../Server_settings/Searchd.md#expansion_limit)。

<!-- proofread -->

