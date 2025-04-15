# Перколяционная таблица

<!-- example pq -->
Перколяционная таблица - это специальная таблица, которая хранит запросы, а не документы. Она используется для перспективных поисков или "поиска в обратном направлении."

* Чтобы узнать больше о выполнении поискового запроса к перколяционной таблице, см. раздел [Перколяционный запрос](../../Searching/Percolate_query.md).
* Чтобы узнать, как подготовить таблицу для поиска, см. раздел [Добавление правил в перколяционную таблицу](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md).

Схема перколяционной таблицы фиксирована и содержит следующие поля:

| Поле | Описание |
| - | - |
| ID| Беззнаковое 64-битное целое число с функцией автоинкремента. Его можно опустить при добавлении правила PQ, как описано в [добавление правила PQ](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) |
| Запрос | [Полнотекстовый запрос](../../Searching/Full_text_matching/Basic_usage.md) правила, который можно рассматривать как значение [условия MATCH](../../Searching/Full_text_matching/Basic_usage.md) или [JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). Если используются [операторы per field](../../Searching/Full_text_matching/Operators.md) внутри запроса, полнотекстовые поля должны быть объявлены в конфигурации перколяционной таблицы. Если сохраняемый запрос предназначен только для фильтрации атрибутов (без полнотекстового запроса), значение запроса может быть пустым или опущенным. Значение этого поля должно соответствовать ожидаемой схеме документа, которая указывается при создании перколяционной таблицы. |
| Фильтры | Необязательно. Фильтры представляют собой необязательную строку, содержащую фильтры атрибутов и/или выражения, определенные так же, как и в [условии WHERE](../../Searching/Filters.md#WHERE) или [JSON-фильтрации](../../Searching/Filters.md#HTTP-JSON). Значение этого поля должно соответствовать ожидаемой схеме документа, которая указывается при создании перколяционной таблицы. |
| Теги | Необязательно. Теги представляют собой список строковых меток, разделенных запятыми, которые могут использоваться для фильтрации/удаления правил PQ. Теги также могут быть возвращены вместе с совпадающими документами при выполнении [перколяционного запроса](../../Searching/Percolate_query.md) |

Обратите внимание, что вам не нужно добавлять вышеуказанные поля при создании перколяционной таблицы.

Что вам нужно помнить при создании новой перколяционной таблицы, так это указать ожидаемую схему документа, которая будет проверяться по отношению к правилам, которые вы добавите позже. Это делается так же, как и для [любой другой локальной таблицы](../../Creating_a_table/Local_tables.md).


<!-- intro -->
##### Создание перколяционной таблицы через SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### Создание перколяционной таблицы через JSON по HTTP:

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
##### Создание перколяционной таблицы через PHP-клиент:

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
##### Создание перколяционной таблицы через конфигурацию:

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
