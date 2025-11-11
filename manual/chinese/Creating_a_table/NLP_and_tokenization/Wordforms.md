# 词形变换

词形变换在通过[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则对传入文本进行分词后应用。它们本质上允许你用一个单词替换另一个单词。通常，这用于将不同的词形形式归一为一个标准形式（例如，将所有变体如“walks”、“walked”、“walking”归一为标准形式“walk”）。它也可以用来实现[词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md)的例外，因为在词形变换列表中找到的单词不会应用词干提取。

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
词形变换字典。可选，默认为空。

词形变换字典用于在建立索引和搜索时规范传入的单词。因此，对于[plain表](../../Creating_a_table/Local_tables/Plain_table.md)来说，需要轮换表以便应用词形变换文件中的更改。

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

Manticore中的词形变换支持设计为能够良好处理大型字典。它们中等程度地影响索引速度；例如，包含100万个条目的字典会使全文索引速度降低约1.5倍。搜索速度完全不受影响。额外的内存占用大致等于字典文件大小，并且字典在多个表之间共享。例如，如果完全相同的50 MB词形变换文件被指定给10个不同的表，额外的`searchd`内存使用将约为50 MB。

<!-- example wf_simple -->
字典文件应为简单的纯文本格式。每行应包含源词形和目标词形，使用UTF-8编码，并由“greater than”符号分隔。加载文件时将应用[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则。因此，如果你不修改`charset_table`，词形变换将是大小写不敏感的，类似于你的其他全文索引数据。下面是文件内容的示例：

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

有一个捆绑的工具叫做[Spelldump](../../Miscellaneous_tools.md#spelldump)，帮助你创建Manticore能读取的字典文件格式。该工具能够读取以`ispell`或`MySpell`格式的源`.dict`和`.aff`字典文件，这些文件随OpenOffice捆绑提供。

你可以将多个源单词映射到一个目标单词。此过程对词元进行，而非源文本，因此空白和标记的差异会被忽略。

<!-- example wf_more_complex -->
你可以使用`=>`符号代替`>`。允许添加注释（以`#`开头）。最后，如果一行以波浪号(`~`)开头，词形变换将在形态学处理后应用，而非之前（注意，此情况下仅支持单一的源词和目标词）。

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
如果需要将`>`、`=`或`~`作为普通字符使用，可以通过在前面加反斜杠(`\`)进行转义。`>`和`=`均应如此转义。示例：

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
你可以指定多个文件，而不仅限于一个。可以使用通配符作为模式，所有匹配的文件将按简单升序处理：

在RT模式中，仅允许绝对路径。

如果使用多字节编码页且文件名包含非拉丁字符，结果顺序可能并非字母排序。如果在多个文件中发现同一个词形变换定义，则以后定义的将覆盖之前的。

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

