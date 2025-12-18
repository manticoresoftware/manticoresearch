# SHOW TABLE INDEXES

<!-- example SHOW TABLE INDEXES -->
Оператор SQL `SHOW TABLE INDEXES` отображает доступные для указанной таблицы вторичные индексы вместе с их свойствами. [Вторичные индексы](../../Server_settings/Searchd.md#secondary_indexes) улучшают производительность запросов, создавая дополнительные структуры данных, ускоряющие поиск по конкретным столбцам.

Синтаксис имеет вид:

```sql
SHOW TABLE table_name INDEXES
```

Отображаемые свойства включают следующие столбцы:

* **Name**: Имя вторичного индекса. Может использоваться в [подсказках оптимизатора запросов](../../Searching/Options.md#Query-optimizer-hints).
* **Type**: Тип данных, хранящихся во вторичном индексе. Для обычных атрибутов он совпадает с типом исходного атрибута. Для вторичных индексов, созданных из JSON-атрибутов, тип определяется путем сканирования всех документов и определения типов всех JSON-свойств.
* **Enabled**: Показывает, включен ли индекс в данный момент и может ли использоваться для ускорения поиска. При обновлении атрибута вторичный индекс для этого атрибута временно отключается до тех пор, пока индекс не будет перестроен. Вы можете перестроить отключенные индексы с помощью команды [ALTER TABLE ... REBUILD SECONDARY](../../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index).
* **Percent**: В RT-таблицах разные дисковые чанки могут содержать разные вторичные индексы, особенно при использовании JSON-атрибутов. Этот процент показывает, сколько чанков содержит индекс с одинаковым именем, типом и состоянием включения.

> **Примечание:** Для RT-таблиц вторичные индексы создаются только для дисковых чанков, а не для данных в RAM-сегментах. Когда вы впервые вставляете данные в RT-таблицу, они остаются в оперативной памяти, и вторичные индексы не будут отображаться. Индексы становятся видимыми только после сброса данных в дисковые чанки, что по умолчанию происходит автоматически, когда таблица становится активной (получает как вставки, так и запросы поиска).

<!--
data for the following examples:

DROP TABLE IF EXISTS test;
CREATE TABLE test(j json);
INSERT INTO test(j) VALUES
( '{"addresses": {"a1":{"id":"1","name":"a"},"a2":{"id":"2","name":"b"},"a3":{"id":"3","name":"c"},"a4":{"id":"4","name":"d"},"a5":{"id":"5","name":"e"},"a6":{"id":"6","name":"f"}},"factor":2,"int_arr":[1,2,3],"tags": ["1":{},"2":{},"3":{}],"arr":"a","str":"a","price":1.0 }' );
FLUSH RAMCHUNK test;
--> 

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW TABLE test INDEXES;
```

<!-- response SQL -->

```sql
+------------------------------+--------+---------+---------+
| Name                         | Type   | Enabled | Percent |
+------------------------------+--------+---------+---------+
| j['addresses']               | uint32 | 1       | 100     |
| j['addresses']['a1']         | uint32 | 1       | 100     |
| j['addresses']['a2']         | uint32 | 1       | 100     |
| j['addresses']['a3']         | uint32 | 1       | 100     |
| j['addresses']['a4']         | uint32 | 1       | 100     |
| j['addresses']['a5']         | uint32 | 1       | 100     |
| j['addresses']['a6']         | uint32 | 1       | 100     |
| j['factor']                  | uint32 | 1       | 100     |
| j['int_arr']                 | uint32 | 1       | 100     |
| j['tags']                    | uint32 | 1       | 100     |
| id                           | int64  | 1       | 100     |
| j['price']                   | float  | 1       | 100     |
| j['addresses']['a1']['id']   | string | 1       | 100     |
| j['addresses']['a1']['name'] | string | 1       | 100     |
| j['addresses']['a2']['id']   | string | 1       | 100     |
| j['addresses']['a2']['name'] | string | 1       | 100     |
| j['addresses']['a3']['id']   | string | 1       | 100     |
| j['addresses']['a3']['name'] | string | 1       | 100     |
| j['addresses']['a4']['id']   | string | 1       | 100     |
| j['addresses']['a4']['name'] | string | 1       | 100     |
| j['addresses']['a5']['id']   | string | 1       | 100     |
| j['addresses']['a5']['name'] | string | 1       | 100     |
| j['addresses']['a6']['id']   | string | 1       | 100     |
| j['addresses']['a6']['name'] | string | 1       | 100     |
| j['arr']                     | string | 1       | 100     |
| j['str']                     | string | 1       | 100     |
| j['tags']['1']               | string | 1       | 100     |
| j['tags']['2']               | string | 1       | 100     |
| j['tags']['3']               | string | 1       | 100     |
+------------------------------+--------+---------+---------+
29 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW TABLE test INDEXES;"
```

<!-- response JSON -->

```JSON
[
  {
    "columns": [
      {
        "Name": {
          "type": "string"
        }
      },
      {
        "Type": {
          "type": "string"
        }
      },
      {
        "Enabled": {
          "type": "string"
        }
      },
      {
        "Percent": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Name": "j['addresses']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a1']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a2']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a3']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a4']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a5']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a6']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['factor']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['int_arr']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['tags']",
        "Type": "uint32",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "id",
        "Type": "int64",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['price']",
        "Type": "float",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a1']['id']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a1']['name']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a2']['id']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a2']['name']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a3']['id']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a3']['name']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a4']['id']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a4']['name']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a5']['id']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a5']['name']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a6']['id']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['addresses']['a6']['name']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['arr']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['str']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['tags']['1']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['tags']['2']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      },
      {
        "Name": "j['tags']['3']",
        "Type": "string",
        "Enabled": "1",
        "Percent": 100
      }
    ],
    "total": 29,
    "error": "",
    "warning": ""
  }
]
```
,
<!-- end -->

