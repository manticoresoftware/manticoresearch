# 词形变化

词形变化在根据[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则对输入文本进行分词后应用。本质上，它们允许你用另一个单词替换一个单词。通常，这用于将不同的词形转换为单一的标准形式（例如，将所有变体如 "walks"、"walked"、"walking" 规范化为标准形式 "walk"）。它也可以用来实现[词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md)异常，因为词干提取不会应用于词形变化列表中的单词。

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
词形变化词典。可选，默认为空。

词形变化词典用于在索引和搜索期间规范化输入的单词。因此，对于[plain table](../../Creating_a_table/Local_tables/Plain_table.md)，需要旋转表以应用词形变化文件中的更改。

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
##### Plain mode example:

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

Manticore 中的词形变化支持设计为能够处理大型词典。它们会适度影响索引速度；例如，包含 100 万条目的词典将使全文索引速度降低约 1.5 倍。搜索速度则完全不受影响。额外的内存开销大致等于词典文件大小，且词典在多张表之间共享。例如，如果同一个 50 MB 的词形变化文件被指定给 10 个不同的表，额外的 `searchd` 内存使用约为 50 MB。

<!-- example wf_simple -->
词典文件应采用简单的纯文本格式。每行应包含源词形和目标词形，使用 UTF-8 编码，并由“大于号”分隔。加载文件时会应用[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中的规则。因此，如果你没有修改 `charset_table`，你的词形变化将是大小写不敏感的，类似于你的其他全文索引数据。以下是文件内容的示例：

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

捆绑的工具[Spelldump](../../Miscellaneous_tools.md#spelldump)可帮助你创建 Manticore 可读取格式的词典文件。该工具可以读取以 `.dict` 和 `.aff` 为后缀的源词典文件，格式为 `ispell` 或 `MySpell`，这些文件通常随 OpenOffice 提供。

你可以将多个源词映射到单个目标词。这个过程作用于分词结果，而非源文本，因此忽略空白和标记的差异。

<!-- example wf_more_complex -->
你可以使用 `=>` 符号代替 `>`。另外，允许存在注释（以 `#` 开头）。最后，如果某行以波浪号 (`~`) 开头，则该词形变化将在形态学处理之后应用，而非之前（注意此情况下只支持单个源词和目标词）。

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
如果你需要将 `>`, `=` 或 `~` 作为普通字符使用，可以通过在前面加反斜杠 (`\`) 来转义它们。`>` 和 `=` 都应该这样转义。示例如下：

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
你可以指定多个文件，而不仅仅是一个。可以使用通配符作为模式，所有匹配的文件将按简单升序处理：

在 RT 模式下，只允许使用绝对路径。

如果使用多字节编码页且文件名包含非拉丁字符，结果排序可能不是严格的字母顺序。如果多个文件中出现相同的词形变化定义，则以后加载的文件的定义为准，覆盖之前的定义。

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

