# SHOW TABLE INDEXES

<!-- example SHOW TABLE INDEXES -->
The `SHOW TABLE INDEXES` SQL statement displays the secondary indexes available for a specified table, along with their properties. [Secondary indexes](../../Server_settings/Searchd.md#secondary_indexes) improve query performance by creating additional data structures that speed up searches on specific columns.

The syntax is:

```sql
SHOW TABLE table_name INDEXES
```

The displayed properties include the following columns:

* **Name**: The name of the secondary index. Can be used in [query optimizer hints](../../Searching/Options.md#Query-optimizer-hints).
* **Type**: The type of data stored in the secondary index. For plain attributes, it matches the type of the original attribute. For secondary indexes generated from JSON attributes, the type is deduced by scanning all documents and determining the types of all JSON properties.
* **Enabled**: Indicates whether the index is currently enabled and can be used to improve search speed. When an attribute is updated, the secondary index for that attribute is temporarily disabled until the index is rebuilt. You can rebuild disabled indexes using the [ALTER TABLE ... REBUILD SECONDARY](../../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) command.
* **Percent**: In an RT table, different disk chunks may contain different secondary indexes especially when JSON attributes are used. This percentage shows how many chunks have an index with the same name, type, and enabled state.

> **Note:** For RT tables, secondary indexes are only created for disk chunks, not for data in RAM segments. When you first insert data into an RT table, it stays in RAM and no secondary indexes will be shown. The indexes become visible only after the data is flushed to disk chunks, which by default happens automatically when the table becomes active (receives both inserts and searches).

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

