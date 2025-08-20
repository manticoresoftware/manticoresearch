# Типы данных

## Полнотекстовые поля и атрибуты

Типы данных Manticore можно разделить на две категории: полноценные текстовые поля и атрибуты.

### Синтаксис имен полей

Имена полей в Manticore должны следовать следующим правилам:

* Могут содержать буквы (a-z, A-Z), цифры (0-9) и дефисы (-)
* Должны начинаться с буквы
* Цифры могут появляться только после букв
* Символ подчеркивания (`_`) является единственным разрешенным специальным символом
* Имена полей нечувствительны к регистру

Например:
* Допустимые имена полей: `title`, `product_id`, `user_name_2`
* Недопустимые имена полей: `2title`, `-price`, `user@name`

### Полнотекстовые поля

Полнотекстовые поля:
* могут индексироваться с помощью алгоритмов обработки естественного языка, поэтому их можно искать по ключевым словам
* не могут использоваться для сортировки или группировки
* содержание оригинального документа может быть извлечено
* содержание оригинального документа может быть использовано для выделения

Полнотекстовые поля представлены типом данных `text`. Все другие типы данных называются "атрибуты".

### Атрибуты

Атрибуты — это значения, не относящиеся к полноценному тексту, связанные с каждым документом, которые могут быть использованы для выполнения фильтрации, сортировки и группировки не по полному тексту во время поиска.

Часто возникает необходимость обрабатывать результаты полнотекстового поиска, основываясь не только на совпадении ID документа и его ранга, но и на ряде других значений для каждого документа. Например, может потребоваться отсортировать результаты поиска новостей по дате, а затем по релевантности, или искать продукты в пределах указанного ценового диапазона, или ограничить поиск по блогу постами, созданными выбранными пользователями, или сгруппировать результаты по месяцам. Чтобы сделать это эффективно, Manticore позволяет не только использовать полнотекстовые поля, но также добавлять дополнительные атрибуты к каждому документу. Эти атрибуты могут использоваться для фильтрации, сортировки или группировки совпадений по полному тексту или для поиска только по атрибутам.

Атрибуты, в отличие от полнотекстовых полей, не индексируются по полному тексту. Они сохраняются в таблице, но их нельзя искать как полнотекстовые.

<!-- example attributes or fields -->

Хорошим примером для атрибутов может быть таблица постов на форуме. Предположим, что только поля заголовка и содержания необходимо сделать доступными для полнотекстового поиска - но иногда также требуется ограничить поиск определенным автором или подфорумом (т.е. искать только те строки, которые имеют определенные значения `author_id` или `forum_id`); или сортировать совпадения по столбцу `post_date`; или группировать совпадающие посты по месяцам `post_date` и подсчитывать количество совпадений на группу.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('forum');
$index->create([
    'title'=>['type'=>'text'],
	'content'=>['type'=>'text'],
	'author_id'=>['type'=>'int'],
	'forum_id'=>['type'=>'int'],
	'post_date'=>['type'=>'timestamp']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```java
utilsApi.Sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table forum
{
	type = rt
	path = forum

	# when configuring fields via config, they are indexed (and not stored) by default
	rt_field = title
	rt_field = content

	# this option needs to be specified for the field to be stored
	stored_fields = title, content

	rt_attr_uint = author_id
	rt_attr_uint = forum_id
	rt_attr_timestamp = post_date
}
```

<!-- end -->

<!-- example filtered query -->

Этот пример показывает выполнение полнотекстового запроса с фильтрацией по `author_id`, `forum_id` и сортировкой по `post_date`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from forum where author_id=123 and forum_id in (1,3,7) order by post_date desc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
  "query":
  {
    "match_all": {},
    "bool":
    {
      "must":
      [
        { "equals": { "author_id": 123 } },
		{ "in": { "forum_id": [1,3,7] } }
      ]
    }
  },
  "sort": [ { "post_date": "desc" } ]
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->search([
        'table' => 'forum',
        'query' =>
        [
            'match_all' => [],
            'bool' => [
                'must' => [
                    'equals' => ['author_id' => 123],
                    'in' => [
                        'forum_id' => [
                            1,3,7
                        ]
                    ]
                ]
            ]
        ],
        'sort' => [
        ['post_date' => 'desc']
    ]
]);
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
HashMap<String,Object> filters = new HashMap<String,Object>(){{
    put("must", new HashMap<String,Object>(){{
        put("equals",new HashMap<String,Integer>(){{
            put("author_id",123);
        }});
        put("in",
            new HashMap<String,Object>(){{
                put("forum_id",new int[] {1,3,7});
        }});
    }});
}};
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("bool",filters);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("post_date","desc");}});
}});
SearchResponse searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var boolFilter = new BoolFilter();
boolFilter.Must = new List<Object> {
    new EqualsFilter("author_id", 123),
    new InFilter("forum_id", new List<Object> {1,3,7})
};
searchRequest.AttrFilter = boolFilter;
searchRequest.Sort = new List<Object> { new SortOrder("post_date", SortOrder.OrderEnum.Desc) };
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut sort = HashMap::new();
sort.insert("post_date".to_string(), serde_json::json!("desc"));
let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(sort)
    ..Default::default()
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

