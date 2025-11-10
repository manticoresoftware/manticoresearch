# 低级分词

当文本在 Manticore 中被索引时，它会被拆分成单词，并进行大小写折叠，以便像 "Abc"、"ABC" 和 "abc" 这样的单词被视为相同的单词。

为了正确执行这些操作，Manticore 必须知道：
* 源文本的编码（应始终为 UTF-8）
* 哪些字符被视为字母，哪些不是
* 哪些字母应折叠为其他字母

您可以使用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 选项按表配置这些设置。charset_table 指定一个数组，将字母字符映射到它们的大小写折叠版本（或您偏好的其他字符）。数组中未出现的字符被视为非字母，并在该表的索引或搜索过程中作为单词分隔符处理。

默认字符集是 `non_cont`，它包括[大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

您还可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

文本 `RED TUBE 5" LONG` 将被索引为 `COLOR TUBE 5 INCH LONG`，而 `PLANK 2" x 4"` 将被索引为 `PLANK 2 INCH x 4 INCH`。这些规则按指定顺序应用。规则也适用于查询，因此搜索 `BLUE TUBE` 实际上会搜索 `COLOR TUBE`。

您可以在[这里了解更多关于 regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)。

## 索引配置选项

### charset_table

```ini
# default
charset_table = non_cont

# only English and Russian letters
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# english charset defined with alias
charset_table = 0..9, english, _

# you can override character mappings by redefining them, e.g. for case insensitive search with German umlauts you can use:
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- example charset_table -->
`charset_table` 指定一个数组，将字母字符映射到它们的大小写折叠版本（或您偏好的其他字符）。默认字符集是 `non_cont`，它包括大多数使用[非连续](https://en.wikipedia.org/wiki/Scriptio_continua)脚本的语言。

`charset_table` 是 Manticore 分词过程的核心，它从文档文本或查询文本中提取关键词。它控制哪些字符被接受为有效字符以及它们应如何转换（例如是否去除大小写）。

默认情况下，每个字符映射为 0，表示它不被视为有效关键词，且被视为分隔符。一旦字符在表中被提及，它就会被映射到另一个字符（通常是映射到自身或小写字母），并被视为有效的关键词部分。

charset_table 使用逗号分隔的映射列表来声明字符为有效或将它们映射到其他字符。语法快捷方式可用于一次映射一系列字符：

* 单字符映射：`A->a`。声明源字符 'A' 在关键词中允许，并映射到目标字符 'a'（但不声明 'a' 为允许字符）。
* 范围映射：`A..Z->a..z`。声明源范围内的所有字符为允许，并映射到目标范围。不会声明目标范围为允许。会检查两个范围的长度。
* 单独字符映射：`a`。声明字符为允许，并映射到自身。等同于 `a->a` 单字符映射。
* 单独范围映射：`a..z`。声明范围内所有字符为允许，并映射到自身。等同于 `a..z->a..z` 范围映射。
* 棋盘范围映射：`A..Z/2`。将每对字符映射到第二个字符。例如，`A..Z/2` 等同于 `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。此映射快捷方式适用于大写和小写字母交错排列的 Unicode 块。

对于代码从 0 到 32 的字符，以及 127 到 8 位 ASCII 和 Unicode 字符范围内的字符，Manticore 总是将它们视为分隔符。为避免配置文件编码问题，8 位 ASCII 字符和 Unicode 字符必须以 `U+XXX` 形式指定，其中 `XXX` 是十六进制代码点编号。接受的最小 Unicode 字符代码是 `U+0021`。

如果默认映射不能满足您的需求，您可以通过再次指定映射来重新定义字符映射。例如，如果内置的 `non_cont` 数组包含字符 `Ä` 和 `ä` 并将它们都映射到 ASCII 字符 `a`，您可以通过添加它们的 Unicode 代码点来重新定义这些字符，如下所示：

```
charset_table = non_cont,U+00E4,U+00C4
```

用于区分大小写的搜索，或

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

用于不区分大小写的搜索。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = 0..9, A..Z->a..z, _, a..z, \
    U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example charset_table 2 -->
除了字符和映射的定义外，还有几个内置别名可用。当前别名有：
* `chinese`
* `cjk`
* `cont`
* `english`
* `japanese`
* `korean`
* `non_cont` (`non_cjk`)
* `russian`
* `thai`

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => '0..9, english, _'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = 0..9, english, _

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

如果您想在搜索中支持不同的语言，定义所有语言的有效字符集和折叠规则可能是一项繁重的工作。我们为您简化了这一过程，提供了默认的字符集表 `non_cont` 和 `cont`，分别涵盖了非连续和连续（中文、日文、韩文、泰文）脚本的语言。在大多数情况下，这些字符集应足以满足您的需求。

请注意，以下语言当前**不支持**：
* 阿萨姆语
* 比什努普里亚语
* 布希德语
* 加罗语
* 苗族语
* 霍语
* 科米语
* 大花苗语
* 马巴语
* 迈蒂利语
* 马拉地语
* 门德语
* 姆鲁语
* 米耶内语
* 恩甘贝语
* 奥里亚语
* 桑塔利语
* 信德语
* 锡尔赫蒂语

所有其他列在[Unicode语言列表](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/)中的语言默认均支持。
要同时处理 cont 和 non-cont 语言，请在配置文件中按如下所示设置选项（中文有一个[例外](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)）：

<!-- example charset_table 3 -->
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'

<!-- request SQL -->

```sql
POST /cli -d "
```

<!-- request JSON -->

```JSON
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'"
$index = new \Manticoresearch\Index($client);
```

<!-- request PHP -->

```php
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cont',
             'ngram_len' => '1',
             'ngram_chars' => 'cont'
        ]);
##### Python:
```
<!-- intro -->
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')

<!-- request Python -->

```python
##### Python-asyncio:
```

<!-- intro -->
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')

<!-- request Python-asyncio -->

```python
##### Javascript:
```

<!-- intro -->
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');

<!-- request javascript -->

```javascript
##### java:
```

<!-- intro -->
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);

