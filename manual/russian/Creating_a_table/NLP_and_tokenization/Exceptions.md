# Исключения

Исключения (также известные как синонимы) позволяют сопоставлять один или несколько токенов (включая токены с символами, которые обычно исключаются) с одним ключевым словом. Они похожи на [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) тем, что также выполняют сопоставление, но имеют ряд важных отличий.

Краткое резюме отличий от [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) следующее:

| Исключения | Формы слов |
| - | - |
| Чувствительны к регистру | Не чувствительны к регистру |
| Могут использовать специальные символы, отсутствующие в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) | Полностью подчиняются [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| Плохо работают с огромными словарями | Предназначены для обработки миллионов записей |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->
Файл исключений для токенизации. Необязательный, по умолчанию пустой.
В режиме RT допускаются только абсолютные пути.

Ожидаемый формат файла — простой текст, по одному исключению на строку. Формат строки следующий:

```ini
map-from-tokens => map-to-token
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

Все токены здесь чувствительны к регистру и **не** будут обрабатываться правилами [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Таким образом, с приведённым выше примером файла исключений, текст `at&t` будет токенизирован как два ключевых слова `at` и `t` из-за строчных букв. С другой стороны, `AT&T` будет точно совпадать и породит одно ключевое слово `AT&T`.

Если вам нужно использовать `>` или `=` как обычные символы, вы можете экранировать их, предваряя каждый обратным слэшем (`\`). И `>`, и `=` должны быть экранированы таким образом.

Обратите внимание, что ключевые слова для сопоставления:
* всегда интерпретируются как *одно* слово
* чувствительны к регистру и пробелам

В приведённом выше примере запрос `ms windows` *не* совпадёт с документом, содержащим текст `MS Windows`. Запрос будет интерпретирован как запрос по двум ключевым словам, `ms` и `windows`. Сопоставление для `MS Windows` — это одно ключевое слово `ms windows` с пробелом посередине. С другой стороны, `standartenfuhrer` найдёт документы с содержимым `Standarten Fuhrer` или `Standarten Fuehrer` (с точным таким же регистром), или любую вариацию регистра самого ключевого слова, например, `staNdarTenfUhreR`. (Однако он не найдёт `standarten fuhrer`: этот текст не совпадает ни с одним из перечисленных исключений из-за чувствительности к регистру и индексируется как два отдельных ключевых слова.)

Пробелы в списке токенов для сопоставления важны, но их количество — нет. Любое количество пробелов в списке для сопоставления будет соответствовать любому другому количеству пробелов в индексированном документе или запросе. Например, токен для сопоставления `AT & T` будет соответствовать тексту `AT & T`, независимо от количества пробелов как в части для сопоставления, так и в индексированном тексте. Такой текст, следовательно, будет индексироваться как специальное ключевое слово `AT&T`, благодаря самой первой записи из примера.

Исключения также позволяют захватывать специальные символы (которые являются исключениями из общих правил `charset_table`; отсюда и название). Предположим, что вы обычно не хотите рассматривать `+` как допустимый символ, но всё же хотите иметь возможность искать некоторые исключения из этого правила, например, `C++`. Приведённый выше пример сделает именно это, полностью независимо от того, какие символы есть в таблице, а каких нет.

При использовании [plain table](../../Creating_a_table/Local_tables/Plain_table.md) необходимо вращать таблицу, чтобы учесть изменения из файла исключений. В случае таблицы реального времени изменения будут применяться только к новым документам.

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
<!-- proofread -->

