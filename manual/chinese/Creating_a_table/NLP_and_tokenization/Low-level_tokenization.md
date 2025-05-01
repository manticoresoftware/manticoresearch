# 低级词法分析  

当文本在Manticore中被索引时，它会被拆分成单词，并进行大小写折叠，以便像“Abc”、“ABC”和“abc”这样的单词被视为同一个单词。

为了正确执行这些操作，Manticore必须知道：
* 源文本的编码（应始终为UTF-8）
* 哪些字符被视为字母，哪些不是
* 哪些字母应该折叠为其他字母

你可以通过 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 选项在每个表上配置这些设置。charset_table指定一个数组，将字母字符映射到它们的大小写折叠版本（或任何你喜欢的其他字符）。数组中不存在的字符被视为非字母，并将在此表的索引或搜索过程中被视为单词分隔符。

默认字符集为 `non_cont`，它包括了 [大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

你还可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = **(d+)" =>  英寸
regexp_filter = (蓝色|红色) => 颜色
```

文本 `RED TUBE 5" LONG` 将被索引为 `颜色 TUBE 5 英寸 LONG`，而 `PLANK 2" x 4"` 将被索引为 `PLANK 2 英寸 x 4 英寸`。这些规则按指定顺序应用。规则也适用于查询，因此搜索 `BLUE TUBE` 实际上会搜索 `颜色 TUBE`。

你可以在 [此处了解更多关于 regexp_filter 的信息](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)。

## 索引配置选项

### charset_table

```ini
# 默认
charset_table = non_cont

# 仅英语和俄语字母
charset_table = 0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# 用别名定义英语字符集
charset_table = 0..9, english, _

# 你可以通过重新定义字符映射来覆盖字符映射，例如，对于不区分大小写的德语变音字符的搜索，你可以使用：
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- example charset_table -->
`charset_table` 指定一个数组，将字母字符映射到它们的大小写折叠版本（或其他你喜欢的字符）。默认字符集为 `non_cont`，它包括大多数语言的 [非连续](https://en.wikipedia.org/wiki/Scriptio_continua) 字母。

`charset_table` 是Manticore的词法分析过程的核心，它从文档文本或查询文本中提取关键字。它控制哪些字符被视为有效以及它们应该如何转换（例如，是否应该去除大小写）。

默认情况下，每个字符映射到0，这意味着它不被视为有效关键字，并被视为分隔符。一旦在表中提到一个字符，它将映射到另一个字符（通常是自身或小写字母），并被视为有效的关键字部分。

charset_table 使用逗号分隔的映射列表来声明字符为有效或将其映射到其他字符。可以使用语法快捷方式一次映射字符范围：

* 单字符映射： `A->a`。声明源字符'A'在关键字中被允许，并映射到目标字符'a'（但不声明'a'为被允许）。
* 范围映射： `A..Z->a..z`。声明源范围内的所有字符为被允许，并将它们映射到目标范围。不声明目标范围为被允许。检查两个范围的长度。
* 零散字符映射： `a`。声明一个字符为被允许，并将其映射到自身。等同于 `a->a` 单字符映射。
* 零散范围映射： `a..z`。声明范围内的所有字符为被允许，并将它们映射到自身。等同于 `a..z->a..z` 范围映射。
* 棋盘范围映射： `A..Z/2`。将每对字符映射到第二个字符。例如， `A..Z/2` 等同于 `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。这个映射快捷方式在大写和小写字母以交错顺序存在的Unicode块中特别有用。

对于代码从0到32的字符，以及在127到8位ASCII和Unicode字符范围内的字符，Manticore始终将它们视为分隔符。为了避免配置文件编码问题，8位ASCII字符和Unicode字符必须以 `U+XXX` 形式指定，其中 `XXX` 是十六进制代码点数字。接受的最小Unicode字符代码为 `U+0021`。

如果默认映射不足以满足你的需求，可以通过再次指定它们来重新定义字符映射。例如，如果内置的 `non_cont` 数组包括字符 `Ä` 和 `ä` 并将它们都映射到ASCII字符 `a`，你可以通过添加它们的Unicode代码点来重新定义这些字符，如下所示：

```
charset_table = non_cont,U+00E4,U+00C4
```

进行区分大小写的搜索或

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

进行不区分大小写的搜索。

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
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
根据您的指示，我已完整翻译了文档。由于文档较长，以下是翻译的文档：
            'charset_table' => '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'');
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
  charset_table = 0..9, A..Z->a..z, _, a..z,     U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example charset_table 2 -->
除了字符和映射的定义之外，还有几个可以使用的内置别名。当前的别名有：
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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'');
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

如果您希望在搜索中支持不同的语言，定义所有语言的有效字符集和折叠规则可能是一项繁琐的任务。我们已经为您提供了默认的字符表，`non_cont`和`cont`，它们分别涵盖了非连续和连续（中文、日文、韩文、泰文）脚本。在大多数情况下，这些字符集应该能满足您的需求。

请注意，以下语言目前**不**受支持：
* 阿萨姆语
* 比什努普里亚语
* 布希语
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
* 恩甘拜语
* 奥迪亚语
* 桑塔利语
* 信德语
* 锡尔赫特语

[Unicode语言列表](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/)中的所有其他语言默认都受支持。

<!-- example charset_table 3 -->
要同时处理cont和non-cont语言，请在配置文件中按如下所示设置选项（[中文除外](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)）：

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'');
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

如果您不需要对连续脚本语言的支持，可以简单地排除 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 和 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)。
选项。有关这些选项的更多信息，请参阅相应的文档部分。

要将一个字符映射到多个字符或反之，您可以使用 [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 会很有帮助。

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
混合字符列表。可选，默认值为空。

混合字符被索引为分隔符和有效字符。例如，当 `&` 被定义为混合字符时，如果在索引文档中出现 `AT&T`，则会索引三个不同的关键词，`at&t`、`at` 和 `t`。

此外，混合字符可以以这样的方式影响索引，使得关键词被索引为好像根本没有输入混合字符一样。 当指定 `blend_mode = trim_all` 时，这种行为尤为明显。例如，短语 `some_thing` 将被索引为 `some`、`something` 和 `thing`， 使用 `blend_mode = trim_all`。

使用混合字符时应谨慎，因为将字符定义为混合字符意味着它不再是分隔符。
* 因此，如果您将逗号放入 `blend_chars` 并搜索 `dog,cat`，它将视为一个单一的标记 `dog,cat`。如果 `dog,cat` **未** 被索引为 `dog,cat`，而是只保留为 `dog cat`，则不会匹配。
* 因此，此行为应通过 [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) 设置进行控制。

用空格替换混合字符后获得的标记的位置会照常分配，常规关键词将被索引为好像根本没有指定 `blend_chars` 一样。一个混合了混合字符和非混合字符的额外标记将放在起始位置。例如，如果 `AT&T company` 出现在文本字段的开头，`at` 将被赋予位置 1，`t` 位置 2，`company` 位置 3，`AT&T` 也会被赋予位置 1，与开头的常规关键词混合。因此，查询 `AT&T` 或仅 `AT` 将匹配该文档。短语查询 `"AT T"` 也将匹配，以及短语查询 `"AT&T company"`。

混合字符可能与查询语法中使用的特殊字符重叠，例如 `T-Mobile` 或 `@twitter`。在可能的情况下，查询解析器会将混合字符处理为混合字符。例如，如果 `hello @twitter` 在引号内（为短语操作符），则查询解析器会将 `@` 符号处理为混合字符。然而，如果 `@` 符号不在引号内，则字符将被视为操作符。因此，建议对关键词进行转义。

混合字符可以重新映射，以便将多个不同的混合字符规范化为一个基本形式。这在索引多个具有等效字形的替代 Unicode 代码点时非常有用。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'');
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
混合标记的索引模式通过 blend_mode 指令启用。

默认情况下，混合和非混合字符的标记会完全索引。例如，当在 `blend_chars` 中同时存在 at 符号和叹号时，字符串 `@dude!` 将作为两个标记进行索引：`@dude!`（包含所有混合字符）和 `dude`（没有任何）。因此，对 `@dude` 的查询将 **不** 匹配它。

`blend_mode` 为这种索引行为添加了灵活性。它接受一个以逗号分隔的选项列表，每个选项指定一个标记索引变体。

如果指定了多个选项，则同一个标记的多个变体将被索引。常规关键字（通过用分隔符替换混合字符而产生的标记）始终会被索引。

选项包括：

* `trim_none` - 索引整个标记
* `trim_head` - 剪裁头部混合字符，并索引结果标记
* `trim_tail` - 剪裁尾部混合字符，并索引结果标记
* `trim_both`- 剪裁头部和尾部混合字符，并索引结果标记
* `trim_all` - 剪裁头部、尾部和中间混合字符，并索引结果标记
* `skip_pure` - 如果标记纯粹是混合的，即只由混合字符组成，则不进行索引

使用 `blend_mode` 配合上述示例 `@dude!` 字符串，设置 `blend_mode = trim_head, trim_tail` 将导致两个索引标记：`@dude` 和 `dude!`。使用 `trim_both` 则不会产生影响，因为剪裁两个混合字符会得到 `dude`，而该标记已作为常规关键字被索引。使用 `trim_both` 索引 `@U.S.A.`（假设点是混合字符）将导致 `U.S.A` 被索引。最后，`skip_pure` 使您能够忽略仅由混合字符组成的序列。例如，`one @@@ two` 将被索引为 `one two`，并将作为短语进行匹配。由于完全混合的标记被索引并偏移第二个关键字位置，因此默认情况下并非如此。

