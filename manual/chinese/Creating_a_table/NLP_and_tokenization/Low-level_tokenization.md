# 低级分词

当文本被索引到 Manticore 中时，它会被拆分成词，并进行大小写折叠，这样像 "Abc"、"ABC" 和 "abc" 这样的词会被视为同一个词。

要正确执行这些操作，Manticore 必须知道：
* 源文本的编码（应始终为 UTF-8）
* 哪些字符被视为字母，哪些不是
* 哪些字母应该折叠成其他字母

你可以使用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 选项按表进行配置。这会指定一个数组，将字母字符映射到它们的大小写折叠形式（或你希望的其他字符）。数组中未出现的字符会被视为非字母，并在该表的索引或搜索过程中被当作分隔符。

默认字符集是 `non_cont`，它包含 [大多数语言](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)。

你也可以定义文本模式替换规则。例如，使用以下规则：

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

文本 `RED TUBE 5" LONG` 会被索引为 `COLOR TUBE 5 INCH LONG`，而 `PLANK 2" x 4"` 会被索引为 `PLANK 2 INCH x 4 INCH`。这些规则按指定顺序应用。规则也会应用于查询，因此搜索 `BLUE TUBE` 实际上搜索的是 `COLOR TUBE`。

你可以在这里了解更多关于 [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) 的信息。

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

# override the default transliteration to preserve German umlauts and sharp s;
# uppercase variants are mapped to lowercase
charset_table = non_cont, german
```

<!-- example charset_table -->
`charset_table` 指定一个数组，将字母字符映射到它们的大小写折叠形式（或你希望的其他字符）。默认字符集是 `non_cont`，它包含使用 [非连续](https://en.wikipedia.org/wiki/Scriptio_continua) 脚本的大多数语言。

`charset_table` 是 Manticore 分词流程中的核心组件，它负责从文档文本或查询文本中提取关键词。它控制哪些字符会被接受为有效字符，以及它们应如何转换（例如是否去除大小写）。

默认情况下，每个字符都会映射到 0，这意味着它不被视为有效关键词，而是作为分隔符处理。一旦某个字符出现在表中，它就会被映射到另一个字符（最常见的是映射到它自身或一个小写字母），并被视为有效关键词的一部分。

charset_table 使用以逗号分隔的映射列表来声明字符为有效字符，或将其映射到其他字符。也提供了用于一次映射一段字符范围的简写语法：

* 单字符映射：`A->a`。将源字符 'A' 声明为关键词中允许出现的字符，并将其映射到目标字符 'a'（但不会将 'a' 声明为允许字符）。
* 范围映射：`A..Z->a..z`。将源范围内的所有字符声明为允许字符，并将它们映射到目标范围。不会将目标范围声明为允许字符。会检查两个范围的长度。
* 单独字符映射：`a`。将某个字符声明为允许字符，并将其映射到它自身。等价于单字符映射 `a->a`。
* 单独范围映射：`a..z`。将该范围内的所有字符声明为允许字符，并将它们映射到它们自身。等价于范围映射 `a..z->a..z`。
* 棋盘式范围映射：`A..Z/2`。将每两个字符映射到第二个字符。例如，`A..Z/2` 等价于 `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`。这种映射简写对大小写字母交错排列的 Unicode 块很有帮助。

对于编码从 0 到 32 的字符，以及 127 到 8 位 ASCII 和 Unicode 字符范围内的字符，Manticore 一律将它们视为分隔符。为了避免配置文件编码问题，8 位 ASCII 字符和 Unicode 字符必须以 `U+XXX` 形式指定，其中 `XXX` 是十六进制码点。可接受的最小 Unicode 字符编码是 `U+0021`。

如果默认映射不能满足你的需求，你可以通过再次指定这些字符来重新定义字符映射。例如，如果内置的 `non_cont` 数组包含字符 `Ä` 和 `ä`，并把它们都映射到 ASCII 字符 `a`，你可以像这样通过添加它们的 Unicode 码点来重新定义这些字符：

```
charset_table = non_cont,U+00E4,U+00C4
```

用于区分大小写搜索，或者

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'');
```
<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- intro -->
##### Rust：

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
除了字符和映射的定义之外，还可以使用若干内置别名。当前别名如下：
* `chinese`
* `cjk`
* `cont`
* `english`
* `german`
* `japanese`
* `korean`
* `non_cont` (`non_cjk`)
* `russian`
* `thai`

