# 反向查询表

<!-- example pq -->
反向查询表是一种特殊的表，存储的是查询，而不是文档。它用于前瞻搜索，或称为“反向搜索”。

* 若想了解如何针对反向查询表执行搜索查询，请参阅章节 [反向查询](../../Searching/Percolate_query.md)。
* 若想了解如何准备表以进行搜索，请参阅章节 [向反向查询表添加规则](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)。

反向查询表的模式是固定的，包含以下字段：

| 字段 | 说明 |
| - | - |
| ID| 无符号 64 位整数，带有自动递增功能。添加 PQ 规则时可以省略，如 [添加 PQ 规则](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) 所述 |
| Query | 规则的[全文查询](../../Searching/Full_text_matching/Basic_usage.md)，可以理解为 [MATCH 子句](../../Searching/Full_text_matching/Basic_usage.md) 或 [JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 的值。如果在查询内使用了[按字段操作符](../../Searching/Full_text_matching/Operators.md)，则需要在反向查询表配置中声明全文字段。如果存储的查询仅用于属性过滤（不含全文查询），则查询值可以为空或省略。此字段的值应与创建反向查询表时指定的预期文档模式相对应。 |
| Filters | 可选。Filters 是包含属性过滤条件和/或表达式的可选字符串，定义方式与[WHERE 子句](../../Searching/Filters.md#WHERE)或[JSON 过滤](../../Searching/Filters.md#HTTP-JSON)相同。此字段的值应与创建反向查询表时指定的预期文档模式相对应。 |
| Tags | 可选。Tags 表示由逗号分隔的字符串标签列表，可用于过滤/删除 PQ 规则。执行[反向查询](../../Searching/Percolate_query.md)时，标签也可以与匹配的文档一起返回。 |

请注意，创建反向查询表时无需添加上述字段。

创建新反向查询表时需注意，需指定预期文档的模式，此模式将用来校验稍后添加的规则。此操作方式与[任何其他本地表](../../Creating_a_table/Local_tables.md)相同。


<!-- intro -->
##### 通过 SQL 创建反向查询表：

<!-- request SQL -->

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### 通过 JSON over HTTP 创建反向查询表：

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
##### 通过 PHP 客户端创建反向查询表：

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, meta json) type=\'pq\'')
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
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, meta json) type='pq'", Some(true)).await;
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
##### 通过配置创建反向查询表：

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

