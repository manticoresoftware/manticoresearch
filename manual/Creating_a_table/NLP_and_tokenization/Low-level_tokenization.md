# Low-level tokenization  

When text is indexed in Manticore, it is split into words and case folding is done so that words like "Abc", "ABC", and "abc" are treated as the same word.

To perform these operations correctly, Manticore must know:
* the encoding of the source text (which should always be UTF-8)
* which characters are considered letters and which are not
* which letters should be folded to other letters

You can configure these settings on a per-table basis using the  [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) option. charset_table specifies an array that maps letter characters to their case-folded versions (or any other characters that you prefer). Characters that are not present in the array are considered to be non-letters and will be treated as word separators during indexing or searching in this table.

The default character set is `non_cont`, which includes [most languages](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md).

You can also define text pattern replacement rules. For example, with the following rules:

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

The text `RED TUBE 5" LONG` would be indexed as `COLOR TUBE 5 INCH LONG`, and `PLANK 2" x 4"` would be indexed as `PLANK 2 INCH x 4 INCH`. These rules are applied in the specified order. The rules also apply to queries, so a search for `BLUE TUBE` would actually search for `COLOR TUBE`.

You can learn more about [regexp_filter here](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

## Index configuration options

### charset_table

```ini
# default
charset_table = non_cont

# only English and Russian letters
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# english charset defined with alias
charset_table = 0..9, english, _

# you can override character mappings by redefining them, e.g. for case insensitive search with German umlauts you can use:
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- example charset_table -->
`charset_table` specifies an array that maps letter characters to their case-folded versions (or any other characters if you prefer). The default character set is `non_cont`, which includes most languages with [non-continuous](https://en.wikipedia.org/wiki/Scriptio_continua) scripts.

`charset_table` is a workhorse of Manticore's tokenization process, which extracts keywords from document text or query text. It controls what characters are accepted as valid and how they should be transformed (e.g. whether case should be removed or not).

By default, every character maps to 0, which means that it is not considered a valid keyword and is treated as a separator. Once a character is mentioned in the table, it is mapped to another character (most frequently, either to itself or to a lowercase letter) and is treated as a valid keyword part.

charset_table uses a comma-separated list of mappings to declare characters as valid or to map them to other characters. Syntax shortcuts are available for mapping ranges of characters at once:

* Single char mapping: `A->a`. Declares the source character 'A' as allowed within keywords and maps it to the destination character 'a' (but does not declare 'a' as allowed).
* Range mapping: `A..Z->a..z`. Declares all characters in the source range as allowed and maps them to the destination range. Does not declare the destination range as allowed. Checks the lengths of both ranges.
* Stray char mapping: `a`. Declares a character as allowed and maps it to itself. Equivalent to `a->a` single char mapping.
* Stray range mapping: `a..z`.  Declares all characters in the range as allowed and maps them to themselves. Equivalent to `a..z->a..z` range mapping.
* Checkerboard range mapping: `A..Z/2`. Maps every pair of characters to the second character. For instance, `A..Z/2` is equivalent to `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`. This mapping shortcut is helpful for Unicode blocks where uppercase and lowercase letters go in an interleaved order.

For characters with codes from 0 to 32, and those in the range of 127 to 8-bit ASCII and Unicode characters, Manticore always treats them as separators. To avoid configuration file encoding issues, 8-bit ASCII characters and Unicode characters must be specified in `U+XXX` form, where `XXX` is a hexadecimal code point number. The minimal accepted Unicode character code is `U+0021`.

If the default mappings are insufficient for your needs, you can redefine the character mappings by specifying them again with another mapping. For example, if the built-in `non_cont` array includes characters `Ä` and `ä` and maps them both to the ASCII character `a`, you can redefine those characters by adding the Unicode code points for them, like this:

```
charset_table = non_cont,U+00E4,U+00C4
```

for case sensitive search or

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

for case insensitive search.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = 0..9, A..Z->a..z, _, a..z, \
    U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example charset_table 2 -->
Besides definitions of characters and mappings, there are several built-in aliases that can be used. Current aliases are:
* `chinese`
* `cjk`
* `cont`
* `english`
* `japanese`
* `korean`
* `non_cont` (`non_cjk`)
* `russian`
* `thai`

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = 0..9, english, _

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

If you want to support different languages in your search, it can be a laborious task to define sets of valid characters and folding rules for all of them. We have simplified this for you by providing default charset tables, `non_cont` and `cont`, that cover languages with non-continuous and continuous (Chinese, Japanese, Korean, Thai) scripts, respectively. In most cases, these charsets should be sufficient for your needs.

Please note that the following languages are currently **not** supported:
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

All other languages listed in the [Unicode languages
list](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/) are supported by default.

<!-- example charset_table 3 -->
To work with both cont and non-cont languages, set the options in your configuration file as shown below (with an [exception](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) for Chinese):

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cont',
             'ngram_len' => '1',
             'ngram_chars' => 'cont'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table       = non_cont
  ngram_len           = 1
  ngram_chars         = cont

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

If you do not require support for continuous-script languages, you can simply exclude the [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) and [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars).
options. For more information on these options, refer to the corresponding documentation sections.

To map one character to multiple characters or vice versa, you can use [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) can be helpful.

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
Blended characters list. Optional, default is empty.

Blended characters are indexed as both separators and valid characters. For example, when `&` is defined as a blended character and `AT&T` appears in an indexed document, three different keywords will be indexed, `at&t`, `at` and `t`.

Additionally, blended characters can influence indexing in such a way that keywords are indexed as if the blended characters were not typed at all. This behavior is particularly evident when `blend_mode = trim_all` is specified. For example, the phrase `some_thing` will be indexed as `some`, `something`, and `thing` with `blend_mode = trim_all`.

Care should be taken when using blended characters as defining a character as blended means that it is no longer a separator.
* Therefore, if you put a comma to the `blend_chars` and search for `dog,cat`, it will treat that as a single token `dog,cat`. If `dog,cat` was **not** indexed as `dog,cat`, but left as `dog cat` only, then it will not match.
* Hence, this behavior should be controlled with the [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) setting.

Positions for tokens obtained by replacing blended characters with whitespace are assigned as usual, and regular keywords will be indexed as if there were no `blend_chars` specified at all. An additional token that mixes blended and non-blended characters will be put at the starting position. For instance, if `AT&T company` occurs in the very beginning of the text field, `at` will be given position 1, `t` position 2, `company` position 3, and `AT&T` will also be given position 1, blending with the opening regular keyword. As a result, queries for `AT&T` or just `AT` will match that document. A phrase query for `"AT T"` will also match, as well as a phrase query for `"AT&T company"`.

Blended characters can overlap with special characters used in query syntax, such as `T-Mobile` or `@twitter`. Where possible, the query parser will handle the blended character as blended. For instance, if `hello @twitter` is within quotes (a phrase operator), the query parser will handle the `@` symbol as blended. However, if the `@` symbol was not within quotes, the character would be handled as an operator. Therefore, it is recommended to escape keywords.

Blended characters can be remapped so that multiple different blended characters can be normalized into one base form. This is useful when indexing multiple alternative Unicode codepoints with equivalent glyphs.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
```

<!-- request CONFIG -->

```ini
table products {
  blend_chars = +, &, U+23, @->_

  type = rt
  path = tbl
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
The blended tokens indexing mode is enabled by the blend_mode directive.

By default, tokens that mix blended and non-blended characters get indexed entirely. For example, when both an at-sign and an exclamation are in `blend_chars`, the string `@dude!` will be indexed as two tokens: `@dude!` (with all the blended characters) and `dude` (without any). As a result, a query of `@dude` will **not** match it.

`blend_mode` adds flexibility to this indexing behavior. It takes a comma-separated list of options, each of which specifies a token indexing variant.

If multiple options are specified, multiple variants of the same token will be indexed. Regular keywords (resulting from that token by replacing blended characters with a separator) are always indexed.

The options are:

* `trim_none` - Index the entire token
* `trim_head` - Trim heading blended characters, and index the resulting token
* `trim_tail` - Trim trailing blended characters, and index the resulting token
* `trim_both`- Trim both heading and trailing blended characters, and index the resulting token
* `trim_all` - Trim heading, trailing, and middle blended characters, and index the resulting token
* `skip_pure` - Do not index the token if it is purely blended, that is, consists of blended characters only

Using `blend_mode` with the example `@dude!` string above, the setting `blend_mode = trim_head, trim_tail` would result in two indexed tokens: `@dude` and `dude!`. Using `trim_both` would have no effect because trimming both blended characters results in `dude`, which is already indexed as a regular keyword. Indexing `@U.S.A.` with `trim_both` (and assuming that dot is blended two) would result in `U.S.A` being indexed. Lastly, `skip_pure` enables you to ignore sequences of blended characters only. For example, `one @@@ two` would be indexed as `one two`, and match that as a phrase. This is not the case by default because a fully blended token gets indexed and offsets the second keyword position.

Default behavior is to index the entire token, equivalent to `blend_mode = trim_none`.

Be aware that using blend modes limits your search, even with the default mode `trim_none` if you assume `.` is a blended character:
* `.dog.` will become `.dog. dog` during indexing
* and you won't be able to find it by `dog.`.

Using more modes increases the chance your keyword will match something.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
```

<!-- request CONFIG -->

```ini
table products {
  blend_mode = trim_tail, skip_pure
  blend_chars = +, &

  type = rt
  path = tbl
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

min_word_len is an optional index configuration option in Manticore that specifies the minimum indexed word length. The default value is 1, which means that everything is indexed.

Only those words that are not shorter than this minimum will be indexed. For example, if min_word_len is 4, then 'the' won't be indexed, but 'they' will be.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_word_len = '4'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
```

<!-- request CONFIG -->

```ini
table products {
  min_word_len = 4

  type = rt
  path = tbl
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

N-grams provide basic support for continuous-script languages in unsegmented texts. The issue with searching in languages using continuous scripts is the absence of clear separators between words. In some cases, you may not want to use dictionary-based segmentation, such as [the one available for Chinese](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md). In those instances, n-gram segmentation might also work well.

When this feature is enabled, streams of such languages (or any other characters defined in [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)) are indexed as N-grams. For example, if the incoming text is "ABCDEF" (where A to F represent some language characters) and ngram_len is 1, it will be indexed as if it were "A B C D E F". Only ngram_len=1 is currently supported. Only those characters that are listed in [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) table will be split this way; others will not be affected.

Note that if the search query is segmented, i.e. there are separators between individual words, then wrapping the words in quotes and using extended mode will result in proper matches being found even if the text was **not** segmented. For instance, assume that the original query is `BC DEF`. After wrapping in quotes on the application side, it should look like `"BC" "DEF"` (*with* quotes). This query will be passed to Manticore and internally split into 1-grams too, resulting in `"B C" "D E F"` query, still with quotes that are the phrase matching operator. And it will match the text even though there were no separators in the text.

Even if the search query is not segmented, Manticore should still produce good results, thanks to phrase-based ranking: it will pull closer phrase matches (which in the case of N-gram words can mean closer multi-character word matches) to the top.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_chars

```ini
ngram_chars = cont

ngram_chars = cont, U+3000..U+2FA1F
```

<!-- example ngram_chars -->
N-gram characters list. Optional, default is empty.

To be used in conjunction with in [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len), this list defines characters, sequences of which are subject to N-gram extraction. Words comprised of other characters will not be affected by N-gram indexing feature. The value format is identical to [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). N-gram characters cannot appear in the [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = U+3000..U+2FA1F
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```

<!-- end -->

<!-- example ngram_chars 2 -->
Also you can use an alias for our default N-gram table as in the example. It should be sufficient in most cases.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
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

The syntax is the same as for [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), but it's only allowed to declare characters, and not allowed to map them. Also, the ignored characters must not be present in charset_table.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
```

<!-- request CONFIG -->

```ini
table products {
  ignore_chars = U+AD

  type = rt
  path = tbl
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
* `first_freq`, only index word pairs where the *first* word is in a list of frequent words (see [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). For example, with `bigram_freq_words = the, in, i, a`, indexing "alone in the dark" text will result in "in the" and "the dark" pairs being stored as bigrams, because they begin with a frequent keyword (either "in" or "the" respectively), but "alone in" would **not** be indexed, because "in" is a *second* word in that pair.
* `both_freq`, only index word pairs where both words are frequent. Continuing with the same example, in this mode indexing "alone in the dark" would only store "in the" (the very worst of them all from searching perspective) as a bigram, but none of the other word pairs.

For most use cases, `both_freq` would be the best mode, but your mileage may vary.

It's important to note that `bigram_index` works only at the tokenization level and doesn't account for transformations like `morphology`, `wordforms` or `stopwords`. This means the tokens it creates are very straightforward, which makes searching phrases more exact and strict. While this can improve the accuracy of phrase matching, it also makes the system less able to recognize different forms of words or variations in how words appear.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
```

<!-- request CONFIG -->

```ini
table products {
  bigram_index = both_freq
  bigram_freq_words = the, a, you, i

  type = rt
  path = tbl
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

Some of the bigram indexing modes (see [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) require to define a list of frequent keywords. These are **not** to be confused with stop words. Stop words are completely eliminated when both indexing and searching. Frequent keywords are only used by bigrams to determine whether to index a current word pair or not.

`bigram_freq_words` lets you define a list of such keywords.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
```

<!-- request CONFIG -->

```ini
table products {
  bigram_freq_words = the, a, you, i
  bigram_index = first_freq

  type = rt
  path = tbl
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
The type of keywords dictionary used is identified by one of two known values, 'crc' or 'keywords'. This is optional, with 'keywords' as the default.

Using the keywords dictionary mode (dict=keywords) can significantly decrease the indexing burden and enable substring searches on extensive collections. This mode can be utilized for both plain and RT tables.

CRC dictionaries do not store the original keyword text in the index. Instead, they replace keywords with a control sum value (computed using FNV64) during both searching and indexing processes. This value is used internally within the index. This approach has two disadvantages:
* Firstly, there's a risk of control sum collisions between different keywords pairs. This risk grows in proportion to the number of unique keywords in the index. Nonetheless, this concern is minor as the probability of a single FNV64 collision in a dictionary of 1 billion entries is roughly 1 in 16, or 6.25 percent. Most dictionaries will have far fewer than a billion keywords given that a typical spoken human language has between 1 and 10 million word forms.
* Secondly, and more crucially, it's not straightforward to perform substring searches with control sums. Manticore addressed this issue by pre-indexing all possible substrings as separate keywords (see [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) directives). This method even has an added advantage of matching substrings in the fastest way possible. Yet, pre-indexing all substrings significantly increases the index size (often by factors of 3-10x or more) and subsequently affects the indexing time, making substring searches on large indexes rather impractical.

The keywords dictionary resolves both of these issues. It stores keywords in the index and performs search-time wildcard expansion. For instance, a search for a `test*` prefix could internally expand to a 'test|tests|testing' query based on the dictionary's contents. This expansion process is entirely invisible to the application, with the exception that the separate per-keyword statistics for all the matched keywords are now also reported.

For substring (infix) searches, extended wildcards can be used. Special characters such as `?` and `%` are compatible with substring (infix) search (e.g., `t?st*`, `run%`, `*abc*`). Note that the [wildcards operators](Searching/Full_text_matching/Operators.md#Wildcard-operators) and the [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) only function with `dict=keywords`.

Indexing with a keywords dictionary is approximately 1.1x to 1.3x slower than regular, non-substring indexing - yet significantly faster than substring indexing (either prefix or infix). The index size should only be slightly larger than that of the standard non-substring table, with a total difference of 1..10% percent. The time it takes for regular keyword searching should be nearly the same or identical across all three index types discussed (CRC non-substring, CRC substring, keywords). Substring searching time can significantly fluctuate based on how many actual keywords match the given substring (i.e., how many keywords the search term expands into). The maximum number of matched keywords is limited by the [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) directive.

In summary, keywords and CRC dictionaries offer two different trade-off decisions for substring searching. You can opt to either sacrifice indexing time and index size to achieve the fastest worst-case searches (CRC dictionary), or minimally impact indexing time but sacrifice worst-case searching time when the prefix expands into a high number of keywords (keywords dictionary).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) dict = 'keywords'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
```

<!-- request CONFIG -->

```ini
table products {
  dict = keywords

  type = rt
  path = tbl
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

When you create a table the above mentioned files can be either saved externally along with the table or embedded directly into the table. Files sized under `embedded_limit` get stored into the table. For bigger files, only the file names are stored. This also simplifies moving table files to a different machine; you may get by just copying a single file.

With smaller files, such embedding reduces the number of the external files on which the table depends, and helps maintenance. But at the same time it makes no sense to embed a 100 MB wordforms dictionary into a tiny delta table. So there needs to be a size threshold, and `embedded_limit` is that threshold.

<!-- request CONFIG -->

```ini
table products {
  embedded_limit = 32K

  type = rt
  path = tbl
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

On a multi-table cluster, per-keyword frequencies are quite likely to differ across different tables. That means that when the ranking function uses TF-IDF based values, such as BM25 family of factors, the results might be ranked slightly differently depending on what cluster node they reside.

The easiest way to fix that issue is to create and utilize a global frequency dictionary, or a global IDF file for short. This directive lets you specify the location of that file. It is suggested (but not required) to use an .idf extension. When the IDF file is specified for a given table *and* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) is set to 1, the engine will use the keyword frequencies and collection documents counts from the global_idf file, rather than just the local table. That way, IDFs and the values that depend on them will stay consistent across the cluster.

IDF files can be shared across multiple tables. Only a single copy of an IDF file will be loaded by `searchd`, even when many tables refer to that file. Should the contents of an IDF file change, the new contents can be loaded with a SIGHUP.

You can build an .idf file using [indextool](../../Miscellaneous_tools.md#indextool) utility, by dumping dictionaries using `--dumpdict dict.txt --stats` switch first, then converting those to .idf format using `--buildidf`, then merging all the .idf files across cluster using `--mergeidf`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
```

<!-- request CONFIG -->

```ini
table products {
  global_idf = /usr/local/manticore/var/global.idf

  type = rt
  path = tbl
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

Hitless index will generally use less space than the respective regular full-text index (about 1.5x can be expected). Both indexing and searching should be faster, at a cost of missing positional query and ranking support.  

If used in positional queries (e.g. phrase queries) the hitless words are taken out from them and used as operand without a position.  For example if "hello" and "world" are hitless and "simon" and "says" are not hitless, the phrase query  `"simon says hello world"` will be converted to `("simon says" & hello & world)`, matching "hello" and "world" anywhere in the document and "simon says" as an exact phrase.

A positional query than contains only hitless words will result in an empty phrase node, therefore the entire query will return an empty result and a warning. If the whole dictionary is hitless (using `all`) only boolean matching can be used on the respective index.



<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words = 'all'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
```

<!-- request CONFIG -->

```ini
table products {
  hitless_words = all

  type = rt
  path = tbl
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
Enables computing and storing of field lengths (both per-document and average per-index values) into the full-text index. Optional, default is 0 (do not compute and store).

When `index_field_lengths` is set to 1 Manticore will:
* create a respective length attribute for every full-text field, sharing the same name but with `__len` suffix
* compute a field length (counted in keywords) for every document and store in to a respective attribute
* compute the per-index averages. The lengths attributes will have a special TOKENCOUNT type, but their values are in fact regular 32-bit integers, and their values are generally accessible.

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) and [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) functions in the expression ranker are based on these lengths and require `index_field_lengths` to be enabled. Historically, Manticore used a simplified, stripped-down variant of BM25 that, unlike the complete function, did **not** account for document length. There's also support for both a complete variant of BM25, and its extension towards multiple fields, called BM25F. They require per-document length and per-field lengths, respectively. Hence the additional directive.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_field_lengths = '1'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  index_field_lengths = 1

  type = rt
  path = tbl
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
Index-time token filter for full-text indexing. Optional, default is empty.

The index_token_filter directive specifies an optional index-time token filter for full-text indexing. This directive is used to create a custom tokenizer that makes tokens according to custom rules. The filter is created by the indexer on indexing source data into a plain table or by an RT table on processing `INSERT` or `REPLACE` statements. The plugins are defined using the format `library name:plugin name:optional string of settings`. For example, `my_lib.so:custom_blend:chars=@#&`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
```

<!-- request CONFIG -->

```ini
table products {
  index_token_filter = my_lib.so:custom_blend:chars=@#&

  type = rt
  path = tbl
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
Position increment on overshort (less than [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) keywords. Optional, allowed values are 0 and 1, default is 1.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  overshort_step = 1

  type = rt
  path = tbl
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

This list controls what characters will be treated as phrase boundaries, in order to adjust word positions and enable phrase-level search emulation through proximity search. The syntax is similar to [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), but mappings are not allowed and the boundary characters must not overlap with anything else.

On phrase boundary, additional word position increment (specified by [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)) will be added to current word position. This enables phrase-level searching through proximity queries: words in different phrases will be guaranteed to be more than phrase_boundary_step distance away from each other; so proximity search within that distance will be equivalent to phrase-level search.

Phrase boundary condition will be raised if and only if such character is followed by a separator; this is to avoid abbreviations such as S.T.A.L.K.E.R or URLs being treated as several phrases.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary = ., ?, !, U+2026
  phrase_boundary_step = 10

  type = rt
  path = tbl
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

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary_step = 100
  phrase_boundary = ., ?, !, U+2026

  type = rt
  path = tbl
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
Regular expressions (regexps) used to filter the fields and queries. This directive is optional, multi-valued, and its default is an empty list of regular expressions. The regular expressions engine used by Manticore Search is Google's RE2, which is known for its speed and safety. For detailed information on the syntax supported by RE2, you can visit the [RE2 syntax guide](https://github.com/google/re2/wiki/Syntax).

In certain applications such as product search, there can be many ways to refer to a product, model, or property. For example, `iPhone 3gs` and `iPhone 3 gs` (or even `iPhone3 gs`) are very likely to refer to the same product. Another example could be different ways to express a laptop screen size, such as `13-inch`, `13 inch`, `13"`, or `13in`.

Regexps provide a mechanism to specify rules tailored to handle such cases. In the first example, you could possibly use a wordforms file to handle a handful of iPhone models, but in the second example, it's better to specify rules that would normalize "13-inch" and "13in" to something identical.

Regular expressions listed in `regexp_filter` are applied in the order they are listed, at the earliest stage possible, before any other processing (including [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)), even before tokenization. That is, regexps are applied to the raw source fields when indexing, and to the raw search query text when searching.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'
```

<!-- request JSON -->

```JSON
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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'");
```

<!-- request CONFIG -->

```ini
table products {
  # index '13"' as '13inch'
  regexp_filter = \b(\d+)\" => \1inch

  # index 'blue' or 'red' as 'color'
  regexp_filter = (blue|red) => color

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->
