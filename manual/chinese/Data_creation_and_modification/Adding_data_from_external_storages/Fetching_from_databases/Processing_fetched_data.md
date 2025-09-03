# 处理获取的数据

默认情况下，`sql_query`结果集的第一列被索引为文档ID。

文档ID *必须* 是最首字段，且 **必须是** 唯一的有符号（非零）整数，范围从 -9223372036854775808 到 9223372036854775807。

你可以指定最多 256 个全文字段和任意数量的属性。所有既不是文档ID（第一列）也不是属性的列将被索引为全文字段。


## 属性声明：

### sql_attr_bigint

声明一个64位*有符号*整数。

### sql_attr_bool

声明一个布尔属性。等同于位数为1的整型属性。

### sql_attr_float

声明一个浮点属性。

值将以单精度、32位IEEE 754格式存储。表示范围大约从1e-38到1e+38。能精确存储的小数位数约为7位。

浮点属性的重要用途之一是存储经纬度值（弧度制），以便在查询时进行地球距离计算。

### sql_attr_json

声明一个JSON属性。

索引JSON属性时，Manticore期望文本字段包含JSON格式数据。JSON属性支持任意嵌套层级和类型的JSON数据。

### sql_attr_multi

声明一个多值属性。

普通属性每个文档只允许附加一个值。然而，有些情况下（如标签或分类）希望附加多个同属性的值，并能对值列表应用筛选或分组。

MVA可以从一列中取值（像其他数据类型一样）——这时，结果集中的该列必须提供以逗号分隔的多个整型值的字符串 —— 或通过执行单独的查询获取值。

执行查询时，引擎运行主查询，按ID分组结果，并将值赋给表中的对应文档。对于ID找不到的值会被丢弃。执行查询前，任何定义的 `sql_query_pre_all` 都将运行。

sql_attr_multi 的声明格式如下：

```ini
sql_attr_multi = ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE \
    [;QUERY] \
    [;RANGED-QUERY]
```

其中

* ATTR-TYPE 是 `uint`、`bigint` 或 `timestamp`。
* SOURCE-TYPE 是 `field`、`query`、`ranged-query` 或 `ranged-main-query`。
* QUERY 是可选的SQL查询，用于获取所有 (docid, attrvalue) 对。
* RANGED-QUERY 是可选的SQL查询，用于获取最小和最大ID值，类似 `sql_query_range`。
* 反斜杠仅为清晰起见；所有声明也可写成一行。

它用于 ranged-query 类型。如果使用 `ranged-main-query` 类型，则省略 RANGED-QUERY，会自动使用 `sql_query_range` 中相同的查询（在复杂继承结构中非常有用，避免多次重复相同查询）。

```ini
sql_attr_multi = uint tag from field
sql_attr_multi = uint tag from query; SELECT id, tag FROM tags
sql_attr_multi = bigint tag from ranged-query; \
    SELECT id, tag FROM tags WHERE id>=$start AND id<=$end; \
    SELECT MIN(id), MAX(id) FROM tags
```

### sql_attr_string

声明一个字符串属性。每个值的最大大小固定为4GB。

### sql_attr_timestamp

声明一个UNIX时间戳。

时间戳可以存储从1970年1月1日到2038年1月19日范围内的日期和时间，精确到秒。预期的列值应是UNIX格式时间戳，即从1970年1月1日午夜（GMT）起经过的秒数，32位无符号整数。时间戳内部在各处均以整数存储和处理。除了作为整数使用外，还能配合时间段排序或按天/周/月/年提取做GROUP BY。

注意，MySQL中DATE或DATETIME类型不能直接用作Manticore中的时间戳属性，必须用UNIX_TIMESTAMP函数显式转换（数据在范围内）。

注意：时间戳不能表示1970年1月1日之前的日期，MySQL的UNIX_TIMESTAMP()在此范围外不会返回预期结果。如果只需处理日期而非时间，请考虑使用MySQL的`TO_DAYS()`函数。


