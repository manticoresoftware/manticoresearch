# 低级分词

当文本在 Manticore 中被索引时，会被拆分成单词，并进行大小写折叠，使得像 "Abc"、"ABC" 和 "abc" 这样的单词被视作相同的单词。

为了正确执行这些操作，Manticore 必须知道：
* 源文本的编码（应始终为 UTF-8）
* 哪些字符被视为字母，哪些不被视为字母
* 哪些字母应折叠为其他字母

您可以使用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 选项按表进行配置。charset_table 指定一个数组，将字母字符映射到其大小写折叠版本（或其他您喜欢的字符）。数组中未出现的字符会被视为非字母，并在本表中索引或搜索时作为单词分隔符处理。

默认字符集是 `non_cont`，包括 [大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

您还可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

文本 `RED TUBE 5" LONG` 将被索引为 `COLOR TUBE 5 INCH LONG`，`PLANK 2" x 4"` 将被索引为 `PLANK 2 INCH x 4 INCH`。这些规则按指定顺序应用。规则同样适用于查询，因此搜索 `BLUE TUBE` 实际会搜索 `COLOR TUBE`。

您可以在[这里了解更多关于 regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)的信息。

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
`charset_table` 指定一个数组，将字母字符映射到其大小写折叠版本（如果您愿意，也可以是其他字符）。默认字符集是 `non_cont`，包括采用 [非连续](https://en.wikipedia.org/wiki/Scriptio_continua)脚本的大多数字符。

`charset_table` 是 Manticore 分词过程的核心，它从文档文本或查询文本中提取关键词。它控制哪些字符被接受为有效字符及其转换方式（例如是否去除大小写敏感）。

默认情况下，所有字符映射为 0，表示不被视作有效关键词，会被视为分隔符。一旦表中提及某字符，它就映射到另一字符（通常映射为本身或小写字母），并被视为有效关键词的一部分。

charset_table 使用逗号分隔的映射列表声明字符有效性或将字符映射到其他字符。提供了几个语法快捷方式用于一次映射字符范围：

* 单字符映射：`A->a`。声明源字符 'A' 在关键词中允许，映射为字符 'a'（但不声明 'a' 本身为允许字符）。
* 范围映射：`A..Z->a..z`。声明源范围内所有字符允许，并映射到目标范围。未声明目标范围内字符为允许。检查两个范围长度是否相同。
* 单独字符映射：`a`。声明字符允许并映射为自身。等同于 `a->a`。
* 单独范围映射：`a..z`。声明范围内所有字符允许并映射为自身。等同于 `a..z->a..z`。
* 双字符映射：`A..Z/2`。将每两字符映射到第二个字符。例如，`A..Z/2` 等同于 `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。此映射快捷方式适用于大小写字母以交叉顺序排列的 Unicode 块。

对于代码从 0 至 32，以及 127 至 8 位 ASCII 和 Unicode 字符的范围，Manticore 始终将它们视为分隔符。为避免配置文件编码问题，8 位 ASCII 和 Unicode 字符必须以 `U+XXX` 格式指定，其中 `XXX` 是十六进制代码点。接受的最小 Unicode 字符码是 `U+0021`。

如果默认映射无法满足您的需求，可以通过重新指定映射来重新定义字符映射。例如，如果内置 `non_cont` 数组包含字符 `Ä` 和 `ä` 并将它们映射到 ASCII 字符 `a`，您可以添加其 Unicode 代码点重新定义它们，如下：

```
charset_table = non_cont,U+00E4,U+00C4
```

用于区分大小写搜索，或

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

用于不区分大小写搜索。

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
除了字符和映射定义外，还可以使用几个内置别名。当前别名有：
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

如果你想要在搜索中支持不同的语言,为所有语言定义有效字符集和转换规则可能是一项繁琐的任务。我们通过提供默认字符集表 `non_cont` 和 `cont` 来简化这一工作,分别用于非连续和连续(中文、日语、韩语、泰语)脚本的语言。在大多数情况下,这些字符集应该足以满足你的需求。

请注意,以下语言目前**不**支持:
* 阿萨姆语
* 比什努普里亚语
* 布希德语
* 加罗语
* 苗语
* 霍语
* 科米语
* 大花苗语
* 马巴语
* 迈蒂利语
* 马拉地语
* 门德语
* 姆鲁语
* 米耶内语
* 恩甘巴伊语
* 奥里亚语
* 桑塔利语
* 信德语
* 锡尔赫特语

所有其他在[Unicode语言
列表](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/)中列出的语言默认都是支持的。

<!-- example charset_table 3 -->
要同时处理cont和non-cont语言,请按如下所示在配置文件中设置选项(对于中文有一个[例外](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)):

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cont',
             'ngram_len' => '1',
             'ngram_chars' => 'cont'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  charset_table       = non_cont
  ngram_len           = 1
  ngram_chars         = cont

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

如果你不需要支持连续脚本语言,你可以简单地排除[ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)和[ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)选项。
有关这些选项的更多信息,请参阅相应的文档部分。

要将一个字符映射到多个字符或反之,可以使用[regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)。

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
混合字符列表。可选,默认为空。

混合字符既被索引为分隔符又被索引为有效字符。例如,当将`&`定义为混合字符时,如果索引文档中出现`AT&T`,将会索引三个不同的关键词:`at&t`、`at`和`t`。

此外,混合字符可以影响索引,使关键词被索引时就像完全没有输入混合字符一样。当指定`blend_mode = trim_all`时,这种行为特别明显。例如,短语`some_thing`将被索引为`some`、`something`和`thing`。

使用混合字符时应该谨慎,因为将字符定义为混合字符意味着它不再是分隔符。
* 因此,如果你将逗号放入`blend_chars`并搜索`dog,cat`,它会将其作为单个标记`dog,cat`处理。如果`dog,cat`**没有**被索引为`dog,cat`,而是仅保留为`dog cat`,那么它将不会匹配。
* 因此,这种行为应该通过[blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)设置来控制。

通过将混合字符替换为空格获得的标记位置按常规分配,常规关键词的索引就像完全没有指定`blend_chars`一样。混合了混合字符和非混合字符的额外标记将被放在起始位置。例如,如果`AT&T company`出现在文本字段的最开始,`at`将被赋予位置1,`t`位置2,`company`位置3,而`AT&T`也将被赋予位置1,与开头的常规关键词混合。因此,对`AT&T`或仅`AT`的查询都将匹配该文档。短语查询`"AT T"`也会匹配,同样`"AT&T company"`的短语查询也会匹配。

混合字符可能与查询语法中使用的特殊字符重叠,例如`T-Mobile`或`@twitter`。在可能的情况下,查询解析器会将混合字符作为混合字符处理。例如,如果`hello @twitter`在引号内(短语运算符),查询解析器会将`@`符号作为混合字符处理。但是,如果`@`符号不在引号内,该字符将被作为运算符处理。因此,建议对关键词进行转义。

混合字符可以被重新映射,这样多个不同的混合字符可以被规范化为一个基本形式。这在索引具有等效字形的多个替代Unicode码点时很有用。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_chars' => '+, &, U+23, @->_'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  blend_chars = +, &, U+23, @->_

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### blend_mode

```ini
blend_mode = option [, option [, ...]]
option = trim_none | trim_head | trim_tail | trim_both | trim_all | skip_pure
```

<!-- example blend_mode -->
混合标记索引模式通过blend_mode指令启用。

默认情况下,混合和非混合字符混合的标记会被完整索引。例如,当@符号和感叹号都在`blend_chars`中时,字符串`@dude!`将被索引为两个标记:`@dude!`(包含所有混合字符)和`dude`(不包含任何混合字符)。因此,`@dude`的查询将**不**匹配它。

`blend_mode`为这种索引行为增加了灵活性。它接受一个逗号分隔的选项列表,每个选项指定一个标记索引变体。

如果指定了多个选项，则同一标记的多个变体将被索引。常规关键字（通过将混合字符替换为分隔符从该标记得出）始终被索引。

选项有：

* `trim_none` - 索引整个标记
* `trim_head` - 修剪开头的混合字符，并索引结果标记
* `trim_tail` - 修剪尾部的混合字符，并索引结果标记
* `trim_both` - 修剪开头和尾部的混合字符，并索引结果标记
* `trim_all` - 修剪开头、尾部和中间的混合字符，并索引结果标记
* `skip_pure` - 如果标记纯粹由混合字符组成（即仅包含混合字符），则不索引该标记

使用上述示例字符串 `@dude!` 的 `blend_mode` 设置为 `trim_head, trim_tail`，将导致索引两个标记：`@dude` 和 `dude!`。使用 `trim_both` 不会产生效果，因为修剪两端的混合字符后得到的是 `dude`，它已作为常规关键字被索引。使用 `trim_both` 索引 `@U.S.A.`（假设点是混合字符），将产生 `U.S.A` 被索引。最后，`skip_pure` 选项允许忽略纯由混合字符组成的序列。例如，`one @@@ two` 会被索引为 `one two`，并作为短语匹配。默认情况下不是这样，因为完全混合的标记会被索引并偏移第二个关键字的位置。

默认行为是索引整个标记，相当于 `blend_mode = trim_none`。

注意，即使使用默认模式 `trim_none`，如果假设 `.` 是混合字符，使用混合模式会限制搜索：
* `.dog.` 在索引时将变成 `.dog. dog`
* 并且你无法通过 `dog.` 进行搜索。

使用更多的模式会增加关键字匹配的机会。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_mode' => 'trim_tail, skip_pure',
            'blend_chars' => '+, &'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  blend_mode = trim_tail, skip_pure
  blend_chars = +, &

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### min_word_len

```ini
min_word_len = length
```

<!-- example min_word_len -->

min_word_len 是 Manticore 可选的索引配置选项，指定最小索引的单词长度。默认值为 1，意味着所有内容都会被索引。

只有长度不小于此最小值的单词才会被索引。例如，如果 min_word_len 是 4，那么 'the' 不会被索引，但 'they' 会被索引。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_word_len = '4'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) min_word_len = '4'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_word_len' => '4'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  min_word_len = 4

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_len

```ini
ngram_len = 1
```

<!-- example ngram_len -->
N-gram 长度，用于 N-gram 索引。可选，默认值为 0（禁用 N-gram 索引）。已知有效值为 0 和 1。

N-gram 提供了对连续脚本语言未分词文本的基本支持。使用连续脚本语言搜索的问题在于没有明确的单词分隔符。在某些情况下，你可能不想使用字典分词，例如 [中文可用的分词](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些情况下，N-gram 分词也能很好地工作。

启用此功能后，该类语言（或者 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 定义的其他字符）将以 N-gram 形式被索引。例如，若输入文本为 "ABCDEF"（其中 A 至 F 代表某种语言字符），且 ngram_len 为 1，则其被索引时等同于 "A B C D E F"。目前仅支持 ngram_len=1。仅 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 中列出的字符会按此方式拆分；其他字符不受影响。

请注意，如果搜索查询已经分词，即单词之间有分隔符，将单词用引号括起并使用扩展模式，即使文本未分词，也能正确匹配。例如，假设原查询为 `BC DEF`，在应用端加引号后变成 `"BC" "DEF"`（带引号）。该查询传给 Manticore 后也会内部分成 1-gram，变为 `"B C" "D E F"`，仍带引号作为短语匹配操作符。这样即使文本中没有分隔符，也能匹配文本。

即使搜索查询未分词，得益于基于短语的排名，Manticore 仍能给出较好结果：相近的短语匹配（对于 N-gram 词语，可能是相近的多字符词匹配）会被排在前面。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_chars

```ini
ngram_chars = cont

ngram_chars = cont, U+3000..U+2FA1F
```

<!-- example ngram_chars -->
N-gram 字符列表。可选，默认值为空。

与 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 一起使用时，此列表定义了其序列会被用于 N-gram 提取的字符。由其他字符组成的单词不会受 N-gram 索引功能的影响。值的格式与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同。N-gram 字符不能出现在 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'U+3000..U+2FA1F',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = U+3000..U+2FA1F
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```

<!-- end -->

<!-- example ngram_chars 2 -->
你也可以像示例中一样使用我们默认的 N-gram 表的别名。在大多数情况下，这应该足够。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ignore_chars

```ini
ignore_chars = U+AD
```

<!-- example ignore_chars -->
忽略字符列表。可选，默认值为空。

在某些情况下非常有用，例如某些字符（如软连字符 U+00AD）不仅应被视为分隔符，而是完全被忽略。例如，如果 '-' 不出现在 charset_table 中，文本 "abc-def" 会被索引为 "abc" 和 "def" 两个关键字。相反，如果 '-' 被添加到 ignore_chars 列表中，同样的文本会被索引为单个关键字 "abcdef"。

语法与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同，但只允许声明字符，不允许映射它们。此外，被忽略的字符不能出现在 charset_table 中。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'ignore_chars' => 'U+AD'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ignore_chars = U+AD

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_index

```ini
bigram_index = {none|all|first_freq|both_freq}
```

<!-- example bigram_index -->
双字索引模式。可选，默认值为无。

双字索引是一个加速短语搜索的功能。索引时，它会将所有或部分相邻单词对的文档列表存储到索引中。然后在搜索时使用该列表可显著加快短语或子短语匹配。

`bigram_index` 控制特定单词对的选择。已知的模式有：

* `all`，索引每一个单词对
* `first_freq`，仅索引第一个单词是频繁词列表中的单词对（见 [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)）。例如，设 `bigram_freq_words = the, in, i, a`，索引文本 "alone in the dark" 会将 "in the" 和 "the dark" 这两个对作为双字存储，因为它们以频繁关键字（"in" 或 "the"）开头，但 "alone in" **不会** 被索引，因为 "in" 是这个对中的第二个单词。
* `both_freq`，仅索引两个单词都频繁的单词对。以同样示例，索引 "alone in the dark" 只会将 "in the" （搜索角度看最差的那个）作为双字存储，其他单词对都不会索引。

对于大多数用例，`both_freq` 是最佳模式，但具体效果视情况而定。

重要的是，`bigram_index` 只在分词阶段工作，不考虑 `morphology`、`wordforms` 或 `stopwords` 等转换。这意味着生成的 token 非常直接，使得短语搜索更为准确和严格。虽然这可以提高短语匹配的准确性，但也使系统在识别单词的不同形式或出现变体时能力较弱。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'both_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_index = both_freq
  bigram_freq_words = the, a, you, i

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_freq_words

```ini
bigram_freq_words = the, a, you, i
```

<!-- example bigram_freq_words -->
一组被认为是双字索引时的“频繁”关键字列表。可选，默认值为空。

一些二元索引模式（参见 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)）需要定义一个常用关键词列表。这些**不**应与停用词混淆。停用词在索引和搜索时会被完全剔除。常用关键词仅由二元组用来决定是否索引当前的词对。

`bigram_freq_words` 允许你定义这样一个关键词列表。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'first_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_freq_words = the, a, you, i
  bigram_index = first_freq

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### dict

```ini
dict = {keywords|crc}
```

<!-- example dict -->
所使用的关键词字典类型由两个已知值中的一个标识，分别是 'crc' 或 'keywords'。此项为可选，默认是 'keywords'。

使用关键词字典模式（dict=keywords）可以显著降低索引负担，并支持对大型集合的子串搜索。此模式可以用于普通表和 RT 表。

CRC 字典在索引中不存储原始关键词文本。相反，它们在搜索和索引过程中将关键词替换为一个控制和校验值（使用 FNV64 计算）。该值在索引内部使用。这种方法有两个缺点：
* 首先，不同关键词之间存在控制和校验值碰撞的风险。该风险随索引中唯一关键词数量的增加而增加。尽管如此，这个问题不大，因为在一个包含 10 亿条目的字典中，单个 FNV64 碰撞的概率约为 1/16，即 6.25%。考虑到普通口语人类语言的词形变化通常在 1 到 1000 万之间，大多数字典的关键词数量远小于 10 亿。
* 其次，更重要的是，使用控制和校验值执行子串搜索并不直观。Manticore 通过将所有可能的子串预先索引为独立关键词解决了这个问题（参见 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)、[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 指令）。这种方法还有一个额外优点，即能够以最快速的方式匹配子串。然而，预索引所有子串会显著增加索引大小（通常增大 3-10 倍或更多）并且随之影响索引时间，使得对大型索引进行子串搜索变得不切实际。

关键词字典解决了这两个问题。它在索引中存储关键词，并在搜索时进行通配符展开。例如，对 `test*` 前缀的搜索可以根据字典内容内部扩展为 'test|tests|testing' 的查询。这个扩展过程对应用程序完全不可见，唯一的区别是所有匹配关键词的单独统计信息也会报告。

对子串（中缀）搜索，可以使用扩展通配符。特殊字符如 `?` 和 `%` 兼容子串（中缀）搜索（例如 `t?st*`，`run%`，`*abc*`）。注意，[通配符运算符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators) 和 [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) 仅在 `dict=keywords` 时有效。

使用关键词字典索引比普通非子串索引大约慢 1.1 倍到 1.3 倍，但明显快于子串索引（无论是前缀还是中缀）。索引大小应仅略大于标准非子串表的大小，总体差别在 1% 到 10% 之间。常规关键词搜索时间在上述三种索引类型（CRC 非子串、CRC 子串、keywords）之间应几乎相同或相近。子串搜索时间可能显著波动，取决于有多少关键词与给定子串匹配（即搜索词扩展为多少关键词）。匹配关键词的最大数量受 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) 指令限制。