### Хранение атрибутов по строкам и по столбцам

Manticore поддерживает два типа хранения атрибутов:
* по строкам - традиционное хранение, доступное в Manticore Search из коробки
* по столбцам - предоставляется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)

Как можно понять из их названий, они хранят данные по-разному. Традиционное **хранение по строкам**:
* хранит атрибуты без сжатия
* все атрибуты одного документа хранятся в одной строке близко друг к другу
* строки хранятся одна за одной
* доступ к атрибутам осуществляется путем умножения ID строки на шаг (длину одного вектора) и получения запрашиваемого атрибута из рассчитанного местоположения в памяти. Это обеспечивает очень низкую латентность случайного доступа.
* атрибуты должны находиться в памяти, чтобы обеспечить приемлемую производительность, иначе из-за строкового характера хранения Manticore может оказаться в ситуации, когда ему придется читать слишком много ненужных данных с диска, что в большинстве случаев является неоптимальным.

С **хранением по столбцам**:
* каждый атрибут хранится независимо от всех других атрибутов в своем отдельном "столбце"
* хранение разбивается на блоки по 65536 записей
* блоки хранятся в сжатом виде. Это часто позволяет хранить всего несколько различных значений вместо того, чтобы хранить все, как в хранении по строкам. Высокое соотношение сжатия позволяет быстрее читать с диска и значительно снижает требования к памяти
* при индексации данных схема хранения выбирается для каждого блока независимо. Например, если все значения в блоке одинаковы, он получает "const" хранение и для всего блока сохраняется только одно значение. Если уникальных значений в блоке меньше 256, он получает "table" хранение и сохраняет индексы на таблицу значений вместо самих значений
* поиск в блоке может быть заранее отклонен, если очевидно, что запрашиваемое значение отсутствует в блоке.

Хранение по столбцам было разработано для работы с большими объемами данных, которые не помещаются в ОЗУ, поэтому рекомендации таковы:
* если у вас достаточно памяти для всех ваших атрибутов, вы получите преимущества от хранения по строкам
* в противном случае хранение по столбцам все равно может предоставить вам достойную производительность с гораздо меньшим объемом памяти, что позволит вам хранить гораздо больше документов в вашей таблице

### Как переключаться между хранилищами

Традиционное хранение по строкам является значением по умолчанию, поэтому, если вы хотите, чтобы все было сохранено в строковом формате, вам ничего не нужно делать при создании таблицы.

