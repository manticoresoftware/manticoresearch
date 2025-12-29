# Пагинация результатов поиска

<!-- example general -->

По умолчанию Manticore Search возвращает 20 наиболее релевантных документов в наборе результатов.

#### SQL
В SQL вы можете перемещаться по набору результатов с помощью предложения `LIMIT`.

`LIMIT` может принимать либо одно число как размер возвращаемого набора с нулевым смещением, либо пару значений: смещение и размер.

#### HTTP JSON
При использовании HTTP JSON узлы `offset` и `limit` управляют смещением набора результатов и размером возвращаемого набора. Альтернативно, вы можете использовать пару `size` и `from`.

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

По умолчанию Manticore Search использует окно набора результатов в 1000 наиболее релевантных документов, которые могут быть возвращены в наборе результатов. Если пагинация выходит за это значение, запрос завершится ошибкой.

Это ограничение можно изменить с помощью опции запроса [max_matches](../Searching/Options.md#max_matches).

Увеличение `max_matches` до очень высоких значений следует делать только в случае необходимости навигации до таких точек. Высокое значение `max_matches` требует больше памяти и может увеличить время ответа на запрос. Один из способов работы с глубокими наборами результатов — установить `max_matches` как сумму смещения и лимита.

Уменьшение `max_matches` ниже 1000 имеет преимущество в виде снижения используемой запросом памяти. Это также может сократить время выполнения запроса, но в большинстве случаев выигрыш может быть незначительным.

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

## Опция прокрутки (Scroll Search)

Опция прокрутки (scroll search) предоставляет эффективный и надежный способ пагинации по большим наборам результатов. В отличие от традиционной пагинации на основе смещения, прокрутка обеспечивает лучшую производительность для глубокой пагинации и предлагает более простой способ её реализации.
Хотя она использует то же окно `max_matches`, что и пагинация на основе смещения, прокрутка **может возвращать больше документов, чем значение `max_matches`**, извлекая результаты за несколько запросов с использованием токена прокрутки.
При использовании пагинации через прокрутку нет необходимости использовать `offset` и `limit` вместе — это избыточно и обычно считается излишним усложнением. Вместо этого просто укажите `limit` вместе с токеном `scroll` для получения каждой последующей страницы.

#### Прокрутка через SQL

<!-- example scroll_sql_init -->
**Начальный запрос с критериями сортировки**

Начните с выполнения начального запроса с желаемыми критериями сортировки. Единственное требование — `id` должен быть включен в предложение ORDER BY для обеспечения согласованной пагинации. Запрос вернет как ваши результаты, так и токен прокрутки для последующих страниц.

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
Вы должны вызывать `SHOW SCROLL` после **каждого** запроса в последовательности прокрутки, чтобы получить обновленный токен прокрутки для следующей страницы.
Каждый запрос генерирует новый токен, который отражает последнюю позицию прокрутки.

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
**Пагинированный запрос с использованием scroll**

Чтобы получить следующую страницу результатов, включите токен прокрутки в последующий запрос как опцию. Когда предоставлена опция `scroll`, указание критериев сортировки необязательно.
Не забудьте снова вызвать `SHOW SCROLL` после этого запроса, чтобы получить новый токен, необходимый для следующей страницы.

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 encoded scroll token>'[, ...];
```

Это обеспечивает бесперебойное продолжение пагинации, сохраняя контекст сортировки, установленный в начальном запросе.

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

В начальном запросе укажите `"scroll": true` в опциях и желаемые критерии сортировки. Обратите внимание, что `id` должен присутствовать в массиве `sort`. Ответ будет содержать токен прокрутки, который можно использовать для пагинации в последующих запросах.

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
**Пагинированный запрос с использованием scroll**

Чтобы продолжить пагинацию, включите токен прокрутки, полученный из предыдущего ответа, в объект опций следующего запроса. Указание критериев сортировки необязательно.

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

