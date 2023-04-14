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

Lemmatizers require dictionary `.pak` files that can be [downloaded from the Manticore website](https://manticoresearch.com/install/#other-downloads). The dictionaries need to be put in the directory specified by [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base). Additionally, the [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) setting can be used to speed up lemmatizing by spending more RAM for an uncompressed dictionary cache.

The Chinese language segmentation can be performed using [ICU](http://site.icu-project.org/). It provides more precise segmentation compared to n-grams but is slightly slower. The [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) must include all Chinese characters, which can be done by using the "cjk" alias. When "morphology=icu_chinese" is specified, the documents are first pre-processed by ICU. Then, the result is processed by the tokenizer according to the charset_table, and finally, other morphology processors specified in the "morphology" option are applied. Only those parts of texts that contain Chinese are passed to ICU for segmentation, while others can be modified by different means such as different morphologies or charset_table.

Built-in English and Russian stemmers are faster than their libstemmer counterparts but may produce slightly different results

Soundex implementation matches that of MySQL. Metaphone implementation is based on Double Metaphone algorithm and indexes the primary code.

To use the `morphology` option, specify one or multiple of the built-in options, including:
* none: do not perform any morphology processing
* lemmatize_ru - apply Russian lemmatizer and pick a single root form
* lemmatize_uk - apply Ukrainian lemmatizer and pick a single root form (install it first in [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) or [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer)). For correct work of the lemmatizer make sure specific Ukrainian characters are preserved in your `charset_table` since by default they are not. For that override them, like this: `charset_table='non_cjk,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`. [Here](https://play.manticoresearch.com/ua-lemmatizer/) is an interactive course about how to install and use the urkainian lemmatizer.
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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'");
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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'");
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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'");
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

This option enables indexing of the original keywords along with the stemmed or remapped versions. The default value is 0, which means this feature is disabled.

When `index_exact_words` is enabled, the raw keywords are added to the full-text index along with the stemmed or remapped versions. This allows the use of the [exact form operator](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier) in the query language. Keep in mind that enabling this feature will increase the full-text index size and indexing time, but will not impact search performance.

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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'");
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
<!-- proofread -->
