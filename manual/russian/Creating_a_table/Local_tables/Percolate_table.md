# Percolate table

<!-- example pq -->
Перколят таблица — это специальная таблица, которая хранит запросы, а не документы. Она используется для перспективного поиска, или "поиска в обратном направлении."

* Чтобы узнать больше о выполнении поискового запроса по перколят таблице, смотрите раздел [Percolate query](../../Searching/Percolate_query.md).
* Чтобы узнать, как подготовить таблицу для поиска, смотрите раздел [Adding rules to a percolate table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md).

Схема перколят таблицы фиксирована и содержит следующие поля:

| Field | Description |
| - | - |
| ID| Беззнаковое 64-битное целое число с функцией автоинкремента. Его можно опустить при добавлении правила PQ, как описано в [add a PQ rule](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) |
| Query | [Полнотекстовый запрос](../../Searching/Full_text_matching/Basic_usage.md) правила, который можно рассматривать как значение [MATCH clause](../../Searching/Full_text_matching/Basic_usage.md) или [JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). Если в запросе используются [операторы по полям](../../Searching/Full_text_matching/Operators.md), полнотекстовые поля необходимо объявить в конфигурации перколят таблицы. Если хранимый запрос предназначен только для фильтрации атрибутов (без полнотекстового поиска), значение запроса может быть пустым или опущенным. Значение этого поля должно соответствовать ожидаемой схеме документа, которая указывается при создании перколят таблицы. |
| Filters | Опционально. Фильтры — это необязательная строка, содержащая фильтры атрибутов и/или выражения, определённые так же, как в [WHERE clause](../../Searching/Filters.md#WHERE) или [JSON filtering](../../Searching/Filters.md#HTTP-JSON). Значение этого поля должно соответствовать ожидаемой схеме документа, которая указывается при создании перколят таблицы. |
| Tags | Опционально. Теги представляют собой список строковых меток, разделённых запятыми, которые могут использоваться для фильтрации/удаления правил PQ. Теги также могут возвращаться вместе с совпадающими документами при выполнении [Percolate query](../../Searching/Percolate_query.md) |

Обратите внимание, что перечисленные выше поля не нужно добавлять при создании перколят таблицы.

Что важно помнить при создании новой перколят таблицы — это указать ожидаемую схему документа, которая будет проверяться относительно добавляемых правил. Это делается так же, как и для [любой другой локальной таблицы](../../Creating_a_table/Local_tables.md).


<!-- intro -->
##### Создание перколят таблицы через SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### Создание перколят таблицы через JSON по HTTP:

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
##### Создание перколят таблицы через PHP клиент:

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
##### Создание перколят таблицы через конфигурацию:

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

