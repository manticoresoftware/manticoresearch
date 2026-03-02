# Исключения

Исключения (также известные как синонимы) позволяют сопоставить один или несколько токенов (включая токены с символами, которые обычно исключаются) с одним ключевым словом. Они похожи на [словоформы](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) тем, что также выполняют сопоставление, но имеют ряд важных отличий.

Краткое описание отличий от [словоформ](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) следующее:

| Исключения | Словоформы |
| - | - |
| Чувствительны к регистру | Нечувствительны к регистру |
| Могут использовать специальные символы, которых нет в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) | Полностью подчиняются [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| Плохо справляются с огромными словарями | Предназначены для обработки миллионов записей |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->
Файл исключений для токенизации. Необязательный, по умолчанию пустой.
В режиме RT допускаются только абсолютные пути.

Ожидаемый формат файла — обычный текст, по одному исключению на строку. Формат строки следующий:

```ini
source-form => destination-form
```

Пример файла:

```ini
at & t => at&t
AT&T => AT&T
Standarten   Fuehrer => standartenfuhrer
Standarten Fuhrer => standartenfuhrer
MS Windows => ms windows
Microsoft Windows => ms windows
C++ => cplusplus
c++ => cplusplus
C plus plus => cplusplus
\=\>abc\> => abc
```

Все формы здесь чувствительны к регистру и **не** будут обрабатываться правилами [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Таким образом, с приведённым выше файлом исключений текст `at&t` будет токенизирован как два ключевых слова `at` и `t` из-за строчных букв. С другой стороны, `AT&T` будет точно соответствовать и создаст одно ключевое слово `AT&T`.

Если вам нужно использовать `>` или `=` как обычные символы, вы можете экранировать их, поставив перед каждым обратную косую черту (`\`). Оба символа `>` и `=` должны быть экранированы таким образом.

Обратите внимание, что ключевые слова целевой формы:
* всегда интерпретируются как *одно* слово
* чувствительны как к регистру, так и к пробелам

В приведённом выше примере запрос `ms windows` *не* будет соответствовать документу с текстом `MS Windows`. Запрос будет интерпретирован как запрос двух ключевых слов, `ms` и `windows`. Сопоставление для `MS Windows` — это одно ключевое слово `ms windows` с пробелом посередине. С другой стороны, `standartenfuhrer` найдет документы с содержимым `Standarten Fuhrer` или `Standarten Fuehrer` (с точно таким же написанием заглавных букв) или любой вариант написания самого ключевого слова, например, `staNdarTenfUhreR`. (Однако он не найдет `standarten fuhrer`: этот текст не соответствует ни одному из перечисленных исключений из-за чувствительности к регистру и индексируется как два отдельных ключевых слова.)

Пробелы в списке исходных форм имеют значение, но их количество — нет. Любое количество пробелов в списке исходных форм будет соответствовать любому другому количеству пробелов в индексируемом документе или запросе. Например, исходная форма `AT & T` будет соответствовать тексту `AT & T`, независимо от количества пробелов как в части исходной формы, так и в индексируемом тексте. Такой текст, следовательно, будет проиндексирован как специальное ключевое слово `AT&T`, благодаря самой первой записи из примера.

Исключения также позволяют захватывать специальные символы (которые являются исключениями из общих правил `charset_table`; отсюда и название). Предположим, что вы в целом не хотите рассматривать `+` как допустимый символ, но всё же хотите иметь возможность искать некоторые исключения из этого правила, такие как `C++`. Приведённый выше пример сделает именно это, полностью независимо от того, какие символы есть в таблице, а каких нет.

При использовании [обычной таблицы](../../Creating_a_table/Local_tables/Plain_table.md) необходимо выполнить ротацию таблицы, чтобы включить изменения из файла исключений. В случае таблицы реального времени изменения будут применяться только к новым документам.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'exceptions' => '/usr/local/manticore/data/exceptions.txt'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  exceptions = /usr/local/manticore/data/exceptions.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## exceptions_list

```ini
exceptions_list = 'source-form => destination-form; ...'
```

<!-- example exceptions_list -->
Настройка `exceptions_list` позволяет указать исключения непосредственно в операторе `CREATE TABLE`. Поддерживается только в [режиме RT](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29).

Значения должны быть разделены точкой с запятой (`;`). Поскольку исключения могут содержать `>` или `=>` в качестве разделителей и, возможно, другие специальные символы, убедитесь, что экранируете точки с запятой, если они являются частью самой формы (например, `\;`).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'exceptions_list' => 'at & t => at&t; MS Windows => ms windows'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions_list = \'at & t => at&t; MS Windows => ms windows\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions_list = \'at & t => at&t; MS Windows => ms windows\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions_list = \'at & t => at&t; MS Windows => ms windows\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'", Some(true)).await;
```

<!-- end -->
<!-- proofread -->