Чтобы включить колонное хранилище, вам нужно:
* указать `engine='columnar'` в [CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE), чтобы сделать все атрибументы таблицы колонными. Затем, если вы хотите сохранить конкретный атрибут в строковом формате, вам нужно добавить `engine='rowwise'` при его объявлении. Например:
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* указать `engine='columnar'` для конкретного атрибута в `CREATE TABLE`, чтобы сделать его колонным. Например:
```sql
create table tbl(title text, type int, price float engine='columnar');
```
или
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* в [простом режиме](../Read_this_first.md#Real-time-mode-vs-plain-mode) вам нужно перечислить атрибуты, которые вы хотите сделать колонными, в [columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs).


Ниже приведен список поддерживаемых типов данных Manticore Search:

## Идентификатор документа

Идентификатор документа является обязательным атрибутом, который должен быть уникальным 64-битным беззнаковым целым числом. Идентификаторы документов могут быть явно указаны при создании таблицы, но они всегда включены, даже если не указаны. Идентификаторы документов не могут быть обновлены.

Когда вы создаете таблицу, вы можете явно указать ID, но независимо от используемого типа данных он всегда будет вести себя так, как описано выше - храниться как беззнаковое 64-битное, но отображаться как знаковое 64-битное целое число.

```sql
mysql> CREATE TABLE tbl(id bigint, content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec)
```

Вы также можете вовсе не указывать ID, он будет включен автоматически.
```sql
mysql> CREATE TABLE tbl(content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec) 
```

При работе с идентификаторами документов важно знать, что они хранятся внутренне как беззнаковые 64-битные целые числа, но отображаются как знаковые 64-битные целые числа в запросах и результатах. Это значит:

* Идентификаторы, превышающие 2^63-1, будут отображаться как отрицательные числа.
* При фильтрации по таким большим идентификаторам вы должны использовать их знаковое представление.
* Используйте функцию [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29), чтобы увидеть фактическое беззнаковое значение.

Например, давайте создадим таблицу и вставим некоторые значения вокруг 2^63:
```sql
mysql> create table t(id_text string);
Query OK, 0 rows affected (0.01 sec)

mysql> insert into t values(9223372036854775807, '2 ^ 63 - 1'),(9223372036854775808, '2 ^ 63');
Query OK, 2 rows affected (0.00 sec)
```

Некоторые идентификаторы отображаются как отрицательные числа в результатах, потому что они превышают 2^63-1. Однако использование `UINT64(id)` может показать их фактические беззнаковые значения:
```sql
mysql> select *, uint64(id) from t;
+----------------------+------------+---------------------+
| id                   | id_text    | uint64(id)          |
+----------------------+------------+---------------------+
|  9223372036854775807 | 2 ^ 63 - 1 | 9223372036854775807 |
| -9223372036854775808 | 2 ^ 63     | 9223372036854775808 |
+----------------------+------------+---------------------+
2 rows in set (0.00 sec)
--- 2 out of 2 results in 0ms ---
```

Для запроса документов с идентификаторами меньше 2^63, вы можете использовать беззнаковое значение напрямую:
```sql
mysql> select * from t where id = 9223372036854775807;
+---------------------+------------+
| id                  | id_text    |
+---------------------+------------+
| 9223372036854775807 | 2 ^ 63 - 1 |
+---------------------+------------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

Однако для идентификаторов, начиная с 2^63, вам нужно использовать знаковое значение:
```sql
mysql> select * from t where id = -9223372036854775808;
+----------------------+---------+
| id                   | id_text |
+----------------------+---------+
| -9223372036854775808 | 2 ^ 63  |
+----------------------+---------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

Если вы используете беззнаковое значение вместо этого, вы получите ошибку:
```sql
mysql> select * from t where id = 9223372036854775808;
ERROR 1064 (42000): number 9223372036854775808 is out of range [-9223372036854775808..9223372036854775807]
```

Значения, которые не помещаются в 64 бита, вызовут аналогичную ошибку:

```sql
mysql> select * from t where id = -9223372036854775809;
ERROR 1064 (42000): number -9223372036854775809 is out of range [-9223372036854775808..9223372036854775807]
```

## Символьные типы данных

Общая синтаксическая структура:
```
string|text [stored|attribute] [indexed]
```

**Свойства:**

1. `indexed` - полнотекстовый индекс (можно использовать в полнотекстовых запросах)
2. `stored` - хранится в docstore (хранится на диске, не в ОЗУ, ленивое чтение)
3. `attribute` - делает его строковым атрибутом (можно сортировать/группировать по нему)

Указание хотя бы одного свойства переопределяет все значения по умолчанию (см. ниже), т.е. если вы решите использовать свою индивидуальную комбинацию свойств, вам нужно перечислить все свойства, которые вы хотите.

**Нет указанных свойств:**

`string` и `text` - это синонимы, но если вы не указываете никаких свойств, они по умолчанию означают разные вещи:

* просто `string` по умолчанию означает `attribute` (подробности смотрите [ниже](../Creating_a_table/Data_types.md#Text)).
* просто `text` по умолчанию означает `stored` + `indexed` (подробности смотрите [ниже](../Creating_a_table/Data_types.md#String)).

### Текст

<!-- example working with text -->

Тип данных текст (просто `text` или `text/string indexed`) формирует полнотекстовую часть таблицы. Поля текста индексируются и могут быть использованы для поиска по ключевым словам.

Текст проходит через анализатор, который преобразует текст в слова, применяет морфологические преобразования и т.д. В конечном итоге полнотекстовая таблица (специальная структура данных, которая позволяет быстро искать ключевое слово) создается из этого текста.

Полнотекстовые поля могут использоваться только в предложении `MATCH()` и не могут быть использованы для сортировки или агрегации. Слова хранятся в инвертированном индексе вместе с ссылками на поля, к которым они принадлежат, и позициями в поле. Это позволяет искать слово внутри каждого поля и использовать продвинутые операторы, такие как близость. По умолчанию оригинальный текст полей индексируется и хранится в хранилище документов. Это означает, что оригинальный текст может быть возвращен вместе с результатами запроса и использован в [подсветке результатов поиска](../Searching/Highlighting.md).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	# when configuring fields via config, they are indexed (and not stored) by default
	rt_field = title

	# this option needs to be specified for the field to be stored
	stored_fields = title
}
```

<!-- end -->

<!-- example working with indexed only  -->

Это поведение может быть переопределено путем явного указания, что текст только индексируется.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text indexed);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text indexed)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text','options'=>['indexed']]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text indexed)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text indexed)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text indexed)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text indexed)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	# when configuring fields via config, they are indexed (and not stored) by default
	rt_field = title
}
```

<!-- end -->

<!-- example for field naming  -->

Поля имеют имена, и вы можете ограничить свои поиски одним полем (например, искать только по "title") или подмножеством полей (например, только по "title" и "abstract"). Вы можете иметь до 256 полей полного текста.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where match('@title first');
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
	"table": "products",
	"query":
	{
		"match": { "title": "first" }
	}
}
```



<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('@title')->get();

```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match":{"title":"first"}}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match":{"title":"first"}}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match":{"title":"first"}}});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text indexed)", Some(true)).await;
```

<!-- end -->

### Строка

<!-- example for string attributes  -->

В отличие от полей полного текста, строковые атрибуты (просто `string` или `string/text attribute`) хранятся в том виде, в каком они были получены, и не могут использоваться в полных текстовых поисках. Вместо этого они возвращаются в результатах, могут использоваться в условии `WHERE` для фильтрации по сравнению или `REGEX`, и могут использоваться для сортировки и агрегации. В общем, не рекомендуется хранить большие тексты в строковых атрибутах, рекомендуется использовать строковые атрибуты для метаданных, таких как имена, заголовки, теги, ключи.

Если вы хотите также индексировать строковый атрибут, вы можете указать оба как `string attribute indexed`. Это позволит выполнять поиск по полному тексту и будет работать как атрибут.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, keys string);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, keys string)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'keys'=>['type'=>'string']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, keys string)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, keys string)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, keys string)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, keys string)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_string = keys
}
```

<!-- end -->

<details>
<summary>Подробнее</summary>

<!-- example string field -->

Вы можете создать поле полного текста, которое также хранится как строковый атрибут. Этот подход создает поле полного текста и строковый атрибут, которые имеют одно и то же имя. Обратите внимание, что вы не можете добавить свойство `stored`, чтобы хранить данные как строковый атрибут и в хранилище документов одновременно.

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` означает, что мы работаем с типом данных строки, который хранится как атрибут и индексируется как поле полного текста.

```sql
CREATE TABLE products ( title string attribute indexed );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products ( title string attribute indexed )"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'string','options'=>['indexed','attribute']]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products ( title string attribute indexed )')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products ( title string attribute indexed )')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products ( title string attribute indexed )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products ( title string attribute indexed )", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	rt_attr_string = title
}
```

<!-- end -->

</details>

### Хранение бинарных данных в Manticore

<!-- example binary -->

Manticore не имеет специального типа поля для бинарных данных, но вы можете надежно хранить их, используя кодирование base64 и типы полей `text stored` или `string stored` (которые являются синонимами). Если вы не кодируете бинарные данные, их части могут быть потеряны — например, Manticore обрезает конец строки, если сталкивается с нулевым байтом.

Вот пример, где мы кодируем команду `ls` с использованием base64, храним ее в Manticore, а затем декодируем, чтобы убедиться, что контрольная сумма MD5 остается неизменной:

<!-- request Example -->
```bash
# md5sum /bin/ls
43d1b8a7ccda411118e2caba685f4329  /bin/ls
# encoded_data=`base64 -i /bin/ls `
# mysql -P9306 -h0 -e "drop table if exists test; create table test(data text stored); insert into test(data) values('$encoded_data')"
# mysql -P9306 -h0 -NB -e "select data from test" | base64 -d > /tmp/ls | md5sum
43d1b8a7ccda411118e2caba685f4329  -
```
<!-- end -->

## Целое число

<!-- example for integers  -->

Целочисленный тип позволяет хранить 32-битные **беззнаковые** целые значения.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price int);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price int)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'price'=>['type'=>'int']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price int)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price int)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price int)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, price int)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_uint = type
}
```

