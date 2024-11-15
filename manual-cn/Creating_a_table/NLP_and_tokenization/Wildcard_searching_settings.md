# 通配符搜索设置

通配符搜索是一种常见的文本搜索类型。在 Manticore 中，它是在字典级别执行的。默认情况下，普通表和实时表使用称为 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 的字典类型。在此模式下，单词按原样存储，因此启用通配符不会影响表的大小。当执行通配符搜索时，系统会搜索字典以查找通配符单词的所有可能扩展。这种扩展在查询时可能会带来计算上的问题，特别是当扩展的单词提供许多扩展或具有巨大命中的情况下，尤其是在通配符被添加到单词的开头和结尾时。为避免此类问题，可以使用 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit)。

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

此设置确定要索引和搜索的最小单词前缀长度。默认值设置为 0，这意味着不允许前缀。

前缀允许通过 `wordstart*` 通配符进行搜索。

例如，如果单词 "example" 的最小前缀长度设置为 3，则可以通过搜索 "exa"、"exam"、"examp"、"exampl" 以及完整单词来找到它。

请注意，对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc，min_prefix_len 会影响全文索引的大小，因为每个单词扩展都会额外存储。

Manticore 可以将精确单词匹配与前缀匹配区分开，并在满足以下条件时对前者进行更高的排名：

- [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
- [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1（默认关闭）
- [expand_keywords](../../Searching/Options.md#expand_keywords)=1（同样默认关闭）

请注意，如果 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 模式或上述任何选项被禁用，则无法区分前缀和完整单词，且精确单词匹配无法获得更高的排名。

当 [最小中缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 设置为正数时，最小前缀长度始终被视为 1。


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
min_infix_len 设置确定了要索引和搜索的中缀前缀的最小长度。它是可选的，默认值为 0，这意味着不允许中缀。允许的最小非零值为 2。

启用后，中缀允许使用术语模式进行通配符搜索，例如 `start*`、`*end`、`*middle*` 等。它还允许禁用过短的通配符，如果它们的搜索成本过高。

如果满足以下条件，Manticore 可以将精确单词匹配与中缀匹配区分开，并对前者进行更高的排名：

- [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（默认开启）
- [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1（默认关闭）
- [expand_keywords](../../Searching/Options.md#expand_keywords)=1（同样默认关闭）

请注意，如果 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc 模式或上述任何选项被禁用，则无法区分中缀和完整单词，因此精确单词匹配无法获得更高的排名。

中缀通配符搜索查询的时间可能会有很大差异，这取决于子字符串实际扩展为多少关键字。像 `*in*` 或 `*ti*` 这样的短而常见的音节可能会扩展为过多的关键字，而这些关键字都需要被匹配和处理。因此，为了普遍启用子字符串搜索，您应该将 min_infix_len 设置为 2。为了限制过短通配符搜索的影响，您可以将其设置得更高。

中缀必须至少为 2 个字符长，并且出于性能原因，不允许使用像 `*a*` 这样的通配符。

当 min_infix_len 设置为正数时， [最小前缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 被视为 1。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 来说，前缀和中缀不能同时启用。对于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 和其他字段，如果通过 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields) 声明了前缀，则禁止在两个列表中声明同一字段。

如果 dict=keywords，除了通配符 `*`，还可以使用两个其他通配符字符：

- `?` 可以匹配任何（一个）字符： `t?st` 将匹配 `test`，但不匹配 `teast`
- `%` 可以匹配零个或一个字符： `tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`


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

`prefix_fields` 设置用于在 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式下将前缀索引限制为特定的全文字段。默认情况下，所有字段都在前缀模式下索引，但由于前缀索引会影响索引和搜索性能，可能希望将其限制在某些特定字段上。

要将前缀索引限制为特定字段，请使用 `prefix_fields` 设置，后跟以逗号分隔的字段名称列表。如果未设置 `prefix_fields`，则所有字段都将在前缀模式下索引。

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

`infix_fields` 设置允许你指定一个全文字段列表，以将中缀索引限制在这些字段中。该设置仅适用于 [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式，且为可选项，默认情况下，所有字段都在中缀模式下索引。

此设置类似于 [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)，但它允许将中缀索引限制在特定字段上。


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

`max_substring_len` 指令设置用于前缀或中缀搜索的最大子字符串长度。该设置为可选项，默认值为 0（即索引所有可能的子字符串）。该指令仅适用于 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式。

在 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 模式中，默认情况下，子字符串索引将 **所有** 可能的子字符串作为独立关键词进行索引，这可能会导致全文索引过大。因此，`max_substring_len` 指令允许跳过可能永远不会被搜索的过长子字符串。

例如，一个包含 10,000 篇博客文章的测试表根据设置的不同会占用不同的磁盘空间：

- 6.4 MB 基线（没有子字符串）
- 24.3 MB (3.8倍) 当 `min_prefix_len = 3`
- 22.2 MB (3.5倍) 当 `min_prefix_len = 3, max_substring_len = 8`
- 19.3 MB (3.0倍) 当 `min_prefix_len = 3, max_substring_len = 6`
- 94.3 MB (14.7倍) 当 `min_infix_len = 3`
- 84.6 MB (13.2倍) 当 `min_infix_len = 3, max_substring_len = 8`
- 70.7 MB (11.0倍) 当 `min_infix_len = 3, max_substring_len = 6`

因此，限制最大子字符串长度可以节省 10-15% 的表空间。

在 `dict=keywords` 模式下，子字符串长度对性能没有影响，因此该指令不适用并且在此模式下被禁止使用。不过，如果需要，仍然可以在应用代码中限制搜索的子字符串长度。


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

该设置用于扩展关键词的确切形式和/或在关键词周围添加星号。支持的值有：
*  1 - 扩展为确切形式和带星号的形式。例如，`running` 会被扩展为 `(running | *running* | =running)`
* ``exact`  - 仅扩展关键词的确切形式。例如，`running` 会被扩展为 `(running | =running)`
* `star` - 通过在关键词周围添加 `*` 扩展关键词。例如，`running` 会被扩展为 `(running | *running*)`

当启用 `expand_keywords` 功能时，对表执行的查询会进行内部扩展：如果构建表时启用了前缀或中缀索引，系统会将每个关键词内部替换为该关键词本身及相应的前缀或中缀（带星号的关键词）的析取形式。如果表在构建时启用了词干提取和 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)，则还会添加关键词的确切形式。

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

扩展查询自然会增加查询完成的时间，但可能会提高搜索质量，因为匹配确切形式的文档通常会比匹配词干或中缀的文档排名更高。

**注意，现有的查询语法无法模拟这种扩展**，因为内部扩展是在关键词级别上进行的，且扩展关键词包括短语或配额操作符内的关键词（这是查询语法无法实现的）。下列示例展示了 `expand_keywords` 如何影响搜索结果权重，以及如何通过 "runs" 查找到 "runsy"，而不需要添加星号：

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

该指令不会对 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具) 产生任何影响，它仅对 [searchd](../../Starting_the_server/Manually.md) 有效。


## expansion_limit

```ini
expansion_limit = number
```

单个通配符扩展的最大关键词数量。详情请参见 [这里](../../Server_settings/Searchd.md#expansion_limit)。

<!-- proofread -->
