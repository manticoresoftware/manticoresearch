# SHOW TABLE INDEXES

<!-- example SHOW TABLE INDEXES -->
`SHOW TABLE INDEXES` SQL 语句显示指定表可用的二级索引及其属性。[二级索引](../../Server_settings/Searchd.md#secondary_indexes) 通过创建额外的数据结构加速对特定列的搜索，从而提高查询性能。

语法如下：

```sql
SHOW TABLE table_name INDEXES
```

显示的属性包括以下列：

* **Name**：二级索引的名称。可用于[查询优化器提示](../../Searching/Options.md#Query-optimizer-hints)。
* **Type**：存储在二级索引中的数据类型。对于普通属性，类型与原始属性类型相同。对于从 JSON 属性生成的二级索引，类型通过扫描所有文档并确定所有 JSON 属性的类型推断得出。
* **Enabled**：表示索引当前是否启用，是否可以用于提高搜索速度。当属性被更新时，该属性的二级索引会暂时禁用，直到索引被重建。您可以使用[ALTER TABLE ... REBUILD SECONDARY](../../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令重建禁用的索引。
* **Percent**：在 RT 表中，不同的磁盘块可能包含不同的二级索引，特别是当使用 JSON 属性时。此百分比显示具有相同名称、类型和启用状态索引的块的比例。

> **注意：** 对于 RT 表，二级索引仅为磁盘块创建，而不为内存段中的数据创建。当您首次向 RT 表插入数据时，数据保持在内存中，不显示任何二级索引。只有数据刷新到磁盘块后，索引才会可见，默认情况下，当表变为活动状态（同时接收插入和搜索）时会自动发生刷新。

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
POST /sql?mode=raw -d "SHOW TABLE test INDEXES"
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