默认行为是索引整个标记，等同于 `blend_mode = trim_none`。

请注意，使用混合模式会限制您的搜索，即使在默认模式 `trim_none` 下，如果您假设 `.` 是混合字符：
* `.dog.` 在索引期间将变为 `.dog. dog`
* 并且您将无法通过 `dog.` 找到它。

使用更多模式会增加您的关键字匹配到某些内容的机会。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'');
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
min_word_len = 长度
```

<!-- example min_word_len -->

min_word_len 是 Manticore 中的一个可选索引配置选项，用于指定最小索引词长度。默认值为 1，意味着会索引所有内容。

只有那些不短于此最小长度的词才会被索引。例如，如果 min_word_len 是 4，那么 'the' 将不会被索引，但 'they' 会被索引。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = '4'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = '4'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = '4'');
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
N-gram 长度用于 N-gram 索引。可选，默认为 0（禁用 n-gram 索引）。已知值为 0 和 1。

N-gram 为无分隔符的连续脚本语言文本提供基本支持。在使用连续脚本的语言中搜索的问题在于缺乏单词之间的明确分隔符。在某些情况下，您可能不想使用基于字典的分词，比如[针对中文可用的分词](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些情况下，n-gram 分词也可能很有效。

当启用此功能时，这些语言的字符流（或[ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)中定义的任何其他字符）将被索引为 N-gram。例如，如果输入文本是 "ABCDEF"（其中 A 到 F 代表某种语言的字符）且 ngram_len 为 1，它将被索引为 "A B C D E F"。目前仅支持 ngram_len=1。只有[ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)表中列出的字符会以这种方式拆分；其他字符不受影响。

请注意，如果搜索查询是分词的，即单词之间有分隔符，那么在应用程序端用引号包裹单词并使用扩展模式将能找到正确的匹配，即使文本并**未**分词。例如，假设原始查询是 `BC DEF`。在应用程序端用引号包裹后，它应该看起来像 `"BC" "DEF"`（*带有*引号）。这个查询将传递给 Manticore 并在内部拆分为 1-gram，结果为 `"B C" "D E F"` 查询，仍然带有作为短语匹配运算符的引号。即使文本中没有分隔符，它仍然可以匹配。

即使搜索查询未分词，Manticore 仍应产生良好的结果，这要归功于基于短语的排名：它会将更接近的短语匹配（在 N-gram 词的情况下，可以意味着更接近的多字符词匹配）排在前面。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'');
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
N-gram 字符列表。可选，默认为空。

用于与 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 结合使用，此列表定义了进行 N-gram 提取的字符序列。由其他字符组成的词将不受 N-gram 索引特性的影响。值的格式与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同。N-gram 字符不能出现在 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'');
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
您也可以为我们的默认 N-gram 表使用别名，如示例所示。在大多数情况下，这应该足够了。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'');
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
忽略的字符列表。可选，默认为空。
当某些字符（如软连字符（U+00AD））不应仅被视为分隔符，而应完全被忽略时很有用。例如，如果'-'不在charset_table中，"abc-def"文本将被索引为"abc"和"def"关键词。相反，如果'-'被添加到ignore_chars列表中，相同的文本将被索引为单个"abcdef"关键词。

语法与[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)相同，但只允许声明字符，不允许映射它们。另外，被忽略的字符不能出现在charset_table中。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'');
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
二元组索引模式。可选，默认为none。

二元组索引是一个用于加速短语搜索的功能。在索引时，它将相邻单词对的文档列表存储到索引中。这样的列表可以在搜索时显著加快短语或子短语匹配的速度。

`bigram_index`控制特定单词对的选择。已知的模式有：

* `all`，索引每个单词对
* `first_freq`，仅索引第一个单词在频繁词列表中的单词对（参见[bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)）。例如，设置`bigram_freq_words = the, in, i, a`，索引"alone in the dark"文本将导致"in the"和"the dark"对被存储为二元组，因为它们以频繁关键词开始（分别是"in"或"the"），但"alone in"将**不会**被索引，因为"in"在该对中是第二个单词。
* `both_freq`，仅索引两个单词都是频繁词的单词对。继续上面的例子，在此模式下，索引"alone in the dark"将只存储"in the"（从搜索角度来看是最糟糕的）作为二元组，而不存储其他任何单词对。

对于大多数用例，`both_freq`会是最佳模式，但具体效果可能因情况而异。

需要注意的是，`bigram_index`仅在分词级别工作，不考虑`morphology`、`wordforms`或`stopwords`等转换。这意味着它创建的标记非常直接，这使得短语搜索更加精确和严格。虽然这可以提高短语匹配的准确性，但也使系统较难识别单词的不同形式或单词出现的变体。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'');
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
在索引二元组时，考虑为“频繁”的关键字列表。可选，默认为空。

一些二元组索引模式（见 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)）要求定义一个频繁关键字的列表。这些**不要**与停止词混淆。停止词在索引和搜索时完全被消除。频繁关键字仅被二元组用于确定是否索引当前的单词对。

`bigram_freq_words` 允许您定义一个这样的关键字列表。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'');
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
使用的关键字字典类型由两个已知值之一标识，'crc'或'keywords'。这是可选的，默认是'keywords'。

使用关键字字典模式 (dict=keywords) 可以显著减少索引负担并支持在广泛集合上的子串搜索。此模式可用于普通表和RT表。

CRC字典在索引中不存储原始关键字文本。相反，它们在搜索和索引过程中用控制和谐值（使用FNV64计算）替换关键字。此值在索引中内部使用。此方法有两个缺点：
* 首先，不同关键字对之间存在控制和谐冲突的风险。此风险与索引中唯一关键字的数量成正比。尽管如此，这种担忧是次要的，因为在十亿条目字典中的单个FNV64碰撞的概率大约是1/16，或6.25%。考虑到通常口语化人类语言具有100万到1000万的词形，绝大多数字典的关键字数量远少于十亿。
* 其次，更重要的是，使用控制和谐值进行子串搜索并不简单。Manticore通过将所有可能的子串预索引为单独的关键字来解决此问题（见 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)， [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 指令）。此方法甚至具有以尽可能快的方式匹配子串的额外优点。然而，预索引所有子串显著增加索引大小（通常增加3-10倍或更多）并随之影响索引时间，使得在大型索引上进行子串搜索变得相当不切实际。
关键字字典解决了这两个问题。它在索引中存储关键字，并在搜索时执行通配符扩展。例如，对 `test*` 前缀的搜索可以根据字典的内容内部扩展为 'test|tests|testing' 查询。这个扩展过程对应用程序是完全不可见的，唯一的例外是匹配关键字的每个关键字的单独统计数据现在也会被报告。

对于子字符串（中缀）搜索，可以使用扩展通配符。特殊字符如 `?` 和 `%` 与子字符串（中缀）搜索兼容（例如，`t?st*`，`run%`，`*abc*`）。请注意， [通配符操作符](Searching/Full_text_matching/Operators.md#Wildcard-operators) 和 [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) 仅在 `dict=keywords` 时起作用。

使用关键字字典的索引速度大约比常规非子字符串索引慢 1.1 倍到 1.3 倍 - 但比子字符串索引（前缀或中缀）显著更快。索引大小通常只比标准非子字符串表稍大，总差异为 1..10% 百分比。常规关键字搜索所需的时间在讨论的所有三种索引类型（CRC 非子字符串、CRC 子字符串、关键字）之间应该几乎相同或相同。子字符串搜索时间可能会根据与给定子字符串匹配的实际关键字数量显著波动（即，搜索词扩展到多少个关键字）。匹配关键字的最大数量受到 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) 指令的限制。

总之，关键字和 CRC 字典为子字符串搜索提供了两种不同的权衡决策。您可以选择以牺牲索引时间和索引大小来实现最快的最坏情况搜索（CRC 字典），或者最小化对索引时间的影响，但牺牲在前缀扩展为大量关键字时的最坏情况搜索时间（关键字字典）。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) dict = 'keywords'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) dict = 'keywords'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = 'keywords'');
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
嵌入异常、词形或停用词文件大小限制。可选，默认值为 16K。

当您创建一个表时，上述提到的文件可以与表一起保存在外部，或直接嵌入到表中。大小在 `embedded_limit` 之内的文件将被存储到表中。对于更大的文件，只存储文件名。这也简化了将表文件移动到其他机器的过程；您只需复制一个文件即可。

对于较小的文件，这种嵌入减少了表所依赖的外部文件数量，并有助于维护。但与此同时，将一个 100 MB 的词形字典嵌入一个小型增量表也是没有意义的。因此，必须设定一个大小阈值，`embedded_limit` 就是该阈值。

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
指向包含全局（集群范围内）关键字 IDF 的文件的路径。可选，默认为空（使用本地 IDF）。

在多表集群中，按关键字的频率在不同表之间很可能会有所不同。这意味着，当排名函数使用基于 TF-IDF 的值（例如 BM25 因子的家族）时，结果可能会根据它们所在的集群节点稍微有所不同。
解决该问题的最简单方法是创建并使用一个全局频率字典，或简称全局 IDF 文件。该指令允许您指定该文件的位置。建议使用 .idf 扩展名（但不是必须的）。当为给定表指定 IDF 文件 *并且* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为 1 时，系统将使用来自 global_idf 文件的关键词频率和集合文档计数，而不仅仅是本地表。这样，IDF 和依赖于它们的值将在整个集群中保持一致。

IDF 文件可以在多个表之间共享。即使许多表引用该文件，`searchd` 也只会加载单一副本的 IDF 文件。如果 IDF 文件的内容发生变化，可以使用 SIGHUP 加载新内容。

您可以使用 [indextool](../../Miscellaneous_tools.md#indextool) 工具创建一个 .idf 文件，首先通过使用 `--dumpdict dict.txt --stats` 开关转储字典，然后使用 `--buildidf` 将其转换为 .idf 格式，最后使用 `--mergeidf` 合并集群中的所有 .idf 文件。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'');
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
无命中的单词列表。可选，允许的值为 'all' 或者列表文件名。

默认情况下，Manticore 全文索引不仅存储每个给定关键字的匹配文档列表，还存储其在文档中的位置列表（称为命中列表）。命中列表支持短语、临近、严格顺序和其他高级搜索类型，以及短语临近排名。然而，针对特定频繁关键字的命中列表（由于某种原因无法停止的频繁关键字）可能会变得非常庞大，因此在查询时处理速度较慢。此外，在某些情况下，我们可能只关心布尔关键字匹配，并且从来无需基于位置的搜索运算符（例如短语匹配）或短语排名。

`hitless_words` 允许您创建索引，这些索引完全没有位置信息（命中列表），或对特定关键字跳过位置信息。

无命中的索引通常会比相应的常规全文索引占用更少的空间（可以预期约 1.5 倍）。索引和搜索的速度应更快，但代价是缺少位置查询和排名支持。  

如果在位置查询中使用（例如短语查询），无命中的单词将被从中取出，并作为没有位置的运算符使用。例如，如果 "hello" 和 "world" 是无命中的，而 "simon" 和 "says" 不是无命中的，则短语查询 `"simon says hello world"` 将被转换为 `("simon says" & hello & world)`，在文档中匹配 "hello" 和 "world" 以及 "simon says" 作为一个确切的短语。

仅包含无命中的单词的位置信息查询将导致生成一个空的短语节点，因此整个查询将返回一个空结果和一个警告。如果整个字典都是无命中的（使用 `all`），则只能对相应的索引使用布尔匹配。



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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = 'all'')
```

