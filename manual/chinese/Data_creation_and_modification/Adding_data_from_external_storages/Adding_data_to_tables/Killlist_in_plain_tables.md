# Killlist 在普通表中的应用

使用普通表时，面临的一个挑战是需要保持表中的数据尽可能新鲜。

在这种情况下，会使用一个或多个辅助表（也称为增量表）来捕获主表创建时间与当前时间之间的修改数据。修改的数据可以包括新增、更新或删除的文档。搜索时会同时在**主表和增量表**中进行。这在仅向增量表添加**新**文档时工作得很顺畅，但当涉及更新或删除文档时，仍存在以下问题。

如果一个文档同时存在于主表和增量表中，搜索时可能会出现问题，因为引擎会看到文档的两个版本，不知道该选择哪个。因此，增量表需要以某种方式通知搜索，主表中有些已删除的文档应被忽略。**这就是 kill list 的作用所在**。

## 表的 kill-list
一个表可以维护一个文档 ID 列表，用于抑制其他表中的记录。此功能适用于使用数据库源的普通表或使用 XML 源的普通表。对于数据库源，源需要提供一个由 `sql_query_killlist` 定义的额外查询。该查询会在表中存储一个文档列表，服务器可以使用该列表从其他普通表中移除文档。

该查询预期返回多行单列数据，每行仅包含一个文档 ID。

在许多情况下，该查询是检索更新文档列表和删除文档列表的查询的联合，例如：

```ini
sql_query_killlist = \
    SELECT id FROM documents WHERE updated_ts>=@last_reindex UNION \
    SELECT id FROM documents_deleted WHERE deleted_ts>=@last_reindex
```

## 从普通表中移除文档
普通表可以包含一个名为 [killlist_target](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 的指令，告诉服务器它可以提供一个文档 ID 列表，用于从某些现有表中移除文档。该表可以使用其文档 ID 作为该列表的来源，或提供一个单独的 [列表](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)。

### killlist_target

<!-- example killlist_target 1 -->
设置 kill-list 将应用到的表。可选，默认值为空。

当你使用 [普通表](../../../Creating_a_table/Local_tables/Plain_table.md) 时，通常需要维护的不仅仅是单个表，而是一组表，以便更快地添加/更新/删除新文档（详见[增量表更新](../../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)）。为了抑制在后续（**增量**）表中更新或删除的前一个（**主**）表中的匹配项，你需要：

1.  使用 [sql_query_killlist](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list) 在**增量**表中创建 kill-list
2.  在**增量**表设置中将 **主** 表指定为 `killlist_target`：


<!-- intro -->
##### 配置：

<!-- request CONFIG -->

```ini
table products {
  killlist_target = main:kl

  path = products
  source = src_base
}
```
<!-- end -->

当指定了 `killlist_target` 时，kill-list 会在 `searchd` 启动时应用到其中列出的所有表。如果 `killlist_target` 中的任何表被轮换，kill-list 会重新应用到这些表。应用 kill-list 后，受影响的表会将这些更改保存到磁盘。

`killlist_target` 有三种操作模式：

1.  `killlist_target = main:kl`。**增量**表的 kill-list 中的文档 ID 会在 **主** 表中被抑制（参见 `sql_query_killlist`）。
2.  `killlist_target = main:id`。**增量**表中的所有文档 ID 会在 **主** 表中被抑制。kill-list 会被忽略。
3.  `killlist_target = main`。**增量**表中的文档 ID 及其 kill-list 中的文档 ID 都会在 **主** 表中被抑制。

可以指定多个目标，用逗号分隔，如：

```ini
killlist_target = table_one:kl,table_two:kl
```

<!-- example killlist_target 2 -->
你可以使用 `ALTER` 修改表的 `killlist_target` 设置，而无需重建表。

但是，由于“旧”的主表已经将更改写入磁盘，即使它不再是**增量**表的 `killlist_target`，在其中被删除的文档仍将**保持**删除状态。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER TABLE delta KILLLIST_TARGET='new_main_table:kl'
```

<!-- request HTTP -->

```http
POST /cli -d "
ALTER TABLE delta KILLLIST_TARGET='new_main_table:kl'"
```
<!-- end -->
<!-- proofread -->

