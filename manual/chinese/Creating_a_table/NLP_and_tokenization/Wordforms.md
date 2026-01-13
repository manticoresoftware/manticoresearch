# 词形变换

词形变换在通过[字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则对输入文本进行分词后应用。它们本质上允许您将一个词替换为另一个词。通常，这用于将不同的词形变体归一化为单一的标准形式（例如，将"walks"、"walked"、"walking"等所有变体归一化为标准形式"walk"）。它也可以用于实现[词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md)的例外情况，因为词干提取不会应用于词形变换列表中找到的单词。

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
词形变换字典。可选，默认为空。

词形变换字典用于在索引和搜索过程中对输入的单词进行归一化。因此，对于[普通表](../../Creating_a_table/Local_tables/Plain_table.md)，需要轮换表以获取词形变换文件的更改。

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

Manticore 中的词形变换支持旨在很好地处理大型字典。它们对索引速度有适度影响；例如，包含 100 万条目的字典会使全文索引速度减慢约 1.5 倍。搜索速度完全不受影响。额外的 RAM 影响大致等于字典文件大小，并且字典在表之间共享。例如，如果为 10 个不同的表指定了完全相同的 50 MB 词形变换文件，则额外的 `searchd` RAM 使用量将约为 50 MB。

<!-- example wf_simple -->
字典文件应为简单的纯文本格式。每行应包含源词形和目标词形，采用 UTF-8 编码，用大于号分隔。加载文件时将应用[字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中的规则。因此，如果您不修改 `charset_table`，您的词形变换将不区分大小写，类似于其他全文索引数据。以下是文件内容的示例：

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

有一个捆绑的实用程序称为 [Spelldump](../../Miscellaneous_tools.md#spelldump)，可帮助您创建 Manticore 可读格式的字典文件。该实用程序可以读取 `ispell` 或 `MySpell` 格式的源 `.dict` 和 `.aff` 字典文件，这些格式与 OpenOffice 捆绑。

您可以将多个源词映射到单个目标词。该过程作用于词元，而非源文本，因此空格和标记的差异将被忽略。

<!-- example wf_more_complex -->
您可以使用 `=>` 符号代替 `>`。也允许注释（以 `#` 开头）。最后，如果一行以波浪号（`~`）开头，则词形变换将在形态学处理后应用，而不是之前（注意，在这种情况下仅支持单个源词和目标词）。

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
如果您需要将 `>`、`=` 或 `~` 用作普通字符，可以通过在每个字符前加反斜杠（`\`）来转义它们。`>` 和 `=` 都应以此方式转义。以下是一个示例：

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
您可以指定多个目标词元：

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
您可以指定多个文件，而不仅仅是一个。可以使用通配符作为模式，所有匹配的文件将按简单的升序处理：

在 RT 模式下，仅允许绝对路径。

如果使用多字节代码页且文件名包含非拉丁字符，结果顺序可能不完全按字母顺序排列。如果在多个文件中找到相同的词形变换定义，则使用后一个并覆盖先前的定义。

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

