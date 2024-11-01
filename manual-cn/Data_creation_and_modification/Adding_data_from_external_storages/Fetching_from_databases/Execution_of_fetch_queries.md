# 执行数据获取查询

使用所有SQL驱动程序时，构建普通表的过程通常如下：

- 建立与数据库的连接。
- 执行 `sql_query_pre_all` 查询以进行任何必要的初始设置，例如使用MySQL设置每个连接的编码。这些查询在整个索引过程中执行一次，并在重新连接后用于索引MVA属性和连接字段。
- 执行 `sql_query_pre` 预查询以进行任何必要的初始设置，例如设置临时表或维护计数器表。这些查询在整个索引过程中执行一次。
- 执行主查询 `sql_query` 并处理其返回的行。
- 执行 `sql_query_post` 后查询以进行必要的清理。
- 关闭与数据库的连接。
- 索引器执行排序阶段（准确地说，是表类型特定的后处理）。
- 再次建立与数据库的连接。
- 执行 `sql_query_post_index` 后处理查询以进行最终清理。
- 再次关闭与数据库的连接。

从MySQL获取数据的源示例如下：

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

这是用于从SQL服务器检索文档的查询。必须声明且只能声明一个 `sql_query`。详细信息请参阅 [处理获取的数据](../../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#Processing-fetched-data)

## sql_query_pre

预取查询或预查询。这是一个多值、可选设置，默认值为空列表。预查询在 `sql_query` 之前按其在配置文件中的顺序执行，预查询的结果会被忽略。

预查询非常有用，可以用于设置编码、标记将要被索引的记录、更新内部计数器、设置各种每连接SQL服务器选项和变量等。

预查询最常见的用途是指定服务器将用于返回行的编码。请注意，Manticore仅接受UTF-8文本。以下是设置编码的两个MySQL特定示例：

```ini
sql_query_pre = SET CHARACTER_SET_RESULTS=utf8
sql_query_pre = SET NAMES utf8
```

对于MySQL源，关闭查询缓存（仅对索引器连接有效）也是有用的，因为索引查询不会经常重新运行，缓存结果没有意义。这可以通过以下方式实现：

```ini
sql_query_pre = SET SESSION query_cache_type=OFF
```

## sql_query_post

后取查询。这是一个可选设置，默认值为空。

该查询在 `sql_query` 成功完成后立即执行。如果后取查询产生错误，会报告为警告，但不会终止索引，其结果集会被忽略。请注意，在此查询执行时，索引尚未完成，后续的索引过程可能仍会失败。因此，不应在此进行任何永久性更新。例如，不能在 `sql_query_post` 查询中更新最后成功索引的ID，而应在 `sql_query_post_index` 查询中运行这些更新。

## sql_query_post_index

后处理查询。这是一个可选设置，默认值为空。

该查询在索引完全成功完成时执行。如果此查询产生错误，会报告为警告，但不会终止索引，其结果集会被忽略。查询的文本中可以使用 `$maxid` 宏；它将扩展为索引期间从数据库实际获取的最大文档ID。如果没有索引文档，`$maxid` 将扩展为0。

示例：

```ini
sql_query_post_index = REPLACE INTO counters ( id, val ) \
    VALUES ( 'max_indexed_id', $maxid )
```

`sql_query_post` 和 `sql_query_post_index` 之间的区别在于，`sql_query_post` 是在Manticore接收到所有文档后立即运行的，但由于其他原因，进一步的索引仍可能失败。而在执行 `sql_query_post_index` 查询时，可以确保表已成功创建。由于排序阶段可能非常耗时，因此会关闭并重新建立数据库连接，否则连接可能会超时。

<!-- proofread -->
