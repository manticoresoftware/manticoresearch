# Word forms

Word forms are applied after tokenizing incoming text by [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) rules. They essentially let you replace one word with another. Normally, that would be used to bring different word forms to a single normal form (e.g. to normalize all the variants such as "walks", "walked", "walking" to the normal form "walk"). It can also be used to implement [stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md) exceptions, because stemming is not applied to words found in the forms list.

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
Word forms dictionary. Optional, default is empty.

The word forms dictionaries are used to normalize incoming words both during indexing and searching. Therefore, when it comes to a [plain table](../../Creating_a_table/Local_tables/Plain_table.md), it's required to rotate the table in order to pick up changes in the word forms file.

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

Word forms support in Manticore is designed to handle large dictionaries well. They moderately affect indexing speed; for example, a dictionary with 1 million entries slows down full-text indexing by about 1.5 times. Searching speed is not affected at all. The additional RAM impact is roughly equal to the dictionary file size, and dictionaries are shared across tables. For instance, if the very same 50 MB word forms file is specified for 10 different tables, the additional `searchd` RAM usage will be about 50 MB.

<!-- example wf_simple -->
The dictionary file should be in a simple plain text format. Each line should contain source and destination word forms in UTF-8 encoding, separated by a 'greater than' sign. The rules from the [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) will be applied when the file is loaded. Therefore, if you do not modify `charset_table`, your word forms will be case-insensitive, similar to your other full-text indexed data. Below is a sample of the file contents:

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

There is a bundled utility called [Spelldump](../../Miscellaneous_tools.md#spelldump) that helps you create a dictionary file in a format that Manticore can read. The utility can read from source `.dict` and `.aff` dictionary files in the `ispell` or `MySpell` format, as bundled with OpenOffice.

You can map several source words to a single destination word. The process happens on tokens, not the source text, so differences in whitespace and markup are ignored.

<!-- example wf_more_complex -->
You can use the `=>` symbol instead of `>`. Comments (starting with `#`) are also allowed. Finally, if a line starts with a tilde (`~`), the wordform will be applied after morphology, instead of before (note that only a single source and destination word are supported in this case).

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
If you need to use `>`, `=` or `~` as normal characters, you can escape them by preceding each with a backslash (`\`). Both `>` and `=` should be escaped in this manner. Here's an example:

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
You can specify multiple destination tokens:

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
You can specify multiple files, not just one. Masks can be used as a pattern, and all matching files will be processed in simple ascending order:

In the RT mode, only absolute paths are allowed.

If multi-byte codepages are used and file names include non-latin characters, the resulting order may not be exactly alphabetic. If the same wordform definition is found in multiple files, the latter one is used and overrides previous definitions.

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