总结来说，关键词字典和 CRC 字典为子串搜索提供了两种不同的权衡方案。你可以选择牺牲索引时间和索引大小来实现最坏情况下的最快搜索（CRC 字典），或者尽量减少索引时间影响，但在前缀扩展到大量关键词时牺牲最坏情况的搜索时间（关键词字典）。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) dict = 'keywords'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) dict = 'keywords'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'dict' => 'keywords'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  dict = keywords

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### embedded_limit

```ini
embedded_limit = size
```

<!-- example embedded_limit -->
内嵌异常、词形变化或停用词文件大小限制。可选，默认值为 16K。

当你创建表时，上述文件可以存储在表外部，或直接嵌入表中。文件大小低于 `embedded_limit` 的会存储到表中。对于较大的文件，只存储文件名。这也简化了将表文件迁移到其他机器的过程；你可能只需复制一个文件即可。

对于较小的文件，这样的嵌入减少了表所依赖的外部文件数量，有助于维护。但同时，将一个100 MB的词形字典嵌入到一个很小的增量表中是没有意义的。所以需要有一个大小阈值，而`embedded_limit`就是该阈值。

<!-- request CONFIG -->

```ini
table products {
  embedded_limit = 32K

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### global_idf

```ini
global_idf = /path/to/global.idf
```

<!-- example global_idf -->
全局（集群范围）关键词IDF文件的路径。可选，默认为空（使用本地IDF）。

在多表集群中，每个关键词的频率很可能在不同表之间有所不同。这意味着当排名函数使用基于TF-IDF的值，如BM25系列因子时，结果的排名可能会因它们所在的集群节点不同而略有差异。

解决此问题的最简单方法是创建并使用全局频率字典，简称全局IDF文件。此指令允许您指定该文件的位置。建议（但不要求）使用`.idf`扩展名。当为某个表指定了IDF文件*且* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为1时，引擎将使用global_idf文件中的关键词频率和文档集合计数，而不是仅使用本地表。这样，IDF及依赖其的值将在整个集群中保持一致。

IDF文件可以被多个表共享。即使很多表都引用该文件，`searchd`也只会加载IDF文件的一份副本。如果IDF文件内容发生变化，可以通过SIGHUP加载新内容。

您可以使用[indextool](../../Miscellaneous_tools.md#indextool)工具构建.idf文件，先使用`--dumpdict dict.txt --stats`参数导出字典，然后使用`--buildidf`转换成.idf格式，最后使用`--mergeidf`合并整个集群的所有.idf文件。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'global_idf' => '/usr/local/manticore/var/global.idf'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  global_idf = /usr/local/manticore/var/global.idf

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### hitless_words

```ini
hitless_words = {all|path/to/file}
```

<!-- example hitless_words -->
无命中词列表。可选，允许的值为'all'，或列表文件名。

默认情况下，Manticore全文索引不仅存储每个关键词匹配的文档列表，还存储其文档内的位置列表（称为hitlist）。Hitlist支持短语、邻近、严格顺序及其他高级搜索类型，以及短语邻近排名。然而，某些频繁关键词（因某些原因无法停止）其hitlist可能非常庞大，导致查询时处理缓慢。此外，在某些情况下，我们可能只关心布尔关键词匹配，不需要基于位置的搜索操作（如短语匹配）和短语排名。

`hitless_words`允许您创建不包含位置信息（hitlist）的索引，或仅对特定关键词跳过位置信息。

无命中索引通常比相应的常规模糊索引占用更少空间（大约减少1.5倍）。索引和搜索速度更快，但代价是无法支持基于位置的查询和排名。

如果在位置查询（如短语查询）中使用，无命中词将被去除位置信息，并作为操作数使用。例如，如果“hello”和“world”为无命中词，“simon”和“says”非无命中词，则短语查询 `"simon says hello world"` 会被转换为 `("simon says" & hello & world)`，其中“hello”和“world”可出现在文档任意位置，而“simon says”必须作为精确短语匹配。

仅包含无命中词的位置信息查询将产生空的短语节点，因此整个查询将返回空结果并给出警告。如果整个字典都是无命中词（使用`all`），则相应索引只支持布尔匹配。



<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words = 'all'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) hitless_words = 'all'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words' => 'all'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  hitless_words = all

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_field_lengths

```ini
index_field_lengths = {0|1}
```

<!-- example index_field_lengths -->
启用计算并存储字段长度（包括每文档和每索引的平均值）到全文索引中。可选，默认为0（不计算和存储）。

当`index_field_lengths`设置为1时，Manticore将：
* 为每个全文字段创建相应的长度属性，该属性与字段同名但添加`__len`后缀
* 计算每个文档的字段长度（以关键词数计）并存储到对应属性中
* 计算每个索引的平均值。长度属性将具有特殊的TOKENCOUNT类型，但它们的值实际上是普通32位整数，且通常可被访问。

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 函数在表达式排名器中基于这些长度，并且需要启用 `index_field_lengths`。历史上，Manticore 使用了 BM25 的简化版，区别于完整函数，它**不**考虑文档长度。现在也支持 BM25 的完整变体及其多字段扩展，称为 BM25F。它们分别需要每个文档长度和每个字段长度。因此需要额外的指令。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_field_lengths = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_field_lengths = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_field_lengths' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_field_lengths = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_token_filter

```ini
index_token_filter = my_lib.so:custom_blend:chars=@#&
```

<!-- example index_token_filter -->
全文索引时的索引时间令牌过滤器。可选，默认值为空。

index_token_filter 指令指定全文索引的索引时间令牌过滤器。此指令用于创建一个根据自定义规则生成令牌的自定义分词器。过滤器由索引器在将源数据索引到普通表时创建，或由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。插件定义格式为 `库名:插件名:可选设置字符串`。例如，`my_lib.so:custom_blend:chars=@#&`。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_token_filter' => 'my_lib.so:custom_blend:chars=@#&'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_token_filter = my_lib.so:custom_blend:chars=@#&

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### overshort_step

