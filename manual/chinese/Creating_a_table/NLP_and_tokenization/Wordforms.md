# 词形转换

词形转换是在通过 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 规则对传入文本进行分词处理之后应用的。它们本质上允许你将一个词替换为另一个词。通常，这用于将不同的词形转换为一个统一形式（例如，将所有变体如 "walks"、"walked"、"walking" 转换为统一形式 "walk"）。它也可以用于实现 [词干提取](../../Creating_a_table/NLP_and_tokenization/Morphology.md) 的例外情况，因为词干提取不会应用于 forms 列表中找到的词。

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
词形转换字典。可选，缺省为空。

词形转换字典在索引和搜索期间都用于规范化传入的词。因此，当涉及到 [plain table](../../Creating_a_table/Local_tables/Plain_table.md) 时，需要旋转表以获取词形转换文件中的更改。

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

Manticore 中的词形转换支持旨在良好处理大型字典。它们对索引速度有中等影响；例如，一个包含 100 万个条目的字典会使全文索引变慢约 1.5 倍。搜索速度完全不受影响。额外的 RAM 影响大致等于字典文件的大小，并且字典在表之间共享。例如，如果为 10 个不同的表指定了相同的 50 MB 词形转换文件，那么额外的 `searchd` RAM 使用量将约为 50 MB。

<!-- example wf_simple -->
词典文件应为简单的纯文本格式。每行应包含源词和目标词形，使用 UTF-8 编码，用“大于”符号分隔。加载文件时将应用来自 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 的规则。因此，如果你不修改 `charset_table`，你的词形转换将不区分大小写，类似于你的其他全文索引数据。以下是文件内容的示例：

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

有一个名为 [Spelldump](../../Miscellaneous_tools.md#spelldump) 的捆绑工具，可以帮助你创建 Manticore 可读的字典文件。该工具可以从 `ispell` 或 `MySpell` 格式的源 `.dict` 和 `.aff` 字典文件中读取，如 OpenOffice 所附带的。

你可以将多个源词映射到一个目标词。该过程发生在分词上，而不是源文本上，因此忽略空格和标记的差异。

<!-- example wf_more_complex -->
你可以使用 `=>` 符号代替 `>`。允许使用注释（以 `#` 开头）。最后，如果一行以波浪号（`~`）开头，词形转换将在形态学之后应用，而不是之前（注意在这种情况下仅支持单个源词和目标词）。

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
如果你需要将 `>`、`=` 或 `~` 作为普通字符使用，可以通过在每个字符前加上反斜杠（`\`）来转义它们。`>` 和 `=` 应该以这种方式转义。以下是一个示例：

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
你可以指定多个目标词：

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
你可以指定多个文件，而不仅仅是一个文件。可以使用通配符作为模式，所有匹配的文件将按简单升序处理：

在 RT 模式下，仅允许使用绝对路径。

如果使用多字节代码页且文件名包含非拉丁字符，结果顺序可能不是完全按字母顺序排列。如果在多个文件中找到相同的词形转换定义，后者将被使用并覆盖之前的定义。

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

## wordforms_list

```ini
wordforms_list = 'map-from-token > map-to-token; ...'
```

<!-- example wordforms_list -->
`wordforms_list` 设置允许你在 `CREATE TABLE` 语句中直接指定词形转换。它仅支持 [RT 模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)。

值必须用分号（`;`）分隔。由于词形转换可能包含 `>` 或 `=>` 作为分隔符，以及可能的其他特殊字符，请确保如果分号是标记本身的一部分（例如 `\;`），则转义分号。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'wordforms_list' => 'walks > walk; walked > walk'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) wordforms_list = \'walks > walk; walked > walk\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) wordforms_list = \'walks > walk; walked > walk\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) wordforms_list = \'walks > walk; walked > walk\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", Some(true)).await;
```

<!-- end -->

<!-- end -->

## wordforms_list

```ini
wordforms_list = 'map-from-token > map-to-token; ...'
```

<!-- example wordforms_list -->
`wordforms_list` 设置允许你在 `CREATE TABLE` 语句中直接指定词形转换。它仅支持 [RT 模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)。

##### SQL:

<!-- intro -->
CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'"
```

<!-- request PHP -->

```php
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) wordforms_list = \'walks > walk; walked > walk\'')
##### Python-asyncio:
await utilsApi.sql('CREATE TABLE products(title text, price float) wordforms_list = \'walks > walk; walked > walk\'')
##### Javascript:
res = await utilsApi.sql('CREATE TABLE products(title text, price float) wordforms_list = \'walks > walk; walked > walk\'');
##### Java:
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", true);
```
<!-- intro -->
##### C#:

<!-- request Python -->

```python
utilsApi.Sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", true);
```

<!-- intro -->
##### Rust:

<!-- request Python-asyncio -->

```python
utils_api.sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", Some(true)).await;
```

<!-- intro -->
utils_api.sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", Some(true)).await;

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) wordforms_list = \'walks > walk; walked > walk\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) wordforms_list = 'walks > walk; walked > walk'", Some(true)).await;
```

<!-- end -->

<!-- proofread -->

