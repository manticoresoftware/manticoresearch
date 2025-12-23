# 词形变化

词形变化是在根据 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 规则对输入文本进行分词后应用的。它们本质上允许你用一个词替换另一个词。通常，这用于将不同的词形统一到一个标准形式（例如，将 “walks”、“walked”、“walking” 等所有变体规范为标准形式 “walk”）。它还可用于实现 [词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md) 的例外情况，因为在词干提取过程中，词形列表中的词不会被词干提取处理。

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
词形变化字典。可选，默认为空。

词形变化字典用于在索引和搜索过程中规范输入词。因此，对于 [plain 表](../../Creating_a_table/Local_tables/Plain_table.md)，需要切换表以拾取词形文件中的更改。

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
##### Plain 模式示例：

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

Manticore 中的词形变化支持设计得能很好地处理大型词典。它们对索引速度有适度影响；例如，一个包含 100 万条目的字典会将全文索引速度降低约 1.5 倍。搜索速度完全不受影响。额外的内存使用大致等同于词典文件大小，并且词典在表之间共享。例如，如果为 10 个不同的表指定同一个 50 MB 的词形文件，额外的 `searchd` 内存使用量约为 50 MB。

<!-- example wf_simple -->
词典文件应为简单的纯文本格式。每行应包含源和目标词形，采用 UTF-8 编码，并用 “>” 分隔。加载文件时将应用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中的规则。因此，如果你不修改 `charset_table`，你的词形将是大小写不敏感的，类似于你的其他全文索引数据。以下是文件内容示例：

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

捆绑实用工具 [Spelldump](../../Miscellaneous_tools.md#spelldump) 可以帮助你创建 Manticore 可识别格式的词典文件。该工具能够读取以 `ispell` 或 `MySpell` 格式的源 `.dict` 和 `.aff` 词典文件，这些词典文件随 OpenOffice 一起捆绑。

你可以将多个源词映射到一个目标词。这一过程发生在词元级别，而非源文本，因此忽略了空白和标记的差异。

<!-- example wf_more_complex -->
你可以使用 `=>` 符号代替 `>`。允许添加注释（以 `#` 开始）。此外，如果一行以波浪号 (`~`) 开头，则词形转换将在形态学处理后应用，而不是之前（注意，这种情况下仅支持单一源词和单一目标词）。

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
如果需要将 `>`、`=` 或 `~` 作为普通字符使用，可以通过在它们前面加反斜杠 (`\`) 来转义。符号 `>` 和 `=` 都应当这样转义。示例如下：

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
你可以指定多个目标词元：

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
你可以指定多个文件，而不仅限于一个。可使用通配符作为模式，所有匹配的文件将按照简单的升序处理：

在 RT 模式下，仅允许使用绝对路径。

如果使用多字节编码页且文件名包含非拉丁字符，最终顺序可能不会完全是字母顺序。如果在多个文件中找到相同的词形定义，则使用后者覆盖之前的定义。

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