<!-- request Java -->

```java
##### C#:
```

<!-- intro -->
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);

<!-- request C# -->

```clike
##### Rust:
```

<!-- intro -->
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", Some(true)).await;

<!-- request Rust -->

```rust
table products {
```

<!-- request CONFIG -->

```ini
  charset_table       = non_cont
  ngram_len           = 1
  ngram_chars         = cont
  type = rt

  path = tbl
  rt_field = title
  rt_attr_uint = price
}
如果您不需要支持连续脚本语言，可以简单地排除[ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)和[ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)选项。有关这些选项的更多信息，请参阅相应的文档部分。
```
<!-- end -->

要将一个字符映射到多个字符或反之，您可以使用[regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)。
### blend_chars

blend_chars = +, &, U+23

blend_chars = +, &->+

```ini
混合字符列表。可选，默认为空。
混合字符既被索引为分隔符，也被索引为有效字符。例如，当 `&` 被定义为混合字符且索引文档中出现 `AT&T` 时，将索引三个不同的关键词：`at&t`、`at` 和 `t`。
```

<!-- example blend_chars -->
此外，混合字符还可以影响索引，使得关键词的索引效果仿佛混合字符根本未被输入。这种行为在指定 `blend_mode = trim_all` 时尤为明显。例如，短语 `some_thing` 在 `blend_mode = trim_all` 下将被索引为 `some`、`something` 和 `thing`。

使用混合字符时应谨慎，因为将字符定义为混合字符意味着它不再是分隔符。

* 因此，如果您将逗号放入 `blend_chars` 并搜索 `dog,cat`，它将被视为单个标记 `dog,cat`。如果 `dog,cat` **未** 被索引为 `dog,cat`，而仅作为 `dog cat`，则不会匹配。

* 因此，这种行为应通过[blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)设置进行控制。
通过将混合字符替换为空格获得的标记位置按常规分配，常规关键词将被索引，仿佛未指定任何 `blend_chars`。一个混合了混合字符和非混合字符的额外标记将放置在起始位置。例如，如果文本字段开头出现 `AT&T company`，则 `at` 位置为1，`t` 位置为2，`company` 位置为3，同时 `AT&T` 也被赋予位置1，与开头的常规关键词混合。因此，查询 `AT&T` 或仅 `AT` 都会匹配该文档。短语查询 `"AT T"` 也会匹配，短语查询 `"AT&T company"` 亦然。
混合字符可能与查询语法中使用的特殊字符重叠，例如 `T-Mobile` 或 `@twitter`。查询解析器会尽可能将混合字符作为混合字符处理。例如，如果 `hello @twitter` 在引号内（短语操作符），查询解析器会将 `@` 视为混合字符。但如果 `@` 不在引号内，则该字符会被视为操作符。因此，建议对关键词进行转义。

