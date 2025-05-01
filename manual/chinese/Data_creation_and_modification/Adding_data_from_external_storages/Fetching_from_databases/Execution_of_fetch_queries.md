# 执行获取查询

对于所有的 SQL 驱动程序，建立一个普通表通常按以下步骤进行。

* 建立与数据库的连接。
* 执行 `sql_query_pre_all` 查询以执行任何必要的初始设置，例如使用 MySQL 设置每个连接的编码。这些查询在整个索引过程之前运行，并且在重新连接以索引 MVA 属性和连接字段后也会运行。
* 执行 `sql_query_pre` 预查询以执行任何必要的初始设置，例如设置临时表或维护计数器表。这些查询在整个索引过程运行一次。
* 执行 `sql_query_pre` 预查询以执行任何必要的初始设置，例如设置临时
  表或维护计数器表。这些查询在整个索引过程中运行一次。
* 执行主查询 `sql_query` 并处理它返回的行。
* 执行 `sql_query_post` 后查询以执行一些必要的清理。
* 关闭与数据库的连接。
* 索引器完成排序阶段（具体到表类型的后处理）。
* 再次建立与数据库的连接。
* 执行后处理查询 `sql_query_post_index` 以执行一些必要的最终清理。
* 再次关闭与数据库的连接。

从 MYSQL 获取数据的来源示例：

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

这是用于从 SQL 服务器检索文档的查询。只能声明一个 sql_query，并且必须有一个。另请参阅 [处理获取的数据](../../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#Processing-fetched-data)

## sql_query_pre

预取查询或预查询。这是一个多值的、可选的设置，默认是一个空的查询列表。预查询按它们在配置文件中出现的顺序在 sql_query 之前执行。预查询的结果被忽略。

预查询在许多方面都很有用。它们可以用于设置编码、标记将要索引的记录、更新内部计数器、设置各种每连接的 SQL 服务器选项和变量等等。

或许预查询最常见的用途是指定服务器将用于返回行的编码。注意，Manticore 仅接受 UTF-8 文本。设置编码的两个 MySQL 特定示例是：

```ini
sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
sql_query_pre = SET NAMES utf8
```

同样，对于 MySQL 来源，最好在预查询中禁用查询缓存（仅针对索引器连接），因为索引查询不大可能会经常重新运行，因此缓存其结果没有意义。
可以通过以下方式实现：

```ini
sql_query_pre = SET SESSION query_cache_type=OFF
```

## sql_query_post

后取查询。这是一个可选设置，默认值为空。

此查询在 sql_query 成功完成后立即执行。当后取查询产生错误时，会报告为警告，但不会终止索引。其结果集被忽略。注意，此时索引尚未完成，而进一步的索引可能仍会失败。因此，此处不应进行任何永久更新。例如，更新永久更改最后成功索引的 ID 的辅助表不应从 `sql_query_post` 查询中运行；它们应从 `sql_query_post_index` 查询中运行。

## sql_query_post_index

后处理查询。这是一个可选设置，默认值为空。

此查询在索引完全并成功完成后执行。如果此查询产生错误，会报告为警告，但不会终止索引。其结果集被忽略。可在其文本中使用 `$maxid` 宏；它将展开为索引过程中实际从数据库中获取的最大文档 ID。如果没有索引文档，`$maxid` 将展开为 0。

示例：
```ini
sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
```

`sql_query_post` 和 `sql_query_post_index` 的区别在于 `sql_query_post` 在 Manticore 接收到所有文档后立即运行，但由于某些其他原因，进一步的索引仍可能失败。相反，执行 `sql_query_post_index` 查询时，可以保证表已成功创建。数据库连接已断开并重新建立，因为排序阶段可能非常漫长，否则将会超时。

<!-- proofread -->