<!-- end -->

<!-- example for bit integers  -->

Целые числа могут храниться в более коротких размерах, чем 32-битные, если указать количество бит. Например, если мы хотим сохранить числовое значение, которое, как мы знаем, не будет больше 8, тип может быть определен как `bit(3)`. Целые числа с указанием битов работают медленнее, чем целые числа полного размера, но требуют меньше RAM. Они сохраняются в 32-битных блоках, поэтому для экономии места их следует размещать в конце определений атрибутов (в противном случае целое число с указанием битов между двумя целыми числами полного размера займет также 32 бита).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, flags bit(3), tags bit(2) );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, flags bit(3), tags bit(2))"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'flags'=>['type'=>'bit(3)'],
	'tags'=>['type'=>'bit(2)']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_uint = flags:3
	rt_attr_uint = tags:2
}
```

<!-- end -->

## Большие целые числа

<!-- example for bigints  -->

Большие целые числа (bigint) - это 64-битные **знаковые** целые числа.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price bigint );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price bigint)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'price'=>['type'=>'bigint']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price bigint )')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price bigint )')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price bigint )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, price bigint )", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_bigint = type
}
```

<!-- end -->

## Булевый тип

<!-- example for boolean  -->

Объявляет булевый атрибут. Это эквивалентно целочисленному атрибуту с количеством битов 1.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, sold bool );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, sold bool)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'sold'=>['type'=>'bool']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, sold bool )')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, sold bool )')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, sold bool )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, sold bool )", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_bool = sold
}
```

<!-- end -->

## Метки времени

<!-- example for timestamps  -->

Тип метки времени представляет собой метки времени Unix, которые хранятся как 32-битные целые числа. В отличие от базовых целых чисел, тип метки времени позволяет использовать функции [времени и даты](../Functions/Date_and_time_functions.md). Преобразование из строковых значений следуют следующим правилам:

- Числа без разделителей, длиной не менее 10 символов, преобразуются в метки времени как есть.
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`
- `%Y-%m-%d`
- `%Y-%m`
- `%Y`