`german` 别名会保留 `ä`、`ö`、`ü` 和 `ß`，而不是把它们映射为 ASCII 字符，并会将其大写变体映射为小写，包括把 `ẞ` 映射为 `ß`。如上所示，将它追加在 `non_cont` 之后，以保留其余默认字符映射。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- intro -->
##### Rust：

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

如果你需要在搜索中支持多种语言，逐一为所有语言定义有效字符集和折叠规则会很繁琐。我们通过提供默认的 charset 表 `non_cont` 和 `cont` 简化了这件事，分别覆盖使用非连续脚本和连续脚本（中文、日文、韩文、泰文）的语言。在大多数情况下，这些字符集已经足够。

请注意，目前以下语言**不**受支持：
* 阿萨姆语
* 比什努普里亚语
* 布希德语
* 加洛语
* 苗语
* 霍语
* 科米语
* 大花苗语
* 马巴语
* 迈蒂利语
* 马拉地语
* 门德语
* Mru 语
* Myene 语
* 恩甘贝语
* 奥里亚语
* 桑塔利语
* 信德语
* Sylheti 语

Unicode 语言列表中列出的
其他所有语言默认都受支持。

<!-- example charset_table 3 -->
要同时处理 cont 和 non-cont 语言，请按如下方式在配置文件中设置选项（中文有一个[例外](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)）：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### Rust：

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

如果你不需要支持连续脚本语言，可以直接去掉 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 和 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)。
选项。有关这些选项的更多信息，请参阅对应文档章节。

要实现一个字符与多个字符之间的双向映射，`regexp_filter` 会很有帮助。

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
混合字符列表。可选，默认为空。

混合字符会同时作为分隔符和有效字符被索引。例如，当 `&` 被定义为混合字符，并且 `AT&T` 出现在已索引文档中时，会索引出三个不同的关键词：`at&t`、`at` 和 `t`。

此外，混合字符还会影响索引，使关键词被索引时仿佛这些混合字符根本没有输入过。这个行为在指定 `blend_mode = trim_all` 时尤为明显。例如，短语 `some_thing` 在 `blend_mode = trim_all` 下会被索引为 `some`、`something` 和 `thing`。

使用混合字符时要小心，因为将某个字符定义为混合字符，就意味着它不再是分隔符。
* 因此，如果你把逗号放进 `blend_chars`，然后搜索 `dog,cat`，它会把它当作单个 token `dog,cat`。如果 `dog,cat` **没有**被索引为 `dog,cat`，而只是保留为 `dog cat`，那么它就无法匹配。
* 因而，这种行为应通过 [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) 设置来控制。

通过用空白替换混合字符得到的 token，其位置会按常规分配，普通关键词会像完全没有指定 `blend_chars` 一样被索引。一个额外的 token 会把混合字符和非混合字符组合起来，并放在起始位置。例如，如果 `AT&T company` 出现在文本字段的最开头，`at` 的位置会是 1，`t` 的位置会是 2，`company` 的位置会是 3，同时 `AT&T` 也会被放在位置 1，并与开头的普通关键词合并。因此，查询 `AT&T` 或仅 `AT` 都会匹配该文档。短语查询 `"AT T"` 也会匹配，短语查询 `"AT&T company"` 同样会匹配。

混合字符可能与查询语法中使用的特殊字符重叠，例如 `T-Mobile` 或 `@twitter`。在可能的情况下，查询解析器会把混合字符按混合字符处理。例如，如果 `hello @twitter` 位于引号内（短语操作符），查询解析器会把 `@` 符号当作混合字符处理。不过，如果 `@` 符号不在引号内，这个字符就会被当作操作符处理。因此，建议对关键词进行转义。

混合字符可以重新映射，从而把多个不同的混合字符归一化为一个基础形式。这在索引多个具有等价字形的 Unicode 码位变体时很有用。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### Rust：

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
通过 `blend_mode` 指令启用混合 token 索引模式。

默认情况下，混合字符和非混合字符混合在一起的 token 会整体被索引。例如，当 `blend_chars` 中同时包含 at 符号和感叹号时，字符串 `@dude!` 会被索引成两个 token：`@dude!`（包含所有混合字符）和 `dude`（不包含任何混合字符）。因此，查询 `@dude` **不会**匹配它。

