# Формы слов

Формы слов применяются после токенизации входящего текста по правилам [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). По сути, они позволяют заменить одно слово другим. Обычно это используется для приведения различных форм слова к единой нормальной форме (например, для нормализации всех вариантов, таких как "walks", "walked", "walking", к нормальной форме "walk"). Это также можно использовать для реализации исключений [стемминга](../../Creating_a_table/NLP_and_tokenization/Morphology.md), поскольку стемминг не применяется к словам, найденным в списке форм.

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
Словарь форм слов. Необязательный параметр, по умолчанию пустой.

Словари форм слов используются для нормализации входящих слов как при индексации, так и при поиске. Поэтому, когда речь идет о [обычной таблице](../../Creating_a_table/Local_tables/Plain_table.md), необходимо выполнить ротацию таблицы, чтобы подхватить изменения в файле форм слов.

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

Поддержка форм слов в Manticore разработана для эффективной работы с большими словарями. Они умеренно влияют на скорость индексации; например, словарь с 1 миллионом записей замедляет полную текстовую индексацию примерно в 1,5 раза. Скорость поиска не затрагивается вообще. Дополнительное влияние на оперативную память примерно равно размеру файла словаря, и словари являются общими для таблиц. Например, если один и тот же файл форм слов размером 50 МБ указан для 10 разных таблиц, дополнительное использование оперативной памяти `searchd` составит около 50 МБ.

<!-- example wf_simple -->
Файл словаря должен быть в простом текстовом формате. Каждая строка должна содержать исходную и целевую формы слов в кодировке UTF-8, разделенные знаком 'больше'. Правила из [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) будут применены при загрузке файла. Поэтому, если вы не изменяете `charset_table`, ваши формы слов будут нечувствительны к регистру, как и другие данные, проиндексированные для полнотекстового поиска. Ниже приведен пример содержимого файла:

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

В комплекте есть утилита [Spelldump](../../Miscellaneous_tools.md#spelldump), которая помогает создать файл словаря в формате, читаемом Manticore. Утилита может читать исходные файлы словарей `.dict` и `.aff` в формате `ispell` или `MySpell`, поставляемые с OpenOffice.

Вы можете сопоставить несколько исходных слов с одним целевым словом. Процесс происходит на уровне токенов, а не исходного текста, поэтому различия в пробелах и разметке игнорируются.

<!-- example wf_more_complex -->
Вы можете использовать символ `=>` вместо `>`. Также допускаются комментарии (начинающиеся с `#`). Наконец, если строка начинается с тильды (`~`), форма слова будет применена после морфологии, а не до (обратите внимание, что в этом случае поддерживается только одно исходное и одно целевое слово).

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
Если вам нужно использовать `>`, `=` или `~` как обычные символы, вы можете экранировать их, поставив перед каждым обратную косую черту (`\`). И `>`, и `=` должны быть экранированы таким образом. Вот пример:

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
Вы можете указать несколько файлов, а не только один. В качестве шаблона можно использовать маски, и все соответствующие файлы будут обработаны в простом порядке возрастания:

В режиме RT допускаются только абсолютные пути.

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

## wordforms_list

```ini
wordforms_list = 'map-from-token > map-to-token; ...'
```

<!-- example wordforms_list -->
Настройка `wordforms_list` позволяет указать формы слов непосредственно в операторе `CREATE TABLE`. Поддерживается только в [режиме RT](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29).

Значения должны быть разделены точкой с запятой (`;`). Поскольку формы слов могут содержать `>` или `=>` в качестве разделителей и, возможно, другие специальные символы, убедитесь, что экранируете точки с запятой, если они являются частью самого токена (например, `\;`).

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
Настройка `wordforms_list` позволяет указать формы слов непосредственно в операторе `CREATE TABLE`. Поддерживается только в [режиме RT](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29).

Значения должны быть разделены точкой с запятой (`;`). Поскольку формы слов могут содержать `>` или `=>` в качестве разделителей, а также потенциально другие специальные символы, убедитесь, что вы экранируете точки с запятой, если они являются частью самого токена (например, `\;`).

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

<!-- proofread -->