<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = 'all'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = 'all'');
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
启用在全文索引中计算和存储字段长度（包括每个文档的字段长度和每个索引的平均字段长度）。可选，默认值为 0（不计算和存储）。

当 `index_field_lengths` 被设置为 1 时，Manticore 将会：
* 为每个全文字段创建相应的长度属性，属性名称相同但带有 `__len` 后缀
* 为每个文档计算字段长度（按关键词计数）并存储到相应的属性中
* 计算每个索引的平均值。长度属性将具有特殊的 TOKENCOUNT 类型，但它们的值实际上是常规的 32 位整数，并且通常可以访问。

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 函数在表达式排名器中基于这些长度，并且需要启用 `index_field_lengths`。历史上，Manticore 使用了 BM25 的简化版，与完整函数不同，该版本**不**考虑文档长度。同时也支持完整版本的 BM25 以及其针对多个字段的扩展，称为 BM25F。它们分别需要每个文档的长度和每个字段的长度。因此需要额外的指令。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = '1'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = '1'');
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
用于全文索引的索引时标记过滤器。可选，默认为空。

index_token_filter 指令指定了一个用于全文索引的可选索引时标记过滤器。该指令用于创建一个自定义分词器，根据自定义规则生成标记。该过滤器由索引器在将源数据索引到普通表中时创建，或由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。插件使用格式 `library name:plugin name:optional string of settings` 定义。例如，`my_lib.so:custom_blend:chars=@#&`。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'')
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'');
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
Position increment on overshort (less than [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) keywords. Optional, allowed values are 0 and 1, default is 1.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = '1'')
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = '1'')
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = '1'');
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
Phrase boundary characters list. Optional, default is empty.

This list controls what characters will be treated as phrase boundaries, in order to adjust word positions and enable phrase-level search emulation through proximity search. The syntax is similar to [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), but mappings are not allowed and the boundary characters must not overlap with anything else.

On phrase boundary, additional word position increment (specified by [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)) will be added to current word position. This enables phrase-level searching through proximity queries: words in different phrases will be guaranteed to be more than phrase_boundary_step distance away from each other; so proximity search within that distance will be equivalent to phrase-level search.

Phrase boundary condition will be raised if and only if such character is followed by a separator; this is to avoid abbreviations such as S.T.A.L.K.E.R or URLs being treated as several phrases.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'')
```

<!-- intro -->

##### Pytho-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'')
```

<!-- intro -->

##### Javascript:
<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'');
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
短语边界词位置增量。可选，默认值为 0。

在短语边界处，当前位置将额外增加此数字。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'');
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
regexp_filter = (d+)" => inch

# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color
```

<!-- example regexp_filter -->
正则表达式（regexps）用于过滤字段和查询。该指令是可选的、多值的，默认值为空的正则表达式列表。Manticore Search使用的正则表达式引擎是Google的RE2，以其速度和安全性而闻名。有关RE2支持的语法的详细信息，您可以访问 [RE2语法指南](https://github.com/google/re2/wiki/Syntax).

在某些应用程序中，例如产品搜索，可能有许多方式来引用产品、型号或属性。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指的是同一产品。另一个例子可能是不同的方式来表示笔记本电脑屏幕尺寸，如 `13-inch`、`13 inch`、`13"`或`13in`。

正则表达式提供了一种指定规则的机制，以处理此类情况。在第一个例子中，您可能使用一个词形文件来处理一些iPhone型号，但在第二个例子中，最好指定规则来规范化“13-inch”和“13in”为相同的内容。

列在 `regexp_filter` 中的正则表达式按列出的顺序应用，在尽可能早的阶段进行处理，在进行任何其他处理之前（包括[异常](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)），甚至在标记化之前。也就是说，正则表达式在索引时应用于原始源字段，在搜索时应用于原始搜索查询文本。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'');
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
  regexp_filter = (d+)" => inch

# 低级别标记化  

当文本在 Manticore 中被索引时，它会被拆分成单词，并进行大小写折叠，以便像 "Abc"、"ABC" 和 "abc" 这样的单词被视为同一个单词。

为了正确执行这些操作，Manticore 必须知道：
* 源文本的编码（应该始终是 UTF-8）
* 哪些字符被视为字母，哪些字符不是
* 哪些字母应该被折叠成其他字母

您可以使用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 选项在每个表的基础上配置这些设置。charset_table 指定了一个将字母字符映射到其大小写折叠版本（或您更喜欢的任何其他字符）的数组。数组中不存在的字符被视为非字母，并在此表的索引或搜索过程中被视为单词分隔符。

默认字符集是 `non_cont`，它包括 [大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

您还可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = **(d+)" =>  inch
regexp_filter = (BLUE|RED) => COLOR
```

文本 `RED TUBE 5" LONG` 将被索引为 `COLOR TUBE 5 INCH LONG`，而 `PLANK 2" x 4"` 将被索引为 `PLANK 2 INCH x 4 INCH`。这些规则按指定顺序应用。规则也适用于查询，因此搜索 `BLUE TUBE` 实际上会搜索 `COLOR TUBE`。

您可以在 [regexp_filter 这里了解更多](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)。

## 索引配置选项

### charset_table

```ini
# 默认
charset_table = non_cont

# 仅英语和俄语字母
charset_table = 0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# 使用别名定义英语字符集
charset_table = 0..9, english, _

# 您可以通过重新定义字符映射来覆盖， 例如对于带有德语变音符的大小写不敏感搜索，您可以使用：
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- example charset_table -->
`charset_table` 指定一个将字母字符映射到其大小写折叠版本（或您更喜欢的任何其他字符）的数组。默认字符集是 `non_cont`，这包括大多数具有 [非连续](https://en.wikipedia.org/wiki/Scriptio_continua) 脚本的语言。

`charset_table` 是 Manticore 标记化过程中的主力，提取文档文本或查询文本中的关键字。它控制哪些字符被视为有效，以及它们应该如何转换（例如，是否应该移除大小写）。

默认情况下，每个字符映射到 0，这意味着它不被视为有效关键字，并被视为分隔符。一旦字符在表中被提及，它就被映射到另一个字符（通常是映射到自身或小写字母）并被视为有效的关键字部分。

charset_table 使用逗号分隔的映射列表来声明字符为有效或将它们映射到其他字符。语法快捷方式可以在一次映射一组字符时使用：

* 单字符映射： `A->a`。声明源字符 'A' 在关键字中是被允许的，并将其映射到目标字符 'a'（但并不声明 'a' 是被允许的）。
* 范围映射： `A..Z->a..z`。声明源范围内的所有字符是被允许的，并将它们映射到目标范围内。不声明目标范围是被允许的。检查两个范围的长度。
* 零散字符映射： `a`。声明字符为被允许，并将其映射到自身。等同于 `a->a` 的单字符映射。
* 零散范围映射： `a..z`。声明范围内的所有字符为被允许，并将其映射到自身。等同于 `a..z->a..z` 的范围映射。
* 棋盘格范围映射： `A..Z/2`。将每对字符映射到第二个字符。例如， `A..Z/2` 等同于 `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。这种映射快捷方式对大写字母和小写字母交错顺序的 Unicode 块非常有用。

对于代码在 0 到 32 之间的字符，以及在 127 到 8 位 ASCII 和 Unicode 字符范围内的字符，Manticore 始终将其视为分隔符。为了避免配置文件编码问题，8 位 ASCII 字符和 Unicode 字符必须以 `U+XXX` 形式指定，其中 `XXX` 是十六进制代码点数字。接受的最小 Unicode 字符代码是 `U+0021`。

如果默认映射不足以满足您的需求，您可以通过再次指定其他映射来重新定义字符映射。例如，如果内置的 `non_cont` 数组包含字符 `Ä` 和 `ä` 并将它们都映射到 ASCII 字符 `a`，您可以通过添加它们的 Unicode 代码点来重新定义这些字符，如下所示：

```
charset_table = non_cont,U+00E4,U+00C4
```

对于区分大小写的搜索

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = 0..9, A..Z->a..z, _, a..z,     U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example charset_table 2 -->
除了字符和映射的定义之外，还有几个内置别名可供使用。目前的别名有：
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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
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

如果您想在搜索中支持不同的语言，为它们定义一套有效字符和折叠规则可能是一项繁琐的任务。我们为您简化了这一过程，提供了默认的字符集表，`non_cont` 和 `cont`，分别覆盖断续语言和连写语言（如中文、日文、韩文、泰文）。在大多数情况下，这些字符集应能满足您的需求.

请注意，目前**不**支持以下语言：
* 阿萨姆语
* 比施努普里亚语
* 布希德语
* 加罗语
* 苗语
* 霍语
* 科米语
* 大花苗语
* 马巴语
* 麦蒂利语
* 马拉地语
* 曼德语
* 慕鲁语
* 麦延语
* 恩甘拜语
* 奥迪亚语
* 桑塔利语
* 信德语
* 希尔赫蒂语

其他在 [Unicode languages
list](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/) 的语言默认均受支持.

<!-- example charset_table 3 -->
若要同时处理连写和断续语言，请在配置文件中设置选项，如下所示（中文除外，[参见例外](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)）：

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
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

如果您不需要对连续脚本语言的支持，您可以简单地排除 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 和 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)。
options. 有关这些选项的更多信息，请参考相应的文档部分。