混合字符可以重新映射，使多个不同的混合字符规范化为一个基本形式。这在索引多个具有等效字形的替代 Unicode 代码点时非常有用。

CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_chars' => '+, &, U+23, @->_'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  blend_chars = +, &, U+23, @->_
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### blend_mode
blend_mode = option [, option [, ...]]
```
<!-- end -->

option = trim_none | trim_head | trim_tail | trim_both | trim_all | skip_pure

```ini
混合标记索引模式由 blend_mode 指令启用。
默认情况下，混合和非混合字符混合的标记会被完整索引。例如，当 `blend_chars` 中同时包含 at 符号和感叹号时，字符串 `@dude!` 会被索引为两个标记：`@dude!`（包含所有混合字符）和 `dude`（不含任何混合字符）。因此，查询 `@dude` **不会** 匹配它。
```

<!-- example blend_mode -->
`blend_mode` 为此索引行为增加了灵活性。它接受一个逗号分隔的选项列表，每个选项指定一种标记索引变体。

如果指定了多个选项，将会索引同一标记的多个变体。常规关键字（通过用分隔符替换混合字符从该标记生成的）始终会被索引。

选项包括：

* `trim_none` - 索引整个标记

* `trim_head` - 修剪开头的混合字符，并索引结果标记

* `trim_tail` - 修剪结尾的混合字符，并索引结果标记
* `trim_both`- 修剪开头和结尾的混合字符，并索引结果标记
* `trim_all` - 修剪开头、结尾和中间的混合字符，并索引结果标记
* `skip_pure` - 如果标记纯粹由混合字符组成，则不索引该标记
使用上述示例字符串 `@dude!` 的 `blend_mode`，设置 `blend_mode = trim_head, trim_tail` 会导致索引两个标记：`@dude` 和 `dude!`。使用 `trim_both` 不会有影响，因为修剪两端的混合字符后得到的 `dude` 已作为常规关键字被索引。使用 `trim_both` 索引 `@U.S.A.`（假设点是混合字符）会索引 `U.S.A`。最后，`skip_pure` 允许忽略仅由混合字符组成的序列。例如，`one @@@ two` 会被索引为 `one two`，并作为短语匹配。默认情况下不是这样，因为完全混合的标记会被索引并影响第二个关键字的位置。
默认行为是索引整个标记，相当于 `blend_mode = trim_none`。

请注意，即使使用默认模式 `trim_none`，如果假设 `.` 是混合字符，使用混合模式也会限制搜索：

* `.dog.` 在索引时会变成 `.dog. dog`

* 你将无法通过 `dog.` 找到它。
使用更多模式会增加关键字匹配的可能性。
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_mode' => 'trim_tail, skip_pure',
            'blend_chars' => '+, &'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  blend_mode = trim_tail, skip_pure
```

<!-- request CONFIG -->

```ini
  blend_chars = +, &
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### min_word_len
min_word_len = length
```
<!-- end -->

min_word_len 是 Manticore 中的一个可选索引配置选项，指定最小索引词长。默认值为 1，表示所有内容都会被索引。

```ini
只有长度不小于该最小值的词才会被索引。例如，如果 min_word_len 是 4，则 'the' 不会被索引，但 'they' 会被索引。
```

<!-- example min_word_len -->

