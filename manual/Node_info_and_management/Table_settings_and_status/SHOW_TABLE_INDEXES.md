# SHOW TABLE INDEXES

<!-- example SHOW TABLE INDEXES -->

The `SHOW TABLE INDEXES` SQL statement displays the secondary indexes available for a specified table, along with their properties.

The syntax is:

```sql
SHOW TABLE table_name INDEXES
```

The displayed properties include the following columns:

* **Name**: The name of the secondary index. Can be used in index hints.
* **Type**: The type of data stored in the secondary index. For plain attributes, it matches the type of the original attribute. For secondary indexes generated from JSON attributes, the type is deduced by scanning all documents and determining the types of all JSON properties.
* **Enabled**:  Indicates whether the index is currently enabled and can be used to improve search speed. When an attribute is updated, the secondary index for that attribute is disabled.
* **Percent**:  For RT indexes and secondary indexes generated from JSON attributes, different disk chunks may contain different secondary indexes. This percentage shows how many chunks have an index with the same name, type, and enabled state.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW TABLE test STATUS;
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
<!-- end -->