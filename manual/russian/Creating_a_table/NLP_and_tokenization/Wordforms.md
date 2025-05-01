# Форма слов

Форма слов применяется после токенизации входящего текста по правилам [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Они по сути позволяют вам заменить одно слово на другое. Обычно это используется для того, чтобы привести разные формы слова к единой нормальной форме (например, нормализовать все варианты, такие как "walks", "walked", "walking" к нормальной форме "walk"). Это также может быть использовано для реализации исключений [stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md), так как стемминг не применяется к словам, найденным в списке форм.

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
Словарь форм слов. Необязательный, по умолчанию пустой.

Словари форм слов используются для нормализации входящих слов как во время индексации, так и при поиске. Поэтому, когда речь идет о [простой таблице](../../Creating_a_table/Local_tables/Plain_table.md), необходимо поворачивать таблицу, чтобы учитывать изменения в файле форм слов.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float)wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'');
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
##### Пример в простом режиме:

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

Поддержка форм слов в Manticore предназначена для хорошей работы с большими словарями. Они умеренно влияют на скорость индексации; например, словарь с 1 миллионом записей замедляет полнотекстовую индексацию примерно в 1,5 раза. Скорость поиска вообще не затрагивается. Дополнительное влияние на память RAM примерно равно размеру файла словаря, и словари используются совместно между таблицами. Например, если один и тот же файл форм слов размером 50 МБ указан для 10 различных таблиц, дополнительное использование RAM `searchd` будет около 50 МБ.

<!-- example wf_simple -->
Файл словаря должен быть в простом текстовом формате. Каждая строка должна содержать исходные и конечные формы слов в кодировке UTF-8, разделенные знаком 'больше чем'. Правила из [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) будут применены при загрузке файла. Поэтому, если вы не изменяете `charset_table`, ваши формы слов будут нечувствительны к регистру, аналогично другим данным с полнотекстовой индексацией. Ниже приведен пример содержимого файла:

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

