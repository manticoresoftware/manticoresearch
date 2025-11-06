# 词形变化

词形变化在对输入文本进行基于 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 规则的分词后应用。它们本质上允许你用另一个词替换一个词。通常，这用于将不同的词形转换到一个统一的标准形式（例如，将所有变形如 "walks"、"walked"、"walking" 规范化为标准形式 "walk"）。它也可用于实现 [词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md)的例外处理，因为词干提取不会应用于表中出现的词形。

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
词形变化词典。可选，默认为空。

词形变化词典用于在索引和搜索时规范化输入词汇。因此，对于一个 [纯表](../../Creating_a_table/Local_tables/Plain_table.md)，需要对表进行轮换，以便应用词形变化文件的更改。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt /var/lib/manticore/dict*.txt'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'wordforms' => [
                '/var/lib/manticore/wordforms.txt',
                '/var/lib/manticore/alternateforms.txt',
                '/var/lib/manticore/dict*.txt'
            ]
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float)wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'", Some(true)).await;
```

<!-- intro -->
##### 纯模式示例:

<!-- request CONFIG -->

```ini
table products {
  wordforms = /var/lib/manticore/wordforms.txt
  wordforms = /var/lib/manticore/alternateforms.txt
  wordforms = /var/lib/manticore/dict*.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

Manticore 中的词形变化支持旨在良好处理大型词典。它们对索引速度有中度影响；例如，包含 100 万条词条的词典会使全文索引速度降低约 1.5 倍。搜索速度完全不受影响。额外的内存占用大致等于词典文件大小，并且词典在多个表之间共享。例如，如果同一个 50MB 的词形变化文件被指定给 10 个不同的表，`searchd` 的额外内存使用大约是 50MB。

<!-- example wf_simple -->
词典文件应为简单的纯文本格式。每行应包含以 UTF-8 编码的源词形和目标词形，由“大于号”分隔。加载文件时会应用来自 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 的规则。因此，如果你不修改 `charset_table`，词形变化将不区分大小写，类似于其他全文索引数据。以下是文件内容的示例：

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

捆绑了一个名为 [Spelldump](../../Miscellaneous_tools.md#spelldump) 的工具，帮助你创建 Manticore 可读取格式的词典文件。该工具可以读取以 `ispell` 或 `MySpell` 格式的源 `.dict` 和 `.aff` 词典文件，这些文件与 OpenOffice 一同捆绑。

你可以将多个源词映射到单个目标词。该过程作用于分词，而非源文本，因此忽略了空白和标记的差异。

<!-- example wf_more_complex -->
你可以使用 `=>` 符号代替 `>`。注释（以 `#` 开头）也是允许的。最后，如果一行以波浪线（`~`）开头，则词形变化将在形态学处理之后应用，而非之前（请注意，这种情况下只支持单一的源词和目标词）。

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
如果你需要将 `>`、`=` 或 `~` 用作普通字符，可以在它们前面加反斜杠（`\`）进行转义。`>` 和 `=` 都应该这样转义。示例如下：

<!-- request Example -->
```ini
a\> > abc
\>b > bcd
c\=\> => cde
\=\>d => def
\=\>a \> f \> => foo
\~g => bar
```
<!-- end -->

<!-- example wf_multiple_tokens -->
你可以指定多个目标分词：

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
你可以指定多个文件，而不只是一个。掩码可用作模式，所有匹配的文件将以简单升序顺序处理：

在 RT 模式中，仅允许使用绝对路径。

如果使用多字节编码页且文件名包含非拉丁字符，结果排序可能不完全按字母顺序。如果同一词形定义在多个文件中出现，以后出现的定义将覆盖之前的。

<!-- request SQL -->
```sql
create table tbl1 ... wordforms='/tmp/wf*'
create table tbl2 ... wordforms='/tmp/wf, /tmp/wf2'
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "create table tbl1 ... wordforms='/tmp/wf*'"
POST /sql?mode=raw -d "create table tbl2 ... wordforms='/tmp/wf, /tmp/wf2'"
```

<!-- request Config -->
```ini
wordforms=/tmp/wf
wordforms=/tmp/wf2
wordforms=/tmp/wf_new*
```

<!-- end -->


<!-- proofread -->

