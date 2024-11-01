# 高级词法

词法预处理器可以在索引过程中应用于单词，以规范相同单词的不同形式，从而改善分词效果。例如，英语词干提取器可以将“dogs”和“dog”规范化为“dog”，从而使这两个关键字的搜索结果相同。

Manticore 内置了四种词法预处理器：

- **词元化器（Lemmatizer）**：将单词简化为其根或词元。例如，“running”可以简化为“run”，“octopi”可以简化为“octopus”。请注意，一些单词可能具有多个对应的根形式。例如，“dove”可以是“dive”的过去式，或者是名词“鸽子”，例如“白鸽飞过杜鹃窝”。在这种情况下，词元化器可以生成所有可能的根形式。
- **词干提取器（Stemmer）**：通过去除或替换某些已知后缀来将单词简化为其词干。结果词干不一定是有效单词。例如，Porter 英语词干提取器将“running”简化为“run”，“business”简化为“busi”（不是有效单词），而“octopi”则不作任何简化。
- **语音算法（Phonetic algorithms）**：将单词替换为发音代码，即使单词不同但发音相近的单词也会生成相同的代码。
- **分词算法（Word breaking algorithms）**：将文本拆分为单词。目前仅适用于中文。

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->

“词法”指令指定要应用于索引单词的词法预处理器列表。此设置是可选的，默认情况下不应用任何预处理器。

Manticore 提供了内置的词法预处理器，支持以下语言：

- 英语、俄语和德语的词元化器
- 英语、俄语、阿拉伯语和捷克语的词干提取器
- SoundEx 和 MetaPhone 语音算法
- 中文的分词算法
- 还提供了支持超过 [15 种其他语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) 的 Snowball（libstemmer）词干提取器。

