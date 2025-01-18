# 低级标记化  

当文本在 Manticore 中被索引时，它会被分割成单词，并且会进行大小写折叠，以便像 "Abc"、"ABC" 和 "abc" 这样的单词被视为同一个词。

为了正确执行这些操作，Manticore 需要知道以下信息：

* 源文本的编码（应始终为 UTF-8）
* 哪些字符被视为字母，哪些不是
* 哪些字母应被折叠为其他字母

你可以使用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 选项在每个表的基础上配置这些设置。`charset_table` 指定了一个数组，该数组将字母字符映射到其大小写折叠版本（或你偏好的其他字符）。不在数组中的字符被视为非字母字符，在该表中索引或搜索时会被视为单词分隔符。

默认字符集是 `non_cont`，它涵盖了[大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

你还可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

文本 `RED TUBE 5" LONG` 将被索引为 `COLOR TUBE 5 INCH LONG`，而 `PLANK 2" x 4"` 将被索引为 `PLANK 2 INCH x 4 INCH`。这些规则按指定的顺序应用。这些规则也适用于查询，因此搜索 `BLUE TUBE` 实际上会搜索 `COLOR TUBE`。

你可以在[这里](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)了解更多关于 `regexp_filter` 的信息。

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

`charset_table` 指定了一个数组，该数组将字母字符映射为其小写版本（或您想要的其他字符）。默认的字符集是 `non_cont`，包括了大多数使用[非连续](https://en.wikipedia.org/wiki/Scriptio_continua)字符的语言。

`charset_table` 是 Manticore 分词过程的主力工具，它从文档文本或查询文本中提取关键词。它控制哪些字符被接受为有效字符，以及它们应如何转换（例如是否需要忽略大小写）。

默认情况下，每个字符都映射为 0，这意味着它不会被视为有效的关键词，而是作为分隔符处理。只要一个字符在表中被提及，它就会被映射到另一个字符（通常是它本身或小写字母），并被视为有效的关键词部分。

`charset_table` 使用逗号分隔的映射列表来声明字符为有效字符或将其映射到其他字符。语法快捷方式允许一次性映射多个字符范围：

- 单字符映射：`A->a`。将源字符 'A' 声明为关键字中允许的字符，并将其映射为目标字符 'a'（但不声明 'a' 为允许字符）。
- 范围映射：`A..Z->a..z`。声明源范围内的所有字符为允许字符，并将其映射到目标范围。不会声明目标范围为允许字符。检查两者范围的长度是否一致。
- 单个字符声明：`a`。声明字符为允许字符，并将其映射到自身。相当于 `a->a` 单字符映射。
- 范围声明：`a..z`。声明范围内的所有字符为允许字符，并将它们映射到自身。相当于 `a..z->a..z` 范围映射。
- 交错范围映射：`A..Z/2`。将每对字符映射到第二个字符。例如，`A..Z/2` 等效于 `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。这个映射快捷方式对于 Unicode 区块中大写和小写字母交替出现的情况非常有用。

对于从 0 到 32 的字符代码，以及 127 到 8 位 ASCII 和 Unicode 字符，Manticore 始终将它们视为分隔符。为了避免配置文件的编码问题，8 位 ASCII 字符和 Unicode 字符必须以 `U+XXX` 形式指定，其中 `XXX` 是一个十六进制的代码点数。最小的可接受 Unicode 字符代码是 `U+0021`。

如果默认映射不足以满足您的需求，您可以通过再次指定字符映射来重新定义它们。例如，如果内置的 `non_cont` 数组包含字符 `Ä` 和 `ä` 并将它们都映射到 ASCII 字符 `a`，您可以通过添加它们的 Unicode 代码点来重新定义这些字符，如下所示：

```
charset_table = non_cont,U+00E4,U+00C4
```

用于区分大小写的搜索，或者：

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

除了字符和映射定义之外，还有几个内置的别名可以使用。当前的别名有：

- `chinese` - 支持中文字符集
- `cjk` - 支持所有中日韩字符集（中文、日文、韩文）
- `cont` - 支持所有使用连续文字的语言
- `english` - 支持英文字符集
- `japanese` - 支持日文字符集
- `korean` - 支持韩文字符集
- `non_cont` (`non_cjk`) - 支持所有不使用连续文字的语言
- `russian` - 支持俄文字符集
- `thai` - 支持泰文字符集

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

如果您想在搜索中支持不同的语言，定义所有语言的有效字符集和折叠规则可能是一项繁琐的任务。为了简化这一过程，我们提供了默认的字符集表 `non_cont` 和 `cont`，分别涵盖了不连续和连续（中文、日文、韩文、泰文）书写系统的语言。在大多数情况下，这些字符集应该能满足您的需求。

请注意，以下语言目前**不**受支持：

- 阿萨姆语（Assamese）
- 毕苏努普里亚语（Bishnupriya）
- 布希德语（Buhid）
- 加罗语（Garo）
- 瑶族语（Hmong）
- 霍语（Ho）
- 科米语（Komi）
- 大花苗语（Large Flowery Miao）
- 马巴语（Maba）
- 马依提利语（Maithili）
- 马拉地语（Marathi）
- 门德语（Mende）
- 穆鲁语（Mru）
- 美耶语（Myene）
- 恩甘拜语（Ngambay）
- 奥里亚语（Odia）
- 圣塔利语（Santali）
- 信德语（Sindhi）
- 西尔赫提语（Sylheti）

在[Unicode语言列表](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/)中列出的所有其他语言都默认受到支持。

<!-- example charset_table 3 -->
要同时处理连续和非连续语言，请在您的配置文件中设置如下选项（中文除外）：

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

如果您不需要支持连续书写语言，可以简单地排除 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 和 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 选项。有关这些选项的更多信息，请参阅相应的文档部分。

要将一个字符映射到多个字符或反之，您可以使用 [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 来帮助实现。

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->

混合字符列表。可选，默认为空。

混合字符既被索引为分隔符，也被索引为有效字符。例如，当 `&` 被定义为混合字符时，如果在索引文档中出现 `AT&T`，则会索引三个不同的关键字：`at&t`、`at` 和 `t`。

此外，混合字符可以影响索引，使得关键字的索引表现得好像没有输入混合字符。这种行为在指定 `blend_mode = trim_all` 时尤为明显。例如，短语 `some_thing` 将被索引为 `some`、`something` 和 `thing`，使用 `blend_mode = trim_all`。

使用混合字符时应谨慎，因为将一个字符定义为混合字符意味着它不再是分隔符。

- 因此，如果您将逗号放入 `blend_chars` 中并搜索 `dog,cat`，它会将其视为单个令牌 `dog,cat`。如果 `dog,cat` **未**被索引为 `dog,cat`，而仅留作 `dog cat`，那么它将不匹配。
- 因此，这种行为应通过 [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) 设置来控制。

通过将混合字符替换为空格获得的令牌位置会照常分配，而常规关键字将被索引，仿佛根本没有指定 `blend_chars`。一个额外的令牌如果同时包含混合字符和非混合字符，将放在起始位置。例如，如果 `AT&T company` 出现在文本字段的最开始，`at` 将被分配位置 1，`t` 位置 2，`company` 位置 3，同时 `AT&T` 也会被分配位置 1，与开头的常规关键字混合。因此，对 `AT&T` 或仅 `AT` 的查询将匹配该文档。短语查询 `"AT T"` 也会匹配，短语查询 `"AT&T company"` 也会匹配。

混合字符可能与查询语法中使用的特殊字符重叠，例如 `T-Mobile` 或 `@twitter`。在可能的情况下，查询解析器将处理混合字符。例如，如果 `hello @twitter` 在引号内（短语操作符），查询解析器将把 `@` 符号处理为混合字符。但是，如果 `@` 符号不在引号内，则该字符将被视为操作符。因此，建议对关键字进行转义。

混合字符可以重新映射，以便将多个不同的混合字符标准化为一个基础形式。这在为多个具有等效字形的备用 Unicode 代码点建立索引时非常有用。

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

<!-- example blend_mode -->混合令牌索引模式通过 `blend_mode` 指令启用。

默认情况下，混合字符和非混合字符混合的令牌会被完全索引。例如，当 `blend_chars` 中同时包含 @ 符号和感叹号时，字符串 `@dude!` 将被索引为两个令牌：`@dude!`（包含所有混合字符）和 `dude`（不包含任何）。因此，查询 `@dude` 将**不**匹配它。

`blend_mode` 为这种索引行为增加了灵活性。它接受一个用逗号分隔的选项列表，每个选项指定一种令牌索引变体。

如果指定了多个选项，则会索引相同令牌的多个变体。常规关键字（通过将混合字符替换为分隔符产生的令牌）总是被索引。

可用的选项包括：

- `trim_none` - 索引整个令牌
- `trim_head` - 修剪前导的混合字符，并索引结果令牌
- `trim_tail` - 修剪尾部的混合字符，并索引结果令牌
- `trim_both` - 修剪前导和尾部的混合字符，并索引结果令牌
- `trim_all` - 修剪前导、尾部和中间的混合字符，并索引结果令牌
- `skip_pure` - 如果令牌完全由混合字符组成，则不进行索引

使用 `blend_mode` 结合上述示例字符串 `@dude!`，设置 `blend_mode = trim_head, trim_tail` 将产生两个索引令牌：`@dude` 和 `dude!`。使用 `trim_both` 将没有效果，因为修剪两个混合字符的结果是 `dude`，该令牌已作为常规关键字索引。索引 `@U.S.A.` 时，使用 `trim_both`（并假设点也是混合的）将导致 `U.S.A` 被索引。最后，`skip_pure` 使您能够忽略仅由混合字符组成的序列。例如，`one @@@ two` 将被索引为 `one two`，并将其视为短语匹配。默认情况下，并非如此，因为完全混合的令牌被索引并抵消了第二个关键字的位置。

默认行为是索引整个令牌，相当于 `blend_mode = trim_none`。

请注意，使用混合模式会限制您的搜索，即使使用默认模式 `trim_none`，如果您假设 `.` 是一个混合字符：

- `.dog.` 在索引过程中将变为 `.dog. dog`
- 您将无法通过 `dog.` 找到它。

使用更多模式会增加您的关键字匹配某些内容的机会。

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

`min_word_len` 是 Manticore 中的一个可选索引配置选项，用于指定索引中单词的最小长度。默认值为 1，这意味着所有内容都被索引。

只有那些长度不小于该最小值的单词才会被索引。例如，如果 `min_word_len` 设置为 4，则单词 'the' 不会被索引，但 'they' 将被索引。

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

N-gram 为无分隔文本的连续脚本语言提供基本支持。使用连续脚本语言进行搜索时的问题在于缺乏清晰的单词分隔符。在某些情况下，您可能不希望使用基于词典的分割，例如 [针对中文的分割](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些情况下，n-gram 分割可能也能很好地工作。

当启用此功能时，这些语言（或在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 中定义的任何其他字符）的流会被索引为 N-gram。例如，如果输入文本为 "ABCDEF"（其中 A 到 F 代表某些语言字符），并且 ngram_len 为 1，则它将被索引为 "A B C D E F"。目前仅支持 ngram_len=1。只有在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 表中列出的字符会这样被拆分；其他字符则不会受到影响。

请注意，如果搜索查询是分割的，即在单个单词之间有分隔符，则将单词用引号包裹并使用扩展模式将导致找到正确的匹配，即使文本 **未** 被分割。例如，假设原始查询为 `BC DEF`。在应用程序端将其包裹在引号中后，应变为 `"BC" "DEF"`（*带* 引号）。该查询将被传递给 Manticore，并在内部也会拆分为 1-grams，结果为 `"B C" "D E F"` 查询，仍然带有引号，表示短语匹配操作符。即使文本中没有分隔符，它也会匹配该文本。

即使搜索查询未被分割，Manticore 也应产生良好的结果，这要归功于基于短语的排名：它会将更接近的短语匹配（在 N-gram 词的情况下，这可能意味着更接近的多字符词匹配）拉到顶部。

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

此列表与 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 一起使用，定义了其序列将进行 N-gram 提取的字符。由其他字符组成的单词将不会受到 N-gram 索引功能的影响。值的格式与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同。N-gram 字符不能出现在 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中。

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
您还可以使用我们默认 N-gram 表的别名，如示例所示。这在大多数情况下应该是足够的。

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

忽略字符列表。可选，默认值为空。

在某些情况下，某些字符（例如软连字符标记 U+00AD）不仅应被视为分隔符，而应完全忽略。例如，如果字符 '-' 仅不在 charset_table 中，则文本 "abc-def" 将被索引为 "abc" 和 "def" 关键字。相反，如果 '-' 被添加到 ignore_chars 列表中，则相同的文本将被索引为单个 "abcdef" 关键字。

语法与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同，但只允许声明字符，而不允许映射它们。此外，忽略的字符必须不在 charset_table 中。

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

大ram索引模式。可选，默认值为无。

大ram索引是一项加速短语搜索的功能。在索引时，它将所有或某些相邻单词对的文档列表存储到索引中。这样的列表在搜索时可以显著加速短语或子短语匹配。

`bigram_index` 控制特定单词对的选择。已知模式如下：

- `all`：索引每一个单词对
- `first_freq`：仅索引第一个单词在频繁单词列表中的单词对（参见 [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)）。例如，使用 `bigram_freq_words = the, in, i, a`，索引文本 "alone in the dark" 将导致 "in the" 和 "the dark" 对被存储为大ram，因为它们以频繁关键字（分别是 "in" 或 "the"）开头，但 "alone in" 将 **不** 被索引，因为 "in" 是该对中的第二个单词。
- `both_freq`：仅索引两个单词都是频繁的单词对。继续使用相同的示例，在此模式下，索引 "alone in the dark" 只会存储 "in the"（从搜索的角度来看是最糟糕的）作为大ram，而不会存储其他单词对。

对于大多数用例，`both_freq` 将是最佳模式，但具体情况可能有所不同。

需要注意的是，`bigram_index` 仅在标记化级别工作，并不考虑诸如 `morphology`、`wordforms` 或 `stopwords` 等转换。这意味着它创建的令牌非常简单，这使得短语搜索更精确和严格。虽然这可以提高短语匹配的准确性，但也使系统对单词的不同形式或单词出现方式的变化的识别能力降低。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
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

被认为是“频繁”的关键字列表，用于索引大ram。可选，默认值为空。

某些大ram索引模式（参见 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)）需要定义一个频繁关键字的列表。这些**不能**与停用词混淆。停用词在索引和搜索时会被完全消除。频繁关键字仅被大ram用于决定是否索引当前的单词对。

`bigram_freq_words` 让你定义这样关键字的列表。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
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

用于关键字字典的类型由两种已知值之一确定，'crc' 或 'keywords'。这是可选的，默认值为 'keywords'。

使用关键字字典模式（dict=keywords）可以显著减少索引负担，并允许对大量集合进行子字符串搜索。此模式可用于普通表和实时表。

CRC 字典不在索引中存储原始关键字文本。相反，它们在搜索和索引过程中用控制总和值（使用 FNV64 计算）替换关键字。该值在索引内被内部使用。这种方法有两个缺点：

- 首先，存在不同关键字对之间控制总和冲突的风险。随着索引中唯一关键字数量的增加，这种风险也随之增加。尽管如此，这一问题的影响很小，因为在包含 10 亿条目字典中，单个 FNV64 冲突的概率大约是 1/16，即 6.25%。考虑到典型的口语人类语言的词形在 1 到 1000 万之间，大多数字典的关键字数量远低于十亿。
- 其次，更重要的是，使用控制总和进行子字符串搜索并不简单。Manticore 通过将所有可能的子字符串预先索引为单独的关键字来解决此问题（参见 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 和 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 指令）。这种方法还具有以最快的方式匹配子字符串的额外优势。然而，预索引所有子字符串会显著增加索引大小（通常增加 3-10 倍或更多），并随后影响索引时间，使大索引上的子字符串搜索变得不切实际。

关键字字典解决了这两个问题。它在索引中存储关键字，并在搜索时进行通配符扩展。例如，对 `test*` 前缀的搜索可以根据字典的内容在内部扩展为 'test|tests|testing' 查询。这个扩展过程对应用程序是完全不可见的，唯一的例外是所有匹配关键字的每个关键字统计信息现在也会被报告。

对于子字符串（中缀）搜索，可以使用扩展通配符。特殊字符如 `?` 和 `%` 与子字符串（中缀）搜索兼容（例如，`t?st*`、`run%`、`*abc*`）。请注意，[通配符运算符](../../Searching/Full_text_matching/Operators.md#通配符操作符) 和 [REGEX](../../Searching/Full_text_matching/Operators.md#正则表达式操作符) 仅在 `dict=keywords` 时有效。

使用关键字字典进行索引的速度大约比常规非子字符串索引慢 1.1 倍到 1.3 倍 - 但显著快于子字符串索引（无论是前缀还是中缀）。索引大小应仅比标准非子字符串表略大，总差异为 1..10% 之间。常规关键字搜索所需的时间在所有三种索引类型（CRC 非子字符串、CRC 子字符串、关键字）之间应该几乎相同或相同。子字符串搜索时间可能会根据实际匹配给定子字符串的关键字数量（即搜索术语扩展成多少关键字）显著波动。匹配关键字的最大数量受 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) 指令的限制。

总之，关键字和 CRC 字典为子字符串搜索提供了两种不同的权衡选择。您可以选择牺牲索引时间和索引大小以实现最快的最坏情况搜索（CRC 字典），或者在对索引时间的影响最小化的情况下牺牲最坏情况搜索时间，当前缀扩展为大量关键字时（关键字字典）。

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

嵌入的异常、词形或停用词文件大小限制。可选，默认值为 16K。

创建表时，上述文件可以与表一起保存为外部文件或直接嵌入到表中。大小低于 `embedded_limit` 的文件将存储在表中。对于更大的文件，仅存储文件名。这还简化了将表文件移动到其他计算机的过程；您只需复制一个文件即可。

对于较小的文件，这种嵌入减少了表所依赖的外部文件数量，并有助于维护。但是，同时将一个 100 MB 的词形字典嵌入到一个小的增量表中没有意义。因此，需要设置一个大小阈值，`embedded_limit` 就是这个阈值。

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

全局（集群范围）关键字 IDF 文件的路径。可选，默认值为空（使用本地 IDF）。

在多表集群中，按关键字频率在不同表之间很可能存在差异。这意味着当排名函数使用基于 TF-IDF 的值时，如 BM25 系列因素，结果可能会根据它们所在的集群节点而稍微有所不同。

解决该问题的最简单方法是创建并使用一个全局频率字典，简称为全局 IDF 文件。此指令让您指定该文件的位置。建议（但不要求）使用 .idf 扩展名。当为给定表指定 IDF 文件并且 [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为 1 时，引擎将使用来自 global_idf 文件的关键字频率和集合文档计数，而不仅仅是来自本地表。这样，IDF 和依赖于它们的值在集群中将保持一致。

IDF 文件可以在多个表之间共享。即使多个表引用该文件，`searchd` 只会加载 IDF 文件的单个副本。如果 IDF 文件的内容发生变化，可以使用 SIGHUP 加载新内容。

您可以使用 [indextool](../../Miscellaneous_tools.md#indextool) 工具构建 .idf 文件，首先使用 `--dumpdict dict.txt --stats` 开关导出字典，然后使用 `--buildidf` 转换为 .idf 格式，最后使用 `--mergeidf` 合并集群中的所有 .idf 文件。

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

无命中词列表。可选，允许的值为 'all' 或列表文件名。

默认情况下，Manticore 的全文索引不仅存储每个给定关键字的匹配文档列表，还存储其在文档中的位置列表（称为命中列表）。命中列表使得短语、相似性、严格顺序和其他高级搜索类型成为可能，以及短语的相似性排名。然而，特定频繁关键字的命中列表（由于某种原因不能被停止的频繁关键字）可能会变得非常庞大，从而在查询时处理速度缓慢。此外，在某些情况下，我们可能只关心布尔关键字匹配，而从来不需要基于位置的搜索操作（如短语匹配）或短语排名。

`hitless_words` 让您创建索引，这些索引要么根本没有位置信息（命中列表），要么跳过特定关键字的位置信息。

无命中索引通常会比相应的常规全文索引占用更少的空间（预计约为 1.5 倍）。索引和搜索的速度都应该更快，但代价是缺少位置查询和排名支持。

如果在位置查询中使用（例如短语查询），则无命中词将从查询中删除，并作为没有位置的操作数使用。例如，如果 "hello" 和 "world" 是无命中词，而 "simon" 和 "says" 不是无命中词，则短语查询 `"simon says hello world"` 将被转换为 `("simon says" & hello & world)`，匹配文档中的 "hello" 和 "world" 以及作为确切短语的 "simon says"。

仅包含无命中词的定位查询将导致空短语节点，因此整个查询将返回空结果并发出警告。如果整个字典是无命中词（使用 `all`），则只能在相应的索引上使用布尔匹配。



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

启用计算和存储字段长度（每个文档和每个索引的平均值）到全文索引中。可选，默认值为 0（不计算和存储）。

当 `index_field_lengths` 设置为 1 时，Manticore 将：

- 为每个全文字段创建相应的长度属性，属性名称相同，但以 `__len` 后缀结尾
- 计算每个文档的字段长度（以关键字计），并存储到相应的属性中
- 计算每个索引的平均值。长度属性将具有特殊的 TOKENCOUNT 类型，但其值实际上是常规的 32 位整数，且其值通常是可访问的。

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A()) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F()) 函数在表达式排名器中基于这些长度，并需要启用 `index_field_lengths`。历史上，Manticore 使用了一种简化的、删减版本的 BM25，与完整函数不同，它并不考虑文档长度。此外，还支持 BM25 的完整变体及其扩展版本，称为 BM25F，它们分别需要每个文档的长度和每个字段的长度。因此，增加了这个指令。

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

索引时间的令牌过滤器，用于全文索引。可选，默认值为空。

`index_token_filter` 指令指定一个可选的索引时间令牌过滤器，用于全文索引。此指令用于创建一个自定义分词器，根据自定义规则生成令牌。过滤器由索引器在将源数据索引到普通表中或在处理 `INSERT` 或 `REPLACE` 语句的实时表时创建。插件使用格式 `library name:plugin name:optional string of settings` 定义。例如，`my_lib.so:custom_blend:chars=@#&`。

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
在超短（少于 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)）关键词上的位置增量。可选，允许的值为 0 和 1，默认值为 1。

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
phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis
```

<!-- example phrase_boundary -->

短语边界字符列表。可选，默认值为空。

该列表控制哪些字符将被视为短语边界，以调整单词位置并通过接近搜索启用短语级搜索仿真。语法类似于 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)，但不允许进行映射，并且边界字符不得与其他字符重叠。

在短语边界处，将向当前单词位置添加额外的单词位置增量（由 [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 指定）。这使得通过接近查询进行短语级搜索成为可能：不同短语中的单词之间将确保距离超过 `phrase_boundary_step`；因此，在该距离内的接近搜索将等同于短语级搜索。

短语边界条件将仅在该字符后跟一个分隔符时引发；这旨在避免像 S.T.A.L.K.E.R 或 URL 这样的缩写被视为多个短语。

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

短语边界单词位置增量。可选，默认值为 0。

在短语边界上，当前单词位置将额外增加该数值。

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

用于过滤字段和查询的正则表达式（regexps）。该指令是可选的，多值的，默认值为空正则表达式列表。Manticore Search 使用的正则表达式引擎是 Google 的 RE2，以其速度和安全性而闻名。有关 RE2 支持的语法的详细信息，请访问 [RE2 语法指南](https://github.com/google/re2/wiki/Syntax)。

在某些应用中，例如产品搜索，可能会有许多方式来引用产品、型号或属性。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指的是同一产品。另一个例子可能是表达笔记本电脑屏幕尺寸的不同方式，例如 `13-inch`、`13 inch`、`13"` 或 `13in`。

正则表达式提供了一种机制，可以指定专门处理此类情况的规则。在第一个示例中，您可能使用一个词形文件来处理少量的 iPhone 型号，但在第二个示例中，最好指定规则来将 "13-inch" 和 "13in" 标准化为相同的内容。

在 `regexp_filter` 中列出的正则表达式将按顺序应用，尽早处理，在任何其他处理（包括 [例外](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)）之前，甚至在分词之前。也就是说，正则表达式在索引时应用于原始源字段，在搜索时应用于原始搜索查询文本。

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
