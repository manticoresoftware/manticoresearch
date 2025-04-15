# Удаление таблицы

<!-- example drop -->

Удаление таблицы выполняется внутри в 2 шага:
1. Таблица очищается (аналогично [TRUNCATE](Emptying_a_table.md))
2. Все файлы таблицы удаляются из папки таблицы. Все внешние файлы таблицы, которые использовались таблицей (такие как формы слов, расширения или стоп-слова), также удаляются. Обратите внимание, что эти внешние файлы копируются в папку таблицы при использовании `CREATE TABLE`, поэтому оригинальные файлы, указанные в `CREATE TABLE`, не будут удалены.

Удаление таблицы возможно только когда сервер работает в режиме RT. Возможно удаление таблиц RT, таблиц PQ и распределенных таблиц.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE products;
```
<!-- response -->

```sql
Запрос выполнен, затронуто 0 строк (0.02 сек)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "DROP TABLE products"
```

<!-- response JSON -->
```JSON
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params = [ 'table' => 'products' ];

$response = $client->indices()->drop($params);
```

<!-- response PHP -->
```php
Array
(
    [total] => 0
    [error] =>
    [warning] =>
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->

Вот синтаксис оператора `DROP TABLE` в SQL:

```sql
DROP TABLE [IF EXISTS] table_name
```

<!-- example drop-if-exists -->

При удалении таблицы через SQL, добавление `IF EXISTS` может быть использовано для удаления таблицы только в случае, если она существует. Если вы пытаетесь удалить несуществующую таблицу с опцией `IF EXISTS`, ничего не произойдет.

При удалении таблицы через PHP, вы можете добавить необязательный параметр `silent`, который работает аналогично `IF EXISTS`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE IF EXISTS products;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "DROP TABLE IF EXISTS products"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params =
[
  'table' => 'products',
  'body' => ['silent' => true]
];

$client->indices()->drop($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE IF EXISTS products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE IF EXISTS products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE IF EXISTS products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE IF EXISTS products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->
<!-- proofread -->
