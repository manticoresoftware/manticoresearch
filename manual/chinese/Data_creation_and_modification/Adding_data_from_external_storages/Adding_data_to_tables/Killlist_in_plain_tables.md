# 纯表中的淘汰列表

在使用纯表时，面临的一个挑战是需要让表中的数据尽可能新鲜。

在这种情况下，使用一个或多个辅助（也称为增量）表来捕获主表创建时间与当前时间之间的修改数据。修改的数据可以包括新增、更新或删除的文档。搜索变成对**主表和增量表**的联合搜索。当你仅向增量表添加**新**文档时，这种方式工作得很顺畅，但对于更新或删除的文档，则存在以下问题。

如果一份文档同时存在于主表和增量表中，搜索时会出问题，因为引擎会看到文档的两个版本，不知道该选择哪一个。因此，增量需要以某种方式告诉搜索，有些删除的文档在主表中应被忽略。**这时就用到了淘汰列表**。

## 表的淘汰列表
一个表可以维护一份文档 ID 列表，用于抑制其它表中的记录。这个功能适用于使用数据库源的纯表或使用 XML 源的纯表。在数据库源的情况下，源需要提供一个由`sql_query_killlist`定义的附加查询。它会在表中存储一份文档列表，供服务器用来从其它纯表中移除文档。

这个查询预期返回多个单列行，每行只包含一个文档 ID。

在许多情况下，该查询是检索更新文档列表和已删除文档列表的查询的联合，例如：

```ini
sql_query_killlist = \
    SELECT id FROM documents WHERE updated_ts>=@last_reindex UNION \
    SELECT id FROM documents_deleted WHERE deleted_ts>=@last_reindex
```

## 从纯表中移除文档
纯表中可以包含名为 [killlist_target](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 的指令，该指令告诉服务器可以提供文档 ID 列表，这些文档应从某些现有表中移除。表可以使用自身的文档 ID 作为该列表的来源，或者提供一个单独的 [列表](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)。

### killlist_target

<!-- example killlist_target 1 -->
设置淘汰列表将应用到的表。可选，默认值为空。

当你使用 [纯表](../../../Creating_a_table/Local_tables/Plain_table.md) 时，经常需要维护不止一个表，而是一组表，以便能更快地添加/更新/删除新文档（阅读有关[增量表更新](../../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)的内容）。为了抑制在前一个（**主**）表中被更新或删除但出现在下一个（**增量**）表中的匹配项，你需要：

1.  使用 [sql_query_killlist](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list) 在**增量**表中创建淘汰列表
2.  在**增量**表设置中将**主**表指定为 `killlist_target`：


<!-- intro -->
##### 配置:

<!-- request CONFIG -->

```ini
table products {
  killlist_target = main:kl

  path = products
  source = src_base
}
```
<!-- end -->

当指定了 `killlist_target` 时，淘汰列表会在 `searchd` 启动时应用于它列出的所有表。如果 `killlist_target` 中的任何表被轮换，淘汰列表会重新应用于这些表。应用淘汰列表时，受影响的表会将这些变化保存到磁盘。

`killlist_target` 有3种操作模式：

1.  `killlist_target = main:kl`。来自**增量**表淘汰列表的文档 ID 会在**主**表中被抑制（参见 `sql_query_killlist`）。
2.  `killlist_target = main:id`。**增量**表中的所有文档 ID 都会在**主**表中被抑制。淘汰列表被忽略。
3.  `killlist_target = main`。**增量**表及其淘汰列表中的文档 ID 都会在**主**表中被抑制。

可以指定多个目标，用逗号分隔，如：

```ini
killlist_target = table_one:kl,table_two:kl
```

<!-- example killlist_target 2 -->
你可以使用 `ALTER` 修改表的 `killlist_target` 设置，无需重建表。

然而，由于“旧”的主表已经将变化写入磁盘，在它中的被删除文档即使不再是**增量**表的 `killlist_target`，也会**保持**删除状态。


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