`blend_mode` 为这种索引行为增加了灵活性。它接受一个逗号分隔的选项列表，每个选项都指定一种 token 索引变体。

如果指定了多个选项，同一个 token 会索引出多个变体。普通关键词（即把混合字符替换为分隔符后得到的 token）始终会被索引。

可用选项如下：

* `trim_none` - 索引整个 token
* `trim_head` - 去掉开头的混合字符，并索引结果 token
* `trim_tail` - 去掉结尾的混合字符，并索引结果 token
* `trim_both`- 去掉开头和结尾的混合字符，并索引结果 token
* `trim_all` - 去掉开头、结尾和中间的混合字符，并索引结果 token
* `skip_pure` - 如果 token 纯粹由混合字符组成，则不索引它

使用上面的 `@dude!` 示例字符串，设置 `blend_mode = trim_head, trim_tail` 会得到两个被索引的 token：`@dude` 和 `dude!`。使用 `trim_both` 不会有任何效果，因为去掉两端的混合字符后会得到 `dude`，而它已经作为普通关键词被索引了。使用 `trim_both` 索引 `@U.S.A.`（并假设点号也是混合字符）时，会得到 `U.S.A` 被索引。最后，`skip_pure` 允许你忽略完全由混合字符组成的序列。例如，`one @@@ two` 会被索引为 `one two`，并可按短语匹配。这在默认情况下并不会发生，因为完全混合的 token 会被索引，并将第二个关键词的位置偏移。

默认行为是索引整个 token，这等同于 `blend_mode = trim_none`。

请注意，使用混合模式会限制你的搜索范围，即使是默认模式 `trim_none`，如果你假定 `.` 是混合字符也是如此：
* `.dog.` 在索引时会变成 `.dog. dog`
* 而且你无法通过 `dog.` 找到它。

使用更多模式会提高你的关键词匹配到某些内容的概率。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```
<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### Rust：

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

min_word_len 是 Manticore 中一个可选的索引配置项，用于指定可被索引的最小词长。默认值是 1，这意味着所有内容都会被索引。

只有不短于该最小长度的词才会被索引。例如，如果 min_word_len 为 4，那么 'the' 不会被索引，而 'they' 会被索引。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### Rust：

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
用于 N-gram 索引的 N-gram 长度。可选，默认值为 0（禁用 n-gram 索引）。已知值为 0 和 1。

N-gram 为未分词文本中的连续脚本语言提供基础支持。使用连续脚本语言进行搜索的问题在于单词之间没有清晰的分隔符。在某些情况下，你可能不想使用基于词典的分词，例如[中文所使用的方式](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)。在这些场景下，n-gram 分词也可能效果很好。

启用此功能后，这类语言的文本流（或在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 中定义的其他字符）会按 N-gram 方式索引。例如，如果输入文本是 "ABCDEF"（其中 A 到 F 代表某种语言字符），并且 ngram_len 为 1，那么它会像 "A B C D E F" 一样被索引。目前只支持 ngram_len=1。只有在 [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) 表中列出的字符才会以这种方式拆分；其他字符不受影响。

请注意，如果搜索查询已经分词，也就是单词之间有分隔符，那么在应用侧把单词加上引号并使用扩展模式，即使文本本身**没有**分词，也能得到正确匹配。例如，假设原始查询是 `BC DEF`。在应用侧加上引号后，它应该看起来像 `"BC" "DEF"`（*带*引号）。这个查询会传给 Manticore，并在内部也拆成 1-gram，结果变成 `"B C" "D E F"` 查询，仍然带着作为短语匹配操作符的引号。这样即使文本中没有分隔符，也能匹配到文本。

即使搜索查询没有分词，借助短语相关性排序，Manticore 仍应能产生良好结果：它会把更接近短语匹配的结果（在 N-gram 词的情况下，这可能意味着更接近的多字符词匹配）排到前面。

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
##### java：

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

### ngram_chars

```ini
ngram_chars = cont

