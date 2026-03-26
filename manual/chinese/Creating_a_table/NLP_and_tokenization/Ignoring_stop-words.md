# 忽略停用词

停用词是在索引和搜索过程中被忽略的词语，通常是因为它们的频率高且对搜索结果的贡献较低。

Manticore Search 默认会对停用词应用 [stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md)，这可能导致不理想的结果，但可以使用 [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed) 来关闭此功能。

小型停用词文件存储在表头中，嵌入文件的大小有限制，由 [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit) 选项定义。

停用词不会被索引，但会影响关键词的位置。例如，如果 "the" 是一个停用词，文档 1 包含短语 "in office"，而文档 2 包含短语 "in the office"，搜索 "in office" 作为精确短语时，只会返回第一个文档，即使第二个文档中的 "the" 被跳过作为停用词。此行为可以通过 [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) 指令进行修改。

## stopwords

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
stopwords 设置是可选的，默认为空。它允许您指定一个或多个停用词文件的路径，用空格分隔。所有文件都将被加载。在实时模式下，仅允许使用绝对路径。

停用词文件格式是简单的纯文本，使用 UTF-8 编码。文件数据将根据 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 设置进行分词，因此您可以使用与索引数据相同的分隔符。

当 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 索引处于活动状态时，由 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 中字符组成的停用词本身会被分词为 N-gram。因此，每个单独的 N-gram 都会成为单独的停用词。例如，使用 `ngram_len=1` 和合适的 `ngram_chars`，停用词 `test` 将被解释为 `t`、`e`、`s`、`t` 四个不同的停用词。

停用词文件可以手动或半自动创建。[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 提供了一种模式，可以创建表的频率字典，按关键词频率排序。该字典中的顶级关键词通常可以用作停用词。有关详细信息，请参阅 [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 和 [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 开关。该字典中的顶级关键词通常可以用作停用词。


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

或者，您可以使用 Manticore 自带的默认停用词文件。目前有 50 种语言的停用词可用。以下是它们的完整别名列表：

* af - 阿非利卡语
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
例如，要在配置文件中使用意大利语的停用词，请添加以下行：


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
如果需要使用多种语言的停用词，应列出所有别名，用逗号（RT 模式）或空格（plain 模式）分隔：


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

## stopwords_list

```ini
stopwords_list = 'value1; value2; ...'
```

<!-- example stopwords_list -->
设置 `stopwords_list` 允许您直接在 `CREATE TABLE` 语句中指定停用词。它仅支持在 [RT 模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) 中。

值必须用分号（`;`）分隔。如果需要使用分号作为字面字符，必须用反斜杠（`\;`）转义。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords_list = 'a; the'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords_list = 'a; the'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords_list' => 'a; the'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords_list = \'a; the\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords_list = \'a; the\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords_list = \'a; the\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords_list = 'a; the'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords_list = 'a; the'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords_list = 'a; the'", Some(true)).await;
```

<!-- end -->

## stopword_step

```ini
stopword_step={0|1}
```

<!-- example stopword_step -->
在 [停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) 上的 position_increment 设置是可选的，允许的值为 0 和 1，默认值为 1。


<!-- intro -->
##### SQL:

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
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", true);
```

<!-- intro -->
##### Rust:

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
在词干提取之前或之后应用停用词。可选，默认值为 0（在词干提取之后应用停用词过滤器）。

默认情况下，停用词本身会被词干提取，然后应用于词干提取（或任何其他形态处理）后的标记。这意味着当 stem(token) 等于 stem(stopword) 时，标记会被停止。这种默认行为可能导致意外结果，当标记被错误地词干提取为被停止的根时。例如，“Andes”可能会被词干提取为“and”，所以当“and”是停用词时，“Andes”也会被跳过。

但是，您可以通过启用 `stopwords_unstemmed` 指令来更改此行为。启用后，停用词会在词干提取之前应用（因此应用于原始词形），当标记等于停用词时，标记会被跳过。

<!-- intro -->
##### SQL:

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
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", true);
```

<!-- intro -->
##### Rust:

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

