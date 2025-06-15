# 数据类型

## 全文字段和属性

Manticore 的数据类型可以分为两类：全文字段和属性。

### 字段名称语法

Manticore 中的字段名称必须遵循以下规则：

* 可以包含字母（a-z，A-Z）、数字（0-9）和连字符（-）
* 必须以字母开头
* 数字只能出现在字母之后
* 下划线（`_`）是唯一允许的特殊字符
* 字段名称不区分大小写

例如：
* 有效的字段名称：`title`，`product_id`，`user_name_2`
* 无效的字段名称：`2title`，`-price`，`user@name`

### 全文字段

全文字段：
* 可以通过自然语言处理算法进行索引，因此可以搜索关键词
* 不能用于排序或分组
* 可以检索原始文档内容
* 可以用于高亮显示原始文档内容

全文字段由数据类型 `text` 表示。所有其他数据类型称为“属性”。

### 属性

属性是与每个文档关联的非全文值，可用于在搜索期间执行非全文过滤、排序和分组。

通常希望基于匹配的文档 ID 及其排名之外的一些其他每文档值处理全文搜索结果。例如，可能需要按日期和相关性排序新闻搜索结果，或搜索指定价格范围内的产品，或限制博客搜索仅限选定用户发布的帖子，或按月份分组结果。为此，Manticore 不仅允许添加全文字段，还允许为每个文档添加额外属性。这些属性可用于过滤、排序或分组全文匹配，或仅通过属性进行搜索。

与全文字段不同，属性没有全文索引。它们存储在表中，但不能作为全文搜索。

<!-- example attributes or fields -->

属性的一个很好的例子是论坛帖子表。假设只有标题和内容字段需要全文搜索——但有时还需要限制搜索指定作者或子论坛（即仅搜索具有某些特定 author_id 或 forum_id 值的行）；或按 post_date 列排序匹配结果；或按 post_date 的月份分组匹配帖子并计算每组匹配数。

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

此示例展示了按 `author_id`、`forum_id` 过滤并按 `post_date` 排序的全文查询。

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

### 行式和列式属性存储

Manticore 支持两种属性存储类型：
* 行式 —— Manticore Search 默认支持的传统存储
* 列式 —— 由 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 提供

顾名思义，它们存储数据的方式不同。传统的**行式存储**：
* 以未压缩形式存储属性
* 同一文档的所有属性存储在一行中，彼此相邻
* 行按顺序存储
* 访问属性时，基本通过将行 ID 乘以跨度（单个向量长度），再从计算出的内存位置获取所需属性，实现非常低的随机访问延迟
* 属性需在内存中才能获得可接受的性能，否则由于行式存储的特性，Manticore 可能需要从磁盘读取大量不必要的数据，这在许多情况下不够优化

而**列式存储**：
* 各属性独立存储在它自己的“列”中
* 存储划分为 65536 条目的块
* 块以压缩形式存储。通常这允许只存少数不同值，而不是像行式存储那样存所有值。高压缩率使得从磁盘读取更快，内存需求也大大降低
* 数据索引时，每个块独立选择存储方案。例如，如果一个块内所有值相同，就采用“const”存储，该块只存储一个值。如果一个块中不同值少于 256 个，则采用“table”存储，存储的是指向值表的索引，而不是值本身
* 如果确定请求值不在某块中，可以提前排除此块的搜索

列式存储设计用来处理不能全部放入 RAM 的大数据量，因此推荐如下：
* 如果内存足够容纳所有属性，使用行式存储会受益更多
* 否则，列式存储仍能提供不错性能且内存占用更低，可以让你在表中存储更多文档

### 如何在存储方式间切换

传统的行存储是默认的，因此如果你希望所有内容都以行存储方式保存，创建表时无需做任何操作。