```ini
overshort_step = {0|1}
```

<!-- example overshort_step -->
对过短（少于 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)）关键词的位置增量。可选，允许值为 0 和 1，默认是 1。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) overshort_step = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'overshort_step' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  overshort_step = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary

```ini
phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis
```

<!-- example phrase_boundary -->
短语边界字符列表。可选，默认空。

该列表控制哪些字符被视为短语边界，以便调整词的位置并通过邻近搜索实现短语级搜索模拟。语法类似于 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)，但不允许映射且边界字符不得与其他内容重叠。

在短语边界处，会向当前词位置额外添加由 [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 指定的位置增量。这实现了通过邻近查询的短语级搜索：不同短语中的词之间的距离保证大于 phrase_boundary_step；因此在该距离内的邻近搜索等同于短语级搜索。

只有当此字符后跟分隔符时，才判定为短语边界；这样做是为了避免诸如 S.T.A.L.K.E.R 这样的缩略词或 URL 被当作多个短语处理。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary' => '., ?, !, U+2026',
             'phrase_boundary_step' => '10'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Pytho-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary = ., ?, !, U+2026
  phrase_boundary_step = 10

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary_step

```ini
phrase_boundary_step = 100
```

<!-- example phrase_boundary_step -->
短语边界词位置增量。可选，默认是 0。

在短语边界处，当前词位置将额外增加此数字。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary_step' => '100',
             'phrase_boundary' => '., ?, !, U+2026'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary_step = 100
  phrase_boundary = ., ?, !, U+2026

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### regexp_filter

```ini
# index '13"' as '13inch'
regexp_filter = \b(\d+)\" => \1inch

# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color
```

<!-- example regexp_filter -->
正则表达式（regexps）用于过滤字段和查询。该指令是可选的，支持多值，默认值是一个空的正则表达式列表。Manticore Search 使用的正则表达式引擎是 Google 的 RE2，该引擎以其速度和安全性闻名。有关 RE2 支持的语法的详细信息，可以访问 [RE2 语法指南](https://github.com/google/re2/wiki/Syntax)。

在某些应用中，如产品搜索，可能有多种方式来指代某个产品、型号或属性。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指的是同一款产品。另一个例子是表达笔记本屏幕尺寸的不同方式，如 `13-inch`、`13 inch`、`13"` 或 `13in`。

正则表达式提供了一种机制，用于指定处理此类情况的规则。在第一个例子中，您可能会使用词形变化文件来处理少量的 iPhone 型号，但在第二个例子中，更好地指定规则，将 “13-inch” 和 “13in” 规范化为相同的形式。

列在 `regexp_filter` 中的正则表达式按列出的顺序应用，在可能的最早阶段，在任何其他处理之前（包括[例外情况](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)），甚至在分词之前。也就是说，正则表达式应用于建立索引时的原始源字段，以及搜索时的原始搜索查询文本。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'regexp_filter' => '(blue|red) => color'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  # index '13"' as '13inch'
  regexp_filter = \b(\d+)\" => \1inch

  # index 'blue' or 'red' as 'color'
  regexp_filter = (blue|red) => color

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

