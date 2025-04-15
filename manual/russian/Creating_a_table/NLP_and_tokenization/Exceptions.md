# Исключения

Исключения (также известные как синонимы) позволяют связывать один или несколько токенов (включая токены с символами, которые обычно исключаются) с одним ключевым словом. Они аналогичны [формам слов](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) в том, что также выполняют связывание, но имеют ряд важных различий.

Краткое резюме различий по сравнению с [формами слов](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) выглядит следующим образом:

| Исключения | Формы слов |
| - | - |
| Чувствительные к регистру | Нечувствительные к регистру |
| Могут использовать специальные символы, которые не находятся в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) | Полностью подчиняются [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| Производительность низкая на больших словарях | Спроектированы для обработки миллионов записей |

## исключения

```ini
исключения = путь/к/исключениям.txt
```

<!-- пример исключений -->
Токенизация файла исключений. Необязательно, по умолчанию пусто.
В режиме RT разрешены только абсолютные пути.

Ожидаемый формат файла - простой текст, с одной строкой на исключение. Формат строки следующий:

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

Все токены здесь чувствительны к регистру и **не** будут обрабатываться по правилам [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Таким образом, с приведенным выше примером файла исключений текст `at&t` будет токенизирован как два ключевых слова `at` и `t` из-за строчных букв. С другой стороны, `AT&T` будет точно совпадать и выдаст одно ключевое слово `AT&T`.

Если вам нужно использовать `>` или `=` в качестве обычных символов, вы можете экранировать их, предваряя каждый обратным слэшем (`\`). Оба `>` и `=` должны быть экранированы таким образом.

Обратите внимание, что ключевые слова map-to:
* всегда интерпретируются как *единое* слово
* чувствительны как к регистру, так и к пробелам

В приведенном выше примере запрос `ms windows` *не* совпадет с документом, содержащим текст `MS Windows`. Запрос будет интерпретирован как запрос на два ключевых слова, `ms` и `windows`. Отображение для `MS Windows` представляет собой одно ключевое слово `ms windows`, с пробелом посередине. С другой стороны, `standartenfuhrer` вернет документы с содержимым `Standarten Fuhrer` или `Standarten Fuehrer` (с точно таким же написанием), или любой вариант написания самого ключевого слова, например, `staNdarTenfUhreR`. (Тем не менее, он не поймает `standarten fuhrer`: этот текст не совпадает ни с одним из перечисленных исключений из-за чувствительности к регистру и индексируется как два отдельных ключевых слова.)

Пробелы в списке токенов map-from имеют значение, но их количество - нет. Любое количество пробелов в списке map-from будет соответствовать любому другому количеству пробелов в индексированном документе или запросе. Например, токен map-from `AT & T` будет соответствовать тексту `AT & T`, независимо от количества пробелов в обеих частях map-from и индексированном тексте. Такой текст, следовательно, будет индексироваться как специальное ключевое слово `AT&T`, благодаря самой первой записи из примера.

Исключения также позволяют захватывать специальные символы (которые являются исключениями из общих правил `charset_table`; откуда и название). Предположим, что вы обычно не хотите рассматривать `+` как допустимый символ, но все же хотите иметь возможность искать некоторые исключения из этого правила, такие как `C++`. Приведенный выше пример сделает именно это, совершенно независимо от того, какие символы находятся в таблице, а какие - нет.

При использовании [простого таблицы](../../Creating_a_table/Local_tables/Plain_table.md) необходимо поворачивать таблицу, чтобы включить изменения из файла исключений. В случае реальной таблицы изменения будут применяться только к новым документам.

<!-- запрос SQL -->

```sql
CREATE TABLE products(title text, price float) исключения = '/usr/local/manticore/data/exceptions.txt'
```

<!-- запрос JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) исключения = '/usr/local/manticore/data/exceptions.txt'"
```

<!-- запрос PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'исключения' => '/usr/local/manticore/data/exceptions.txt'
        ]);
```
<!-- ввод -->
##### Python:

<!-- запрос Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) исключения = \'/usr/local/manticore/data/exceptions.txt\'')
```
<!-- ввод -->
##### Javascript:

<!-- запрос javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) исключения = \'/usr/local/manticore/data/exceptions.txt\'');
```

<!-- ввод -->
##### Java:
<!-- запрос Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) исключения = '/usr/local/manticore/data/exceptions.txt'");
```

<!-- ввод -->
##### C#:
<!-- запрос C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) исключения = '/usr/local/manticore/data/exceptions.txt'");
```

<!-- запрос CONFIG -->

```ini
table products {
  исключения = /usr/local/manticore/data/exceptions.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- конец -->
<!-- корректура -->
