# 低级别分词

当文本在Manticore中被索引时，它会被分割成单词并进行大小写折叠，这样像"Abc"、"ABC"和"abc"这样的单词就会被视为同一个单词。

为了正确执行这些操作，Manticore必须知道：
* 源文本的编码（应始终为UTF-8）
* 哪些字符被视为字母，哪些不是
* 哪些字母应被折叠为其他字母

您可以使用[字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)选项在每个表的基础上配置这些设置。字符集表指定了一个数组，将字母字符映射到它们的大小写折叠版本（或您喜欢的任何其他字符）。数组中不存在的字符被视为非字母，并在此表的索引或搜索过程中被视为单词分隔符。

默认的字符集是`non_cont`，它包含了[大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

您还可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

文本`RED TUBE 5" LONG`将被索引为`COLOR TUBE 5 INCH LONG`，而`PLANK 2" x 4"`将被索引为`PLANK 2 INCH x 4 INCH`。这些规则按指定顺序应用。这些规则也适用于查询，因此搜索`BLUE TUBE`实际上会搜索`COLOR TUBE`。

您可以在[regexp_filter这里](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)了解更多信息。

## 索引配置选项

### 字符集表

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
`字符集表`指定了一个数组，将字母字符映射到它们的大小写折叠版本（或您喜欢的任何其他字符）。默认的字符集是`non_cont`，它包含了大多数使用[非连续](https://en.wikipedia.org/wiki/Scriptio_continua)文字的语言。

`字符集表`是Manticore分词过程的核心，它从文档文本或查询文本中提取关键词。它控制哪些字符被视为有效字符以及它们应如何转换（例如，是否应移除大小写）。

默认情况下，每个字符都映射到0，这意味着它不被视为有效关键词，并被当作分隔符处理。一旦一个字符在表中被提及，它就会被映射到另一个字符（最常见的是映射到自身或小写字母），并被当作有效关键词的一部分。

字符集表使用逗号分隔的映射列表来声明字符为有效字符或将它们映射到其他字符。语法快捷方式可用于一次性映射字符范围：

* 单字符映射：`A->a`。声明源字符'A'在关键词中允许，并将其映射到目标字符'a'（但不声明'a'为允许）。
* 范围映射：`A..Z->a..z`。声明源范围内的所有字符为允许，并将它们映射到目标范围。不声明目标范围为允许。检查两个范围的长度。
* 游离字符映射：`a`。声明一个字符为允许，并将其映射到自身。等同于`a->a`单字符映射。
* 游离范围映射：`a..z`。声明范围内的所有字符为允许，并将它们映射到自身。等同于`a..z->a..z`范围映射。
* 棋盘式范围映射：`A..Z/2`。将每对字符映射到第二个字符。例如，`A..Z/2`等同于`A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。这个映射快捷方式对于大写和小写字母交错排列的Unicode块很有帮助。

对于代码从0到32的字符，以及8位ASCII和Unicode字符范围内127到8位的字符，Manticore始终将它们视为分隔符。为了避免配置文件编码问题，8位ASCII字符和Unicode字符必须以`U+XXX`形式指定，其中`XXX`是十六进制码点数字。可接受的最小Unicode字符代码是`U+0021`。

如果默认映射不足以满足您的需求，您可以通过指定另一个映射来重新定义字符映射。例如，如果内置的`non_cont`数组包含字符`Ä`和`ä`并将它们都映射到ASCII字符`a`，您可以通过添加它们的Unicode码点来重新定义这些字符，像这样：

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
除了字符和映射的定义外，还有几个内置的别名可以使用。当前的别名有：
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

如果您想在搜索中支持不同的语言，定义所有语言的有效字符集和折叠规则可能是一项繁重的任务。我们为您简化了这一过程，提供了默认的字符集表 `non_cont` 和 `cont`，分别涵盖非连写脚本语言和连写（中文、日文、韩文、泰文）脚本的语言。在大多数情况下，这些字符集应该足以满足您的需求。

Please note that the following languages are currently **not** supported:
* Assamese
* Bishnupriya
* Buhid
* Garo
* Hmong
* Ho
* Komi
* Large Flowery Miao
* Maba
* Maithili
* Marathi
* Mende
* Mru
* Myene
* Ngambay
* Odia
* Santali
* Sindhi
* Sylheti

所有其他列在[Unicode 语言
列表](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/)默认均受支持。

<!-- example charset_table 3 -->
要同时处理 cont 和 non-cont 语言，请在您的配置文件中按以下示例设置选项（对于中文有一个[例外](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)）：

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

如果您不需要连续脚本语言的支持，您可以简单地排除 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 和 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)。
选项。有关这些选项的更多信息，请参阅相应的文档章节。

要将一个字符映射到多个字符或反之，使用 [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 会很有帮助。

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
混合字符列表。可选，默认为空。

混合字符既被索引为分隔符，也被索引为有效字符。例如，当 `&` 被定义为混合字符且文档中出现 `AT&T` 时，将索引三种不同的关键字：`at&t`、`at` 和 `t`。

此外，混合字符会以一种仿佛未输入混合字符的方式影响索引。这种行为在指定 `blend_mode = trim_all` 时尤其明显。例如，对于短语 `some_thing`，在 `blend_mode = trim_all` 下将分别索引为 `some`、`something` 和 `thing`。

在使用混合字符时需谨慎，因为将某个字符定义为混合字符意味着它不再被视为分隔符。
* 因此，如果您将逗号包含在 `blend_chars` 中并搜索 `dog,cat`，它将被视为单一标记 `dog,cat`。如果 `dog,cat` **未**被索引为 `dog,cat`，而仅被拆分为 `dog cat`，则不会匹配。
* 因此，应通过 [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) 设置来控制此行为。

将混合字符替换为空格后得到的标记位置按常规分配，常规关键字的索引则好像完全未指定 `blend_chars` 一样。还会在起始位置插入一个混合了混合字符和非混合字符的额外标记。例如，如果 `AT&T company` 出现在文本字段的最前面，则 `at` 将被赋予位置1，`t` 位置2，`company` 位置3，同时 `AT&T` 也将被赋予位置1，与首个常规关键字混合。因此，对 `AT&T` 或 `AT` 的查询都会匹配该文档。对 `"AT T"` 的短语查询也会匹配，对 `"AT&T company"` 的短语查询同样匹配。

混合字符可能与查询语法中使用的特殊字符重叠，例如 `T-Mobile` 或 `@twitter`。在可能的情况下，查询解析器会将这些混合字符按混合字符处理。例如，如果 `hello @twitter` 位于引号内（即短语操作符），查询解析器会将 `@` 符号视为混合字符。但如果 `@` 符号未位于引号内，则该字符会被视为操作符。因此，建议对关键字进行转义。

混合字符可以被重新映射，以便将多种不同的混合字符规范化为一种基础形式。这在为多个具有等效字形的 Unicode 代码点创建索引时非常有用。

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
混合令牌索引模式由 blend_mode 指令启用。

默认情况下，混合了混合字符和非混合字符的标记会被完整索引。例如，当 `blend_chars` 中同时包含 `@` 符号和感叹号时，字符串 `@dude!` 将被索引为两个标记：`@dude!`（包含所有混合字符）和 `dude`（不包含任何混合字符）。因此，对 `@dude` 的查询将**不会**匹配该标记。

`blend_mode` 为此索引行为添加了灵活性。它接受一个以逗号分隔的选项列表，每个选项指定一种标记索引变体。

如果指定了多个选项，同一个词元将会有多个变体被索引。常规关键词（通过将混合字符替换为分隔符从该词元生成）总是会被索引。

可用的选项包括：

* `trim_none` - 索引整个词元
* `trim_head` - 去除开头的混合字符，并索引处理后的词元
* `trim_tail` - 去除结尾的混合字符，并索引处理后的词元
* `trim_both` - 去除开头和结尾的混合字符，并索引处理后的词元
* `trim_all` - 去除开头、结尾和中间的混合字符，并索引处理后的词元
* `skip_pure` - 如果词元完全由混合字符组成（即纯混合字符），则不进行索引

使用 `blend_mode` 处理上面的例子 `@dude!`，设置 `blend_mode = trim_head, trim_tail` 将导致两个被索引的词元：`@dude` 和 `dude!`。使用 `trim_both` 则不会有额外效果，因为去除两端的混合字符后得到 `dude`，这已经作为常规关键词被索引了。索引 `@U.S.A.` 时使用 `trim_both`（假设点号也是混合字符）将导致 `U.S.A` 被索引。最后，`skip_pure` 允许你忽略纯混合字符序列。例如，`one @@@ two` 将被索引为 `one two`，并作为短语匹配。默认情况下并非如此，因为一个完全混合的词元会被索引，并会偏移第二个关键词的位置。

默认行为是索引整个词元，相当于 `blend_mode = trim_none`。

请注意，使用混合模式会限制你的搜索，即使使用默认模式 `trim_none`，如果你假设 `.` 是混合字符：
* `.dog.` 在索引时会变成 `.dog. dog`
* 你将无法通过 `dog.` 找到它。

使用更多模式会增加你的关键词匹配到内容的几率。

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

min_word_len 是 Manticore 中一个可选的索引配置选项，用于指定被索引单词的最小长度。默认值为 1，这意味着所有内容都会被索引。

只有长度不小于此最小值的单词才会被索引。例如，如果 min_word_len 设置为 4，那么 'the' 将不会被索引，但 'they' 会被索引。

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
用于 N-gram 索引的 N-gram 长度。可选，默认值为 0（禁用 n-gram 索引）。已知的值为 0 和 1。

N-gram 为未分词文本中的连续脚本语言提供了基本支持。在连续脚本语言中进行搜索的问题在于缺乏清晰的单词分隔符。在某些情况下，你可能不想使用基于词典的分词，例如[中文可用的那种](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些情况下，n-gram 分词也可能效果良好。

启用此功能后，这些语言（或任何其他在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 中定义的字符）的字符流将被索引为 N-gram。例如，如果输入的文本是 "ABCDEF"（其中 A 到 F 代表某种语言的字符）且 ngram_len 为 1，它将被索引为 "A B C D E F"。目前仅支持 ngram_len=1。只有列在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 表中的字符会以这种方式分割；其他字符不会受到影响。

请注意，如果搜索查询是分词的，即单词之间有分隔符，那么将单词用引号括起来并使用扩展模式，即使文本**没有**分词，也能找到正确的匹配。例如，假设原始查询是 `BC DEF`。在应用程序端用引号括起来后，它应该看起来像 `"BC" "DEF"`（*带*引号）。这个查询将被传递给 Manticore，并在内部也分割为 1-gram，形成 `"B C" "D E F"` 查询，仍然带有作为短语匹配操作符的引号。即使文本中没有分隔符，它也能匹配到文本。

即使搜索查询没有分词，Manticore 仍然应该能产生良好的结果，这得益于基于短语的排名：它会将更接近的短语匹配（在 N-gram 单词的情况下，可能意味着更接近的多字符单词匹配）排在前面。

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
N-gram字符列表。可选，默认为空。

与 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 共同使用，此列表定义了哪些字符序列将被提取为N-gram。由其他字符组成的单词不会受到N-gram索引功能的影响。值的格式与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同。N-gram字符不能出现在 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java：

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust：

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
您也可以使用我们的默认N-gram表的别名，如示例所示。在大多数情况下，这应该是足够的。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java：

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust：

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
忽略字符列表。可选，默认为空。

当某些字符，例如软连字符（U+00AD），不仅应被视为分隔符，而且完全忽略时，此功能很有用。例如，如果'-'不在charset_table中，则"text-abc-def"文本将被索引为"abc"和"def"关键字。相反，如果'- '添加到ignore_chars列表中，则相同的文本将被索引为单个"abcdef"关键字。

语法与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同，但只允许声明字符，不允许映射它们。此外，忽略的字符必须不在charset_table中。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### Rust：

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
双字节索引模式。可选，默认为无。

双字节索引是一种加速短语搜索的功能。在索引时，它会存储文档列表，这些列表可以是所有或部分相邻词对。这些列表可以在搜索时用于显著加速短语或子短语匹配。

`bigram_index` 控制特定词对的选择。已知的模式有：

* `all`，索引每个单独的词对
* `first_freq`，仅索引第一个词是频繁词的词对（见 [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)）。例如，`bigram_freq_words = the, in, i, a`，则索引"text alone in the dark"文本时，将存储"the dark"和"in the"词对作为双字节，因为它们以频繁关键词（分别为"the"或"in"）开头，但"alone in"不会被索引，因为"in"是该词对中的第二个词。
* `both_freq`，仅索引两个词都是频繁词的词对。继续上述例子，在这种模式下索引"text alone in the dark"只会存储"in the"（从搜索角度来看最糟糕的一个）作为双字节，但没有任何其他词对。

对于大多数用例，`both_freq` 是最佳模式，但您的体验可能会有所不同。

重要的是要注意，`bigram_index` 只在分词级别工作，并不考虑诸如 `形态学`、`词形变化` 或 `停用词` 等转换。这意味着它创建的词元非常直接，这使得短语匹配更加精确和严格。虽然这可以提高短语匹配的准确性，但也使系统更难以识别不同形式的词或词的不同表现形式。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### Rust：

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
一个关键词列表，当索引双字节时被认为是“频繁”的。可选，默认为空。

一些双词索引模式（参见[bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)）需要定义一组频繁关键词。这些关键词**不**与停用词混淆。停用词在索引和搜索时都会被完全消除。频繁关键词仅用于双词来确定是否索引当前词对。

`bigram_freq_words` 允许您定义这样一组关键词。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### Rust：

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
关键词字典的类型由两个已知值之一标识，'crc' 或 'keywords'。这是可选的，默认值为 'keywords'。

使用关键词字典模式（dict=keywords）可以显著减少索引负担，并允许在大量集合上进行子字符串搜索。此模式可用于普通表和RT表。

CRC字典在索引中不存储原始关键词文本。相反，在搜索和索引过程中，它们会将关键词替换为控制和值（使用FNV64计算）。该值在索引内部使用。这种方法有两个缺点：
* 首先，不同关键词对之间存在控制和碰撞的风险。这种风险随着索引中唯一关键词数量的增长而增长。然而，这一担忧是次要的，因为在包含10亿条条目的字典中，单个FNV64碰撞的概率约为1/16或6.25％。大多数字典中的关键词数量远少于10亿，因为典型的口语人类语言有1到1000万个词形。
* 其次，更重要的是，使用控制和进行子字符串搜索并不直观。Manticore通过预索引所有可能的子字符串作为单独的关键词解决了这个问题（参见[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)，[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)指令）。这种方法甚至具有匹配子字符串的最快方式的优势。然而，预索引所有子字符串会显著增加索引大小（通常增加3-10倍或更多），从而影响索引时间，使得在大型索引上进行子字符串搜索变得不太实际。

关键词字典解决了这两个问题。它在索引中存储关键词并在搜索时执行通配符扩展。例如，对于前缀`test*`的搜索可能会根据字典内容内部扩展为'test|tests|testing'查询。这个扩展过程对应用程序来说是完全不可见的，除了每个匹配关键词的独立统计信息现在也会报告。

对于子字符串（通配符）搜索，可以使用扩展通配符。特殊字符如`?`和`%`与子字符串（通配符）搜索兼容（例如，`t?st*`，`run%`，`*abc*`）。请注意，[通配符操作符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)和[REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator)仅在`dict=keywords`时生效。

使用关键词字典的索引大约比常规非子字符串索引慢1.1倍到1.3倍，但比子字符串索引（无论是前缀还是通配符）要快得多。索引大小应仅略大于标准非子字符串表，总差异为1％至10％。常规关键词搜索所需的时间应在三种索引类型（CRC非子字符串，CRC子字符串，关键词）之间几乎相同或相同。子字符串搜索时间可以根据给定子字符串的实际关键词匹配数量（即搜索词扩展成多少关键词）显著波动。匹配关键词的最大数量受[expansion_limit](../../Server_settings/Searchd.md#expansion_limit)指令限制。

总之，关键词和CRC字典提供了两种不同的权衡决策以实现子字符串搜索。您可以选择牺牲索引时间和索引大小以获得最快的最坏情况搜索（CRC字典），或者最小地影响索引时间但在前缀扩展成大量关键词时牺牲最坏情况搜索时间（关键词字典）。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### Rust：

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
嵌入式异常、词形变体或停用词文件大小限制。可选，默认值为16K。

当创建表时，上述文件可以保存在表外部或直接嵌入到表中。小于`embedded_limit`的文件会被存储到表中。对于较大的文件，仅存储文件名。这也简化了将表文件移动到另一台机器的过程；您只需复制一个文件即可。

对于较小的文件，这种嵌入方式减少了表格依赖的外部文件数量，有助于维护。但同时，将100MB的词形词典嵌入到一个微小的增量表中是没有意义的。因此需要有一个大小阈值，`embedded_limit`就是这个阈值。

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
包含全局（集群范围）关键词IDF的文件路径。可选，缺省为空（使用本地IDF）。

在一个多表集群中，每个关键词的频率很可能在不同表之间存在差异。这意味着当排名函数使用基于TF-IDF的值（例如BM25系列因子）时，根据它们所在的集群节点，结果可能会被略微不同地排序。

解决这个问题的最简单方法是创建并使用一个全局频率字典，或简称为全局IDF文件。此指令允许您指定该文件的位置。建议（但不强制）使用.idf扩展名。当为某个表指定了IDF文件*且* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为1时，引擎将使用global_idf文件中的关键词频率和集合文档计数，而不是仅使用本地表。这样，IDF及其依赖的值将在整个集群中保持一致。

IDF文件可以在多个表之间共享。即使许多表引用了该文件，`searchd`也只会加载一个IDF文件的副本。如果IDF文件内容发生变化，可以通过SIGHUP信号加载新内容。

您可以使用 [indextool](../../Miscellaneous_tools.md#indextool) 工具构建.idf文件，首先通过 `--dumpdict dict.txt --stats` 开关转储字典，然后使用 `--buildidf` 将这些字典转换为.idf格式，最后使用 `--mergeidf` 将集群中所有.idf文件合并。

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
无命中词列表。可选，允许的值为 'all' 或列表文件名。

默认情况下，Manticore 全文索引不仅为每个关键词存储匹配文档列表，还存储其在文档中的位置列表（称为命中列表）。命中列表支持短语、邻近、严格顺序等高级搜索类型，以及短语邻近排序。然而，某些频繁关键词（由于某些原因无法被过滤，尽管它们很频繁）的命中列表可能会变得非常大，从而在查询时处理速度变慢。此外，在某些情况下，我们可能只关心布尔关键词匹配，而不需要基于位置的搜索操作符（如短语匹配）或短语排序。

`hitless_words` 允许您创建不包含任何位置信息（命中列表）的索引，或为特定关键词跳过位置信息。

无命中索引通常比相应的常规全文索引占用更少的空间（预计减少约1.5倍）。索引和搜索速度会更快，代价是缺少位置查询和排序支持。

如果在位置查询（例如短语查询）中使用无命中词，这些词将被移除并作为操作数使用，不带位置信息。例如，如果 "hello" 和 "world" 是无命中词，而 "simon" 和 "says" 不是，则短语查询 `"simon says hello world"` 将被转换为 `("simon says" & hello & world)`，匹配文档中任何位置的 "hello" 和 "world"，以及作为精确短语的 "simon says"。

仅包含无命中词的位置查询将导致空的短语节点，因此整个查询将返回空结果并发出警告。如果整个词典都是无命中词（使用 `all`），则只能在相应索引上使用布尔匹配。



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

### hitless_words_list

```ini
hitless_words_list = 'word1; word2; ...'
```

<!-- example hitless_words_list -->
`hitless_words_list` 设置允许您在 `CREATE TABLE` 语句中直接指定无命中词。此功能仅支持 [RT模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)。

值必须用分号（`;`）分隔。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words_list' => 'hello; world'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", Some(true)).await;
```

<!-- end -->

### index_field_lengths

```ini
index_field_lengths = {0|1}
```

<!-- example index_field_lengths -->
启用计算和存储字段长度（每文档和每索引平均值）到全文索引中。可选，缺省值为 0（不计算和存储）。

当 `index_field_lengths` 设置为 1 时，Manticore 将：
* 为每个全文字段创建相应的长度属性，名称相同但带有 `__len` 后缀
* 为每个文档计算字段长度（以关键词计数），并存储到相应属性中
* 计算每索引平均值。长度属性将具有特殊的 TOKENCOUNT 类型，但其值实际上是常规的 32 位整数，且其值通常可访问。

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 表达式排序器函数基于这些长度，需要启用 `index_field_lengths`。历史上，Manticore 使用了 BM25 的简化版，与完整函数不同，它**未**考虑文档长度。现在也支持完整的 BM25 变体及其扩展到多字段的版本，称为 BM25F。它们分别需要每文档长度和每字段长度。因此需要此附加指令。

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
全文索引的索引时标记过滤器。可选，缺省为空。

index_token_filter 指令指定全文索引的可选索引时标记过滤器。该指令用于创建根据自定义规则生成标记的自定义分词器。该过滤器由索引器在将源数据索引到普通表中时创建，或由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。插件通过 `library name:plugin name:optional string of settings` 格式定义。例如，`my_lib.so:custom_blend:chars=@#&`。

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
短于 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len) 的关键词的位移增量。可选，允许值为 0 和 1，缺省值为 1。

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
短语边界字符列表。可选，缺省为空。

此列表控制哪些字符将被视为短语边界，以调整词位置并启用通过邻近搜索模拟短语级搜索。语法类似于 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)，但不允许映射，且边界字符不得与其他内容重叠。

