# Очистка таблицы

<!-- example truncate -->

Таблицу можно очистить с помощью SQL-запроса `TRUNCATE TABLE` или через функцию клиента PHP `truncate()`.

Ниже приведён синтаксис SQL-запроса:

```sql
TRUNCATE TABLE table_name [WITH RECONFIGURE]
```

При выполнении этого запроса RT или распределённая таблица полностью очищается. Освобождаются данные в памяти, удаляются все файлы данных таблицы и освобождаются связанные двоичные логи.

Для очистки распределённой таблицы используйте синтаксис без опции `with reconfigure`. Просто выполните стандартный запрос TRUNCATE для вашей распределённой таблицы.

```sql
TRUNCATE TABLE distributed_table
```

> ПРИМЕЧАНИЕ: Очистка распределённой таблицы требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Таблицу также можно очистить с помощью `DELETE FROM index WHERE id>0`, но это не рекомендуется, так как это медленнее чем `TRUNCATE`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
TRUNCATE TABLE products;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "TRUNCATE TABLE products"
```

<!-- response JSON -->
```http
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
$response = $client->indices()->truncate($params);
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
utilsApi.sql('TRUNCATE TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('TRUNCATE TABLE products')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('TRUNCATE TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("TRUNCATE TABLE products", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products", true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("TRUNCATE TABLE products", Some(true)).await;
```

<!-- response Rust -->
```rust
{total=0, error="", warning=""}
```

<!-- end -->

Одно из возможных применений этой команды — перед [присоединением таблицы](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md).

<!-- example truncate with RECONFIGURE -->

При использовании опции `RECONFIGURE` новые настройки токенизации, морфологии и другие параметры обработки текста, указанные в конфиге, вступают в силу после очистки таблицы. В случае если [описание схемы](Creating_a_table/Data_types.md) в конфиге отличается от схемы таблицы, новая схема из конфига применяется после очистки таблицы.

> ПРИМЕЧАНИЕ: Опция `RECONFIGURE` имеет смысл только в [Plain режиме](Read_this_first#Real-time-mode-vs-plain-mode), где применяются настройки из файла конфигурации. Обратите внимание, что `TRUNCATE` поддерживается только для RT таблиц, и опция `RECONFIGURE` может использоваться с RT таблицами только когда Manticore работает в Plain режиме.

С этой опцией очистка и перенастройка таблицы становятся одной атомарной операцией.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
TRUNCATE TABLE products with reconfigure;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request HTTP -->

```http
POST /cli -d "TRUNCATE TABLE products with reconfigure"
```

<!-- response HTTP -->
```http
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
$params = [ 'table' => 'products', 'with' => 'reconfigure' ];
$response = $client->indices()->truncate($params);
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
utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("TRUNCATE TABLE products WITH RECONFIGURE", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products WITH RECONFIGURE" ,true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("TRUNCATE TABLE products WITH RECONFIGURE", Some(true)).await;
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->
<!-- proofread -->

