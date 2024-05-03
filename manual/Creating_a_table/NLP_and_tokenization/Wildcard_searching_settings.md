# Wildcard searching settings

Wildcard searching is a common text search type. In Manticore, it is performed at the dictionary level. By default, both plain and RT tables use a dictionary type called [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). In this mode, words are stored as they are, so enabling wildcarding does not affect the size of the table. When a wildcard search is performed, the dictionary is searched to find all possible expansions of the wildcarded word. This expansion can be problematic in terms of computation at query time when the expanded word provides many expansions or expansions that have huge hitlists, especially in the case of infixes where the wildcard is added at the start and end of the word. To avoid such problems, the  [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) can be used.

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

This setting determines the minimum word prefix length to index and search. By default, it is set to 0, meaning prefixes are not allowed.

Prefixes allow for wildcard searching by `wordstart*` wildcards.

For example, if the word "example" is indexed with min_prefix_len=3, it can be found by searching for "exa", "exam", "examp", "exampl", as well as the full word.

Note that with [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc min_prefix_len will affect the size of the full-text index since each word expansion will be stored additionally.

Manticore can differentiate perfect word matches from prefix matches and rank the former higher if the following conditions are met:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (on by default)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (off by default),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (also off by default)

Note that with either  [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc mode or any of the above options disabled, it is not possible to differentiate between prefixes and full words, and perfect word matches cannot be ranked higher.

When the [minimum infix length](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) is set to a positive number, the minimum prefix length is always considered 1.


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
The min_infix_len setting determines the minimum length of an infix prefix to index and search. It is optional and its default value is 0, which means that infixes are not allowed. The minimum allowed non-zero value is 2.

When enabled, infixes allow for wildcard searching with term patterns like `start*`, `*end`, `*middle*`, , and so on. It also allows you to disable too short wildcards if they are too expensive to search for.

If the following conditions are met, Manticore can differentiate perfect word matches from infix matches and rank the former higher:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (on by default)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (off by default),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (also off by default)

Note that with the [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc mode or any of the above options disabled, there is no way to differentiate between infixes and full words, and thus perfect word matches cannot be ranked higher.

Infix wildcard search query time can vary greatly, depending on how many keywords the substring will actually expand to. Short and frequent syllables like `*in*` or `*ti*` might expand to way too many keywords, all of which would need to be matched and processed. Therefore, to generally enable substring searches, you would set min_infix_len to 2. To limit the impact from wildcard searches with too short wildcards, you might set it higher.

Infixes must be at least 2 characters long, and wildcards like `*a*` are not allowed for performance reasons.

When min_infix_len is set to a positive number, the [minimum prefix length](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) is considered 1. For [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) word infixing and prefixing cannot be both enabled at the same time. For [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) and other fields to have prefixes declared with [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), it is forbidden to declare the same field in both lists.

If dict=keywords, besides the wildcard `*` two other wildcard characters can be used:
* `?` can match any (one) character:  `t?st` will match `test`, but not `teast`
* `%` can match zero or one character:  `tes%` will match `tes` or `test`, but not `testing`


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

The prefix_fields setting is used to limit prefix indexing to specific full-text fields in [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) mode. By default, all fields are indexed in prefix mode, but because prefix indexing can affect both indexing and searching performance, it may be desired to limit it to certain fields.

To limit prefix indexing to specific fields, use the prefix_fields setting followed by a comma-separated list of field names. If prefix_fields is not set, then all fields will be indexed in prefix mode.

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

The infix_fields setting allows you to specify a list of full-text fields to limit infix indexing to. This applies to [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) only and is optional, with the default being to index all fields in infix mode.
This setting is similar to [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict), but instead allows you to limit infix indexing to specific fields.


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

The max_substring_len directive sets the maximum substring length to be indexed for either prefix or infix searches. This setting is optional, and its default value is 0 (which means that all possible substrings are indexed). It only applies to [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict).

By default, substring indexing in [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) indexes  **all** possible substrings as separate keywords, which can result in an overly large full-text index. Therefore, the max_substring_len directive allows you to skip too-long substrings that will probably never be searched for.

For example, a test table of 10,000 blog posts takes up a different amount of disk space depending on the settings:
* 6.4 MB baseline (no substrings)
* 24.3 MB (3.8x) with min_prefix_len = 3
* 22.2 MB (3.5x) with min_prefix_len = 3, max_substring_len = 8
* 19.3 MB (3.0x) with min_prefix_len = 3, max_substring_len = 6
* 94.3 MB (14.7x) with min_infix_len = 3
* 84.6 MB (13.2x) with min_infix_len = 3, max_substring_len = 8
* 70.7 MB (11.0x) with min_infix_len = 3, max_substring_len = 6

Therefore, limiting the max substring length can save 10-15% of the table size.

When using dict=keywords mode, there is no performance impact associated with substring length. Therefore, this directive is not applicable and is intentionally forbidden in that case. However, if required, you can still limit the length of a substring that you search for in the application code.


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

This setting expands keywords with their exact forms and/or with stars when possible. The supported values are:
* 1 -  expand to both the exact form and the form with the stars. For instance,`running` will become `(running | *running* | =running)`
* `exact` - - augment the keyword with only its exact form. For instance, `running` will become `(running | =running)`
* `star` - augment the keyword by adding `*` around it. For instance, `running` will become `(running | *running*)`
This setting is optional, and the default value is 0 (keywords are not expanded).

Queries against tables with `expand_keywords` feature enabled are internally expanded as follows: if the table was built with prefix or infix indexing enabled, every keyword gets internally replaced with a disjunction of the keyword itself and a respective prefix or infix (keyword with stars). If the table was built with both stemming and [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) enabled, exact form is also added.

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

Expanded queries take naturally longer to complete, but can possibly improve the search quality, as the documents with exact form matches should be ranked generally higher than documents with stemmed or infix matches.

**Note that the existing query syntax does not allow to emulate this kind of expansion**, because internal expansion works on keyword level and expands keywords within phrase or quorum operators too (which is not possible through the query syntax). Take a look at the examples and how expand_keywords affects the search result weights and how "runsy" is found by "runs" w/o the need to add a star:

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

This directive does not affect [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) in any way, it only affects [searchd](../../Starting_the_server/Manually.md).


## expansion_limit

```ini
expansion_limit = number
```

Maximum number of expanded keywords for a single wildcard. Details are [here](../../Server_settings/Searchd.md#expansion_limit).

<!-- proofread -->
