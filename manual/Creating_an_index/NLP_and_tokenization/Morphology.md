# Advanced morphology

Morphology preprocessors can be applied to the words being indexed to replace different forms of the same word with the base, normalized form or improve segmentation. For instance, English stemmer will normalize both "dogs" and "dog" to "dog", making search results for the both keywords the same.

There are 4 different morphology preprocessors that Manticore implements.

*   **Lemmatizer** reduces a keyword form to a so-called lemma, a proper normal form, or in other words, a valid natural language root word. For example, "running" could be reduced to "run", the infinitive verb form, and "octopi" would be reduced to "octopus", the singular noun form. Note that sometimes a word form can have multiple corresponding root words. For instance, by looking at "dove" it is not possible to tell whether this is a past tense of "dive" the verb as in "He dove into a pool.", or "dove" the noun as in "White dove flew over the cuckoo's nest." In this case lemmatizer can generate all the possible root forms.
*   **Stemmer** reduces a keyword form to a so-called stem by removing and/or replacing certain well-known suffixes. The resulting stem is however *not* guaranteed to be a valid word on itself. For instance, with a Porter English stemmers "running" would still reduce to "run", which is fine, but "business" would reduce to "busi", which is not a word, and "octopi" would not reduce at all. Stemmers are essentially (much) simpler but still pretty good replacements of full-blown lemmatizers.
*   **Phonetic algorithms** replace the words with specially crafted phonetic codes that are equal even when the words original are different, but phonetically close.
*   **Word breaking algorithms** split text into words. Currently available only for Chinese.

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
A list of morphology preprocessors to apply. Optional, default is empty (do not apply any preprocessor).

The morphology processors that come with our own built-in Manticore implementations are:

* English, Russian, and German lemmatizers
* English, Russian, Arabic, and Czech stemmers
* SoundEx and MetaPhone phonetic algorithms
* Chinese word breaking algorithm
* Snowball (libstemmer) stemmers for more than [15 other languages](../../Creating_an_index/NLP_and_tokenization/Supported_languages.md).

Lemmatizers require dictionary `.pak` files that you can [download from the website](https://manticoresearch.com/install/#other-downloads). The dictionaries needs to be put in the directory specified by [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base). Also, there is the [lemmatizer_cache](../../Adding_data_from_external_storages/Plain_indexes_creation.md#lemmatizer_cache) setting which lets you speed up lemmatizing (and therefore indexing) by spending more RAM for, basically, an uncompressed dictionary cache.

The Chinese language segmentation using [ICU](http://site.icu-project.org/) is also available. It is a much more precise, but a little bit slower way (compared to n-grams) to segment Chinese documents. [charset_table](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#charset_table) must contain all Chinese characters (you can use alias "cjk"). In case of "morphology=icu_chinese" documents are first pre-processed by ICU, then the result is processed by the tokenizer (according to your charset_table) and then other morphology processors specified in the  "morphology" option are applied. When the documents are processed by ICU, only those parts of texts that contain Chinese are passed to ICU for segmentation, others can be modified by other means (different morphologies, charset_table etc.)

Built-in English and Russian stemmers should be faster than their libstemmer counterparts, but can produce slightly different results, because they are based on an older version.

Soundex implementation matches that of MySQL. Metaphone implementation is based on Double Metaphone algorithm and indexes the primary code.

Built-in values that are available for use in the `morphology` option are as follows:
* none - do not perform any morphology processing
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
* libstemmer_* . Refer to the [list of supported languages](../../Creating_an_index/NLP_and_tokenization/Supported_languages.md) for details

Several stemmers can be specified at once comma-separated. They will be applied to incoming words in the order they are listed, and the processing will stop once one of the stemmers actually modifies the word. Also when [wordforms](../../Creating_an_index/NLP_and_tokenization/Wordforms.md#wordforms) feature is enabled the word will be looked up in word forms dictionary first, and if there is a matching entry in the dictionary, stemmers will not be applied at all. Or in other words, [wordforms](../../Creating_an_index/NLP_and_tokenization/Wordforms.md#wordforms) can be used to implement stemming exceptions.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'
```

<!-- request HTTP -->

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
index products {
  morphology = stem_en, libstemmer_sv

  type = rt
  path = idx
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

<!-- request HTTP -->

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
index products {
  morphology_skip_fields = name
  morphology = stem_en

  type = rt
  path = idx
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

Stemmers are not perfect, and might sometimes produce undesired results. For instance, running "gps" keyword through Porter stemmer for English results in "gp", which is not really the intent. `min_stemming_len` feature lets you suppress stemming based on the source word length, ie. to avoid stemming too short words. Keywords that are shorter than the given threshold will not be stemmed. Note that keywords that are exactly as long as specified **will** be stemmed. So in order to avoid stemming 3-character keywords, you should specify 4 for the value. For more finely grained control, refer to [wordforms](../../Creating_an_index/NLP_and_tokenization/Wordforms.md#wordforms) feature.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'
```

<!-- request HTTP -->

```http
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
index products {
  min_stemming_len = 4
  morphology = stem_en

  type = rt
  path = idx
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

Whether to index the original keywords along with the stemmed/remapped versions. Optional, default is 0 (do not index).

When enabled, `index_exact_words` forces indexation to put the raw keywords in the index along with the stemmed versions. That, in turn, enables [exact form operator](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier) in the query language to work. This impacts the index size and the indexing time. However, searching performance is not impacted at all.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'
```

<!-- request HTTP -->

```http
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
index products {
  index_exact_words = 1
  morphology = stem_en

  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