### sql_attr_uint

声明一个无符号整型属性。

可通过属性名后附加`:BITCOUNT`指定整型属性的位数（见下例）。位数小于默认32位，或位字段，性能较差。

```ini
sql_attr_uint = group_id
sql_attr_uint = forum_id:9 # 9 bits for forum_id
```

### sql_field_string

声明一个组合字符串属性/文本字段。该值将被索引为全文字段，同时以同名字符串属性存储。注意，仅当你确定希望字段既能全文搜索，也能作为属性（以便排序和分组）时使用。若只需获取字段原始值，除非你显式地从 [stored_fields](../../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) 中移除了字段，否则无需任何操作。

```ini
sql_field_string = name
```

### sql_file_field

声明基于文件的字段。

该指令使索引器将字段内容视为文件名，加载并处理引用的文件。大于 max_file_field_buffer 的文件会被跳过。加载时发生的任何错误（IO错误、超限等）将作为索引警告报告，且不会提前终止索引。此类文件不会索引任何内容。

```ini
sql_file_field = field_name
```

### sql_joined_field

联合/负载字段提取查询。多值，可选，默认是空查询列表。

`sql_joined_field`让你使用两种功能：联合字段和负载（payload）字段。语法如下：

```ini
sql_joined_field = FIELD-NAME 'from'  ( 'query' | 'payload-query' | 'ranged-query' | 'ranged-main-query' ); \
		QUERY [ ; RANGE-QUERY ]
```

其中

* FIELD-NAME 是联合/负载字段名称
* QUERY 是必须运行的用于获取值的SQL查询
* RANGE-QUERY 是可选的SQL查询，用于获取处理值的范围

**Joined fields** 允许您避免在主文档获取查询（sql_query）中使用 JOIN 和/或 GROUP_CONCAT 语句。当 SQL 端 JOIN 速度较慢、需要在 Manticore 端卸载，或仅仅为了模拟 MySQL 特有的 `GROUP_CONCAT` 功能（当数据库服务器不支持时）时，这非常有用。

查询必须返回恰好两列：文档 ID 和要追加到 joined 字段的文本。文档 ID 可以重复，但必须按升序排列。对于给定 ID 获取的所有文本行将被串联在一起，串联结果将作为整个 joined 字段的内容进行索引。行将按查询返回的顺序串联，并在它们之间插入分隔空白。例如, 如果 joined 字段查询返回以下行：
```ini
( 1, 'red' )
( 1, 'right' )
( 1, 'hand' )
( 2, 'mysql' )
( 2, 'manticore' )
```

那么索引结果等同于向文档 1 添加一个值为 'red right hand' 的新文本字段，向文档 2 添加一个值为 'mysql sphinx' 的新文本字段，其中包含关键字在字段内的顺序位置。如果行需要特定顺序，则必须在查询中明确定义。

joined 字段仅以不同方式进行索引。joined 字段和通用文本字段之间没有其他区别。

在执行 joined fields 查询之前，任何一组 `sql_query_pre_all` 会被执行（如果存在）。这允许您在 joined fields 的上下文中设置所需编码等。

当单个查询不够高效或者因为数据库驱动限制而无法工作时，可以使用范围查询。它的工作方式类似于主索引循环中的范围查询。范围将被一次性查询并预先获取，然后将运行多个带有不同 `$start` 和 `$end` 替换的查询以获取实际数据。

使用 `ranged-main-query` 查询时，省略 `ranged-query`，它会自动使用来自 `sql_query_range` 的相同查询（这是复杂继承设置中节省手动重复相同查询的有用选项）。

<!-- example payload -->
**Payloads** 允许您创建一个特殊字段，其中存储的不是关键字位置，而是所谓的用户负载。负载是附加到每个关键字的自定义整数值。在搜索时，它们可用于影响排名。

负载查询必须返回恰好三列：
- 文档 ID
- 关键字
- 以及整数负载值。

