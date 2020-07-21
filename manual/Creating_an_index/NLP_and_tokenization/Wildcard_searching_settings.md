# Wildcard searching settings

Wildcard searching is a common text search type. In Manticore it is performed at dictionary level. By default, both plain and RT indexes use a dictionary type called [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict). In this mode words are stored as they are, so the size of the index is not affected by enabling wildcarding. When a wildcard search is performed, in the dictionary a lookup is made to find all possible expansions of the wildcarded word. This expansion can be problematic in terms of computation at query time in cases where the expanded word can provide lots of expansions or expansions that have huge hitlists. The penalties are higher in case of infixes, where wildcard is added at the start and end of the words. [expansion_limit](Server_settings/Searchd.md#expansion_limit) is to be used to avoid such problems. 

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

Minimum word prefix length to index and search. Optional, default is 0 (do not allow prefixes).

Prefixes allow to implement wildcard searching by `wordstart*` wildcards. 

For instance, if you index word "example" with min_prefix_len=3 you will be able to find it by "exa", "exam", "examp", "exampl" prefixes along with the word itself. 

Be aware that in case of [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc min_prefix_len will also affect index size as each word expansion will be stored additionally.

Manticore can differentiate perfect word matches from prefix matches and rank the former higher if you conform the following conditions:
* [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (on by default)
* [index_exact_words](Creating_an_index/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (off by default),
* [expand_keywords](Searching/Options.md#expand_keywords)=1 (also off by default)

Note that either with the [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc mode or with any of the above options disabled, there is no way to differentiate between the prefixes and full words, and thus perfect word matches can't be ranked higher.

When [minimum infix length](Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) is set to a positive number, minimum prefix length is always considered 1.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) min_prefix_len = '3'
```

<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
create table products(title text, price float) min_prefix_len = '3'"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'min_prefix_len' => '3'
        ],
        'columns' => [
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ]
    ],
    'index' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```

<!-- request CONFIG -->

```ini
index products {
  min_prefix_len = 3

  type = rt
  path = idx
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
* [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (on by default)
* [index_exact_words](Creating_an_index/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (off by default),
* [expand_keywords](Searching/Options.md#expand_keywords)=1 (also off by default)

Note that either with the [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc mode or with any of the above options disabled, there is no way to differentiate between the infixes and full words, and thus perfect word matches can't be ranked higher.

Infix wildcard search query time can vary greatly, depending on how many keywords the substring will actually expand to. Short and frequent syllables like `*in*` or `*ti*` just might expand to way too many keywords, all of which would need to be matched and processed. Therefore, to generally enable substring searches you would set min_infix_len to 2; and to limit the impact from wildcard searches with too short wildcards, you might set it higher.

Infixes must be at least 2 characters long, wildcards like `*a*` are not allowed for performance reasons.

When minimum infix length is set to a positive number, [minimum prefix length](Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) is considered 1. For [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict) word infixing and prefixing cannot be both enabled at the same. For [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict) it is possible to specify only some fields to have infixes declared with [infix_fields](Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#infix_fields) and other fields to have prefixes declared with [prefix_fields](Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), but it's forbidden to declare same field in the both lists.

In case of dict=keywords, beside the wildcard `*` two other wildcard characters can be used:
* `?` can match any(one) character: `t?st` will match `test`, but not `teast`
* `%` can match zero or one character : `tes%` will match `tes` or `test`, but not `testing`


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) min_infix_len = '3'
```

<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
create table products(title text, price float) min_infix_len = '3'"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'min_infix_len' => '3'
        ],
        'columns' => [
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ]
    ],
    'index' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```

<!-- request CONFIG -->

```ini
index products {
  min_infix_len = 3

  type = rt
  path = idx
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

List of full-text fields to limit prefix indexing to. Applies to [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict) only. Optional, default is empty (index all fields in prefix mode).

Because prefix indexing impacts both indexing and searching performance, it might be desired to limit it to specific full-text fields only: for  instance, to provide prefix searching through URLs, but not through page contents. prefix_fields specifies what fields will be prefix-indexed; all other fields will be indexed in normal mode. The value format is a comma-separated list of field names.


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
index products {
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

The list of full-text fields to limit infix indexing to. Applies to [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict) only. Optional, default is empty (index all fields in infix mode).

Similar to [prefix_fields](Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields), but lets you limit infix-indexing to given fields.


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
index products {
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

Maximum substring (either prefix or infix) length to index. Optional, default is 0 (do not limit indexed substrings). Applies to [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict) only.

By default, substring (either prefix or infix) indexing in the [dict](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict) will index **all** possible substrings as separate keywords. That might result in an overly large index. So this directive lets you limit the impact of substring indexing by skipping too-long substrings (which, chances are, will never get searched for anyway).

For example, a test index of 10,000 blog posts takes this much disk space depending on the settings:
* 6.4 MB baseline (no substrings)
* 24.3 MB (3.8x) with min_prefix_len = 3
* 22.2 MB (3.5x) with min_prefix_len = 3, max_substring_len = 8
* 19.3 MB (3.0x) with min_prefix_len = 3, max_substring_len = 6
* 94.3 MB (14.7x) with min_infix_len = 3
* 84.6 MB (13.2x) with min_infix_len = 3, max_substring_len = 8
* 70.7 MB (11.0x) with min_infix_len = 3, max_substring_len = 6

So in this test limiting the max substring length saved us 10-15% on the index size.

There is no performance impact associated with substring length when using dict=keywords mode, so this directive is not applicable and intentionally forbidden in that case. If required, you can still limit the length of a substring that you search for in the application code.


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
index products {
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

Expand keywords with exact forms and/or stars when possible. The value can additionally enumerate options such us `exact` and `star`. Optional, default is 0 (do not expand keywords).

Queries against indexes with `expand_keywords` feature enabled are internally expanded as follows. If the index was built with prefix or infix indexing enabled, every keyword gets internally replaced with a disjunction of keyword itself and a respective prefix or infix (keyword  with stars). If the index was built with both stemming and [index_exact_words](Creating_an_index/NLP_and_tokenization/Morphology.md#index_exact_words)  enabled, exact form is also added. Here's an example that shows how internal expansion works when all of the above (infixes, stemming, and exact words) are combined:

```ini
running -> ( running | *running* | =running )
```

(as `expand_keywords =  1` or `expand_keywords = star,exact`) or expansion limited by exact option even infixes enabled for index

```ini
running -> ( running | =running )
```

(as `expand_keywords = exact`)

Expanded queries take naturally longer to complete, but can possibly improve the search quality, as the documents with exact form matches should be ranked generally higher than documents with stemmed or infix matches.

Note that the existing query syntax does not allow to emulate this kind of expansion, because internal expansion works on keyword level and expands keywords within phrase or quorum operators too (which is not possible through the query syntax).

This directive does not affect [indexer](Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool) in any way, it only affects [searchd](Starting_the_server/Manually.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) expand_keywords = '1'
```

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=
create table products(title text, price float) expand_keywords = '1'"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
             'expand_keywords' => '1'
        ],
        'columns' => [
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ]
    ],
    'index' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```

<!-- request CONFIG -->

```ini
index products {
  expand_keywords = 1

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## expansion_limit

```ini
expansion_limit = number
```

Maximum number of expanded keywords for a single wildcard. Details are [here](Server_settings/Searchd.md#expansion_limit).
