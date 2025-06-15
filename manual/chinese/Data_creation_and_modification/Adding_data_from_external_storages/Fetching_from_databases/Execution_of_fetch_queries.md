# 执行 fetch 查询

使用所有 SQL 驱动时，构建普通表通常按如下步骤进行。

* 建立与数据库的连接。
* 执行 `sql_query_pre_all` 查询，以执行任何必要的初始设置，例如为 MySQL 设置每连接编码。这些查询在整个索引过程开始前执行，并且在重新连接后用于索引 MVA 属性和联接字段后执行。
* 执行 `sql_query_pre` 预查询，以执行任何必要的初始设置，例如设置临时表或维护计数器表。这些查询在整个索引过程只执行一次。
* 执行诸如 `sql_query_pre` 的预查询，以执行任何必要的初始设置，例如设置临时
  表，或维护计数器表。这些查询在整个索引过程中只执行一次。
* 执行主查询 `sql_query` 并处理它返回的行。
* 执行后查询 `sql_query_post`，以执行必要的清理工作。
* 关闭与数据库的连接。
* 索引器执行排序阶段（严格来说，是特定表类型的后处理）。
* 再次建立与数据库的连接。
* 执行后处理查询 `sql_query_post_index`，以执行必要的最终清理。
* 再次关闭与数据库的连接。

从 MYSQL 获取数据的示例：

```ini
source mysource {
  type             = mysql
  path             = /path/to/realtime
  sql_host         = localhost
  sql_user         = myuser
  sql_pass         = mypass
  sql_db           = mydb
  sql_query_pre    = SET CHARACTER_SET_RESULTS=utf8
  sql_query_pre    = SET NAMES utf8
  sql_query        =  SELECT id, title, description, category_id  FROM mytable
  sql_query_post   = DROP TABLE view_table
  sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
  sql_attr_uint    = category_id
  sql_field_string = title
 }

table mytable {
  type   = plain
  source = mysource
  path   = /path/to/mytable
  ...
 }
```

## sql_query

这是用来从 SQL 服务器检索文档的查询。只能声明一个 `sql_query`，并且必须有一个。另请参见 [处理获取的数据](../../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#Processing-fetched-data)

## sql_query_pre

预获取查询或预查询。这是一个多值的可选设置，默认是空查询列表。预查询会按照它们在配置文件中出现的顺序在 `sql_query` 之前执行。预查询的结果会被忽略。

预查询有多种用途。它们可用于设置编码，标记将要被索引的记录，更新内部计数器，设置各种每连接的 SQL 服务器选项和变量等。

预查询最常见的用途可能是指定服务器返回行时使用的编码。注意 Manticore 只接受 UTF-8 文本。两个 MySQL 特定的设置编码的示例如下：

```ini
sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
sql_query_pre = SET NAMES utf8
```

此外，对于 MySQL 源，预查询中禁用查询缓存（仅针对索引连接）是有用的，因为索引查询不会频繁重跑，缓存其结果也没有意义。
这可以通过以下方式实现：

```ini
sql_query_pre = SET SESSION query_cache_type=OFF
```

## sql_query_post

后获取查询。这是一个可选设置，默认值为空。

该查询在 `sql_query` 成功完成后立即执行。当后获取查询产生错误时，会以警告形式报告，但不会终止索引。其结果集会被忽略。请注意，在执行此查询时索引还未完成，后续索引可能依然失败。因此，不应从这里进行任何永久更新。例如，永久更改最后成功索引 ID 的辅助表更新不应在 `sql_query_post` 查询中执行；应在 `sql_query_post_index` 查询中执行。

## sql_query_post_index

后处理查询。这是一个可选设置，默认值为空。

该查询在索引完全且成功完成后执行。如果此查询产生错误，会以警告形式报告，但不会终止索引。其结果集会被忽略。其文本中可以使用 `$maxid` 宏；它会被展开为索引过程中实际从数据库获取的最大文档 ID。如果未索引任何文档，`$maxid` 会被展开为 0。

示例：
```ini
sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
```

`sql_query_post` 和 `sql_query_post_index` 的区别在于，`sql_query_post` 在 Manticore 获得所有文档后立即运行，但后续索引可能因其他原因失败。相反，到执行 `sql_query_post_index` 查询时，保证表已成功创建。数据库连接会被断开并重建，因为排序阶段可能非常耗时，否则会超时。

<!-- proofread -->