ngram_chars = cont, U+3000..U+2FA1F
```

<!-- example ngram_chars -->
N-gram 字符列表。可选，默认为空。

要与 [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) 配合使用，该列表定义了哪些字符的连续序列会被提取为 N-gram。由其他字符组成的词不会受到 N-gram 索引功能的影响。其值格式与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同。N-gram 字符不能出现在 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中。

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
你也可以像示例中那样为我们的默认 N-gram 表使用别名。在大多数情况下，这已经足够。

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
被忽略的字符列表。可选，默认为空。

这在某些字符（例如软连字符 U+00AD）不应仅仅被视为分隔符，而应被完全忽略时很有用。例如，如果 `-` 只是不在 charset_table 中，那么文本 "abc-def" 会被索引为 "abc" 和 "def" 两个关键词。相反，如果把 `-` 加入 ignore_chars 列表，同样的文本会被索引为单个 "abcdef" 关键词。

语法与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 相同，但这里只允许声明字符，不允许映射它们。另外，被忽略的字符不能出现在 charset_table 中。

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
bigram_index = {none|all|first_freq|both_freq|second_numeric|second_has_digit}
```

<!-- example bigram_index -->
双字索引模式。可选，默认是 none。

双字索引是一种加速短语搜索的功能。在索引时，它会把所有或部分相邻词对的文档列表存入索引。之后在搜索时，这个列表可被用来显著加速短语或子短语匹配。

`bigram_index` 控制具体词对的选择。已知模式如下：

* `all`，索引每一个词对
* `first_freq`，仅索引那些*第一个*词在高频词列表中的词对（见 [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)）。例如，设置 `bigram_freq_words = the, in, i, a` 时，索引 "alone in the dark" 文本会把 "in the" 和 "the dark" 这两个词对作为 bigram 存储，因为它们都以高频关键词（分别是 "in" 或 "the"）开头，但 "alone in" **不会**被索引，因为 "in" 在该词对中是*第二*个词。
* `both_freq`，仅索引两个词都属于高频词的词对。继续沿用同一个例子，在这种模式下索引 "alone in the dark" 时，只会把 "in the"（从搜索角度看最差的那个）作为 bigram 存储，而其他词对都不会被索引。
* `second_numeric`，仅索引*第二个* token 只包含 ASCII 数字的词对。例如，`xt 806` 会匹配，但 `xt rt9600` 和 `xt v2` 不会。
* `second_has_digit`，仅索引*第二个* token 至少包含一个 ASCII 数字的词对。例如，`xt 806`、`xt rt9600` 和 `xt v2` 会匹配，但 `xt abc` 不会。

对于大多数用例，`both_freq` 是最佳模式，但实际效果可能因场景而异。

需要注意的是，`bigram_index` 只在分词层面起作用，不会考虑 `morphology`、`wordforms` 或 `stopwords` 等转换。这意味着它创建的 token 非常直接，这会让短语搜索更精确、更严格。虽然这可以提高短语匹配的准确性，但也会降低系统识别词形变化或词语不同写法的能力。

与数字相关的模式只使用 ASCII 数字（`0-9`）。它们不会把 `+`、`-` 或 Unicode 数字视为数字。检查还会使用当前分词器路径生成的 token 文本，不会做额外的标点归一化。

使用 [bigram_delimiter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_delimiter) 可以控制符合条件的 bigram 是作为内部带分隔符的 token 存储，还是作为像 `iphone17` 这样的粘连 token 存储，或者两种形式都存。

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

### bigram_delimiter

```ini
bigram_delimiter = {true|none|both}
```

<!-- example bigram_delimiter -->
双字 token 存储模式。可选，默认是 `true`。

`bigram_delimiter` 控制由 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index) 选出的符合条件的 bigram 存储哪种 token 形式：

* `true`，只存储内部带分隔符的 bigram token。这是当前默认行为。
* `none`，只存储粘连 token 形式，例如 `iphone17`。
* `both`，同时存储内部带分隔符的形式和粘连形式。

搜索行为取决于所选模式：

* 使用 `true` 时，短语优化会把符合条件的短语词对改写为内部带分隔符的 token
* 使用 `none` 时，短语优化会把符合条件的短语词对改写为粘连 token，例如 `"iphone 17"` 会变成 `iphone17`
* 使用 `both` 时，短语优化会被跳过，短语查询仍保持普通短语查询，但因为粘连形式也被存储了，粘连 token 搜索仍然可以匹配

