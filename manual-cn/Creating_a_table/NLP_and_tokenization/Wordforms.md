# 词形

词形是在根据 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 规则对输入文本进行标记化后应用的。它们本质上允许你将一个词替换为另一个词。通常，这用于将不同的词形归结为单一的标准形式（例如将"walks"、"walked"、"walking"等词形归一为"walk"）。它还可以用于实现 [词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md) 的例外情况，因为词形列表中的词不会被词干提取算法处理。

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
词形字典用于在索引和搜索过程中规范化输入的单词。因此，在处理 [普通表](../../Creating_a_table/Local_tables/Plain_table.md) 时，必须旋转表以便更新词形文件中的更改。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float)wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'");
```

<!-- intro -->
##### 普通模式实例：

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

词形支持在 Manticore 中旨在有效处理大型字典。它们对索引速度有适度影响；例如，具有 100 万个条目的字典会使全文本索引速度降低大约 1.5 倍。搜索速度没有受到影响。额外的内存影响大致等于字典文件的大小，并且字典在表之间是共享的。例如，如果相同的 50 MB 词形文件为 10 个不同的表指定，则额外的 `searchd` 内存使用量将约为 50 MB。

<!-- example wf_simple -->
字典文件应采用简单的纯文本格式。每行应包含源和目标词形，使用 UTF-8 编码，并用“>”符号分隔。当文件被加载时，将应用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中的规则。因此，如果您不修改 `charset_table`，您的词形将不区分大小写，类似于其他全文本索引数据。以下是文件内容的示例：

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

有一个名为 [Spelldump](../../Miscellaneous_tools.md#spelldump) 的捆绑工具，可以帮助你创建一个Manticore可读格式的字典文件。该工具可以从以 `ispell` 或 `MySpell` 格式捆绑在OpenOffice中的源 `.dict` 和 `.aff` 字典文件中读取。

你可以将多个源词映射到一个目标词。该过程发生在标记上，而不是源文本，因此会忽略空白和标记之间的差异。

<!-- example wf_more_complex -->
你可以使用 `=>` 符号代替 `>`。注释（以 `#` 开头）也是允许的。最后，如果一行以波浪号（`~`）开头，则该词形会在词法处理后应用，而不是之前（请注意，这种情况下仅支持单个源词和目标词）。

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
如果你需要将 `>`、`=` 或 `~` 用作普通字符，可以在每个字符前加上反斜杠（`\`）进行转义。`>` 和 `=` 都应该以这种方式进行转义。以下是一个示例：

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
你可以指定多个目标标记：

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->

你可以指定多个文件，而不仅仅是一个。可以使用通配符作为模式，所有匹配的文件将按简单的升序处理：

在RT模式下，仅允许使用绝对路径。

如果使用多字节字符集且文件名包含非拉丁字符，结果顺序可能不会完全按照字母顺序排列。如果在多个文件中找到相同的词形定义，将使用后一个文件，并覆盖之前的定义。

<!-- request SQL -->
```sql
create table tbl1 ... wordforms='/tmp/wf*'
create table tbl2 ... wordforms='/tmp/wf, /tmp/wf2'
```

<!-- request Config -->
```ini
wordforms=/tmp/wf
wordforms=/tmp/wf2
wordforms=/tmp/wf_new*
```

<!-- end -->


<!-- proofread -->