词元化器需要字典 `.pak` 文件，这些文件可以从 Manticore 网站 [下载](https://manticoresearch.com/install/#other-downloads)。字典需要放在由 [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) 指定的目录中。此外，[lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) 设置可用于通过占用更多 RAM 来加速词元化过程，生成未压缩的字典缓存。

中文分词可通过 [ICU](http://site.icu-project.org/) 或 [Jieba](https://github.com/yanyiwu/cppjieba) 实现。这两种库提供的分词准确度高于 n-grams，但速度稍慢。`charset_table` 必须包含所有中文字符，可以使用 `cont`、`cjk` 或 `chinese` 字符集完成。当设置 `morphology=icu_chinese` 或 `morphology=jieba_chinese` 时，文档首先会通过 ICU 或 Jieba 进行预处理。然后，分词器根据 `charset_table` 处理结果，最后应用“词法”选项中的其他词法处理器。只有包含中文的文本部分会被传递给 ICU/Jieba 进行分词，其他部分可以通过不同的方式修改，例如使用不同的词法或 `charset_table`。

内置的英语和俄语词干提取器速度比其 libstemmer 对应物快，但可能产生略微不同的结果。

Soundex 的实现与 MySQL 的一致。Metaphone 的实现基于 Double Metaphone 算法，并为每个单词生成主要代码。

要使用 `morphology` 选项，请指定一个或多个内置选项，包括：

* none：不执行任何词法处理
* lemmatize_ru：应用俄语词元化器并选择一个根形式
* lemmatize_uk：应用乌克兰语词元化器并选择一个根形式（首先在 [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) 或 [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) 中安装）。为确保词元化器正常工作，请确保在`charset_table` 中保留特定的乌克兰字符，因为默认情况下不会保留。要覆盖它们，可以这样设置：`charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`。关于如何安装和使用乌克兰语词元化器，可以参考 [这里](https://play.manticoresearch.com/ua-lemmatizer/) 的互动课程。
* lemmatize_en：应用英语词元化器并选择一个根形式
* lemmatize_de：应用德语词元化器并选择一个根形式
* lemmatize_ru_all：应用俄语词元化器并索引所有可能的根形式
* lemmatize_uk_all：应用乌克兰语词元化器并索引所有可能的根形式。有关安装链接，请参阅上文，并确保 `charset_table` 的正确性。
* lemmatize_en_all：应用英语词元化器并索引所有可能的根形式
* lemmatize_de_all：应用德语词元化器并索引所有可能的根形式
* stem_en：应用 Porter's 英语词干提取器
* stem_ru：应用 Porter's 俄语词干提取器
* stem_enru：应用 Porter's 英语和俄语词干提取器
* stem_cz：应用捷克语词干提取器
* stem_ar：应用阿拉伯语词干提取器
* soundex：用 SOUNDEX 代码替换关键字
* metaphone：用 METAPHONE 代码替换关键字
* icu_chinese：应用 ICU 进行中文文本分词
* jieba_chinese：应用 Jieba 进行中文文本分词
* libstemmer_*：有关详细信息，请参阅 [支持的语言列表](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

可以指定多个词干提取器，以逗号分隔。它们将按顺序应用于传入单词，一旦其中一个词干提取器修改了单词，处理将停止。此外，当启用 [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 特性时，系统将首先在词形字典中查找单词。如果在字典中有匹配条目，则不应用词干提取器。 [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 可用于实现词干提取例外。

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

一个要跳过词法预处理的字段列表。可选参数，默认值为空（对所有字段应用预处理器）。

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

词法处理启用的最小单词长度。可选，默认值为1（处理所有单词）。

词法处理并不完美，有时可能会产生不希望的结果。例如，将“gps”关键字通过英语的Porter词法处理后，结果为“gp”，这并不是原意。`min_stemming_len`功能允许您根据源单词的长度抑制词法处理，即避免对过短的单词进行处理。长度低于给定阈值的关键字将不会进行词法处理。请注意，恰好等于指定长度的关键字**将**被处理。因此，要避免对3个字符的关键字进行处理，您应将值指定为4。有关更细粒度的控制，请参考[词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能。

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

此选项允许在索引时同时保存原始关键字及其形态变化版本。然而，通过[词形变化](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)和[例外](../../Creating_a_table/NLP_and_tokenization/Exceptions.md)进行重映射的原始关键字无法被索引。默认值为0，表示此功能默认禁用。

启用此功能后，可以在查询语言中使用[精确形式操作符](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier)。启用该功能将增加全文索引的大小和索引时间，但不会影响搜索性能。

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

启用或禁用 Jieba 分词工具中的 HMM（隐马尔可夫模型）。可选；默认值为 1。

在 Jieba 中，HMM 选项指的是用于分词的算法。具体来说，它允许 Jieba 通过识别未知词（特别是字典中不存在的词）来进行中文分词。

Jieba 主要使用基于字典的方法来分词已知词，但当 HMM 选项启用时，它会应用统计模型来识别不在字典中的词或短语的可能词边界。这在分词新词、罕见词、名字和俚语时特别有用。

总之，`jieba_hmm` 选项有助于提高分词的准确性，但会影响索引性能。它必须与 `morphology = jieba_chinese` 一起使用，详见[中文、日文和韩文（CJK）及泰语](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba 分词模式。可选；默认值为 `accurate`。

在精确模式下，Jieba 使用字典匹配将句子拆分为最精确的词汇。该模式侧重于准确性，确保分词尽可能准确。

在全模式下，Jieba 尝试将句子拆分为所有可能的词组合，旨在包括所有潜在的词汇。该模式专注于最大化召回率，即识别尽可能多的词汇，即使其中一些重叠或使用频率较低。它返回字典中找到的所有词汇。

在搜索模式下，Jieba 同时将文本拆分为完整词和较小部分，结合精确分词和额外细节，提供重叠的词片段。该模式在准确性和召回率之间取得平衡，适用于搜索引擎。

`jieba_mode` 应与 `morphology = jieba_chinese` 一起使用。请参见[中文、日文和韩文（CJK）及泰语](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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
<!-- proofread -->