`bigram_delimiter` 只改变已存储 token 的形态。它并不决定哪些词对符合条件；这仍由 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index) 控制。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_index' => 'all',
            'bigram_delimiter' => 'none'
        ]);
```
<!-- intro -->
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_index = \'all\' bigram_delimiter = \'none\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_index = \'all\' bigram_delimiter = \'none\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_index = \'all\' bigram_delimiter = \'none\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'", true);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_index = all
  bigram_delimiter = none

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
在索引 bigram 时被视为“高频”的关键词列表。可选，默认为空。

某些 bigram 索引模式（见 [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)）需要一个高频关键词列表。这些词**不要**与停用词混淆。停用词会在索引和搜索时被完全移除。高频关键词只被 bigram 用来判断是否要索引当前词对。

`bigram_freq_words` 允许你定义这样一组关键词。

只有在 `first_freq` 和 `both_freq` 中才需要这个选项。

以下模式中它必须保持为空：

* `none`
* `all`
* `second_numeric`
* `second_has_digit`

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
dict = {keywords|keywords_32k|crc}
```

<!-- example dict -->
字典类型由三个已知值之一标识：`keywords`、`keywords_32k` 或 `crc`。该设置是可选的；`keywords` 是默认值。

`dict=keywords` 和 `dict=keywords_32k` 是词典。词典会在索引中存储原始关键词文本，并在搜索时执行通配符扩展。`dict=crc` 则存储关键词校验值。

`dict=keywords` 是默认的词典。

