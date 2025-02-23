# Advanced morphology

Morphology preprocessors can be applied to words during indexing to normalize different forms of the same word and improve segmentation. For example, an English stemmer can normalize "dogs" and "dog" to "dog", resulting in identical search results for both keywords.

Manticore has four built-in morphology preprocessors:

*   **Lemmatizer**: reduces a word to its root or lemma. For example, "running" can be reduced to "run" and "octopi" can be reduced to "octopus". Note that some words may have multiple corresponding root forms. For example, "dove" can be either the past tense of "dive" or a noun meaning a bird, as in "A white dove flew over the cuckoo's nest." In this case, a lemmatizer can generate all the possible root forms.
*   **Stemmer**: reduces a word to its stem by removing or replacing certain known suffixes. The resulting stem may not necessarily be a valid word. For example, the Porter English stemmer reduces "running" to "run", "business" to "busi" (not a valid word), and does not reduce "octopi" at all.
*   **Phonetic algorithms**: replace words with phonetic codes that are the same even if the words are different but phonetically close.
*   **Word breaking algorithms**: split text into words. Currently available only for Chinese.

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
The morphology directive specifies a list of morphology preprocessors to apply to the words being indexed. This is an optional setting, with the default being no preprocessor applied.

Manticore comes with built-in morphological preprocessors for:

* English, Russian, and German lemmatizers
* English, Russian, Arabic, and Czech stemmers
* SoundEx and MetaPhone phonetic algorithms
* Chinese word breaking algorithm
* Snowball (libstemmer) stemmers for more than [15 other languages](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) are also available.

