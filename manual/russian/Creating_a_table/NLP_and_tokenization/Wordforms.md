# Формы слов

Формы слов применяются после токенизации входящего текста правилами [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Они позволяют заменять одно слово другим. Обычно это используется для приведения различных форм слова к единой нормальной форме (например, нормализация всех вариантов таких как "walks", "walked", "walking" к нормальной форме "walk"). Это также можно использовать для реализации исключений в [стемминге](../../Creating_a_table/NLP_and_tokenization/Morphology.md), так как стемминг не применяется к словам, найденным в списке форм.

## wordforms

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- example wordforms -->
Словарь форм слов. Необязательно, по умолчанию пуст.

Словари форм слов используются для нормализации входящих слов как при индексировании, так и при поиске. Поэтому, когда речь идёт о [plain table](../../Creating_a_table/Local_tables/Plain_table.md), требуется перезагрузка таблицы, чтобы подхватить изменения в файле форм слов.

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
##### Пример в Plain режиме:

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

Поддержка форм слов в Manticore устроена так, чтобы эффективно работать с большими словарями. Они умеренно влияют на скорость индексирования; например, словарь с 1 миллионом записей замедляет полнотекстовое индексирование примерно в 1.5 раза. Скорость поиска при этом не страдает. Дополнительное потребление ОЗУ примерно равно размеру файла словаря, и словари разделяются между таблицами. Например, если один и тот же файл форм слов размером 50 МБ используется для 10 разных таблиц, дополнительное использование ОЗУ `searchd` составит примерно 50 МБ.

<!-- example wf_simple -->
Файл словаря должен быть в простом текстовом формате. Каждая строка должна содержать исходную и целевую формы слова в кодировке UTF-8, разделённые знаком "больше" (`>`). Правила из [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) будут применены при загрузке файла. Следовательно, если вы не изменяете `charset_table`, ваши формы слов будут нечувствительны к регистру, как и остальные ваши данные, индексированные полнотекстово. Ниже приведён пример содержимого файла:

<!-- request Example -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- end -->

В комплекте есть утилита под названием [Spelldump](../../Miscellaneous_tools.md#spelldump), которая помогает создавать файл словаря в формате, который понимает Manticore. Утилита может читать исходные файлы `.dict` и `.aff` формата `ispell` или `MySpell`, как поставляется в OpenOffice.

Вы можете сопоставлять несколько исходных слов с одним целевым. Процесс происходит на уровне токенов, а не исходного текста, поэтому различия в пробелах и разметке игнорируются.

<!-- example wf_more_complex -->
Можно использовать символ `=>` вместо `>`. Также допускаются комментарии (начинающиеся с `#`). Наконец, если строка начинается с тильды (`~`), форма слова применяется после морфологии, а не до неё (обратите внимание, что в таком случае поддерживается только одно исходное и одно целевое слово).

<!-- request Example -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # Some people write '2duo' together...
~run > walk # Along with stem_en morphology enabled replaces 'run', 'running', 'runs' (and any other words that stem to just 'run') to 'walk'
```
<!-- end -->

<!-- example wf_escaping -->
Если нужно использовать `>`, `=` или `~` как обычные символы, их можно экранировать, ставя перед каждым обратный слеш (`\`). Обе конструкции `>` и `=` нужно экранировать таким образом. Вот пример:

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
Можно указать несколько целевых токенов:

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
Можно указать несколько файлов, не только один. Маски можно использовать как шаблон, и все подходящие файлы будут обработаны в простом порядке возрастания:

В режиме RT разрешены только абсолютные пути.

Если используются многобайтовые кодировки, и имена файлов содержат нелатинские символы, итоговый порядок может быть не совсем алфавитным. Если одно и то же определение формы слова встречается в нескольких файлах, используется последнее из них, переопределяя предыдущие.

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

