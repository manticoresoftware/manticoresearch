# Low-level tokenization  

When indexing some text Manticore splits it into words, and does case folding so that e.g. "Abc", "ABC" and "abc" would be treated as the same word.

To do that properly Manticore needs to know:
* what encoding is the source text in (and it should always be UTF-8)
* what characters are letters and what are not
* what letters should be folded to other letters

This can be configured on a per-index basis using [charset_table](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#charset_table) option. charset_table specifies the array that maps letter characters to their case folded versions (or any other characters if you like). The characters that are not in the array are considered to be non-letters and will be treated as word separators when indexing or searching through this index.

The default character set is `non_cjk` and includes [most languages](../../Creating_an_index/NLP_and_tokenization/Supported_languages.md).

You can also specify text pattern replacement rules. For example, given the rules:

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

text `RED TUBE 5" LONG` would be indexed as `COLOR TUBE 5 INCH LONG`, and `PLANK 2" x 4"` - as `PLANK 2 INCH x 4 INCH`. The rules are applied in the given order. Text in queries is also replaced; search for `BLUE TUBE` would actually become a search for `COLOR TUBE`.

Read more about [regexp_filter here](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

## Index configuration options

### charset_table

```ini
# default
charset_table = non_cjk

# only English and Russian letters
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# english charset defined with alias
charset_table = 0..9, english, _

# you can override character mappings by redefining them, e.g. for case insensitive search with German umlauts you can use:
charset_table = non_cjk, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- example charset_table -->
Accepted characters array, with case folding rules. Optional, default values are all characters of most non-CJK languages (`non_cjk`).

`charset_table` is a main workhorse of Manticore tokenization process, the process of extracting keywords from document text or query text. It controls what characters are accepted as valid and what are not, and how the accepted characters should be transformed (e.g. should the case be removed or not).

You can think of `charset_table` as of a big table or array that has a mapping for each and every of 100K+ characters in Unicode. By default, every character maps to 0, which means that it does not occur within keywords and should be treated as a separator. Once mentioned in the table, character is mapped to some other character (most frequently, either to itself or to a lowercase letter), and is treated as a valid keyword part.

The expected value format is a comma-separated list of mappings. Two simplest mappings simply declare a character as valid, and map a single character to another single character, respectively. But specifying the whole table in such form would result in bloated and barely manageable specifications. So there are several syntax shortcuts that let you map ranges of characters at once. The complete list is as follows:

* `A->a` - Single char mapping, declares source char 'A' as allowed to occur within keywords and maps it to destination char 'a' (but does *not* declare 'a' as allowed).
* `A..Z->a..z` - Range mapping, declares all chars in source range as allowed and maps them to the destination range. Does *not* declare destination range as allowed. Also checks range's lengths (the lengths must be equal).
* `a` - Stray char mapping, declares a character as allowed and maps it to itself. Equivalent to a->a single char mapping.
* `a..z` - Stray range mapping, declares all characters in range as allowed and maps them to themselves. Equivalent to `a..z->a..z` range mapping.
* `A..Z/2` - Checkerboard range map. Maps every pair of chars to the second char. More formally, declares odd characters in range as allowed and maps them to the even ones; also declares even characters as allowed and maps them to themselves. For instance, `A..Z/2` is equivalent to `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`. This mapping shortcut is helpful for a number of Unicode blocks where uppercase and lowercase letters go in such interleaved order instead of contiguous chunks.

Control characters with codes from 0 to 32 are always treated as separators. Characters with codes 33 to 127, i.e. 7-bit ASCII characters, can be used in the mappings as is. To avoid configuration file encoding issues, 8-bit ASCII characters and Unicode characters must be specified in `U+xxx` form, where `xxx` is hexadecimal codepoint number. This form can also be used for 7-bit ASCII characters to encode special ones: e.g. use `U+2E` to encode dot, `U+2C` to encode comma. The minimal accepted unicode character code is `U+0021`.


You can redefine character mapping by specifying it again with another mapping. For example built-in array `non_cjk` includes characters `Ä` and `ä` and maps them both to ascii character `a` which may not work in some cases (e.g. for the German language). In this case you can redefine the characters so:

```
charset_table = non_cjk,U+00E4,U+00C4
```

for case sensitive search or

```
characters = non_cjk,U+00E4,U+00C4->U+00E4
```

for case insensitive search.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
```
<!-- request CONFIG -->

```ini
index products {
  charset_table = 0..9, A..Z->a..z, _, a..z, \
    U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example charset_table 2 -->
Besides definitions of characters and mappings, there are several built-in aliases that can be used. Current aliases are:
* `english`
* `russian`
* `non_cjk`
* `cjk`

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => '0..9, english, _'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
```
<!-- request CONFIG -->

```ini
index products {
  charset_table = 0..9, english, _

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

So if you want your search to support different languages you will need to define sets of valid characters and folding rules for all of them what can be quite a laborious task. We have performed this task for you by preparing default charset tables, non_cjk and cjk, that comprise non-cjk and cjk-languages respectively. These charsets should be sufficient to use in most cases.

The languages that are currently **not** supported are:
* Assamese
* Bishnupriya
* Buhid
* Garo
* Hmong
* Ho
* Komi
* Large Flowery Miao
* Maba
* Maithili
* Marathi
* Mende
* Mru
* Myene
* Ngambay
* Odia
* Santali
* Sindhi
* Sylheti

All other languages listed in the following list are supported by default: [Unicode languages
list](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/).

<!-- example charset_table 3 -->
To be able to work with both cjk and non-cjk languages you should set the options in your configuration file as shown below (with an [exception](../../Creating_an_index/NLP_and_tokenization/CJK.md) for Chinese):

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cjk',
             'ngram_len' => '1',
             'ngram_chars' => 'cjk'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cjk\' ngram_len = \'1\' ngram_chars = \'cjk\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cjk\' ngram_len = \'1\' ngram_chars = \'cjk\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'");
```
<!-- request CONFIG -->

```ini
index products {
  charset_table       = non_cjk
  ngram_len           = 1
  ngram_chars         = cjk

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

In case you don't need support for cjk-languages you can just omit [ngram_len](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) and [ngram_chars](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
options. For more information on those see the appropriate documentation sections.

If you're looking for mapping one character to multiple or vice-versa [regexp_filter](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) can be helpful.

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
Blended characters list. Optional, default is empty.

Blended characters are indexed both as separators and valid characters. For instance, assume that `&` is configured as blended and `AT&T` occurs in an indexed document. Three different keywords will get indexed, namely `at&t`, treating blended characters as valid, plus `at` and `t`, treating them as separators.

Blended characters should be used carefully:
* since as soon as a character is defined as blended it is not a separator any more which can affect search. For example if you put a comma to the `blend_chars` and then search for `dog,cat` it will treat that as a single token `dog,cat` and if during indexation you **didn't** index `dog,cat` as `dog,cat`, but left only `dog cat` then it won't be matched.
* therefore you need to make sure that this behaviour is desired and control it with help of the other setting [blend_mode](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)

Positions for tokens obtained by replacing blended characters with whitespace are assigned as usual, so regular keywords will be indexed just as if there was no `blend_chars` specified at all. An additional token that mixes blended and non-blended characters will be put at the starting position. For instance, if `AT&T company` occurs in the very beginning of the text field, `at` will be given position 1, `t` position 2, `company` position 3, and `AT&T` will also be given position 1 ("blending" with the opening regular keyword). Thus, querying for either `AT&T` or just `AT` will match that document, and querying for `"AT T"` as a phrase will also match it. Last but not least, phrase query for `"AT&T company"` will *also* match it, despite the position.

Blended characters can overlap with special characters used in query syntax (think of `T-Mobile` or `@twitter`). Where possible, query parser will automatically handle blended character as blended. For instance, `"hello @twitter"` within quotes (a phrase operator) would handle @-sign as blended, because @-syntax for field operator is not allowed within phrases. Otherwise, the character would be handled as an operator. So you might want to escape the keywords.

Blended characters can be remapped, so that multiple different blended characters could be normalized into just one base form. This is useful when indexing multiple alternative Unicode codepoints with equivalent glyphs.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_chars' => '+, &, U+23, @->_'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
```

<!-- request CONFIG -->

```ini
index products {
  blend_chars = +, &, U+23, @->_

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### blend_mode

```ini
blend_mode = option [, option [, ...]]
option = trim_none | trim_head | trim_tail | trim_both | trim_all | skip_pure
```

<!-- example blend_mode -->
Blended tokens indexing mode. Optional, default is `trim_none`.

By default, tokens that mix blended and non-blended characters get indexed in there entirety. For instance, when both at-sign and an exclamation are in `blend_chars`, `@dude!` will get result in two tokens indexed: `@dude!` (with all the blended characters) and `dude` (without any). Therefore `@dude` query will *not* match it.

`blend_mode` directive adds flexibility to this indexing behavior. It takes a comma-separated list of options.

Options specify token indexing variants. If multiple options are specified, multiple variants of the same token will be indexed. Regular keywords (resulting from that token by replacing blended with a separator) are always indexed.

* `trim_none` - Index the entire token
* `trim_head` - Trim heading blended characters, and index the resulting token
* `trim_tail` - Trim trailing blended characters, and index the resulting token
* `trim_both` - Trim both heading and trailing blended characters, and index the resulting token
* `trim_all` - Trim heading, trailing and middle blended characters, and index the resulting token
* `skip_pure` - Do not index the token if it's purely blended, that is, consists of blended characters only

Returning to the `@dude!` example above, setting `blend_mode = trim_head, trim_tail` will result in two tokens being indexed, `@dude` and `dude!`. In this particular example, `trim_both` would have no effect, because trimming both blended characters results in `dude` which is already indexed as a regular keyword. Indexing `@U.S.A.` with `trim_both` (and assuming that dot is blended two) would result in `U.S.A` being indexed. Last but not least, `skip_pure` enables you to fully ignore sequences of blended characters only. For example, `one @@@ two` would be indexed exactly as `one two`, and match that as a phrase. That is not the case by default because a fully blended token gets indexed and offsets the second keyword position.

Default behavior is to index the entire token, equivalent to `blend_mode = trim_none`.

Make sure you undestand that either of the blend modes limits your search, even the default one `trim_none` as with it and assuming `.` is a blended char:
* `.dog.` will become `.dog. dog` during indexation
* and you won't be able to find it by `dog.`.

The more modes you use, the higher the chance your keyword will match something.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_mode' => 'trim_tail, skip_pure',
            'blend_chars' => '+, &'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
```

<!-- request CONFIG -->

```ini
index products {
  blend_mode = trim_tail, skip_pure
  blend_chars = +, &

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### min_word_len

```ini
min_word_len = length
```

<!-- example min_word_len -->
Minimum indexed word length. Optional, default is 1 (index everything).

Only those words that are not shorter than this minimum will be indexed. For instance, if min_word_len is 4, then 'the' won't be indexed, but 'they' will be.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_word_len = '4'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) min_word_len = '4'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_word_len' => '4'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
```

<!-- request CONFIG -->

```ini
index products {
  min_word_len = 4

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_len

```ini
ngram_len = 1
```

<!-- example ngram_len -->
N-gram lengths for N-gram indexing. Optional, default is 0 (disable n-gram indexing). Known values are 0 and 1.

N-grams provide basic CJK (Chinese, Japanese, Korean) support for unsegmented texts. The issue with CJK searching is that there could be no clear separators between the words. In some cases you might not want to use dictionary-based segmentation as [the one available for Chinese](../../Creating_an_index/NLP_and_tokenization/CJK.md). In those cases n-gram segmentation might work well too.

When this feature is enabled, streams of CJK (or any other defined in [ngram_chars](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)) characters are indexed as N-grams. For example, if incoming text is "ABCDEF" (where A to F represent some CJK characters) and ngram_len is 1, in will be indexed as if it was "A B C D E F". Only ngram_len=1 is supported at the moment. Only those characters that are listed in [ngram_chars](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) table will be split this way; other ones will not be affected.

Note that if search query is segmented, i.e. there are separators between individual words, then wrapping the words in quotes and using extended mode will result in proper matches being found even if the text was **not** segmented. For instance, assume that the original query is `BC DEF`. After wrapping in quotes on the application side, it should look like `"BC" "DEF"` (*with* quotes). This query will be passed to Manticore and internally split into 1-grams too, resulting in `"B C" "D E F"` query, still with quotes that are the phrase matching operator. And it will match the text even though there were no separators in the text.

Even if the search query is not segmented, Manticore should still produce good results, thanks to phrase based ranking: it will pull closer phrase matches (which in case of N-gram CJK words can mean closer multi-character word matches) to the top.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cjk' ngram_len = '1'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cjk' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cjk',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cjk\' ngram_len = \'1\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cjk\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cjk' ngram_len = '1'");
```

<!-- request CONFIG -->

```ini
index products {
  ngram_chars = cjk
  ngram_len = 1

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_chars

```ini
ngram_chars = cjk

ngram_chars = cjk, U+3000..U+2FA1F
```

<!-- example ngram_chars -->
N-gram characters list. Optional, default is empty.

To be used in conjunction with in [ngram_len](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_len), this list defines characters, sequences of which are subject to N-gram extraction. Words comprised of other characters will not be affected by N-gram indexing feature. The value format is identical to [charset_table](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#charset_table). N-gram characters cannot appear in the [charset_table](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#charset_table).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'U+3000..U+2FA1F',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
```
<!-- request CONFIG -->

```ini
index products {
  ngram_chars = U+3000..U+2FA1F
  ngram_len = 1

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
res = async utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
```

<!-- end -->

<!-- example ngram_chars 2 -->
Also you can use an alias for our default N-gram table as in the example. It should be sufficient in most cases.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cjk' ngram_len = '1'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cjk' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cjk',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cjk\' ngram_len = \'1\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cjk\' ngram_len = \'1\'');
```
<!-- request CONFIG -->

```ini
index products {
  ngram_chars = cjk
  ngram_len = 1

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ignore_chars

```ini
ignore_chars = U+AD
```

<!-- example ignore_chars -->
Ignored characters list. Optional, default is empty.

Useful in cases when some characters, such as soft hyphenation mark (U+00AD), should be not just treated as separators but rather fully ignored. For example, if '-' is simply not in the charset_table, "abc-def" text will be indexed as "abc" and "def" keywords. On the contrary, if '-' is added to ignore_chars list, the same text will be indexed as a single "abcdef" keyword.

The syntax is the same as for [charset_table](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#charset_table), but it's only allowed to declare characters, and not allowed to map them. Also, the ignored characters must not be present in charset_table.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'ignore_chars' => 'U+AD'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
```

<!-- request CONFIG -->

```ini
index products {
  ignore_chars = U+AD

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_index

```ini
bigram_index = {none|all|first_freq|both_freq}
```

<!-- example bigram_index -->
Bigram indexing mode. Optional, default is none.

Bigram indexing is a feature to accelerate phrase searches. When indexing, it stores a document list for either all or some of the adjacent words pairs into the index. Such a list can then be used at searching time to significantly accelerate phrase or sub-phrase matching.

`bigram_index` controls the selection of specific word pairs. The known modes are:

* `all`, index every single word pair
* `first_freq`, only index word pairs where the *first* word is in a list of frequent words (see [bigram_freq_words](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). For example, with `bigram_freq_words = the, in, i, a`, indexing "alone in the dark" text will result in "in the" and "the dark" pairs being stored as bigrams, because they begin with a frequent keyword (either "in" or "the" respectively), but "alone in" would **not** be indexed, because "in" is a *second* word in that pair.
* `both_freq`, only index word pairs where both words are frequent. Continuing with the same example, in this mode indexing "alone in the dark" would only store "in the" (the very worst of them all from searching perspective) as a bigram, but none of the other word pairs.

For most use cases, `both_freq` would be the best mode, but your mileage may vary.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'both_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
```

<!-- request CONFIG -->

```ini
index products {
  bigram_index = both_freq
  bigram_freq_words = the, a, you, i

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_freq_words

```ini
bigram_freq_words = the, a, you, i
```

<!-- example bigram_freq_words -->
A list of keywords considered "frequent" when indexing bigrams. Optional, default is empty.

Some of the bigram indexing modes (see [bigram_index](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) require to define a list of frequent keywords. These are **not** to be confused with stop words. Stop words are completely eliminated when both indexing and searching. Frequent keywords are only used by bigrams to determine whether to index a current word pair or not.

`bigram_freq_words` lets you define a list of such keywords.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'first_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
```

<!-- request CONFIG -->

```ini
index products {
  bigram_freq_words = the, a, you, i
  bigram_index = first_freq

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### dict

```ini
dict = {keywords|crc}
```

<!-- example dict -->
The keywords dictionary type. Known values are 'crc' and 'keywords'. Optional, default is 'keywords'.

Keywords dictionary mode (dict=keywords), (greatly) reduces indexing impact and enable substring searches on huge collections. That mode is supported both for plain and RT indexes.

CRC dictionaries never store the original keyword text in the index. Instead, keywords are replaced with their control sum value (calculated using FNV64) both when searching and indexing, and that value is used internally in the index.

That approach has two drawbacks. First, there is a chance of control sum collision between several pairs of different keywords, growing quadratically with the number of unique keywords in the index. However, it is not a big concern as a chance of a single FNV64 collision in a dictionary of 1 billion entries is approximately 1:16, or 6.25 percent. And most dictionaries will be much more compact that a billion keywords, as a typical spoken human language has in the region of 1 to 10 million word forms.) Second, and more importantly, substring searches are not directly possible with control sums. Manticore alleviated that by pre-indexing all the possible substrings as separate keywords (see [min_prefix_len](../../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) directives). That actually has an added benefit of matching substrings in the quickest way possible. But at the same time pre-indexing all substrings grows the index size a lot (factors of 3-10x and even more would not be unusual) and impacts the indexing time respectively, rendering substring searches on big indexes rather impractical.

Keywords dictionary fixes both these drawbacks. It stores the keywords in the index and performs search-time wildcard expansion. For example, a search for a 'test\*'prefix could internally expand to 'test|tests|testing' query based on the dictionary contents. That expansion is fully transparent to the application, except that the separate per-keyword statistics for all the actually matched keywords would now also be reported.

For substring (infix) search extended wildcards may be used. Special symbols like '?' and '%' are supported along with substring (infix) search (e.g. "t?st\*","run%","\*abc\*"). Note, however, these wildcards work only with dict=keywords, and not elsewhere.

Indexing with keywords dictionary should be 1.1x to 1.3x slower compared to regular, non-substring indexing - but times faster compared to substring indexing (either prefix or infix). Index size should only be slightly bigger that than of the regular non-substring index, with a 1..10% percent total difference. Regular keyword searching time must be very close or identical across all three discussed index kinds (CRC non-substring, CRC substring, keywords). Substring searching time can vary greatly depending on how many actual keywords match the given substring (in other words, into how many keywords does the search term expand). The maximum number of keywords matched is restricted by the [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) directive.

Essentially, keywords and CRC dictionaries represent the two different trade-off substring searching decisions. You can choose to either sacrifice indexing time and index size in favor of top-speed worst-case searches (CRC dictionary), or only slightly impact indexing time but sacrifice worst-case searching time when the prefix expands into very many keywords (keywords dictionary).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) dict = 'keywords'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) dict = 'keywords'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'dict' => 'keywords'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
```

<!-- request CONFIG -->

```ini
index products {
  dict = keywords

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### embedded_limit

```ini
embedded_limit = size
```

<!-- example embedded_limit -->
Embedded exceptions, wordforms, or stop words file size limit. Optional, default is 16K.

When you create an index the above mentioned files can be either saved externally along with the index or embedded directly into the index. Files sized under `embedded_limit` get stored into the index. For bigger files, only the file names are stored. This also simplifies moving index files to a different machine; you may get by just copying a single file.

With smaller files, such embedding reduces the number of the external files on which the index depends, and helps maintenance. But at the same time it makes no sense to embed a 100 MB wordforms dictionary into a tiny delta index. So there needs to be a size threshold, and `embedded_limit` is that threshold.

<!-- request CONFIG -->

```ini
index products {
  embedded_limit = 32K

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### global_idf

```ini
global_idf = /path/to/global.idf
```

<!-- example global_idf -->
The path to a file with global (cluster-wide) keyword IDFs. Optional, default is empty (use local IDFs).

On a multi-index cluster, per-keyword frequencies are quite likely to differ across different indexes. That means that when the ranking function uses TF-IDF based values, such as BM25 family of factors, the results might be ranked slightly differently depending on what cluster node they reside.

The easiest way to fix that issue is to create and utilize a global frequency dictionary, or a global IDF file for short. This directive lets you specify the location of that file. It is suggested (but not required) to use an .idf extension. When the IDF file is specified for a given index *and* [OPTION global_idf](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#global_idf) is set to 1, the engine will use the keyword frequencies and collection documents counts from the global_idf file, rather than just the local index. That way, IDFs and the values that depend on them will stay consistent across the cluster.

IDF files can be shared across multiple indexes. Only a single copy of an IDF file will be loaded by `searchd`, even when many indexes refer to that file. Should the contents of an IDF file change, the new contents can be loaded with a SIGHUP.

You can build an .idf file using [indextool](../../Miscellaneous_tools.md#indextool) utility, by dumping dictionaries using `--dumpdict dict.txt --stats` switch first, then converting those to .idf format using `--buildidf`, then merging all the .idf files across cluster using `--mergeidf`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'global_idf' => '/usr/local/manticore/var/global.idf'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
```


<!-- request CONFIG -->

```ini
index products {
  global_idf = /usr/local/manticore/var/global.idf

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### hitless_words

```ini
hitless_words = {all|path/to/file}
```

<!-- example hitless_words -->
Hitless words list. Optional, allowed values are 'all', or a list file name.

By default, Manticore full-text index stores not only a list of matching documents for every given keyword, but also a list of its in-document positions (known as hitlist). Hitlists enables phrase, proximity, strict order and other advanced types of searching, as well as phrase proximity ranking. However, hitlists for specific frequent keywords (that can not be stopped for some reason despite being frequent) can get huge and thus slow to process while querying. Also, in some cases we might only care about boolean keyword matching, and never need position-based searching operators (such as phrase matching) nor phrase ranking.

`hitless_words` lets you create indexes that either do not have positional information (hitlists) at all, or skip it for specific keywords.

Hitless index will generally use less space than the respective regular index (about 1.5x can be expected). Both indexing and searching should be faster, at a cost of missing positional query and ranking support.  

If used in positional queries (e.g. phrase queries) the hitless words are taken out from them and used as operand without a position.  For example if "hello" and "world" are hitless and "simon" and "says" are not hitless, the phrase query  `"simon says hello world"` will be converted to `("simon says" & hello & world)`, matching "hello" and "world" anywhere in the document and "simon says" as an exact phrase.

A positional query than contains only hitless words will result in an empty phrase node, therefore the entire query will return an empty result and a warning. If the whole dictionary is hitless (using `all`) only boolean matching can be used on the respective index.



<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words = 'all'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) hitless_words = 'all'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words' => 'all'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
```

<!-- request CONFIG -->

```ini
index products {
  hitless_words = all

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_field_lengths

```ini
index_field_lengths = {0|1}
```

<!-- example index_field_lengths -->
Enables computing and storing of field lengths (both per-document and average per-index values) into the index. Optional, default is 0 (do not compute and store).

When `index_field_lengths` is set to 1 Manticore will:
* create a respective length attribute for every full-text field, sharing the same name but with `__len` suffix
* compute a field length (counted in keywords) for every document and store in to a respective attribute
* compute the per-index averages. The lengths attributes will have a special TOKENCOUNT type, but their values are in fact regular 32-bit integers, and their values are generally accessible.

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) and [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) functions in the expression ranker are based on these lengths and require `index_field_lengths` to be enabled. Historically, Manticore used a simplified, stripped-down variant of BM25 that, unlike the complete function, did **not** account for document length. There's also support for both a complete variant of BM25, and its extension towards multiple fields, called BM25F. They require per-document length and per-field lengths, respectively. Hence the additional directive.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_field_lengths = '1'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) index_field_lengths = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_field_lengths' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
```
<!-- request CONFIG -->

```ini
index products {
  index_field_lengths = 1

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_token_filter

```ini
index_token_filter = my_lib.so:custom_blend:chars=@#&
```

<!-- example index_token_filter -->
Index-time token filter for index. Optional, default is empty.

Index-time token filter gets created by indexer on indexing source data into a plain index or by RT index on processing `INSERT` or `REPLACE` statements and lets you implement a custom tokenizer that makes tokens according to custom rules. The plugins are defined as `library name:plugin name:optional string of settings`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_token_filter' => 'my_lib.so:custom_blend:chars=@#&'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
```

<!-- request CONFIG -->

```ini
index products {
  index_token_filter = my_lib.so:custom_blend:chars=@#&

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### overshort_step

```ini
overshort_step = {0|1}
```

<!-- example overshort_step -->
Position increment on overshort (less than [min_word_len](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) keywords. Optional, allowed values are 0 and 1, default is 1.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) overshort_step = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'overshort_step' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
```

<!-- request CONFIG -->

```ini
index products {
  overshort_step = 1

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary

```ini
phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis
```

<!-- example phrase_boundary -->
Phrase boundary characters list. Optional, default is empty.

This list controls what characters will be treated as phrase boundaries, in order to adjust word positions and enable phrase-level search emulation through proximity search. The syntax is similar to [charset_table](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#charset_table), but mappings are not allowed and the boundary characters must not overlap with anything else.

On phrase boundary, additional word position increment (specified by [phrase_boundary_step](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)) will be added to current word position. This enables phrase-level searching through proximity queries: words in different phrases will be guaranteed to be more than phrase_boundary_step distance away from each other; so proximity search within that distance will be equivalent to phrase-level search.

Phrase boundary condition will be raised if and only if such character is followed by a separator; this is to avoid abbreviations such as S.T.A.L.K.E.R or URLs being treated as several phrases.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary' => '., ?, !, U+2026',
             'phrase_boundary_step' => '10'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
```
<!-- request CONFIG -->

```ini
index products {
  phrase_boundary = ., ?, !, U+2026
  phrase_boundary_step = 10

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary_step

```ini
phrase_boundary_step = 100
```

<!-- example phrase_boundary_step -->
Phrase boundary word position increment. Optional, default is 0.

On phrase boundary, current word position will be additionally incremented by this number.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary_step' => '100',
             'phrase_boundary' => '., ?, !, U+2026'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
```

<!-- request CONFIG -->

```ini
index products {
  phrase_boundary_step = 100
  phrase_boundary = ., ?, !, U+2026

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### regexp_filter

```ini
# index '13"' as '13inch'
regexp_filter = \b(\d+)\" => \1inch

# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color
```

<!-- example regexp_filter -->
Regular expressions (regexps) to filter the fields and queries with. Optional, multi-value, default is an empty list of regexps.

In certain applications (like product search) there can be many different ways to call a model, or a product, or a property, and so on. For instance, 'iphone 3gs' and 'iphone 3 gs' (or even 'iphone3 gs') are very likely to mean the same product. Or, for a more tricky example, '13-inch', '13 inch', '13"', and '13in' in a laptop screen size descriptions do mean the same.

Regexps provide you with a mechanism to specify a number of rules specific to your application to handle such cases. In the first 'iphone 3gs' example, you could possibly get away with a wordforms files tailored to handle a handful of iPhone models. However even in a comparatively simple second '13-inch' example there is just way too many individual forms and you are better off specifying rules that would normalize both '13-inch' and '13in' to something identical.

Regular expressions listed in `regexp_filter` are applied in the order they are listed. That happens at the earliest stage possible, before any other processing, even before tokenization. That is, regexps are applied to the raw source fields when indexing, and to the raw search query text when searching.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'regexp_filter' => '(blue|red) => color'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'");
```

<!-- request CONFIG -->

```ini
index products {
  # index '13"' as '13inch'
  regexp_filter = \b(\d+)\" => \1inch

  # index 'blue' or 'red' as 'color'
  regexp_filter = (blue|red) => color

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
