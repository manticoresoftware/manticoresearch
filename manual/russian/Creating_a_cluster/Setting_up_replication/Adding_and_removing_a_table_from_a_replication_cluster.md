# Добавление и удаление таблицы из репликационного кластера 

<!-- example adding and removing a table from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <table_name>[, <table_name>]` добавляет одну или несколько существующих локальных таблиц в кластер. Узел, который получает запрос ALTER, отправляет таблицу(ы) на другие узлы в кластере. Все локальные таблицы с тем же именем на других узлах кластера заменяются новыми таблицами.

Как только таблицы реплицированы, операции записи могут выполняться на любом узле, но имена таблиц должны начинаться с имени кластера, например `INSERT INTO <clusterName>:<table_name>`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER click_query ADD clicks_daily_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER click_query ADD clicks_daily_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'click_query',
  'body' => [
     'operation' => 'add',
     'table' => 'clicks_daily_index'
      
  ]
];
$response = $client->cluster()->alter($params);        
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER click_query ADD clicks_daily_index", Some(true)).await;
```

<!-- end -->

<!-- example adding and removing a table from a replication cluster 2 -->
`ALTER CLUSTER <cluster_name> DROP <table_name>[, <table_name>]` забывает об одной или нескольких существующих таблицах, что означает, что это не удаляет файлы таблиц на узлах, а просто делает их неактивными, нереплицируемыми таблицами.

Как только таблица удалена из кластера, она становится `локальной` таблицей, и оператора записи должны использовать только имя таблицы, как `INSERT INTO <table_name>`, без префикса кластера.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts DROP weekly_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts DROP weekly_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'drop',
     'table' => 'weekly_index'
      
  ]
];
$response = $client->cluster->alter($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts DROP weekly_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER posts DROP weekly_index')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts DROP weekly_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER posts DROP weekly_index", Some(true)).await;
```

<!-- end -->
<!-- proofread -->
# Добавление и удаление таблицы из кластера репликации 

<!-- example adding and removing a table from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <table_name>[, <table_name>]` добавляет одну или несколько существующих локальных таблиц в кластер. Узел, который получает запрос ALTER, отправляет таблицы на другие узлы в кластере. Все локальные таблицы с таким же именем на других узлах кластера заменяются новыми таблицами.

После репликации таблиц операторы записи могут выполняться на любом узле, но имена таблиц должны начинаться с имени кластера, например, `INSERT INTO <clusterName>:<table_name>`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER click_query ADD clicks_daily_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER click_query ADD clicks_daily_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'click_query',
  'body' => [
     'operation' => 'add',
     'table' => 'clicks_daily_index'
      
  ]
];
$response = $client->cluster()->alter($params);        
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- end -->

<!-- example adding and removing a table from a replication cluster 2 -->
`ALTER CLUSTER <cluster_name> DROP <table_name>[, <table_name>]` забывает о одной или нескольких существующих таблицах, что означает, что она не удаляет файлы таблиц на узлах, а просто делает их неактивными, непропагируемыми таблицами.

После удаления таблицы из кластера она становится `локальной` таблицей, а операторы записи должны использовать только имя таблицы, например, `INSERT INTO <table_name>`, без префикса кластера.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts DROP weekly_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts DROP weekly_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'drop',
     'table' => 'weekly_index'
      
  ]
];
$response = $client->cluster->alter($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts DROP weekly_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts DROP weekly_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- end -->
<!-- proofread -->
