# 高级词形处理

词形处理预处理器可以在索引过程中应用于单词，以规范化同一单词的不同形式并改进分词。例如，英语词干提取器可以将 "dogs" 和 "dog" 规范化为 "dog"，从而使得这两个关键词的搜索结果相同。

Manticore 有四种内置的词形处理预处理器：

*   **词形还原器**：将单词还原为其词根或词形。例如，"running" 可以还原为 "run"，"octopi" 可以还原为 "octopus"。请注意，某些单词可能有多个对应的词根形式。例如，"dove" 可以是 "dive" 的过去式，也可以是名词“鸽子”的意思，如 "A white dove flew over the cuckoo's nest." 在这种情况下，词形还原器可以生成所有可能的词根形式。
*   **词干提取器**：通过删除或替换某些已知的后缀将单词还原为词干。生成的词干可能不是有效的单词。例如，Porter 英语词干提取器将 "running" 还原为 "run"，将 "business" 还原为 "busi"（不是有效单词），并且不会还原 "octopi"。
*   **语音算法**：将单词替换为语音代码，即使单词不同但语音接近，这些代码也相同。
*   **分词算法**：将文本拆分为单词。目前仅适用于中文。

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
morphology 指令指定应用于索引单词的词形处理预处理器列表。这是一个可选设置，默认情况下不应用任何预处理器。

Manticore 提供了以下内置的词形处理预处理器：

* 英语、俄语和德语的词形还原器
* 英语、俄语、阿拉伯语和捷克语的词干提取器
* SoundEx 和 MetaPhone 语音算法
* 中文分词算法
* 还有超过 [15 种其他语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) 的 Snowball（libstemmer）词干提取器也提供支持。

