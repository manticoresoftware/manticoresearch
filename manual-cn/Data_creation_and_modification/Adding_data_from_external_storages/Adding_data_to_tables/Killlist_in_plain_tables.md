# 普通表中的删除列表（Kill List）

在使用普通表时，会遇到一个挑战，即需要确保表中的数据尽可能最新。

在这种情况下，一个或多个辅助（也称为增量）表用于捕获从创建主表到当前时间之间发生的数据修改。修改的数据可以包括新增、更新或删除的文档。搜索变成在**主表和增量表**上的联合搜索。当只向增量表添加**新**文档时，这种方式可以无缝工作，但是当涉及更新或删除文档时，会出现一个问题。

如果一个文档同时存在于主表和增量表中，在搜索过程中会出现问题，因为引擎会看到文档的两个版本，并且无法确定哪个是正确的。因此，增量表需要以某种方式告知搜索引擎主表中有删除的文档，这些文档应该被忽略。**这时删除列表（Kill List）就派上用场了**。

## 表级删除列表

表可以维护一个文档ID列表，用于在其他表中抑制记录。这个功能适用于使用数据库源或XML源的普通表。在数据库源的情况下，源需要提供一个由`sql_query_killlist`定义的附加查询，它将存储在表中，用于服务器移除其他普通表中的文档。

这个查询需要返回一列包含文档ID的行。

在许多情况下，这个查询是一个从更新文档列表和删除文档列表中联合获取数据的查询，例如：

```ini
sql_query_killlist = \
    SELECT id FROM documents WHERE updated_ts>=@last_reindex UNION \
    SELECT id FROM documents_deleted WHERE deleted_ts>=@last_reindex
```

## 从普通表中移除文档

普通表可以包含一个称为[killlist_target](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)的指令，用来告知服务器提供一个应该从某些现有表中删除的文档ID列表。表可以使用自己的文档ID作为此列表的来源，或者提供一个单独的[列表](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#表级删除列表)。

### killlist_target

<!-- example killlist_target 1 -->设置删除列表将应用到的表。可选，默认值为空。

当你使用[普通表](../../../Creating_a_table/Local_tables/Plain_table.md)时，通常需要维护不止一个表，以便能够更快地添加/更新/删除新文档（参见[增量表更新](../../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md)）。为了抑制在主表中匹配的已被增量表更新或删除的文档，你需要：

1. 在**增量**表中使用[sql_query_killlist](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#表级删除列表)创建一个删除列表。
2. 在**增量**表设置中将**主表**指定为`killlist_target`：


<!-- intro -->
##### 配置示例：

<!-- request CONFIG -->

```ini
table products {
  killlist_target = main:kl

  path = products
  source = src_base
}
```
<!-- end -->

当指定`killlist_target`时，删除列表会在`searchd`启动时应用到所有列出的表。如果`killlist_target`中的任何表被轮换，删除列表将重新应用到这些表中。当删除列表应用时，受影响的表将保存这些更改到磁盘。

`killlist_target`有三种操作模式：

1. `killlist_target = main:kl`：**增量**表的删除列表中的文档ID在**主表**中被抑制（见`sql_query_killlist`）。
2. `killlist_target = main:id`：**增量**表中的所有文档ID在**主表**中被抑制，忽略删除列表。
3. `killlist_target = main`：**增量**表中的文档ID及其删除列表在**主表**中被抑制。

可以通过逗号分隔符指定多个目标，例如：

```ini
killlist_target = table_one:kl,table_two:kl
```

<!-- example killlist_target 2 -->
然而，由于“旧的”主表已经将这些更改写入了磁盘，即使它不再是**增量**表的 `killlist_target`，那些被删除的文档仍然会**继续**被标记为已删除。

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