要启用列存储，你需要：
* 在[CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE)中指定`engine='columnar'`，以使表的所有属性都成为列存储。然后，如果你想保持某个特定属性为行存储，需要在声明该属性时添加`engine='rowwise'`。例如：
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* 在`CREATE TABLE`中为特定属性指定`engine='columnar'`，使其成为列存储。例如：
```sql
create table tbl(title text, type int, price float engine='columnar');
```
或
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* 在[plain模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)下，你需要在[columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)中列出你想要列存储的属性。


以下是 Manticore Search 支持的数据类型列表：

## 文档ID

文档标识符是一个必填属性，必须是唯一的64位无符号整数。创建表时可以显式指定文档ID，但即使不指定，文档ID始终启用。文档ID不能被更新。

创建表时你可以显式指定ID，但无论使用哪种数据类型，它总会按上述所描述的方式表现——作为64位无符号数存储，但以64位有符号整数形式暴露。

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

你也可以完全不指定ID，它会自动启用。
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

在处理文档ID时，需要了解它们内部作为64位无符号整数存储，但不同接口的处理方式不同：

**MySQL/SQL接口：**
* 大于2^63-1的ID会显示为负数。
* 使用这些较大ID过滤时，必须使用其有符号表示。
* 使用[UINT64()](../Functions/Type_casting_functions.md#UINT64%28%29)函数查看其实际的无符号值。

**JSON/HTTP接口：**
* ID始终显示为其原始的无符号值，无论其大小如何。
* 筛选时可以使用有符号和无符号两种表示。
* 插入操作接受完整的无符号64位范围。

例如，创建一个表并插入一些围绕2^63的值：
```sql
mysql> create table t(id_text string);
Query OK, 0 rows affected (0.01 sec)

mysql> insert into t values(9223372036854775807, '2 ^ 63 - 1'),(9223372036854775808, '2 ^ 63');
Query OK, 2 rows affected (0.00 sec)
```

由于一些ID超过2^63-1，它们在结果中显示为负数。但使用 `UINT64(id)` 可以显示它们的实际无符号值：
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

查询ID小于2^63的文档时，可以直接使用无符号值：
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

但对于ID从2^63开始的文档，需要使用有符号值：
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

若使用无符号值，会报错：
```sql
mysql> select * from t where id = 9223372036854775808;
ERROR 1064 (42000): number 9223372036854775808 is out of range [-9223372036854775808..9223372036854775807]
```

超出64位的值会触发类似的错误：

```sql
mysql> select * from t where id = -9223372036854775809;
ERROR 1064 (42000): number -9223372036854775809 is out of range [-9223372036854775808..9223372036854775807]
```

### 大ID的接口差异

MySQL/SQL和JSON/HTTP接口在处理非常大的文档ID时表现出更明显的差异。以下是一个完整示例：

**MySQL/SQL接口：**
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

**JSON/HTTP接口：**
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

这意味着处理大型文档ID时：
1. **MySQL接口** 查询时需使用有符号表示，但可用 `UINT64()` 显示无符号值
2. **JSON接口** 显示始终使用无符号值，筛选时接受两种表示

## 字符数据类型

通用语法：
```
string|text [stored|attribute] [indexed]
```

**属性：**

1. `indexed` - 全文索引（可用于全文查询）
2. `stored` - 存储在文档存储区（存于磁盘，非RAM，懒加载）
3. `attribute` - 作为字符串属性（可排序/分组）

指定至少一个属性会覆盖所有默认属性（详见下文），即如果你决定使用自定义属性组合，你需要列出所有需要的属性。

**未指定任何属性时：**

`string`和`text`是别名，但如果不指定任何属性，它们默认代表不同含义：

* 仅`string`默认表示`attribute`（详情见[下文](../Creating_a_table/Data_types.md#Text)）。
* 仅`text`默认表示`stored` + `indexed`（详情见[下文](../Creating_a_table/Data_types.md#String)）。

### Text

<!-- example working with text -->

text（仅`text`或`text/string indexed`）数据类型构成表的全文部分。文本字段被索引，可搜索关键字。

文本通过分析器管道转换成词语，应用形态变换等。最终，从文本构建全文表（一种启用关键词快速搜索的特殊数据结构）。

全文字段只能用于 `MATCH()` 子句，不能用于排序或聚合。词语存储在倒排索引中，附带字段引用及字段中的位置。这允许在各字段中搜索某词，并使用高级操作符如邻近。默认情况下，字段的原始文本既被索引也存储于文档存储区。意味着查询结果可以返回原始文本，并用于[搜索结果高亮](../Searching/Highlighting.md)。

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

字段被命名，您可以将搜索限制在单个字段（例如仅搜索“title”）或字段子集（例如仅“title”和“abstract”）。您最多可以拥有256个全文字段。

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

与全文字段不同，字符串属性（仅 `string` 或 `string/text attribute`）以接收时的形式存储，不可用于全文搜索。相反，它们会在结果中返回，可以用于 `WHERE` 子句中的比较过滤或 `REGEX`，并可用于排序和聚合。通常，不建议在字符串属性中存储大量文本，而是将字符串属性用于元数据，如名称、标题、标签、键。

如果您还想索引字符串属性，可以同时指定为 `string attribute indexed`。这样既允许全文搜索，又作为属性使用。

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

您可以创建一个既是全文字段又作为字符串属性存储的字段。这种方法创建了一个同名的全文字段和字符串属性。请注意，您不能同时添加 `stored` 属性来将数据作为字符串属性存储在文档存储中。

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` 表示我们正在处理作为属性存储并作为全文字段索引的字符串数据类型。

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

Manticore 没有专门的二进制数据字段类型，但您可以使用 base64 编码以及 `text stored` 或 `string stored` 字段类型（它们是同义词）安全存储。如果您不对二进制数据进行编码，部分数据可能会丢失——例如，如果 Manticore 在字符串中遇到空字节，则会截断字符串末尾。

下面是一个示例，我们用 base64 编码 `ls` 命令，将其存储到 Manticore 中，然后解码以验证 MD5 校验和未发生变化：

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

整数可以通过指定位数来存储比32位更短的大小。例如，如果我们想存储一个我们知道不会大于8的数值，则类型可以定义为 `bit(3)`。位数计数整数的性能比全尺寸整数慢，但它们需要更少的RAM。它们以32位块进行保存，因此为了节省空间，应将它们放在属性定义的末尾（否则位数计数整数夹在两个全尺寸整数之间也将占用32位）。

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

声明一个布尔属性。它等价于一个位数为1的整数属性。

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

时间戳类型表示Unix时间戳，以32位整数存储。与基本整数不同，时间戳类型允许使用[时间和日期](../Functions/Date_and_time_functions.md)函数。从字符串值转换遵循以下规则：

- 无分隔符的数字，至少10个字符长，将按原样转换为时间戳。
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`
- `%Y-%m-%d`
- `%Y-%m`
- `%Y`

这些转换说明符的含义详见[strptime手册](https://man7.org/linux/man-pages/man3/strptime.3.html)，除 `%E*S` 表示毫秒之外。

请注意，普通表中不支持时间戳的自动转换。

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
实数以32位IEEE 754单精度浮点数存储。

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

与整数类型不同，由于可能存在四舍五入误差，不推荐直接比较两个浮点数是否相等。更可靠的方法是使用近似相等比较，即检查绝对误差范围。

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

另一种替代方法，也可以用来执行 `IN(attr,val1,val2,val3)`，是通过选择一个乘数因子将浮点数转换为整数来进行比较。下面的示例展示了如何修改 `IN(attr,2.0,2.5,3.5)` 以使用整数值进行操作。

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
Manticore 中的浮点值以高精度显示，以确保反映确切的存储值。引入这种方法是为了防止精度损失，尤其是在地理坐标等情况下，四舍五入到6位小数会导致不准确。

现在，Manticore 首先输出6位数字的数字，然后解析并与原始值进行比较。如果不匹配，将添加更多数字直到匹配为止。

例如，如果一个浮点值被插入为 `19.45`，Manticore 会显示为 `19.450001`，以准确表示存储的值。

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

这种数据类型允许存储 JSON 对象，对于处理无模式数据特别有用。定义 JSON 值时，确保包含对象的左花括号 `{` 和右花括号 `}`，或者数组的方括号 `[` 和 `]`。虽然列存储不支持 JSON，但它可以存储在传统的行存储中。值得注意的是，两种存储类型可以在同一个表中组合使用。

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

JSON 属性可以用于大多数操作。还有一些特殊函数，如 [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)、[ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)、[GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29)、[LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29) 和 [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29)，允许遍历属性数组。

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

文本属性被视为字符串，因此不能将其用于全文匹配表达式中。但可以使用字符串函数，如 [REGEX()](../Functions/String_functions.md#REGEX%28%29)。

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

对于 JSON 属性，有时需要强制数据类型以确保某些情况的正确功能。例如，处理浮点值时，必须使用 [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29) 以实现正确排序。

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
浮点向量属性允许存储可变长度的浮点数列表，主要用于机器学习应用和相似性搜索。此类型与多值属性（MVAs）在几个重要方面不同：
- 保持值的精确顺序（不像 MVA 可能会重新排序）
- 保留重复值（不像 MVA 会去重）
- 插入时无额外处理（不像 MVA 会排序和去重）

### 使用和限制
- 目前仅支持实时表
- 只能用于 KNN（k 最近邻）搜索
- 不支持普通表或其他函数/表达式
- 与 KNN 设置一起使用时，不能 `UPDATE` `float_vector` 值。应使用 `REPLACE`
- 不带 KNN 设置时，可以 `UPDATE` `float_vector` 值
- 浮点向量不能用于常规过滤器或排序
- 过滤 `float_vector` 值的唯一方式是通过向量搜索操作（KNN）

### 常见用例
- 图像嵌入用于相似性搜索
- 文本嵌入用于语义搜索
- 机器学习的特征向量
- 推荐系统向量

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

有关在搜索中使用浮点向量的信息，请参见 [KNN 搜索](../Searching/KNN.md)。

## 多值整数（MVA）

<!-- example for creating MVA32 -->

多值属性允许存储可变长度的 32 位无符号整数列表。这对于存储一对多的数值非常有用，例如标签、产品类别和属性。

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
它支持过滤和聚合，但不支持排序。过滤可以使用条件，要求至少一个元素通过（使用 [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)）或所有元素通过（[ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)）。


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

可以提取如[最小](../Functions/Mathematical_functions.md#LEAST%28%29)或[最大](../Functions/Mathematical_functions.md#GREATEST%28%29)元素及列表长度等信息。示例显示按多值属性的最小元素排序。

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
当按多值属性分组时，一个文档将贡献给该文档关联的多个不同值所对应的多个组。例如，如果集合中有且仅有一份文档，其 "product_codes" 多值属性的值分别为 5、7 和 11，则对 "product_codes" 分组将产生 3 个组，`COUNT(*)` 都为 1，`GROUPBY()` 的键值分别为 5、7 和 11。同时注意，按多值属性分组时，结果集中可能有文档重复，因为每个文档可以参与多个组。

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
多值属性作为值插入的数字顺序不会被保留。值在内部存储为排序集合。

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

一种允许存储可变长度64位有符号整数字列表的数据类型。它具有与多值整数相同的功能。

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

## 列式属性特性

当您使用列式存储时，可以为属性指定以下特性。

<!-- example fast_fetch -->
### fast_fetch

默认情况下，Manticore 列式存储将所有属性以列式方式存储，同时以特殊的文档存储（docstore）逐行存储。这使得类似 `SELECT * FROM ...` 这类查询执行快速，特别是一次获取大量记录时更为高效。然而，如果您确定不需要此功能或希望节省磁盘空间，可以在创建表时通过指定 `fast_fetch='0'` 来禁用它，或者（如果您在配置文件中定义表）通过使用 `columnar_no_fast_fetch` 实现，如以下示例所示。

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

