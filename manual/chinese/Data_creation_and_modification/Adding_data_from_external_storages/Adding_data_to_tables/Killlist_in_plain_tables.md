# 在纯表格中的杀单列表

使用纯表格时，有一个挑战是需要确保表中的数据尽可能新鲜。

在这种情况下，一个或多个次要（也称为增量）表用于捕获主表创建时间和当前时间之间的修改数据。修改的数据可以包括新文档、更新文档或删除文档。搜索变成了在**主表和增量表**之间进行的搜索。当只是向增量表添加新文档时，这可以无缝工作，但当涉及到更新或删除文档时，仍然存在以下问题。

如果一个文档同时存在于主表和增量表中，在搜索时可能会出现问题，因为引擎会看到两个版本的文档，并不知道如何选择正确的版本。因此，增量表需要以某种方式通知搜索，主表中有应被忽略的已删除文档。**这就是杀单列表发挥作用的地方**。

## 表杀单列表
一个表可以维护一个文档ID列表，用于抑制其他表中的记录。此功能适用于使用数据库源的纯表或使用XML源的纯表。对于数据库源，源需要提供一个额外的查询定义为`sql_query_killlist`。它将存储在表中的文档ID列表，服务器可以使用这些文档ID从其他纯表中移除文档。

该查询预期返回多行，每行包含一个仅有的文档ID。

在许多情况下，查询是一个检索更新文档列表和删除文档列表的联合查询，例如：

```ini
sql_query_killlist = \
    SELECT id FROM documents WHERE updated_ts>=@last_reindex UNION \
    SELECT id FROM documents_deleted WHERE deleted_ts>=@last_reindex
```

## 纯表中移除文档
纯表可以包含一个名为[killlist_target](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)的指令，告诉服务器它可以提供一个文档ID列表，应该从某些现有表中移除。该表可以使用其文档ID作为此列表的来源，或者提供一个单独的[列表](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)。

### killlist_target

<!-- example killlist_target 1 -->
设置将在其中应用杀单列表的表（可选，默认值为空）。

当你使用[纯表](../../../Creating_a_table/Local_tables/Plain_table.md)时，你通常需要维护不止一个表，而是需要一组表以便更快地添加/更新/删除新文档（有关[增量表更新](../../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)的更多信息）。为了抑制在下一个（**增量**）表中更新或删除的新文档在上一个（**主**）表中的匹配项，你需要：

1.  在**增量**表中创建一个杀单列表，使用[sql_query_killlist](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list)
2.  在**增量**表设置中指定**主**表作为`killlist_target`：


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

当指定了`killlist_target`时，杀单列表会在`searchd`启动时应用于所有列出的表。如果`killlist_target`中的任何表被旋转，则重新应用这些表上的杀单列表。当杀单列表被应用时，受影响的表将这些更改保存到磁盘。

`killlist_target`有三种操作模式：

1.  `killlist_target = main:kl`。**增量**表的杀单列表中的文档ID在**主**表中被抑制（参见`sql_query_killlist`）。
2.  `killlist_target = main:id`。**增量**表中的所有文档ID在**主**表中被抑制。忽略杀单列表。
3.  `killlist_target = main`。**增量**表及其杀单列表中的所有文档ID在**主**表中被抑制。

可以指定多个目标，用逗号分隔：

```ini
killlist_target = table_one:kl,table_two:kl
```

<!-- example killlist_target 2 -->
你可以通过使用`ALTER`而不重建表来更改表的`killlist_target`设置。

然而，由于“旧”的主表已经将更改写入磁盘，即使它不再在**增量**表的`killlist_target`中，之前在主表中删除的文档仍会保持删除状态。


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

