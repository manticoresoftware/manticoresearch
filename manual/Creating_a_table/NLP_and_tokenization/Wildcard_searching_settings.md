# Wildcard searching settings

Wildcard searching is a common text search type. In Manticore it is performed at dictionary level. By default, both plain and RT tables use a dictionary type called [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). In this mode words are stored as they are, so the size of the table is not affected by enabling wildcarding. When a wildcard search is performed, in the dictionary a lookup is made to find all possible expansions of the wildcarded word. This expansion can be problematic in terms of computation at query time in cases where the expanded word can provide lots of expansions or expansions that have huge hitlists. The penalties are higher in case of infixes, where wildcard is added at the start and end of the words. [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) is to be used to avoid such problems.

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

Minimum word prefix length to index and search. Optional, default is 0 (do not allow prefixes).

Prefixes allow to implement wildcard searching by `wordstart*` wildcards.

For instance, if you index word "example" with min_prefix_len=3 you will be able to find it by "exa", "exam", "examp", "exampl" prefixes along with the word itself.

Be aware that in case of [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc min_prefix_len will also affect full-text index size as each word expansion will be stored additionally.

Manticore can differentiate perfect word matches from prefix matches and rank the former higher if you conform the following conditions:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (on by default)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (off by default),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (also off by default)

Note that either with the [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc mode or with any of the above options disabled, there is no way to differentiate between the prefixes and full words, and thus perfect word matches can't be ranked higher.

When [minimum infix length](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) is set to a positive number, minimum prefix length is always considered 1.


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'");
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

Minimum infix prefix length to index and search. Optional, default is 0 (do not allow infixes), and minimum allowed non-zero value is 2.

Infix length setting enables wildcard searches with term patterns like `start*`, `*end`, `*middle*`, and so on. It also lets you disable too short wildcards if those are too expensive to search for.

Manticore can differentiate perfect word matches from infix matches and rank the former higher if you conform the following conditions:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (on by default)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (off by default),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (also off by default)

Note that either with the [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc mode or with any of the above options disabled, there is no way to differentiate between the infixes and full words, and thus perfect word matches can't be ranked higher.

Infix wildcard search query time can vary greatly, depending on how many keywords the substring will actually expand to. Short and frequent syllables like `*in*` or `*ti*` just might expand to way too many keywords, all of which would need to be matched and processed. Therefore, to generally enable substring searches you would set min_infix_len to 2; and to limit the impact from wildcard searches with too short wildcards, you might set it higher.

Infixes must be at least 2 characters long, wildcards like `*a*` are not allowed for performance reasons.

When minimum infix length is set to a positive number, [minimum prefix length](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) is considered 1. For [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) word infixing and prefixing cannot be both enabled at the same. For [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) and other fields to have prefixes declared with [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), but it's forbidden to declare same field in the both lists.

In case of dict=keywords, beside the wildcard `*` two other wildcard characters can be used:
* `?` can match any(one) character: `t?st` will match `test`, but not `teast`
* `%` can match zero or one character : `tes%` will match `tes` or `test`, but not `testing`


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_infix_len = '3'");
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

List of full-text fields to limit prefix indexing to. Applies to [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) only. Optional, default is empty (index all fields in prefix mode).

Because prefix indexing impacts both indexing and searching performance, it might be desired to limit it to specific full-text fields only: for  instance, to provide prefix searching through URLs, but not through page contents. prefix_fields specifies what fields will be prefix-indexed; all other fields will be indexed in normal mode. The value format is a comma-separated list of field names.


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

The list of full-text fields to limit infix indexing to. Applies to [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) only. Optional, default is empty (index all fields in infix mode).

Similar to [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict), but lets you limit infix-indexing to given fields.


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

Maximum substring (either prefix or infix) length to index. Optional, default is 0 (do not limit indexed substrings). Applies to [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) only.

By default, substring (either prefix or infix) indexing in the [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) will index **all** possible substrings as separate keywords. That might result in an overly large full-text index. So this directive lets you limit the impact of substring indexing by skipping too-long substrings (which, chances are, will never get searched for anyway).

For example, a test table of 10,000 blog posts takes this much disk space depending on the settings:
* 6.4 MB baseline (no substrings)
* 24.3 MB (3.8x) with min_prefix_len = 3
* 22.2 MB (3.5x) with min_prefix_len = 3, max_substring_len = 8
* 19.3 MB (3.0x) with min_prefix_len = 3, max_substring_len = 6
* 94.3 MB (14.7x) with min_infix_len = 3
* 84.6 MB (13.2x) with min_infix_len = 3, max_substring_len = 8
* 70.7 MB (11.0x) with min_infix_len = 3, max_substring_len = 6

So in this test limiting the max substring length saved us 10-15% on the table size.

There is no performance impact associated with substring length when using dict=keywords mode, so this directive is not applicable and intentionally forbidden in that case. If required, you can still limit the length of a substring that you search for in the application code.


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

Expands keywords with their exact forms (i.e. the forms of the keywords before applying any morphological modifications) and/or stars when possible. The supported values are:
* 1 - expand to both the exact form and the form with the stars. `running` will become `(running | *running* | =running)`
* `exact` - augment the keyword with only its exact form. `running` will become `(running | =running)`
* `star` - augment the keyword by adding `*` around it.  `running` will become `(running | *running*)`
Optional, default is 0 (do not expand keywords).

Queries against tables with `expand_keywords` feature enabled are internally expanded as follows: if the table was built with prefix or infix indexing enabled, every keyword gets internally replaced with a disjunction of the keyword itself and a respective prefix or infix (keyword  with stars). If the table was built with both stemming and [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) enabled, exact form is also added.

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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) expand_keywords = '1'");
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
