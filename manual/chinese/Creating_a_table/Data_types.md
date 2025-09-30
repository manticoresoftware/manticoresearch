# 数据类型

## 全文字段和属性

Manticore 的数据类型可以分为两类：全文字段和属性。

### 字段名称语法

Manticore 中的字段名称必须遵循以下规则：

* 可以包含字母 (a-z, A-Z)、数字 (0-9) 和连字符 (-)
* 必须以字母开头
* 数字只能出现在字母之后
* 下划线 (`_`) 是唯一允许的特殊字符
* 字段名称不区分大小写

例如：
* 有效的字段名称：`title`，`product_id`，`user_name_2`
* 无效的字段名称：`2title`，`-price`，`user@name`

### 全文字段

全文字段：
* 可以用自然语言处理算法进行索引，因此可以基于关键词搜索
* 不能用于排序或分组
* 可以检索原始文档内容
* 可以用于高亮显示原始文档内容

全文字段由数据类型 `text` 表示。所有其他数据类型称为“属性”。

### 属性

属性是与每个文档相关联的非全文值，可以在搜索时用来进行非全文过滤、排序和分组。

通常希望根据不仅仅是匹配的文档 ID 及其排名，还有其他一些每个文档的值来处理全文搜索结果。例如，可能需要按日期然后按相关性排序新闻搜索结果，或在指定价格范围内搜索产品，或将博客搜索限制为特定用户发布的帖子，或者按月份对结果进行分组。为了高效实现这些功能，Manticore 不仅支持全文字段，还允许向每个文档添加附加的属性。这些属性可以用来过滤、排序或分组全文匹配项，也可以仅基于属性进行搜索。

属性不同于全文字段，不是全文索引。它们存储在表中，但不能像全文一样搜索。

<!-- example attributes or fields -->

一个属性的好例子是论坛帖子表。假设只需要对标题和内容字段进行全文搜索—但有时也需要限制搜索只针对某个作者或子论坛（即只搜索具有特定 author_id 或 forum_id 的行）；或者按 post_date 列对匹配项排序；或者按 post_date 的月份对匹配帖子分组并计算每组的匹配数量。

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

此示例展示了运行一个全文查询，过滤条件为 `author_id`、`forum_id`，并按 `post_date` 排序。

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

### 行存和列存属性存储

Manticore 支持两种类型的属性存储：
* 行存 — Manticore Search 默认提供的传统存储方式
* 列存 — 由 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 提供

顾名思义，它们以不同方式存储数据。传统的**行存储**：
* 属性未压缩存储
* 同一文档的所有属性存储在一行中，彼此相邻
* 行一个接一个地存储
* 访问属性基本上是通过将行 ID 乘以跨度（单个向量的长度），然后从计算出的内存位置获取请求的属性。它具有非常低的随机访问延迟。
* 属性必须驻留在内存中才能获得可接受的性能，否则由于行存特性，Manticore 可能不得不从磁盘读取大量不必要的数据，通常这是不理想的。

而**列存储**：
* 每个属性独立存储在其单独的“列”中，与其他属性无关
* 存储被分成 65536 条目的块
* 块以压缩形式存储。这通常允许存储少量不同值，而不是像行存储那样存储全部值。高压缩率使得磁盘读取更快，内存需求更低。
* 索引数据时，每个块的存储方案独立选择。例如，如果一个块内所有值都相同，则采用“const”存储，仅为整个块存储一个值。如果每块内唯一值少于 256 个，则采用“table”存储，存储值表的索引而非值本身。
* 如果请求的值在块内不存在，则可以提前拒绝该块的搜索。

列存设计用于处理不适合放入内存的大量数据，因此建议：
* 如果你有足够内存存放所有属性，使用行存储会受益最多
* 否则，列存储依然可以提供不错的性能，同时大幅降低内存占用，使你能在表中存储更多文档

### 如何在存储方式之间切换

传统的行存储是默认的，因此如果您希望所有内容都以行式存储，则在创建表时无需做任何操作。

