# SHOW TABLE INDEXES

<!-- example SHOW TABLE INDEXES -->
`SHOW TABLE INDEXES` SQL 语句显示指定表的可用二级索引及其属性。[二级索引](../../Server_settings/Searchd.md#secondary_indexes) 通过创建额外的数据结构来加速特定列的搜索，从而提升查询性能。

语法如下：

```sql
SHOW TABLE table_name INDEXES
```

显示的属性包括以下列：

* **Name**：二级索引的名称。可用于[查询优化器提示](../../Searching/Options.md#Query-optimizer-hints)。
* **Type**：二级索引中存储的数据类型。对于普通属性，类型与原始属性类型相同。对于从 JSON 属性生成的二级索引，类型通过扫描所有文档并确定所有 JSON 属性的类型来推断。
* **Enabled**：指示索引当前是否启用，是否可用于提升搜索速度。当属性被更新时，该属性的二级索引会暂时禁用，直到索引重建。您可以使用[ALTER TABLE ... REBUILD SECONDARY](../../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) 命令重建被禁用的索引。
* **Percent**：在 RT 表中，不同的磁盘块可能包含不同的二级索引，尤其是在使用 JSON 属性时。此百分比显示有多少块包含具有相同名称、类型和启用状态的索引。

> **注意：** 对于 RT 表，二级索引仅为磁盘块创建，不为 RAM 段中的数据创建。当您首次向 RT 表插入数据时，数据保存在 RAM 中，二级索引不会显示。索引仅在数据刷新到磁盘块后可见，默认情况下，当表变为活动状态（同时接收插入和搜索）时会自动刷新。

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
<!-- end -->