在短语边界处，将根据 [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 指定的数值，额外增加当前词位置。这使得通过邻近查询实现短语级搜索：不同短语中的词将保证彼此之间的距离超过 phrase_boundary_step；因此，在此距离内的邻近搜索将等同于短语级搜索。

只有当此类字符后跟分隔符时，才会触发短语边界条件；这是为了避免像 S.T.A.L.K.E.R 或 URL 被视为多个短语。

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
短语边界词位置增量。可选，缺省值为 0。

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
用于过滤字段和查询的正则表达式（regexps）。该指令是可选的，多值的，默认值为空的正则表达式列表。Manticore Search 使用的正则表达式引擎是 Google 的 RE2，以其速度和安全性著称。有关 RE2 支持的语法的详细信息，您可以访问 [RE2 语法指南](https://github.com/google/re2/wiki/Syntax)。

在某些应用场景（如产品搜索）中，可能存在许多指代产品、型号或属性的方式。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指代同一产品。另一个例子可能是笔记本电脑屏幕尺寸的不同表达方式，如 `13-inch`、`13 inch`、`13"` 或 `13in`。

正则表达式提供了一种机制，可以指定针对此类情况的定制规则。在第一个示例中，您可以使用词形文件来处理少量 iPhone 型号，但在第二个示例中，最好指定将 "13-inch" 和 "13in" 规范化为相同内容的规则。

`regexp_filter` 中列出的正则表达式将按照列出的顺序应用，在其他处理（包括 [例外](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)）之前尽可能早的阶段应用，甚至在分词之前。也就是说，正则表达式在索引时应用于原始源字段，在搜索时应用于原始查询文本。

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

