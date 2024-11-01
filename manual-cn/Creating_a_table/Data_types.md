# 数据类型

## 全文字段和属性

Manticore 的数据类型可以分为两类：全文字段和属性。

### 全文字段

全文字段：
* 可以使用自然语言处理算法进行索引，因此可以按关键词进行搜索。
* 不能用于排序或分组。
* 可以检索原始文档的内容。
* 可以用于高亮显示原始文档的内容。

全文字段由数据类型 text 表示。所有其他数据类型称为“属性”。

### 属性

属性是与每个文档相关联的非全文值，可以在搜索过程中用于非全文的过滤、排序和分组。

通常希望基于匹配的文档ID及其排名之外的其他值来处理全文搜索结果。例如，可能需要按日期然后按相关性对新闻搜索结果进行排序，或者在指定的价格范围内搜索产品，或将博客搜索限制为特定用户发布的帖子，或者按月份对结果进行分组。为了高效地实现这些操作，Manticore 允许为每个文档添加不仅限于全文字段的附加属性。这些属性可以用于过滤、排序或分组全文匹配结果，甚至可以仅通过属性进行搜索。

与全文字段不同，属性并没有进行全文索引。它们存储在表中，但无法像全文字段那样进行搜索。

<!-- example attributes or fields -->

属性的一个典型示例是论坛帖子表。假设只有 title（标题）和 content（内容）字段需要进行全文搜索，但有时还需要将搜索限制在特定的作者或子论坛内（即仅搜索那些 author_id 或 forum_id 具有特定值的行）；或者按 post_date 列对匹配结果进行排序；或者按 post_date 的月份对匹配帖子进行分组，并计算每个分组的匹配数量。

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

此示例展示了一个全文查询，同时根据 author_id 和 forum_id 进行过滤，并按 post_date 排序。

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
  "index": "forum",
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
        'index' => 'forum',
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
searchApi.search({"index":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]});
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

<!-- end -->

### 行存储和列存储属性

Manticore 支持两种属性存储类型：
* 行存储 - Manticore Search 默认提供的传统存储方式
* 列存储 - 由 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 提供

从它们的名称可以看出，它们以不同的方式存储数据。传统的**行存储**：
* 以未压缩的方式存储属性
* 同一文档的所有属性存储在一个行中，彼此靠近
* 行一个接一个地存储
* 访问属性的方式基本上是通过将行 ID 乘以步长（单个向量的长度），并从计算出的内存位置获取所请求的属性。这使得随机访问的延迟非常低。
* 属性必须存储在内存中以获得可接受的性能，否则，由于存储的行存储特性，Manticore 可能需要从磁盘读取过多不需要的数据，这在许多情况下效率不佳。

对于**列存储**：
* 每个属性独立于其他属性存储在它自己的“列”中
* 存储被分为65536条数据的块
* 块以压缩形式存储。这通常允许只存储少量不同的值，而不是像行存储那样存储所有值。高压缩率使得从磁盘读取更快，并且降低了内存需求
* 当数据被索引时，每个块会独立选择存储方案。例如，如果一个块中的所有值都相同，它会使用“常量”存储，只为整个块存储一个值。如果每块的唯一值少于256个，则使用“表”存储，将值的索引存储到一个值表中，而不是存储实际的值
* 如果确定请求的值不在某个块中，搜索可以提前排除该块。

列存储旨在处理无法完全存入RAM的大数据量，因此建议：
* 如果你有足够的内存存储所有属性，使用行存储会更有优势
* 否则，列存储仍然能在较低的内存占用下提供不错的性能，这将允许你在表中存储更多文档

### 如何在存储方式之间切换

传统的行存储是默认选项，因此如果你希望所有数据都以行存储方式存储，创建表时无需做任何操作。

要启用列存储，你需要：
* 在[CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE)中指定engine='columnar'，以使表的所有属性采用列存储。如果你想让某个特定属性保持行存储，则在声明该属性时需要添加engine='rowwise'。例如：
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* 在 CREATE TABLE 中为特定属性指定 engine='columnar' 以将其设置为列式存储。例如：
```sql
create table tbl(title text, type int, price float engine='columnar');
```
或者
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* 在[plain模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)中，你需要在[columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)中列出你希望作为列式存储的属性。