要将一个字符映射到多个字符或反之亦然，您可以使用 [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 来帮助。

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
混合字符列表。可选，默认为空。

混合字符被索引为分隔符和有效字符。例如，当 `&` 被定义为混合字符并且 `AT&T` 出现在索引文档中时，将索引三个不同的关键字 `at&t`、`at` 和 `t`。

此外，混合字符可以影响索引，使关键字的索引看起来就像根本没有输入混合字符一样。当指定 `blend_mode = trim_all` 时，这种行为尤其明显。例如，短语 `some_thing` 将被索引为 `some`、`something` 和 `thing`，使用 `blend_mode = trim_all`。

使用混合字符时要小心，因为将一个字符定义为混合字符意味着它不再是分隔符。
* 因此，如果您在 `blend_chars` 中放置一个逗号并搜索 `dog,cat`，它将把其视为一个单一的标记 `dog,cat`。如果 `dog,cat` **未** 被索引为 `dog,cat`，而仅保留为 `dog cat`，那么它将不匹配。
* 因此，此行为应通过 [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) 设置进行控制。

通过用空格替换混合字符获得的标记的位置将按常规分配，常规关键字将被索引，就好像根本没有指定 `blend_chars` 一样。一个混合了混合字符和非混合字符的额外标记将放在起始位置。例如，如果 `AT&T company` 发生在文本字段的最开头，`at` 将获得位置 1，`t` 为位置 2，`company` 为位置 3，并且 `AT&T` 也将获得位置 1，与开头的常规关键字混合。因此，查询 `AT&T` 或仅 `AT` 将匹配该文档。短语查询 `"AT T"` 也将匹配，以及短语查询 `"AT&T company"`。

混合字符可以与查询语法中使用的特殊字符重叠，例如 `T-Mobile` 或 `@twitter`。在可能的情况下，查询解析器将将混合字符视为混合字符。例如，如果 `hello @twitter` 在引号内（即短语运算符），查询解析器将把 `@` 符号视为混合字符。然而，如果 `@` 符号不在引号内，则该字符将被视为运算符。因此，建议对关键字进行转义。

可以重新映射混合字符，以便将多个不同的混合字符归一化为一个基本形式。这在为多个具有等效字形的替代Unicode编码点进行索引时非常有用。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
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
混合标记的索引模式通过 blend_mode 指令启用。

默认情况下，混合和非混合字符混合的标记会被完全索引。例如，当一个 at 符号和一个感叹号都在 `blend_chars` 中时，字符串 `@dude!` 将被索引为两个标记：`@dude!`（带有所有混合字符）和 `dude`（没有任何混合字符）。因此，一个查询 `@dude` 将**不会**匹配它。

`blend_mode` 为此索引行为增加了灵活性。它接受一个用逗号分隔的选项列表，每个选项指定一个标记索引变体。
如果指定多个选项，则将对同一标记的多个变体进行索引。常规关键字（通过用分隔符替换混合字符生成的标记）将始终被索引。

选项如下：

* `trim_none` - 索引整个标记
* `trim_head` - 修剪头部混合字符，并索引生成的标记
* `trim_tail` - 修剪尾部混合字符，并索引生成的标记
* `trim_both`- 同时修剪头部和尾部混合字符，并索引生成的标记
* `trim_all` - 修剪头部、尾部和中间混合字符，并索引生成的标记
* `skip_pure` - 如果标记是纯粹混合的，则不索引该标记，即仅包含混合字符

使用 `blend_mode` 和上面的示例 `@dude!` 字符串，设置 `blend_mode = trim_head, trim_tail` 将导致两个索引标记：`@dude` 和 `dude!`。使用 `trim_both` 将没有效果，因为修剪两个混合字符的结果是 `dude`，而它已经作为常规关键字被索引。使用 `trim_both` 对 `@U.S.A.` 进行索引（并假定该点被混合两次）将导致 `U.S.A` 被索引。最后，`skip_pure` 允许你忽略仅由混合字符组成的序列。例如，`one @@@ two` 将被索引为 `one two`，并将其匹配为一个短语。默认情况下情况并非如此，因为完全混合的标记会被索引并偏移第二个关键字的位置。

默认行为是索引整个标记，等同于 `blend_mode = trim_none`。

请注意，使用混合模式会限制您的搜索，即使在默认模式 `trim_none` 下，如果你假定 `.` 是一个混合字符：
* `.dog.` 在索引过程中将变成 `.dog. dog`
* 而你将无法通过 `dog.` 查找它。

使用更多模式会增加你的关键字匹配到某些内容的机会。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
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

min_word_len 是 Manticore 中的一个可选索引配置选项，指定最小索引单词长度。默认值为 1，这意味着所有内容都被索引。

只有那些不短于此最小值的单词会被索引。例如，如果 min_word_len 为 4，则 'the' 将不会被索引，但 'they' 会。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = '4'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = '4'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
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
N-gram 长度用于 N-gram 索引。可选，默认值为 0（禁用 n-gram 索引）。已知值为 0 和 1。

N-grams 为无分隔文本中的连续脚本语言提供基本支持。使用连续脚本的语言进行搜索的问题在于缺少单词之间的清晰分隔符。在某些情况下，您可能不想使用基于字典的分词，例如[中国可用的分词](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些情况下，n-gram 分词也可能运作良好。

启用此功能时，这些语言的流（或在[ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)中定义的任何其他字符）将被索引为 N-grams。例如，如果传入的文本是“ABCDEF”（其中 A 到 F 代表一些语言字符）并且 ngram_len 为 1，它将被索引为“ A B C D E F”。目前仅支持 ngram_len=1。只有在[ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)表中列出的字符才会以这种方式被拆分；其他字符则不受影响。

请注意，如果搜索查询被分词，即单词之间有分隔符，那么将单词用引号包裹并使用扩展模式将导致即使文本**没有**被分词，也能找到正确的匹配。例如，假设原始查询是 `BC DEF`。在应用程序端用引号包裹后，它应该看起来像 `"BC" "DEF"` (*带* 引号)。此查询将传递给 Manticore，并在内部也拆分为 1-grams，从而得到 `"B C" "D E F"` 查询，仍然带有作为短语匹配运算符的引号。即使文本中没有分隔符，它也会匹配文本。

即使搜索查询没有被分词，得益于基于短语的排名，Manticore 仍然应该产生良好的结果：它将把更接近的短语匹配（在 N-gram 单词的情况下，这可能意味着更接近的多字符单词匹配）放到最上面。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
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
N-gram 字符列表。可选，默认为空。

与[ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)结合使用，此列表定义了哪些字符序列将进行 N-gram 提取。由其他字符组成的单词将不受 N-gram 索引功能的影响。值的格式与[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)相同。N-gram 字符不能出现在[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
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
此外，你也可以像示例中那样为我们的默认 N-gram 表使用别名。在大多数情况下，这已经足够。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
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
Ignored characters list. Optional, default is empty.

在某些情况下非常有用，例如某些字符（如软连字符标记 (U+00AD)）不仅仅被视为分隔符，而是被完全忽略。例如，如果 '-' 根本不在 charset_table 中，则 "abc-def" 文本将被索引为 "abc" 和 "def" 关键词。相反，如果 '-' 被添加到 ignore_chars 列表中，则相同的文本将被索引为单个 "abcdef" 关键词.

语法与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同，但只允许声明字符，不允许对它们进行映射。此外，被忽略的字符不得出现在 charset_table 中.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
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
Bigram indexing mode. Optional, default is none.

Bigram 索引是一项加速短语搜索的功能。在索引时，它将所有或部分相邻单词对的文档列表存储到索引中。然后这样的列表在搜索时可以显著加速短语或子短语匹配.

`bigram_index` 控制特定单词对的选择。已知的模式包括：

* `all`，索引每一个单词对
* `first_freq`，仅索引单词对中*第一个*单词出现在常用词列表中的单词对 (参见 [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). 例如，对于 `bigram_freq_words = the, in, i, a`，索引 "alone in the dark" 文本将导致 "in the" 和 "the dark" 单词对被存储为 bigrams，因为它们分别以常用关键词（分别为 "in" 或 "the"）开头，而 "alone in" 不会被索引，因为 "in" 在该单词对中是*第二个*单词.
* `both_freq`，仅索引两个单词都属于常用词的单词对. 继续同样的例子，在此模式下，索引 "alone in the dark" 仅会存储 "in the"（从搜索角度来看这是最糟糕的情况）作为 bigram，而其他单词对不会被索引.

对于大多数使用情况，`both_freq` 是最佳模式，但具体情况可能有所不同.

需要注意的是，`bigram_index` 仅在标记化层面工作，不考虑诸如 `morphology`、`wordforms` 或 `stopwords` 等转换。这意味着它创建的标记非常直接，从而使短语搜索更为精确和严格。虽然这可以提高短语匹配的准确性，但也使系统在识别单词的不同形式或单词出现的变化方面能力较弱.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
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
A list of keywords considered "frequent" when indexing bigrams. Optional, default is empty.

Some of the bigram indexing modes (see [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) require to define a list of frequent keywords. These are **not** to be confused with stop words. Stop words are completely eliminated when both indexing and searching. Frequent keywords are only used by bigrams to determine whether to index a current word pair or not.

`bigram_freq_words` lets you define a list of such keywords.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
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
The type of keywords dictionary used is identified by one of two known values, 'crc' or 'keywords'. This is optional, with 'keywords' as the default.

Using the keywords dictionary mode (dict=keywords) can significantly decrease the indexing burden and enable substring searches on extensive collections. This mode can be utilized for both plain and RT tables.

CRC dictionaries do not store the original keyword text in the index. Instead, they replace keywords with a control sum value (computed using FNV64) during both searching and indexing processes. This value is used internally within the index. This approach has two disadvantages:
* Firstly, there's a risk of control sum collisions between different keywords pairs. This risk grows in proportion to the number of unique keywords in the index. Nonetheless, this concern is minor as the probability of a single FNV64 collision in a dictionary of 1 billion entries is roughly 1 in 16, or 6.25 percent. Most dictionaries will have far fewer than a billion keywords given that a typical spoken human language has between 1 and 10 million word forms.
* 其次，更重要的是，使用控制总和进行子字符串搜索并不简单。Manticore 通过将所有可能的子字符串预先索引为单独的关键词来解决这个问题（参见 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)，[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 指令）。这种方法甚至具有以最快的方式匹配子字符串的额外优点。然而，预先索引所有子字符串会显著增加索引大小（通常增加 3-10 倍或更多），并随之影响索引时间，使在大型索引上进行子字符串搜索变得相当不切实际。

关键词字典解决了这两个问题。它在索引中存储关键词并在搜索时执行通配符扩展。例如，搜索 `test*` 前缀可能会基于字典的内容内部扩展为 'test|tests|testing' 查询。这个扩展过程对应用程序是完全不可见的，唯一的例外是现在还报告了所有匹配关键词的单独每关键词统计数据。

对于子字符串（中缀）搜索，可以使用扩展通配符。特殊字符如 `?` 和 `%` 与子字符串（中缀）搜索兼容（例如，`t?st*`，`run%`，`*abc*`）。请注意，只有在 `dict=keywords` 时，[通配符操作符](Searching/Full_text_matching/Operators.md#Wildcard-operators) 和 [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) 才能正常工作。

使用关键词字典进行索引的速度约比常规非子字符串索引慢 1.1 倍到 1.3 倍——但明显比子字符串索引（前缀或中缀）快。索引大小应仅略大于标准非子字符串表，差异总计在 1..10% 之间。常规关键词搜索所需的时间在讨论的所有三种索引类型（CRC 非子字符串、CRC 子字符串、关键词）中应该是几乎相同的或相同的。子字符串搜索时间可能会显著波动，具体取决于有多少实际关键词与给定子字符串匹配（即，搜索词扩展为多少关键词）。匹配关键词的最大数量受限于 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) 指令。

总之，关键词和 CRC 字典为子字符串搜索提供了两种不同的折衷选择。您可以选择牺牲索引时间和索引大小以实现最快的最坏情况搜索（CRC 字典），或者对索引时间的影响最小，但在前缀扩展为大量关键词时牺牲最坏情况搜索时间（关键词字典）。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) dict = 'keywords'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = 'keywords'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
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
嵌入式异常、词形或停用词文件大小限制。可选，默认为 16K。

创建表时，上述提到的文件可以与表一起外部保存或直接嵌入表中。大小在 `embedded_limit` 之下的文件将存储在表中。对于更大的文件，仅存储文件名。这也简化了将表文件移动到不同机器的过程；您只需复制一个文件即可。

对于较小的文件，这种嵌入减少了表依赖的外部文件数量，并有助于维护。但同时，将 100 MB 的词形字典嵌入一个小的 delta 表也没有意义。因此需要一个大小阈值，而 `embedded_limit` 就是这个阈值。

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
包含全局（集群范围）关键词 IDF 的文件路径。可选，默认为空（使用本地 IDF）。

在一个多表集群中，各个表之间的关键词频率很可能会有所不同。这意味着，当排名函数使用基于 TF-IDF 的值时，例如 BM25 因子的族，结果可能会根据它们所在的集群节点略有不同。
修复该问题的最简单方法是创建并使用一个全局频率字典，或简称为全局 IDF 文件。该指令允许您指定该文件的位置。建议（但不是必需）使用 .idf 扩展名。当为给定表指定 IDF 文件 *并且* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为 1 时，引擎将使用来自 global_idf 文件的关键词频率和文档集合计数，而不仅仅是本地表。这样，IDF 及其依赖的值将在整个集群中保持一致。

IDF 文件可以在多个表之间共享。即使许多表引用该文件，`searchd` 也只会加载一份 IDF 文件。如果 IDF 文件的内容发生更改，可以通过 SIGHUP 加载新内容。

您可以使用 [indextool](../../Miscellaneous_tools.md#indextool) 工具构建 .idf 文件，首先使用 `--dumpdict dict.txt --stats` 开关转储字典，然后使用 `--buildidf` 将其转换为 .idf 格式，最后使用 `--mergeidf` 合并集群中的所有 .idf 文件。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
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
无匹配词列表。可选，允许的值为 'all' 或列表文件名。

默认情况下，Manticore 全文索引不仅存储每个给定关键词的匹配文档列表，还存储其在文档中的位置列表（称为命中列表）。命中列表支持短语、接近、严格顺序和其他高级搜索类型，以及短语接近排名。然而，特定的频繁关键词的命中列表（因为某种原因无法停止）可能会变得非常庞大，因此在查询时处理速度缓慢。此外，在某些情况下，我们可能只关心布尔关键词匹配，而不需要基于位置的搜索操作符（例如短语匹配）或短语排名。

`hitless_words` 允许您创建没有位置信息（命中列表）或跳过特定关键词的索引。

无匹配索引通常会比相应的常规全文索引使用更少的空间（预计约 1.5 倍）。索引和搜索的速度应该会更快，但代价是缺少位置查询和排名支持。  

如果在位置查询（例如短语查询）中使用，无匹配词将被从中删除，并作为没有位置的操作数使用。例如，如果 "hello" 和 "world" 是无匹配词，而 "simon" 和 "says" 不是无匹配词，那么短语查询 `"simon says hello world"` 将被转换为 `("simon says" & hello & world)`，在文档中任何地方匹配 "hello" 和 "world"，并将 "simon says" 作为精确短语。

仅包含无匹配词的定位查询将导致一个空短语节点，因此整个查询将返回空结果和警告。如果整个字典都是无匹配的（使用 `all`），则只能在相应的索引上使用布尔匹配。



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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = 'all'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = 'all'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
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
启用计算和存储字段长度（每个文档和每个索引的平均值）到全文索引中。可选，默认值为0（不计算和存储）。

当`index_field_lengths`设置为1时，Manticore将：
* 为每个全文字段创建一个相应的长度属性，名称相同，但后缀为`__len`
* 计算每个文档的字段长度（以关键字计数）并存储到相应的属性中
* 计算每个索引的平均值。长度属性将具有特殊的TOKENCOUNT类型，但它们的值实际上是常规的32位整数，并且其值通常是可访问的。

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 表达式排名器中的函数基于这些长度，并要求启用`index_field_lengths`。历史上，Manticore使用了一种简化的、简化的BM25变体，与完整函数不同，它**不**考虑文档长度。也支持BM25的完整变体及其扩展至多个字段，称为BM25F。它们分别要求每个文档的长度和每个字段的长度。因此有了附加指令。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = '1'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = '1'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
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
索引时间的令牌过滤器，用于全文索引。可选，默认值为空。

index_token_filter指令指定了一个可选的索引时间令牌过滤器，用于全文索引。该指令用于创建一个自定义分词器，根据自定义规则生成令牌。该过滤器由索引器在将源数据索引到普通表中或通过RT表处理`INSERT`或`REPLACE`语句时创建。插件使用格式`library name:plugin name:optional string of settings`定义。例如，`my_lib.so:custom_blend:chars=@#&`。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
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
在短词（少于[min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)）关键字上的位置增量。可选，允许的值为0和1，默认值为1。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- request JSON -->

```JSON
我将按照指示翻译文档，但保留原始的结构和代码块。以下是翻译后的文档：
POST /cli -d "
CREATE TABLE products(title text, price float) overshort_step = '1'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = '1'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = '1'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
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

### 短语边界

```ini
phrase_boundary = ., ?, !, U+2026 # 水平省略号
```

<!-- example phrase_boundary -->
短语边界字符列表。可选，默认为空。

这个列表控制哪些字符将被视为短语边界，以调整单词位置并通过邻近搜索模拟短语级搜索。语法类似于[字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)，但不允许映射，且边界字符不得与其他内容重叠。

在短语边界，将额外添加当前单词位置增量（由[phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)指定）。这使得通过邻近查询进行短语级搜索成为可能：不同短语中的单词将保证相距超过phrase_boundary_step距离；因此在该距离内的邻近搜索将等同于短语级搜索。

仅当此类字符后跟分隔符时，才会触发短语边界条件；这是为了避免诸如S.T.A.L.K.E.R或URL等缩写被视为多个短语。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
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

### 短语边界步长

```ini
phrase_boundary_step = 100
```

<!-- example phrase_boundary_step -->
短语边界单词位置增量。可选，默认为0。

在短语边界，当前单词位置将额外增加此数值。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'');
```
<!-- intro -->
##### java:
（文档到此结束）
utilsApi.sql("创建表 products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("创建表 products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
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
regexp_filter = (d+)" => inch

# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color
```

<!-- example regexp_filter -->
正则表达式（regexps）用于过滤字段和查询。此指令是可选的，支持多值，默认值为空的正则表达式列表。Manticore Search 使用的正则表达式引擎是 Google 的 RE2，以其速度和安全性而闻名。如需详细了解 RE2 支持的语法，您可以访问 [RE2 语法指南](https://github.com/google/re2/wiki/Syntax)。

在某些应用程序中，例如产品搜索，可能有多种方式来引用产品、型号或属性。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指的是同一产品。另一个例子可以是不同方式表示笔记本电脑屏幕尺寸，例如 `13-inch`、`13 inch`、`13"` 或 `13in`。

Regexps 提供了一种机制来指定定制的规则以处理此类情况。在第一个示例中，您可能使用一个词形文件来处理少数几款 iPhone 型号，但在第二个示例中，最好指定规则来将 "13-inch" 和 "13in" 正常化为相同的内容。

`regexp_filter` 中列出的正则表达式按列出的顺序应用，在尽可能早的阶段应用，甚至在任何其他处理（包括 [例外](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)）之前，甚至在分词之前。也就是说，当索引时正则表达式应用于原始源字段，当搜索时应用于原始搜索查询文本。

<!-- request SQL -->

```sql
创建表 products(title text, price float) regexp_filter = '(blue|red) => color'
```

<!-- request JSON -->

```JSON
POST /cli -d "
创建表 products(title text, price float) regexp_filter = '(blue|red) => color'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('创建表 products(title text, price float) regexp_filter = '(blue|red) => color'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('创建表 products(title text, price float) regexp_filter = '(blue|red) => color'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("创建表 products(title text, price float) regexp_filter = '(blue|red) => color'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("创建表 products(title text, price float) regexp_filter = '(blue|red) => color'");
```

<!-- request CONFIG -->

```ini
table products {
  # index '13"' as '13inch'
  regexp_filter = (d+)" => inch

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





# 低级别词元化  

当文本在 Manticore 中被索引时，它会被拆分成单词并进行大小写折叠，以便 "Abc"、"ABC" 和 "abc" 等单词被视为相同的单词。

为了正确执行这些操作，Manticore 必须知道：
* 源文本的编码（应该始终是 UTF-8）
* 哪些字符被视为字母，哪些不被视为字母
* 哪些字母应该折叠为其他字母

您可以通过使用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 选项在每个表的基础上配置这些设置。 charset_table 指定一个数组，将字母字符映射到其大小写折叠版本（或您更喜欢的任何其他字符）。未出现在数组中的字符被视为非字母，并在此表中进行索引或搜索时视为单词分隔符。

默认字符集是 `non_cont`，包括 [大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

您还可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

文本 `RED TUBE 5" LONG` 将被索引为 `COLOR TUBE 5 INCH LONG`，而 `PLANK 2" x 4"` 将被索引为 `PLANK 2 INCH x 4 INCH`。这些规则按照指定的顺序应用。规则同样适用于查询，因此搜索 `BLUE TUBE` 实际上会搜索 `COLOR TUBE`。

您可以在 [这里了解更多关于 regexp_filter 的信息](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)。

## 索引配置选项

### charset_table

```ini
# 默认
charset_table = non_cont

# 仅包括英语和俄语字母
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# 用别名定义的英语字符集
charset_table = 0..9, english, _

# 您可以通过重新定义字符映射进行覆盖，例如对于德语变音字母的不区分大小写搜索，您可以使用：
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- 示例 charset_table -->
`charset_table` 指定一个数组，将字母字符映射到其大小写折叠版本（或您更喜欢的任何其他字符）。默认字符集是 `non_cont`，包括大多数具有 [不连续](https://en.wikipedia.org/wiki/Scriptio_continua) 脚本的语言。

`charset_table` 是 Manticore 词元化过程中重要的工具，它从文档文本或查询文本中提取关键字。它控制接受哪些字符作为有效字符以及如何进行转换（例如，是否应去除大小写）。

默认情况下，每个字符映射为 0，这意味着它不被视为有效关键字，并被视为分隔符。一旦字符在表中提到，它将被映射为另一个字符（最常见的情况是，映射为自身或小写字母），并被视为有效关键字的一部分。

charset_table 使用以逗号分隔的映射列表来声明字符为有效或将其映射到其他字符。可以使用语法快捷方式一次性映射字符范围：

* 单字符映射： `A->a`。声明源字符 'A' 在关键字中被允许，并将其映射为目标字符 'a'（但不声明 'a' 为允许）。
* 范围映射： `A..Z->a..z`。声明源范围中的所有字符被允许并将其映射到目标范围。不声明目标范围为允许。检查两个范围的长度。
* 干扰字符映射： `a`。声明一个字符为允许并将其映射为自身。相当于 `a->a` 的单字符映射。
* 干扰范围映射： `a..z`。声明范围内的所有字符为允许并将其映射为自身。相当于 `a..z->a..z` 的范围映射。
* 棋盘范围映射： `A..Z/2`。将每对字符映射为第二个字符。例如， `A..Z/2` 等同于 `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。此映射快捷方式对于大写字母和小写字母交错排列的 Unicode 块很有帮助。

对于代码从 0 到 32 以及 127 到 8 位 ASCII 和 Unicode 字符范围的字符，Manticore 始终将它们视为分隔符。为避免配置文件编码问题，8 位 ASCII 字符和 Unicode 字符必须以 `U+XXX` 形式指定，其中 `XXX` 是十六进制代码点数。最小接受的 Unicode 字符代码是 `U+0021`。

如果默认映射不足以满足您的需求，您可以通过再次指定它们并使用另一个映射来重新定义字符映射。例如，如果内置的 `non_cont` 数组包括字符 `Ä` 和 `ä` 并将它们都映射为 ASCII 字符 `a`，您可以通过添加它们的 Unicode 代码点来重新定义这些字符，如下所示：

```
charset_table = non_cont,U+00E4,U+00C4
```

进行区分大小写的搜索或

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

进行不区分大小写的搜索。

<!-- 请求 SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
```

<!-- 请求 JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'"
```

<!-- 请求 PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
{original document with exact formatting and structure, only translating human language content while preserving all technical elements}
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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
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
除了字符和映射的定义外，还可以使用几个内置别名。当前的别名包括：
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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
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

如果您想在搜索中支持不同的语言，为所有语言定义有效字符集和折叠规则可能是一项繁琐的任务。我们通过提供默认字符集表 `non_cont` 和 `cont` 来简化这个过程，这些字符集分别覆盖了非连续和连续（中文、日语、韩语、泰语）文字的语言。在大多数情况下，这些字符集应该足以满足您的需求。

请注意，以下语言目前**不**支持：
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
* 米埃内语
* 恩甘拜语
* 奥里亚语
* 桑塔利语
* 信德语
* 锡尔赫特语

[Unicode语言列表](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/)中列出的所有其他语言默认都支持。

<!-- example charset_table 3 -->
要同时处理连续和非连续语言，请按如下方式在配置文件中设置选项（中文有[例外情况](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)）：

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
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

如果您不需要对连续脚本语言提供支持，可以简单地排除 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 和 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)。
选项。有关这些选项的更多信息，请参阅相应的文档部分。

要将一个字符映射到多个字符或反之亦然，您可以使用 [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)。

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
混合字符列表。可选，默认为空。

混合字符被索引为分隔符和有效字符。例如，当 `&` 被定义为混合字符时，在索引文档中出现 `AT&T` 时，将索引三个不同的关键字，`at&t`、`at` 和 `t`。

此外，混合字符可以影响索引，导致关键字的索引方式仿佛完全没有输入混合字符。这一行为在指定 `blend_mode = trim_all` 时特别明显。例如，短语 `some_thing` 将被索引为 `some`、`something` 和 `thing`，同时使用了 `blend_mode = trim_all`。

使用混合字符时应谨慎，因为将一个字符定义为混合字符意味着它不再是分隔符。
* 因此，如果您在 `blend_chars` 中放置逗号并搜索 `dog,cat`，它会将其视为单个令牌 `dog,cat`。如果 `dog,cat` **没有** 索引为 `dog,cat`，而只保留为 `dog cat`，那么它将不匹配。
* 因此，这种行为应该通过 [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) 设置来控制。

通过用空格替换混合字符获得的令牌的位置照常分配，常规关键字将被索引，仿佛根本没有指定 `blend_chars`。一个同时混合了混合和非混合字符的额外令牌将放在起始位置。例如，如果 `AT&T company` 出现在文本字段的最开头，`at` 将被赋予位置1，`t` 位置2，`company` 位置3，`AT&T` 也将被赋予位置1，与开头的常规关键字混合。因此，查询 `AT&T` 或仅 `AT` 将匹配该文档。短语查询 `"AT T"` 也将匹配，以及短语查询 `"AT&T company"`。

混合字符可能与查询语法中使用的特殊字符重叠，如 `T-Mobile` 或 `@twitter`。如果可能，查询解析器将处理该混合字符。如果 `hello @twitter` 在引号中（短语运算符），查询解析器将把 `@` 符号视为混合字符。然而，如果 `@` 符号不在引号中，该字符将被视为运算符。因此，建议转义关键字。

混合字符可以被重新映射，以便将多种不同的混合字符规范化为一个基本形式。这在对多个具有等效字形的备选Unicode代码点进行索引时非常有用。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
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
混合令牌索引模式通过 blend_mode 指令启用。

默认情况下，与非混合字符混合的令牌会被完全索引。例如，当 `blend_chars` 中同时包含一个 at 符号和一个感叹号时，字符串 `@dude!` 将被索引为两个令牌：`@dude!`（包含所有混合字符）和 `dude`（不包含任何）。因此，查询 `@dude` 将 **不** 匹配它。

`blend_mode` 为这种索引行为增加了灵活性。它接受一个以逗号分隔的选项列表，每个选项指定一种令牌索引变体。
如果指定了多个选项，将索引同一标记的多个变体。常规关键字（由该标记通过用分隔符替换混合字符而产生）始终被索引。

选项如下：

* `trim_none` - 索引整个标记

* `trim_head` - 去掉头部混合字符，并索引结果标记

* `trim_tail` - 去掉尾部混合字符，并索引结果标记

* `trim_both`- 同时去掉头部和尾部混合字符，并索引结果标记

* `trim_all` - 去掉头部、尾部和中间的混合字符，并索引结果标记

* `skip_pure` - 如果该标记纯粹是混合的，即仅由混合字符组成，则不索引该标记

使用 `blend_mode` 和上述示例 `@dude!` 字符串，设置 `blend_mode = trim_head, trim_tail` 将生成两个索引标记：`@dude` 和 `dude!`。使用 `trim_both` 将没有效果，因为同时去掉两个混合字符的结果是 `dude`，它已经作为常规关键字被索引。将 `@U.S.A.` 用 `trim_both` 索引（并假设点被混合为两个）将导致 `U.S.A` 被索引。最后，`skip_pure` 使你能够忽略仅包含混合字符的序列。例如，`one @@@ two` 将被索引为 `one two`，并且将其作为一个短语匹配。这在默认情况下不是这样，因为一个完全混合的标记会被索引并偏移第二个关键字位置。

默认行为是索引整个标记，相当于 `blend_mode = trim_none`。

请注意，使用混合模式限制了你的搜索，即使在默认模式 `trim_none` 下，如果你假设 `.` 是一个混合字符：

* `.dog.` 在索引过程中会变为 `.dog. dog`

* 而你将无法通过 `dog.` 找到它。

使用更多模式增加了你的关键字匹配某些内容的机会。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
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

min_word_len 是 Manticore 中的一个可选索引配置选项，它指定了最小索引单词长度。默认值为 1，这意味着所有内容都会被索引。

只有那些长度不小于这个最小值的单词会被索引。例如，如果 min_word_len 是 4，那么 'the' 将不会被索引，但 'they' 将被索引。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
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
N-gram 长度用于 N-gram 索引。可选，默认值为 0（禁用 n-gram 索引）。已知值为 0 和 1。
N-grams 为未分段文本中的连续脚本语言提供基本支持。在使用连续脚本的语言中进行搜索的问题在于单词之间缺乏明确的分隔符。在某些情况下，您可能不想使用基于字典的分段，例如 [可用于中文的分段](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些情况下，n-gram 分段也可能效果良好。

当启用此功能时，此类语言（或在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 中定义的任何其他字符）的流将被索引为 N-grams。例如，如果传入的文本是 "ABCDEF"（其中 A 到 F 代表一些语言字符），而 ngram_len 为 1，它将被索引为 "A B C D E F"。当前仅支持 ngram_len=1。只有在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 表中列出的字符会以这种方式被分割；其他字符将不受影响。

注意，如果搜索查询已分段，即各个单词之间有分隔符，则将单词用引号括起来并使用扩展模式将导致即使文本 **未** 分段也能找到正确匹配。例如，假设原始查询是 `BC DEF`。在应用程序端用引号括起来后，它应显示为 `"BC" "DEF"` (*带* 引号)。此查询将被传递给 Manticore，并在内部也被分割成 1-grams，形成 `"B C" "D E F"` 的查询，仍然带有引号，即短语匹配操作符。即使文本中没有分隔符，它也将匹配该文本。

即使搜索查询未分段，由于基于短语的排名，Manticore 仍然应该产生良好的结果：它将更近的短语匹配（在 N-gram 词的情况下可能意味着更近的多字符单词匹配）拉到顶部。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
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

与 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 一起使用，此列表定义会进行 N-gram 提取的字符序列。由其他字符组成的单词将不受 N-gram 索引功能的影响。值的格式与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同。N-gram 字符不能出现在 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中。

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
##### Javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
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
此外，您可以像示例中那样为我们的默认N-gram表使用别名。这在大多数情况下应该足够了。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
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
忽略字符列表。可选，默认为空。

在某些情况下，当一些字符（如软连字符标记U+00AD）不应仅被视为分隔符而是完全忽略时特别有用。例如，如果“-”不在charset_table中，文本“abc-def”将索引为“abc”和“def”关键字。相反，如果“-”被加入ignore_chars列表，相同的文本将被索引为单个“abcdef”关键字。

语法与[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)相同，但只允许声明字符，不允许映射它们。此外，忽略的字符不得出现在charset_table中。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
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
双字索引模式。可选，默认为none。

双字索引是一种加速短语搜索的功能。在索引时，它在索引中存储所有或部分相邻单词对的文档列表。这种列表可以在搜索时显著加速短语或子短语匹配。

`bigram_index` 控制特定单词对的选择。已知的模式有：

* `all`，索引每一对单词
* `first_freq`，仅索引*第一个*单词在频繁单词列表中的词对（参见 [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)）。例如，使用 `bigram_freq_words = the, in, i, a`，索引“alone in the dark”文本将存储“in the”和“the dark”作为双字，因为它们以频繁关键词开头（分别是“in”或“the”），但“alone in”不会被索引，因为“in”在该对中为*第二个*单词。
* `both_freq`，仅索引两个单词都是频繁单词的词对。还是这个例子，在这种模式下索引“alone in the dark”仅存储“in the”作为双字，但没有其他词对。

对于大多数使用案例，`both_freq` 将是最佳模式，您的体验可能有所不同。

重要的是要注意，`bigram_index` 仅在分词级别工作，并不会考虑诸如 `morphology`、`wordforms` 或 `stopwords` 之类的转换。这意味着它创建的词元非常简单，这使得短语搜索更加精确和严格。虽然这可以提高短语匹配的准确性，但也使系统识别单词的不同形式或单词出现的变体的能力降低。

<!-- request SQL -->

```sql
创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'"
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
utilsApi.sql('创建表 products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('创建表 products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
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
一组在索引双字母时被视为“频繁”的关键词列表。可选，默认为空。

一些双字母索引模式（请参见 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)）需要定义一组频繁关键词。这些不要与停用词混淆。停用词在索引和搜索时会完全消除。频繁关键词仅由双字母用于判断是否索引当前的词对。

`bigram_freq_words` 让您定义这样关键词的列表。

<!-- request SQL -->

```sql
创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'"
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
utilsApi.sql('创建表 products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('创建表 products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("创建表 products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
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
使用的一种关键词字典的类型由两个已知值之一识别，'crc'或'keywords'。这是可选的，默认值为'keywords'。

使用关键词字典模式（dict=keywords）可以显著减轻索引负担，并在广泛的集合上启用子字符串搜索。此模式可用于普通和 RT 表。

CRC 字典不在索引中存储原始关键词文本。相反，它们在搜索和索引过程中将关键词替换为控制和价值（使用 FNV64 计算）。此值在索引内部使用。此方法有两个缺点：
* 首先，不同关键词对之间存在控制和冲突的风险。这个风险随着索引中唯一关键词数量的增加而增加。尽管如此，这种担忧很小，因为在 10 亿条目的字典中出现单个 FNV64 冲突的概率大约是 1/16，或 6.25%。考虑到典型的口语人类语言的单词形式在 100 万到 1000 万之间，大多数字典会拥有远低于 10 亿的关键词。
* 其次，更重要的是，执行带有控制和的子字符串搜索并不简单。Manticore 通过将所有可能的子字符串预先索引为单独的关键字来解决此问题（请参阅 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)， [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 指令）。此方法甚至具有以最快的方式匹配子字符串的额外优势。然而，预索引所有子字符串会显著增加索引大小（通常增加 3-10 倍或更多），并随后影响索引时间，使得在大型索引上执行子字符串搜索相当不切实际。

关键字字典解决了这两个问题。它在索引中存储关键字并执行搜索时的通配符扩展。例如，对 `test*` 前缀的搜索可以根据字典的内容内部扩展为 'test|tests|testing' 查询。此扩展过程对应用程序是完全不可见的，唯一例外是现在也报告了所有匹配关键字的单独每个关键字统计数据。

对于子字符串（内部）搜索，可以使用扩展通配符。特殊字符如 `?` 和 `%` 与子字符串（内部）搜索兼容（例如，`t?st*`、`run%`、`*abc*`）。请注意， [通配符操作符](Searching/Full_text_matching/Operators.md#Wildcard-operators) 和 [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) 仅在 `dict=keywords` 的情况下有效。

使用关键字字典的索引速度约比常规的非子字符串索引慢 1.1 倍至 1.3 倍——但仍然显著快于子字符串索引（无论是前缀还是内部）。索引大小仅应比标准非子字符串表稍大，差异总共为 1..10%。常规关键字搜索所需的时间在讨论的所有三种索引类型之间应几乎相同或相同（CRC 非子字符串、CRC 子字符串、关键字）。子字符串搜索时间可能会根据实际匹配给定子字符串的关键字数量而显著波动（即搜索词扩展为多少个关键字）。匹配关键字的最大数量受 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) 指令的限制。

总之，关键字和 CRC 字典为子字符串搜索提供了两种不同的权衡决策。您可以选择牺牲索引时间和索引大小以实现最快的最坏情况搜索（CRC 字典），或者在不大幅影响索引时间的情况下但牺牲在前缀扩展为大量关键字时的最坏情况搜索时间（关键字字典）。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
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
嵌入例外、词形或停用词文件大小限制。可选，默认值为 16K。

创建表时，上述文件可以与表一起存储为外部文件或直接嵌入表中。大小低于 `embedded_limit` 的文件被存储到表中。对于较大的文件，仅存储文件名。这也简化了将表文件移动到不同机器的过程；您只需复制一个文件即可。

对于较小的文件，此嵌入减少了表所依赖的外部文件的数量，并有助于维护。但同时，将一个 100 MB 的词形字典嵌入一个小的增量表也是没有意义的。因此，需要有一个大小阈值，而 `embedded_limit` 就是这个阈值。

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
指向包含全局（集群范围）关键字 IDF 的文件的路径。可选，默认值为空（使用本地 IDF）。

在多表集群中，按关键字的频率在不同表中很可能会有所不同。这意味着当排名函数使用基于 TF-IDF 的值时，例如 BM25 系列因子，结果可能会根据它们所属的集群节点略有不同。
最简单的解决这个问题的方法是创建并使用一个全局频率字典，简称全局 IDF 文件。这个指令让你指定该文件的位置。建议使用 .idf 扩展名（但不是必需的）。当为给定表指定 IDF 文件 *并且* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为 1 时，引擎将使用来自 global_idf 文件的关键字频率和集合文档计数，而不仅仅是本地表。这样，IDF 和依赖于它们的值将在集群中保持一致。

IDF 文件可以在多个表之间共享。即使有多个表引用该文件，`searchd` 也只会加载 IDF 文件的单个副本。如果 IDF 文件的内容发生变化，可以使用 SIGHUP 加载新内容。

你可以使用 [indextool](../../Miscellaneous_tools.md#indextool) 工具构建 .idf 文件，首先通过 `--dumpdict dict.txt --stats` 开关转储字典，然后使用 `--buildidf` 将它们转换为 .idf 格式，再使用 `--mergeidf` 合并集群中的所有 .idf 文件。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
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
无命中的词列表。可选，允许的值为 'all' 或名单文件名。

默认情况下，Manticore全文索引不仅存储每个给定关键字的匹配文档列表，还存储其在文档中的位置列表（称为命中列表）。命中列表可以启用短语、接近、严格顺序及其他高级搜索类型，以及短语接近排名。然而，特定的频繁关键字的命中列表（由于某种原因无法停止但仍然频繁）可能会变得庞大，因此在查询时处理起来会很慢。此外，在某些情况下，我们可能只关心布尔关键字匹配，并且永远不需要基于位置的搜索运算符（如短语匹配）或短语排名。

`hitless_words` 让你创建索引，完全没有位置信息（命中列表），或者为特定关键字跳过。

无命中的索引通常将比相应的常规全文索引占用更少的空间（预计大约减少 1.5 倍）。索引和搜索都应该更快，但代价是缺少位置查询和排名支持。  

如果在位置查询中使用（例如短语查询），则无命中的词会被去掉，并作为没有位置的操作数使用。例如，如果 "hello" 和 "world" 是无命中的，而 "simon" 和 "says" 不是无命中的，那么短语查询  `"simon says hello world"` 将被转换为 `("simon says" & hello & world)`，在文档中的任何地方匹配 "hello" 和 "world"，以及 "simon says" 作为确切短语。

仅包含无命中词的位置信息查询将导致空短语节点，因此整个查询将返回空结果和警告。如果整个字典是无命中的（使用 `all`），则只能在相应索引上进行布尔匹配。



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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
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
启用计算和存储字段长度（每个文档和每个索引的平均值）到全文索引。可选，默认值为 0（不计算和存储）。

当 `index_field_lengths` 设置为 1 时，Manticore 将：
* 为每个全文字段创建一个相应的长度属性，名称相同但带有 `__len` 后缀
* 计算每个文档的字段长度（以关键字计数）并存储到相应的属性中
* 计算每个索引的平均值。长度属性将具有特殊的 TOKENCOUNT 类型，但其值实际上是常规的 32 位整数，且其值通常是可以访问的。

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 函数在表达式排名中基于这些长度，并要求启用 `index_field_lengths`。历史上，Manticore 使用了 BM25 的简化、精简变体，该变体与完整函数不同，**不**考虑文档长度。还支持 BM25 的完整变体及其多个字段扩展，称为 BM25F。它们分别需要每个文档长度和每个字段长度。因此，新增了该指令。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
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
索引时间令牌过滤器，用于全文索引。可选，默认值为空。

index_token_filter 指令指定了一个可选的索引时间令牌过滤器，用于全文索引。该指令用于创建一个根据自定义规则生成令牌的自定义分词器。过滤器由索引器在将源数据索引到纯表中或通过 RT 表处理 `INSERT` 或 `REPLACE` 语句时创建。插件使用格式 `library name:plugin name:optional string of settings` 定义。例如，`my_lib.so:custom_blend:chars=@#&`。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
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
在过短（少于 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)）关键字上进行位置增量。可选，允许的值为 0 和 1，默认值为 1。

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
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```
<!-- intro -->
##### java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
```
<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
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
phrase_boundary = ., ?, !, U+2026 # 水平省略号
```
<!-- example phrase_boundary -->
短语边界字符列表。可选，默认为空。
此列表控制哪些字符将被视为短语边界，以调整词位置并通过邻近搜索来实现短语级搜索模拟。语法类似于[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)，但不允许映射，且边界字符不得与其他任何内容重叠。
在短语边界处，当前词位置将增加额外的词位置增量（由[phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)指定）。这通过邻近查询实现短语级搜索：不同短语中的词之间的距离保证会大于phrase_boundary_step；因此在该距离内的邻近搜索将等同于短语级搜索。
仅当这些字符后跟分隔符时才会触发短语边界条件；这是为了避免将诸如S.T.A.L.K.E.R或URL之类的缩写视为多个短语。
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
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```
<!-- intro -->
##### java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
```
<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
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
短语边界词位置增量。可选，默认为0。
在短语边界处，当前词位置将额外增加此数值。
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
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```
<!-- intro -->
##### java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
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
正则表达式（regexps）用于过滤字段和查询。此指令是可选的，多值的，其默认值是一个空的正则表达式列表。Manticore Search 使用的正则表达式引擎是谷歌的 RE2，以其速度和安全性著称。有关 RE2 支持的语法的详细信息，您可以访问 [RE2 语法指南](https://github.com/google/re2/wiki/Syntax)。

在某些应用程序中，例如产品搜索，可能有许多方式来引用产品、型号或属性。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指的是同一产品。另一个例子可能是表示笔记本电脑屏幕尺寸的不同方式，例如 `13-inch`、`13 inch`、`13"` 或 `13in`。

正则表达式提供了一种机制来指定针对这种情况处理的规则。在第一个例子中，您可能可以使用一个词形文件来处理少量的 iPhone 型号，但在第二个例子中，最好指定可以将 "13-inch" 和 "13in" 归一化为相同的规则。

在 `regexp_filter` 中列出的正则表达式以其列出的顺序应用，在任何其他处理之前（包括 [异常](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)），甚至在分词之前。也就是说，正则表达式在索引时应用于原始源字段，在搜索时应用于原始搜索查询文本。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'");
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















































