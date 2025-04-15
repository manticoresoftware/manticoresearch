# 忽略停用词

停用词是指在索引和搜索过程中被忽略的单词，通常是因为它们的高频率和对搜索结果的低价值。 

Manticore Search 默认对停用词应用 [词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md)，这可能导致不希望的结果，但可以通过使用 [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed) 将其关闭。

小型停用词文件存储在表头中，嵌入文件的大小有限制，具体由 [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit) 选项定义。

停用词未被索引，但会影响关键字位置。例如，如果 "the" 是一个停用词，并且文档 1 包含短语 "in office"，而文档 2 包含短语 "in the office"，那么搜索 "in office" 作为精确短语只会返回第一个文档，即使在第二个文档中 "the" 被跳过作为停用词。这个行为可以通过使用 [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) 指令进行修改。

## 停用词

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
停用词设置是可选的，默认情况下为空。它允许您指定一个或多个停用词文件的路径，以空格分隔。所有文件都会被加载。在实时模式下，仅允许绝对路径。

停用词文件格式是简单的纯文本，采用 UTF-8 编码。文件数据将根据 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 设置进行分词，因此您可以使用与索引数据相同的分隔符。

停用词文件可以手动或半自动创建。 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 提供了一种模式，该模式创建表的频率词典，按关键字频率排序。该词典中的顶级关键字通常可以用作停用词。有关详细信息，请参见 [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 和 [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 开关。该词典中的顶级关键字通常可以用作停用词。


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'");
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

或者您可以使用 Manticore 附带的默认停用词文件。目前可用 50 种语言的停用词。以下是它们的全部别名列表：

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
* st - 塞索托语
* sv - 瑞典语
* sw - 斯瓦希里语
* th - 泰语
* tr - 土耳其语
* yo - 约鲁巴语
* zh - 中文
* zu - 祖鲁语

<!-- example stopwords 1 -->
例如，要在意大利语中使用停用词，只需在配置文件中加入以下行：


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'it'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = 'it'");
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
如果您需要为多种语言使用停用词，您应该列出它们所有的别名，用逗号（RT模式）或空格（普通模式）分隔：


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'");
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
[stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)上的position_increment设置是可选的，允许的值为0和1，默认值为1。


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'");
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
是否在词干提取之前或之后应用停用词。可选，默认值为0（在词干提取后应用停用词）。

默认情况下，停用词本身会被词干提取，然后在词干提取（或任何其他形态处理）后应用于令牌。这意味着，当stem(token)等于stem(stopword)时，令牌被停止。这种默认行为在令牌错误地词干提取为被停用的根时，可能导致意外结果。例如，“安第斯”可能被词干提取为“和”，所以当“和”是一个停用词时，“安第斯”也会被跳过。
然而，您可以通过启用 `stopwords_unstemmed` 指令来更改此行为。启用此功能后，停用词在词干提取之前应用（因此适用于原始单词形式），并且当标记等于停用词时，将跳过该标记。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'");
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

