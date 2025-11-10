# 高级形态学

形态学预处理器可以在索引期间应用于单词，以规范同一单词的不同形式并改进分词。例如，英文词干提取器可以将 "dogs" 和 "dog" 规范为 "dog"，从而使这两个关键词的搜索结果相同。

Manticore 有四种内置的形态学预处理器：

*   **词形还原器**：将单词还原为其词根或词元。例如，"running" 可以还原为 "run"，"octopi" 可以还原为 "octopus"。注意，有些单词可能有多个对应的词根形式。例如，"dove" 既可以是 "dive" 的过去式，也可以是表示鸟类的名词，如 "A white dove flew over the cuckoo's nest." 在这种情况下，词形还原器可以生成所有可能的词根形式。
*   **词干提取器**：通过移除或替换某些已知后缀，将单词还原为词干。得到的词干不一定是有效单词。例如，Porter 英文词干提取器将 "running" 还原为 "run"，将 "business" 还原为 "busi"（不是有效单词），并且不会对 "octopi" 进行还原。
*   **语音算法**：用语音编码替换单词，即使单词不同但发音相近，编码也相同。
*   **分词算法**：将文本拆分成单词。目前仅支持中文。

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
morphology 指令指定要应用于被索引单词的形态学预处理器列表。这是一个可选设置，默认不应用任何预处理器。

Manticore 内置了以下形态学预处理器：

* 英语、俄语和德语词形还原器
* 英语、俄语、阿拉伯语和捷克语词干提取器
* SoundEx 和 MetaPhone 语音算法
* 中文分词算法
* 还提供了 Snowball（libstemmer）词干提取器，支持超过 [15 种其他语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

词形还原器需要字典 `.pak` 文件，可以通过 `manticore-language-packs` 软件包安装，或从 [Manticore 网站下载](https://manticoresearch.com/install/#other-downloads)。在后者情况下，字典需要放置在由 [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) 指定的目录中。

此外，[lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) 设置可以通过使用更多内存缓存未压缩字典来加速词形还原。

中文分词可以使用 [ICU](http://site.icu-project.org/) 或 [Jieba](https://github.com/yanyiwu/cppjieba)（需要 `manticore-language-packs` 包）。这两个库提供比 n-gram 更准确的分词，但速度稍慢。[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 必须包含所有中文字符，可以使用 `cont`、`cjk` 或 `chinese` 字符集来实现。当设置 `morphology=icu_chinese` 或 `morphology=jieba_chinese` 时，文档首先由 ICU 或 Jieba 预处理。然后，分词器根据 charset_table 处理结果，最后应用 `morphology` 选项中的其他形态学处理器。只有包含中文的文本部分会传递给 ICU/Jieba 进行分词，其他部分可以通过不同方式修改，如不同的形态学处理或 `charset_table`。

内置的英语和俄语词干提取器比 libstemmer 版本更快，但可能产生略有不同的结果。

Soundex 实现与 MySQL 一致。Metaphone 实现基于 Double Metaphone 算法，索引主编码。

要使用 `morphology` 选项，请指定一个或多个内置选项，包括：
* none：不执行任何形态学处理
* lemmatize_ru - 应用俄语词形还原器并选择单一词根形式
* lemmatize_uk - 应用乌克兰语词形还原器并选择单一词根形式（先在 [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) 或 [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) 安装）。为确保词形还原器正常工作，请确保在 `charset_table` 中保留特定的乌克兰字符，因为默认不包含。可通过如下方式覆盖：`charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`。[这里](https://play.manticoresearch.com/ua-lemmatizer/) 有一个关于如何安装和使用乌克兰词形还原器的交互式课程。
* lemmatize_en - 应用英语词形还原器并选择单一词根形式
* lemmatize_de - 应用德语词形还原器并选择单一词根形式
* lemmatize_ru_all - 应用俄语词形还原器并索引所有可能的词根形式
* lemmatize_uk_all - 应用乌克兰语词形还原器并索引所有可能的词根形式。安装链接见上，并注意 `charset_table` 设置。
* lemmatize_en_all - 应用英语词形还原器并索引所有可能的词根形式
* lemmatize_de_all - 应用德语词形还原器并索引所有可能的词根形式
* stem_en - 应用 Porter 英语词干提取器
* stem_ru - 应用 Porter 俄语词干提取器
* stem_enru - 同时应用 Porter 英语和俄语词干提取器
* stem_cz - 应用捷克语词干提取器
* stem_ar - 应用阿拉伯语词干提取器
* soundex - 用 SOUNDEX 代码替换关键词
* metaphone - 用 METAPHONE 代码替换关键词
* icu_chinese - 使用 ICU 进行中文分词
* jieba_chinese - 使用 Jieba 进行中文分词（需要 `manticore-language-packs` 包）
* libstemmer_* 。详情请参阅 [支持语言列表](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)

可以指定多个词干提取器，用逗号分隔。它们将按照列出的顺序应用于输入的单词，并且一旦其中一个词干提取器修改了单词，处理将停止。此外，当启用[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能时，单词将首先在词形字典中查找。如果字典中有匹配的条目，则根本不会应用词干提取器。[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)可用于实现词干提取的例外情况。

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", Some(true)).await;
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

跳过形态学预处理的字段列表。可选，默认值为空（对所有字段应用预处理器）。

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", Some(true)).await;
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

启用词干提取的最小单词长度。可选，默认值为1（对所有单词进行词干提取）。

词干提取器并不完美，有时可能产生不理想的结果。例如，将“gps”关键词通过英语的Porter词干提取器处理，结果是“gp”，这并非真正的意图。`min_stemming_len`功能允许您根据源单词长度抑制词干提取，即避免对过短的单词进行词干提取。比给定阈值更短的关键词将不会被词干提取。注意，长度恰好等于指定值的关键词**会**被词干提取。因此，为了避免对3个字符的关键词进行词干提取，您应将值指定为4。有关更细粒度的控制，请参阅[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能。

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
##### Python-asyncio:

<!-- request Python-asycnio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", Some(true)).await;
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

此选项允许对原始关键词及其形态学修改版本进行索引。但是，被[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)和[exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md)重新映射的原始关键词无法被索引。默认值为0，表示默认情况下此功能被禁用。

这允许在查询语言中使用[精确形式操作符](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier)。启用此功能将增加全文索引的大小和索引时间，但不会影响搜索性能。

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", Some(true)).await;
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

启用或禁用Jieba分词工具中的HMM。可选，默认值为1。

在Jieba中，HMM（隐马尔可夫模型）选项指的是一种用于分词的算法。具体来说，它允许Jieba通过识别未知词，尤其是词典中不存在的词，来执行中文分词。

Jieba主要使用基于词典的方法来分割已知词，但当启用HMM选项时，它会应用统计模型来识别词典中不存在的词或短语的可能词边界。这对于分割新词、罕见词、姓名和俚语特别有用。

总之，`jieba_hmm`选项有助于提高分词准确性，但会牺牲索引性能。它必须与`morphology = jieba_chinese`一起使用，详见[中文、日文和韩文（CJK）及泰语](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", Some(true)).await;
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

`jieba_mode` should be used with `morphology = jieba_chinese`. See [Chinese, Japanese, Korean (CJK) and Thai languages](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", Some(true)).await;
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

`jieba_user_dict_path` should be used with `morphology = jieba_chinese`. For more details, see [Chinese, Japanese, Korean (CJK), and Thai languages](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", Some(true)).await;
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