Существует встроенная утилита под названием [Spelldump](../../Miscellaneous_tools.md#spelldump), которая помогает вам создать файл словаря в формате, который может прочитать Manticore. Утилита может читать из исходных файлов словаря `.dict` и `.aff` в формате `ispell` или `MySpell`, как это поставляется с OpenOffice.

Вы можете сопоставить несколько исходных слов с одним конечным словом. Процесс происходит на токенах, а не на исходном тексте, поэтому различия в пробелах и разметке игнорируются.

<!-- example wf_more_complex -->
Вы можете использовать символ `=>` вместо `>`. Комментарии (начинающиеся с `#`) также допускаются. Наконец, если строка начинается с тильды (`~`), форма слова будет применена после морфологии, а не до (заметьте, что в этом случае поддерживается только одно исходное и одно целевое слово).

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Некоторые люди пишут '2duo' вместе...
~run > walk # Вместе с активированной морфологией stem_en заменяет 'run', 'running', 'runs' (и любые другие слова, которые относятся к 'run') на 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
Если вам нужно использовать `>`, `=` или `~` как обычные символы, вы можете экранировать их, предваряя каждый обратной косой чертой (``). Оба символа `>` и `=` должны быть экранированы таким образом. Вот пример:

<!-- request Example -->
```ini
a> > abc
>b > bcd
c=> => cde
=>d => def
=>a > f > => foo
~g => bar
```
<!-- end -->

<!-- example wf_multiple_tokens -->
Вы можете указать несколько целевых токенов:

<!-- request Example -->
```ini
s02e02 > сезон 2 эпизод 2
s3 e3 > сезон 3 эпизод 3
```
<!-- end -->

<!-- example wf_multiple_files -->
Вы можете указать несколько файлов, а не только один. Маски могут использоваться в качестве шаблона, и все соответствующие файлы будут обработаны в простом возрастающем порядке:

В режиме RT разрешены только абсолютные пути.

Если используются многобайтовые кодовые страницы и имена файлов содержат нелатинские символы, результирующий порядок может быть не совсем алфавитным. Если одно и то же определение формы слова найдено в нескольких файлах, используется последнее, и оно переопределяет предыдущие определения.

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
# Форма слов

Форма слов применяется после токенизации входного текста по правилам [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Они позволяют заменить одно слово на другое. Обычно это используется для приведения разных форм слова к одной нормальной форме (например, для нормализации всех вариантов, таких как "walks", "walked", "walking" к нормальной форме "walk"). Это также может быть использовано для реализации исключений [stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md), потому что стемминг не применяется к словам, найденным в списке форм.

## формы слов

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
Словарь форм слов. Необязательно, по умолчанию пустой.

Словари форм слов используются для нормализации входящих слов как во время индексации, так и во время поиска. Поэтому, когда речь идет о [простой таблице](../../Creating_a_table/Local_tables/Plain_table.md), необходимо поворачивать таблицу, чтобы учесть изменения в файле форм слов.

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
##### Пример простого режима:

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

Поддержка форм слов в Manticore разработана для хорошей обработки больших словарей. Они умеренно влияют на скорость индексации; например, словарь с 1 миллионом записей замедляет полнотекстовую индексацию примерно в 1.5 раза. Скорость поиска вообще не затрагивается. Дополнительное использование RAM примерно равно размеру файла словаря, и словари используются совместно между таблицами. Например, если тот же файл форм слов объемом 50 МБ указан для 10 различных таблиц, дополнительное использование RAM `searchd` будет примерно 50 МБ.

<!-- example wf_simple -->
Файл словаря должен быть в простом формате обычного текста. Каждая строка должна содержать исходные и целевые формы слов в кодировке UTF-8, разделенные знаком "больше". Правила из [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) будут применены при загрузке файла. Поэтому, если вы не измените `charset_table`, ваши формы слов будут нечувствительны к регистру, аналогично вашим другим данным с полнотекстовой индексацией. Ниже приведен пример содержимого файла:

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

Существует встроенная утилита под названием [Spelldump](../../Miscellaneous_tools.md#spelldump), которая помогает вам создать файл словаря в формате, который может прочитать Manticore. Утилита может читать из исходных файлов словаря `.dict` и `.aff` в формате `ispell` или `MySpell`, как это предоставлено с OpenOffice.

Вы можете сопоставить несколько исходных слов с одним целевым словом. Процесс происходит на токенах, а не на исходном тексте, поэтому различия в пробелах и разметке игнорируются.

<!-- example wf_more_complex -->
Вы можете использовать символ `=>` вместо `>`. Комментарии (начинающиеся с `#`) также разрешены. Наконец, если строка начинается с тильды (`~`), форма слова будет применяться после морфологии, а не до (обратите внимание, что в этом случае поддерживается только одно исходное и одно целевое слово).

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Некоторые люди пишут '2duo' вместе...
~run > walk # Вместе с включенной морфологией stem_en заменяет 'run', 'running', 'runs' (и любые другие слова, которые стеммируются только до 'run') на 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
Если вам нужно использовать `>`, `=` или `~` как обычные символы, вы можете экранировать их, предшествуя каждому обратной косой чертой (`\`). Оба `>` и `=` должны быть экранированы таким образом. Вот пример:

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
s02e02 > сезон 2 серия 2
s3 e3 > сезон 3 серия 3
```
<!-- end -->

<!-- example wf_multiple_files -->
Вы можете указать несколько файлов, а не только один. Маски могут использоваться как шаблоны, и все подходящие файлы будут обработаны в простом порядке возрастания:

В режиме RT разрешены только абсолютные пути.

Если используются многобайтовые кодовые страницы и имена файлов содержат нелатинские символы, результирующий порядок может не быть строго алфавитным. Если одно и то же определение формы слова найдено в нескольких файлах, используется последнее, которое перекрывает предыдущие определения.

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
