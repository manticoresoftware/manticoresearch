# 高级形态学

形态学预处理器可以在索引时应用于单词，以规范化同一单词的不同形式并改进分词。例如，英文词干提取器可以将 "dogs" 和 "dog" 规范化为 "dog"，从而这两个关键词的搜索结果相同。

Manticore 有四种内置的形态学预处理器：

*   **词形还原器（Lemmatizer）**：将单词还原为其根或词元。例如，“running”可还原为“run”，“octopi”可还原为“octopus”。注意，有些单词可能有多个对应的根形式。例如，“dove”既可以是“dive”的过去式，也可以是名词“鸽子”，如在句子“A white dove flew over the cuckoo's nest.”中。此时，词形还原器可以生成所有可能的根形式。
*   **词干提取器（Stemmer）**：通过移除或替换某些已知的后缀，将单词还原为词干。所得的词干不一定是有效词。例如，Porter 英文词干提取器会将“running”还原为“run”，“business”还原为“busi”（非有效词），且不会还原“octopi”。
*   **语音算法**：将单词替换为语音编码，即使单词不同但发音相近，编码也相同。
*   **分词算法**：将文本拆分成词。目前仅对中文有效。

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
morphology 指令指定要应用于被索引单词的一系列形态学预处理器。这是一个可选设置，默认是不应用任何预处理器。

Manticore 具有内置的形态学预处理器，支持：