词形还原器需要使用 `.pak` 格式的词典文件，这些文件可以通过 `manticore-language-packs` 包安装或 [从 Manticore 网站下载](https://manticoresearch.com/install/#other-downloads)。在后一种情况下，词典需要放在 [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) 指定的目录中。

此外，可以使用 [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) 设置通过使用更多 RAM 来加快词形还原速度，该设置会为未压缩的词典缓存分配更多内存。

中文分词可以使用 [ICU](http://site.icu-project.org/) 或 [Jieba](https://github.com/yanyiwu/cppjieba)（需要 `manticore-language-packs` 包）。这两种库提供的分词精度比 n-gram 更高，但速度稍慢。[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 必须包含所有中文字符，这可以通过 `cont`、`cjk` 或 `chinese` 字符集实现。当设置 `morphology=icu_chinese` 或 `morphology=jieba_chinese` 时，文档首先会通过 ICU 或 Jieba 进行预处理。然后，分词器根据 charset_table 处理结果，最后应用 `morphology` 选项中的其他词形处理程序。只有包含中文的部分会传递给 ICU/Jieba 进行分词，而其他部分可以通过不同的方式修改，例如不同的词形处理或 `charset_table`。

内置的英语和俄语词干提取器比其 libstemmer 对应版本更快，但可能会产生略微不同的结果。

Soundex 实现与 MySQL 一致。Metaphone 实现基于 Double Metaphone 算法，并索引主代码。

要使用 `morphology` 选项，请指定一个或多个内置选项，包括：
* none：不进行任何词形处理
* lemmatize_ru - 应用俄语词形还原器并选择一个词根形式
* lemmatize_uk - 应用乌克兰语词形还原器并选择一个词根形式
* lemmatize_en - 应用英语词形还原器并选择一个词根形式
* lemmatize_de - 应用德语词形还原器并选择一个词根形式
* lemmatize_ru_all - 应用俄语词形还原器并索引所有可能的词根形式
* lemmatize_uk_all - 应用乌克兰语词形还原器并索引所有可能的词根形式
* lemmatize_en_all - 应用英语词形还原器并索引所有可能的词根形式
* lemmatize_de_all - 应用德语词形还原器并索引所有可能的词根形式
* stem_en - 应用 Porter 英语词干提取器
* stem_ru - 应用 Porter 俄语词干提取器
* stem_enru - 应用 Porter 英语和俄语词干提取器
* stem_cz - 应用捷克语词干提取器
* stem_ar - 应用阿拉伯语词干提取器
* soundex - 将关键词替换为其 SOUNDEX 代码
* metaphone - 将关键词替换为其 METAPHONE 代码
* icu_chinese - 使用 ICU 进行中文文本分词
* jieba_chinese - 使用 Jieba 进行中文文本分词（需要 `manticore-language-packs` 包）
* libstemmer_* 。有关详细信息，请参阅 [支持的语言列表](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)

可以指定多个词干提取器，用逗号分隔。它们将按照列出的顺序应用于传入的单词，一旦其中一个词干提取器修改了单词，处理就会停止。此外，当启用 [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 功能时，单词会首先在词形字典中查找。如果字典中有匹配项，将完全不应用词干提取器。[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 可用于实现词干提取的例外情况。

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

用于跳过形态学预处理的字段列表。可选，默认为空（对所有字段应用预处理器）。

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

启用词干提取的最小词长。可选，默认为 1（对所有词进行词干提取）。

词干提取器并不完美，有时可能会产生不理想的结果。例如，将 "gps" 关键词通过英语的 Porter 词干提取器处理后会得到 "gp"，这显然不是预期的结果。`min_stemming_len` 功能允许您根据源词的长度来抑制词干提取，即避免对过短的词进行词干提取。长度小于指定阈值的关键词将不会被提取。请注意，长度正好等于指定值的关键词 **将会** 被提取。因此，为了避免对三字符关键词进行提取，应将值设为 4。如需更精细的控制，请参考 [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 功能。

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

此选项允许对原始关键词及其形态学修改后的版本进行索引。但是，通过 [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 和 [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md) 重新映射的原始关键词无法被索引。默认值为 0，表示此功能默认禁用。

此功能允许在查询语言中使用 [精确形式操作符](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier)。启用此功能会增加全文索引的大小和索引时间，但不会影响搜索性能。

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

启用或禁用 Jieba 分词工具中的 HMM（隐马尔可夫模型）。可选，默认为 1。

在 Jieba 中，HMM 选项指的是用于分词的算法。具体来说，它允许 Jieba 通过识别不在其词典中的未知词（尤其是未收录的词）来进行中文分词。

Jieba 主要使用基于词典的方法对已知词进行分词，但当启用 HMM 选项时，它会应用统计模型来识别不在其词典中的词或短语的可能分词边界。这对于分割新词、生僻词、人名和俚语特别有用。

总之，`jieba_hmm` 选项通过牺牲索引性能来提高分词准确性。它必须与 `morphology = jieba_chinese` 一起使用，参见 [中文、日文和韩文（CJK）及泰语](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba 分词模式。可选，默认为 `accurate`。

在精确模式下，Jieba 通过词典匹配将句子拆分为最精确的词语。此模式注重精确度，确保分词尽可能准确。

在完整模式下，Jieba 尝试将句子拆分为所有可能的词语组合，旨在包含所有潜在的词语。此模式注重召回率，即尽可能多地识别词语，即使其中一些词语重叠或使用频率较低。它会返回其词典中找到的所有词语。

在搜索模式下，Jieba 将文本拆分为完整的单词和更小的部分，通过提供重叠的单词片段，结合精确的分词和额外的细节。此模式在精确度和召回率之间取得平衡，适用于搜索引擎。

`jieba_mode` 应与 `morphology = jieba_chinese` 一起使用。请参见 [中文、日文、韩文（CJK）和泰语](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba 用户字典的路径。可选。

Jieba 是一个中文文本分词库，使用字典文件来辅助分词。这些字典文件的格式如下：每行包含一个单词，分为三部分，用空格分隔 —— 单词本身、词频和词性（POS）标签。词频和词性标签是可选的，可以省略。字典文件必须使用 UTF-8 编码。

示例：

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` 应与 `morphology = jieba_chinese` 一起使用。更多详情请参见 [中文、日文、韩文（CJK）和泰语](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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
