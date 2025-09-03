# 高级形态学

形态学预处理器可以在索引时应用于单词，以规范同一单词的不同形式并改善分词。例如，英文词干提取器可以将 "dogs" 和 "dog" 规范为 "dog"，使两个关键字的搜索结果相同。

Manticore 有四种内置形态学预处理器：

*   **词形还原器（Lemmatizer）**：将单词缩减为其词根或词元。例如，"running" 可以缩减为 "run"，"octopi" 可以缩减为 "octopus"。注意有些单词可能对应多个词根形式。例如，"dove" 可以是 "dive" 的过去式，也可以是表示鸟类的名词，如 “A white dove flew over the cuckoo's nest.” 在这种情况下，词形还原器可以生成所有可能的词根形式。
*   **词干提取器（Stemmer）**：通过移除或替换某些已知后缀将单词缩减为词干。结果词干未必是一个有效单词。例如，Porter 英文词干提取器将 "running" 缩减为 "run"，将 "business" 缩减为 "busi"（不是一个有效词），并且不缩减 "octopi"。
*   **语音算法（Phonetic algorithms）**：将词替换为拼音代码，即使单词不同但发音相近，代码也是相同的。
*   **分词算法（Word breaking algorithms）**：将文本拆分成单词。目前仅支持中文。

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
morphology 指令指定一个要应用于被索引单词的形态学预处理器列表。这是一个可选设置，默认不应用任何预处理器。

Manticore 内置了针对以下语言的形态学预处理器：

* 英文、俄文和德文词形还原器
* 英文、俄文、阿拉伯文和捷克语词干提取器
* SoundEx 和 MetaPhone 语音算法
* 中文分词算法
* 还提供适用于超过[15种其他语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)的 Snowball（libstemmer）词干提取器。

