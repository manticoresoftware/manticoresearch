# 透析表

<!-- example pq -->透析表是一种特殊的表，用于存储查询而非文档。它用于前瞻性搜索或“反向搜索”。

- 要了解如何对透析表执行搜索查询，请参见[透析查询](../../Searching/Percolate_query.md)部分。
- 要了解如何准备表以进行搜索，请参见[向透析表添加规则](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)部分。

透析表的模式是固定的，包含以下字段：

| 字段    | 描述                                                         |
| ------- | ------------------------------------------------------------ |
| ID      | 一个带有自动递增功能的无符号 64 位整数。添加 PQ 规则时可以省略，如[添加 PQ 规则](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)所述。 |
| Query   | 规则的[全文查询](../../Searching/Full_text_matching/Basic_usage.md)，可以视为[MATCH 子句](../../Searching/Full_text_matching/Basic_usage.md)或[JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)的值。如果在查询中使用了[按字段操作符](../../Searching/Full_text_matching/Operators.md)，则需要在透析表配置中声明全文字段。如果存储的查询仅用于属性过滤（没有全文查询），则查询值可以为空或省略。该字段的值应与创建透析表时指定的预期文档模式相对应。 |
| Filters | 可选。过滤器是一个可选字符串，包含属性过滤器和/或表达式，定义方式与[WHERE 子句](../../Searching/Filters.md#WHERE)或[JSON 过滤](../../Searching/Filters.md#HTTP-JSON)相同。该字段的值应与创建透析表时指定的预期文档模式相对应。 |
| Tags    | 可选。标签表示以逗号分隔的字符串标签列表，可用于过滤/删除 PQ 规则。当执行[透析查询](../../Searching/Percolate_query.md)时，标签也可以与匹配的文档一起返回。 |

请注意，在创建透析表时，您不需要添加上述字段。

创建新透析表时，需要记住的是指定文档的预期模式，该模式将在您稍后添加的规则中进行检查。此过程与[其他本地表](../../Creating_a_table/Local_tables.md)的创建方式相同。


<!-- intro -->
##### 通过 SQL 创建透析表：

<!-- request SQL -->

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### 通过 HTTP 使用 JSON 创建透析表：

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
##### 通过PHP客户端创建透析表：

<!-- request PHP -->

```php
$index = [
    'index' => 'products',
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
##### java:

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
##### 通过配置文件创建透析表

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