要启用列存储，需要：
* 在 [CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE) 中指定 `engine='columnar'`，使表的所有属性都变为列存储。然后，如果您想让某个特定属性保持行存储，需要在声明它时添加 `engine='rowwise'`。例如：
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* 在 `CREATE TABLE` 中为特定属性指定 `engine='columnar'`，使它成为列存储。例如：
```sql
create table tbl(title text, type int, price float engine='columnar');
```
或者
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* 在[plain模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)中，您需要在 [columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) 中列出您希望设为列存储的属性。


下面是 Manticore Search 支持的数据类型列表：

## 文档 ID

文档标识符是一个必需的属性，必须是唯一的 64 位无符号整数。文档 ID 可以在创建表时显式指定，但即使不指定，文档 ID 也始终启用。文档 ID 不能被更新。

创建表时，您可以显式指定 ID，但无论您使用什么数据类型，它的行为都将如上描述——存储为无符号 64 位整数，但以有符号 64 位整数形式暴露。

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

您也可以完全省略 ID，它将自动启用。
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

在处理文档 ID 时，重要的是要知道它们在内部以无符号 64 位整数存储，但根据接口不同处理方式也不同：

**MySQL/SQL 接口：**
* 大于 2^63-1 的 ID 会显示为负数。
* 过滤此类大 ID 时，必须使用它们的有符号表示。
* 使用 [UINT64()](../Functions/Type_casting_functions.md#UINT64%28%29) 函数查看实际的无符号值。

**JSON/HTTP 接口：**
* ID 总是显示为其原始的无符号值，无论大小如何。
* 筛选可以使用有符号和无符号两种表示。
* 插入操作接受完整的无符号 64 位范围。

例如，创建一个表并插入一些接近 2^63 的值：
```sql
mysql> create table t(id_text string);
Query OK, 0 rows affected (0.01 sec)

mysql> insert into t values(9223372036854775807, '2 ^ 63 - 1'),(9223372036854775808, '2 ^ 63');
Query OK, 2 rows affected (0.00 sec)
```

一些 ID 在结果中显示为负数，因为它们超过了 2^63-1。然而，使用 `UINT64(id)` 可以显示它们的实际无符号值：
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

查询 ID 小于 2^63 的文档时，您可以直接使用无符号值：
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

但针对从 2^63 开始的 ID，您需要使用有符号值：
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

如果使用无符号值，则会出现错误：
```sql
mysql> select * from t where id = 9223372036854775808;
ERROR 1064 (42000): number 9223372036854775808 is out of range [-9223372036854775808..9223372036854775807]
```

超出 64 位范围的值也会触发类似错误：

```sql
mysql> select * from t where id = -9223372036854775809;
ERROR 1064 (42000): number -9223372036854775809 is out of range [-9223372036854775808..9223372036854775807]
```

### 大 ID 的接口差异

MySQL/SQL 和 JSON/HTTP 接口之间的行为差异在处理非常大的文档 ID 时更为明显。以下是一个完整示例：

**MySQL/SQL 接口：**
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

**JSON/HTTP 接口：**
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

这意味着在处理大文档 ID 时：
1. **MySQL 接口** 需要用有符号表示进行查询，但可以使用 `UINT64()` 显示无符号值。
2. **JSON 接口** 始终使用无符号值显示，并且接受两种表示进行筛选。

## 字符数据类型

通用语法：
```
string|text [stored|attribute] [indexed]
```

**属性：**

1. `indexed` - 全文索引（可用于全文查询）
2. `stored` - 存储在 docstore 中（存储在磁盘上，而非 RAM 中，延迟读取）
3. `attribute` - 使其成为字符串属性（可按其排序/分组）

指定至少一个属性会覆盖所有默认属性（见下文），也就是说，如果您决定使用自定义属性组合，需要列出所有您想要的属性。

**未指定属性时：**

`string` 和 `text` 是别名，但如果不指定任何属性，它们默认为不同含义：

* 单独 `string` 默认表示 `attribute`（详情见 [下文](../Creating_a_table/Data_types.md#Text)）。
* 单独 `text` 默认表示 `stored` + `indexed`（详情见 [下文](../Creating_a_table/Data_types.md#String)）。

### Text

<!-- example working with text -->

text（仅 `text` 或 `text/string indexed`）数据类型构成了表的全文部分。文本字段被索引，可以搜索关键字。

文本经过分析器管道处理，转换成单词，应用形态变化等。最终，从文本构建全文表（一种特殊数据结构，实现快速关键词搜索）。

全文字段只能在 `MATCH()` 子句中使用，不能用于排序或聚合。单词存储在倒排索引中，附带它们所属字段的引用和字段中的位置。这允许在每个字段内搜索单词，并使用诸如邻近等高级操作符。默认情况下，字段的原始文本既被索引也存储在文档存储中。这意味着原始文本可以随查询结果返回，并用于[搜索结果高亮](../Searching/Highlighting.md)。

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

此行为可以通过显式指定文本仅被索引来覆盖。

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

字段有名称，您可以将搜索限制在单个字段（例如仅搜索“title”）或字段的子集（例如仅“title”和“abstract”）。您最多可以有256个全文字段。

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

### 字符串

<!-- example for string attributes  -->

不同于全文字段，字符串属性（仅 `string` 或 `string/text attribute`）按接收到的原样存储，不能用于全文搜索。相反，它们会返回在结果中，可以用于 `WHERE` 子句进行比较过滤或 `REGEX`，并且可以用来排序和聚合。一般不建议在字符串属性中存储大段文本，而是将字符串属性用于元数据如名字、标题、标签、键。

如果你也想索引字符串属性，可以指定为 `string attribute indexed`。它既允许全文搜索，又作为属性使用。

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
<summary>更多</summary>

<!-- example string field -->

你可以创建一个同时以字符串属性形式存储的全文字段。这种做法会创建一个具有相同名称的全文字段和字符串属性。注意你不能添加 `stored` 属性以同时将数据存储为字符串属性和文档存储。

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` 意味着我们使用的是作为属性存储并且作为全文字段索引的字符串数据类型。

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

### 在 Manticore 中存储二进制数据

<!-- example binary -->

Manticore 没有专门用于二进制数据的字段类型，但你可以通过使用 base64 编码和 `text stored` 或 `string stored` 字段类型（它们是同义词）安全地存储它。如果你不对二进制数据进行编码，部分数据可能会丢失——例如，Manticore 在遇到空字节时会截断字符串末尾。

这里有一个示例，我们用 base64 编码 `ls` 命令，存储到 Manticore，然后解码以验证 MD5 校验和是否保持不变：

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

## 整数

<!-- example for integers  -->

整数类型允许存储32位**无符号**整数值。

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

整数可以通过指定位数来存储比32位更短的大小。例如，如果我们想存储一个我们知道不会超过8的数值，则可以将类型定义为 `bit(3)`。位数整数的性能比全尺寸整数慢，但它们需要更少的RAM。它们以32位块保存，因此为了节省空间，应将它们在属性定义的末尾分组（否则，位数整数夹在两个全尺寸整数之间也将占用32位）。

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

## 大整数

<!-- example for bigints  -->

大整数（bigint）是64位宽的**有符号**整数。

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

## 布尔值

<!-- example for boolean  -->

声明一个布尔属性。它相当于一个位数为1的整数属性。

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

## 时间戳

<!-- example for timestamps  -->

时间戳类型表示Unix时间戳，存储为32位整数。与基本整数不同，时间戳类型允许使用[时间和日期](../Functions/Date_and_time_functions.md)函数。字符串值的转换遵循以下规则：

- 没有分隔符、长度至少10个字符的数字，将按原样转换为时间戳。
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`
- `%Y-%m-%d`
- `%Y-%m`
- `%Y`

这些转换说明符的含义详见[strptime手册](https://man7.org/linux/man-pages/man3/strptime.3.html)，其中 `%E*S` 代表毫秒。

注意，纯表中不支持时间戳的自动转换。

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

## 浮点数

<!-- example for float -->
实数存储为32位IEEE 754单精度浮点数。

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

与整数类型不同，不建议直接比较两个浮点数是否相等，因为可能出现舍入误差。更可靠的方法是使用近似相等比较，通过检查绝对误差范围来判断。

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

另一种替代方法，同样可以用于执行 `IN(attr,val1,val2,val3)`，是通过选择一个乘数因子，将浮点数转换为整数进行比较操作。下面的示例说明了如何修改 `IN(attr,2.0,2.5,3.5)` 以便使用整数值。

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
Manticore 中的浮点值以精确的精度显示，以确保准确反映存储的值。这种方式是为防止精度丢失引入的，特别是在地理坐标等场景中，四舍五入到小数点后6位会导致不准确。

现在，Manticore 首先输出一个6位数字的数值，然后解析并与原值比较。如果两者不匹配，则会不断增加数字位数直到匹配。

例如，如果一个浮点值被插入为 `19.45`，Manticore 会显示为 `19.450001`，以准确代表存储的值。

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

这种数据类型允许存储JSON对象，非常适合处理无模式数据。定义JSON值时，需确保包括对象的左大括号 `{` 和右大括号 `}`，或数组的左中括号 `[` 和右中括号 `]`。虽然JSON不支持列式存储，但可以存储在传统的行存储中。值得注意的是，这两种存储类型可以在同一个表内混合使用。

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

JSON 属性可以用于大多数操作。还有一些特殊函数，如 [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29), [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29), [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29), [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29) 和 [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) ，它们允许遍历属性数组。

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

文本属性被当做字符串处理，因此无法在全文匹配表达式中使用它们。但可以使用字符串函数，如 [REGEX()](../Functions/String_functions.md#REGEX%28%29)。

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

对于 JSON 属性，在某些情况下，可能需要强制使用数据类型以确保正常工作。例如，处理浮点值时，必须使用 [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29) 来实现正确排序。

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

浮点向量属性允许存储可变长度的浮点列表，主要用于机器学习应用和相似性搜索。此类型在几个重要方面区别于[多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)（MVA）：
- 保持值的准确顺序（不同于可能重新排序的MVA）
- 保留重复值（不同于去重的MVA）
- 插入时无额外处理（不同于排序和去重的MVA）

浮点向量属性允许存储可变长度的浮点列表，主要用于机器学习应用和相似性搜索。

### 用法和限制
- 目前仅支持实时表
- 只能用于KNN（k近邻）搜索
- 不支持普通表或其他函数/表达式
- 在使用KNN设置时，不能`UPDATE` `float_vector`值。应使用`REPLACE`
- 未使用KNN设置时，可以`UPDATE` `float_vector`值
- 浮点向量不能用于常规过滤或排序
- 过滤`float_vector`值的唯一方法是通过向量搜索操作（KNN）

### 常见用例
- 用于语义搜索的文本嵌入
- 推荐系统向量
- 用于相似性搜索的图像嵌入
- 机器学习特征向量

** 请注意，`float_vector`数据类型不兼容[自动架构](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)机制。**

有关设置浮点向量及在搜索中使用的更多细节，见[KNN搜索](../Searching/KNN.md)。

### 自动嵌入（推荐）

使用**自动嵌入**是处理浮点向量最方便的方法。此功能使用机器学习模型自动从文本数据生成嵌入，省去了手动计算和插入向量的麻烦。

#### 自动嵌入的优点
- **简化工作流程**：只需插入文本，嵌入自动生成
- **无需手动计算向量**：无需运行单独的嵌入模型
- **嵌入一致性**：同一模型确保向量表示一致
- **多模型支持**：可选择[sentence-transformers](https://huggingface.co/sentence-transformers/models)、OpenAI、Voyage和Jina模型
- **灵活字段选择**：控制用于生成嵌入的字段

#### 创建带自动嵌入的表

创建带自动嵌入的表时，指定以下额外参数：
- `MODEL_NAME`：用于自动向量生成的嵌入模型
- `FROM`：用于生成嵌入的字段（空字符串表示所有文本/字符串字段）

**支持的嵌入模型：**
- **Sentence Transformers**：任何[适用的基于BERT的Hugging Face模型](https://huggingface.co/sentence-transformers/models)（如`sentence-transformers/all-MiniLM-L6-v2`）—无需API密钥。Manticore在创建表时下载模型。
- **OpenAI**：OpenAI嵌入模型，如`openai/text-embedding-ada-002` — 需要参数`API_KEY='<OPENAI_API_KEY>'`
- **Voyage**：Voyage AI嵌入模型 — 需要参数`API_KEY='<VOYAGE_API_KEY>'`
- **Jina**：Jina AI嵌入模型 — 需要参数`API_KEY='<JINA_API_KEY>'`

<!-- intro -->
##### SQL:
<!-- request SQL -->

使用[sentence-transformers模型](https://huggingface.co/sentence-transformers/models)（无需API密钥）
```sql
CREATE TABLE products (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

使用OpenAI模型（需要API_KEY参数）
```sql
CREATE TABLE products_openai (
    title TEXT,
    content TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='cosine'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,content' API_KEY='<OPENAI_API_KEY>'
);
```

使用所有文本字段生成嵌入（FROM为空）
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

#### FROM参数用法

`FROM`参数控制用于生成嵌入的字段：

- **指定字段**：`FROM='title'` — 只使用title字段
- **多个字段**：`FROM='title,description'` — 合并title和description字段一起使用
- **所有文本字段**：`FROM=''`（为空） — 使用表中所有的`text`（全文字段）和`string`（字符串属性）字段
- **空向量**：仍可用`()`插入空向量，以排除文档参加向量搜索

#### 使用自动嵌入插入数据

使用自动嵌入时，**不要在INSERT语句中指定向量字段**。嵌入由指定的文本字段自动生成：

```sql
-- Insert text data - embeddings generated automatically
INSERT INTO products (title, description) VALUES 
('smartphone', 'latest mobile device with camera'),
('laptop computer', 'portable workstation for developers');

-- Insert with empty vector (excluded from vector search)
INSERT INTO products (title, description, embedding_vector) VALUES
('no-vector item', 'this item has no embedding', ());
```

### 手动浮点向量用法

<!-- example for creating float_vector -->
另外，您可以使用手动计算的浮点向量。

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

## 多值整数（MVA）

<!-- example for creating MVA32 -->

多值属性允许存储可变长度的32位无符号整数列表。这适用于存储一对多的数值，如标签、产品类别和属性。

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
它支持过滤和聚合，但不支持排序。过滤可以使用一个条件，该条件要求至少有一个元素通过（使用 [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)）或者所有元素（[ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)）都通过。


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

可以提取诸如[最小值](../Functions/Mathematical_functions.md#LEAST%28%29)或[最大值](../Functions/Mathematical_functions.md#GREATEST%28%29)元素以及列表长度等信息。示例展示了如何按多值属性的最小元素进行排序。

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
当按多值属性分组时，文档将根据该文档关联的不同值贡献到多个分组中。例如，如果一个集合中恰好有一个文档，其‘product_codes’多值属性具有值5、7和11，那么基于‘product_codes’的分组将产生3个分组，`COUNT(*)`均为1，`GROUPBY()`键值分别为5、7和11。此外，请注意按多值属性分组可能导致结果集中出现重复文档，因为每个文档可以参与多个分组。

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
插入为多值属性的数值的顺序不会被保留。值在内部以排序集合的形式存储。

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


## 多值大整数

<!-- example for creating MVA64 -->

一种允许存储可变长度64位有符号整数列表的数据类型。它具有与多值整数相同的功能。

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

## 列存属性属性

当使用列式存储时，可以为属性指定以下属性。

<!-- example fast_fetch -->
### fast_fetch

默认情况下，Manticore 列存储以列式方式存储所有属性，同时以特殊的文档存储行逐行存储。这使得查询如 `SELECT * FROM ...` 的执行非常快速，尤其是在一次获取大量记录时。然而，如果您确定不需要此功能或者希望节省磁盘空间，可以通过在创建表时指定 `fast_fetch='0'` 或者（如果在配置中定义表）使用 `columnar_no_fast_fetch` 来禁用，如下面示例所示。

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