CREATE TABLE products(title text, price float) min_word_len = '4'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_word_len = '4'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_word_len' => '4'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  min_word_len = 4
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### ngram_len
ngram_len = 1
```
<!-- end -->

N-gram 长度，用于 N-gram 索引。可选，默认值为 0（禁用 N-gram 索引）。已知值为 0 和 1。

```ini
N-gram 为连续书写语言的无分词文本提供基本支持。使用连续书写语言搜索的问题在于缺少明确的词间分隔符。在某些情况下，你可能不想使用基于词典的分词，比如[中文的分词](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些情况下，N-gram 分词也可能效果良好。
```

<!-- example ngram_len -->
启用此功能时，这些语言的文本流（或任何在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 中定义的字符）将作为 N-gram 索引。例如，如果输入文本是 "ABCDEF"（其中 A 到 F 代表某种语言字符），且 ngram_len 是 1，则会被索引为 "A B C D E F"。目前仅支持 ngram_len=1。只有列在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 表中的字符会被这样拆分；其他字符不受影响。

请注意，如果搜索查询已分词，即单词间有分隔符，则在应用端将单词用引号包裹并使用扩展模式，仍能正确匹配未分词的文本。例如，假设原始查询是 `BC DEF`。在应用端包裹引号后，应为 `"BC" "DEF"`（带引号）。该查询传递给 Manticore 后，内部也会拆分成 1-gram，变成 `"B C" "D E F"` 查询，仍带有作为短语匹配操作符的引号。即使文本中没有分隔符，也能匹配该文本。

即使搜索查询未分词，Manticore 仍应产生良好结果，得益于基于短语的排名：它会将更接近的短语匹配（对于 N-gram 词来说，意味着更接近的多字符词匹配）排在前面。

CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ngram_chars = cont
```

<!-- request CONFIG -->

```ini
  ngram_len = 1
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### ngram_chars
ngram_chars = cont
```
<!-- end -->

ngram_chars = cont, U+3000..U+2FA1F

```ini
N-gram 字符列表。可选，默认为空。

与 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 配合使用时，此列表定义了哪些字符序列会被用于 N-gram 提取。由其他字符组成的单词不会受到 N-gram 索引功能的影响。值的格式与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同。N-gram 字符不能出现在 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中。
```

<!-- example ngram_chars -->
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'U+3000..U+2FA1F',
             'ngram_len' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request javascript -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->
```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->
```clike
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ngram_chars = U+3000..U+2FA1F
```

<!-- request CONFIG -->

```ini
  ngram_len = 1
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
你也可以像示例中那样使用我们默认的 N-gram 表的别名。在大多数情况下，这应该足够了。
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- end -->

<!-- example ngram_chars 2 -->
POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->
```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->
```clike
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ngram_chars = cont
```

<!-- request CONFIG -->

```ini
  ngram_len = 1
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### ignore_chars
ignore_chars = U+AD
```
<!-- end -->

忽略字符列表。可选，默认为空。

```ini
当某些字符（如软连字符 U+00AD）不仅应被视为分隔符，而应完全忽略时，此功能非常有用。例如，如果 '-' 不在 charset_table 中，文本 "abc-def" 将被索引为 "abc" 和 "def" 两个关键词。相反，如果 '-' 被添加到 ignore_chars 列表中，同样的文本将被索引为单个关键词 "abcdef"。
```

<!-- example ignore_chars -->
语法与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同，但只允许声明字符，不允许映射它们。此外，被忽略的字符不能出现在 charset_table 中。

CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'ignore_chars' => 'U+AD'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ignore_chars = U+AD
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### bigram_index
bigram_index = {none|all|first_freq|both_freq}
```
<!-- end -->

二元组索引模式。可选，默认无。

```ini
二元组索引是一种加速短语搜索的功能。索引时，它会将所有或部分相邻词对的文档列表存储到索引中。该列表随后可在搜索时用来显著加快短语或子短语匹配。
```

<!-- example bigram_index -->
`bigram_index` 控制特定词对的选择。已知的模式有：

* `all`，索引每一个词对

* `first_freq`，仅索引第一个词是频繁词列表中的词的词对（见 [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)）。例如，设置 `bigram_freq_words = the, in, i, a` 时，索引文本 "alone in the dark" 会存储 "in the" 和 "the dark" 这两个二元组，因为它们以频繁词（"in" 或 "the"）开头，但 "alone in" 不会被索引，因为 "in" 是该词对的第二个词。

* `both_freq`，仅索引两个词都是频繁词的词对。以同样的例子，使用此模式索引 "alone in the dark" 只会存储 "in the"（从搜索角度看是最差的情况），其他词对不会被存储。
对于大多数用例，`both_freq` 是最佳模式，但具体情况可能有所不同。
需要注意的是，`bigram_index` 仅在分词层面工作，不考虑诸如 `morphology`、`wordforms` 或 `stopwords` 等转换。这意味着它创建的词元非常直接，使短语搜索更精确和严格。虽然这可以提高短语匹配的准确性，但也降低了系统识别词形变化或词语变体的能力。

CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'both_freq'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  bigram_index = both_freq
```

<!-- request CONFIG -->

```ini
  bigram_freq_words = the, a, you, i
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### bigram_freq_words
bigram_freq_words = the, a, you, i
```
<!-- end -->

在索引二元组时被视为“频繁”的关键词列表。可选，默认为空。

```ini
一些二元索引模式（参见 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)）需要定义一个频繁关键词列表。这些**不**应与停用词混淆。停用词在索引和搜索时会被完全剔除。频繁关键词仅被二元组用来判断是否索引当前的词对。
```

<!-- example bigram_freq_words -->
`bigram_freq_words` 允许你定义这样一个关键词列表。

CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'first_freq'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  bigram_freq_words = the, a, you, i
```

