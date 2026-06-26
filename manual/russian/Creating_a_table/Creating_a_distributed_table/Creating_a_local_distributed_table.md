# Создание локальной распределенной таблицы

<!-- example local_dist -->

Распределенная таблица в Manticore Search действует как "главный узел", который проксирует требуемый запрос к другим таблицам и предоставляет объединенные результаты из полученных ответов. Сама таблица не хранит никаких данных. Она может подключаться как к локальным таблицам, так и к таблицам, расположенным на других серверах. Локальная распределенная таблица — это просто распределенная таблица, все дочерние таблицы которой являются локальными. Если вам нужно только совместно искать по нескольким локальным таблицам, вы можете запрашивать их напрямую, без создания распределенной таблицы. Если вы все же создаете локальную распределенную таблицу, в SQL вы можете указать несколько локальных таблиц либо путем повторения `local='...'`, либо передав их в виде списка, разделенного запятыми, в одном предложении `local='index1,index2'`.


<!-- intro -->
##### Конфигурация:

<!-- request Config -->
```ini
table index_dist {
  type  = distributed
  local = index1
  local = index2
  ...
 }
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE local_dist type='distributed' local='index1' local='index2';
```

<!-- request SQL -->
```sql
CREATE TABLE local_dist type='distributed' local='index1,index2';
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'type' => 'distributed',
            'local' => [
                'index1',
                'index2'
            ]
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'", Some(true)).await;
```

<!-- end -->

<!-- example local_tables_direct_query -->
Прямой запрос к нескольким локальным таблицам работает как в SQL, так и в JSON.

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SELECT * FROM index1, index2, index3;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->
```json
POST /search
{
  "table": "index1,index2,index3",
  "query": { "match_all": {} }
}
```
<!-- end -->
