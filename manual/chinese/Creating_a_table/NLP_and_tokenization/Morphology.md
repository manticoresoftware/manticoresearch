# 高级形态学

在索引过程中，可以对词语应用形态学预处理器，以规范同一词语的不同形式并改善分词。例如，英文词干提取器可以将“dogs”和“dog”规范化为“dog”，从而对这两个关键字产生相同的搜索结果。

Manticore 提供四种内置的形态学预处理器：

*   **词形还原**：将词语还原到其根或词根。例如，“running”可以被还原为“run”，“octopi”可以被还原为“octopus”。注意，有些词可能有多个对应的根形式。例如，“dove”可以是“dive”的过去式或是指鸟的名词，如“白色鸽子飞过杜鹃的巢。”在这种情况下，词形还原器可以生成所有可能的根形式。
*   **词干提取器**：通过移除或替换某些已知后缀来将词语还原到其词干。生成的词干不一定是有效单词。例如，Porter 英文词干提取器将“running”还原为“run”，“business”还原为“busi”（不是有效单词），并且不会对“octopi”进行任何还原。
*   **音码算法**：用音码替换单词，即使这些单词不同但在发音上相近。
*   **分词算法**：将文本分割为单词。目前仅适用于中文。

## 形态学

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- 示例形态学 -->
形态学指令指定了一系列要应用于被索引词语的形态学预处理器。这是一个可选设置，默认情况下不应用任何预处理器。

Manticore 提供以下内置形态学预处理器：

* 英文、俄文和德文词形还原器
* 英文、俄文、阿拉伯文和捷克文的词干提取器
* SoundEx 和 MetaPhone 音码算法
* 中文分词算法
* 还有用于超过 [15 种其他语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) 的 Snowball (libstemmer) 词干提取器可用。