Lemmatizers require dictionary `.pak` files that can be installed using the `manticore-language-packs` packages or [downloaded from the Manticore website](https://manticoresearch.com/install/#other-downloads). In the latter case the dictionaries need to be put in the directory specified by [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base). 

Additionally, the [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) setting can be used to speed up lemmatizing by spending more RAM for an uncompressed dictionary cache.

The Chinese language segmentation can be done using [ICU](http://site.icu-project.org/) or [Jieba](https://github.com/yanyiwu/cppjieba) (requires package `manticore-language-packs`). Both libraries provide more accurate segmentation than n-grams, but are slightly slower. The [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) must include all Chinese characters, which can be done using the `cont`, `cjk` or `chinese` character sets. When you set `morphology=icu_chinese` or `morphology=jieba_chinese`, the documents are first pre-processed by ICU or Jieba. Then, the tokenizer processes the result according to the charset_table, and finally, other morphology processors from the `morphology` option are applied. Only those parts of the text that contain Chinese are passed to ICU/Jieba for segmentation, while the other parts can be modified by different means such as different morphologies or `charset_table`.

Built-in English and Russian stemmers are faster than their libstemmer counterparts but may produce slightly different results

Soundex implementation matches that of MySQL. Metaphone implementation is based on Double Metaphone algorithm and indexes the primary code.

To use the `morphology` option, specify one or multiple of the built-in options, including:
* none: do not perform any morphology processing
* lemmatize_ru - apply Russian lemmatizer and pick a single root form
* lemmatize_uk - apply Ukrainian lemmatizer and pick a single root form (install it first in [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) or [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer)). For correct work of the lemmatizer make sure specific Ukrainian characters are preserved in your `charset_table` since by default they are not. For that override them, like this: `charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`. [Here](https://play.manticoresearch.com/ua-lemmatizer/) is an interactive course about how to install and use the urkainian lemmatizer.
* lemmatize_en - apply English lemmatizer and pick a single root form
* lemmatize_de - apply German lemmatizer and pick a single root form
* lemmatize_ru_all - apply Russian lemmatizer and index all possible root forms
* lemmatize_uk_all - apply Ukrainian lemmatizer and index all possible root forms. Find the installation links above and take care of the `charset_table`.
* lemmatize_en_all - apply English lemmatizer and index all possible root forms
* lemmatize_de_all - apply German lemmatizer and index all possible root forms
* stem_en - apply Porter's English stemmer
* stem_ru - apply Porter's Russian stemmer
* stem_enru - apply Porter's English and Russian stemmers
* stem_cz - apply Czech stemmer
* stem_ar - apply Arabic stemmer
* soundex - replace keywords with their SOUNDEX code
* metaphone - replace keywords with their METAPHONE code
* icu_chinese - apply Chinese text segmentation using ICU
* jieba_chinese - apply Chinese text segmentation using Jieba (requires package `manticore-language-packs`)
* libstemmer_* . Refer to the [list of supported languages](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) for details

Multiple stemmers can be specified, separated by commas. They will be applied to incoming words in the order they are listed, and the processing will stop once one of the stemmers modifies the word. Additionally, when [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) feature is enabled, the word will be looked up in the word forms dictionary first. If there is a matching entry in the dictionary, stemmers will not be applied at all.  [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) сan be used to implement stemming exceptions.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'
```

<!-- request JSON -->

```json
POST /cli -d "CREATE TABLE products(title text, price float)  morphology = 'stem_en, libstemmer_sv'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology' => 'stem_en, libstemmer_sv'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = stem_en, libstemmer_sv

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## morphology_skip_fields

<!-- example morphology_skip_fields -->

```ini
morphology_skip_fields = field1[, field2, ...]
```

A list of fields to skip morphology preprocessing. Optional, default is empty (apply preprocessors to all fields).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology_skip_fields' => 'name',
            'morphology' => 'stem_en'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology_skip_fields = name
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

## min_stemming_len

<!-- example min_stemming_len -->

```ini
min_stemming_len = length
```

Minimum word length at which to enable stemming. Optional, default is 1 (stem everything).

Stemmers are not perfect, and might sometimes produce undesired results. For instance, running "gps" keyword through Porter stemmer for English results in "gp", which is not really the intent. `min_stemming_len` feature lets you suppress stemming based on the source word length, ie. to avoid stemming too short words. Keywords that are shorter than the given threshold will not be stemmed. Note that keywords that are exactly as long as specified **will** be stemmed. So in order to avoid stemming 3-character keywords, you should specify 4 for the value. For more finely grained control, refer to [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) feature.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'min_stemming_len' => '4',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'");
```

<!-- request CONFIG -->

```ini
table products {
  min_stemming_len = 4
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## index_exact_words

<!-- example index_exact_words -->

```ini
index_exact_words = {0|1}
```

This option allows for the indexing of original keywords along with their morphologically modified versions. However, original keywords that are remapped by the [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) and [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md) cannot be indexed. The default value is 0, indicating that this feature is disabled by default.

This allows the use of the [exact form operator](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier) in the query language. Enabling this feature will increase the full-text index size and indexing time, but will not impact search performance.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'index_exact_words' => '1',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'");
```

<!-- request CONFIG -->

```ini
table products {
  index_exact_words = 1
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_hmm

<!-- example jieba_hmm -->

```ini
jieba_hmm = {0|1}
```

Enable or disable HMM in the Jieba segmentation tool. Optional; the default is 1.

In Jieba, the HMM (Hidden Markov Model) option refers to an algorithm used for word segmentation. Specifically, it allows Jieba to perform Chinese word segmentation by recognizing unknown words, especially those not present in its dictionary.

Jieba primarily uses a dictionary-based method for segmenting known words, but when the HMM option is enabled, it applies a statistical model to identify probable word boundaries for words or phrases that are not in its dictionary. This is particularly useful for segmenting new or rare words, names, and slang.

In summary, the `jieba_hmm` option helps improve segmentation accuracy at the expense of indexing performance. It must be used with `morphology = jieba_chinese`, see [Chinese, Japanese and Korean (CJK) and Thai languages](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
	  		 'jieba_hmm'='1'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_hmm = 0

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_mode

<!-- example jieba_mode -->

```ini
jieba_mode = {accurate|full|search}
```

Jieba segmentation mode. Optional; the default is `accurate`.

In accurate mode, Jieba splits the sentence into the most precise words using dictionary matching. This mode focuses on precision, ensuring that the segmentation is as accurate as possible.

In full mode, Jieba tries to split the sentence into every possible word combination, aiming to include all potential words. This mode focuses on maximizing recall, meaning it identifies as many words as possible, even if some of them overlap or are less commonly used. It returns all the words found in its dictionary.

In search mode, Jieba breaks the text into both whole words and smaller parts, combining precise segmentation with extra detail by providing overlapping word fragments. This mode balances precision and recall, making it useful for search engines.

`jieba_mode` should be used with `morphology = jieba_chinese`. See [Chinese, Japanese, Korean (CJK) and Thai languages](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
	  		 'jieba_mode'='full'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_mode = full

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_user_dict_path

<!-- example jieba_user_dict_path -->

```ini
jieba_user_dict_path = path/to/stopwords/file
```

Path to the Jieba user dictionary. Optional.

Jieba, a Chinese text segmentation library, uses dictionary files to assist with word segmentation. The format of these dictionary files is as follows: each line contains a word, split into three parts separated by spaces — the word itself, word frequency, and part of speech (POS) tag. The word frequency and POS tag are optional and can be omitted. The dictionary file must be UTF-8 encoded.

Example:

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` should be used with `morphology = jieba_chinese`. For more details, see [Chinese, Japanese, Korean (CJK), and Thai languages](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
             'jieba_user_dict_path' = '/usr/local/manticore/data/user-dict.txt'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_user_dict_path = /usr/local/manticore/data/user-dict.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->