Значения этих спецификаторов преобразования подробно рассмотрены в [руководстве strptime](https://man7.org/linux/man-pages/man3/strptime.3.html), за исключением `%E*S`, которое обозначает миллисекунды.

Имейте в виду, что автоматическое преобразование меток времени не поддерживается в простых таблицах.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, date timestamp);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, date timestamp)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'date'=>['type'=>'timestamp']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, date timestamp)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, date timestamp)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, date timestamp)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, date timestamp)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_timestamp = date
}
```

<!-- end -->

## Вещественные числа

<!-- example for float -->
Действительные числа хранятся как 32-битные числа с плавающей запятой IEEE 754 двойной точности.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, coeff float);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, coeff float)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'coeff'=>['type'=>'float']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, coeff float)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, coeff float)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, coeff float)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, coeff float)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_float = coeff
}
```

<!-- end -->

<!-- example for eps comparison -->

В отличие от целых типов, сравнение двух чисел с плавающей точкой по равенству не рекомендуется из-за потенциальных ошибок округления. Более надежный подход - использовать сравнение на близость, проверяя предельно допустимую погрешность.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select abs(a-b)<=0.00001 from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
  "query": { "match_all": {} } },
  "expressions": { "eps": "abs(a-b)" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('eps','abs(a-b)')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("ebs","abs(a-b)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object>{
    new Dictionary<string, string> { {"ebs", "abs(a-b)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("ebs".to_string(), serde_json::json!("abs(a-b)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- example for float mul -->

Еще один альтернативный подход, который также можно использовать для выполнения `IN(attr,val1,val2,val3)`, - это сравнивать числа с плавающей точкой как целые числа, выбрав множитель и преобразовывая числа с плавающей точкой в целые числа в операциях. Следующий пример иллюстрирует изменение `IN(attr,2.0,2.5,3.5)`, чтобы работать с целочисленными значениями.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select in(ceil(attr*100),200,250,350) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
  "query": { "match_all": {} } },
  "expressions": { "inc": "in(ceil(attr*100),200,250,350)" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('inc','in(ceil(attr*100),200,250,350)')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("inc","in(ceil(attr*100),200,250,350)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"ebs", "in(ceil(attr*100),200,250,350)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("ebs".to_string(), serde_json::json!("in(ceil(attr*100),200,250,350)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- example float_accuracy -->
Значения с плавающей запятой в Manticore отображаются с точностью, чтобы гарантировать, что они отражают точно сохраненное значение. Этот подход был введен, чтобы предотвратить потерю точности, особенно в таких случаях, как географические координаты, где округление до 6 десятичных знаков вызывает неточности.

Теперь Manticore сначала выводит число с 6 цифрами, затем разбирает и сравнивает его с оригинальным значением. Если они не совпадают, добавляются дополнительные цифры, пока они не совпадут.

Например, если значение с плавающей запятой было вставлено как `19.45`, Manticore отобразит его как `19.450001`, чтобы точно представить сохраненное значение.

<!-- request Example -->
```sql
insert into t(id, f) values(1, 19.45)
--------------

Query OK, 1 row affected (0.02 sec)

--------------
select * from t
--------------

+------+-----------+
| id   | f         |
+------+-----------+
|    1 | 19.450001 |
+------+-----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

<!-- end -->


## JSON

<!-- example for creating json -->

Этот тип данных позволяет хранить JSON-объекты, что особенно полезно для обработки данных без схемы. При определении JSON-значений убедитесь, что открывающие и закрывающие фигурные скобки `{` и `}` включены для объектов, или квадратные скобки `[` и `]` для массивов. Несмотря на то, что JSON не поддерживается колоночным хранением, его можно хранить в традиционном построчном хранилище. Стоит отметить, что оба типа хранения могут быть объединены в одной таблице.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, data json);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, data json)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'data'=>['type'=>'json']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, data json)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, data json)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, data json)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, data json)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, data json)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, data json)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_json = data
}
```

<!-- end -->


<!-- example for INDEXOF() json -->

JSON-свойства могут быть использованы в большинстве операций. Существуют также специальные функции, такие как [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29), [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29), [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29), [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29) и [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29), которые позволяют обходить массивы свойств.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select indexof(x>2 for x in data.intarray) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query": { "match_all": {} } },
  "expressions": { "idx": "indexof(x>2 for x in data.intarray)" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('idx','indexof(x>2 for x in data.intarray)')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("idx".to_string(), serde_json::json!("indexof(x>2 for x in data.intarray)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```


<!-- end -->

<!-- example for REGEX() json -->

Свойства текста обрабатываются так же, как строки, поэтому их невозможно использовать в выражениях полного текстового совпадения. Однако можно использовать строковые функции, такие как [REGEX()](../Functions/String_functions.md#REGEX%28%29).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select regex(data.name, 'est') as c from products where c>0
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query":
  {
    "match_all": {},
	"range": { "c": { "gt": 0 } } }
  },
  "expressions": { "c": "regex(data.name, 'est')" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('idx',"regex(data.name, 'est')")->filter('c','gt',0)->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("range", new HashMap<String,Object>(){{
    put("c", new HashMap<String,Object>(){{
        put("gt",0);
    }});
}});
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var rangeFilter = new RangeFilter("c");
rangeFilter.Gt = 0;
searchRequest.AttrFilter = rangeFilter;
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("idx".to_string(), serde_json::json!("indexof(x>2 for x in data.intarray)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```


<!-- end -->

<!-- example for DOUBLE() -->

В случае с JSON-свойствами может потребоваться принудительное указание типа данных для правильного функционирования в определенных ситуациях. Например, при работе со значениями с плавающей запятой необходимо использовать [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29) для правильной сортировки.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products order by double(data.myfloat) desc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query": { "match_all": {} } },
  "sort": [ { "double(data.myfloat)": { "order": "desc"} } ]
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->sort('double(data.myfloat)','desc')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("double(data.myfloat)",new HashMap<String,String>(){{ put("order","desc");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortOrder("double(data.myfloat)", SortOrder.OrderEnum.Desc)
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut sort = HashMap::new();
sort.insert("double(data.myfloat)".to_string(), serde_json::json!("desc"));
let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(sort)
    ..Default::default()
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

## Float vector

<!-- example for creating float_vector -->
Атрибуты векторов с плавающей запятой позволяют хранить списки чисел с плавающей запятой переменной длины, в основном используемые для приложений машинного обучения и поиска по сходству. Этот тип отличается от атрибутов с несколькими значениями (MVA) по нескольким важным причинам:
- Сохраняет точный порядок значений (в отличие от MVA, которые могут менять порядок)
- Сохраняет дубликаты значений (в отличие от MVA, которые удаляют дубликаты)
- Не требует дополнительной обработки при вставке (в отличие от MVA, которые сортируют и удаляют дубликаты)

### Использование и ограничения
- В настоящее время поддерживается только в таблицах реального времени
- Может использоваться только в поисках KNN (k-ближайших соседей)
- Не поддерживается в обычных таблицах или других функциях/выражениях
- При использовании с настройками KNN вы не можете `UPDATE` значения `float_vector`. Используйте `REPLACE` вместо этого
- При использовании без настроек KNN вы можете `UPDATE` значения `float_vector`
- Векторы с плавающей запятой не могут использоваться в обычных фильтрах или сортировке
- Единственный способ фильтрации по значениям `float_vector` - это операции поиска по вектору (KNN)

### Распространенные случаи использования
- Встраивание изображений для поиска по сходству
- Встраивание текста для семантического поиска
- Векторные характеристики для машинного обучения
- Векторы для рекомендательных систем

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, image_vector float_vector);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, image_vector float_vector)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'image_vector'=>['type'=>'float_vector']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, image_vector float_vector)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_float_vector = image_vector
}
```

<!-- end -->

Для получения информации о использовании векторных чисел с плавающей точкой в поиске смотрите [KNN search](../Searching/KNN.md).

## Многоценный целое число (MVA)

<!-- example for creating MVA32 -->

Многоценные атрибуты позволяют хранить списки переменной длины неотрицательных 32-битных целых чисел. Это может быть полезно для хранения значений «один-ко-многим», таких как теги, категории продуктов и свойства.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, product_codes multi);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, product_codes multi)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'product_codes'=>['type'=>'multi']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, product_codes multi)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, product_codes multi)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, product_codes multi)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, product_codes multi)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_multi = product_codes
}
```

<!-- end -->


<!-- example for any/all MVA -->
Он поддерживает фильтрацию и агрегацию, но не сортировку. Фильтрация может быть выполнена с использованием условия, требующего, чтобы как минимум один элемент прошел (с использованием [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)) или все элементы ([ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)) прошли.


<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where any(product_codes)=3
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query":
  {
    "match_all": {},
    "equals" : { "any(product_codes)": 3 }
  }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->filter('any(product_codes)','equals',3)->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})'
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("equals",new HashMap<String,Integer>(){{
     put("any(product_codes)",3);
}});
searchRequest.setQuery(query);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.AttrFilter = new EqualsFilter("any(product_codes)", 3);
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- example for least/greatest MVA -->

Информация о таких элементах, как [least](../Functions/Mathematical_functions.md#LEAST%28%29) или [greatest](../Functions/Mathematical_functions.md#GREATEST%28%29) и длина списка могут быть извлечены. Пример показывает порядок по наименьшему элементу многоценного атрибута.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select least(product_codes) l from products order by l asc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query":
  {
    "match_all": {},
	"sort": [ { "product_codes":{ "order":"asc", "mode":"min" } } ]
  }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->sort('product_codes','asc','min')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("product_codes",new HashMap<String,String>(){{ put("order","asc");put("mode","min");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortMVA("product_codes", SortOrder.OrderEnum.Asc, SortMVA.ModeEnum.Min)
};
searchResponse = searchApi.Search(searchRequest);
```

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut sort_opts = HashMap::new(); 
sort_opts.insert("order".to_string(), serde_json::json!("asc"));
sort_opts.insert("mode".to_string(), serde_json::json!("min"));
sort_expr.insert("product_codes".to_string(), serde_json::json!(sort_opts));
let sort: [HashMap; 1] = [sort_expr];

let search_req = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(sort),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- example for grouping by MVA -->
При группировке по многоценному атрибуту документ будет вносить вклад в столько групп, сколько существует различных значений, связанных с этим документом. Например, если коллекция содержит ровно один документ с многоценным атрибутом 'product_codes' со значениями 5, 7 и 11, то группировка по 'product_codes' создаст 3 группы с `COUNT(*)`, равным 1, и значениями ключей `GROUPBY()` равными 5, 7 и 11 соответственно. Также стоит отметить, что группировка по многоценным атрибутам может привести к дублированию документов в результирующем наборе, поскольку каждый документ может участвовать в многих группах.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products values ( 1, 'doc one', (5,7,11) );
select id, count(*), groupby() from products group by product_codes;
```

<!-- response mysql -->

```sql
Query OK, 1 row affected (0.00 sec)

+------+----------+-----------+
| id   | count(*) | groupby() |
+------+----------+-----------+
|    1 |        1 |        11 |
|    1 |        1 |         7 |
|    1 |        1 |         5 |
+------+----------+-----------+
3 rows in set (0.00 sec)
```

<!-- end -->

<!-- example for MVA value order -->
Порядок чисел, вставленных в качестве значений многоценных атрибутов, не сохраняется. Значения хранятся внутренне как отсортированное множество.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into product values (1,'first',(4,2,1,3));
select * from products;
```

<!-- response mysql -->

```sql
Query OK, 1 row affected (0.00 sec)

+------+---------------+-------+
| id   | product_codes | title |
+------+---------------+-------+
|    1 | 1,2,3,4       | first |
+------+---------------+-------+
1 row in set (0.01 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{
	"table":"products",
	"id":1,
	"doc":
	{
		"title":"first",
		"product_codes":[4,2,1,3]
	}
}

POST /search
{
  "table": "products",
  "query": { "match_all": {} }
}
```

<!-- response JSON -->

```JSON
{
   "table":"products",
   "_id":1,
   "created":true,
   "result":"created",
   "status":201
}

{
   "took":0,
   "timed_out":false,
   "hits":{
      "total":1,
      "hits":[
         {
            "_id": 1,
            "_score":1,
            "_source":{
               "product_codes":[
                  1,
                  2,
                  3,
                  4
               ],
               "title":"first"
            }
         }
      ]
   }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->addDocument([
    "title"=>"first",
    "product_codes"=>[4,2,1,3]
]);
$index->search('')-get();
```

<!-- response PHP -->

```php
Array
(
    [_index] => products
    [_id] => 1
    [created] => 1
    [result] => created
    [status] => 201
)
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] => Array
                                (
                                    [product_codes] => Array
                                        (
                                            [0] => 1
                                            [1] => 2
                                            [2] => 3
                                            [3] => 4
                                        )

                                    [title] => first
                                )
                        )
                )
        )
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
searchApi.search({"table":"products","query":{"match_all":{}}})
```
<!-- response Python -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'table': 'products',
 'result': 'created'}
{'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'product_codes': [1, 2, 3, 4],
                                 u'title': u'first'}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 29}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
await searchApi.search({"table":"products","query":{"match_all":{}}})
```
<!-- response Python-asyncio -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'table': 'products',
 'result': 'created'}
{'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'product_codes': [1, 2, 3, 4],
                                 u'title': u'first'}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 29}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
await indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}});
res = await searchApi.search({"table":"products","query":{"match_all":{}}});
```
<!-- response javascript -->

```javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1,"_source":{"product_codes":[1,2,3,4],"title":"first"}}]}}
```
<!-- intro -->
##### java:

<!-- request java -->

```java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","first");
    put("product_codes",new int[] {4,2,1,3});
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("products");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
System.out.println(searchResponse.toString() );
```
<!-- response java -->

```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "first");
doc.Add("product_codes", new List<Object> {4,2,1,3});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
object query =  new { match_all=null };
var searchRequest = new SearchRequest("products", query);
var searchResponse = searchApi.Search(searchRequest);
Console.WriteLine(searchResponse.ToString())
```
<!-- response C# -->

```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("first"));
doc.insert("product_codes".to_string(), serde_json::json!([4,2,1,3]));
let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;

