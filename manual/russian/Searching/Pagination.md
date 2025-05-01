# Пагинация результатов поиска

<!-- example general -->

Manticore Search по умолчанию возвращает 20 лучших документов в наборе результатов.

#### SQL
В SQL вы можете перемещаться по набору результатов, используя предложение `LIMIT`.

`LIMIT` может принимать либо одно число в качестве размера возвращаемого набора с нулевым смещением, либо пару значений смещения и размера.

#### HTTP JSON
При использовании HTTP JSON, узлы `offset` и `limit` управляют смещением набора результатов и размером возвращаемого набора. В качестве альтернативы, вы можете использовать пару `size` и `from`.

<!-- intro -->

<!-- request SQL -->

```sql
SELECT  ... FROM ...  [LIMIT [offset,] row_count]
SELECT  ... FROM ...  [LIMIT row_count][ OFFSET offset]
```


<!-- request JSON -->

```json
{
  "table": "<table_name>",
  "query": ...
  ...  
  "limit": 20,
  "offset": 0
}
{
  "table": "<table_name>",
  "query": ...
  ...  
  "size": 20,
  "from": 0
}
```

<!-- end -->

<!-- example maxMatches -->
### Окно набора результатов

По умолчанию Manticore Search использует окно набора результатов из 1000 лучших документов, которые могут быть возвращены в наборе результатов. Если набор результатов разбивается на страницы за пределами этого значения, запрос завершится с ошибкой.

Это ограничение можно настроить с помощью параметра запроса [max_matches](../Searching/Options.md#max_matches).

Увеличение `max_matches` до очень больших значений следует делать только в том случае, если это необходимо для навигации к таким точкам. Высокое значение `max_matches` требует больше памяти и может увеличить время ответа на запрос. Один из способов работы с глубокими наборами результатов — установить `max_matches` как сумму смещения и лимита.

Понижение `max_matches` ниже 1000 имеет преимущества, так как уменьшает объем памяти, используемой запросом. Это также может снизить время запроса, но в большинстве случаев это может оказаться незначительным выигрышем.

<!-- intro -->


<!-- request SQL -->

```sql
SELECT  ... FROM ...   OPTION max_matches=<value>
```


<!-- request JSON -->


```json
{
  "table": "<table_name>",
  "query": ...
  ...
  "max_matches":<value>
  }
}

```

<!-- end -->

## Опция поиска по прокрутке

Опция поиска по прокрутке предоставляет эффективный и надежный способ пагинации по большим наборам результатов. В отличие от традиционной пагинации на основе смещения, прокрутка предлагает лучшую производительность для глубокой пагинации и предоставляет более простой способ реализации пагинации.

#### Прокрутка через SQL

<!-- example scroll_sql_init -->
**Начальный запрос с критериями сортировки**

Начните с выполнения начального запроса с вашими желаемыми критериями сортировки. Единственным требованием является то, что `id` должен быть включен в предложение ORDER BY для обеспечения последовательной пагинации. Запрос вернет как ваши результаты, так и токен прокрутки для последующих страниц.

```sql
SELECT ... ORDER BY [... ,] id {ASC|DESC};
```

<!-- intro -->
Пример:
<!-- request Initial Query Example -->
```sql
SELECT weight(), id FROM test WHERE match('hello') ORDER BY weight() desc, id asc limit 2;
```

<!-- response Initial Query Example -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    1 |
|     1281 |    2 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example scroll_sql_show -->
**Получение токена прокрутки**

После выполнения начального запроса получите токен прокрутки, выполнив команду `SHOW SCROLL`.

```sql
SHOW SCROLL;
```

Ответ:
```sql
| scroll_token                       |
|------------------------------------|
| <base64 encoded scroll token>      |
```

<!-- intro -->
Пример:

<!-- request Scroll Token Example -->
```sql
SHOW SCROLL;
```

<!-- response Scroll Token Example -->
```sql
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| scroll_token                                                                                                                                                                                                             |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHJiiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0= |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example scroll_sql_paginated -->
**Пагинированный запрос, использующий прокрутку**

 Чтобы получить следующую страницу результатов, включите токен прокрутки в последующий запрос в качестве параметра. Когда параметр `scroll` указан, указание критериев сортировки является необязательным.

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 encoded scroll token>'[, ...];
```

Это гарантирует, что пагинация продолжается бесшовно, сохраняя контекст сортировки, установленный в начальном запросе.

<!-- intro -->
Пример:

<!-- request Paginated Query Example -->
```sql
SELECT weight(), id FROM test WHERE match('hello') limit 2
OPTION scroll='eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHJiiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0=';
```

<!-- response Paginated Query Example -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    3 |
|     1281 |    4 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

#### Прокрутка через JSON

<!-- example scroll_json_init -->
**Начальный запрос**

В начальном запросе укажите `"scroll": true` в опциях и желаемые критерии сортировки. Обратите внимание, что `id` должен присутствовать в массиве `sort`. Ответ будет включать токен прокрутки, который можно использовать для постраничной навигации в последующих запросах.

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {

"scroll": true
  }, 
  
  ...
  
  "sort": [
    ...
    { "id":{ "order":"{asc|desc}"} }
  ]
}
```

Пример вывода:

```json
{
    "timed_out": false,
    "hits": {


...
    
    },
    "scroll": "<base64 encoded scroll token>"
}
```

<!-- request Initial Request Example -->
```json
POST /search
{ 
  "table": "test",
  "options":
  {

"scroll": true
  },
  "query":
  {  

"query_string":"hello"
  },
  "sort":
  [
    { "_score":{ "order":"desc"} },

{ "id":{ "order":"asc"} }
  ],
  "track_scores": true,
  "limit":2
}
```

<!-- response Initial Request Example -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 10,
    "total_relation": "eq",
    "hits":

[
      {
        "_id": 1,
        "_score": 1281,
        "_source":

{
          "title": "привет мир1"
        }
      },
      {
        "_id": 2,
        "_score": 1281,
        "_source":

{
          "title": "привет мир2"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- example scroll_json_paginated -->
**Постраничный запрос с использованием прокрутки**

Для продолжения постраничной навигации включите токен прокрутки, полученный из предыдущего ответа, в объект опций следующего запроса. Указание критериев сортировки является необязательным.

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
    "scroll": "<base64 encoded scroll token>"
  },

  ...

}
```

