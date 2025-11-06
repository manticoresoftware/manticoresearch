# Формы слов

Формы слов применяются после токенизации входящего текста по правилам [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Они позволяют заменить одно слово другим. Обычно это используется для приведения различных форм слова к единой нормальной форме (например, для нормализации всех вариантов таких как "walks", "walked", "walking" к нормальной форме "walk"). Также это может использоваться для реализации исключений при [стемминге](../../Creating_a_table/NLP_and_tokenization/Morphology.md), так как стемминг не применяется к словам, найденным в списке форм.

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
Словарь форм слов. Опционально, по умолчанию пустой.

Словари форм слов используются для нормализации входящих слов как при индексировании, так и при поиске. Поэтому, когда речь идёт о [plain table](../../Creating_a_table/Local_tables/Plain_table.md), требуется ротировать таблицу, чтобы учесть изменения в файле форм слов.

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
##### Пример в обычном режиме (Plain mode):

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

Поддержка форм слов в Manticore разработана для эффективной работы с большими словарями. Они умеренно влияют на скорость индексирования; например, словарь с 1 миллионом записей замедляет полнотекстовое индексирование примерно в 1.5 раза. Скорость поиска не затрагивается вообще. Дополнительное использование RAM примерно равно размеру файла словаря, словари разделяются между таблицами. Например, если один и тот же файл форм слов размером 50 МБ указан для 10 разных таблиц, дополнительное использование RAM `searchd` будет около 50 МБ.

<!-- example wf_simple -->
Файл словаря должен иметь простой текстовый формат. Каждая строка должна содержать исходную и целевую формы слова в кодировке UTF-8, разделённые знаком "больше" (greater than). При загрузке файла применяются правила из [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Поэтому, если вы не меняете `charset_table`, ваши формы слов будут нечувствительны к регистру, аналогично вашим другим данным с полнотекстовым индексированием. Ниже приведён пример содержимого файла:

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

В комплекте есть утилита [Spelldump](../../Miscellaneous_tools.md#spelldump), которая помогает создавать файл словаря в формате, который понимает Manticore. Утилита может читать исходные файлы `.dict` и `.aff` в формате `ispell` или `MySpell`, поставляемых вместе с OpenOffice.

Вы можете сопоставить нескольким исходным словам одно целевое слово. Процесс происходит с токенами, а не с исходным текстом, поэтому различия в пробелах и разметке игнорируются.

<!-- example wf_more_complex -->
Вы можете использовать символ `=>` вместо `>`. Допускаются комментарии (начинающиеся с `#`). Наконец, если строка начинается с тильды (`~`), форма слова будет применяться после морфологии, а не до (обратите внимание, что в этом случае поддерживается только одна исходная и одна целевая форма слова).

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
Если вам нужно использовать `>`, `=` или `~` как обычные символы, вы можете экранировать их, поставив перед каждым обратный слеш (`\`). И `>`, и `=` должны быть экранированы таким образом. Вот пример:

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
Вы можете указать несколько целевых токенов:

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
Вы можете указать несколько файлов, а не только один. Можно использовать маски как паттерны, и будут обработаны все подходящие файлы в простом порядке возрастания:

В режиме RT разрешены только абсолютные пути.

Если используются мультибайтовые кодировки и имена файлов содержат нелатинские символы, итоговый порядок может быть не строго алфавитным. Если одна и та же форма слова найдена в нескольких файлах, используется последняя, переопределяя предыдущие дефиниции.

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

