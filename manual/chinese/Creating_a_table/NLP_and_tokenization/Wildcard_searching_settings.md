# 通配符搜索设置

通配符搜索是一种常见的文本搜索类型。在 Manticore 中，它是在字典级别进行的。默认情况下，普通表和 RT 表都使用一种名为 [dict] 的字典类型（参见 ../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict）。在这种模式下，单词以原样存储，因此启用通配符不会影响表的大小。当执行通配符搜索时，会在字典中查找所有可能扩展的通配符单词。这种扩展在查询时可能会导致计算问题，尤其是当扩展的单词提供许多扩展或具有巨大的命中列表时，特别是在通配符添加到单词开头和结尾的情况下。为了避免这些问题，可以使用 [expansion_limit]（参见 ../../Server_settings/Searchd.md#expansion_limit）。

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

此设置确定要索引和搜索的最小单词前缀长度。默认情况下，它设置为 0，这意味着不允许前缀。

前缀允许通过 `wordstart*` 通配符进行通配符搜索。

例如，如果单词 "example" 使用 min_prefix_len=3 索引，则可以通过搜索 "exa"、"exam"、"examp" 和 "exampl" 以及完整的单词来找到它。

请注意，在 [dict]=crc 模式下，min_prefix_len 将会影响全文索引的大小，因为每个单词扩展都会额外存储。

Manticore 可以区分完美匹配的单词和前缀匹配，并在满足以下条件时将前者排名更高：
* [dict]=keywords（默认开启）
* [index_exact_words]=1（默认关闭）
* [expand_keywords]=1（也默认关闭）

请注意，如果使用 [dict]=crc 模式或上述任一选项禁用，则无法区分前缀和完整单词，也无法将完美匹配的单词排名更高。

当设置了 [最小后缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 为正数时，最小前缀长度始终被视为 1。


<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'", true);
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'", true);
```

<!-- intro -->
##### Rust：

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
min_infix_len 设置确定要索引和搜索的最小后缀前缀长度。它是可选的，默认值为 0，这意味着不允许后缀。最小允许的非零值为 2。

启用后，后缀允许使用如 `start*`、`*end`、`*middle*` 等术语模式进行通配符搜索。它还允许您禁用太短且搜索成本过高的通配符。

如果满足以下条件，Manticore 可以区分完美匹配的单词和后缀匹配，并将前者排名更高：
* [dict]=keywords（默认开启）
* [index_exact_words]=1（默认关闭）
* [expand_keywords]=1（也默认关闭）

请注意，如果使用 [dict]=crc 模式或上述任一选项禁用，则无法区分后缀和完整单词，因此无法将完美匹配的单词排名更高。

后缀通配符搜索的查询时间可能会有很大差异，这取决于子字符串实际扩展成多少关键词。像 `*in*` 或 `*ti*` 这样的短而频繁的音节可能会扩展成太多关键词，所有这些都需要匹配和处理。因此，为了启用子字符串搜索，通常会将 min_infix_len 设置为 2。为了限制来自太短通配符的搜索的影响，可能会将其设置得更高。

后缀必须至少为两个字符长，出于性能原因，`*a*` 类似的通配符是不允许的。

当 min_infix_len 设置为正数时，[最小前缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 被视为 1。对于 [dict] 字段，前后缀不能同时启用。如果要在 [dict] 和其他字段中声明前缀字段 [prefix_fields]，则禁止在同一列表中声明相同的字段。

如果 dict=keywords，除了通配符 `*` 外，还可以使用另外两个通配符字符：
* `?` 可匹配任何（一个）字符：`t?st` 将匹配 `test`，但不匹配 `teast`
* `%` 可匹配零个或一个字符：`tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`


<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'')
```

<!-- intro -->
##### Javascript：

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

prefix_fields 设置用于在 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式下限制前缀索引到特定的全文字段。默认情况下，所有字段都在前缀模式下进行索引，但因为前缀索引可能会影响索引和搜索性能，所以可能需要将其限制在某些字段上。

要将前缀索引限制到特定字段，请使用 prefix_fields 设置，后跟逗号分隔的字段名列表。如果未设置 prefix_fields，则所有字段将在前缀模式下进行索引。

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

infix_fields 设置允许您指定一个列表的全文字段，以限制中缀索引。此设置仅适用于 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，是可选的，默认情况下所有字段都在中缀模式下进行索引。
此设置类似于 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，但它允许您将中缀索引限制到特定字段。


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

max_substring_len 指令设置了前缀或中缀搜索时要索引的最大子字符串长度。此设置是可选的，默认值为 0（即索引所有可能的子字符串）。它仅适用于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)。

默认情况下，在 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 中，子字符串索引会将所有可能的子字符串作为单独的关键字进行索引，这可能会导致一个过于庞大的全文索引。因此，max_substring_len 指令允许跳过那些几乎永远不会被搜索的太长的子字符串。

例如，一个包含 10,000 篇博客文章的测试表在不同设置下的磁盘空间使用量如下：
* 基线：6.4 MB（没有子字符串）
* min_prefix_len = 3：24.3 MB（3.8 倍）
* min_prefix_len = 3，max_substring_len = 8：22.2 MB（3.5 倍）
* min_prefix_len = 3，max_substring_len = 6：19.3 MB（3.0 倍）
* min_infix_len = 3：94.3 MB（14.7 倍）
* min_infix_len = 3，max_substring_len = 8：84.6 MB（13.2 倍）
* min_infix_len = 3，max_substring_len = 6：70.7 MB（11.0 倍）

因此，限制最大子字符串长度可以节省 10-15% 的表大小。

在使用 dict=keywords 模式时，子字符串长度不会影响性能。因此，此指令在此情况下是不适用的，并且有意禁止。然而，如果需要，你仍然可以在应用程序代码中限制你要搜索的子字符串的长度。


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

此设置扩展关键字及其精确形式和/或星号。支持的值包括：
* 1 - 扩展为精确形式和带有星号的形式。例如，`running` 将变为 `(running | *running* | =running)`
* `exact` - 仅增加关键字的精确形式。例如，`running` 将变为 `(running | =running)`
* `star` - 通过在其周围添加星号来增加关键字。例如，`running` 将变为 `(running | *running*)`
此设置是可选的，默认值为 0（关键字不扩展）。

启用 `expand_keywords` 特性的表中的查询内部会进行扩展：如果表是在前缀或中缀索引启用的情况下构建的，每个关键字都会被替换为其自身和相应的前缀或中缀（带星号）的析取。如果表是在启用了词干提取和 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 的情况下构建的，精确形式也会被添加。

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

扩展查询自然会花费更长时间完成，但可能会提高搜索质量，因为具有精确形式匹配的文档通常应该比具有词干或中缀匹配的文档排名更高。

请注意，现有的查询语法无法模拟这种扩展，因为内部扩展工作在关键字级别，并且会在短语或群集操作符内扩展关键字（这通过查询语法是不可能实现的）。请参阅示例以及 expand_keywords 如何影响搜索结果权重，以及如何通过不需要添加星号的方式找到“runsy”：

<!-- intro -->
##### expand_keywords 是启用的
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
##### expand_keywords 是禁用的

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

此指令不会影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)，仅影响 [searchd](../../Starting_the_server/Manually.md)。


## expansion_limit

```ini
expansion_limit = number
```

单个通配符展开的最大关键词数。详情见 [这里](../../Server_settings/Searchd.md#expansion_limit)。

<!-- proofread -->

