# Killlist in plain tables

当使用普通表时，面临的挑战是需要尽可能保持表中的数据是最新的。

在这种情况下，使用一个或多个辅助（也称为增量）表来捕获主表创建与当前时间之间的修改数据。修改的数据可以包括新文件、更新的文件或已删除的文件。搜索变成了对**主表和增量表**的搜索。当您仅向增量表中添加**新**文档时，这是无缝的，但当涉及到更新或删除的文档时，仍然存在以下问题。

如果某个文档在主表和增量表中都存在，则在搜索过程中可能会出现问题，因为搜索引擎会看到两个版本的文档，而不知道如何选择正确的一个。因此，增量表需要以某种方式通知搜索，主表中有应该被忽略的已删除文档。**这就是杀死列表的用武之地**。

## 表杀死列表
表可以维护一个文档 ID 列表，可以用于抑制其他表中的记录。该功能适用于使用数据库源的普通表或使用 XML 源的普通表。在使用数据库源的情况下，源需要提供由`sql_query_killlist`定义的额外查询。它将在表中存储一个文档列表，服务器可以使用该列表从其他普通表中删除文档。

该查询预计返回一个包含仅文档 ID 的单列行的数量。

在许多情况下，该查询是一个联合查询，检索更新文档列表和已删除文档列表，例如：

```ini
sql_query_killlist = \
    SELECT id FROM documents WHERE updated_ts>=@last_reindex UNION \
    SELECT id FROM documents_deleted WHERE deleted_ts>=@last_reindex
```

## 在普通表中删除文档
普通表可以包含一个名为 [killlist_target](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 的指令，告诉服务器它可以提供一个应从某些现有表中删除的文档 ID 列表。该表可以使用其文档 ID 作为此列表的来源，或提供一个单独的 [列表](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)。

### killlist_target

<!-- example killlist_target 1 -->
设置将应用杀死列表的表。可选，默认值为空。

当您使用 [普通表](../../../Creating_a_table/Local_tables/Plain_table.md) 时，您通常需要维护的不仅仅是一个单一表，而是一组表，以便能够更早地添加/更新/删除新文档（了解 [增量表更新](../../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)）。为了抑制在下一个（**增量**）表中更新或删除的前一个（**主**）表中的匹配项，您需要：

1. 创建一个在**增量**表中的杀死列表，使用 [sql_query_killlist](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)
2. 在**增量**表设置中指定**主**表为 `killlist_target`：


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

当指定 `killlist_target` 时，杀死列表会在 `searchd` 启动时应用于其中列出的所有表。如果 `killlist_target` 中的任何表被旋转，杀死列表将重新应用于这些表。当杀死列表应用时，受影响的表将这些更改保存到磁盘。

`killlist_target` 有 3 种操作模式：

1. `killlist_target = main:kl`。**增量**表的杀死列表中的文档 ID 在**主**表中被抑制（参见 `sql_query_killlist`）。
2. `killlist_target = main:id`。**增量**表中的所有文档 ID 在**主**表中被抑制。杀死列表被忽略。
3. `killlist_target = main`。在**主**表中抑制**增量**表中的文档 ID 及其杀死列表。

可以指定多个目标，用逗号分隔，例如：

```ini
killlist_target = table_one:kl,table_two:kl
```

<!-- example killlist_target 2 -->
您可以通过使用 `ALTER` 来更改表的 `killlist_target` 设置，而无需重建它。

然而，由于“旧”的主表已将更改写入磁盘，在其中删除的文档将**保留**已删除，即使它不再在**增量**表的 `killlist_target` 中。


<!-- intro -->
##### SQL：

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