以下是Manticore Search支持的数据类型列表：

## 文档 ID（Document ID）

<!-- example id -->
文档标识符是一个必需的属性，文档ID必须是**唯一的64位无符号整数**。文档ID可以显式指定，但即使不指定，它们仍然是启用的。文档ID不能被更新。请注意，当检索文档ID时，它们被视为有符号的64位整数，这意味着它们可能为负数。如有必要，可以使用[UINT64()](Functions/Type_casting_functions.md#UINT64())函数将它们转换为无符号64位整数。

<!-- request Explicit ID -->

在创建表时，你可以显式指定ID，但无论使用什么数据类型，它始终会如上所述存储为一个有符号的64位整数。

```sql
CREATE TABLE tbl(id bigint, content text);
DESC tbl;
```

<!-- response Explicit ID -->
```sql
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec)
```

<!-- request Implicit ID -->

你也可以完全省略指定ID，它会自动启用。

```sql
CREATE TABLE tbl(content text);
DESC tbl;
```

<!-- response Implicit ID -->

```sql
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec)
```

<!-- end -->

## 字符数据类型

通用语法：
```
string|text [stored|attribute] [indexed]
```

**属性:**

1. `indexed` - 全文索引（可以用于全文查询）
2. `stored` - 存储在docstore中（存储在磁盘上，不在RAM中，延迟读取）
3. `attribute` - 使其成为字符串属性（可以按其排序/分组）

指定至少一个属性会覆盖所有默认属性（见下文），也就是说，如果你决定使用自定义的属性组合，你需要列出所有你想要的属性。

**未指定属性：**

string和text是别名，但如果你不指定任何属性，它们默认的含义不同：

* 仅string默认意味着attribute（详见[下文](../Creating_a_table/Data_types.md#Text)）。
* 仅text默认意味着stored + indexed（详见[下文](../Creating_a_table/Data_types.md#String)）。

### 文本 (Text)

<!-- example working with text -->

文本数据类型（仅text或text/string indexed）构成表的全文部分。文本字段会被索引，可以用于关键词搜索。

文本通过一个分析管道处理，该管道将文本转换为词汇，应用词法转换等。最终，从该文本构建了一个全文表（一个特殊的数据结构，用于快速搜索关键词）。

全文字段只能在MATCH()子句中使用，不能用于排序或聚合。单词存储在倒排索引中，并与其所属字段及在字段中的位置相关联。这允许在每个字段中搜索单词并使用高级操作符，如邻近搜索。默认情况下，字段的原始文本既被索引，也存储在文档存储中。这意味着原始文本可以与查询结果一起返回，并可用于[搜索结果高亮](../Searching/Highlighting.md)。

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

This behavior can be overridden by explicitly specifying that the text is only indexed.

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

字段是有名称的，你可以将搜索限定在单个字段中（例如仅搜索“title”字段），或者限定在字段的一个子集内（例如仅搜索“title”和“abstract”字段）。你最多可以使用256个全文字段。

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
	"index": "products",
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
searchApi.search({"index":"products","query":{"match":{"title":"first"}}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match":{"title":"first"}}});
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

<!-- end -->

### 字符串（String）

<!-- example for string attributes  -->

与全文字段不同，字符串属性（只使用 string 或 string/text attribute）按收到的原样存储，不能用于全文搜索。相反，它们可以在结果中返回，可用于 WHERE 子句中的比较过滤或 REGEX，并可用于排序和聚合。通常不建议将大文本存储在字符串属性中，但字符串属性适合用于存储元数据，例如名称、标题、标签、键。

如果你还想对字符串属性进行索引，可以指定为 string attribute indexed。这样既可以进行全文搜索，也可以作为属性使用。

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
<summary>More</summary>
<!-- example string field -->

你可以创建一个同时存储为字符串属性的全文字段。这种方法会创建一个具有相同名称的全文字段和字符串属性。需要注意的是，不能同时将 stored 属性添加到一个字段，以便既作为字符串属性存储又同时存储在文档存储中。

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` 意味着我们正在处理一个字符串数据类型，它作为属性存储，并被索引为全文字段。

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

Manticore 没有专门的二进制数据字段类型，但你可以通过使用 base64 编码，并结合 `text stored` 或 `string stored` 字段类型（它们是同义词），安全地存储二进制数据。如果不对二进制数据进行编码，部分数据可能会丢失——例如，当 Manticore 遇到空字节（null-byte）时，会截断字符串的末尾。



下面是一个示例，其中我们使用 base64 对 ls 命令进行编码，将其存储在 Manticore 中，然后解码以验证 MD5 校验和保持不变：

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

## 整数（Integer）

<!-- example for integers  -->

整数类型允许存储 32 位的**无符号**整数值。

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

整数可以通过指定位数存储为比32位更短的大小。例如，如果我们知道要存储的数值不会大于8，则可以将类型定义为 bit(3)。位数较少的整数比全尺寸整数的执行速度要慢，但它们需要更少的内存。它们以32位块保存，因此为了节省空间，应该将它们放在属性定义的末尾进行分组（否则位数较少的整数位于两个全尺寸整数之间时仍会占用32位）。

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

## 大整数（Big Integer）

<!-- example for bigints  -->

Big integers (bigint) are 64-bit wide **signed** integers.

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

## 布尔值（Boolean）

<!-- example for boolean  -->

声明布尔属性，它等价于位数为1的整数属性。

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

## 时间戳（Timestamp）

<!-- example for timestamps  -->

时间戳类型表示Unix时间戳，存储为32位整数。与基本整数不同，时间戳类型允许使用[时间和日期](../Functions/Date_and_time_functions.md)函数。字符串值的转换遵循以下规则：

无分隔符的数字，至少10个字符长，按原样转换为时间戳。

- %Y-%m-%dT%H:%M:%E*S%Z

- %Y-%m-%d'T'%H:%M:%S%Z

- %Y-%m-%dT%H:%M:%E*S

- %Y-%m-%dT%H:%M:%s

- %Y-%m-%dT%H:%M

- %Y-%m-%dT%H

- %Y-%m-%d

- %Y-%m

- %Y




这些转换格式的含义可以在[strptime手册](https://man7.org/linux/man-pages/man3/strptime.3.html)中找到，除了 %E*S，它代表毫秒。



请注意，自动转换时间戳在普通表中不受支持。

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

## 浮点数（Float）

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

与整数类型不同，不推荐直接比较两个浮点数是否相等，因为可能存在舍入误差。更可靠的方法是使用近似比较，通过检查绝对误差范围。

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
  "index": "products",
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
searchApi.search({"index":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"eps":"abs(a-b)"}});
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
<!-- end -->

<!-- example for float mul -->

另一种方法是将浮点数转换为整数进行比较，这可以用于执行 IN(attr,val1,val2,val3) 操作。你可以选择一个乘数因子，将浮点数转换为整数来进行操作。下面的例子展示了如何将 IN(attr,2.0,2.5,3.5) 修改为与整数值一起使用。

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
  "index": "products",
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
searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}});
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
<!-- end -->

<!-- example float_accuracy -->

Manticore中的浮点数显示时具有精度，以确保它们准确反映存储的值。这种方法是为了防止精度丢失，尤其是在处理地理坐标等场景时，舍入到小数点后6位可能导致不准确。

现在，Manticore首先以6位小数的形式输出数字，然后解析并与原始值进行比较。如果它们不匹配，则会继续增加小数位，直到匹配为止。

例如，如果插入的浮点值是 19.45，Manticore 将显示为 19.450001 以准确表示存储的值。

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

该数据类型允许存储JSON对象，这对于存储无模式数据非常有用。然而，JSON数据类型不支持列式存储。但它可以存储在传统存储中，因为可以在同一张表中结合使用这两种存储类型。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, data json)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql'CREATE TABLE products(title text, data json)');
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql'CREATE TABLE products(title text, data json)');
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

JSON属性可以用于大多数操作中。此外，还有一些特殊函数，如 [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL())、[ANY()](../Functions/Arrays_and_conditions_functions.md#ANY())、[GREATEST()](../Functions/Mathematical_functions.md#GREATEST())、[LEAST()](../Functions/Mathematical_functions.md#LEAST()) 和 [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF())，它们允许遍历属性数组。

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
  "index": "products",
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
searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}});
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

<!-- end -->

<!-- example for REGEX() json -->

Text properties are treated the same as strings, so it's not possible to use them in full-text match expressions. However, string functions such as [REGEX()](../Functions/String_functions.md#REGEX%28%29) can be used.

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
  "index": "products",
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
searchApi.search({"index":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}});
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
<!-- end -->

<!-- example for DOUBLE() -->

在处理JSON属性时，某些情况下可能需要强制指定数据类型以确保功能正常。例如，在处理浮点值时，必须使用 [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE()) 来确保正确排序。

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
  "index": "products",
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
searchApi.search({"index":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]});
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

<!-- end -->

## 浮点向量（Float vector）

<!-- example for creating float_vector -->

浮点向量属性允许存储可变长度的浮点数列表。需要注意的是，这个概念与多值属性不同。多值属性（MVAs）本质上是集合，不保留值的顺序，且不保留重复值。相比之下，浮点向量在插入时对值不进行任何额外处理。

浮点向量属性可以用于k最近邻（k-nearest neighbor）搜索；请参阅 [KNN 搜索](../Searching/KNN.md)。

** 目前，float_vector 字段只能在实时表中用于 KNN 搜索，不支持在其他函数或表达式中使用，也不支持普通表。 **

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

## 多值整数（Multi-value integer 、MVA）

<!-- example for creating MVA32 -->

多值属性允许存储可变长度的 32 位无符号整数列表。这对于存储一对多的数值信息非常有用，例如标签、产品类别和属性。

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
它支持过滤和聚合，但不支持排序。可以使用至少一个元素通过的条件进行过滤（使用 [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY())）或所有元素都通过的条件（使用 [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL())）。


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
  "index": "products",
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
searchApi.search({"index":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})'
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
<!-- end -->

<!-- example for least/greatest MVA -->

可以提取列表中  [least](../Functions/Mathematical_functions.md#LEAST%28%29) 或 [greatest](../Functions/Mathematical_functions.md#GREATEST%28%29) 元素以及列表的长度。以下示例展示了如何按多值属性的最小元素排序。

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
  "index": "products",
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
searchApi.search({"index":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}});
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
searchResponse = searchApi.search(searchRequest);
```

<!-- end -->

<!-- example for grouping by MVA -->
当按多值属性进行分组时，一个文档将根据与其相关的不同值贡献给多个组。例如，如果一个集合中有一个文档的 ‘product_codes’ 多值属性包含值 5、7 和 11，那么按 ‘product_codes’ 进行分组将生成 3 个组，COUNT(*) 等于 1，GROUPBY() 键值分别为 5、7 和 11。此外，注意按多值属性分组可能会导致结果集中出现重复的文档，因为每个文档可以参与多个组。

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
插入为多值属性的数字顺序不会被保留。这些值在内部会被存储为排序后的集合。

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
	"index":"products",
	"id":1,
	"doc":
	{
		"title":"first",
		"product_codes":[4,2,1,3]
	}
}

POST /search
{
  "index": "products",
  "query": { "match_all": {} }
}
```

<!-- response JSON -->

```JSON
{
   "_index":"products",
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
indexApi.insert({"index":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
searchApi.search({"index":"products","query":{"match_all":{}}})
```
<!-- response Python -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'index': 'products',
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
await indexApi.insert({"index":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}});
res = await searchApi.search({"index":"products","query":{"match_all":{}}});
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

<!-- end -->


## 多值大整数（Multi-value big integer）

<!-- example for creating MVA64 -->

一种允许存储可变长度的 64 位有符号整数列表的数据类型。它的功能与多值整数相同。

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

## 列式属性的属性

When you use the columnar storage you can specify the following properties for the attributes.

<!-- example fast_fetch -->
### fast_fetch

默认情况下，Manticore 列式存储将所有属性以列的方式存储，同时也会以逐行的方式存储在一个特殊的 docstore 中。这使得像 SELECT * FROM ... 这样的查询可以快速执行，特别是在一次获取大量记录时。然而，如果您确定不需要此功能，或者希望节省磁盘空间，可以通过在创建表时指定 fast_fetch='0' 来禁用它，或者（如果您在配置中定义表）通过使用 columnar_no_fast_fetch 来实现，如以下示例所示。

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
<!-- proofread -->
