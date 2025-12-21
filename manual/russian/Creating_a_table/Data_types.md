# Типы данных

## Полнотекстовые поля и атрибуты

Типы данных Manticore можно разделить на две категории: полнотекстовые поля и атрибуты.

### Синтаксис имени поля

Имена полей в Manticore должны соответствовать следующим правилам:

* Могут содержать буквы (a-z, A-Z), цифры (0-9) и дефисы (-)
* Должны начинаться с буквы
* Цифры могут появляться только после букв
* Подчеркивание (`_`) — единственный разрешённый специальный символ
* Имена полей не чувствительны к регистру

Например:
* Допустимые имена полей: `title`, `product_id`, `user_name_2`
* Недопустимые имена полей: `2title`, `-price`, `user@name`

### Полнотекстовые поля

Полнотекстовые поля:
* могут индексироваться с помощью алгоритмов обработки естественного языка, поэтому по ним можно искать ключевые слова
* не могут использоваться для сортировки или группировки
* можно получить исходное содержимое документа
* исходное содержимое документа можно использовать для подсветки

Полнотекстовые поля представлены типом данных `text`. Все остальные типы данных называются «атрибутами».

### Атрибуты

Атрибуты — это не полнотекстовые значения, связанные с каждым документом, которые можно использовать для фильтрации, сортировки и группировки по неполнотекстовым критериям во время поиска.

Часто требуется обрабатывать результаты полнотекстового поиска не только на основе совпадения ID документа и его ранга, но и на основе ряда других значений, связанных с документом. Например, может понадобиться отсортировать результаты поиска новостей по дате, а затем по релевантности, или искать товары в заданном ценовом диапазоне, или ограничить поиск по блогу постами, сделанными выбранными пользователями, или сгруппировать результаты по месяцу. Для эффективного выполнения таких задач Manticore позволяет добавлять к каждому документу не только полнотекстовые поля, но и дополнительные атрибуты. Эти атрибуты можно использовать для фильтрации, сортировки или группировки полнотекстовых совпадений, а также для поиска только по атрибутам.

Атрибуты, в отличие от полнотекстовых полей, не индексируются полнотекстово. Они хранятся в таблице, но по ним нельзя выполнять полнотекстовый поиск.

<!-- example attributes or fields -->

Хорошим примером атрибутов может служить таблица сообщений форума. Предположим, что полнотекстовым поиском должны быть доступны только поля title и content — но иногда требуется ограничить поиск определённым автором или подфорумом (то есть искать только те строки, у которых есть конкретные значения author_id или forum_id); или сортировать совпадения по столбцу post_date; или группировать совпадающие сообщения по месяцу из post_date и подсчитывать количество совпадений в каждой группе.

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

В этом примере показан запуск полнотекстового запроса с фильтрацией по `author_id`, `forum_id` и сортировкой по `post_date`.

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

### Строковое и колонковое хранение атрибутов

Manticore поддерживает два типа хранения атрибутов:
* строковое — традиционное хранение, доступное в Manticore Search из коробки
* колонковое — предоставляется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)

Как можно понять из названий, они хранят данные по-разному. Традиционное **строковое хранение**:
* хранит атрибуты без сжатия
* все атрибуты одного документа хранятся в одной строке рядом друг с другом
* строки хранятся одна за другой
* доступ к атрибутам осуществляется просто умножением ID строки на шаг (длину одного вектора) и получением нужного атрибута из рассчитанного адреса в памяти. Это обеспечивает очень низкую задержку случайного доступа.
* атрибуты должны находиться в памяти для достижения приемлемой производительности, иначе из-за строковой природы хранения Manticore может читать с диска слишком много ненужных данных, что во многих случаях не оптимально.

В случае **колонкового хранения**:
* каждый атрибут хранится независимо от всех остальных в отдельной «колонке»
* хранилище разбито на блоки по 65536 записей
* блоки хранятся в сжатом виде. Это часто позволяет хранить всего несколько различных значений вместо хранения всех значений, как в строковом хранении. Высокое сжатие позволяет быстрее читать с диска и значительно снижает требования к памяти
* при индексировании данных схема хранения выбирается для каждого блока отдельно. Например, если все значения в блоке одинаковы, он получает «const» хранение, и для всего блока хранится только одно значение. Если в блоке менее 256 уникальных значений, он получает «table» хранение и хранит индексы в таблицу значений вместо самих значений
* поиск в блоке может быть досрочно отклонён, если ясно, что запрашиваемое значение отсутствует в блоке.

