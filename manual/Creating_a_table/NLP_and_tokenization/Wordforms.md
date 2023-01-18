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

The dictionaries are used to normalize incoming words both during indexing and searching. Therefore, when it comes to a [plain table](../../Creating_a_table/Local_tables/Plain_table.md) to pick up changes in wordforms file it's required to rotate the table.

Word forms support in Manticore is designed to support big dictionaries well. They moderately affect indexing speed: for instance, a dictionary with 1 million entries slows down indexing about 1.5 times. Searching speed is not affected at all. Additional RAM impact is roughly equal to the dictionary file size, and dictionaries are shared across tables: i.e. if the very same 50 MB wordforms file is specified for 10 different tables, additional `searchd` RAM usage will be about 50 MB.

Dictionary file should be in a simple plain text format. Each line should contain source and destination word forms, in UTF-8 encoding, separated by "greater" sign. Rules from the [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) will be applied when the file is loaded. So basically it's as case sensitive as your other full-text indexed data, ie. typically case insensitive. Here's the file contents sample:

```ini
walks > walk
walked > walk
walking > walk
```

There is a bundled [Spelldump](../../Miscellaneous_tools.md#spelldump) utility that helps you create a dictionary file in the format Manticore can read from source `.dict` and `.aff` dictionary files in `ispell` or `MySpell` format (as bundled with OpenOffice).

You can map several source words to a single destination word. Because the work happens on tokens, not the source text, differences in whitespace and markup are ignored.

You can use `=>` instead of `>`. Comments (starting with `#` are also allowed. Finally, if a line starts with a tilde (`~`) the wordform will be applied after morphology, instead of before (only single source word is supported).

```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```

You can specify multiple destination tokens:

```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```

You can specify several files and not only just one. Masks can be used as a pattern, and all matching files will be processed in simple ascending order.

In the RT mode only absolute paths are allowed.

If multi-byte codepages are used, and file names can include foreign characters, the resulting order may not be exactly alphabetic. If the same wordform definition is found in several files, the latter one is used, and it overrides previous definitions.


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float)wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'");
```
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