`dict=keywords_32k` 是一个可启用的词典，支持 32 KiB 关键词 token。它在普通表和 RT 表中都支持最长 32768 字节的规范化 token。超过该限制的 token 会被跳过并发出警告，而不会被作为截断词条索引。当启用了常规的精确、前缀或内联前缀设置时，它支持精确查找、前缀查找和内联查找。有关 42 字节常规 token 限制以及 32768 字节 `keywords_32k` 限制的详细信息，请参阅[词长限制](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md#Token-length-limit)。

`keywords_32k` 适用于较长的机器生成值，例如哈希、生成的 ID、消息标识符以及长的类邮箱 token。

以下功能目前还不支持 `dict=keywords_32k`：

* `CALL SUGGEST` 和 `CALL QSUGGEST` 不能在使用 `dict=keywords_32k` 的表上工作。
* Percolate 表不能使用 `dict=keywords_32k`。
* 摘要和高亮仍使用常规 token 限制。最长 42 字节的 token 可以被高亮；更长的 `keywords_32k` token 会在摘要/高亮处理中被跳过。
* `indextool --dumpdict` 目前还不能转储 `dict=keywords_32k` 词典。

CRC 词典不会在索引中存储原始关键词文本。相反，它们在搜索和索引过程中都会用一个控制和数值（使用 FNV64 计算）替换关键词。这个值在索引内部使用。此方法有两个缺点：
* 首先，不同关键词对之间存在控制和碰撞的风险。这个风险会随着索引中唯一关键词数量的增加而增长。不过，这一问题相对较小，因为在一个包含 10 亿条目的词典中，单次 FNV64 碰撞的概率大约是 1/16，也就是 6.25%。由于典型的人类自然语言只有 100 万到 1000 万种词形，大多数词典的关键词数量会远少于 10 亿。
* 其次，更关键的是，用控制和执行子串搜索并不直接。Manticore 通过预先将所有可能的子串索引为独立关键词来解决这个问题（见 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)、[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 指令）。这种方法还有一个额外优势，就是能以最快的方式匹配子串。然而，预先索引所有子串会显著增加索引大小（通常会增加 3 到 10 倍甚至更多），并进一步影响索引时间，使大索引上的子串搜索相当不实用。

词典方式解决了这两个问题。它把关键词存储在索引中，并在搜索时执行通配符扩展。例如，搜索前缀 `test*` 时，系统可能会基于词典内容在内部扩展为 `test|tests|testing` 查询。这个扩展过程对应用完全不可见，唯一的例外是：所有匹配关键词的独立每关键词统计信息现在也会被报告。

对于子串（内联）搜索，可以使用扩展通配符。`?` 和 `%` 等特殊字符兼容子串（内联）搜索（例如，`t?st*`、`run%`、`*abc*`）。请注意，[通配符操作符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)适用于 `dict=keywords` 和 `dict=keywords_32k`，而 [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) 操作符仅适用于 `dict=keywords`。

对于正常大小的 token，使用词典索引大约比常规的非子串索引慢 1.1 倍到 1.3 倍，但仍明显快于子串索引（无论是前缀还是内联）。索引大小应只比标准的非子串表略大，总差异大约为 1..10%。常规关键词搜索的耗时在这三种索引类型之间应当几乎相同或完全一致（CRC 非子串、CRC 子串、词典）。子串搜索时间会随实际匹配到该子串的关键词数量而明显波动（也就是搜索词会扩展成多少关键词）。可匹配关键词的最大数量受 [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) 指令限制。

总之，词典和 CRC 词典为子串搜索提供了两种不同的取舍。你可以选择牺牲索引时间和索引大小，以获得最快的最坏情况搜索性能（CRC 词典）；或者尽量减少对索引时间的影响，但在前缀扩展到大量关键词时牺牲最坏情况搜索时间（词典）。

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
嵌入式 exceptions、wordforms 或 stop words 文件大小限制。可选，默认是 16K。

创建表时，上面提到的这些文件可以与表一起保存到外部，也可以直接嵌入表中。大小低于 `embedded_limit` 的文件会存储到表里。对于更大的文件，只会存储文件名。这样也简化了将表文件迁移到其他机器的过程；你可能只需要复制一个文件。

对于较小的文件，这种嵌入方式减少了表所依赖的外部文件数量，有助于维护。但与此同时，把一个 100 MB 的 wordforms 词典嵌入一个很小的 delta 表里显然没有意义。所以需要一个大小阈值，而 `embedded_limit` 就是这个阈值。

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

在多表集群中，不同表之间每个关键词的频率很可能不同。这意味着当排序函数使用基于 TF-IDF 的值时，例如 BM25 系列因子，结果的排序可能会因所在集群节点不同而略有差异。

修复这个问题最简单的方法是创建并使用一个全局频率字典，简称全局 IDF 文件。该指令允许你指定该文件的位置。建议使用 `.idf` 扩展名，但不是强制要求。当给定表指定了 IDF 文件，并且 [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 设置为 1 时，引擎将使用 global_idf 文件中的关键词频率和集合文档数，而不是仅使用本地表的数据。这样，IDF 以及依赖它们的值就能在整个集群中保持一致。

IDF 文件可以在多个表之间共享。即使有很多表引用同一个 IDF 文件，`searchd` 也只会加载该文件的一份副本。如果 IDF 文件内容发生变化，可以通过 SIGHUP 加载新内容。

你可以使用 [indextool](../../Miscellaneous_tools.md#indextool) 工具生成 .idf 文件：先用 `--dumpdict dict.txt --stats` 开关转储词典，再用 `--buildidf` 将其转换为 .idf 格式，然后用 `--mergeidf` 合并集群中的所有 .idf 文件。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```
<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### Rust：

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
无位置词列表。可选，允许的值是 'all' 或一个列表文件名。

默认情况下，Manticore 全文索引不仅会为每个给定关键词存储匹配文档列表，还会存储其在文档中的位置列表（称为 hitlist）。Hitlist 支持短语、邻近、严格顺序以及其他高级搜索类型，同时也支持短语邻近排序。不过，某些高频关键词的 hitlist（这些词由于某种原因即使很常见也不能被停用）可能会非常大，因此在查询时处理起来会很慢。另外，在某些场景下，我们可能只关心布尔关键词匹配，而从不需要基于位置的搜索操作符（例如短语匹配）或短语排序。

`hitless_words` 允许你创建完全不包含位置信息（hitlist）的索引，或者仅对特定关键词跳过位置信息。

无位置索引通常会比相应的常规全文索引占用更少空间（通常可预期约 1.5 倍）。索引和搜索速度都应更快，但代价是缺少位置查询和排序支持。

如果在位置查询中使用（例如短语查询），这些无位置词会从查询中移除，并作为不带位置的操作数使用。例如，如果 "hello" 和 "world" 是无位置词，而 "simon" 和 "says" 不是，那么短语查询 `"simon says hello world"` 会被转换为 `("simon says" & hello & world)`，从而匹配文档中任意位置的 "hello" 和 "world"，以及作为精确短语的 "simon says"。

如果一个位置查询只包含无位置词，那么它会生成一个空的短语节点，因此整个查询将返回空结果并给出警告。如果整个词典都是无位置的（使用 `all`），那么在相应索引上只能使用布尔匹配。



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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### Rust：

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
`hitless_words_list` 设置允许你在 `CREATE TABLE` 语句中直接指定无位置词。它仅支持 [RT 模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)。

这些值必须用分号（`;`）分隔。

<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", true);
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", true);
```

<!-- intro -->
##### Rust：

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
启用将字段长度（每文档和每索引平均值）计算并存储到全文索引中。可选，默认是 0（不计算也不存储）。

当 `index_field_lengths` 设置为 1 时，Manticore 会：
* 为每个全文字段创建一个对应的长度属性，名称相同但带有 `__len` 后缀
* 为每个文档计算字段长度（按关键词计数）并存储到相应属性中
* 计算每个索引的平均值。这些长度属性会采用特殊的 TOKENCOUNT 类型，但其值实际上是普通的 32 位整数，并且通常可以直接访问。

表达式排序器中的 [BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) 和 [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) 函数基于这些长度，并且需要启用 `index_field_lengths`。历史上，Manticore 使用的是一个简化版、裁剪版的 BM25，与完整函数不同，它**不**考虑文档长度。现在也支持完整版本的 BM25，以及它面向多个字段的扩展，称为 BM25F。它们分别需要每文档长度和每字段长度。因此才有了这个额外指令。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```
<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### Rust：

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
全文索引的索引时 token 过滤器。可选，默认为空。

`index_token_filter` 指令指定了一个可选的全文索引时 token 过滤器。该指令用于创建自定义分词器，以便根据自定义规则生成 token。这个过滤器由 indexer 在将源数据索引到普通表时创建，或由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。插件使用如下格式定义：`library name:plugin name:optional string of settings`。例如，`my_lib.so:custom_blend:chars=@#&`。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```
<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### Rust：

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
超短（短于 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)）关键词的位置增量。可选，允许的值是 0 和 1，默认值是 1。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### Rust：

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
短语边界字符列表。可选，默认为空。

该列表控制哪些字符会被视为短语边界，以便调整词的位置，并通过邻近搜索实现短语级搜索模拟。语法与 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 类似，但不允许映射，且边界字符不得与其他内容重叠。

在短语边界处，会向当前词位置额外加上一个词位置增量（由 [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 指定）。这使得可以通过邻近查询实现短语级搜索：不同短语中的词将被保证彼此相距超过 phrase_boundary_step；因此，在该距离内的邻近搜索就等同于短语级搜索。

只有当此类字符后面跟着一个分隔符时，才会触发短语边界条件；这样可以避免像 S.T.A.L.K.E.R 这样的缩写或 URL 被当作多个短语。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Pytho-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```
<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### Rust：

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

在短语边界处，当前词位置会额外增加这个数值。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```
<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### Rust：

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
用于过滤字段和查询的正则表达式。该指令是可选的，支持多值，默认是空正则表达式列表。Manticore Search 使用的正则引擎是 Google 的 RE2，它以速度快和安全性高而闻名。关于 RE2 支持的语法细节，可以查看 [RE2 语法指南](https://github.com/google/re2/wiki/Syntax)。

在某些应用中，例如商品搜索，产品、型号或属性可能有很多不同的表达方式。例如，`iPhone 3gs` 和 `iPhone 3 gs`（甚至 `iPhone3 gs`）很可能指的是同一款产品。另一个例子是笔记本屏幕尺寸的不同表达方式，例如 `13-inch`、`13 inch`、`13"` 或 `13in`。

正则表达式提供了一种机制，可以为这类情况指定定制规则。在第一个例子里，你或许可以用 wordforms 文件处理少量 iPhone 型号，但在第二个例子里，最好指定规则，把 "13-inch" 和 "13in" 归一化为同一种形式。

`regexp_filter` 中列出的正则表达式会按列出的顺序，在尽可能早的阶段应用，也就是在任何其他处理之前（包括 [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)），甚至在分词之前。也就是说，正则表达式会在索引时作用于原始源字段，在搜索时作用于原始查询文本。

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### Rust：

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
