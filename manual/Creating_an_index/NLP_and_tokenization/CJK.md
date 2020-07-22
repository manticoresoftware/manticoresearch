# Chinese, Japanese and Korean (CJK) languages

Manticore has built-in support for indexing CJK texts. There are two ways how CJK text can be processed:

<!-- example cjk 1 -->
* Precise segmentation using ICU library (only Chinese is supported for now)


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) charset_table = 'cjk' morphology = 'icu_chinese'
```

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=
create table products(title text, price float) charset_table = 'cjk' morphology = 'icu_chinese'"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'charset_table' => 'cjk',
            'morphology' => 'icu_chinese'
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
  charset_table = cjk
  morphology = icu_chinese
  
  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example cjk 2 -->
* Basic support with N-grams options [ngram_len](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) and [ngram_chars](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
There are separate charset tables(`chinese`, `korean`, `japanese`) that can be used for each CJK-language or, alternatively, common `cjk` charset table can be applied. 


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'
```

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=
create table products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
             'charset_table' => 'non_cjk',
             'ngram_len' => '1',
             'ngram_chars' => 'cjk'
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
  charset_table = non_cjk
  ngram_len = 1
  ngram_chars = cjk

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example cjk 3 -->
There's also built-in [stopwords](Creating_an_index/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) for Chinese with alias `zh`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'
```

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=
create table products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'charset_table' => 'chinese',
            'morphology' => 'icu_chinese',
            'stopwords' => 'zh'
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
  charset_table = chinese
  morphology = icu_chinese
  stopwords = zh
  
  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