<!-- request CONFIG -->

```ini
  bigram_index = first_freq
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### dict
dict = {keywords|crc}
```
<!-- end -->

所使用的关键词字典类型由两个已知值之一标识，'crc' 或 'keywords'。这是可选的，默认值为 'keywords'。

```ini
使用关键词字典模式（dict=keywords）可以显著减少索引负担，并在大规模集合上启用子串搜索。此模式可用于普通表和 RT 表。
```

<!-- example dict -->
CRC 字典不会在索引中存储原始关键词文本。相反，它们在搜索和索引过程中用一个控制和校验值（使用 FNV64 计算）替代关键词。该值在索引内部使用。这种方法有两个缺点：

* 首先，不同关键词对之间存在控制和校验值冲突的风险。该风险随着索引中唯一关键词数量的增加而增加。尽管如此，这个问题较小，因为在一个包含 10 亿条目的字典中，单个 FNV64 冲突的概率大约是 1/16，即 6.25%。大多数字典的关键词数量远少于 10 亿，因为典型的口语人类语言词形数量在 1 到 1000 万之间。

* 其次，更重要的是，使用控制和校验值进行子串搜索并不简单。Manticore 通过预先索引所有可能的子串作为独立关键词来解决此问题（参见 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)，[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 指令）。这种方法还有一个额外优点，即以最快的方式匹配子串。然而，预索引所有子串会显著增加索引大小（通常增加 3-10 倍或更多），并随后影响索引时间，使得在大型索引上进行子串搜索变得不太实用。
关键词字典解决了这两个问题。它在索引中存储关键词，并在搜索时执行通配符扩展。例如，搜索 `test*` 前缀时，内部可能扩展为基于字典内容的 'test|tests|testing' 查询。此扩展过程对应用程序完全透明，唯一的区别是所有匹配关键词的单独统计数据也会被报告。
对于子串（中缀）搜索，可以使用扩展通配符。特殊字符如 `?` 和 `%` 与子串（中缀）搜索兼容（例如，`t?st*`，`run%`，`*abc*`）。请注意，[通配符操作符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators) 和 [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) 仅在 `dict=keywords` 时有效。

使用关键词字典索引的速度大约比常规非子串索引慢 1.1 到 1.3 倍，但明显快于子串索引（无论是前缀还是中缀）。索引大小应仅比标准非子串表略大，总差异在 1% 到 10% 之间。常规关键词搜索的时间在这三种索引类型（CRC 非子串、CRC 子串、关键词）中应几乎相同或相等。子串搜索时间会根据匹配给定子串的实际关键词数量（即搜索词扩展成多少关键词）显著波动。匹配关键词的最大数量受 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) 指令限制。

总之，关键词和 CRC 字典为子串搜索提供了两种不同的权衡选择。你可以选择牺牲索引时间和索引大小以获得最快的最坏情况搜索（CRC 字典），或者最小化索引时间影响，但在前缀扩展成大量关键词时牺牲最坏情况搜索时间（关键词字典）。

CREATE TABLE products(title text, price float) dict = 'keywords'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) dict = 'keywords'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'dict' => 'keywords'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  dict = keywords
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### embedded_limit
embedded_limit = size
```
<!-- end -->

嵌入式例外、词形或停用词文件大小限制。可选，默认值为 16K。

```ini
当你创建表时，上述文件可以与表一起外部保存，或者直接嵌入到表中。大小低于 `embedded_limit` 的文件会存储到表中。对于较大的文件，仅存储文件名。这也简化了将表文件移动到另一台机器的过程；你可能只需复制一个文件即可。
```

<!-- example embedded_limit -->
对于较小的文件，这种嵌入减少了表所依赖的外部文件数量，有助于维护。但同时，将一个100 MB的词形字典嵌入到一个微小的增量表中是没有意义的。因此需要有一个大小阈值，而`embedded_limit`就是这个阈值。

table products {

  embedded_limit = 32K

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### global_idf
global_idf = /path/to/global.idf
```
<!-- end -->