<!-- request Paginated Request Example -->

```json
POST /search
{ 
  "table": "test",
  "options":
  {

"scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
  },
  "query":
  {  

"query_string":"hello"
  },
  "track_scores": true,
  "limit":2
}
```

<!-- response Paginated Request Example -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 8,
    "total_relation": "eq",
    "hits":
   [
      {
        "_id": 3,
        "_score": 1281,
        "_source":
        {
          "title": "привет мир3"
        }
      },
      {
        "_id": 4,
        "_score": 1281,
        "_source":
        {
          "title": "привет мир4"
# Пагинация результатов поиска

<!-- example general -->

Manticore Search возвращает по умолчанию 20 лучших совпадающих документов в наборе результатов.

#### SQL
В SQL вы можете перемещаться по набору результатов, используя оператор `LIMIT`.

`LIMIT` может принимать либо одно число, как размер возвращаемого набора с нулевым смещением, либо пару значений смещения и размера.

#### HTTP JSON
При использовании HTTP JSON узлы `offset` и `limit` управляют смещением набора результатов и размером возвращаемого набора. В качестве альтернативы вы можете использовать пару `size` и `from`.

<!-- intro -->

<!-- request SQL -->

```sql
SELECT  ... FROM ...  [LIMIT [offset,] row_count]
SELECT  ... FROM ...  [LIMIT row_count][ OFFSET offset]
```


<!-- request JSON -->

```json
{
  "table": "<table_name>",
  "query": ...
  ...  
  "limit": 20,
  "offset": 0
}
{
  "table": "<table_name>",
  "query": ...
  ...  
  "size": 20,
  "from": 0
}
```

<!-- end -->

<!-- example maxMatches -->
### Окно набора результатов

По умолчанию Manticore Search использует окно набора результатов из 1000 лучших ранжированных документов, которые могут быть возвращены в наборе результатов. Если набор результатов разбит на страницы сверх этого значения, запрос завершится с ошибкой.

Это ограничение можно изменить с помощью параметра запроса [max_matches](../Searching/Options.md#max_matches).

Увеличение `max_matches` до очень больших значений должно производиться только в случае, если это необходимо для навигации к таким точкам. Высокое значение `max_matches` требует больше памяти и может увеличить время отклика запроса. Один из способов работы с глубокими наборами результатов — установить `max_matches` как сумму смещения и размера.

Понижение значения `max_matches` ниже 1000 дает преимущества в снижении памяти, используемой запросом. Это также может сократить время запроса, но в большинстве случаев это может быть незначительным улучшением.

<!-- intro -->


<!-- request SQL -->

```sql
SELECT  ... FROM ...   OPTION max_matches=<value>
```


<!-- request JSON -->


```json
{
  "table": "<table_name>",
  "query": ...
  ...
  "max_matches":<value>
  }
}

```

<!-- end -->

# Опция прокрутки поиска

Опция прокрутки поиска предоставляет эффективный и надежный способ пагинации через большие наборы результатов. В отличие от традиционной пагинации на основе смещения, прокрутка поиска предлагает лучшую производительность для глубокой пагинации и обеспечивает более простой способ реализации пагинации.

#### Прокрутка через SQL

<!-- example scroll_sql_init -->
**Начальный запрос с критериями сортировки**

Начните с выполнения начального запроса с вашими желаемыми критериями сортировки. Единственным требованием является то, что `id` должен быть включен в предложение ORDER BY, чтобы обеспечить согласованную пагинацию. Запрос вернет как ваши результаты, так и токен прокрутки для последующих страниц.

```sql
SELECT ... ORDER BY [... ,] id {ASC|DESC};
```

<!-- intro -->
Пример:
<!-- request Initial Query Example -->
```sql
SELECT weight(), id FROM test WHERE match('hello') ORDER BY weight() desc, id asc limit 2;
```

<!-- response Initial Query Example -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    1 |
|     1281 |    2 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example scroll_sql_show -->
**Получение токена прокрутки**

После выполнения начального запроса получите токен прокрутки, выполнив команду `SHOW SCROLL`.

```sql
SHOW SCROLL;
```

Ответ:
```sql
| scroll_token                       |
|------------------------------------|
| <base64 encoded scroll token>      |
```

<!-- intro -->
Пример:

<!-- request Scroll Token Example -->
```sql
SHOW SCROLL;
```

<!-- response Scroll Token Example -->
```sql
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| scroll_token                                                                                                                                                                                                             |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0= |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example scroll_sql_paginated -->
**Запрос с пагинацией с использованием прокрутки**

 Чтобы получить следующую страницу результатов, включите токен прокрутки в последующий запрос в качестве параметра. Когда параметр `scroll` задан, указание критериев сортировки является необязательным.

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 encoded scroll token>'[, ...];
```

Это гарантирует, что пагинация продолжится бесшовно, сохраняя контекст сортировки, установленный в начальном запросе.

<!-- intro -->
Пример:

<!-- request Paginated Query Example -->
```sql
SELECT weight(), id FROM test WHERE match('hello') limit 2
OPTION scroll='eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0=';
```

<!-- response Paginated Query Example -->
```sql
+----------+------+
| вес()    | id   |
+----------+------+
|     1281 |    3 |
|     1281 |    4 |
+----------+------+
2 строки в наборе (0.00 сек)
```

<!-- end -->

#### Прокрутка через JSON

<!-- example scroll_json_init -->
**Изначальный запрос**

В изначальном запросе укажите `"scroll": true` в опциях и желаемые критерии сортировки. Обратите внимание, что `id` должен присутствовать в массиве `sort`. Ответ будет включать токен прокрутки, который можно использовать для постраничного просмотра в последующих запросах.

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
	  "scroll": true
  }, 
  
  ...
  
  "sort": [
    ...
    { "id":{ "order":"{asc|desc}"} }
  ]
}
```

Пример вывода:

```json
{
    "timed_out": false,
    "hits": {

		...
    
    },
    "scroll": "<base64 encoded scroll token>"
}
```

<!-- request Initial Request Example -->
```json
POST /search
{ 
  "table": "test",
  "options":
  {
	"scroll": true
  },
  "query":
  {  
	"query_string":"hello"
  },
  "sort":
  [
    { "_score":{ "order":"desc"} },
	{ "id":{ "order":"asc"} }
  ],
  "track_scores": true,
  "limit":2
}
```

<!-- response Initial Request Example -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 10,
    "total_relation": "eq",
    "hits":
	[
      {
        "_id": 1,
        "_score": 1281,
        "_source":
		{
          "title": "hello world1"
        }
      },
      {
        "_id": 2,
        "_score": 1281,
        "_source":
		{
          "title": "hello world2"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- example scroll_json_paginated -->
**Постраничный запрос с использованием прокрутки**

Чтобы продолжить постраничный просмотр, включите токен прокрутки, полученный из предыдущего ответа, в объект опций следующего запроса. Указание критерия сортировки является необязательным.

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
    "scroll": "<base64 encoded scroll token>"
  },

  ...

}
```

<!-- request Paginated Request Example -->

```json
POST /search
{ 
  "table": "test",
  "options":
  {
	"scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
  },
  "query":
  {  
	"query_string":"hello"
  },
  "track_scores": true,
  "limit":2
}
```

<!-- response Paginated Request Example -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 8,
    "total_relation": "eq",
    "hits":
   [
      {
        "_id": 3,
        "_score": 1281,
        "_source":
        {
          "title": "hello world3"
        }
      },
      {
        "_id": 4,
        "_score": 1281,
        "_source":
        {
          "title": "hello world4"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjo0LCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- proofread -->
