# SHOW TABLE INDEXES

<!-- example SHOW TABLE INDEXES -->
`SHOW TABLE INDEXES` SQL 语句显示指定表可用的二级索引及其属性。[二级索引](../../Server_settings/Searchd.md#secondary_indexes)通过创建额外的数据结构来加速对特定列的搜索，从而提高查询性能。

语法为：

```sql
SHOW TABLE table_name INDEXES
```

显示的属性包括以下列：

* **Name**：二级索引的名称。可以在[查询优化器提示](../../Searching/Options.md#Query-optimizer-hints)中使用。
* **Type**：二级索引中存储的数据类型。对于普通属性，类型与原始属性的类型相匹配。对于由 JSON 属性生成的二级索引，类型通过扫描所有文档并确定所有 JSON 属性的类型来推断。
* **Enabled**：指示索引当前是否启用且可用于提升搜索速度。当属性更新时，该属性的二级索引会暂时被禁用，直到索引被重建。您可以使用[ALTER TABLE ... REBUILD SECONDARY](../../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index)命令重建被禁用的索引。
* **Percent**：在 RT 表中，不同的磁盘块可能包含不同的二级索引，特别是在使用 JSON 属性时。此百分比显示有多少块包含相同名称、类型和启用状态的索引。

> **注意：** 对于 RT 表，二级索引仅为磁盘块创建，而不为 RAM 分段中的数据创建。当您首次将数据插入 RT 表时，数据保留在 RAM 中，且不会显示二级索引。只有在数据被刷新到磁盘块后索引才可见，默认情况下，这会在表变为活动状态（同时接收插入和搜索）时自动发生。

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

<!-- end -->