Колонковое хранение было разработано для обработки больших объёмов данных, которые не помещаются в ОЗУ, поэтому рекомендации таковы:
* если у вас достаточно памяти для всех атрибутов, вы получите выгоду от строкового хранения
* в противном случае колонковое хранение всё равно может обеспечить достойную производительность с гораздо меньшим потреблением памяти, что позволит хранить гораздо больше документов в таблице

### Как переключаться между хранилищами

Традиционное построчное хранение является значением по умолчанию, поэтому если вы хотите, чтобы всё хранилось построчно, вам не нужно ничего делать при создании таблицы.

Чтобы включить колонковое хранение, нужно:
* указать `engine='columnar'` в [CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE), чтобы все атрибуты таблицы были колонковыми. Если вы хотите, чтобы конкретный атрибут оставался построчным, нужно добавить `engine='rowwise'` при его объявлении. Например:
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* указать `engine='columnar'` для конкретного атрибута в `CREATE TABLE`, чтобы сделать его колонковым. Например:
```sql
create table tbl(title text, type int, price float engine='columnar');
```
или
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* в [plain режиме](../Read_this_first.md#Real-time-mode-vs-plain-mode) нужно перечислить атрибуты, которые вы хотите сделать колонковыми, в [columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs).


Ниже приведён список типов данных, поддерживаемых Manticore Search:

## Идентификатор документа

Идентификатор документа — обязательный атрибут, который должен быть уникальным 64-битным беззнаковым целым числом. Идентификаторы документов могут быть явно указаны при создании таблицы, но они всегда включены, даже если не указаны. Идентификаторы документов нельзя обновлять.

При создании таблицы вы можете явно указать ID, но независимо от используемого типа данных он всегда будет вести себя, как описано выше — храниться как беззнаковое 64-битное число, но отображаться как знаковое 64-битное целое.

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

Вы также можете вообще не указывать ID, он будет включён автоматически.
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

При работе с идентификаторами документов важно знать, что они хранятся внутренне как беззнаковые 64-битные целые, но обрабатываются по-разному в зависимости от интерфейса:

**Интерфейс MySQL/SQL:**
* Идентификаторы больше 2^63-1 будут отображаться как отрицательные числа.
* При фильтрации по таким большим ID нужно использовать их знаковое представление.
* Используйте функцию [UINT64()](../Functions/Type_casting_functions.md#UINT64%28%29), чтобы увидеть фактическое беззнаковое значение.

**Интерфейс JSON/HTTP:**
* ID всегда отображаются как их исходные беззнаковые значения, независимо от размера.
* Для фильтрации можно использовать как знаковое, так и беззнаковое представление.
* Операции вставки принимают полный диапазон беззнаковых 64-битных чисел.

Например, создадим таблицу и вставим несколько значений около 2^63:
```sql
mysql> create table t(id_text string);
Query OK, 0 rows affected (0.01 sec)

mysql> insert into t values(9223372036854775807, '2 ^ 63 - 1'),(9223372036854775808, '2 ^ 63');
Query OK, 2 rows affected (0.00 sec)
```

Некоторые ID в результатах отображаются как отрицательные числа, потому что они превышают 2^63-1. Однако использование `UINT64(id)` позволяет увидеть их фактические беззнаковые значения:
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

Для запросов документов с ID меньше 2^63 можно использовать беззнаковое значение напрямую:
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

Однако для ID, начиная с 2^63, нужно использовать знаковое значение:
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

Если использовать беззнаковое значение вместо этого, будет ошибка:
```sql
mysql> select * from t where id = 9223372036854775808;
ERROR 1064 (42000): number 9223372036854775808 is out of range [-9223372036854775808..9223372036854775807]
```

Значения, не помещающиеся в 64 бита, вызовут аналогичную ошибку:

```sql
mysql> select * from t where id = -9223372036854775809;
ERROR 1064 (42000): number -9223372036854775809 is out of range [-9223372036854775808..9223372036854775807]
```

### Различия интерфейсов при больших ID

Различия в поведении между интерфейсами MySQL/SQL и JSON/HTTP становятся более заметными при очень больших идентификаторах документов. Вот полный пример:

**Интерфейс MySQL/SQL:**
```sql
mysql> drop table if exists t; create table t; insert into t values(17581446260360033510);
Query OK, 0 rows affected (0.01 sec)

mysql> select * from t;
+---------------------+
| id                  |
+---------------------+
| -865297813349518106 |
+---------------------+

mysql> select *, uint64(id) from t;
+---------------------+----------------------+
| id                  | uint64(id)           |
+---------------------+----------------------+
| -865297813349518106 | 17581446260360033510 |
+---------------------+----------------------+

mysql> select * from t where id = -865297813349518106;
+---------------------+
| id                  |
+---------------------+
| -865297813349518106 |
+---------------------+

mysql> select * from t where id = 17581446260360033510;
ERROR 1064 (42000): number 17581446260360033510 is out of range [-9223372036854775808..9223372036854775807]
```

**Интерфейс JSON/HTTP:**
```bash
# Search returns the original unsigned value
curl -s 0:9308/search -d '{"table": "t"}'
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 17581446260360033510,
        "_score": 1,
        "_source": {}
      }
    ]
  }
}

# Both signed and unsigned values work for filtering
curl -s 0:9308/search -d '{"table": "t", "query": {"equals": {"id": 17581446260360033510}}}'
curl -s 0:9308/search -d '{"table": "t", "query": {"equals": {"id": -865297813349518106}}}'

# Insert with maximum unsigned 64-bit value
curl -s 0:9308/insert -d '{"table": "t", "id": 18446744073709551615, "doc": {}}'
```

Это означает, что при работе с большими идентификаторами документов:
1. **Интерфейс MySQL** требует использования знакового представления для запросов, но может отображать беззнаковое значение с помощью `UINT64()`
2. **Интерфейс JSON** последовательно использует беззнаковые значения для отображения и принимает оба представления для фильтрации

## Типы данных символов

Общий синтаксис:
```
string|text [stored|attribute] [indexed]
```

**Свойства:**

1. `indexed` — полнотекстовый индекс (можно использовать в полнотекстовых запросах)
2. `stored` — хранится в docstore (хранится на диске, не в ОЗУ, ленивое чтение)
3. `attribute` — делает его строковым атрибутом (можно сортировать/группировать по нему)

Указание хотя бы одного свойства отменяет все свойства по умолчанию (см. ниже), то есть если вы решите использовать пользовательскую комбинацию свойств, нужно перечислить все свойства, которые хотите.

**Если свойства не указаны:**

`string` и `text` — это синонимы, но если не указать свойства, они по умолчанию означают разное:

* просто `string` по умолчанию означает `attribute` (подробности [ниже](../Creating_a_table/Data_types.md#Text)).
* просто `text` по умолчанию означает `stored` + `indexed` (подробности [ниже](../Creating_a_table/Data_types.md#String)).

### Текст

<!-- example working with text -->

Тип данных текст (просто `text` или `text/string indexed`) формирует полнотекстовую часть таблицы. Текстовые поля индексируются и по ним можно искать ключевые слова.

Текст проходит через конвейер анализатора, который разбивает текст на слова, применяет морфологические преобразования и т. д. В итоге из этого текста строится полнотекстовая таблица (специальная структура данных, позволяющая быстро искать ключевое слово).

Полнотекстовые поля можно использовать только в условии `MATCH()` и нельзя использовать для сортировки или агрегации. Слова хранятся в обратном индексе вместе с ссылками на поля, которым они принадлежат, и позициями в поле. Это позволяет искать слово внутри каждого поля и использовать расширенные операторы, например, близость. По умолчанию исходный текст полей индексируется и хранится в хранилище документов. Это значит, что исходный текст можно вернуть в результатах запроса и использовать в [подсветке результатов поиска](../Searching/Highlighting.md).

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

Это поведение можно переопределить, явно указав, что текст индексируется только.

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

Поля имеют имена, и вы можете ограничить поиск одним полем (например, искать только по "title") или подмножеством полей (например, только "title" и "abstract"). Вы можете иметь до 256 полнотекстовых полей.

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

В отличие от полнотекстовых полей, строковые атрибуты (просто `string` или `string/text attribute`) хранятся в том виде, в каком они получены, и не могут использоваться в полнотекстовом поиске. Вместо этого они возвращаются в результатах, могут использоваться в условии `WHERE` для фильтрации по сравнению или `REGEX`, а также для сортировки и агрегации. В общем, не рекомендуется хранить большие тексты в строковых атрибутах, лучше использовать строковые атрибуты для метаданных, таких как имена, заголовки, теги, ключи.

Если вы хотите также индексировать строковый атрибут, вы можете указать его как `string attribute indexed`. Это позволит полнотекстовый поиск и будет работать как атрибут.

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

Вы можете создать полнотекстовое поле, которое также хранится как строковый атрибут. Такой подход создаёт полнотекстовое поле и строковый атрибут с одинаковым именем. Обратите внимание, что нельзя добавить свойство `stored`, чтобы одновременно хранить данные как строковый атрибут и в хранилище документов.

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` означает, что мы работаем со строковым типом данных, который хранится как атрибут и индексируется как полнотекстовое поле.

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

В Manticore нет специального типа поля для бинарных данных, но вы можете безопасно хранить их, используя кодирование base64 и типы полей `text stored` или `string stored` (которые являются синонимами). Если не кодировать бинарные данные, их части могут потеряться — например, Manticore обрезает конец строки, если встречает нулевой байт.

Вот пример, где мы кодируем команду `ls` с помощью base64, сохраняем её в Manticore, а затем декодируем, чтобы проверить, что контрольная сумма MD5 осталась неизменной:

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

Тип integer позволяет хранить 32-битные **беззнаковые** целочисленные значения.

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

Целые числа могут храниться в меньших размерах, чем 32-битные, путем указания количества бит. Например, если мы хотим сохранить числовое значение, которое, как мы знаем, не будет больше 8, тип можно определить как `bit(3)`. Целые числа с битовой длиной работают медленнее, чем полноразмерные, но требуют меньше оперативной памяти. Они сохраняются в 32-битных блоках, поэтому для экономии места их следует группировать в конце определения атрибутов (иначе целое число с битовой длиной между двумя полноразмерными целыми также займет 32 бита).

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

## Большое целое число

<!-- example for bigints  -->

Большие целые числа (bigint) — это 64-битные **знаковые** целые числа.

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

Объявляет булевый атрибут. Это эквивалентно целочисленному атрибуту с битовой длиной 1.

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

Тип timestamp представляет собой Unix-метки времени, которые хранятся как 32-битные целые числа. В отличие от базовых целых чисел, тип timestamp позволяет использовать функции [времени и даты](../Functions/Date_and_time_functions.md). Преобразование из строковых значений происходит по следующим правилам:

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

Значения этих спецификаторов преобразования подробно описаны в [руководстве strptime](https://man7.org/linux/man-pages/man3/strptime.3.html), за исключением `%E*S`, который обозначает миллисекунды.

Обратите внимание, что автоматическое преобразование меток времени не поддерживается в простых таблицах.

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

## Число с плавающей точкой

<!-- example for float -->
Вещественные числа хранятся как 32-битные числа с плавающей точкой одинарной точности по стандарту IEEE 754.

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

В отличие от целочисленных типов, сравнивать два числа с плавающей точкой на равенство не рекомендуется из-за возможных ошибок округления. Более надежный подход — использовать сравнение с допуском, проверяя абсолютную погрешность.

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

Еще одна альтернатива, которая также может использоваться для выполнения `IN(attr,val1,val2,val3)` — сравнивать числа с плавающей точкой как целые, выбирая множитель и преобразуя числа с плавающей точкой в целые в операциях. Следующий пример иллюстрирует изменение `IN(attr,2.0,2.5,3.5)` для работы с целочисленными значениями.

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
Числа с плавающей точкой в Manticore отображаются с точностью, чтобы отражать точное сохраненное значение. Этот подход был введен для предотвращения потери точности, особенно в случаях, таких как географические координаты, где округление до 6 знаков после запятой приводило к неточностям.

Теперь Manticore сначала выводит число с 6 знаками, затем парсит и сравнивает его с исходным значением. Если они не совпадают, добавляются дополнительные знаки, пока не совпадут.

Например, если значение с плавающей точкой было вставлено как `19.45`, Manticore отобразит его как `19.450001`, чтобы точно представить сохраненное значение.

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

Этот тип данных позволяет хранить JSON-объекты, что особенно полезно для работы с данными без схемы. При определении JSON-значений убедитесь, что включены открывающие и закрывающие фигурные скобки `{` и `}` для объектов или квадратные скобки `[` и `]` для массивов. Хотя JSON не поддерживается колонковым хранением, он может храниться в традиционном построчном хранении. Стоит отметить, что оба типа хранения могут комбинироваться в одной таблице.

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

Свойства JSON могут использоваться в большинстве операций. Также существуют специальные функции, такие как [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29), [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29), [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29), [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29) и [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29), которые позволяют обходить массивы свойств.

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

Текстовые свойства обрабатываются так же, как строки, поэтому их нельзя использовать в выражениях полнотекстового поиска. Однако можно использовать строковые функции, такие как [REGEX()](../Functions/String_functions.md#REGEX%28%29).

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

В случае свойств JSON может потребоваться принудительное указание типа данных для правильной работы в некоторых ситуациях. Например, при работе с числами с плавающей точкой необходимо использовать [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29) для правильной сортировки.

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

<!-- example float_vector_auto -->

Атрибуты вектора с плавающей точкой позволяют хранить списки переменной длины из чисел с плавающей точкой, преимущественно используемые для задач машинного обучения и поиска по сходству. Этот тип отличается от [многозначных атрибутов](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) (MVA) по нескольким важным аспектам:
- Сохраняет точный порядок значений (в отличие от MVA, которые могут менять порядок)
- Сохраняет дубликаты значений (в отличие от MVA, которые удаляют дубликаты)
- Нет дополнительной обработки при вставке (в отличие от MVA, которые сортируют и удаляют дубликаты)

Атрибуты вектора с плавающей точкой позволяют хранить списки переменной длины из чисел с плавающей точкой, преимущественно используемые для задач машинного обучения и поиска по сходству.

### Использование и ограничения
- В настоящее время поддерживаются только в таблицах реального времени
- Могут использоваться только в KNN-поисках (k-ближайших соседей)
- Не поддерживаются в обычных таблицах или в других функциях/выражениях
- При использовании с настройками KNN нельзя `UPDATE` значения `float_vector`. Используйте `REPLACE`
- При использовании без настроек KNN можно `UPDATE` значения `float_vector`
- Векторы с плавающей точкой нельзя использовать в обычных фильтрах или сортировках
- Единственный способ фильтровать по значениям `float_vector` — через операции векторного поиска (KNN)

### Распространённые случаи использования
- Встраивания текста для семантического поиска
- Векторы для систем рекомендаций
- Встраивания изображений для поиска по сходству
- Векторы признаков для машинного обучения

** Учтите, что тип данных `float_vector` несовместим с механизмом [Auto schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). **

Для подробностей по настройке векторов с плавающей точкой и использованию их в поисках смотрите [KNN search](../Searching/KNN.md).

### Автоматические встраивания (рекомендуется)

Самый удобный способ работы с векторами с плавающей точкой — использование **автоматических встраиваний**. Эта функция автоматически генерирует встраивания из текстовых данных с помощью моделей машинного обучения, избавляя от необходимости вручную вычислять и вставлять векторы.

#### Преимущества автоматических встраиваний
- **Упрощённый рабочий процесс**: просто вставляйте текст — встраивания генерируются автоматически
- **Нет необходимости в ручном вычислении векторов**: не нужно запускать отдельные модели генерации векторов
- **Постоянство встраиваний**: одна и та же модель обеспечивает согласованное представление векторов
- **Поддержка нескольких моделей**: выбирайте из моделей [sentence-transformers](https://huggingface.co/sentence-transformers/models), OpenAI, Voyage и Jina
- **Гибкий выбор полей**: управлять тем, какие поля используются для генерации встраиваний

#### Создание таблиц с автоматическими встраиваниями

При создании таблицы с автоembedding укажите следующие дополнительные параметры:
- `MODEL_NAME`: модель встраивания для автоматической генерации векторов
- `FROM`: какие поля использовать для генерации встраиваний (пустая строка — все текстовые/строковые поля)

**Поддерживаемые модели встраиваний:**
- **Sentence Transformers**: любая [подходящая модел ь Hugging Face на базе BERT](https://huggingface.co/sentence-transformers/models) (например, `sentence-transformers/all-MiniLM-L6-v2`) — API ключ не нужен. Manticore загружает модель при создании таблицы.
- **OpenAI**: модели встраивания OpenAI, например `openai/text-embedding-ada-002` — требует параметр `API_KEY='<OPENAI_API_KEY>'`
- **Voyage**: модели встраивания Voyage AI — требует параметр `API_KEY='<VOYAGE_API_KEY>'`
- **Jina**: модели встраивания Jina AI — требует параметр `API_KEY='<JINA_API_KEY>'`

<!-- intro -->
##### SQL:
<!-- request SQL -->

Использование модели [sentence-transformers](https://huggingface.co/sentence-transformers/models) (API ключ не нужен)
```sql
CREATE TABLE products (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

Использование модели OpenAI (требуется параметр API_KEY)
```sql
CREATE TABLE products_openai (
    title TEXT,
    content TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='cosine'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,content' API_KEY='<OPENAI_API_KEY>'
);
```

Использование всех текстовых полей для встраиваний (FROM пустой)
```sql
CREATE TABLE products_all_fields (
    title TEXT,
    description TEXT,
    tags TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM=''
);
```

<!-- end -->

#### Использование параметра FROM

Параметр `FROM` контролирует, какие поля используются для генерации встраиваний:

- **Конкретные поля**: `FROM='title'` — используется только поле title
- **Несколько полей**: `FROM='title,description'` — поля title и description объединяются и используются
- **Все текстовые поля**: `FROM=''` (пусто) — используются все поля `text` (поля полного текста) и `string` (строковые атрибуты) таблицы
- **Пустые векторы**: можно вставлять пустые векторы с помощью `()`, чтобы исключить документы из векторного поиска

#### Вставка данных с автоматическими встраиваниями

При использовании автоembedding **не указывайте векторное поле** в командах INSERT. Встраивания будут созданы автоматически из указанных текстовых полей:

```sql
-- Insert text data - embeddings generated automatically
INSERT INTO products (title, description) VALUES 
('smartphone', 'latest mobile device with camera'),
('laptop computer', 'portable workstation for developers');

-- Insert with empty vector (excluded from vector search)
INSERT INTO products (title, description, embedding_vector) VALUES
('no-vector item', 'this item has no embedding', ());
```

### Ручное использование векторов с плавающей точкой

<!-- example for creating float_vector -->
Кроме того, можно работать с вручную вычисленными векторами с плавающей точкой.

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

## Многозначное целочисленное значение (MVA)

<!-- example for creating MVA32 -->

Многозначные атрибуты позволяют хранить списки переменной длины из 32-битных беззнаковых целых чисел. Это может быть полезно для хранения значений один-ко-многим, таких как теги, категории товаров и свойства.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, product_codes multi);
```
или
```sql
CREATE TABLE products(title text, product_codes mva);
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
Поддерживается фильтрация и агрегация, но не сортировка. Фильтрация может быть выполнена с помощью условия, требующего, чтобы хотя бы один элемент прошёл проверку (используя [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)) или все элементы ([ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)) прошли проверку.


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

Можно получить информацию, как [наименьший](../Functions/Mathematical_functions.md#LEAST%28%29), так и [наибольший](../Functions/Mathematical_functions.md#GREATEST%28%29) элемент, а также длину списка. В примере показана сортировка по наименьшему элементу множества значений атрибута.

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
При группировке по атрибуту с множественными значениями, документ будет участвовать в таком количестве групп, сколько у него различных значений этого атрибута. Например, если в коллекции есть ровно один документ с мультизначным атрибутом 'product_codes' со значениями 5, 7 и 11, группировка по 'product_codes' создаст 3 группы с `COUNT(*)` равным 1 и ключами `GROUPBY()` 5, 7 и 11 соответственно. Также стоит отметить, что группировка по многозначным атрибутам может привести к дублированию документов в результирующем наборе, поскольку один документ может участвовать во многих группах.

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
Порядок чисел, вставляемых как значения многозначных атрибутов, не сохраняется. Значения хранятся внутренне в виде отсортированного множества.

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


## Многозначное большое целое число

<!-- example for creating MVA64 -->

Тип данных, позволяющий хранить списки переменной длины из 64-битных знаковых целых чисел. Имеет ту же функциональность, что и многозначное целое число.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, values multi64);
```
или
```sql
CREATE TABLE products(title text, values mva64);
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

## Свойства колонно-ориентированных атрибутов

При использовании колоночного хранения для атрибутов можно задать следующие свойства.

<!-- example fast_fetch -->
### fast_fetch

По умолчанию Manticore Columnar хранит все атрибуты и в колонно-ориентированном формате, и в специальном docstore построчно. Это обеспечивает быструю обработку запросов типа `SELECT * FROM ...`, особенно при выборке большого количества записей за один раз. Однако, если вы уверены, что это не нужно или хотите сэкономить пространство на диске, вы можете отключить данную возможность, указав `fast_fetch='0'` при создании таблицы или (если таблица определяется в конфиге) используя `columnar_no_fast_fetch`, как показано в следующем примере.

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