文档 ID 可以重复，但必须按升序排列。负载 **必须** 是 24 位无符号整数，即从 0 到 16777215。

唯一考虑负载的排名器是 `proximity_bm25`（默认的[排名器](../../../Searching/Sorting_and_ranking.md#Available-built-in-rankers)）。对于带有负载字段的表，它将自动切换到匹配这些字段中关键字的变体，计算匹配负载乘以字段权重的总和， 并将该总和加到最终排名中。

请注意，对于包含复杂操作符的全文查询，负载字段将被忽略。它仅适用于简单的词袋查询。

<!-- intro -->
配置示例：
<!-- request Configuration file -->
```ini
source min {
    type = mysql
    sql_host = localhost
    sql_user = test
    sql_pass =
    sql_db = test
    sql_query = select 1, 'Nike bag' f \
    UNION select 2, 'Adidas bag' f \
    UNION select 3, 'Reebok bag' f \
    UNION select 4, 'Nike belt' f

    sql_joined_field = tag from payload-query; select 1 id, 'nike' tag, 10 weight \
    UNION select 4 id, 'nike' tag, 10 weight;
}

index idx {
    path = idx
    source = min
}
```

<!-- request Just SELECT -->
```sql
mysql> select * from idx;
+------+------------+------+
| id   | f          | tag  |
+------+------------+------+
|    1 | Nike bag   | nike |
|    2 | Adidas bag |      |
|    3 | Reebok bag |      |
|    4 | Nike belt  | nike |
+------+------------+------+
4 rows in set (0.00 sec)
```

<!-- request Full-text search -->

注意，当您搜索 `nike | adidas` 时，包含 "nike" 的结果由于 "nike" 标签及其来自负载查询的权重而获得更高权重。
```sql
mysql> select *, weight() from idx where match('nike|adidas');
+------+------------+------+----------+
| id   | f          | tag  | weight() |
+------+------------+------+----------+
|    1 | Nike bag   | nike |    11539 |
|    4 | Nike belt  | nike |    11539 |
|    2 | Adidas bag |      |     1597 |
+------+------------+------+----------+
3 rows in set (0.01 sec)
```

<!-- request Complex full-text search -->

请注意，特殊负载字段在包含复杂操作符的全文查询中被忽略。它仅适用于简单的词袋查询。

```sql
mysql> select *, weight() from idx where match('"nike bag"|"adidas bag"');
+------+------------+------+----------+
| id   | f          | tag  | weight() |
+------+------------+------+----------+
|    2 | Adidas bag |      |     2565 |
|    1 | Nike bag   | nike |     2507 |
+------+------------+------+----------+
2 rows in set (0.00 sec)
```

<!-- end -->

### sql_column_buffers

```ini
sql_column_buffers = <colname>=<size>[K|M] [, ...]
```

每列缓冲区大小。可选，默认为空（自动推断大小）。仅适用于 `odbc`，`mssql` 数据源类型。

ODBC 和 MS SQL 驱动有时无法返回预期的最大实际列大小。例如，`NVARCHAR(MAX)` 列即使实际使用的长度可能小得多，也总是向 `indexer` 报告长度为 2147483647 字节。然而，接收缓冲区仍需预先分配，且缓冲区大小必须确定。当驱动完全不报告列长度时，Manticore 为每个非字符列分配默认的 1 KB 缓冲区，为每个字符列分配 1 MB 缓冲区。驱动报告的列长度还会被 8 MB 的上限限制，因此如果驱动报告了（几乎）2 GB 的列长度，它将被限制住，转而为该列分配 8 MB 缓冲区。可以使用 `sql_column_buffers` 指令覆盖这些硬编码限制，以节省实际较短列的内存或突破实际较长列的 8 MB 限制。该指令值必须是逗号分隔的所选列名和大小列表：

示例：

```ini
sql_query = SELECT id, mytitle, mycontent FROM documents
sql_column_buffers = mytitle=64K, mycontent=10M
```
<!-- proofread -->

