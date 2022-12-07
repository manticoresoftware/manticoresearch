# Exceptions

Exceptions (also known as synonyms) allow to map one or more tokens (including tokens with characters that would normally be excluded) to a single keyword. They are similar to [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) in that they also perform mapping, but have a number of important differences.

Short summary of the differences from [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) is as follows:

| Exceptions | Word forms |
| - | - |
| Case sensitive | Case insensitive |
| Can use special characters that are not in [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) | Fully obey [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| Underperform on huge dictionaries | Designed to handle millions of entries |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->
Tokenizing exceptions file. Optional, default is empty.
In the RT mode only absolute paths are allowed.

The expected file format is also plain text, with one line per exception, and the line format is as follows:

```ini
map-from-tokens => map-to-token
```

Example file:

```ini
at & t => at&t
AT&T => AT&T
Standarten   Fuehrer => standartenfuhrer
Standarten Fuhrer => standartenfuhrer
MS Windows => ms windows
Microsoft Windows => ms windows
C++ => cplusplus
c++ => cplusplus
C plus plus => cplusplus
```

All tokens here are case sensitive: they will **not** be processed by [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) rules. Thus, with the example exceptions file above, "at&t" text will be tokenized as two keywords "at" and "t", because of lowercase letters. On the other hand, "AT&T" will match exactly and produce single "AT&T" keyword.

Note that this map-to keyword is:
* always interpreted as a *single* word
* and is both case and space sensitive

In our sample, "ms windows" query will *not* match the document with "MS Windows" text. The query will be interpreted as a query for two keywords, "ms" and "windows". And what "MS Windows" gets mapped to is a *single* keyword "ms windows", with a space in the middle. On the other hand, "standartenfuhrer" will retrieve documents with "Standarten Fuhrer" or "Standarten Fuehrer" contents (capitalized exactly like this), or any capitalization variant of the keyword itself, eg. "staNdarTenfUhreR". (It won't catch "standarten fuhrer", however: this text does not match any of the listed exceptions because of case sensitivity, and gets indexed as two separate keywords.)

Whitespace in the map-from tokens list matters, but its amount does not. Any amount of the whitespace in the map-form list will match any other amount of whitespace in the indexed document or query. For instance, "AT & T" map-from token will match "AT & T" text, whatever the amount of space in both map-from part and the indexed text. Such text will therefore be indexed as a special "AT&T" keyword, thanks to the very first entry from the sample.

Exceptions also allow to capture special characters (that are exceptions from general [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) rules; hence the name). Assume that you generally do not want to treat '+' as a valid character, but still want to be able search for some exceptions from this rule such as 'C++'. The sample above will do just that, totally independent of what characters are in the table and what are not.

Exceptions are applied to raw incoming document and query data during indexing and searching respectively. Therefore, when it comes to a [plain table](../../Creating_a_table/Local_tables/Plain_table.md) to pick up changes in the file it's required to reindex and restart `searchd`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'
```

<!-- request HTTP -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'exceptions' => '/usr/local/manticore/data/exceptions.txt'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'");
```
<!-- request CONFIG -->

```ini
table products {
  exceptions = /usr/local/manticore/data/exceptions.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