词形还原器需要字典 `.pak` 文件，可以通过 `manticore-language-packs` 包安装或从 [Manticore 网站下载](https://manticoresearch.com/install/#other-downloads)。在后者情况下，字典需要放在 [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) 指定的目录中。

此外，[lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) 设置可以通过为未压缩的字典缓存分配更多 RAM 来加速词形还原。

中文语言分割可以使用 [ICU](http://site.icu-project.org/) 或 [Jieba](https://github.com/yanyiwu/cppjieba)（需要 `manticore-language-packs` 包）。这两个库提供比 n-grams 更准确的分割，但速度稍慢。[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 必须包含所有中文字符，可以使用 `cont`、`cjk` 或 `chinese` 字符集来实现。当你设置 `morphology=icu_chinese` 或 `morphology=jieba_chinese` 时，文档首先通过 ICU 或 Jieba 进行预处理。然后，分词器根据 charset_table 处理结果，最后应用 `morphology` 选项中的其他形态学处理器。只有包含中文的文本部分会传递给 ICU/Jieba 进行分割，而其他部分可以通过不同的方式修改，例如不同的形态学或 `charset_table`。

内置的英文和俄文词干提取器的速度比其 libstemmer 对应物更快，但可能会产生略微不同的结果

Soundex 实现与 MySQL 的实现匹配。Metaphone 实现基于双音码算法并索引主要代码。

要使用 `morphology` 选项，请指定一个或多个内置选项，包括：
* none：不执行任何形态学处理
* lemmatize_ru - 应用俄文词形还原器并选择单一根形式
* lemmatize_uk - 应用乌克兰文词形还原器并选择单一根形式（首先在 [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) 或 [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) 中安装）。为了确保词形还原器的正确工作，请确保在 `charset_table` 中保留特定的乌克兰字符，因为默认情况下它们不会被保留。为此，覆盖它们，如下所示：`charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`。[这里](https://play.manticoresearch.com/ua-lemmatizer/) 有一个关于如何安装和使用乌克兰词形还原器的互动课程。
* lemmatize_en - 应用英文词形还原器并选择单一根形式
* lemmatize_de - 应用德文词形还原器并选择单一根形式
* lemmatize_ru_all - 应用俄文词形还原器并索引所有可能的根形式
* lemmatize_uk_all - 应用乌克兰文词形还原器并索引所有可能的根形式。有关安装链接，请参见上文，并注意 `charset_table`。
* lemmatize_en_all - 应用英文词形还原器并索引所有可能的根形式
* lemmatize_de_all - 应用德文词形还原器并索引所有可能的根形式
* stem_en - 应用 Porter 的英文词干提取器
* stem_ru - 应用 Porter 的俄文词干提取器
* stem_enru - 应用 Porter 的英文和俄文词干提取器
* stem_cz - 应用捷克文词干提取器
* stem_ar - 应用阿拉伯文词干提取器
* soundex - 用其 SOUNDEX 代码替换关键字
* metaphone - 用其 METAPHONE 代码替换关键字
* icu_chinese - 使用 ICU 应用中文文本分割
* jieba_chinese - 使用 Jieba 应用中文文本分割（需要 `manticore-language-packs` 包）
* libstemmer_* . 有关详细信息，请参阅 [支持语言列表](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)
可以使用多个词干提取器，用逗号分隔。这些词干提取器将按照列出的顺序应用于传入的词，并且一旦其中一个词干提取器修改了词，处理就会停止。另外，当启用[词形变化](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能时，将首先在词形变化词典中查找该词。如果词典中存在匹配项，则不会应用词干提取器。[词形变化](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)可用于实现词干提取例外情况。

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

启用词干提取的最小单词长度。可选，默认为1（对所有内容进行词干提取）。

词干提取器并不完美，有时可能会产生不希望的结果。例如，通过英语波特词干提取器处理"gps"关键词会得到"gp"，这并不是本意。`min_stemming_len`功能允许您根据源词长度抑制词干提取，即避免对过短的词进行词干提取。短于给定阈值的关键词将不会被词干提取。请注意，长度恰好等于指定值的关键词**将**被词干提取。因此，为避免对3个字符的关键词进行词干提取，您应该指定4作为值。要获得更精细的控制，请参考[词形变化](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)功能。

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

此选项允许索引原始关键词及其形态学修改后的版本。但是，被[词形映射](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)和[例外](../../Creating_a_table/NLP_and_tokenization/Exceptions.md)重新映射的原始关键词无法被索引。默认值为0，表示此功能默认禁用。

这允许在查询语言中使用[精确形式运算符](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier)。启用此功能将增加全文索引大小和索引时间，但不会影响搜索性能。

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

启用或禁用Jieba分词工具中的HMM。可选；默认为1。

在Jieba中，HMM（隐马尔可夫模型）选项是指用于分词的算法。具体来说，它允许Jieba通过识别未知词语来执行中文分词，特别是那些不在其词典中的词语。

Jieba主要使用基于词典的方法来分割已知词语，但当启用HMM选项时，它会应用统计模型来识别不在其词典中的词语或短语的可能词边界。这对于分割新词、罕见词、名称和俚语特别有用。

总的来说，`jieba_hmm`选项通过牺牲索引性能来帮助提高分词准确性。必须与`morphology = jieba_chinese`一起使用，请参见[中文、日文和韩文（CJK）以及泰语](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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
jieba_mode = {准确|完整|搜索}
```

Jieba 分词模式。可选；默认是 `准确`。

在准确模式下，Jieba 通过字典匹配将句子拆分为最精确的词。此模式侧重于精度，确保分词尽可能准确。

在完整模式下，Jieba 尝试将句子拆分为每个可能的词组合，旨在包括所有潜在的词。此模式侧重于最大化召回率，这意味着它识别尽可能多的词，即使其中一些重叠或使用较少。它返回字典中找到的所有词。

在搜索模式下，Jieba 将文本拆分为完整词和更小的部分，通过提供重叠的词片段结合精确的分词和额外的细节。此模式在精度和召回率之间取得平衡，使其对搜索引擎非常有用。

`jieba_mode` 应与 `morphology = jieba_chinese` 一起使用。请参阅 [中文、日文、韩文 (CJK) 和泰文](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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

Jieba 用户字典的路径。可选。

Jieba 是一个中文文本分词库，使用字典文件来辅助分词。这些字典文件的格式如下：每行包含一个词，由空格分隔为三个部分——词本身、词频和词性（POS）标签。词频和词性标签是可选的，可以省略。字典文件必须为 UTF-8 编码。

示例：

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` 应与 `morphology = jieba_chinese` 一起使用。有关更多详细信息，请参阅 [中文、日文、韩文 (CJK) 和泰文](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。

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


