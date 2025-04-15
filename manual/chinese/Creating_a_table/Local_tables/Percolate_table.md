# 渗透表

<!-- example pq -->
渗透表是存储查询而不是文档的特殊表。用于前瞻性搜索，或称为“逆向搜索”。

* 要了解如何对渗透表执行搜索查询，请参见[渗透查询](../../Searching/Percolate_query.md)部分。
* 要了解如何准备表以进行搜索，请参见[向渗透表添加规则](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)部分。

渗透表的模式是固定的，并包含以下字段：

| 字段 | 描述 |
| - | - |
| ID| 一个带有自增功能的无符号64位整数。在添加PQ规则时可以省略，如[添加PQ规则](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)中所述 |
| 查询 | 规则的[全文查询](../../Searching/Full_text_matching/Basic_usage.md)，可以看作[匹配子句](../../Searching/Full_text_matching/Basic_usage.md)或[JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)的值。如果在查询中使用了[每字段操作符](../../Searching/Full_text_matching/Operators.md)，则需要在渗透表配置中声明全文字段。如果存储的查询仅用于属性过滤（没有全文查询），则查询值可以为空或省略。此字段的值应与创建渗透表时指定的预期文档模式相对应。 |
| 过滤器 | 可选。过滤器是一个可选字符串，包含属性过滤器和/或表达式，定义方式与[WHERE子句](../../Searching/Filters.md#WHERE)或[JSON过滤](../../Searching/Filters.md#HTTP-JSON)相同。此字段的值应与创建渗透表时指定的预期文档模式相对应。 |
| 标签 | 可选。标签表示由逗号分隔的字符串标签列表，可用于过滤/删除PQ规则。当执行[渗透查询](../../Searching/Percolate_query.md)时，标签也可以与匹配的文档一起返回。 |

请注意，创建渗透表时不需要添加上述字段。

创建新渗透表时需要记住的是指定文档的预期模式，这将与您稍后添加的规则进行检查。做到这一点与[任何其他本地表](../../Creating_a_table/Local_tables.md)的方式相同。


<!-- intro -->
##### 通过SQL创建渗透表：

<!-- request SQL -->

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### 通过HTTP上的JSON创建渗透表：

<!-- request JSON -->

```json
POST /cli -d "CREATE TABLE products(title text, meta json) type='pq'"
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### 通过PHP客户端创建渗透表：

<!-- request PHP -->

```php
$index = [
    'table' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'meta' => ['type' => 'json']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);
```
<!-- response PHP -->
```php
Array(
    [total] => 0
    [error] =>
    [warning] =>
)
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, meta json) type=\'pq\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, meta json) type=\'pq\'');
```
<!-- intro -->
##### Java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### TypeScript:

<!-- request typescript -->

```typescript
res = await utilsApi.sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### Go:

<!-- request go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, meta json) type='pq'").Execute()
```

<!-- intro -->
##### 通过配置创建渗透表：

<!-- request CONFIG -->

```ini
table products {
  type = percolate
  path = tbl_pq
  rt_field = title
  rt_attr_json = meta
}
```
<!-- end -->
<!-- proofread -->