* 英语、俄语和德语的词形还原器
* 英语、俄语、阿拉伯语和捷克语的词干提取器
* SoundEx 和 MetaPhone 语音算法
* 中文分词算法
* Snowball（libstemmer）词干提取器，支持超过[15 种其他语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

词形还原器需要字典 `.pak` 文件，可以通过 `manticore-language-packs` 包安装，或者从[Manticore官网](https://manticoresearch.com/install/#other-downloads)下载。后一种情况需要将字典放入由 [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) 指定的目录。

此外，设置 [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) 可以通过使用更多内存缓存未压缩字典，加快词形还原速度。

中文分词可以使用 [ICU](http://site.icu-project.org/) 或 [Jieba](https://github.com/yanyiwu/cppjieba)（需要安装 `manticore-language-packs` 包）。这两个库提供比 n-gram 更精准的分词，但速度稍慢。 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 必须包含所有中文字符，可以使用 `cont`、`cjk` 或 `chinese` 字符集来实现。当设置 `morphology=icu_chinese` 或 `morphology=jieba_chinese` 时，文档首先由 ICU 或 Jieba 预处理，然后分词器根据 charset_table 处理结果，最后应用 `morphology` 选项的其他形态处理器。仅包含中文的文本部分会传递给 ICU/Jieba 进行分词，其他部分可以通过不同方法（如不同形态学处理或 `charset_table`）进行修改。

内置的英语和俄语词干提取器速度快于对应的 libstemmer 版本，但可能产生略有不同的结果。

Soundex 实现与 MySQL 一致。Metaphone 实现基于双重 Metaphone 算法，索引其主码。

要使用 `morphology` 选项，请指定一个或多个内置选项，包括：
* none：不执行任何形态学处理
* lemmatize_ru - 使用俄语词形还原器，选择单一根形式
* lemmatize_uk - 使用乌克兰语词形还原器，选择单一根形式（请先在 [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) 或 [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) 安装）。为保证词形还原器正常工作，务必保留 `charset_table` 中的乌克兰特有字符，因默认不会保留。可通过如下方法覆盖：`charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`。[此处](https://play.manticoresearch.com/ua-lemmatizer/) 有一个关于如何安装和使用乌克兰词形还原器的交互课程。
* lemmatize_en - 使用英语词形还原器，选择单一根形式
* lemmatize_de - 使用德语词形还原器，选择单一根形式
* lemmatize_ru_all - 使用俄语词形还原器，索引所有可能的根形式
* lemmatize_uk_all - 使用乌克兰语词形还原器，索引所有可能的根形式。安装链接见上，注意 `charset_table` 设置。
* lemmatize_en_all - 使用英语词形还原器，索引所有可能的根形式
* lemmatize_de_all - 使用德语词形还原器，索引所有可能的根形式
* stem_en - 使用 Porter's 英语词干提取器
* stem_ru - 使用 Porter's 俄语词干提取器
* stem_enru - 使用 Porter's 英语和俄语词干提取器
* stem_cz - 使用捷克语词干提取器
* stem_ar - 使用阿拉伯语词干提取器
* soundex - 用 SOUNDEX 代码替换关键词
* metaphone - 用 METAPHONE 代码替换关键词
* icu_chinese - 使用 ICU 进行中文分词
* jieba_chinese - 使用 Jieba 进行中文分词（需安装 `manticore-language-packs` 包）
* libstemmer_* 。详情请参考[支持语言列表](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)

可以指定多个词干提取器，用逗号分隔。它们将按列出的顺序应用于传入的单词，一旦其中一个词干提取器修改了单词，处理将停止。此外，当启用[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能时，将首先在词形变化字典中查找该词。如果字典中有匹配的条目，则完全不会应用词干提取器。[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)可以用来实现词干提取的例外情况。

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

要跳过形态学预处理的字段列表。可选，默认是空（对所有字段应用预处理器）。

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

词干提取器并不完美，有时可能产生不理想的结果。例如，将"gps"关键词通过英语的Porter词干提取器处理会得到"gp"，这并不是预期的结果。`min_stemming_len`功能允许您根据源单词长度抑制词干提取，即避免对过短的单词进行词干提取。比给定阈值短的关键词将不会被词干提取。请注意，长度恰好等于指定值的关键词**会**被词干提取。所以要避免对3字符关键词进行词干提取，您应该将值设为4。若需更细粒度的控制，请参考[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能。

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

此选项允许对原始关键词以及它们经过形态学修改的版本进行索引。但因[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)和[exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md)重映射的原始关键词无法被索引。默认值为0，表示默认禁用此功能。

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

启用或禁用Jieba分词工具中的HMM。可选，默认是1。

在Jieba中，HMM（隐马尔可夫模型）选项指的是用于分词的算法。具体来说，它允许Jieba通过识别未知词，特别是词典中不存在的词，进行中文分词。

Jieba主要使用基于词典的方法对已知词进行分词，但在启用HMM选项时，它会应用统计模型来识别词典中不存在的词语或短语的可能词边界。这对于分割新词、罕见词、名称和俚语特别有用。

总之，`jieba_hmm`选项有助于提高分词准确性，但会牺牲索引性能。它必须与`morphology = jieba_chinese`一起使用，详见[中文、日文、韩文（CJK）和泰国语言](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba 分词模式。可选；默认是 `accurate`。

在精准模式下，Jieba 使用词典匹配将句子切分为最精确的词语。该模式侧重于精准度，确保分词尽可能准确。

在全模式下，Jieba 尝试将句子切分成所有可能的词语组合，旨在包含所有潜在的词语。该模式侧重于最大化召回率，即尽可能识别所有词语，即便其中有重叠或较少使用的词。它返回词典中所有找到的词语。

在搜索模式下，Jieba 将文本切分为完整词和较小部分，结合精准分词与额外细节，通过提供重叠的词片段。该模式在精准度和召回率之间取得平衡，适合搜索引擎使用。

`jieba_mode` 应与 `morphology = jieba_chinese` 一起使用。参见 [中文、日文、韩文（CJK）和泰语](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba 用户词典的路径。可选。

Jieba 是一个中文文本分词库，使用词典文件辅助分词。这些词典文件的格式如下：每行包含一个词，分为三部分用空格分隔——词语本身、词频和词性标签。词频和词性标签是可选的，可以省略。词典文件必须是 UTF-8 编码。

示例：

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` 应与 `morphology = jieba_chinese` 一起使用。详情见 [中文、日文、韩文（CJK）和泰语](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