全局（集群范围）关键词IDF文件的路径。可选，默认为空（使用本地IDF）。

```ini
在多表集群中，每个关键词的频率很可能在不同的表之间有所不同。这意味着当排名函数使用基于TF-IDF的值（如BM25系列因子）时，结果可能会根据它们所在的集群节点而略有不同。
```

<!-- example global_idf -->
解决该问题的最简单方法是创建并使用全局频率字典，简称全局IDF文件。该指令允许您指定该文件的位置。建议（但不强制）使用.idf扩展名。当为给定表指定了IDF文件*且* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为1时，引擎将使用global_idf文件中的关键词频率和集合文档计数，而不仅仅是本地表。这样，IDF及其依赖的值将在整个集群中保持一致。

IDF文件可以被多个表共享。即使许多表引用该文件，`searchd`也只会加载IDF文件的单个副本。如果IDF文件的内容发生变化，可以通过SIGHUP加载新内容。

您可以使用[indextool](../../Miscellaneous_tools.md#indextool)工具构建.idf文件，先使用`--dumpdict dict.txt --stats`开关导出字典，然后使用`--buildidf`将其转换为.idf格式，最后使用`--mergeidf`合并整个集群的所有.idf文件。

CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'global_idf' => '/usr/local/manticore/var/global.idf'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  global_idf = /usr/local/manticore/var/global.idf
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### hitless_words
hitless_words = {all|path/to/file}
```
<!-- end -->

无命中词列表。可选，允许的值为'all'，或一个列表文件名。

```ini
默认情况下，Manticore全文索引不仅存储每个关键词匹配的文档列表，还存储其在文档中的位置列表（称为hitlist）。Hitlist支持短语、邻近、严格顺序及其他高级搜索类型，以及短语邻近排名。然而，对于某些频繁关键词（尽管频繁但无法被停止词过滤），hitlist可能非常庞大，导致查询时处理缓慢。此外，在某些情况下，我们可能只关心布尔关键词匹配，根本不需要基于位置的搜索操作符（如短语匹配）或短语排名。
```

<!-- example hitless_words -->
`hitless_words`允许您创建完全没有位置信息（hitlist）的索引，或对特定关键词跳过位置信息。

无命中索引通常比相应的常规全文索引占用更少空间（大约可节省1.5倍）。索引和搜索速度都应更快，但代价是缺少位置查询和排名支持。

如果在位置查询（例如短语查询）中使用，无命中词会被从查询中剔除，并作为无位置的操作数使用。例如，如果“hello”和“world”是无命中词，而“simon”和“says”不是无命中词，则短语查询 `"simon says hello world"` 会被转换为 `("simon says" & hello & world)`，匹配文档中任意位置的“hello”和“world”，以及作为精确短语的“simon says”。

仅包含无命中词的位置信息查询将导致空短语节点，因此整个查询将返回空结果并发出警告。如果整个字典都是无命中词（使用`all`），则只能在相应索引上使用布尔匹配。

CREATE TABLE products(title text, price float) hitless_words = 'all'

POST /cli -d "



<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words = 'all'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words' => 'all'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  hitless_words = all
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### index_field_lengths
index_field_lengths = {0|1}
```
<!-- end -->

启用计算和存储字段长度（包括每文档和每索引的平均值）到全文索引中。可选，默认值为0（不计算和存储）。

```ini
当`index_field_lengths`设置为1时，Manticore将：
```

<!-- example index_field_lengths -->
* 为每个全文字段创建相应的长度属性，名称相同但带有`__len`后缀

* 计算每个文档的字段长度（以关键词计数）并存储到相应属性中
* 计算每个索引的平均值。长度属性将具有特殊的TOKENCOUNT类型，但其值实际上是常规的32位整数，且其值通常是可访问的。
[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 表达式排序器中的函数基于这些长度，并且需要启用 `index_field_lengths`。历史上，Manticore 使用了一个简化的、精简版的 BM25，该版本与完整函数不同，**不**考虑文档长度。现在也支持 BM25 的完整变体及其多字段扩展，称为 BM25F。它们分别需要每个文档长度和每个字段长度。因此需要额外的指令。
CREATE TABLE products(title text, price float) index_field_lengths = '1'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_field_lengths = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_field_lengths' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  index_field_lengths = 1
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### index_token_filter
index_token_filter = my_lib.so:custom_blend:chars=@#&
```
<!-- end -->

全文索引时的索引时令牌过滤器。可选，默认为空。

```ini
index_token_filter 指令指定全文索引时的可选索引时令牌过滤器。该指令用于创建一个根据自定义规则生成令牌的自定义分词器。该过滤器由索引器在将源数据索引到普通表时创建，或由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。插件使用格式 `library name:plugin name:optional string of settings` 定义。例如，`my_lib.so:custom_blend:chars=@#&`。
```

<!-- example index_token_filter -->
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_token_filter' => 'my_lib.so:custom_blend:chars=@#&'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  index_token_filter = my_lib.so:custom_blend:chars=@#&
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### overshort_step
overshort_step = {0|1}
```
<!-- end -->

对过短（少于 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)）关键词的位置增量。可选，允许值为 0 和 1，默认是 1。

```ini
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- example overshort_step -->
POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'overshort_step' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  overshort_step = 1
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### phrase_boundary
phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis
```
<!-- end -->

短语边界字符列表。可选，默认为空。

```ini
该列表控制哪些字符将被视为短语边界，以调整词的位置并通过邻近搜索实现短语级搜索模拟。语法类似于 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)，但不允许映射，且边界字符不得与其他任何内容重叠。
```

<!-- example phrase_boundary -->
在短语边界处，将向当前词位置添加额外的词位置增量（由 [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 指定）。这使得通过邻近查询实现短语级搜索成为可能：不同短语中的词将保证相距超过 phrase_boundary_step；因此在该距离内的邻近搜索等同于短语级搜索。

只有当该字符后面跟着分隔符时，才会触发短语边界条件；这是为了避免将缩写如 S.T.A.L.K.E.R 或 URL 视为多个短语。

CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary' => '., ?, !, U+2026',
             'phrase_boundary_step' => '10'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')

```
<!-- intro -->
##### Pytho-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  phrase_boundary = ., ?, !, U+2026
```

<!-- request CONFIG -->

```ini
  phrase_boundary_step = 10
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### phrase_boundary_step
phrase_boundary_step = 100
```
<!-- end -->

短语边界词位置增量。可选，默认是 0。

```ini
在短语边界处，当前词位置将额外增加此数值。
```

<!-- example phrase_boundary_step -->
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary_step' => '100',
             'phrase_boundary' => '., ?, !, U+2026'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')

```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  phrase_boundary_step = 100
```

<!-- request CONFIG -->

```ini
  phrase_boundary = ., ?, !, U+2026
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### regexp_filter
# index '13"' as '13inch'
```
<!-- end -->

regexp_filter = \b(\d+)\" => \1inch

```ini
# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color

正则表达式（regexps）用于过滤字段和查询。此指令是可选的，可多值，默认值为空的正则表达式列表。Manticore Search 使用的正则表达式引擎是 Google 的 RE2，以其速度和安全性著称。有关 RE2 支持的语法的详细信息，您可以访问 [RE2 语法指南](https://github.com/google/re2/wiki/Syntax)。
在某些应用中，例如产品搜索，可能有多种方式来指代一个产品、型号或属性。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指的是同一款产品。另一个例子是表达笔记本屏幕尺寸的不同方式，如 `13-inch`、`13 inch`、`13"` 或 `13in`。
```

<!-- example regexp_filter -->
正则表达式提供了一种机制，用于指定处理此类情况的规则。在第一个例子中，您可能会使用一个词形文件来处理少量的 iPhone 型号，但在第二个例子中，更好的是指定规则，将“13-inch”和“13in”规范化为相同的形式。

列在 `regexp_filter` 中的正则表达式按列出顺序应用，在尽可能早的阶段应用，早于任何其他处理（包括[例外](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)），甚至早于分词。也就是说，正则表达式应用于索引时的原始源字段，以及搜索时的原始搜索查询文本。

CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'regexp_filter' => '(blue|red) => color'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')

```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  # index '13"' as '13inch'
```

<!-- request CONFIG -->

```ini
  regexp_filter = \b(\d+)\" => \1inch
  # index 'blue' or 'red' as 'color'
  regexp_filter = (blue|red) => color

  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