let query = SearchQuery::new();
let search_req = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
println!("{:?}", search_res);
```

<!-- response Rust -->

```rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}
```

<!-- end -->


## Многоценное большое целое число

<!-- example for creating MVA64 -->

Тип данных, который позволяет хранить списки переменной длины со знаковыми 64-битными целыми числами. У него такая же функциональность, как у многоценного целого числа.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, values multi64);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, values multi64)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'values'=>['type'=>'multi64']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, values multi64))')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, values multi64))')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, values multi64))');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, values multi64))");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, values multi64))");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, values multi64))", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_multi_64 = values
}
```

<!-- end -->

## Свойства столбцовых атрибутов

Когда вы используете столбцовое хранение, вы можете указать следующие свойства для атрибутов.

<!-- example fast_fetch -->
### fast_fetch

По умолчанию столбцовое хранилище Manticore хранит все атрибуты в формате колонок, а также в специальном docstore построчно. Это позволяет быстро выполнять запросы, такие как `SELECT * FROM ...`, особенно при извлечении большого количества записей сразу. Однако, если вы уверены, что это вам не нужно или хотите сэкономить дисковое пространство, вы можете отключить это, указав `fast_fetch='0'` при создании таблицы или (если вы определяете таблицу в конфигурации) с помощью `columnar_no_fast_fetch`, как показано в следующем примере.

<!-- request RT mode -->
```sql
create table t(a int, b int fast_fetch='0') engine='columnar'; desc t;
```

<!-- response RT mode -->

```sql
+-------+--------+---------------------+
| Field | Type   | Properties          |
+-------+--------+---------------------+
| id    | bigint | columnar fast_fetch |
| a     | uint   | columnar fast_fetch |
| b     | uint   | columnar            |
+-------+--------+---------------------+
3 rows in set (0.00 sec)
```

<!-- request Plain mode -->

```ini
source min {
    type = mysql
    sql_host = localhost
    sql_user = test
    sql_pass =
    sql_db = test
    sql_query = select 1, 1 a, 1 b
    sql_attr_uint = a
    sql_attr_uint = b
}

table tbl {
    path = tbl/col
    source = min
    columnar_attrs = *
    columnar_no_fast_fetch = b
}
```

<!-- response Plain mode -->

```sql
+-------+--------+---------------------+
| Field | Type   | Properties          |
+-------+--------+---------------------+
| id    | bigint | columnar fast_fetch |
| a     | uint   | columnar fast_fetch |
| b     | uint   | columnar            |
+-------+--------+---------------------+
```

<!-- end -->
