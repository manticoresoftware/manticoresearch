# 词形

词形在通过 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 规则对传入文本进行分词后应用。它们基本上允许你用另一个词替换一个词。通常情况下，这将用于将不同的词形变为单一的标准形式（例如，将所有变体 "walks"、"walked"、"walking" 规范化为标准形式 "walk"）。它还可以用于实现 [词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md) 例外，因为词干提取不应用于在词形列表中找到的单词。

## 词形

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- 示例词形 -->
词形字典。可选，默认为空。

词形字典用于在索引和搜索期间规范化传入单词。因此，当涉及 [普通表](../../Creating_a_table/Local_tables/Plain_table.md) 时，必须旋转表以便获取词形文件中的更改。

<!-- 介绍 -->
##### SQL:

<!-- 请求 SQL -->

```sql
CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt /var/lib/manticore/dict*.txt'
```

<!-- 请求 JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'"
```

<!-- 请求 PHP -->

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
<!-- 介绍 -->
##### Python:

<!-- 请求 Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'')
```
<!-- 介绍 -->
##### Javascript:

<!-- 请求 javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float)wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'');
```

<!-- 介绍 -->
##### Java:
<!-- 请求 Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'");
```

<!-- 介绍 -->
##### C#:
<!-- 请求 C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'");
```

<!-- 介绍 -->
##### 普通模式示例：

<!-- 请求 CONFIG -->

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
<!-- 结束 -->

Manticore 中的词形支持旨在良好处理大型字典。它们对索引速度的影响有限；例如，具有 100 万条目的字典会使全文索引速度减慢约 1.5 倍。搜索速度则完全不受影响。额外的 RAM 影响大致等于字典文件大小，并且字典在多个表之间共享。例如，如果为 10 个不同表指定了同一个 50 MB 的词形文件，额外的 `searchd` RAM 使用量将约为 50 MB。

<!-- 示例 wf_simple -->
字典文件应为简单的纯文本格式。每行应包含源词和目标词，使用 UTF-8 编码并用 '>' 符号分隔。当文件被加载时，将应用 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中的规则。因此，如果您不修改 `charset_table`，您的词形将是大小写不敏感的，类似于您其他的全文索引数据。以下是文件内容的示例：

<!-- 请求 示例 -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- 结束 -->

有一个名为 [Spelldump](../../Miscellaneous_tools.md#spelldump) 的工具可帮助您创建 Manticore 可读取格式的字典文件。该工具可以从 `ispell` 或 `MySpell` 格式的源 `.dict` 和 `.aff` 字典文件中读取，这些文件与 OpenOffice 一起捆绑。

您可以将多个源单词映射到一个目标单词。该过程发生在标记上，而不是源文本，因此空白和标记的差异将被忽略。

<!-- 示例 wf_more_complex -->
您可以使用 `=>` 符号代替 `>`。也允许有以 `#` 开头的注释。最后，如果一行以波浪符号 (`~`) 开头，该词形将在形态学之后应用，而不是在之前应用（请注意，只有一个源和目标单词在这种情况下是支持的）。

<!-- 请求 示例 -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # 一些人将 '2duo' 写做一起...
~run > walk # 启用 stem_en 形态学将 'run'、'running'、'runs'（以及任何其他词都降到 'run' 的词）替换为 'walk'
```
<!-- 结束 -->

<!-- 示例 wf_escaping -->
如果您需要将 `>`, `=` 或 `~` 作为普通字符使用，可以通过在每个字符前加反斜杠 (`\`) 来转义它们。`>` 和 `=` 应该用这种方式进行转义。以下是一个示例：

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
您可以指定多个目标标记：

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
您可以指定多个文件，而不仅仅是一个。可以使用掩码作为模式，所有匹配的文件将按简单的升序处理：

在 RT 模式下，仅允许绝对路径。

如果使用多字节代码页并且文件名包含非拉丁字符，则结果顺序可能不完全是字母顺序。如果在多个文件中找到相同的词形定义，则使用后者并覆盖之前的定义。

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
