# 忽略停用词

停用词是在索引和搜索过程中被忽略的词语，通常是因为它们出现频率高但对搜索结果价值低。

Manticore Search 默认对停用词应用[词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md)，这可能导致不理想的结果，但可以通过 [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed) 选项关闭此功能。

小的停用词文件存储在表头中，可嵌入的文件大小有限制，由 [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit) 选项定义。

停用词不会被索引，但它们会影响关键词的位置。例如，如果 "the" 是一个停用词，文档1包含短语 "in office"，而文档2包含短语 "in the office"，那么将 "in office" 作为精确短语搜索时，只会返回第一个文档，尽管第二个文档中的 "the" 作为停用词被跳过。此行为可以通过 [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) 指令修改。

## stopwords

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
stopwords 设置是可选的，默认为空。它允许您指定一个或多个停用词文件的路径，用空格分隔。所有文件都将被加载。在实时模式下，只允许绝对路径。

停用词文件格式是简单的纯文本，采用 UTF-8 编码。文件数据将根据 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 设置进行分词，因此您可以使用与索引数据相同的分隔符。

当 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 索引激活时，由属于 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 范围内的字符组成的停用词本身会被分词为 N-gram。因此，每个单独的 N-gram 成为一个独立的停用词。例如，在 `ngram_len=1` 和合适的 `ngram_chars` 设置下，停用词 `test` 将被解释为 `t`、`e`、`s`、`t` 四个不同的停用词。

停用词文件可以手动或半自动创建。[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 提供了一种模式，可以创建表的频率词典，按关键词频率排序。该词典中的高频词通常可以用作停用词。详情请参见 [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 和 [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 开关。该词典中的高频词通常可以用作停用词。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt stopwords-ru.txt stopwords-en.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => '/usr/local/manticore/data/stopwords.txt stopwords-ru.txt stopwords-en.txt'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = /usr/local/manticore/data/stopwords.txt
  stopwords = stopwords-ru.txt stopwords-en.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

或者，您可以使用 Manticore 自带的默认停用词文件。目前支持 50 种语言的停用词。以下是它们的完整别名列表：

* af - 南非荷兰语
* ar - 阿拉伯语
* bg - 保加利亚语
* bn - 孟加拉语
* ca - 加泰罗尼亚语
* ckb- 库尔德语
* cz - 捷克语
* da - 丹麦语
* de - 德语
* el - 希腊语
* en - 英语
* eo - 世界语
* es - 西班牙语
* et - 爱沙尼亚语
* eu - 巴斯克语
* fa - 波斯语
* fi - 芬兰语
* fr - 法语
* ga - 爱尔兰语
* gl - 加利西亚语
* hi - 印地语
* he - 希伯来语
* hr - 克罗地亚语
* hu - 匈牙利语
* hy - 亚美尼亚语
* id - 印度尼西亚语
* it - 意大利语
* ja - 日语
* ko - 韩语
* la - 拉丁语
* lt - 立陶宛语
* lv - 拉脱维亚语
* mr - 马拉地语
* nl - 荷兰语
* no - 挪威语
* pl - 波兰语
* pt - 葡萄牙语
* ro - 罗马尼亚语
* ru - 俄语
* sk - 斯洛伐克语
* sl - 斯洛文尼亚语
* so - 索马里语
* st - 索托语
* sv - 瑞典语
* sw - 斯瓦希里语
* th - 泰语
* tr - 土耳其语
* yo - 约鲁巴语
* zh - 中文
* zu - 祖鲁语

<!-- example stopwords 1 -->
例如，要使用意大利语的停用词，只需在配置文件中添加以下行：


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'it'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'it'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'it'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'it'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = 'it'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = 'it'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = it

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example stopwords 2 -->
如果您需要使用多种语言的停用词，应列出所有语言的别名，用逗号（RT 模式）或空格（普通模式）分隔：


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = en it ru

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## stopword_step

```ini
stopword_step={0|1}
```

<!-- example stopword_step -->
The position_increment setting on [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) is optional, and the allowed values are 0 and 1, with the default being 1.


<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru',
            'stopword_step' => '1'
        ]);
```
<!-- intro -->
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", true);
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", true);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = en
  stopword_step = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## stopwords_unstemmed

```ini
stopwords_unstemmed={0|1}
```

<!-- example stopwords_unstemmed -->
Whether to apply stop words before or after stemming. Optional, default is 0 (apply stop word filter after stemming).

By default, stop words are stemmed themselves, and then applied to tokens *after* stemming (or any other morphology processing). This means that a token is stopped when stem(token) is equal to stem(stopword). This default behavior can lead to unexpected results when a token is erroneously stemmed to a stopped root. For example, "Andes" might get stemmed to "and", so when "and" is a stopword, "Andes" is also skipped.

However, you can change this behavior by enabling the `stopwords_unstemmed` directive. When this is enabled, stop words are applied before stemming (and therefore to the original word forms), and the tokens are skipped when the token is equal to the stopword.

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru',
            'stopwords_unstemmed' => '1'
        ]);
```
<!-- intro -->
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", true);
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", true);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = en
  stopwords_unstemmed = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