词形还原器需要字典 `.pak` 文件，可以通过安装 `manticore-language-packs` 软件包获得，或者从[Manticore官网](https://manticoresearch.com/install/#other-downloads)下载。如果是后一种方式，字典需要放置在由[lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) 指定的目录中。

此外，可以使用[lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) 设置，通过更多RAM使用来加速词形还原的无压缩字典缓存。

中文分词可以使用 [ICU](http://site.icu-project.org/) 或 [Jieba](https://github.com/yanyiwu/cppjieba)（需要 `manticore-language-packs` 包）。这两个库提供比n-gram更准确的分词，但速度略慢。[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 必须包含所有中文字符，可以使用 `cont`、`cjk` 或 `chinese` 字符集完成此操作。当设置 `morphology=icu_chinese` 或 `morphology=jieba_chinese` 时，文档会先由 ICU 或 Jieba 预处理，然后分词器根据 charset_table 处理结果，最后应用 `morphology` 选项中其他形态学处理。文本中只有包含中文的部分会被传递给 ICU/Jieba 进行分词，其它部分可以通过不同的方式修改，例如不同的形态学处理或 `charset_table`。

内置的英文和俄文词干提取器比 libstemmer 对应版本速度更快，但结果可能略有不同。

Soundex 实现与 MySQL 相同。Metaphone 实现基于双重 Metaphone 算法，只索引主代码。

使用 `morphology` 选项时，可以指定一个或多个内置选项，包括：
* none：不执行任何形态学处理
* lemmatize_ru - 应用俄文词形还原器并选择单一词根形式
* lemmatize_uk - 应用乌克兰语词形还原器并选择单一词根形式（需先在 [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) 或 [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) 上安装）。为确保词形还原器正确工作，确保在你的 `charset_table` 中保留了特定乌克兰字符，因为默认情况下这些字符没有被保留。可通过如下方式覆盖：`charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`。[这里](https://play.manticoresearch.com/ua-lemmatizer/) 有一个关于如何安装和使用乌克兰词形还原器的互动课程。
* lemmatize_en - 应用英文词形还原器并选择单一词根形式
* lemmatize_de - 应用德文词形还原器并选择单一词根形式
* lemmatize_ru_all - 应用俄文词形还原器并索引所有可能词根形式
* lemmatize_uk_all - 应用乌克兰词形还原器并索引所有可能词根形式。请参见上述安装链接并注意 `charset_table` 设置。
* lemmatize_en_all - 应用英文词形还原器并索引所有可能词根形式
* lemmatize_de_all - 应用德文词形还原器并索引所有可能词根形式
* stem_en - 应用 Porter 英文词干提取器
* stem_ru - 应用 Porter 俄文词干提取器
* stem_enru - 同时应用 Porter 英文和俄文词干提取器
* stem_cz - 应用捷克语词干提取器
* stem_ar - 应用阿拉伯语词干提取器
* soundex - 用 SOUNDEX 代码替换关键字
* metaphone - 用 METAPHONE 代码替换关键字
* icu_chinese - 使用 ICU 应用中文分词
* jieba_chinese - 使用 Jieba 应用中文分词（需要 `manticore-language-packs` 软件包）
* libstemmer_* . 详见[支持语言列表](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)

可以指定多个词干提取器，用逗号分隔。它们将按照列出的顺序应用于输入单词，一旦其中一个词干提取器修改了单词，处理将停止。此外，当启用[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能时，单词将首先在词形词典中查找。如果词典中有匹配的条目，则根本不会应用词干提取器。[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 可用于实现词干提取例外。

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

要跳过形态学预处理的字段列表。可选，默认为空（对所有字段应用预处理器）。

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

启用词干提取的最小单词长度。可选，默认值为1（提取所有单词的词干）。

词干提取器并非完美，有时可能会产生不良结果。例如，将“gps”关键字通过英文的Porter词干提取器处理后会变成“gp”，这并非真正意图。`min_stemming_len`功能允许你根据源单词长度抑制词干提取，即避免对过短单词进行词干提取。比指定阈值更短的关键字将不会被词干提取。注意，正好等于指定长度的关键字**会**被词干提取。因此，为了避免对3个字符的关键字进行词干提取，应将值指定为4。有关更细粒度的控制，请参考[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能。

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

此选项允许索引原始关键字及其形态学修改后的版本。然而，被[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)和[exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md)重新映射的原始关键字不能被索引。默认值为0，表示默认禁用此功能。

这允许在查询语言中使用[精确形式操作符](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier)。启用此功能会增加全文索引大小和索引时间，但不会影响搜索性能。

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

在Jieba中，HMM（隐马尔可夫模型）选项指的是用于分词的算法。具体来说，它允许Jieba通过识别未知词来进行中文分词，特别是词典中不存在的词。

Jieba主要使用基于词典的方法对已知词进行分词，但当启用HMM选项时，它会应用统计模型来识别词典中不存在的单词或短语的可能词边界。这对于分割新词、罕见词、名字和俚语尤其有用。

总之，`jieba_hmm`选项有助于提高分词准确性，但会以索引性能为代价。它必须与`morphology = jieba_chinese`一起使用，详见[中文、日文和韩文（CJK）及泰语](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba分词模式。可选；默认是`accurate`。

在精确模式下，Jieba 使用词典匹配将句子切分为最精确的词语。此模式侧重于精确性，确保分词尽可能准确。

在全模式下，Jieba 尝试将句子切分为所有可能的词语组合，旨在包含所有潜在词语。此模式侧重于最大化召回率，即识别尽可能多的词语，即使其中一些词语重叠或较少使用。它返回词典中找到的所有词语。

在搜索模式下，Jieba 将文本切分为完整词语和更小的部分，结合了精确分词和额外细节，通过提供重叠的词语片段。此模式平衡了精确性和召回率，非常适合搜索引擎使用。

`jieba_mode` 应与 `morphology = jieba_chinese` 配合使用。详情见[中文、日文、韩文（CJK）和泰语语言](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba 用户词典路径。可选。

Jieba 是一个中文文本分词库，使用词典文件来辅助分词。这些词典文件的格式如下：每行包含一个词语，分为三部分，用空格分隔——词语本身、词频和词性（POS）标记。词频和词性是可选的，可以省略。词典文件必须是 UTF-8 编码。

示例：

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` 应与 `morphology = jieba_chinese` 配合使用。详情见[中文、日文、韩文（CJK）和泰语语言](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

