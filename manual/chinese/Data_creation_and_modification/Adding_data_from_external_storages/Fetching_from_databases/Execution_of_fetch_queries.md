# 执行 fetch 查询

对于所有的 SQL 驱动，构建一个普通表通常按以下步骤进行。

* 建立与数据库的连接。
* 执行 `sql_query_pre_all` 查询以进行任何必要的初始设置，例如使用 MySQL 设置每个连接的编码。这些查询在整个索引过程之前运行，也在重新连接后用于索引 MVA 属性和连接字段。
* 执行 `sql_query_pre` 预查询以进行任何必要的初始设置，例如设置临时表或维护计数器表。这些查询在整个索引过程中只运行一次。
* 执行 `sql_query_pre` 预查询以进行任何必要的初始设置，例如设置临时
  表，或维护计数器表。这些查询在整个索引过程中只运行一次。
* 执行主查询 `sql_query` 并处理其返回的行。
* 执行后查询 `sql_query_post` 以进行一些必要的清理。
* 关闭与数据库的连接。
* 索引器执行排序阶段（严格来说，是表类型特定的后处理）。
* 再次建立与数据库的连接。
* 执行后处理查询 `sql_query_post_index` 以进行一些必要的最终清理。
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

这是用于从 SQL 服务器检索文档的查询。只能声明一个 sql_query，且必须有一个。另请参见 [处理获取的数据](../../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#Processing-fetched-data)

## sql_query_pre

预获取查询或预查询。这是一个多值的可选设置，默认是一个空的查询列表。预查询按照它们在配置文件中出现的顺序在 sql_query 之前执行。预查询的结果会被忽略。

预查询在很多方面都很有用。它们可以用于设置编码，标记将要被索引的记录，更新内部计数器，设置各种每连接的 SQL 服务器选项和变量，等等。

预查询最常见的用途可能是指定服务器将用于返回行的编码。请注意，Manticore 只接受 UTF-8 文本。两个 MySQL 特定的设置编码的示例如下：

```ini
sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
sql_query_pre = SET NAMES utf8
```

此外，针对 MySQL 源，在预查询中禁用查询缓存（仅针对索引器连接）是有用的，因为索引查询无论如何不会频繁重新运行，缓存它们的结果没有意义。
这可以通过以下方式实现：

```ini
sql_query_pre = SET SESSION query_cache_type=OFF
```

## sql_query_post

后获取查询。这是一个可选设置，默认值为空。

该查询在 sql_query 成功完成后立即执行。当后获取查询产生错误时，会作为警告报告，但不会终止索引。其结果集会被忽略。请注意，在执行此查询时索引尚未完成，后续索引仍可能失败。因此，不应从这里进行任何永久更新。例如，不应从 `sql_query_post` 查询中运行永久更改最后成功索引 ID 的辅助表更新；这些应从 `sql_query_post_index` 查询中运行。

## sql_query_post_index

后处理查询。这是一个可选设置，默认值为空。

该查询在索引完全且成功完成时执行。如果此查询产生错误，会作为警告报告，但不会终止索引。其结果集会被忽略。其文本中可以使用 `$maxid` 宏；它将被展开为索引期间实际从数据库获取的最大文档 ID。如果没有索引任何文档，`$maxid` 将被展开为 0。

示例：
```ini
sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
```

`sql_query_post` 和 `sql_query_post_index` 的区别在于，`sql_query_post` 在 Manticore 接收完所有文档后立即运行，但后续索引可能因其他原因失败。相反，当执行 `sql_query_post_index` 查询时，已保证表成功创建。数据库连接被断开并重新建立，因为排序阶段可能非常耗时，否则会超时。

<!-- proofread -->

