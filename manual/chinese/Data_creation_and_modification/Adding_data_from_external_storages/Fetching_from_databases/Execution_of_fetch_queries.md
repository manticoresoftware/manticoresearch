# 执行 fetch 查询

使用所有 SQL 驱动程序，构建一个普通表通常按以下方式进行。

* 建立与数据库的连接。
* 执行 `sql_query_pre_all` 查询以进行任何必要的初始设置，例如使用 MySQL 设置每个连接的编码。这些查询在整个索引过程之前执行，也在重新连接后执行，以索引 MVA 属性和联接字段。
* 执行 `sql_query_pre` 预查询以进行任何必要的初始设置，例如设置临时表或维护计数器表。这些查询在整个索引过程只执行一次。
* 执行 `sql_query_pre` 的预查询以进行任何必要的初始设置，例如设置临时
  表，或维护计数器表。这些查询在整个索引过程中只执行一次。
* 执行主查询 `sql_query`，并处理它返回的行。
* 执行后查询 `sql_query_post` 以进行必要的清理。
* 关闭与数据库的连接。
* 索引器执行排序阶段（严格来说，是表类型特定的后处理）。
* 再次建立与数据库的连接。
* 执行后处理查询 `sql_query_post_index` 以进行必要的最终清理。
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

这是用于从 SQL 服务器检索文档的查询。只能声明一个 sql_query，且必须有一个。另见 [Processing fetched data](../../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#Processing-fetched-data)

## sql_query_pre

预取查询或预查询。这是一个多值的可选设置，默认是一个空的查询列表。预查询在 sql_query 之前按其在配置文件中的顺序执行。预查询的结果会被忽略。

预查询用途多样。它们可用于设置编码、标记将要被索引的记录、更新内部计数器、设置各种每连接的 SQL 服务器选项和变量，等等。

预查询最常见的用途可能是指定服务器返回行所用的编码。注意 Manticore 只接受 UTF-8 文本。设定编码的两个 MySQL 特定示例如下：

```ini
sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
sql_query_pre = SET NAMES utf8
```

此外，特定于 MySQL 源，禁用查询缓存（仅限索引器连接）在预查询中很有用，因为索引查询不会频繁重跑，缓存它们的结果没有意义。
这可以通过以下方式实现：

```ini
sql_query_pre = SET SESSION query_cache_type=OFF
```

## sql_query_post

后取查询。此为可选设置，默认值为空。

此查询在 sql_query 成功完成后立即执行。后取查询如果出现错误，会以警告方式报告，但不会终止索引。它的结果集会被忽略。注意此时索引尚未完成，后续索引仍可能失败。因此，不应从这里执行任何永久更新。例如，永久改变最后成功索引 ID 的辅助表更新，不应从 `sql_query_post` 查询运行，而应从 `sql_query_post_index` 查询运行。

## sql_query_post_index

后处理查询。此为可选设置，默认值为空。

此查询在索引完全且成功完成时执行。如果此查询出现错误，会以警告方式报告，但不会终止索引。它的结果集会被忽略。文本中可以使用 `$maxid` 宏；它将被展开为索引期间实际从数据库获取的最大文档 ID。如果没有索引任何文档，`$maxid` 将被展开为 0。

示例：
```ini
sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
```

`sql_query_post` 和 `sql_query_post_index` 的区别在于，`sql_query_post` 在 Manticore 接收到所有文档时立即运行，但后续索引可能因其他原因失败。相反，在执行 `sql_query_post_index` 查询时，表已确保成功创建。数据库连接会断开重建，因为排序阶段可能非常长，否则会超时。

<!-- proofread -->

